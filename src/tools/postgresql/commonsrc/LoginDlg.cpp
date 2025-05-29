/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // LoginDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "LoginDlg.h"

#include "scr_pass.h"
#include "connectlist.h"

#include "file_winutil.h"
#include "fileutil.h"

#include "ConnectInfoDlg.h"
#include "common.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MIN_DLG_SIZE_X	350
#define MIN_DLG_SIZE_Y	350
#define KEYWORD_EDIT_CTRL_ID	203

static CString GetFilterFileName()
{
	CString file_name = GetAppPath() + _T("data\\login_filter.txt");
	return file_name;
}

CString encode_passwd_v2(const TCHAR *passwd)
{
	CString result;
	BOOL	err_flg;

	for(;;) {
		srand((unsigned)time( NULL ));
		int solt = (rand() % 10);

		const TCHAR *p = passwd;
		result = _T("");
		err_flg = FALSE;

		TCHAR *r = result.GetBuffer(255);
		*r = 'a' + solt;
		r++;
		scr_encode(passwd, r, solt);
		result.ReleaseBuffer();

		if(err_flg) continue;

		// デコードできるかチェック
		if(decode_passwd_v2(result.GetBuffer(0)).Compare(passwd) == 0) break;

		TRACE(_T("decode error!\n"));
	}

	return result;
}

CString decode_passwd_v2(const TCHAR *passwd)
{
	int solt = *passwd - 'a';
	passwd++;

	CString result = _T("");
	TCHAR *r = result.GetBuffer(255);
	scr_decode(passwd, r, solt);
	result.ReleaseBuffer();

	return result;
}

static BOOL CheckRegexp(HREG_DATA hreg, const TCHAR* p)
{
	CString str = p;
	OREG_DATASRC str_src;

	str.MakeLower();
	oreg_make_str_datasrc(&str_src, str);

	if(oreg_exec2(hreg, &str_src) == OREGEXP_FOUND) return TRUE;

	return FALSE;
}

static void LoadConnectInfoToList(CListCtrl& list_ctrl, 
	const TCHAR* opt_prof_name, const TCHAR* opt_registry_key, HREG_DATA hreg,
	CConnectList* connect_list, BOOL reload_list)
{
	if(reload_list) {
		connect_list->load_list();
	}

	CPtrList	*p_list = connect_list->GetList();
	int			r;
	int			item_idx;
	POSITION	pos;
	CConnectInfo *p_info;

	int			encode_ver = AfxGetApp()->GetProfileInt(_T("CONNECT_INFO"), _T("ENCODE_VER"), 1);
	LV_ITEM		item;

	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iSubItem = 0;

	list_ctrl.SetRedraw(FALSE);
	list_ctrl.DeleteAllItems();

	for(item_idx = 0, r = 0, pos = p_list->GetHeadPosition(); pos != NULL; r++, p_list->GetNext(pos)) {
		p_info = (CConnectInfo *)p_list->GetAt(pos);

		if(hreg != NULL) {
			if(!CheckRegexp(hreg, p_info->m_user.GetBuffer(0)) &&
				!CheckRegexp(hreg, p_info->m_host.GetBuffer(0)) &&
				!CheckRegexp(hreg, p_info->m_port.GetBuffer(0)) &&
				!CheckRegexp(hreg, p_info->m_option.GetBuffer(0)) &&
				!CheckRegexp(hreg, p_info->m_connect_name.GetBuffer(0))) {
				continue;
			}
		}

		CString pwd;
		item.iItem = item_idx;
		item.pszText = p_info->m_user.GetBuffer(0);
		if(p_info->m_passwd != _T("")) {
			pwd = decode_passwd_v2(p_info->m_passwd.GetBuffer(0));
			item.lParam = (LPARAM)new CString(pwd);
		} else {
			item.lParam = NULL;
		}
		item.iItem = list_ctrl.InsertItem(&item);
		if(p_info->m_passwd != _T("")) {
			list_ctrl.SetItemText(item.iItem, LIST_PASSWD, _T("*****"));
		}

		list_ctrl.SetItemText(item_idx, LIST_DBNAME, p_info->m_dbname);
		list_ctrl.SetItemText(item_idx, LIST_HOST, p_info->m_host);
		list_ctrl.SetItemText(item_idx, LIST_PORT, p_info->m_port);
		list_ctrl.SetItemText(item_idx, LIST_OPTION, p_info->m_option);
		list_ctrl.SetItemText(item_idx, LIST_CONNECT_NAME, p_info->m_connect_name);

		item_idx++;
	}

	list_ctrl.SetColumnWidth(LIST_USER, LVSCW_AUTOSIZE_USEHEADER);
	list_ctrl.SetColumnWidth(LIST_PASSWD, LVSCW_AUTOSIZE_USEHEADER);
	list_ctrl.SetColumnWidth(LIST_DBNAME, LVSCW_AUTOSIZE_USEHEADER);
	list_ctrl.SetColumnWidth(LIST_HOST, LVSCW_AUTOSIZE_USEHEADER);
	list_ctrl.SetColumnWidth(LIST_PORT, LVSCW_AUTOSIZE_USEHEADER);
	list_ctrl.SetColumnWidth(LIST_OPTION, LVSCW_AUTOSIZE_USEHEADER);

	list_ctrl.SetRedraw(TRUE);
}

