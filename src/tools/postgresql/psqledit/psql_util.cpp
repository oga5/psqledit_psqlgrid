/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "mbutil.h"
#include "ostrutil.h"

static struct {
	int		encoding;
} pset;

static int PQmblen(const TCHAR *str, int encoding)
{
	return get_char_len(str);
}

static int pg_strncasecmp(const TCHAR *s1, const TCHAR *s2, size_t n)
{
	return ostr_strncmp_nocase(s1, s2, (int)n);
}

/*
 * Advance the given char pointer over white space and SQL comments.
 */
static const TCHAR *
skip_white_space(const TCHAR *query)
{
    int         cnestlevel = 0; /* slash-star comment nest level */

    while (*query)
    {
        int         mblen = PQmblen(query, pset.encoding);

        /*
         * Note: we assume the encoding is a superset of ASCII, so that for
         * example "query[0] == '/'" is meaningful.  However, we do NOT assume
         * that the second and subsequent bytes of a multibyte character
         * couldn't look like ASCII characters; so it is critical to advance
         * by mblen, not 1, whenever we haven't exactly identified the
         * character we are skipping over.
         */
        if (isspace(*query))
            query += mblen;
        else if (query[0] == '/' && query[1] == '*')
        {
            cnestlevel++;
            query += 2;
        }
        else if (cnestlevel > 0 && query[0] == '*' && query[1] == '/')
        {
            cnestlevel--;
            query += 2;
        }
        else if (cnestlevel == 0 && query[0] == '-' && query[1] == '-')
        {
            query += 2;

            /*
             * We have to skip to end of line since any slash-star inside the
             * -- comment does NOT start a slash-star comment.
             */
            while (*query)
            {
                if (*query == '\n')
                {
                    query++;
                    break;
                }
                query += PQmblen(query, pset.encoding);
            }
        }
        else if (cnestlevel > 0)
            query += mblen;
        else
            break;              /* found first token */
    }

    return query;
}

// FIXME: 最新のpostgesコードから取得する？
bool command_no_begin(const TCHAR *query)
{
    int         wordlen;

    /*
     * First we must advance over any whitespace and comments.
     */
    query = skip_white_space(query);

    /*
     * Check word length (since "beginx" is not "begin").
     */
    wordlen = 0;
    while (isalpha(query[wordlen]))
        wordlen += PQmblen(&query[wordlen], pset.encoding);

    /*
     * Transaction control commands.  These should include every keyword that
     * gives rise to a TransactionStmt in the backend grammar, except for the
     * savepoint-related commands.
     *
     * (We assume that START must be START TRANSACTION, since there is
     * presently no other "START foo" command.)
     */
    if (wordlen == 5 && pg_strncasecmp(query, _T("abort"), 5) == 0)
        return true;
    if (wordlen == 5 && pg_strncasecmp(query, _T("begin"), 5) == 0)
        return true;
    if (wordlen == 5 && pg_strncasecmp(query, _T("start"), 5) == 0)
        return true;
    if (wordlen == 6 && pg_strncasecmp(query, _T("commit"), 6) == 0)
        return true;
    if (wordlen == 3 && pg_strncasecmp(query, _T("end"), 3) == 0)
        return true;
    if (wordlen == 8 && pg_strncasecmp(query, _T("rollback"), 8) == 0)
        return true;
    if (wordlen == 7 && pg_strncasecmp(query, _T("prepare"), 7) == 0)
    {
        /* PREPARE TRANSACTION is a TC command, PREPARE foo is not */
        query += wordlen;

        query = skip_white_space(query);

        wordlen = 0;
        while (isalpha((unsigned char) query[wordlen]))
            wordlen += PQmblen(&query[wordlen], pset.encoding);

        if (wordlen == 11 && pg_strncasecmp(query, _T("transaction"), 11) == 0)
            return true;
        return false;
    }

    /*
     * Commands not allowed within transactions.  The statements checked for
     * here should be exactly those that call PreventTransactionChain() in the
     * backend.
     */
    if (wordlen == 6 && pg_strncasecmp(query, _T("vacuum"), 6) == 0)
        return true;
    if (wordlen == 7 && pg_strncasecmp(query, _T("cluster"), 7) == 0)
    {
        /* CLUSTER with any arguments is allowed in transactions */
        query += wordlen;

        query = skip_white_space(query);

        if (isalpha((unsigned char) query[0]))
            return false;       /* has additional words */
        return true;            /* it's CLUSTER without arguments */
    }

    if (wordlen == 6 && pg_strncasecmp(query, _T("create"), 6) == 0)
    {
        query += wordlen;

        query = skip_white_space(query);

        wordlen = 0;
        while (isalpha((unsigned char) query[wordlen]))
            wordlen += PQmblen(&query[wordlen], pset.encoding);

        if (wordlen == 8 && pg_strncasecmp(query, _T("database"), 8) == 0)
            return true;
        if (wordlen == 10 && pg_strncasecmp(query, _T("tablespace"), 10) == 0)
            return true;

        /* CREATE [UNIQUE] INDEX CONCURRENTLY isn't allowed in xacts */
        if (wordlen == 6 && pg_strncasecmp(query, _T("unique"), 6) == 0)
        {
            query += wordlen;

            query = skip_white_space(query);
            wordlen = 0;
            while (isalpha((unsigned char) query[wordlen]))
                wordlen += PQmblen(&query[wordlen], pset.encoding);
        }

        if (wordlen == 5 && pg_strncasecmp(query, _T("index"), 5) == 0)
        {
            query += wordlen;

            query = skip_white_space(query);

            wordlen = 0;
            while (isalpha((unsigned char) query[wordlen]))
                wordlen += PQmblen(&query[wordlen], pset.encoding);

            if (wordlen == 12 && pg_strncasecmp(query, _T("concurrently"), 12) == 0)
                return true;
        }

        return false;
    }

    /*
     * Note: these tests will match DROP SYSTEM and REINDEX TABLESPACE, which
     * aren't really valid commands so we don't care much. The other four
     * possible matches are correct.
     */
    if ((wordlen == 4 && pg_strncasecmp(query, _T("drop"), 4) == 0) ||
        (wordlen == 7 && pg_strncasecmp(query, _T("reindex"), 7) == 0))
    {
        query += wordlen;

        query = skip_white_space(query);

        wordlen = 0;
        while (isalpha((unsigned char) query[wordlen]))
            wordlen += PQmblen(&query[wordlen], pset.encoding);

        if (wordlen == 8 && pg_strncasecmp(query, _T("database"), 8) == 0)
            return true;
        if (wordlen == 6 && pg_strncasecmp(query, _T("system"), 6) == 0)
            return true;
        if (wordlen == 10 && pg_strncasecmp(query, _T("tablespace"), 10) == 0)
            return true;
    }

    return false;
}
