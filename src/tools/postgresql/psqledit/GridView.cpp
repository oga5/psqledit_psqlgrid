/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // GridView.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "GridView.h"

#include "GridSortDlg.h"
#include "GridFilterDlg.h"
#include "MainFrm.h"

#include "common.h"
#include "InputDlg.h"

#define MAX_SORT_COLUMN		5

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridView

IMPLEMENT_DYNCREATE(CGridView, CView)

CGridView::CGridView()
{
}

CGridView::~CGridView()
{
}


BEGIN_MESSAGE_MAP(CGridView, CView)
	//{{AFX_MSG_MAP(CGridView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_SEARCH, OnSearch)
	ON_COMMAND(ID_SEARCH_NEXT, OnSearchNext)
	ON_COMMAND(ID_SEARCH_PREV, OnSearchPrev)
	ON_COMMAND(ID_SELECT_ALL, OnSelectAll)
	ON_COMMAND(ID_GRID_COPY_CSV, OnGridCopyCsv)
	ON_COMMAND(ID_GRID_COPY_CSV_CNAME, OnGridCopyCsvCname)
	ON_COMMAND(ID_GRID_COPY_TAB, OnGridCopyTab)
	ON_COMMAND(ID_GRID_COPY_TAB_CNAME, OnGridCopyTabCname)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_GRID_COPY_FIX_LEN, OnGridCopyFixLen)
	ON_COMMAND(ID_GRID_COPY_FIX_LEN_CNAME, OnGridCopyFixLenCname)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_CLEAR_SEARCH_TEXT, OnClearSearchText)
	ON_COMMAND(ID_GRID_SORT, OnGridSort)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_GRID_COPY_TAB, ID_GRID_COPY_FIX_LEN_CNAME, OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_INDENT, ID_EDIT_INDENT_REV, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TO_LOWER, ID_TO_UPPER, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_REVERSE_ROWS, ID_EDIT_SORT_ROWS_DESC, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SET_BOOK_MARK, ID_SET_SQL_BOOKMARK, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CONVERT_TO_C, ID_CONVERT_FROM_VB, OnUpdateDisableMenu)
	ON_UPDATE_COMMAND_UI(ID_GRID_CREATE_INSERT_SQL, &CGridView::OnUpdateGrcreateInsertSql)
	ON_COMMAND(ID_GRID_CREATE_INSERT_SQL, &CGridView::OnGrcreateInsertSql)
	ON_COMMAND(ID_SEARCH_COLUMN, &CGridView::OnSearchColumn)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_EDIT_COPY_COLUMN_NAME, &CGridView::OnEditCopyColumnName)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY_COLUMN_NAME, &CGridView::OnUpdateEditCopyColumnName)
	ON_COMMAND(ID_SHOW_GRID_DATA, &CGridView::OnShowGridData)
	ON_UPDATE_COMMAND_UI(ID_SHOW_GRID_DATA, &CGridView::OnUpdateShowGridData)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CGridView::OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_CUT, &CGridView::OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CGridView::OnUpdateEditCut)
	ON_COMMAND(ID_GRID_COLUMN_WIDTH, &CGridView::OnGrcolumnWidth)
	ON_UPDATE_COMMAND_UI(ID_GRID_COLUMN_WIDTH, &CGridView::OnUpdateGrcolumnWidth)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridView 描画

void CGridView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: この位置に描画用のコードを追加してください
}

/////////////////////////////////////////////////////////////////////////////
// CGridView 診断

#ifdef _DEBUG
void CGridView::AssertValid() const
{
	CView::AssertValid();
}

void CGridView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGridView メッセージ ハンドラ


int CGridView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	GetDocument()->GetOrigGridData()->SetDataset(GetDocument()->GetDataset());
	m_grid_ctrl.SetGridData(GetDocument()->GetGridData());

	m_grid_ctrl.SetSplitterMode(TRUE);

	m_grid_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD, 
		CRect(0, 0, lpCreateStruct->cx, lpCreateStruct->cy), this, NULL);
	m_grid_ctrl.SetGridStyle(GRS_COL_HEADER | GRS_ROW_HEADER | GRS_MULTI_SELECT | GRS_HIGHLIGHT_HEADER);

	m_grid_ctrl.SetHWheelScrollEnable(TRUE);
	m_grid_ctrl.SetFont(&g_font);

	SetGridOption();

	return 0;
}

