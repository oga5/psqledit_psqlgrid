/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // psqledit.cpp : アプリケーション用クラスの機能定義を行います。
//

#include "stdafx.h"

#include <process.h>

#define MAIN_PROG
#include "psqledit.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "psqleditDoc.h"
#include "SQLEditView.h"

#include "AboutDlg.h"
#include "LoginDlg.h"
#include "SQLLibraryDlg.h"
#include "ConnectInfoDlg.h"

#include "fileutil.h"
#include "file_winutil.h"
#include "OptionSheet.h"

#include "ostrutil.h"
#include "getsrc.h"
#include "SqlListMaker.h"

#include "completion_util.h"

#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct pre_login_thr_st {
	CMainFrame	*pMainFrame;
};

unsigned int _stdcall pre_login_thr(void *lpvThreadParam)
{
	struct pre_login_thr_st *p_st = (struct pre_login_thr_st *)lpvThreadParam;

	CString app_path = GetAppPath();

	// キーワードの初期化
	CString	key_word_file = app_path + "data\\keywords.txt";
	g_sql_str_token.initDefaultKeyword(key_word_file.GetBuffer(0), g_msg_buf);

	g_sql_log_array = new CSqlLogArray(g_max_sql_log);
	g_sql_logger = new CSqlLogger(g_option.sql_log_dir.GetBuffer(0));

	// アクセラレータを初期化する
	p_st->pMainFrame->InitAccelerator();

	// ショートカットSQLをロード
	CPsqleditDoc::LoadShortCutSqlList();

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
		pg_user(ss), g_msg_buf, 
		g_option.text_editor.table_name_completion, 
		g_option.text_editor.column_name_completion,
		make_completion_object_type());
	if(ret_v != 0) goto ERR1;

	pg_logout(ss);

	return 0;

ERR1:
	pg_logout(ss);
	return ret_v;
}


/////////////////////////////////////////////////////////////////////////////
// CPsqleditApp

BEGIN_MESSAGE_MAP(CPsqleditApp, CWinApp)
	//{{AFX_MSG_MAP(CPsqleditApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_OPTION, OnOption)
	ON_COMMAND(ID_LINE_MODE_LEN, OnLineModeLen)
	ON_COMMAND(ID_LINE_MODE_NORMAL, OnLineModeNormal)
	ON_COMMAND(ID_LINE_MODE_RIGHT, OnLineModeRight)
	ON_UPDATE_COMMAND_UI(ID_LINE_MODE_LEN, OnUpdateLineModeLen)
	ON_UPDATE_COMMAND_UI(ID_LINE_MODE_NORMAL, OnUpdateLineModeNormal)
	ON_UPDATE_COMMAND_UI(ID_LINE_MODE_RIGHT, OnUpdateLineModeRight)
	ON_COMMAND(ID_LOGFILE_OPEN, OnLogfileOpen)
	ON_COMMAND(ID_SET_FONT, OnSetFont)
	ON_COMMAND(ID_SET_FONT_DLG_BAR, OnSetFontDlgBar)
	ON_COMMAND(ID_REFRESH_TABLE_LIST, OnRefreshTableList)
	ON_COMMAND(ID_SQL_LIBRARY, OnSqlLibrary)
	ON_COMMAND(ID_SET_LOGIN_INFO, OnSetLoginInfo)
	ON_COMMAND(ID_RECONNECT, OnReconnect)
	ON_COMMAND(ID_DISCONNECT, OnDisconnect)
	ON_UPDATE_COMMAND_UI(ID_DISCONNECT, OnUpdateDisconnect)
	ON_COMMAND(ID_GET_OBJECT_SOURCE, OnGetObjectSource)
	ON_UPDATE_COMMAND_UI(ID_GET_OBJECT_SOURCE, OnUpdateGetObjectSource)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// 標準のファイル基本ドキュメント コマンド
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_NEW_WINDOW, &CPsqleditApp::OnNewWindow)
	ON_COMMAND(ID_GET_OBJECT_SOURCE_DETAIL_BAR, &CPsqleditApp::OnGetObjectSourceDetailBar)
	ON_UPDATE_COMMAND_UI(ID_GET_OBJECT_SOURCE_DETAIL_BAR, &CPsqleditApp::OnUpdateGetObjectSourceDetailBar)
	ON_COMMAND(ID_SHOW_CREDITS, &CPsqleditApp::OnShowCredits)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsqleditApp クラスの構築

CPsqleditApp::CPsqleditApp()
{
	g_sql_log_array = NULL;
	g_sql_logger = NULL;
	g_max_sql_log = 100;
	g_login_flg = FALSE;
	m_h_thread = NULL;

	g_ss = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// 唯一の CPsqleditApp オブジェクト

CPsqleditApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPsqleditApp クラスの初期化

BOOL CPsqleditApp::InitInstance()
{
	// 標準的な初期化処理
	// もしこれらの機能を使用せず、実行ファイルのサイズを小さく
	// したければ以下の特定の初期化ルーチンの中から不必要なもの
	// を削除してください。
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

	// 設定が保存される下のレジストリ キーを変更します。
	// TODO: この文字列を、会社名または所属など適切なものに
	// 変更してください。
	SetRegistry();

	// オプション情報のロード
	LoadOption();
	LoadFontOption();
	LoadDlgBarFontOption();

	LoadStdProfileSettings(g_option.max_mru);  // 標準の INI ファイルのオプションをロードします (MRU を含む)

	// フォント作成
	CreateFont();
	CreateDlgBarFont();

	// アプリケーション用のドキュメント テンプレートを登録します。ドキュメント テンプレート
	//  はドキュメント、フレーム ウィンドウとビューを結合するために機能します。

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_PSQLEDTYPE,
		RUNTIME_CLASS(CPsqleditDoc),
		RUNTIME_CLASS(CChildFrame), // カスタム MDI 子フレーム
		RUNTIME_CLASS(CSQLEditView));
	AddDocTemplate(pDocTemplate);

	// メイン MDI フレーム ウィンドウを作成
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// ファイルのドロップを受け取る
	m_pMainWnd->DragAcceptFiles();

	// DDE Execute open を使用可能にします。
	EnableShellOpen();

	// DDE、file open など標準のシェル コマンドのコマンドラインを解析します。
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	{
		// shortnameをlongnameに変換
		TCHAR	long_name[_MAX_PATH];
		if(GetLongPath(cmdInfo.m_strFileName, long_name) != FALSE) {
			cmdInfo.m_strFileName = long_name;
		}
	}

	// ログイン前に初期化できる処理を実行
	struct pre_login_thr_st	pre_st;
	pre_st.pMainFrame = pMainFrame;
	UINT thread_addr;
	UINT_PTR h_thread = _beginthreadex(NULL, 0, pre_login_thr, &pre_st, 0, &thread_addr);
	if(h_thread == NULL) {
		AfxMessageBox(_T("スレッド作成エラー"));
		return FALSE;
	}

	// ログイン
	if(pg_library_is_ok() == FALSE || Login() != 0) {
		g_login_flg = FALSE;
		// ステータスバーに，ログイン情報を表示
		SetSessionInfo(_T("未接続"));
	} else {
		g_login_flg = TRUE;
	}
	
	// スレッドの同期
	WaitForSingleObject((void *)h_thread, INFINITE);
	CloseHandle((void *)h_thread);

	// コマンドラインでディスパッチ コマンドを指定します。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// カレントディレクトリを設定
	SetCurrentDirectory(g_option.initial_dir);

	// childframeを最大表示する
	CFrameWnd *pf = pMainFrame->GetActiveFrame();
	if(pf != NULL && pf != pMainFrame) pf->ShowWindow(SW_SHOWMAXIMIZED);

	// メイン ウィンドウが初期化されたので、表示と更新を行います。
//	pMainFrame->ShowWindow(m_nCmdShow);
	if(GetProfileInt(_T("WINDOW"), _T("ZOOMED"), FALSE) != FALSE) {
		m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
	} else {
		m_pMainWnd->ShowWindow(SW_SHOW);
	}
	pMainFrame->UpdateWindow();

	if(g_login_flg) PostLogin();

	return TRUE;
}

// ダイアログを実行するためのアプリケーション コマンド
void CPsqleditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CPsqleditApp メッセージ ハンドラ


void CPsqleditApp::SetRegistry()
{
	SetRegistryKey(_T("OGAWA\\POSTGRESQL"));
}

void CPsqleditApp::LoadFontOption()
{
	g_option.font.face_name = GetProfileString(_T("FONT"), _T("FACENAME"), _T("Terminal"));
	g_option.font.point = GetProfileInt(_T("FONT"), _T("POINT"), 100);
	g_option.font.weight = GetProfileInt(_T("FONT"), _T("WEIGHT"), FW_NORMAL);
	g_option.font.italic = GetProfileInt(_T("FONT"), _T("ITALIC"), 0);
}

