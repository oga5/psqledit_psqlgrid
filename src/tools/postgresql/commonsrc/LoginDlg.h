/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_LOGINDLG_H__7888C097_5C7D_40F7_96F6_5700B1B364A6__INCLUDED_)
#define AFX_LOGINDLG_H__7888C097_5C7D_40F7_96F6_5700B1B364A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoginDlg.h : ヘッダー ファイル
//

#include "pglib.h"
#include "resource.h"
#include "FilterEditCtrl.h"
#include "ConnectList.h"

CString decode_passwd_v2(const TCHAR *passwd);
CString encode_passwd_v2(const TCHAR *passwd);
void LoadConnectInfoToList(CListCtrl &list_ctrl,
	const TCHAR *opt_prof_name, const TCHAR *opt_registry_key);

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg ダイアログ

class CLoginDlg : public CDialog
{
// コンストラクション
public:
	CLoginDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void SetOptProfName(const TCHAR *opt_prof_name, const TCHAR *registry_key) {
		m_opt_prof_name = opt_prof_name;
		m_opt_registry_key = registry_key;
	}

	CString		m_title;
	HPgSession	m_ss;
	CString		m_connect_str;
	CString		m_connect_name;

// ダイアログ データ
	//{{AFX_DATA(CLoginDlg)
	enum { IDD = IDD_LOGIN };
	CButton	m_cancel;
	CButton	m_btn_set_login_info;
	CButton	m_ok;
	CListCtrl	m_list;
	CString	m_host;
	CString	m_user;
	CString	m_port;
	CString	m_passwd;
	CString	m_dbname;
	CString	m_option;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CLoginDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CLoginDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditUser();
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteitemList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetLoginInfo();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	DECLARE_MESSAGE_MAP()

private:
	void CheckOkButton();
	void LoadConnectInfo(BOOL reload_list);

	void RelayoutControl();

	int GetConnectInfoIdx();

	void OnKeywordChanged();

	CString			m_opt_prof_name;
	CString			m_opt_registry_key;
	CConnectList	m_connect_list;

	CStatic		m_static_keyword;
	CFilterEditCtrl	m_edit_keyword;
	HWND		m_keyword_edit_hwnd;

public:
	CEdit m_edit_host;
	CEdit m_edit_option;
	afx_msg void OnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnObjectbarFilterAdd();
	afx_msg void OnObjectbarFilterClear();
	afx_msg void OnObjectbarFilterDel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_LOGINDLG_H__7888C097_5C7D_40F7_96F6_5700B1B364A6__INCLUDED_)
