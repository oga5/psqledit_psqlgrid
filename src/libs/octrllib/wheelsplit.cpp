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
	DWORD	last_sig;		// 前回描画時の状態(再描画が必要かの判定用)
	BOOL	tracking;		// 矢印ボタンの押下を自前で追跡中
	BOOL	repeat;			// 自動リピートスクロール中
};

#define SBARROW_TIMER_ID		0x5342
#define SBARROW_FIRST_DELAY		400	// 自動リピート開始までの時間(ms)
#define SBARROW_REPEAT_DELAY	50		// 自動リピートの間隔(ms)

// 親ウィンドウにスクロールメッセージを送る
static void SBSendScrollMsg(HWND hWnd, int button)
{
	DWORD	style = ::GetWindowLong(hWnd, GWL_STYLE);
	UINT	msg;
	int		code;
	if((style & SBS_VERT) != 0) {
		msg = WM_VSCROLL;
		code = (button == 1) ? SB_LINEUP : SB_LINEDOWN;
	} else {
		msg = WM_HSCROLL;
		code = (button == 1) ? SB_LINELEFT : SB_LINERIGHT;
	}
	HWND	parent = ::GetParent(hWnd);
	if(parent != NULL) {
		::SendMessage(parent, msg, MAKELONG(code, 0), (LPARAM)hWnd);
	}
}

static void SBSendEndScroll(HWND hWnd)
{
	DWORD	style = ::GetWindowLong(hWnd, GWL_STYLE);
	HWND	parent = ::GetParent(hWnd);
	if(parent != NULL) {
		::SendMessage(parent,
			((style & SBS_VERT) != 0) ? WM_VSCROLL : WM_HSCROLL,
			MAKELONG(SB_ENDSCROLL, 0), (LPARAM)hWnd);
	}
}

