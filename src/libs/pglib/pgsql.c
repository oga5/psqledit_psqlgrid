/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdio.h>

#include "localdef.h"
#include "pgapi.h"
#include "pgmsg.h"
#include "apidef.h"

#ifdef WIN32
#undef ERROR
#include <windows.h>
#endif

//#include <libpq-int.h>

// 非同期通知を受け取る
int pg_notice(HPgSession ss, TCHAR *msg_buf)
{
	PGnotify			*notify = NULL;
	char				buf[1024];

	fp_pqconsumeInput(ss->conn);

	for(;;) {
		notify = fp_pqnotifies(ss->conn);
		if(notify == NULL) break;

		if(ss->notice_proc != NULL) {
			sprintf(buf, "Asynchronous NOTIFY '%.900s' from backend with pid '%d' received.\n",
				notify->relname, notify->be_pid);
			ss->notice_proc(ss->notice_arg, buf);
		}
		
		fp_pqfreeNotify(notify);
	}

	return 0;
}

void pg_clean_result(HPgSession ss)
{
	PGresult	*res = NULL;

	for(;;) {
		res = fp_pqgetResult(ss->conn);
		if(res == NULL) break;
		fp_pqclear(res);
	}
}

static int pg_cancel(HPgSession ss, TCHAR *msg_buf)
{
	fp_pqrequestCancel(ss->conn);

//	pg_clean_result(ss);

	if(msg_buf != NULL) _stprintf(msg_buf, PGERR_CANCEL_MSG);
	return PGERR_CANCEL;
}

int pg_send_query(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf)
{
	char *sql_buf;
	int buf_size;

	buf_size = win_str_to_oci_str(sql, NULL, 0);
	sql_buf = (char *)malloc(buf_size);
	win_str_to_oci_str(sql, sql_buf, buf_size);

	// SQLを送信
	if(fp_pqsendQuery(ss->conn, sql_buf) == FALSE) {
		pg_err_msg(ss, msg_buf);
		free(sql_buf);
		return 1;
	}

	free(sql_buf);
	return 0;
}

static int pg_wait_result(HPgSession ss, TCHAR *msg_buf, volatile int *cancel_flg, 
	int clean_at_cancel)
{
	unsigned int sock = -1;
	fd_set		input_mask;
	struct timeval timeout;

	sock = fp_pqsocket(ss->conn);
	if(sock == -1) {
		if(msg_buf != NULL) _stprintf(msg_buf, PGERR_SOCKET_IS_NOT_OPEN_MSG);
		return PGERR_SOCKET_IS_NOT_OPEN;
	}

	for(;;) {
		if(cancel_flg != NULL) {
			// キャンセルダイアログの表示中は，処理を止める
			for(; *cancel_flg == 2;) msleep(500);
			// キャンセル時の処理
			if(*cancel_flg == 1) {
				int ret_v = pg_cancel(ss, msg_buf);
				if(clean_at_cancel) pg_clean_result(ss);
				return ret_v;
			}
		}

		FD_ZERO(&input_mask);
		FD_SET(sock, &input_mask);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10 * 1000;	// 10msec

		if(select(sock + 1, &input_mask, (fd_set *)NULL, (fd_set *)NULL, &timeout) < 0) {
			if(msg_buf != NULL) _stprintf(msg_buf, PGERR_SOCKET_SELECT_MSG);
			return PGERR_SOCKET_SELECT;
		}
		if(FD_ISSET(sock, &input_mask) == 1) break;
	}

	return 0;
}

int pg_recv_result(HPgSession ss, TCHAR *msg_buf, volatile int *cancel_flg, 
	void *hWnd, int clean_at_cancel)
{
	int		ret_v;
#ifdef WIN32
	int		next_post_msg_cnt = 50;		// GetTickCountを頻繁に実行しないためのカウンタ
	DWORD	tick_count = GetTickCount();
	int		count1 = 0;
	int		count2 = 0;
#endif

	// 結果を受信
	for(;;) {
		// 受信待ち
		ret_v = pg_wait_result(ss, msg_buf, cancel_flg, clean_at_cancel);
		if(ret_v != 0) return ret_v;

		// 結果を受け取る
		fp_pqconsumeInput(ss->conn);
		
		if(fp_pqisBusy(ss->conn) == 0) break;

#ifdef WIN32
		if(hWnd != NULL && fp_pqcurrentNtuples != NULL) {
			next_post_msg_cnt--;
			if(next_post_msg_cnt <= 0) {
				// 50msecに1回にする
				if(GetTickCount() - tick_count > 50) {
					PostMessage(hWnd, WM_OCI_DLG_ROW_CNT, 0, fp_pqcurrentNtuples(ss->conn));
					next_post_msg_cnt = 50;
					tick_count = GetTickCount();
					count2++;
				} else {
					next_post_msg_cnt = 50;
					count1++;
				}
			}
		}
#endif
	}

	return 0;
}

int pg_exec_sql_ex(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf,
	volatile int *cancel_flg)
{
	ExecStatusType exec_status;
	PGresult	*res = NULL;

	if(msg_buf != NULL) _tcscpy(msg_buf, _T(""));

	// SQLを送信
	if(pg_send_query(ss, sql, msg_buf) != 0) {
		return 1;
	}

	// 結果を受信
	if(pg_recv_result(ss, msg_buf, cancel_flg, NULL, 1) != 0) {
		return 1;
	}

	res = fp_pqgetResult(ss->conn);
	if(res == NULL) {
		pg_err_msg(ss, msg_buf);
		return 1;
	}
	
	exec_status = fp_pqresultStatus(res);
	if(exec_status != PGRES_COMMAND_OK && exec_status != PGRES_TUPLES_OK) {
		pg_err_msg_result(res, msg_buf);
		pg_clean_result(ss);
		fp_pqclear(res);
		return 1;
	}

	if(msg_buf != NULL) {
		char *cmd_tuples = fp_pqcmdTuples(res);
		if(strlen(cmd_tuples) == 0) {
			if(msg_buf != NULL) _stprintf(msg_buf, PGMSG_SQL_OK);
		} else {
			TCHAR buf[1000];
			oci_str_to_win_str(cmd_tuples, buf, PG_ARRAY_SIZEOF(buf));
			if(msg_buf != NULL) _stprintf(msg_buf, PGMSG_NROW_OK_MSG, buf);
		}
	}

	fp_pqclear(res);

	// 実行結果をクリーンにする(検索SQLではないので不要かもしれない)
	pg_clean_result(ss);

	return 0;
}

int pg_exec_sql(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf)
{
	return pg_exec_sql_ex(ss, sql, msg_buf, NULL);
}

