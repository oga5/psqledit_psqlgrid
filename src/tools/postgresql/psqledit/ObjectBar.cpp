/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ObjectBar.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "ObjectBar.h"
#include "localsql.h"
#include "common.h"
#include "file_winutil.h"

#include "ComboBoxUtil.h"
#include "ListCtrlUtil.h"

//#include "sqltuneDoc.h"

#include "oregexp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LIST_CTRL_ID			200
#define OWNER_COMBO_CTRL_ID		201
#define TYPE_COMBO_CTRL_ID		202
#define KEYWORD_EDIT_CTRL_ID	203
#define KEYWORD_BTN_CTRL_ID		204

#define MAX_COLUMN_CNT	20

static const struct {
	const TCHAR *name;
	int			idx;
} object_type_list[] = {
	{ _T("TABLE"), 0 },
	{ _T("FOREIGN TABLE"), 9 },
	{ _T("INDEX"), 1 },
	{ _T("VIEW"), 2 },
	{ _T("MATERIALIZED VIEW"), 7 },
	{ _T("SEQUENCE"), 3 },
	{ _T("FUNCTION"), 4 },
	{ _T("PROCEDURE"), 8 },
	{ _T("TRIGGER"), 5 },
	{ _T("TYPE"), 6 },
};

// カラムの幅を保存する領域
static int list_column_width[sizeof(object_type_list)/sizeof(object_type_list[0])][MAX_COLUMN_CNT];

