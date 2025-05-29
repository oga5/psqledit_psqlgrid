/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// MainFrm.cpp : CMainFrame クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "psqlgrid.h"

#include "MainFrm.h"

#include "PsqlgridDoc.h"

#include "AcceleratorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_WM_INITMENU()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_ACCELERATOR, OnAccelerator)
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SESSION_INFO, OnUpdateSessionInfo)

	ON_UPDATE_COMMAND_UI(ID_INDICATOR_GRID_CALC, OnUpdateGridCalc)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_END, OnUpdateEnd)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRID_CALC_TYPE_TOTAL, ID_GRID_CALC_TYPE_NONE, OnUpdateGridCalcType)
	ON_COMMAND_RANGE(ID_GRID_CALC_TYPE_TOTAL, ID_GRID_CALC_TYPE_NONE, OnGridCalcType)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_INDICATOR_GRID_CALC,
	ID_INDICATOR_SESSION_INFO,
	ID_INDICATOR_DATA_LOCK,
//	ID_INDICATOR_CURSOR_POS,
//	ID_INDICATOR_KANA,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
	ID_INDICATOR_END,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの構築/消滅

CMainFrame::CMainFrame()
{
	m_accel = NULL;
}

CMainFrame::~CMainFrame()
{
	DestroyAccelerator();
}

BOOL CMainFrame::SetIndicators()
{
	int indicator_cnt = sizeof(indicators)/sizeof(UINT);

	if(!g_option.grid.end_key_like_excel) indicator_cnt--;

	return m_wndStatusBar.SetIndicators(indicators, indicator_cnt);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
/*	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // 作成に失敗
	}
*/
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT, 
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | 
		CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // 作成に失敗
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // 作成に失敗
	}

	if(!SetIndicators()) {
		TRACE0("Failed to create status bar\n");
		return -1;      // 作成に失敗
	}

	// インジケータの見た目を調整
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_GRID_CALC, SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_SESSION_INFO, SBPS_NORMAL, 150);
	m_wndStatusBar.SetPaneInfo(3, ID_INDICATOR_DATA_LOCK, SBPS_NORMAL, 60);
//	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_CURSOR_POS, SBPS_NORMAL, 100);

	// ステータスバーに，ログイン情報を表示
	m_wndStatusBar.SetPaneText(1, g_session_info, TRUE);

	// TODO: もしツール チップスが必要ない場合、ここを削除してください。
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: ツール バーをドッキング可能にしない場合は以下の３行を削除
	//       してください。
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して、Window クラスやスタイルを
	//       修正してください。
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ


BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	m_wndSplitter.SetDeletePaneMessage(UPD_SET_HEADER_STYLE);
	return m_wndSplitter.Create(this, 2, 2, CSize(10, 10), pContext);
	
//	return CFrameWnd::OnCreateClient(lpcs, pContext);
}

void CMainFrame::OnUpdateSessionInfo(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(g_session_info);
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CDocument* pDocument = GetActiveDocument();
	if(pDocument != NULL) {
		CString title;
//		title.Format("OSqlGrid - %s - %s", pDocument->GetTitle(), g_session_info);
		title.Format(_T("%s - %s"), g_session_info, pDocument->GetTitle());
		SetWindowText(title);
	}
}

void CMainFrame::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	
	if(GetActiveDocument()) {
		GetActiveDocument()->UpdateAllViews(NULL, UPD_WINDOW_MOVED, 0);
	}	
}

void CMainFrame::CreateAccelerator()
{
	DestroyAccelerator();

	m_accel = CreateAcceleratorTable(m_accel_list.get_accel_list(), m_accel_list.get_accel_cnt());
	m_hAccelTable = m_accel;
}

void CMainFrame::DestroyAccelerator()
{
	if(m_accel != NULL) {
		DestroyAcceleratorTable(m_accel);
		m_accel = NULL;
		m_hAccelTable = NULL;
	}
}

