/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // SqlLibraryDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "SqlLibraryDlg.h"
#include "common.h"
#include "fileutil.h"

#include "ostrutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum list_column {
	LIST_FILE_NAME,
	LIST_MEMO,
	LIST_SORT_NO,
	LIST_KIND,
};

enum item_kind {
	ITEM_PARENT_FOLDER,
	ITEM_FOLDER,
	ITEM_SQLFILE,
};

/////////////////////////////////////////////////////////////////////////////
// CSqlLibraryDlg ダイアログ


CSqlLibraryDlg::CSqlLibraryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSqlLibraryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSqlLibraryDlg)
	m_category = _T("");
	//}}AFX_DATA_INIT
}


void CSqlLibraryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSqlLibraryDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_SQL_LIST, m_list);
	DDX_Text(pDX, IDC_EDIT_CATEGORY, m_category);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSqlLibraryDlg, CDialog)
	//{{AFX_MSG_MAP(CSqlLibraryDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_SQL_LIST, OnDblclkSqlList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SQL_LIST, OnItemchangedSqlList)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_SQL_LIST, OnColumnclickSqlList)
	ON_BN_CLICKED(IDC_EDIT_INDEX, OnEditIndex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSqlLibraryDlg メッセージ ハンドラ

void CSqlLibraryDlg::OnOK() 
{
	int	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	m_file_name = m_current_dir + _T("\\") + m_list.GetItemText(selected_row, LIST_FILE_NAME);
	
	CDialog::OnOK();
}

BOOL CSqlLibraryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// イメージリストを作成
	HINSTANCE h_inst = AfxGetInstanceHandle();

	if(m_image_list.Create(16, 16, ILC_COLOR4 | ILC_MASK, 10, 20) == FALSE) goto ERR1;
	if(m_image_list.Add(LoadIcon(h_inst, MAKEINTRESOURCE(IDI_PARENT_FOLDER))) == -1) goto ERR1;
	if(m_image_list.Add(LoadIcon(h_inst, MAKEINTRESOURCE(IDI_FOLDER))) == -1) goto ERR1;
	if(m_image_list.Add(LoadIcon(h_inst, MAKEINTRESOURCE(IDR_MAINFRAME))) == -1) goto ERR1;

	// イメージリストを設定
	m_list.SetImageList(&m_image_list, LVSIL_SMALL);

	m_list.SetFont(m_font);
	m_list.InsertColumn(LIST_FILE_NAME, _T("file name"), LVCFMT_LEFT, 150);
	m_list.InsertColumn(LIST_MEMO, _T("memo"), LVCFMT_LEFT, 350);
	m_list.InsertColumn(LIST_SORT_NO, _T("sort_no"), LVCFMT_RIGHT, 0);
	m_list.InsertColumn(LIST_KIND, _T("kind"), LVCFMT_RIGHT, 0);

	// アイテムを選択したときに，行全体を反転させるようにする
	ListView_SetExtendedListViewStyle(m_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	if(SetSqlList(m_root_dir) != 0) EndDialog(IDCANCEL);

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります

ERR1:
	MessageBox(_T("ダイアログ初期化エラー"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	EndDialog(IDCANCEL);
	return TRUE;
}

void CSqlLibraryDlg::OnDblclkSqlList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;
	
	CString	dir_name;

	switch(_ttoi(m_list.GetItemText(selected_row, LIST_KIND).GetBuffer(0))) {
	case ITEM_PARENT_FOLDER:
		dir_name = m_current_dir;
		make_parent_dirname(dir_name.GetBuffer(0));
		dir_name.ReleaseBuffer();
		SetSqlList(dir_name);
		break;
	case ITEM_FOLDER:
		dir_name = m_current_dir + _T("\\") + m_list.GetItemText(selected_row, LIST_FILE_NAME);
		SetSqlList(dir_name);
		break;
	case ITEM_SQLFILE:
		OnOK();
		break;
	}

	*pResult = 0;
}

void CSqlLibraryDlg::OnItemchangedSqlList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = 0;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;

	int	selected_row;
	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1 || 
		_ttoi(m_list.GetItemText(selected_row, LIST_KIND).GetBuffer(0)) != ITEM_SQLFILE) {
		m_ok.EnableWindow(FALSE);
		return;
	}

	m_ok.EnableWindow(TRUE);
}