void CGridView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	m_grid_ctrl.MoveWindow(0, 0, cx, cy);
}

void CGridView::SetGridMsg(const TCHAR* msg)
{
	m_grid_msg = msg;
}

void CGridView::SetGridSql(const TCHAR *sql)
{
	if(sql) {
		m_grid_sql = sql;
	} else {
		m_grid_sql.Empty();
	}
}

void CGridView::SetHeaderStyle()
{
	if(!GetParent()->IsKindOf(RUNTIME_CLASS(CSplitterWnd))) return;
	CSplitterWnd *sp = (CSplitterWnd *)GetParent();

	int grid_style = m_grid_ctrl.GetGridStyle();
	grid_style &= ~(GRS_COL_HEADER);
	grid_style &= ~(GRS_ROW_HEADER);

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

void CGridView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	switch(lHint) {
	case UPD_GRID_DATASET:
		GetDocument()->GetOrigGridData()->SetDataset(GetDocument()->GetDataset());
		break;
	case UPD_SET_GRID_MSG:
		SetGridMsg((const TCHAR*)pHint);
		break;
	case UPD_GRID_SQL:
		SetGridSql((const TCHAR *)pHint);
		break;
	case UPD_FONT:
		m_grid_ctrl.SetFont((CFont *)pHint);
		break;
	case UPD_REDRAW:
		m_grid_ctrl.Update();
		break;
	case UPD_GRID_CTRL:
		if(IsWindowVisible()) {
			m_grid_ctrl.Update();
		}
		break;
	case UPD_ACTIVE_DOC:
		GetDocument()->CalcGridSelectedData();
		SetStatusBarInfo();
		break;
	case UPD_SWITCH:
		m_grid_ctrl.InitSelectedCell();
		m_grid_ctrl.Update();
		GetDocument()->CalcGridSelectedData();
		SetStatusBarInfo();
		if(CSearchDlgSingleton::IsVisible() && CSearchDlgSingleton::GetDlg().GetWnd() == this) {
			CSearchDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
		}
		break;
	case UPD_CALC_GRID_SELECTED_DATA:
		GetDocument()->CalcGridSelectedData();
		break;
	case UPD_ADJUST_COL_WIDTH:
		AdjustAllColWidth();
		break;
	case UPD_EQUAL_COL_WIDTH:
		EqualAllColWidth();
		break;
	case UPD_CHANGE_EDITOR_OPTION:
		SetGridOption();
		break;
	case UPD_GRID_SWAP_ROW_COL:
		SwapRowCol();
		break;
	case UPD_GRID_FILTER_DATA_ON:
		GridFilterOn();
		break;
	case UPD_GRID_FILTER_DATA_OFF:
		GridFilterOff();
		break;
	case UPD_SET_HEADER_STYLE:
		SetHeaderStyle();
		break;
	}
}

void CGridView::OnEditCopy() 
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.CopyFromKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.CopyFromKeyword();
		return;
	}
	m_grid_ctrl.Copy();
}

void CGridView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	
	m_grid_ctrl.SetFocus();	
}

void CGridView::OnSearch() 
{
//	CSearchDlg		dlg;
//	dlg.DoModal(this, WM_SEARCH_TEXT, &g_search_data, FALSE);
	GetDocument()->HideSearchDlgs();
	CSearchDlgSingleton::GetDlg().m_title = _T("検索(Grid)");
	CSearchDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_TEXT, &g_search_data, FALSE);
}

void CGridView::OnSearchNext() 
{
	BOOL b_looped;
	int ret_v = m_grid_ctrl.SearchNext2(g_search_data.m_search_text.GetBuffer(0),
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, 1, b_looped);
}

void CGridView::OnSearchPrev() 
{
	BOOL b_looped;
	int ret_v = m_grid_ctrl.SearchPrev2(g_search_data.m_search_text.GetBuffer(0),
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii, g_search_data.m_regexp, &b_looped);
	SearchMsg(ret_v, -1, b_looped);
}

void CGridView::CellChanged(int zero_base_x, int zero_base_y)
{
	// 画面に表示する位置は1から始める
	g_cur_pos.x = zero_base_x + 1;
	g_cur_pos.y = zero_base_y + 1;

	if(GetDocument()->IsShowDataDialogVisible()) {
		OnShowGridData();
	}
}

