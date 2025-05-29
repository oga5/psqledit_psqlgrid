/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
// psqlgrid.cpp : アプリケーション用クラスの機能定義を行います。
//

#include "stdafx.h"
#define _MAIN
#include "psqlgrid.h"

#include "MainFrm.h"
#include "psqlgridDoc.h"
#include "psqlgridView.h"

#include "LoginDlg.h"

#include "file_winutil.h"
#include "fileutil.h"
#include "ostrutil.h"

#include "AboutDlg.h"

#include <process.h>
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define _MAIN
#include "global.h"

#include "optionsheet.h"
#include "ConnectInfoDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridApp

BEGIN_MESSAGE_MAP(CPsqlgridApp, CWinApp)
	//{{AFX_MSG_MAP(CPsqlgridApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_SET_FONT, OnSetFont)
	ON_COMMAND(ID_OPTION, OnOption)
	ON_COMMAND(ID_SET_LOGIN_INFO, OnSetLoginInfo)
	ON_COMMAND(ID_RECONNECT, OnReconnect)
	//}}AFX_MSG_MAP
	// 標準のファイル基本ドキュメント コマンド
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_SHOW_CREDITS, &CPsqlgridApp::OnShowCredits)
END_MESSAGE_MAP()

struct pre_login_thr_st {
	CMainFrame	*pMainFrame;
};

unsigned int _stdcall pre_login_thr(void *lpvThreadParam)
{
	struct pre_login_thr_st *p_st = (struct pre_login_thr_st *)lpvThreadParam;

	CString app_path = GetAppPath();

	// キーワードの初期化
	CString	key_word_file = app_path + _T("data\\keywords.txt");
	g_sql_str_token.initDefaultKeyword(key_word_file.GetBuffer(0), g_msg_buf);

	return 0;
}

unsigned int _stdcall post_login_thr(void *lpvThreadParam)
{
	int			ret_v;
	HPgSession	ss;

	ss = pg_login(g_connect_str.GetBuffer(0), g_msg_buf);
	if(ss == NULL) return 1;

	// キーワード補完用のデータを初期化
	ret_v = g_sql_str_token.initCompletionWord(ss, 
		pg_user(ss), g_msg_buf, TRUE, TRUE,
		_T("'r', 'v'"));
	if(ret_v != 0) goto ERR1;

	pg_logout(ss);

	return 0;

ERR1:
	pg_logout(ss);
	return ret_v;
}

static void get_psqledit_profname(CString &prof_name, CString &registry_key)
{
	CWinApp *pApp = AfxGetApp();

	registry_key = _T("OGAWA\\POSTGRESQL");
	prof_name = _T("psqledit");

	// psqleditの設定が，INIファイルを使うか調べる
	CString file_name;
	file_name.Format(_T("%spsqledit.ini"), GetAppPath());

	if(is_file_exist(file_name) == FALSE) return;

	// ファイルがあるとき，INIファイルの中身をしらべる
	const TCHAR *old_profile_name = pApp->m_pszProfileName;
	const TCHAR *old_registry_key = pApp->m_pszRegistryKey;
	pApp->m_pszRegistryKey = NULL;
	pApp->m_pszProfileName = file_name.GetBuffer(0);
	if(pApp->GetProfileInt(_T("APPLICATION"), _T("USE_INI_FILE"), 0) == 1) {
		registry_key = _T("");
		prof_name = file_name;
	}
	pApp->m_pszRegistryKey = old_registry_key;
	pApp->m_pszProfileName = old_profile_name;

	return;
}


/////////////////////////////////////////////////////////////////////////////
// CPsqlgridApp クラスの構築

CPsqlgridApp::CPsqlgridApp()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
	g_ss = NULL;
	g_login_flg = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CPsqlgridApp オブジェクト

CPsqlgridApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridApp クラスの初期化

