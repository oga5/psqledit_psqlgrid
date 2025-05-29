/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FUNCOPTIONPAGE_H__71AC5184_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_FUNCOPTIONPAGE_H__71AC5184_A085_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FuncOptionPage.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CFuncOptionPage ダイアログ

class CFuncOptionPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CFuncOptionPage)

// コンストラクション
public:
	CFuncOptionPage();
	~CFuncOptionPage();

// ダイアログ データ
	//{{AFX_DATA(CFuncOptionPage)
	enum { IDD = IDD_OPT_FUNC };
	CEdit	m_edit_sql_log_dir;
	CButton	m_btn_sql_log_dir;
	BOOL	m_download_column_name;
	BOOL	m_ignore_sql_error;
	BOOL	m_adjust_col_width_after_select;
	BOOL	m_sql_logging;
	CString	m_sql_log_dir;
	BOOL	m_receive_async_notify;
	BOOL	m_make_select_sql_use_semicolon;
	BOOL	m_auto_commit;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CFuncOptionPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CFuncOptionPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSqlLogDir();
	afx_msg void OnCheckSqlLogging();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CheckSqlLog();
	void CheckMakeSelectCheckBox();

public:
	BOOL m_sql_run_selected_area;
	BOOL m_make_select_add_alias_name;
//	CString m_make_select_alias_name;
	afx_msg void OnClickedCheckMakeSelectAddAliasName();
	CEdit m_edit_make_select_alias_name;
	CString m_make_select_alias_name;

	BOOL m_refresh_table_list_after_ddl;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FUNCOPTIONPAGE_H__71AC5184_A085_11D4_B06E_00E018A83B1B__INCLUDED_)
