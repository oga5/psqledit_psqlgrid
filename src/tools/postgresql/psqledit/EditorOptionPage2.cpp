/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // EditorOptionPage2.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "EditorOptionPage2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage2 プロパティ ページ

IMPLEMENT_DYNCREATE(CEditorOptionPage2, CPropertyPage)

CEditorOptionPage2::CEditorOptionPage2() : CPropertyPage(CEditorOptionPage2::IDD)
{
	//{{AFX_DATA_INIT(CEditorOptionPage2)
	m_auto_indent = FALSE;
	m_copy_lower_name = FALSE;
	m_drag_drop_edit = FALSE;
	m_tab_as_space = FALSE;
	m_row_copy_at_no_sel = FALSE;
	//}}AFX_DATA_INIT
}

CEditorOptionPage2::~CEditorOptionPage2()
{
}

void CEditorOptionPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditorOptionPage2)
	DDX_Check(pDX, IDC_CHECK_AUTO_INDENT, m_auto_indent);
	DDX_Check(pDX, IDC_CHECK_COPY_LOWER_NAME, m_copy_lower_name);
	DDX_Check(pDX, IDC_CHECK_DRAG_DROP_EDIT, m_drag_drop_edit);
	DDX_Check(pDX, IDC_CHECK_TAB_AS_SPACE, m_tab_as_space);
	DDX_Check(pDX, IDC_CHECK_ROW_COPY_AT_NO_SEL, m_row_copy_at_no_sel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditorOptionPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CEditorOptionPage2)
		// メモ: ClassWizard はこの位置に DDX および DDV の呼び出しコードを追加します。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage2 メッセージ ハンドラ