// 矢印ボタン押下の追跡を終了する
static void SBArrowEndTrack(HWND hWnd, CSBArrowFix *fix)
{
	if(fix == NULL || !fix->tracking) return;
	fix->tracking = FALSE;
	fix->repeat = FALSE;
	::KillTimer(hWnd, SBARROW_TIMER_ID);
	SBSendEndScroll(hWnd);
	if(::GetCapture() == hWnd) ::ReleaseCapture();
}

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
	if(disabled || !::IsWindowEnabled(hWnd)) {
		// クラシックな無効ボタンと同じエンボス表示にする
		RECT	rc_hi = rc;
		::OffsetRect(&rc_hi, 1, 1);
		::SetTextColor(hDC, ::GetSysColor(COLOR_3DHILIGHT));
		::DrawText(hDC, &glyph, 1, &rc_hi,
			DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		::SetTextColor(hDC, ::GetSysColor(COLOR_GRAYTEXT));
	} else {
		::SetTextColor(hDC, ::GetSysColor(COLOR_BTNTEXT));
	}
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

	// キャプチャ中にカーソルがボタンの外に出た場合は、押下表示を解除する
	POINT	point;
	::GetCursorPos(&point);
	::ScreenToClient(hWnd, &point);

	RECT	rc_client;
	::GetClientRect(hWnd, &rc_client);

	// ボタンが押されているのはこのウィンドウがキャプチャ中で、
	// マウスの左ボタンが押されているときだけ。
	// WM_LBUTTONUP直後はrgstateのPRESSEDやキャプチャが残ることが
	// あるので、物理ボタンが押されているかでも絞る
	BOOL	captured = (::GetCapture() == hWnd);
	BOOL	lbtn_down = (::GetKeyState(VK_LBUTTON) < 0);
	BOOL	disabled[2], pressed[2];
	for(int i = 0; i < 2; i++) {
		RECT	rect;
		GetSBArrowRect(hWnd, i + 1, &rect);
		int		state_idx = (i == 0) ? 1 : 5;
		disabled[i] = have_sbi &&
			(sbi.rgstate[state_idx] & STATE_SYSTEM_UNAVAILABLE) != 0;
		pressed[i] = !disabled[i] && captured && lbtn_down &&
			((have_sbi && (sbi.rgstate[state_idx] & STATE_SYSTEM_PRESSED) != 0) ||
			(fix != NULL && fix->pressed_button == i + 1 &&
			::PtInRect(&rect, point)));
	}

	HDC	hDC = ::GetDC(hWnd);
	if(hDC == NULL) return;

	BOOL	vert = (style & SBS_VERT) != 0;
	TCHAR	glyphs[2] = { vert ? _T('5') : _T('3'), vert ? _T('6') : _T('4') };

	// ちらつきを抑えるため、ボタンはオフスクリーンで描いてから転送する
	HDC	hMemDC = ::CreateCompatibleDC(hDC);
	for(int i = 0; i < 2; i++) {
		RECT	rect;
		GetSBArrowRect(hWnd, i + 1, &rect);
		int		w = rect.right - rect.left;
		int		h = rect.bottom - rect.top;
		if(w <= 0 || h <= 0) continue;
		HBITMAP	hBmp =
			(hMemDC != NULL) ? ::CreateCompatibleBitmap(hDC, w, h) : NULL;
		if(hBmp == NULL) {
			DrawSBArrowButton(hWnd, hDC, &rect, glyphs[i],
				pressed[i], disabled[i]);
			continue;
		}
		HBITMAP	hBmpOld = (HBITMAP)::SelectObject(hMemDC, hBmp);
		RECT	rc_local = { 0, 0, w, h };
		DrawSBArrowButton(hWnd, hMemDC, &rc_local, glyphs[i],
			pressed[i], disabled[i]);
		::BitBlt(hDC, rect.left, rect.top, w, h, hMemDC, 0, 0, SRCCOPY);
		::SelectObject(hMemDC, hBmpOld);
		::DeleteObject(hBmp);
	}
	if(hMemDC != NULL) ::DeleteDC(hMemDC);

	// OSが描いた矢印ボタンが実際のボタンサイズより大きい場合、はみ出した
	// 部分の描画が残るので、溝と同じ色で消去する
	// (os_btnはOSが使う矢印ボタンの幅または高さ)
	int		os_btn = have_sbi ? sbi.dxyLineButton :
		(vert ? ::GetSystemMetrics(SM_CYVSCROLL) :
		::GetSystemMetrics(SM_CXVSCROLL));
	HBRUSH	br = ::GetSysColorBrush(COLOR_SCROLLBAR);
	RECT	rc;
	if(vert) {
		int	w = rc_client.right - rc_client.left;
		::SetRect(&rc, 0, w, rc_client.right, os_btn);
		if(rc.bottom > rc.top) ::FillRect(hDC, &rc, br);
		::SetRect(&rc, 0, rc_client.bottom - os_btn, rc_client.right,
			rc_client.bottom - w);
		if(rc.bottom > rc.top) ::FillRect(hDC, &rc, br);
	} else {
		int	h = rc_client.bottom - rc_client.top;
		::SetRect(&rc, h, 0, os_btn, rc_client.bottom);
		if(rc.right > rc.left) ::FillRect(hDC, &rc, br);
		::SetRect(&rc, rc_client.right - os_btn, 0, rc_client.right - h,
			rc_client.bottom);
		if(rc.right > rc.left) ::FillRect(hDC, &rc, br);
	}

	::ReleaseDC(hWnd, hDC);
}

// 再描画の要否を判定するため、表示に関わる状態をまとめて取得する
static DWORD SBArrowSignature(HWND hWnd)
{
	DWORD	sig = (DWORD)::GetWindowLong(hWnd, GWL_STYLE);

	RECT	rc_client;
	::GetClientRect(hWnd, &rc_client);
	sig ^= (DWORD)rc_client.right ^ ((DWORD)rc_client.bottom << 16);

	if(!::IsWindowEnabled(hWnd)) sig ^= 0x40000000;

	SCROLLBARINFO	sbi;
	sbi.cbSize = sizeof(sbi);
	if(::GetScrollBarInfo(hWnd, OBJID_CLIENT, &sbi)) {
		sig ^= (sbi.rgstate[1] &
			(STATE_SYSTEM_UNAVAILABLE | STATE_SYSTEM_PRESSED))
			^ ((sbi.rgstate[5] &
			(STATE_SYSTEM_UNAVAILABLE | STATE_SYSTEM_PRESSED)) << 4)
			^ ((DWORD)sbi.dxyLineButton << 8);
	}

	return sig;
}

