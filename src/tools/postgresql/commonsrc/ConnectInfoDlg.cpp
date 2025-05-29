/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ConnectInfoDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "ConnectInfoDlg.h"

#include "logindlg.h"
#include "ConnectList.h"

#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConnectInfoDlg ダイアログ


CConnectInfoDlg::CConnectInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConnectInfoDlg)
	m_edit_user = _T("");
	m_edit_passwd = _T("");
	m_dbname = _T("");
	m_host = _T("");
	m_port = _T("");
	m_option = _T("");
	m_connect_name = _T("");
	//}}AFX_DATA_INIT
}


void CConnectInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConnectInfoDlg)
	DDX_Control(pDX, IDC_BTN_UP, m_btn_up);
	DDX_Control(pDX, IDC_BTN_DOWN, m_btn_down);
	DDX_Control(pDX, IDC_BTN_DEL, m_btn_del);
	DDX_Control(pDX, IDC_BTN_ADD, m_btn_add);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_USER, m_edit_user);
	DDX_Text(pDX, IDC_EDIT_PASSWD, m_edit_passwd);
	DDX_Text(pDX, IDC_EDIT_DBNAME, m_dbname);
	DDX_Text(pDX, IDC_EDIT_HOST, m_host);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDX_Text(pDX, IDC_EDIT_OPTION, m_option);
	DDX_Text(pDX, IDC_EDIT_CONNECT_NAME, m_connect_name);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_BTN_COPY, m_btn_copy);
}


BEGIN_MESSAGE_MAP(CConnectInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CConnectInfoDlg)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, OnBtnDel)
	ON_BN_CLICKED(IDC_BTN_DOWN, OnBtnDown)
	ON_BN_CLICKED(IDC_BTN_UP, OnBtnUp)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_EN_CHANGE(IDC_EDIT_USER, OnChangeEditUser)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST1, OnDeleteitemList1)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_COPY, &CConnectInfoDlg::OnClickedBtnCopy)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CConnectInfoDlg::OnColumnclickList1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectInfoDlg メッセージ ハンドラ

