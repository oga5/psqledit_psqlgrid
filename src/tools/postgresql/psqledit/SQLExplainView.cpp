/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // SQLExplainView.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"

#include "SQLExplainView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSQLExplainView

IMPLEMENT_DYNCREATE(CSQLExplainView, CView)

CSQLExplainView::CSQLExplainView()
{
}

CSQLExplainView::~CSQLExplainView()
{
}


BEGIN_MESSAGE_MAP(CSQLExplainView, CView)
	//{{AFX_MSG_MAP(CSQLExplainView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_SELECT_ALL, OnSelectAll)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_SEARCH, OnSearch)
	ON_COMMAND(ID_SEARCH_NEXT, OnSearchNext)
	ON_COMMAND(ID_SEARCH_PREV, OnSearchPrev)
	ON_COMMAND(ID_CLEAR_SEARCH_TEXT, OnClearSearchText)
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_CHAR_LEFT, OnCharLeft)
	ON_COMMAND(ID_CHAR_LEFT_EXTEND, OnCharLeftExtend)
	ON_COMMAND(ID_CHAR_RIGHT, OnCharRight)
	ON_COMMAND(ID_CHAR_RIGHT_EXTEND, OnCharRightExtend)
	ON_COMMAND(ID_LINE_UP, OnLineUp)
	ON_COMMAND(ID_LINE_UP_EXTEND, OnLineUpExtend)
	ON_COMMAND(ID_LINE_DOWN, OnLineDown)
	ON_COMMAND(ID_LINE_DOWN_EXTEND, OnLineDownExtend)
	ON_COMMAND(ID_WORD_LEFT, OnWordLeft)
	ON_COMMAND(ID_WORD_LEFT_EXTEND, OnWordLeftExtend)
	ON_COMMAND(ID_WORD_RIGHT, OnWordRight)
	ON_COMMAND(ID_WORD_RIGHT_EXTEND, OnWordRightExtend)
	ON_COMMAND(ID_LINE_START, OnLineStart)
	ON_COMMAND(ID_LINE_START_EXTEND, OnLineStartExtend)
	ON_COMMAND(ID_LINE_END, OnLineEnd)
	ON_COMMAND(ID_LINE_END_EXTEND, OnLineEndExtend)
	ON_COMMAND(ID_DOCUMENT_END, OnDocumentEnd)
	ON_COMMAND(ID_DOCUMENT_END_EXTEND, OnDocumentEndExtend)
	ON_COMMAND(ID_DOCUMENT_START, OnDocumentStart)
	ON_COMMAND(ID_DOCUMENT_START_EXTEND, OnDocumentStartExtend)
	ON_COMMAND(ID_SCROLL_UP, OnScrollUp)
	ON_COMMAND(ID_SCROLL_DOWN, OnScrollDown)
	ON_COMMAND(ID_SCROLL_PAGE_UP, OnScrollPageUp)
	ON_COMMAND(ID_SCROLL_PAGE_DOWN, OnScrollPageDown)
	ON_COMMAND(ID_PAGE_UP, OnPageUp)
	ON_COMMAND(ID_PAGE_DOWN, OnPageDown)
	ON_COMMAND(ID_PAGE_DOWN_EXTEND, OnPageDownExtend)
	ON_COMMAND(ID_PAGE_UP_EXTEND, OnPageUpExtend)
	ON_COMMAND(ID_SELECT_ROW, OnSelectRow)
	ON_COMMAND(ID_BOX_SELECT, OnBoxSelect)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRID_COPY_TAB, ID_GRID_COPY_FIX_LEN_CNAME, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_INDENT, ID_EDIT_INDENT_REV, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TO_LOWER, ID_TO_UPPER, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_REVERSE_ROWS, ID_EDIT_SORT_ROWS_DESC, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SET_BOOK_MARK, ID_SET_SQL_BOOKMARK, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CONVERT_TO_C, ID_CONVERT_FROM_VB, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CSQLExplainView::OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_CUT, &CSQLExplainView::OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CSQLExplainView::OnUpdateEditCut)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSQLExplainView 描画

void CSQLExplainView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSQLExplainView 診断

#ifdef _DEBUG
void CSQLExplainView::AssertValid() const
{
	CView::AssertValid();
}

void CSQLExplainView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSQLExplainView メッセージ ハンドラ

