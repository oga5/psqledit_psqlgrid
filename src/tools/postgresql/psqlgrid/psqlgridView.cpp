/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
// psqlgridView.cpp : CPsqlgridView クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "psqlgrid.h"

#include "psqlgridDoc.h"
#include "psqlgridView.h"

#include "global.h"

#include "insertRowsDlg.h"
#include "file_winutil.h"

#include "replaceDlg.h"
#include "DataEditDlg.h"
#include "GridFilterDlg.h"
#include "griddatafilter.h"

#include "InputSequenceDlg.h"

#include "GridSortDlg.h"
#include "ostrutil.h"
#include "InputDlg.h"

#define MAX_SORT_COLUMN		5

#define WM_SEARCH_COLUMN	WM_USER + 201

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridView

IMPLEMENT_DYNCREATE(CPsqlgridView, CScrollView)

BEGIN_MESSAGE_MAP(CPsqlgridView, CScrollView)
	//{{AFX_MSG_MAP(CPsqlgridView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_DELETE_ROW, OnDeleteRow)
	ON_COMMAND(ID_INSERT_ROW, OnInsertRow)
	ON_WM_SETFOCUS()
	ON_UPDATE_COMMAND_UI(ID_DELETE_ROW, OnUpdateDeleteRow)
	ON_UPDATE_COMMAND_UI(ID_INSERT_ROW, OnUpdateInsertRow)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_ENTER_EDIT, OnEnterEdit)
	ON_COMMAND(ID_INPUT_ENTER, OnInputEnter)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_SEARCH, OnSearch)
	ON_COMMAND(ID_SEARCH_NEXT, OnSearchNext)
	ON_COMMAND(ID_SEARCH_PREV, OnSearchPrev)
	ON_UPDATE_COMMAND_UI(ID_INSERT_ROWS, OnUpdateInsertRows)
	ON_COMMAND(ID_INSERT_ROWS, OnInsertRows)
	ON_COMMAND(ID_GRID_COPY_CSV, OnGridCopyCsv)
	ON_COMMAND(ID_GRID_COPY_CSV_CNAME, OnGridCopyCsvCname)
	ON_COMMAND(ID_GRID_COPY_TAB, OnGridCopyTab)
	ON_COMMAND(ID_GRID_COPY_TAB_CNAME, OnGridCopyTabCname)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_REPLACE, OnReplace)
	ON_UPDATE_COMMAND_UI(ID_SEARCH, OnUpdateSearch)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_NEXT, OnUpdateSearchNext)
	ON_UPDATE_COMMAND_UI(ID_SEARCH_PREV, OnUpdateSearchPrev)
	ON_UPDATE_COMMAND_UI(ID_REPLACE, OnUpdateReplace)
	ON_COMMAND(ID_GRID_COPY_FIX_LEN, OnGridCopyFixLen)
	ON_COMMAND(ID_GRID_COPY_FIX_LEN_CNAME, OnGridCopyFixLenCname)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_CLEAR_SEARCH_TEXT, OnClearSearchText)
	ON_COMMAND(ID_DELETE_NULL_ROWS, OnDeleteNullRows)
	ON_COMMAND(ID_EDIT_PASTE_TO_CELL, OnEditPasteToCell)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_TO_CELL, OnUpdateEditPasteToCell)
	ON_COMMAND(ID_DATA_EDIT, OnDataEdit)
	ON_UPDATE_COMMAND_UI(ID_DATA_EDIT, OnUpdateDataEdit)
	ON_COMMAND(ID_DELETE_ALL_NULL_ROWS, OnDeleteAllNullRows)
	ON_UPDATE_COMMAND_UI(ID_DELETE_ALL_NULL_ROWS, OnUpdateDeleteAllNullRows)
	ON_UPDATE_COMMAND_UI(ID_DELETE_NULL_ROWS, OnUpdateDeleteNullRows)
	ON_COMMAND(ID_TO_LOWER, OnToLower)
	ON_COMMAND(ID_TO_UPPER, OnToUpper)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU, OnZenkakuToHankaku)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU_ALPHA, OnZenkakuToHankakuAlpha)
	ON_COMMAND(ID_ZENKAKU_TO_HANKAKU_KATA, OnZenkakuToHankakuKata)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU, OnHankakuToZenkaku)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU_ALPHA, OnHankakuToZenkakuAlpha)
	ON_COMMAND(ID_HANKAKU_TO_ZENKAKU_KATA, OnHankakuToZenkakuKata)
	ON_COMMAND(ID_INPUT_SEQUENCE, OnInputSequence)
	ON_UPDATE_COMMAND_UI(ID_INPUT_SEQUENCE, OnUpdateInputSequence)
	ON_COMMAND(ID_ADJUST_ALL_COL_WIDTH_DATAONLY, OnAdjustAllColWidthDataonly)
	ON_COMMAND(ID_ADJUST_ALL_COL_WIDTH_USE_COLNAME, OnAdjustAllColWidthUseColname)
	ON_COMMAND(ID_GRID_SORT, OnGridSort)
	ON_COMMAND(ID_SEARCH_COLUMN, OnSearchColumn)
	ON_COMMAND(ID_EQUAL_ALL_COL_WIDTH, OnEqualAllColWidth)
	ON_COMMAND(ID_DATA_SET_BLANK, OnDataSetBlank)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_END, OnUpdateEnd)
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRID_COPY_TAB, ID_GRID_COPY_FIX_LEN_CNAME, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI_RANGE(ID_HANKAKU_TO_ZENKAKU, ID_ZENKAKU_TO_HANKAKU_ALPHA, OnUpdateValidCurPt)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TO_LOWER, ID_TO_UPPER, OnUpdateValidCurPt)
	ON_COMMAND(ID_GRID_COLUMN_WIDTH, &CPsqlgridView::OnGrcolumnWidth)
	ON_UPDATE_COMMAND_UI(ID_GRID_COLUMN_WIDTH, &CPsqlgridView::OnUpdateGrcolumnWidth)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridView クラスの構築/消滅