void CPsqleditApp::LoadDlgBarFontOption()
{
	g_option.dlg_bar_font.face_name = GetProfileString(_T("FONT"), _T("DLG_BAR_FACENAME"), g_option.font.face_name);
	g_option.dlg_bar_font.point = GetProfileInt(_T("FONT"), _T("DLG_BAR_POINT"), g_option.font.point);
	g_option.dlg_bar_font.weight = GetProfileInt(_T("FONT"), _T("DLG_BAR_WEIGHT"), FW_NORMAL);
	g_option.dlg_bar_font.italic = GetProfileInt(_T("FONT"), _T("DLG_BAR_ITALIC"), 0);
}

BOOL CPsqleditApp::CreateFont()
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

BOOL CPsqleditApp::CreateDlgBarFont()
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = g_option.dlg_bar_font.point;
	lf.lfWeight = g_option.dlg_bar_font.weight;
	lf.lfItalic = g_option.dlg_bar_font.italic;
	lf.lfCharSet = DEFAULT_CHARSET;
	_tcscpy(lf.lfFaceName, g_option.dlg_bar_font.face_name);

	return g_dlg_bar_font.CreatePointFontIndirect(&lf);
}

void CPsqleditApp::LoadOption()
{
	g_option.text_editor.tabstop = GetProfileInt(_T("TEXT_EDITOR"), _T("TABSTOP"), 4);
	g_option.text_editor.row_space = GetProfileInt(_T("TEXT_EDITOR"), _T("ROW_SPACE"), 0);
	g_option.text_editor.char_space = GetProfileInt(_T("TEXT_EDITOR"), _T("CHAR_SPACE"), 0);
	g_option.text_editor.top_space = GetProfileInt(_T("TEXT_EDITOR"), _T("TOP_SPACE"), 5);
	g_option.text_editor.left_space = GetProfileInt(_T("TEXT_EDITOR"), _T("LEFT_SPACE"), 5);
	g_option.text_editor.line_mode = GetProfileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), EC_LINE_MODE_NORMAL);
	g_option.text_editor.line_len = GetProfileInt(_T("TEXT_EDITOR"), _T("LINE_LEN"), 80);
	g_option.text_editor.show_line_feed = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_LINE_FEED"), FALSE);
	g_option.text_editor.show_tab = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_TAB"), FALSE);
	g_option.text_editor.show_row_num = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_NUM"), FALSE);
	g_option.text_editor.show_col_num = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_COL_NUM"), FALSE);
	g_option.text_editor.show_space = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_SPACE"), FALSE);
	g_option.text_editor.show_2byte_space = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_2BYTE_SPACE"), FALSE);
	g_option.text_editor.show_row_line = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_LINE"), FALSE);
	g_option.text_editor.show_edit_row = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_EDIT_ROW"), FALSE);
	g_option.text_editor.show_brackets_bold = GetProfileInt(_T("TEXT_EDITOR"), _T("SHOW_BRACKETS_BOLD"), TRUE);
	g_option.text_editor.table_name_completion = GetProfileInt(_T("TEXT_EDITOR"), _T("TABLE_NAME_COMPLETION"), TRUE);
	g_option.text_editor.column_name_completion = GetProfileInt(_T("TEXT_EDITOR"), _T("COLUMN_NAME_COMPLETION"), TRUE);
	g_option.text_editor.copy_lower_name = GetProfileInt(_T("TEXT_EDITOR"), _T("COPY_LOWER_NAME"), FALSE);
	g_option.text_editor.auto_indent = GetProfileInt(_T("TEXT_EDITOR"), _T("AUTO_INDENT"), FALSE);
	g_option.text_editor.tab_as_space = GetProfileInt(_T("TEXT_EDITOR"), _T("TAB_AS_SPACE"), FALSE);
	g_option.text_editor.drag_drop_edit = GetProfileInt(_T("TEXT_EDITOR"), _T("DRAG_DROP_EDIT"), FALSE);
	g_option.text_editor.row_copy_at_no_sel = GetProfileInt(_T("TEXT_EDITOR"), _T("ROW_COPY_AT_NO_SEL"), TRUE);
	g_option.text_editor.ime_caret_color = GetProfileInt(_T("TEXT_EDITOR"), _T("IME_CARET_COLOR"), TRUE);

	g_option.download.put_column_name = GetProfileInt(_T("DOWNLOAD"), _T("PUT_COLUMN_NAME"), 1);

	g_option.sql.ignore_sql_error = GetProfileInt(_T("SQL"), _T("IGNORE_SQL_ERROR"), 1);
	g_option.sql.refresh_table_list_after_ddl = GetProfileInt(_T("SQL"), _T("REFRESH_TABLE_LIST_AFTER_DDL"), 1);
	g_option.sql.adjust_col_width_after_select = GetProfileInt(_T("SQL"), _T("ADJUST_COL_WIDTH_AFTER_SELECT"), 0);
	g_option.sql.receive_async_notify = GetProfileInt(_T("SQL"), _T("RECEIVE_ASYNC_NOTIFY"), 1);
	g_option.sql.auto_commit = GetProfileInt(_T("SQL"), _T("AUTO_COMMIT"), 1);
	g_option.sql.sql_run_selected_area = GetProfileInt(_T("SQL"), _T("SQL_RUN_SELECTED_AREA"), FALSE);

	g_option.make_sql.select_sql_use_semicolon = GetProfileInt(_T("MAKE_SQL"), _T("SELECT_SQL_USE_SEMICOLON"), 1);
	g_option.make_sql.add_alias_name = GetProfileInt(_T("MAKE_SQL"), _T("ADD_ALIAS_NAME"), 0);
	g_option.make_sql.alias_name = GetProfileString(_T("MAKE_SQL"), _T("ALIAS_NAME"), _T(""));

	// FIXME: 設定画面を用意する
	g_option.sql_lint.comma_position_is_before_cname = GetProfileInt(_T("SQL_LINT"), _T("COMMA_POSITION_IS_BEFORE_CNAME"), FALSE);
	g_option.sql_lint.word_set_right = GetProfileInt(_T("SQL_LINT"), _T("WORD_SET_RIGHT"), FALSE);
	g_option.sql_lint.put_semicolon = GetProfileInt(_T("SQL_LINT"), _T("PUT_SEMICOLON"), TRUE);
	g_option.sql_lint.next_line_pos = GetProfileInt(_T("SQL_LINT"), _T("NEXT_LINE_POS"), 78);

	g_option.code_assist.use_keyword_window = GetProfileInt(_T("CODE_ASSIST"), _T("USE_KEYWORD_WINDOW"), 1);
	g_option.code_assist.enable_code_assist = GetProfileInt(_T("CODE_ASSIST"), _T("ENABLE_CODE_ASSIST"), 1);
	g_option.code_assist.sort_column_key = GetProfileInt(_T("CODE_ASSIST"), _T("SORT_COLUMN_KEY"), CODE_ASSIST_SORT_COLUMN_NAME);

	g_option.text_editor.color[TEXT_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("TEXT_COLOR"), RGB(0, 0, 0));
	g_option.text_editor.color[KEYWORD_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("KEYWORD_COLOR"), RGB(0, 0, 205));
	g_option.text_editor.color[KEYWORD2_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("KEYWORD2_COLOR"), RGB(0, 128, 192));
	g_option.text_editor.color[COMMENT_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("COMMENT_COLOR"), RGB(0, 120, 0));
	g_option.text_editor.color[BG_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("BG_COLOR"), RGB(255, 255, 255));
	g_option.text_editor.color[PEN_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("PEN_COLOR"), RGB(0, 150, 150));
	g_option.text_editor.color[QUOTE_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("QUOTE_COLOR"), RGB(220, 0, 0));
	g_option.text_editor.color[SEARCH_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("SEARCH_COLOR"), RGB(200, 200, 200));
	g_option.text_editor.color[SELECTED_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("SELECTED_COLOR"), RGB(0, 0, 50));
	g_option.text_editor.color[RULER_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("RULER_COLOR"), RGB(0, 100, 0));
	g_option.text_editor.color[OPERATOR_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("OPERATOR_COLOR"), RGB(128, 0, 0));
	g_option.text_editor.color[IME_CARET_COLOR] = GetProfileInt(_T("TEXT_EDITOR"), _T("IME_CARET_COLOR_RGB"), RGB(0x00, 0xa0, 0x00));

	g_option.text_editor.color[BRACKET_COLOR1] = GetProfileInt(_T("TEXT_EDITOR"), _T("BRACKET_COLOR1"), g_option.text_editor.color[OPERATOR_COLOR]);
	g_option.text_editor.color[BRACKET_COLOR2] = GetProfileInt(_T("TEXT_EDITOR"), _T("BRACKET_COLOR2"), g_option.text_editor.color[OPERATOR_COLOR]);
	g_option.text_editor.color[BRACKET_COLOR3] = GetProfileInt(_T("TEXT_EDITOR"), _T("BRACKET_COLOR3"), g_option.text_editor.color[OPERATOR_COLOR]);

	g_option.grid.show_space = GetProfileInt(_T("GRID"), _T("SHOW_SPACE"), FALSE);
	g_option.grid.show_2byte_space = GetProfileInt(_T("GRID"), _T("SHOW_2BYTE_SPACE"), FALSE);
	g_option.grid.show_line_feed = GetProfileInt(_T("GRID"), _T("SHOW_LINE_FEED"), FALSE);
	g_option.grid.show_tab = GetProfileInt(_T("GRID"), _T("SHOW_TAB"), FALSE);
	g_option.grid.col_header_dbl_clk_paste = GetProfileInt(_T("GRID"), _T("COL_HEADER_DBL_CLK_PASTE"), FALSE);
	g_option.grid.color[GRID_BG_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_BG_COLOR"), RGB(255, 255, 255));
	g_option.grid.color[GRID_HEADER_BG_COLOR] = GetProfileInt(_T("GRID"), _T("GRID_HEADER_BG_COLOR"), RGB(230, 230, 230));
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

	g_option.max_msg_row = GetProfileInt(_T("APPLICATION"), _T("MAX_MSG_ROW"), 1000);
	g_option.save_modified = GetProfileInt(_T("APPLICATION"), _T("SAVE_MODIFIED"), TRUE);
	g_option.sql_logging = GetProfileInt(_T("APPLICATION"), _T("SQL_LOGGING"), 0);
	g_option.initial_dir = GetProfileString(_T("APPLICATION"), _T("INITIAL_DIR"), _T(""));
	if(g_option.initial_dir == _T("")) {
		g_option.initial_dir = GetAppPath();
	}
	g_option.sql_log_dir = GetProfileString(_T("APPLICATION"), _T("SQL_LOG_DIR"), _T(""));
	if(g_option.sql_log_dir == _T("")) {
		g_option.sql_log_dir = GetAppPath() + _T("log\\");
	}
	g_option.sql_lib_dir = GetProfileString(_T("APPLICATION"), _T("SQL_LIB_DIR"), _T(""));
	if(g_option.sql_lib_dir == _T("")) {
		g_option.sql_lib_dir = GetAppPath() + _T("sql_lib\\");
	}

	g_option.tab_title_is_path_name = GetProfileInt(_T("APPLICATION"), _T("TAB_TITLE_IS_PATH_NAME"), FALSE);
	g_option.tab_close_at_mclick = GetProfileInt(_T("APPLICATION"), _T("TAB_CLOSE_AT_MCLICK"), FALSE);
	g_option.tab_create_at_dblclick = GetProfileInt(_T("APPLICATION"), _T("TAB_CREATE_AT_DBLCLICK"), FALSE);
	g_option.tab_drag_move = GetProfileInt(_T("APPLICATION"), _T("TAB_DRAG_MOVE"), TRUE);
	g_option.close_btn_on_tab = GetProfileInt(_T("APPLICATION"), _T("CLOSE_BTN_ON_TAB"), TRUE);
	g_option.show_tab_tooltip = GetProfileInt(_T("APPLICATION"), _T("SHOW_TAB_TOOLTIP"), TRUE);

	g_option.search_loop_msg = GetProfileInt(_T("APPLICATION"), _T("SEARCH_LOOP_MSG"), FALSE);
	g_option.max_mru = GetProfileInt(_T("APPLICATION"), _T("MAX_MRU"), 4);

	g_option.init_object_list_type = GetProfileString(_T("APPLICATION"), _T("INIT_OBJECT_LIST_TYPE"), _T("TABLE"));
	g_option.init_table_list_user = GetProfileInt(_T("APPLICATION"), _T("INIT_TABLE_LIST_USER"), 0);
	g_option.object_list_filter_column = GetProfileInt(_T("APPLICATION"), _T("OBJECT_LIST_FILTER_COLUMN"), OFC_FIRST_AND_COMMENT_COLUMN);
	g_option.sql_stmt_str = GetProfileString(_T("APPLICATION"), _T("SQL_STMT_STR"), _T("sql_stmt"));

	for(int i = 0; i < COT_COUNT; i++) {
		CString name;
		name.Format(_T("COT_%d"), i);
		g_option.completion_object_type[i] = GetProfileInt(_T("COT"), name.GetBuffer(0), TRUE);
	}

	g_object_bar.LoadColumnWidth();
	g_object_detail_bar.LoadColumnWidth();
}

