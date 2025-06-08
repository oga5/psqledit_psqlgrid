/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include "localdef.h"
#include "pgapi.h"
#include "pgmsg.h"
#include "apidef.h"

static void pg_init_dataset(HPgDataset dataset)
{
	dataset->row_cnt = 0;
	dataset->col_cnt = 0;
	dataset->res = NULL;
	dataset->col_data = NULL;
	dataset->buf = NULL;
}

void pg_free_dataset(HPgDataset dataset)
{
	if(dataset == NULL) return;

	if(dataset->buf != NULL) free(dataset->buf);
	if(dataset->col_data != NULL) free(dataset->col_data);
	if(dataset->cname != NULL) free(dataset->cname);
	if(dataset->res != NULL) fp_pqclear(dataset->res);

	free(dataset);
}

static HPgDataset pg_alloc_dataset(TCHAR *msg_buf)
{
	HPgDataset	dataset = NULL;

	dataset = (HPgDataset)malloc(sizeof(PgDataset));
	if(dataset == NULL) {
		if(msg_buf != NULL) _stprintf(msg_buf, PGERR_MEM_ALLOC_MSG);
		return NULL;
	}

	pg_init_dataset(dataset);

	return dataset;
}

static void pg_after_select(HPgDataset dataset, TCHAR *msg_buf)
{
	int r, c;
	int memsize = 0;
	TCHAR *p;
	int cnt;

	if(dataset->res == NULL) return;

	// 行数を取得
	dataset->row_cnt = fp_pqntuples(dataset->res);
	// カラム数を取得
	dataset->col_cnt = fp_pqnfields(dataset->res);	

	for(c = 0; c < dataset->col_cnt; c++) {
		memsize += oci_str_to_win_str(fp_pqfname(dataset->res, c), NULL, 0);
	}
	for(r = 0; r < dataset->row_cnt; r++) {
		for(c = 0; c < dataset->col_cnt; c++) {
			memsize += oci_str_to_win_str(fp_pqgetvalue(dataset->res, r, c), NULL, 0);
		}
	}

	dataset->buf = (TCHAR *)malloc(memsize * sizeof(TCHAR));
	dataset->col_data = (TCHAR **)malloc(r * c * sizeof(TCHAR *));
	dataset->cname = (TCHAR **)malloc(c * sizeof(TCHAR *));
	p = dataset->buf;

	for(c = 0; c < dataset->col_cnt; c++) {
		dataset->cname[c] = p;
		cnt = oci_str_to_win_str(fp_pqfname(dataset->res, c), p, memsize);
		p += cnt;
	}
	for(r = 0; r < dataset->row_cnt; r++) {
		for(c = 0; c < dataset->col_cnt; c++) {
			dataset->col_data[r * dataset->col_cnt + c] = p;
			cnt = oci_str_to_win_str(fp_pqgetvalue(dataset->res, r, c), p, memsize);
			p += cnt;
		}
	}

	return;
}

static int pg_get_result(HPgSession ss, PGresult **res,
	TCHAR *msg_buf, volatile int *cancel_flg, void *hWnd)
{
	int			ret_v;
	ExecStatusType exec_status;

	// 結果を受信
	ret_v = pg_recv_result(ss, msg_buf, cancel_flg, hWnd, 0);
	if(ret_v == PGERR_CANCEL) {
		if(fp_pqsetErrIgnoreFlg) {
			// 検索途中の結果を取得する
			fp_pqsetErrIgnoreFlg(ss->conn, 1);
			*res = fp_pqgetResult(ss->conn);
			fp_pqsetErrIgnoreFlg(ss->conn, 0);

			if(*res == NULL) {
				pg_err_msg_result(*res, msg_buf);
				pg_clean_result(ss);
				return 1;
			}

			pg_clean_result(ss);
			return PGERR_CANCEL;
		} else {
			pg_clean_result(ss);
			return 1;
		}
	}
	if(ret_v != 0) return 1;

	*res = fp_pqgetResult(ss->conn);
	if(*res == NULL) {
		pg_err_msg_result(*res, msg_buf);
		pg_clean_result(ss);
		return 1;
	}

	exec_status = fp_pqresultStatus(*res);
	if(exec_status != PGRES_COMMAND_OK && exec_status != PGRES_TUPLES_OK) {
		pg_err_msg_result(*res, msg_buf);
		pg_clean_result(ss);
		fp_pqclear(*res);
		return 1;
	}

#ifdef WIN32
	if(hWnd != NULL) PostMessage(hWnd, WM_OCI_DLG_ROW_CNT, 0, fp_pqntuples(*res));
#endif

	pg_clean_result(ss);

	return 0;
}

