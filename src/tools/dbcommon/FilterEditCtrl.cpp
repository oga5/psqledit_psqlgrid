/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // FilterEditCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
//#include "sqltune.h"
#include "FilterEditCtrl.h"
#include "ostrutil.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// 編集用エディットコントロールの，サブクラス後のウィンドウプロシージャ
static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_RBUTTONDOWN:
		::SetFocus(hwnd);
		break;

	case WM_CONTEXTMENU:
		{
			CPoint	pt((short)LOWORD(lParam), (short)HIWORD(lParam));

			TRACE(_T("WM_CONTEXTMENU %d %d:%d\n"), wParam, pt.x, pt.y);

			// メニュー作成
			CMenu	menu;
			menu.LoadMenu(IDR_OBJECTBAR_FILTER_MENU);

			// CMainFrameを親ウィンドウにすると、メニューの有効無効を、メインメニューと同じにできる
			CWnd	wnd;
			wnd.Attach(hwnd);

			if(wnd.GetParentFrame() != NULL) {
				menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y,
					wnd.GetParentFrame());
			} else {
				CWnd *mainWnd = AfxGetMainWnd();
				menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, mainWnd);
			}
			wnd.Detach();

			// メニュー削除
			menu.DestroyMenu();
		}
		return 0;
	}

	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////
// CFilterEditCtrl

CFilterEditCtrl::CFilterEditCtrl() :
	m_keyword_is_ok(TRUE), m_keyword_list_is_modify(FALSE), m_child_wnd(NULL)
{
}

CFilterEditCtrl::~CFilterEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CFilterEditCtrl, CComboBox)
	//{{AFX_MSG_MAP(CFilterEditCtrl)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterEditCtrl メッセージ ハンドラ

#define MAX_FILTER_CNT	20
#define FILTER_BUF_SIZE	256

BOOL CFilterEditCtrl::Create(CString keyword_file_name, CRect rect, CWnd *parent, UINT nID)
{
	m_keyword_file_name = keyword_file_name;

	if(!CComboBox::Create(
		CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
		rect, parent, nID)) {
		return FALSE;
	}

	m_child_wnd = ChildWindowFromPoint(CPoint(10, 10));

	// サブクラス化
	if(m_child_wnd != NULL) {
		HWND hwnd = m_child_wnd->GetSafeHwnd();
		// 古いウィンドウプロシージャを保存する
		::SetWindowLongPtr (hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
		// ウィンドウプロシージャを切り替える
		::SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);
	}

	LoadFilterText();

	return TRUE;
}

void CFilterEditCtrl::SetKeywordOK(BOOL b_ok)
{
	if(m_keyword_is_ok == b_ok) return;

	m_keyword_is_ok = b_ok;
	RedrawWindow();
}

void CFilterEditCtrl::CutKeyword()
{
	DWORD selected = GetEditSel();
	if(LOWORD(selected) == HIWORD(selected)) SelectAllKeyword();

	Cut();
}

void CFilterEditCtrl::LoadFilterText()
{
	// コンボボックスをクリア
	for(; GetCount() != 0;) {
		DeleteString(0);
	}

	FILE	*fp;
	TCHAR	buf[1024];
	int		idx = 0;

	fp = _tfopen(m_keyword_file_name, _T("rb"));
	if(fp == NULL) return;

	for(;;) {
		if(_fgetts(buf, ARRAY_SIZEOF(buf), fp) == NULL) break;
		ostr_chomp(buf, ' ');
		if(buf[0] == '\0' || _tcslen(buf) >= FILTER_BUF_SIZE) continue;

		InsertString(idx, buf);
		idx++;
	}

	fclose(fp);
}

void CFilterEditCtrl::SaveFilterText()
{
	if(!m_keyword_list_is_modify) return;

	FILE	*fp;
	TCHAR	buf[FILTER_BUF_SIZE];
	int		len;
	int		i;

	fp = _tfopen(m_keyword_file_name, _T("wb"));
	if(fp == NULL) return;

	for(i = 0; i < GetCount() && i < MAX_FILTER_CNT; i++) {
		// テキストを取得
		len = GetLBTextLen(i);
		if(len == CB_ERR) break;
		if(len >= sizeof(buf)) continue;
		len = GetLBText(i, buf);

		_ftprintf(fp, _T("%s\n"), buf);
	}

	fclose(fp);
}

void CFilterEditCtrl::AddFilterText(const TCHAR *add_text)
{
	m_keyword_list_is_modify = TRUE;

	if(add_text == NULL || _tcslen(add_text) == 0) return;
	if(_tcslen(add_text) >= FILTER_BUF_SIZE) return;

	DeleteFilterText(add_text);
	InsertString(0, add_text);

	for(; GetCount() > MAX_FILTER_CNT;) {
		DeleteString(MAX_FILTER_CNT);
	}

	SetCurSel(0);
}

void CFilterEditCtrl::DeleteFilterText(const TCHAR *del_text)
{
	m_keyword_list_is_modify = TRUE;

	int			i;
	TCHAR		buf[FILTER_BUF_SIZE];
	int			len;

	if(del_text == NULL || _tcslen(del_text) == 0) return;
	if(_tcslen(del_text) >= sizeof(buf)) return;

	for(i = 0; i < GetCount() && i < MAX_FILTER_CNT; i++) {
		// テキストを取得
		len = GetLBTextLen(i);
		if(len == CB_ERR) break;
		if(len >= sizeof(buf)) continue;
		len = GetLBText(i, buf);

		if(_tcscmp(buf, del_text) == 0) {
			DeleteString(i);
			break;
		}
	}
}

CString CFilterEditCtrl::GetFilterText()
{
	if(!IsWindowVisible()) return _T("");

	CString search_text;
	GetWindowText(search_text);
	return search_text;
}

void CFilterEditCtrl::OnDestroy() 
{
	CComboBox::OnDestroy();
	
	SaveFilterText();
}

HREG_DATA CFilterEditCtrl::GetRegexpData()
{
	CString search_text = GetFilterText();

	HREG_DATA hreg = NULL;

	if(search_text != _T("")) {
		hreg = oreg_comp(search_text, 1);
		if(hreg == NULL) {
			SetKeywordOK(FALSE);
		} else {
			SetKeywordOK(TRUE);
		}
	}

	return hreg;
}


HBRUSH CFilterEditCtrl::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);

	if(!m_keyword_is_ok) {
		pDC->SetTextColor(RGB(255, 0, 0));
	}
	
	// TODO: デフォルトのブラシが望みのものでない場合には、違うブラシを返してください
	return hbr;
}
