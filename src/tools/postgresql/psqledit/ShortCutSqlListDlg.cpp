/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ShortCutSqlListDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "ShortCutSqlListDlg.h"

#include "ShortCutSqlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum sql_list
{
	LIST_NAME,
	LIST_SQL,
	LIST_KEY,
	LIST_IS_SHOW_DLG,
	LIST_IS_PASTE_TO_EDITOR,
};

#define SHOW_DLG_STR	_T("○")

/////////////////////////////////////////////////////////////////////////////
// CShortCutSqlListDlg ダイアログ


CShortCutSqlListDlg::CShortCutSqlListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShortCutSqlListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShortCutSqlListDlg)
	m_max_cnt = _T("");
	//}}AFX_DATA_INIT
}


void CShortCutSqlListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShortCutSqlListDlg)
	DDX_Control(pDX, IDC_BTN_MODIFY, m_btn_modify);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btn_delete);
	DDX_Control(pDX, IDC_BTN_ADD, m_btn_add);
	DDX_Control(pDX, IDC_SQL_LIST, m_list_view);
	DDX_Text(pDX, IDC_STATIC_MAX_CNT, m_max_cnt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShortCutSqlListDlg, CDialog)
	//{{AFX_MSG_MAP(CShortCutSqlListDlg)
	ON_BN_CLICKED(IDOK2, OnOk2)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_BN_CLICKED(IDC_BTN_MODIFY, OnBtnModify)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SQL_LIST, OnItemchangedSqlList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShortCutSqlListDlg メッセージ ハンドラ

BOOL CShortCutSqlListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	if(InitSqlListView() == FALSE || InitSqlList() == FALSE) {
		MessageBox(_T("ListView初期化エラー"), _T("Error"), MB_ICONERROR | MB_OK);
		CDialog::EndDialog(IDCANCEL);
		return FALSE;
	}

	m_max_cnt.Format(_T("最大%d個まで設定可能です。"), MAX_SHORT_CUT_SQL);
	UpdateData(FALSE);

	CheckBtn();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CShortCutSqlListDlg::OnOK() 
{
}

void CShortCutSqlListDlg::OnOk2() 
{
	if(SaveData() == FALSE) {
		MessageBox(_T("データ保存でエラーが発生しました"), _T("Error"), MB_ICONERROR | MB_OK);
		return;
	}

	CDialog::OnOK();
}

