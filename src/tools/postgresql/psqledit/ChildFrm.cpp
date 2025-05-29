/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ChildFrm.cpp : CChildFrame クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "psqledit.h"

#include "ChildFrm.h"

#include "PSQLEditDoc.h"
#include "SQLEditView.h"
#include "SQLExplainView.h"
#include "GridView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_COMMAND(ID_ADJUST_ALL_COL_WIDTH, OnAdjustAllColWidth)
	ON_UPDATE_COMMAND_UI(ID_ADJUST_ALL_COL_WIDTH, OnUpdateAdjustAllColWidth)
	ON_COMMAND(ID_EQUAL_ALL_COL_WIDTH, OnEqualAllColWidth)
	ON_UPDATE_COMMAND_UI(ID_EQUAL_ALL_COL_WIDTH, OnUpdateEqualAllColWidth)
	ON_COMMAND(ID_ACTIVE_RESULT_GRID, OnActiveResultGrid)
	ON_COMMAND(ID_ACTIVE_SQL_EDITOR, OnActiveSqlEditor)
	ON_COMMAND(ID_ACTIVE_SQL_RESULT, OnActiveSqlResult)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_SQL_EDITOR_MAXIMIZE, OnSqlEditorMaximize)
	ON_COMMAND(ID_RESULT_GRID_MAXIMIZE, OnResultGridMaximize)
	//}}AFX_MSG_MAP
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame クラスの構築/消滅

CChildFrame::CChildFrame()
{
	// TODO: メンバ初期化コードをこの位置に追加してください。
	m_sql_editor_height = 0;
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs の設定を行って、Window クラスまたは
	//       スタイルを変更してください。

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CChildFrame クラスの診断

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame クラスのメッセージハンドラ

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
	CRect	rect;

	// 分割ウィンドウを作成
	if(m_wnd_splitter.CreateStatic(this, 2, 1) == 0) goto ERR;

/*
	if(m_wnd_splitter.CreateView(0, 0, RUNTIME_CLASS(CSQLEditView),
		CSize(300, 100), pContext) == 0) goto ERR;
	m_edit_view = (CView *)m_wnd_splitter.GetPane(0, 0);

	if(m_wnd_splitter.CreateView(1, 0, RUNTIME_CLASS(CGridView),
		CSize(300, 100), pContext) == 0) goto ERR;
	m_grid_view = (CView *)m_wnd_splitter.GetPane(1, 0);

    ::SetWindowLong(m_grid_view->m_hWnd, GWL_ID, 0);
    m_grid_view->ShowWindow(SW_HIDE);
*/
	{
		m_wnd_edit_splitter.SetDeletePaneMessage(UPD_DELETE_PANE);

		CCreateContext editContext = *pContext;
		editContext.m_pNewViewClass = RUNTIME_CLASS(CSQLEditView);

		// CSize(5, 5)を大きくすると，分割ウィンドウが動作しなくなるので，変更しないこと
		if(m_wnd_edit_splitter.Create(&m_wnd_splitter, 2, 1, CSize(5, 5), &editContext,
			WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | SPLS_DYNAMIC_SPLIT,
			m_wnd_splitter.IdFromRowCol(0, 0)) == 0) goto ERR;
	}

	{
		m_wnd_grid_splitter.SetDeletePaneMessage(UPD_SET_HEADER_STYLE);

		CCreateContext gridContext = *pContext;
		gridContext.m_pNewViewClass = RUNTIME_CLASS(CGridView);

		// CSize(5, 5)を大きくすると，分割ウィンドウが動作しなくなるので，変更しないこと
		if(m_wnd_grid_splitter.Create(&m_wnd_splitter, 2, 2, CSize(5, 5), &gridContext,
			WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | SPLS_DYNAMIC_SPLIT,
			m_wnd_splitter.IdFromRowCol(1, 0)) == 0) goto ERR;

		::SetWindowLong(m_wnd_grid_splitter.m_hWnd, GWL_ID, 0);
		m_wnd_grid_splitter.SetActivePane(0, 0);
		m_wnd_grid_splitter.ShowWindow(SW_HIDE);
	}
	
	if(m_wnd_splitter.CreateView(1, 0, RUNTIME_CLASS(CSQLExplainView),
		CSize(300, 100), pContext) == 0) goto ERR;
	m_msg_view = (CView *)m_wnd_splitter.GetPane(1, 0);

    m_wnd_splitter.RecalcLayout();

	m_view_kind = MSG_VIEW;

	// 各ペインの大きさを設定
	GetWindowRect(rect);
	m_wnd_splitter.SetRowInfo(0, rect.Height() * 3 / 5, 20);

	m_wnd_edit_splitter.SetActivePane(0, 0);
	m_wnd_splitter.SetActivePane(0, 0);

	return TRUE;

ERR:
	MessageBox(_T("Error at CChildFlame::OnCreateClient"), _T("ERROR"), 
		MB_ICONEXCLAMATION | MB_OK);
	return FALSE;
}

