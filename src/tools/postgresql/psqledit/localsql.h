/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "UnicodeArchive.h"

#define ALL_USERS	_T("ALL USERS")

CString get_table_schema_name(HPgSession ss, const TCHAR *table_name, TCHAR *msg_buf);

int get_dataset(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf, 
	void *hWnd, volatile int *cancel_flg, HPgDataset *result);

CString explain_plan(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf);

int execute_sql(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf,
	volatile int *cancel_flg);

int download(HPgSession ss, CUnicodeArchive *ar, const TCHAR *sql, TCHAR *msg_buf,
	BOOL put_column_name, void *hWnd, volatile int *cancel_flg);

HPgDataset get_object_list(HPgSession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf);
HPgDataset get_user_list(HPgSession ss, TCHAR *msg_buf);
HPgDataset get_column_list(HPgSession ss, const TCHAR *relname, const TCHAR *schema, TCHAR *msg_buf);
HPgDataset get_index_list_by_table(HPgSession ss, const TCHAR *relname, 
	const TCHAR *schema, TCHAR *msg_buf);
HPgDataset get_trigger_list_by_table(HPgSession ss, const TCHAR *relname,
	const TCHAR *schema, TCHAR *msg_buf);

HPgDataset get_object_properties(HPgSession ss, const TCHAR *type, const TCHAR *name, 
	const TCHAR *schema, TCHAR *msg_buf);

CString show_search_path(HPgSession ss, TCHAR *msg_buf);
int set_search_path(HPgSession ss, const TCHAR *search_path, TCHAR *msg_buf);

bool is_schema_in_search_path(HPgSession ss, const TCHAR* schema_name, TCHAR* msg_buf);
