/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SetupPage.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CSetupPage ダイアログ

class CSetupPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSetupPage)

// コンストラクション
public:
	CSetupPage();
	~CSetupPage();

	CString m_init_object_list_type_str;

// ダイアログ データ
	//{{AFX_DATA(CSetupPage)
	enum { IDD = IDD_OPT_SETUP };
	CComboBox	m_combo_init_object_list_type;
	CString	m_initial_dir;
	BOOL	m_register_shell;
	BOOL	m_save_modified;
	int		m_max_msg_row;
	BOOL	m_tab_title_is_path_name;
	UINT	m_max_mru;
	BOOL	m_search_loop_msg;
	CString	m_sql_lib_dir;
	CString	m_sql_stmt_str;
	int		m_init_table_list_user;
	int		m_init_object_list_type;
	BOOL	m_show_tab_tooltip;
	BOOL	m_tab_close_at_mclick;
	BOOL	m_tab_create_at_dblclick;
	BOOL	m_close_btn_on_tab;
	BOOL	m_tab_drag_move;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSetupPage)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSetupPage)
	afx_msg void OnBtnInitialDir();
	afx_msg void OnBtnSqlLibDir();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SETUPPAGE_H__2F9C2722_AFF8_11D4_B06E_00E018A83B1B__INCLUDED_)
