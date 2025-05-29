/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "psqledit.h"

#include "lintsql.h"
#include "sqlparser.h"
#include "ostrutil.h"

static void NextLine(CEditData *edit_data, int indent_level)
{
	if(edit_data->is_blank_row(edit_data->get_cur_row())) {
		edit_data->del_row(edit_data->get_cur_row());
	}
	if(edit_data->get_prev_char() == ' ') edit_data->back_space();

	edit_data->input_char('\r');
	for(int i = 0; i < indent_level; i++) {
		edit_data->input_char('\t');
	}
}

static const TCHAR *next_line_word[] = {
	_T("select"), _T("insert"), _T("delete"), _T("update"),
	_T("from"), _T("where"), _T("into"),
	_T("order"), _T("group"), _T("having"), 
	_T("union"), _T("intersect"), _T("minus"), 
	_T("and"), _T("or"), 
	_T("values"), _T("set"),
};

static const TCHAR *from_clause_next_line_word[] = {
	_T("on"), _T("using"),
	_T("left"), _T("right"), _T("full"), _T("inner"), 
};

static int right_set_len = 6; // selectにあわせる
static const TCHAR *right_set_word[] = {
	_T("select"), _T("insert"), _T("delete"), _T("update"),
	_T("from"), _T("where"), _T("into"),
	_T("order"), _T("group"), _T("having"), 
	_T("and"), _T("or"), 
	_T("values"), _T("set"),
	_T("on"), _T("using"),
	_T("left"), _T("right"), _T("full"), _T("inner"), 
};

static bool is_next_line_keyword(const TCHAR *buf, CString &cur_clause)
{
	int		i;

	if(ostr_strcmp_nocase(cur_clause, _T("between")) == 0 &&
		ostr_strcmp_nocase(buf, _T("and")) == 0) return false;
	if(ostr_strcmp_nocase(cur_clause, _T("delete")) == 0 &&
		ostr_strcmp_nocase(buf, _T("from")) == 0) return false;
	if(ostr_strcmp_nocase(cur_clause, _T("insert")) == 0 &&
		ostr_strcmp_nocase(buf, _T("into")) == 0) return false;

	if(ostr_strcmp_nocase(cur_clause, _T("from")) == 0) {
		for(i = 0; i < sizeof(from_clause_next_line_word) / sizeof(from_clause_next_line_word[0]); i++) {
			if(ostr_strcmp_nocase(buf, from_clause_next_line_word[i]) == 0) {
				// cur_clauseは"from"のままにする
				return true;
			}
		}
	}

	for(i = 0; i < sizeof(next_line_word) / sizeof(next_line_word[0]); i++) {
		if(ostr_strcmp_nocase(buf, next_line_word[i]) == 0) {
			cur_clause = _T("");
			return true;
		}
	}
	return false;
}

static bool is_right_set_word(const TCHAR *buf)
{
	for(int i = 0; i < sizeof(right_set_word) / sizeof(right_set_word[0]); i++) {
		if(ostr_strcmp_nocase(buf, right_set_word[i]) == 0) return true;
	}
	return false;
}

static void put_space(CEditData *edit_data, int cnt)
{
	for(; cnt > 0; cnt--) {
		edit_data->input_char(' ');
	}
}

static void CopyComment(CEditData *src_data, CEditData *edit_data, int indent_level)
{
	if(src_data->is_end_of_text()) return;

	POINT cur_pt = src_data->get_cur_pt();
//	if(cur_pt.x == 0) edit_data->paste(_T("\n"));
	src_data->skip_comment();
	POINT end_pt = src_data->get_cur_pt();

	int buf_size = src_data->calc_buf_size(&cur_pt, &end_pt);
	
	CString buf;
	src_data->copy_text_data(&cur_pt, &end_pt,
		buf.GetBuffer(buf_size), buf_size);
	buf.ReleaseBuffer();

	edit_data->paste(buf);

	return;
}

static bool isSQLOperatorChar(unsigned int ch)
{
	// +, -, *, /, =, !, ^, |, <, >を識別するテーブル
	const static unsigned char operator_char_table[16] = {
		0x00, 0x00, 0x00, 0x00, 0x02, 0xac, 0x00, 0x70,
		0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x10,
	};

	return (ch < 128 && (operator_char_table[ch / 8] & (0x01 << ch % 8)) != 0);
}

