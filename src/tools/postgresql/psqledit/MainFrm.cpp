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
#include "psqledit.h"

#include "MainFrm.h"
#include "PsqleditDoc.h"
#include "AcceleratorDlg.h"
#include "ShortCutSqlListDlg.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_INITMENU()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_NEXT_DOC, OnNextDoc)
	ON_COMMAND(ID_PREV_DOC, OnPrevDoc)
	ON_COMMAND(ID_ACCELERATOR, OnAccelerator)
	ON_COMMAND(ID_TAB_MOVE_LAST, OnTabMoveLast)
	ON_COMMAND(ID_TAB_MOVE_LEFT, OnTabMoveLeft)
	ON_COMMAND(ID_TAB_MOVE_RIGHT, OnTabMoveRight)
	ON_COMMAND(ID_TAB_MOVE_TOP, OnTabMoveTop)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_LAST, OnUpdateTabMoveLast)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_LEFT, OnUpdateTabMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_RIGHT, OnUpdateTabMoveRight)
	ON_UPDATE_COMMAND_UI(ID_TAB_MOVE_TOP, OnUpdateTabMoveTop)
	ON_COMMAND(ID_SORT_TAB, OnSortTab)
	ON_UPDATE_COMMAND_UI(ID_SORT_TAB, OnUpdateSortTab)
	ON_COMMAND(ID_COPY_VALUE, OnCopyValue)
	ON_COMMAND(ID_SETUP_SHORTCUT_SQL, OnSetupShortcutSql)
	ON_COMMAND(ID_VIEW_OBJECT_BAR, OnViewObjectBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECT_BAR, OnUpdateViewObjectBar)
	ON_COMMAND(ID_VIEW_OBJECT_DETAIL_BAR, OnViewObjectDetailBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECT_DETAIL_BAR, OnUpdateViewObjectDetailBar)
	ON_COMMAND(ID_VIEW_TAB_BAR, OnViewTabBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TAB_BAR, OnUpdateViewTabBar)
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CURSOR_POS, OnUpdateCursorPos)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SESSION_INFO, OnUpdateSessionInfo)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NEXT_DOC, ID_PREV_DOC, OnUpdateNextPrevDoc)

	ON_UPDATE_COMMAND_UI(ID_INDICATOR_FILE_TYPE, OnUpdateFileType)

	ON_UPDATE_COMMAND_UI(ID_INDICATOR_GRID_CALC, OnUpdateGridCalc)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRID_CALC_TYPE_TOTAL, ID_GRID_CALC_TYPE_NONE, OnUpdateGridCalcType)
	ON_COMMAND_RANGE(ID_GRID_CALC_TYPE_TOTAL, ID_GRID_CALC_TYPE_NONE, OnGridCalcType)
	ON_COMMAND(ID_OBJECTBAR_FILTER_ADD, &CMainFrame::OnObjectbarFilterAdd)
	ON_COMMAND(ID_OBJECTBAR_FILTER_CLEAR, &CMainFrame::OnObjectbarFilterClear)
	ON_COMMAND(ID_OBJECTBAR_FILTER_DEL, &CMainFrame::OnObjectbarFilterDel)
	ON_COMMAND(ID_GRID_CALC_COPY, &CMainFrame::OnGrcalcCopy)
	ON_UPDATE_COMMAND_UI(ID_GRID_CALC_COPY, &CMainFrame::OnUpdateGrcalcCopy)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // ステータス ライン インジケータ
	ID_INDICATOR_GRID_CALC,
	ID_INDICATOR_FILE_TYPE,
	ID_INDICATOR_SESSION_INFO,
	ID_INDICATOR_CURSOR_POS,
//	ID_INDICATOR_KANA,
	ID_INDICATOR_OVR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの構築/消滅

CMainFrame::CMainFrame()
{
	// TODO: この位置にメンバの初期化処理コードを追加してください。
	m_doc_accel = NULL;
	m_frame_accel = NULL;
	g_bind_dlg_accel = NULL;
	m_accel_kind = ACCEL_KIND_FRAME;
}

