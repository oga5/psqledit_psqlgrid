/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
// psqlgrid.h : PSQLGRID アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_PSQLGRID_H__48EEDD56_E420_41C4_A7AB_0AE6EB44287E__INCLUDED_)
#define AFX_PSQLGRID_H__48EEDD56_E420_41C4_A7AB_0AE6EB44287E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "global.h"

#include "resource.h"       // メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridApp:
// このクラスの動作の定義に関しては psqlgrid.cpp ファイルを参照してください。
//

class CPsqlgridApp : public CWinApp
{
public:
	CPsqlgridApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COgridApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CPsqlgridApp)
	afx_msg void OnAppAbout();
	afx_msg void OnSetFont();
	afx_msg void OnOption();
	afx_msg void OnSetLoginInfo();
	afx_msg void OnReconnect();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int Login();
	void Logout();
	int PostLogin();
	int ReConnect();

	void WaitPostLoginThr();

	void LoadOption();
	void SaveOption();
	void LoadFontOption();
	void SaveFontOption();
	void LoadBootOption();
	void SaveBootOption();
	void UpdateAllDocViews(CView* pSender, LPARAM lHint, CObject* pHint);
	BOOL CreateFont();

	uintptr_t m_h_thread;
	BOOL m_license_installed;

	BOOL CheckRegisterShellFileTypes();
	void UnRegisterShellFileTypes();

	void SetSessionInfo(TCHAR *session_info);
public:
	afx_msg void OnShowCredits();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PSQLGRID_H__48EEDD56_E420_41C4_A7AB_0AE6EB44287E__INCLUDED_)
