/*
 * Copyright (c) 2025 Atsushi Ogawa
 *
 * Some functions are modified or adapted from PostgreSQL source code.
 * Refer to PostgreSQL License for details. See LICENSE_PostgreSQL file.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/* The following lines contain the original credits from PostgreSQL. */
/*
PostgreSQL Database Management System
(formerly known as Postgres, then as Postgres95)

Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group

Portions Copyright (c) 1994, The Regents of the University of California

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose, without fee, and without a written agreement
is hereby granted, provided that the above copyright notice and this
paragraph and the following two paragraphs appear in all copies.

IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATIONS TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <stdafx.h>
#include "pglib.h"
#include "pgmsg.h"

#include "localsql.h"
#include "editdata.h"
#include "mbutil.h"
#include "ostrutil.h"

// memo
// pg_dump.cから移植
//  - getTables()のSQLを使って、テーブル情報を取得
//  - dumpTableSchema()の処理を移植する

int dumpTableSchema(HPgSession ss, const TCHAR *oid, CEditData *edit_data, TCHAR *msg_buf);

// postgres_ext.h
#define InvalidOid      0
typedef unsigned int Oid;
// c.h
#define OidIsValid(objectId)  ((bool) ((objectId) != InvalidOid))

// pg_class.h
#define       RELKIND_RELATION        'r'       /* ordinary table */
#define       RELKIND_INDEX           'i'       /* secondary index */
#define       RELKIND_SEQUENCE        'S'       /* sequence object */
#define       RELKIND_TOASTVALUE      't'       /* for out-of-line values */
#define       RELKIND_VIEW            'v'       /* view */
#define       RELKIND_COMPOSITE_TYPE  'c'       /* composite type */
#define       RELKIND_FOREIGN_TABLE   'f'       /* foreign table */
#define       RELKIND_UNCATALOGED     'u'       /* not yet cataloged */
#define       RELKIND_PARTITIONED_TABLE     'p'       /* not yet cataloged */

#define       RELPERSISTENCE_PERMANENT  'p'     /* regular table */
#define       RELPERSISTENCE_UNLOGGED   'u'     /* unlogged permanent table */
#define       RELPERSISTENCE_TEMP       't'     /* temporary table */


static const TCHAR *username_subquery;
static bool std_strings = false;
static int  no_security_labels = 0;

const TCHAR *fmtId(const TCHAR *rawid)
{
	static TCHAR buf[1024];
	const TCHAR *cp;
	bool need_quotes = false;

	_tcscpy(buf, _T(""));

	/* slightly different rules for first character */
	if (!((rawid[0] >= 'a' && rawid[0] <= 'z') || rawid[0] == '_')) {
		need_quotes = true;
	} else {
		/* otherwise check the entire string */
		for (cp = rawid; *cp; cp++) {
			if (!((*cp >= 'a' && *cp <= 'z') || (*cp >= '0' && *cp <= '9') || (*cp == '_'))) {
				need_quotes = true;
				break;
			}
		}
	}

// FIXME: keyword checkするか？
//	if (!need_quotes)
//	{
//		/*
//		* Check for keyword.  We quote keywords except for unreserved ones.
//		* (In some cases we could avoid quoting a col_name or type_func_name
//		* keyword, but it seems much harder than it's worth to tell that.)
//		*
//		* Note: ScanKeywordLookup() does case-insensitive comparison, but
//		* that's fine, since we already know we have all-lower-case.
//		*/
//		const ScanKeyword *keyword = ScanKeywordLookup(rawid,
//			FEScanKeywords,
//			NumFEScanKeywords);
//
//		if (keyword != NULL && keyword->category != UNRESERVED_KEYWORD)
//			need_quotes = true;
//	}

	if (!need_quotes) {
		return rawid;
	} else {
		TCHAR *p = buf;
		*p = '\"';
		p++;

		for (cp = rawid; *cp; cp++) {
			/*
			* Did we find a double-quote in the string? Then make this a
			* double double-quote per SQL99. Before, we put in a
			* backslash/double-quote pair. - thomas 2000-08-05
			*/
			if (*cp == '\"') {
				*p = '\"';
				p++;
			}
			*p = *cp;
			p++;
		}
		*p = '\"';
		p++;
		*p = '\0';
	}

	return buf;
}

static int selectSourceSchema(HPgSession ss, const TCHAR *schemaName, TCHAR *msg_buf)
{
	static TCHAR *curSchemaName = NULL;
	TCHAR	sql_buf[4096];
	int		pg_version = pg_get_remote_version(ss);
	int		ret_v;

	/* Not relevant if fetching from pre-7.3 DB */
	if (pg_version < 70300)
		return 0;
	/* Ignore null schema names */
	if (schemaName == NULL || *schemaName == '\0')
		return 0;
	/* Optimize away repeated selection of same schema */
	if (curSchemaName && _tcscmp(curSchemaName, schemaName) == 0)
		return 0;

	if (curSchemaName) {
		free(curSchemaName);
		curSchemaName = NULL;
	}

	_stprintf(sql_buf, _T("SET search_path = %s"), fmtId(schemaName));
	if (_tcscmp(schemaName, _T("pg_catalog")) != 0)
		_tcscat(sql_buf, _T(", pg_catalog"));

	ret_v = pg_exec_sql(ss, sql_buf, msg_buf);
	if(ret_v != 0) return ret_v;

	curSchemaName = _tcsdup(schemaName);

	return 0;
}

