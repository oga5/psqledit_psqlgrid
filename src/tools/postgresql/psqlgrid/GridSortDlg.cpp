/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
// GridSortDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "GridSortDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridSortDlg ダイアログ


CGridSortDlg::CGridSortDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGridSortDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGridSortDlg)
	m_sort_order1 = -1;
	m_sort_order2 = -1;
	m_sort_order3 = -1;
	//}}AFX_DATA_INIT
}


void CGridSortDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGridSortDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_COMBO_SORT_KEY3, m_combo_sort_key3);
	DDX_Control(pDX, IDC_COMBO_SORT_KEY2, m_combo_sort_key2);
	DDX_Control(pDX, IDC_COMBO_SORT_KEY1, m_combo_sort_key1);
	DDX_Radio(pDX, IDC_RADIO_ORDER_ASC1, m_sort_order1);
	DDX_Radio(pDX, IDC_RADIO_ORDER_ASC2, m_sort_order2);
	DDX_Radio(pDX, IDC_RADIO_ORDER_ASC3, m_sort_order3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGridSortDlg, CDialog)
	//{{AFX_MSG_MAP(CGridSortDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_SORT_KEY1, OnSelchangeComboSortKey1)
	ON_CBN_SELCHANGE(IDC_COMBO_SORT_KEY2, OnSelchangeComboSortKey2)
	ON_CBN_SELCHANGE(IDC_COMBO_SORT_KEY3, OnSelchangeComboSortKey3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridSortDlg メッセージ ハンドラ

void CGridSortDlg::InitComboBox(CComboBox *combo, int sel)
{
	int		i;

	combo->InsertString(0, _T(""));
	combo->SetItemData(0, CB_ERR);
	combo->SetCurSel(0);

	for(i = 0; i < m_grid_data->Get_ColCnt(); i++) {
		combo->InsertString(i + 1, m_grid_data->Get_ColName(i));
		combo->SetItemData(i + 1, i);
		if(sel == i) combo->SetCurSel(i + 1);
	}
}

BOOL CGridSortDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	InitComboBox(&m_combo_sort_key1, m_sort_col_no[0]);
	InitComboBox(&m_combo_sort_key2, m_sort_col_no[1]);
	InitComboBox(&m_combo_sort_key3, m_sort_col_no[2]);

	m_sort_order1 = 0;
	m_sort_order2 = 0;
	m_sort_order3 = 0;

	UpdateData(FALSE);

	CheckBtn();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CGridSortDlg::InitData(CGridData *grid_data, int *sort_col_no, int *sort_order,
	int *sort_cnt)
{
	m_grid_data = grid_data;
	m_sort_col_no = sort_col_no;
	m_sort_order = sort_order;
	m_sort_cnt = sort_cnt;
}

void CGridSortDlg::GetData()
{
	UpdateData(TRUE);

	int sort_col_no[3];
	int sort_order[3];

	sort_col_no[0] = m_combo_sort_key1.GetCurSel();
	if(sort_col_no[0] != CB_ERR) sort_col_no[0] = (int)m_combo_sort_key1.GetItemData(sort_col_no[0]);
	sort_col_no[1] = m_combo_sort_key2.GetCurSel();
	if(sort_col_no[1] != CB_ERR) sort_col_no[1] = (int)m_combo_sort_key1.GetItemData(sort_col_no[1]);
	sort_col_no[2] = m_combo_sort_key3.GetCurSel();
	if(sort_col_no[2] != CB_ERR) sort_col_no[2] = (int)m_combo_sort_key1.GetItemData(sort_col_no[2]);

	sort_order[0] = (m_sort_order1 == 0) ? 1 : -1;
	sort_order[1] = (m_sort_order2 == 0) ? 1 : -1;
	sort_order[2] = (m_sort_order3 == 0) ? 1 : -1;

	int sort_cnt = 0;
	for(int i = 0; i < 3; i++) {
		if(sort_col_no[i] != CB_ERR) {
			m_sort_col_no[sort_cnt] = sort_col_no[i];
			m_sort_order[sort_cnt] = sort_order[i];
			sort_cnt++;
		}
	}
	*m_sort_cnt = sort_cnt;
}

void CGridSortDlg::CheckBtn()
{
	GetData();
	m_ok.EnableWindow(*m_sort_cnt != 0);
}

void CGridSortDlg::OnSelchangeComboSortKey1() 
{
	CheckBtn();
}

void CGridSortDlg::OnSelchangeComboSortKey2() 
{
	CheckBtn();
}

void CGridSortDlg::OnSelchangeComboSortKey3() 
{
	CheckBtn();
}

void CGridSortDlg::OnOK() 
{
	GetData();

	if(*m_sort_cnt == 0) {
		AfxMessageBox(_T("ソート条件が無効です"));
		return;
	}

	CDialog::OnOK();
}


