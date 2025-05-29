/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "sqlparser.h"

//
//
//
CSqlParser::CSqlParser()
{
	m_str_token = NULL;
	m_sql_buf = NULL;
	m_sql_buf_alloc_cnt = 0;
	m_skip_space_sql = NULL;
	m_sql_kind = SQL_BLANK;
}

CSqlParser::~CSqlParser()
{
	FreeSqlBuf();
}

TCHAR *CSqlParser::AllocSqlBuf(int size)
{
	if(m_sql_buf_alloc_cnt > size) return m_sql_buf;

	int alloc_cnt = m_sql_buf_alloc_cnt;
	if(alloc_cnt == 0) alloc_cnt = 2048;
	for(; size >= alloc_cnt;) alloc_cnt = alloc_cnt * 2;

	m_sql_buf = (TCHAR *)realloc(m_sql_buf, alloc_cnt * sizeof(TCHAR));
	if(m_sql_buf == NULL) {
		AfxMessageBox(_T("メモリが確保できません(error in AllocSqlBuf())"));
		m_sql_buf_alloc_cnt = 0;
		return NULL;
	}

	m_sql_buf_alloc_cnt = alloc_cnt;
	m_skip_space_sql = NULL;

	return m_sql_buf;
}

void CSqlParser::FreeSqlBuf()
{
	if(m_sql_buf != NULL) {
		free(m_sql_buf);
		m_sql_buf = NULL;
		m_sql_buf_alloc_cnt = 0;
		m_skip_space_sql = NULL;
		m_sql_kind = SQL_BLANK;
	}
}

int CSqlParser::GetSQL(int start_row, CEditData *edit_data)
{
	ASSERT(m_str_token != NULL);

	int		i;
	int		loop_cnt;
	int		row;
	int		pl_sql_flg = 0;
	int		sql_len = 0;
	int		semicolon_col;
	TCHAR	*p;

	m_sql_kind = SQL_NORMAL;
	if(m_sql_buf == NULL && AllocSqlBuf(1024) == NULL) return -1;
	m_sql_buf[0] = '\0';
	m_skip_space_sql = NULL;
	p = m_sql_buf;

	loop_cnt = edit_data->get_row_cnt() - start_row;

	for(i = 0, row = start_row; i < loop_cnt; i++, row++) {
		// '/'だけの行がきたら，SQLの区切りと判断
		if(_tcscmp(edit_data->get_row_buf(row), _T("/")) == 0 &&
			m_skip_space_sql != NULL) break;

		// バッファサイズを計算
		if(i != 0) sql_len++;	// 改行分
		sql_len = sql_len + edit_data->get_row_len(row);

		// バッファを拡張
		// '\0'や末尾のスペースなどのために，10バイト余計に取得しておく
		if((sql_len + 10) >= m_sql_buf_alloc_cnt) {
			if(AllocSqlBuf(sql_len + 10) == NULL) return -1;
			p = m_sql_buf + _tcslen(m_sql_buf);

			// メモリの位置が変わるので，m_skip_space_sqlを再計算
			m_skip_space_sql = m_str_token->skipCommentAndSpace2(m_sql_buf, TRUE);
		}

		// SQLを作成
		if(i != 0) {
			_tcscpy(p, _T("\n"));	// 改行を追加
			p++;
		}
		_tcscpy(p, edit_data->get_row_buf(row));
		p = p + edit_data->get_row_len(row);

		if(pl_sql_flg == 1 || edit_data->get_row_len(row) == 0) continue;

		// コメントを読み飛ばす
		if(m_skip_space_sql == NULL) {
			m_skip_space_sql = m_str_token->skipCommentAndSpace2(m_sql_buf, TRUE);
			if(m_skip_space_sql == NULL) continue;
		}

		// コマンドの処理
		{
			// ファイルコマンド
			if(*m_skip_space_sql == '@') {
				m_sql_kind = SQL_FILE_RUN;
				break;
			}

			// コマンド
			if(m_str_token->isCommand(m_skip_space_sql)) {
				m_sql_kind = SQL_COMMAND;
				break;
			}
		}

		// 末尾に ';'がきたときの処理
		if(inline_strchr(edit_data->get_row_buf(row), ';')) {
			if(m_str_token->isPLSQL(m_skip_space_sql, NULL, NULL)) {
				pl_sql_flg = 1;
			} else {
				if(is_sql_end2(m_sql_buf, &semicolon_col)) {
					// 末尾の';'を取り除く
					m_sql_buf[semicolon_col] = ' ';
					break;
				}
			}
		}
	}

	// コマンドのとき，末尾の';'と改行は削除する
	if(m_sql_kind != SQL_NORMAL) {
		if(m_sql_buf[sql_len - 1] == ';') m_sql_buf[sql_len - 1] = '\0';

		for(;;) {
			if(m_sql_buf[sql_len - 1] != '\n') break;
			m_sql_buf[sql_len - 1] = '\0';
		}
	}

	// PL/SQLを判定する
	if(pl_sql_flg) {
		m_sql_kind = SQL_PLSQL;
	} else if(m_skip_space_sql != NULL) {
		// select文を判定する
		if(m_str_token->isSelectSQL(m_skip_space_sql)) {
			m_sql_kind = SQL_SELECT;
		} else if(m_sql_kind == SQL_NORMAL && 
			m_str_token->isPLSQL(m_skip_space_sql, NULL, NULL)) {
			// wrapped PL/SQLを判定する
			pl_sql_flg = 1;
			m_sql_kind = SQL_PLSQL;
		}
	}

	// select * from tab"
	// 上のsqlのように，末尾が不要な"で終わるとき，oparse()が戻ってこないバグを回避する
	if(m_sql_buf[sql_len - 1] == '"') {
		_tcscat(m_sql_buf, _T(" "));
	}

	if(row != edit_data->get_row_cnt()) row++;

	return row;
}

