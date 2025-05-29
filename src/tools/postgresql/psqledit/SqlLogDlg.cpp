/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 // SqlLogDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "SqlLogDlg.h"

#include "common.h"
#include "ostrutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum list_column {
	LIST_SQL,
	LIST_TIME,
	LIST_NO,
};

/////////////////////////////////////////////////////////////////////////////
// CSqlLogDlg ダイアログ


CSqlLogDlg::CSqlLogDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSqlLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSqlLogDlg)
	//}}AFX_DATA_INIT
}


void CSqlLogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSqlLogDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_LIST1, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSqlLogDlg, CDialog)
	//{{AFX_MSG_MAP(CSqlLogDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSqlLogDlg メッセージ ハンドラ

void CSqlLogDlg::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = 0;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;

	m_edit_ctrl.ClearAll();

	int	selected_row;
	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) {
		m_ok.EnableWindow(FALSE);
		return;
	}

	m_selected_row = (int)m_list.GetItemData(selected_row);
	m_edit_data.replace_all(m_sql_log_array->GetAt(m_selected_row)->GetSql().GetBuffer(0));
	m_edit_ctrl.Redraw();

	m_ok.EnableWindow(TRUE);
	
	UpdateData(FALSE);
}

BOOL CSqlLogDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// エディタを作成
	CreateEditCtrl();

	m_list.SetFont(m_font);
	m_list.InsertColumn(LIST_SQL, _T("SQL"), LVCFMT_LEFT, 310);
	m_list.InsertColumn(LIST_TIME, _T("Time"), LVCFMT_LEFT, 70);
	m_list.InsertColumn(LIST_NO, _T("No."), LVCFMT_RIGHT, 30);

	// アイテムを選択したときに，行全体を反転させるようにする
	ListView_SetExtendedListViewStyle(m_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	if(SetLogList() != 0) EndDialog(IDCANCEL);

	int iItem = m_list.GetItemCount() - 1;
	if(iItem >= 0) {
		m_list.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_list.EnsureVisible(iItem, FALSE);
		m_list.RedrawItems(iItem, iItem);
	}

	int width = AfxGetApp()->GetProfileInt(_T("SQL_LOG_DLG"), _T("WIDTH"), 0);
	int height = AfxGetApp()->GetProfileInt(_T("SQL_LOG_DLG"), _T("HEIGHT"), 0);
	if(width > 200 && height > 200) {
		SetWindowPos(NULL, 0, 0, width, height, 
			SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOCOPYBITS);
	}

	return FALSE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

int CSqlLogDlg::SetLogList()
{
	int			i, cnt;
	LV_ITEM		item;
	CString str;
	TCHAR	*p;

	m_list.DeleteAllItems();

	cnt = (int)m_sql_log_array->GetSize();

	item.mask = LVIF_PARAM;
	item.iSubItem = 0;
	for(i = 0; i < cnt; i++) {
		item.iItem = i;
		item.lParam = i;
		item.iItem = m_list.InsertItem(&item);
		if(item.iItem == -1) goto ERR1;

		// 先頭の100文字のみ表示
		str = m_sql_log_array->GetAt(i)->GetSql().Left(100);
		p = str.GetBuffer(0);
		ostr_char_replace(p, '\r', ' ');
		ostr_char_replace(p, '\n', ' ');
		str.ReleaseBuffer();
		m_list.SetItemText(i, LIST_SQL, str);

		m_list.SetItemText(i, LIST_TIME, m_sql_log_array->GetAt(i)->GetTime());

		str.Format(_T("%d"), m_sql_log_array->GetAt(i)->GetNumber() + 1);
		m_list.SetItemText(i, LIST_NO, str);
	}

	return 0;

ERR1:
	MessageBox(_T("リストビュー初期化エラー"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void CSqlLogDlg::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	selected_row;
	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	m_selected_row = (int)m_list.GetItemData(selected_row);

	OnOK();
	
	*pResult = 0;
}

int CSqlLogDlg::CreateEditCtrl()
{
	RECT list_rect;
	m_list.GetWindowRect(&list_rect);
	ScreenToClient(&list_rect);

	RECT ok_rect;
	GetDlgItem(IDOK)->GetWindowRect(&ok_rect);
	ScreenToClient(&ok_rect);

	m_edit_data.set_str_token(&g_sql_str_token);

	m_edit_ctrl.SetEditData(&m_edit_data);
	m_edit_ctrl.SetReadOnly(TRUE);
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER, 
		CRect(list_rect.left, list_rect.bottom + 10,
		list_rect.right, ok_rect.top - 10), 
		this, NULL);
	m_edit_ctrl.SetFont(m_font);
	m_edit_ctrl.SetExStyle2(ECS_ON_DIALOG | ECS_BRACKET_MULTI_COLOR_ENABLE);

	return 0;
}

void CSqlLogDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControl();
}

void CSqlLogDlg::RelayoutControl()
{
	if(IsWindow(GetSafeHwnd()) == FALSE) return;

	CRect	win_rect, ctrl_rect;
	int		x, y;
	int		width, height;

	CWnd	*wnd;

	GetClientRect(&win_rect);

	// OKボタン，CALCELボタン
	wnd = GetDlgItem(IDOK);
	if(wnd == NULL) return;
	wnd->GetClientRect(&ctrl_rect);
	x = win_rect.Width() / 2 - (ctrl_rect.Width() + 10);
	y = win_rect.Height() - ctrl_rect.Height() - 10;
	wnd->SetWindowPos(NULL, x, y, 0, 0, 
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);
	x = (win_rect.right - win_rect.left) / 2 + 10;
	wnd = GetDlgItem(IDCANCEL);
	if(wnd == NULL) return;
	wnd->SetWindowPos(NULL, x, y, 0, 0, 
		SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOCOPYBITS);

	if(IsWindow(m_edit_ctrl.GetSafeHwnd()) == FALSE) return;
	width = win_rect.Width() - 20;
	height = (win_rect.Height() - ctrl_rect.Height() - 10) / 3 - 10;
	x = win_rect.left + 10;
	y = y - height - 10;
	m_edit_ctrl.SetWindowPos(NULL, x, y, width, height, 
		SWP_NOOWNERZORDER | SWP_NOCOPYBITS);

	if(IsWindow(m_list.GetSafeHwnd()) == FALSE) return;
	width = win_rect.Width() - 20;
	height = (win_rect.Height() - ctrl_rect.Height() - 10) / 3 * 2 - 20;
	x = win_rect.left + 10;
	y = win_rect.top + 10;
	m_list.SetWindowPos(NULL, x, y, width, height, 
		SWP_NOOWNERZORDER | SWP_NOCOPYBITS);
}

void CSqlLogDlg::OnDestroy() 
{
	CRect	win_rect;
	GetWindowRect(&win_rect);

	AfxGetApp()->WriteProfileInt(_T("SQL_LOG_DLG"), _T("WIDTH"), win_rect.Width());
	AfxGetApp()->WriteProfileInt(_T("SQL_LOG_DLG"), _T("HEIGHT"), win_rect.Height());

	CDialog::OnDestroy();
}
