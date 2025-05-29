/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(_SHORTCUT_SQL_INCLUDED_)
#define _SHORTCUT_SQL_INCLUDED_

#define MAX_SHORT_CUT_SQL	10

class CShortCutSql {
private:
	CString			m_name;
	CString			m_sql;
	WORD			m_cmd;
	BOOL			m_show_dlg;
	BOOL			m_paste_to_editor;

public:
	CShortCutSql();
	CShortCutSql(CString name, CString sql, WORD cmd, BOOL show_dlg, BOOL paste_to_editor);

	CString GetName() { return m_name; }
	CString GetSql() { return m_sql; }
	WORD GetCommand() { return m_cmd; }
	BOOL IsShowDlg() { return m_show_dlg; }
	BOOL IsPasteToEditor() { return m_paste_to_editor; }
};

class CShortCutSqlList {
private:
	CPtrArray		m_list;
	CShortCutSql	m_null_data;

	BOOL ClearRegistry();
	void ClearArray();

public:
	CShortCutSqlList();
	~CShortCutSqlList();

	BOOL Load();
	BOOL Save();

	BOOL Add(CString name, CString sql, WORD cmd, BOOL show_dlg, BOOL paste_to_editor);

	INT_PTR GetSqlCnt() { return m_list.GetSize(); }
	CShortCutSql *GetShortCutSql(int row);

	void MakeRegIdx(int i, CString &name_idx, CString &sql_idx, 
		CString &cmd_idx, CString &show_dlg_idx, CString &paste_to_editor_idx);
};

#endif _SHORTCUT_SQL_INCLUDED_
