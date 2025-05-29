/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ShortCutSqlDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "ShortCutSqlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "ShortCutKeyDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CShortCutSqlDlg ダイアログ


CShortCutSqlDlg::CShortCutSqlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShortCutSqlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShortCutSqlDlg)
	m_name = _T("");
	m_shortcut_key_name = _T("");
	m_is_paste_to_editor = FALSE;
	m_is_show_dlg = FALSE;
	//}}AFX_DATA_INIT
}


void CShortCutSqlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShortCutSqlDlg)
	DDX_Control(pDX, IDC_CHECK_SHOW_DLG, m_check_show_dlg);
	DDX_Control(pDX, IDOK2, m_ok);
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	DDX_Text(pDX, IDC_EDIT_SHORTCUT_KEY, m_shortcut_key_name);
	DDX_Check(pDX, IDC_CHECK_PASTE_TO_EDITOR, m_is_paste_to_editor);
	DDX_Check(pDX, IDC_CHECK_SHOW_DLG, m_is_show_dlg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShortCutSqlDlg, CDialog)
	//{{AFX_MSG_MAP(CShortCutSqlDlg)
	ON_BN_CLICKED(IDOK2, OnOk2)
	ON_BN_CLICKED(IDC_BTN_SHORTCUT_KEY, OnBtnShortcutKey)
	ON_EN_CHANGE(IDC_EDIT_NAME, OnChangeEditName)
	ON_BN_CLICKED(IDC_CHECK_PASTE_TO_EDITOR, OnCheckPasteToEditor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShortCutSqlDlg メッセージ ハンドラ

void CShortCutSqlDlg::OnOK() 
{
}

void CShortCutSqlDlg::OnOk2() 
{
	UpdateData(TRUE);
	m_sql = m_edit_data.get_all_text();

	if(m_sql.GetLength() >= 1024) {
		MessageBox(_T("SQLが長すぎます。\n1024バイト以内にして下さい。"),
			_T("Error"), MB_ICONWARNING | MB_OK);
		return;
	}

	if(m_is_paste_to_editor) m_is_show_dlg = FALSE;

	CDialog::OnOK();
}

BOOL CShortCutSqlDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: この位置に初期化の補足処理を追加してください
	if(CreateEditCtrl() == FALSE) {
		MessageBox(_T("ダイアログ初期化エラー"), _T("Error"), MB_ICONERROR | MB_OK);
		CDialog::EndDialog(IDCANCEL);
		return FALSE;
	}

	UpdateData(FALSE);
	SetShortCutKeyName();
	CheckBtn();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

BOOL CShortCutSqlDlg::CreateEditCtrl()
{
	RECT name_rect, key_rect;
	GetDlgItem(IDC_EDIT_NAME)->GetWindowRect(&name_rect);
	GetDlgItem(IDC_EDIT_SHORTCUT_KEY)->GetWindowRect(&key_rect);

	ScreenToClient(&name_rect);
	ScreenToClient(&key_rect);

	m_edit_data.set_str_token(&g_sql_str_token);

	m_edit_ctrl.SetEditData(&m_edit_data);
	m_edit_ctrl.Create(NULL, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP, 
		CRect(name_rect.left, name_rect.bottom + 10,
			name_rect.right, key_rect.top - 10), 
		this, NULL);
	m_edit_ctrl.SetFont(&g_font);
	m_edit_ctrl.SetExStyle2(ECS_ON_DIALOG);

	m_edit_ctrl.Paste(m_sql.GetBuffer(0));

	m_edit_ctrl.SetWindowPos(GetDlgItem(IDC_EDIT_NAME), 0, 0, 0, 0, 
		SWP_NOMOVE | SWP_NOSIZE);

	return TRUE;
}

void CShortCutSqlDlg::OnBtnShortcutKey() 
{
	CShortCutKeyDlg		dlg;

	dlg.m_command = m_command;
	dlg.m_accel_list = m_accel_list;

	if(dlg.DoModal() == IDOK) {
		m_accel_list = dlg.m_accel_list;
		SetShortCutKeyName();
	}
}

void CShortCutSqlDlg::SetShortCutKeyName()
{
	UpdateData(TRUE);

	CString key;
	if(m_accel_list.search_accel_str2(m_command, key) == 0) {
		m_shortcut_key_name = key;
	}

	UpdateData(FALSE);
}

void CShortCutSqlDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_name == "") {
		m_ok.EnableWindow(FALSE);
	} else {
		m_ok.EnableWindow(TRUE);
	}
}

void CShortCutSqlDlg::OnChangeEditName() 
{
	CheckBtn();
}

void CShortCutSqlDlg::OnCheckPasteToEditor() 
{
	UpdateData(TRUE);

	m_check_show_dlg.EnableWindow(!m_is_paste_to_editor);
}
