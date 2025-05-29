/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#pragma once

struct _thr_sql_run_st
{
	CPsqleditDoc* pdoc;
	CEditData* edit_data;
	int			start_row;
	int			end_row;
	HWND		hWnd;
	int			ret_v;
	BOOL* obj_update_flg;
	CUnicodeArchive* ar;
	int			sql_cnt;
	volatile int* cancel_flg;
};

void PutResultString(HWND hwnd, const TCHAR* str);

int do_command(HPgSession ss, CPsqleditDoc* doc, const TCHAR* command, CSQLStrToken* str_token,
	TCHAR* msg_buf, HWND hWnd, struct _thr_sql_run_st* sql_run_st);