BOOL CPsqlgridApp::InitInstance()
{
	// OLE ライブラリの初期化
	if (!AfxOleInit())
	{
//		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	{
		WSADATA		wsadata;
		WSAStartup(MAKEWORD(1, 1), &wsadata);
	}

	if(pg_init_library(g_msg_buf) != 0) {
		AfxMessageBox(g_msg_buf);
	}

	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さく
	//  したければ以下の特定の初期化ルーチンの中から不必要なもの
	//  を削除してください。

	// 設定が保存される下のレジストリ キーを変更します。
	// 会社名または所属など、適切な文字列に
	// 変更してください。
	SetRegistryKey(_T("OGAWA\\POSTGRESQL"));

	LoadStdProfileSettings(0);  // 標準の INI ファイルのオプションをロードします (MRU を含む)
	LoadOption();
	LoadFontOption();
	LoadBootOption();
	CreateFont();

	// カレントディレクトリを設定
	SetCurrentDirectory(g_option.initial_dir);

	// アプリケーション用のドキュメント テンプレートを登録します。ドキュメント テンプレート
	//  はドキュメント、フレーム ウィンドウとビューを結合するために機能します。

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CPsqlgridDoc),
		RUNTIME_CLASS(CMainFrame),       // メイン SDI フレーム ウィンドウ
		RUNTIME_CLASS(CPsqlgridView));
	AddDocTemplate(pDocTemplate);

	// DDE、file open など標準のシェル コマンドのコマンドラインを解析します。
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// ログイン前に初期化できる処理を実行
	struct pre_login_thr_st	pre_st;
	pre_st.pMainFrame = (CMainFrame *)m_pMainWnd;
	UINT thread_addr;
	uintptr_t h_thread = _beginthreadex(NULL, 0, pre_login_thr, &pre_st, 0, &thread_addr);
	if(h_thread == NULL) {
		AfxMessageBox(_T("スレッド作成エラー"));
		return FALSE;
	}

	// コマンドラインの処理
	{
		TCHAR *p = _tcsstr(m_lpCmdLine, _T("LOGON="));
		if(p != NULL) {
			p += 6;
			CString cmd_buf = p;

			if(pg_library_is_ok()) {
				g_ss = pg_login(cmd_buf.GetBuffer(0), g_msg_buf);
				if(g_ss != NULL) {
					g_login_flg = TRUE;
				}
			}

			if(cmdInfo.m_strFileName.Find(_T("LOGON=")) == 0) {
				cmdInfo.m_strFileName = _T("");
				cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
			}
		}
	}

	// ログイン
	if(!g_login_flg) {
		if(pg_library_is_ok() == FALSE || Login() != 0) {
			g_login_flg = FALSE;
			// ステータスバーに，ログイン情報を表示
			SetSessionInfo(_T("未接続"));
		} else {
			g_login_flg = TRUE;
		}
	}
	
	// スレッドの同期
	WaitForSingleObject((void *)h_thread, INFINITE);
	CloseHandle((void *)h_thread);

	// ステータスバーに表示する，ログイン情報を作成
	if(PostLogin() != 0) return FALSE;

	// コマンドをFileNewに書きかえる
	//cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;

	// コマンドラインでディスパッチ コマンドを指定します。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// メイン ウィンドウが初期化されたので、表示と更新を行います。
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	// アクセラレータを初期化する
	((CMainFrame *)m_pMainWnd)->InitAccelerator();

	if(cmdInfo.m_nShellCommand == CCommandLineInfo::FileOpen) {
//		m_pMainWnd->SetForegroundWindow();
		
		HWND hWnd = m_pMainWnd->GetSafeHwnd();
		// それぞれのウィンドウのスレッドIDを取得
		DWORD nForegroundID	= GetWindowThreadProcessId(GetForegroundWindow(),NULL);
		DWORD nTargetID	= GetWindowThreadProcessId(hWnd,NULL);
		// スレッドのインプット状態を結び付ける
		AttachThreadInput(nTargetID,nForegroundID,TRUE);
		BringWindowToTop(hWnd);
		// スレッドのインプット状態を切り離す
		AttachThreadInput(nTargetID,nForegroundID,FALSE);
	}

	return TRUE;
}

// ダイアログを実行するためのアプリケーション コマンド
void CPsqlgridApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridApp コマンド

int CPsqlgridApp::ExitInstance() 
{
	Logout();

	g_sql_str_token.FreeDataset();

	pg_free_library(NULL);

	WSACleanup();

	SaveBootOption();
	
	return CWinApp::ExitInstance();
}