CPsqlgridView::CPsqlgridView()
{
}

CPsqlgridView::~CPsqlgridView()
{
}

BOOL CPsqlgridView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
	//  修正してください。

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridView クラスの描画

void CPsqlgridView::OnDraw(CDC* pDC)
{
	CPsqlgridDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: この場所にネイティブ データ用の描画コードを追加します。
}

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridView クラスの診断

#ifdef _DEBUG
void CPsqlgridView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CPsqlgridView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CPsqlgridDoc* CPsqlgridView::GetDocument() // 非デバッグ バージョンはインラインです。
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPsqlgridDoc)));
	return (CPsqlgridDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridView クラスのメッセージ ハンドラ

int CPsqlgridView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_grid_ctrl.SetGridData(GetDocument()->GetGridData());
	m_grid_ctrl.SetSplitterMode(TRUE);

	m_grid_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD, 
		CRect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy), this, NULL);
	m_grid_ctrl.SetGridStyle(GRS_COL_HEADER | GRS_ROW_HEADER | GRS_MULTI_SELECT |
		GRS_SEARCH_SELECTED_AREA | GRS_HIGHLIGHT_HEADER);
	
	m_grid_ctrl.SetHWheelScrollEnable(TRUE);
	m_grid_ctrl.SetFont(&g_font);
	SetGridOption();
	
	return 0;
}

void CPsqlgridView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	
	m_grid_ctrl.MoveWindow(0, 0, cx, cy);
}

void CPsqlgridView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

	SetScrollSizes(MM_TEXT, CSize(INT_MAX, INT_MAX));

	SetHeaderStyle();
}

void CPsqlgridView::GridFilterOn()
{
	// FIXME: Filter時のアクティブセルについて、Filter実行後も現在のセルが表示されるのであれば、そこをアクティブにしたい
	// 指定した行が表示されているかどうか、バイナリサーチでチェックできるか？ソートしてからフィルタなどの操作もあるので工夫がいるかも
	int col;
	if(!GetDocument()->GetGridSwapRowColMode()) {
		col = m_grid_ctrl.GetSelectedCol();
	} else {
		col = m_grid_ctrl.GetSelectedRow();
	}

//	CGridFilterDlg dlg;
//	dlg.DoModal(this, WM_GRID_FILTER_RUN, &m_search_data, GetDocument()->GetPgGridData(), col, TRUE);
	HideSearchDlgs();
	CGridFilterDlgSingleton::GetDlg().ShowDialog(this, &m_search_data,
		GetDocument()->GetPgGridData(), col, FALSE);
}