void CPsqleditApp::Logout()
{
	WaitPostLoginThr();

	if(g_ss == NULL) return;

	pg_logout(g_ss);

	g_ss = NULL;
	g_login_flg = FALSE;

	// オブジェクトデータをクリア
	g_object_bar.ClearData();
	g_object_detail_bar.ClearColumnList();

	SetSessionInfo(_T("未接続"));
}

static void my_notice_processor(void *arg, const char *message)
{
	TCHAR buf[1024];
	oci_str_to_win_str((const unsigned char *)message, buf, ARRAY_SIZEOF(buf));

	TRACE(buf);

	CMainFrame	*pMainFrame = (CMainFrame *)AfxGetApp()->m_pMainWnd;
	if(pMainFrame == NULL || pMainFrame->GetActiveFrame() == NULL) return;

	CDocument *pdoc = ::GetActiveDocument();
	if(pdoc == NULL) return;

	pdoc->UpdateAllViews(NULL, UPD_SET_RESULT_CARET, 0);
	pdoc->UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)buf);
}

int CPsqleditApp::Login()
{
	CLoginDlg	dlg;

	dlg.m_title = _T("PSqlEdit");

	dlg.m_user = GetProfileString(_T("CONNECT_INFO"), _T("USER-CUR"), _T(""));
	dlg.m_host = GetProfileString(_T("CONNECT_INFO"), _T("HOST-CUR"), _T(""));
	dlg.m_port = GetProfileString(_T("CONNECT_INFO"), _T("PORT-CUR"), _T(""));
	dlg.m_dbname = GetProfileString(_T("CONNECT_INFO"), _T("DBNAME-CUR"), _T(""));

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

	// ステータスバーに，ログイン情報を表示
	CString session_info;
	session_info.Format(_T("%s@%s:%s.%s"), pg_user(g_ss), pg_host(g_ss), pg_port(g_ss), pg_db(g_ss));
	if(pg_is_ssl_mode(g_ss)) session_info += _T("(SSL)");

	if(!dlg.m_connect_name.IsEmpty()) {
		session_info = dlg.m_connect_name + _T(" (") + session_info + _T(")");
	}

	SetSessionInfo(session_info.GetBuffer(0));

	pg_set_notice_processor(g_ss, my_notice_processor, NULL);

	return 0;
}

int CPsqleditApp::PostLogin()
{
	if(g_login_flg == FALSE) return 0;

	CWaitCursor		wait_cursor;
	UINT			thread_addr;
	int ret_v;

	// オブジェクトバーを初期化
	ret_v = g_object_bar.InitializeList(pg_user(g_ss));
	if(ret_v != 0) return 1;

	m_h_thread = _beginthreadex(NULL, 0, post_login_thr, NULL, 0, &thread_addr);
	if(m_h_thread == NULL) {
		_stprintf(g_msg_buf, _T("スレッド作成エラー"));
		return 1;
	}

	CSqlListMaker::ClearCache();

	PrintConnectInfo();

	return 0;
}

int CPsqleditApp::ExitInstance() 
{
	g_show_clob_dlg.Clear();

	if(g_sql_log_array != NULL) delete g_sql_log_array;
	if(g_sql_logger != NULL) delete g_sql_logger;

	Logout();

	// pg_free_libraryの後に，デストラクタでpg_free_datasetを実行すると一般保護違反になるのを回避
	g_object_bar.FreeObjectList();
	g_object_detail_bar.FreeObjectList();
	g_sql_str_token.FreeDataset();
	CSqlListMaker::ClearCache();
	
	pg_free_library(NULL);

	WSACleanup();

	return CWinApp::ExitInstance();
}

