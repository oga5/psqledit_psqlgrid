/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // EditorOptionPage.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "EditorOptionPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void DrawColorBtn(LPDRAWITEMSTRUCT lpDrawItemStruct, COLORREF clr)
{
	CRect rect(lpDrawItemStruct->rcItem);

	::DrawEdge(lpDrawItemStruct->hDC, rect, EDGE_BUMP, BF_RECT);

	rect.DeflateRect(2, 2);
	::SetBkColor(lpDrawItemStruct->hDC, clr);
	::ExtTextOut(lpDrawItemStruct->hDC, 0, 0, ETO_OPAQUE, 
		rect, NULL, 0, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage プロパティ ページ

IMPLEMENT_DYNCREATE(CEditorOptionPage, CPropertyPage)

CEditorOptionPage::CEditorOptionPage() : CPropertyPage(CEditorOptionPage::IDD)
, m_show_brackets_bold(FALSE)
{
	//{{AFX_DATA_INIT(CEditorOptionPage)
	m_show_line_feed = FALSE;
	m_show_tab = FALSE;
	m_tabstop = 0;
	m_show_row_num = FALSE;
	m_show_col_num = FALSE;
	m_char_space = 0;
	m_left_space = 0;
	m_row_space = 0;
	m_top_space = 0;
	m_show_2byte_space = FALSE;
	m_line_len = 0;
	m_show_row_line = FALSE;
	m_show_edit_row = FALSE;
	m_show_space = FALSE;
	m_ime_caret_color = FALSE;
	//}}AFX_DATA_INIT
}

CEditorOptionPage::~CEditorOptionPage()
{
}

void CEditorOptionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorOptionPage)
	DDX_Check(pDX, IDC_CHECK_SHOW_LINE_FEED, m_show_line_feed);
	DDX_Check(pDX, IDC_CHECK_SHOW_TAB, m_show_tab);
	DDX_Text(pDX, IDC_EDIT_TABSTOP, m_tabstop);
	DDX_Check(pDX, IDC_CHECK_SHOW_ROW_NUM, m_show_row_num);
	DDX_Check(pDX, IDC_CHECK_SHOW_COL_NUM, m_show_col_num);
	DDX_Text(pDX, IDC_EDIT_CHAR_SPACE, m_char_space);
	DDX_Text(pDX, IDC_EDIT_LEFT_SPACE, m_left_space);
	DDX_Text(pDX, IDC_EDIT_ROW_SPACE, m_row_space);
	DDX_Text(pDX, IDC_EDIT_TOP_SPACE, m_top_space);
	DDX_Check(pDX, IDC_CHECK_SHOW_2BYTE_SPACE, m_show_2byte_space);
	DDX_Text(pDX, IDC_EDIT_LINE_LEN, m_line_len);
	DDX_Check(pDX, IDC_CHECK_SHOW_ROW_LINE, m_show_row_line);
	DDX_Check(pDX, IDC_CHECK_SHOW_EDIT_ROW, m_show_edit_row);
	DDX_Check(pDX, IDC_CHECK_SHOW_SPACE, m_show_space);
	DDX_Check(pDX, IDC_CHECK_IME_CARET_COLOR, m_ime_caret_color);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_SHOW_BRACKETS_BOLD, m_show_brackets_bold);
}