// セミコロンの後に，コメント以外のtokenがあるか調べる
static BOOL check_line_end(const TCHAR *p, CStrToken *token)
{
	p = token->skipCommentAndSpace(p);
	if(p == NULL || *p == '\0') return TRUE;
	return FALSE;
}

BOOL CSqlParser::is_sql_end2(const TCHAR *p, int *semicolon_col)
{
	const TCHAR *p_start = p;

	for(; *p != '\0';) {
		p = m_str_token->skipCommentAndSpace(p);
		if(p == NULL) break;

		if(*p == ';') {
			if(check_line_end(p + 1, m_str_token)) {
				if(semicolon_col) *semicolon_col = (int)(p - p_start);
				return TRUE;
			}
		}
		p += m_str_token->get_sql_word_len(p);
	}

	return FALSE;
}

// 行の末尾に';'があるかチェックする(スペースは無視する)
BOOL is_sql_end(CEditData *edit_data, int row, CStrToken *token)
{
	TCHAR *p_start = edit_data->get_row_buf(row);
	TCHAR *p = p_start;

	// FIXME: 以下のケースで正しく認識できない
	// (1)insert into zzz values('1', '2"');などのように、シングルクォート内に
	//    ダブルクォートがある
	// (2)insert into zzz values('1', '
	//    2;
	//    ');などのように複数行のテキストの末尾にセミコロンがある
	for(; *p != '\0';) {
		if(*p == ';') {
			int char_type = edit_data->check_char_type(row, (int)(p - p_start));
			if((char_type == CHAR_TYPE_NORMAL || char_type == CHAR_TYPE_IN_QUOTE) &&
				check_line_end(p + 1, token)) {
				return TRUE;
			}
		}
		if(*p == '\'') {
			// 複数行のテキストをシングルクォートで囲んだとき，SQLの区切りを正しく
			// 認識できないので、シングルクォートは全部無視する
			p++;
		} else {
			p += token->get_word_len(p);
		}
	}

	return FALSE;
}