int CSQLExplainView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetDocument()->GetMsg()->set_tabstop(g_option.text_editor.tabstop);

	m_edit_ctrl.SetEditData(GetDocument()->GetMsg());
	m_edit_ctrl.SetReadOnly(TRUE);
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD, 
		CRect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy), this, NULL);

	m_edit_ctrl.SetHWheelScrollEnable(TRUE);
	m_edit_ctrl.SetFont(&g_font);
	m_edit_ctrl.SetCaret(TRUE, 1);

	SetEditorOption();
	m_edit_ctrl.SetLineMode(g_option.text_editor.line_mode, g_option.text_editor.line_len);	

	return 0;
}

void CSQLExplainView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	m_edit_ctrl.MoveWindow(0, 0, cx, cy);
}

void CSQLExplainView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	m_edit_ctrl.SetFocus();
}

void CSQLExplainView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint) {
	case UPD_FONT:
		m_edit_ctrl.SetFont((CFont *)pHint);
		break;
	case UPD_REDRAW:
		m_edit_ctrl.Redraw();
		break;
	case UPD_SWITCH:
		m_edit_ctrl.Redraw();
		if(CSearchDlgSingleton::IsVisible() && CSearchDlgSingleton::GetDlg().GetWnd() == this) {
			CSearchDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
		}
		break;
	case UPD_CLEAR_RESULT:
		m_edit_ctrl.ClearAll();
		break;
	case UPD_PUT_RESULT:
		PutResult((TCHAR *)pHint);
		break;
	case UPD_PUT_STATUS_BAR_MSG:
		PutStatusBarMsg((TCHAR *)pHint);
		break;
	case UPD_PUT_SEPALATOR:
		PutSepalator();
		break;
	case UPD_SET_RESULT_CARET:
		MoveCaretEndData();
		break;
	case UPD_CHANGE_EDITOR_OPTION:
		SetEditorOption();
		break;
	case UPD_CHANGE_LINE_MODE:
		m_edit_ctrl.SetLineMode(g_option.text_editor.line_mode, g_option.text_editor.line_len);
		break;
	}
}

void CSQLExplainView::SetEditorOption()
{
	QWORD		option = ECS_DISABLE_KEY_DOWN | ECS_BRACKET_MULTI_COLOR_ENABLE;

	if(GetDocument()->GetSqlData()->get_tabstop() != g_option.text_editor.tabstop) {
		GetDocument()->GetSqlData()->set_tabstop(g_option.text_editor.tabstop);
	}

	if(m_edit_ctrl.GetRowSpace() != g_option.text_editor.row_space ||
		m_edit_ctrl.GetCharSpaceSetting() != g_option.text_editor.char_space ||
		m_edit_ctrl.GetTopSpace() != g_option.text_editor.top_space ||
		m_edit_ctrl.GetLeftSpace() != g_option.text_editor.left_space) {
		m_edit_ctrl.SetSpaces(g_option.text_editor.row_space, 
			g_option.text_editor.char_space,
			g_option.text_editor.top_space,
			g_option.text_editor.left_space);
	}

	if(g_option.text_editor.show_line_feed) {
		option |= ECS_SHOW_LINE_FEED;
	}
	if(g_option.text_editor.show_tab) {
		option |= ECS_SHOW_TAB;
	}
	if(g_option.text_editor.show_space) {
		option |= ECS_SHOW_SPACE;
	}
	if(g_option.text_editor.show_2byte_space) {
		option |= ECS_SHOW_2BYTE_SPACE;
	}
	if(g_option.search_loop_msg) {
		option |= ECS_SEARCH_LOOP_MSG;
	}
	if(g_option.text_editor.ime_caret_color) {
		option |= ECS_IME_CARET_COLOR;
	}
	if(g_option.text_editor.show_brackets_bold) {
		option |= ECS_SHOW_BRACKETS_BOLD;
	}

	m_edit_ctrl.SetExStyle2(option);

	for(int i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		m_edit_ctrl.SetColor(i, g_option.text_editor.color[i]);
	}
	
	m_edit_ctrl.Redraw();
}

void CSQLExplainView::OnEditCopy() 
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.CopyFromKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.CopyFromKeyword();
		return;
	}
	m_edit_ctrl.Copy();
}

void CSQLExplainView::PutResult(TCHAR * msg)
{
	m_edit_ctrl.Paste(msg);

	if(GetDocument()->GetMsg()->get_row_cnt() > GetDocument()->GetMsgMaxRow() + 100) {
		POINT pt1 = {0, 0};
		POINT pt2 = {0, GetDocument()->GetMsg()->get_row_cnt() - GetDocument()->GetMsgMaxRow()};
		GetDocument()->GetMsg()->del(&pt1, &pt2, FALSE);
		m_edit_ctrl.Redraw();
	}

	MoveCaretEndData();
}