void CPsqleditApp::SaveOption()
{
	WriteProfileInt(_T("TEXT_EDITOR"), _T("TABSTOP"), g_option.text_editor.tabstop);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("ROW_SPACE"), g_option.text_editor.row_space);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("CHAR_SPACE"), g_option.text_editor.char_space);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("TOP_SPACE"), g_option.text_editor.top_space);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("LEFT_SPACE"), g_option.text_editor.left_space);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("LINE_MODE"), g_option.text_editor.line_mode);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("LINE_LEN"), g_option.text_editor.line_len);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_LINE_FEED"), g_option.text_editor.show_line_feed);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_TAB"), g_option.text_editor.show_tab);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_NUM"), g_option.text_editor.show_row_num);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_COL_NUM"), g_option.text_editor.show_col_num);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_SPACE"), g_option.text_editor.show_space);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_2BYTE_SPACE"), g_option.text_editor.show_2byte_space);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_ROW_LINE"), g_option.text_editor.show_row_line);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_EDIT_ROW"), g_option.text_editor.show_edit_row);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SHOW_BRACKETS_BOLD"), g_option.text_editor.show_brackets_bold);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("TABLE_NAME_COMPLETION"), g_option.text_editor.table_name_completion);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("COLUMN_NAME_COMPLETION"), g_option.text_editor.column_name_completion);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("COPY_LOWER_NAME"), g_option.text_editor.copy_lower_name);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("AUTO_INDENT"), g_option.text_editor.auto_indent);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("TAB_AS_SPACE"), g_option.text_editor.tab_as_space);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("DRAG_DROP_EDIT"), g_option.text_editor.drag_drop_edit);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("ROW_COPY_AT_NO_SEL"), g_option.text_editor.row_copy_at_no_sel);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("IME_CARET_COLOR"), g_option.text_editor.ime_caret_color);

	WriteProfileInt(_T("TEXT_EDITOR"), _T("BRACKET_COLOR1"), g_option.text_editor.color[BRACKET_COLOR1]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("BRACKET_COLOR2"), g_option.text_editor.color[BRACKET_COLOR2]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("BRACKET_COLOR3"), g_option.text_editor.color[BRACKET_COLOR3]);

	WriteProfileInt(_T("DOWNLOAD"), _T("PUT_COLUMN_NAME"), g_option.download.put_column_name);

	WriteProfileInt(_T("SQL"), _T("IGNORE_SQL_ERROR"), g_option.sql.ignore_sql_error);
	WriteProfileInt(_T("SQL"), _T("REFRESH_TABLE_LIST_AFTER_DDL"), g_option.sql.refresh_table_list_after_ddl);
	WriteProfileInt(_T("SQL"), _T("ADJUST_COL_WIDTH_AFTER_SELECT"), g_option.sql.adjust_col_width_after_select);
	WriteProfileInt(_T("SQL"), _T("RECEIVE_ASYNC_NOTIFY"), g_option.sql.receive_async_notify);
	WriteProfileInt(_T("SQL"), _T("AUTO_COMMIT"), g_option.sql.auto_commit);
	WriteProfileInt(_T("SQL"), _T("SQL_RUN_SELECTED_AREA"), g_option.sql.sql_run_selected_area);

	WriteProfileInt(_T("MAKE_SQL"), _T("SELECT_SQL_USE_SEMICOLON"), g_option.make_sql.select_sql_use_semicolon);
	WriteProfileInt(_T("MAKE_SQL"), _T("ADD_ALIAS_NAME"), g_option.make_sql.add_alias_name);
	WriteProfileString(_T("MAKE_SQL"), _T("ALIAS_NAME"), g_option.make_sql.alias_name);

	WriteProfileInt(_T("CODE_ASSIST"), _T("USE_KEYWORD_WINDOW"), g_option.code_assist.use_keyword_window);
	WriteProfileInt(_T("CODE_ASSIST"), _T("ENABLE_CODE_ASSIST"), g_option.code_assist.enable_code_assist);
	WriteProfileInt(_T("CODE_ASSIST"), _T("SORT_COLUMN_KEY"), g_option.code_assist.sort_column_key);

	WriteProfileInt(_T("TEXT_EDITOR"), _T("TEXT_COLOR"), g_option.text_editor.color[TEXT_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("KEYWORD_COLOR"), g_option.text_editor.color[KEYWORD_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("KEYWORD2_COLOR"), g_option.text_editor.color[KEYWORD2_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("COMMENT_COLOR"), g_option.text_editor.color[COMMENT_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("BG_COLOR"), g_option.text_editor.color[BG_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("PEN_COLOR"), g_option.text_editor.color[PEN_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("QUOTE_COLOR"), g_option.text_editor.color[QUOTE_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SEARCH_COLOR"), g_option.text_editor.color[SEARCH_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("SELECTED_COLOR"), g_option.text_editor.color[SELECTED_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("RULER_COLOR"), g_option.text_editor.color[RULER_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("OPERATOR_COLOR"), g_option.text_editor.color[OPERATOR_COLOR]);
	WriteProfileInt(_T("TEXT_EDITOR"), _T("IME_CARET_COLOR_RGB"), g_option.text_editor.color[IME_CARET_COLOR]);

	WriteProfileInt(_T("GRID"), _T("SHOW_SPACE"), g_option.grid.show_space);
	WriteProfileInt(_T("GRID"), _T("SHOW_2BYTE_SPACE"), g_option.grid.show_2byte_space);
	WriteProfileInt(_T("GRID"), _T("SHOW_LINE_FEED"), g_option.grid.show_line_feed);
	WriteProfileInt(_T("GRID"), _T("SHOW_TAB"), g_option.grid.show_tab);
	WriteProfileInt(_T("GRID"), _T("COL_HEADER_DBL_CLK_PASTE"), g_option.grid.col_header_dbl_clk_paste);
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
	WriteProfileInt(_T("GRID"), _T("GRID_SEARCH_COLOR"), 	g_option.grid.color[GRID_SEARCH_COLOR]);
	WriteProfileInt(_T("GRID"), _T("GRID_HEADER_LINE_COLOR"), g_option.grid.color[GRID_HEADER_LINE_COLOR]);

	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_TOP"), g_option.grid.cell_padding_top);
	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_BOTTOM"), g_option.grid.cell_padding_bottom);
	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_LEFT"), g_option.grid.cell_padding_left);
	WriteProfileInt(_T("GRID"), _T("CELL_PADDING_RIGHT"), g_option.grid.cell_padding_right);

	WriteProfileInt(_T("APPLICATION"), _T("MAX_MSG_ROW"), g_option.max_msg_row);
	WriteProfileInt(_T("APPLICATION"), _T("SAVE_MODIFIED"), g_option.save_modified);
	WriteProfileInt(_T("APPLICATION"), _T("SQL_LOGGING"), g_option.sql_logging);
	WriteProfileString(_T("APPLICATION"), _T("INITIAL_DIR"), g_option.initial_dir);
	WriteProfileString(_T("APPLICATION"), _T("SQL_LOG_DIR"), g_option.sql_log_dir);
	WriteProfileString(_T("APPLICATION"), _T("SQL_LIB_DIR"), g_option.sql_lib_dir);

	WriteProfileInt(_T("APPLICATION"), _T("TAB_TITLE_IS_PATH_NAME"), g_option.tab_title_is_path_name);
	WriteProfileInt(_T("APPLICATION"), _T("TAB_CLOSE_AT_MCLICK"), g_option.tab_close_at_mclick);
	WriteProfileInt(_T("APPLICATION"), _T("TAB_CREATE_AT_DBLCLICK"), g_option.tab_create_at_dblclick);
	WriteProfileInt(_T("APPLICATION"), _T("TAB_DRAG_MOVE"), g_option.tab_drag_move);
	WriteProfileInt(_T("APPLICATION"), _T("CLOSE_BTN_ON_TAB"), g_option.close_btn_on_tab);
	WriteProfileInt(_T("APPLICATION"), _T("SHOW_TAB_TOOLTIP"), g_option.show_tab_tooltip);
	WriteProfileInt(_T("APPLICATION"), _T("MAX_MRU"), g_option.max_mru);
	WriteProfileInt(_T("APPLICATION"), _T("SEARCH_LOOP_MSG"), g_option.search_loop_msg);
	WriteProfileString(_T("APPLICATION"), _T("INIT_OBJECT_LIST_TYPE"), g_option.init_object_list_type);
	WriteProfileInt(_T("APPLICATION"), _T("INIT_TABLE_LIST_USER"), g_option.init_table_list_user);
	WriteProfileInt(_T("APPLICATION"), _T("OBJECT_LIST_FILTER_COLUMN"), g_option.object_list_filter_column);
	WriteProfileString(_T("APPLICATION"), _T("SQL_STMT_STR"), g_option.sql_stmt_str);

	for(int i = 0; i < COT_COUNT; i++) {
		CString name;
		name.Format(_T("COT_%d"), i);
		WriteProfileInt(_T("COT"), name.GetBuffer(0), g_option.completion_object_type[i]);
	}

	g_object_bar.SaveColumnWidth();
	g_object_detail_bar.SaveColumnWidth();
}

void CPsqleditApp::SaveFontOption()
{
	WriteProfileString(_T("FONT"), _T("FACENAME"), g_option.font.face_name);
	WriteProfileInt(_T("FONT"), _T("POINT"), g_option.font.point);
	WriteProfileInt(_T("FONT"), _T("WEIGHT"), g_option.font.weight);
	WriteProfileInt(_T("FONT"), _T("ITALIC"), g_option.font.italic);
}

void CPsqleditApp::SaveDlgBarFontOption()
{
	WriteProfileString(_T("FONT"), _T("DLG_BAR_FACENAME"), g_option.dlg_bar_font.face_name);
	WriteProfileInt(_T("FONT"), _T("DLG_BAR_POINT"), g_option.dlg_bar_font.point);
	WriteProfileInt(_T("FONT"), _T("DLG_BAR_WEIGHT"), g_option.dlg_bar_font.weight);
	WriteProfileInt(_T("FONT"), _T("DLG_BAR_ITALIC"), g_option.dlg_bar_font.italic);
}

void CPsqleditApp::OnOption() 
{
	COptionSheet dlg(_T("オプション"), AfxGetMainWnd());
	int		i;
	BOOL	b_register_shell = CheckRegisterShellFileTypes();

	dlg.m_func_page.m_download_column_name = g_option.download.put_column_name;
	dlg.m_func_page.m_ignore_sql_error = g_option.sql.ignore_sql_error;
	dlg.m_func_page.m_refresh_table_list_after_ddl = g_option.sql.refresh_table_list_after_ddl;
	dlg.m_func_page.m_adjust_col_width_after_select = g_option.sql.adjust_col_width_after_select;
	dlg.m_func_page.m_receive_async_notify = g_option.sql.receive_async_notify;
	dlg.m_func_page.m_auto_commit = g_option.sql.auto_commit;
	dlg.m_func_page.m_sql_run_selected_area = g_option.sql.sql_run_selected_area;

	dlg.m_func_page.m_make_select_sql_use_semicolon = g_option.make_sql.select_sql_use_semicolon;

	dlg.m_func_page.m_sql_logging = g_option.sql_logging;
	dlg.m_func_page.m_sql_log_dir = g_option.sql_log_dir;

	dlg.m_func_page.m_make_select_add_alias_name = g_option.make_sql.add_alias_name;
	dlg.m_func_page.m_make_select_alias_name = g_option.make_sql.alias_name;

	dlg.m_setup_page.m_max_msg_row = g_option.max_msg_row;
	dlg.m_setup_page.m_save_modified = g_option.save_modified;
	dlg.m_setup_page.m_initial_dir = g_option.initial_dir;
	dlg.m_setup_page.m_sql_lib_dir = g_option.sql_lib_dir;
	dlg.m_setup_page.m_register_shell = b_register_shell;
	dlg.m_setup_page.m_tab_title_is_path_name = g_option.tab_title_is_path_name;
	dlg.m_setup_page.m_tab_close_at_mclick = g_option.tab_close_at_mclick;
	dlg.m_setup_page.m_tab_create_at_dblclick = g_option.tab_create_at_dblclick;
	dlg.m_setup_page.m_tab_drag_move = g_option.tab_drag_move;
	dlg.m_setup_page.m_close_btn_on_tab = g_option.close_btn_on_tab;
	dlg.m_setup_page.m_show_tab_tooltip = g_option.show_tab_tooltip;
	dlg.m_setup_page.m_search_loop_msg = g_option.search_loop_msg;
	dlg.m_setup_page.m_max_mru = g_option.max_mru;
	dlg.m_setup_page.m_init_object_list_type_str = g_option.init_object_list_type;
	dlg.m_setup_page.m_init_table_list_user = g_option.init_table_list_user;
	dlg.m_setup_page.m_sql_stmt_str = g_option.sql_stmt_str;

	dlg.m_editor_page.m_tabstop = g_option.text_editor.tabstop;
	dlg.m_editor_page.m_row_space = g_option.text_editor.row_space;
	dlg.m_editor_page.m_char_space = g_option.text_editor.char_space;
	dlg.m_editor_page.m_top_space = g_option.text_editor.top_space;
	dlg.m_editor_page.m_left_space = g_option.text_editor.left_space;
	dlg.m_editor_page.m_line_len = g_option.text_editor.line_len;
	dlg.m_editor_page.m_show_line_feed = g_option.text_editor.show_line_feed;
	dlg.m_editor_page.m_show_tab = g_option.text_editor.show_tab;
	dlg.m_editor_page.m_show_row_num = g_option.text_editor.show_row_num;
	dlg.m_editor_page.m_show_col_num = g_option.text_editor.show_col_num;
	dlg.m_editor_page.m_show_space = g_option.text_editor.show_space;
	dlg.m_editor_page.m_show_2byte_space = g_option.text_editor.show_2byte_space;
	dlg.m_editor_page.m_show_row_line = g_option.text_editor.show_row_line;
	dlg.m_editor_page.m_show_edit_row = g_option.text_editor.show_edit_row;
	dlg.m_editor_page.m_show_brackets_bold = g_option.text_editor.show_brackets_bold;
	dlg.m_editor_page.m_ime_caret_color = g_option.text_editor.ime_caret_color;
	dlg.m_func_page2.m_use_keyword_window = g_option.code_assist.use_keyword_window;
	dlg.m_func_page2.m_enable_code_assist = g_option.code_assist.enable_code_assist;
	dlg.m_func_page2.m_code_assist_sort_column = g_option.code_assist.sort_column_key;
	dlg.m_func_page2.m_table_name_completion = g_option.text_editor.table_name_completion;
	dlg.m_func_page2.m_column_name_completion = g_option.text_editor.column_name_completion;
	dlg.m_func_page2.m_object_list_filter_column = g_option.object_list_filter_column;
	dlg.m_editor_page2.m_copy_lower_name = g_option.text_editor.copy_lower_name;
	dlg.m_editor_page2.m_auto_indent = g_option.text_editor.auto_indent;
	dlg.m_editor_page2.m_tab_as_space = g_option.text_editor.tab_as_space;
	dlg.m_editor_page2.m_drag_drop_edit = g_option.text_editor.drag_drop_edit;
	dlg.m_editor_page2.m_row_copy_at_no_sel = g_option.text_editor.row_copy_at_no_sel;


	for(i = 0; i < COT_COUNT; i++) {
		dlg.m_func_page2.m_completion_object_type[i] = g_option.completion_object_type[i];
	}
	for(i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		dlg.m_editor_page.m_color[i] = g_option.text_editor.color[i];
	}

	dlg.m_grid_page.m_show_space = g_option.grid.show_space;
	dlg.m_grid_page.m_show_2byte_space = g_option.grid.show_2byte_space;
	dlg.m_grid_page.m_show_line_feed = g_option.grid.show_line_feed;
	dlg.m_grid_page.m_show_tab = g_option.grid.show_tab;
	dlg.m_grid_page.m_col_header_dbl_clk_paste = g_option.grid.col_header_dbl_clk_paste;
	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		dlg.m_grid_page.m_color[i] = g_option.grid.color[i];
	}

	dlg.m_grid_page.m_cell_padding_top = g_option.grid.cell_padding_top;
	dlg.m_grid_page.m_cell_padding_bottom = g_option.grid.cell_padding_bottom;
	dlg.m_grid_page.m_cell_padding_left = g_option.grid.cell_padding_left;
	dlg.m_grid_page.m_cell_padding_right = g_option.grid.cell_padding_right;

	if(dlg.DoModal() != IDOK) return;

	g_option.download.put_column_name = dlg.m_func_page.m_download_column_name;
	g_option.sql.ignore_sql_error = dlg.m_func_page.m_ignore_sql_error;
	g_option.sql.refresh_table_list_after_ddl = dlg.m_func_page.m_refresh_table_list_after_ddl;
	g_option.sql.adjust_col_width_after_select = dlg.m_func_page.m_adjust_col_width_after_select;
	g_option.sql.receive_async_notify = dlg.m_func_page.m_receive_async_notify;
	g_option.sql.auto_commit = dlg.m_func_page.m_auto_commit;
	g_option.sql.sql_run_selected_area = dlg.m_func_page.m_sql_run_selected_area;

	g_option.make_sql.select_sql_use_semicolon = dlg.m_func_page.m_make_select_sql_use_semicolon;

	g_option.code_assist.use_keyword_window = dlg.m_func_page2.m_use_keyword_window;
	g_option.code_assist.enable_code_assist = dlg.m_func_page2.m_enable_code_assist;
	g_option.code_assist.sort_column_key = dlg.m_func_page2.m_code_assist_sort_column;

	g_option.sql_logging = dlg.m_func_page.m_sql_logging;
	g_option.sql_log_dir = dlg.m_func_page.m_sql_log_dir;

	g_option.make_sql.add_alias_name = dlg.m_func_page.m_make_select_add_alias_name;
	g_option.make_sql.alias_name = dlg.m_func_page.m_make_select_alias_name;

	g_sql_logger->SetLogDir(g_option.sql_log_dir.GetBuffer(0));

	g_option.max_msg_row = dlg.m_setup_page.m_max_msg_row;
	g_option.save_modified = dlg.m_setup_page.m_save_modified;
	if(g_option.initial_dir.Compare(dlg.m_setup_page.m_initial_dir) != 0) {
		g_option.initial_dir = dlg.m_setup_page.m_initial_dir;
		SetCurrentDirectory(g_option.initial_dir);
	}
	g_option.sql_lib_dir = dlg.m_setup_page.m_sql_lib_dir;

	if(dlg.m_setup_page.m_tab_title_is_path_name != g_option.tab_title_is_path_name) {
		g_option.tab_title_is_path_name = dlg.m_setup_page.m_tab_title_is_path_name;
		g_tab_bar.SetTabNameAll();
	}
	g_option.tab_close_at_mclick = dlg.m_setup_page.m_tab_close_at_mclick;
	g_option.tab_create_at_dblclick = dlg.m_setup_page.m_tab_create_at_dblclick;
	g_option.tab_drag_move = dlg.m_setup_page.m_tab_drag_move;
	g_option.close_btn_on_tab = dlg.m_setup_page.m_close_btn_on_tab;
	g_option.show_tab_tooltip = dlg.m_setup_page.m_show_tab_tooltip;
	g_option.max_mru = dlg.m_setup_page.m_max_mru;
	g_option.search_loop_msg = dlg.m_setup_page.m_search_loop_msg;

	if(b_register_shell != dlg.m_setup_page.m_register_shell) {
		if(dlg.m_setup_page.m_register_shell == TRUE) {
			UnRegisterShellFileTypes();
			RegisterShellFileTypes(FALSE);
		} else {
			UnRegisterShellFileTypes();
		}
	}

	g_option.text_editor.tabstop = dlg.m_editor_page.m_tabstop;
	g_option.text_editor.row_space = dlg.m_editor_page.m_row_space;
	g_option.text_editor.char_space = dlg.m_editor_page.m_char_space;
	g_option.text_editor.top_space = dlg.m_editor_page.m_top_space;
	g_option.text_editor.left_space = dlg.m_editor_page.m_left_space;
	if(g_option.text_editor.line_len != dlg.m_editor_page.m_line_len) {
		g_option.text_editor.line_len = dlg.m_editor_page.m_line_len;
		UpdateAllDocViews(NULL, UPD_CHANGE_LINE_MODE, 0);
	}
	g_option.text_editor.show_line_feed = dlg.m_editor_page.m_show_line_feed;
	g_option.text_editor.show_tab = dlg.m_editor_page.m_show_tab;
	g_option.text_editor.show_row_num = dlg.m_editor_page.m_show_row_num;
	g_option.text_editor.show_col_num = dlg.m_editor_page.m_show_col_num;
	g_option.text_editor.show_row_line = dlg.m_editor_page.m_show_row_line;
	g_option.text_editor.show_edit_row = dlg.m_editor_page.m_show_edit_row;
	g_option.text_editor.show_brackets_bold = dlg.m_editor_page.m_show_brackets_bold;
	g_option.text_editor.ime_caret_color = dlg.m_editor_page.m_ime_caret_color;
	g_option.text_editor.show_space = dlg.m_editor_page.m_show_space;
	g_option.text_editor.show_2byte_space = dlg.m_editor_page.m_show_2byte_space;

	BOOL check_completion_object_type = FALSE;
	for(i = 0; i < COT_COUNT; i++) {
		if(g_option.completion_object_type[i] != dlg.m_func_page2.m_completion_object_type[i]) {
			check_completion_object_type = TRUE;
			break;
		}
	}

	if(check_completion_object_type ||
		g_option.text_editor.table_name_completion != dlg.m_func_page2.m_table_name_completion ||
		g_option.text_editor.column_name_completion != dlg.m_func_page2.m_column_name_completion) {

		g_option.text_editor.table_name_completion = dlg.m_func_page2.m_table_name_completion;
		g_option.text_editor.column_name_completion = dlg.m_func_page2.m_column_name_completion;
		for(i = 0; i < COT_COUNT; i++) {
			g_option.completion_object_type[i] = dlg.m_func_page2.m_completion_object_type[i];
		}

		if(g_ss != NULL) {
			// キーワード補完用のデータを初期化
			CWaitCursor		wait_cursor;
			g_sql_str_token.initCompletionWord(g_ss,
				g_object_bar.GetSelectedUser().GetBuffer(0), g_msg_buf,
				dlg.m_func_page2.m_table_name_completion,
				dlg.m_func_page2.m_column_name_completion,
				make_completion_object_type());

			CSqlListMaker::ClearCache();
		}
	}
	g_option.object_list_filter_column = dlg.m_func_page2.m_object_list_filter_column;

	g_option.text_editor.copy_lower_name = dlg.m_editor_page2.m_copy_lower_name;
	g_option.text_editor.auto_indent = dlg.m_editor_page2.m_auto_indent;
	g_option.text_editor.tab_as_space = dlg.m_editor_page2.m_tab_as_space;
	g_option.text_editor.drag_drop_edit = dlg.m_editor_page2.m_drag_drop_edit;
	g_option.text_editor.row_copy_at_no_sel = dlg.m_editor_page2.m_row_copy_at_no_sel;
	g_option.init_object_list_type = dlg.m_setup_page.m_init_object_list_type_str;
	g_option.init_table_list_user = dlg.m_setup_page.m_init_table_list_user;
	g_option.sql_stmt_str = dlg.m_setup_page.m_sql_stmt_str;

	for(i = 0; i < EDIT_CTRL_COLOR_CNT; i++) {
		g_option.text_editor.color[i] = dlg.m_editor_page.m_color[i];
	}

	g_option.grid.show_space = dlg.m_grid_page.m_show_space;
	g_option.grid.show_2byte_space = dlg.m_grid_page.m_show_2byte_space;
	g_option.grid.show_line_feed = dlg.m_grid_page.m_show_line_feed;
	g_option.grid.show_tab = dlg.m_grid_page.m_show_tab;
	g_option.grid.col_header_dbl_clk_paste = dlg.m_grid_page.m_col_header_dbl_clk_paste;
	for(i = 0; i < GRID_CTRL_COLOR_CNT; i++) {
		g_option.grid.color[i] = dlg.m_grid_page.m_color[i];
	}

	g_option.grid.cell_padding_top = dlg.m_grid_page.m_cell_padding_top;
	g_option.grid.cell_padding_bottom = dlg.m_grid_page.m_cell_padding_bottom;
	g_option.grid.cell_padding_left = dlg.m_grid_page.m_cell_padding_left;
	g_option.grid.cell_padding_right = dlg.m_grid_page.m_cell_padding_right;

	UpdateAllDocViews(NULL, UPD_CHANGE_EDITOR_OPTION, 0);

	g_tab_bar.SetOption();

	SaveOption();
}

void CPsqleditApp::UnRegisterShellFileTypes()
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

BOOL CPsqleditApp::CheckRegisterShellFileTypes()
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

void CPsqleditApp::UpdateAllDocViews(CView* pSender, LPARAM lHint, CObject* pHint)
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

void CPsqleditApp::SetLineMode(int line_mode)
{
	g_option.text_editor.line_mode = line_mode;
	UpdateAllDocViews(NULL, UPD_CHANGE_LINE_MODE, 0);
}

void CPsqleditApp::OnLineModeLen() 
{
	SetLineMode(EC_LINE_MODE_LEN);
}

void CPsqleditApp::OnLineModeNormal() 
{
	SetLineMode(EC_LINE_MODE_NORMAL);
}

void CPsqleditApp::OnLineModeRight() 
{
	SetLineMode(EC_LINE_MODE_RIGHT);
}

void CPsqleditApp::CheckLineMode(CCmdUI *pCmdUI, int line_mode)
{
	if(g_option.text_editor.line_mode == line_mode) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CPsqleditApp::OnUpdateLineModeLen(CCmdUI* pCmdUI) 
{
	CheckLineMode(pCmdUI, EC_LINE_MODE_LEN);
}

void CPsqleditApp::OnUpdateLineModeNormal(CCmdUI* pCmdUI) 
{
	CheckLineMode(pCmdUI, EC_LINE_MODE_NORMAL);
}

void CPsqleditApp::OnUpdateLineModeRight(CCmdUI* pCmdUI) 
{
	CheckLineMode(pCmdUI, EC_LINE_MODE_RIGHT);
}

void CPsqleditApp::OnLogfileOpen() 
{
	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(sizeof(cur_dir)/sizeof(cur_dir[0]), cur_dir);

	CString newName;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  OFN_FILEMUSTEXIST, TRUE, NULL, g_option.sql_log_dir.GetBuffer(0)))
		return; // open cancelled

	// 新しいドキュメントを作成
//	OpenDocumentFile(newName);
	// OpenDocumentFileでは，最近使ったファイルリストに入ってしまう
   	CPsqleditDoc *pDoc = (CPsqleditDoc *)CreateNewDocument(_T(""));
	pDoc->OnOpenDocument(newName);
	pDoc->SetPathName(newName);

	// ログファイルは実行不可にする
	CPsqleditDoc *pdoc = (CPsqleditDoc *)::GetActiveDocument();	
	pdoc->SetNoSqlRun();

	SetCurrentDirectory(cur_dir);
}

static void AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	ASSERT_VALID(pTemplate);
	ASSERT_KINDOF(CDocTemplate, pTemplate);

	CString strFilterExt, strFilterName;
	if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
	 !strFilterExt.IsEmpty() &&
	 pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
	 !strFilterName.IsEmpty())
	{
		// a file based document template - add to filter list
		ASSERT(strFilterExt[0] == '.');
		if (pstrDefaultExt != NULL)
		{
			// set the default extension
			*pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;  // skip the '.'
			ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
			ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
		}

		// add to filter
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());  // must have a file type name
		filter += (TCHAR)'\0';  // next string please
		filter += (TCHAR)'*';
		filter += strFilterExt;
		filter += (TCHAR)'\0';  // next string please
		ofn.nMaxCustFilter++;
	}
}

BOOL CPsqleditApp::DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate,
			TCHAR *dir)
{
	TCHAR	cur_dir[_MAX_PATH];
	GetCurrentDirectory(sizeof(cur_dir)/sizeof(cur_dir[0]), cur_dir);
	if(dir == NULL) dir = cur_dir;

	CFileDialog dlgFile(bOpenFileDialog);

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags;
	dlgFile.m_ofn.lpstrInitialDir = dir;

	CString strFilter;
	CString strDefault;

	if(pTemplate != NULL) {
		AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
	} else {
		// do for all doc template
		POSITION pos = 	m_pDocManager->GetFirstDocTemplatePosition();
		BOOL bFirst = TRUE;
		while (pos != NULL)
		{
			CDocTemplate* pTmpl = m_pDocManager->GetNextDocTemplate(pos);
			AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTmpl,
				bFirst ? &strDefault : NULL);
			bFirst = FALSE;
		}
	}

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';   // next string please
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';   // last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();

	if(bResult) {
		// ファイル選択した場合、カレントディレクトリをファイルのディレクトリにする
		// (次回、ファイル選択ダイアログを表示したときに、前回選択した場所から開けるようにする)
		TCHAR	cur_dir2[_MAX_PATH];
		_tcscpy(cur_dir2, fileName.GetBuffer(0));
		make_parent_dirname(cur_dir2);
		SetCurrentDirectory(cur_dir2);
	}
	return bResult;
}