int CPsqlgridView::GridFilterRun(int grid_filter_col_no)
{
	int col = grid_filter_col_no;

	CGridData* grid_data = GetDocument()->GetPgGridData();

	CString filter_msg;
	int find_cnt = 0;

	int ret_v = grid_data->FilterData(grid_filter_col_no, m_search_data.m_search_text.GetBuffer(0),
		m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii,
		m_search_data.m_regexp, &find_cnt, &filter_msg);
	if(ret_v != 0) {
		MessageBox(filter_msg, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	PostGridFilterOnOff();
	return 0;
}

void CPsqlgridView::GridFilterOff()
{
	if(!GetDocument()->GetGridFilterMode()) return;

	CGridData* grid_data = GetDocument()->GetPgGridData();
	grid_data->FilterOff();
	PostGridFilterOnOff();
}

void CPsqlgridView::PostGridFilterOnOff()
{
	int col = GetDocument()->GetPgGridData()->get_cur_col();

	GetDocument()->PostGridFilterOnOff();
	m_grid_ctrl.SetGridData(GetDocument()->GetGridData());
	SetGridOption();

	GetDocument()->CalcGridSelectedData();

	if(!GetDocument()->GetGridSwapRowColMode()) {
		m_grid_ctrl.SetCell(col, 0);
	} else {
		m_grid_ctrl.SetCell(0, col);
	}

	SetStatusBarInfo();
}

void CPsqlgridView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint) {
	case UPD_FONT:
		m_grid_ctrl.SetFont((CFont *)pHint);
		break;
	case UPD_GRID_DATA:
		{
			HideSearchDlgs();
			m_grid_ctrl.SetGridData(GetDocument()->GetGridData());
			INT_PTR b = (INT_PTR)pHint;
			if(b == TRUE) {
				if(g_option.adjust_col_width) {
					m_grid_ctrl.AdjustAllColWidth(!g_option.grid.adjust_col_width_no_use_colname, FALSE);
				} else {
					m_grid_ctrl.EqualAllColWidth();
				}
			}
		}
		break;
	case UPD_ERROR:
		POINT *pt;
		pt = (POINT *)pHint;
		m_grid_ctrl.Update();

		m_grid_ctrl.ClearSelected();
		if(pt->x == -1) {
			m_grid_ctrl.SelectRow(pt->y);
		} else {
			m_grid_ctrl.SetCell(pt->x, pt->y);
		}
		break;
	case UPD_FLUSH_DATA:
		m_grid_ctrl.LeaveEdit();
		break;
	case UPD_SET_HEADER_STYLE:
		SetHeaderStyle();
		break;
	case UPD_GRID_OPTION:
		SetGridOption();
		break;
	case UPD_CLEAR_DATA:
		GetDocument()->ClearData();
		break;
	case UPD_RECONNECT:
		GetDocument()->SetTitle(_T(""));
		break;
	case UPD_WINDOW_MOVED:
		m_grid_ctrl.WindowMoved();
		break;
	case UPD_CALC_GRID_SELECTED_DATA:
		GetDocument()->CalcGridSelectedData();
		break;
	case UPD_INVALIDATE_CUR_CELL:
		m_grid_ctrl.InvalidateCell(GetDocument()->GetGridData()->get_cur_cell());
		break;
	case UPD_GRID_SWAP_ROW_COL:
		{
			DWORD grid_option = m_grid_ctrl.GetGridStyle();
			if(GetDocument()->GetGridSwapRowColMode()) {
				grid_option |= GRS_SWAP_ROW_COL_MODE;
			} else {
				grid_option &= ~(GRS_SWAP_ROW_COL_MODE);
			}
			m_grid_ctrl.SetGridStyle(grid_option, FALSE);
		}
		m_grid_ctrl.SetGridData(GetDocument()->GetGridData());
		GetDocument()->CalcGridSelectedData();
		SetGridOption();
		break;
	case UPD_GRID_CTRL:
		if(IsWindowVisible()) {
			m_grid_ctrl.Update();
		}
		break;
	case UPD_GRID_FILTER_DATA_ON:
		GridFilterOn();
		break;
	case UPD_GRID_FILTER_DATA_OFF:
		GridFilterOff();
		break;
	}
}

void CPsqlgridView::SetStatusBarInfo()
{
	CGridData* grid_data = GetDocument()->GetGridData();

	int row = grid_data->get_cur_row();
	int col = grid_data->get_cur_col();

	CString msg = _T("");

	if(GetDocument()->GetGridFilterMode()) {
		msg += _T("Filter中");
	}

	if(row >= 0 && col >= 0) {
		if(!msg.IsEmpty()) {
			msg += _T(", ");
		}

		CString col_type;
		if(!GetDocument()->GetGridSwapRowColMode()) {
			col_type = GetDocument()->GetPgGridData()->GetDispColType(col);
		} else {
			col_type = GetDocument()->GetPgGridData()->GetDispColType(row);
		}

		CString cell_msg;
		if(grid_data->IsUpdateCell(row, col)) {
			cell_msg.Format(_T("Type: %s, Original Text: %s"), col_type.GetString(), grid_data->GetOriginalText(row, col));
		} else {
			cell_msg.Format(_T("Type: %s"), col_type.GetString());
		}

		msg += cell_msg;
	}

	GetParentFrame()->SetMessageText(msg);
}

void CPsqlgridView::OnDeleteRow() 
{
	m_grid_ctrl.DeleteRow();
}

void CPsqlgridView::OnInsertRow() 
{
	m_grid_ctrl.InsertRows(1);
}

void CPsqlgridView::OnSetFocus(CWnd* pOldWnd) 
{
	CScrollView::OnSetFocus(pOldWnd);
	
	m_grid_ctrl.SetFocus();	
}

void CPsqlgridView::OnUpdateDeleteRow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetGridData()->CanDeleteRow(
		GetDocument()->GetGridData()->get_cur_row()));
}

void CPsqlgridView::OnUpdateInsertRow(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetGridData()->CanInsertRow());
}

void CPsqlgridView::OnEditUndo() 
{
	BOOL b_grid_filter = GetDocument()->GetGridFilterMode();

	m_grid_ctrl.Undo();
	GetParentFrame()->SetMessageText(_T(""));

	if(b_grid_filter != GetDocument()->GetGridFilterMode()) {
		PostGridFilterOnOff();
	}
}

void CPsqlgridView::OnEditRedo() 
{
	BOOL b_grid_filter = GetDocument()->GetGridFilterMode();

	m_grid_ctrl.Redo();
	GetParentFrame()->SetMessageText(_T(""));

	if(b_grid_filter != GetDocument()->GetGridFilterMode()) {
		PostGridFilterOnOff();
	}
}

