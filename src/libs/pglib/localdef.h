/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _PGLIB_LOCAL_DEF_H_INCLUDE_
#define _PGLIB_LOCAL_DEF_H_INCLUDE_

#include <libpq-fe.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#include <tchar.h>

#define PG_ARRAY_SIZEOF(arr)   (sizeof(arr)/sizeof(arr[0]))

extern unsigned int g_oci_code_page;

typedef struct pg_session_st *		HPgSession;
typedef struct pg_session_st		PgSession;
struct pg_session_st {
	PGconn				*conn;
	PQnoticeProcessor	notice_proc;
	void				*notice_arg;
	int					remote_version;
	int					auto_commit_off;
	int					has_objsubid;

	TCHAR				*user;
	TCHAR				*host;
	TCHAR				*db;
	TCHAR				*port;
	TCHAR				*saved_connect_str;
};

typedef struct pg_dataset_st *		HPgDataset;
typedef struct pg_dataset_st		PgDataset;
struct pg_dataset_st {
	int				row_cnt;
	int				col_cnt;
	PGresult		*res;
	TCHAR			**col_data;
	TCHAR			**cname;
	TCHAR			*buf;
};

/* local functions */
/* pgutil.c */
void msleep(int msec);

/* pgsql.c */
void pg_clean_result(HPgSession ss);
//int pg_cancel(HPgSession ss, char *msg_buf);
int pg_send_query(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf);
int pg_recv_result(HPgSession ss, TCHAR *msg_buf, volatile int *cancel_flg, 
	void *hWnd, int clean_at_cancel);

/* pgerr.c */
void pg_err_msg(HPgSession ss, TCHAR *msg_buf);
void pg_err_msg_result(PGresult *res, TCHAR *msg_buf);

#include "pgapi.h"

#endif _PGLIB_LOCAL_DEF_H_INCLUDE_

