/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // TabBar.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "TabBar.h"

#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TAB_CTRL_ID				301

/////////////////////////////////////////////////////////////////////////////
// CTabBar

CTabBar::CTabBar()
{
	m_font_height = 10;
	m_szMin.cy = 30;
}

CTabBar::~CTabBar()
{
}


BEGIN_MESSAGE_MAP(CTabBar, CSizingControlBar)
	//{{AFX_MSG_MAP(CTabBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTabBar メッセージ ハンドラ

int CTabBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	CRect rect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy);

// 外観をフラットボタンにする
//	m_tab_ctrl.Create(WS_VISIBLE | WS_CHILD | TCS_FOCUSNEVER | TCS_SINGLELINE | TCS_BUTTONS | TCS_FLATBUTTONS,
//		rect, this, TAB_CTRL_ID);
//	m_tab_ctrl.SetExtendedStyle(TCS_EX_FLATSEPARATORS);
	m_tab_ctrl.Create(WS_VISIBLE | WS_CHILD | TCS_FOCUSNEVER | TCS_SINGLELINE | TCS_OWNERDRAWFIXED,
		rect, this, TAB_CTRL_ID);

	AdjustChildWindows();

	return 0;
}

void CTabBar::OnSize(UINT nType, int cx, int cy) 
{
	CSizingControlBar::OnSize(nType, cx, cy);

	m_tab_ctrl.SetWindowPos(&wndBottom, 0, 0, cx, cy, SWP_NOACTIVATE);
	AdjustChildWindows();
}

void CTabBar::SetFont(CFont *font)
{
	m_font_height = CalcFontHeight(this, font);
	m_tab_ctrl.SetFont(font);
	AdjustChildWindows();
}

void CTabBar::AdjustChildWindows()
{
	RECT rect;
	GetClientRect(&rect);

	m_tab_ctrl.AdjustRect(FALSE, &rect);
}

void CTabBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(point.x <= 0) CSizingControlBar::OnLButtonDown(nFlags, point);
}

void CTabBar::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if(point.x <= 0) {
		CSizingControlBar::OnLButtonDblClk(nFlags, point);
	} else {
		if(g_option.tab_create_at_dblclick)
			SendMessage(WM_COMMAND, ID_FILE_NEW, 0);
	}
}

CString CTabBar::GetTabName(CDocument *pdoc)
{
	CString path_name = pdoc->GetPathName();
	if(path_name == _T("")) {
		path_name = pdoc->GetTitle();
	} else {
		if(pdoc->IsModified()) path_name += _T(" *");
	}

	TCHAR	*file_name = path_name.GetBuffer(0);
	TCHAR   *p;
	int		last_sepa_pos = 0;
	int		pre_last_sepa_pos = 0;

	/* '\'はSJISの2バイト目でも使われるので，文字列の前から調べていく */
	for(p = file_name; p[0] != '\0'; p++) {
		/* 2バイト文字はスキップ */
		if(is_lead_byte(*p) == 1) {
			p++;
			continue;
		}

		if(p[0] == '\\') {
			pre_last_sepa_pos = last_sepa_pos;
			last_sepa_pos = (int)(p - file_name);
		}
	}

	CString tab_name;
	TCHAR *name = tab_name.GetBuffer(_MAX_PATH);

	if(last_sepa_pos == 0) {
		_tcscpy(name, file_name);
	} else if(pre_last_sepa_pos == 0 || g_option.tab_title_is_path_name == FALSE) {
		_stprintf(name, _T("%s"), file_name + last_sepa_pos + 1);
	} else {
		_stprintf(name, _T("%s"), file_name + pre_last_sepa_pos + 1);
	}

	tab_name.ReleaseBuffer();
	return tab_name;
}

void CTabBar::InsertDoc(CDocument *pdoc)
{
	CString path_name = GetTabName(pdoc);

	int tab_cnt = m_tab_ctrl.GetItemCount();
	int tab_idx = SearchDoc(pdoc);
	if(tab_idx < 0) {
		tab_idx = m_tab_ctrl.InsertItem(tab_cnt, path_name, (LPARAM)pdoc);
	}

	//ActiveDoc(tab_idx);
	m_tab_ctrl.SetCurSel(tab_idx);

	// タブが無い状態からタブを追加したとき，表示がおかしくなるのを回避する
	if(m_tab_ctrl.GetItemCount() == 1) {
		m_tab_ctrl.RedrawWindow();
	}
}

void CTabBar::DeleteDoc(CDocument *pdoc)
{
	int		tab_idx = SearchDoc(pdoc);
	if(tab_idx < 0) return;

	// タブを削除する前に，選択中のドキュメントを変更する
	// (選択中のタブを削除すると，タブの表示がおかしくなる)
	if(m_tab_ctrl.GetItemCount() > 1) {
		int idx = tab_idx - 1;
		if(idx < 0) idx = 0;
		ActiveDoc(idx);
	}
	m_tab_ctrl.DeleteItem(tab_idx);
}