int CSqlLibraryDlg::SetSqlList(CString dir_name)
{
	m_list.DeleteAllItems();

	m_current_dir = dir_name;

	m_category = m_current_dir.Right(m_current_dir.GetLength() - m_root_dir.GetLength());
	UpdateData(FALSE);

	CStringList	str_list;
	CreateIndex(dir_name, str_list);

	int			i;
	LV_ITEM		item;
	CString		find_file_name;

	find_file_name = dir_name + _T("\\*.*");

	HANDLE				hFind;
	WIN32_FIND_DATA		find_data;
	hFind = FindFirstFile(find_file_name.GetBuffer(0), &find_data);
	if(hFind == INVALID_HANDLE_VALUE) return 0;

	CString		kind;
	item.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
	item.iSubItem = 0;
	for(i = 0;;) {
		if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if(_tcscmp(find_data.cFileName, _T(".")) == 0 ||
				(_tcscmp(find_data.cFileName, _T("..")) == 0 && m_current_dir.Compare(m_root_dir) == 0)) {
				if(FindNextFile(hFind, &find_data) == FALSE) break;
				continue;
			}

			if(_tcscmp(find_data.cFileName, _T("..")) == 0) {
				item.iImage = ITEM_PARENT_FOLDER;
			} else {
				item.iImage = ITEM_FOLDER;
			}
		} else {
			if(check_ext(find_data.cFileName, _T(".sql")) == FALSE) {
				if(FindNextFile(hFind, &find_data) == FALSE) break;
				continue;
			}
			item.iImage = ITEM_SQLFILE;
		}

		item.iItem = i;
		item.lParam = i;
		item.pszText = find_data.cFileName;

		item.iItem = m_list.InsertItem(&item);
		if(item.iItem == -1) goto ERR1;

		SetFileData(item.iItem, find_data.cFileName, str_list);

		kind.Format(_T("%d"), item.iImage);
		if(m_list.SetItemText(item.iItem, LIST_KIND, kind) == FALSE) goto ERR1;

		i++;

		if(FindNextFile(hFind, &find_data) == FALSE) break;
	}

	FindClose(hFind);

	ListCtrlSort(&m_list, LIST_SORT_NO, 1);

	return 0;

ERR1:
	FindClose(hFind);
	MessageBox(_T("リストビュー初期化エラー"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void CSqlLibraryDlg::OnColumnclickSqlList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	static int sort_order = 1;

	ListCtrlSort(&m_list, pNMListView->iSubItem, sort_order);

	// 昇順・降順を切りかえる
	sort_order = -sort_order;
	
	*pResult = 0;
}

int CSqlLibraryDlg::CreateIndex(CString dir_name, CStringList &str_list)
{
	FILE	*fp = NULL;
	TCHAR	buf[2048];

	// FIXME: ANSI版と互換性とる？
	CString	index_file = dir_name + _T("\\index.txt");
	fp = _tfopen(index_file.GetBuffer(0), _T("r"));
	if(fp == NULL) {
		return 0;
	}

	for(; _fgetts(buf, ARRAY_SIZEOF(buf), fp) != 0;) {
		ostr_chomp(buf, '\n');
		if(_tcslen(buf) == 0 || buf[0] == '#') continue;
		str_list.AddTail(buf);
	}

	fclose(fp);

	return 0;
}

int CSqlLibraryDlg::SetFileData(int row, TCHAR *file_name, CStringList &str_list)
{
	int			sort_no;
	CString		memo = _T("");

	if(_tcscmp(file_name, _T("..")) == 0) {
		memo = _T("<Parent Directory>");
		sort_no = 0;
	} else {
		POSITION	pos;
		sort_no = 1;
		for(pos = str_list.GetHeadPosition(); pos != NULL; str_list.GetNext(pos)) {
			if(_tcsncmp(file_name, str_list.GetAt(pos).GetBuffer(0), _tcslen(file_name)) == 0 &&
				str_list.GetAt(pos).GetBuffer(0)[_tcslen(file_name)] == ':') {
				memo = str_list.GetAt(pos).GetBuffer(0) + _tcslen(file_name) + 1;
				break;
			}
			sort_no++;
		}
	}

	if(m_list.SetItemText(row, LIST_MEMO, memo) == FALSE) return 1;

	CString no;
	no.Format(_T("%d"), sort_no);
	if(m_list.SetItemText(row, LIST_SORT_NO, no) == FALSE) return 1;

	return 0;
}

void CSqlLibraryDlg::OnEditIndex() 
{
	FILE	*fp = NULL;
	int		i;

	CString	index_file = m_current_dir + _T("\\index.txt");
	fp = _tfopen(index_file.GetBuffer(0), _T("w"));
	if(fp == NULL) {
		return;
	}

	_ftprintf(fp, _T("#%s\n"), m_category.GetBuffer(0));
	for(i = 0; i < m_list.GetItemCount(); i++) {
		if(_ttoi(m_list.GetItemText(i, LIST_KIND).GetBuffer(0)) == ITEM_PARENT_FOLDER) continue;

		_ftprintf(fp, _T("%s:%s\n"),
			m_list.GetItemText(i, LIST_FILE_NAME).GetBuffer(0),
			m_list.GetItemText(i, LIST_MEMO).GetBuffer(0));
	}

	fclose(fp);

	m_file_name = m_current_dir + _T("\\index.txt");
	
	CDialog::OnOK();

	return;
}
