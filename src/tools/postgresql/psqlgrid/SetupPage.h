/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SETUPPAGE_H__97485B61_567D_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_SETUPPAGE_H__97485B61_567D_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
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

// ダイアログ データ
	//{{AFX_DATA(CSetupPage)
	enum { IDD = IDD_OPT_SETUP };
	CComboBox	m_combo_timezone;
	CComboBox	m_combo_nls_timestamp_tz_format;
	CComboBox	m_combo_nls_timestamp_format;
	CComboBox	m_combo_nls_date_format;
	CString	m_nls_date_format;
	BOOL	m_adjust_col_width;
	BOOL	m_put_column_name;
	CString	m_initial_dir;
	BOOL	m_register_shell;
	BOOL	m_use_message_beep;
	CString	m_nls_timestamp_format;
	CString	m_nls_timestamp_tz_format;
	CString	m_timezone;
	BOOL	m_use_user_updatable_columns;
	BOOL	m_share_connect_info;
	BOOL	m_disp_all_delete_null_rows_warning;
	BOOL	m_edit_other_owner;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSetupPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSetupPage)
	afx_msg void OnBtnInitialDir();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SETUPPAGE_H__97485B61_567D_11D5_8505_00E018A83B1B__INCLUDED_)
