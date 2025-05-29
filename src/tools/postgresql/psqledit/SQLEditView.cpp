/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // SQLEditView.cpp : インプリメンテーション ファイル
//


#include "stdafx.h"
#include "psqledit.h"

#include "SQLEditView.h"
#include "ChildFrm.h"
#include "MainFrm.h"

#include "mbutil.h"

#include "printdlg.h"
#include "printeditdata.h"
#include "linejumpdlg.h"

#include "lintsql.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LEFT_SPACE 0

#define ROW_DATA_BREAK		ROW_DATA_USER_DEF1

/////////////////////////////////////////////////////////////////////////////
// CSQLEditView

IMPLEMENT_DYNCREATE(CSQLEditView, CView)

CSQLEditView::CSQLEditView()
{
}

CSQLEditView::~CSQLEditView()
{
	CPrintDlg::DeleteBackupData();
}


BEGIN_MESSAGE_MAP(CSQLEditView, CView)
	//{{AFX_MSG_MAP(CSQLEditView)
	ON_WM_CREATE()
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_SELECT_ALL, OnSelectAll)
	ON_COMMAND(ID_SEARCH_NEXT, OnSearchNext)
	ON_COMMAND(ID_SEARCH_PREV, OnSearchPrev)
	ON_COMMAND(ID_SEARCH, OnSearch)
	ON_COMMAND(ID_REPLACE, OnReplace)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_TO_LOWER, OnToLower)
	ON_COMMAND(ID_TO_UPPER, OnToUpper)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_JUMP_BOOK_MARK, OnJumpBookMark)
	ON_COMMAND(ID_SET_BOOK_MARK, OnSetBookMark)
	ON_COMMAND(ID_JUMP_BOOK_MARK_BACK, OnJumpBookMarkBack)
	ON_COMMAND(ID_CONVERT_TO_C, OnConvertToC)
	ON_COMMAND(ID_CONVERT_TO_HTML, OnConvertToHtml)
	ON_COMMAND(ID_CONVERT_TO_JAVA, OnConvertToJava)
	ON_COMMAND(ID_CONVERT_TO_PERL, OnConvertToPerl)
	ON_COMMAND(ID_CONVERT_FROM_C, OnConvertFromC)
	ON_COMMAND(ID_CONVERT_FROM_HTML, OnConvertFromHtml)
	ON_COMMAND(ID_CONVERT_FROM_JAVA, OnConvertFromJava)
	ON_COMMAND(ID_CONVERT_FROM_PERL, OnConvertFromPerl)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_CONVERT_TO_VB, OnConvertToVb)
	ON_COMMAND(ID_CONVERT_FROM_VB, OnConvertFromVb)
	ON_COMMAND(ID_CLEAR_ALL_BOOK_MARK, OnClearAllBookMark)
	ON_COMMAND(ID_EDIT_REVERSE_ROWS, OnEditReverseRows)
	ON_COMMAND(ID_EDIT_SORT_ROWS_ASC, OnEditSortRowsAsc)
	ON_COMMAND(ID_EDIT_SORT_ROWS_DESC, OnEditSortRowsDesc)
	ON_COMMAND(ID_SET_SQL_BOOKMARK, OnSetSqlBookmark)
	ON_COMMAND(ID_CLEAR_SEARCH_TEXT, OnClearSearchText)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_INSERT_COMMENT, OnInsertComment)
	ON_COMMAND(ID_DELETE_COMMENT, OnDeleteComment)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_INDENT, OnEditIndent)
	ON_COMMAND(ID_EDIT_INDENT_REV, OnEditIndentRev)
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
	ON_COMMAND(ID_EDIT_BACKSPACE, OnEditBackspace)
	ON_COMMAND(ID_KEYWORD_COMPLETION, OnKeywordCompletion)
	ON_COMMAND(ID_KEYWORD_COMPLETION_REV, OnKeywordCompletionRev)
	ON_COMMAND(ID_SCROLL_UP, OnScrollUp)
	ON_COMMAND(ID_SCROLL_DOWN, OnScrollDown)
	ON_COMMAND(ID_SCROLL_PAGE_UP, OnScrollPageUp)
	ON_COMMAND(ID_SCROLL_PAGE_DOWN, OnScrollPageDown)
	ON_COMMAND(ID_PAGE_UP, OnPageUp)
	ON_COMMAND(ID_PAGE_UP_EXTEND, OnPageUpExtend)
	ON_COMMAND(ID_PAGE_DOWN, OnPageDown)
	ON_COMMAND(ID_PAGE_DOWN_EXTEND, OnPageDownExtend)
	ON_COMMAND(ID_SELECT_ROW, OnSelectRow)
	ON_COMMAND(ID_BOX_SELECT, OnBoxSelect)
	ON_COMMAND(ID_TOGGLE_OVERWRITE, OnToggleOverwrite)
	ON_COMMAND(ID_TAB_TO_SPACE, OnTabToSpace)
	ON_COMMAND(ID_SPACE_TO_TAB, OnSpaceToTab)
	ON_COMMAND(ID_JUMP_BOOK_MARK_EXTEND, OnJumpBookMarkExtend)
	ON_COMMAND(ID_JUMP_BOOK_MARK_BACK_EXTEND, OnJumpBookMarkBackExtend)
	ON_COMMAND(ID_LINE_JUMP, OnLineJump)
	ON_COMMAND(ID_EDIT_DELETE_ROW, OnEditDeleteRow)
	ON_COMMAND(ID_EDIT_DELETE_AFTER_CURSOR, OnEditDeleteAfterCursor)
	ON_COMMAND(ID_LAST_EDIT_POS, OnLastEditPos)
	ON_COMMAND(ID_EDIT_COPY_ROW, OnEditCopyRow)
	ON_COMMAND(ID_EDIT_CUT_ROW, OnEditCutRow)
	ON_COMMAND(ID_EDIT_PASTE_ROW, OnEditPasteRow)
	ON_COMMAND(ID_EDIT_PASTE_ROW_UP, OnEditPasteRowUp)
	ON_COMMAND(ID_SPLIT_START, OnSplitStart)
	ON_COMMAND(ID_SPLIT_START_EXTEND, OnSplitStartExtend)
	ON_COMMAND(ID_SPLIT_END, OnSplitEnd)
	ON_COMMAND(ID_SPLIT_END_EXTEND, OnSplitEndExtend)
	ON_COMMAND(ID_SELECT_WORD, OnSelectWord)
	ON_COMMAND(ID_EDIT_JOIN_ROW, OnEditJoinRow)
	ON_COMMAND(ID_SEARCH_SELECTED, OnSearchSelected)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_SELECTED, OnUpdateSearchSelected)
	ON_COMMAND(ID_CODE_ASSIST_ON, OnCodeAssistOn)
	ON_COMMAND(ID_SQL_LINT, OnSqlLint)
	ON_UPDATE_COMMAND_UI(ID_SQL_LINT, OnUpdateSqlLint)
	ON_COMMAND(ID_CONVERT_TO_C_UNICODE, OnConvertToCUnicode)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRID_COPY_TAB, ID_GRID_COPY_FIX_LEN_CNAME, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TO_LOWER, ID_TO_UPPER, OnUpdateLowerUpper)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_REVERSE_ROWS, ID_EDIT_SORT_ROWS_DESC, OnUpdateMultiLineEdit)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SET_BOOK_MARK, ID_SET_SQL_BOOKMARK, OnUpdateEnableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CONVERT_TO_C, ID_CONVERT_FROM_VB, OnUpdateConvertMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CONVERT_TO_PYTHON, ID_CONVERT_FROM_JAVASCRIPT, OnUpdateConvertMenu)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_OVR, OnUpdateOverwrite)
		ON_WM_HSCROLL()
		ON_WM_VSCROLL()
		ON_COMMAND(ID_CONVERT_FROM_PYTHON, &CSQLEditView::OnConvertFromPython)
		ON_COMMAND(ID_CONVERT_TO_PYTHON, &CSQLEditView::OnConvertToPython)
		ON_COMMAND(ID_CONVERT_FROM_JAVASCRIPT, &CSQLEditView::OnConvertFromJavascript)
		ON_COMMAND(ID_CONVERT_TO_JAVASCRIPT, &CSQLEditView::OnConvertToJavascript)
		ON_COMMAND(ID_TO_LOWER_WITHOUT_COMMENT_LITERAL, &CSQLEditView::OnToLowerWithoutCommentLiteral)
		ON_COMMAND(ID_TO_UPPER_WITHOUT_COMMENT_LITERAL, &CSQLEditView::OnToUpperWithoutCommentLiteral)
		END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSQLEditView 描画

