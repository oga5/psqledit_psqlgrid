/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __SQL_PAESER_H_INCLUDED__
#define __SQL_PAESER_H_INCLUDED__

#include "EditData.h"
#include "sqlstrtoken.h"

typedef enum {
	SQL_NORMAL,
	SQL_PLSQL,
	SQL_SELECT,
	SQL_COMMAND,
	SQL_FILE_RUN,
	SQL_BLANK,
} SQL_KIND;

class CSqlParser
{
public:
	CSqlParser();
	~CSqlParser();

	void set_str_token(CSQLStrToken *token) { m_str_token = token; }

	const TCHAR *GetSqlBuf() {
		if(m_sql_buf == NULL) return _T("");
		return m_sql_buf;
	}
	const TCHAR *GetSkipSpaceSql() {
		if(m_skip_space_sql == NULL) return _T("");
		return m_skip_space_sql;
	}
	SQL_KIND GetSqlKind() { return m_sql_kind; }
	int GetSQL(int start_row, CEditData *edit_data);

	void FreeSqlBuf();

private:
	TCHAR *AllocSqlBuf(int size);
	BOOL is_sql_end2(const TCHAR *p, int *semicolon_col);

	CSQLStrToken	*m_str_token;
	TCHAR			*m_sql_buf;
	int				m_sql_buf_alloc_cnt;
	TCHAR			*m_skip_space_sql;
	SQL_KIND		m_sql_kind;
};

// global function
BOOL is_sql_end(CEditData *edit_data, int row, CStrToken *token);

#endif __SQL_PAESER_H_INCLUDED__