BOOL CConnectInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	ListView_SetExtendedListViewStyle(m_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	m_list.InsertColumn(LIST_USER, _T("USER"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_PASSWD, _T("PASSWD"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_DBNAME, _T("DBNAME"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_HOST, _T("HOST"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_PORT, _T("PORT"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_OPTION, _T("OPTION"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_CONNECT_NAME, _T("NAME"), LVCFMT_LEFT, 100);

	LoadConnectInfo();

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CConnectInfoDlg::AddData(CString user, CString passwd, CString dbname, CString host, 
	CString port, CString option, CString connect_name)
{
	LV_ITEM		item;

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;
	item.iItem = m_list.GetItemCount();
	item.pszText = _T("");
	item.lParam = NULL;
	item.iItem = m_list.InsertItem(&item);

	EditData(item.iItem, user, passwd, dbname, host, port, option, connect_name);
}

void CConnectInfoDlg::EditData(int row, CString user, CString passwd, 
	CString dbname, CString host, CString port, CString option, CString connect_name)
{
	m_list.SetItemText(row, LIST_USER, user);
	if(m_list.GetItemData(row) != NULL) {
		CString	*pwd = (CString *)m_list.GetItemData(row);
		delete pwd;
	}
	if(passwd == _T("")) {
		m_list.SetItemText(row, LIST_PASSWD, _T(""));
		m_list.SetItemData(row, NULL);
	} else {
		CString *pwd = new CString(passwd);
		m_list.SetItemText(row, LIST_PASSWD, _T("*****"));
		m_list.SetItemData(row, (DWORD_PTR)pwd);
	}
	
	m_list.SetItemText(row, LIST_DBNAME, dbname);
	m_list.SetItemText(row, LIST_HOST, host);
	m_list.SetItemText(row, LIST_PORT, port);
	m_list.SetItemText(row, LIST_OPTION, option);
	m_list.SetItemText(row, LIST_CONNECT_NAME, connect_name);

	CheckSelect();
}

void CConnectInfoDlg::OnBtnAdd() 
{
	UpdateData(TRUE);

	int selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) {
		AddData(m_edit_user, m_edit_passwd, m_dbname, m_host, m_port, m_option, m_connect_name);
	} else {
		EditData(selected_row, m_edit_user, m_edit_passwd, m_dbname, m_host, m_port, m_option, m_connect_name);
	}
}

void CConnectInfoDlg::OnBtnDel() 
{
	int		selected_row;

	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	m_list.DeleteItem(selected_row);

	CheckSelect();
}

void CConnectInfoDlg::OnBtnDown() 
{
	int		selected_row;

	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	SwapItem(selected_row, selected_row + 1);
}

void CConnectInfoDlg::OnBtnUp() 
{
	int		selected_row;

	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	SwapItem(selected_row, selected_row - 1);
}

void CConnectInfoDlg::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = 0;

	CheckSelect();
}

void CConnectInfoDlg::LoadConnectInfo()
{
	::LoadConnectInfoToList(m_list, m_opt_prof_name, m_opt_registry_key);
}

void CConnectInfoDlg::SaveConnectInfo()
{
	CConnectList	connect_list(m_opt_prof_name, m_opt_registry_key);

	int			i;

	for(i = 0; i < m_list.GetItemCount(); i++) {
		CString		user;
		CString		passwd;
		CString		dbname;
		CString		host;
		CString		port;
		CString		option;
		CString		connect_name;

		user = m_list.GetItemText(i, LIST_USER);
		if(m_list.GetItemData(i) != NULL) {
			CString	*pwd = (CString *)m_list.GetItemData(i);
			passwd = encode_passwd_v2(pwd->GetBuffer(0));
		} else {
			passwd = _T("");
		}
		dbname = m_list.GetItemText(i, LIST_DBNAME);
		host = m_list.GetItemText(i, LIST_HOST);
		port = m_list.GetItemText(i, LIST_PORT);
		option = m_list.GetItemText(i, LIST_OPTION);
		connect_name = m_list.GetItemText(i, LIST_CONNECT_NAME);

		connect_list.add_list(user.GetBuffer(0), passwd.GetBuffer(0),
			dbname.GetBuffer(0), host.GetBuffer(0), port.GetBuffer(0), option.GetBuffer(0),
			connect_name.GetBuffer(0));
	}

	connect_list.save_list();

/*
	int			i;

	for(i = 0; i < m_list.GetItemCount(); i++) {
		char		user_entry[20];
		char		tns_entry[20];
		char		passwd_entry[20];
		CString		user;
		CString		tns;
		CString		passwd;

		sprintf(user_entry, "USER-%d", i);
		sprintf(tns_entry, "TNS-%d", i);
		sprintf(passwd_entry, "PASSWD-%d", i);

		user = m_list.GetItemText(i, LIST_USER);
		tns = m_list.GetItemText(i, LIST_TNS);
		if(m_list.GetItemData(i) != NULL) {
			CString	*pwd = (CString *)m_list.GetItemData(i);
			passwd = encode_passwd_v2((unsigned char *)pwd->GetBuffer(0));
		} else {
			passwd = "";
		}

		AfxGetApp()->WriteProfileString("CONNECT_INFO", user_entry, user);
		AfxGetApp()->WriteProfileString("CONNECT_INFO", tns_entry, tns);
		AfxGetApp()->WriteProfileString("CONNECT_INFO", passwd_entry, passwd);
	}
	AfxGetApp()->WriteProfileInt("CONNECT_INFO", "COUNT", m_list.GetItemCount());
	AfxGetApp()->WriteProfileInt("CONNECT_INFO", "ENCODE_VER", 2);
*/
}

void CConnectInfoDlg::CheckAddButton()
{
	UpdateData(TRUE);

	if(m_edit_user == _T("")) {
		m_btn_add.EnableWindow(FALSE);
	} else {
		m_btn_add.EnableWindow(TRUE);
	}
}

void CConnectInfoDlg::OnChangeEditUser() 
{
	CheckAddButton();
}

void CConnectInfoDlg::OnOK() 
{
	UpdateData(TRUE);
	
	SaveConnectInfo();

	CDialog::OnOK();
}

void CConnectInfoDlg::CheckSelect()
{
	UpdateData(TRUE);

	int		selected_row;

	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);

	if(selected_row == -1) {
		m_btn_add.SetWindowText(_T("Add"));
		m_btn_del.EnableWindow(FALSE);
		m_btn_copy.EnableWindow(FALSE);
		m_btn_up.EnableWindow(FALSE);
		m_btn_down.EnableWindow(FALSE);

		m_edit_user = _T("");
		m_edit_passwd = _T("");
		m_dbname = _T("");
		m_host = _T("");
		m_port = _T("");
		m_option = _T("");
		m_connect_name = _T("");
	} else {
		m_btn_add.SetWindowText(_T("Edit"));
		m_btn_del.EnableWindow(TRUE);
		m_btn_copy.EnableWindow(TRUE);
		m_btn_up.EnableWindow(TRUE);
		m_btn_down.EnableWindow(TRUE);

		m_edit_user = m_list.GetItemText(selected_row, LIST_USER);
		if(m_list.GetItemData(selected_row) != NULL) {
			CString	*pwd = (CString *)m_list.GetItemData(selected_row);
			m_edit_passwd = *pwd;
		} else {
			m_edit_passwd = _T("");
		}
		m_dbname = m_list.GetItemText(selected_row, LIST_DBNAME);
		m_host = m_list.GetItemText(selected_row, LIST_HOST);
		m_port = m_list.GetItemText(selected_row, LIST_PORT);
		m_option = m_list.GetItemText(selected_row, LIST_OPTION);
		m_connect_name = m_list.GetItemText(selected_row, LIST_CONNECT_NAME);
	}
	UpdateData(FALSE);
	CheckAddButton();
}

void CConnectInfoDlg::SwapItem(int item1, int item2)
{
	CString		user;
	CString		passwd;
	CString		dbname, host, port, option, connect_name;
	DWORD_PTR	param;

	if(item1 < 0 || item1 >= m_list.GetItemCount()) return;
	if(item2 < 0 || item2 >= m_list.GetItemCount()) return;

	user = m_list.GetItemText(item2, LIST_USER);
	passwd = m_list.GetItemText(item2, LIST_PASSWD);
	dbname = m_list.GetItemText(item2, LIST_DBNAME);
	host = m_list.GetItemText(item2, LIST_HOST);
	port = m_list.GetItemText(item2, LIST_PORT);
	option = m_list.GetItemText(item2, LIST_OPTION);
	connect_name = m_list.GetItemText(item2, LIST_CONNECT_NAME);
	param = m_list.GetItemData(item2);

	m_list.SetItemText(item2, LIST_USER, m_list.GetItemText(item1, LIST_USER));
	m_list.SetItemText(item2, LIST_PASSWD, m_list.GetItemText(item1, LIST_PASSWD));
	m_list.SetItemText(item2, LIST_DBNAME, m_list.GetItemText(item1, LIST_DBNAME));
	m_list.SetItemText(item2, LIST_HOST, m_list.GetItemText(item1, LIST_HOST));
	m_list.SetItemText(item2, LIST_PORT, m_list.GetItemText(item1, LIST_PORT));
	m_list.SetItemText(item2, LIST_OPTION, m_list.GetItemText(item1, LIST_OPTION));
	m_list.SetItemText(item2, LIST_CONNECT_NAME, m_list.GetItemText(item1, LIST_CONNECT_NAME));
	m_list.SetItemData(item2, m_list.GetItemData(item1));

	m_list.SetItemText(item1, LIST_USER, user);
	m_list.SetItemText(item1, LIST_PASSWD, passwd);
	m_list.SetItemText(item1, LIST_DBNAME, dbname);
	m_list.SetItemText(item1, LIST_HOST, host);
	m_list.SetItemText(item1, LIST_PORT, port);
	m_list.SetItemText(item1, LIST_OPTION, option);
	m_list.SetItemText(item1, LIST_CONNECT_NAME, connect_name);
	m_list.SetItemData(item1, param);

	m_list.SetItemState(item2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void CConnectInfoDlg::OnDeleteitemList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if(pNMListView->lParam != NULL) delete (CString *)(pNMListView->lParam);
	
	*pResult = 0;
}

void CConnectInfoDlg::OnClickedBtnCopy()
{
	UpdateData(TRUE);

	AddData(m_edit_user, m_edit_passwd, m_dbname, m_host, m_port, m_option, m_connect_name);

	int idx = m_list.GetItemCount() - 1;
	m_list.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}


void CConnectInfoDlg::OnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	static int sort_order = 1;

	ListCtrlSortEx(&m_list, pNMLV->iSubItem, sort_order);

	// 昇順・降順を切りかえる
	sort_order = -sort_order;

	*pResult = 0;
}