int pg_create_dataset_ex(HPgSession ss, const TCHAR *sql,
	TCHAR *msg_buf, volatile int *cancel_flg, void *hWnd,
	HPgDataset *result)
{
	PGresult	*res = NULL;
	ExecStatusType exec_status;
	int			ret_v;

	if(msg_buf != NULL) _tcscpy(msg_buf, _T(""));

#ifdef WIN32
//	if(hWnd != NULL) SendMessage(hWnd, WM_OCI_DLG_STATIC, 0, (LPARAM)PGMSG_DB_SEARCH_MSG);
#endif

	// SQLを送信
	if(pg_send_query(ss, sql, msg_buf) != 0) {
		return 1;
	}

	// データを保存
	ret_v = pg_get_result(ss, &res, msg_buf, cancel_flg, hWnd);
	if(ret_v != 0 && ret_v != PGERR_CANCEL) {
		return 1;
	}

	exec_status = fp_pqresultStatus(res);

	if(ret_v == 0) {
		if(exec_status == PGRES_COMMAND_OK) {
			// 検索結果がないSQLのとき
			if(msg_buf != NULL) {
				char *cmd_tuples = fp_pqcmdTuples(res);
				if(strlen(cmd_tuples) == 0) {
					_stprintf(msg_buf, PGMSG_SQL_OK);
				} else {
					TCHAR buf[1000];
					oci_str_to_win_str(cmd_tuples, buf, PG_ARRAY_SIZEOF(buf));
					_stprintf(msg_buf, PGMSG_NROW_OK_MSG, buf);
				}
			}
			fp_pqclear(res);
			*result = NULL;
		} else if(exec_status == PGRES_TUPLES_OK) {
			// 検索結果があるとき，datasetを作成する
			HPgDataset	dataset = NULL;
			dataset = pg_alloc_dataset(msg_buf);
			if(dataset == NULL) {
				fp_pqclear(res);
				return 1;
			}
			dataset->res = res;
			pg_after_select(dataset, msg_buf);
			*result = dataset;
			if(msg_buf != NULL) _stprintf(msg_buf, PGMSG_SELECT_MSG, dataset->row_cnt);

#ifdef WIN32
		if(hWnd != NULL) SendMessage(hWnd, WM_OCI_DLG_STATIC, 0, (LPARAM)PGMSG_DISP_SEARCH_RESULT_MSG);
#endif
		}
	} else {
		// キャンセルされたとき
		if(fp_pqntuples(res) == 0) {
			fp_pqclear(res);
			*result = NULL;
		} else {
			// 検索結果があるとき，datasetを作成する
			HPgDataset	dataset = NULL;
			dataset = pg_alloc_dataset(msg_buf);
			if(dataset == NULL) {
				fp_pqclear(res);
				return 1;
			}
			dataset->res = res;
			pg_after_select(dataset, msg_buf);
			*result = dataset;
		}
	}

	return 0;
}

HPgDataset pg_create_dataset(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf)
{
	HPgDataset	dataset = NULL;
	pg_create_dataset_ex(ss, sql, msg_buf, NULL, NULL, &dataset);
	return dataset;
}

int pg_dataset_row_cnt(HPgDataset dataset)
{
	return dataset->row_cnt;
}

int pg_dataset_col_cnt(HPgDataset dataset)
{
	return dataset->col_cnt;
}

static PGresult *get_result(HPgDataset dataset)
{
	if(dataset == NULL) return NULL;

	return dataset->res;
}

const TCHAR *pg_dataset_data(HPgDataset dataset, int row, int col)
{
//	return fp_pqgetvalue(dataset->res, row, col);
	return dataset->col_data[row * dataset->col_cnt + col];
}

size_t pg_dataset_len(HPgDataset dataset, int row, int col)
{
//	return fp_pqgetlength(dataset->res, row, col);
	return _tcslen(pg_dataset_data(dataset, row, col));
}

const TCHAR *pg_dataset_data2(HPgDataset dataset, int row, const TCHAR *colname)
{
	int col;
	ocichar cname[1000];

	win_str_to_oci_str(colname, cname, sizeof(cname));

	col = fp_pqfnumber(dataset->res, cname);
	return pg_dataset_data(dataset, row, col);
}

int pg_dataset_get_col_no(HPgDataset dataset, const TCHAR *colname)
{
	ocichar cname[1000];

	win_str_to_oci_str(colname, cname, sizeof(cname));

	return fp_pqfnumber(dataset->res, cname);
}

const TCHAR *pg_dataset_get_colname(HPgDataset dataset, int col)
{
	return dataset->cname[col];
}

int pg_dataset_get_colsize(HPgDataset dataset, int col)
{
	PGresult *res = get_result(dataset);
	if(res == NULL) return 0;

	// カラムサイズを取得
	return fp_pqfsize(res, col);
}

pg_oid pg_dataset_get_coltype(HPgDataset dataset, int col)
{
	PGresult *res = get_result(dataset);
	if(res == NULL) return 0;

	// カラムサイズを取得
	return fp_pqftype(res, col);
}

int pg_dataset_is_null(HPgDataset dataset, int row, int col)
{
	PGresult *res = get_result(dataset);
	return fp_pqgetisnull(res, row, col);
}

int pg_dataset_is_null2(HPgDataset dataset, int row, const TCHAR *colname)
{
	int col = pg_dataset_get_col_no(dataset, colname);
	return fp_pqgetisnull(dataset->res, row, col);
}

int pg_dataset_is_valid(HPgDataset dataset)
{
	if(dataset->res == NULL) return 0;
	return 1;
}