void CMainFrame::OnAccelerator() 
{
	CAcceleratorDlg	dlg;

	dlg.m_accel_list = m_accel_list;

	if(dlg.DoModal() != IDOK) return;

	m_accel_list = dlg.m_accel_list;
	m_accel_list.save_accel_list();

	CreateAccelerator();
}

void CMainFrame::SetAcceleratorToMenu(CMenu *pMenu)
{
	int	i;
	CString menu_str;
	CString accel_str;
	CString new_menu_str;
	UINT	menu_state;

	for(i = 0; i < pMenu->GetMenuItemCount(); i++) {
		if(pMenu->GetMenuItemID(i) == 0) continue;	// セパレータ
		if(pMenu->GetMenuItemID(i) == -1) {			// ポップアップ
			if(pMenu->GetSubMenu(i) != NULL) {
				//SetAcceleratorToMenu(pMenu->GetSubMenu(i));
				continue;
			}
		}

		pMenu->GetMenuString(i, menu_str, MF_BYPOSITION);
		if(menu_str == "") continue;

		// 既にショートカットキーが含まれている場合，削除する
		new_menu_str = menu_str;
		int tab_pos = new_menu_str.Find('\t');
		if(tab_pos != -1) new_menu_str.Delete(tab_pos, menu_str.GetLength() - tab_pos);

		if(m_accel_list.search_accel_str2(pMenu->GetMenuItemID(i), accel_str) >= 0) {
			new_menu_str += "\t" + accel_str;
		}

		if(new_menu_str != menu_str) {
			menu_state = pMenu->GetMenuState(i, MF_BYPOSITION);

			pMenu->ModifyMenu(i, MF_BYPOSITION | MF_STRING | menu_state, 
				pMenu->GetMenuItemID(i), new_menu_str.GetBuffer(0));
		}
	}
}

void CMainFrame::InitAccelerator()
{
	m_accel_list.load_accel_list();
	CreateAccelerator();
}

void CMainFrame::OnInitMenu(CMenu* pMenu) 
{
	CFrameWnd::OnInitMenu(pMenu);

	SetAcceleratorToMenu(pMenu);
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	SetAcceleratorToMenu(pPopupMenu);
}

BOOL CMainFrame::HitTestStatusBar(CPoint pt)
{
	if(!m_wndStatusBar.IsWindowVisible()) return FALSE;

	CRect	wnd_rect;

	m_wndStatusBar.GetWindowRect(wnd_rect);

	return wnd_rect.PtInRect(pt);
}

void CMainFrame::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if(HitTestStatusBar(point)) {
		CMenu	menu;
		menu.LoadMenu(IDR_GRID_CALC_MENU);

		// CMainFrameを親ウィンドウにすると、メニューの有効無効を、メインメニューと同じにできる
		// MDIなので，親の親を引数に渡す
		// メニューを上方向に表示するため，TPM_BOTTOMALIGNを指定
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, point.x, point.y, this);

		// メニュー削除
		menu.DestroyMenu();
	}
}

void CMainFrame::OnUpdateGridCalc(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(g_grid_calc);
}

void CMainFrame::OnUpdateGridCalcType(CCmdUI* pCmdUI)
{
	if((UINT)g_option.grid_calc_type == pCmdUI->m_nID - ID_GRID_CALC_TYPE_TOTAL) {
		pCmdUI->SetCheck(TRUE);
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

void CMainFrame::OnGridCalcType(UINT nID)
{
	g_option.grid_calc_type = (GRID_CALC_TYPE)(nID - ID_GRID_CALC_TYPE_TOTAL);

	if(GetActiveFrame() && GetActiveFrame()->GetActiveDocument()) {
		GetActiveFrame()->GetActiveDocument()->UpdateAllViews(NULL, UPD_CALC_GRID_SELECTED_DATA, 0);
	}
}

void CMainFrame::OnUpdateEnd(CCmdUI* pCmdUI)
{
	pCmdUI->SetText(_T(""));
}