static int getTableInfo(HPgSession ss, const TCHAR *oid, HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR sql_buf[4096];
	int pg_version = pg_get_remote_version(ss);
	int ret_v;

	/* Make sure we are in proper schema */
	ret_v = selectSourceSchema(ss, _T("pg_catalog"), msg_buf);
	if(ret_v != 0) return ret_v;

	/* Select the appropriate subquery to convert user IDs to names */
	if (pg_version >= 80100)
		username_subquery = _T("SELECT rolname FROM pg_catalog.pg_roles WHERE oid =");
	else if (pg_version >= 70300)
		username_subquery = _T("SELECT usename FROM pg_catalog.pg_user WHERE usesysid =");
	else
		username_subquery = _T("SELECT usename FROM pg_user WHERE usesysid =");

	// pg_dump.cのgetTables()のSQLを使う
	// ^\s+\"(.+)\" -> \t\t\t_T("\1")で、UNICODE Textに変換
	if(pg_version >= 120000) {
		_stprintf(sql_buf,
			_T("SELECT c.tableoid, c.oid, c.relname, ")
			_T("c.relacl, c.relkind, c.relnamespace, ")
			_T("(%s c.relowner) AS rolname, ")
			_T("c.relchecks, c.relhastriggers, ")
			_T("c.relhasindex, c.relhasrules, false relhasoids, ")
			_T("c.relfrozenxid, tc.oid AS toid, ")
			_T("tc.relfrozenxid AS tfrozenxid, ")
			_T("c.relpersistence, ")
			_T("CASE WHEN c.reloftype <> 0 THEN c.reloftype::pg_catalog.regtype ELSE NULL END AS reloftype, ")
			_T("d.refobjid AS owning_tab, ")
			_T("d.refobjsubid AS owning_col, ")
			_T("(SELECT spcname FROM pg_tablespace t WHERE t.oid = c.reltablespace) AS reltablespace, ")
			_T("array_to_string(c.reloptions, ', ') AS reloptions, ")
			_T("array_to_string(array(SELECT 'toast.' || x FROM unnest(tc.reloptions) x), ', ') AS toast_reloptions ")
			_T("FROM pg_class c ")
			_T("LEFT JOIN pg_depend d ON ")
			_T("(c.relkind = '%c' AND ")
			_T("d.classid = c.tableoid AND d.objid = c.oid AND ")
			_T("d.objsubid = 0 AND ")
			_T("d.refclassid = c.tableoid AND d.deptype = 'a') ")
			_T("LEFT JOIN pg_class tc ON (c.reltoastrelid = tc.oid) ")
			_T("WHERE c.relkind in ('%c', '%c', '%c', '%c', '%c', '%c') ")
			_T("AND c.oid = '%s'::pg_catalog.oid "),
			username_subquery,
			RELKIND_SEQUENCE,
			RELKIND_RELATION, RELKIND_SEQUENCE,
			RELKIND_VIEW, RELKIND_COMPOSITE_TYPE,
			RELKIND_FOREIGN_TABLE, RELKIND_PARTITIONED_TABLE, oid);
	} else if (pg_version >= 90100)
	{
		/*
		 * Left join to pick up dependency info linking sequences to their
		 * owning column, if any (note this dependency is AUTO as of 8.2)
		 */
		_stprintf(sql_buf,
			_T("SELECT c.tableoid, c.oid, c.relname, ")
			_T("c.relacl, c.relkind, c.relnamespace, ")
			_T("(%s c.relowner) AS rolname, ")
			_T("c.relchecks, c.relhastriggers, ")
			_T("c.relhasindex, c.relhasrules, c.relhasoids, ")
			_T("c.relfrozenxid, tc.oid AS toid, ")
			_T("tc.relfrozenxid AS tfrozenxid, ")
			_T("c.relpersistence, ")
			_T("CASE WHEN c.reloftype <> 0 THEN c.reloftype::pg_catalog.regtype ELSE NULL END AS reloftype, ")
			_T("d.refobjid AS owning_tab, ")
			_T("d.refobjsubid AS owning_col, ")
			_T("(SELECT spcname FROM pg_tablespace t WHERE t.oid = c.reltablespace) AS reltablespace, ")
			_T("array_to_string(c.reloptions, ', ') AS reloptions, ")
			_T("array_to_string(array(SELECT 'toast.' || x FROM unnest(tc.reloptions) x), ', ') AS toast_reloptions ")
			_T("FROM pg_class c ")
			_T("LEFT JOIN pg_depend d ON ")
			_T("(c.relkind = '%c' AND ")
			_T("d.classid = c.tableoid AND d.objid = c.oid AND ")
			_T("d.objsubid = 0 AND ")
			_T("d.refclassid = c.tableoid AND d.deptype = 'a') ")
			_T("LEFT JOIN pg_class tc ON (c.reltoastrelid = tc.oid) ")
			_T("WHERE c.relkind in ('%c', '%c', '%c', '%c', '%c') ")
			_T("AND c.oid = '%s'::pg_catalog.oid "),
			username_subquery,
			RELKIND_SEQUENCE,
			RELKIND_RELATION, RELKIND_SEQUENCE,
			RELKIND_VIEW, RELKIND_COMPOSITE_TYPE,
			RELKIND_FOREIGN_TABLE, oid);
	}
	else if (pg_version >= 90000)
	{
		/*
		 * Left join to pick up dependency info linking sequences to their
		 * owning column, if any (note this dependency is AUTO as of 8.2)
		 */
		_stprintf(sql_buf,
			_T("SELECT c.tableoid, c.oid, c.relname, ")
			_T("c.relacl, c.relkind, c.relnamespace, ")
			_T("(%s c.relowner) AS rolname, ")
			_T("c.relchecks, c.relhastriggers, ")
			_T("c.relhasindex, c.relhasrules, c.relhasoids, ")
			_T("c.relfrozenxid, tc.oid AS toid, ")
			_T("tc.relfrozenxid AS tfrozenxid, ")
			_T("'p' AS relpersistence, ")
			_T("CASE WHEN c.reloftype <> 0 THEN c.reloftype::pg_catalog.regtype ELSE NULL END AS reloftype, ")
			_T("d.refobjid AS owning_tab, ")
			_T("d.refobjsubid AS owning_col, ")
			_T("(SELECT spcname FROM pg_tablespace t WHERE t.oid = c.reltablespace) AS reltablespace, ")
			_T("array_to_string(c.reloptions, ', ') AS reloptions, ")
			_T("array_to_string(array(SELECT 'toast.' || x FROM unnest(tc.reloptions) x), ', ') AS toast_reloptions ")
			_T("FROM pg_class c ")
			_T("LEFT JOIN pg_depend d ON ")
			_T("(c.relkind = '%c' AND ")
			_T("d.classid = c.tableoid AND d.objid = c.oid AND ")
			_T("d.objsubid = 0 AND ")
			_T("d.refclassid = c.tableoid AND d.deptype = 'a') ")
			_T("LEFT JOIN pg_class tc ON (c.reltoastrelid = tc.oid) ")
			_T("WHERE c.relkind in ('%c', '%c', '%c', '%c') ")
			_T("AND c.oid = '%s'::pg_catalog.oid "),
			username_subquery,
			RELKIND_SEQUENCE,
			RELKIND_RELATION, RELKIND_SEQUENCE,
			RELKIND_VIEW, RELKIND_COMPOSITE_TYPE, oid);
	}
	else if (pg_version >= 80400)
	{
		/*
		 * Left join to pick up dependency info linking sequences to their
		 * owning column, if any (note this dependency is AUTO as of 8.2)
		 */
		_stprintf(sql_buf,
			_T("SELECT c.tableoid, c.oid, c.relname, ")
			_T("c.relacl, c.relkind, c.relnamespace, ")
			_T("(%s c.relowner) AS rolname, ")
			_T("c.relchecks, c.relhastriggers, ")
			_T("c.relhasindex, c.relhasrules, c.relhasoids, ")
			_T("c.relfrozenxid, tc.oid AS toid, ")
			_T("tc.relfrozenxid AS tfrozenxid, ")
			_T("'p' AS relpersistence, ")
			_T("NULL AS reloftype, ")
			_T("d.refobjid AS owning_tab, ")
			_T("d.refobjsubid AS owning_col, ")
			_T("(SELECT spcname FROM pg_tablespace t WHERE t.oid = c.reltablespace) AS reltablespace, ")
			_T("array_to_string(c.reloptions, ', ') AS reloptions, ")
			_T("array_to_string(array(SELECT 'toast.' || x FROM unnest(tc.reloptions) x), ', ') AS toast_reloptions ")
			_T("FROM pg_class c ")
			_T("LEFT JOIN pg_depend d ON ")
			_T("(c.relkind = '%c' AND ")
			_T("d.classid = c.tableoid AND d.objid = c.oid AND ")
			_T("d.objsubid = 0 AND ")
			_T("d.refclassid = c.tableoid AND d.deptype = 'a') ")
			_T("LEFT JOIN pg_class tc ON (c.reltoastrelid = tc.oid) ")
			_T("WHERE c.relkind in ('%c', '%c', '%c', '%c') ")
			_T("AND c.oid = '%s'::pg_catalog.oid "),
			username_subquery,
			RELKIND_SEQUENCE,
			RELKIND_RELATION, RELKIND_SEQUENCE,
			RELKIND_VIEW, RELKIND_COMPOSITE_TYPE, oid);
	}
	else if (pg_version >= 80200)
	{
		/*
		 * Left join to pick up dependency info linking sequences to their
		 * owning column, if any (note this dependency is AUTO as of 8.2)
		 */
		_stprintf(sql_buf,
			_T("SELECT c.tableoid, c.oid, c.relname, ")
			_T("c.relacl, c.relkind, c.relnamespace, ")
			_T("(%s c.relowner) AS rolname, ")
			_T("c.relchecks, (c.reltriggers <> 0) AS relhastriggers, ")
			_T("c.relhasindex, c.relhasrules, c.relhasoids, ")
			_T("c.relfrozenxid, tc.oid AS toid, ")
			_T("tc.relfrozenxid AS tfrozenxid, ")
			_T("'p' AS relpersistence, ")
			_T("NULL AS reloftype, ")
			_T("d.refobjid AS owning_tab, ")
			_T("d.refobjsubid AS owning_col, ")
			_T("(SELECT spcname FROM pg_tablespace t WHERE t.oid = c.reltablespace) AS reltablespace, ")
			_T("array_to_string(c.reloptions, ', ') AS reloptions, ")
			_T("NULL AS toast_reloptions ")
			_T("FROM pg_class c ")
			_T("LEFT JOIN pg_depend d ON ")
			_T("(c.relkind = '%c' AND ")
			_T("d.classid = c.tableoid AND d.objid = c.oid AND ")
			_T("d.objsubid = 0 AND ")
			_T("d.refclassid = c.tableoid AND d.deptype = 'a') ")
			_T("LEFT JOIN pg_class tc ON (c.reltoastrelid = tc.oid) ")
			_T("WHERE c.relkind in ('%c', '%c', '%c', '%c') ")
			_T("AND c.oid = '%s'::pg_catalog.oid "),
			username_subquery,
			RELKIND_SEQUENCE,
			RELKIND_RELATION, RELKIND_SEQUENCE,
			RELKIND_VIEW, RELKIND_COMPOSITE_TYPE, oid);
	}
	else if (pg_version >= 80000)
	{
		/*
		 * Left join to pick up dependency info linking sequences to their
		 * owning column, if any
		 */
		_stprintf(sql_buf,
			_T("SELECT c.tableoid, c.oid, relname, ")
			_T("relacl, relkind, relnamespace, ")
			_T("(%s relowner) AS rolname, ")
			_T("relchecks, (reltriggers <> 0) AS relhastriggers, ")
			_T("relhasindex, relhasrules, relhasoids, ")
			_T("0 AS relfrozenxid, ")
			_T("0 AS toid, ")
			_T("0 AS tfrozenxid, ")
			_T("'p' AS relpersistence, ")
			_T("NULL AS reloftype, ")
			_T("d.refobjid AS owning_tab, ")
			_T("d.refobjsubid AS owning_col, ")
			_T("(SELECT spcname FROM pg_tablespace t WHERE t.oid = c.reltablespace) AS reltablespace, ")
			_T("NULL AS reloptions, ")
			_T("NULL AS toast_reloptions ")
			_T("FROM pg_class c ")
			_T("LEFT JOIN pg_depend d ON ")
			_T("(c.relkind = '%c' AND ")
			_T("d.classid = c.tableoid AND d.objid = c.oid AND ")
			_T("d.objsubid = 0 AND ")
			_T("d.refclassid = c.tableoid AND d.deptype = 'i') ")
			_T("WHERE relkind in ('%c', '%c', '%c', '%c') ")
			_T("AND c.oid = '%s'::pg_catalog.oid "),
			username_subquery,
			RELKIND_SEQUENCE,
			RELKIND_RELATION, RELKIND_SEQUENCE,
			RELKIND_VIEW, RELKIND_COMPOSITE_TYPE, oid);
	}
	else if (pg_version >= 70300)
	{
		/*
		 * Left join to pick up dependency info linking sequences to their
		 * owning column, if any
		 */
		_stprintf(sql_buf,
			_T("SELECT c.tableoid, c.oid, relname, ")
			_T("relacl, relkind, relnamespace, ")
			_T("(%s relowner) AS rolname, ")
			_T("relchecks, (reltriggers <> 0) AS relhastriggers, ")
			_T("relhasindex, relhasrules, relhasoids, ")
			_T("0 AS relfrozenxid, ")
			_T("0 AS toid, ")
			_T("0 AS tfrozenxid, ")
			_T("'p' AS relpersistence, ")
			_T("NULL AS reloftype, ")
			_T("d.refobjid AS owning_tab, ")
			_T("d.refobjsubid AS owning_col, ")
			_T("NULL AS reltablespace, ")
			_T("NULL AS reloptions, ")
			_T("NULL AS toast_reloptions ")
			_T("FROM pg_class c ")
			_T("LEFT JOIN pg_depend d ON ")
			_T("(c.relkind = '%c' AND ")
			_T("d.classid = c.tableoid AND d.objid = c.oid AND ")
			_T("d.objsubid = 0 AND ")
			_T("d.refclassid = c.tableoid AND d.deptype = 'i') ")
			_T("WHERE relkind IN ('%c', '%c', '%c', '%c') ")
			_T("AND c.oid = '%s'::pg_catalog.oid "),
			username_subquery,
			RELKIND_SEQUENCE,
			RELKIND_RELATION, RELKIND_SEQUENCE,
			RELKIND_VIEW, RELKIND_COMPOSITE_TYPE, oid);
	}
	else
	{
		// FIXME: エラーメッセージ
		return 1;
	}

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int getInhParents(HPgSession ss, const TCHAR *oid, HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR sql_buf[4096];

	_stprintf(sql_buf,
		_T("select inhrelid, inhparent, \n")
		_T("	(select c.relname \n")
		_T("	 from pg_catalog.pg_class c \n")
		_T("	 where c.oid = i.inhparent) parent_rel_name, \n")
		_T("	(select n.nspname  \n")
		_T("	 from pg_catalog.pg_class c, pg_catalog.pg_namespace n  \n")
		_T("	 where n.oid = c.relnamespace \n")
		_T("	 and c.oid = i.inhparent) parent_namespace_name \n")
		_T("from pg_inherits i \n")
		_T("where inhrelid = '%s'::pg_catalog.oid \n"),
		oid);
	
	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int getTableAttrs(HPgSession ss, const TCHAR *oid, HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR sql_buf[4096];
	int		pg_version = pg_get_remote_version(ss);

	if (pg_version >= 90200) {
		/*
		* attfdwoptions is new in 9.2.
		*/
		_stprintf(sql_buf,
			_T("SELECT a.attnum, a.attname, a.atttypmod, ")
			_T("a.attstattarget, a.attstorage, t.typstorage, ")
			_T("a.attnotnull, a.atthasdef, a.attisdropped, ")
			_T("a.attlen, a.attalign, a.attislocal, ")
			_T("pg_catalog.format_type(t.oid,a.atttypmod) AS atttypname, ")
			_T("array_to_string(a.attoptions, ', ') AS attoptions, ")
			_T("CASE WHEN a.attcollation <> t.typcollation ")
			_T("THEN a.attcollation ELSE 0 END AS attcollation, ")
			_T("pg_catalog.array_to_string(ARRAY(")
			_T("SELECT pg_catalog.quote_ident(option_name) || ")
			_T("' ' || pg_catalog.quote_literal(option_value) ")
			_T("FROM pg_catalog.pg_options_to_table(attfdwoptions) ")
			_T("ORDER BY option_name")
			_T("), E',\n    ') AS attfdwoptions ")
			_T("FROM pg_catalog.pg_attribute a LEFT JOIN pg_catalog.pg_type t ")
			_T("ON a.atttypid = t.oid ")
			_T("WHERE a.attrelid = '%s'::pg_catalog.oid ")
			_T("AND a.attnum > 0::pg_catalog.int2 ")
			_T("ORDER BY a.attrelid, a.attnum"),
			oid);
	} else if (pg_version >= 90100) {
		/*
		* attcollation is new in 9.1.  Since we only want to dump COLLATE
		* clauses for attributes whose collation is different from their
		* type's default, we use a CASE here to suppress uninteresting
		* attcollations cheaply.
		*/
		_stprintf(sql_buf,
			_T("SELECT a.attnum, a.attname, a.atttypmod, ")
			_T("a.attstattarget, a.attstorage, t.typstorage, ")
			_T("a.attnotnull, a.atthasdef, a.attisdropped, ")
			_T("a.attlen, a.attalign, a.attislocal, ")
			_T("pg_catalog.format_type(t.oid,a.atttypmod) AS atttypname, ")
			_T("array_to_string(a.attoptions, ', ') AS attoptions, ")
			_T("CASE WHEN a.attcollation <> t.typcollation ")
			_T("THEN a.attcollation ELSE 0 END AS attcollation, ")
			_T("NULL AS attfdwoptions ")
			_T("FROM pg_catalog.pg_attribute a LEFT JOIN pg_catalog.pg_type t ")
			_T("ON a.atttypid = t.oid ")
			_T("WHERE a.attrelid = '%s'::pg_catalog.oid ")
			_T("AND a.attnum > 0::pg_catalog.int2 ")
			_T("ORDER BY a.attrelid, a.attnum"),
			oid);
	} else if (pg_version >= 90000) {
		/* attoptions is new in 9.0 */
		_stprintf(sql_buf,
			_T("SELECT a.attnum, a.attname, a.atttypmod, ")
			_T("a.attstattarget, a.attstorage, t.typstorage, ")
			_T("a.attnotnull, a.atthasdef, a.attisdropped, ")
			_T("a.attlen, a.attalign, a.attislocal, ")
			_T("pg_catalog.format_type(t.oid,a.atttypmod) AS atttypname, ")
			_T("array_to_string(a.attoptions, ', ') AS attoptions, ")
			_T("0 AS attcollation, ")
			_T("NULL AS attfdwoptions ")
			_T("FROM pg_catalog.pg_attribute a LEFT JOIN pg_catalog.pg_type t ")
			_T("ON a.atttypid = t.oid ")
			_T("WHERE a.attrelid = '%s'::pg_catalog.oid ")
			_T("AND a.attnum > 0::pg_catalog.int2 ")
			_T("ORDER BY a.attrelid, a.attnum"),
			oid);
	} else if (pg_version >= 70300) {
		/* need left join here to not fail on dropped columns ... */
		_stprintf(sql_buf,
			_T("SELECT a.attnum, a.attname, a.atttypmod, ")
			_T("a.attstattarget, a.attstorage, t.typstorage, ")
			_T("a.attnotnull, a.atthasdef, a.attisdropped, ")
			_T("a.attlen, a.attalign, a.attislocal, ")
			_T("pg_catalog.format_type(t.oid,a.atttypmod) AS atttypname, ")
			_T("'' AS attoptions, 0 AS attcollation, ")
			_T("NULL AS attfdwoptions ")
			_T("FROM pg_catalog.pg_attribute a LEFT JOIN pg_catalog.pg_type t ")
			_T("ON a.atttypid = t.oid ")
			_T("WHERE a.attrelid = '%s'::pg_catalog.oid ")
			_T("AND a.attnum > 0::pg_catalog.int2 ")
			_T("ORDER BY a.attrelid, a.attnum"),
			oid);
	} else if (pg_version >= 70100) {
		/*
		* attstattarget doesn't exist in 7.1.  It does exist in 7.2, but
		* we don't dump it because we can't tell whether it's been
		* explicitly set or was just a default.
		*
		* attislocal doesn't exist before 7.3, either; in older databases
		* we assume it's TRUE, else we'd fail to dump non-inherited atts.
		*/
		_stprintf(sql_buf,
			_T("SELECT a.attnum, a.attname, a.atttypmod, ")
			_T("-1 AS attstattarget, a.attstorage, ")
			_T("t.typstorage, a.attnotnull, a.atthasdef, ")
			_T("false AS attisdropped, a.attlen, ")
			_T("a.attalign, true AS attislocal, ")
			_T("format_type(t.oid,a.atttypmod) AS atttypname, ")
			_T("'' AS attoptions, 0 AS attcollation, ")
			_T("NULL AS attfdwoptions ")
			_T("FROM pg_attribute a LEFT JOIN pg_type t ")
			_T("ON a.atttypid = t.oid ")
			_T("WHERE a.attrelid = '%s'::oid ")
			_T("AND a.attnum > 0::int2 ")
			_T("ORDER BY a.attrelid, a.attnum"),
			oid);
	} else {
		/* format_type not available before 7.1 */
		_stprintf(sql_buf,
			_T("SELECT attnum, attname, atttypmod, ")
			_T("-1 AS attstattarget, ")
			_T("attstorage, attstorage AS typstorage, ")
			_T("attnotnull, atthasdef, false AS attisdropped, ")
			_T("attlen, attalign, ")
			_T("true AS attislocal, ")
			_T("(SELECT typname FROM pg_type WHERE oid = atttypid) AS atttypname, ")
			_T("'' AS attoptions, 0 AS attcollation, ")
			_T("NULL AS attfdwoptions ")
			_T("FROM pg_attribute a ")
			_T("WHERE attrelid = '%s'::oid ")
			_T("AND attnum > 0::int2 ")
			_T("ORDER BY attrelid, attnum"),
			oid);
	}

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int getNameSpaceName(HPgSession ss, const TCHAR *oid, TCHAR *namespace_name, TCHAR *msg_buf)
{
	HPgDataset result = NULL;
	TCHAR	sql_buf[4096];
	int		ret_v;
	int		pg_version = pg_get_remote_version(ss);

	if (pg_version < 70300) {
		_tcscpy(namespace_name, _T(""));
		return 0;
	}

	_stprintf(sql_buf,
		_T("select n.nspname, n.nspowner, n.oid, n.tableoid \n")
		_T("from pg_catalog.pg_namespace n \n")
		_T("where n.oid = %s \n"),
		oid);

	ret_v = pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, &result);
	if(ret_v != 0) return ret_v;

	if(pg_dataset_row_cnt(result) != 1) {
		_stprintf(msg_buf, _T("namespace not found(%s)"), oid);
		return 1;
	}

	_tcscpy(namespace_name, pg_dataset_data2(result, 0, _T("nspname")));

	pg_free_dataset(result);

	return 0;
}

static int shouldPrintColumn(HPgDataset tbl_attrs, int colno)
{
	if(pg_dataset_data2(tbl_attrs, colno, _T("attislocal"))[0] == 't' &&
	   pg_dataset_data2(tbl_attrs, colno, _T("attisdropped"))[0] != 't') {
		return 1;
	}
	return 0;
}

static void dumpLiteral(const TCHAR *data, CEditData *edit_data)
{
	const TCHAR *p = data;

	edit_data->put_char('\'');
	for(; *p != '\0';) {
		if(*p == '\'' || (*p == '\\' && !std_strings)) {
			edit_data->put_char(*p);
		}

		edit_data->put_char(*p);
		p++;
	}
	edit_data->put_char('\'');
}

static int getComments(HPgSession ss, const TCHAR *classoid, const TCHAR *objoid,
	HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR sql_buf[4096];

	_stprintf(sql_buf,
		_T("SELECT description, classoid, objoid, objsubid ")
		_T("FROM pg_catalog.pg_description ")
		_T("WHERE classoid = '%s'::oid ")
		_T("AND objoid = '%s'::oid ")
		_T("ORDER BY classoid, objoid, objsubid "),
		classoid, objoid);

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int dumpTableConstraintComment(HPgSession ss, HPgDataset tbl_info, HPgDataset constraints,
	int idx, CEditData *edit_data, TCHAR *msg_buf)
{
	int ret_v = 0;
	HPgDataset comments = NULL;

	// FIXME: 中身を実装する

	ret_v = getComments(ss, 
		pg_dataset_data2(tbl_info, 0, _T("tableoid")),
		pg_dataset_data2(tbl_info, 0, _T("oid")),
		&comments, msg_buf);
	if(ret_v != 0) goto ERR1;

/*
	relname = pg_dataset_data2(tbl_info, 0, _T("relname"));

	for(i = 0; i < pg_dataset_row_cnt(comments); i++) {
		const TCHAR *descr = pg_dataset_data2(comments, i, _T("description"));
		int objsubid = _ttoi(pg_dataset_data2(comments, i, _T("objsubid")));

		if(objsubid == 0) {
			edit_data->paste(_T("COMMENT ON "), FALSE);
			edit_data->paste(reltypename, FALSE);
			edit_data->paste(_T(" "), FALSE);
			edit_data->paste(fmtId(relname), FALSE);
			edit_data->paste(_T(" IS "), FALSE);
			dumpLiteral(descr, edit_data);
			edit_data->paste(_T(";\n"), FALSE);
		} else {
			edit_data->paste(_T("COMMENT ON COLUMN "), FALSE);
			edit_data->paste(fmtId(relname), FALSE);
			edit_data->paste(_T("."), FALSE);
			edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, objsubid - 1, _T("attname"))), FALSE);
			edit_data->paste(_T(" IS "), FALSE);
			dumpLiteral(descr, edit_data);
			edit_data->paste(_T(";\n"), FALSE);
		}
	}
*/
	if(comments != NULL) pg_free_dataset(comments);
	return 0;

ERR1:
	if(comments != NULL) pg_free_dataset(comments);
	return ret_v;
}

static int getIndexes(HPgSession ss, const TCHAR *oid, HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	int		pg_version = pg_get_remote_version(ss);

	if (pg_version >= 90000) {
		_stprintf(sql_buf,
			_T("SELECT t.tableoid, t.oid, ")
			_T("t.relname AS indexname, ")
			_T("pg_catalog.pg_get_indexdef(i.indexrelid) AS indexdef, ")
			_T("t.relnatts AS indnkeys, ")
			_T("i.indkey, i.indisclustered, ")
			_T("c.contype, c.conname, ")
			_T("c.condeferrable, c.condeferred, ")
			_T("c.tableoid AS contableoid, ")
			_T("c.oid AS conoid, ")
			_T("pg_catalog.pg_get_constraintdef(c.oid, false) AS condef, ")
			_T("(SELECT spcname FROM pg_catalog.pg_tablespace s WHERE s.oid = t.reltablespace) AS tablespace, ")
			_T("array_to_string(t.reloptions, ', ') AS options ")
			_T("FROM pg_catalog.pg_index i ")
			_T("JOIN pg_catalog.pg_class t ON (t.oid = i.indexrelid) ")
			_T("LEFT JOIN pg_catalog.pg_constraint c ")
			_T("ON (i.indrelid = c.conrelid AND ")
			_T("i.indexrelid = c.conindid AND ")
			_T("c.contype IN ('p','u','x')) ")
			_T("WHERE i.indrelid = '%s'::pg_catalog.oid ")
			_T("ORDER BY indexname"),
			oid);
	} else if (pg_version >= 80200) {
		_stprintf(sql_buf,
			_T("SELECT t.tableoid, t.oid, ")
			_T("t.relname AS indexname, ")
			_T("pg_catalog.pg_get_indexdef(i.indexrelid) AS indexdef, ")
			_T("t.relnatts AS indnkeys, ")
			_T("i.indkey, i.indisclustered, ")
			_T("c.contype, c.conname, ")
			_T("c.condeferrable, c.condeferred, ")
			_T("c.tableoid AS contableoid, ")
			_T("c.oid AS conoid, ")
			_T("null AS condef, ")
			_T("(SELECT spcname FROM pg_catalog.pg_tablespace s WHERE s.oid = t.reltablespace) AS tablespace, ")
			_T("array_to_string(t.reloptions, ', ') AS options ")
			_T("FROM pg_catalog.pg_index i ")
			_T("JOIN pg_catalog.pg_class t ON (t.oid = i.indexrelid) ")
			_T("LEFT JOIN pg_catalog.pg_depend d ")
			_T("ON (d.classid = t.tableoid ")
			_T("AND d.objid = t.oid ")
			_T("AND d.deptype = 'i') ")
			_T("LEFT JOIN pg_catalog.pg_constraint c ")
			_T("ON (d.refclassid = c.tableoid ")
			_T("AND d.refobjid = c.oid) ")
			_T("WHERE i.indrelid = '%s'::pg_catalog.oid ")
			_T("ORDER BY indexname"),
			oid);
	} else if (pg_version >= 80000) {
		_stprintf(sql_buf,
			_T("SELECT t.tableoid, t.oid, ")
			_T("t.relname AS indexname, ")
			_T("pg_catalog.pg_get_indexdef(i.indexrelid) AS indexdef, ")
			_T("t.relnatts AS indnkeys, ")
			_T("i.indkey, i.indisclustered, ")
			_T("c.contype, c.conname, ")
			_T("c.condeferrable, c.condeferred, ")
			_T("c.tableoid AS contableoid, ")
			_T("c.oid AS conoid, ")
			_T("null AS condef, ")
			_T("(SELECT spcname FROM pg_catalog.pg_tablespace s WHERE s.oid = t.reltablespace) AS tablespace, ")
			_T("null AS options ")
			_T("FROM pg_catalog.pg_index i ")
			_T("JOIN pg_catalog.pg_class t ON (t.oid = i.indexrelid) ")
			_T("LEFT JOIN pg_catalog.pg_depend d ")
			_T("ON (d.classid = t.tableoid ")
			_T("AND d.objid = t.oid ")
			_T("AND d.deptype = 'i') ")
			_T("LEFT JOIN pg_catalog.pg_constraint c ")
			_T("ON (d.refclassid = c.tableoid ")
			_T("AND d.refobjid = c.oid) ")
			_T("WHERE i.indrelid = '%s'::pg_catalog.oid ")
			_T("ORDER BY indexname"),
			oid);
	} else if (pg_version >= 70300) {
		_stprintf(sql_buf,
			_T("SELECT t.tableoid, t.oid, ")
			_T("t.relname AS indexname, ")
			_T("pg_catalog.pg_get_indexdef(i.indexrelid) AS indexdef, ")
			_T("t.relnatts AS indnkeys, ")
			_T("i.indkey, i.indisclustered, ")
			_T("c.contype, c.conname, ")
			_T("c.condeferrable, c.condeferred, ")
			_T("c.tableoid AS contableoid, ")
			_T("c.oid AS conoid, ")
			_T("null AS condef, ")
			_T("NULL AS tablespace, ")
			_T("null AS options ")
			_T("FROM pg_catalog.pg_index i ")
			_T("JOIN pg_catalog.pg_class t ON (t.oid = i.indexrelid) ")
			_T("LEFT JOIN pg_catalog.pg_depend d ")
			_T("ON (d.classid = t.tableoid ")
			_T("AND d.objid = t.oid ")
			_T("AND d.deptype = 'i') ")
			_T("LEFT JOIN pg_catalog.pg_constraint c ")
			_T("ON (d.refclassid = c.tableoid ")
			_T("AND d.refobjid = c.oid) ")
			_T("WHERE i.indrelid = '%s'::pg_catalog.oid ")
			_T("ORDER BY indexname"),
			oid);
	} else {
		// FIXME:
		return 1;
	}

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int dumpTableComment(HPgSession ss, HPgDataset tbl_info, HPgDataset tbl_attrs,
	const TCHAR *reltypename, CEditData *edit_data, TCHAR *msg_buf)
{
	int ret_v = 0;
	int i;
	HPgDataset comments = NULL;
	const TCHAR *relname;
	int numatts;

	ret_v = getComments(ss, 
		pg_dataset_data2(tbl_info, 0, _T("tableoid")),
		pg_dataset_data2(tbl_info, 0, _T("oid")),
		&comments, msg_buf);
	if(ret_v != 0) goto ERR1;

	relname = pg_dataset_data2(tbl_info, 0, _T("relname"));
	numatts = pg_dataset_row_cnt(tbl_attrs);

	for(i = 0; i < pg_dataset_row_cnt(comments); i++) {
		const TCHAR *descr = pg_dataset_data2(comments, i, _T("description"));
		int objsubid = _ttoi(pg_dataset_data2(comments, i, _T("objsubid")));

		if(objsubid == 0) {
			edit_data->paste(_T("COMMENT ON "), FALSE);
			edit_data->paste(reltypename, FALSE);
			edit_data->paste(_T(" "), FALSE);
			edit_data->paste(fmtId(relname), FALSE);
			edit_data->paste(_T(" IS "), FALSE);
			dumpLiteral(descr, edit_data);
			edit_data->paste(_T(";\n"), FALSE);
		} else {
			if(objsubid <= numatts) {
				edit_data->paste(_T("COMMENT ON COLUMN "), FALSE);
				edit_data->paste(fmtId(relname), FALSE);
				edit_data->paste(_T("."), FALSE);
				edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, objsubid - 1, _T("attname"))), FALSE);
				edit_data->paste(_T(" IS "), FALSE);
				dumpLiteral(descr, edit_data);
				edit_data->paste(_T(";\n"), FALSE);
			}
		}
	}

	if(comments != NULL) pg_free_dataset(comments);
	return 0;

ERR1:
	if(comments != NULL) pg_free_dataset(comments);
	return ret_v;
}

static int getSecLabel(HPgSession ss, const TCHAR *classoid, const TCHAR *objoid,
	HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR sql_buf[4096];

	_stprintf(sql_buf,
		_T("SELECT label, provider, classoid, objoid, objsubid ")
		_T("FROM pg_catalog.pg_seclabel ")
		_T("WHERE classoid = '%s'::oid ")
		_T("AND objoid = '%s'::oid ")
		_T("ORDER BY classoid, objoid, objsubid "),
		classoid, objoid);

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int dumpTableSecLabel(HPgSession ss, HPgDataset tbl_info, HPgDataset tbl_attrs,
	const TCHAR *reltypename, CEditData *edit_data, TCHAR *msg_buf)
{
	int ret_v = 0;
	int i;
	HPgDataset sec_labels = NULL;
	const TCHAR *relname;

	if(no_security_labels) return 0;

	ret_v = getSecLabel(ss, 
		pg_dataset_data2(tbl_info, 0, _T("tableoid")),
		pg_dataset_data2(tbl_info, 0, _T("oid")),
		&sec_labels, msg_buf);
	if(ret_v != 0) goto ERR1;

	relname = pg_dataset_data2(tbl_info, 0, _T("relname"));

	for(i = 0; i < pg_dataset_row_cnt(sec_labels); i++) {
		const TCHAR *provider = pg_dataset_data2(sec_labels, i, _T("provider"));
		const TCHAR *label = pg_dataset_data2(sec_labels, i, _T("label"));
		int         objsubid = _ttoi(pg_dataset_data2(sec_labels, i, _T("objsubid")));

		edit_data->paste(_T("SECURITY LABEL FOR "), FALSE);
		edit_data->paste(provider, FALSE);
		edit_data->paste(_T(" ON "), FALSE);

		if(objsubid == 0) {
			edit_data->paste(reltypename, FALSE);
			edit_data->paste(_T(" "), FALSE);
			edit_data->paste(fmtId(relname), FALSE);
		} else {
			edit_data->paste(fmtId(relname), FALSE);
			edit_data->paste(_T("."), FALSE);
			edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, objsubid - 1, _T("attname"))), FALSE);
		}

		edit_data->paste(_T(" IS "), FALSE);
		dumpLiteral(label, edit_data);
		edit_data->paste(_T(";\n"), FALSE);
	}

	if(sec_labels != NULL) pg_free_dataset(sec_labels);
	return 0;