int CTabBar::SearchDoc(CDocument *pdoc)
{
	if(pdoc == NULL) return -1;

	int		tab_idx;
	for(tab_idx = 0; tab_idx < m_tab_ctrl.GetItemCount(); tab_idx++) {
		if(GetDoc(tab_idx) == pdoc) return tab_idx;
	}

	return -1;
}

void CTabBar::SetTabName(CDocument *pdoc)
{
	SetTabName(SearchDoc(pdoc));
}

void CTabBar::SetTabName(int tab_idx)
{
	if(tab_idx < 0) return;

	CDocument *pdoc = GetDoc(tab_idx);
	if(pdoc == NULL) return;

	TC_ITEM	item;

	item.mask = TCIF_PARAM;
	m_tab_ctrl.GetItem(tab_idx, &item);

	CString path_name = GetTabName(pdoc);
	m_tab_ctrl.SetItemText(tab_idx, path_name);
}

BOOL CTabBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch(wParam) {
	case TAB_CTRL_ID:
		{
			NMHDR *p_nmhdr = (NMHDR *)lParam;
			switch(p_nmhdr->code) {
			case TCN_SELCHANGE:
				OnTabSelChanged();
				break;
			case NM_RCLICK:
				OnTabRClicked();
				break;
			case TC_MCLICK:
				OnTabMClicked();
				break;
			case TC_MOVE_TAB:
				{
					int cur_pos = m_tab_ctrl.GetCurSel();
					int new_pos = (int)(p_nmhdr->idFrom);
					MoveTab(cur_pos, new_pos);
				}
				break;
			case TC_CLOSE_BTN:
				CloseActiveDocument();
				break;
			case TC_TOOL_TIP_NAME:
				OnTabToolTipName(p_nmhdr);
				break;
			}
		}
		break;
	}
	
	return CSizingControlBar::OnNotify(wParam, lParam, pResult);
}

void CTabBar::OnTabSelChanged()
{
	ActiveDoc(m_tab_ctrl.GetCurSel());
}

CDocument *CTabBar::GetDoc(int tab_idx)
{
	if(tab_idx < 0 || tab_idx >= m_tab_ctrl.GetItemCount()) return NULL;

	TC_ITEM	item;
	item.mask = TCIF_PARAM;
	m_tab_ctrl.GetItem(tab_idx, &item);
	CDocument *pdoc = (CDocument *)item.lParam;

	if(pdoc == NULL) return NULL;
	if(!pdoc->IsKindOf(RUNTIME_CLASS(CDocument))) return NULL;

	return pdoc;
}

int CTabBar::TabHitTest(CPoint point)
{
	m_tab_ctrl.ScreenToClient(&point);

	TCHITTESTINFO tc_info;
	tc_info.pt = point;
	tc_info.flags = TCHT_ONITEM;

	return m_tab_ctrl.HitTest(&tc_info);
}

void CTabBar::OnTabRClicked()
{
	POINT pt;
	GetCursorPos(&pt);
	int tab_idx = TabHitTest(pt);
	if(tab_idx == -1) return;

	ActiveDoc(tab_idx);

	CMenu	menu;
	menu.LoadMenu(IDR_TAB_MENU);

	// CMainFrameを親ウィンドウにすると、メニューの有効無効を、メインメニューと同じにできる
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, GetParentFrame());

	// メニュー削除
	menu.DestroyMenu();
}

void CTabBar::TabMoveLeft() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == 0) return;

	MoveTab(tab_idx, tab_idx - 1);
}

void CTabBar::TabMoveRight() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == m_tab_ctrl.GetItemCount() - 1) return;

	MoveTab(tab_idx, tab_idx + 1);
}

void CTabBar::MoveTab(int old_pos, int new_pos)
{
	TCHAR tab_name[_MAX_PATH];
	TC_ITEM	item;
	item.mask = TCIF_PARAM | TCIF_TEXT;
	item.pszText = tab_name;
	item.cchTextMax = sizeof(tab_name)/sizeof(tab_name[0]);
	m_tab_ctrl.GetItem(old_pos, &item);

	m_tab_ctrl.DeleteItem(old_pos);
	m_tab_ctrl.InsertItem(new_pos, &item);

	ActiveDoc(new_pos);
}

void CTabBar::SortTab()
{
	// 選択ソート
	int		i, j, min;
	CString	str_j, str_min;

	for(i = 0; i < m_tab_ctrl.GetItemCount() - 1; i++) {
		min = i;
		for(j = i + 1; j < m_tab_ctrl.GetItemCount(); j++) {
			if(GetTabName(GetDoc(j)) < GetTabName(GetDoc(min))) {
				min = j;
			}
		}
		if(i != min) MoveTab(min, i);
	}

	ActiveDoc(0);
}