void CSQLEditView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: この位置に描画用のコードを追加してください
	CRect	rect, winrect;
	CBrush	break_point_brush(RGB(200, 0, 0));
	CBrush	book_mark_brush(RGB(170, 170, 250));
	CBrush	*old_brush;
	CPoint	pt(2, 2);

	GetClientRect(winrect);

	rect.top = 0;
	rect.bottom = winrect.bottom;
	rect.left = 0;
	rect.right = LEFT_SPACE;
	pDC->FillSolidRect(rect, RGB(230, 230, 255));

	int loop_cnt, scr_y, row_height, i;

	scr_y = m_edit_ctrl.GetScrollPos(SB_VERT);
	loop_cnt = m_edit_ctrl.GetShowRow() + scr_y;
	if(loop_cnt >= GetDocument()->GetSqlData()->get_row_cnt()) {
		loop_cnt = GetDocument()->GetSqlData()->get_row_cnt();
	}
	row_height = m_edit_ctrl.GetRowHeight();

	for(i = scr_y; i < loop_cnt; i++) {
		if(GetDocument()->GetSqlData()->get_row_data_flg(i, ROW_DATA_BREAK)) {
			rect.left = 1;
			rect.right = LEFT_SPACE - 1;
			rect.top = (i - scr_y) * row_height + 2 + m_edit_ctrl.GetScrollTopMargin();
			rect.bottom = rect.top + row_height - 2;

			old_brush = pDC->SelectObject(&break_point_brush);
			pDC->RoundRect(&rect, pt);
			pDC->SelectObject(old_brush);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSQLEditView 診断

#ifdef _DEBUG
void CSQLEditView::AssertValid() const
{
	CView::AssertValid();
}

void CSQLEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSQLEditView メッセージ ハンドラ

int CSQLEditView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if(CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_edit_ctrl.SetSplitterMode(TRUE);
	m_edit_ctrl.SetEditData(GetDocument()->GetSqlData());
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD, 
		CRect(LEFT_SPACE, 0, lpCreateStruct->cx, lpCreateStruct->cy), this, NULL);

	m_edit_ctrl.SetHWheelScrollEnable(TRUE);
	m_edit_ctrl.SetFont(&g_font);
	//if(m_droptarget.Register(this) == -1) return -1;

	SetEditorOption();
	m_edit_ctrl.SetLineMode(g_option.text_editor.line_mode, g_option.text_editor.line_len);	

	return 0;
}

void CSQLEditView::OnEditPaste() 
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.PasteToKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.PasteToKeyword();
		return;
	}
	m_edit_ctrl.Paste();
}

void CSQLEditView::OnEditCut() 
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.CutKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.CutKeyword();
		return;
	}
	m_edit_ctrl.Cut();
}

void CSQLEditView::OnEditCopy() 
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

void CSQLEditView::OnEditUndo() 
{
	m_edit_ctrl.Undo();
}

void CSQLEditView::OnEditRedo() 
{
	m_edit_ctrl.Redo();
}

void CSQLEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint) {
	case UPD_FONT:
		m_edit_ctrl.SetFont((CFont *)pHint);
		break;
	case UPD_CLEAR_SELECTED:
		m_edit_ctrl.ClearSelected();
		break;
	case UPD_REDRAW:
		m_edit_ctrl.Redraw();
		break;
	case UPD_INVALIDATE:
		m_edit_ctrl.Invalidate();
		break;
	case UPD_PASTE_SQL:
		if(IsPrevActiveView()) {
			if(GetDocument()->GetSqlData()->get_cur_col() != 0) {
				m_edit_ctrl.Paste(_T("\n"));
			}
			m_edit_ctrl.Paste((TCHAR *)pHint);
			SetFocus();
		}
		break;
	case UPD_PASTE_SQL2:
		if(IsPrevActiveView()) {
			m_edit_ctrl.Paste((TCHAR *)pHint);
			SetFocus();
			break;
		}
	case UPD_PASTE_OBJECT_NAME:
		if(IsPrevActiveView()) {
			PasteObjectName((TCHAR *)pHint);
		}
		break;
	case UPD_SET_ERROR_INFO:
		if(IsPrevActiveView()) {
			INT_PTR v = (INT_PTR)pHint;
			SetErrorInfo((int)v);
		}
		break;
	case UPD_CHANGE_EDITOR_OPTION:
		SetEditorOption();
		break;
	case UPD_CHANGE_LINE_MODE:
		m_edit_ctrl.SetLineMode(g_option.text_editor.line_mode, g_option.text_editor.line_len);
		break;
	case UPD_SWITCH_VIEW:
		{
			INT_PTR v = (INT_PTR)pHint;
			if (IsPrevActiveView()) ((CChildFrame *)GetParentFrame())->SwitchView((int)v);
		}
		break;
	case UPD_DELETE_PANE:
		SetEditorOption();
		m_edit_ctrl.Redraw();
		GetDocument()->SetSplitEditorPrevActivePane(0);
		break;
	case UPD_MDI_ACTIVATE:
	{
		INT_PTR v = (INT_PTR)pHint;
		BOOL bActivate = (BOOL)v;
		if(!bActivate) {
			// タブが切り替わるとき、検索ダイアログを閉じる
			GetDocument()->HideSearchDlgs();
		}
	}
	break;
	}
}

void CSQLEditView::SetEditorOption()
{
	QWORD		option = 0;

	option = ECS_INVERT_BRACKETS | ECS_DISABLE_KEY_DOWN | ECS_BRACKET_MULTI_COLOR_ENABLE;

	if(g_option.text_editor.auto_indent) {
		GetDocument()->GetSqlData()->set_indent_mode(INDENT_MODE_AUTO);
	} else {
		GetDocument()->GetSqlData()->set_indent_mode(INDENT_MODE_NONE);
	}
	GetDocument()->GetSqlData()->set_tab_as_space(g_option.text_editor.tab_as_space);

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
	if(g_option.text_editor.show_row_num) {
		option |= ECS_SHOW_ROW_NUM;
	}
	if(g_option.text_editor.show_col_num) {
		if(IsPrimaryView()) option |= ECS_SHOW_COL_NUM;
	}
	if(g_option.text_editor.show_space) {
		option |= ECS_SHOW_SPACE;
	}
	if(g_option.text_editor.show_2byte_space) {
		option |= ECS_SHOW_2BYTE_SPACE;
	}
	if(g_option.text_editor.show_row_line) {
		option |= ECS_SHOW_ROW_LINE;
	}
	if(g_option.text_editor.show_edit_row) {
		option |= ECS_SHOW_EDIT_ROW;
	}
	if(g_option.text_editor.show_brackets_bold) {
		option |= ECS_SHOW_BRACKETS_BOLD;
	}
	if(g_option.text_editor.drag_drop_edit) {
		option |= ECS_DRAG_DROP_EDIT;
	}
	if(g_option.text_editor.row_copy_at_no_sel) {
		option |= ECS_ROW_COPY_AT_NO_SEL;
	}
	if(g_option.search_loop_msg) {
		option |= ECS_SEARCH_LOOP_MSG;
	}
	if(g_option.text_editor.ime_caret_color) {
		option |= ECS_IME_CARET_COLOR;
	}

	m_edit_ctrl.SetExStyle2(option);
	if(g_option.code_assist.sort_column_key == CODE_ASSIST_SORT_COLUMN_NAME) {
		CSqlListMaker::SetSortColumns(TRUE);
	} else {
		CSqlListMaker::SetSortColumns(FALSE);
	}

	int i;
	for(i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		m_edit_ctrl.SetColor(i, g_option.text_editor.color[i]);
	}
	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		m_edit_ctrl.SetCodeAssistWndColor(i, g_option.grid.color[i]);
	}
	m_edit_ctrl.SetCodeAssistWndColor(GRID_BG_COLOR, g_option.text_editor.color[BG_COLOR]);
	m_edit_ctrl.SetCodeAssistWndColor(GRID_TEXT_COLOR, g_option.text_editor.color[TEXT_COLOR]);
	m_edit_ctrl.SetCodeAssistWndColor(GRID_SELECT_COLOR, g_option.text_editor.color[SELECTED_COLOR]);
	m_edit_ctrl.SetCodeAssistWnd_InvertSelectText(TRUE);

	m_edit_ctrl.SetPasteLower(g_option.text_editor.copy_lower_name);
	m_edit_ctrl.SetUseKeywordWindow(g_option.code_assist.use_keyword_window);
	m_edit_ctrl.SetEnableCodeAssist(g_option.code_assist.enable_code_assist);

	m_edit_ctrl.Redraw();
}