ERR1:
	if(sec_labels != NULL) pg_free_dataset(sec_labels);
	return ret_v;
}

static int checkHasNotNull(HPgSession ss, HPgDataset tbl_attrs, int colno, 
	HPgDataset parent_tbls, int *result, TCHAR *msg_buf)
{
	int		i, j;
	int		ret_v;
	HPgDataset parent_attrs = NULL;
	const TCHAR *attname = pg_dataset_data2(tbl_attrs, colno, _T("attname"));

	if(pg_dataset_data2(tbl_attrs, colno, _T("attnotnull"))[0] != 't') {
		*result = 0;
		return 0;
	}

	for(i = 0; i < pg_dataset_row_cnt(parent_tbls); i++) {
		ret_v = getTableAttrs(ss, pg_dataset_data2(parent_tbls, i, _T("inhparent")),
			&parent_attrs, msg_buf);
		if(ret_v != 0) goto ERR1;
		
		for(j = 0; j < pg_dataset_row_cnt(parent_attrs); j++) {
			if(_tcscmp(attname, pg_dataset_data2(parent_attrs, j, _T("attname"))) == 0) {
				if(pg_dataset_data2(parent_attrs, j, _T("attnotnull"))[0] == 't') {
					*result = 0;
					return 0;
				}
			}
		}

		if(parent_attrs != NULL) pg_free_dataset(parent_attrs);
	}

	*result = 1;
	return 0;

ERR1:
	if(parent_attrs != NULL) pg_free_dataset(parent_attrs);
	return ret_v;
}