void CSQLExplainView::PutStatusBarMsg(TCHAR * msg)
{
	GetParentFrame()->SetMessageText(msg);
}

void CSQLExplainView::PutSepalator()
{
	TCHAR	sepa[255];

	if(GetDocument()->GetMsg()->get_cur_col() != 0) {
		PutResult(_T("\n"));
	}

	_stprintf(sepa, _T("%.*s\n"),
		min(80, g_option.text_editor.line_len),
		_T("------------------------------------------------------------------------------------------------"));
	
	PutResult(sepa);
}

void CSQLExplainView::MoveCaretEndData()
{
	int		row, col;

	row = GetDocument()->GetMsg()->get_row_cnt() - 1;
	col = GetDocument()->GetMsg()->get_row_len(row);
	if(col < 0) col = 0;

	m_edit_ctrl.ClearSelected();
	GetDocument()->GetMsg()->set_cur(row, col);
	m_edit_ctrl.SetCaret(TRUE, 1);
}

void CSQLExplainView::OnSelectAll() 
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.SelectAllKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.SelectAllKeyword();
		return;
	}
	m_edit_ctrl.SelectAll();
}

void CSQLExplainView::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CView::OnShowWindow(bShow, nStatus);
	
	if(bShow == TRUE) m_edit_ctrl.SetCaret(TRUE, 1);
}

LRESULT CSQLExplainView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case EC_WM_SETFOCUS:
		((CMainFrame *)AfxGetApp()->m_pMainWnd)->SetAccelerator(ACCEL_KIND_DOC);
		break;
	case EC_WM_KILLFOCUS:
		((CMainFrame *)AfxGetApp()->m_pMainWnd)->SetAccelerator(ACCEL_KIND_FRAME);
		break;
	case EC_WM_LBUTTONDBLCLK:
		if(lParam < 0 || lParam >= GetDocument()->GetMsg()->get_row_cnt()) break;

		TCHAR *p1;

		p1 = GetDocument()->GetMsg()->get_row_buf((int)lParam);
		if(_tcsncmp(p1, _T("[row:"), 5) == 0) {
			_stprintf(g_msg_buf, _T("%.50s"), p1);
			GetDocument()->UpdateAllViews(this, UPD_SET_ERROR_INFO);
		}
		break;
	case EC_WM_CHANGE_CARET_POS:
		g_cur_pos.x = (LONG)wParam + 1;
		g_cur_pos.y = (LONG)lParam + 1;
		break;
	case WM_SEARCH_TEXT:
		{
			BOOL b_looped;
			int ret_v = m_edit_ctrl.SearchText2(g_search_data.m_search_text,
				g_search_data.m_dir, g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, &b_looped);
			SearchMsg(ret_v, g_search_data.m_dir, b_looped);
		}
		break;
	}
	
	return CView::WindowProc(message, wParam, lParam);
}

void CSQLExplainView::OnSearch() 
{

	if(m_edit_ctrl.HaveSelected() && m_edit_ctrl.HaveSelectedMultiLine() == FALSE) {
		g_search_data.m_search_text = m_edit_ctrl.GetSelectedText();
	}

//	CSearchDlg		dlg;
//	dlg.DoModal(this, WM_SEARCH_TEXT, &g_search_data, FALSE);
	GetDocument()->HideSearchDlgs();
	CSearchDlgSingleton::GetDlg().m_title = _T("検索(Result)");
	CSearchDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, &g_search_data, FALSE);
}

void CSQLExplainView::OnSearchNext() 
{
	BOOL b_looped;
	int ret_v = m_edit_ctrl.SearchNext2(g_search_data.m_search_text,
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, 1, b_looped);
}

void CSQLExplainView::OnSearchPrev() 
{
	BOOL b_looped;
	int ret_v = m_edit_ctrl.SearchPrev2(g_search_data.m_search_text,
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, -1, b_looped);
}

void CSQLExplainView::OnUpdateDisableMenu(CCmdUI* pCmdUI) 
{
	if(pCmdUI->m_pSubMenu != NULL) {
        // ポップアップ メニューを選択不可にする
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		return;
	}
	pCmdUI->Enable(FALSE);
}

