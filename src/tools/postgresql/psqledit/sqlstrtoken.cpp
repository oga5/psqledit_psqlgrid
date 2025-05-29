/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "strtoken.h"
#include "pglib.h"
#include "sqlstrtoken.h"

#include "ostrutil.h"
#include "mbutil.h"

const static TCHAR *plsql_object_type_list[] = {_T("FUNCTION"), _T("PROCEDURE"), _T("TRIGGER"), _T("TYPE"),};

static int compare_key_word(const void *key1, const void *key2) {
	return _tcscmp(((struct _key_word_st *)key1)->str, ((struct _key_word_st *)key2)->str);
}

/*------------------------------------------------------------------------------
 オブジェクト名リストの取得
------------------------------------------------------------------------------*/
static HPgDataset get_object_name_list(HPgSession ss, const TCHAR *owner, 
	CString object_type_list, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	_stprintf(sql_buf,
		_T("select distinct c.relname, c.relname as org_relname, \n")
		_T("	case c.relkind \n")
		_T("		when 'r' then 'table' \n")
		_T("		when 'i' then 'index' \n")
		_T("		when 'S' then 'sequence' \n")
		_T("		when 'v' then 'view' \n")
		_T("		when 's' then 'special' \n")
		_T("		when 't' then 'toast table' \n")
		_T("		else 'other' end as relkind \n")
		_T("from pg_class c \n")
		_T("where pg_get_userbyid(c.relowner) = '%s' \n"),
		owner);

	if(object_type_list != _T("")) {
		_tcscat(sql_buf, _T("and c.relkind in ("));
		_tcscat(sql_buf, object_type_list.GetBuffer(0));
		_tcscat(sql_buf, _T(") \n"));
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

/*------------------------------------------------------------------------------
 フィールドリストの取得(全部)
------------------------------------------------------------------------------*/
static HPgDataset get_all_column_list(HPgSession ss, const TCHAR *owner, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	_stprintf(sql_buf,
		_T("select distinct a.attname, a.attname as org_attname, \n")
		_T("	t.typname || '(' || a.attlen::text || ')' as typename \n")
		_T("from pg_attribute a, pg_class c, pg_type t \n")
		_T("where c.oid = a.attrelid \n")
		_T("and t.oid = a.atttypid \n")
		_T("and a.attnum > 0 \n")
		_T("and pg_get_userbyid(c.relowner) = '%s' \n"),
		owner);

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

CSQLStrToken::CSQLStrToken()
{
	m_completion_words = NULL;
	m_completion_word_cnt = 0;
	m_is_init_keyword = FALSE;

	initCharTable();

	m_dset_table = NULL;
	m_dset_column = NULL;

	InitializeCriticalSection(&m_critical_section);
}

CSQLStrToken::~CSQLStrToken()
{
	EnterCriticalSection(&m_critical_section);

	freeCompletionKeyword();

	FreeDataset();

	LeaveCriticalSection(&m_critical_section);

	DeleteCriticalSection(&m_critical_section);
}

void CSQLStrToken::freeCompletionKeyword()
{
	if(m_completion_words == NULL) return;

	free(m_completion_words);
	m_completion_words = NULL;
	m_completion_word_cnt = 0;
}

void CSQLStrToken::initCharTable()
{
	ClearCharTables();

	// コメントの文字列を設定
	SetOpenComment(_T("/*"));
	SetCloseComment(_T("*/"));
	SetRowComment(_T("--"));

	// 囲み文字の設定
	SetQuoteChars(_T("'\""));
	SetDoubleBracketIsEscapeChar('\'');

	SetBreakChars(_T(" ,;'\".%()=><[]"));
	SetBracketChars(_T(")"));
	SetOperatorChars(_T("=!*/+-%;.,(){}"));
}

int CSQLStrToken::addDatasetToKeywordList(HPgDataset dataset, const TCHAR *owner, TCHAR *msg_buf,
	int column_idx, int org_column_idx, int type_idx)
{
	int		i, idx, current_word_cnt;

	current_word_cnt = m_completion_word_cnt;
	m_completion_word_cnt = m_completion_word_cnt + pg_dataset_row_cnt(dataset);

	m_completion_words = (struct _key_word_st *)realloc(m_completion_words,
		sizeof(struct _key_word_st) * m_completion_word_cnt);
	if(m_completion_words == NULL) {
		_stprintf(msg_buf, _T("CSQLStrToken::initTableNameList(): memory allocate error"));
		return 1;
	}

	// datasetをキーワードリストにマージする
	for(i = 0, idx = current_word_cnt; i < pg_dataset_row_cnt(dataset); i++, idx++) {
		m_completion_words[idx].lwr_str = (TCHAR *)pg_dataset_data(dataset, i, column_idx);
		m_completion_words[idx].org_str = pg_dataset_data(dataset, i, org_column_idx);
		m_completion_words[idx].str = m_completion_words[idx].lwr_str;
		m_completion_words[idx].type_name = pg_dataset_data(dataset, i, type_idx);
		m_completion_words[idx].len = (unsigned int)_tcslen(m_completion_words[idx].str);

		my_mbslwr(m_completion_words[idx].lwr_str);

		if(m_max_key_word_len < m_completion_words[idx].len) {
			m_max_key_word_len = m_completion_words[idx].len;
		}

		// typeを設定
		m_completion_words[idx].type = 4;
	}

	// ここで実行すると異常終了するケースがある
	//initKeywordBuf(msg_buf);

	return 0;
}

void CSQLStrToken::FreeDataset()
{
	pg_free_dataset(m_dset_table);
	pg_free_dataset(m_dset_column);
	m_dset_table = NULL;
	m_dset_column = NULL;
}

int CSQLStrToken::initCompletionWord(HPgSession ss,
	const TCHAR *owner, TCHAR *msg_buf, BOOL b_object_name, BOOL b_column_name,
	CString object_type_list)
{
	int		ret_v;
	int		i;

	EnterCriticalSection(&m_critical_section);

	m_comp_key_words = m_key_words;
	m_comp_key_word_cnt = m_key_word_cnt;
	m_is_init_keyword = FALSE;

	freeCompletionKeyword();
	FreeDataset();

	m_completion_word_cnt = m_key_word_cnt;
	m_completion_words = (struct _key_word_st *)realloc(m_completion_words,
		sizeof(struct _key_word_st) * m_completion_word_cnt);
	if(m_completion_words == NULL) {
		_stprintf(msg_buf, _T("CSQLStrToken::initKeywordForCompletion(): memory allocate error"));
		ret_v = 1;
		goto ERR1;
	}

	for(i = 0; i < m_completion_word_cnt; i++) {
		m_completion_words[i] = m_key_words[i];
	}

	if(b_object_name == TRUE) {
		m_dset_table = get_object_name_list(ss, owner, object_type_list, msg_buf);
		if(m_dset_table == NULL) goto ERR1;

		ret_v = addDatasetToKeywordList(m_dset_table, owner, msg_buf, 0, 1, 2);
		if(ret_v != 0) goto ERR1;
	}

	if(b_column_name == TRUE) {
		m_dset_column = get_all_column_list(ss, owner, msg_buf);
		if(m_dset_column == NULL) goto ERR1;

		ret_v = addDatasetToKeywordList(m_dset_column, owner, msg_buf, 0, 1, 2);
		if(ret_v != 0) goto ERR1;
	}

	// キーワードをソートする(バイナリサーチするため)
	postAddKeyword(m_completion_words, m_completion_word_cnt);

	m_is_init_keyword = TRUE;
	m_comp_key_words = m_completion_words;
	m_comp_key_word_cnt = m_completion_word_cnt;
	
	LeaveCriticalSection(&m_critical_section);

	return 0;

ERR1:
	LeaveCriticalSection(&m_critical_section);
	return ret_v;
}

const TCHAR *CSQLStrToken::skipCommentAndSpace(const TCHAR *p)
{
	return skipCommentAndSpace2(p, FALSE);
}

TCHAR *CSQLStrToken::skipCommentAndSpace2(const TCHAR *p, BOOL b_rem)
{
	if(p == NULL) return NULL;

	for(;;) {
		// 先頭のスペースを読み飛ばす
		p = skipSpaceMultibyte(p);

		if(*p == '\0') return NULL;

		if((p[0] == '-' && p[1] == '-') || 
			(b_rem && ostr_strncmp_nocase(p, _T("REM"), 3) == 0 && isBreakChar(p[3]))) {
			p = _tcschr(p, '\n');
			if(p == NULL) return NULL;
			p++;
			continue;
		}

		if(!isOpenComment(p)) return (TCHAR *)p;
		p += GetOpenCommentLen();

		p = _tcsstr(p, GetCloseComment());
		// コメントが閉じられてないとき
		if(p == NULL) return NULL;
		p += GetCloseCommentLen();
	}

	// 末尾のスペースを読み飛ばす
	p = skipSpaceMultibyte(p);
	if(*p == '\0') return NULL;

	return (TCHAR *)p;
}

const TCHAR *CSQLStrToken::getObjectName(const TCHAR *p, TCHAR *name, int name_buf_size)
{
	_tcscpy(name, _T(""));

	p = get_word(p, name, name_buf_size);
	if(p == NULL) return NULL;

	// owner.object_nameの形式か調べる
	if(*p != '.') return p;

	p = get_word(p, name, name_buf_size);
	return p;
}

/*
 SQLを解析して，PL/SQLオブジェクトの作成SQLの場合，オブジェクト種類とオブジェクト名を取得する
 (object_type_listに記述してあるオブジェクトを識別する)
*/
BOOL CSQLStrToken::isPLSQL(const TCHAR *sql, TCHAR *object_type, TCHAR *object_name)
{
	const TCHAR *p = sql;
	TCHAR 	tmp[100];
	TCHAR 	type[100];
	TCHAR 	name[100];
	int		i;

	if(object_type != NULL) _tcscpy(object_type, _T(""));
	if(object_name != NULL) _tcscpy(object_name, _T(""));

	p = get_word(p, tmp, sizeof(tmp));
	if(p == NULL) return FALSE;
	my_mbsupr(tmp);

	if(_tcscmp(tmp, _T("CREATE")) != 0) return FALSE;

	p = get_word(p, type, sizeof(type));
	if(p == NULL) return FALSE;
	my_mbsupr(type);
	if(_tcscmp(type, _T("OR")) == 0) {
		p = get_word(p, tmp, sizeof(tmp));
		if(p == NULL) return FALSE;
		my_mbsupr(tmp);
		if(_tcscmp(tmp, _T("REPLACE")) != 0) return FALSE;

		p = get_word(p, type, sizeof(type));
		if(p == NULL) return FALSE;
		my_mbsupr(type);
	}

	for(i = 0; i < sizeof(plsql_object_type_list) / sizeof(plsql_object_type_list[0]); i++) {
		if(_tcscmp(type, plsql_object_type_list[i]) == 0) break;
	}
	if(i == sizeof(plsql_object_type_list) / sizeof(plsql_object_type_list[0])) return FALSE;

	p = getObjectName(p, name, sizeof(name));
	if(p == NULL) return FALSE;

	// nameがBODYかどうか比較する
	strcpy((char *)tmp, (char *)name);
	my_mbsupr(tmp);

	if(_tcscmp(tmp, _T("BODY")) == 0) {
		_tcscat(type, _T(" BODY"));
		p = getObjectName(p, name, sizeof(name));
	}

	// 結果の保存
	if(object_type != NULL) _tcscpy(object_type, type);
	if(object_name != NULL) {
		_tcscpy(object_name, name);
		// '"'で囲まれてない場合，オブジェクト名を大文字に変換する
if(p[0] != '"') my_mbsupr(object_name);
	}

	return TRUE;
}

BOOL CSQLStrToken::CheckSQL(const TCHAR* sql, const TCHAR* keyword)
{
	const TCHAR* p = sql;
	TCHAR	tmp[100];

	// カッコで囲まれたSelect文も，判定できるようにする
	for(;;) {
		p = skipCommentAndSpace(p);
		if(p == NULL || *p == '\0') return FALSE;

		if(*p != '(') break;
		p++;
	}

	p = get_word(p, tmp, sizeof(tmp));
	if(p == NULL) return FALSE;
	my_mbsupr(tmp);
	if(_tcscmp(tmp, keyword) == 0) return TRUE;

	return FALSE;
}

/*
 SQLを解析して，Select文かチェックする
*/
BOOL CSQLStrToken::isSelectSQL(const TCHAR* sql)
{
	return (CheckSQL(sql, _T("SELECT")) || CheckSQL(sql, _T("SHOW")));
}

/*
 SQLを解析して，Explain文かチェックする
*/
BOOL CSQLStrToken::isExplainSQL(const TCHAR* sql)
{
	return CheckSQL(sql, _T("EXPLAIN"));
}

/*
 SQLを解析して，DDL文かチェックする
*/
BOOL CSQLStrToken::isDDLSQL(const TCHAR* sql)
{
	return (CheckSQL(sql, _T("CREATE")) || CheckSQL(sql, _T("DROP")) || CheckSQL(sql, _T("ALTER")));
}

/*
 コマンドかチェックする
*/
BOOL CSQLStrToken::isCommand(const TCHAR* sql)
{
	if(*sql == '!') return TRUE;
	if(_tcsncmp(sql, _T("\\set"), 4) == 0) return TRUE;

	const TCHAR* p = sql;
	TCHAR	tmp[100];

	p = get_word(p, tmp, sizeof(tmp));
	if(p == NULL) return FALSE;
	my_mbsupr(tmp);
	if(_tcscmp(tmp, _T("DESC")) == 0 ||
		_tcscmp(tmp, _T("DESCRIBE")) == 0) return TRUE;
	if(_tcscmp(tmp, _T("CONNECT")) == 0 ||
		_tcscmp(tmp, _T("DISCONNECT")) == 0) return TRUE;

	return FALSE;
}

BOOL CSQLStrToken::isExecuteCommand(const TCHAR* sql)
{
	const TCHAR* p = sql;
	TCHAR	tmp[100];

	p = get_word(p, tmp, sizeof(tmp));
	if(p == NULL) return FALSE;
	my_mbsupr(tmp);

	if(_tcscmp(tmp, _T("EXEC")) == 0) return TRUE;
	if(_tcscmp(tmp, _T("EXECUTE")) == 0) return TRUE;

	return FALSE;
}

BOOL CSQLStrToken::isBindParam(const TCHAR* p)
{
	if(p == NULL || p[0] != ':') return FALSE;
	if(p[1] != '\'' && isBreakChar(p[1])) return FALSE;
	if(p[0] == ':' && p[1] == ':') return FALSE;

	return TRUE;
}

const TCHAR* CSQLStrToken::get_word_for_bind(const TCHAR* p, TCHAR* word_buf, int buf_size, CString &bind_name)
{
	if(*p != ':') {
		bind_name = _T("");
		return get_word(p, word_buf, buf_size, FALSE);
	}
	if(*p == ':' && *(p + 1) == ':') {
		bind_name = _T("");
		_tcscpy(word_buf, _T("::"));
		return p + 2;
	}

	const TCHAR* p_start = p;
	p++;

	if(isQuoteStart(p)) {
		int word_len = get_word_len_quote(p, TRUE);
		p += word_len;
	} else {
		int ch_len;

		for(;;) {
			if(*p == ':' && *(p + 1) == ':') break;

			ch_len = get_char_len(p);
			if(isBreakChar(get_char(p))) {
				break;
			}
			p += ch_len;
		}
	}

	_stprintf(word_buf, _T("%.*s"), p - p_start, p_start);

	if(word_buf[1] == '\'' && word_buf[_tcslen(word_buf) - 1] == '\'') {
		// ':'と両端のシングルクォートを削除するので-3
		bind_name.Format(_T("%.*s"), _tcslen(word_buf) - 3, word_buf + 2);
	} else {
		bind_name = word_buf + 1;
	}
	TRACE(_T("bind:%s:%s:\n"), word_buf, bind_name);

	return p;
}