void CPsqlgridApp::SaveOption()
{
	WriteProfileInt(_T("SETUP"), _T("PUT_COLUMN_NAME"), g_option.put_column_name);
	WriteProfileInt(_T("SETUP"), _T("ADJUST_COL_WIDTH"), g_option.adjust_col_width);

	WriteProfileInt(_T("SETUP"), _T("USE_MESSAGE_BEEP"), g_option.use_message_beep);

	WriteProfileString(_T("SESSION"), _T("NLS_DATE_FORMAT"), g_option.nls_date_format);
	WriteProfileString(_T("SESSION"), _T("NLS_TIMESTAMP_FORMAT"), g_option.nls_timestamp_format);
	WriteProfileString(_T("SESSION"), _T("NLS_TIMESTAMP_TZ_FORMAT"), g_option.nls_timestamp_tz_format);

	WriteProfileInt(_T("SETUP"), _T("USE_USER_UPDATABLE_COLUMNS"), g_option.use_user_updatable_columns);
	WriteProfileInt(_T("SETUP"), _T("SHARE_CONNECT_INFO"), g_option.share_connect_info);
	WriteProfileInt(_T("SETUP"), _T("EDIT_OTHER_OWNER"), g_option.edit_other_owner);
	WriteProfileInt(_T("SETUP"), _T("DISP_DELETE_ALL_NULL_ROWS_WARNING"), g_option.disp_all_delete_null_rows_warning);
	WriteProfileInt(_T("SESSION"), _T("DATA_LOCK"), g_option.data_lock);
	WriteProfileString(_T("SESSION"), _T("TIMEZONE"), g_option.timezone);

	WriteProfileString(_T("APPLICATION"), _T("INITIAL_DIR"), g_option.initial_dir);

	WriteProfileInt(_T("GRID"), _T("SHOW_SPACE"), g_option.grid.show_space);
	WriteProfileInt(_T("GRID"), _T("SHOW_2BYTE_SPACE"), g_option.grid.show_2byte_space);
	WriteProfileInt(_T("GRID"), _T("SHOW_TAB"), g_option.grid.show_tab);
	WriteProfileInt(_T("GRID"), _T("SHOW_LINE_FEED"), g_option.grid.show_line_feed);
	WriteProfileInt(_T("GRID"), _T("ADJUST_COL_WIDTH_NO_USE_COLNAME"), g_option.grid.adjust_col_width_no_use_colname);
	WriteProfileInt(_T("GRID"), _T("INVERT_SELECT_TEXT"), g_option.grid.invert_select_text);
	WriteProfileInt(_T("GRID"), _T("COPY_ESCAPE_DBLQUOTE"), g_option.grid.copy_escape_dblquote);
	WriteProfileInt(_T("GRID"), _T("IME_CARET_COLOR"), g_option.grid.ime_caret_color);
	WriteProfileInt(_T("GRID"), _T("END_KEY_LIKE_EXCEL"), g_option.grid.end_key_like_excel);
	WriteProfileString(_T("GRID"), _T("NULL_TEXT"), g_option.grid.null_text);

	WriteProfileInt(_T("GRID"), _T("GRID_BG_COLOR"), g_option.grid.color[GRID_BG_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_HEADER_BG_COLOR"), g_option.grid.color[GRID_HEADER_BG_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_LINE_COLOR"), g_option.grid.color[GRID_LINE_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_SELECT_COLOR"), g_option.grid.color[GRID_SELECT_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_DELETE_COLOR"), g_option.grid.color[GRID_DELETE_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_INSERT_COLOR"), g_option.grid.color[GRID_INSERT_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_UPDATE_COLOR"), g_option.grid.color[GRID_UPDATE_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_UNEDITABLE_COLOR"), g_option.grid.color[GRID_UNEDITABLE_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_TEXT_COLOR"), g_option.grid.color[GRID_TEXT_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_NULL_CELL_COLOR"), g_option.grid.color[GRID_NULL_CELL_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_MARK_COLOR"), g_option.grid.color[GRID_MARK_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_SEARCH_COLOR"), g_option.grid.color[GRID_SEARCH_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_HEADER_LINE_COLOR"), g_option.grid.color[GRID_HEADER_LINE_COLOR]);

	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_TOP"), g_option.grid.cell_padding_top);
	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_BOTTOM"), g_option.grid.cell_padding_bottom);
	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_LEFT"), g_option.grid.cell_padding_left);
	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_RIGHT"), g_option.grid.cell_padding_right);
}

