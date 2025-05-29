/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // SqlLogFileDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "SqlLogFileDlg.h"

#include "ostrutil.h"
#include "octrl_util.h"
#include "fileutil.h"

#include <algorithm>

#pragma warning( disable : 4786 )

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

static const int size_of_buf = 1024 * 256;

#define WMU_EDIT_KEYDOWN		WM_APP + 0x01

static FILE *OpenLogFile(const TCHAR *file_name)
{
	FILE *fp;
	fp = _tfopen(file_name, _T("r"));
	if(fp == NULL) return NULL;

	// Unicodeなら開きなおす
	if(check_utf16le_signature_fp(fp)) {
		fclose(fp);
		fp = _tfopen(file_name, _T("rb"));
		if(fp == NULL) return NULL;

		skip_utf16le_signature(fp);
	}

	return fp;
}

static LRESULT CALLBACK Edit_SubclassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case WM_KEYDOWN:
		switch(wParam) {
		case VK_RETURN:
		case VK_ESCAPE:
		case VK_TAB:
			::PostMessage(::GetParent(hwnd), WMU_EDIT_KEYDOWN, wParam, lParam);
			return 0;
		}
		break;
	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS;
	default:
		break;
	}
	return (CallWindowProc((WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA),
		hwnd, message, wParam, lParam));
}

/////////////////////////////////////////////////////////////////////////////
// CSqlLogFileDlg ダイアログ

CSqlLogFileElement::CSqlLogFileElement(const TCHAR *file_name,
	const TCHAR *user, const TCHAR *sql_time, 
	long offset, int row_cnt)
{
	m_file_name = file_name;
	m_user = user;
	m_time = sql_time;
	m_offset = offset;
	m_row_cnt = row_cnt;
	m_summary[0] = '\0';
}

const TCHAR *CSqlLogFileElement::GetSummary(FILE *fp)
{
	if(m_summary[0] != '\0') return m_summary;

	// バッファサイズ分読み込んで，タブと改行を空白にする
	fseek(fp, m_offset, SEEK_SET);

	TCHAR	buf[4096];
	TCHAR	*p = m_summary;
	int m_summary_char_cnt = sizeof(m_summary)/sizeof(TCHAR) - 1;
	int row_cnt = m_row_cnt;
	int len;

	for(; row_cnt; row_cnt--) {
		if(_fgetts(buf, ARRAY_SIZEOF(buf) - 1, fp) == NULL) break;
		if(_tcscmp(buf, _T("/\n")) == 0) break;

		_stprintf(p, _T("%.*s"), m_summary_char_cnt, buf);

		len = (int)_tcslen(buf);
		m_summary_char_cnt -= len;
		if(m_summary_char_cnt <= 0) break;
		p += len;
	}

	// TabをSpaceに変換
	ostr_char_replace(m_summary, '\n', ' ');
	ostr_char_replace(m_summary, '\t', ' ');

	return m_summary;
}

void CSqlLogFileElement::GetSql(FILE *fp, TCHAR *buf, int buf_size, CEditData *edit_data,
	BOOL no_delete) const
{
	if(no_delete) {
		edit_data->move_document_end();
	} else {
		edit_data->del_all();
	}

	fseek(fp, m_offset, SEEK_SET);

	int row_cnt = m_row_cnt;

	for(; row_cnt; row_cnt--) {
		if(_fgetts(buf, buf_size, fp) == NULL) break;
		edit_data->paste_no_undo(buf);
	}

	edit_data->recalc_disp_size();
}

void CSqlLogFileElement::GetSql(CEditData *edit_data, BOOL no_delete) const
{
	TCHAR *buf = (TCHAR *)malloc(size_of_buf * sizeof(TCHAR));
	if(buf == NULL) return;

	FILE *fp = OpenLogFile(m_file_name);
	if(fp == NULL) {
		free(buf);
		return;
	}

	GetSql(fp, buf, size_of_buf, edit_data, no_delete);

	fclose(fp);
	free(buf);
}

CSqlLogFileDlg::CSqlLogFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSqlLogFileDlg::IDD, pParent)
	, m_date2(0)
{
	//{{AFX_DATA_INIT(CSqlLogFileDlg)
	m_filter = _T("");
	m_user = _T("");
	m_date = 0;
	//}}AFX_DATA_INIT
}

void CSqlLogFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSqlLogFileDlg)
	DDX_Control(pDX, IDC_EDIT_FILTER, m_edit_filter);
	DDX_Control(pDX, IDC_COMBO_USER, m_combo_user);
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Text(pDX, IDC_EDIT_FILTER, m_filter);
	DDX_CBString(pDX, IDC_COMBO_USER, m_user);
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_date);
	//}}AFX_DATA_MAP
	DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_date2);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_date_picker);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_date_picker2);
}


BEGIN_MESSAGE_MAP(CSqlLogFileDlg, CDialog)
	//{{AFX_MSG_MAP(CSqlLogFileDlg)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnItemchangedList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER1, OnDatetimechangeDatetimepicker1)
	ON_CBN_SELCHANGE(IDC_COMBO_USER, OnSelchangeComboUser)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnButtonSearch)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST1, OnGetdispinfoList1)
	//}}AFX_MSG_MAP
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATETIMEPICKER1, &CSqlLogFileDlg::OnCloseupDatetimepicker1)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER2, &CSqlLogFileDlg::OnDatetimechangeDatetimepicker2)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATETIMEPICKER2, &CSqlLogFileDlg::OnCloseupDatetimepicker2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSqlLogFileDlg メッセージ ハンドラ

BOOL CSqlLogFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// エディタを作成
	CreateEditCtrl();

	m_list.SetFont(m_font);
	m_list.InsertColumn(LIST_SQL, _T("SQL"), LVCFMT_LEFT, max(AfxGetApp()->GetProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH_SQL"), 0), 310));
	m_list.InsertColumn(LIST_TIME, _T("Time"), LVCFMT_LEFT, max(AfxGetApp()->GetProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH_TIME"), 0), 70));
	m_list.InsertColumn(LIST_NO, _T("No."), LVCFMT_RIGHT, max(AfxGetApp()->GetProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH_NO"), 0), 30));

	// アイテムを選択したときに，行全体を反転させるようにする
	ListView_SetExtendedListViewStyle(m_list.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

	ReadLogFile();
	if(SetLogList() != 0) EndDialog(IDCANCEL);

	int iItem = m_list.GetItemCount() - 1;
	if(iItem >= 0) {
		m_list.SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_list.EnsureVisible(iItem, FALSE);
		m_list.RedrawItems(iItem, iItem);
	}

	int width = AfxGetApp()->GetProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH"), 0);
	int height = AfxGetApp()->GetProfileInt(_T("SQL_LOG_FILE_DLG"), _T("HEIGHT"), 0);
	if(width > 200 && height > 200) {
		SetWindowPos(NULL, 0, 0, width, height, 
			SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOCOPYBITS);
	}

	UpdateData(FALSE);
	
	{
		// サブクラス化
		// 古いウィンドウプロシージャを保存する
		HWND hwnd = m_edit_filter.GetSafeHwnd();
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, GetWindowLongPtr(hwnd, GWLP_WNDPROC));
		// ウィンドウプロシージャを切り替える
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)Edit_SubclassWndProc);
	}

	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

int CSqlLogFileDlg::CreateEditCtrl()
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

void CSqlLogFileDlg::RelayoutControl()
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

	// Editor
	if(IsWindow(m_edit_ctrl.GetSafeHwnd()) == FALSE) return;
	width = win_rect.Width() - 20;
	height = (win_rect.Height() - ctrl_rect.Height() - 10) / 3 - 10;
	x = win_rect.left + 10;
	y = y - height - 10;
	m_edit_ctrl.SetWindowPos(NULL, x, y, width, height, 
		SWP_NOOWNERZORDER | SWP_NOCOPYBITS);

	// List
	if(IsWindow(m_list.GetSafeHwnd()) == FALSE) return;
	CRect list_rect;
	m_list.GetWindowRect(list_rect);
	ScreenToClient(list_rect);
	width = win_rect.Width() - 20;
	height = y - list_rect.top - 10;
	m_list.SetWindowPos(NULL, 0, 0, width, height, 
		SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOCOPYBITS);
}

void CSqlLogFileDlg::Init(CFont *font, const TCHAR *sql_log_dir, const CTime &date,
	const TCHAR *user, const TCHAR *filter)
{
	m_font = font;
	m_sql_log_dir = sql_log_dir;
	m_date = date;
	m_date2 = date;
	m_user = user;
	m_filter = filter;
}

int CSqlLogFileDlg::SetLogList()
{
	CWaitCursor		cur;

	m_list.DeleteAllItems();
	m_edit_ctrl.ClearAll();
	m_ok.EnableWindow(FALSE);

	if(m_sql_log_list.empty()) return 0;

	FILE *fp = NULL;

	CString		cur_file_name;
	LV_ITEM		item;
	CString		str;
	int			i = 0;
	int			idx = 0;

	CRegData	reg_data;
	CEditData	edit_data;
	TCHAR *buf = NULL;

	if(!m_filter.IsEmpty()) {
		buf = (TCHAR *)malloc(size_of_buf * sizeof(TCHAR));
		if(buf == NULL) {
			// エラー
			MessageBox(_T("メモリが足りません"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
			return 1;
		}

		if(!reg_data.Compile(m_filter, 0)) {
			// エラー
			MessageBox(_T("正規表現が正しくありません"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
			return 1;
		}
	}

	CString user = GetSelectedUser();
	{
		ul_it	user_it = m_user_list.find(user.GetBuffer(0));
		if(user_it != m_user_list.end()) m_list.SetItemCount(user_it->second);
	}

	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.iSubItem = 0;
	item.pszText = LPSTR_TEXTCALLBACK;

	for(SLFEIterator it = m_sql_log_list.begin(); it != m_sql_log_list.end(); it++, idx++) {
		if(user.Compare(it->GetUser()) != 0) continue;

		if(cur_file_name.Compare(it->GetFileName()) != 0) {
			if(fp != NULL) fclose(fp);
			fp = OpenLogFile(it->GetFileName());
			if(fp == NULL) continue;
			cur_file_name = it->GetFileName();
		}

		// フィルタを処理する
		if(buf) {
			it->GetSql(fp, buf, size_of_buf, &edit_data);

			POINT	pt;
			POINT	start_pt = {0, 0};
			if(edit_data.search_text_regexp(&pt, 1, TRUE, &start_pt, NULL,
				reg_data.GetRegData(), NULL) < 0) continue;
		}

		item.iItem = i;
		item.lParam = i;
		item.iItem = m_list.InsertItem(&item);
		if(item.iItem == -1) goto ERR1;

		it->GetSummary(fp);
		it->SetNo(i + 1);

		m_list.SetItemData(i, idx);
		i++;
	}

	if(buf != NULL) free(buf);
	if(fp != NULL) fclose(fp);
	return 0;

ERR1:
	if(fp != NULL) fclose(fp);
	MessageBox(_T("リストビュー初期化エラー"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	return 1;
}

void CSqlLogFileDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	RelayoutControl();	
}

void CSqlLogFileDlg::OnDestroy() 
{
	CRect	win_rect;
	GetWindowRect(&win_rect);

	AfxGetApp()->WriteProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH"), win_rect.Width());
	AfxGetApp()->WriteProfileInt(_T("SQL_LOG_FILE_DLG"), _T("HEIGHT"), win_rect.Height());

	AfxGetApp()->WriteProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH_SQL"), m_list.GetColumnWidth(LIST_SQL));
	AfxGetApp()->WriteProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH_TIME"), m_list.GetColumnWidth(LIST_TIME));
	AfxGetApp()->WriteProfileInt(_T("SQL_LOG_FILE_DLG"), _T("WIDTH_NO"), m_list.GetColumnWidth(LIST_NO));

	CDialog::OnDestroy();
}

BOOL CSqlLogFileDlg::RefreshEditData()
{
	m_edit_ctrl.ClearAll();

	int selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);

	for(; selected_row != -1; ) {
		int idx = (int)m_list.GetItemData(selected_row);
		m_sql_log_list[idx].GetSql(&m_edit_data, TRUE);

		selected_row = m_list.GetNextItem(selected_row, LVNI_SELECTED | LVNI_ALL);
	}

	m_edit_ctrl.Redraw();
	m_ok.EnableWindow(TRUE);

	return TRUE;
}

void CSqlLogFileDlg::OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	*pResult = 0;

	if((pNMListView->uChanged & LVIF_STATE) == 0) return;
	if((pNMListView->uNewState & LVIS_SELECTED) == 0) return;

	int	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) {
		m_ok.EnableWindow(FALSE);
		return;
	}

	RefreshEditData();
}

void CSqlLogFileDlg::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	int	selected_row;
	selected_row = m_list.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
	if(selected_row == -1) return;

	OnOK();	
}

void CSqlLogFileDlg::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	
	CDialog::OnOK();
}

CString CSqlLogFileDlg::GetSelectedSQL()
{
	return m_edit_data.get_all_text();
}

BOOL CSqlLogFileDlg::ReadLogFileMain(const TCHAR* file_name, CString sql_date, TCHAR* buf)
{
	FILE *fp;
	fp = OpenLogFile(file_name);
	if(fp == NULL) return TRUE;

	// 先頭3行をスキップ
	for(;;) {
		if(_fgetts(buf, size_of_buf, fp) == NULL) break;
		if(_tcsncmp(buf, _T("-- "), 3) == 0) continue;
		break;
	}

	long	start_offset = 0;
	int		row_cnt = 0;
	CString user;
	CString sql_time;
	BOOL	sql_load_flg = FALSE;
	ul_it	it;
	CEditData	edit_data;

	edit_data.set_str_token(&g_sql_str_token);
	
	for(;;) {
		if(_fgetts(buf, size_of_buf, fp) == NULL) break;

		if(!sql_load_flg && _tcsncmp(buf, _T("--"), 2) == 0) {
			ostr_chomp(buf, '\0');

			TCHAR *p1, *p2;

			p1 = buf + 2;
			for(p2 = p1; *p2 != ' ' && *p2 != '\0'; p2++) ;

			if(p1 == p2) continue;

			if(_tcsncmp(user, p1, (p2 - p1)) != 0) {
				user.Format(_T("%.*s"), p2 - p1, p1);
				it = m_user_list.find(user.GetBuffer(0));
				if(it == m_user_list.end()) {
					it = m_user_list.insert(UserListContainer::value_type(user.GetBuffer(0), 1)).first;
				} else {
					(it->second)++;
				}
			} else {
				if(it != m_user_list.end()) (it->second)++;
			}
			
			if(*p2 == ' ') {
				p2++;
				sql_time = sql_date + ' ' + p2;
			}

			start_offset = ftell(fp);
			row_cnt = 0;
			sql_load_flg = TRUE;
		} else if(sql_load_flg) {
			edit_data.paste_no_undo(buf);

			row_cnt++;
			if(_tcscmp(buf, _T("--/\n")) == 0) {
				edit_data.check_comment_row(-1, -1, 0, NULL);
				edit_data.move_document_end();
				if(edit_data.check_char_type() == CHAR_TYPE_NORMAL) {
					CSqlLogFileElement e(file_name, user, sql_time, start_offset, row_cnt - 1);
					m_sql_log_list.push_back(e);
					sql_load_flg = FALSE;
					edit_data.del_all();
				}
			}
		}
	}

	fclose(fp);

	return TRUE;
}

BOOL CSqlLogFileDlg::ReadLogFile()
{
	CWaitCursor		cur;

	m_sql_log_list.clear();
	m_user_list.clear();

	if(m_date.Format(_T("%Y%m%d")).IsEmpty()) return TRUE;

	// ディレクトリ名の最後の \をはずす
	TCHAR search_folder[_MAX_PATH];
	_tcscpy(search_folder, m_sql_log_dir);
	make_dirname2(search_folder);

	CTime cur_date, end_date;
	if (m_date2.Format(_T("%Y%m%d")).IsEmpty()) {
		cur_date = m_date;
		end_date = m_date;
	}
	else {
		cur_date = m_date;
		end_date = m_date2;
	}

	if (end_date - cur_date >= CTimeSpan(30 * 24 * 60 * 60)) {
		if (MessageBox(_T("日付範囲が30日以上あります。処理を実行しますか？"), _T("確認"), MB_ICONQUESTION | MB_YESNO) == IDNO) {
			return FALSE;
		}
	}

	CTimeSpan one_day(24 * 60 * 60);

	TCHAR* buf = (TCHAR*)malloc(size_of_buf * sizeof(TCHAR));
	if (buf == NULL) {
		// エラー
		MessageBox(_T("メモリが足りません"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	for (; cur_date <= end_date;) {
		if (m_sql_log_list.size() > 10000) {
			MessageBox(_T("SQLが10,000件を超えたので処理を中断します"), _T("Message"), MB_OK | MB_ICONEXCLAMATION);
			break;
		}

		WIN32_FIND_DATA		find_data;
		CString find_path;
		find_path.Format(_T("%s\\%s*.sql"), search_folder, cur_date.Format("%Y%m%d").GetString());

		HANDLE hFind = FindFirstFile(find_path, &find_data);
		if (hFind != INVALID_HANDLE_VALUE) {
			CString file_name;
			for (;;) {
				file_name.Format(_T("%s\\%s"), search_folder, find_data.cFileName);
				ReadLogFileMain(file_name, cur_date.Format("%Y/%m/%d"), buf);
				if (FindNextFile(hFind, &find_data) == FALSE) break;
			}
			FindClose(hFind);
		}

		cur_date += one_day;
	}

	free(buf);

	m_last_date = m_date;
	m_last_date2 = m_date2;

	// 現在選択中のユーザーを取得
	CString user = GetSelectedUser();
	// コンボボックスをクリアする
	m_combo_user.ResetContent();
	m_user.Empty();

	for(ul_it it = m_user_list.begin(); it != m_user_list.end(); it++) {
		CString str;
		str.Format(_T("%s (%d)"), it->first.c_str(), it->second);
		m_combo_user.AddString(str);

		// 選択中のユーザーを選択する
		if(user.Compare(it->first.c_str()) == 0) m_user = str;
	}

	// ユーザーが選択できなかったとき，先頭のユーザーを選択する
	if(!m_user_list.empty() && m_user.IsEmpty()) {
		m_user = m_user_list.begin()->first.c_str();
	}

	UpdateData(FALSE);

	return TRUE;
}

void CSqlLogFileDlg::OnSelchangeComboUser() 
{
	UpdateData(TRUE);
	SetLogList();
}

void CSqlLogFileDlg::OnButtonSearch() 
{
	UpdateData(TRUE);
	SetLogList();
}

CString CSqlLogFileDlg::GetSelectedUser()
{
	if(m_user.IsEmpty()) return m_user;

	CString user = m_user;
	int i = user.Find('(');
	if(i == -1) return m_user;

	i--;
	user.Delete(i, user.GetLength() - i);
	return user;
}

void CSqlLogFileDlg::OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	int idx = (int)pDispInfo->item.lParam;

	*pResult = 0;

	if(pDispInfo->item.mask & LVIF_TEXT) {
		// 画面表示する文字列
		const TCHAR *p;
		if(pDispInfo->item.iSubItem == LIST_SQL) {
			p = m_sql_log_list[idx].GetSummary();
		} else if(pDispInfo->item.iSubItem == LIST_TIME) {
			p = m_sql_log_list[idx].GetTime();
		} else if(pDispInfo->item.iSubItem == LIST_NO) {
			p = m_sql_log_list[idx].GetNo();
		} else {
			return;
		}

		_tcsncpy(pDispInfo->item.pszText, p, pDispInfo->item.cchTextMax - 1);
		pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = '\0'; // バッファに入りきらないときの処理
		*pResult = _tcslen(pDispInfo->item.pszText); // 文字数をセット 
	}
}

LRESULT CSqlLogFileDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if(message == WMU_EDIT_KEYDOWN) {
		switch(wParam) {
		case VK_RETURN:
			OnButtonSearch();
			break;
		case VK_ESCAPE:
			OnCancel();
			break;
		case VK_TAB:
			NextDlgCtrl();
			break;
		}
	}
	
	return CDialog::WindowProc(message, wParam, lParam);
}

void CSqlLogFileDlg::OnDatetimechangeDatetimepicker1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMDATETIMECHANGE pnm = (LPNMDATETIMECHANGE)pNMHDR;

	// カレンダー表示中は処理しない (カレンダーからの選択はCloseupで処理)
	if (m_date_picker.GetMonthCalCtrl() != NULL) return;

	UpdateData(TRUE);

	if (m_date != m_last_date || m_date2 != m_last_date2) {
		// 日付が変わったら実行する
		ReadLogFile();
		SetLogList();
	}

	*pResult = 0;
}

void CSqlLogFileDlg::OnDatetimechangeDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMDATETIMECHANGE pnm = (LPNMDATETIMECHANGE)pNMHDR;

	// カレンダー表示中は処理しない (カレンダーからの選択はCloseupで処理)
	if (m_date_picker2.GetMonthCalCtrl() != NULL) return;

	UpdateData(TRUE);

	if (m_date != m_last_date || m_date2 != m_last_date2) {
		// 日付が変わったら実行する
		ReadLogFile();
		SetLogList();
	}

	*pResult = 0;
}

void CSqlLogFileDlg::OnCloseupDatetimepicker1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// カレンダーから選択した場合，２回通知がくる
	UpdateData(TRUE);

	if (m_date != m_last_date || m_date2 != m_last_date2) {
		// 日付が変わったら実行する
		ReadLogFile();
		SetLogList();
	}

	*pResult = 0;
}

void CSqlLogFileDlg::OnCloseupDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult)
{
	// カレンダーから選択した場合，２回通知がくる
	UpdateData(TRUE);

	if (m_date != m_last_date || m_date2 != m_last_date2) {
		// 日付が変わったら実行する
		ReadLogFile();
		SetLogList();
	}

	*pResult = 0;
}
