/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // OptionSheet.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "psqledit.h"
#include "OptionSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define OPTION_DLG_FONT _T("ＭＳ Ｐゴシック")
#define OPTION_DLG_FONT_SIZE 9

/////////////////////////////////////////////////////////////////////////////
// COptionSheet

IMPLEMENT_DYNAMIC(COptionSheet, CPropertySheet)

COptionSheet::COptionSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

COptionSheet::COptionSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

COptionSheet::~COptionSheet()
{
}


BEGIN_MESSAGE_MAP(COptionSheet, CPropertySheet)
	//{{AFX_MSG_MAP(COptionSheet)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionSheet メッセージ ハンドラ

int CALLBACK PropSheetProc(HWND hWndDlg, UINT uMsg, LPARAM lParam)
{
	switch(uMsg)
	{
	case PSCB_PRECREATE:
	{
		if(1)
		{
			LPDLGTEMPLATE pResource = (LPDLGTEMPLATE)lParam;
			CDialogTemplate dlgTemplate(pResource);
			dlgTemplate.SetFont(OPTION_DLG_FONT, OPTION_DLG_FONT_SIZE);
			memmove((void*)lParam, dlgTemplate.m_hTemplate, dlgTemplate.m_dwTemplateSize);
		}
	}
	break;
	}
	return 0;
}

INT_PTR COptionSheet::DoModal()
{
	// TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	m_psh.dwFlags |= PSH_NOAPPLYNOW;	// 適用ボタンを外す
	m_psh.pfnCallback = PropSheetProc;
	m_psh.dwFlags |= PSH_USECALLBACK;

	AddPage(&m_editor_page);
	AddPage(&m_editor_page2);
	AddPage(&m_grid_page);
	AddPage(&m_func_page);
	AddPage(&m_func_page2);
	AddPage(&m_setup_page);
	
	return CPropertySheet::DoModal();
}

void COptionSheet::BuildPropPageArray()
{
	CPropertySheet::BuildPropPageArray();

	if(1)
	{
		LPCPROPSHEETPAGE ppsp = m_psh.ppsp;
		const INT_PTR nSize = m_pages.GetSize();

		for(INT_PTR nPage = 0; nPage < nSize; nPage++)
		{
			const DLGTEMPLATE* pResource = ppsp->pResource;
			CDialogTemplate dlgTemplate(pResource);
			dlgTemplate.SetFont(OPTION_DLG_FONT, OPTION_DLG_FONT_SIZE);
			memmove((void*)pResource, dlgTemplate.m_hTemplate, dlgTemplate.m_dwTemplateSize);

			(BYTE*&)ppsp += ppsp->dwSize;
		}
	}
}

BOOL COptionSheet::Create(CWnd* pParentWnd, DWORD dwStyle, DWORD dwExStyle)
{
	m_psh.pfnCallback = PropSheetProc;
	m_psh.dwFlags |= PSH_USECALLBACK;
	return CPropertySheet::Create(pParentWnd, dwStyle, dwExStyle);
}
