/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // psqledit.h : PSQLEDIT アプリケーションのメイン ヘッダー ファイル
//

#if !defined(AFX_PSQLEDIT_H__F8E50C1E_1E85_45C5_AE6D_1241D3E2F8FC__INCLUDED_)
#define AFX_PSQLEDIT_H__F8E50C1E_1E85_45C5_AE6D_1241D3E2F8FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CPsqleditApp:
// このクラスの動作の定義に関しては psqledit.cpp ファイルを参照してください。
//

#include "global.h"

class CPsqleditApp : public CWinApp
{
public:
	CPsqleditApp();

	virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate,
			TCHAR *dir = NULL);

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CPsqleditApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション
	//{{AFX_MSG(CPsqleditApp)
	afx_msg void OnAppAbout();
	afx_msg void OnOption();
	afx_msg void OnLineModeLen();
	afx_msg void OnLineModeNormal();
	afx_msg void OnLineModeRight();
	afx_msg void OnUpdateLineModeLen(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLineModeNormal(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLineModeRight(CCmdUI* pCmdUI);
	afx_msg void OnLogfileOpen();
	afx_msg void OnSetFont();
	afx_msg void OnSetFontDlgBar();
	afx_msg void OnRefreshTableList();
	afx_msg void OnSqlLibrary();
	afx_msg void OnSetLoginInfo();
	afx_msg void OnReconnect();
	afx_msg void OnDisconnect();
	afx_msg void OnUpdateDisconnect(CCmdUI* pCmdUI);
	afx_msg void OnGetObjectSource();
	afx_msg void OnUpdateGetObjectSource(CCmdUI* pCmdUI);
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	uintptr_t m_h_thread;

	void SetRegistry();

	void LoadOption();
	void LoadFontOption();
	void LoadDlgBarFontOption();

	void SaveOption();
	void SaveFontOption();
	void SaveDlgBarFontOption();

	BOOL CreateFont();
	BOOL CreateDlgBarFont();

	int Login();
	void Logout();
	int PostLogin();
	int ReConnect();

	void PrintConnectInfo();

	void UnRegisterShellFileTypes();
	BOOL CheckRegisterShellFileTypes();

	void SetLineMode(int line_mode);
	void CheckLineMode(CCmdUI *pCmdUI, int line_mode);

	CDocument *CreateNewDocument(TCHAR *title);

	void UpdateAllDocViews(CView* pSender, LPARAM lHint = 0L, CObject* pHint = NULL);

	void SetSessionInfo(const TCHAR *session_info);

	int GetObjectSource(const TCHAR *owner, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name, const TCHAR *schema);
	int GetObjectSource(const TCHAR* owner, CStringArray* object_name_list, CStringArray* object_type_list,
		CStringArray* oid_list, CStringArray* schema_list);
	int GetObjectSource();
	int GetObjectSourceDetailBar();

	void WaitPostLoginThr();
public:
	afx_msg void OnNewWindow();
	afx_msg void OnGetObjectSourceDetailBar();
	afx_msg void OnUpdateGetObjectSourceDetailBar(CCmdUI* pCmdUI);
	afx_msg void OnShowCredits();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PSQLEDIT_H__F8E50C1E_1E85_45C5_AE6D_1241D3E2F8FC__INCLUDED_)