LRESULT CGridView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case GC_WM_CHANGE_CELL_POS:
		CellChanged((int)wParam, (int)lParam);
		break;
	case GC_WM_CHANGE_SELECT_AREA:
		GetDocument()->CalcGridSelectedData();
		break;
	case GC_WM_DBL_CLK_COL_HEADER:
		if(g_option.grid.col_header_dbl_clk_paste) {
			CUndoSetMode undo_obj(GetDocument()->GetSqlData()->get_undo());
			CString paste_str;

			for(int col = (int)wParam; col <= (int)lParam; col++) {
				const TCHAR *col_name;
				if(!GetDocument()->GetGridSwapRowColMode()) {
					col_name = GetDocument()->GetGridData()->Get_ColName(col);
				} else {
					col_name = GetDocument()->GetGridData()->GetRowHeader(col);
				}
				make_object_name(&paste_str, col_name, g_option.text_editor.copy_lower_name);
				GetDocument()->UpdateAllViews(this, UPD_PASTE_OBJECT_NAME, (CObject *)paste_str.GetBuffer(0));
			}
		}
		break;
	case WM_SEARCH_TEXT:
		{
			BOOL b_looped;
			int ret_v = m_grid_ctrl.SearchText2(g_search_data.m_search_text.GetBuffer(0),
				g_search_data.m_dir, g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, TRUE, &b_looped);
			SearchMsg(ret_v, g_search_data.m_dir, b_looped);
		}
		break;
	case WM_SEARCH_COLUMN:
		{
			BOOL b_looped;
			int ret_v = m_grid_ctrl.SearchColumn2(g_search_data.m_search_text.GetBuffer(0),
				g_search_data.m_dir, g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
				g_search_data.m_regexp, TRUE, &b_looped);
			SearchMsg(ret_v, g_search_data.m_dir, b_looped);
			return ret_v;
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
	return CView::WindowProc(message, wParam, lParam);
}

void CGridView::OnSelectAll() 
{
	if(g_object_bar.isKeywordActive()) {
		g_object_bar.SelectAllKeyword();
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		g_object_detail_bar.SelectAllKeyword();
		return;
	}
	m_grid_ctrl.SelectAll();
}

void CGridView::OnGridCopyCsv() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_CSV);
}

void CGridView::OnGridCopyCsvCname() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_CSV_CNAME);
}

void CGridView::OnGridCopyTab() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_TAB);
}

void CGridView::OnGridCopyTabCname() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_TAB_CNAME);
}

void CGridView::OnGridCopyFixLen() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_FIX_LEN);
}

void CGridView::OnGridCopyFixLenCname() 
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_FIX_LEN_CNAME);
}

void CGridView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	if(g_object_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}
	if(g_object_detail_bar.isKeywordActive()) {
		pCmdUI->Enable(TRUE);
		return;
	}

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


void CGridView::OnUpdateDisableMenu(CCmdUI* pCmdUI) 
{
	if(pCmdUI->m_pSubMenu != NULL) {
        // ポップアップ メニューを選択不可にする
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		return;
	}
	pCmdUI->Enable(FALSE);
}

void CGridView::AdjustAllColWidth() 
{
	if(IsWindowVisible() == FALSE) return;
	m_grid_ctrl.AdjustAllColWidth(TRUE, FALSE);
}

void CGridView::EqualAllColWidth() 
{
	if(IsWindowVisible() == FALSE) return;
	m_grid_ctrl.EqualAllColWidth();
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

void CGridView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	CMenu	menu;
	menu.LoadMenu(IDR_GRID_MENU);

	if(m_grid_ctrl.IsValidCurPt() && !m_grid_ctrl.HaveSelected()) {
		POINT cell_pt;

		cell_pt = *(GetDocument()->GetGridData()->get_cur_cell());
		if(cell_pt.y >= 0 && cell_pt.y < GetDocument()->GetGridData()->Get_RowCnt()) {
			int data_len = (int)_tcslen(GetDocument()->GetGridData()->Get_ColData(cell_pt.y, cell_pt.x)) * sizeof(TCHAR);
			CString menu_name;
			menu_name.Format(_T("データ参照(%sbyte)"), GetDataSize(data_len).GetString());
			menu.GetSubMenu(0)->InsertMenu(0, MF_STRING | MF_BYPOSITION, ID_SHOW_GRID_DATA, menu_name);
			menu.GetSubMenu(0)->InsertMenu(1, MF_SEPARATOR | MF_BYPOSITION);
		}
	}

	if(GetDocument()->GetOrigGridData()->Get_RowCnt() > 0) {
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
	menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
		GetParentFrame()->GetParentFrame());

	// メニュー削除
	menu.DestroyMenu();	
}

