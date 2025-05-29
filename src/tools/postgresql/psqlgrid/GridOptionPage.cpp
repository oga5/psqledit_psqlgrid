/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
// GridOptionPage.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqlgrid.h"
#include "GridOptionPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void DrawColorBtn(LPDRAWITEMSTRUCT lpDrawItemStruct, COLORREF clr)
{
	CRect rect(lpDrawItemStruct->rcItem);

	::DrawEdge(lpDrawItemStruct->hDC, rect, EDGE_BUMP, BF_RECT);

	rect.DeflateRect(2, 2);
	::SetBkColor(lpDrawItemStruct->hDC, clr);
	::ExtTextOut(lpDrawItemStruct->hDC, 0, 0, ETO_OPAQUE, 
		rect, NULL, 0, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CGridOptionPage プロパティ ページ

IMPLEMENT_DYNCREATE(CGridOptionPage, CPropertyPage)

CGridOptionPage::CGridOptionPage() : CPropertyPage(CGridOptionPage::IDD)
, m_cell_padding_top(0)
, m_cell_padding_right(0)
, m_cell_padding_left(0)
, m_cell_padding_bottom(0)
{
	//{{AFX_DATA_INIT(CGridOptionPage)
	m_show_2byte_space = FALSE;
	m_show_space = FALSE;
	m_adjust_col_width_no_use_colname = FALSE;
	m_invert_select_text = FALSE;
	m_show_line_feed = FALSE;
	m_show_tab = FALSE;
	m_copy_escape_dblquote = FALSE;
	m_ime_caret_color = FALSE;
	m_end_key_like_excel = FALSE;
	m_null_text = _T("");
	//}}AFX_DATA_INIT
}

CGridOptionPage::~CGridOptionPage()
{
}

void CGridOptionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGridOptionPage)
	DDX_Check(pDX, IDC_CHECK_SHOW_2BYTE_SPACE, m_show_2byte_space);
	DDX_Check(pDX, IDC_CHECK_SHOW_SPACE, m_show_space);
	DDX_Check(pDX, IDC_CHECK_ADJUST_COL_WIDTH_NO_USE_COLNAME, m_adjust_col_width_no_use_colname);
	DDX_Check(pDX, IDC_CHECK_INVERT_SELECT_TEXT, m_invert_select_text);
	DDX_Check(pDX, IDC_CHECK_SHOW_LINE_FEED, m_show_line_feed);
	DDX_Check(pDX, IDC_CHECK_SHOW_TAB, m_show_tab);
	DDX_Check(pDX, IDC_CHECK_COPY_ESCAPE_DBLQUOTE, m_copy_escape_dblquote);
	DDX_Check(pDX, IDC_CHECK_IME_CARET_COLOR, m_ime_caret_color);
	DDX_Check(pDX, IDC_CHECK_END_KEY_LIKE_EXCEL, m_end_key_like_excel);
	DDX_Text(pDX, IDC_EDIT_NULL_TEXT, m_null_text);
	DDV_MaxChars(pDX, m_null_text, 10);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT_CELL_PADDING_TOP, m_cell_padding_top);
	DDX_Text(pDX, IDC_EDIT_CELL_PADDING_RIGHT, m_cell_padding_right);
	DDX_Text(pDX, IDC_EDIT_CELL_PADDING_LEFT, m_cell_padding_left);
	DDX_Text(pDX, IDC_EDIT_CELL_PADDING_BOTTOM, m_cell_padding_bottom);
}


BEGIN_MESSAGE_MAP(CGridOptionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGridOptionPage)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BTN_DEFAULT_COLOR, OnBtnDefaultColor)
	ON_BN_CLICKED(IDC_CHECK_SHOW_2BYTE_SPACE, OnCheckShow2byteSpace)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SPACE, OnCheckShowSpace)
	ON_BN_CLICKED(IDC_CHECK_ADJUST_COL_WIDTH_NO_USE_COLNAME, OnCheckAdjustColWidthNoUseColname)
	ON_BN_CLICKED(IDC_CHECK_INVERT_SELECT_TEXT, OnCheckInvertSelectText)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TAB, OnCheckShowTab)
	ON_BN_CLICKED(IDC_CHECK_SHOW_LINE_FEED, OnCheckShowLineFeed)
	ON_BN_CLICKED(IDC_CHECK_END_KEY_LIKE_EXCEL, OnCheckEndKeyLikeExcel)
	ON_EN_CHANGE(IDC_EDIT_NULL_TEXT, OnChangeEditNullText)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BTN_GRID_BG_COLOR, IDC_BTN_GRID_HEADER_LINE_COLOR, OnColorBtn)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_TOP, &CGridOptionPage::OnEnChangeEditCellPaddingTop)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_RIGHT, &CGridOptionPage::OnChangeEditCellPaddingRight)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_LEFT, &CGridOptionPage::OnChangeEditCellPaddingLeft)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_BOTTOM, &CGridOptionPage::OnChangeEditCellPaddingBottom)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridOptionPage メッセージ ハンドラ

