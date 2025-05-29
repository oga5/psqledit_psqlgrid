/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ObjectDetailBar.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "ObjectDetailBar.h"

#include "localsql.h"
#include "common.h"
#include "file_winutil.h"
#include "ListCtrlUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_COLUMN_CNT	10

enum ListCtrl
{
	COLUMN_LIST,
	INDEX_LIST,
	TRIGGER_LIST,
	PROPERTY_LIST,
	ListCtrlSize
};

struct column_info_st {
	TCHAR	*name;
	int		format;
	int		width;
};
static struct column_info_st column_info[ListCtrlSize][MAX_COLUMN_CNT];

#define OBJECT_TYPE_TABLE	0
#define OBJECT_TYPE_INDEX	1
#define OBJECT_TYPE_VIEW	2
#define OBJECT_TYPE_SYNONYM	3
#define OBJECT_TYPE_MATERIALIZED_VIEW	4
#define OBJECT_TYPE_FOREIGN_TABLE	5
#define OBJECT_TYPE_OTHER	99

#define TAB_CTRL_ID				202
#define LIST_CTRL_ID			203
#define KEYWORD_EDIT_CTRL_ID	214
#define KEYWORD_BTN_CTRL_ID		215

enum Column_list
{
	TABLE_COLUMN_NAME,
	DATA_TYPE,
	DATA_LENGTH,
	NOT_NULL,
	COMMENTS,
	COLUMN_ID,
};

enum Index_list
{
	INDEX_NAME,
	INDEX_COLUMN_NAME,
	INDEX_IS_UNIQUE,
	INDEX_IS_PRIMARY,
	INDEX_OID,
	TABLE_OID,
};

enum Trigger_list
{
	TRIGGER_NAME,
	TRIGGER_TYPE,
	TRIGGER_EVENT,
	TRIGGER_STATUS,
};

enum Property_list
{
	PROPERTY_NAME,
	PROPERTY_VALUE,
};


/////////////////////////////////////////////////////////////////////////////
// CObjectDetailBar

CObjectDetailBar::CObjectDetailBar()
{
	m_tab_ctrl_type = -1;
	m_list_type = -1;
	m_dataset = NULL;
	m_keyword_is_ok = TRUE;

	int		i;
	int		j;
	for(i = 0; i < ListCtrlSize; i++) {
		for(j = 0; j < MAX_COLUMN_CNT; j++) {
			column_info[i][j].name = NULL;
			column_info[i][j].format = LVCFMT_LEFT;
			column_info[i][j].width = -1;
		}
	}

	column_info[COLUMN_LIST][TABLE_COLUMN_NAME].name = _T("column name");
	column_info[COLUMN_LIST][TABLE_COLUMN_NAME].width = 100;
	column_info[COLUMN_LIST][DATA_TYPE].name = _T("data type");
	column_info[COLUMN_LIST][DATA_TYPE].width = 70;
	column_info[COLUMN_LIST][DATA_LENGTH].name = _T("length");
	column_info[COLUMN_LIST][DATA_LENGTH].format = LVCFMT_RIGHT;
	column_info[COLUMN_LIST][DATA_LENGTH].width = 30;
	column_info[COLUMN_LIST][NOT_NULL].name = _T("not null");
	column_info[COLUMN_LIST][NOT_NULL].width = 30;
	column_info[COLUMN_LIST][COMMENTS].name = _T("comment");
	column_info[COLUMN_LIST][COMMENTS].width = 100;
	column_info[COLUMN_LIST][COLUMN_ID].name = _T("no");
	column_info[COLUMN_LIST][COLUMN_ID].format = LVCFMT_RIGHT;
	column_info[COLUMN_LIST][COLUMN_ID].width = 25;

	column_info[INDEX_LIST][INDEX_NAME].name = _T("index name");
	column_info[INDEX_LIST][INDEX_NAME].width = 120;
	column_info[INDEX_LIST][INDEX_COLUMN_NAME].name = _T("column name");
	column_info[INDEX_LIST][INDEX_COLUMN_NAME].width = 120;
	column_info[INDEX_LIST][INDEX_IS_UNIQUE].name = _T("unique");
	column_info[INDEX_LIST][INDEX_IS_UNIQUE].width = 50;
	column_info[INDEX_LIST][INDEX_IS_PRIMARY].name = _T("primary");
	column_info[INDEX_LIST][INDEX_IS_PRIMARY].width = 50;
	column_info[INDEX_LIST][INDEX_OID].name = _T("index oid");
	column_info[INDEX_LIST][INDEX_OID].width = 10;
	column_info[INDEX_LIST][TABLE_OID].name = _T("table oid");
	column_info[INDEX_LIST][TABLE_OID].width = 10;

	column_info[TRIGGER_LIST][TRIGGER_NAME].name = _T("trigger name");
	column_info[TRIGGER_LIST][TRIGGER_NAME].width = 120;
	column_info[TRIGGER_LIST][TRIGGER_TYPE].name = _T("type");
	column_info[TRIGGER_LIST][TRIGGER_TYPE].width = 70;
	column_info[TRIGGER_LIST][TRIGGER_EVENT].name = _T("event");
	column_info[TRIGGER_LIST][TRIGGER_EVENT].width = 70;
	column_info[TRIGGER_LIST][TRIGGER_STATUS].name = _T("status");
	column_info[TRIGGER_LIST][TRIGGER_STATUS].width = 70;

	column_info[PROPERTY_LIST][PROPERTY_NAME].name = _T("name");
	column_info[PROPERTY_LIST][PROPERTY_NAME].width = 100;
	column_info[PROPERTY_LIST][PROPERTY_VALUE].name = _T("value");
	column_info[PROPERTY_LIST][PROPERTY_VALUE].width = 100;
}