static int getAttDefault(HPgSession ss, const TCHAR *oid, int colno, HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	int		pg_version = pg_get_remote_version(ss);

	ASSERT(pg_version >= 70300);

	_stprintf(sql_buf,
		_T("SELECT tableoid, oid, adnum, ")
		_T("pg_catalog.pg_get_expr(adbin, adrelid) AS adsrc ")
		_T("FROM pg_catalog.pg_attrdef ")
		_T("WHERE adrelid = '%s'::pg_catalog.oid ")
		_T("AND adnum = %d"),
		oid, colno + 1);

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int getCollation(HPgSession ss, const TCHAR *collation_oid, HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	_stprintf(sql_buf,
		_T("SELECT tableoid, oid, collname, collnamespace ")
		_T("FROM pg_collation ")
		_T("WHERE oid = '%s'::pg_catalog.oid "),
		collation_oid);

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int getConstraints(HPgSession ss, const TCHAR *oid, HPgDataset *result, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	int		pg_version = pg_get_remote_version(ss);

	ASSERT(pg_version >= 70300);

	if (pg_version >= 90200) {
         /*
          * convalidated is new in 9.2 (actually, it is there in 9.1,
          * but it wasn't ever false for check constraints until 9.2).
          */
		_stprintf(sql_buf,
			_T("SELECT tableoid, oid, conname, ")
			_T("pg_catalog.pg_get_constraintdef(oid) AS consrc, ")
			_T("conislocal, convalidated ")
			_T("FROM pg_catalog.pg_constraint ")
			_T("WHERE conrelid = '%s'::pg_catalog.oid ")
			_T("   AND contype = 'c' ")
			_T("ORDER BY conname"),
			oid);
     } else if (pg_version >= 80400) {
         /* conislocal is new in 8.4 */
		_stprintf(sql_buf,
			_T("SELECT tableoid, oid, conname, ")
			_T("pg_catalog.pg_get_constraintdef(oid) AS consrc, ")
			_T("conislocal, true AS convalidated ")
			_T("FROM pg_catalog.pg_constraint ")
			_T("WHERE conrelid = '%s'::pg_catalog.oid ")
			_T("   AND contype = 'c' ")
			_T("ORDER BY conname"),
			oid);
     } else if (pg_version >= 70400) {
		_stprintf(sql_buf,
			_T("SELECT tableoid, oid, conname, ")
			_T("pg_catalog.pg_get_constraintdef(oid) AS consrc, ")
			_T("true AS conislocal, true AS convalidated ")
			_T("FROM pg_catalog.pg_constraint ")
			_T("WHERE conrelid = '%s'::pg_catalog.oid ")
			_T("   AND contype = 'c' ")
			_T("ORDER BY conname"),
			oid);
     } else if (pg_version >= 70300) {
         /* no pg_get_constraintdef, must use consrc */
		_stprintf(sql_buf,
			_T("SELECT tableoid, oid, conname, ")
			_T("'CHECK (' || consrc || ')' AS consrc, ")
			_T("true AS conislocal, true AS convalidated ")
			_T("FROM pg_catalog.pg_constraint ")
			_T("WHERE conrelid = '%s'::pg_catalog.oid ")
			_T("   AND contype = 'c' ")
			_T("ORDER BY conname"),
			oid);
     } else {
		// FIXME: エラーメッセージ
		 return 1;
	 }

	 return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, result);
}

static int parseOidArray(const TCHAR *str, Oid *array, int arraysize, TCHAR *msg_buf)
{
    int         j,
                argNum;
    TCHAR       temp[100];
    TCHAR       s;

    argNum = 0;
    j = 0;
    for (;;)
    {
        s = *str++;
        if (s == ' ' || s == '\0')
        {
            if (j > 0)
            {
                if (argNum >= arraysize)
                {
                    _stprintf(msg_buf, _T("could not parse numeric array \"%s\": too many numbers\n"), str);
					return 1;
                }
                temp[j] = '\0';
                array[argNum++] = _ttoi(temp);
                j = 0;
            }
            if (s == '\0')
                break;
        }
        else
        {
            if (!(isdigit((unsigned char) s) || s == '-') ||
                j >= sizeof(temp) - 1)
            {
                _stprintf(msg_buf, _T("could not parse numeric array \"%s\": invalid character in number\n"), str);
				return 1;
            }
            temp[j++] = s;
        }
    }

    while (argNum < arraysize)
        array[argNum++] = InvalidOid;

	return 0;
}

static int dumpIndexes(HPgSession ss, HPgDataset tbl_info, HPgDataset tbl_attrs,
	HPgDataset partition_child_info,
	const TCHAR *oid, const TCHAR *namespace_name, CEditData *edit_data, TCHAR *msg_buf)
{
	HPgDataset indexes = NULL;
	int ret_v;
	int i, j;
	TCHAR contype;
	const TCHAR *relname = pg_dataset_data2(tbl_info, 0, _T("relname"));
	Oid ind_attrs[100];
	int indnkeys;

	ret_v = getIndexes(ss, oid, &indexes, msg_buf);
	if(ret_v != 0) goto ERR1;

	for(i = 0; i < pg_dataset_row_cnt(indexes); i++) {
		contype = pg_dataset_data2(indexes, i, _T("contype"))[0];

		// partition tableの子テーブルはprimary keyのconstraintを出力しない
		if(partition_child_info && contype == 'p') continue;

		if(contype == 'p' || contype == 'u' || contype == 'x') {
			edit_data->paste(_T("ALTER TABLE ONLY "), FALSE);
			edit_data->paste(fmtId(namespace_name), FALSE);
			edit_data->paste(_T("."), FALSE);
			edit_data->paste(fmtId(relname), FALSE);
			edit_data->paste(_T("\n"), FALSE);
			edit_data->paste(_T("    ADD CONSTRAINT "), FALSE);
			edit_data->paste(fmtId(pg_dataset_data2(indexes, i, _T("indexname"))), FALSE);
			edit_data->paste(_T(" "), FALSE);

			if(_tcslen(pg_dataset_data2(indexes, i, _T("condef"))) != 0) {
				edit_data->paste(pg_dataset_data2(indexes, i, _T("condef")), FALSE);
				edit_data->paste(_T(";\n"), FALSE);
			} else {
				// FIXME: test PG8.2以下でテスト
				if(contype == 'p') {
					edit_data->paste(_T("PRIMARY KEY ("), FALSE);
				} else if(contype == 'u') {
					edit_data->paste(_T("UNIQUE ("), FALSE);
				}

				indnkeys = _ttoi(pg_dataset_data2(indexes, i, _T("indnkeys")));
				ret_v = parseOidArray(pg_dataset_data2(indexes, i, _T("indkey")), ind_attrs, ARRAY_SIZEOF(ind_attrs), msg_buf);
				if(ret_v != 0) goto ERR1;

				for(j = 0; j < indnkeys; j++) {
					int indkey = ind_attrs[j];

					if(j != 0) {
						edit_data->paste(_T(", "), FALSE);
					}
					edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, indkey - 1, _T("attname"))), FALSE);
				}
				edit_data->paste(_T(")"), FALSE);

				if(_tcslen(pg_dataset_data2(indexes, i, _T("options"))) != 0) {
					// FIXME: test
					edit_data->paste(_T(" WITH ("), FALSE);
					edit_data->paste(pg_dataset_data2(indexes, i, _T("options")), FALSE);
					edit_data->paste(_T(")"), FALSE);
				}

				if(pg_dataset_data2(indexes, i, _T("condeferrable"))[0] == 't') {
					// FIXME: test
					edit_data->paste(_T(" DEFERRABLE"), FALSE);
					if(pg_dataset_data2(indexes, i, _T("condeferred"))[0] == 't') {
						edit_data->paste(_T(" INITIALLY DEFERRED"), FALSE);
					}
				}

				edit_data->paste(_T(";\n"), FALSE);
			}
		} else if(contype == '\0') {
			edit_data->paste(pg_dataset_data2(indexes, i, _T("indexdef")), FALSE);
			edit_data->paste(_T(";\n"), FALSE);
		}

		if(pg_dataset_data2(indexes, i, _T("indisclustered"))[0] == 't') {
			// FIXME: test
			edit_data->paste(_T("\nALTER TABLE "), FALSE);
			edit_data->paste(fmtId(relname), FALSE);
			edit_data->paste(_T(" CLUSTER ON "), FALSE);
			edit_data->paste(fmtId(pg_dataset_data2(indexes, i, _T("indexname"))), FALSE);
			edit_data->paste(_T(";\n"), FALSE);
		}
	}

	if(indexes != NULL) pg_free_dataset(indexes);
	return 0;