static bool is_end_p(CEditData *src_data, POINT *p_end_pt)
{
	if(p_end_pt) {
		if(p_end_pt->y < src_data->get_cur_row()) return true;
		if(p_end_pt->y == src_data->get_cur_row() && p_end_pt->x <= src_data->get_cur_col()) return true;
	}
	return false;
}

static void skip_space(CEditData *src_data)
{
	for(;;) {
		if(src_data->is_end_of_text()) break;

		src_data->skip_space(1);

		if(src_data->is_end_of_line()) {
			src_data->move_cur_col(1);
		} else {
			break;
		}
	}
}

static int LintSqlMain(CEditData *src_data, CEditData *edit_data, 
	int indent_level, POINT *p_end_pt)
{
	bool			first_word_flg = true;
	bool			comma_next_line_mode = false;
	CString			cur_clause = _T("");
	POINT			end_pt = {-1, -1};
	CStrToken		*str_token = src_data->get_str_token();

	for(; !src_data->is_end_of_text();) {
		skip_space(src_data);
		if(is_end_p(src_data, p_end_pt)) break;

		// コメントブロックは，そのままコピーする
		int ch_type = src_data->check_char_type();
		if(ch_type == CHAR_TYPE_IN_COMMENT || ch_type == CHAR_TYPE_IN_ROWCOMMENT) {
			int tmp_indent_level = indent_level;
			if(!first_word_flg) tmp_indent_level++;
			CopyComment(src_data, edit_data, tmp_indent_level);
			continue;
		}

		if(src_data->is_end_of_text()) break;

		if(src_data->get_cur_char() == ';') {
			indent_level = 0;
			first_word_flg = true;
			comma_next_line_mode = false;
			cur_clause = "";
			src_data->move_cur_col(1);
			edit_data->paste(_T(";\n\n"));
			continue;
		}

		if(src_data->get_cur_char() == '(') {
			POINT brackets_end_pt;
			BOOL b_brackets = src_data->search_brackets_ex('(', ')', -1, p_end_pt, '\0', 1, &brackets_end_pt);

			int tmp_indent_level = indent_level + 1;

			CString row_buf = str_token->skipSpace(edit_data->get_row_buf(edit_data->get_cur_row()));
			row_buf.MakeLower();
			if(row_buf.Find(_T("insert into"), 0) == 0 || row_buf.Find(_T("values"), 0) == 0) {
				tmp_indent_level--;
				NextLine(edit_data, indent_level + 1);
			}

			edit_data->input_char('(');
			src_data->move_cur_col(1);
			if(b_brackets) {
				LintSqlMain(src_data, edit_data, tmp_indent_level, &brackets_end_pt);
				if(src_data->get_cur_char() == ')') {
					edit_data->input_char(')');
					src_data->move_cur_col(1);
				}
				skip_space(src_data);
				if(is_end_p(src_data, p_end_pt)) break;
				if(src_data->get_cur_char() != ',') edit_data->input_char(' ');
			} else {
				LintSqlMain(src_data, edit_data, tmp_indent_level, NULL);
				return 0;
			}
			continue;
		}

		if(is_end_p(src_data, p_end_pt)) break;

		const TCHAR *buf;
		{
			const TCHAR *p_start = src_data->get_row_buf(src_data->get_cur_row()) + src_data->get_cur_col();
			const TCHAR *p = str_token->get_word2(p_start, &buf);
			
			if(p == NULL) break;
			src_data->set_cur(src_data->get_cur_row(), src_data->get_cur_col() + (int)(p - p_start));
		}

		if(!first_word_flg && is_next_line_keyword(buf, cur_clause)) {
			NextLine(edit_data, indent_level);
			if(g_option.sql_lint.word_set_right && is_right_set_word(buf)) {
				put_space(edit_data, right_set_len - (int)_tcslen(buf)); 
			}
			comma_next_line_mode = false;
		}
		if(first_word_flg) {
			first_word_flg = false;
		}

		if(ostr_strcmp_nocase(buf, _T("between")) == 0 || 
			ostr_strcmp_nocase(buf, _T("delete")) == 0 ||
			ostr_strcmp_nocase(buf, _T("insert")) == 0) {
			cur_clause = buf;
			cur_clause.MakeLower();
		} else if(ostr_strcmp_nocase(buf, _T("select")) == 0 ||
			ostr_strcmp_nocase(buf, _T("from")) == 0 ||
			ostr_strcmp_nocase(buf, _T("set")) == 0) {
			comma_next_line_mode = true;
			cur_clause = buf;
			cur_clause.MakeLower();
		}

		{
			unsigned int prev_ch = edit_data->get_prev_char();

			if((prev_ch == ' ' || prev_ch == '(' || prev_ch == ',') &&
					edit_data->get_row_len(edit_data->get_cur_row()) + _tcslen(buf) > g_option.sql_lint.next_line_pos) {
				NextLine(edit_data, indent_level + 1);
			}
		}

		// 演算子の前後は空白にする
		if(isSQLOperatorChar(buf[0])) {
			unsigned int prev_ch = edit_data->get_prev_char();
			if(prev_ch != ' ' && prev_ch != '(' && prev_ch != ':') {
				edit_data->input_char(' ');
			}
			edit_data->paste(buf);

			// commentと混在しないようにする
			for(; isSQLOperatorChar(src_data->get_cur_char()); src_data->move_cur_col(1)) {
				const TCHAR *p = src_data->get_cur_buf();
				if(str_token->isOpenComment(p) || str_token->isRowComment(p)) break;

				edit_data->input_char(src_data->get_cur_char());
			}

			skip_space(src_data);
			if(is_end_p(src_data, p_end_pt)) break;

			if(src_data->get_cur_char() != ')') {
				edit_data->input_char(' ');
			}
			continue;
		}

		edit_data->paste(buf);

		if(comma_next_line_mode && buf[0] == ',') {
			if(g_option.sql_lint.comma_position_is_before_cname && 
				ostr_strcmp_nocase(cur_clause, _T("select")) == 0) {
				edit_data->back_space();
				NextLine(edit_data, indent_level + 1);
				edit_data->input_char(',');
			} else {
				NextLine(edit_data, indent_level + 1);
			}
		} else {
			unsigned int ch = src_data->get_cur_char();
			if(ch == '\0' || ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || buf[0] == ',' || ch == L'　') {
				edit_data->input_char(' ');
			}
		}
	}

	if(edit_data->is_blank_row(edit_data->get_cur_row())) {
		edit_data->del_row(edit_data->get_cur_row());
	}
	if(edit_data->get_prev_char() == ' ') edit_data->back_space();

	return 0;
}