static int get_object_type_index(TCHAR *name)
{
	int		i;

	for(i = 0; i < sizeof(object_type_list) / sizeof(object_type_list[0]); i++) {
		if(_tcscmp(object_type_list[i].name, name) == 0) return object_type_list[i].idx;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////
// CObjectBar

CObjectBar::CObjectBar()
{
	int		i, j;
	for(i = 0; i < sizeof(object_type_list) / sizeof(object_type_list[0]); i++) {
		for(j = 0; j < MAX_COLUMN_CNT; j++) {
			list_column_width[i][j] = -1;
		}
	}

	m_cur_type = "";

	m_object_list = NULL;
	m_keyword_is_ok = TRUE;
}

CObjectBar::~CObjectBar()
{
}

void CObjectBar::FreeObjectList()
{
	if(m_object_list == NULL) return;

	pg_free_dataset(m_object_list);
	m_object_list = NULL;
}

BEGIN_MESSAGE_MAP(CObjectBar, CSizingControlBar)
	//{{AFX_MSG_MAP(CObjectBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CObjectBar::LoadColumnWidth()
{
	// レジストリからロードする
	int		i, j;
	CString	entry_name = _T("");
	for(i = 0; i < sizeof(object_type_list) / sizeof(object_type_list[0]); i++) {
		for(j = 0; j < MAX_COLUMN_CNT; j++) {
			entry_name.Format(_T("COL_WIDTH_%d_%d"), i, j);
			list_column_width[i][j] = AfxGetApp()->GetProfileInt(_T("OBJECT_BAR"), entry_name, -1);
		}
	}
}

void CObjectBar::SaveColumnWidth()
{
	SaveCurColumnWidth();

	// レジストリに保存する
	int		i, j;
	CString	entry_name = _T("");
	for(i = 0; i < sizeof(object_type_list) / sizeof(object_type_list[0]); i++) {
		for(j = 0; j < MAX_COLUMN_CNT; j++) {
			if(list_column_width[i][j] > 0) {
				entry_name.Format(_T("COL_WIDTH_%d_%d"), i, j);
				AfxGetApp()->WriteProfileInt(_T("OBJECT_BAR"), entry_name, list_column_width[i][j]);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CObjectBar メッセージ ハンドラ

int CObjectBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rect(0, 0, 100, 100);

	m_static_owner.Create(_T("owner"), WS_CHILD | WS_VISIBLE, rect, this);

	m_combo_owner.Create(CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
		rect, this, OWNER_COMBO_CTRL_ID);

	m_static_type.Create(_T("type"), WS_CHILD | WS_VISIBLE, rect, this);

	m_combo_type.Create(CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
		rect, this, TYPE_COMBO_CTRL_ID);

	m_static_keyword.Create(_T("filter"), WS_CHILD | WS_VISIBLE, rect, this);

	m_edit_keyword.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, 
		rect, this, KEYWORD_EDIT_CTRL_ID);
	m_edit_keyword.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	m_btn_keyword.Create(_T("Clear"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		rect, this, KEYWORD_BTN_CTRL_ID);

	m_list.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | 
		LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS,
		rect, this, LIST_CTRL_ID);

	// アイテムを選択したときに，行全体を反転させるようにする
	ListView_SetExtendedListViewStyle(m_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	// マウスホイールのチルトで横スクロール可能にする
	ListCtrl_EnableMouseHWheel(&m_list);

	// キーボード操作の改善
	ComboBoxKeyboardExtend(&m_combo_owner);
	ComboBoxKeyboardExtend(&m_combo_type);

	return 0;
}

void CObjectBar::OnSize(UINT nType, int cx, int cy) 
{
	CSizingControlBar::OnSize(nType, cx, cy);

	SetControlPosition();
}

CString CObjectBar::GetSelectedUser()
{
	CString owner;

	if(m_combo_owner.GetCurSel() == CB_ERR) {
		owner = pg_user(g_ss);
	} else {
		m_combo_owner.GetLBText(m_combo_owner.GetCurSel(), owner);
	}

	return owner;
}

CString CObjectBar::GetSelectedType()
{
	CString type;

	if(m_combo_type.GetCurSel() == CB_ERR) {
		type = "";
	} else {
		m_combo_type.GetLBText(m_combo_type.GetCurSel(), type);
	}

	return type;
}

CString CObjectBar::GetSelectedObject()
{
	CString object_name;

	int selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) {
		object_name = _T("");
	} else {
		object_name = m_list.GetItemText(selected_row, 0);
	}

	return object_name;
}

int CObjectBar::GetColumnIdx(const TCHAR* colname)
{
	int col_cnt = ListCtrl_GetColumnCount(&m_list);

	for(int c = 0; c < col_cnt; c++) {
		if(ListCtrl_GetColumnName(&m_list, c).CompareNoCase(colname) == 0) {
			return c;
		}
	}

	return -1;
}


CString CObjectBar::GetSelectedData(const TCHAR *colname)
{
	CString result = _T("");

	int selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row >= 0) {
		result = m_list.GetItemText(selected_row, GetColumnIdx(colname));
	}

	return result;
}

CString CObjectBar::GetSelectedOid()
{
	return GetSelectedData(_T("oid"));
}

CString CObjectBar::GetSelectedSchema()
{
	return GetSelectedData(_T("nspname"));
}

int CObjectBar::GetSelectedObjectList(CStringArray* object_name_list, CStringArray* object_type_list,
	CStringArray* oid_list, CStringArray* schema_list)
{
	int	i;
	int selected_row;
	int selected_cnt = m_list.GetSelectedCount();

	int oid_idx = GetColumnIdx(_T("oid"));
	int nspname_idx = GetColumnIdx(_T("nspname"));

	for(i = 0, selected_row = -1; i < selected_cnt; i++) {
		selected_row = m_list.GetNextItem(selected_row, LVNI_SELECTED | LVNI_ALL);
		if(selected_row == -1) break;

		CString object_name = m_list.GetItemText(selected_row, 0);
		object_name_list->Add(object_name);

		CString object_type = GetSelectedType();
		object_type_list->Add(object_type);

		CString oid = m_list.GetItemText(selected_row, oid_idx);
		oid_list->Add(oid);

		CString schema = m_list.GetItemText(selected_row, nspname_idx);
		schema_list->Add(schema);
	}

	return selected_cnt;
}



int CObjectBar::SetObjectList(const TCHAR *object_name)
{
	if(GetSelectedUser() == _T("")) return 0;
	if(GetSelectedType() == _T("")) return 0;
	if(g_login_flg == FALSE) return 0;
	
	CWaitCursor	wait_cursor;
	int			ret_v = 0;

	FreeObjectList();

	m_object_list = get_object_list(g_ss, 
		GetSelectedUser().GetBuffer(0), 
		GetSelectedType().GetBuffer(0), 
		g_msg_buf);
	if(m_object_list == NULL) {
		MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		goto ERR1;
	}

	ret_v = SetObjectList(m_object_list, object_name);
	if(ret_v != 0) goto ERR1;

	return 0;

ERR1:
	FreeObjectList();

	return ret_v;
}

void CObjectBar::SetKeywordOK(BOOL b_ok)
{
	if(m_keyword_is_ok == b_ok) return;

	m_keyword_is_ok = b_ok;
	m_edit_keyword.RedrawWindow();
}

int CObjectBar::SetObjectList(HPgDataset dataset, const TCHAR *object_name)
{
	if(dataset == NULL) return 0;

	CWaitCursor	wait_cursor;
	int			r, c;
	LV_ITEM		item;
	CListCtrl	*p_list = &m_list;

	if(!IsWindow(p_list->GetSafeHwnd())) return 1;

	SetListColumns(dataset);

	p_list->SetRedraw(FALSE);

	p_list->DeleteAllItems();
	p_list->SetItemCount(pg_dataset_row_cnt(dataset));

	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.iSubItem = 0;

	CString search_text;
	m_edit_keyword.GetWindowText(search_text);

	HREG_DATA hreg = NULL;

	if(search_text != _T("")) {
		hreg = oreg_comp(search_text, 1);
		if(hreg == NULL) {
			SetKeywordOK(FALSE);
		} else {
			SetKeywordOK(TRUE);
		}
	}

	CString data_str;
	int item_idx;

	int comment_idx = pg_dataset_get_col_no(dataset, _T("comment"));
	int regex_result;

	for(r = 0, item_idx = 0; r < pg_dataset_row_cnt(dataset); r++) {
		if(hreg != NULL) {
			OREG_DATASRC data_src;
			data_str = pg_dataset_data(dataset, r, 0);
			data_str.MakeLower();
			oreg_make_str_datasrc(&data_src, data_str);

			regex_result = oreg_exec2(hreg, &data_src);
			if (regex_result != OREGEXP_FOUND) {
				if (g_option.object_list_filter_column == OFC_FIRST_AND_COMMENT_COLUMN && comment_idx > 0) {
					// コメントを検索
					data_str = pg_dataset_data(dataset, r, comment_idx);
					data_str.MakeLower();
					oreg_make_str_datasrc(&data_src, data_str);
					regex_result = oreg_exec2(hreg, &data_src);
				}
				if (g_option.object_list_filter_column == OFC_ALL_COLUMN) {
					for (c = 1; c < pg_dataset_col_cnt(dataset); c++) {
						data_str = pg_dataset_data(dataset, r, c);
						data_str.MakeLower();
						oreg_make_str_datasrc(&data_src, data_str);
						regex_result = oreg_exec2(hreg, &data_src);
						if (regex_result == OREGEXP_FOUND) break;
					}
				}
			}
			if (regex_result != OREGEXP_FOUND) continue;
		}

		item.iItem = item_idx;
		item.lParam = r;
		item.pszText = (TCHAR *)pg_dataset_data(dataset, r, 0);
		item.iItem = p_list->InsertItem(&item);
		if(item.iItem == -1) goto ERR1;

		for(c = 1; c < pg_dataset_col_cnt(dataset); c++) {
			p_list->SetItemText(item.iItem, c, pg_dataset_data(dataset, r, c));
		}

		if(object_name != NULL && _tcscmp(object_name, item.pszText) == 0) {
			p_list->SetItemState(item.iItem, LVIS_SELECTED | LVIS_FOCUSED, 
				LVIS_SELECTED | LVIS_FOCUSED);
			// アイテムを可視にする
			p_list->EnsureVisible(item.iItem, FALSE);
		}

		item_idx++;
	}

	if(object_name != NULL && GetSelectedObject() == "") {
		g_object_detail_bar.ClearColumnList();
	}

	p_list->SetRedraw(TRUE);

	if(hreg != NULL) oreg_free(hreg);

	return 0;

ERR1:
//	MessageBox("リストビュー初期化エラー", "Error", MB_ICONEXCLAMATION | MB_OK);
	p_list->SetRedraw(TRUE);

	if(hreg != NULL) oreg_free(hreg);

	return 1;
}

void CObjectBar::OnListViewItemChanged(NM_LISTVIEW *p_nmhdr)
{
	if((p_nmhdr->uChanged & LVIF_STATE) == 0) return;

	if((p_nmhdr->uNewState & (LVIS_SELECTED | LVNI_ALL)) != 0) {
		CWaitCursor wait_cursor;
		CString object_name;

		g_object_detail_bar.SetObjectData(GetSelectedUser(), GetSelectedObject(),
			GetSelectedOid(), GetSelectedSchema(), GetSelectedType());
	} else {
		g_object_detail_bar.ClearColumnList();
	}
}

void CObjectBar::OnListViewColumnClick(NM_LISTVIEW *p_nmhdr)
{
	static int sort_order = 1;

	ListCtrlSortEx(&m_list, p_nmhdr->iSubItem, sort_order);

	// 昇順・降順を切りかえる
	sort_order = -sort_order;
}

void CObjectBar::OnRClickListView()
{
	if(GetSelectedUser() == _T("") || GetSelectedObject() == _T("") || GetSelectedType() == _T("")) return;

	POINT	pt;
	CMenu	menu;

	int selected_item_cnt = m_list.GetSelectedCount();

	GetCursorPos(&pt);

	// ポップアップメニューを作成
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, ID_GET_OBJECT_SOURCE, _T("オブジェクトのソースを取得"));
	menu.AppendMenu(MF_SEPARATOR, 0, _T(""));

	menu.AppendMenu(MF_STRING, ID_MAKE_SELECT_SQL_ALL, _T("Select文作成"));
	menu.AppendMenu(MF_STRING, ID_MAKE_INSERT_SQL_ALL, _T("Insert文作成"));

	if(g_login_flg == FALSE) {
		menu.EnableMenuItem(ID_GET_OBJECT_SOURCE, MF_GRAYED);
		menu.EnableMenuItem(ID_MAKE_SELECT_SQL_ALL, MF_GRAYED);
		menu.EnableMenuItem(ID_MAKE_INSERT_SQL_ALL, MF_GRAYED);
	} else {
		if(!CanGetSource()) {
			menu.EnableMenuItem(ID_GET_OBJECT_SOURCE, MF_GRAYED);
		}
		if((GetSelectedType() != _T("TABLE") &&
			GetSelectedType() != _T("FOREIGN TABLE") &&
			GetSelectedType() != _T("VIEW") &&
			GetSelectedType() != _T("MATERIALIZED VIEW")) ||
			selected_item_cnt != 1) {
			menu.EnableMenuItem(ID_MAKE_SELECT_SQL_ALL, MF_GRAYED);
			menu.EnableMenuItem(ID_MAKE_INSERT_SQL_ALL, MF_GRAYED);
		}

		if((GetSelectedType() == _T("TABLE")) &&
			GetSelectedSchema() != _T("pg_catalog") &&
			GetSelectedSchema() != _T("information_schema") &&
			selected_item_cnt == 1) {
			menu.AppendMenu(MF_SEPARATOR, 0, _T(""));
			menu.AppendMenu(MF_STRING, ID_PSQLGRID_OBJECTBAR, _T("PSqlGridを起動"));
		}
	}

	// メニュー表示
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);

	// メニュー削除
	menu.DestroyMenu();
}

void CObjectBar::OnDblClkListView()
{
	CString		table_name;
	int			selected_row;

	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	make_object_name(&table_name, m_list.GetItemText(selected_row, 0).GetBuffer(0),
		g_option.text_editor.copy_lower_name);

	int nspname_idx = GetColumnIdx(_T("nspname"));
	if (nspname_idx >= 0) {
		CString nspname = m_list.GetItemText(selected_row, nspname_idx);
		if (!is_schema_in_search_path(g_ss, nspname, g_msg_buf)) {
			CString		schema_name;

			// スキーマが検索パスにない場合は、スキーマ名を付加する
			if (nspname != _T("")) {
				make_object_name(&schema_name, nspname.GetBuffer(0), g_option.text_editor.copy_lower_name);
				table_name = schema_name + "." + table_name;
			}
		}
	}

	CDocument *pdoc = ::GetActiveDocument();
	if(pdoc != NULL) {
		pdoc->UpdateAllViews(NULL, UPD_PASTE_OBJECT_NAME, (CObject *)table_name.GetBuffer(0));
	}
}

int CObjectBar::SetUserList(const TCHAR *user_name)
{
	if(g_login_flg == FALSE) return 0;

	CWaitCursor	wait_cursor;
	int			ret_v = 0;
	int			i;
	int			item;
	HPgDataset	dataset = NULL;
	CComboBox	*p_combo = &m_combo_owner;

	if(!IsWindow(p_combo->GetSafeHwnd())) goto ERR1;

	for(;;) {
		if(p_combo->DeleteString(0) == CB_ERR) break;
	}

	dataset = get_user_list(g_ss, g_msg_buf);
	if(dataset == NULL) {
		MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		goto ERR1;
	}

	for(i = 0; i < pg_dataset_row_cnt(dataset); i++) {
		item = p_combo->AddString(pg_dataset_data(dataset, i, 0));
		if(item == CB_ERR) {
			ret_v = 1;
			goto ERR1;
		}
		const TCHAR *user = pg_dataset_data(dataset, i, 0);
		if(_tcscmp(user_name, pg_dataset_data(dataset, i, 0)) == 0) {
			p_combo->SetCurSel(item);
		}
	}
	item = p_combo->AddString(ALL_USERS);
	if(item == CB_ERR) {
		ret_v = 1;
		goto ERR1;
	} else {
		if(_tcscmp(user_name, ALL_USERS) == 0) {
			p_combo->SetCurSel(item);
		}
	}

	pg_free_dataset(dataset);

	// オブジェクトタイプリストを登録
//	int i;
	if(m_combo_type.GetItemData(0) == CB_ERR) {
		for(i = 0; i < sizeof(object_type_list) / sizeof(object_type_list[0]); i++) {
			m_combo_type.AddString(object_type_list[i].name);
		}
	}

	return 0;

ERR1:
	pg_free_dataset(dataset);
	return ret_v;
}

void CObjectBar::OnOwnerChanged()
{
	SetObjectList(NULL);
}

void CObjectBar::OnTypeChanged()
{
	g_object_detail_bar.SetObjectType(GetSelectedType().GetBuffer(0)); 
	SetObjectList(NULL);
}

BOOL CObjectBar::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(LOWORD(wParam)) {
	case OWNER_COMBO_CTRL_ID:
		switch(HIWORD(wParam)) {
		case CBN_SELCHANGE:
			OnOwnerChanged();
			break;
		}
		break;
	case TYPE_COMBO_CTRL_ID:
		switch(HIWORD(wParam)) {
		case CBN_SELCHANGE:
			OnTypeChanged();
			break;
		}
		break;
	case KEYWORD_EDIT_CTRL_ID:
		switch(HIWORD(wParam)) {
		case EN_CHANGE:
			OnKeywordChanged();
			break;
		}
		break;
	case KEYWORD_BTN_CTRL_ID:
		switch(HIWORD(wParam)) {
		case BN_CLICKED:
			{
				CString search_text;
				m_edit_keyword.GetWindowText(search_text);
				if(search_text != _T("")) m_edit_keyword.SetWindowText(_T(""));
			}
			break;
		}
	}
	
	return CSizingControlBar::OnCommand(wParam, lParam);
}

