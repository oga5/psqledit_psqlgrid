/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "localdef.h"
#include "pgapi.h"
#include "pgmsg.h"
#include "apidef.h"

#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

static int _parse_version(const TCHAR *v)
{
	int		r = 0;
	int		cnt;
	int		vmaj, vmin, vrev;

	for(; !(*v >= '0' && *v <= '9'); v++) ;

	cnt = swscanf(v, _T("%d.%d.%d"), &vmaj, &vmin, &vrev);
	if(cnt < 2) return 0;
	if(cnt == 2) vrev = 0;
	
	r = (100 * vmaj + vmin) * 100 + vrev;

	return r;
}

static int _get_remote_version(HPgSession ss, TCHAR *msg_buf) 
{
	TCHAR	ver_str[1000];

	if(fp_pqserverversion != NULL) {
		ss->remote_version = fp_pqserverversion(ss->conn);
		return 0;
	}

	HPgDataset dataset = pg_create_dataset(ss, _T("select version()"), msg_buf);
	if(dataset == NULL) return 1;

	_stprintf(ver_str, _T("%.*s"), (int)(sizeof(ver_str)/sizeof(TCHAR)) - 1,
		pg_dataset_data(dataset, 0, 0));

	ss->remote_version = _parse_version(ver_str);

	pg_free_dataset(dataset);

	return 0;
}

static int _check_has_objsubid(HPgSession ss, TCHAR *msg_buf) 
{
/*
	HPgDataset dataset = pg_create_dataset(ss, _T("select * from pg_description"), msg_buf);
	if(dataset == NULL) return 1;

	if(pg_dataset_get_col_no(dataset, _T("objsubid")) < 0) {
		ss->has_objsubid = 0;
	} else {
		ss->has_objsubid = 1;
	}

	pg_free_dataset(dataset);

	return 0;
*/
	ss->has_objsubid = 1;
	return 0;
}

static void pg_init_session(HPgSession ss)
{
	ss->conn = NULL;
	ss->notice_arg = NULL;
	ss->notice_proc = NULL;
	ss->remote_version = 0;
	ss->auto_commit_off = 0;
	ss->has_objsubid = 0;

	ss->user = NULL;
	ss->host = NULL;
	ss->port = NULL;
	ss->db = NULL;
	ss->saved_connect_str = NULL;
}

static void pg_free_session(HPgSession ss)
{
	if(ss == NULL) return;
	if(ss->user != NULL) free(ss->user);
	if(ss->host != NULL) free(ss->host);
	if(ss->port != NULL) free(ss->port);
	if(ss->db != NULL) free(ss->db);
	if(ss->saved_connect_str != NULL) free(ss->saved_connect_str);
	free(ss);
}

int pg_auto_commit_off(HPgSession ss, TCHAR *msg_buf)
{
	ss->auto_commit_off = 1;
	
	pg_exec_sql(ss, _T("rollback"), NULL);
	return pg_exec_sql(ss, _T("begin"), msg_buf);
}

int pg_commit(HPgSession ss, TCHAR *msg_buf)
{
	int ret_v = pg_exec_sql(ss, _T("commit"), msg_buf);
	if(ret_v == 0 && ss->auto_commit_off) {
		return pg_exec_sql(ss, _T("begin"), msg_buf);
	}
	return ret_v;
}

int pg_rollback(HPgSession ss, TCHAR *msg_buf)
{
	int ret_v = pg_exec_sql(ss, _T("rollback"), msg_buf);
	if(ret_v == 0 && ss->auto_commit_off) {
		return pg_exec_sql(ss, _T("begin"), msg_buf);
	}
	return ret_v;
}