CObjectDetailBar::~CObjectDetailBar()
{
}

void CObjectDetailBar::FreeObjectList()
{
	if(m_dataset == NULL) return;

	pg_free_dataset(m_dataset);
	m_dataset = NULL;
}

BEGIN_MESSAGE_MAP(CObjectDetailBar, CSizingControlBar)
	//{{AFX_MSG_MAP(CObjectDetailBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CObjectDetailBar::LoadColumnWidth()
{
	// レジストリからロードする
	int		i, j;
	CString	entry_name = _T("");
	for(i = 0; i < ListCtrlSize; i++) {
		for(j = 0; j < MAX_COLUMN_CNT; j++) {
			entry_name.Format(_T("COL_WIDTH_%d_%d"), i, j);
			if(AfxGetApp()->GetProfileInt(_T("OBJECT_DETAIL_BAR"), entry_name, -1) != -1) {
				column_info[i][j].width = AfxGetApp()->GetProfileInt(_T("OBJECT_DETAIL_BAR"), entry_name, -1);
			}
		}
	}
}

void CObjectDetailBar::SaveColumnWidth()
{
	SaveCurColumnWidth();

	// レジストリに保存する
	int		i, j;
	CString	entry_name = _T("");
	for(i = 0; i < ListCtrlSize; i++) {
		for(j = 0; j < MAX_COLUMN_CNT; j++) {
			if(column_info[i][j].width != -1) {
				entry_name.Format(_T("COL_WIDTH_%d_%d"), i, j);
				AfxGetApp()->WriteProfileInt(_T("OBJECT_DETAIL_BAR"), entry_name, column_info[i][j].width);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CObjectDetailBar メッセージ ハンドラ

int CObjectDetailBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CSizingControlBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rect(0, 0, 100, 100);

	m_tab_ctrl.Create(WS_CHILD | WS_VISIBLE | TCS_TABS | TCS_SINGLELINE | TCS_RAGGEDRIGHT | TCS_OWNERDRAWFIXED,
		rect, this, TAB_CTRL_ID);
	m_tab_ctrl.b_min_erase_bkgnd = TRUE;

	m_static_keyword.Create(_T("filter"), WS_CHILD | WS_VISIBLE, rect, this);

	m_edit_keyword.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, rect, this, KEYWORD_EDIT_CTRL_ID);
	m_edit_keyword.ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	m_btn_keyword.Create(_T("clear"), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		rect, this, KEYWORD_BTN_CTRL_ID);

	m_list.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS,
		rect, this, LIST_CTRL_ID);

	ListView_SetExtendedListViewStyle(m_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	// マウスホイールのチルトで横スクロール可能にする
	ListCtrl_EnableMouseHWheel(&m_list);

	return 0;
}

void CObjectDetailBar::OnSize(UINT nType, int cx, int cy) 
{
	CSizingControlBar::OnSize(nType, cx, cy);

	m_tab_ctrl.SetWindowPos(&wndBottom, 0, 0, cx, cy, SWP_NOACTIVATE);
	AdjustChildWindows();
}

void CObjectDetailBar::OnListCtrlColumnClick(NM_LISTVIEW *p_nmhdr, CListCtrl *list_ctrl)
{
	static int sort_order = 1;

	ListCtrlSortEx(list_ctrl, p_nmhdr->iSubItem, sort_order);

	// 昇順・降順を切りかえる
	sort_order = -sort_order;
}

void CObjectDetailBar::OnDblClkListView()
{
	CString paste_str;
	int		selected_cnt;

	selected_cnt = m_list.GetSelectedCount();

	if(selected_cnt != 1) {
		paste_str = "\n\t";
	}

	paste_str += MakeColumnList(0, FALSE, NULL, FALSE);

	CDocument *pdoc = ::GetActiveDocument();
	if(pdoc != NULL) {
		pdoc->UpdateAllViews(NULL, UPD_PASTE_OBJECT_NAME, (CObject *)paste_str.GetBuffer(0));
	}
}

void CObjectDetailBar::SetObjectType(TCHAR *object_type)
{
	ClearColumnList();

	SetTabCtrl(object_type);
}

void CObjectDetailBar::ClearColumnList()
{
	if(IsWindow(this->GetSafeHwnd()) == FALSE) return;

	m_list.DeleteAllItems();
}

int CObjectDetailBar::SetObjectList(HPgDataset dataset)
{
	if(dataset == NULL) return 0;

	CWaitCursor	wait_cursor;
	int			r, c;
	LV_ITEM		item;
	CListCtrl	*p_list = &m_list;

	if(!IsWindow(p_list->GetSafeHwnd())) return 1;

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

	p_list->SetRedraw(FALSE);

	p_list->DeleteAllItems();
	p_list->SetItemCount(pg_dataset_row_cnt(dataset));

	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.iSubItem = 0;

	int item_idx;
	int comment_idx = pg_dataset_get_col_no(dataset, _T("description"));

	int regex_result;

	for(r = 0, item_idx = 0; r < pg_dataset_row_cnt(dataset); r++) {
		if(m_list_type == COLUMN_LIST && hreg != NULL) {
			OREG_DATASRC data_src;
			CString data_str = pg_dataset_data(dataset, r, TABLE_COLUMN_NAME);
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
			if(m_list_type == COLUMN_LIST && c == DATA_LENGTH &&
				_tcscmp(pg_dataset_data(dataset, r, DATA_TYPE), _T("numeric")) == 0) {

				int len = _ttoi(pg_dataset_data(dataset, r, DATA_LENGTH));
				int prec = (len >> 16) & 0xffff;
				int scale = len & 0xffff;

				CString num_len = _T("-1");
				if(prec != 0xffff && scale != 0xffff) {
					if(scale == 0) {
						num_len.Format(_T("%d"), prec);
					} else {
						num_len.Format(_T("%d,%d"), prec, scale);
					}
				}
				p_list->SetItemText(item.iItem, c, num_len);
			} else {
				p_list->SetItemText(item.iItem, c, pg_dataset_data(dataset, r, c));
			}
		}

		item_idx++;
	}

	p_list->SetRedraw(TRUE);

	if (hreg != NULL) oreg_free(hreg);

	return 0;

ERR1:
//	MessageBox("リストビュー初期化エラー", "Error", MB_ICONEXCLAMATION | MB_OK);
	p_list->SetRedraw(TRUE);

	if (hreg != NULL) oreg_free(hreg);

	return 1;
}

int CObjectDetailBar::SetColumnList()
{
	if(g_login_flg == FALSE) return 0;

	CWaitCursor	wait_cursor;

	FreeObjectList();

	m_dataset = get_column_list(g_ss, m_object_name, m_schema, g_msg_buf);
	if(m_dataset == NULL) goto ERR1;

	SetListColumns(COLUMN_LIST);
	SetObjectList(m_dataset);

	return 0;

ERR1:
	MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void CObjectDetailBar::OnRClickListView()
{
	if(m_list.GetSelectedCount() <= 0) return;

	POINT	pt;
	CMenu	menu;

	GetCursorPos(&pt);

	// ポップアップメニューを作成
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, ID_MAKE_SELECT_SQL, _T("Select文作成"));
	menu.AppendMenu(MF_STRING, ID_MAKE_INSERT_SQL, _T("Insert文作成"));
	menu.AppendMenu(MF_STRING, ID_MAKE_UPDATE_SQL, _T("Update文作成"));

	// メニュー表示
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);

	// メニュー削除
	menu.DestroyMenu();
}

CString CObjectDetailBar::GetSelectedItemString()
{
	if (m_list.GetSelectedCount() <= 0) return _T("");

	int selected_row = -1;

	selected_row = m_list.GetNextItem(selected_row, LVNI_SELECTED | LVNI_ALL);
	if (selected_row == -1) return _T("");

	return m_list.GetItemText(selected_row, 0);
}

void CObjectDetailBar::OnRClickListView_Index()
{
	if (m_list.GetSelectedCount() <= 0) return;
	if (!CanGetSource()) return;

	POINT	pt;
	CMenu	menu;

	GetCursorPos(&pt);

	// ポップアップメニューを作成
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, ID_GET_OBJECT_SOURCE_DETAIL_BAR, _T("オブジェクトのソースを取得"));

	// メニュー表示
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);

	// メニュー削除
	menu.DestroyMenu();
}