BOOL CObjectBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	if(wParam == LIST_CTRL_ID) {
		NM_LISTVIEW	*p_nmhdr = (NM_LISTVIEW *)lParam;
		switch(p_nmhdr->hdr.code) {
		case LVN_ITEMCHANGED:
			OnListViewItemChanged(p_nmhdr);
			break;
		case LVN_COLUMNCLICK:
			OnListViewColumnClick(p_nmhdr);
			break;
		case NM_DBLCLK:
			OnDblClkListView();
			break;
		case NM_RCLICK:
			OnRClickListView();
			break;
		}
	}

	return CSizingControlBar::OnNotify(wParam, lParam, pResult);
}

void CObjectBar::SetFont(CFont *font)
{
	m_static_owner.SetFont(font);
	m_combo_owner.SetFont(font);

	m_static_type.SetFont(font);
	m_combo_type.SetFont(font);

	m_static_keyword.SetFont(font);
	m_edit_keyword.SetFont(font);
	m_btn_keyword.SetFont(font);

	m_list.SetFont(font);

	m_font_height = CalcFontHeight(this, font);
	SetControlPosition();
}

void CObjectBar::SetControlPosition()
{
	CRect	rect;
	RECT	child_rect;

	// リサイズ中のちらつきを抑える
	LockWindowUpdate();

	GetClientRect(rect);

	m_static_owner.MoveWindow(0, 0, 50, m_font_height);
	m_combo_owner.MoveWindow(60, 0, rect.Width() - 60, rect.bottom - 20);

	m_combo_owner.GetWindowRect(&child_rect);
	ScreenToClient(&child_rect);

	m_static_type.MoveWindow(0, child_rect.bottom + 2, 50, m_font_height);
	m_combo_type.MoveWindow(60, child_rect.bottom + 2, rect.Width() - 60, rect.bottom - 20);

	m_combo_type.GetWindowRect(&child_rect);
	ScreenToClient(&child_rect);

	m_static_keyword.MoveWindow(0, child_rect.bottom + 2, 50, m_font_height);
	m_edit_keyword.MoveWindow(60, child_rect.bottom + 2, 
		rect.Width() - 60 - m_font_height * 3, m_font_height + 6);
	m_btn_keyword.MoveWindow(rect.Width() - m_font_height * 3, child_rect.bottom + 2,
		m_font_height * 3, m_font_height + 6);

	m_edit_keyword.GetWindowRect(&child_rect);
	ScreenToClient(&child_rect);

	m_list.MoveWindow(0, child_rect.bottom + 2, 
		rect.Width(), rect.Height() - (child_rect.bottom + 2));

	UnlockWindowUpdate();

//	Invalidate();
}