HPgSession pg_login(const TCHAR *connect_info, TCHAR *msg_buf)
{
	ocichar connect_info_buf[1000] = "";
	HPgSession	ss = NULL;

	win_str_to_oci_str(connect_info, connect_info_buf, sizeof(connect_info_buf)/2);

	ss = (HPgSession)malloc(sizeof(PgSession));
	if(ss == NULL) {
		if(msg_buf != NULL) _stprintf(msg_buf, PGERR_MEM_ALLOC_MSG);
		return NULL;
	}

	pg_init_session(ss);

	ss->conn = fp_pqconnectdb(connect_info_buf);
	if(ss->conn == NULL) {
		pg_free_session(ss);
		if(msg_buf != NULL) _stprintf(msg_buf, PGERR_MEM_ALLOC_MSG);
		return NULL;
	}

	if(fp_pqstatus(ss->conn) != CONNECTION_OK) {
		if(msg_buf != NULL) {
			char *msg = fp_pqerrorMessage(ss->conn);

			_stprintf(msg_buf, PGERR_NOT_LOGIN_MSG);
			if(msg != NULL) {
				TCHAR buf[1000];
				oci_str_to_win_str(msg, buf, PG_ARRAY_SIZEOF(buf));
				_tcscat(msg_buf, _T("\n"));
				_tcscat(msg_buf, buf);
			}
		}
		fp_pqfinish(ss->conn);
		pg_free_session(ss);
		return NULL;
	}

	pg_exec_sql(ss, _T("set client_encoding to 'UNICODE'"), msg_buf);

	_get_remote_version(ss, msg_buf);
	_check_has_objsubid(ss, msg_buf);

	{
		TCHAR buf[1000];
		oci_str_to_win_str(fp_pquser(ss->conn), buf, PG_ARRAY_SIZEOF(buf));
		ss->user = _tcsdup(buf);
		oci_str_to_win_str(fp_pqhost(ss->conn), buf, PG_ARRAY_SIZEOF(buf));
		ss->host = _tcsdup(buf);
		oci_str_to_win_str(fp_pqdb(ss->conn), buf, PG_ARRAY_SIZEOF(buf));
		ss->db = _tcsdup(buf);
		oci_str_to_win_str(fp_pqport(ss->conn), buf, PG_ARRAY_SIZEOF(buf));
		ss->port = _tcsdup(buf);

		ss->saved_connect_str = _tcsdup(connect_info);
	}

	return ss;
}

void pg_logout(HPgSession ss)
{
	if(ss == NULL) return;
	if(ss->conn != NULL) {
		fp_pqfinish(ss->conn);
	}
	pg_free_session(ss);
}

void msleep(int msec)
{
#ifdef WIN32
	Sleep(msec);
#else
	struct timeval sleeptime;

	sleeptime.tv_sec = msec / 1000;				// 秒
	sleeptime.tv_usec = (msec % 1000) * 1000;	// マイクロ秒

	select(0, 0, 0, 0, &sleeptime);
#endif
}

const TCHAR *pg_user(HPgSession ss)
{
	return ss->user;
}

const TCHAR *pg_host(HPgSession ss)
{
	return ss->host;
}

const TCHAR *pg_db(HPgSession ss)
{
	return ss->db;
}

const TCHAR *pg_port(HPgSession ss)
{
	return ss->port;
}

int pg_trans_is_idle(HPgSession ss)
{
	return (fp_pqtransactionstatus(ss->conn) == PQTRANS_IDLE);
}

#define MAX_EXPLAIN_LEN		1024 * 32
static void plan_notice_processor(void *arg, const char *message)
{
	TCHAR buf[MAX_EXPLAIN_LEN];

	oci_str_to_win_str(message, buf, PG_ARRAY_SIZEOF(buf));

	if(_tcslen((TCHAR *)arg) + _tcslen(buf) + 1 >= MAX_EXPLAIN_LEN) return;
	_tcscat((TCHAR *)arg, buf);
}

/*------------------------------------------------------------------------------
 SQL実行計画の取得
------------------------------------------------------------------------------*/
const TCHAR *pg_explain_plan(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf)
{
	int		ret_v;
	TCHAR	*plan = NULL;

	if(pg_get_remote_version(ss) >= 70300) {
		size_t		buf_size = 0;
		int			r;
		HPgDataset dset = pg_create_dataset(ss, sql, msg_buf);
		if(dset == NULL) goto ERR1;

		for(r = 0; r < pg_dataset_row_cnt(dset); r++) {
			buf_size += (_tcslen(pg_dataset_data(dset, r, 0)) + 1) * sizeof(TCHAR);
		}
		buf_size += sizeof(TCHAR);

		plan = (TCHAR *)malloc(buf_size);
		if(plan == NULL) {
			if(msg_buf != NULL) _stprintf(msg_buf, PGERR_MEM_ALLOC_MSG);
			goto ERR1;
		}
		_tcscpy(plan, _T(""));

		for(r = 0; r < pg_dataset_row_cnt(dset); r++) {
			_tcscat(plan, pg_dataset_data(dset, r, 0));
			_tcscat(plan, _T("\n"));
		}
		pg_free_dataset(dset);
	} else {
		plan = (TCHAR *)malloc(MAX_EXPLAIN_LEN * sizeof(TCHAR));
		if(plan == NULL) {
			if(msg_buf != NULL) _stprintf(msg_buf, PGERR_MEM_ALLOC_MSG);
			goto ERR1;
		}
		_tcscpy(plan, _T(""));

		fp_pqsetNoticeProcessor(ss->conn, plan_notice_processor, (void *)plan);
		ret_v = pg_exec_sql_ex(ss, sql, msg_buf, NULL);

		fp_pqsetNoticeProcessor(ss->conn, ss->notice_proc, ss->notice_arg);
		if(ret_v != 0) goto ERR1;
	}

	return plan;

ERR1:
	if(plan != NULL) free(plan);
	return NULL;
}

