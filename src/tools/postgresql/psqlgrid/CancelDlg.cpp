/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// CancelDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "CancelDlg.h"

#include "pglib.h"
#include "pgmsg.h"
#define OCI_DB_TRANSFER_MSG		_T("検索結果転送中(%d)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCancelDlg ダイアログ


CCancelDlg::CCancelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCancelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCancelDlg)
	m_static0 = _T("");
	m_static1 = _T("");
	//}}AFX_DATA_INIT

	m_timer = 0;
	m_timer_flg = TRUE;
}


void CCancelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCancelDlg)
	DDX_Control(pDX, IDCANCEL, m_cancel);
	DDX_Text(pDX, IDC_STATIC0, m_static0);
	DDX_Text(pDX, IDC_STATIC1, m_static1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCancelDlg, CDialog)
	//{{AFX_MSG_MAP(CCancelDlg)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCancelDlg メッセージ ハンドラ

void CCancelDlg::OnCancel() 
{
	// キャンセル問い合わせ中
	m_cancel_flg = 2;
	
	if(MessageBox(_T("キャンセルしますか？"), _T("確認"),
		 MB_ICONQUESTION | MB_YESNO) == IDNO) {
		// キャンセルしない
		m_cancel_flg = 0;
		return;
	}

	// キャンセルする
	m_cancel_flg = 1;
	// キャンセルボタンを2重に押せなくする
	m_cancel.EnableWindow(FALSE);
}

BOOL CCancelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	m_cancel_flg = 0;
	*m_p_hWnd = GetSafeHwnd();
	ResumeThread((HANDLE)m_hThread);

	return TRUE;
}

LRESULT CCancelDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	switch(message) {
	case WM_OCI_DLG_ROW_CNT:
		if(m_timer != 0) break;
		m_static0.Format(OCI_DB_TRANSFER_MSG, lParam);
		UpdateData(FALSE);
		break;
	case WM_OCI_DLG_STATIC:
		if(wParam == 0) {
			m_static0 = (char *)lParam;
		} else if(wParam == 1) {
			m_static1 = (char *)lParam;
		}
		UpdateData(FALSE);
		break;
	case WM_OCI_DLG_EXIT:
		m_exit_code = (int)wParam;
		if(m_cancel_flg == 2) {
			CDialog::OnCancel();
		} else {
			CDialog::OnOK();
		}
		break;
	case WM_OCI_DLG_ENABLE_CANCEL:
		m_cancel.EnableWindow((BOOL)wParam);
		break;
	case WM_UPDATE_ALL_VIEWS:
		m_pdoc->UpdateAllViews(NULL, wParam, (CObject *)lParam);
		break;
	default:
		break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

int CCancelDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_create_struct = *lpCreateStruct;

	if(m_timer_flg) {
		m_timer = SetTimer(1, 500, NULL);
		if(m_timer != 0) {
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}

	return 0;
}

void CCancelDlg::OnTimer(UINT_PTR nIDEvent)
{
	CRect	rect;
	AfxGetMainWnd()->GetWindowRect(rect);

	m_create_struct.x = rect.left + rect.Width() / 2 - m_create_struct.cx / 2;
	m_create_struct.y = rect.top + rect.Height() / 2 - m_create_struct.cy / 2;

	if(m_timer != 0) {
		KillTimer(m_timer);
		m_timer = 0;
	}

	SetWindowPos(NULL, m_create_struct.x, m_create_struct.y, 
		m_create_struct.cx, m_create_struct.cy,
		SWP_NOZORDER | SWP_NOACTIVATE);

	CDialog::OnTimer(nIDEvent);
}

void CCancelDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if(m_timer != 0) KillTimer(m_timer);
}