void CPsqlgridView::OnEditCopy() 
{
	m_grid_ctrl.Copy();
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnEditPaste() 
{
	m_grid_ctrl.Paste();
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnEditCut() 
{
	m_grid_ctrl.Cut();
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnEditSelectAll() 
{
	m_grid_ctrl.SelectAll();
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnEnterEdit() 
{
	m_grid_ctrl.EnterEdit();
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnInputEnter() 
{
	m_grid_ctrl.InputEnter();
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	if(m_grid_ctrl.CanCut()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CPsqlgridView::OnUpdateEditRedo(CCmdUI* pCmdUI) 
{
	if(m_grid_ctrl.CanRedo()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CPsqlgridView::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	if(m_grid_ctrl.CanUndo()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CPsqlgridView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	if (pCmdUI->m_pSubMenu != NULL) {
        // ポップアップ メニュー [コピー(フォーマット指定)] 全体を選択可能にする
		BOOL bEnable = m_grid_ctrl.CanCopy();

		// この場合 CCmdUI::Enable は何もしないので
		//   代わりにここで適切な処理を行う
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, 
			MF_BYPOSITION | (bEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		return;
	}
	pCmdUI->Enable(m_grid_ctrl.CanCopy());
}

void CPsqlgridView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	if(m_grid_ctrl.CanPaste()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CPsqlgridView::OnSearch() 
{
//	CSearchDlg		dlg;
//	dlg.DoModal(this, WM_SEARCH_TEXT, &m_search_data, FALSE);
	HideSearchDlgs();
	CSearchDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, &m_search_data, FALSE);
}

void CPsqlgridView::OnSearchNext() 
{
	BOOL b_looped;
	int ret_v = m_grid_ctrl.SearchNext2(m_search_data.m_search_text.GetBuffer(0),
		m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii, m_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, 1, b_looped);
}

void CPsqlgridView::OnSearchPrev() 
{
	BOOL b_looped;
	int ret_v = m_grid_ctrl.SearchPrev2(m_search_data.m_search_text.GetBuffer(0),
		m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii, m_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, -1, b_looped);
}

LRESULT CPsqlgridView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case GC_WM_CHANGE_SELECT_AREA:
		GetDocument()->CalcGridSelectedData();
		break;
	case WM_SEARCH_TEXT:
		{
			BOOL b_looped;
			int ret_v = m_grid_ctrl.SearchText2(m_search_data.m_search_text,
				m_search_data.m_dir, m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii,
				m_search_data.m_regexp, TRUE, &b_looped);
			SearchMsg(ret_v, m_search_data.m_dir, b_looped);
		}
		break;
	case WM_REPLACE_TEXT:
		{
			int replace_cnt;
			BOOL b_looped;
			int ret_v = m_grid_ctrl.ReplaceText2(m_search_data.m_search_text,
				m_search_data.m_replace_text,
				m_search_data.m_dir, m_search_data.m_distinct_lwr_upr, m_search_data.m_distinct_width_ascii,
				m_search_data.m_regexp, m_search_data.m_all,
				m_search_data.m_replace_selected_area, &replace_cnt, &b_looped);

			if(m_search_data.m_all) {
				CString msg;
				msg.Format(_T("%d件置換しました"), replace_cnt);
				GetParentFrame()->SetMessageText(msg);
			} else {
				SearchMsg(ret_v, m_search_data.m_dir, b_looped);
			}
		}
		break;
	case WM_SEARCH_COLUMN:
		{
			BOOL b_looped;
			int ret_v = m_grid_ctrl.SearchColumn(m_search_data.m_search_text.GetBuffer(0),
				m_search_data.m_dir, m_search_data.m_distinct_lwr_upr,
				m_search_data.m_regexp, TRUE, &b_looped);
			SearchMsg(ret_v, m_search_data.m_dir, b_looped);
		}
		break;
	case WM_GRID_FILTER_RUN:
		{
			int ret_v = GridFilterRun((int)wParam);
			return ret_v;
		}
		break;
	case WM_GRID_FILTER_CLEAR:
		GridFilterOff();
		break;
	}
	
	return CScrollView::WindowProc(message, wParam, lParam);
}

void CPsqlgridView::OnAdjustAllColWidthDataonly() 
{
	m_grid_ctrl.AdjustAllColWidth(FALSE, FALSE);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnAdjustAllColWidthUseColname() 
{
	m_grid_ctrl.AdjustAllColWidth(TRUE, FALSE);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnEqualAllColWidth() 
{
	m_grid_ctrl.EqualAllColWidth();
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnUpdateInsertRows(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetGridData()->CanInsertRow());
}

void CPsqlgridView::OnInsertRows() 
{
	CInsertRowsDlg	dlg;

	if(dlg.DoModal() != IDOK) return;

	m_grid_ctrl.InsertRows(dlg.m_row_cnt);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnGridCopyCsv() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_CSV);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnGridCopyCsvCname() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_CSV_CNAME);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnGridCopyTab() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_TAB);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnGridCopyTabCname() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_TAB_CNAME);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnGridCopyFixLen() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_FIX_LEN);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnGridCopyFixLenCname() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_FIX_LEN_CNAME);
	GetParentFrame()->SetMessageText(_T(""));
}

static CString GetDataSize(unsigned int size)
{
	CString result;

	if(size < 1024) {
		result.Format(_T("%d"), size);	
	} else if(size >= 1024) {
		result.Format(_T("%dk"), size / 1024);
	} else if(size >= (1024 * 1024)) {
		result.Format(_T("%dM"), size / (1024 * 1024));
	} else {
		result.Format(_T("%dG"), size / (1024 * 1024 * 1024));
	}

	return result;
}

void CPsqlgridView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	menu.LoadMenu(IDR_EDIT_MENU);

	if(m_grid_ctrl.IsValidCurPt() && !m_grid_ctrl.HaveSelected()) {
		POINT cell_pt = *(GetDocument()->GetGridData()->get_cur_cell());
		size_t data_len = _tcslen(GetDocument()->GetGridData()->Get_ColData(cell_pt.y, cell_pt.x));
		CString menu_name;
		menu_name.Format(_T("データ編集(%sbyte)"), GetDataSize((unsigned int)data_len));

		menu.GetSubMenu(0)->InsertMenu(0, MF_STRING | MF_BYPOSITION, ID_DATA_EDIT, menu_name);
		menu.GetSubMenu(0)->InsertMenu(1, MF_STRING | MF_BYPOSITION, ID_DATA_SET_BLANK, _T("空の文字列にする"));
		menu.GetSubMenu(0)->InsertMenu(2, MF_SEPARATOR | MF_BYPOSITION);
	}

	if(GetDocument()->GetGridData()->Get_RowCnt() > 0) {
		UINT filter_menu_id;
		if(!GetDocument()->GetGridFilterMode()) {
			filter_menu_id = ID_GRID_FILTER_ON;
		} else {
			filter_menu_id = ID_GRID_FILTER_OFF;
		}
		CString filter_menu_str;
		filter_menu_str.LoadStringW(filter_menu_id);
		filter_menu_str = filter_menu_str.Right(filter_menu_str.GetLength() - filter_menu_str.Find('\n'));
		menu.GetSubMenu(0)->InsertMenu(-1, MF_STRING | MF_BYPOSITION, filter_menu_id, filter_menu_str);
	}

	// CMainFrameを親ウィンドウにすると、メニューの有効無効を、メインメニューと同じにできる
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetParentFrame());

	// メニュー削除
	menu.DestroyMenu();	
}

void CPsqlgridView::OnReplace() 
{
	BOOL disable_selected_area = FALSE;

	if(m_grid_ctrl.HaveSelected()) {
		m_search_data.m_replace_selected_area = TRUE;
		disable_selected_area = FALSE;
	} else {
		m_search_data.m_replace_selected_area = FALSE;
		disable_selected_area = TRUE;
	}

//	CReplaceDlg		dlg;
//	dlg.DoModal(this, WM_SEARCH_TEXT, WM_REPLACE_TEXT, disable_selected_area, &m_search_data);
	HideSearchDlgs();
	CReplaceDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, WM_REPLACE_TEXT, &m_search_data);
}

void CPsqlgridView::OnUpdateSearch(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetTableName() != _T(""));
}