BOOL CGridOptionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	UpdateData(FALSE);
	CreateGridCtrl();
	SetGridOption();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CGridOptionPage::CreateGridCtrl()
{
	RECT rect;
	GetClientRect(&rect);

	RECT button_rect;
	GetDlgItem(IDC_BTN_DEFAULT_COLOR)->GetWindowRect(&button_rect);
	ScreenToClient(&button_rect);

	m_grid_data.Init(3, 2);

	m_grid_data.SetColName(0, _T("CNAME1"));
	m_grid_data.SetColName(1, _T("CNAME2"));

	m_grid_data.UpdateCell(0, 0, _T("AAA"), 0);
	m_grid_data.UpdateCell(0, 1, _T("1"), 0);
	m_grid_data.UpdateCell(1, 0, _T("BBB\n"), 0);
	m_grid_data.UpdateCell(1, 1, _T("2"), 0);
	m_grid_data.UpdateCell(2, 0, _T("C C　C\tC"), 0);
	m_grid_data.UpdateCell(2, 1, _T(""), 0);
	m_grid_data.SetEditable(FALSE);

	m_grid_ctrl.SetGridData(&m_grid_data);
	m_grid_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER, 
		CRect(20, button_rect.bottom + 15, rect.right - 20, rect.bottom - 10), 
		this, NULL);
	m_grid_ctrl.SetGridStyle(GRS_COL_HEADER | GRS_ROW_HEADER | GRS_MULTI_SELECT | 
		GRS_SHOW_NULL_CELL | GRS_HIGHLIGHT_HEADER);
	m_grid_ctrl.SetFont(&g_font);
}

int CGridOptionPage::GetColorId(unsigned int ctrl_id)
{
	switch(ctrl_id) {
	case IDC_BTN_GRID_BG_COLOR:
		return GRID_BG_COLOR;
	case IDC_BTN_GRID_HEADER_BG_COLOR:
		return GRID_HEADER_BG_COLOR;
	case IDC_BTN_GRID_SELECT_COLOR:
		return GRID_SELECT_COLOR;
	case IDC_BTN_GRID_NULL_CELL_COLOR:
		return GRID_NULL_CELL_COLOR;
	case IDC_BTN_GRID_LINE_COLOR:
		return GRID_LINE_COLOR;
	case IDC_BTN_GRID_TEXT_COLOR:
		return GRID_TEXT_COLOR;
	case IDC_BTN_GRID_MARK_COLOR:
		return GRID_MARK_COLOR;
	case IDC_BTN_GRID_INSERT_COLOR:
		return GRID_INSERT_COLOR;
	case IDC_BTN_GRID_UPDATE_COLOR:
		return GRID_UPDATE_COLOR;
	case IDC_BTN_GRID_DELETE_COLOR:
		return GRID_DELETE_COLOR;
	case IDC_BTN_GRID_SEARCH_COLOR:
		return GRID_SEARCH_COLOR;
	case IDC_BTN_GRID_HEADER_LINE_COLOR:
		return GRID_HEADER_LINE_COLOR;
	}
	return -1;
}

void CGridOptionPage::OnColorBtn(unsigned int ctrl_id)
{
	int color_id = GetColorId(ctrl_id);
	if(color_id == -1) return;

	CColorDialog	dlg(m_color[color_id]);

	if(dlg.DoModal() != IDOK) return;

	m_color[color_id] = dlg.GetColor();
	SetGridOption();
	Invalidate();
}