// OSが矢印ボタンを描き直す可能性のあるメッセージかを判定する
static BOOL SBMessageMayRedraw(UINT msg, const CSBArrowFix *fix)
{
	switch(msg) {
	case WM_PAINT:
	case WM_ERASEBKGND:
	case WM_TIMER:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_CAPTURECHANGED:
	case WM_CANCELMODE:
	case WM_ENABLE:
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_SIZE:
	case WM_WINDOWPOSCHANGED:
	case WM_SHOWWINDOW:
	case WM_STYLECHANGED:
	case WM_SYSCOLORCHANGE:
	case WM_SETREDRAW:
	case SBM_SETPOS:
	case SBM_SETRANGE:
	case SBM_SETRANGEREDRAW:
	case SBM_SETSCROLLINFO:
	case SBM_ENABLE_ARROWS:
		return TRUE;
	case WM_MOUSEMOVE:
		// ボタン押下中のカーソル移動はOSが押下表示を更新する可能性がある
		return fix != NULL && fix->pressed_button != 0;
	}
	return FALSE;
}

static LRESULT CALLBACK SBArrowFixWndProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	CSBArrowFix	*fix = (CSBArrowFix *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if(fix == NULL) return ::DefWindowProc(hWnd, msg, wParam, lParam);

	// 矢印ボタン上のボタンダウンはOSに渡さない。OSの追跡ループは内部で
	// ボタンを描き直して小さいグリフが残るため、押下の追跡と自動リピート
	// は自前で行う
	if(msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) {
		POINT	point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		int		hit = SBArrowHitTest(hWnd, point);
		if(hit != 0 && ::IsWindowEnabled(hWnd) && !fix->tracking) {
			::SetCapture(hWnd);
			fix->pressed_button = hit;
			fix->tracking = TRUE;
			fix->repeat = FALSE;
			::SetTimer(hWnd, SBARROW_TIMER_ID, SBARROW_FIRST_DELAY, NULL);
			SBSendScrollMsg(hWnd, hit);	// 押し込んだ瞬間に1回スクロール
			RedrawSBArrows(hWnd, fix);
			fix->last_sig = SBArrowSignature(hWnd);
			return 0;
		}
	}

	LRESULT	result = ::CallWindowProc(fix->old_proc, hWnd, msg, wParam, lParam);

	switch(msg) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK: {
		POINT	point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		fix->pressed_button = SBArrowHitTest(hWnd, point);
		break; }
	case WM_TIMER:
		if(wParam == SBARROW_TIMER_ID && fix->tracking) {
			if(!fix->repeat) {
				fix->repeat = TRUE;
				::SetTimer(hWnd, SBARROW_TIMER_ID, SBARROW_REPEAT_DELAY,
					NULL);
			}
			// カーソルがボタン上にある間だけスクロールを続ける
			POINT	point;
			::GetCursorPos(&point);
			::ScreenToClient(hWnd, &point);
			RECT	rect;
			GetSBArrowRect(hWnd, fix->pressed_button, &rect);
			if(::PtInRect(&rect, point)) {
				SBSendScrollMsg(hWnd, fix->pressed_button);
			}
		}
		break;
	case WM_LBUTTONUP:
	case WM_CANCELMODE:
	case WM_CAPTURECHANGED:
		SBArrowEndTrack(hWnd, fix);
		fix->pressed_button = 0;
		break;
	case WM_NCDESTROY:
		delete fix;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		return result;
	}

	// 矢印ボタンを描き直す。
	// 毎回描き直すとちらつくため、OSがボタンを描き直す可能性のある
	// メッセージか、ボタンの状態が変化したときだけ再描画する
	DWORD	sig = SBArrowSignature(hWnd);
	if(SBMessageMayRedraw(msg, fix) || sig != fix->last_sig) {
		RedrawSBArrows(hWnd, fix);
		fix->last_sig = sig;
	}

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
			fix->last_sig = 0;
			fix->tracking = FALSE;
			fix->repeat = FALSE;
			fix->old_proc = (WNDPROC)::SetWindowLongPtr(hScrollBar,
				GWLP_WNDPROC, (LONG_PTR)SBArrowFixWndProc);
			::SetWindowLongPtr(hScrollBar, GWLP_USERDATA, (LONG_PTR)fix);
		}
	}

	return TRUE;
}
