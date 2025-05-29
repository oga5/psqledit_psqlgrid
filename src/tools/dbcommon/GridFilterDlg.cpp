/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // GridFilterDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "GridFilterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SAVE_SEARCH_TEXT_CNT	10
#define GRID_FILTER_DLG_MIN_WIDTH	450

#define SEARCH_STATUS_REGEXP			(0x01 << 0)
#define SEARCH_STATUS_DISTINCT_LWR_UPR	(0x01 << 1)
#define SEARCH_STATUS_DISTINCT_WIDTH_ASCII	(0x01 << 2)


/////////////////////////////////////////////////////////////////////////////
// CGridFilterDlg ダイアログ


CGridFilterDlg::CGridFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridFilterDlg::IDD, pParent)
	, m_distinct_width_ascii(FALSE)
	, m_data(NULL)
	, m_dlg_height(0)
	, m_wnd(NULL)
	, m_b_modal(FALSE)
{
	//{{AFX_DATA_INIT(CGridFilterDlg)
	m_distinct_lwr_upr = FALSE;
	m_regexp = FALSE;
	//}}AFX_DATA_INIT

	m_initialized = FALSE;
	m_close_at_found = FALSE;

	m_search_status = new int[SAVE_SEARCH_TEXT_CNT];
	if(m_search_status != NULL) {
		int		i;
		for(i = 0; i < SAVE_SEARCH_TEXT_CNT; i++) {
			m_search_status[i] = 0;
		}
	}

	m_title.Empty();
}

CGridFilterDlg::~CGridFilterDlg()
{
	if(m_search_status != NULL) {
		delete[] m_search_status;
		m_search_status = NULL;
	}
}

void CGridFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGridFilterDlg)
	DDX_Control(pDX, IDC_CHECK_REGEXP, m_btn_check_regexp);
	DDX_Control(pDX, IDCANCEL, m_btn_cancel);
	DDX_Control(pDX, IDC_COMBO_SEARCH_TEXT, m_combo_search_text);
	DDX_Control(pDX, ID_BTN_FILTER_RUN, m_btn_filter_run);
	DDX_Check(pDX, IDC_CHECK_DISTINCT_LWR_UPR, m_distinct_lwr_upr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, m_regexp);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_DISTINCT_WIDTH_ASCII, m_distinct_width_ascii);
	DDX_Control(pDX, IDC_BTN_BOTTOM, m_btn_bottom);
	DDX_Control(pDX, IDC_COMBO_GRID_COLUMN, m_combo_grid_column);
	DDX_Control(pDX, ID_FILTER_CLEAR, m_btn_filter_clear);
}


BEGIN_MESSAGE_MAP(CGridFilterDlg, CDialog)
	//{{AFX_MSG_MAP(CGridFilterDlg)
	ON_BN_CLICKED(ID_BTN_FILTER_RUN, OnBtnFilterRun)
	ON_CBN_EDITCHANGE(IDC_COMBO_SEARCH_TEXT, OnEditchangeComboSearchText)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELENDOK(IDC_COMBO_SEARCH_TEXT, OnSelendokComboSearchText)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(ID_FILTER_CLEAR, &CGridFilterDlg::OnClickedFilterClear)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridFilterDlg メッセージ ハンドラ

void CGridFilterDlg::SaveGridFilterDlgData()
{
	UpdateData(TRUE);

	m_data->m_search_text = m_search_text;
	m_data->m_distinct_lwr_upr = m_distinct_lwr_upr;
	m_data->m_distinct_width_ascii = m_distinct_width_ascii;
	m_data->m_regexp = m_regexp;
}

void CGridFilterDlg::GetData()
{
	int cur_sel = m_combo_grid_column.GetCurSel();
	if(cur_sel == CB_ERR) {
		m_filter_col_no = CB_ERR;
	} else {
		m_filter_col_no = (int)m_combo_grid_column.GetItemData(cur_sel);
	}
}


void CGridFilterDlg::OnBtnFilterRun()
{
	if(!::IsWindow(m_wnd->GetSafeHwnd())) {
		ShowWindow(SW_HIDE);
		return;
	}

	CWaitCursor wait_cursor;
	UpdateData(TRUE);
	GetData();

	if(m_search_text == _T("")) return;
	if(m_filter_col_no == CB_ERR) return;

	SaveGridFilterDlgData();
	m_data->m_dir = 1;

	LRESULT ret_v = m_wnd->SendMessage(WM_GRID_FILTER_RUN, m_filter_col_no, 0);

	SaveSearchTextList();

	m_combo_search_text.SetFocus();

	if(m_close_at_found && ret_v == 0) OnCancel();
}

void CGridFilterDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_search_text == _T("")) {
		m_btn_filter_run.EnableWindow(FALSE);
	} else {
		m_btn_filter_run.EnableWindow(TRUE);
	}

	if(m_grid_data->GetGridFilterMode()) {
		m_btn_filter_clear.EnableWindow(TRUE);
	} else {
		m_btn_filter_clear.EnableWindow(FALSE);
	}
}

void CGridFilterDlg::ShowDialog(CWnd* wnd, CSearchDlgData* search_data,
	CGridData* grid_data, int filter_col_no, BOOL b_close_at_found)
{
	m_wnd = wnd;
	m_data = search_data;
	m_initialized = TRUE;
	m_close_at_found = b_close_at_found;

	m_filter_col_no = filter_col_no;
	m_grid_data = grid_data;

	m_b_modal = FALSE;

	InitDialog();

	ShowWindow(SW_SHOWNORMAL);
}

void CGridFilterDlg::InitColumnComboBox(CComboBox* combo, int sel)
{
	int		i;
	int		base = 0;

//	combo->InsertString(0, _T(""));
//	combo->SetItemData(0, CB_ERR);
//	combo->SetCurSel(0);
//	base++;

	combo->ResetContent();

	for(i = 0; i < m_grid_data->Get_ColCnt(); i++) {
		combo->InsertString(i + base, m_grid_data->Get_ColName(i));
		combo->SetItemData(i + base, i);
		if(sel == i) combo->SetCurSel(i + base);
	}
}

void CGridFilterDlg::InitDialog()
{
	// ダイアログの高さを決める
	// ディスプレイの設定が変更される可能性があるので、表示する度に計算する
	CRect	btn_rect;
	POINT pt = { 0, 0 };

	m_btn_bottom.GetWindowRect(btn_rect);
	pt.x = btn_rect.left;
	pt.y = btn_rect.bottom;
	::ScreenToClient(GetSafeHwnd(), &pt);
	m_dlg_height = pt.y + 10 + ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYSIZEFRAME) * 2;

	if(m_initialized == FALSE) return;

	InitColumnComboBox(&m_combo_grid_column, m_filter_col_no);

	m_search_text = m_data->m_search_text;
	m_regexp = m_data->m_regexp;
	m_distinct_lwr_upr = m_data->m_distinct_lwr_upr;
	m_distinct_width_ascii = m_data->m_distinct_width_ascii;

	LoadSearchTextList();
	if(m_search_text == "" && m_combo_search_text.GetCount() != 0) {
		m_combo_search_text.GetLBText(0, m_search_text);
	}
	m_regexp = is_regexp(m_search_status[0]);
	m_distinct_lwr_upr = is_distinct_lwr_upr(m_search_status[0]);
	m_distinct_width_ascii = is_distinct_width_ascii(m_search_status[0]);

	m_combo_search_text.SetFocus();

	UpdateData(FALSE);
	CheckBtn();

	LoadDlgSize();
}

BOOL CGridFilterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	InitDialog();

	if(!m_title.IsEmpty()) {
		SetWindowText(m_title);
	}

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CGridFilterDlg::LoadSearchTextList()
{
	::LoadSearchTextList(&m_combo_search_text, _T("SEARCH_TEXT"), SAVE_SEARCH_TEXT_CNT, m_search_status);
}

void CGridFilterDlg::SaveSearchTextList()
{
	int cur_status = make_search_status2(m_regexp, m_distinct_lwr_upr, m_distinct_width_ascii);

	::SaveSearchTextList(_T("SEARCH_TEXT"), SAVE_SEARCH_TEXT_CNT, m_search_text,
		TRUE, cur_status);

	LoadSearchTextList();
	UpdateData(FALSE);
	CheckBtn();
}

void CGridFilterDlg::OnEditchangeComboSearchText() 
{
	CheckBtn();
}

void CGridFilterDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	// 親ウィンドウの中央に表示する
	if(bShow) {
		CRect	win_rect, parent_rect;
		GetWindowRect(win_rect);
		GetParent()->GetWindowRect(parent_rect);

		int x, y;
		x = parent_rect.left + parent_rect.Width() / 2 - win_rect.Width() / 2;
		y = parent_rect.top + parent_rect.Height() / 2 - win_rect.Height() / 2;

		SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		m_combo_search_text.SetFocus();
	}
}

void CGridFilterDlg::OnOK() 
{
	OnBtnFilterRun();
}