void CObjectDetailBar::OnRClickListView_Property()
{
	if(m_list.GetSelectedCount() <= 0) return;

	POINT	pt;
	CMenu	menu;

	GetCursorPos(&pt);

	// ポップアップメニューを作成
	menu.CreatePopupMenu();

	menu.AppendMenu(MF_STRING, ID_COPY_VALUE, _T("Copy value"));

	// メニュー表示
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);

	// メニュー削除
	menu.DestroyMenu();
}

BOOL CObjectDetailBar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
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

BOOL CObjectDetailBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch(wParam) {
	case TAB_CTRL_ID:
		{
			NMHDR *p_nmhdr = (NMHDR *)lParam;
			switch(p_nmhdr->code) {
			case TCN_SELCHANGE:
				OnTabSelChanged();
				break;
			}
		}
		break;
	case LIST_CTRL_ID:
		switch(m_list_type) {
			case COLUMN_LIST:
			{
				NM_LISTVIEW *p_nmhdr = (NM_LISTVIEW *)lParam;
				switch(p_nmhdr->hdr.code) {
				case LVN_COLUMNCLICK:
					OnListCtrlColumnClick(p_nmhdr, &m_list);
					break;
				case NM_DBLCLK:
					OnDblClkListView();
					break;
				case NM_RCLICK:
					OnRClickListView();
					break;
				}
			}
			break;
			case INDEX_LIST:
			{
				NM_LISTVIEW* p_nmhdr = (NM_LISTVIEW*)lParam;
				switch (p_nmhdr->hdr.code) {
				case NM_RCLICK:
					OnRClickListView_Index();
					break;
				}
			}
			break;
			case TRIGGER_LIST:
			{
				NM_LISTVIEW *p_nmhdr = (NM_LISTVIEW *)lParam;
				switch(p_nmhdr->hdr.code) {
				case LVN_COLUMNCLICK:
					OnListCtrlColumnClick(p_nmhdr, &m_list);
					break;
				case NM_DBLCLK:
					OnDblClkListView();
					break;
				}
			}
			break;
			case PROPERTY_LIST:
			{
				NM_LISTVIEW *p_nmhdr = (NM_LISTVIEW *)lParam;
				switch(p_nmhdr->hdr.code) {
				case LVN_COLUMNCLICK:
					OnListCtrlColumnClick(p_nmhdr, &m_list);
					break;
				case NM_RCLICK:
					OnRClickListView_Property();
					break;
				}
			}
			break;
			default:
			{
				NM_LISTVIEW *p_nmhdr = (NM_LISTVIEW *)lParam;
				switch(p_nmhdr->hdr.code) {
				case NM_DBLCLK:
					OnDblClkListView();
					break;
				}
			}
			break;
		}
	}
	
	return CSizingControlBar::OnNotify(wParam, lParam, pResult);
}