void CSQLEditView::ErrorHighlight(CPoint pt)
{
	CPoint			pt1, pt2;

	if(pt.y < 0 || pt.y >= GetDocument()->GetSqlData()->get_row_cnt()) return;

/*
	unsigned char	*p;
	int				len;
	// 実装１
	pt1 = pt;

	p = GetDocument()->GetSqlData()->get_row_buf(pt.y);
	if(p == NULL) return;
	len = strlen((char *)p);
	if(strlen((char *)p) <= (unsigned int)pt.x) return;
	p = p + pt.x;

	for(;; p++) {
		if(is_lead_byte(*p)) {
			if(g_sql_str_token.isBreakChar((((*p) & 0xff) << 8) + ((*(p+1)) & 0xff)) != 0) break;
			(pt.x)++;
			p++;
			if(*p == '\0') break;
		} else {
			if(g_sql_str_token.isBreakChar(*p) != 0) break;
		}
		(pt.x)++;
	}
	pt2 = pt;
	m_edit_ctrl.SetSelectedPoint(pt1, pt2);
*/
// 実装２ FIXME: こっちの実装のほうがいいかも
	GetDocument()->GetSqlData()->set_cur(pt.y, pt.x);
	pt1.x = GetDocument()->GetSqlData()->get_cur_col();
	pt1.y = GetDocument()->GetSqlData()->get_cur_row();

	GetDocument()->GetSqlData()->move_word(1);
	pt2.x = GetDocument()->GetSqlData()->get_cur_col();
	pt2.y = GetDocument()->GetSqlData()->get_cur_row();
	m_edit_ctrl.SetSelectedPoint(pt1, pt2);
}

void CSQLEditView::SetErrorPos(int row_offset)
{
	CPoint	pt;
	int		row;
	int		col;

	row = row_offset;
	col = 0;
	
	if(row < 0 || col < 0) return;

	pt.y = row;
	pt.x = col;

	ErrorHighlight(pt);
}

void CSQLEditView::OnSelectAll() 
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

void CSQLEditView::SetErrorInfo(int row_offset)
{
	((CChildFrame *)GetParentFrame())->SwitchView(MSG_VIEW);
	SetErrorPos(row_offset);
	GetParentFrame()->SetActiveView(this);
}

void CSQLEditView::OnSearch() 
{

	if(m_edit_ctrl.HaveSelected() && m_edit_ctrl.HaveSelectedMultiLine() == FALSE) {
		g_search_data.m_search_text = m_edit_ctrl.GetSelectedText();
	}

//	CSearchDlg		dlg;
//	dlg.DoModal(this, WM_SEARCH_TEXT, &g_search_data, FALSE);
	GetDocument()->HideSearchDlgs();
	CSearchDlgSingleton::GetDlg().m_title = _T("検索(Editor)");
	CSearchDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, &g_search_data, FALSE);
}

void CSQLEditView::SearchString(const TCHAR* search_text)
{
	if(search_text != NULL && *search_text == '\0') return;

	g_search_data.m_dir = 1;
	g_search_data.m_distinct_lwr_upr = FALSE;
	g_search_data.m_distinct_width_ascii = TRUE;
	g_search_data.m_regexp = FALSE;
	g_search_data.m_search_text = search_text;

	OnSearchNext();
}

void CSQLEditView::OnSearchSelected()
{
	SearchString(m_edit_ctrl.GetSelectedText());
}

void CSQLEditView::OnUpdateSearchSelected(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelected());
}

void CSQLEditView::OnSearchNext() 
{
	BOOL b_looped;
	int ret_v = m_edit_ctrl.SearchNext2(g_search_data.m_search_text.GetBuffer(0),
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, 1, b_looped);
}

void CSQLEditView::OnSearchPrev() 
{
	BOOL b_looped;
	int ret_v = m_edit_ctrl.SearchPrev2(g_search_data.m_search_text.GetBuffer(0),
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, -1, b_looped);
}

void CSQLEditView::OnReplace() 
{

	if(m_edit_ctrl.HaveSelected() && m_edit_ctrl.HaveSelectedMultiLine() == FALSE) {
		g_search_data.m_search_text = m_edit_ctrl.GetSelectedText();
	}

	g_search_data.m_replace_selected_area = m_edit_ctrl.HaveSelectedMultiLine();
	BOOL disable_selected_area = !m_edit_ctrl.HaveSelected();

//	CReplaceDlg		dlg;
//	dlg.DoModal(this, WM_SEARCH_TEXT, WM_REPLACE_TEXT, disable_selected_area, &g_search_data);
	GetDocument()->HideSearchDlgs();
	CReplaceDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, WM_REPLACE_TEXT, &g_search_data);
}

void CSQLEditView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	m_edit_ctrl.MoveWindow(LEFT_SPACE, 0, cx - LEFT_SPACE, cy);
}

BOOL CSQLEditView::IsPrimaryView()
{
	if(GetParent()->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) {
		CSplitterWnd *sp = (CSplitterWnd *)GetParent();
		if(sp == NULL) return TRUE;

		// 分割されていないときは，TRUEを返す
		if(sp->GetRowCount() == 1) return TRUE;

		return (sp->GetPane(0, 0) == this);
	}

	return TRUE;
}

BOOL CSQLEditView::IsPrevActiveView()
{
	if(GetParent()->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) {
		CSplitterWnd *sp = (CSplitterWnd *)GetParent();
		if(sp == NULL) return TRUE;

		// 分割されていないときは，TRUEを返す
		if(sp->GetRowCount() == 1) return TRUE;

		return (sp->GetPane(GetDocument()->GetSplitEditorPrevActivePane(), 0) == this);
	}

	return TRUE;
}

void CSQLEditView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	m_edit_ctrl.SetFocus();
	int pane_row = 0;
	if(!IsPrimaryView()) pane_row = 1;
	GetDocument()->SetSplitEditorPrevActivePane(pane_row);
}