BEGIN_MESSAGE_MAP(CEditorOptionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CEditorOptionPage)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_CHECK_SHOW_LINE_FEED, OnCheckShowLineFeed)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TAB, OnCheckShowTab)
	ON_EN_CHANGE(IDC_EDIT_TABSTOP, OnChangeEditTabstop)
	ON_BN_CLICKED(IDC_BTN_TEXT_COLOR, OnBtnTextColor)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_BTN_PEN_COLOR, OnBtnPenColor)
	ON_BN_CLICKED(IDC_BTN_DEFAULT_COLOR, OnBtnDefaultColor)
	ON_BN_CLICKED(IDC_BTN_BG_COLOR, OnBtnBgColor)
	ON_BN_CLICKED(IDC_BTN_COMMENT_COLOR, OnBtnCommentColor)
	ON_BN_CLICKED(IDC_BTN_KEYWORD_COLOR, OnBtnKeywordColor)
	ON_BN_CLICKED(IDC_BTN_QUOTE_COLOR, OnBtnQuoteColor)
	ON_BN_CLICKED(IDC_CHECK_SHOW_COL_NUM, OnCheckShowColNum)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ROW_NUM, OnCheckShowRowNum)
	ON_EN_CHANGE(IDC_EDIT_CHAR_SPACE, OnChangeEditCharSpace)
	ON_EN_CHANGE(IDC_EDIT_LEFT_SPACE, OnChangeEditLeftSpace)
	ON_EN_CHANGE(IDC_EDIT_ROW_SPACE, OnChangeEditRowSpace)
	ON_EN_CHANGE(IDC_EDIT_TOP_SPACE, OnChangeEditTopSpace)
	ON_BN_CLICKED(IDC_CHECK_SHOW_2BYTE_SPACE, OnCheckShow2byteSpace)
	ON_BN_CLICKED(IDC_BTN_SEARCH_COLOR, OnBtnSearchColor)
	ON_BN_CLICKED(IDC_BTN_SELECTED_COLOR, OnBtnSelectedColor)
	ON_EN_CHANGE(IDC_EDIT_LINE_LEN, OnChangeEditLineLen)
	ON_BN_CLICKED(IDC_CHECK_SHOW_ROW_LINE, OnCheckShowRowLine)
	ON_BN_CLICKED(IDC_BTN_RULER_COLOR, OnBtnRulerColor)
	ON_BN_CLICKED(IDC_BTN_OPERATOR_COLOR, OnBtnOperatorColor)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EDIT_ROW, OnCheckShowEditRow)
	ON_BN_CLICKED(IDC_BTN_KEYWORD2_COLOR, OnBtnKeyword2Color)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SPACE, OnCheckShowSpace)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_SHOW_BRACKETS_BOLD, &CEditorOptionPage::OnClickedCheckShowBracketsBold)
	ON_BN_CLICKED(IDC_BTN_BRACKET_COLOR1, &CEditorOptionPage::OnClickedBtnBracketColor1)
	ON_BN_CLICKED(IDC_BTN_BRACKET_COLOR2, &CEditorOptionPage::OnClickedBtnBracketColor2)
	ON_BN_CLICKED(IDC_BTN_BRACKET_COLOR3, &CEditorOptionPage::OnClickedBtnBracketColor3)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage メッセージ ハンドラ

int CEditorOptionPage::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: この位置に固有の作成用コードを追加してください
	m_edit_data.set_str_token(&g_sql_str_token);
	m_edit_data.paste(
		_T("/* comment　 */\n")
		_T("select * from piyo\n")
		_T("	where a = 'A';")
	);
	m_edit_data.set_cur(0, 0);

	return 0;
}

BOOL CEditorOptionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	UpdateData(FALSE);

	CreateEditCtrl();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

BOOL CEditorOptionPage::OnApply() 
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	
	return CPropertyPage::OnApply();
}

void CEditorOptionPage::CreateEditCtrl()
{
	RECT rect;
	GetClientRect(&rect);

	RECT button_rect;
	GetDlgItem(IDC_BTN_DEFAULT_COLOR)->GetWindowRect(&button_rect);
	ScreenToClient(&button_rect);

	m_edit_ctrl.SetEditData(&m_edit_data);
	m_edit_ctrl.SetReadOnly(TRUE);
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER, 
		CRect(20, button_rect.bottom + 15, rect.right - 20, rect.bottom - 10), 
		this, NULL);
	m_edit_ctrl.SetFont(&g_font);

	SetEditColor();
	SetEditorOption();
}

