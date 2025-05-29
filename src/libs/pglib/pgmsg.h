#ifndef _PLIG_PG_MSG_H_INCLUDE_
#define _PLIG_PG_MSG_H_INCLUDE_


#define PGERR_MEM_ALLOC			100
#define PGERR_MEM_ALLOC_MSG		_T("memory allocation error.")

#define PGERR_OPEN_FILE			101
#define PGERR_OPEN_FILE_MSG		_T("ファイルを開けません(%s)。")

#define PGERR_CLOSE_FILE		102
#define PGERR_CLOSE_FILE_MSG	_T("ファイルのクローズに失敗しました。")

#define PGERR_LOAD_LIBPQ		103
#define PGERR_LOAD_LIBPQ_MSG	_T("libpq.dllが見つかりません")

#define PGERR_INIT_LIBPQ		104
#define PGERR_INIT_LIBPQ_MSG	_T("libpq.dllの初期化に失敗しました。\nlibpq.dllのバージョンが古い可能性があります。")

#define PGERR_FREE_LIBPQ		105
#define PGERR_FREE_LIBPQ_MSG	_T("libpq.dllの開放に失敗しました。")

#define PGERR_SOCKET_IS_NOT_OPEN	106
#define PGERR_SOCKET_IS_NOT_OPEN_MSG	_T("socket is not open.")

#define PGERR_SOCKET_SELECT		107
#define PGERR_SOCKET_SELECT_MSG	_T("socket select error.")


#define PGERR_CANCEL			110
#define PGERR_CANCEL_MSG		_T("キャンセルされました。")

#define PGERR_NOT_LOGIN			200
#define PGERR_NOT_LOGIN_MSG		_T("ログインできませんでした。")

#define PG_ERR_FATAL			199

#define PGMSG_SELECT_MSG		_T("%d件選択されました。")
#define PGMSG_NROW_OK_MSG		_T("%s行処理されました。")
#define PGMSG_SQL_OK			_T("SQLが実行されました。")

#define PGMSG_DB_SEARCH_MSG		_T("検索実行中")
#define PGMSG_DISP_SEARCH_RESULT_MSG	_T("検索結果表示中")

/* キャンセルダイアログ用WindowMessage */
#ifdef WIN32
#include <windows.h>
#define WM_OCI_DLG_EXIT			WM_USER + 100
#define WM_OCI_DLG_STATIC		WM_USER + 101
#define WM_OCI_DLG_ENABLE_CANCEL	WM_USER + 102
#define WM_OCI_DLG_ROW_CNT		WM_USER + 103
#endif

#endif _PLIG_PG_MSG_H_INCLUDE_

