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
#include "psqledit.h"
#include "GridOptionPage.h"

#include "EditorOptionPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridOptionPage プロパティ ページ

IMPLEMENT_DYNCREATE(CGridOptionPage, CPropertyPage)

CGridOptionPage::CGridOptionPage() : CPropertyPage(CGridOptionPage::IDD)
, m_col_header_dbl_clk_paste(FALSE)
, m_cell_padding_top(0)
, m_cell_padding_right(0)
, m_cell_padding_left(0)
, m_cell_padding_bottom(0)
{
	//{{AFX_DATA_INIT(CGridOptionPage)
	m_show_2byte_space = FALSE;
	m_show_space = FALSE;
	m_show_line_feed = FALSE;
	m_show_tab = FALSE;
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
	DDX_Check(pDX, IDC_CHECK_SHOW_LINE_FEED, m_show_line_feed);
	DDX_Check(pDX, IDC_CHECK_SHOW_TAB, m_show_tab);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_COL_HEADER_DBL_CLK_PASTE, m_col_header_dbl_clk_paste);
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
	ON_BN_CLICKED(IDC_CHECK_SHOW_LINE_FEED, OnCheckShowLineFeed)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TAB, OnCheckShowTab)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BTN_GRID_BG_COLOR, IDC_BTN_GRID_HEADER_LINE_COLOR, OnColorBtn)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_BOTTOM, &CGridOptionPage::OnChangeEditCellPaddingBottom)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_LEFT, &CGridOptionPage::OnChangeEditCellPaddingLeft)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_RIGHT, &CGridOptionPage::OnChangeEditCellPaddingRight)
	ON_EN_CHANGE(IDC_EDIT_CELL_PADDING_TOP, &CGridOptionPage::OnChangeEditCellPaddingTop)
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
	m_grid_data.UpdateCell(1, 0, _T("BBB"), 0);
	m_grid_data.UpdateCell(1, 1, _T("2"), 0);
	m_grid_data.UpdateCell(2, 0, _T("C C　C"), 0);
	m_grid_data.UpdateCell(2, 1, _T(""), 0);
	m_grid_data.SetEditable(FALSE);

	m_grid_ctrl.SetGridData(&m_grid_data);
	m_grid_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER, 
		CRect(20, button_rect.bottom + 15, rect.right - 20, rect.bottom - 10), 
		this, NULL);
	m_grid_ctrl.SetGridStyle(GRS_COL_HEADER | GRS_ROW_HEADER | GRS_MULTI_SELECT | GRS_SHOW_NULL_CELL | GRS_HIGHLIGHT_HEADER);
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
	m_color[GRID_SEARCH_COLOR] = RGB(255, 255, 50);
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
	if(m_show_line_feed) {
		grid_option |= GRS_SHOW_LINE_FEED;
	} else {
		grid_option &= ~(GRS_SHOW_LINE_FEED);
	}
	if(m_show_tab) {
		grid_option |= GRS_SHOW_TAB;
	} else {
		grid_option &= ~(GRS_SHOW_TAB);
	}

	m_grid_ctrl.SetCellPadding(m_cell_padding_top, m_cell_padding_bottom, m_cell_padding_left, m_cell_padding_right);

	m_grid_ctrl.SetGridStyle(grid_option);

	m_grid_ctrl.RedrawWindow();
}

void CGridOptionPage::OnCheckShowLineFeed() 
{
	SetGridOption();
}

void CGridOptionPage::OnCheckShowTab() 
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditCellPaddingBottom()
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditCellPaddingLeft()
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditCellPaddingRight()
{
	SetGridOption();
}

void CGridOptionPage::OnChangeEditCellPaddingTop()
{
	SetGridOption();
}