void CGridView::SearchMsg(int ret_v, int dir, BOOL b_looped)
{
	GetParentFrame()->SetMessageText(MakeSearchMsg(ret_v, dir, b_looped));
}

void CGridView::SetGridOption()
{
	int		i;
	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		m_grid_ctrl.SetColor(i, g_option.grid.color[i]);
	}

	BOOL b_update_window = FALSE;
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
	if(g_option.grid.show_line_feed) {
		grid_option |= GRS_SHOW_LINE_FEED;
	} else {
		grid_option &= ~(GRS_SHOW_LINE_FEED);
	}
	if(g_option.grid.show_tab) {
		grid_option |= GRS_SHOW_TAB;
	} else {
		grid_option &= ~(GRS_SHOW_TAB);
	}

	if(GetDocument()->GetGridSwapRowColMode()) {
		if(!(grid_option & GRS_SWAP_ROW_COL_MODE)) b_update_window = TRUE;
		grid_option |= GRS_SWAP_ROW_COL_MODE;
	} else {
		if(grid_option & GRS_SWAP_ROW_COL_MODE) b_update_window = TRUE;
		grid_option &= ~(GRS_SWAP_ROW_COL_MODE);
	}

	m_grid_ctrl.SetCellPadding(g_option.grid.cell_padding_top, g_option.grid.cell_padding_bottom,
		g_option.grid.cell_padding_left, g_option.grid.cell_padding_right);
	m_grid_ctrl.SetGridStyle(grid_option, b_update_window);

	m_grid_ctrl.RedrawWindow();
}

void CGridView::OnClearSearchText() 
{
	m_grid_ctrl.ClearSearchText();
}

void CGridView::SwapRowCol()
{
	// セルの現在位置や選択範囲を保持したまま、グリッドの行列を入れ替える
	int row = m_grid_ctrl.GetSelectedRow();
	int col = m_grid_ctrl.GetSelectedCol();

	struct grid_data_select_area_st *selected_area = NULL;
	if(m_grid_ctrl.HaveSelected()) {
		selected_area = GetDocument()->GetGridData()->GetSelectArea();
	}

	GetDocument()->ToggleGridSwapRowColMode();

	m_grid_ctrl.SetGridData(GetDocument()->GetGridData());
	GetDocument()->CalcGridSelectedData();
	SetGridOption();

	if(selected_area) {
		// x, y入れ替えて初期化
		CPoint pt(row, col);
		CPoint pos1(selected_area->pos1.y, selected_area->pos1.x);
		CPoint pos2(selected_area->pos2.y, selected_area->pos2.x);
		m_grid_ctrl.SelectArea(pt, pos1, pos2);
	} else {
		m_grid_ctrl.SetCell(row, col);
	}
}

void CGridView::GridFilterOn()
{
	CGridFilterDlg dlg;

	// FIXME: Filter時のアクティブセルについて、Filter実行後も現在のセルが表示されるのであれば、そこをアクティブにしたい
	// 指定した行が表示されているかどうか、バイナリサーチでチェックできるか？ソートしてからフィルタなどの操作もあるので工夫がいるかも
	int col;
	if(!GetDocument()->GetGridSwapRowColMode()) {
		col = m_grid_ctrl.GetSelectedCol();
	} else {
		col = m_grid_ctrl.GetSelectedRow();
	}

//	dlg.DoModal(this, WM_GRID_FILTER_RUN, &g_search_data, GetDocument()->GetOrigGridData(), col, TRUE);
	GetDocument()->HideSearchDlgs();
	CGridFilterDlgSingleton::GetDlg().ShowDialog(this, &g_search_data,
		GetDocument()->GetOrigGridData(), col, FALSE);
}

