/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // SetupPage.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "SetupPage.h"

#include "fileutil.h"
#include "ComboBoxUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetupPage プロパティ ページ

IMPLEMENT_DYNCREATE(CSetupPage, CPropertyPage)

CSetupPage::CSetupPage() : CPropertyPage(CSetupPage::IDD)
{
	//{{AFX_DATA_INIT(CSetupPage)
	m_initial_dir = _T("");
	m_register_shell = FALSE;
	m_save_modified = FALSE;
	m_max_msg_row = 0;
	m_tab_title_is_path_name = FALSE;
	m_max_mru = 0;
	m_search_loop_msg = FALSE;
	m_sql_lib_dir = _T("");
	m_sql_stmt_str = _T("");
	m_init_table_list_user = -1;
	m_init_object_list_type = -1;
	m_show_tab_tooltip = FALSE;
	m_tab_close_at_mclick = FALSE;
	m_tab_create_at_dblclick = FALSE;
	m_close_btn_on_tab = FALSE;
	m_tab_drag_move = FALSE;
	//}}AFX_DATA_INIT
}

CSetupPage::~CSetupPage()
{
}

void CSetupPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupPage)
	DDX_Control(pDX, IDC_COMBO_INIT_OBJECT_LIST_TYPE, m_combo_init_object_list_type);
	DDX_Text(pDX, IDC_EDIT_INITIAL_DIR, m_initial_dir);
	DDX_Check(pDX, IDC_CHECK_REGISTER_SHELL, m_register_shell);
	DDX_Check(pDX, IDC_CHECK_SAVE_MODIFIED, m_save_modified);
	DDX_Text(pDX, IDC_EDIT_MAX_MSG_ROW, m_max_msg_row);
	DDV_MinMaxInt(pDX, m_max_msg_row, 100, 99999);
	DDX_Check(pDX, IDC_CHECK_TAB_TITLE_IS_PATH_NAME, m_tab_title_is_path_name);
	DDX_Text(pDX, IDC_EDIT_MAX_MRU, m_max_mru);
	DDX_Check(pDX, IDC_CHECK_SEARCH_LOOP_MSG, m_search_loop_msg);
	DDX_Text(pDX, IDC_EDIT_SQL_LIB_DIR, m_sql_lib_dir);
	DDX_Text(pDX, IDC_EDIT_SQL_STMT_STR, m_sql_stmt_str);
	DDX_CBIndex(pDX, IDC_COMBO_INIT_OBJECT_LIST_USER, m_init_table_list_user);
	DDX_CBIndex(pDX, IDC_COMBO_INIT_OBJECT_LIST_TYPE, m_init_object_list_type);
	DDX_Check(pDX, IDC_CHECK_SHOW_TAB_TOOLTIP, m_show_tab_tooltip);
	DDX_Check(pDX, IDC_CHECK_TAB_CLOSE_AT_MCLICK, m_tab_close_at_mclick);
	DDX_Check(pDX, IDC_CHECK_TAB_CREATE_AT_DBLCLICK, m_tab_create_at_dblclick);
	DDX_Check(pDX, IDC_CHECK_CLOSE_BTN_ON_TAB, m_close_btn_on_tab);
	DDX_Check(pDX, IDC_CHECK_TAB_DRAG_MOVE, m_tab_drag_move);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetupPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSetupPage)
	ON_BN_CLICKED(IDC_BTN_INITIAL_DIR, OnBtnInitialDir)
	ON_BN_CLICKED(IDC_BTN_SQL_LIB_DIR, OnBtnSqlLibDir)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetupPage メッセージ ハンドラ

void CSetupPage::OnBtnInitialDir() 
{
	TCHAR	dir[MAX_PATH];
	TCHAR	*p;

	// ディレクトリ名の最後の \をはずす
	UpdateData(TRUE);
	p = m_initial_dir.GetBuffer(MAX_PATH);
	make_dirname2(p);
	m_initial_dir.ReleaseBuffer();

	if(SelectFolder(GetSafeHwnd(), dir, m_initial_dir.GetBuffer(0),
		_T("起動時のディレクトリの選択")) == TRUE) {
		m_initial_dir = dir;
		UpdateData(FALSE);
	}
}


BOOL CSetupPage::OnApply() 
{
	UpdateData(TRUE);
	if(m_max_mru > 16) m_max_mru = 16;
	if(m_max_mru < 1) m_max_mru = 1;

	m_init_object_list_type_str = ComboGetCurSelString(m_combo_init_object_list_type);

	return CPropertyPage::OnApply();
}

void CSetupPage::OnBtnSqlLibDir() 
{
	TCHAR	dir[MAX_PATH];
	TCHAR	*p;

	// ディレクトリ名の最後の \をはずす
	UpdateData(TRUE);
	p = m_initial_dir.GetBuffer(MAX_PATH);
	make_dirname2(p);
	m_sql_lib_dir.ReleaseBuffer();

	if(SelectFolder(GetSafeHwnd(), dir, m_sql_lib_dir.GetBuffer(0),
		_T("SQLライブラリのディレクトリの選択")) == TRUE) {
		m_sql_lib_dir = dir;
		UpdateData(FALSE);
	}
}

BOOL CSetupPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_combo_init_object_list_type.AddString(_T(""));
	m_combo_init_object_list_type.AddString(_T("TABLE"));
	m_combo_init_object_list_type.AddString(_T("VIEW"));
	int idx = ComboFindString(m_combo_init_object_list_type, m_init_object_list_type_str);
	if(idx != CB_ERR) {
		m_combo_init_object_list_type.SetCurSel(idx);
	}
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