CDocument *CPsqleditApp::CreateNewDocument(TCHAR *title)
{
	CWaitCursor			wait_cursor;
	int			ret_v = 0;

	// 新しいドキュメントを作成
	POSITION	doc_tmpl_pos = GetFirstDocTemplatePosition();
	CDocTemplate	*doc_tmpl = GetNextDocTemplate(doc_tmpl_pos);
	CDocument *pDoc = doc_tmpl->CreateNewDocument();
	pDoc->OnNewDocument();
	CFrameWnd *pWnd = doc_tmpl->CreateNewFrame(pDoc, (CFrameWnd *)m_pMainWnd);

	// フレームをアクティブにする
	pWnd->ActivateFrame(SW_SHOW);

	// ビューをアクティブにする
	POSITION pos = pDoc->GetFirstViewPosition();
	CView* pView = pDoc->GetNextView(pos);
	pWnd->SetActiveView(pView);

	// タイトルを設定
//	pWnd->SetWindowText(title);
	pDoc->SetTitle(title);

	return pDoc;
}


void CPsqleditApp::OnSetFont() 
{
	CFontDialog		fontdlg;

	// ダイアログ初期化
	fontdlg.m_cf.Flags &= ~CF_EFFECTS;
	fontdlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
	g_font.GetLogFont(fontdlg.m_cf.lpLogFont);

	// ダイアログ表示
	if(fontdlg.DoModal() != IDOK) return;

	g_option.font.face_name = fontdlg.GetFaceName();
	g_option.font.point = fontdlg.GetSize();
	g_option.font.weight = fontdlg.GetWeight();
	g_option.font.italic = fontdlg.IsItalic();

	g_font.DeleteObject();
	CreateFont();

	UpdateAllDocViews(NULL, UPD_FONT, &g_font);
	SaveFontOption();
}