LRESULT CSQLEditView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case EC_WM_SCROLL:
		Invalidate();
		break;
	case EC_WM_INVALIDATED:
		Invalidate();
		break;
	case EC_WM_SETFOCUS:
		((CMainFrame *)AfxGetApp()->m_pMainWnd)->SetAccelerator(ACCEL_KIND_DOC);
		break;
	case EC_WM_KILLFOCUS:
		((CMainFrame *)AfxGetApp()->m_pMainWnd)->SetAccelerator(ACCEL_KIND_FRAME);
		break;
	case EC_WM_CHANGE_CARET_POS:
		g_cur_pos.x = (LONG)wParam + 1;
		g_cur_pos.y = (LONG)lParam + 1;
		GetDocument()->CheckModified();	// 編集されている場合，ウィンドウタイトルに'*'マークをつける
		break;
	case WM_SEARCH_TEXT:
		{
			BOOL b_looped;
			int ret_v = m_edit_ctrl.SearchText2(g_search_data.m_search_text,
				g_search_data.m_dir, g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, &b_looped);
			SearchMsg(ret_v, g_search_data.m_dir, b_looped);
			return ret_v;
		}
		break;
	case WM_REPLACE_TEXT:
		{
			int replace_cnt;
			BOOL b_looped;
			int ret_v = m_edit_ctrl.ReplaceText2(g_search_data.m_search_text,
				g_search_data.m_replace_text,
				g_search_data.m_dir, g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, g_search_data.m_all,
				g_search_data.m_replace_selected_area, &replace_cnt, &b_looped);
			GetDocument()->CheckModified();	// 編集されている場合，ウィンドウタイトルに'*'マークをつける

			CString msg;
			msg.Format(_T("%d件置換しました"), replace_cnt);
			GetParentFrame()->SetMessageText(msg);

			if(g_search_data.m_all) {
				CString msg;
				msg.Format(_T("%d件置換しました"), replace_cnt);
				GetParentFrame()->SetMessageText(msg);
			} else {
				SearchMsg(ret_v, g_search_data.m_dir, b_looped);
			}
		}
		break;
	}
	
	return CView::WindowProc(message, wParam, lParam);
}

void CSQLEditView::Invalidate()
{
	RECT rect;
	GetClientRect(&rect);
	rect.right = LEFT_SPACE;
	InvalidateRect(&rect);
}

void CSQLEditView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanUndo());
}

void CSQLEditView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.CanRedo());
}

void CSQLEditView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	if(g_object_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	pCmdUI->Enable(m_edit_ctrl.CanCut());
}

void CSQLEditView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	if(g_object_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	pCmdUI->Enable(m_edit_ctrl.CanCopy());
}

void CSQLEditView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	if(g_object_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	pCmdUI->Enable(m_edit_ctrl.CanPaste());
}

void CSQLEditView::OnToLower() 
{
	m_edit_ctrl.ToLower();
}

void CSQLEditView::OnToUpper() 
{
	m_edit_ctrl.ToUpper();
}

void CSQLEditView::OnUpdateLowerUpper(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelected());
}

void CSQLEditView::OnUpdateMultiLineEdit(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelectedMultiLine());
}

BOOL CSQLEditView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;

//	return CWnd::OnEraseBkgnd(pDC);
}

void CSQLEditView::OnSetBookMark() { m_edit_ctrl.SetBookMark(); }
void CSQLEditView::OnJumpBookMark() { m_edit_ctrl.JumpBookMark(FALSE, FALSE); }
void CSQLEditView::OnJumpBookMarkBack() { m_edit_ctrl.JumpBookMark(TRUE, FALSE); }
void CSQLEditView::OnJumpBookMarkExtend() { m_edit_ctrl.JumpBookMark(FALSE, TRUE); }
void CSQLEditView::OnJumpBookMarkBackExtend() { m_edit_ctrl.JumpBookMark(TRUE, TRUE);}

void CSQLEditView::ReplaceText(TCHAR *search_text, TCHAR *replace_text, BOOL b_regexp)
{
	m_edit_ctrl.ReplaceText(search_text, replace_text, 1, TRUE, 
		b_regexp, TRUE, TRUE, NULL, NULL);
}

void CSQLEditView::OnConvertToC() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("\\"), _T("\\\\"), FALSE);
	ReplaceText(_T("\""), _T("\\\""), FALSE);
	ReplaceText(_T("^(.+)$"), _T("\"$1 \\\\n\""), TRUE);
}

void CSQLEditView::OnConvertToCUnicode() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("\\"), _T("\\\\"), FALSE);
	ReplaceText(_T("\""), _T("\\\""), FALSE);
	ReplaceText(_T("^(.+)$"), _T("_T(\"$1 \\\\n\")"), TRUE);
}

void CSQLEditView::OnConvertToHtml() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("&"), _T("&amp"), FALSE);
	ReplaceText(_T("<"), _T("&lt"), FALSE);
	ReplaceText(_T(">"), _T("&gt"), FALSE);
	ReplaceText(_T("\""), _T("&quot"), FALSE);
}

void CSQLEditView::OnConvertToJava() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("\\"), _T("\\\\"), FALSE);
	ReplaceText(_T("\""), _T("\\\""), FALSE);
	ReplaceText(_T("^"), _T("\""), TRUE);
	ReplaceText(_T("$"), _T(" \\\\n\" +"), TRUE);
}

void CSQLEditView::OnConvertToPerl() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("\\"), _T("\\\\"), FALSE);
	ReplaceText(_T("\""), _T("\\\""), FALSE);
	ReplaceText(_T("^"), _T("\""), TRUE);
	ReplaceText(_T("$"), _T(" \\\\n\" ."), TRUE);
}

void CSQLEditView::OnConvertToVb() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CString	sql_stmt;
	sql_stmt.Format(_T("%s = %s & \""), g_option.sql_stmt_str.GetString(), g_option.sql_stmt_str.GetString());

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("\""), _T("\"\""), FALSE);
	ReplaceText(_T("^"), sql_stmt.GetBuffer(0), TRUE);
	ReplaceText(_T("$"), _T("\" & vbCrLf"), TRUE);

	// FIXME: その場しのぎの実装なので，変更する
	// 先頭行は，文字連結しない
	POINT	pt1, pt2, pt3;
	m_edit_ctrl.GetSelectedPoint(&pt1, &pt2);

	CString	sql_stmt_hat, sql_stmt_first;
	sql_stmt_hat.Format(_T("^%s"), sql_stmt.GetString());
	sql_stmt_first.Format(_T("%s = \""), g_option.sql_stmt_str.GetString());

	if(pt1.y != pt2.y) {
		pt3.y = pt1.y;
		pt3.x = GetDocument()->GetSqlData()->get_row_len(pt1.y);
		m_edit_ctrl.SetSelectedPoint(pt1, pt3);
		ReplaceText(sql_stmt_hat.GetBuffer(0), 
			sql_stmt_first.GetBuffer(0), TRUE);
		m_edit_ctrl.SetSelectedPoint(pt1, pt2);
	} else {
		ReplaceText(sql_stmt_hat.GetBuffer(0), 
			sql_stmt_first.GetBuffer(0), TRUE);
	}
}

