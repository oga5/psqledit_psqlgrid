/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

///////////////////////////////////////////////////////////////////////////// 
// WheelSplitterWnd.cpp
#include "stdafx.h"

#include "wheelsplit.h"
#include "octrl_msg.h"

#include <windowsx.h>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

/////////////////////////////////////////////////////////////////////////////
// Windows 8以降、テーマが適用されない(クラシックな)スクロールバーコントロールでは、
// 両端の矢印ボタンとそのグリフがスクロールバーの大きさと一致しない。
// 分割ウィンドウでは子ウィンドウのスクロールバーを使うため、各スクロールバーを
// サブクラス化して、矢印ボタンをスクロールバーの大きさに合わせて描き直す。

struct CSBArrowFix {
	WNDPROC	old_proc;
	int		pressed_button;	// 0: 押されていない, 1: 上または左, 2: 下または右
};

// 矢印ボタンの矩形を取得する
//   button: 1 = 上または左, 2 = 下または右
static void GetSBArrowRect(HWND hWnd, int button, RECT *rect)
{
	DWORD	style = ::GetWindowLong(hWnd, GWL_STYLE);
	RECT	rc_client;
	::GetClientRect(hWnd, &rc_client);

	if((style & SBS_VERT) != 0) {
		int		w = rc_client.right - rc_client.left;
		if(button == 1) ::SetRect(rect, 0, 0, w, w);
		else ::SetRect(rect, 0, rc_client.bottom - w, w, rc_client.bottom);
	} else {
		int		h = rc_client.bottom - rc_client.top;
		if(button == 1) ::SetRect(rect, 0, 0, h, h);
		else ::SetRect(rect, rc_client.right - h, 0, rc_client.right, h);
	}
}

static int SBArrowHitTest(HWND hWnd, POINT point)
{
	for(int i = 1; i <= 2; i++) {
		RECT	rect;
		GetSBArrowRect(hWnd, i, &rect);
		if(::PtInRect(&rect, point)) return i;
	}
	return 0;
}

static void DrawSBArrowButton(HWND hWnd, HDC hDC, const RECT *rect,
	TCHAR glyph, BOOL pressed, BOOL disabled)
{
	::FillRect(hDC, rect, ::GetSysColorBrush(COLOR_BTNFACE));

	RECT	rc = *rect;
	::DrawEdge(hDC, &rc, pressed ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT);

	// Marlettフォントのスクロールバー用グリフ:
	//   '5' = 上向き, '6' = 下向き, '3' = 左向き, '4' = 右向き
	int		size = rc.bottom - rc.top;
	HFONT	hFont = ::CreateFont(-size - 2, 0, 0, 0, FW_NORMAL, 0, 0, 0,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, FF_DONTCARE, _T("Marlett"));
	if(hFont == NULL) return;

	HFONT	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	::SetBkMode(hDC, TRANSPARENT);
	::SetTextColor(hDC, ::GetSysColor(
		(disabled || !::IsWindowEnabled(hWnd)) ? COLOR_GRAYTEXT : COLOR_BTNTEXT));
	if(pressed) ::OffsetRect(&rc, 1, 1);
	::DrawText(hDC, &glyph, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	::SelectObject(hDC, hFontOld);
	::DeleteObject(hFont);
}

static void RedrawSBArrows(HWND hWnd, CSBArrowFix *fix)
{
	// テーマが有効な場合はOS標準の描画をそのまま使う
	if(::GetWindowTheme(hWnd) != NULL) return;

	DWORD	style = ::GetWindowLong(hWnd, GWL_STYLE);
	if((style & (SBS_SIZEBOX | SBS_SIZEGRIP)) != 0) return;

	// 矢印ボタンごとの状態を取得する
	// SCROLLBARINFO.rgstate: 0 = バー全体, 1 = 上または左の矢印ボタン,
	//   2～4 = ページ領域とつまみ, 5 = 下または右の矢印ボタン
	SCROLLBARINFO	sbi;
	sbi.cbSize = sizeof(sbi);
	BOOL	have_sbi = ::GetScrollBarInfo(hWnd, OBJID_CLIENT, &sbi);

	HDC	hDC = ::GetDC(hWnd);
	if(hDC == NULL) return;

	// キャプチャ中にカーソルがボタンの外に出た場合は、押下表示を解除する
	POINT	point;
	::GetCursorPos(&point);
	::ScreenToClient(hWnd, &point);

	RECT	rc_client;
	::GetClientRect(hWnd, &rc_client);

	BOOL	vert = (style & SBS_VERT) != 0;
	TCHAR	glyphs[2] = { vert ? _T('5') : _T('3'), vert ? _T('6') : _T('4') };
	for(int i = 0; i < 2; i++) {
		RECT	rect;
		GetSBArrowRect(hWnd, i + 1, &rect);
		int		state_idx = (i == 0) ? 1 : 5;
		BOOL	disabled = have_sbi &&
			(sbi.rgstate[state_idx] & STATE_SYSTEM_UNAVAILABLE) != 0;
		BOOL	pressed = !disabled &&
			((have_sbi && (sbi.rgstate[state_idx] & STATE_SYSTEM_PRESSED) != 0) ||
			(fix != NULL && fix->pressed_button == i + 1 &&
			::GetCapture() == hWnd && ::PtInRect(&rect, point)));
		DrawSBArrowButton(hWnd, hDC, &rect, glyphs[i], pressed, disabled);
	}

	// OSが描いた矢印ボタンが実際のボタンサイズより大きい場合、はみ出した
	// 部分の描画が残るので、溝と同じ色で消去する
	// (dxyLineButtonはOSが使う矢印ボタンの幅または高さ)
	if(have_sbi) {
		HBRUSH	br = ::GetSysColorBrush(COLOR_SCROLLBAR);
		RECT	rc;
		if(vert) {
			int	w = rc_client.right - rc_client.left;
			::SetRect(&rc, 0, w, rc_client.right, sbi.dxyLineButton);
			if(rc.bottom > rc.top) ::FillRect(hDC, &rc, br);
			::SetRect(&rc, 0, rc_client.bottom - sbi.dxyLineButton,
				rc_client.right, rc_client.bottom - w);
			if(rc.bottom > rc.top) ::FillRect(hDC, &rc, br);
		} else {
			int	h = rc_client.bottom - rc_client.top;
			::SetRect(&rc, h, 0, sbi.dxyLineButton, rc_client.bottom);
			if(rc.right > rc.left) ::FillRect(hDC, &rc, br);
			::SetRect(&rc, rc_client.right - sbi.dxyLineButton, 0,
				rc_client.right - h, rc_client.bottom);
			if(rc.right > rc.left) ::FillRect(hDC, &rc, br);
		}
	}

	::ReleaseDC(hWnd, hDC);
}

static LRESULT CALLBACK SBArrowFixWndProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	CSBArrowFix	*fix = (CSBArrowFix *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if(fix == NULL) return ::DefWindowProc(hWnd, msg, wParam, lParam);

	LRESULT	result = ::CallWindowProc(fix->old_proc, hWnd, msg, wParam, lParam);

	switch(msg) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK: {
		POINT	point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		fix->pressed_button = SBArrowHitTest(hWnd, point);
		break; }
	case WM_LBUTTONUP:
	case WM_CANCELMODE:
	case WM_CAPTURECHANGED:
		fix->pressed_button = 0;
		break;
	case WM_NCDESTROY:
		delete fix;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		return result;
	}

	// 矢印ボタンを描き直す
	RedrawSBArrows(hWnd, fix);

	return result;
}