ERR1:
	if(indexes != NULL) pg_free_dataset(indexes);
	return ret_v;
}

static int init_params(HPgSession ss, TCHAR *msg_buf)
{
	int		ret_v;
	int		pg_version = pg_get_remote_version(ss);
	TCHAR	buf[100];

	ret_v = pg_parameter_status(ss, _T("standard_conforming_strings"), 
		buf, ARRAY_SIZEOF(buf), msg_buf);
	if(ret_v != 0) goto ERR1;

	if(_tcscmp(buf, _T("on")) == 0) {
		std_strings = true;
	} else {
		std_strings = false;
	}

	if (pg_version < 90100) {
		no_security_labels = 1;
	}

	return 0;

ERR1:
	return ret_v;
}

static int dumpPartitionInfo(HPgSession ss, const TCHAR *oid, CEditData *edit_data, TCHAR *msg_buf)
{
	HPgDataset dataset = NULL;
	int ret_v;
	TCHAR	sql_buf[4096];
	TCHAR	partstrat;
	int		i;

	_stprintf(sql_buf,
		_T("select a.attname, t.partstrat, t.ordinality \n")
		_T("from  (select t.partrelid, t.partstrat, attnum, ordinality  \n")
		_T("	from pg_partitioned_table t, unnest(t.partattrs) with ordinality as attnum) t,  \n")
		_T("    pg_attribute a \n")
		_T("where t.partrelid = a.attrelid \n")
		_T("and t.attnum = a.attnum \n")
		_T("and t.partrelid = '%s' \n")
		_T("order by t.ordinality \n"),
		oid);

	ret_v = pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, &dataset);
	if(ret_v != 0) goto ERR1;

	if(pg_dataset_row_cnt(dataset) > 0) {
		edit_data->paste(_T("\nPARTITION BY "), FALSE);

		partstrat = pg_dataset_data2(dataset, 0, _T("partstrat"))[0];
		if(partstrat == 'l') {
			edit_data->paste(_T("LIST ("), FALSE);
		} else if(partstrat == 'r') {
			edit_data->paste(_T("RANGE ("), FALSE);
		}
		for(i = 0; i < pg_dataset_row_cnt(dataset); i++) {
			if(i != 0) {
				edit_data->paste(_T(", "), FALSE);
			}
			edit_data->paste(pg_dataset_data2(dataset, i, _T("attname")));
		}
		edit_data->paste(_T(")"), FALSE);
	}

	if(dataset != NULL) pg_free_dataset(dataset);
	return 0;