void CPsqleditApp::OnSetFontDlgBar() 
{
	CFontDialog		fontdlg;

	// ダイアログ初期化
	fontdlg.m_cf.Flags &= ~CF_EFFECTS;
	fontdlg.m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
	g_dlg_bar_font.GetLogFont(fontdlg.m_cf.lpLogFont);

	// ダイアログ表示
	if(fontdlg.DoModal() != IDOK) return;

	g_option.dlg_bar_font.face_name = fontdlg.GetFaceName();
	g_option.dlg_bar_font.point = fontdlg.GetSize();
	g_option.dlg_bar_font.weight = fontdlg.GetWeight();
	g_option.dlg_bar_font.italic = fontdlg.IsItalic();

	g_dlg_bar_font.DeleteObject();
	CreateDlgBarFont();

	g_object_bar.SetFont(&g_dlg_bar_font);
	g_object_detail_bar.SetFont(&g_dlg_bar_font);
	g_tab_bar.SetFont(&g_dlg_bar_font);

	SaveDlgBarFontOption();
}

void CPsqleditApp::WaitPostLoginThr()
{
	if(m_h_thread != NULL) {
		WaitForSingleObject((HANDLE)m_h_thread, INFINITE);
		CloseHandle((HANDLE)m_h_thread);
		m_h_thread = NULL;
	}
}

