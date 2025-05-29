/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_CANCELDLG_H__1AB3D6A2_4928_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_CANCELDLG_H__1AB3D6A2_4928_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CancelDlg.h : ヘッダー ファイル
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CCancelDlg ダイアログ
#define	WM_UPDATE_ALL_VIEWS	WM_USER + 200

class CCancelDlg : public CDialog
{
private:
	CREATESTRUCT	m_create_struct;
	UINT_PTR		m_timer;

// コンストラクション
public:
	CCancelDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ
	HWND			*m_p_hWnd;
	uintptr_t		m_hThread;
	volatile int	m_cancel_flg;
	int				m_exit_code;
	CDocument		*m_pdoc;
	BOOL			m_timer_flg;

// ダイアログ データ
	//{{AFX_DATA(CCancelDlg)
	enum { IDD = IDD_CANCEL_DLG };
	CButton	m_cancel;
	CString	m_static0;
	CString	m_static1;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CCancelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CCancelDlg)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR  nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_CANCELDLG_H__1AB3D6A2_4928_11D4_B06E_00E018A83B1B__INCLUDED_)