CMainFrame::~CMainFrame()
{
	DestroyAccelerator();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetWindowPosition(lpCreateStruct);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // 作成に失敗
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // 作成に失敗
	}

    if (!g_object_bar.Create(_T("ObjectBar"), this, CSize(80, 80),
        TRUE, AFX_IDW_CONTROLBAR_FIRST + 33))
    {
        TRACE0("Failed to create ObjBar\n");
        return -1;      // fail to create
	}
    if (!g_object_detail_bar.Create(_T("ObjectDetailBar"), this, CSize(80, 80),
        TRUE, AFX_IDW_CONTROLBAR_FIRST + 34))
    {
        TRACE0("Failed to create ObjBar\n");
        return -1;      // fail to create
	}
    if (!g_tab_bar.Create(_T("TabBar"), this, CSize(300, 10),
        TRUE, AFX_IDW_CONTROLBAR_FIRST + 35))
    {
        TRACE0("Failed to create TabBar\n");
        return -1;      // fail to create
	}

	g_object_bar.SetFont(&g_dlg_bar_font);
	g_object_detail_bar.SetFont(&g_dlg_bar_font);
	g_tab_bar.SetFont(&g_dlg_bar_font);
	g_tab_bar.SetOption();

	// インジケータの見た目を調整
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_GRID_CALC, SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_FILE_TYPE, SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneInfo(3, ID_INDICATOR_SESSION_INFO, SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(4, ID_INDICATOR_CURSOR_POS, SBPS_NORMAL, 90);

	g_object_bar.SetBarStyle(g_object_bar.GetBarStyle() | CBRS_SIZE_DYNAMIC);
	g_object_detail_bar.SetBarStyle(g_object_detail_bar.GetBarStyle() | CBRS_SIZE_DYNAMIC);
	g_tab_bar.SetBarStyle(g_tab_bar.GetBarStyle() | CBRS_SIZE_DYNAMIC);

	// TODO: ツール バーをドッキング可能にしない場合は以下の３行を削除
	//       してください。
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

    g_object_bar.EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&g_object_bar, AFX_IDW_DOCKBAR_RIGHT);
    g_object_detail_bar.EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&g_object_detail_bar, AFX_IDW_DOCKBAR_RIGHT);
    g_tab_bar.EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
    DockControlBar(&g_tab_bar, AFX_IDW_DOCKBAR_TOP);

	SetBarState();

	CSizingControlBar::GlobalLoadState(_T("BarState"));
	LoadBarState(_T("BarState"));

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	// TODO: この位置で CREATESTRUCT cs を修正して、Window クラスやスタイルを
	//       修正してください。
	cs.style &= ~FWS_ADDTOTITLE;
	cs.style &= ~FWS_PREFIXTITLE;

	if(AfxGetApp()->GetProfileInt(_T("WINDOW"), _T("ZOOMED"), FALSE) != FALSE) {
		cs.style |= WS_MAXIMIZE;
	}

	return TRUE;
}

