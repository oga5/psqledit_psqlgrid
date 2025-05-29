/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __COMMON_H_INCLUDED__
#define __COMMON_H_INCLUDED__

int is_contain_lower(TCHAR *str);

// リストビューのソート用
typedef CString (*FP_GetItemText)(struct compare_struct *cs, LPARAM lParam);
struct compare_struct {
	CListCtrl *list;
	int sub_item;
	int sort_order;
	FP_GetItemText	fp;
};

CString GenericGetListViewItemText(struct compare_struct *cs, LPARAM lParam);
int CALLBACK CompareFuncStr(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK CompareFuncInt(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
void ListCtrlSort(CListCtrl* list_ctrl, int iSubItem, int sort_order);
void ListCtrlSortEx(CListCtrl *list_ctrl, int iSubItem, int sort_order);

void make_object_name(CString *str, const TCHAR *object_name, BOOL lower = FALSE);

int CalcFontHeight(CWnd *pwnd, CFont *font);

int CopyClipboard(TCHAR * str);

#endif /* __COMMON_H_INCLUDED__ */