void CPsqlgridView::OnUpdateSearchNext(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetTableName() != _T(""));
}

void CPsqlgridView::OnUpdateSearchPrev(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetTableName() != _T(""));
}

void CPsqlgridView::OnUpdateReplace(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetTableName() != _T(""));
}

void CPsqlgridView::SearchMsg(int ret_v, int dir, BOOL b_looped)
{
	GetParentFrame()->SetMessageText(MakeSearchMsg(ret_v, dir, b_looped));
}

void CPsqlgridView::SetHeaderStyle()
{
	if(!GetParent()->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) return;
	CSplitterWnd *sp = (CSplitterWnd *)GetParent();

	int grid_style = m_grid_ctrl.GetGridStyle();
	grid_style &= ~GRS_COL_HEADER;
	grid_style &= ~GRS_ROW_HEADER;

	if(sp->GetPane(0, 0) == this) {
		m_grid_ctrl.SetGridStyle(grid_style | GRS_COL_HEADER | GRS_ROW_HEADER, TRUE);
	} else if(sp->GetColumnCount() == 2 && sp->GetPane(0, 1) == this) {
		m_grid_ctrl.SetGridStyle(grid_style | GRS_COL_HEADER, TRUE);
	} else if(sp->GetRowCount() == 2 && sp->GetPane(1, 0) == this) {
		m_grid_ctrl.SetGridStyle(grid_style | GRS_ROW_HEADER, TRUE);
	} else if(sp->GetRowCount() == 2 && sp->GetColumnCount() == 2 && sp->GetPane(1, 1) == this) {
		m_grid_ctrl.SetGridStyle(grid_style, TRUE);
	}
}