void CMainFrame::GetMessageString(UINT nID, CString& rMessage) const
{
	switch(nID) {
	case AFX_IDS_IDLEMESSAGE:
		rMessage = m_idle_msg;
		break;
	default:
		CFrameWnd::GetMessageString(nID, rMessage);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

void CMainFrame::CreateAccelerator()
{
	DestroyAccelerator();

	CAccelList frame_accel_list;
	frame_accel_list.make_no_allow_key_list(m_accel_list);
	m_frame_accel = CreateAcceleratorTable(frame_accel_list.get_accel_list(),
		frame_accel_list.get_accel_cnt());

	m_doc_accel = CreateAcceleratorTable(m_accel_list.get_accel_list(), m_accel_list.get_accel_cnt());

	g_escape_cmd = m_accel_list.search_cmd(_T("ESCAPE"));
	
	SetAccelerator(m_accel_kind);

	{
		WORD key_arr[] = { ID_EDIT_UNDO, ID_EDIT_REDO, ID_EDIT_COPY, ID_EDIT_PASTE, ID_EDIT_CUT };
		CAccelList bind_dlg_accel_list;
		bind_dlg_accel_list.make_filterd_accel_list(m_accel_list, key_arr, sizeof(key_arr) / sizeof(key_arr[0]));
		if(bind_dlg_accel_list.get_accel_cnt() > 0) {
			g_bind_dlg_accel = CreateAcceleratorTable(bind_dlg_accel_list.get_accel_list(), bind_dlg_accel_list.get_accel_cnt());
		}
	}
}

void CMainFrame::DestroyAccelerator()
{
	if(m_doc_accel != NULL) {
		DestroyAcceleratorTable(m_doc_accel);
		m_doc_accel = NULL;
	}
	if(m_frame_accel != NULL) {
		DestroyAcceleratorTable(m_frame_accel);
		m_frame_accel = NULL;
		m_hAccelTable = NULL;
	}

	if(g_bind_dlg_accel != NULL) {
		DestroyAcceleratorTable(g_bind_dlg_accel);
		g_bind_dlg_accel = NULL;
	}
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

void CMainFrame::SetAccelerator(int accel_kind)
{
	switch(accel_kind) {
	case ACCEL_KIND_FRAME:
		m_hAccelTable = m_frame_accel;
		break;
	case ACCEL_KIND_DOC:
		m_hAccelTable = m_doc_accel;
		break;
	}
	m_accel_kind = accel_kind;
}

void CMainFrame::OnInitMenu(CMenu* pMenu) 
{
	CMDIFrameWnd::OnInitMenu(pMenu);

	SetAcceleratorToMenu(pMenu);
}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CMDIFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	SetAcceleratorToMenu(pPopupMenu);
}

void CMainFrame::OnUpdateCursorPos(CCmdUI *pCmdUI)
{
	static POINT cur_pos = {-1, -1};

	if(cur_pos.x != g_cur_pos.x || cur_pos.y != g_cur_pos.y) {
		cur_pos = g_cur_pos;
		CString str;
		str.Format(_T(" %d 行，%d 列"), cur_pos.y, cur_pos.x);
		pCmdUI->Enable();
		pCmdUI->SetText(str);
	}
}

void CMainFrame::OnUpdateNextPrevDoc(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_tab_bar.CanNextPrevDoc());
}

BOOL CMainFrame::DestroyWindow() 
{
	g_object_bar.SaveColumnWidth();
	g_object_detail_bar.SaveColumnWidth();

	CSizingControlBar::GlobalSaveState(_T("BarState"));
	SaveBarState(_T("BarState"));
	
	return CMDIFrameWnd::DestroyWindow();
}

void CMainFrame::OnUpdateSessionInfo(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(g_session_info);
}

void CMainFrame::SetBarState()
{
	CWinApp	*pApp = AfxGetApp();

	// 初回の起動かチェックする
	if(pApp->GetProfileInt(_T("BarState-Summary"), _T("Bars"), 0xffffffff) != 0xffffffff) return;
	
	pApp->WriteProfileInt(_T("BarState-Summary"), _T("Bars"), 0x00000007);
	pApp->WriteProfileInt(_T("BarState-Summary"), _T("ScreenCX"), 0x00000400);
	pApp->WriteProfileInt(_T("BarState-Summary"), _T("ScreenCY"), 0x00000300);
	
	pApp->WriteProfileInt(_T("BarState-SCBar-59425"), _T("sizeHorzCX"), 0x0000033e);
	pApp->WriteProfileInt(_T("BarState-SCBar-59425"), _T("sizeHorzCY"), 0x00000050);
	pApp->WriteProfileInt(_T("BarState-SCBar-59425"), _T("sizeVertCX"), 0x00000103);
	pApp->WriteProfileInt(_T("BarState-SCBar-59425"), _T("sizeVertCY"), 0x000000de);
	pApp->WriteProfileInt(_T("BarState-SCBar-59425"), _T("sizeFloatCX"), 0x00000108);
	pApp->WriteProfileInt(_T("BarState-SCBar-59425"), _T("sizeFloatCY"), 0x000000e7);
	
	pApp->WriteProfileInt(_T("BarState-SCBar-59426"), _T("sizeHorzCX"), 0x00000050);
	pApp->WriteProfileInt(_T("BarState-SCBar-59426"), _T("sizeHorzCY"), 0x00000050);
	pApp->WriteProfileInt(_T("BarState-SCBar-59426"), _T("sizeVertCX"), 0x00000103);
	pApp->WriteProfileInt(_T("BarState-SCBar-59426"), _T("sizeVertCY"), 0x000000ef);
	pApp->WriteProfileInt(_T("BarState-SCBar-59426"), _T("sizeFloatCX"), 0x00000102);
	pApp->WriteProfileInt(_T("BarState-SCBar-59426"), _T("sizeFloatCY"), 0x000000e1);
	
	pApp->WriteProfileInt(_T("BarState-SCBar-59427"), _T("sizeHorzCX"), 0x0000033e);
	pApp->WriteProfileInt(_T("BarState-SCBar-59427"), _T("sizeHorzCY"), 0x00000020);
	pApp->WriteProfileInt(_T("BarState-SCBar-59427"), _T("sizeVertCX"), 0x0000012c);
	pApp->WriteProfileInt(_T("BarState-SCBar-59427"), _T("sizeVertCY"), 0x00000020);
	pApp->WriteProfileInt(_T("BarState-SCBar-59427"), _T("sizeFloatCX"), 0x0000012c);
	pApp->WriteProfileInt(_T("BarState-SCBar-59427"), _T("sizeFloatCY"), 0x00000020);
	
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("BarID"), 0x0000e800);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("XPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("YPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUWidth"), 0x000001c2);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("Docking"), 0x00000001);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUDockID"), 0x0000e81b);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUDockLeftPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUDockTopPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUDockRightPos"), 0x000001cb);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUDockBottomPos"), 0x00000018);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUFloatStyle"), 0x00002000);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUFloatXPos"), 0x000000c4);
	pApp->WriteProfileInt(_T("BarState-Bar0"), _T("MRUFloatYPos"), 0x00000117);
	
	pApp->WriteProfileInt(_T("BarState-Bar1"), _T("BarID"), 0x0000e801);
	
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("BarID"), 0x0000e821);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("XPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("YPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("Docking"), 0x00000001);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUDockID"), 0x0000e81d);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUDockLeftPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUDockTopPos"), 0x000000eb);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUDockRightPos"), 0x00000101);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUDockBottomPos"), 0x000001c9);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUFloatStyle"), 0x00001000);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUFloatXPos"), 0x00000110);
	pApp->WriteProfileInt(_T("BarState-Bar2"), _T("MRUFloatYPos"), 0x0000011a);
	
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("BarID"), 0x0000e822);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("XPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("YPos"), 0x000000da);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("Docking"), 0x00000001);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUDockID"), 0x0000e81d);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUDockLeftPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUDockTopPos"), 0x000000da);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUDockRightPos"), 0x00000101);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUDockBottomPos"), 0x000001c9);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUFloatStyle"), 0x00001000);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUFloatXPos"), 0x0000009e);
	pApp->WriteProfileInt(_T("BarState-Bar3"), _T("MRUFloatYPos"), 0x0000014a);
	
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("BarID"), 0x0000e823);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("XPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("YPos"), 0x00000016);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("Docking"), 0x00000001);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUDockID"), 0x0000e81b);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUDockLeftPos"), 0xfffffffe);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUDockTopPos"), 0x00000016);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUDockRightPos"), 0x0000033c);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUDockBottomPos"), 0x00000036);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUFloatStyle"), 0x00002000);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUFloatXPos"), 0x0000008b);
	pApp->WriteProfileInt(_T("BarState-Bar4"), _T("MRUFloatYPos"), 0x000000ea);
	
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("BarID"), 0x0000e81b);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bars"), 0x00000007);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bar#0"), 0x00000000);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bar#1"), 0x0000e800);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bar#2"), 0x00000000);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bar#3"), 0x0000e823);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bar#4"), 0x00000000);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bar#5"), 0x0001e821);
	pApp->WriteProfileInt(_T("BarState-Bar5"), _T("Bar#6"), 0x00000000);
	
	pApp->WriteProfileInt(_T("BarState-Bar6"), _T("BarID"), 0x0000e81d);
	pApp->WriteProfileInt(_T("BarState-Bar6"), _T("Bars"), 0x00000004);
	pApp->WriteProfileInt(_T("BarState-Bar6"), _T("Bar#0"), 0x00000000);
	pApp->WriteProfileInt(_T("BarState-Bar6"), _T("Bar#1"), 0x0000e821);
	pApp->WriteProfileInt(_T("BarState-Bar6"), _T("Bar#2"), 0x0000e822);
	pApp->WriteProfileInt(_T("BarState-Bar6"), _T("Bar#3"), 0x00000000);
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

