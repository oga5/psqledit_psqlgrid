/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_CONNECTINFODLG_H__C641A7A0_689E_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_CONNECTINFODLG_H__C641A7A0_689E_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ConnectInfoDlg.h : ヘッダー ファイル
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CConnectInfoDlg ダイアログ

class CConnectInfoDlg : public CDialog
{
// コンストラクション
public:
	CConnectInfoDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void SetOptProfName(const TCHAR *opt_prof_name, const TCHAR *registry_key) {
		m_opt_prof_name = opt_prof_name;
		m_opt_registry_key = registry_key;
	}

// ダイアログ データ
	//{{AFX_DATA(CConnectInfoDlg)
	enum { IDD = IDD_CONNECT_INFO_DLG };
	CButton	m_btn_up;
	CButton	m_btn_down;
	CButton	m_btn_del;
	CButton	m_btn_add;
	CListCtrl	m_list;
	CButton	m_ok;
	CString	m_edit_user;
	CString	m_edit_passwd;
	CString	m_dbname;
	CString	m_host;
	CString	m_port;
	CString	m_option;
	CString	m_connect_name;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CConnectInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CConnectInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDel();
	afx_msg void OnBtnDown();
	afx_msg void OnBtnUp();
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditUser();
	virtual void OnOK();
	afx_msg void OnDeleteitemList1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SwapItem(int item1, int item2);
	void CheckSelect();
	void CheckAddButton();
	void SaveConnectInfo();
	void LoadConnectInfo();

	void AddData(CString user, CString passwd, CString dbname, 
		CString host, CString port, CString option, CString connect_name);
	void EditData(int row, CString user, CString passwd, CString dbname, 
		CString host, CString port, CString option, CString connect_name);

	CString			m_opt_prof_name;
	CString			m_opt_registry_key;
public:
	afx_msg void OnClickedBtnCopy();
	CButton m_btn_copy;
	afx_msg void OnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_CONNECTINFODLG_H__C641A7A0_689E_11D4_B06E_00E018A83B1B__INCLUDED_)
