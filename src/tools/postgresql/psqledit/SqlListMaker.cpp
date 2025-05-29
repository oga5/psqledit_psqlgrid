/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "SqlListMaker.h"
#include "sqlparse.h"
#include "ostrutil.h"
#include "str_inline.h"
#include "mbutil.h"

#include "global.h"
#include "localsql.h"
#include "getsrc.h"

#define SQL_LIST_MAKER_OBJECT_CACHE_SIZE	10

void CSqlListMaker::SetSortColumns(BOOL b_sort)
{
	if(m_sort_column_name == b_sort) return;
	m_sort_column_name = b_sort;
	CSqlListMaker::ClearCache();
}

BOOL						CSqlListMaker::m_sort_column_name = TRUE;

/*------------------------------------------------------------------------------
 オブジェクトリストの取得(スキーマ指定)
------------------------------------------------------------------------------*/
static HPgDataset get_object_list_in_schema_for_list_maker(HPgSession ss, const TCHAR* schema, TCHAR* msg_buf)
{
	TCHAR	sql_buf[4096];
	CString tmp_schema;

	if(pg_get_remote_version(ss) < 70300) return NULL;

	_stprintf(sql_buf,
		_T("select t.tablename attname, 'TABLE' typename from pg_tables t \n")
		_T("where t.schemaname = '%s' \n")
		_T("union all \n")
		_T("select v.viewname attname, 'VIEW' typename from pg_views v \n")
		_T("where v.schemaname = '%s' \n")
		_T("union all \n")
		_T("select p.proname attname, 'FUNCTION' typename \n")
		_T("from pg_proc p, pg_namespace n \n")
		_T("where p.pronamespace = n.oid \n")
		_T("and n.nspname = '%s' \n")
		_T("order by 1  \n"),
		schema, schema, schema);

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

/*------------------------------------------------------------------------------
 フィールドリストの取得(全部)
------------------------------------------------------------------------------*/
static HPgDataset get_column_list_for_list_maker(HPgSession ss, const TCHAR *schema, const TCHAR *table_name, BOOL sort_column_name, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	TCHAR	sort_col[100];
	CString tmp_schema;

	if(_tcslen(table_name) == 0) {
		return NULL;
	}

	if(pg_get_remote_version(ss) >= 70300 && (schema == NULL || schema[0] == '\0')) {
		tmp_schema = get_table_schema_name(ss, table_name, msg_buf);
		if(!tmp_schema.IsEmpty()) {
			schema = tmp_schema.GetBuffer(0);
		}
	}

	if(sort_column_name) {
		_tcscpy(sort_col, _T("attname"));
	} else {
		_tcscpy(sort_col, _T("attnum"));
	}

	if(pg_get_remote_version(ss) >= 70300 && schema != NULL && schema[0] != '\0') {
		_stprintf(sql_buf,
			_T("select a.attname, \n")
			_T("	t.typname || '(' || \n")
			_T("		(case \n")
			_T("			when t.typname = 'numeric' and (atttypmod - 4) %% 65536 = 0 then to_char((atttypmod - 4) / 65536, 'FM999999') \n")
			_T("			when t.typname = 'numeric' then to_char((atttypmod - 4) / 65536, 'FM999999') || ',' || to_char((atttypmod - 4) %% 65536, 'FM999999') \n")
			_T("			when a.atttypmod > 4 then to_char(a.atttypmod - 4, 'FM999999') \n")
			_T("			else to_char(a.attlen, 'FM999999') \n")
			_T("		 end) || \n")
			_T("		')' as typename \n")
			_T("from pg_attribute a, pg_class c, pg_type t \n")
			_T("where c.oid = a.attrelid \n")
			_T("and t.oid = a.atttypid \n")
			_T("and a.attnum > 0 \n")
			_T("and c.relnamespace = (select oid from pg_namespace where nspname = '%s') \n")
			_T("and c.relname = '%s' \n")
			_T("order by %s \n"),
			schema, table_name, sort_col);
	} else {
		_stprintf(sql_buf,
			_T("select a.attname, \n")
			_T("	t.typname || '(' || \n")
			_T("		(case \n")
			_T("			when t.typname = 'numeric' and (atttypmod - 4) %% 65536 = 0 then to_char((atttypmod - 4) / 65536, 'FM999999') \n")
			_T("			when t.typname = 'numeric' then to_char((atttypmod - 4) / 65536, 'FM999999') || ',' || to_char((atttypmod - 4) %% 65536, 'FM999999') \n")
			_T("			when a.atttypmod > 4 then to_char(a.atttypmod - 4, 'FM999999') \n")
			_T("			else to_char(a.attlen, 'FM999999') \n")
			_T("		 end) || \n")
			_T("		')' as typename \n")
			_T("from pg_attribute a, pg_class c, pg_type t \n")
			_T("where c.oid = a.attrelid \n")
			_T("and t.oid = a.atttypid \n")
			_T("and a.attnum > 0 \n")
			_T("and c.relname = '%s' \n")
			_T("order by %s \n"),
			table_name, sort_col);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

static BOOL isTableName(CStrToken *str_token, const CString &table_name)
{
	const TCHAR *p = table_name;

	for(; *p != '\0'; p++) {
		if(inline_isalnum(*p) || is_lead_byte(*p)) break;
	}
	if(ostr_strncmp_nocase(p, _T("select"), 6) == 0) return FALSE;

	return TRUE;
}

CSqlListMakerCache_Object::CSqlListMakerCache_Object()
{
	m_object_name = new CString[SQL_LIST_MAKER_OBJECT_CACHE_SIZE];
	m_dataset_arr = (HPgDataset **)malloc(sizeof(HPgDataset *) * SQL_LIST_MAKER_OBJECT_CACHE_SIZE);
	m_dataset_buf = (HPgDataset *)malloc(sizeof(HPgDataset) * SQL_LIST_MAKER_OBJECT_CACHE_SIZE);
	for(int i = 0; i < SQL_LIST_MAKER_OBJECT_CACHE_SIZE; i++) {
		m_dataset_buf[i] = NULL;
	}
	ClearCache();
}

CSqlListMakerCache_Object::~CSqlListMakerCache_Object()
{
	delete[] m_object_name;
	for(int i = 0; i < SQL_LIST_MAKER_OBJECT_CACHE_SIZE; i++) {
		pg_free_dataset(m_dataset_buf[i]);
		m_dataset_buf[i] = NULL;
	}
	free(m_dataset_arr);
	free(m_dataset_buf);
}

void CSqlListMakerCache_Object::ClearCache()
{
	int		i;
	for(i = 0; i < SQL_LIST_MAKER_OBJECT_CACHE_SIZE; i++) {
		pg_free_dataset(m_dataset_buf[i]);
		m_dataset_buf[i] = NULL;
	}
	for(i = 0; i < SQL_LIST_MAKER_OBJECT_CACHE_SIZE; i++) {
		m_object_name[i] = "";
		m_dataset_arr[i] = &m_dataset_buf[i];
	}
}

void CSqlListMakerCache_Object::LRU(int idx)
{
	CString			object_name;
	HPgDataset		*dataset;

	if(idx == -1) {
		idx = SQL_LIST_MAKER_OBJECT_CACHE_SIZE - 1;
	}

	object_name = m_object_name[idx];
	dataset = m_dataset_arr[idx];

	for(int i = idx; i > 0; i--) {
		m_object_name[i] = m_object_name[i - 1];
		m_dataset_arr[i] = m_dataset_arr[i - 1];
	}
	m_object_name[0] = object_name;
	m_dataset_arr[0] = dataset;
}

BOOL CSqlListMakerCache_Object::MakeList(CString object_name, HPgDataset *&dataset, BOOL sort_column_name)
{
	CString owner = "";
	CString obj_name = "";
	BOOL have_dot_flg = FALSE;

	if(object_name.Find('.') != -1) {
		have_dot_flg = TRUE;
		owner = object_name.Left(object_name.Find('.'));
		obj_name = object_name.Right(object_name.GetLength() - object_name.Find('.') - 1);
	} else {
		//owner = pg_user(g_ss);
		obj_name = object_name;
	}

	//if(owner != "" && owner.GetAt(0) != '"') owner.MakeLower();
	if(obj_name != "" && obj_name.GetAt(0) != '"') obj_name.MakeLower();
	
	int		i;
	for(i = 0; i < SQL_LIST_MAKER_OBJECT_CACHE_SIZE; i++) {
		if(object_name == m_object_name[i]) {
			if(i != 0) LRU(i);
			dataset = m_dataset_arr[0];
			return TRUE;
		}
	}

	if(m_object_name[0] != "") {
		LRU(-1);
	}

	HPgDataset *dataset_tmp = m_dataset_arr[0];

	pg_free_dataset(*dataset_tmp);
	*dataset_tmp = NULL;

	*dataset_tmp = get_column_list_for_list_maker(g_ss, owner.GetBuffer(0), obj_name.GetBuffer(0), sort_column_name, NULL);
	if(*dataset_tmp == NULL) return FALSE;

	if(pg_dataset_row_cnt(*dataset_tmp) == 0) {
		pg_free_dataset(*dataset_tmp);
		*dataset_tmp = NULL;

		if(have_dot_flg) {
			return FALSE;
		}

		*dataset_tmp = get_object_list_in_schema_for_list_maker(g_ss, obj_name.GetBuffer(0), NULL);
		if(pg_dataset_row_cnt(*dataset_tmp) == 0) {
			pg_free_dataset(*dataset_tmp);
			*dataset_tmp = NULL;
			return FALSE;
		}
	}

	m_object_name[0] = object_name;
	dataset = dataset_tmp;

	return TRUE;
}

CSqlListMaker::CSqlListMaker()
{
}

BOOL CSqlListMaker::GetSqlRow(CEditData *edit_data, POINT &start_pt, POINT &end_pt)
{
	int		row;

	// START行を見つける
	row = edit_data->get_cur_row() - 1;
	for(; row >= 0; row--) {
		if(is_sql_end(edit_data, row)) {
			start_pt.y = row + 1;
			start_pt.x = 0;
			break;
		}
	}
	if(row < 0) {
		start_pt.x = 0;
		start_pt.y = 0;
	}

	// コメントをスキップ
	int char_type = edit_data->check_char_type(start_pt.y, start_pt.x);
	for(; char_type == CHAR_TYPE_IN_COMMENT || char_type == CHAR_TYPE_IN_ROWCOMMENT;) {
		if(start_pt.y == edit_data->get_cur_row()) break;
		start_pt.y++;
		char_type = edit_data->check_char_type(start_pt.y, start_pt.x);
	}

	// END行を見つける
	for(row = start_pt.y; row < edit_data->get_row_cnt(); row++) {
		if(is_sql_end(edit_data, row)) {
			end_pt.y = row;
			end_pt.x = edit_data->get_row_len(end_pt.y) - 1;
			break;
		}
	}
	if(row == edit_data->get_row_cnt()) {
		end_pt.y = edit_data->get_row_cnt() - 1;
		end_pt.x = edit_data->get_row_len(end_pt.y) - 1;
	}

	return TRUE;
}

void CSqlListMaker::ParseBracket(CEditData *edit_data, POINT parse_start_pt, POINT parse_end_pt,
	POINT &bracket_end_pt)
{
	CStrToken *str_token = edit_data->get_str_token();
	POINT pt1, pt2;
	TCHAR *p;
	int		cnt = 0;
	int		len;

	pt1 = parse_start_pt;
	pt2 = parse_start_pt;
	
	for(;;) {
		pt2 = edit_data->skip_comment_and_space(pt2, TRUE, FALSE);
		if(pt2.y > parse_end_pt.y || (pt2.y == parse_end_pt.y && pt2.x > parse_end_pt.x)) {
			bracket_end_pt = pt2;
			break;
		}
		p = edit_data->get_row_buf(pt2.y) + pt2.x;
		len = str_token->get_word_len(p);

		if(*p == '\0') {
			if(pt2.y == parse_end_pt.y || pt2.y >= edit_data->get_row_cnt() - 1) {
				bracket_end_pt = pt2;
				break;
			}
			pt2.y++;
			pt2.x = 0;
			p = edit_data->get_row_buf(pt2.y);
		}
		if(len == 1 && *p == '(') cnt++;
		if(len == 1 && *p == ')') {
			cnt--;
			if(cnt == 0) {
				bracket_end_pt = pt2;
				break;
			}
		}

		p += len;
		pt2.x += len;
	}
}

static BOOL IsCurPtInArea(CEditData *edit_data, POINT pt1, POINT pt2)
{
	if(pt1.y > edit_data->get_cur_row() || pt2.y < edit_data->get_cur_row()) return FALSE;
	if(pt1.y == edit_data->get_cur_row() && pt1.x > edit_data->get_cur_col()) return FALSE;
	if(pt2.y == edit_data->get_cur_row() && pt2.x < edit_data->get_cur_col()) return FALSE;
	return TRUE;
}

static BOOL IsCurPtOver(CEditData *edit_data, POINT pt)
{
	if(pt.y < edit_data->get_cur_row()) return FALSE;
	if(pt.y == edit_data->get_cur_row() && pt.x < edit_data->get_cur_col()) return FALSE;
	return TRUE;
}

static void SkipBracket(CEditData *edit_data, POINT bracket_end_pt, POINT &cur_pt,
	TCHAR *&p, int &len)
{
	cur_pt = bracket_end_pt;

	p = edit_data->get_row_buf(cur_pt.y);
	p += cur_pt.x;
	if(*p == ')') p++; // 閉じ括弧の次に移動
	len = 0;
}

static void AppendStringMap(CMapStringToString &map1, CMapStringToString &map2)
{
	CString key, data;

	POSITION pos = map2.GetStartPosition();
	for(; pos != NULL;) {
		map2.GetNextAssoc(pos, key, data);
		map1.SetAt(key, data);
	}
}

static BOOL check_word(const TCHAR *p, int plen, const TCHAR *word, int word_len)
{
	if(plen != word_len) return FALSE;
	return (ostr_strncmp_nocase(p, word, word_len) == 0);
}

BOOL CSqlListMaker::MakeTableMap(CMapStringToString &table_map,
	CEditData *edit_data, POINT start_pt, POINT end_pt)
{
	if(g_login_flg == NULL) return FALSE;

	CStrToken *str_token = edit_data->get_str_token();
	TCHAR *p;
	int		len;
	POINT	cur_pt;

	BOOL	from_flg = FALSE;
	CMapStringToString	local_table_map;
	CString	table_name;
	CString	alias_name;

	table_name = "";
	alias_name = "";
	
	cur_pt = start_pt;
	p = edit_data->get_row_buf(cur_pt.y);
	p += cur_pt.x;

	for(;;) {
		if(*p == '\0') {
			if(cur_pt.y >= end_pt.y) break;
			cur_pt.y++;
			cur_pt.x = 0;
			p = edit_data->get_row_buf(cur_pt.y);
		}

		cur_pt = edit_data->skip_comment_and_space(cur_pt, TRUE, FALSE);
		if(cur_pt.y > end_pt.y) break;
		if(cur_pt.y == end_pt.y && cur_pt.x > end_pt.x) break;

		p = edit_data->get_row_buf(cur_pt.y) + cur_pt.x;
		len = str_token->get_word_len(p);

		if((len == 5 || len == 9) && 
			(ostr_strncmp_nocase(p, _T("union"), 5) == 0 ||
				ostr_strncmp_nocase(p, _T("minus"), 5) == 0 ||
				ostr_strncmp_nocase(p, _T("intersect"), 9) == 0)) {

			if(IsCurPtOver(edit_data, cur_pt)) {
				break;
			}

			local_table_map.RemoveAll();
			from_flg = 0;
			table_name = "";
		}

		if(from_flg) {
			// skip 'as'
			if(check_word(p, len, _T("as"), 2)) goto NEXT_WORD;

			if(len == 1 && *p == '(') {
				// inline view
				POINT bracket_end_pt;
				ParseBracket(edit_data, cur_pt, end_pt, bracket_end_pt);
				cur_pt.x++;

				if(IsCurPtInArea(edit_data, cur_pt, bracket_end_pt)) {
					MakeTableMap(table_map, edit_data, cur_pt, bracket_end_pt);
					return TRUE;
				} else {
					table_name = edit_data->get_point_text(cur_pt, bracket_end_pt);
				}
				SkipBracket(edit_data, bracket_end_pt, cur_pt, p, len);
			} else if((len == 5 || len == 6) && 
				(ostr_strncmp_nocase(p, _T("where"), 5) == 0 ||
					ostr_strncmp_nocase(p, _T("order"), 5) == 0 ||
					ostr_strncmp_nocase(p, _T("group"), 5) == 0 ||
					ostr_strncmp_nocase(p, _T("values"), 6) == 0)) {
				from_flg = 0;
				table_name = _T("");
			} else if(len == 1 && *p == ',') {
				table_name = _T("");
			} else if(len > 1 || !str_token->isBreakChar(*p)) {
				if(*(p + len) == '.' || *(p + len) == '@') {
					len++;
					for(;; len += get_char_len(p + len)) {
						if(*(p + len) != '@' && str_token->isBreakChar(*(p + len))) break;
					}
				}
				if(table_name != _T("")) {
					alias_name.Format(_T("%.*s"), len, p);
					local_table_map.SetAt(alias_name, table_name);
					table_name = "";
				} else {
					table_name.Format(_T("%.*s"), len, p);
				}
			}
		} else if((len == 4 || len == 6) && 
			(ostr_strncmp_nocase(p, _T("from"), 4) == 0 ||
			ostr_strncmp_nocase(p, _T("into"), 4) == 0 ||
				ostr_strncmp_nocase(p, _T("update"), 6) == 0)) {
			// select, delete, update, insertに対応
			from_flg = 1;
			table_name = "";
		} else if(len == 1 && *p == '(') {
			// subquery
			POINT bracket_end_pt;
			ParseBracket(edit_data, cur_pt, end_pt, bracket_end_pt);
			cur_pt.x++;

			if(IsCurPtInArea(edit_data, cur_pt, bracket_end_pt)) {
				MakeTableMap(local_table_map, edit_data, cur_pt, bracket_end_pt);
			}
			SkipBracket(edit_data, bracket_end_pt, cur_pt, p, len);
		}

NEXT_WORD:
		p += len;
		cur_pt.x += len;
		if(cur_pt.y == end_pt.y && cur_pt.x >= end_pt.x) break;
	}

	AppendStringMap(table_map, local_table_map);

	return TRUE;
}

// inline viewの処理
BOOL CSqlListMaker::MakeObjectList_InlineView(CCodeAssistData *assist_data, 
	const CString &object_name)
{
	const TCHAR *p = object_name;
	int				len;

	// selectを読み飛ばす
	p = g_sql_str_token.skipCommentAndSpace(p);
	len = g_sql_str_token.get_word_len(p);
	if(ostr_strncmp_nocase(p, _T("select"), 6) != 0) return FALSE;
	p += len;

	assist_data->InitData(FALSE);
	TCHAR	*buf_p = m_buffer.GetBuffer(object_name.GetLength());
	int				idx = 0;
	BOOL			as_flg = FALSE;

	// カラムリストを作成
	for(;;) {
		p = g_sql_str_token.skipCommentAndSpace(p);
		if(p == NULL || *p == '\0') break;

		if(*p == '(') {
			p = g_sql_str_token.skipBracket(p);
			continue;
		}

		len = g_sql_str_token.get_word_len(p);

		if(*p == ',' || ostr_strncmp_nocase(p, _T("from"), 4) == 0) {
			if(_tcscmp(buf_p, _T("*")) == 0) break;

			p += len;
			assist_data->AddRow();
			assist_data->SetData(idx, 0, buf_p);
			assist_data->SetData(idx, 1, _T(""));
			idx++;

			if(ostr_strncmp_nocase(p, _T("from"), 4) == 0) break;

			buf_p += _tcslen(buf_p) + 1;
		}

		_stprintf(buf_p, _T("%.*s"), len, p);
		p += len;
	}

	m_buffer.ReleaseBuffer();

	if(idx == 0) return FALSE;

	return TRUE;
}

BOOL CSqlListMaker::MakeObjectList(CCodeAssistData *assist_data, const CString &object_name)
{
	if(!isTableName(&g_sql_str_token, object_name)) {
		return MakeObjectList_InlineView(assist_data, object_name);
	}

	HPgDataset *dataset = NULL;
	if(m_cache_object.MakeList(object_name, dataset, m_sort_column_name) == FALSE) return FALSE;

	assist_data->InitData(FALSE);
	for(int i = 0; i < pg_dataset_row_cnt(*dataset); i++) {
		assist_data->AddRow();
		assist_data->SetData(i, 0, pg_dataset_data(*dataset, i, 0));
		assist_data->SetData(i, 1, pg_dataset_data(*dataset, i, 1));
	}

	return TRUE;
}

BOOL CSqlListMaker::MakeList(CCodeAssistData *assist_data, CEditData *edit_data)
{
	CString alias_name;

	int char_type = edit_data->check_char_type();
	if(char_type == CHAR_TYPE_IN_ROWCOMMENT || char_type == CHAR_TYPE_IN_COMMENT || 
		char_type == CHAR_TYPE_IN_QUOTE) return FALSE;

	POINT	cur_pt;
	cur_pt.y = edit_data->get_cur_row();
	cur_pt.x = edit_data->get_cur_col();
	edit_data->move_cur_col(-1);

	alias_name = edit_data->get_word(GET_WORD_CURRENT_PT, GET_WORD_MOVE_METHOD_BREAK_CHAR);
	edit_data->move_break_char(-1);
	if(edit_data->get_prev_char() == '.') {
		edit_data->move_cur_col(-1);
		CString owner = edit_data->get_word(GET_WORD_CURRENT_PT, GET_WORD_MOVE_METHOD_BREAK_CHAR);
		alias_name = owner + _T(".") + alias_name;
	}
	edit_data->set_cur(cur_pt.y, cur_pt.x);
	if(alias_name == _T("") || inline_isdigit(alias_name.GetBuffer(0)[0])) return FALSE;

	POINT	start_pt, end_pt;
	if(GetSqlRow(edit_data, start_pt, end_pt) == FALSE) return FALSE;

	m_table_map.RemoveAll();
	if(MakeTableMap(m_table_map, edit_data, start_pt, end_pt) == FALSE) return FALSE;

	CString table_name;
	if(m_table_map.Lookup(alias_name, table_name) == FALSE) {
		table_name = alias_name;
	}

	return MakeObjectList(assist_data, table_name);
}

CSqlListMakerCache_Object	CSqlListMaker::m_cache_object;

