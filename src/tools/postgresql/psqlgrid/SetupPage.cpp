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
#include "psqlgrid.h"
#include "SetupPage.h"

#include "fileutil.h"

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
	m_nls_date_format = _T("");
	m_adjust_col_width = FALSE;
	m_put_column_name = FALSE;
	m_initial_dir = _T("");
	m_register_shell = FALSE;
	m_use_message_beep = FALSE;
	m_nls_timestamp_format = _T("");
	m_nls_timestamp_tz_format = _T("");
	m_timezone = _T("");
	m_use_user_updatable_columns = FALSE;
	m_share_connect_info = FALSE;
	m_disp_all_delete_null_rows_warning = FALSE;
	m_edit_other_owner = FALSE;
	//}}AFX_DATA_INIT
}

CSetupPage::~CSetupPage()
{
}

void CSetupPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetupPage)
	DDX_Control(pDX, IDC_COMBO_TIMEZONE, m_combo_timezone);
	DDX_Control(pDX, IDC_COMBO_NLS_TIMESTAMP_FORMAT_TZ, m_combo_nls_timestamp_tz_format);
	DDX_Control(pDX, IDC_COMBO_NLS_TIMESTAMP_FORMAT, m_combo_nls_timestamp_format);
	DDX_Control(pDX, IDC_COMBO_NLS_DATE_FORMAT, m_combo_nls_date_format);
	DDX_CBString(pDX, IDC_COMBO_NLS_DATE_FORMAT, m_nls_date_format);
	DDX_Check(pDX, IDC_CHECK_ADJUST_COL_WIDTH, m_adjust_col_width);
	DDX_Check(pDX, IDC_CHECK_PUT_COLUMN_NAME, m_put_column_name);
	DDX_Text(pDX, IDC_EDIT_INITIAL_DIR, m_initial_dir);
	DDX_Check(pDX, IDC_CHECK_REGISTER_SHELL, m_register_shell);
	DDX_Check(pDX, IDC_CHECK_USE_MESSAGE_BEEP, m_use_message_beep);
	DDX_CBString(pDX, IDC_COMBO_NLS_TIMESTAMP_FORMAT, m_nls_timestamp_format);
	DDX_CBString(pDX, IDC_COMBO_NLS_TIMESTAMP_FORMAT_TZ, m_nls_timestamp_tz_format);
	DDX_CBString(pDX, IDC_COMBO_TIMEZONE, m_timezone);
	DDX_Check(pDX, IDC_CHECK_USE_USER_UPDATABLE_COLUMNS, m_use_user_updatable_columns);
	DDX_Check(pDX, IDC_CHECK_SHARE_CONNECT_INFO, m_share_connect_info);
	DDX_Check(pDX, IDC_CHECK_DISP_ALL_DELETE_NULL_ROWS_WARNING, m_disp_all_delete_null_rows_warning);
	DDX_Check(pDX, IDC_CHECK_EDIT_OTHER_OWNER, m_edit_other_owner);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetupPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSetupPage)
	ON_BN_CLICKED(IDC_BTN_INITIAL_DIR, OnBtnInitialDir)
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


static void setup_combo_box(CComboBox &combo, CString &data)
{
	int idx = combo.FindStringExact(-1, data);

	if(idx != CB_ERR) {
		combo.SetCurSel(idx);
		return;
	}

	combo.InsertString(-1, data);
	idx = combo.FindStringExact(-1, data);
	if(idx != CB_ERR) combo.SetCurSel(idx);
}

BOOL CSetupPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	UpdateData(FALSE);

	setup_combo_box(m_combo_nls_date_format, m_nls_date_format);
	setup_combo_box(m_combo_nls_timestamp_format, m_nls_timestamp_format);
	setup_combo_box(m_combo_nls_timestamp_tz_format, m_nls_timestamp_tz_format);
	setup_combo_box(m_combo_timezone, m_timezone);
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}
