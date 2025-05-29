/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdafx.h>
#include "shortcutsql.h"

#include "Resource.h"

#define PROFILE_SECTION_NAME		_T("SHORTCUT_SQL")
#define LINE_FEED_SPECIAL_CHAR		_T("<meta>\\n:;</mata>")

CShortCutSql::CShortCutSql()
{
	m_name = _T("");
	m_sql = _T("");
	m_cmd = ID_SHORT_CUT_SQL_DUMMY;
	m_show_dlg = FALSE;
	m_paste_to_editor = FALSE;
}

CShortCutSql::CShortCutSql(CString name, CString sql, WORD cmd, BOOL show_dlg,
	BOOL paste_to_editor)
{
	m_name = name;
	m_sql = sql;
	m_cmd = cmd;
	m_show_dlg = show_dlg;
	m_paste_to_editor = paste_to_editor;

	if(m_paste_to_editor) m_show_dlg = FALSE;
}

CShortCutSqlList::CShortCutSqlList()
{
}

CShortCutSqlList::~CShortCutSqlList()
{
	ClearArray();
}

void CShortCutSqlList::ClearArray()
{
	int cnt = (int)m_list.GetSize();
	
	for(int i = 0; i < cnt; i++) {
		CShortCutSql *data = GetShortCutSql(i);
		m_list.SetAt(i, NULL);
		delete data;
	}

	m_list.RemoveAll();
}

void CShortCutSqlList::MakeRegIdx(int i, CString &name_idx, CString &sql_idx,
	CString &cmd_idx, CString &show_dlg_idx, CString &paste_to_editor_idx)
{
	name_idx.Format(_T("NAME_%d"), i);
	sql_idx.Format(_T("SQL_%d"), i);
	cmd_idx.Format(_T("CMD_%d"), i);
	show_dlg_idx.Format(_T("SHOW_DLG_%d"), i);
	paste_to_editor_idx.Format(_T("PASTE_TO_EDITOR_%d"), i);
}

BOOL CShortCutSqlList::Load()
{
	int cnt = AfxGetApp()->GetProfileInt(PROFILE_SECTION_NAME, _T("SQL_CNT"), 0);

	CString name_idx, sql_idx, cmd_idx, show_dlg_idx, paste_to_editor_idx;

	ClearArray();
	for(int i = 0; i < cnt; i++) {
		MakeRegIdx(i, name_idx, sql_idx, cmd_idx, show_dlg_idx, paste_to_editor_idx);

		CString sql = AfxGetApp()->GetProfileString(PROFILE_SECTION_NAME, sql_idx, _T(""));
		sql.Replace(LINE_FEED_SPECIAL_CHAR, _T("\r\n"));

		CShortCutSql *data = new CShortCutSql(
			AfxGetApp()->GetProfileString(PROFILE_SECTION_NAME, name_idx, _T("")),
			sql,
			AfxGetApp()->GetProfileInt(PROFILE_SECTION_NAME, cmd_idx, 0),
			AfxGetApp()->GetProfileInt(PROFILE_SECTION_NAME, show_dlg_idx, FALSE),
			AfxGetApp()->GetProfileInt(PROFILE_SECTION_NAME, paste_to_editor_idx, FALSE));
		if(data == NULL) return FALSE;

		m_list.Add(data);
	}

	return TRUE;
}

BOOL CShortCutSqlList::ClearRegistry()
{
	CString name_idx, sql_idx, cmd_idx, show_dlg_idx, paste_to_editor_idx;

	for(int i = 0; i < MAX_SHORT_CUT_SQL; i++) {
		MakeRegIdx(i, name_idx, sql_idx, cmd_idx, show_dlg_idx, paste_to_editor_idx);

		if(AfxGetApp()->WriteProfileString(PROFILE_SECTION_NAME, name_idx, NULL) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileString(PROFILE_SECTION_NAME, sql_idx, NULL) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileString(PROFILE_SECTION_NAME, cmd_idx, NULL) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileString(PROFILE_SECTION_NAME, show_dlg_idx, NULL) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileString(PROFILE_SECTION_NAME, paste_to_editor_idx, NULL) == FALSE) return FALSE;
	}

	return TRUE;
}

BOOL CShortCutSqlList::Save()
{
	int cnt = (int)m_list.GetSize();
	AfxGetApp()->WriteProfileInt(PROFILE_SECTION_NAME, _T("SQL_CNT"), cnt);

	ClearRegistry();

	CString name_idx, sql_idx, cmd_idx, show_dlg_idx, paste_to_editor_idx;

	for(int i = 0; i < cnt; i++) {
		MakeRegIdx(i, name_idx, sql_idx, cmd_idx, show_dlg_idx, paste_to_editor_idx);

		CShortCutSql *data = GetShortCutSql(i);

		CString sql = data->GetSql();
		sql.Replace(_T("\r\n"), LINE_FEED_SPECIAL_CHAR);

		if(AfxGetApp()->WriteProfileString(PROFILE_SECTION_NAME, name_idx, data->GetName()) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileString(PROFILE_SECTION_NAME, sql_idx, sql) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileInt(PROFILE_SECTION_NAME, cmd_idx, data->GetCommand()) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileInt(PROFILE_SECTION_NAME, show_dlg_idx, data->IsShowDlg()) == FALSE) return FALSE;
		if(AfxGetApp()->WriteProfileInt(PROFILE_SECTION_NAME, paste_to_editor_idx, data->IsPasteToEditor()) == FALSE) return FALSE;
	}

	return TRUE;
}

BOOL CShortCutSqlList::Add(CString name, CString sql, WORD cmd, BOOL show_dlg, BOOL paste_to_editor)
{
	CShortCutSql *data = new CShortCutSql(name, sql, cmd, show_dlg, paste_to_editor);
	if(data == NULL) return FALSE;

	m_list.Add(data);

	return TRUE;
}

CShortCutSql *CShortCutSqlList::GetShortCutSql(int row)
{
	if(row < 0 || row > m_list.GetUpperBound()) return &m_null_data;

	return (CShortCutSql *)m_list.GetAt(row);
}