void CMainFrame::OnTabMoveLeft() 
{
	g_tab_bar.TabMoveLeft();
}

void CMainFrame::OnTabMoveRight() 
{
	g_tab_bar.TabMoveRight();
}

void CMainFrame::OnUpdateTabMoveLeft(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_tab_bar.IsWindowVisible() && g_tab_bar.CanTabMoveLeft());
}

void CMainFrame::OnUpdateTabMoveRight(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_tab_bar.IsWindowVisible() && g_tab_bar.CanTabMoveRight());
}

void CMainFrame::OnTabMoveTop() 
{
	g_tab_bar.TabMoveTop();
}

void CMainFrame::OnTabMoveLast() 
{
	g_tab_bar.TabMoveLast();
}

void CMainFrame::OnUpdateTabMoveTop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_tab_bar.IsWindowVisible() && g_tab_bar.CanTabMoveLeft());
}

void CMainFrame::OnUpdateTabMoveLast(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_tab_bar.IsWindowVisible() && g_tab_bar.CanTabMoveRight());
}

void CMainFrame::OnNextDoc() 
{
	g_tab_bar.NextDoc();
}

void CMainFrame::OnPrevDoc() 
{
	g_tab_bar.PrevDoc();
}

void CMainFrame::OnSortTab() 
{
	g_tab_bar.SortTab();
}