void CSQLEditView::OnConvertFromC() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("^\\s*(?:_T\\()?\\\"(?:\\\\n)?"), _T(""), TRUE);
	ReplaceText(_T("\\s*(\\\\n)?\\s*\"\\s*\\)?,?\\s*$"), _T(""), TRUE);
	ReplaceText(_T("\\\""), _T("\""), FALSE);
	ReplaceText(_T("\\\\"), _T("\\"), FALSE);
}

void CSQLEditView::OnConvertFromHtml() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("&amp"), _T("&"), FALSE);
	ReplaceText(_T("&lt"), _T("<"), FALSE);
	ReplaceText(_T("&gt"), _T(">"), FALSE);
	ReplaceText(_T("&quot"), _T("\""), FALSE);
}

void CSQLEditView::OnConvertFromJava() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("^\\s*\\\""), _T(""), TRUE);
	ReplaceText(_T("\\s*(\\\\n)?\\s*\"\\s*\\+?\\s*;?$"), _T(""), TRUE);
	ReplaceText(_T("\\\""), _T("\""), FALSE);
	ReplaceText(_T("\\\\"), _T("\\"), FALSE);
}

void CSQLEditView::OnConvertFromPerl() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("^\\s*(\"|')"), _T(""), TRUE);
	ReplaceText(_T("\\s*(\\\\n)?\\s*(\"|')\\s*\\.?\\s*\\)?;?$"), _T(""), TRUE);
	ReplaceText(_T("\\\""), _T("\""), FALSE);
	ReplaceText(_T("\\\\"), _T("\\"), FALSE);
}

void CSQLEditView::OnConvertFromVb() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("^.*?\""), _T(""), TRUE);
	ReplaceText(_T("\\s*\"\\s*&?\\s*_?\\s*(vbCrLf)?\\s*$"), _T(""), TRUE);
	ReplaceText(_T("\"\""), _T("\""), FALSE);
}

void CSQLEditView::OnUpdateConvertMenu(CCmdUI* pCmdUI) 
{
	if(pCmdUI->m_pSubMenu != NULL) {
        // ポップアップ メニュー [テキスト変換] 全体を選択可能にする
		BOOL bEnable = m_edit_ctrl.HaveSelected();

		// この場合 CCmdUI::Enable は何もしないので
		//   代わりにここで適切な処理を行う
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, 
			MF_BYPOSITION | (bEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		return;
	}
	pCmdUI->Enable(m_edit_ctrl.HaveSelected());
}

void CSQLEditView::PasteObjectName(TCHAR *object_name)
{
	CString	paste_str;

	if(m_edit_ctrl.HaveSelected() == FALSE &&
		g_sql_str_token.isBreakChar(GetDocument()->GetSqlData()->get_cur_char()) == FALSE) return;

	if(m_edit_ctrl.HaveSelected() == FALSE &&
		(g_sql_str_token.isBreakChar(GetDocument()->GetSqlData()->get_prev_char()) == FALSE ||
			GetDocument()->GetSqlData()->get_prev_char() == '"')) {
		paste_str.Format(_T(", %s"), object_name);
	} else {
		paste_str.Format(_T("%s"), object_name);
	}
	m_edit_ctrl.Paste(paste_str.GetBuffer(0));

	GetParentFrame()->SetActiveView(this);
	m_edit_ctrl.SetFocus();
}

void CSQLEditView::OnClearAllBookMark() 
{
	m_edit_ctrl.ClearAllBookMark();
}

void CSQLEditView::OnEditIndent() 
{
	m_edit_ctrl.InsertTab(FALSE);
}

void CSQLEditView::OnEditIndentRev() 
{
	BOOL haveSelected = m_edit_ctrl.HaveSelected();

	m_edit_ctrl.InsertTab(TRUE);

	if(!haveSelected) {
		m_edit_ctrl.ClearSelected();
		m_edit_ctrl.LineStart(FALSE);
	}
}

void CSQLEditView::OnEditReverseRows() 
{
	m_edit_ctrl.ReverseSelectedRows();
}

void CSQLEditView::OnEditSortRowsAsc() 
{
	m_edit_ctrl.SortSelectedRows(FALSE);
}

void CSQLEditView::OnEditSortRowsDesc() 
{
	m_edit_ctrl.SortSelectedRows(TRUE);
}

void CSQLEditView::OnUpdateDisableMenu(CCmdUI* pCmdUI) 
{
	if(pCmdUI->m_pSubMenu != NULL) {
        // ポップアップ メニューを選択不可にする
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		return;
	}
	pCmdUI->Enable(FALSE);
}

void CSQLEditView::OnUpdateEnableMenu(CCmdUI* pCmdUI) 
{
	if(pCmdUI->m_pSubMenu != NULL) {
        // ポップアップ メニューを選択可能にする
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION | MF_ENABLED);
		return;
	}
	pCmdUI->Enable(TRUE);
}

void CSQLEditView::OnSetSqlBookmark() 
{
	int		start_row, end_row, row;

	start_row = 0;
	end_row = GetDocument()->GetSqlData()->get_row_cnt();

	for(row = start_row; row < end_row;) {
		row = GetDocument()->SkipNullRow(row, GetDocument()->GetSqlData());
		if(row == end_row) break;

		GetDocument()->GetSqlData()->set_row_data_flg(row, ROW_DATA_BOOK_MARK);

		row = GetDocument()->GetSQL(row, GetDocument()->GetSqlData());
	}

	Invalidate();
}

void CSQLEditView::OnEditDelete() 
{
	m_edit_ctrl.DeleteKey();
}

void CSQLEditView::OnClearSearchText() 
{
	m_edit_ctrl.ClearSearchText();	
}