IMPLEMENT_DYNCREATE(CWheelSplitterWnd, CSplitterWnd)

BEGIN_MESSAGE_MAP(CWheelSplitterWnd, CSplitterWnd)
	//{{AFX_MSG_MAP(CWheelSplitterWnd)
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CWheelSplitterWnd::CWheelSplitterWnd()
{
	m_delete_pane_msg = 0;
	m_wheel_mode = WHEEL_MODE_VSCROLL_MSG | WHEEL_MODE_ACTIVE_VIEW;

	// splitter barの幅
	// 標準は7, 細くする場合は5 or 6にする (4以下だと表示がおかしくなったり、マウスでDragできなくなったりする)
	// FIXME: 設定可能にする
//	m_cxSplitter = m_cySplitter = 6;		// Drag中に表示する線の幅
//	m_cxBorderShare = m_cyBorderShare = 0;
//	m_cxSplitterGap = m_cySplitterGap = 6;	// barの幅
//	m_cxBorder = m_cyBorder = 2;
}

CWheelSplitterWnd::~CWheelSplitterWnd()
{
}

BOOL CWheelSplitterWnd::OnMouseWheel(UINT fFlags, short zDelta, CPoint point)
{
	CWnd		*pPane = NULL;

	if(m_wheel_mode & WHEEL_MODE_ACTIVE_VIEW) {
		pPane = GetActivePane();
	} else {
		int		c, r;
		for(r = 0; r < GetRowCount(); r++) {
			for(c = 0; c < GetColumnCount(); c++) {
				CWnd *pWnd = GetPane(r, c);
				if(pWnd == NULL) continue;

				CRect rect;
				pWnd->GetWindowRect(rect);
				if(rect.PtInRect(point)) {
					pPane = pWnd;
					break;
				}
			}
		}
	}
	if(pPane == NULL) return TRUE;

	if(m_wheel_mode & WHEEL_MODE_VSCROLL_MSG) {
		if(zDelta > 0) {
			pPane->SendMessage(WM_VSCROLL, MAKELPARAM(SB_LINEUP, 0), 0);
		} else {
			pPane->SendMessage(WM_VSCROLL, MAKELPARAM(SB_LINEDOWN, 0), 0);
		}
	} else {
		pPane->SendMessage(WSW_WM_WHEEL_TRANSFER_MSG, MAKELPARAM(fFlags, zDelta), MAKEWPARAM(point.x, point.y));
	}

	return TRUE;
}

