/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SHORTCUTSQLDLG_H__8CF113C7_66DD_4D38_9AA4_84D063B3BEA0__INCLUDED_)
#define AFX_SHORTCUTSQLDLG_H__8CF113C7_66DD_4D38_9AA4_84D063B3BEA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShortCutSqlDlg.h : ヘッダー ファイル
//

#include "EditCtrl.h"
#include "AccelList.h"

/////////////////////////////////////////////////////////////////////////////
// CShortCutSqlDlg ダイアログ

class CShortCutSqlDlg : public CDialog
{
// コンストラクション
public:
	CShortCutSqlDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	CString		m_sql;
	CAccelList	m_accel_list;
	WORD		m_command;

// ダイアログ データ
	//{{AFX_DATA(CShortCutSqlDlg)
	enum { IDD = IDD_SHORTCUT_SQL_DLG };
	CButton	m_check_show_dlg;
	CButton	m_ok;
	CString	m_name;
	CString	m_shortcut_key_name;
	BOOL	m_is_paste_to_editor;
	BOOL	m_is_show_dlg;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CShortCutSqlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CShortCutSqlDlg)
	virtual void OnOK();
	afx_msg void OnOk2();
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnShortcutKey();
	afx_msg void OnChangeEditName();
	afx_msg void OnCheckPasteToEditor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CEditData	m_edit_data;
	CEditCtrl	m_edit_ctrl;

	BOOL CreateEditCtrl();
	void SetShortCutKeyName();

	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SHORTCUTSQLDLG_H__8CF113C7_66DD_4D38_9AA4_84D063B3BEA0__INCLUDED_)