ERR1:
	if(dataset != NULL) pg_free_dataset(dataset);
	return ret_v;
}

static int getPartitionChildInfo(HPgSession ss, const TCHAR *oid, HPgDataset *dataset, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	_stprintf(sql_buf,
		_T("select ns.nspname parent_namespace_name, pc.relname parent_rel_name, pg_get_expr(c.relpartbound, c.oid) partition_value \n")
		_T("from pg_catalog.pg_inherits i, pg_class c, pg_class pc, pg_namespace ns \n")
		_T("where c.oid = %s \n")
		_T("and c.oid = i.inhrelid \n")
		_T("and pc.oid = i.inhparent \n")
		_T("and pc.relnamespace = ns.oid \n")
		_T("and c.relpartbound is not null \n"),
		oid);

	return pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, dataset);
}

static int dumpPartitionChildTableSchema(HPgSession ss, const TCHAR *parent_oid, CEditData *edit_data, TCHAR *msg_buf)
{
	int		ret_v;
	int		i;
	TCHAR	sql_buf[4096];
	HPgDataset child_tbl_info = NULL;

	_stprintf(sql_buf,
		_T("select i.inhrelid, ns.nspname, c.relname \n")
		_T("from pg_catalog.pg_inherits i, pg_class c, pg_namespace ns \n")
		_T("where i.inhparent = %s \n")
		_T("and c.oid = i.inhrelid \n")
		_T("and c.relnamespace = ns.oid \n")
		_T("order by c.relname \n"),
		parent_oid);

	ret_v = pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, &child_tbl_info);
	if(ret_v != 0) goto ERR1;

	for(i = 0; i < pg_dataset_row_cnt(child_tbl_info); i++) {
		edit_data->paste(_T("\n"), FALSE);

		ret_v = dumpTableSchema(ss, pg_dataset_data2(child_tbl_info, i, _T("inhrelid")), edit_data, msg_buf);
		if(ret_v != 0) goto ERR1;
	}

	if(child_tbl_info != NULL) pg_free_dataset(child_tbl_info);

	return 0;

ERR1:
	if(child_tbl_info != NULL) pg_free_dataset(child_tbl_info);
	return ret_v;
}