void CSQLEditView::OnFilePrint() 
{
	CWaitCursor	wait_cursor;
	CPrintDlg	dlg;
	TCHAR		msg_buf[1024];

	dlg.m_line_mode = g_option.text_editor.line_mode;
	dlg.m_line_len = g_option.text_editor.line_len;
	dlg.m_row_num_digit = m_edit_ctrl.GetRowNumDigit();

	if(dlg.DoModal() != IDOK) {
		return;
	}

	CString	title;
	if(dlg.m_print_title) {
		title = GetDocument()->GetTitle();

		// タイトルの" *"を取り除く
		if(title.Find(_T(" *"), 0) != -1) {
			TCHAR *p;
			p = title.GetBuffer(0);
			p[_tcslen(p) - 2] = '\0';
			title.ReleaseBuffer();
		}
	} else {
		title = _T("");
	}

	int line_len = -1;
	if(g_option.text_editor.line_mode == EC_LINE_MODE_LEN) line_len = g_option.text_editor.line_len;
	
	CRect	space(dlg.m_space_left, dlg.m_space_top, dlg.m_space_right, dlg.m_space_bottom);

	print_edit_data(dlg.printer_dc, title.GetBuffer(0), GetDocument()->GetSqlData(),
		dlg.m_font_face_name.GetBuffer(0), dlg.m_font_point,
		space, dlg.m_print_page, dlg.m_print_date, line_len, dlg.m_row_num_digit, msg_buf);

	MessageBox(_T("印刷しました"), _T("印刷"), MB_ICONINFORMATION | MB_OK);
}

void CSQLEditView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	menu.LoadMenu(IDR_EDIT_MENU);

	// CMainFrameを親ウィンドウにすると、メニューの有効無効を、メインメニューと同じにできる
	// MDIなので，親の親を引数に渡す
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		GetParentFrame()->GetParentFrame());

	// メニュー削除
	menu.DestroyMenu();
}

void CSQLEditView::OnInsertComment() 
{
	BOOL haveSelected = m_edit_ctrl.HaveSelected();
	m_edit_ctrl.InfrateSelectedArea();

	ReplaceText(_T("^"), _T("--"), TRUE);

	if(!haveSelected) m_edit_ctrl.ClearSelected();
}

void CSQLEditView::OnDeleteComment() 
{
	BOOL haveSelected = m_edit_ctrl.HaveSelected();
	m_edit_ctrl.InfrateSelectedArea();

	ReplaceText(_T("^--"), _T(""), TRUE);

	if(!haveSelected) m_edit_ctrl.ClearSelected();
}

void CSQLEditView::OnCharLeft() { m_edit_ctrl.CharLeft(FALSE); }
void CSQLEditView::OnCharLeftExtend() { m_edit_ctrl.CharLeft(TRUE); }
void CSQLEditView::OnCharRight() { m_edit_ctrl.CharRight(FALSE); }
void CSQLEditView::OnCharRightExtend() { m_edit_ctrl.CharRight(TRUE); }
void CSQLEditView::OnLineDown() { m_edit_ctrl.LineDown(FALSE); }
void CSQLEditView::OnLineDownExtend() { m_edit_ctrl.LineDown(TRUE); }
void CSQLEditView::OnLineUp() { m_edit_ctrl.LineUp(FALSE); }
void CSQLEditView::OnLineUpExtend() { m_edit_ctrl.LineUp(TRUE); }

void CSQLEditView::OnWordLeft() { m_edit_ctrl.WordLeft(FALSE); }
void CSQLEditView::OnWordLeftExtend() { m_edit_ctrl.WordLeft(TRUE); }
void CSQLEditView::OnWordRight() { m_edit_ctrl.WordRight(FALSE); }
void CSQLEditView::OnWordRightExtend() { m_edit_ctrl.WordRight(TRUE); }

void CSQLEditView::OnLineStart() { m_edit_ctrl.LineStart(FALSE); }
void CSQLEditView::OnLineStartExtend() { m_edit_ctrl.LineStart(TRUE); }
void CSQLEditView::OnLineEnd() { m_edit_ctrl.LineEnd(FALSE); }
void CSQLEditView::OnLineEndExtend() { m_edit_ctrl.LineEnd(TRUE); }

void CSQLEditView::OnSplitStart() { m_edit_ctrl.SplitStart(FALSE); }
void CSQLEditView::OnSplitStartExtend() { m_edit_ctrl.SplitStart(TRUE); }
void CSQLEditView::OnSplitEnd() { m_edit_ctrl.SplitEnd(FALSE); }
void CSQLEditView::OnSplitEndExtend() { m_edit_ctrl.SplitEnd(TRUE); }

void CSQLEditView::OnDocumentStart() { m_edit_ctrl.DocumentStart(FALSE); }
void CSQLEditView::OnDocumentStartExtend() { m_edit_ctrl.DocumentStart(TRUE); }
void CSQLEditView::OnDocumentEnd() { m_edit_ctrl.DocumentEnd(FALSE); }
void CSQLEditView::OnDocumentEndExtend() { m_edit_ctrl.DocumentEnd(TRUE); }

void CSQLEditView::OnEditBackspace() { m_edit_ctrl.BackSpace(); }
void CSQLEditView::OnKeywordCompletion() {
	if(m_edit_ctrl.GetAssistMode() == ASSIST_CODE &&
		m_edit_ctrl.IsCodeAssistOn() && g_escape_cmd == ID_KEYWORD_COMPLETION) {
		m_edit_ctrl.CodeAssistOff();
		return;
	}

	m_edit_ctrl.KeywordCompletion(FALSE);
}
void CSQLEditView::OnKeywordCompletionRev() { m_edit_ctrl.KeywordCompletion(TRUE); }

void CSQLEditView::OnScrollUp() { m_edit_ctrl.ScrollUp(); }
void CSQLEditView::OnScrollDown() { m_edit_ctrl.ScrollDown(); }
void CSQLEditView::OnScrollPageUp() { m_edit_ctrl.ScrollPageUp(); }
void CSQLEditView::OnScrollPageDown() { m_edit_ctrl.ScrollPageDown(); }