pg_notice_processor pg_set_notice_processor(HPgSession ss, 
	pg_notice_processor proc, void *arg)
{
	ss->notice_proc = proc;
	ss->notice_arg = arg;

	return fp_pqsetNoticeProcessor(ss->conn, proc, arg);
}

int pg_get_remote_version(HPgSession ss)
{
	return ss->remote_version;
}

int pg_has_objsubid(HPgSession ss)
{
	return ss->has_objsubid;
}

int pg_is_ssl_mode(HPgSession ss)
{
	if(fp_pqgetssl == NULL) return 0;
	if(fp_pqgetssl(ss->conn) == NULL) return 0;
	return 1;
}


///////////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------
  Oracleの文字コードから、Windowsの文字コードに変換する
----------------------------------------------------------------------*/
static int _oci_replace_wave_dash = 1;
unsigned int g_oci_code_page = CP_UTF8;

static int oci_str_to_win_str_normal(const ocichar *src, TCHAR *dst, int sz)
{
	return MultiByteToWideChar(g_oci_code_page, 0, src, -1, dst, sz);
}

static int oci_str_to_win_str_replace_wave_dash(const ocichar *src, TCHAR *dst, int sz)
{
	int r = MultiByteToWideChar(g_oci_code_page, 0, src, -1, dst, sz);

	// Unicode: 波ダッシュ(U+301C:Wave dash)を全角チルダ(U+FF5E:Fullwidth Tilde)に変換する
	if(dst) {
		TCHAR *p = dst;
		for(; *p; p++) {
			if(*p == 0x0301c) *p = 0xff5e;
		}
	}

	return r;
}

int (*oci_str_to_win_str_func)(const ocichar *src, TCHAR *dst, int sz) = oci_str_to_win_str_normal;

static void set_oci_str_to_win_str_func()
{
	oci_str_to_win_str_func = oci_str_to_win_str_normal;
	if(_oci_replace_wave_dash) {
		oci_str_to_win_str_func = oci_str_to_win_str_replace_wave_dash;
	}
}

void oci_set_replace_wave_dash(int flg)
{
	_oci_replace_wave_dash = flg;
	set_oci_str_to_win_str_func();
}

int oci_str_to_win_str(const ocichar *src, TCHAR *dst, int sz)
{
	if(src == NULL && sz > 0) {
		dst[0] = '\0';
		return 0;
	}
	return oci_str_to_win_str_func(src, dst, sz);
}

/*----------------------------------------------------------------------
  Windowsの文字コードから、Oracleの文字コードに変換する
----------------------------------------------------------------------*/
int win_str_to_oci_str(const TCHAR *src, ocichar *dst, int sz)
{
	return WideCharToMultiByte(g_oci_code_page, 0, src, -1, dst, sz, NULL, NULL);
}


int pg_parameter_status(HPgSession ss, const TCHAR *paramName, TCHAR *buf, int buf_size,
	TCHAR *msg_buf)
{
	char m_param_name[1000];
	const char *p;
	win_str_to_oci_str(paramName, m_param_name, PG_ARRAY_SIZEOF(m_param_name));

	p = fp_pqparameterstatus(ss->conn, m_param_name);
	if(p == NULL) {
		// FIXME: エラーにする？
		_tcscpy(buf, _T(""));
		return 0;
	}

	oci_str_to_win_str(p, buf, buf_size);
	return 0;
}