int dumpTableSchema(HPgSession ss, const TCHAR *oid, CEditData *edit_data, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	int		pg_version = pg_get_remote_version(ss);
	int		ret_v;
	TCHAR	namespace_name[100];
	int			actual_atts;	/* number of attrs in this CREATE statement */
	const TCHAR *reltypename = NULL;
	TCHAR		*storage = NULL;
	TCHAR		*srvname = NULL;
	TCHAR		*ftoptions;
	int			j, k;
	HPgDataset tbl_info = NULL;
	HPgDataset tbl_attrs = NULL;
	HPgDataset parent_tbls = NULL;
	HPgDataset partition_child_info = NULL;
	HPgDataset attr_default = NULL;
	HPgDataset collation = NULL;
	HPgDataset result = NULL;
	HPgDataset constraints = NULL;
	TCHAR			relkind;
	TCHAR			relpersistence;
	const TCHAR		*relname = NULL;
	const TCHAR		*reloptions = NULL;
	const TCHAR		*toast_reloptions = NULL;
	const TCHAR		*reloftype = NULL;
	int			numatts;

	CString     org_search_path = show_search_path(ss, msg_buf);

	if (pg_version < 70300) {
		_stprintf(msg_buf, _T("This function support for postgresql 7.3 or later"));
		ret_v = 1;
		goto ERR1;
	}

	ret_v = init_params(ss, msg_buf);
	if(ret_v != 0) goto ERR1;

	ret_v = getTableInfo(ss, oid, &tbl_info, msg_buf);
	if(ret_v != 0) goto ERR1;

	if(pg_dataset_row_cnt(tbl_info) != 1) {
		_stprintf(msg_buf, _T("table not found(%s)"), oid);
		ret_v = 1;
		goto ERR1;
	}
	relkind = pg_dataset_data2(tbl_info, 0, _T("relkind"))[0];
	relpersistence = pg_dataset_data2(tbl_info, 0, _T("relpersistence"))[0];
	relname = pg_dataset_data2(tbl_info, 0, _T("relname"));
	reloptions = pg_dataset_data2(tbl_info, 0, _T("reloptions"));
	toast_reloptions = pg_dataset_data2(tbl_info, 0, _T("toast_reloptions"));
	reloftype = pg_dataset_data2(tbl_info, 0, _T("reloftype"));

	ret_v = getInhParents(ss, oid, &parent_tbls, msg_buf);
	if(ret_v != 0) goto ERR1;

	if(pg_version >= 100000) {
		ret_v = getPartitionChildInfo(ss, oid, &partition_child_info, msg_buf);
		if(ret_v != 0) goto ERR1;

		if(partition_child_info && pg_dataset_row_cnt(partition_child_info) == 0) {
			pg_free_dataset(partition_child_info);
			partition_child_info = NULL;
		}
	}

	ret_v = getNameSpaceName(ss, pg_dataset_data2(tbl_info, 0, _T("relnamespace")), 
		namespace_name, msg_buf);
	if(ret_v != 0) goto ERR1;

	/* Make sure we are in proper schema */
	ret_v = selectSourceSchema(ss, namespace_name, msg_buf);
	if(ret_v != 0) goto ERR1;

	/* Is it a table or a view? */
	if (relkind == RELKIND_VIEW) {
		const TCHAR	   *viewdef;

		reltypename = _T("VIEW");

		/* Fetch the view definition */
		/* Beginning in 7.3, viewname is not unique; rely on OID */
		ASSERT(pg_version >= 70300);
		_stprintf(sql_buf,
			_T("SELECT pg_catalog.pg_get_viewdef('%s'::pg_catalog.oid) AS viewdef"),
			oid);

		ret_v = pg_create_dataset_ex(ss, sql_buf, msg_buf, NULL, NULL, &result);
		if(ret_v != 0) goto ERR1;

		if (pg_dataset_row_cnt(result) != 1) {
			if (pg_dataset_row_cnt(result) < 1) {
				_stprintf(msg_buf, _T("query to obtain definition of view \"%s\" returned no data\n"), relname);
			} else {
				_stprintf(msg_buf, _T("query to obtain definition of view \"%s\" returned more than one definition\n"), relname);
			}
		}

		viewdef = pg_dataset_data(result, 0, 0);

		if (_tcslen(viewdef) == 0)
			_stprintf(msg_buf, _T("definition of view \"%s\" appears to be empty (length zero)\n"), relname);

		edit_data->paste(_T("CREATE VIEW "), FALSE);

		edit_data->paste(fmtId(namespace_name), FALSE);
		edit_data->paste(_T("."), FALSE);
		edit_data->paste(fmtId(relname), FALSE);

		if (reloptions && _tcslen(reloptions) > 0) {
			edit_data->paste(_T(" WITH ("), FALSE);
			edit_data->paste(reloptions, FALSE);
			edit_data->paste(_T(")"), FALSE);
		}
		edit_data->paste(_T(" AS\n"), FALSE);
		edit_data->paste(viewdef, FALSE);
		edit_data->paste(_T("\n"), FALSE);

		pg_free_dataset(result);
		result = NULL;
	} else {
		if (relkind == RELKIND_FOREIGN_TABLE) {
// FIXME: foreign tableに対応する
//			int			i_srvname;
//			int			i_ftoptions;
//
//			reltypename = _T("FOREIGN TABLE");
//
//			/* retrieve name of foreign server and generic options */
//			appendPQExpBuffer(query,
//							  "SELECT fs.srvname, "
//							  "pg_catalog.array_to_string(ARRAY("
//							  "SELECT pg_catalog.quote_ident(option_name) || "
//							  "' ' || pg_catalog.quote_literal(option_value) "
//							"FROM pg_catalog.pg_options_to_table(ftoptions) "
//							  "ORDER BY option_name"
//							  "), E',\n    ') AS ftoptions "
//							  "FROM pg_catalog.pg_foreign_table ft "
//							  "JOIN pg_catalog.pg_foreign_server fs "
//							  "ON (fs.oid = ft.ftserver) "
//							  "WHERE ft.ftrelid = '%u'",
//							  tbinfo->dobj.catId.oid);
//			res = ExecuteSqlQueryForSingleRow(fout, query->data);
//			i_srvname = PQfnumber(res, "srvname");
//			i_ftoptions = PQfnumber(res, "ftoptions");
//			srvname = pg_strdup(PQgetvalue(res, 0, i_srvname));
//			ftoptions = pg_strdup(PQgetvalue(res, 0, i_ftoptions));
//			PQclear(res);
		} else {
			reltypename = _T("TABLE");
			srvname = NULL;
			ftoptions = NULL;
		}

		edit_data->paste(_T("CREATE "), FALSE);
		if(relpersistence == RELPERSISTENCE_UNLOGGED) {
			edit_data->paste(_T("UNLOGGED "), FALSE);
		}
		edit_data->paste(reltypename, FALSE);
		edit_data->paste(_T(" "), FALSE);

		edit_data->paste(fmtId(namespace_name), FALSE);
		edit_data->paste(_T("."), FALSE);
		edit_data->paste(fmtId(relname), FALSE);

		/*
		 * Attach to type, if reloftype; except in case of a binary upgrade,
		 * we dump the table normally and attach it to the type afterward.
		 */
		if (reloftype && _tcslen(reloftype) > 0) {
			edit_data->paste(_T(" OF "), FALSE);
			edit_data->paste(reloftype, FALSE);
		}

		ret_v = getTableAttrs(ss, oid, &tbl_attrs, msg_buf);
		if(ret_v != 0) goto ERR1;

		numatts = pg_dataset_row_cnt(tbl_attrs);
		actual_atts = 0;
		for (j = 0; j < numatts; j++) {
			/*
			 * Normally, dump if it's locally defined in this table, and not
			 * dropped.  But for binary upgrade, we'll dump all the columns,
			 * and then fix up the dropped and nonlocal cases below.
			 */
			if (shouldPrintColumn(tbl_attrs, j)) {
				/*
				 * Default value --- suppress if to be printed separately.
				 */
				int has_default = 0;
				if(pg_dataset_data2(tbl_attrs, j, _T("atthasdef"))[0] == 't') {
					ret_v = getAttDefault(ss, oid, j, &attr_default, msg_buf);
					if(ret_v != 0) goto ERR1;
					if(pg_dataset_row_cnt(attr_default) == 1) {
						has_default = 1;
					}
				}

				/*
				 * Not Null constraint --- suppress if inherited, except in
				 * binary-upgrade case where that won't work.
				 */
				int has_notnull;
				ret_v = checkHasNotNull(ss, tbl_attrs, j, parent_tbls, &has_notnull, msg_buf);
				if(ret_v != 0) goto ERR1;

				/* Skip column if fully defined by reloftype */
				if (reloftype && _tcslen(reloftype) > 0 && !has_default && !has_notnull)
					continue;

				/* Format properly if not first attr */
				if (actual_atts == 0)
					edit_data->paste(_T(" ("), FALSE);
				else
					edit_data->paste(_T(","), FALSE);
				edit_data->paste(_T("\n    "), FALSE);
				actual_atts++;

				/* Attribute name */
				edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, j, _T("attname"))), FALSE);
				edit_data->paste(_T(" "), FALSE);

				if (pg_dataset_data2(tbl_attrs, j, _T("attisdropped"))[0] == 't') {
					/*
					 * ALTER TABLE DROP COLUMN clears pg_attribute.atttypid,
					 * so we will not have gotten a valid type name; insert
					 * INTEGER as a stopgap.  We'll clean things up later.
					 */
					edit_data->paste(_T("INTEGER /* dummy */"), FALSE);
					/* Skip all the rest, too */
					continue;
				}

				/* Attribute type */
				if (reloftype && _tcslen(reloftype) > 0) {
					edit_data->paste(_T("WITH OPTIONS"), FALSE);
				} else {
					ASSERT(pg_version >= 70100);
					edit_data->paste(pg_dataset_data2(tbl_attrs, j, _T("atttypname")), FALSE);
				}

				/* Add collation if not default for the type */
				if (OidIsValid(_ttoi(pg_dataset_data2(tbl_attrs, j, _T("attcollation"))))) {
					ret_v = getCollation(ss, pg_dataset_data2(tbl_attrs, j, _T("attcollation")), 
						&collation, msg_buf);
					if(ret_v != 0) goto ERR1;

					if(pg_dataset_row_cnt(collation) == 1) {
						TCHAR	coll_namespace_name[100];
						ret_v = getNameSpaceName(ss, pg_dataset_data2(collation, 0, _T("collnamespace")),
							coll_namespace_name, msg_buf);
						if(ret_v != 0) goto ERR1;
						edit_data->paste(_T(" COLLATE "), FALSE);
						edit_data->paste(fmtId(coll_namespace_name), FALSE);
						edit_data->paste(_T("."), FALSE);
						edit_data->paste(fmtId(pg_dataset_data2(collation, 0, _T("collname"))), FALSE);
					}

					pg_free_dataset(collation);
					collation = NULL;
				}

				if (has_default) {
					edit_data->paste(_T(" DEFAULT "), FALSE);
					edit_data->paste(pg_dataset_data2(attr_default, 0, _T("adsrc")), FALSE);
					pg_free_dataset(attr_default);
					attr_default = NULL;
				}

				if (has_notnull)
					edit_data->paste(_T(" NOT NULL"), FALSE);
			}
		}

		/*
		 * Add non-inherited CHECK constraints, if any.
		 */
		ret_v = getConstraints(ss, oid, &constraints, msg_buf);
		if(ret_v != 0) goto ERR1;

		// inline constraint (checkなど)
		for (j = 0; j < pg_dataset_row_cnt(constraints); j++) {
			if(pg_dataset_data2(constraints, j, _T("convalidated"))[0] != 't' ||
				pg_dataset_data2(constraints, j, _T("conislocal"))[0] != 't') {
				continue;
			}

			if (actual_atts == 0)
				edit_data->paste(_T(" (\n    "), FALSE);
			else
				edit_data->paste(_T(",\n    "), FALSE);

			edit_data->paste(_T("CONSTRAINT "), FALSE);
			edit_data->paste(pg_dataset_data2(constraints, j, _T("conname")), FALSE);
			edit_data->paste(_T(" "), FALSE);
			edit_data->paste(pg_dataset_data2(constraints, j, _T("consrc")), FALSE);

			actual_atts++;
		}

		if (actual_atts) {
			edit_data->paste(_T("\n)"), FALSE);
		} else if (partition_child_info == NULL && !(reloftype && _tcslen(reloftype) > 0)) {
			/*
			 * We must have a parenthesized attribute list, even though empty,
			 * when not using the OF TYPE syntax.
			 */
			edit_data->paste(_T("(\n)"), FALSE);
		}

		if(partition_child_info) {
			edit_data->paste(_T("\nPARTITION OF "), FALSE);
			if(_tcscmp(namespace_name, pg_dataset_data2(partition_child_info, 0, _T("parent_namespace_name"))) != 0) {
				edit_data->paste(fmtId(pg_dataset_data2(partition_child_info, 0, _T("parent_namespace_name"))), FALSE);
				edit_data->paste(_T("."), FALSE);
			}
			edit_data->paste(fmtId(pg_dataset_data2(partition_child_info, 0, _T("parent_rel_name"))), FALSE);
			edit_data->paste(_T("\n"), FALSE);
			edit_data->paste(pg_dataset_data2(partition_child_info, 0, _T("partition_value")), FALSE);
		} else if (pg_dataset_row_cnt(parent_tbls) > 0) {
			edit_data->paste(_T("\nINHERITS ("), FALSE);
			for (k = 0; k < pg_dataset_row_cnt(parent_tbls); k++) {
				if (k > 0) edit_data->paste(_T(", "), FALSE);
				if (_tcscmp(namespace_name, pg_dataset_data2(parent_tbls, k, _T("parent_namespace_name"))) != 0) {
					edit_data->paste(fmtId(pg_dataset_data2(parent_tbls, k, _T("parent_namespace_name"))), FALSE);
					edit_data->paste(_T("."), FALSE);
				}
				edit_data->paste(fmtId(pg_dataset_data2(parent_tbls, k, _T("parent_rel_name"))), FALSE);
			}
			edit_data->paste(_T(")"), FALSE);
		}