void CPsqlgridView::SetGridOption()
{
	int		i;
	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		m_grid_ctrl.SetColor(i, g_option.grid.color[i]);
	}

	int grid_option = m_grid_ctrl.GetGridStyle();
	if(g_option.grid.color[GRID_BG_COLOR] != g_option.grid.color[GRID_NULL_CELL_COLOR]) {
		grid_option |= GRS_SHOW_NULL_CELL;
	} else {
		grid_option &= ~(GRS_SHOW_NULL_CELL);
	}

	if(g_option.grid.show_space) {
		grid_option |= GRS_SHOW_SPACE;
	} else {
		grid_option &= ~(GRS_SHOW_SPACE);
	}
	if(g_option.grid.show_2byte_space) {
		grid_option |= GRS_SHOW_2BYTE_SPACE;
	} else {
		grid_option &= ~(GRS_SHOW_2BYTE_SPACE);
	}
	if(g_option.grid.show_tab) {
		grid_option |= GRS_SHOW_TAB;
	} else {
		grid_option &= ~(GRS_SHOW_TAB);
	}
	if(g_option.grid.show_line_feed) {
		grid_option |= GRS_SHOW_LINE_FEED;
	} else {
		grid_option &= ~(GRS_SHOW_LINE_FEED);
	}
	if(g_option.grid.adjust_col_width_no_use_colname) {
		grid_option |= GRS_ADJUST_COL_WIDTH_NO_USE_COL_NAME;
	} else {
		grid_option &= ~(GRS_ADJUST_COL_WIDTH_NO_USE_COL_NAME);
	}
	if(g_option.grid.invert_select_text) {
		grid_option |= GRS_INVERT_SELECT_TEXT;
	} else {
		grid_option &= ~(GRS_INVERT_SELECT_TEXT);
	}
	if(g_option.grid.copy_escape_dblquote) {
		grid_option |= GRS_COPY_ESCAPE_DBL_QUOTE;
	} else {
		grid_option &= ~(GRS_COPY_ESCAPE_DBL_QUOTE);
	}
	if(g_option.grid.ime_caret_color) {
		grid_option |= GRS_IME_CARET_COLOR;
	} else {
		grid_option &= ~(GRS_IME_CARET_COLOR);
	}
	if(g_option.grid.end_key_like_excel) {
		grid_option |= GRS_END_KEY_LIKE_EXCEL;
	} else {
		grid_option &= ~(GRS_END_KEY_LIKE_EXCEL);
	}
	if(GetDocument()->GetGridSwapRowColMode()) {
		grid_option |= GRS_SWAP_ROW_COL_MODE;
	} else {
		grid_option &= ~(GRS_SWAP_ROW_COL_MODE);
	}
	GetDocument()->GetPgGridData()->SetUseMessageBeep(g_option.use_message_beep);

	m_grid_ctrl.SetNullText(g_option.grid.null_text);
	m_grid_ctrl.SetCellPadding(g_option.grid.cell_padding_top, g_option.grid.cell_padding_bottom,
		g_option.grid.cell_padding_left, g_option.grid.cell_padding_right);
	m_grid_ctrl.SetGridStyle(grid_option);

	m_grid_ctrl.RedrawWindow();
}

void CPsqlgridView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	m_grid_ctrl.HScroll(nSBCode);
}

void CPsqlgridView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	m_grid_ctrl.VScroll(nSBCode);
}

BOOL CPsqlgridView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
	return TRUE;
}

