/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "sqlparse.h"
#include "global.h"

#include "oregexp.h"

// 行の末尾に';'があるかチェックする(スペースは無視する)
BOOL is_sql_end(CEditData *edit_data, int row, int *semicolon_col, BOOL in_quote)
{
	const TCHAR *p_start = edit_data->get_row_buf(row);
	const TCHAR *p = p_start;
	CStrToken *token = edit_data->get_str_token();

	ASSERT(token == &g_sql_str_token);

	if(in_quote) {
		for(; *p != '\0' && *p != '\'';) p++;
		if(*p == '\'') p++;
		p = token->skipCommentAndSpace(p);
		if(p == NULL) return FALSE;

		INT_PTR start;
		if(oregexp_lwr(_T("language\\s+'.+?'\\s*;\\s*$"), 
			p, &start, NULL, 1) != OREGEXP_FOUND) return FALSE;
		p += start;
	}

	for(; *p != '\0';) {
		if(*p == ';') {
			int char_type = edit_data->check_char_type(row, (int)(p - p_start));
			if((!in_quote && char_type == CHAR_TYPE_NORMAL) ||
				(in_quote && char_type == CHAR_TYPE_IN_QUOTE)) {
				if(semicolon_col) *semicolon_col = (int)(p - p_start);
				return TRUE;
			}
		}
		p += token->get_word_len(p);
	}

	return FALSE;
}