int CObjectBar::SetListColumns(HPgDataset dataset)
{
	if(dataset == NULL) return 0;
	ASSERT(pg_dataset_col_cnt(dataset) < MAX_COLUMN_CNT);

	if(m_cur_type.Compare(GetSelectedType()) == 0) return 0;

	SaveCurColumnWidth();

	int			c, col_width;
	CListCtrl	*p_list = &m_list;

	int idx_new = get_object_type_index(GetSelectedType().GetBuffer(0));

	m_cur_type = GetSelectedType();

	// カラムを削除
	for(; p_list->DeleteColumn(0) != FALSE; ) ;

	// カラムを追加
	for(c = 0; c < pg_dataset_col_cnt(dataset); c++) {
		if(list_column_width[idx_new][c] < 0) {
			if(pg_dataset_get_colsize(dataset, c) > 0 && pg_dataset_get_colsize(dataset, c) < 20) {
				col_width = (pg_dataset_get_colsize(dataset, c) + 1) * m_font_height / 2;
			} else {
				col_width = 20 * m_font_height / 2;
			}
		} else {
			col_width = list_column_width[idx_new][c];
		}

		p_list->InsertColumn(c, pg_dataset_get_colname(dataset, c), LVCFMT_LEFT, col_width);
	}

	return 0;
}

