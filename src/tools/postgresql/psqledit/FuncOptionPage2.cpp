/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // FuncOptionPage2.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "FuncOptionPage2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef ListView_SetCheckState
   #define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)+1), LVIS_STATEIMAGEMASK)
#endif

/////////////////////////////////////////////////////////////////////////////
// CFuncOptionPage2 プロパティ ページ

IMPLEMENT_DYNCREATE(CFuncOptionPage2, CPropertyPage)

CFuncOptionPage2::CFuncOptionPage2() : CPropertyPage(CFuncOptionPage2::IDD)
, m_code_assist_sort_column(0)
, m_object_list_filter_column(0)
{
	//{{AFX_DATA_INIT(CFuncOptionPage2)
	m_column_name_completion = FALSE;
	m_table_name_completion = FALSE;
	m_use_keyword_window = FALSE;
	m_enable_code_assist = FALSE;
	//}}AFX_DATA_INIT
}

CFuncOptionPage2::~CFuncOptionPage2()
{
}

void CFuncOptionPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFuncOptionPage2)
	DDX_Control(pDX, IDC_LIST_COMPLETION_TYPE, m_list_completion_type);
	DDX_Check(pDX, IDC_CHECK_COLUMN_NAME_COMPLETION, m_column_name_completion);
	DDX_Check(pDX, IDC_CHECK_TABLE_NAME_COMPLETION, m_table_name_completion);
	DDX_Check(pDX, IDC_CHECK_KEYWORD_WINDOW, m_use_keyword_window);
	DDX_Check(pDX, IDC_CHECK_CODE_ASSIST, m_enable_code_assist);
	//}}AFX_DATA_MAP
	DDX_CBIndex(pDX, IDC_COMBO_CODE_ASSIST_SORT_COLUMN, m_code_assist_sort_column);
	DDX_CBIndex(pDX, IDC_COMBO_OBJECT_LIST_FILTER_COLUMN, m_object_list_filter_column);
}


BEGIN_MESSAGE_MAP(CFuncOptionPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CFuncOptionPage2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFuncOptionPage2 メッセージ ハンドラ

BOOL CFuncOptionPage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	InitCompletionObjectTypeList();
	
	return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
	              // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CFuncOptionPage2::InitCompletionObjectTypeList()
{
	m_list_completion_type.InsertColumn(0, _T("OBJECT TYPE"), LVCFMT_LEFT, 200);
	ListView_SetExtendedListViewStyle(m_list_completion_type.GetSafeHwnd(), LVS_EX_CHECKBOXES);

	int			i;
	LV_ITEM		item;

	item.mask = LVIF_PARAM | LVIF_TEXT;
	item.iSubItem = 0;
	for(i = 0; i < COT_COUNT; i++) {
		item.iItem = i;
		item.pszText = completion_object_type_list[i];
		item.lParam = i;
		item.iItem = m_list_completion_type.InsertItem(&item);
	}

	for(i = 0; i < COT_COUNT; i++) {
		if(m_completion_object_type[i] == FALSE) {
			ListView_SetCheckState(m_list_completion_type.GetSafeHwnd(), i, FALSE);
		} else {
			ListView_SetCheckState(m_list_completion_type.GetSafeHwnd(), i, TRUE);
		}
	}
}

BOOL CFuncOptionPage2::OnApply() 
{
	int		i;
	for(i = 0; i < sizeof(completion_object_type_list)/sizeof(completion_object_type_list[0]); i++) {
		m_completion_object_type[i] = ListView_GetCheckState(m_list_completion_type.GetSafeHwnd(), i);
	}
	
	return CPropertyPage::OnApply();
}
