/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ShortCutKeyDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "ShortCutKeyDlg.h"

#include "shortcutsql.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShortCutKeyDlg ダイアログ

static void SetKeyInfo(HWND hwnd, int key, DWORD keyData)
{
	CString	keyInfo = _T("");

	keyInfo = CAccelList::get_accel_str(key, (GetAsyncKeyState(VK_MENU) < 0),
		(GetAsyncKeyState(VK_CONTROL) < 0), (GetAsyncKeyState(VK_SHIFT) < 0));

	SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)keyInfo.GetBuffer(0));
}

// 編集用エディットコントロールの，サブクラス後のウィンドウプロシージャ
static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		SetKeyInfo(hwnd, (int)wParam, (DWORD)lParam);
		return 0;
	case WM_CHAR:
		return 0;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}


CShortCutKeyDlg::CShortCutKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShortCutKeyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShortCutKeyDlg)
	m_new_key = _T("");
	m_current_menu = _T("");
	//}}AFX_DATA_INIT
}


void CShortCutKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShortCutKeyDlg)
	DDX_Control(pDX, IDC_LIST_ACCEL, m_list_accel);
	DDX_Control(pDX, IDC_EDIT_NEW_KEY, m_edit_new_key);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btn_delete);
	DDX_Control(pDX, IDC_BTN_ADD, m_btn_add);
	DDX_Text(pDX, IDC_EDIT_NEW_KEY, m_new_key);
	DDX_Text(pDX, IDC_STATIC_CURRENT_MENU, m_current_menu);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShortCutKeyDlg, CDialog)
	//{{AFX_MSG_MAP(CShortCutKeyDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ACCEL, OnItemchangedListAccel)
	ON_BN_CLICKED(IDOK2, OnOk2)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_EN_CHANGE(IDC_EDIT_NEW_KEY, OnChangeEditNewKey)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShortCutKeyDlg メッセージ ハンドラ

void CShortCutKeyDlg::OnItemchangedListAccel(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;

	CheckBtn();
	
	*pResult = 0;
}

void CShortCutKeyDlg::OnOK() 
{
}

void CShortCutKeyDlg::OnOk2() 
{
	CDialog::OnOK();
}

BOOL CShortCutKeyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	// サブクラス化
	// 古いウィンドウプロシージャを保存する
	HWND hwnd = m_edit_new_key.GetSafeHwnd();
	::SetWindowLongPtr(hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
	// ウィンドウプロシージャを切り替える
	::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);

	ListView_SetExtendedListViewStyle(m_list_accel.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_list_accel.InsertColumn(0, _T("キー"), LVCFMT_LEFT, 170);
	
	SetAccelList(m_command);

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CShortCutKeyDlg::OnBtnAdd() 
{
	UpdateData(TRUE);

	m_accel_list.add_accel(m_command, m_new_key.GetBuffer(0));

	m_new_key = _T("");
	m_current_menu = _T("");
	UpdateData(FALSE);

	ResetAccelList();
}

void CShortCutKeyDlg::OnBtnDelete() 
{
	int accel_selected_row = m_list_accel.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	m_accel_list.delete_accel(m_list_accel.GetItemText(accel_selected_row, 0).GetBuffer(0));

	ResetAccelList();
}

void CShortCutKeyDlg::OnChangeEditNewKey() 
{
	UpdateData(TRUE);

	CheckBtn();

	int menu_id = m_accel_list.search_cmd(m_new_key.GetBuffer(0));
	if(menu_id > 0) {
		CString current_menu = m_accel_list.GetCommandName(menu_id);
		if(current_menu != "") {
			m_current_menu.Format(_T("現在の割り当て：%s"), current_menu);
		} else {
			m_current_menu = _T("");
		}
	} else {
		m_current_menu = _T("");
	}
	UpdateData(FALSE);
}

void CShortCutKeyDlg::ResetAccelList()
{
	SetAccelList(m_command);
}

void CShortCutKeyDlg::SetAccelList(int menu_id)
{
	int			i, pos;
	LV_ITEM		item;
	CString		accel_name;

	m_list_accel.DeleteAllItems();

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;
	pos = 0;
	for(i = 0;; i++) {
		pos = m_accel_list.search_accel_str(menu_id, pos, accel_name);
		if(pos < 0) break;

		item.iItem = i;
		item.pszText = accel_name.GetBuffer(0);
		item.lParam = m_command;
		item.iItem = m_list_accel.InsertItem(&item);
	}

	CheckBtn();
}

void CShortCutKeyDlg::CheckBtn()
{
	UpdateData(TRUE);

	int accel_selected_row = m_list_accel.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);

	if(accel_selected_row == -1) {
		m_btn_delete.EnableWindow(FALSE);
	} else {
		m_btn_delete.EnableWindow(TRUE);
	}

	if(m_new_key == _T("")  || m_new_key.GetLength() == 1 ||
		m_new_key.GetAt(m_new_key.GetLength() - 1) == '+') {
		m_btn_add.EnableWindow(FALSE);
	} else {
		m_btn_add.EnableWindow(TRUE);
	}
}