void CGridOptionPage::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if(nIDCtl >= IDC_BTN_GRID_BG_COLOR && nIDCtl <= IDC_BTN_GRID_HEADER_LINE_COLOR) {
		int color_id = GetColorId(nIDCtl);
		if(color_id != -1) DrawColorBtn(lpDrawItemStruct, m_color[color_id]);
	}
	
	CPropertyPage::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CGridOptionPage::OnBtnDefaultColor() 
{
	m_color[GRID_BG_COLOR] = RGB(255, 255, 255);
	m_color[GRID_HEADER_BG_COLOR] = RGB(230, 230, 255);
	m_color[GRID_LINE_COLOR] = RGB(200, 200, 250);
	m_color[GRID_SELECT_COLOR] = RGB(200, 200, 220);
	m_color[GRID_DELETE_COLOR] = RGB(150, 150, 150);
	m_color[GRID_INSERT_COLOR] = RGB(220, 220, 255);
	m_color[GRID_UPDATE_COLOR] = RGB(255, 220, 220);
	m_color[GRID_UNEDITABLE_COLOR] = RGB(240, 240, 240);
	m_color[GRID_TEXT_COLOR] = RGB(0, 0, 0);
	m_color[GRID_NULL_CELL_COLOR] = RGB(245, 245, 245);
	m_color[GRID_MARK_COLOR] = RGB(0, 150, 150);
	m_color[GRID_DELETE_COLOR] = RGB(150, 150, 150);
	m_color[GRID_INSERT_COLOR] = RGB(220, 220, 255);
	m_color[GRID_UPDATE_COLOR] = RGB(255, 220, 220);
	m_color[GRID_HEADER_LINE_COLOR] = RGB(100, 100, 100);

	SetGridOption();
	Invalidate();
}

void CGridOptionPage::OnCheckShow2byteSpace() 
{
	SetGridOption();
}

void CGridOptionPage::OnCheckShowSpace() 
{
	SetGridOption();
}

void CGridOptionPage::SetGridOption()
{
	UpdateData(TRUE);

	int		i;
	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		m_grid_ctrl.SetColor(i, m_color[i]);
	}

	int grid_option = m_grid_ctrl.GetGridStyle();
	if(m_color[GRID_BG_COLOR] != m_color[GRID_NULL_CELL_COLOR]) {
		grid_option |= GRS_SHOW_NULL_CELL;
	} else {
		grid_option &= ~(GRS_SHOW_NULL_CELL);
	}

	if(m_show_space) {
		grid_option |= GRS_SHOW_SPACE;
	} else {
		grid_option &= ~(GRS_SHOW_SPACE);
	}
	if(m_show_2byte_space) {
		grid_option |= GRS_SHOW_2BYTE_SPACE;
	} else {
		grid_option &= ~(GRS_SHOW_2BYTE_SPACE);
	}
	if(m_show_tab) {
		grid_option |= GRS_SHOW_TAB;
	} else {
		grid_option &= ~(GRS_SHOW_TAB);
	}
	if(m_show_line_feed) {
		grid_option |= GRS_SHOW_LINE_FEED;
	} else {
		grid_option &= ~(GRS_SHOW_LINE_FEED);
	}
	if(m_adjust_col_width_no_use_colname) {
		grid_option |= GRS_ADJUST_COL_WIDTH_NO_USE_COL_NAME;
	} else {
		grid_option &= ~(GRS_ADJUST_COL_WIDTH_NO_USE_COL_NAME);
	}
	if(m_invert_select_text) {
		grid_option |= GRS_INVERT_SELECT_TEXT;
	} else {
		grid_option &= ~(GRS_INVERT_SELECT_TEXT);
	}
	if(m_end_key_like_excel) {
		grid_option |= GRS_END_KEY_LIKE_EXCEL;
	} else {
		grid_option &= ~(GRS_END_KEY_LIKE_EXCEL);
	}

	m_grid_ctrl.SetNullText(m_null_text);
	m_grid_ctrl.SetCellPadding(m_cell_padding_top, m_cell_padding_bottom, m_cell_padding_left, m_cell_padding_right);
	m_grid_ctrl.SetGridStyle(grid_option);

	m_grid_ctrl.RedrawWindow();
}

void CGridOptionPage::OnCheckAdjustColWidthNoUseColname() 
{
	SetGridOption();	
}

void CGridOptionPage::OnCheckInvertSelectText() 
{
	SetGridOption();	
}

void CGridOptionPage::OnCheckShowTab() 
{
	SetGridOption();
}

void CGridOptionPage::OnCheckShowLineFeed() 
{
	SetGridOption();
}

void CGridOptionPage::OnCheckEndKeyLikeExcel() 
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditNullText() 
{
	SetGridOption();
}

void CGridOptionPage::OnEnChangeEditCellPaddingTop()
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditCellPaddingRight()
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditCellPaddingLeft()
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditCellPaddingBottom()
{
	SetGridOption();
}
