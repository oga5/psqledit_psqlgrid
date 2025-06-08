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
#include "apidef.h"

/*----------------------------------------------------------------------
 エラーメッセージを作成
----------------------------------------------------------------------*/
void pg_err_msg(HPgSession ss, TCHAR *msg_buf)
{
	char local_buf[2000];

	if(msg_buf == NULL) return;

	sprintf(local_buf, "%s", fp_pqerrorMessage(ss->conn));
	oci_str_to_win_str(local_buf, msg_buf, 512);
}

/*----------------------------------------------------------------------
 エラーメッセージを作成
----------------------------------------------------------------------*/
void pg_err_msg_result(PGresult *res, TCHAR *msg_buf)
{
	// FIXME: bufsizeを引数でもらうようにする
	char local_buf[4096];
	if(msg_buf == NULL) return;

	sprintf(local_buf, "%.*s", 4095, fp_pqresultErrorMessage(res));
	oci_str_to_win_str(local_buf, msg_buf, 2000);
}