void CObjectDetailBar::SetFont(CFont *font)
{
	m_tab_ctrl.SetFont(font);
	m_list.SetFont(font);

	m_static_keyword.SetFont(font);
	m_edit_keyword.SetFont(font);
	m_btn_keyword.SetFont(font);

	m_font_height = CalcFontHeight(this, font);

	AdjustChildWindows();
}

int CObjectDetailBar::SetObjectData(const TCHAR *owner, const TCHAR *object_name,
	const TCHAR *oid, const TCHAR *schema, const TCHAR *object_type)
{
	CWaitCursor	wait_cursor;
	int		ret_v;

	m_owner = owner;
	m_object_name = object_name;
	m_oid = oid;
	m_schema = schema;

	SetTabCtrl(object_type);

	// データを更新する
	ret_v = OnTabSelChanged();

	return ret_v;
}

int CObjectDetailBar::SetIndexList()
{
	if(g_login_flg == FALSE) return 0;

	CWaitCursor	wait_cursor;
	HPgDataset	dataset = NULL;

	int			i;
	LV_ITEM		item;
	CString		index_name;

	dataset = get_index_list_by_table(g_ss, m_object_name, m_schema, g_msg_buf);
	if(dataset == NULL) goto ERR1;

	SetListColumns(INDEX_LIST);

	m_list.DeleteAllItems();
	m_list.SetItemCount(pg_dataset_row_cnt(dataset));

	item.mask = LVIF_PARAM;
	item.iSubItem = 0;

	index_name = "";
	for(i = 0; i < pg_dataset_row_cnt(dataset); i++) {
		item.iItem = i;
		item.lParam = i;
		item.iItem = m_list.InsertItem(&item);
		if(item.iItem == -1) goto ERR2;
		if(index_name != pg_dataset_data(dataset, i, 0)) {
			index_name = pg_dataset_data(dataset, i, 0);
			if(m_list.SetItemText(item.iItem, INDEX_NAME,
				pg_dataset_data(dataset, i, 0)) == FALSE) goto ERR2;
			if(m_list.SetItemText(item.iItem, INDEX_IS_UNIQUE,
				pg_dataset_data(dataset, i, 2)) == FALSE) goto ERR2;
			if(m_list.SetItemText(item.iItem, INDEX_IS_PRIMARY,
				pg_dataset_data(dataset, i, 3)) == FALSE) goto ERR2;
			if (m_list.SetItemText(item.iItem, INDEX_OID,
				pg_dataset_data(dataset, i, 4)) == FALSE) goto ERR2;
			if (m_list.SetItemText(item.iItem, TABLE_OID,
				pg_dataset_data(dataset, i, 5)) == FALSE) goto ERR2;
		}

		if(m_list.SetItemText(item.iItem, INDEX_COLUMN_NAME,
			pg_dataset_data(dataset, i, 1)) == FALSE) goto ERR2;
	}

	pg_free_dataset(dataset);

	return 0;

ERR1:
	pg_free_dataset(dataset);
	MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
ERR2:
	pg_free_dataset(dataset);
	MessageBox(_T("リストビュー初期化エラー"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void CObjectDetailBar::SetTabCtrl(const TCHAR *object_type)
{
	if(_tcscmp(object_type, _T("TABLE")) == 0) {
		m_object_type = OBJECT_TYPE_TABLE;
	} else if(_tcscmp(object_type, _T("FOREIGN TABLE")) == 0) {
		m_object_type = OBJECT_TYPE_FOREIGN_TABLE;
	} else if(_tcscmp(object_type, _T("INDEX")) == 0) {
		m_object_type = OBJECT_TYPE_INDEX;
	} else if(_tcscmp(object_type, _T("VIEW")) == 0) {
		m_object_type = OBJECT_TYPE_VIEW;
	} else if(_tcscmp(object_type, _T("MATERIALIZED VIEW")) == 0) {
		m_object_type = OBJECT_TYPE_MATERIALIZED_VIEW;
	} else {
		m_object_type = OBJECT_TYPE_OTHER;
	}
	m_object_type_name = object_type;

	// オブジェクトの種類が変わらないときは，タブの状態を変更しない
	if(m_object_type == m_tab_ctrl_type) return;

	TC_ITEM tc_item;
	tc_item.mask = TCIF_TEXT;
	tc_item.cchTextMax = -1;
	tc_item.iImage = -1;
	tc_item.lParam = 0;

	m_tab_ctrl_type = m_object_type;

	m_tab_ctrl.DeleteAllItems();

	switch(m_object_type) {
	case OBJECT_TYPE_TABLE:
		tc_item.pszText = _T("Column");
		m_tab_ctrl.InsertItem(0, &tc_item);
		tc_item.pszText = _T("Index");
		m_tab_ctrl.InsertItem(1, &tc_item);
		tc_item.pszText = _T("Trigger");
		m_tab_ctrl.InsertItem(2, &tc_item);
		tc_item.pszText = _T("Property");
		m_tab_ctrl.InsertItem(3, &tc_item);
		SetListColumns(COLUMN_LIST);
		break;
	case OBJECT_TYPE_FOREIGN_TABLE:
	case OBJECT_TYPE_VIEW:
	case OBJECT_TYPE_MATERIALIZED_VIEW:
		tc_item.pszText = _T("Column");
		m_tab_ctrl.InsertItem(0, &tc_item);
		tc_item.pszText = _T("Property");
		m_tab_ctrl.InsertItem(1, &tc_item);
		SetListColumns(COLUMN_LIST);
		break;
	case OBJECT_TYPE_INDEX:
		tc_item.pszText = _T("Column");
		m_tab_ctrl.InsertItem(0, &tc_item);
		tc_item.pszText = _T("Property");
		m_tab_ctrl.InsertItem(1, &tc_item);
		SetListColumns(COLUMN_LIST);
		break;
	case OBJECT_TYPE_OTHER:
		tc_item.pszText = _T("Property");
		m_tab_ctrl.InsertItem(0, &tc_item);
		SetListColumns(PROPERTY_LIST);
		break;
	}

	AdjustChildWindows();
}

void CObjectDetailBar::SetListColumns(int list_type)
{
	CListCtrl	*p_list = &m_list;
	int			c;

	// データを削除
	p_list->DeleteAllItems();

	// リストタイプに変更がないときは，なにもしない
	if(m_list_type == list_type) return;

	SaveCurColumnWidth();

	m_list_type = list_type;

	// カラムを削除
	for(; p_list->DeleteColumn(0) != FALSE; ) ;

	// カラムを追加
	for(c = 0; column_info[m_list_type][c].name != NULL; c++) {
		m_list.InsertColumn(c, column_info[m_list_type][c].name,
			column_info[m_list_type][c].format,
			column_info[m_list_type][c].width);
	}
}

int CObjectDetailBar::OnTabSelChanged()
{
	int		ret_v = 0;

	switch(m_object_type) {
	case OBJECT_TYPE_TABLE:
		switch(m_tab_ctrl.GetCurSel()) {
		case 0:
			ret_v = SetColumnList();
			if(ret_v != 0) return ret_v;
			break;
		case 1:
			ret_v = SetIndexList();
			if(ret_v != 0) return ret_v;
			break;
		case 2:
			ret_v = SetTriggerList();
			if(ret_v != 0) return ret_v;
			break;
		case 3:
			ret_v = SetPropertyList();
			if(ret_v != 0) return ret_v;
			break;
		}
		break;
	case OBJECT_TYPE_FOREIGN_TABLE:
	case OBJECT_TYPE_VIEW:
	case OBJECT_TYPE_MATERIALIZED_VIEW:
		switch(m_tab_ctrl.GetCurSel()) {
		case 0:
			ret_v = SetColumnList();
			if(ret_v != 0) return ret_v;
			break;
		case 1:
			ret_v = SetPropertyList();
			if(ret_v != 0) return ret_v;
			break;
		}
		break;
	case OBJECT_TYPE_INDEX:
		switch(m_tab_ctrl.GetCurSel()) {
		case 0:
			ret_v = SetColumnList();
			if(ret_v != 0) return ret_v;
			break;
		case 1:
			ret_v = SetPropertyList();
			if(ret_v != 0) return ret_v;
			break;
		}
		break;
	case OBJECT_TYPE_OTHER:
		switch(m_tab_ctrl.GetCurSel()) {
		case 0:
			ret_v = SetPropertyList();
			if(ret_v != 0) return ret_v;
			break;
		}
		break;
	}

	AdjustChildWindows();

	return 0;
}

void CObjectDetailBar::AdjustChildWindows()
{
	RECT rect;
	GetClientRect(&rect);

	// リサイズ中のちらつきを抑える
	LockWindowUpdate();

	m_tab_ctrl.AdjustRect(FALSE, &rect);

	if(m_list_type == COLUMN_LIST) {
		m_static_keyword.ShowWindow(SW_SHOW);
		m_edit_keyword.ShowWindow(SW_SHOW);
		m_btn_keyword.ShowWindow(SW_SHOW);

		m_static_keyword.MoveWindow(rect.left, rect.top, 50, m_font_height + 6);
		m_edit_keyword.MoveWindow(rect.left + 55, rect.top,
			rect.right - rect.left - 55 - m_font_height * 3, m_font_height + 6);
		m_btn_keyword.MoveWindow(rect.right - m_font_height * 3, rect.top,
			m_font_height * 3, m_font_height + 6);

		rect.top += m_font_height + 10;
	} else {
		m_static_keyword.ShowWindow(SW_HIDE);
		m_edit_keyword.ShowWindow(SW_HIDE);
		m_btn_keyword.ShowWindow(SW_HIDE);
	}

	m_list.MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);

	UnlockWindowUpdate();
}

int CObjectDetailBar::SetTriggerList()
{
	if(g_login_flg == FALSE) return 0;

	CWaitCursor	wait_cursor;
	HPgDataset	dataset = NULL;

	dataset = get_trigger_list_by_table(g_ss, m_object_name, m_schema, g_msg_buf);
	if(dataset == NULL) goto ERR1;

	SetListColumns(TRIGGER_LIST);
	SetObjectList(dataset);

	pg_free_dataset(dataset);

	return 0;

ERR1:
	pg_free_dataset(dataset);
	MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

int CObjectDetailBar::SetPropertyList()
{
	if(g_login_flg == FALSE) return 0;

	CWaitCursor	wait_cursor;
	HPgDataset	dataset = NULL;
	int			i;
	LV_ITEM		item;
	CString		no;

	if(m_object_type_name == _T("FUNCTION") || m_object_type_name == _T("PROCEDURE")) {
		dataset = get_object_properties(g_ss, m_object_type_name, m_oid, m_schema, g_msg_buf);
	} else {
		dataset = get_object_properties(g_ss, m_object_type_name, m_object_name, m_schema, g_msg_buf);
	}
	if(dataset == NULL) goto ERR1;

	SetListColumns(PROPERTY_LIST);

	m_list.DeleteAllItems();
	m_list.SetItemCount(pg_dataset_col_cnt(dataset));

	if(pg_dataset_row_cnt(dataset) == 0) {
		pg_free_dataset(dataset);
		return 0;
	}

	item.mask = LVIF_PARAM;
	item.iSubItem = 0;
	for(i = 0; i < pg_dataset_col_cnt(dataset); i++) {
		no.Format(_T("%d"), i + 1);
		item.iItem = i;
		item.lParam = i;
		item.iItem = m_list.InsertItem(&item);
		if(item.iItem == -1) goto ERR2;

		if(m_list.SetItemText(item.iItem, PROPERTY_NAME,
			pg_dataset_get_colname(dataset, i)) == FALSE) goto ERR2;
		if(m_list.SetItemText(item.iItem, PROPERTY_VALUE,
			pg_dataset_data(dataset, 0, i)) == FALSE) goto ERR2;
	}

	pg_free_dataset(dataset);

	return 0;

ERR1:
	pg_free_dataset(dataset);
	MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
ERR2:
	pg_free_dataset(dataset);
	MessageBox(_T("リストビュー初期化エラー"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void CObjectDetailBar::SaveCurColumnWidth()
{
	// 現在のカラム幅を保存する
	CListCtrl	*p_list = &m_list;
	int idx = m_list_type;

	LV_COLUMN	lv_column;
	lv_column.mask = LVCF_WIDTH;

	int		col_cnt;
	for(col_cnt = 0; p_list->GetColumn(col_cnt, &lv_column) != 0; col_cnt++) ;

	int		c;
	for(c = 0; c < col_cnt; c++) {
		column_info[idx][c].width = p_list->GetColumnWidth(c);
	}	
}

void CObjectDetailBar::OnDestroy() 
{
	CSizingControlBar::OnDestroy();
	
	SaveCurColumnWidth();
}

CString CObjectDetailBar::MakeColumnList(int row_len, BOOL update, CString *insert_values, BOOL b_alias)
{
	CString column_name;
	CString paste_str;
	int		selected_row;
	int		i;
	int selected_cnt = m_list.GetSelectedCount();
	CString paste_table_name;

	if(b_alias && g_option.make_sql.add_alias_name && g_option.make_sql.alias_name.GetLength() > 0) {
		paste_table_name = g_option.make_sql.alias_name;
	}

	for(i = 0, selected_row = -1; i < selected_cnt; i++) {
		selected_row = m_list.GetNextItem(selected_row, LVNI_SELECTED | LVNI_ALL);
		if(selected_row == -1) break;

		make_object_name(&column_name, 
			m_list.GetItemText(selected_row, TABLE_COLUMN_NAME).GetBuffer(0),
			g_option.text_editor.copy_lower_name);
		if(!paste_table_name.IsEmpty()) {
			column_name = paste_table_name + _T(".") + column_name;
		}

		// FIXME: 設定可能にする
		if(update == FALSE && row_len + column_name.GetLength() > 70) {
			paste_str += _T("\n\t");
			row_len = 0;

			if(insert_values != NULL) {
				*insert_values += _T("\n\t");
			}
		}

		paste_str += column_name;
		if(update) paste_str += _T(" = ");

		row_len = row_len + column_name.GetLength();

		// 最後のカラム以外は，', 'を付ける
		if(i != selected_cnt - 1) {
			paste_str += _T(", ");
			row_len += 2;

			if(insert_values != NULL) {
				*insert_values += _T("'', ");
			}

			if(update) paste_str += _T("\n\t");
		} else {
			if(insert_values != NULL) {
				*insert_values += _T("''");
			}
		}
	}

	return paste_str;
}

CString CObjectDetailBar::MakeColumnListAll(int row_len, CString *insert_values, BOOL b_alias)
{
	CString column_name;
	CString paste_str;
	int		i;

	HPgDataset	dataset = NULL;
	CString paste_table_name;

	if(b_alias && g_option.make_sql.add_alias_name && g_option.make_sql.alias_name.GetLength() > 0) {
		paste_table_name = g_option.make_sql.alias_name;
	}

	dataset = get_column_list(g_ss, m_object_name, m_schema, g_msg_buf);
	if(dataset == NULL) return _T("");

	for(i = 0; i < pg_dataset_row_cnt(dataset); i++) {
		make_object_name(&column_name, pg_dataset_data(dataset, i, 0),
			g_option.text_editor.copy_lower_name);
		if(!paste_table_name.IsEmpty()) {
			column_name = paste_table_name + _T(".") + column_name;
		}

		// FIXME: 設定可能にする
		if(row_len + column_name.GetLength() > 70) {
			paste_str += _T("\n\t");
			row_len = 0;

			if(insert_values != NULL) {
				*insert_values += _T("\n\t");
			}
		}

		paste_str += column_name;

		row_len = row_len + column_name.GetLength();

		// 最後のカラム以外は，', 'を付ける
		if(i != pg_dataset_row_cnt(dataset) - 1) {
			paste_str += ", ";
			row_len += 2;

			if(insert_values != NULL) {
				*insert_values += _T("'', ");
			}
		} else {
			if(insert_values != NULL) {
				*insert_values += _T("''");
			}
		}
	}

	pg_free_dataset(dataset);

	return paste_str;
}

CString CObjectDetailBar::MakeSelectSql(BOOL b_all, BOOL b_use_semicolon)
{
	if(b_all == FALSE && m_list.GetSelectedCount() <= 0) return _T("");

	CString paste_str;

	paste_str = _T("select ");
	if(b_all) {
		paste_str += MakeColumnListAll(paste_str.GetLength(), NULL, TRUE);
	} else {
		paste_str += MakeColumnList(paste_str.GetLength(), FALSE, NULL, TRUE);
	}
	paste_str += _T("\nfrom ");
	paste_str += MakeObjectName();

	if(g_option.make_sql.add_alias_name && g_option.make_sql.alias_name.GetLength() > 0) {
		paste_str += _T(" ");
		paste_str += g_option.make_sql.alias_name;
	}

	if(b_use_semicolon) paste_str += _T(";");

	paste_str += _T("\n");

	return paste_str;
}

CString CObjectDetailBar::MakeObjectName()
{
	CString table_name;

	make_object_name(&table_name, m_object_name.GetBuffer(0), g_option.text_editor.copy_lower_name);

	if(m_schema.Compare(m_owner) != 0 && m_schema.Compare(_T("public")) != 0) {
		CString schema_name;
		make_object_name(&schema_name, m_schema.GetBuffer(0), g_option.text_editor.copy_lower_name);
		table_name = schema_name + "." + table_name;
	}

	return table_name;
}

void CObjectDetailBar::MakeInsertSql(BOOL b_all) 
{
	if(b_all == FALSE && m_list.GetSelectedCount() <= 0) return;

	CString paste_str;
	CString insert_values;

	paste_str = _T("insert into ");
	paste_str += MakeObjectName();
	paste_str += _T("\n(");
	if(b_all) {
		paste_str += MakeColumnListAll(1, &insert_values, FALSE);
	} else {
		paste_str += MakeColumnList(1, FALSE, &insert_values, FALSE);
	}

	paste_str += _T(")\nvalues(") + insert_values + _T(");\n");

	CDocument *pdoc = ::GetActiveDocument();
	if(pdoc != NULL) {
		pdoc->UpdateAllViews(NULL, UPD_PASTE_SQL, (CObject *)paste_str.GetBuffer(0));
	}
}

void CObjectDetailBar::MakeUpdateSql() 
{
	if(m_list.GetSelectedCount() <= 0) return;

	CString paste_str;

	paste_str = _T("update ");
	paste_str += MakeObjectName();
	paste_str += _T("\nset ");
	paste_str += MakeColumnList(0, TRUE, NULL, FALSE);
	paste_str += _T("\nwhere ;\n");

	CDocument *pdoc = ::GetActiveDocument();
	if(pdoc != NULL) {
		pdoc->UpdateAllViews(NULL, UPD_PASTE_SQL, (CObject *)paste_str.GetBuffer(0));
	}
}

BOOL CObjectDetailBar::CanMakeSql()
{
	if(m_list_type == COLUMN_LIST && m_list.GetSelectedCount() > 0) return TRUE;

	return FALSE;
}

BOOL CObjectDetailBar::CanGetSource()
{
	if (m_list_type == INDEX_LIST && m_list.GetSelectedCount() > 0 &&
		GetSelectedItemString().GetLength() > 0 &&
		m_list.GetSelectedCount() == 1) return TRUE;

	return FALSE;
}

void CObjectDetailBar::CopyValue()
{
	if(m_list.GetSelectedCount() <= 0) return;

	int selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	CopyClipboard(m_list.GetItemText(selected_row, 1).GetBuffer(0));
}

void CObjectDetailBar::SetKeywordOK(BOOL b_ok)
{
	if(m_keyword_is_ok == b_ok) return;

	m_keyword_is_ok = b_ok;
	m_edit_keyword.RedrawWindow();
}

HBRUSH CObjectDetailBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CSizingControlBar::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO: この位置で DC のアトリビュートを変更してください
	if(pWnd == &m_edit_keyword && m_keyword_is_ok == FALSE) {
		pDC->SetTextColor(RGB(255, 0, 0));
	}

	return hbr;
}
void CObjectDetailBar::OnKeywordChanged()
{
	SetObjectList(m_dataset);
}

BOOL CObjectDetailBar::isKeywordActive()
{
	if(GetFocus() == &m_edit_keyword) return TRUE;
	return FALSE;
}

void CObjectDetailBar::PasteToKeyword()
{
	m_edit_keyword.Paste();
}

void CObjectDetailBar::CopyFromKeyword()
{
	m_edit_keyword.Copy();
}

void CObjectDetailBar::CutKeyword()
{
	DWORD selected = m_edit_keyword.GetSel();
	if(LOWORD(selected) == HIWORD(selected)) SelectAllKeyword();

	m_edit_keyword.Cut();
}

void CObjectDetailBar::SelectAllKeyword()
{
	m_edit_keyword.SetSel(0, -1);
}

int CObjectDetailBar::GetSelectedObjectList(CStringArray* object_name_list, CStringArray* object_type_list,
	CStringArray* oid_list, CStringArray* schema_list)
{
	int	i;
	int selected_row;
	int selected_cnt = m_list.GetSelectedCount();

	int oid_idx = -1;
	if (m_list_type == INDEX_LIST) {
		oid_idx = INDEX_OID;
	}

	for (i = 0, selected_row = -1; i < selected_cnt; i++) {
		selected_row = m_list.GetNextItem(selected_row, LVNI_SELECTED | LVNI_ALL);
		if (selected_row == -1) break;

		CString object_name = m_list.GetItemText(selected_row, 0);
		object_name_list->Add(object_name);

		CString object_type = _T("");
		if (m_list_type == INDEX_LIST) {
			object_type = _T("INDEX");
		}
		object_type_list->Add(object_type);

		CString oid = m_list.GetItemText(selected_row, oid_idx);
		oid_list->Add(oid);

		CString schema = _T("");
		schema_list->Add(schema);
	}

	return selected_cnt;
}