void CPsqlgridApp::LoadOption()
{
	g_option.put_column_name = GetProfileInt(_T("SETUP"), _T("PUT_COLUMN_NAME"), 1);
	g_option.adjust_col_width = GetProfileInt(_T("SETUP"), _T("ADJUST_COL_WIDTH"), 0);

	g_option.use_message_beep = GetProfileInt(_T("SETUP"), _T("USE_MESSAGE_BEEP"), FALSE);

	g_option.use_user_updatable_columns = GetProfileInt(_T("SETUP"), _T("USE_USER_UPDATABLE_COLUMNS"), FALSE);
	g_option.share_connect_info = GetProfileInt(_T("SETUP"), _T("SHARE_CONNECT_INFO"), FALSE);
	g_option.edit_other_owner = GetProfileInt(_T("SETUP"), _T("EDIT_OTHER_OWNER"), FALSE);
	g_option.disp_all_delete_null_rows_warning = GetProfileInt(_T("SETUP"), _T("DISP_DELETE_ALL_NULL_ROWS_WARNING"), 1);

	g_option.data_lock = GetProfileInt(_T("SESSION"), _T("DATA_LOCK"), 1);
	g_option.nls_date_format = GetProfileString(_T("SESSION"), _T("NLS_DATE_FORMAT"), _T("YYYY-MM-DD"));
	g_option.nls_timestamp_format = GetProfileString(_T("SESSION"), _T("NLS_TIMESTAMP_FORMAT"), _T("YYYY-MM-DD HH24:MI:SSXFF"));
	g_option.nls_timestamp_tz_format = GetProfileString(_T("SESSION"), _T("NLS_TIMESTAMP_TZ_FORMAT"), _T("YYYY-MM-DD HH24:MI:SSXFF TZH:TZM"));
	g_option.timezone = GetProfileString(_T("SESSION"), _T("TIMEZONE"), _T("+09:00"));

	g_option.initial_dir = GetProfileString(_T("APPLICATION"), _T("INITIAL_DIR"), _T(""));
	if(g_option.initial_dir == "") {
		g_option.initial_dir = GetAppPath();
	}

	g_option.grid.show_space = GetProfileInt(_T("GRID"), _T("SHOW_SPACE"), FALSE);
	g_option.grid.show_2byte_space = GetProfileInt(_T("GRID"), _T("SHOW_2BYTE_SPACE"), FALSE);
	g_option.grid.show_tab = GetProfileInt(_T("GRID"), _T("SHOW_TAB"), FALSE);
	g_option.grid.show_line_feed = GetProfileInt(_T("GRID"), _T("SHOW_LINE_FEED"), FALSE);
	g_option.grid.adjust_col_width_no_use_colname = GetProfileInt(_T("GRID"), _T("ADJUST_COL_WIDTH_NO_USE_COLNAME"), FALSE);
	g_option.grid.invert_select_text = GetProfileInt(_T("GRID"), _T("INVERT_SELECT_TEXT"), FALSE);
	g_option.grid.copy_escape_dblquote = GetProfileInt(_T("GRID"), _T("COPY_ESCAPE_DBLQUOTE"), FALSE);
	g_option.grid.ime_caret_color = GetProfileInt(_T("GRID"), _T("IME_CARET_COLOR"), TRUE);
	g_option.grid.end_key_like_excel = GetProfileInt(_T("GRID"), _T("END_KEY_LIKE_EXCEL"), FALSE);
	g_option.grid.null_text = GetProfileString(_T("GRID"), _T("NULL_TEXT"), _T(""));

	g_option.grid.color[GRID_BG_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_BG_COLOR"), RGB(255, 255, 255));
	g_option.grid.color[GRID_HEADER_BG_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_HEADER_BG_COLOR"), RGB(230, 230, 255));
	g_option.grid.color[GRID_LINE_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_LINE_COLOR"), RGB(200, 200, 250));
	g_option.grid.color[GRID_SELECT_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_SELECT_COLOR"), RGB(200, 200, 220));
	g_option.grid.color[GRID_DELETE_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_DELETE_COLOR"), RGB(150, 150, 150));
	g_option.grid.color[GRID_INSERT_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_INSERT_COLOR"), RGB(220, 220, 255));
	g_option.grid.color[GRID_UPDATE_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_UPDATE_COLOR"), RGB(255, 220, 220));
	g_option.grid.color[GRID_UNEDITABLE_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_UNEDITABLE_COLOR"), RGB(240, 240, 240));
	g_option.grid.color[GRID_TEXT_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_TEXT_COLOR"), RGB(0, 0, 0));
	g_option.grid.color[GRID_NULL_CELL_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_NULL_CELL_COLOR"), RGB(245, 245, 245));
	g_option.grid.color[GRID_MARK_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_MARK_COLOR"), RGB(0, 150, 150));
	g_option.grid.color[GRID_SEARCH_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_SEARCH_COLOR"), RGB(255, 255, 50));
	g_option.grid.color[GRID_HEADER_LINE_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_HEADER_LINE_COLOR"), RGB(100, 100, 100));

	g_option.grid.cell_padding_top = GetProfileInt(_T("GRID"), _T("CELL_PADDING_TOP"), 1);
	g_option.grid.cell_padding_bottom = GetProfileInt(_T("GRID"), _T("CELL_PADDING_BOTTOM"), 1);
	g_option.grid.cell_padding_left = GetProfileInt(_T("GRID"), _T("CELL_PADDING_LEFT"), 2);
	g_option.grid.cell_padding_right = GetProfileInt(_T("GRID"), _T("CELL_PADDING_RIGHT"), 2);
}

void CPsqlgridApp::SaveBootOption()
{
	WriteProfileInt(_T("APPLICATION"), _T("GRID_CALC_TYPE"), g_option.grid_calc_type);
	WriteProfileInt(_T("SESSION"), _T("DATA_LOCK"), g_option.data_lock);
}

void CPsqlgridApp::LoadBootOption()
{
	g_option.grid_calc_type = (GRID_CALC_TYPE)GetProfileInt(_T("APPLICATION"), _T("GRID_CALC_TYPE"), GRID_CALC_TYPE_TOTAL);
}

void CPsqlgridApp::SaveFontOption()
{
	WriteProfileString(_T("FONT"), _T("FACENAME"), g_option.font.face_name);
	WriteProfileInt(_T("FONT"), _T("POINT"), g_option.font.point);
	WriteProfileInt(_T("FONT"), _T("WEIGHT"), g_option.font.weight);
	WriteProfileInt(_T("FONT"), _T("ITALIC"), g_option.font.italic);
}

void CPsqlgridApp::LoadFontOption()
{
	g_option.font.face_name = GetProfileString(_T("FONT"), _T("FACENAME"), _T("Terminal"));
	g_option.font.point = GetProfileInt(_T("FONT"), _T("POINT"), 100);
	g_option.font.weight = GetProfileInt(_T("FONT"), _T("WEIGHT"), FW_NORMAL);
	g_option.font.italic = GetProfileInt(_T("FONT"), _T("ITALIC"), 0);
}

BOOL CPsqlgridApp::CreateFont()
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = g_option.font.point;
	lf.lfWeight = g_option.font.weight;
	lf.lfItalic = g_option.font.italic;
	lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy(lf.lfFaceName, g_option.font.face_name);

	return g_font.CreatePointFontIndirect(&lf);
}

void CPsqlgridApp::OnSetFont() 
{
	CFontDialog		fontdlg;

	// ダイアログ初期化
	fontdlg.m_cf.Flags &= ~CF_EFFECTS;
	fontdlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
	g_font.GetLogFont(fontdlg.m_cf.lpLogFont);

	// ダイアログ表示
	if(fontdlg.DoModal() != IDOK) return;

	g_font.DeleteObject();

	g_option.font.face_name = fontdlg.GetFaceName();
	g_option.font.point = fontdlg.GetSize();
	g_option.font.weight = fontdlg.GetWeight();
	g_option.font.italic = fontdlg.IsItalic();

	CreateFont();
	
	UpdateAllDocViews(NULL, UPD_FONT, &g_font);
	SaveFontOption();
}

void CPsqlgridApp::UpdateAllDocViews(CView* pSender, LPARAM lHint, CObject* pHint)
{
	POSITION	doc_tmpl_pos = GetFirstDocTemplatePosition();
	for(; doc_tmpl_pos != NULL;) {
		CDocTemplate	*doc_tmpl = GetNextDocTemplate(doc_tmpl_pos);
		POSITION doc_pos = doc_tmpl->GetFirstDocPosition();
		for(; doc_pos != NULL; ) {
			doc_tmpl->GetNextDoc(doc_pos)->UpdateAllViews(pSender, lHint, pHint);
		}
	}
}

int CPsqlgridApp::Login()
{
	CLoginDlg	dlg;

	dlg.m_title = "PSqlGrid";

	dlg.m_user = GetProfileString(_T("CONNECT_INFO"), _T("USER-CUR"), _T(""));
	dlg.m_host = GetProfileString(_T("CONNECT_INFO"), _T("HOST-CUR"), _T(""));
	dlg.m_port = GetProfileString(_T("CONNECT_INFO"), _T("PORT-CUR"), _T(""));
	dlg.m_dbname = GetProfileString(_T("CONNECT_INFO"), _T("DBNAME-CUR"), _T(""));

	if(g_option.share_connect_info) {
		CString prof_name;
		CString registry_key;

		get_psqledit_profname(prof_name, registry_key);
		dlg.SetOptProfName(prof_name, registry_key);
	}

	if(dlg.DoModal() != IDOK) {
		return 1;
	}

	WriteProfileString(_T("CONNECT_INFO"), _T("USER-CUR"), dlg.m_user);
	WriteProfileString(_T("CONNECT_INFO"), _T("HOST-CUR"), dlg.m_host);
	WriteProfileString(_T("CONNECT_INFO"), _T("PORT-CUR"), dlg.m_port);
	WriteProfileString(_T("CONNECT_INFO"), _T("DBNAME-CUR"), dlg.m_dbname);

	if(g_ss != NULL) Logout();

	g_ss = dlg.m_ss;
	g_connect_str = dlg.m_connect_str;

//	pg_set_notice_processor(g_ss, my_notice_processor, NULL);

	return 0;
}

int CPsqlgridApp::PostLogin()
{
	if(g_login_flg == FALSE) return 0;

	CWaitCursor		wait_cursor;
	UINT			thread_addr;

	if(pg_auto_commit_off(g_ss, g_msg_buf) != 0) {
		return 1;
	}

	m_h_thread = _beginthreadex(NULL, 0, post_login_thr, NULL, 0, &thread_addr);
	if(m_h_thread == NULL) {
		_stprintf(g_msg_buf, _T("スレッド作成エラー"));
		return 1;
	}

	// ステータスバーに，ログイン情報を表示
	CString session_info;
	session_info.Format(_T("%s@%s:%s.%s"), pg_user(g_ss), pg_host(g_ss), pg_port(g_ss), pg_db(g_ss));
	if(pg_is_ssl_mode(g_ss)) session_info += "(SSL)";
	SetSessionInfo(session_info.GetBuffer(0));

	return 0;
}

void CPsqlgridApp::Logout()
{
	WaitPostLoginThr();

	if(g_ss == NULL) return;

	pg_logout(g_ss);

	g_ss = NULL;
	g_login_flg = FALSE;

	SetSessionInfo(_T("未接続"));
}

void CPsqlgridApp::OnOption() 
{
	int		i;
	BOOL	b_restart = FALSE;
	BOOL	b_register_shell = CheckRegisterShellFileTypes();

	COptionSheet dlg(_T("オプション"), AfxGetMainWnd());
	
	dlg.m_setup_page.m_register_shell = b_register_shell;
	dlg.m_setup_page.m_put_column_name = g_option.put_column_name;
	dlg.m_setup_page.m_use_message_beep = g_option.use_message_beep;
	dlg.m_setup_page.m_nls_date_format = g_option.nls_date_format;
	dlg.m_setup_page.m_nls_timestamp_format = g_option.nls_timestamp_format;
	dlg.m_setup_page.m_nls_timestamp_tz_format = g_option.nls_timestamp_tz_format;
	dlg.m_setup_page.m_timezone = g_option.timezone;
	dlg.m_setup_page.m_adjust_col_width = g_option.adjust_col_width;
	dlg.m_setup_page.m_initial_dir = g_option.initial_dir;

	dlg.m_setup_page.m_use_user_updatable_columns = g_option.use_user_updatable_columns;
	dlg.m_setup_page.m_share_connect_info = g_option.share_connect_info;
	dlg.m_setup_page.m_edit_other_owner = g_option.edit_other_owner;
	dlg.m_setup_page.m_disp_all_delete_null_rows_warning = g_option.disp_all_delete_null_rows_warning;

	dlg.m_grid_page.m_show_space = g_option.grid.show_space;
	dlg.m_grid_page.m_show_2byte_space = g_option.grid.show_2byte_space;
	dlg.m_grid_page.m_show_tab = g_option.grid.show_tab;
	dlg.m_grid_page.m_show_line_feed = g_option.grid.show_line_feed;
	dlg.m_grid_page.m_adjust_col_width_no_use_colname = g_option.grid.adjust_col_width_no_use_colname;
	dlg.m_grid_page.m_invert_select_text = g_option.grid.invert_select_text;
	dlg.m_grid_page.m_copy_escape_dblquote = g_option.grid.copy_escape_dblquote;
	dlg.m_grid_page.m_ime_caret_color = g_option.grid.ime_caret_color;
	dlg.m_grid_page.m_end_key_like_excel = g_option.grid.end_key_like_excel;
	dlg.m_grid_page.m_null_text = g_option.grid.null_text;
	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		dlg.m_grid_page.m_color[i] = g_option.grid.color[i];
	}

	dlg.m_grid_page.m_cell_padding_top = g_option.grid.cell_padding_top;
	dlg.m_grid_page.m_cell_padding_bottom = g_option.grid.cell_padding_bottom;
	dlg.m_grid_page.m_cell_padding_left = g_option.grid.cell_padding_left;
	dlg.m_grid_page.m_cell_padding_right = g_option.grid.cell_padding_right;

	if(dlg.DoModal() != IDOK) return;

	g_option.put_column_name = dlg.m_setup_page.m_put_column_name;
	g_option.use_message_beep = dlg.m_setup_page.m_use_message_beep;
	g_option.adjust_col_width = dlg.m_setup_page.m_adjust_col_width;

	g_option.use_user_updatable_columns = dlg.m_setup_page.m_use_user_updatable_columns;
	g_option.share_connect_info = dlg.m_setup_page.m_share_connect_info;
	g_option.edit_other_owner = dlg.m_setup_page.m_edit_other_owner;
	g_option.disp_all_delete_null_rows_warning = dlg.m_setup_page.m_disp_all_delete_null_rows_warning;

	g_option.nls_date_format = dlg.m_setup_page.m_nls_date_format;
	g_option.nls_timestamp_format = dlg.m_setup_page.m_nls_timestamp_format;
	g_option.nls_timestamp_tz_format = dlg.m_setup_page.m_nls_timestamp_tz_format;
	g_option.timezone = dlg.m_setup_page.m_timezone;
	if(g_login_flg) {
//		SetupDateTime();
	}

	if(g_option.initial_dir.Compare(dlg.m_setup_page.m_initial_dir) != 0) {
		g_option.initial_dir = dlg.m_setup_page.m_initial_dir;
		SetCurrentDirectory(g_option.initial_dir);
	}

	if(b_register_shell != dlg.m_setup_page.m_register_shell) {
		if(dlg.m_setup_page.m_register_shell == TRUE) {
			UnRegisterShellFileTypes();
			RegisterShellFileTypes(FALSE);
		} else {
			UnRegisterShellFileTypes();
		}
	}

	g_option.grid.show_space = dlg.m_grid_page.m_show_space;
	g_option.grid.show_2byte_space = dlg.m_grid_page.m_show_2byte_space;
	g_option.grid.show_tab = dlg.m_grid_page.m_show_tab;
	g_option.grid.show_line_feed = dlg.m_grid_page.m_show_line_feed;
	g_option.grid.adjust_col_width_no_use_colname = dlg.m_grid_page.m_adjust_col_width_no_use_colname;
	g_option.grid.invert_select_text = dlg.m_grid_page.m_invert_select_text;
	g_option.grid.copy_escape_dblquote = dlg.m_grid_page.m_copy_escape_dblquote;
	g_option.grid.ime_caret_color = dlg.m_grid_page.m_ime_caret_color;
	g_option.grid.null_text = dlg.m_grid_page.m_null_text;

	if(g_option.grid.end_key_like_excel != dlg.m_grid_page.m_end_key_like_excel) {
		g_option.grid.end_key_like_excel = dlg.m_grid_page.m_end_key_like_excel;

		CMainFrame	*pMainFrame = (CMainFrame *)m_pMainWnd;
		pMainFrame->SetIndicators();
	}

	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		g_option.grid.color[i] = dlg.m_grid_page.m_color[i];
	}

	g_option.grid.cell_padding_top = dlg.m_grid_page.m_cell_padding_top;
	g_option.grid.cell_padding_bottom = dlg.m_grid_page.m_cell_padding_bottom;
	g_option.grid.cell_padding_left = dlg.m_grid_page.m_cell_padding_left;
	g_option.grid.cell_padding_right = dlg.m_grid_page.m_cell_padding_right;

	UpdateAllDocViews(NULL, UPD_GRID_OPTION, 0);

	SaveOption();

	if(b_restart) {
		AfxGetMainWnd()->MessageBox(_T("設定を反映するために，osqlgridの再起動が必要です"), 
			_T("Message"), MB_ICONINFORMATION | MB_OK);
	}
}

void CPsqlgridApp::OnSetLoginInfo() 
{
	CConnectInfoDlg	dlg;

	dlg.DoModal();
}

void CPsqlgridApp::UnRegisterShellFileTypes()
{
	m_pDocManager->UnregisterShellFileTypes();

	// 他のツールが関連付けられている場合，削除する
	CString strFilterExt;
	POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
	while (pos != NULL) {
		CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(pos);
		pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt);
		if (!strFilterExt.IsEmpty()) {
			ASSERT(strFilterExt[0] == '.');
			RegDeleteKey(HKEY_CLASSES_ROOT, strFilterExt);
		}
	}
}

BOOL CPsqlgridApp::CheckRegisterShellFileTypes()
{
	CString strFilterExt, strFileTypeId, strTemp;
	long	lSize = MAX_PATH * 2;

	POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();

	while (pos != NULL) {
		CDocTemplate* pTemplate = m_pDocManager->GetNextDocTemplate(pos);
		pTemplate->GetDocString(strFileTypeId, CDocTemplate::regFileTypeId);
		pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt);

		if (!strFilterExt.IsEmpty() && !strFileTypeId.IsEmpty()) {
			ASSERT(strFilterExt[0] == '.');
			LONG lResult = ::RegQueryValue(HKEY_CLASSES_ROOT, strFilterExt,
				strTemp.GetBuffer(lSize), &lSize);
			strTemp.ReleaseBuffer();

			if (strFileTypeId == strTemp) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

void CPsqlgridApp::WaitPostLoginThr()
{
	if(m_h_thread != NULL) {
		WaitForSingleObject((void *)m_h_thread, INFINITE);
		CloseHandle((void *)m_h_thread);
		m_h_thread = NULL;
	}
}

int CPsqlgridApp::ReConnect()
{
	// ログイン
	if(Login() != 0) {
		return 1;
	}

	g_login_flg = TRUE;

	// キーワードリスト，オブジェクトリストを更新
	PostLogin();

	return 0;
}

void CPsqlgridApp::OnReconnect() 
{
	UpdateAllDocViews(NULL, UPD_CLEAR_DATA, 0);
	ReConnect();
	UpdateAllDocViews(NULL, UPD_RECONNECT, 0);
}

void CPsqlgridApp::SetSessionInfo(TCHAR *session_info)
{
	g_session_info = session_info;
	if(::IsWindow(m_pMainWnd->GetSafeHwnd())) {
		CString title;
		title.Format(_T("%s"), session_info);
		m_pMainWnd->SetWindowText(title);
	}
}

void CPsqlgridApp::OnShowCredits()
{
	OpenHelpFile(_T("credits.txt"));
}
