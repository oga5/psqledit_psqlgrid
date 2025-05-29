/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "psqledit.h"
#include "psqleditDoc.h"
#include "CancelDlg.h"
#include "diff_util.h"
#include "ostrutil.h"
#include <algorithm>

#define CMD_BUF_SIZE	255

void PutResultString(HWND hwnd, const TCHAR* str)
{
	SendMessage(hwnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (WPARAM)str);
}

static void _put_grid_row(CEditData* edit_data, CGridData* grid_data, int row)
{
	int		col;
	int		col_cnt = grid_data->Get_ColCnt();

	for(col = 0; col < col_cnt; col++) {
		if(col != 0) edit_data->paste_no_undo(_T("\t"));
		edit_data->paste_no_undo(grid_data->Get_ColData(row, col));
	}
	edit_data->paste_no_undo(_T("\n"));
}

static void _print_diff_row(CEditData* edit_data, TCHAR* msg_buf, CGridData* grid_data,
	const TCHAR* suffix, int row1, int row2)
{
	row1++;
	if(row2 < grid_data->Get_RowCnt()) row2++;

	_stprintf(msg_buf, _T("%s %d,%d\n"), suffix, row1, row2);
	edit_data->paste_no_undo(msg_buf);
}

static int grid_diff(HPgSession ss, const TCHAR* p,
	CSQLStrToken* str_token, HWND hWnd, TCHAR* msg_buf)
{
	CString		data1;
	CString		data2;

	p = str_token->skipCommentAndSpace(p);

	p = str_token->get_word(p, data1.GetBuffer(CMD_BUF_SIZE), CMD_BUF_SIZE);
	data1.ReleaseBuffer();
	data1.Replace(_T("\""), _T(""));

	p = str_token->get_word(p, data2.GetBuffer(CMD_BUF_SIZE), CMD_BUF_SIZE);
	data2.ReleaseBuffer();
	data2.Replace(_T("\""), _T(""));

	int		tab_idx1 = _ttoi(data1) - 1;
	int		tab_idx2 = _ttoi(data2) - 1;

	CPsqleditDoc* doc1 = (CPsqleditDoc*)g_tab_bar.GetDoc(tab_idx1);
	CPsqleditDoc* doc2 = (CPsqleditDoc*)g_tab_bar.GetDoc(tab_idx2);

	if(doc1 == NULL) {
		_stprintf(msg_buf, _T("Tabのindexが無効です(%d)"), tab_idx1);
		return 1;
	}
	if(doc2 == NULL) {
		_stprintf(msg_buf, _T("Tabのindexが無効です(%d)"), tab_idx2);
		return 1;
	}

	CGridData* grid_data1 = doc1->GetGridData();
	CGridData* grid_data2 = doc2->GetGridData();

	int diff_cnt;
	diff_result_v* diff_result = grid_diff(grid_data1, grid_data2, &diff_cnt);

	if(diff_cnt == 0) {
		_stprintf(msg_buf, _T("差分はありません"));
		PutResultString(hWnd, msg_buf);
	} else {
		CEditData	edit_data;
		int			result_cnt = (int)diff_result->size();
		int			r;
		int			i;
		int			start, end;
		int			cmn_cnt, ins_cnt, del_cnt;
		CString		title1 = doc1->GetTitle();
		CString		title2 = doc2->GetTitle();

		_stprintf(msg_buf, _T("*** %s\n"), title1.GetBuffer(0));
		edit_data.paste_no_undo(msg_buf);
		_stprintf(msg_buf, _T("--- %s\n"), title2.GetBuffer(0));
		edit_data.paste_no_undo(msg_buf);

		for(r = 0; r < result_cnt; r++) {
			if((*diff_result)[r].cmd == DIFF_COMMON) continue;

			start = r - 3;
			if(start < 0) start = 0;

			cmn_cnt = ins_cnt = del_cnt = 0;
			for(; r < result_cnt && cmn_cnt < 6; r++, cmn_cnt++) {
				if((*diff_result)[r].cmd != DIFF_COMMON) {
					if((*diff_result)[r].cmd == DIFF_INSERT) ins_cnt++;
					if((*diff_result)[r].cmd == DIFF_DELETE) del_cnt++;
					cmn_cnt = 0;
					continue;
				}
			}
			end = r;
			if(r != result_cnt) end -= 2;

			_print_diff_row(&edit_data, msg_buf, grid_data1, _T("***"),
				(*diff_result)[start].row1, (*diff_result)[end - 1].row1);
			if(del_cnt > 0) {
				for(i = start; i < end; i++) {
					if((*diff_result)[i].cmd == DIFF_INSERT) continue;

					if((*diff_result)[i].cmd == DIFF_DELETE) {
						edit_data.paste_no_undo(_T("- "));
					} else {
						edit_data.paste_no_undo(_T("  "));
					}
					_put_grid_row(&edit_data, grid_data1, (*diff_result)[i].row1);
				}
			}

			_print_diff_row(&edit_data, msg_buf, grid_data2, _T("---"),
				(*diff_result)[start].row2, (*diff_result)[end - 1].row2);
			if(ins_cnt > 0) {
				for(i = start; i < end; i++) {
					if((*diff_result)[i].cmd == DIFF_DELETE) continue;

					if((*diff_result)[i].cmd == DIFF_INSERT) {
						edit_data.paste_no_undo(_T("+ "));
					} else {
						edit_data.paste_no_undo(_T("  "));
					}
					_put_grid_row(&edit_data, grid_data2, (*diff_result)[i].row2);
				}
			}
		}

		PutResultString(hWnd, edit_data.get_all_text());
		PutResultString(hWnd, _T("\n"));
	}

	delete diff_result;

	return 0;
}