BOOL CWheelSplitterWnd::SplitRow( int cyBefore )
{
	if(GetActivePane() == NULL) {
		SetActivePane(0, 0);
	}

	int		r, c;
	int		scr_y = GetPane(0, 0)->GetScrollPos(SB_VERT);
	int		scr_x[10] = {};
	for(c = 0; c < GetColumnCount(); c++) {
		scr_x[c] = GetPane(0, c)->GetScrollPos(SB_HORZ);
	}

	BOOL result = CSplitterWnd::SplitRow(cyBefore);

	for(r = 0; r < GetRowCount(); r++) {
		for(c = 0; c < GetColumnCount(); c++) {
			GetPane(r, c)->SetScrollPos(SB_HORZ, scr_x[c]);
			GetPane(r, c)->SetScrollPos(SB_VERT, scr_y);
			GetPane(r, c)->Invalidate();
		}
	}

	return result;

//	return CSplitterWnd::SplitRow(cyBefore);
}

BOOL CWheelSplitterWnd::SplitColumn( int cxBefore )
{
	if(GetActivePane() == NULL) {
		SetActivePane(0, 0);
	}

	int		r, c;
	int		scr_x = GetPane(0, 0)->GetScrollPos(SB_HORZ);
	int		scr_y[10] = {};
	for(r = 0; r < GetRowCount(); r++) {
		scr_y[r] = GetPane(r, 0)->GetScrollPos(SB_VERT);
	}

	BOOL result = CSplitterWnd::SplitColumn(cxBefore);

	for(r = 0; r < GetRowCount(); r++) {
		for(c = 0; c < GetColumnCount(); c++) {
			GetPane(r, c)->SetScrollPos(SB_VERT, scr_y[r]);
			GetPane(r, c)->SetScrollPos(SB_HORZ, scr_x);
			GetPane(r, c)->Invalidate();
		}
	}

	return result;

//	return CSplitterWnd::SplitColumn(cxBefore);
}

void CWheelSplitterWnd::DeleteRow( int rowDelete )
{
	CSplitterWnd::DeleteRow(rowDelete);

	if(m_delete_pane_msg != 0 && GetPane(0, 0) != NULL && 
			GetPane(0, 0)->IsKindOf(RUNTIME_CLASS(CView))) {
		CView *pview = (CView *)GetPane(0, 0);
		pview->GetDocument()->UpdateAllViews(NULL, m_delete_pane_msg, 0);
	}
}

void CWheelSplitterWnd::DeleteColumn( int colDelete )
{
	CSplitterWnd::DeleteColumn(colDelete);

	if(m_delete_pane_msg != 0 && GetPane(0, 0) != NULL && 
			GetPane(0, 0)->IsKindOf(RUNTIME_CLASS(CView))) {
		CView *pview = (CView *)GetPane(0, 0);
		pview->GetDocument()->UpdateAllViews(NULL, m_delete_pane_msg, 0);
	}
}

void CWheelSplitterWnd::SetMaxCols(int col)
{
	for(; GetColumnCount() > col; ) DeleteColumn(1);
	m_nMaxCols = col;
	RecalcLayout();
}

void CWheelSplitterWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// CSplitterWndの中にCSplitterWndがある場合、クリックしただけで境界位置がずれる問題を回避
	if(m_bTracking && GetPane(0, 0)->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) {
		m_rectTracker.top -= 2;
		CRect rect = m_rectTracker;
		CSplitterWnd::OnLButtonUp(nFlags, point);
		InvalidateRect(rect);
		return;
	}
	
	CSplitterWnd::OnLButtonUp(nFlags, point);
}

#define _AfxGetDlgCtrlID(hWnd)          ((UINT)(WORD)::GetDlgCtrlID(hWnd))

void CWheelSplitterWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == NULL) return;
	int col = _AfxGetDlgCtrlID(pScrollBar->m_hWnd) - AFX_IDW_HSCROLL_FIRST;
	if(col < 0 && col >= m_nMaxCols) return;

	CSplitterWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CWheelSplitterWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == NULL) return;
	int row = _AfxGetDlgCtrlID(pScrollBar->m_hWnd) - AFX_IDW_VSCROLL_FIRST;
	if(row < 0 && row >= m_nMaxRows) return;

	CSplitterWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CWheelSplitterWnd::CreateScrollBarCtrl(DWORD dwStyle, UINT nID)
{
	if(!CSplitterWnd::CreateScrollBarCtrl(dwStyle, nID)) return FALSE;

	// クラシックなスクロールバーコントロールの矢印ボタンを描き直すため
	// サブクラスを設定する(サイズボックスは対象外)
	if((dwStyle & (SBS_SIZEBOX | SBS_SIZEGRIP)) == 0) {
		HWND	hScrollBar = ::GetDlgItem(m_hWnd, nID);
		if(hScrollBar != NULL) {
			CSBArrowFix	*fix = new CSBArrowFix;
			fix->pressed_button = 0;
			fix->old_proc = (WNDPROC)::SetWindowLongPtr(hScrollBar,
				GWLP_WNDPROC, (LONG_PTR)SBArrowFixWndProc);
			::SetWindowLongPtr(hScrollBar, GWLP_USERDATA, (LONG_PTR)fix);
		}
	}

	return TRUE;
}