void CChildFrame::SwitchView(int view_kind)
{
	if(g_show_clob_dlg.IsVisible()) {
		g_show_clob_dlg.GetDlg()->ShowWindow(SW_HIDE);
	}

	if(view_kind == SWITCH_VIEW) {
		if(m_view_kind == MSG_VIEW) {
			view_kind = GRID_VIEW;
		} else {
			view_kind = MSG_VIEW;
		}
	}

	RestorePain1();

	if(m_view_kind == view_kind) return;
	m_view_kind = view_kind;

	CWnd *pOldView, *pNewView;

	if(m_view_kind == MSG_VIEW) {
		pOldView = &m_wnd_grid_splitter;
		pNewView = m_msg_view;
	} else {
		pOldView = m_msg_view;
		pNewView = &m_wnd_grid_splitter;
	}

	// 現在アクティブのビューを取得
	CView *p_old_active_view = m_wnd_splitter.GetParentFrame()->GetActiveView();

	// Set the child window ID of the active view to the ID of the corresponding
    // pane. Set the child ID of the previously active view to some other ID.
    ::SetWindowLong(pOldView->m_hWnd, GWL_ID, 0);
    ::SetWindowLong(pNewView->m_hWnd, GWL_ID, m_wnd_splitter.IdFromRowCol(1, 0));

    // Show the newly active view and hide the inactive view.
    pNewView->ShowWindow(SW_SHOW);
    pOldView->ShowWindow(SW_HIDE);
/*
    // Attach new view
    pDoc->AddView(pNewView);
*/
	// Set active
	if(p_old_active_view != NULL) {
		if(p_old_active_view->IsKindOf(RUNTIME_CLASS(CSQLExplainView))) {
			m_wnd_splitter.GetParentFrame()->SetActiveView((CView *)m_wnd_grid_splitter.GetPane(0, 0));
		} else if(p_old_active_view->IsKindOf(RUNTIME_CLASS(CGridView))) {
			m_wnd_splitter.GetParentFrame()->SetActiveView(m_msg_view);
		}
	}

    m_wnd_splitter.RecalcLayout();

//    pNewView->SendMessage(WM_PAINT);
	CView *edit_view = (CView *)m_wnd_edit_splitter.GetPane(0, 0);
	edit_view->GetDocument()->UpdateAllViews((CView *)m_wnd_splitter.GetPane(0, 0), UPD_SWITCH, NULL);
}


void CChildFrame::OnAdjustAllColWidth() 
{
	CGridView *grid_view = (CGridView *)m_wnd_grid_splitter.GetPane(0, 0);
	((CGridView *)grid_view)->AdjustAllColWidth();
}