static int print_set_vars(CMapStringToString* bind_data, HWND hWnd, TCHAR* msg_buf)
{
	std::vector<CString> str_vec;
	TCHAR	buf[1024 * 16] = _T("");

	CMapStringToString::CPair* pCurVal;
	pCurVal = bind_data->PGetFirstAssoc();
	while(pCurVal != NULL) {
		str_vec.push_back(pCurVal->key);
		pCurVal = bind_data->PGetNextAssoc(pCurVal);
	}

	std::sort(str_vec.begin(), str_vec.end());

	for(size_t i = 0; i < str_vec.size(); i++) {
		CString k = str_vec[i];
		CString v;
		bind_data->Lookup(k, v);
		_stprintf(buf, _T("%s = '%s'\n"), k.GetBuffer(0), v.GetBuffer(0));
		PutResultString(hWnd, buf);
	}

	return 0;
}

static int set_command(CMapStringToString* bind_data,
	const TCHAR* command, CSQLStrToken* str_token, HWND hWnd, TCHAR* msg_buf)
{
	int		ret_v = 0;
	TCHAR	name[1024 * 8] = _T("");
	TCHAR	value[1024 * 8] = _T("");

	const TCHAR* p = command;

	if(p != NULL) {
		p = str_token->skipSpace(p);
		p = str_token->get_word(p, name, sizeof(name), FALSE);

		if(p != NULL) {
			p = str_token->skipSpace(p);
			_tcscpy(value, p);
			ostr_chomp(value, ' ');

			if(value[0] == '\'') {
				if(value[_tcslen(value) - 1] == '\'') value[_tcslen(value) - 1] = '\0';

				CString tmp_value = value + 1;
				tmp_value.Replace(_T("\\r"), _T("\r"));
				tmp_value.Replace(_T("\\n"), _T("\n"));
				tmp_value.Replace(_T("\\t"), _T("\t"));
				tmp_value.Replace(_T("''"), _T("'"));
				_tcscpy(value, tmp_value.GetBuffer(0));
			}
		}
	}

	if(_tcslen(name) == 0) {
		return print_set_vars(bind_data, hWnd, msg_buf);
	}

	TCHAR	buf[1024 * 16] = _T("");
	bind_data->SetAt(name, value);
	_stprintf(buf, _T("[set result] %s = '%s'\n"), name, value);
	PutResultString(hWnd, buf);

	return ret_v;
}

int do_command(HPgSession ss, CPsqleditDoc* doc, const TCHAR* command, CSQLStrToken* str_token,
	TCHAR* msg_buf, HWND hWnd, struct _thr_sql_run_st* sql_run_st)
{
	ASSERT(hWnd != NULL);

	int		ret_v = 0;
	CString			cmd_buf;
	const TCHAR* p = command;

	if(*p == '!') p++;

	p = str_token->get_word(p, cmd_buf.GetBuffer(CMD_BUF_SIZE), CMD_BUF_SIZE);
	cmd_buf.ReleaseBuffer();

	PutResultString(hWnd, command);
	PutResultString(hWnd, _T("%s\n"));

	if(cmd_buf.CompareNoCase(_T("grid_diff")) == 0) {
		ret_v = grid_diff(ss, p, str_token, hWnd, msg_buf);
	} else 	if(cmd_buf.Compare(_T("\\set")) == 0) {
		ret_v = set_command(doc->GetBindData(), p, str_token, hWnd, msg_buf);
	} else {
		_stprintf(msg_buf, _T("invalid command"));
		return 1;
	}

	return 0;
}