void CGridFilterDlg::OnSelendokComboSearchText() 
{
	int idx = m_combo_search_text.GetCurSel();
	if(idx != CB_ERR) {
		m_combo_search_text.GetLBText(idx, m_search_text);
		m_regexp = is_regexp(m_search_status[idx]);
		m_distinct_lwr_upr = is_distinct_lwr_upr(m_search_status[idx]);
		m_distinct_width_ascii = is_distinct_width_ascii(m_search_status[idx]);

		UpdateData(FALSE);
	}

	CheckBtn();
}

INT_PTR CGridFilterDlg::DoModal(CWnd *wnd, CSearchDlgData *search_data, 
	CGridData* grid_data, int filter_col_no, BOOL b_close_at_found)
{
	m_wnd = wnd;
	m_data = search_data;
	m_initialized = TRUE;
	m_close_at_found = b_close_at_found;

	m_filter_col_no = filter_col_no;
	m_grid_data = grid_data;

	m_b_modal = TRUE;

	return CDialog::DoModal();
}

BOOL CGridFilterDlg::UpdateData(BOOL bSaveAndValidate)
{
	UpdateDataCombo(m_combo_search_text, m_search_text, bSaveAndValidate);

	return CDialog::UpdateData(bSaveAndValidate);
}


void CGridFilterDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControls();

	RedrawWindow();
}

void CGridFilterDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	// 横方向のみ可変にする
	lpMMI->ptMinTrackSize.x = GRID_FILTER_DLG_MIN_WIDTH;
	lpMMI->ptMinTrackSize.y = m_dlg_height;
	lpMMI->ptMaxTrackSize.y = m_dlg_height;
}

static void AdjustBtnPos(CButton *btn, CRect *win_rect, POINT *pt)
{
	CRect	btn_rect;
	int		x, y;

	btn->GetWindowRect(btn_rect);
	x = win_rect->Width() - btn_rect.Width() - 18;
	y = btn_rect.top - pt->y;
	btn->SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

static void AdjustComboWidth(CComboBox *combo, CRect *win_rect, CRect *btn_rect)
{
	CRect	combo_rect;
	int		w, h;

	combo->GetWindowRect(combo_rect);
	w = btn_rect->left - combo_rect.left - 10;
	h = combo_rect.Height();
	combo->SetWindowPos(NULL, 0, 0, w, h, SWP_NOMOVE| SWP_NOZORDER);
}

void CGridFilterDlg::RelayoutControls()
{
	if(!::IsWindow(GetSafeHwnd())) return;
	if(!::IsWindow(m_btn_filter_run.GetSafeHwnd())) return;

	CRect	win_rect, btn_rect, combo_rect;
	POINT pt = { 0, 0 };

	GetWindowRect(win_rect);
	::ClientToScreen(GetSafeHwnd(), &pt);

	AdjustBtnPos(&m_btn_filter_run, &win_rect, &pt);
	AdjustBtnPos(&m_btn_filter_clear, &win_rect, &pt);
	AdjustBtnPos(&m_btn_cancel, &win_rect, &pt);

	m_btn_filter_run.GetWindowRect(btn_rect);
	AdjustComboWidth(&m_combo_search_text, &win_rect, &btn_rect);
}

void CGridFilterDlg::LoadDlgSize()
{
	CWinApp*	pApp = AfxGetApp();

	int w = pApp->GetProfileInt(_T("GRID_FILTER_DLG"), _T("DLG_WIDTH"), 0);
	if(w < GRID_FILTER_DLG_MIN_WIDTH) w = GRID_FILTER_DLG_MIN_WIDTH;

	SetWindowPos(NULL, 0, 0, w, m_dlg_height, SWP_NOMOVE | SWP_NOZORDER);
}

void CGridFilterDlg::SaveDlgSize()
{
	CWinApp*	pApp = AfxGetApp();
	int w = pApp->GetProfileInt(_T("GRID_FILTER_DLG"), _T("DLG_WIDTH"), 0);

	CRect	win_rect;
	GetWindowRect(win_rect);

	if(w == win_rect.Width()) return;

	pApp->WriteProfileInt(_T("GRID_FILTER_DLG"), _T("DLG_WIDTH"), win_rect.Width());
}

void CGridFilterDlg::OnCancel() 
{
	SaveDlgSize();

	if(m_b_modal) {
		CDialog::OnCancel();
	} else {
		ShowWindow(SW_HIDE);
		DestroyWindow();
	}
}

void CGridFilterDlg::OnClickedFilterClear()
{
	m_wnd->SendMessage(WM_GRID_FILTER_CLEAR);
	CheckBtn();
}