void CChildFrame::OnUpdateAdjustAllColWidth(CCmdUI* pCmdUI) 
{
	if(m_view_kind == GRID_VIEW) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CChildFrame::OnEqualAllColWidth() 
{
	CGridView *grid_view = (CGridView *)m_wnd_grid_splitter.GetPane(0, 0);
	((CGridView *)grid_view)->EqualAllColWidth();
}

void CChildFrame::OnUpdateEqualAllColWidth(CCmdUI* pCmdUI) 
{
	if(m_view_kind == GRID_VIEW) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CChildFrame::OnActiveSqlEditor() 
{
	CView *edit_view = (CView *)m_wnd_edit_splitter.GetPane(0, 0);
	SetActiveView(edit_view);
	edit_view->SetFocus();
}

void CChildFrame::OnActiveSqlResult() 
{
	SwitchView(MSG_VIEW);
	RestorePain1();
	SetActiveView(m_msg_view);
	m_msg_view->SetFocus();
}

void CChildFrame::OnActiveResultGrid() 
{
	SwitchView(GRID_VIEW);
	RestorePain1();

	CGridView *grid_view = (CGridView *)m_wnd_grid_splitter.GetPane(0, 0);
	SetActiveView(grid_view);
	grid_view->SetFocus();
}

void CChildFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIChildWnd::OnSetFocus(pOldWnd);

	SetTabBar();
}

void CChildFrame::SetTabBar() 
{
	CView *pview = GetActiveView();
	if(pview != NULL) {
		g_tab_bar.ActiveTab(GetActiveView()->GetDocument());
		CPsqleditDoc *pdoc = (CPsqleditDoc *)pview->GetDocument();
		pdoc->OnActiveDocument();
	}
}

void CChildFrame::ActivateFrame(int nCmdShow) 
{
	CMDIChildWnd::ActivateFrame(nCmdShow);

	SetTabBar();
}

int CChildFrame::GetPaneHeight()
{
	int		height;
	int		minheight;
	m_wnd_splitter.GetRowInfo(0, height, minheight);
	return height;
}

void CChildFrame::SetPaneHeight(int pane_height)
{
	// 各ペインの大きさを設定
	CRect	rect;
	GetClientRect(rect);

	int		height1 = pane_height;
	if(height1 < 0) {
		height1 = rect.Height() * 60 / 100;
	}
	int		height2 = rect.Height() - height1;

	m_wnd_splitter.SetRowInfo(0, height1, 0);
	m_wnd_splitter.SetRowInfo(1, height2, 0);

	m_wnd_splitter.RecalcLayout();
}

void CChildFrame::RestorePain1()
{
	// ショートカットキーで非表示にされていない場合、何もしない
	if(m_sql_editor_height == 0) return;

	// 下のpainの大きさを取得
	int		height;
	int		minheight;
	m_wnd_splitter.GetRowInfo(1, height, minheight);

	if(height == 0 && m_sql_editor_height > 0) {
		SetPaneHeight(m_sql_editor_height);
	}
}

void CChildFrame::OnSqlEditorMaximize() 
{
	CView *edit_view = (CView *)m_wnd_edit_splitter.GetPane(0, 0);
	SetActiveView(edit_view);
	edit_view->SetFocus();

	// 下のpainの大きさを取得
	int		height;
	int		minheight;
	m_wnd_splitter.GetRowInfo(1, height, minheight);

	// 各ペインの大きさを設定
	CRect	rect;
	GetClientRect(rect);

	if(height == 0) {
		if(m_sql_editor_height > 0) {
			SetPaneHeight(m_sql_editor_height);
		} else {
			SetPaneHeight(rect.Height() * 60 / 100);
		}
	} else {
		m_sql_editor_height = GetPaneHeight();
		SetPaneHeight(rect.Height());
	}
}

void CChildFrame::OnResultGridMaximize() 
{
	// 下のpainの大きさを取得
	int		height;
	int		minheight;
	m_wnd_splitter.GetRowInfo(0, height, minheight);

	// 各ペインの大きさを設定
	CRect	rect;
	GetClientRect(rect);

	if(height == 0) {
		CView *edit_view = (CView *)m_wnd_edit_splitter.GetPane(0, 0);
		SetActiveView(edit_view);
		edit_view->SetFocus();

		if(m_sql_editor_height > 0) {
			SetPaneHeight(m_sql_editor_height);
		} else {
			SetPaneHeight(rect.Height() * 60 / 100);
		}
	} else {
		SwitchView(GRID_VIEW);

		CGridView *grid_view = (CGridView *)m_wnd_grid_splitter.GetPane(0, 0);
		SetActiveView(grid_view);
		grid_view->SetFocus();

		m_sql_editor_height = GetPaneHeight();
		SetPaneHeight(0);
	}	
}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	// TODO: ここにメッセージ ハンドラー コードを追加します。
	CView* edit_view = (CView*)m_wnd_edit_splitter.GetPane(0, 0);
	INT_PTR act = bActivate;
	edit_view->GetDocument()->UpdateAllViews(NULL, UPD_MDI_ACTIVATE, (CObject*)act);
}