void CPsqleditApp::OnRefreshTableList() 
{
	if(g_login_flg == FALSE) {
		AfxGetMainWnd()->MessageBox(_T("データベースに接続していません"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	CWaitCursor	wait_cursor;

	g_object_bar.RefreshList();
	CSqlListMaker::ClearCache();

	// キーワードをリフレッシュ
	if((g_option.text_editor.table_name_completion || g_option.text_editor.column_name_completion)) {

		WaitPostLoginThr();

		// キーワード補完用のデータを初期化
		UINT	thread_addr;
		m_h_thread = _beginthreadex(NULL, 0, post_login_thr, NULL, 0, &thread_addr);
	}
}

void CPsqleditApp::OnSqlLibrary() 
{
	CSqlLibraryDlg	dlg;

	dlg.m_font = &g_font;
	dlg.m_root_dir = g_option.sql_lib_dir;

	if(dlg.DoModal() == IDCANCEL) return;

//	FIXME: OpenDocumentFile()では，最近使ったファイルリストに入ってしまうし，上書き保存できてしまう
//		CreateNewDocument()→OnOpenDocument()の実装(この関数の末尾)と，どちらがいいか考える
	OpenDocumentFile(dlg.m_file_name);

/*
	// 新しいドキュメントを作成
	char fname[_MAX_FNAME];
	_splitpath(dlg.m_file_name.GetBuffer(0), NULL, NULL, fname, NULL);
   
	CSqltuneDoc *pDoc = (CSqltuneDoc *)CreateNewDocument(fname);
	pDoc->OnOpenDocument(dlg.m_file_name.GetBuffer(0));
*/	
}

void CPsqleditApp::OnSetLoginInfo() 
{
	CConnectInfoDlg		dlg;

	dlg.DoModal();	
}

void CPsqleditApp::OnReconnect() 
{
	ReConnect();
}

int CPsqleditApp::ReConnect()
{
	// ログイン
	if(Login() != 0) {
		return 1;
	}

	g_login_flg = TRUE;

	// キーワードリスト，オブジェクトリストを更新
	PostLogin();
	g_object_bar.RefreshList();

	return 0;
}

void CPsqleditApp::OnDisconnect() 
{
	Logout();
}

void CPsqleditApp::OnUpdateDisconnect(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_login_flg);
}

void CPsqleditApp::SetSessionInfo(const TCHAR *session_info)
{
	g_session_info = session_info;
	if(::IsWindow(m_pMainWnd->GetSafeHwnd())) {
		CString title;
		title.Format(_T("%s"), session_info);
		m_pMainWnd->SetWindowText(title);
	}
}

void CPsqleditApp::PrintConnectInfo()
{
	if(g_ss == NULL) return;

	CMainFrame	*pMainFrame = (CMainFrame *)m_pMainWnd;
	if(pMainFrame == NULL || pMainFrame->GetActiveFrame() == NULL) return;

	CDocument *pdoc = ::GetActiveDocument();
	if(pdoc == NULL) return;

	HPgDataset dataset = pg_create_dataset(g_ss, _T("select version()"), g_msg_buf);
	if(dataset == NULL) return;

	CString message;
	message.Format(
		_T("---------------------------------------------------------------------\n")
		_T("%s\n")
		_T("user='%s' dbname='%s' host='%s' port='%s'\n")
		_T("---------------------------------------------------------------------\n"),
		pg_dataset_data(dataset, 0, 0),
		pg_user(g_ss), pg_db(g_ss), pg_host(g_ss), pg_port(g_ss));

	pdoc->UpdateAllViews(NULL, UPD_SET_RESULT_CARET, 0);
	pdoc->UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)message.GetBuffer(0));

	pg_free_dataset(dataset);
}

