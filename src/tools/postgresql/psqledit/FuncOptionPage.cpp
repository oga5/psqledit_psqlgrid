/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // FuncOptionPage.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "FuncOptionPage.h"

#include "fileutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFuncOptionPage プロパティ ページ

IMPLEMENT_DYNCREATE(CFuncOptionPage, CPropertyPage)

CFuncOptionPage::CFuncOptionPage() : CPropertyPage(CFuncOptionPage::IDD)
, m_sql_run_selected_area(FALSE)
, m_make_select_add_alias_name(FALSE)
, m_make_select_alias_name(_T(""))
, m_refresh_table_list_after_ddl(FALSE)
{
	//{{AFX_DATA_INIT(CFuncOptionPage)
	m_download_column_name = FALSE;
	m_ignore_sql_error = FALSE;
	m_adjust_col_width_after_select = FALSE;
	m_sql_logging = FALSE;
	m_sql_log_dir = _T("");
	m_receive_async_notify = FALSE;
	m_make_select_sql_use_semicolon = FALSE;
	m_auto_commit = FALSE;
	//}}AFX_DATA_INIT
}

CFuncOptionPage::~CFuncOptionPage()
{
}

void CFuncOptionPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFuncOptionPage)
	DDX_Control(pDX, IDC_EDIT_SQL_LOG_DIR, m_edit_sql_log_dir);
	DDX_Control(pDX, IDC_BTN_SQL_LOG_DIR, m_btn_sql_log_dir);
	DDX_Check(pDX, IDC_CHECK_DOWNLOAD_COLUMN_NAME, m_download_column_name);
	DDX_Check(pDX, IDC_CHECK_IGNORE_SQL_ERROR, m_ignore_sql_error);
	DDX_Check(pDX, IDC_CHECK_ADJUST_COL_WIDTH_AFTER_SELECT, m_adjust_col_width_after_select);
	DDX_Check(pDX, IDC_CHECK_SQL_LOGGING, m_sql_logging);
	DDX_Text(pDX, IDC_EDIT_SQL_LOG_DIR, m_sql_log_dir);
	DDX_Check(pDX, IDC_CHECK_RECEIVE_ASYNC_NOTIFY, m_receive_async_notify);
	DDX_Check(pDX, IDC_CHECK_MAKE_SELECT_SQL_USE_SEMICOLON, m_make_select_sql_use_semicolon);
	DDX_Check(pDX, IDC_CHECK_AUTO_COMMIT, m_auto_commit);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_SQL_RUN_SELECTED_AREA, m_sql_run_selected_area);
	DDX_Check(pDX, IDC_CHECK_MAKE_SELECT_ADD_ALIAS_NAME, m_make_select_add_alias_name);
	//  DDX_Text(pDX, IDC_EDIT_MAKE_SELECT_ALIAS_NAME, m_make_select_alias_name);
	DDX_Control(pDX, IDC_EDIT_MAKE_SELECT_ALIAS_NAME, m_edit_make_select_alias_name);
	DDX_Text(pDX, IDC_EDIT_MAKE_SELECT_ALIAS_NAME, m_make_select_alias_name);
	DDX_Check(pDX, IDC_CHECK_REFRESH_TABLE_LIST_AFTER_DDL, m_refresh_table_list_after_ddl);
}


BEGIN_MESSAGE_MAP(CFuncOptionPage, CPropertyPage)
	//{{AFX_MSG_MAP(CFuncOptionPage)
	ON_BN_CLICKED(IDC_BTN_SQL_LOG_DIR, OnBtnSqlLogDir)
	ON_BN_CLICKED(IDC_CHECK_SQL_LOGGING, OnCheckSqlLogging)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_MAKE_SELECT_ADD_ALIAS_NAME, &CFuncOptionPage::OnClickedCheckMakeSelectAddAliasName)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFuncOptionPage メッセージ ハンドラ


BOOL CFuncOptionPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	UpdateData(FALSE);

	CheckSqlLog();
	CheckMakeSelectCheckBox();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CFuncOptionPage::OnBtnSqlLogDir() 
{
	TCHAR	dir[MAX_PATH];
	TCHAR	*p;

	// ディレクトリ名の最後の \をはずす
	UpdateData(TRUE);
	p = m_sql_log_dir.GetBuffer(MAX_PATH);
	make_dirname2(p);
	m_sql_log_dir.ReleaseBuffer();

	if(SelectFolder(GetSafeHwnd(), dir, m_sql_log_dir.GetBuffer(0),
		_T("起動時のディレクトリの選択")) == TRUE) {
		m_sql_log_dir = dir;
		UpdateData(FALSE);
	}
}

void CFuncOptionPage::OnCheckSqlLogging() 
{
	UpdateData(TRUE);

	CheckSqlLog();
}

void CFuncOptionPage::CheckSqlLog()
{
	if(m_sql_logging) {
		m_edit_sql_log_dir.EnableWindow(TRUE);
		m_btn_sql_log_dir.EnableWindow(TRUE);
	} else {
		m_edit_sql_log_dir.EnableWindow(FALSE);
		m_btn_sql_log_dir.EnableWindow(FALSE);
	}
}

void CFuncOptionPage::CheckMakeSelectCheckBox()
{
	if(m_make_select_add_alias_name) {
		m_edit_make_select_alias_name.EnableWindow(TRUE);
	} else {
		m_edit_make_select_alias_name.EnableWindow(FALSE);
	}
}

void CFuncOptionPage::OnClickedCheckMakeSelectAddAliasName()
{
	UpdateData(TRUE);
	BOOL b_add_alias_name = TRUE;

	CheckMakeSelectCheckBox();
}
