/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // PSqlEditCtrl.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "PSqlEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPSqlEditCtrl

CPSqlEditCtrl::CPSqlEditCtrl()
{
	m_paste_lower = FALSE;
	SetCodeAssistListMaker(&m_sql_list_maker);
	m_no_quote_color_char = '"';
}

CPSqlEditCtrl::~CPSqlEditCtrl()
{
}


BEGIN_MESSAGE_MAP(CPSqlEditCtrl, CCodeAssistEditCtrl)
	//{{AFX_MSG_MAP(CPSqlEditCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPSqlEditCtrl メッセージ ハンドラ

void CPSqlEditCtrl::DoCodePaste(TCHAR *paste_str, TCHAR *type)
{
	CString paste_word;
/*
	if(strcmp((char *)type, "KEYWORD") == 0 || strcmp((char *)type, "KEYWORD2") == 0) {
		paste_word = paste_str;
	} else {
		make_object_name(&paste_word, (char *)paste_str, m_paste_lower);
	}
*/
	paste_word = paste_str;

	CString org_str = GetSelectedText();
	if(inline_isupper(org_str.GetBuffer(0)[0]) && paste_word.GetBuffer(0)[0] != '\"') {
		paste_word.MakeUpper();
	}

	// 補完前後のテキストが同じ場合、pasteしない (undoデータを増やさない)
	if(org_str.GetLength() == paste_word.GetLength() &&
		org_str.Compare(paste_word) == 0) {
		ClearSelected();
		return;
	}

	Paste(paste_word.GetBuffer(0));
}