void CEditorOptionPage::SetEditorOption()
{
	UpdateData(TRUE);

	QWORD		option = ECS_BRACKET_MULTI_COLOR_ENABLE;

	if(m_tabstop < 2) m_tabstop = 2;
	if(m_tabstop > 16) m_tabstop = 16;
	if(m_row_space < 0) m_row_space = 0;
	if(m_row_space > 20) m_row_space = 20;
	if(m_char_space < 0) m_char_space = 0;
	if(m_char_space > 20) m_char_space = 20;
	if(m_top_space < 0) m_top_space = 0;
	if(m_top_space > 20) m_top_space = 20;
	if(m_left_space < 0) m_left_space = 0;
	if(m_left_space > 20) m_left_space = 20;
	if(m_line_len < 10) m_line_len = 10;

	if(m_edit_data.get_tabstop() != m_tabstop) {
		m_edit_data.set_tabstop(m_tabstop);
	}

	if(m_edit_ctrl.GetRowSpace() != m_row_space || 
		m_edit_ctrl.GetCharSpaceSetting() != m_char_space ||
		m_edit_ctrl.GetTopSpace() != m_top_space ||
		m_edit_ctrl.GetLeftSpace() != m_left_space) {

		m_edit_ctrl.SetSpaces(m_row_space, m_char_space, m_top_space, m_left_space);
	}

	if(m_show_line_feed) {
		option |= ECS_SHOW_LINE_FEED;
	}
	if(m_show_tab) {
		option |= ECS_SHOW_TAB;
	}
	if(m_show_row_num) {
		option |= ECS_SHOW_ROW_NUM;
	}
	if(m_show_col_num) {
		option |= ECS_SHOW_COL_NUM;
	}
	if(m_show_space) {
		option |= ECS_SHOW_SPACE;
	}
	if(m_show_2byte_space) {
		option |= ECS_SHOW_2BYTE_SPACE;
	}
	if(m_show_row_line) {
		option |= ECS_SHOW_ROW_LINE;
	}
	if(m_show_edit_row) {
		option |= ECS_SHOW_EDIT_ROW;
	}
	if(m_show_brackets_bold) {
		option |= ECS_SHOW_BRACKETS_BOLD;
	}

	m_edit_ctrl.SetExStyle2(option);
	m_edit_ctrl.Redraw();
}

void CEditorOptionPage::OnCheckShowLineFeed() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowTab() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditTabstop() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	switch(nIDCtl) {
	case IDC_BTN_TEXT_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[TEXT_COLOR]);
		break;
	case IDC_BTN_KEYWORD_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[KEYWORD_COLOR]);
		break;
	case IDC_BTN_KEYWORD2_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[KEYWORD2_COLOR]);
		break;
	case IDC_BTN_COMMENT_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[COMMENT_COLOR]);
		break;
	case IDC_BTN_BG_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[BG_COLOR]);
		break;
	case IDC_BTN_PEN_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[PEN_COLOR]);
		break;
	case IDC_BTN_QUOTE_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[QUOTE_COLOR]);
		break;
	case IDC_BTN_SEARCH_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[SEARCH_COLOR]);
		break;
	case IDC_BTN_SELECTED_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[SELECTED_COLOR]);
		break;
	case IDC_BTN_RULER_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[RULER_COLOR]);
		break;
	case IDC_BTN_OPERATOR_COLOR:
		DrawColorBtn(lpDrawItemStruct, m_color[OPERATOR_COLOR]);
		break;
	case IDC_BTN_BRACKET_COLOR1:
		DrawColorBtn(lpDrawItemStruct, m_color[BRACKET_COLOR1]);
		break;
	case IDC_BTN_BRACKET_COLOR2:
		DrawColorBtn(lpDrawItemStruct, m_color[BRACKET_COLOR2]);
		break;
	case IDC_BTN_BRACKET_COLOR3:
		DrawColorBtn(lpDrawItemStruct, m_color[BRACKET_COLOR3]);
		break;
	}
	
	CPropertyPage::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CEditorOptionPage::DrawColorBtn(LPDRAWITEMSTRUCT lpDrawItemStruct, COLORREF clr)
{
	CRect rect(lpDrawItemStruct->rcItem);

	::DrawEdge(lpDrawItemStruct->hDC, rect, EDGE_BUMP, BF_RECT);

	rect.DeflateRect(2, 2);
	::SetBkColor(lpDrawItemStruct->hDC, clr);
	::ExtTextOut(lpDrawItemStruct->hDC, 0, 0, ETO_OPAQUE, 
		rect, NULL, 0, NULL);
}

