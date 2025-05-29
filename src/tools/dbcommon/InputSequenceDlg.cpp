/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// InputSequenceDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "InputSequenceDlg.h"

#include "ostrutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInputSequenceDlg ダイアログ


CInputSequenceDlg::CInputSequenceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInputSequenceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInputSequenceDlg)
	m_incremental_str = _T("");
	m_start_num_str = _T("");
	//}}AFX_DATA_INIT
}


void CInputSequenceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInputSequenceDlg)
	DDX_Control(pDX, IDOK, m_ok);
	DDX_Text(pDX, IDC_EDIT_INCREMENTAL, m_incremental_str);
	DDX_Text(pDX, IDC_EDIT_START_NUM, m_start_num_str);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInputSequenceDlg, CDialog)
	//{{AFX_MSG_MAP(CInputSequenceDlg)
	ON_EN_CHANGE(IDC_EDIT_START_NUM, OnChangeEditStartNum)
	ON_EN_CHANGE(IDC_EDIT_INCREMENTAL, OnChangeEditIncremental)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInputSequenceDlg メッセージ ハンドラ

void CInputSequenceDlg::OnChangeEditStartNum() 
{
	CheckBtn();
}

void CInputSequenceDlg::OnChangeEditIncremental() 
{
	CheckBtn();
}

BOOL CInputSequenceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_start_num_str.Format(_T("%d"), m_start_num);
	m_incremental_str.Format(_T("%d"), m_incremental);

	UpdateData(FALSE);
	
	CheckBtn();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CInputSequenceDlg::CheckBtn()
{
	UpdateData(TRUE);

	if(m_start_num_str.IsEmpty() || m_incremental_str.IsEmpty() ||
		!ostr_str_isnum(m_start_num_str) ||
		!ostr_str_isnum(m_incremental_str)) {
		m_ok.EnableWindow(FALSE);
	} else {
		m_ok.EnableWindow(TRUE);
	}
}

void CInputSequenceDlg::OnOK() 
{
	UpdateData(TRUE);

	m_start_num = _ttoi64(m_start_num_str);
	m_incremental = _ttoi64(m_incremental_str);
	
	CDialog::OnOK();
}