// FIXME: foreign tableに対応する
//		if (tbinfo->relkind == RELKIND_FOREIGN_TABLE)
//			appendPQExpBuffer(q, "\nSERVER %s", fmtId(srvname));

		if(reloptions && _tcslen(reloptions) > 0 ||
			toast_reloptions && _tcslen(toast_reloptions) > 0) {
			bool		addcomma = false;
			edit_data->paste(_T("\nWITH ("), FALSE);
			if(reloptions && _tcslen(reloptions) > 0) {
				addcomma = true;
				edit_data->paste(reloptions, FALSE);
			}
			if(toast_reloptions && _tcslen(toast_reloptions) > 0) {
				if(addcomma) {
					edit_data->paste(_T(", "), FALSE);
				}
				edit_data->paste(toast_reloptions, FALSE);
			}
			edit_data->paste(_T(")"), FALSE);
		}

		/* Dump generic options if any */
		if (ftoptions && ftoptions[0]) {
			edit_data->paste(_T("\nOPTIONS (\n    "), FALSE);
			edit_data->paste(ftoptions, FALSE);
			edit_data->paste(_T("\n)"), FALSE);
		}

		if(relkind == RELKIND_PARTITIONED_TABLE) {
			dumpPartitionInfo(ss, oid, edit_data, msg_buf);
		}

		edit_data->paste(_T(";\n"), FALSE);

		/*
		 * Dump additional per-column properties that we can't handle in the
		 * main CREATE TABLE command.
		 */
		for (j = 0; j < numatts; j++) {
			/* None of this applies to dropped columns */
			if(pg_dataset_data2(tbl_attrs, j, _T("attisdropped"))[0] == 't') continue;

			/*
			 * If we didn't dump the column definition explicitly above, and
			 * it is NOT NULL and did not inherit that property from a parent,
			 * we have to mark it separately.
			 */
			if (!shouldPrintColumn(tbl_attrs, j)) {
				int has_notnull;
				ret_v = checkHasNotNull(ss, tbl_attrs, j, parent_tbls, &has_notnull, msg_buf);
				if(ret_v != 0) goto ERR1;

				if(has_notnull) {
					edit_data->paste(_T("ALTER TABLE ONLY "), FALSE);
					edit_data->paste(fmtId(namespace_name), FALSE);
					edit_data->paste(_T("."), FALSE);
					edit_data->paste(fmtId(relname), FALSE);
					edit_data->paste(_T(" ALTER COLUMN "), FALSE);
					edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, j, _T("attname"))), FALSE);
					edit_data->paste(_T(" SET NOT NULL;\n"), FALSE);
				}
			}

			/*
			 * Dump per-column statistics information. We only issue an ALTER
			 * TABLE statement if the attstattarget entry for this column is
			 * non-negative (i.e. it's not the default value)
			 */
			if (_ttoi(pg_dataset_data2(tbl_attrs, j, _T("attstattarget"))) >= 0) {
				edit_data->paste(_T("ALTER TABLE ONLY "), FALSE);
				edit_data->paste(fmtId(namespace_name), FALSE);
				edit_data->paste(_T("."), FALSE);
				edit_data->paste(fmtId(relname), FALSE);
				edit_data->paste(_T(" ALTER COLUMN "), FALSE);
				edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, j, _T("attname"))), FALSE);
				edit_data->paste(_T(" SET STATISTICS "), FALSE);
				edit_data->paste(pg_dataset_data2(tbl_attrs, j, _T("attstattarget")), FALSE);
				edit_data->paste(_T(";\n"), FALSE);
			}

			/*
			 * Dump per-column storage information.  The statement is only
			 * dumped if the storage has been changed from the type's default.
			 */
			if(_tcscmp(pg_dataset_data2(tbl_attrs, j, _T("attstorage")), 
					   pg_dataset_data2(tbl_attrs, j, _T("typstorage"))) != 0) {
				switch (pg_dataset_data2(tbl_attrs, j, _T("attstorage"))[0])
				{
					case 'p':
						storage = _T("PLAIN");
						break;
					case 'e':
						storage = _T("EXTERNAL");
						break;
					case 'm':
						storage = _T("MAIN");
						break;
					case 'x':
						storage = _T("EXTENDED");
						break;
					default:
						storage = NULL;
				}

				/*
				 * Only dump the statement if it's a storage type we recognize
				 */
				if (storage != NULL) {
					edit_data->paste(_T("ALTER TABLE ONLY "), FALSE);
					edit_data->paste(fmtId(namespace_name), FALSE);
					edit_data->paste(_T("."), FALSE);
					edit_data->paste(fmtId(relname), FALSE);
					edit_data->paste(_T(" ALTER COLUMN "), FALSE);
					edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, j, _T("attname"))), FALSE);
					edit_data->paste(_T(" SET STORAGE "), FALSE);
					edit_data->paste(storage, FALSE);
					edit_data->paste(_T(";\n"), FALSE);
				}
			}

			/*
			 * Dump per-column attributes.
			 */
			if(_tcslen(pg_dataset_data2(tbl_attrs, j, _T("attoptions"))) > 0) {
				edit_data->paste(_T("ALTER TABLE ONLY "), FALSE);
				edit_data->paste(fmtId(namespace_name), FALSE);
				edit_data->paste(_T("."), FALSE);
				edit_data->paste(fmtId(relname), FALSE);
				edit_data->paste(_T(" ALTER COLUMN "), FALSE);
				edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, j, _T("attname"))), FALSE);
				edit_data->paste(_T(" SET ("), FALSE);
				edit_data->paste(pg_dataset_data2(tbl_attrs, j, _T("attoptions")), FALSE);
				edit_data->paste(_T(");\n"), FALSE);
			}

			/*
			 * Dump per-column fdw options.
			 */
			if (relkind == RELKIND_FOREIGN_TABLE &&
				_tcslen(pg_dataset_data2(tbl_attrs, j, _T("attfdwoptions"))) > 0) {
				edit_data->paste(_T("ALTER FOREIGN TABLE "), FALSE);
				edit_data->paste(fmtId(relname), FALSE);
				edit_data->paste(_T(" ALTER COLUMN "), FALSE);
				edit_data->paste(fmtId(pg_dataset_data2(tbl_attrs, j, _T("attname"))), FALSE);
				edit_data->paste(_T(" OPTIONS (\n    "), FALSE);
				edit_data->paste(pg_dataset_data2(tbl_attrs, j, _T("attfdwoptions")), FALSE);
				edit_data->paste(_T("\n);\n"), FALSE);
			}
		}
	}

	/* Dump Table Comments */
	ret_v = dumpTableComment(ss, tbl_info, tbl_attrs, reltypename, edit_data, msg_buf);
	if(ret_v != 0) goto ERR1;

	/* Dump Table Security Labels */
	ret_v = dumpTableSecLabel(ss, tbl_info, tbl_attrs, reltypename, edit_data, msg_buf);
	if(ret_v != 0) goto ERR1;

	/* Dump comments on inlined table constraints */
	if(constraints != NULL) {
		for (j = 0; j < pg_dataset_row_cnt(constraints); j++) {
			if(pg_dataset_data2(constraints, j, _T("convalidated"))[0] != 't' ||
				pg_dataset_data2(constraints, j, _T("conislocal"))[0] != 't') {
				continue;
			}
			dumpTableConstraintComment(ss, tbl_info, constraints, j, edit_data, msg_buf);
		}
	}

	ret_v = dumpIndexes(ss, tbl_info, tbl_attrs, partition_child_info, oid, namespace_name, edit_data,  msg_buf);
	if(ret_v != 0) goto ERR1;

	if(relkind == RELKIND_PARTITIONED_TABLE) {
		ret_v = dumpPartitionChildTableSchema(ss, oid, edit_data, msg_buf);
		if(ret_v != 0) goto ERR1;
	}

	if(tbl_info != NULL) pg_free_dataset(tbl_info);
	if(tbl_attrs != NULL) pg_free_dataset(tbl_attrs);
	if(result != NULL) pg_free_dataset(result);
	if(parent_tbls != NULL) pg_free_dataset(parent_tbls);
	if(partition_child_info != NULL) pg_free_dataset(partition_child_info);
	if(attr_default != NULL) pg_free_dataset(attr_default);
	if(collation != NULL) pg_free_dataset(collation);
	if(constraints != NULL) pg_free_dataset(constraints);

	set_search_path(ss, org_search_path, msg_buf);

	return 0;

ERR1:
	if(tbl_info != NULL) pg_free_dataset(tbl_info);
	if(tbl_attrs != NULL) pg_free_dataset(tbl_attrs);
	if(result != NULL) pg_free_dataset(result);
	if(parent_tbls != NULL) pg_free_dataset(parent_tbls);
	if(partition_child_info != NULL) pg_free_dataset(partition_child_info);
	if(attr_default != NULL) pg_free_dataset(attr_default);
	if(collation != NULL) pg_free_dataset(collation);
	if(constraints != NULL) pg_free_dataset(constraints);

	set_search_path(ss, org_search_path, msg_buf);

	return ret_v;
}

/* この関数は未使用 getsrc.cppのcreate_index_sourceを使う */
int dumpIndexSchema(HPgSession ss, const TCHAR* table_oid, const TCHAR* index_oid, 
	CEditData* edit_data, TCHAR* msg_buf)
{
	int		pg_version = pg_get_remote_version(ss);
	int		ret_v;
	TCHAR	namespace_name[100];
	HPgDataset tbl_info = NULL;
	HPgDataset tbl_attrs = NULL;
	HPgDataset partition_child_info = NULL;

	CString     org_search_path = show_search_path(ss, msg_buf);

	if (pg_version < 70300) {
		_stprintf(msg_buf, _T("This function support for postgresql 7.3 or later"));
		ret_v = 1;
		goto ERR1;
	}

	ret_v = init_params(ss, msg_buf);
	if (ret_v != 0) goto ERR1;

	ret_v = getTableInfo(ss, table_oid, &tbl_info, msg_buf);
	if (ret_v != 0) goto ERR1;

	if (pg_dataset_row_cnt(tbl_info) != 1) {
		_stprintf(msg_buf, _T("table not found(%s)"), table_oid);
		ret_v = 1;
		goto ERR1;
	}

	if (pg_version >= 100000) {
		ret_v = getPartitionChildInfo(ss, table_oid, &partition_child_info, msg_buf);
		if (ret_v != 0) goto ERR1;

		if (partition_child_info && pg_dataset_row_cnt(partition_child_info) == 0) {
			pg_free_dataset(partition_child_info);
			partition_child_info = NULL;
		}
	}

	ret_v = getNameSpaceName(ss, pg_dataset_data2(tbl_info, 0, _T("relnamespace")),
		namespace_name, msg_buf);
	if (ret_v != 0) goto ERR1;

	/* Make sure we are in proper schema */
	ret_v = selectSourceSchema(ss, namespace_name, msg_buf);
	if (ret_v != 0) goto ERR1;

	ret_v = dumpIndexes(ss, tbl_info, tbl_attrs, partition_child_info, table_oid, namespace_name, edit_data, msg_buf);
	if (ret_v != 0) goto ERR1;

	if (tbl_info != NULL) pg_free_dataset(tbl_info);
	if (tbl_attrs != NULL) pg_free_dataset(tbl_attrs);
	if (partition_child_info != NULL) pg_free_dataset(partition_child_info);

	set_search_path(ss, org_search_path, msg_buf);

	return 0;

ERR1:
	if (tbl_info != NULL) pg_free_dataset(tbl_info);
	if (tbl_attrs != NULL) pg_free_dataset(tbl_attrs);
	if (partition_child_info != NULL) pg_free_dataset(partition_child_info);

	set_search_path(ss, org_search_path, msg_buf);

	return ret_v;
}