BOOL CShortCutSqlListDlg::InitSqlListView()
{
	ListView_SetExtendedListViewStyle(m_list_view.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_list_view.InsertColumn(LIST_NAME, _T("名前"), LVCFMT_LEFT, 90);
	m_list_view.InsertColumn(LIST_SQL, _T("SQL"), LVCFMT_LEFT, 200);
	m_list_view.InsertColumn(LIST_KEY, _T("キー"), LVCFMT_LEFT, 80);
	m_list_view.InsertColumn(LIST_IS_SHOW_DLG, _T("確認"), LVCFMT_LEFT, 40);
	m_list_view.InsertColumn(LIST_IS_PASTE_TO_EDITOR, _T("貼付"), LVCFMT_LEFT, 40);

	return TRUE;
}

BOOL CShortCutSqlListDlg::SetList(CString name, CString sql, WORD cmd, BOOL show_dlg, 
	BOOL paste_to_editor, int idx)
{
	CString		key;

	m_list_view.SetItemText(idx, LIST_NAME, name);
	m_list_view.SetItemText(idx, LIST_SQL, sql);

	if(m_accel_list.search_accel_str2(cmd, key) == 0) {
		m_list_view.SetItemText(idx, LIST_KEY, key);
	} else {
		m_list_view.SetItemText(idx, LIST_KEY, _T(""));
	}
	if(show_dlg) {
		m_list_view.SetItemText(idx, LIST_IS_SHOW_DLG, SHOW_DLG_STR);
	} else {
		m_list_view.SetItemText(idx, LIST_IS_SHOW_DLG, _T(""));
	}
	if(paste_to_editor) {
		m_list_view.SetItemText(idx, LIST_IS_PASTE_TO_EDITOR, SHOW_DLG_STR);
	} else {
		m_list_view.SetItemText(idx, LIST_IS_PASTE_TO_EDITOR, _T(""));
	}

	return TRUE;
}

BOOL CShortCutSqlListDlg::AddList(CString name, CString sql, WORD cmd, BOOL show_dlg, 
	BOOL paste_to_editor, int idx)
{
	if(name == _T("") || cmd == 0) return TRUE;

	LV_ITEM		item;

	item.mask = LVIF_PARAM;
	item.iSubItem = 0;

	CString key;

	if(idx == -1) idx = m_list_view.GetItemCount();
	item.iItem = idx;

	item.pszText = name.GetBuffer(0);
	item.lParam = cmd;

	item.iItem = m_list_view.InsertItem(&item);

	return SetList(name, sql, cmd, show_dlg, paste_to_editor, item.iItem);
}

BOOL CShortCutSqlListDlg::InitSqlList()
{
	CShortCutSqlList	short_cut_sql_list;
	LV_ITEM		item;

	if(short_cut_sql_list.Load() == FALSE) return FALSE;

	m_list_view.DeleteAllItems();

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;

	for(int i = 0; i < short_cut_sql_list.GetSqlCnt(); i++) {
		if(AddList(short_cut_sql_list.GetShortCutSql(i)->GetName(),
			short_cut_sql_list.GetShortCutSql(i)->GetSql(),
			short_cut_sql_list.GetShortCutSql(i)->GetCommand(),
			short_cut_sql_list.GetShortCutSql(i)->IsShowDlg(), 
			short_cut_sql_list.GetShortCutSql(i)->IsPasteToEditor(),
			i) == FALSE) {
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CShortCutSqlListDlg::SaveData()
{
	CShortCutSqlList	short_cut_sql_list;

	BOOL is_show_dlg, is_paste_to_editor;

	for(int i = 0; i < m_list_view.GetItemCount(); i++) {
		is_show_dlg = (m_list_view.GetItemText(i, LIST_IS_SHOW_DLG) == SHOW_DLG_STR);
		is_paste_to_editor = (m_list_view.GetItemText(i, LIST_IS_PASTE_TO_EDITOR) == SHOW_DLG_STR);
		
		if(short_cut_sql_list.Add(m_list_view.GetItemText(i, LIST_NAME),
			m_list_view.GetItemText(i, LIST_SQL),
			(WORD)m_list_view.GetItemData(i),
			is_show_dlg, is_paste_to_editor) == FALSE) return FALSE;
	}

	if(short_cut_sql_list.Save() == FALSE) return FALSE;

	return TRUE;
}

void CShortCutSqlListDlg::OnBtnAdd()
{
	CShortCutSqlDlg dlg;

	dlg.m_command = ID_SHORT_CUT_SQL1 + m_list_view.GetItemCount();
	m_accel_list.delete_accel_from_cmd(dlg.m_command);
	dlg.m_accel_list = m_accel_list;

	if(dlg.DoModal() == IDOK) {
		m_accel_list = dlg.m_accel_list;
		AddList(dlg.m_name, dlg.m_sql, dlg.m_command, dlg.m_is_show_dlg, dlg.m_is_paste_to_editor, -1);
		SetListKey();

		m_list_view.SetItemState(m_list_view.GetItemCount() - 1, LVNI_SELECTED | LVNI_ALL,
			LVNI_SELECTED | LVNI_ALL);
	}

	CheckBtn();
}

void CShortCutSqlListDlg::OnBtnDelete() 
{
	int idx = m_list_view.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(idx == -1) return;

	CString msg;
	msg.Format(_T("%sを削除します。\nよろしいですか？"),
		m_list_view.GetItemText(idx, LIST_NAME));
	if(MessageBox(msg, _T("確認"), MB_ICONQUESTION | MB_OKCANCEL) == IDCANCEL) return;

	m_accel_list.delete_accel_from_cmd((WORD)m_list_view.GetItemData(idx));
	m_list_view.DeleteItem(idx);

	CheckBtn();
}

void CShortCutSqlListDlg::OnBtnModify() 
{
	int idx = m_list_view.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(idx == -1) return;

	CShortCutSqlDlg dlg;

	dlg.m_command = (WORD)m_list_view.GetItemData(idx);
	dlg.m_accel_list = m_accel_list;

	dlg.m_name = m_list_view.GetItemText(idx, LIST_NAME);
	dlg.m_sql = m_list_view.GetItemText(idx, LIST_SQL);

	dlg.m_is_show_dlg = (m_list_view.GetItemText(idx, LIST_IS_SHOW_DLG) == SHOW_DLG_STR);
	dlg.m_is_paste_to_editor = (m_list_view.GetItemText(idx, LIST_IS_PASTE_TO_EDITOR) == SHOW_DLG_STR);

	if(dlg.DoModal() == IDOK) {
		m_accel_list = dlg.m_accel_list;
		SetList(dlg.m_name, dlg.m_sql, dlg.m_command, dlg.m_is_show_dlg, 
			dlg.m_is_paste_to_editor, idx);
		SetListKey();
	}

	CheckBtn();
}

void CShortCutSqlListDlg::OnItemchangedSqlList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;

	CheckBtn();
	
	*pResult = 0;
}

void CShortCutSqlListDlg::CheckBtn()
{
	if(m_list_view.GetItemCount() >= MAX_SHORT_CUT_SQL) {
		m_btn_add.EnableWindow(FALSE);
	} else {
		m_btn_add.EnableWindow(TRUE);
	}

	if(m_list_view.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL) == -1) {
		m_btn_modify.EnableWindow(FALSE);
		m_btn_delete.EnableWindow(FALSE);
	} else {
		m_btn_modify.EnableWindow(TRUE);
		m_btn_delete.EnableWindow(TRUE);
	}
}

void CShortCutSqlListDlg::SetListKey()
{
	CString key;
	for(int i = 0; i < m_list_view.GetItemCount(); i++) {
		if(m_accel_list.search_accel_str2((WORD)m_list_view.GetItemData(i), key) == 0) {
			m_list_view.SetItemText(i, LIST_KEY, key);
		} else {
			m_list_view.SetItemText(i, LIST_KEY, _T(""));
		}
	}
}