void CSQLEditView::OnPageUp() { m_edit_ctrl.PageUp(FALSE); }
void CSQLEditView::OnPageUpExtend() { m_edit_ctrl.PageUp(TRUE); }
void CSQLEditView::OnPageDown() { m_edit_ctrl.PageDown(FALSE); }
void CSQLEditView::OnPageDownExtend() { m_edit_ctrl.PageDown(TRUE); }

void CSQLEditView::OnSelectRow() { m_edit_ctrl.SelectRow(); }
void CSQLEditView::OnSelectWord() { m_edit_ctrl.SelectWord(); }

void CSQLEditView::OnBoxSelect() { m_edit_ctrl.NextBoxSelect(); }

void CSQLEditView::OnToggleOverwrite() { m_edit_ctrl.ToggleOverwrite(); }
void CSQLEditView::OnUpdateOverwrite(CCmdUI* pCmdUI) { pCmdUI->Enable(m_edit_ctrl.GetOverwrite()); }

void CSQLEditView::OnTabToSpace() 
{
	BOOL haveSelected = m_edit_ctrl.HaveSelected();

	m_edit_ctrl.InfrateSelectedArea();
	m_edit_ctrl.TabToSpace();

	if(!haveSelected) m_edit_ctrl.ClearSelected();
}

void CSQLEditView::OnSpaceToTab() 
{
	BOOL haveSelected = m_edit_ctrl.HaveSelected();

	m_edit_ctrl.InfrateSelectedArea();
	m_edit_ctrl.SpaceToTab();

	if(!haveSelected) m_edit_ctrl.ClearSelected();
}

void CSQLEditView::OnLineJump() 
{
	CLineJumpDlg	dlg;

	if(dlg.DoModal() != IDOK) {
		return;
	}

	m_edit_ctrl.MoveCaretPos(dlg.m_line_no - 1, 0);	
}

void CSQLEditView::SearchMsg(int ret_v, int dir, BOOL b_looped)
{
	GetParentFrame()->SetMessageText(MakeSearchMsg(ret_v, dir, b_looped));
}

void CSQLEditView::OnEditDeleteRow() { m_edit_ctrl.DeleteRow(); }
void CSQLEditView::OnEditDeleteAfterCursor() { m_edit_ctrl.DeleteAfterCaret(); }
void CSQLEditView::OnLastEditPos() { m_edit_ctrl.MoveLastEditPos(); }
void CSQLEditView::OnEditCopyRow() { m_edit_ctrl.CopyRow(); }
void CSQLEditView::OnEditCutRow() { m_edit_ctrl.CutRow(); }
void CSQLEditView::OnEditPasteRow() { m_edit_ctrl.PasteRow(FALSE); }
void CSQLEditView::OnEditPasteRowUp() { m_edit_ctrl.PasteRow(TRUE); }

void CSQLEditView::OnEditJoinRow() 
{
	m_edit_ctrl.JoinRow();	
}


void CSQLEditView::OnCodeAssistOn() 
{
	m_edit_ctrl.CodeAssistOnCommand();	
}

void CSQLEditView::DoLintSql(const TCHAR *sql)
{
	CEditData	edit_data;

	if(LintSql(sql, &edit_data, &g_sql_str_token) == 0) {
		CString result = edit_data.get_all_text();
		m_edit_ctrl.Paste(result);
	}
}

void CSQLEditView::OnSqlLint() 
{
	m_edit_ctrl.InfrateSelectedArea();

	CString sql = m_edit_ctrl.GetSelectedText();
	DoLintSql(sql);
}

void CSQLEditView::OnUpdateSqlLint(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_edit_ctrl.HaveSelected());
}

void CSQLEditView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_edit_ctrl.VScroll(nSBCode);
}

void CSQLEditView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_edit_ctrl.HScroll(nSBCode);
}

BOOL CSQLEditView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	return TRUE;
}

void CSQLEditView::OnConvertFromPython()
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());

	ReplaceText(_T("^\\s*\\\"(?:\\\\n)?"), _T(""), TRUE);
	ReplaceText(_T("\\s*(\\\\n)?\\s*\"\\s*\\\\?\\s*$"), _T(""), TRUE);
	ReplaceText(_T("\\\""), _T("\""), FALSE);
	ReplaceText(_T("\\\\"), _T("\\"), FALSE);
}


void CSQLEditView::OnConvertToPython()
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("\\"), _T("\\\\"), FALSE);
	ReplaceText(_T("\""), _T("\\\""), FALSE);
	ReplaceText(_T("^(.+)$"), _T("\"$1 \\\\n\" \\\\"), TRUE);
	ReplaceText(_T("(?!\\A)^"), _T("    "), TRUE);
	ReplaceText(_T("\\\\(\\n*)\\=z"), _T(""), TRUE);
}

void CSQLEditView::OnConvertFromJavascript()
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());

	ReplaceText(_T("^\\s*\\\"(?:\\\\n)?"), _T(""), TRUE);
	ReplaceText(_T("\\s*(\\\\n)?\\s*\"\\s*\\+?\\s*$"), _T(""), TRUE);
	ReplaceText(_T("\\\""), _T("\""), FALSE);
	ReplaceText(_T("\\\\"), _T("\\"), FALSE);
}

void CSQLEditView::OnConvertToJavascript()
{
	m_edit_ctrl.InfrateSelectedArea();

	CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
	ReplaceText(_T("\\"), _T("\\\\"), FALSE);
	ReplaceText(_T("\""), _T("\\\""), FALSE);
	ReplaceText(_T("^(.+)$"), _T("\"$1 \\\\n\" +"), TRUE);
	ReplaceText(_T("\\+(\\n*)\\=z"), _T(""), TRUE);
}


void CSQLEditView::OnToLowerWithoutCommentLiteral()
{
	m_edit_ctrl.ToLowerWithOutCommentAndLiteral();
}


void CSQLEditView::OnToUpperWithoutCommentLiteral()
{
	m_edit_ctrl.ToUpperWithOutCommentAndLiteral();
}