void CSQLExplainView::OnClearSearchText() 
{
	m_edit_ctrl.ClearSearchText();	
}

void CSQLExplainView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	menu.LoadMenu(IDR_RESULT_MENU);

	// CMainFrameを親ウィンドウにすると、メニューの有効無効を、メインメニューと同じにできる
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		GetParentFrame()->GetParentFrame());

	// メニュー削除
	menu.DestroyMenu();	
}


void CSQLExplainView::OnCharLeft() { m_edit_ctrl.CharLeft(FALSE); }
void CSQLExplainView::OnCharLeftExtend() { m_edit_ctrl.CharLeft(TRUE); }
void CSQLExplainView::OnCharRight() { m_edit_ctrl.CharRight(FALSE); }
void CSQLExplainView::OnCharRightExtend() { m_edit_ctrl.CharRight(TRUE); }
void CSQLExplainView::OnLineDown() { m_edit_ctrl.LineDown(FALSE); }
void CSQLExplainView::OnLineDownExtend() { m_edit_ctrl.LineDown(TRUE); }
void CSQLExplainView::OnLineUp() { m_edit_ctrl.LineUp(FALSE); }
void CSQLExplainView::OnLineUpExtend() { m_edit_ctrl.LineUp(TRUE); }

void CSQLExplainView::OnWordLeft() { m_edit_ctrl.WordLeft(FALSE); }
void CSQLExplainView::OnWordLeftExtend() { m_edit_ctrl.WordLeft(TRUE); }
void CSQLExplainView::OnWordRight() { m_edit_ctrl.WordRight(FALSE); }
void CSQLExplainView::OnWordRightExtend() { m_edit_ctrl.WordRight(TRUE); }

void CSQLExplainView::OnLineStart() { m_edit_ctrl.LineStart(FALSE); }
void CSQLExplainView::OnLineStartExtend() { m_edit_ctrl.LineStart(TRUE); }
void CSQLExplainView::OnLineEnd() { m_edit_ctrl.LineEnd(FALSE); }
void CSQLExplainView::OnLineEndExtend() { m_edit_ctrl.LineEnd(TRUE); }

void CSQLExplainView::OnDocumentStart() { m_edit_ctrl.DocumentStart(FALSE); }
void CSQLExplainView::OnDocumentStartExtend() { m_edit_ctrl.DocumentStart(TRUE); }
void CSQLExplainView::OnDocumentEnd() { m_edit_ctrl.DocumentEnd(FALSE); }
void CSQLExplainView::OnDocumentEndExtend() { m_edit_ctrl.DocumentEnd(TRUE); }

void CSQLExplainView::OnScrollUp() { m_edit_ctrl.ScrollUp(); }
void CSQLExplainView::OnScrollDown() { m_edit_ctrl.ScrollDown(); }
void CSQLExplainView::OnScrollPageUp() { m_edit_ctrl.ScrollPageUp(); }
void CSQLExplainView::OnScrollPageDown() { m_edit_ctrl.ScrollPageDown(); }

void CSQLExplainView::OnPageUp() { m_edit_ctrl.PageUp(FALSE); }
void CSQLExplainView::OnPageUpExtend() { m_edit_ctrl.PageUp(TRUE); }
void CSQLExplainView::OnPageDown() { m_edit_ctrl.PageDown(FALSE); }
void CSQLExplainView::OnPageDownExtend() { m_edit_ctrl.PageDown(TRUE); }

void CSQLExplainView::OnSelectRow() 
{
	m_edit_ctrl.SelectRow(GetDocument()->GetMsg()->get_cur_row());
}

void CSQLExplainView::OnBoxSelect() { m_edit_ctrl.NextBoxSelect(); }

void CSQLExplainView::SearchMsg(int ret_v, int dir, BOOL b_looped)
{
	GetParentFrame()->SetMessageText(MakeSearchMsg(ret_v, dir, b_looped));
}

void CSQLExplainView::OnEditPaste() 
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.PasteToKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.PasteToKeyword();
		return;
	}
}

void CSQLExplainView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	if(g_object_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	pCmdUI->Enable(FALSE);
}


void CSQLExplainView::OnEditCut()
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.CutKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.CutKeyword();
		return;
	}
}

void CSQLExplainView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	if(g_object_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	pCmdUI->Enable(FALSE);
}


BOOL CSQLExplainView::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
//	return CView::OnEraseBkgnd(pDC);
}