void CMainFrame::OnUpdateSortTab(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_tab_bar.IsWindowVisible() && g_tab_bar.CanSortTab());
}


void CMainFrame::OnCopyValue() 
{
	g_object_detail_bar.CopyValue();
}

void CMainFrame::OnSetupShortcutSql() 
{
	CShortCutSqlListDlg	dlg;

	dlg.m_accel_list = m_accel_list;

	if(dlg.DoModal() != IDOK) return;

	m_accel_list = dlg.m_accel_list;
	m_accel_list.save_accel_list();

	CreateAccelerator();
	CPsqleditDoc::LoadShortCutSqlList();
}

void CMainFrame::OnViewObjectBar() 
{
	ShowControlBar(&g_object_bar, !g_object_bar.IsWindowVisible(), FALSE);
}

void CMainFrame::OnUpdateViewObjectBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(g_object_bar.IsWindowVisible());
}

void CMainFrame::OnViewObjectDetailBar() 
{
	ShowControlBar(&g_object_detail_bar, !g_object_detail_bar.IsWindowVisible(), FALSE);
}

void CMainFrame::OnUpdateViewObjectDetailBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(g_object_detail_bar.IsWindowVisible());
}

void CMainFrame::OnViewTabBar() 
{
	ShowControlBar(&g_tab_bar, !g_tab_bar.IsWindowVisible(), FALSE);
}