int CGridView::GridFilterRun(int grid_filter_col_no)
{
	int col = grid_filter_col_no;

	CGridData* grid_data = GetDocument()->GetOrigGridData();

	CString filter_msg;
	int find_cnt = 0;

	int ret_v = grid_data->FilterData(grid_filter_col_no, g_search_data.m_search_text.GetBuffer(0),
		g_search_data.m_distinct_lwr_upr, g_search_data.m_distinct_width_ascii,
		g_search_data.m_regexp, &find_cnt, &filter_msg);
	if(ret_v != 0) {
		MessageBox(filter_msg, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	PostGridFilterOnOff();
	return 0;
}

void CGridView::GridFilterOff()
{
	if(!GetDocument()->GetGridFilterMode()) return;

	CGridData* grid_data = GetDocument()->GetOrigGridData();
	grid_data->FilterOff();
	PostGridFilterOnOff();
}

void CGridView::PostGridFilterOnOff()
{
	int col = GetDocument()->GetOrigGridData()->get_cur_col();

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

void CGridView::SetStatusBarInfo()
{
	GetDocument()->CalcGridSelectedData();

	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();

	CString msg;
	if(GetDocument()->GetViewKind() == GRID_VIEW) {
		msg = m_grid_msg;

		if(GetDocument()->GetGridFilterMode()) {
			msg += _T(" Filter中");
		}
	}
	pMainFrame->SetIdleMessage(msg);
	pMainFrame->SetMessageText(msg);
}

void CGridView::OnGridSort() 
{
	int		sort_col_no[MAX_SORT_COLUMN];
	int		sort_order[MAX_SORT_COLUMN];
	int		sort_col_cnt;

	// 行列入れ替え表示中でも、元のGridDataを使う
	CGridData *grid_data = GetDocument()->GetOrigGridData();

	InitSortData(sort_col_no, sort_order, sort_col_cnt, 1);

	CGridSortDlg	dlg;
	dlg.InitData(grid_data, sort_col_no, sort_order, &sort_col_cnt);

	if(dlg.DoModal() != IDOK) return;

	SortData(sort_col_no, sort_order, sort_col_cnt);
}

void CGridView::InitSortData(int *sort_col_no, int *sort_order, int &sort_col_cnt, int order)
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

void CGridView::SortData(int *sort_col_no, int *sort_order, int sort_col_cnt)
{
	CWaitCursor wait_cursor;
	CGridData *grid_data = GetDocument()->GetOrigGridData();

	if(GetDocument()->GetGridFilterMode()) {
		grid_data = GetDocument()->GetFilterGridData();
	}

	// 行列入れ替え表示中は、CGridCtrl::SortDataでは正しくSortできないので、
	// こちらでソートする
	grid_data->SortData(sort_col_no, sort_order, sort_col_cnt);

	GetDocument()->UpdateAllViews(NULL, UPD_GRID_CTRL);
}


void CGridView::OnUpdateGrcreateInsertSql(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetGridData()->Get_RowCnt() > 0);
}


void CGridView::OnGrcreateInsertSql()
{
	TCHAR			msg_buf[1024] = _T("");
	CString			table_name1;
	CString			table_name2 = _T("");
	CString			schema_name = _T("");
	CString			alias_name;
	CString			where_clause;
	CStringArray	column_array;

	HPgDataset		tab_def_dataset = NULL;

	// テーブル名とwhere句を取得する
	if(!GetDocument()->ParseSelectSQL(m_grid_sql, msg_buf, 
		schema_name, table_name1, table_name2, alias_name, where_clause)) {
		MessageBox(msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	CGridData *grid_data = GetDocument()->GetGridData();
	int y_start, y_end, x_start, x_end;

	if(!GetDocument()->GetGridSwapRowColMode()) {
		if(m_grid_ctrl.HaveSelected()) {
			y_start = grid_data->GetSelectArea()->pos1.y;
			y_end = grid_data->GetSelectArea()->pos2.y;
		} else {
			y_start = grid_data->get_cur_row();
			y_end = grid_data->get_cur_row();
		}
		x_start = 0;
		x_end = grid_data->Get_ColCnt() - 1;
	} else {
		if(m_grid_ctrl.HaveSelected()) {
			y_start = grid_data->GetSelectArea()->pos1.x;
			y_end = grid_data->GetSelectArea()->pos2.x;
		} else {
			y_start = grid_data->get_cur_col();
			y_end = grid_data->get_cur_col();
		}
		x_start = 0;
		x_end = grid_data->Get_RowCnt() - 1;
	}

	CString column_list;

	CEditData	edit_data;

	int x;
	for(x = 0; x <= x_end; x++) {
		if(x != 0) column_list += ',';

		const TCHAR *cname;
		if(!GetDocument()->GetGridSwapRowColMode()) {
			cname = grid_data->Get_ColName(x);
		} else {
			cname = grid_data->GetRowHeader(x);
		}

		CString col_name;
		make_object_name(&col_name, cname, g_option.text_editor.copy_lower_name);
		column_list += col_name;
	}

	int y;
	for(y = y_start; y <= y_end; y++) {
		CString value_data;

		if(!GetDocument()->GetGridSwapRowColMode()) {
			value_data = m_grid_ctrl.GetCopyString(GR_COPY_FORMAT_SQL,
				y, y, x_start, x_end, GR_COPY_OPTION_USE_NULL);
		} else {
			value_data = m_grid_ctrl.GetCopyString(GR_COPY_FORMAT_SQL,
				x_start, x_end, y, y, GR_COPY_OPTION_USE_NULL);
		}
		
		CString sql;
		sql.Format(_T("insert into %s (%s)\nvalues (%s);\n"),
			table_name1.GetString(), column_list.GetString(), value_data.GetString());

		edit_data.paste_no_undo(sql);
	}

	CString all_text = edit_data.get_all_text();
	const TCHAR *paste_str = all_text.GetBuffer(0);
	GetDocument()->UpdateAllViews(this, UPD_PASTE_SQL, (CObject *)paste_str);
	SetFocus();
}

void CGridView::OnSearchColumn()
{
//	CSearchDlg	dlg;
//	dlg.m_title = _T("カラム名を検索");
//	dlg.DoModal(this, WM_SEARCH_COLUMN, &g_search_data, FALSE);
	GetDocument()->HideSearchDlgs();
	CSearchDlgSingleton::GetDlg().m_title = _T("カラム名を検索");
	CSearchDlgSingleton::GetDlg().ShowDialog(this, WM_SEARCH_COLUMN, &g_search_data, FALSE);
}

void CGridView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	SetHeaderStyle();
}

BOOL CGridView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll)
{
	return TRUE;
}

void CGridView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_grid_ctrl.VScroll(nSBCode);
}

void CGridView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	m_grid_ctrl.HScroll(nSBCode);
}