BOOL CTabBar::CanSortTab()
{
	if(m_tab_ctrl.GetItemCount() <= 1) {
		return FALSE;
	}

	return TRUE;
}

BOOL CTabBar::CanTabMoveLeft()
{
	if(m_tab_ctrl.GetItemCount() == 1 || m_tab_ctrl.GetCurSel() == 0) {
		return FALSE;
	}

	return TRUE;
}

BOOL CTabBar::CanTabMoveRight()
{
	if(m_tab_ctrl.GetItemCount() == 1 || m_tab_ctrl.GetCurSel() == m_tab_ctrl.GetItemCount() - 1) {
		return FALSE;
	}

	return TRUE;
}

void CTabBar::TabMoveTop() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == 0) return;

	MoveTab(tab_idx, 0);
}

void CTabBar::TabMoveLast() 
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1 || tab_idx == m_tab_ctrl.GetItemCount() - 1) return;

	MoveTab(tab_idx, m_tab_ctrl.GetItemCount());
}

void CTabBar::SetTabNameAll()
{
	int		tab_idx;
	for(tab_idx = 0; tab_idx < m_tab_ctrl.GetItemCount(); tab_idx++) {
		SetTabName(tab_idx);		
	}
}

void CTabBar::ActiveTab(CDocument *pdoc)
{
	int		tab_idx = SearchDoc(pdoc);
	if(tab_idx < 0) return;

	m_tab_ctrl.SetCurSel(tab_idx);
}

void CTabBar::ActiveDoc(int tab_idx)
{
	CDocument *pdoc = GetDoc(tab_idx);
	if(pdoc == NULL) return;

	POSITION pos = pdoc->GetFirstViewPosition();
	if(pos == NULL) return;
	CView *pview = pdoc->GetNextView(pos);
	if(pview == NULL) return;
	pview->GetParentFrame()->ActivateFrame();
	//((CMDIFrameWnd *)GetParentFrame())->MDIActivate(pview->GetParentFrame());
}

void CTabBar::NextDoc()
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1) return;

	tab_idx++;
	if(tab_idx >= m_tab_ctrl.GetItemCount()) tab_idx = 0;

	ActiveDoc(tab_idx);
}

void CTabBar::PrevDoc()
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1) return;

	tab_idx--;
	if(tab_idx < 0) tab_idx = m_tab_ctrl.GetItemCount() - 1;

	ActiveDoc(tab_idx);
}

BOOL CTabBar::CanNextPrevDoc()
{
	if(m_tab_ctrl.GetItemCount() <= 1) return FALSE;
	return TRUE;
}

void CTabBar::SetOption()
{
	DWORD style = 0;

	if(g_option.tab_drag_move) style |= FTC_TAB_DRAG_MOVE;
	if(g_option.close_btn_on_tab) style |= FTC_CLOSE_BTN_ON_ALL_TAB;
	if(g_option.show_tab_tooltip) style |= FTC_SHOW_TAB_TOOLTIP;

	m_tab_ctrl.SetExStyle(style);
}

void CTabBar::OnTabMClicked()
{
	if(!g_option.tab_close_at_mclick) return;

	CloseActiveDocument();
}

void CTabBar::OnTabToolTipName(NMHDR *p_nmhdr)
{
	struct file_tab_ctrl_nmhdr *h = (struct file_tab_ctrl_nmhdr *)p_nmhdr;

	CDocument *pdoc = GetDoc((int)(h->nmhdr.idFrom));
	CString path_name = pdoc->GetPathName();
	_stprintf(h->str_buf, _T("%.*s"), (int)((sizeof(h->str_buf) - 1)/sizeof(h->str_buf[0])), path_name.GetBuffer(0));
}

void CTabBar::CloseActiveDocument()
{
	POINT pt;
	GetCursorPos(&pt);
	int tab_idx = TabHitTest(pt);
	if(tab_idx == -1) return;

	int	cur_idx = m_tab_ctrl.GetCurSel();

	ActiveDoc(tab_idx);
	SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0);

	if(tab_idx < cur_idx) cur_idx--;
	if(cur_idx >= 0 && cur_idx < m_tab_ctrl.GetItemCount()) ActiveDoc(cur_idx);
}

CString CTabBar::GetCurrentTabName()
{
	int tab_idx = m_tab_ctrl.GetCurSel();
	if(tab_idx == -1) return "";

	CDocument* pdoc = GetDoc(tab_idx);
	CString result = GetTabName(pdoc);

	if(pdoc->IsModified() && result.Find(_T(" *")) >= 0) {
		result = result.Left(result.GetLength() - 2);
	}
	return result;
}