void CPsqleditApp::OnGetObjectSource() 
{
	GetObjectSource();
}

void CPsqleditApp::OnUpdateGetObjectSource(CCmdUI* pCmdUI) 
{
	if(g_login_flg == FALSE ||
		g_object_bar.GetSelectedUser() == "" ||
		g_object_bar.GetSelectedObject() == "" ||
		g_object_bar.GetSelectedType() == "") {
		pCmdUI->Enable(FALSE);
	} else {
		if(g_object_bar.CanGetSource()) {
			pCmdUI->Enable();
		} else {
			pCmdUI->Enable(FALSE);
		}
	}
}

int CPsqleditApp::GetObjectSource(const TCHAR *owner, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name, const TCHAR *schema)
{
	if(g_login_flg == FALSE) {
		AfxGetMainWnd()->MessageBox(_T("データベースに接続していません"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	CWaitCursor			wait_cursor;
	int			ret_v = 0;

	// タイトルを設定
	CString title;
	title.Format(_T("%s_%s"), object_type, object_name);
	TCHAR *p = title.GetBuffer(0);
	ostr_trim(p);
	ostr_char_replace(p, ' ', '_');
	title.ReleaseBuffer();

	// タイトルが長すぎると異常終了する問題を回避
	if(title.GetLength() > 50) title = title.Left(50) + _T("...");

	// 新しいドキュメントを作成
	CPsqleditDoc *pDoc = (CPsqleditDoc *)CreateNewDocument(title.GetBuffer(0));

	ret_v = create_object_source(g_ss, object_type, oid, object_name, schema, pDoc->GetSqlData(), g_msg_buf);
	if(ret_v != 0) {
		pDoc->UpdateAllViews(NULL, UPD_REDRAW, NULL);
		AfxGetMainWnd()->MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return ret_v;
	}

	pDoc->UpdateAllViews(NULL, UPD_REDRAW, NULL);

	return 0;
}

int CPsqleditApp::GetObjectSource(const TCHAR* owner, CStringArray* object_name_list, CStringArray* object_type_list,
	CStringArray* oid_list, CStringArray* schema_list)
{
	if(g_login_flg == FALSE) {
		AfxGetMainWnd()->MessageBox(_T("データベースに接続していません"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	CWaitCursor			wait_cursor;
	int			ret_v = 0;

	// タイトルを設定
	CString title;
	if(object_name_list->GetCount() == 1) {
		title.Format(_T("%s_%s"), object_type_list->GetAt(0).GetString(), object_name_list->GetAt(0).GetString());
	} else {
		title.Format(_T("%s_%s"), owner, object_type_list->GetAt(0).GetString());
	}
	TCHAR* p = title.GetBuffer(title.GetLength() * 2);
	ostr_trim(p);
	ostr_char_replace(p, ' ', '_');
	title.ReleaseBuffer();

	// タイトルが長すぎると異常終了する問題を回避
	if(title.GetLength() > 50) title = title.Left(50) + _T("...");

	// 新しいドキュメントを作成
	CPsqleditDoc* pDoc = (CPsqleditDoc*)CreateNewDocument(title.GetBuffer(0));

	CEditData* edit_data = pDoc->GetSqlData();
	int	i;
	for(i = 0; i < object_name_list->GetCount(); i++) {
		ret_v = create_object_source(g_ss, 
			object_type_list->GetAt(i).GetString(), 
			oid_list->GetAt(i).GetString(),
			object_name_list->GetAt(i).GetString(),
			schema_list->GetAt(i).GetString(),
			edit_data, g_msg_buf, FALSE);
		if(ret_v != 0) {
			AfxGetMainWnd()->MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
			return ret_v;
		}

		edit_data->paste_no_undo(_T("\n"));
	}

	edit_data->recalc_disp_size();
	edit_data->set_cur(0, 0);
	edit_data->reset_undo();

	pDoc->UpdateAllViews(NULL, UPD_REDRAW, NULL);

	return 0;
}

int CPsqleditApp::GetObjectSource()
{
	CWaitCursor			wait_cursor;

	CStringArray object_name_list;
	CStringArray object_type_list;
	CStringArray oid_list;
	CStringArray schema_list;

	int selected_cnt = g_object_bar.GetSelectedObjectList(&object_name_list, &object_type_list, &oid_list, &schema_list);

	ASSERT(selected_cnt > 0);

	return GetObjectSource(g_object_bar.GetSelectedUser().GetBuffer(0),
		&object_name_list, &object_type_list, &oid_list, &schema_list);
}

void CPsqleditApp::OnFileOpen() 
{
	CString newName;
	if (!DoPromptFileName(newName, AFX_IDS_OPENFILE,
	  OFN_FILEMUSTEXIST, TRUE, NULL))
		return; // open cancelled

	OpenDocumentFile(newName);
}


void CPsqleditApp::OnNewWindow()
{
	CString					cmd;
	PROCESS_INFORMATION		pi;
	STARTUPINFO				si;

	//	((CMainFrame *)AfxGetMainWnd())->SetNextWindowPos();

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	TCHAR path_buffer[_MAX_PATH];
	if(GetModuleFileName(NULL, path_buffer, sizeof(path_buffer) / sizeof(path_buffer[0])) == 0) {
		AfxMessageBox(_T("新しいウィンドウの作成に失敗しました"), MB_OK);
		return;
	}

	cmd = path_buffer;

	// NOTE:
	//  CurrentDirectoryを指定しないでCreateProcessしたとき、tnsnames.oraの参照先が
	//  変わってしまう場合があるので、AppPathを指定して起動する
	CString app_path = GetAppPath();

	if(CreateProcess(NULL, cmd.GetBuffer(0), NULL, NULL, FALSE,
		0, NULL, app_path, &si, &pi) == FALSE) {
		AfxMessageBox(_T("新しいウィンドウの作成に失敗しました"), MB_OK);
		return;
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

int CPsqleditApp::GetObjectSourceDetailBar()
{
	CWaitCursor			wait_cursor;

	CStringArray object_name_list;
	CStringArray object_type_list;
	CStringArray oid_list;
	CStringArray schema_list;

	int selected_cnt = g_object_detail_bar.GetSelectedObjectList(&object_name_list, &object_type_list, &oid_list, &schema_list);

	ASSERT(selected_cnt > 0);

	return GetObjectSource(g_object_bar.GetSelectedUser().GetBuffer(0),
		&object_name_list, &object_type_list, &oid_list, &schema_list);
}

void CPsqleditApp::OnGetObjectSourceDetailBar()
{
	GetObjectSourceDetailBar();
}

void CPsqleditApp::OnUpdateGetObjectSourceDetailBar(CCmdUI* pCmdUI)
{
	if (g_login_flg == FALSE ||
		g_object_bar.GetSelectedUser() == "" ||
		g_object_bar.GetSelectedObject() == "" ||
		g_object_bar.GetSelectedType() == "") {
		pCmdUI->Enable(FALSE);
	}
	else {
		if (g_object_detail_bar.CanGetSource()) {
			pCmdUI->Enable();
		}
		else {
			pCmdUI->Enable(FALSE);
		}
	}
}

void CPsqleditApp::OnShowCredits()
{
	OpenHelpFile(_T("credits.txt"));
}