void CGridView::OnEditCopyColumnName()
{
	m_grid_ctrl.Copy(GR_COPY_FORMAT_COLUMN_NAME);
}

void CGridView::OnUpdateEditCopyColumnName(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_grid_ctrl.CanCopyColumnName());
}

POINT CGridView::GetCurCellOnDataset()
{
	POINT cell_pt = *(GetDocument()->GetGridData()->get_cur_cell());

	if(GetDocument()->GetGridSwapRowColMode()) {
		// x, yを入れ替える
		int tmp = cell_pt.y;
		cell_pt.y = cell_pt.x;
		cell_pt.x = tmp;
	}

	if(GetDocument()->GetGridFilterMode()) {
		cell_pt = GetDocument()->GetFilterGridData()->get_dataset_point(&cell_pt);
	}

	return cell_pt;
}

void CGridView::OnShowGridData()
{
	if(!m_grid_ctrl.IsValidCurPt()) return;

	POINT cell_pt = GetCurCellOnDataset();

	GetDocument()->ShowGridData(cell_pt);
}


void CGridView::OnUpdateShowGridData(CCmdUI* pCmdUI)
{
	if(!m_grid_ctrl.HaveSelected() && m_grid_ctrl.IsValidCurPt()) {
		POINT cell_pt = *(GetDocument()->GetGridData()->get_cur_cell());

		pCmdUI->Enable(!GetDocument()->GetGridData()->IsColDataNull(cell_pt.y, cell_pt.x));
		return;
	}
	pCmdUI->Enable(FALSE);
}

void CGridView::OnEditPaste()
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

void CGridView::OnUpdateEditPaste(CCmdUI* pCmdUI)
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

void CGridView::OnEditCut()
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

void CGridView::OnUpdateEditCut(CCmdUI* pCmdUI)
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

void CGridView::OnGrcolumnWidth()
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

void CGridView::OnUpdateGrcolumnWidth(CCmdUI* pCmdUI)
{
	if (GetDocument()->GetOrigGridData()->Get_ColCnt()) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
}
