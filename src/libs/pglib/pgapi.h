/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _PGLIB_PG_API_H_INCLUDE_
#define _PGLIB_PG_API_H_INCLUDE_

#include <tchar.h>

typedef unsigned int pg_oid;

/* pgutil.c */
HPgSession pg_login(const TCHAR *connect_info, TCHAR *msg_buf);
void pg_logout(HPgSession ss);
const TCHAR *pg_user(HPgSession ss);
const TCHAR *pg_host(HPgSession ss);
const TCHAR *pg_db(HPgSession ss);
const TCHAR *pg_port(HPgSession ss);
const TCHAR *pg_explain_plan(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf);
int pg_get_remote_version(HPgSession ss);
int pg_has_objsubid(HPgSession ss);

typedef void (*pg_notice_processor) (void *arg, const char *message);
pg_notice_processor pg_set_notice_processor(HPgSession ss, pg_notice_processor proc, void *arg);

int pg_auto_commit_off(HPgSession ss, TCHAR *msg_buf);
int pg_rollback(HPgSession ss, TCHAR *msg_buf);
int pg_commit(HPgSession ss, TCHAR *msg_buf);

int pg_is_ssl_mode(HPgSession ss);

int pg_trans_is_idle(HPgSession ss);

extern unsigned int g_oci_code_page;
typedef unsigned char ocichar;
int oci_str_to_win_str(const ocichar *src, TCHAR *dst, int sz);
int win_str_to_oci_str(const TCHAR *src, ocichar *dst, int sz);

int pg_parameter_status(HPgSession ss, const TCHAR *paramName, TCHAR *buf, int buf_size,
	TCHAR *msg_buf);

/* pgerr.c */
void pg_err_msg(HPgSession ss, TCHAR *msg_buf);

/* pgsql.c */
int pg_exec_sql(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf);
int pg_exec_sql_ex(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf,
    volatile int *cancel_flg);
int pg_notice(HPgSession ss, TCHAR *msg_buf);

/* pgdataset.c */
int pg_create_dataset_ex(HPgSession ss, const TCHAR *sql,
	TCHAR *msg_buf, volatile int *cancel_flg, void *hWnd,
	HPgDataset *result);
HPgDataset pg_create_dataset(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf);
void pg_free_dataset(HPgDataset dataset);
int pg_dataset_row_cnt(HPgDataset dataset);
int pg_dataset_col_cnt(HPgDataset dataset);
const TCHAR *pg_dataset_data(HPgDataset dataset, int row, int col);
size_t pg_dataset_len(HPgDataset dataset, int row, int col);
const TCHAR *pg_dataset_data2(HPgDataset dataset, int row, const TCHAR *colname);
const TCHAR *pg_dataset_get_colname(HPgDataset dataset, int col);
int pg_dataset_get_colsize(HPgDataset dataset, int col);
pg_oid pg_dataset_get_coltype(HPgDataset dataset, int col);
int pg_dataset_get_col_no(HPgDataset dataset, const TCHAR *colname);
int pg_dataset_is_null(HPgDataset dataset, int row, int col);
int pg_dataset_is_null2(HPgDataset dataset, int row, const TCHAR *colname);
int pg_dataset_is_valid(HPgDataset dataset);

/* dsetutil.c */
int pg_save_dataset_csv(const TCHAR *path, HPgDataset dataset, int put_colname, TCHAR *msg_buf);
int pg_save_dataset_csv_fp(FILE *fp, HPgDataset dataset, int put_colname, TCHAR *msg_buf);

int pg_save_dataset_tsv(const TCHAR *path, HPgDataset dataset, int put_colname, TCHAR *msg_buf);

int pg_save_dataset_ex(const TCHAR *path, HPgDataset dataset, int put_colname, TCHAR *msg_buf,
	int col_cnt, TCHAR sepa);

/* winpg.c */
#ifdef WIN32
int pg_init_library(TCHAR *msg_buf);
int pg_free_library(TCHAR *msg_buf);
int pg_library_is_ok();
#endif

#endif _PGLIB_PG_API_H_INCLUDE_