void CEditorOptionPage::ChooseColor(int color_id)
{
	CColorDialog	dlg(m_color[color_id]);

	if(dlg.DoModal() != IDOK) return;

	m_color[color_id] = dlg.GetColor();
	m_edit_ctrl.SetColor(color_id, m_color[color_id]);
	m_edit_ctrl.Redraw();
	Invalidate();
}

void CEditorOptionPage::SetEditColor()
{
	for(int i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		m_edit_ctrl.SetColor(i, m_color[i]);
	}
	m_edit_ctrl.Redraw();
}

void CEditorOptionPage::SetDefaultEditColor()
{
	m_color[TEXT_COLOR] = RGB(0, 0, 0);
	m_color[KEYWORD_COLOR] = RGB(0, 0, 205);
	m_color[KEYWORD2_COLOR] = RGB(0, 128, 192);
	m_color[COMMENT_COLOR] = RGB(0, 120, 0);
	m_color[BG_COLOR] = RGB(255, 255, 255);
	m_color[PEN_COLOR] = RGB(0, 150, 150);
	m_color[QUOTE_COLOR] = RGB(220, 0, 0);
	m_color[SEARCH_COLOR] = RGB(200, 200, 200);
	m_color[SELECTED_COLOR] = RGB(0, 0, 50);
	m_color[RULER_COLOR] = RGB(0, 100, 0);
	m_color[OPERATOR_COLOR] = RGB(128, 0, 0);

	m_color[BRACKET_COLOR1] = m_color[OPERATOR_COLOR];
	m_color[BRACKET_COLOR2] = m_color[OPERATOR_COLOR];
	m_color[BRACKET_COLOR3] = m_color[OPERATOR_COLOR];

	SetEditColor();
	Invalidate();
}

void CEditorOptionPage::OnBtnDefaultColor() 
{
	SetDefaultEditColor();
}

void CEditorOptionPage::OnBtnTextColor() 
{
	ChooseColor(TEXT_COLOR);
}

void CEditorOptionPage::OnBtnPenColor() 
{
	ChooseColor(PEN_COLOR);
}

void CEditorOptionPage::OnBtnBgColor() 
{
	ChooseColor(BG_COLOR);
}

void CEditorOptionPage::OnBtnCommentColor() 
{
	ChooseColor(COMMENT_COLOR);
}

void CEditorOptionPage::OnBtnKeywordColor() 
{
	ChooseColor(KEYWORD_COLOR);
}

void CEditorOptionPage::OnBtnQuoteColor() 
{
	ChooseColor(QUOTE_COLOR);
}

void CEditorOptionPage::OnCheckShowColNum() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowRowNum() 
{
	SetEditorOption();	
}

void CEditorOptionPage::OnChangeEditCharSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditLeftSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditRowSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnChangeEditTopSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShow2byteSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnSearchColor() 
{
	ChooseColor(SEARCH_COLOR);	
}

void CEditorOptionPage::OnBtnSelectedColor() 
{
	ChooseColor(SELECTED_COLOR);
}

void CEditorOptionPage::OnChangeEditLineLen() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnCheckShowRowLine() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnRulerColor() 
{
	ChooseColor(RULER_COLOR);
}

void CEditorOptionPage::OnBtnOperatorColor() 
{
	ChooseColor(OPERATOR_COLOR);
}

void CEditorOptionPage::OnCheckShowEditRow() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnBtnKeyword2Color() 
{
	ChooseColor(KEYWORD2_COLOR);
}

void CEditorOptionPage::OnCheckShowSpace() 
{
	SetEditorOption();
}

void CEditorOptionPage::OnClickedCheckShowBracketsBold()
{
	SetEditorOption();
}

void CEditorOptionPage::OnClickedBtnBracketColor1()
{
	ChooseColor(BRACKET_COLOR1);
}

void CEditorOptionPage::OnClickedBtnBracketColor2()
{
	ChooseColor(BRACKET_COLOR2);
}

void CEditorOptionPage::OnClickedBtnBracketColor3()
{
	ChooseColor(BRACKET_COLOR3);
}