int CObjectBar::InitializeList(const TCHAR *user_name)
{
	if(g_option.init_table_list_user == 0) {
		int ret_v = SetUserList(user_name);
		if(ret_v != 0) return ret_v;
	} else {
		int ret_v = SetUserList(ALL_USERS);
		if(ret_v != 0) return ret_v;
	}
	
	int idx = ComboFindString(m_combo_type, g_option.init_object_list_type);
	if(idx != CB_ERR) {
		m_combo_type.SetCurSel(idx);
		OnTypeChanged();
	}

	return 0;
}

int CObjectBar::RefreshList()
{
	CString		user_name;
	CString		object_name;

	user_name = GetSelectedUser();
	object_name = GetSelectedObject();
	
	SetUserList(user_name.GetBuffer(0));
	SetObjectList(object_name.GetBuffer(0));

	return 0;
}

void CObjectBar::SaveCurColumnWidth()
{
	// 現在のカラム幅を保存する
	CListCtrl	*p_list = &m_list;
	int idx = get_object_type_index(m_cur_type.GetBuffer(0));

	LV_COLUMN	lv_column;
	lv_column.mask = LVCF_WIDTH;

	int		col_cnt;
	for(col_cnt = 0; p_list->GetColumn(col_cnt, &lv_column) != 0; col_cnt++) ;

	int		c;
	for(c = 0; c < col_cnt; c++) {
		list_column_width[idx][c] = p_list->GetColumnWidth(c);
	}	
}