BOOL CPsqlgridView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void CPsqlgridView::OnClearSearchText() 
{
	m_grid_ctrl.ClearSearchText();	
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnEditPasteToCell() 
{
	if(m_grid_ctrl.HaveSelected()) return;

	m_grid_ctrl.EnterEdit();
	if(m_grid_ctrl.IsEnterEdit()) {
		m_grid_ctrl.SelectAll();
		m_grid_ctrl.Paste();
	}
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnUpdateEditPasteToCell(CCmdUI* pCmdUI) 
{
	if(m_grid_ctrl.CanPaste() && !m_grid_ctrl.HaveSelected()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CPsqlgridView::OnDataEdit() 
{
	if(m_grid_ctrl.IsEnterEdit()) return;

	POINT				pt;

	pt = *(GetDocument()->GetGridData()->get_cur_cell());

	if(!GetDocument()->GetGridData()->IsEditableCell(pt.y, pt.x)) return;

	const TCHAR *p = GetDocument()->GetGridData()->Get_EditData(pt.y, pt.x);
	int			line_type = check_line_type(p);

	CEditData			edit_data;
	edit_data.set_undo_cnt(INT_MAX);
	edit_data.del_all();
	edit_data.set_limit_text(GetDocument()->GetGridData()->Get_ColLimit(pt.y, pt.x));
	edit_data.set_copy_line_type(line_type);
	edit_data.paste(p);
	edit_data.reset_undo();
	edit_data.set_cur(0, 0);

	DWORD		style = 0;
	if(g_option.grid.show_space) {
		style |= ECS_SHOW_SPACE;
	}
	if(g_option.grid.show_2byte_space) {
		style |= ECS_SHOW_2BYTE_SPACE;
	}
	if(g_option.grid.show_tab) {
		style |= ECS_SHOW_TAB;
	}
	if(g_option.grid.show_line_feed) {
		style |= ECS_SHOW_LINE_FEED;
	}
	if(!(g_option.grid.invert_select_text)) {
		style |= ECS_NO_INVERT_SELECT_TEXT;
	}
	if(g_option.grid.ime_caret_color) {
		style |= ECS_IME_CARET_COLOR;
	}

	CDataEditDlg		dlg;
	dlg.SetData(&edit_data, line_type, &g_font, style, g_option.grid.color);

	if(dlg.DoModal() == IDCANCEL) return;

	m_grid_ctrl.UpdateCell(pt.y, pt.x, edit_data.get_all_text().GetBuffer(0), -1);
	GetParentFrame()->SetMessageText(_T(""));
}

void CPsqlgridView::OnUpdateDataEdit(CCmdUI* pCmdUI) 
{
	if(m_grid_ctrl.HaveSelected() || !m_grid_ctrl.IsValidCurPt()) {
		pCmdUI->Enable(FALSE);
		return;
	}

	POINT cell_pt = *(GetDocument()->GetGridData()->get_cur_cell());

	if(GetDocument()->GetGridData()->IsEditableCell(cell_pt.y, cell_pt.x)) {
		pCmdUI->Enable(TRUE);
		return;
	}
	
	pCmdUI->Enable(FALSE);
}

void CPsqlgridView::DeleteNullRows(BOOL all_flg)
{
	int del_row_cnt = m_grid_ctrl.DeleteNullRows(all_flg);

	CString msg;
	msg.Format(_T("%d行削除しました"), del_row_cnt);
	GetParentFrame()->SetMessageText(msg);
}

void CPsqlgridView::OnDeleteAllNullRows() 
{
	if(g_option.disp_all_delete_null_rows_warning) {
		if(MessageBox(
			_T("現在表示中のカラムが，すべてNULLの行を削除します。よろしいですか？\n")
			_T("(すべてのカラムを表示していない場合，非表示のカラムにデータが入っていても空行と見なします)"),
			_T("Message"),
			MB_ICONQUESTION | MB_YESNO) != IDYES) return;
	}

	DeleteNullRows(TRUE);
}

void CPsqlgridView::OnDeleteNullRows() 
{
	DeleteNullRows(FALSE);
}

void CPsqlgridView::OnUpdateDeleteAllNullRows(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetGridData()->CanInsertRow());
}

void CPsqlgridView::OnUpdateDeleteNullRows(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetGridData()->CanInsertRow());
}

BOOL CPsqlgridView::OnUpdateDataType(CCmdUI *pCmdUI, int type)
{
/*
	if(m_grid_ctrl.HaveSelected() || !m_grid_ctrl.IsValidCurPt()) {
		pCmdUI->Enable(FALSE);
		return FALSE;
	}

	POINT cell_pt = *(GetDocument()->GetGridData()->get_cur_cell());

	if(GetDocument()->GetGridData()->GetDataset()->col[cell_pt.x].dbtype == type &&
		!GetDocument()->GetGridData()->IsDeleteRow(cell_pt.y)) {
		pCmdUI->Enable(TRUE);
		return TRUE;
	}
	
	pCmdUI->Enable(FALSE);
	return FALSE;
*/
	pCmdUI->Enable(FALSE);
	return FALSE;
}


void CPsqlgridView::OnToLower() 
{
	m_grid_ctrl.ToLower();
}

void CPsqlgridView::OnToUpper() 
{
	m_grid_ctrl.ToUpper();
}

void CPsqlgridView::OnZenkakuToHankaku() 
{
	m_grid_ctrl.ToHankaku(TRUE, TRUE);
}

void CPsqlgridView::OnZenkakuToHankakuAlpha() 
{
	m_grid_ctrl.ToHankaku(TRUE, FALSE);
}

void CPsqlgridView::OnZenkakuToHankakuKata() 
{
	m_grid_ctrl.ToHankaku(FALSE, TRUE);
}

void CPsqlgridView::OnHankakuToZenkaku() 
{
	m_grid_ctrl.ToZenkaku(TRUE, TRUE);
}

void CPsqlgridView::OnHankakuToZenkakuAlpha() 
{
	m_grid_ctrl.ToZenkaku(TRUE, FALSE);
}

void CPsqlgridView::OnHankakuToZenkakuKata() 
{
	m_grid_ctrl.ToZenkaku(FALSE, TRUE);
}

void CPsqlgridView::OnUpdateEnd(CCmdUI* pCmdUI)
{
	if(m_grid_ctrl.GetEndKeyFlg()) {
		pCmdUI->SetText(_T("END"));
	} else {
		pCmdUI->SetText(_T(""));
	}
}

void CPsqlgridView::OnInputSequence() 
{
	if(!m_grid_ctrl.HaveSelected()) return;

	struct grid_data_select_area_st selected_area = *(GetDocument()->GetGridData()->GetSelectArea());

	CInputSequenceDlg	dlg;

	dlg.m_start_num = _ttoi64(
		GetDocument()->GetGridData()->Get_DispData(selected_area.pos1.y, selected_area.pos1.x));
	dlg.m_incremental = 1;

	if(dlg.DoModal() != IDOK) return;

	m_grid_ctrl.InputSequence(dlg.m_start_num, dlg.m_incremental);
}

void CPsqlgridView::OnUpdateInputSequence(CCmdUI* pCmdUI) 
{
	BOOL b_enable = FALSE;
	
	if(m_grid_ctrl.HaveSelected()) {
		struct grid_data_select_area_st selected_area = *(GetDocument()->GetGridData()->GetSelectArea());
		if(!GetDocument()->GetGridSwapRowColMode()) {
			if(selected_area.pos1.x == selected_area.pos2.x) b_enable = TRUE;
		} else {
			if(selected_area.pos1.y == selected_area.pos2.y) b_enable = TRUE;
		}
	}

	pCmdUI->Enable(b_enable);
}

void CPsqlgridView::InitSortData(int *sort_col_no, int *sort_order, int &sort_col_cnt, int order)
{
	CGridData *grid_data = GetDocument()->GetGridData();

	int		start_x, end_x, x;

	if(!GetDocument()->GetGridSwapRowColMode()) {
		if(m_grid_ctrl.HaveSelected()) {
			start_x = min(grid_data->GetSelectArea()->pos1.x, grid_data->GetSelectArea()->pos2.x);
			end_x = max(grid_data->GetSelectArea()->pos1.x, grid_data->GetSelectArea()->pos2.x);
		} else {
			start_x = end_x = grid_data->get_cur_col();
		}
	} else {
		if(m_grid_ctrl.HaveSelected()) {
			start_x = min(grid_data->GetSelectArea()->pos1.y, grid_data->GetSelectArea()->pos2.y);
			end_x = max(grid_data->GetSelectArea()->pos1.y, grid_data->GetSelectArea()->pos2.y);
		} else {
			start_x = end_x = grid_data->get_cur_row();
		}
	}

	// 初期値を設定
	for(int i = 0; i < MAX_SORT_COLUMN; i++) sort_col_no[i] = -1;

	sort_col_cnt = 0;
	for(x = start_x; x <= end_x; x++) {
		sort_col_no[sort_col_cnt] = x;
		sort_order[sort_col_cnt] = order;
		sort_col_cnt++;
		if(sort_col_cnt == MAX_SORT_COLUMN) break;
	}
}

void CPsqlgridView::OnGridSort() 
{
	int		sort_col_no[MAX_SORT_COLUMN];
	int		sort_order[MAX_SORT_COLUMN];
	int		sort_col_cnt;

	InitSortData(sort_col_no, sort_order, sort_col_cnt, 1);

	CGridSortDlg	dlg;
	dlg.InitData(GetDocument()->GetGridData(), sort_col_no, sort_order, &sort_col_cnt);

	if(dlg.DoModal() != IDOK) return;

	SortData(sort_col_no, sort_order, sort_col_cnt);
}

void CPsqlgridView::SortData(int *sort_col_no, int *sort_order, int sort_col_cnt)
{
	CWaitCursor wait_cursor;
	CGridData *grid_data = GetDocument()->GetPgGridData();

	if(GetDocument()->GetGridFilterMode()) {
		grid_data = GetDocument()->GetFilterGridData();
	}

	// 行列入れ替え表示中は、CGridCtrl::SortDataでは正しくSortできないので、
	// こちらでソートする
	grid_data->SortData(sort_col_no, sort_order, sort_col_cnt);

	GetDocument()->UpdateAllViews(NULL, UPD_GRID_CTRL);
}

void CPsqlgridView::OnSearchColumn() 
{
	CSearchDlg	dlg;

	dlg.m_title = "カラム名を検索";
	dlg.DoModal(this, WM_SEARCH_COLUMN, &m_search_data, FALSE);
}

void CPsqlgridView::OnUpdateValidCurPt(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_grid_ctrl.IsValidCurPt());
}

void CPsqlgridView::OnDataSetBlank() 
{
	POINT				pt;
	pt = *(GetDocument()->GetGridData()->get_cur_cell());
	if(!GetDocument()->GetGridData()->IsEditableCell(pt.y, pt.x)) return;

	m_grid_ctrl.UpdateCell(pt.y, pt.x, CEditablePgGridData::blank_str, -1);
}

void CPsqlgridView::HideSearchDlgs()
{
	if(CReplaceDlgSingleton::IsVisible()) {
		CReplaceDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
	if(CSearchDlgSingleton::IsVisible()) {
		CSearchDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
	if(CGridFilterDlgSingleton::IsVisible()) {
		CGridFilterDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
}

void CPsqlgridView::OnGrcolumnWidth()
{
	int col = m_grid_ctrl.GetSelectedCol();

	struct grid_data_select_area_st* selected_area = NULL;
	if (m_grid_ctrl.HaveSelected()) {
		selected_area = GetDocument()->GetGridData()->GetSelectArea();
	}

	CInputDlg	dlg;
	CString		val;

	if (selected_area == NULL) {
		val.Format(_T("%d"), m_grid_ctrl.GetDispColWidth(col));
	} else {
		val.Format(_T("%d"), m_grid_ctrl.GetDispColWidth(selected_area->pos1.x));
	}

	dlg.CreateDlg(AfxGetMainWnd(), _T("列幅"), val);
	if (dlg.DoModal() != IDOK) {
		return;
	}
	val = dlg.GetValue();
	int width = _ttoi(val);
	if (width < m_grid_ctrl.GetMinColWidth()) width = m_grid_ctrl.GetMinColWidth();
	if (width > m_grid_ctrl.GetMaxColWidth()) width = m_grid_ctrl.GetMaxColWidth();

	if (selected_area) {
		for (int i = selected_area->pos1.x; i <= selected_area->pos2.x; i++) {
			m_grid_ctrl.SetDispColWidth(i, width);
		}
	}
	else {
		m_grid_ctrl.SetDispColWidth(col, width);
	}

	if (IsWindowVisible()) {
		m_grid_ctrl.Update();
	}
}

void CPsqlgridView::OnUpdateGrcolumnWidth(CCmdUI* pCmdUI)
{
	if (GetDocument()->GetGridData()->Get_ColCnt()) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
}