void CMainFrame::OnUpdateViewTabBar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(g_tab_bar.IsWindowVisible());
}

void CMainFrame::SetWindowPosition(LPCREATESTRUCT lpCreateStruct)
{
	int top = lpCreateStruct->y;
	int left = lpCreateStruct->x;
	int width = AfxGetApp()->GetProfileInt(_T("WINDOW"), _T("WIDTH"), 0);
	int height = AfxGetApp()->GetProfileInt(_T("WINDOW"), _T("HEIGHT"), 0);

	// 最初の起動時
	if(width == 0 || height == 0) return;

	CRect work_rect;
	if(!SystemParametersInfo(SPI_GETWORKAREA, 0, &work_rect, 0)) return;

	// タスクバーが上にあるとき，ウィンドウがタスクバーに隠れないようにする
	if(top < work_rect.top) top = work_rect.top;
	if(left < work_rect.left) left = work_rect.left;

	// 画面からはみ出さないようにする
	if(top + height >= work_rect.Height()) {
		top = work_rect.top;
		// 画面よりウィンドウが大きい場合
		if(top + height >= work_rect.Height()) {
			height = GetSystemMetrics(SM_CYSCREEN);
		}
	}

	if(left + width >= work_rect.Width()) {
		left = work_rect.left;
		// 画面よりウィンドウが大きい場合
		if(left + width >= work_rect.Width()) {
			width = work_rect.Width();
		}
	}

	SetWindowPos(NULL, left, top, width, height, SWP_NOZORDER);
}


void CMainFrame::OnDestroy() 
{
	if(g_show_clob_dlg.IsVisible()) {
		g_show_clob_dlg.GetDlg()->ShowWindow(SW_HIDE);
	}

	CMDIFrameWnd::OnDestroy();
	
	if(IsZoomed()) {
		AfxGetApp()->WriteProfileInt(_T("WINDOW"), _T("ZOOMED"), TRUE);
	} else if(IsIconic() == FALSE) {
		CRect	rect;
		GetWindowRect(rect);
		AfxGetApp()->WriteProfileInt(_T("WINDOW"), _T("WIDTH"), rect.Width());
		AfxGetApp()->WriteProfileInt(_T("WINDOW"), _T("HEIGHT"), rect.Height());
		AfxGetApp()->WriteProfileInt(_T("WINDOW"), _T("ZOOMED"), FALSE);
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

void CMainFrame::OnUpdateFileType(CCmdUI *pCmdUI)
{
	pCmdUI->SetText(g_file_type);
}

void CMainFrame::OnClose() 
{
	g_save_mode = SAVE_MODE_ALL_QUERY;
	
	CMDIFrameWnd::OnClose();
}

void CMainFrame::OnObjectbarFilterAdd()
{
	if(g_object_bar.isKeywordActive()) {
	} else {
		GetActiveWindow()->PostMessage(WM_COMMAND, ID_OBJECTBAR_FILTER_ADD);
	}
}

void CMainFrame::OnObjectbarFilterClear()
{
	if(g_object_bar.isKeywordActive()) {
	} else {
		GetActiveWindow()->PostMessage(WM_COMMAND, ID_OBJECTBAR_FILTER_DEL);
	}
}

void CMainFrame::OnObjectbarFilterDel()
{
	if(g_object_bar.isKeywordActive()) {
	} else {
		GetActiveWindow()->PostMessage(WM_COMMAND, ID_OBJECTBAR_FILTER_CLEAR);
	}
}

void CMainFrame::OnGrcalcCopy()
{
	CString val = m_wndStatusBar.GetPaneText(1);
	CopyClipboard(val.GetBuffer(0));
}

void CMainFrame::OnUpdateGrcalcCopy(CCmdUI* pCmdUI)
{
	CString val = m_wndStatusBar.GetPaneText(1);
	pCmdUI->Enable(!val.IsEmpty());
}