int LintSql(const TCHAR *sql, CEditData *edit_data, CSQLStrToken *str_token)
{
	CEditData		src_data;
	src_data.set_str_token(str_token);
	src_data.paste(sql);
	src_data.set_cur(0, 0);

	CEditData sql_data;
	sql_data.set_str_token(str_token);

	CSqlParser		sql_parser;
	sql_parser.set_str_token(str_token);

	int start_row = 0;
	int end_row = src_data.get_row_cnt();

	for(int row = start_row; row < end_row;) {
		if(src_data.get_row_len(row) == 0) {
			if(row != end_row - 1) edit_data->paste(_T("\n"));
			row++;
			continue;
		}
		if(row >= end_row) break;

		row = sql_parser.GetSQL(row, &src_data);
		if(row == -1) break;

		const TCHAR	*sql_p = sql_parser.GetSqlBuf();
		// sqlが空のときは実行しない
		if(sql_p == NULL || *sql_p == '\0') continue;

		if(sql_parser.GetSqlKind() != SQL_NORMAL && sql_parser.GetSqlKind() != SQL_SELECT) {
			edit_data->paste(sql_p);
			edit_data->paste(_T("\n"));
			if(_tcscmp(src_data.get_row_buf(row - 1), _T("/")) == 0) {
				edit_data->paste(_T("/\n"));
			}
			continue;
		}

		sql_data.del_all();
		sql_data.paste(sql_p);
		sql_data.set_cur(0, 0);

		LintSqlMain(&sql_data, edit_data, 0, NULL);

		if(g_option.sql_lint.put_semicolon) {
			edit_data->paste(_T(";\n"));
		}
	}

	return 0;

//	CEditData		src_data;
//	src_data.set_str_token(str_token);
//	src_data.paste(sql);
//	src_data.set_cur(0, 0);
//
//	return LintSqlMain(&src_data, edit_data, 0, NULL);
}

