/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include "mbstring.h"
#include "ostrutil.h"
#include "common.h"

//#include <afxdllx.h>
#include <shlobj.h>
#include "mbutil.h"

#include <sys/types.h>
#include <sys/stat.h>


//
// 文字の取得(ItemData == 行番号の必要がある)
//
CString GenericGetListViewItemText(struct compare_struct *cs, LPARAM lParam)
{
	return cs->list->GetItemText((int)lParam, cs->sub_item);
}

//
// 文字列の比較
//
int CALLBACK CompareFuncStr(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	struct compare_struct *cs = (struct compare_struct *)lParamSort;

	TRACE(_T("%d:%d:%s:%s:%d\n"), lParam1, lParam2, cs->fp(cs, lParam1), cs->fp(cs, lParam2), _tcscmp(cs->fp(cs, lParam1), cs->fp(cs, lParam2)));

	if(cs->sort_order > 0) {
		return _tcscmp(cs->fp(cs, lParam1), cs->fp(cs, lParam2));
	} else {
		return _tcscmp(cs->fp(cs, lParam2), cs->fp(cs, lParam1));
	}
}

//
// 数字の比較
//
int CALLBACK CompareFuncInt(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	struct compare_struct *cs = (struct compare_struct *)lParamSort;

	if(cs->sort_order > 0) {
		return _ttoi(cs->fp(cs, lParam1)) - _ttoi(cs->fp(cs, lParam2));
	} else {
		return _ttoi(cs->fp(cs, lParam2)) - _ttoi(cs->fp(cs, lParam1));
	}
}

void ListCtrlSort(CListCtrl *list_ctrl, int iSubItem, int sort_order)
{
	CWaitCursor		wait_cursor;
	struct compare_struct	cs;

	cs.list = list_ctrl;
	cs.sub_item = iSubItem;
	cs.sort_order = sort_order;
	cs.fp = GenericGetListViewItemText;

	LV_COLUMN	lv_column;
	lv_column.mask = LVCF_FMT;
	list_ctrl->GetColumn(iSubItem, &lv_column);

	if(lv_column.fmt & LVCFMT_RIGHT) {
		list_ctrl->SortItems(CompareFuncInt, (LPARAM)&cs);
	} else {
		list_ctrl->SortItems(CompareFuncStr, (LPARAM)&cs);
	}
}

void ListCtrlSortEx(CListCtrl* list_ctrl, int iSubItem, int sort_order)
{
	CWaitCursor		wait_cursor;
	struct compare_struct	cs;

	cs.list = list_ctrl;
	cs.sub_item = iSubItem;
	cs.sort_order = sort_order;
	cs.fp = GenericGetListViewItemText;

	LV_COLUMN	lv_column;
	lv_column.mask = LVCF_FMT;
	list_ctrl->GetColumn(iSubItem, &lv_column);

	if(lv_column.fmt & LVCFMT_RIGHT) {
		list_ctrl->SortItemsEx(CompareFuncInt, (LPARAM)&cs);
	} else {
		list_ctrl->SortItemsEx(CompareFuncStr, (LPARAM)&cs);
	}
}

void make_object_name(CString *str, const TCHAR *object_name, BOOL lower)
{
	if(ostr_is_contain_upper(object_name)) {
		str->Format(_T("\"%s\""), object_name);
	} else {
		str->Format(_T("%s"), object_name);
		if(lower != FALSE) str->MakeLower();
	}
}

int CalcFontHeight(CWnd *pwnd, CFont *font)
{
	TEXTMETRIC tm;
	CDC *pdc = pwnd->GetDC();
	pdc->SetMapMode(MM_TEXT);
	CFont *pOldFont = pdc->SelectObject(font);
	pdc->GetTextMetrics(&tm);
	pdc->SelectObject(pOldFont);
	pwnd->ReleaseDC(pdc);

	return tm.tmHeight + tm.tmExternalLeading;
}

int CopyClipboard(TCHAR * str)
{
	HGLOBAL hData = GlobalAlloc(GHND, (_tcslen(str) + 1) * sizeof(TCHAR) );
	TCHAR *pstr = (TCHAR *)GlobalLock(hData);

	_tcscpy(pstr, str);

	GlobalUnlock(hData);

	if ( !OpenClipboard(NULL) ) {
		AfxMessageBox( _T("Cannot open the Clipboard") );
		return 1;
	}
	// Remove the current Clipboard contents
	if( !EmptyClipboard() ) {
		AfxMessageBox( _T("Cannot empty the Clipboard") );
		return 1;
	}
	// ...
	// Get the currently selected data
	// ...
	// For the appropriate data formats...
	if ( SetClipboardData( CF_UNICODETEXT, hData ) == NULL ) {
		AfxMessageBox( _T("Unable to set Clipboard data") );
		CloseClipboard();
		return 1;
	}
	// ...
	CloseClipboard();

	return 0;
}