void LoadConnectInfoToList(CListCtrl& list_ctrl, 
	const TCHAR* opt_prof_name, const TCHAR* opt_registry_name)
{
	CConnectList	connect_list(opt_prof_name, opt_registry_name);
	LoadConnectInfoToList(list_ctrl, opt_prof_name, opt_registry_name, NULL, &connect_list, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg ダイアログ


CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginDlg)
	m_host = _T("");
	m_user = _T("");
	m_port = _T("");
	m_passwd = _T("");
	m_dbname = _T("");
	m_option = _T("");
	//}}AFX_DATA_INIT

	m_title = _T("");
	m_ss = NULL;
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Control(pDX, IDCANCEL, m_cancel);
	DDX_Control(pDX, IDC_SET_LOGIN_INFO, m_btn_set_login_info);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_EDIT_HOST, m_host);
	DDX_Text(pDX, IDC_EDIT_USER, m_user);
	DDX_Text(pDX, IDC_EDIT_PORT_NO, m_port);
	DDX_Text(pDX, IDC_EDIT_PASS, m_passwd);
	DDX_Text(pDX, IDC_EDIT_DB_NAME, m_dbname);
	DDX_Text(pDX, IDC_EDIT_OPTION, m_option);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_EDIT_HOST, m_edit_host);
	DDX_Control(pDX, IDC_EDIT_OPTION, m_edit_option);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_EN_CHANGE(IDC_EDIT_USER, OnChangeEditUser)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST1, OnDeleteitemList1)
	ON_BN_CLICKED(IDC_SET_LOGIN_INFO, OnSetLoginInfo)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CLoginDlg::OnColumnclickList1)
	ON_WM_CREATE()
	ON_COMMAND(ID_OBJECTBAR_FILTER_ADD, &CLoginDlg::OnObjectbarFilterAdd)
	ON_COMMAND(ID_OBJECTBAR_FILTER_CLEAR, &CLoginDlg::OnObjectbarFilterClear)
	ON_COMMAND(ID_OBJECTBAR_FILTER_DEL, &CLoginDlg::OnObjectbarFilterDel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg メッセージ ハンドラ

BOOL CLoginDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if(pg_library_is_ok() == FALSE) {
		MessageBox(_T("libpq.dllが初期化されていません"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		OnCancel();
	}

	// TODO: この位置に初期化の補足処理を追加してください
	ModifyStyleEx(0, WS_EX_APPWINDOW);

	if(m_title != _T("")) SetWindowText(m_title);

	if(m_user != _T("")) {
		GetDlgItem(IDC_EDIT_PASS)->SetFocus();
	}

	ListView_SetExtendedListViewStyle(m_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(LIST_USER, _T("USER"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_PASSWD, _T("PASSWD"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_DBNAME, _T("DBNAME"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_HOST, _T("HOST"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_PORT, _T("PORT"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_OPTION, _T("OPTION"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(LIST_CONNECT_NAME, _T("NAME"), LVCFMT_LEFT, 100);

	LoadConnectInfo(TRUE);

	CheckOkButton();

	CString	file_version = GetAppVersion();
	CString title;
	title.Format(_T("%s Ver.%s Login"), m_title, file_version);
	SetWindowText(title);

	int width, height;
	width = max(AfxGetApp()->GetProfileInt(_T("LOGIN_DLG"), _T("WIDTH"), 0), MIN_DLG_SIZE_X);
	height = max(AfxGetApp()->GetProfileInt(_T("LOGIN_DLG"), _T("HEIGHT"), 0), MIN_DLG_SIZE_Y);
	SetWindowPos(NULL, 0, 0, width, height, 
		SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOCOPYBITS);

	return FALSE;
}

void CLoginDlg::OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI )
{
	lpMMI->ptMinTrackSize.x = MIN_DLG_SIZE_X;
	lpMMI->ptMinTrackSize.y = MIN_DLG_SIZE_Y;
}

void CLoginDlg::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	if(m_ok.IsWindowEnabled() == FALSE) return;

	CWaitCursor	wait_cursor;
	TCHAR		msg_buf[1024];
		
	UpdateData(TRUE);

	m_connect_str.Format(_T("host = '%s' port = '%s' dbname = '%s' ")
		_T("user = '%s' password = '%s' %s"),
		m_host, m_port, m_dbname, m_user, m_passwd, m_option);

	m_ss = pg_login(m_connect_str.GetBuffer(0), msg_buf);
	if(m_ss == NULL) goto ERR1;

	{
		int idx = GetConnectInfoIdx();
		if(idx >= 0) {
			m_connect_name = m_list.GetItemText(idx, LIST_CONNECT_NAME);
		} else {
			m_connect_name = _T("");
		}
	}

	CDialog::OnOK();
	return;

ERR1:
	MessageBox(msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return;
}

int CLoginDlg::GetConnectInfoIdx()
{
	int		i;

	for(i = 0; i < m_list.GetItemCount(); i++) {
		if(m_list.GetItemText(i, LIST_USER) == m_user && 
			m_list.GetItemText(i, LIST_DBNAME) == m_dbname &&
			m_list.GetItemText(i, LIST_HOST) == m_host &&
			m_list.GetItemText(i, LIST_PORT) == m_port &&
			m_list.GetItemText(i, LIST_OPTION) == m_option) {
			return i;
		}
	}

	return -1;
}

void CLoginDlg::LoadConnectInfo(BOOL reload_list)
{
	HREG_DATA hreg = m_edit_keyword.GetRegexpData();

	::LoadConnectInfoToList(m_list, m_opt_prof_name, m_opt_registry_key, hreg, &m_connect_list, reload_list);

	if(hreg != NULL) oreg_free(hreg);

	int		idx = GetConnectInfoIdx();
	if(idx >= 0) {
		CString	*pwd = (CString *)m_list.GetItemData(idx);
		if(pwd != NULL) {
			m_passwd = *pwd;
			UpdateData(FALSE);
		}
	}
}

void CLoginDlg::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* p_nmhdr = (NM_LISTVIEW*)pNMHDR;
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	if((p_nmhdr->uChanged & LVIF_STATE) == 0) return;
	
	if((p_nmhdr->uNewState & (LVIS_SELECTED | LVNI_ALL)) != 0) {
		m_user = m_list.GetItemText(p_nmhdr->iItem, LIST_USER);
		if(m_list.GetItemData(p_nmhdr->iItem) != NULL) {
			CString *pwd = (CString *)m_list.GetItemData(p_nmhdr->iItem);
			m_passwd = pwd->GetBuffer(0);
		} else {
			m_passwd = _T("");
		}
		m_dbname = m_list.GetItemText(p_nmhdr->iItem, LIST_DBNAME);
		m_host = m_list.GetItemText(p_nmhdr->iItem, LIST_HOST);
		m_port = m_list.GetItemText(p_nmhdr->iItem, LIST_PORT);
		m_option = m_list.GetItemText(p_nmhdr->iItem, LIST_OPTION);
	} else {
		m_user = _T("");
		m_passwd = _T("");
		m_dbname = _T("");
		m_host = _T("");
		m_port = _T("");
		m_option = _T("");
	}
	UpdateData(FALSE);
	CheckOkButton();

	*pResult = 0;
}

void CLoginDlg::CheckOkButton()
{
	UpdateData(TRUE);

	if(m_user == _T("")) {
		m_ok.EnableWindow(FALSE);
	} else {
		m_ok.EnableWindow(TRUE);
	}
}

void CLoginDlg::OnChangeEditUser() 
{
	CheckOkButton();
}

void CLoginDlg::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOK();
	
	*pResult = 0;
}

void CLoginDlg::OnDeleteitemList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	if(pNMListView->lParam != NULL) delete (CString *)(pNMListView->lParam);
	
	*pResult = 0;
}

void CLoginDlg::OnSetLoginInfo() 
{
	CConnectInfoDlg		dlg;
	dlg.SetOptProfName(m_opt_prof_name, m_opt_registry_key);

	if(dlg.DoModal() != IDOK) return;

	LoadConnectInfo(TRUE);	
}

void CLoginDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControl();
}

void CLoginDlg::RelayoutControl()
{
	if(IsWindow(GetSafeHwnd()) == FALSE) return;
	if(IsWindow(m_ok.GetSafeHwnd()) == FALSE) return;

	CRect	client_rect;
	CRect	btn_rect;
	CRect	cur_ok_rect;
	CRect	host_rect;
	CRect	option_rect;

	GetClientRect(client_rect);

	m_btn_set_login_info.GetWindowRect(btn_rect);
	ScreenToClient(btn_rect);
	m_ok.GetWindowRect(cur_ok_rect);
	ScreenToClient(cur_ok_rect);
	m_edit_host.GetWindowRect(host_rect);
	ScreenToClient(host_rect);
	m_edit_option.GetWindowRect(option_rect);
	ScreenToClient(option_rect);

	int btn_top = client_rect.bottom - 5 - cur_ok_rect.Height();
	m_ok.SetWindowPos(NULL, (client_rect.Width() / 2) - 10 - cur_ok_rect.Width(),
		btn_top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	m_cancel.SetWindowPos(NULL, (client_rect.Width() / 2) + 10,
		btn_top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	m_edit_host.SetWindowPos(NULL, host_rect.left, host_rect.top,
		client_rect.Width() - host_rect.left - 10, host_rect.Height(), SWP_NOMOVE | SWP_NOZORDER);

	m_edit_option.SetWindowPos(NULL, option_rect.left, option_rect.top,
		client_rect.Width() - option_rect.left - 10, option_rect.Height(), SWP_NOMOVE | SWP_NOZORDER);

	CRect	list_rect;
	list_rect.top = btn_rect.bottom + 5;
	list_rect.bottom = btn_top - 5;
	list_rect.left = client_rect.left + 5;
	list_rect.right = client_rect.right - 5;
	m_list.SetWindowPos(NULL, list_rect.left, list_rect.top,
		list_rect.Width(), list_rect.Height(),
		SWP_NOZORDER);

	m_static_keyword.SetFont(this->GetFont());
	m_edit_keyword.SetFont(this->GetFont());

	CRect	static_keyword_rect;
	m_static_keyword.GetWindowRect(static_keyword_rect);

	CRect	keyword_rect;
	keyword_rect.top = btn_rect.top;
	keyword_rect.bottom = btn_rect.bottom;
	keyword_rect.left = btn_rect.left + btn_rect.Width() + 10;
	keyword_rect.right = keyword_rect.left + static_keyword_rect.Width();
	m_static_keyword.SetWindowPos(NULL, keyword_rect.left,
		keyword_rect.top + 5, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	m_edit_keyword.SetWindowPos(NULL, keyword_rect.right + 10,
		keyword_rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void CLoginDlg::OnDestroy() 
{
	CRect	win_rect;
	GetWindowRect(&win_rect);

	AfxGetApp()->WriteProfileInt(_T("LOGIN_DLG"), _T("WIDTH"), win_rect.Width());
	AfxGetApp()->WriteProfileInt(_T("LOGIN_DLG"), _T("HEIGHT"), win_rect.Height());

	CDialog::OnDestroy();	
}


void CLoginDlg::OnColumnclickList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	static int sort_order = 1;

	ListCtrlSortEx(&m_list, pNMLV->iSubItem, sort_order);

	// 昇順・降順を切りかえる
	sort_order = -sort_order;

	*pResult = 0;
}


int CLoginDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if(CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect static_rect(0, 0, 30, 20);
	CRect edit_rect(0, 0, 120, 100);

	// TODO: ここに特定な作成コードを追加してください。
	m_static_keyword.Create(_T("filter"), WS_CHILD | WS_VISIBLE, static_rect, this);

	m_edit_keyword.Create(GetFilterFileName(), edit_rect, this, KEYWORD_EDIT_CTRL_ID);
	m_keyword_edit_hwnd = m_edit_keyword.GetEdit()->GetSafeHwnd();

	return 0;
}

void CLoginDlg::OnKeywordChanged()
{
	LoadConnectInfo(FALSE);
}

BOOL CLoginDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case KEYWORD_EDIT_CTRL_ID:
		switch(HIWORD(wParam)) {
		case CBN_EDITCHANGE:
			OnKeywordChanged();
			break;
		case CBN_SELENDOK:
			if(m_edit_keyword.GetCurSel() != CB_ERR) {
				PostMessage(WM_COMMAND, MAKEWPARAM(KEYWORD_EDIT_CTRL_ID, CBN_EDITCHANGE),
					(LPARAM)m_edit_keyword.GetSafeHwnd());
			}
			break;
		}
		break;
	}

	return CDialog::OnCommand(wParam, lParam);
}


void CLoginDlg::OnObjectbarFilterAdd()
{
	if(IsWindow(this->GetSafeHwnd()) == FALSE) return;

	m_edit_keyword.AddFilterText(m_edit_keyword.GetFilterText());
}

void CLoginDlg::OnObjectbarFilterDel()
{
	if(IsWindow(this->GetSafeHwnd()) == FALSE) return;

	m_edit_keyword.DeleteFilterText(m_edit_keyword.GetFilterText());
}

void CLoginDlg::OnObjectbarFilterClear()
{
	if(IsWindow(this->GetSafeHwnd()) == FALSE) return;

	m_edit_keyword.Clear();
}