void CObjectBar::OnDestroy() 
{
	CSizingControlBar::OnDestroy();

	SaveCurColumnWidth();
}

void CObjectBar::ClearData()
{
	if(IsWindow(this->GetSafeHwnd()) == FALSE) return;

	// owner
	for(;;) {
		if(m_combo_owner.DeleteString(0) == CB_ERR) break;
	}

	// type
	for(;;) {
		if(m_combo_type.DeleteString(0) == CB_ERR) break;
	}

	// list
	m_list.DeleteAllItems();
}

void CObjectBar::OnKeywordChanged()
{
	SetObjectList(m_object_list, GetSelectedObject().GetBuffer(0));

	POINT pt;
	::GetCursorPos(&pt);
	::SetCursorPos(pt.x, pt.y);
}

HBRUSH CObjectBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CSizingControlBar::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: この位置で DC のアトリビュートを変更してください
	if(pWnd == &m_edit_keyword && m_keyword_is_ok == FALSE) {
		pDC->SetTextColor(RGB(255, 0, 0));
	}

	return hbr;
}

BOOL CObjectBar::isKeywordActive()
{
	if(GetFocus() == &m_edit_keyword) return TRUE;
	return FALSE;
}

void CObjectBar::PasteToKeyword()
{
	m_edit_keyword.Paste();
}

void CObjectBar::CopyFromKeyword()
{
	m_edit_keyword.Copy();
}

void CObjectBar::CutKeyword()
{
	DWORD selected = m_edit_keyword.GetSel();
	if(LOWORD(selected) == HIWORD(selected)) SelectAllKeyword();

	m_edit_keyword.Cut();
}

void CObjectBar::SelectAllKeyword()
{
	m_edit_keyword.SetSel(0, -1);
}


BOOL CObjectBar::CanGetSource()
{
	CString obj_type = GetSelectedType();

	if(obj_type == "FUNCTION" ||
		obj_type == "PROCEDURE" ||
		obj_type == "SEQUENCE" ||
		obj_type == "TRIGGER" ||
		obj_type == "VIEW" ||
		obj_type == "MATERIALIZED VIEW" ||
		obj_type == "TYPE" ||
		obj_type == "INDEX" ||
		obj_type == "TABLE") {
		return TRUE;
	} else {
		return FALSE;
	}
}