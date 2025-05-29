/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // psqleditDoc.cpp : CPsqleditDoc クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "psqledit.h"

#include "psqleditDoc.h"
#include "pgmsg.h"
#include "localsql.h"
#include "fileutil.h"
#include "ostrutil.h"

#include "MainFrm.h"
#include "ChildFrm.h"

#include "CancelDlg.h"
#include "SQLLogDlg.h"
#include "SqlLogFileDlg.h"
#include "DownloadDlg.h"
#include "SqlListMaker.h"
#include "sqlparse.h"
#include "PlacesBarFileDlg.h"
#include "QueryCloseDlg.h"
#include "NameInputDlg.h"
#include "ShowCLobDlg.h"

#include "psql_util.h"
#include "macro.h"
#include "BindParamEditorDlg.h"

#include "SearchDlg.h"
#include "ReplaceDlg.h"
#include "GridFilterDlg.h"

#include <process.h>
#include <float.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPsqleditDoc

IMPLEMENT_DYNCREATE(CPsqleditDoc, CDocument)

BEGIN_MESSAGE_MAP(CPsqleditDoc, CDocument)
	//{{AFX_MSG_MAP(CPsqleditDoc)
	ON_COMMAND(ID_SQL_RUN, OnSqlRun)
	ON_COMMAND(ID_SWITCH_VIEW, OnSwitchView)
	ON_COMMAND(ID_CLEAR_RESULT, OnClearResult)
	ON_COMMAND(ID_SQL_LOG, OnSqlLog)
	ON_COMMAND(ID_FILE_SAVE_GRID_AS, OnFileSaveGridAs)
	ON_COMMAND(ID_SQL_RUN_SELECTED, OnSqlRunSelected)
	ON_UPDATE_COMMAND_UI(ID_SQL_RUN_SELECTED, OnUpdateSqlRunSelected)
	ON_UPDATE_COMMAND_UI(ID_SQL_RUN, OnUpdateSqlRun)
	ON_COMMAND(ID_EXPLAIN_RUN, OnExplainRun)
	ON_UPDATE_COMMAND_UI(ID_EXPLAIN_RUN, OnUpdateExplainRun)
	ON_COMMAND(ID_DOWNLOAD, OnDownload)
	ON_COMMAND(ID_MAKE_INSERT_SQL, OnMakeInsertSql)
	ON_COMMAND(ID_MAKE_SELECT_SQL, OnMakeSelectSql)
	ON_COMMAND(ID_MAKE_UPDATE_SQL, OnMakeUpdateSql)
	ON_COMMAND(ID_MAKE_INSERT_SQL_ALL, OnMakeInsertSqlAll)
	ON_COMMAND(ID_MAKE_SELECT_SQL_ALL, OnMakeSelectSqlAll)
	ON_COMMAND(ID_PSQLGRID, OnPsqlgrid)
	ON_UPDATE_COMMAND_UI(ID_PSQLGRID, OnUpdatePsqlgrid)
	ON_COMMAND(ID_PSQLGRID_OBJECTBAR, OnPsqlgridObjectbar)
	ON_COMMAND(ID_GRID_SWAP_ROW_COL, OnGridSwapRowCol)
	ON_UPDATE_COMMAND_UI(ID_GRID_SWAP_ROW_COL, OnUpdateGridSwapRowCol)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(ID_MAKE_SELECT_SQL, ID_MAKE_UPDATE_SQL, OnUpdateMakeSql)
	ON_COMMAND_RANGE(ID_SHORT_CUT_SQL1, ID_SHORT_CUT_SQL1 + MAX_SHORT_CUT_SQL, OnShortCutSql)
	ON_COMMAND(ID_PSQLGRID_SELECTED, &CPsqleditDoc::OnPsqlgrselected)
	ON_UPDATE_COMMAND_UI(ID_PSQLGRID_SELECTED, &CPsqleditDoc::OnUpdatePsqlgrselected)
	ON_COMMAND(ID_TAB_NAME, &CPsqleditDoc::OnTabName)
	ON_UPDATE_COMMAND_UI(ID_TAB_NAME, &CPsqleditDoc::OnUpdateTabName)
	ON_COMMAND(ID_GRID_FILTER_OFF, &CPsqleditDoc::OnGrfilterOff)
	ON_UPDATE_COMMAND_UI(ID_GRID_FILTER_OFF, &CPsqleditDoc::OnUpdateGrfilterOff)
	ON_COMMAND(ID_GRID_FILTER_ON, &CPsqleditDoc::OnGrfilterOn)
	ON_UPDATE_COMMAND_UI(ID_GRID_FILTER_ON, &CPsqleditDoc::OnUpdateGrfilterOn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsqleditDoc クラスの構築/消滅

CPsqleditDoc::CPsqleditDoc()
{
	m_sql_data.set_str_token(&g_sql_str_token);
	m_sql_data.set_undo_cnt(INT_MAX);

	m_msg_data.set_str_token(&g_sql_str_token);

	m_dataset = NULL;
	m_sql = NULL;
	m_sql_buf_size = 0;

	m_kanji_code = KANJI_CODE_SJIS;
	m_line_type = LINE_TYPE_CR_LF;

	m_no_sql_run = FALSE;

	m_grid_data_swap_row_col.SetGridData(&m_grid_data);
	m_grid_swap_row_col_mode = FALSE;

	m_split_editor_prev_active_row = 0;

	m_bind_param_editor_grid_cell_width[0] = 100;
	m_bind_param_editor_grid_cell_width[1] = 200;
}

CPsqleditDoc::~CPsqleditDoc()
{
	pg_free_dataset(m_dataset);

	FreeSqlBuf();
}

BOOL CPsqleditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	g_tab_bar.InsertDoc(this);

	DispFileType();

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CPsqleditDoc シリアライゼーション

void CPsqleditDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		SaveFile(ar);
	}
	else
	{
		LoadFile(ar);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPsqleditDoc クラスの診断

#ifdef _DEBUG
void CPsqleditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPsqleditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPsqleditDoc コマンド

static int sql_run(_thr_sql_run_st *sql_run_st);

static int file_sql_run(_thr_sql_run_st *sql_run_st, const TCHAR *file_name)
{
	CPsqleditDoc *pdoc = sql_run_st->pdoc;
	HWND hWnd = sql_run_st->hWnd;

	CEditData edit_data;

	edit_data.set_str_token(&g_sql_str_token);
	if(edit_data.load_file(file_name, g_msg_buf) != 0) {
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SWITCH_VIEW, MSG_VIEW);
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)g_msg_buf);
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_SEPALATOR, 0);
		return 1;
	}

	_thr_sql_run_st st;

	// 初期化
	st.pdoc = sql_run_st->pdoc;
	st.edit_data = &edit_data;
	st.start_row = 0;
	st.end_row = 0;
	st.hWnd = sql_run_st->hWnd;
	st.ret_v = 0;
	st.obj_update_flg = sql_run_st->obj_update_flg;
	st.cancel_flg = sql_run_st->cancel_flg;

	st.ar = sql_run_st->ar;
	st.sql_cnt = sql_run_st->sql_cnt;

	return sql_run(&st);
}

static int command_run(_thr_sql_run_st *sql_run_st, const TCHAR *command)
{
	CPsqleditDoc* pdoc = sql_run_st->pdoc;
	HWND hWnd = sql_run_st->hWnd;
	int ret_v = 0;

	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SWITCH_VIEW, MSG_VIEW);
	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);

	ret_v = do_command(g_ss, pdoc, command, &g_sql_str_token, g_msg_buf, hWnd, sql_run_st);

	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);

	// errorのとき
	if(ret_v != 0 && ret_v != PGERR_CANCEL) {
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)_T("-- Error\n"));
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)g_msg_buf);
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_SEPALATOR, 0);
	}

return ret_v;
}

static void ShowError(CPsqleditDoc* pdoc, HWND hWnd, int row_offset, int ret_v)
{
	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_ERROR_INFO, row_offset);

	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);
	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)g_msg_buf);
	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)_T("\n"));

	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_SEPALATOR, 0);
	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_REDRAW, 0);
}

static void sql_run_notice_processor(void* arg, const char* message)
{
	TCHAR buf[1024 * 32];
	oci_str_to_win_str((const unsigned char*)message, buf, ARRAY_SIZEOF(buf));

	TRACE(buf);

	if(arg == NULL) return;

	HWND hWnd = (HWND)arg;

	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SWITCH_VIEW, MSG_VIEW);
	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);
	SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)buf);
}

static void make_bind_err_msg(const TCHAR* sql, const TCHAR* err_p, const TCHAR* bind_param, TCHAR* msg_buf)
{
	int				row = 1;
	int				col = 1;
	const TCHAR* p = sql;

	for(; p < err_p; p++) {
		col++;
		if(*p == '\n') {
			row++;
			col = 1;
		}
	}

	_stprintf(msg_buf, _T("[row:%d,col:%d] 変数\"%s\"が宣言されていません。"),
		row, col, bind_param);
}

static int check_bind_param(SQL_KIND sql_kind, const TCHAR* sql, CSQLStrToken* str_token,
	CMapStringToString* bind_data, TCHAR* msg_buf)
{
	//	if(g_option.sql.check_bind_params == FALSE) return 0;

	const TCHAR* p;
	TCHAR		word_buf[1024 * 32];
	CString		bind_name;

	for(p = sql; p != NULL;) {
		p = str_token->skipCommentAndSpace(p);
		if(p == NULL) break;

		if(!str_token->isBindParam(p)) {
			int len = str_token->get_word_len(p, FALSE);
			p = p + len;
			continue;
		}

		p = str_token->get_word_for_bind(p, word_buf, sizeof(word_buf), bind_name);

		CString	tmp_value;
		if(bind_data->Lookup(bind_name, tmp_value) == FALSE) {
			p = p - _tcslen(word_buf);
			make_bind_err_msg(sql, p, bind_name, msg_buf);

			return 1;
		}
	}

	return 0;
}

static CString do_bind_sql(const TCHAR* sql, CSQLStrToken* str_token,
	CMapStringToString* bind_data)
{
	CEditData tmp_sql_data;
	TCHAR		word_buf[1024 * 32];
	const TCHAR* p;
	CString result;
	CString		bind_name;

	if(bind_data->GetCount() == 0) return _T("");

	for(p = sql; p != NULL;) {
		p = str_token->get_word_for_bind(p, word_buf, sizeof(word_buf), bind_name);

		if(str_token->isBindParam(word_buf)) {
			CString	tmp_value;
			if(bind_data->Lookup(bind_name, tmp_value)) {
				if(word_buf[1] == '\'') {
					tmp_value.Replace(_T("'"), _T("''"));
					tmp_sql_data.paste_no_undo(_T("'"));
					tmp_sql_data.paste_no_undo(tmp_value);
					tmp_sql_data.paste_no_undo(_T("'"));
				} else {
					tmp_sql_data.paste_no_undo(tmp_value);
				}
				continue;
			}
		}

		tmp_sql_data.paste_no_undo(word_buf);
	}

	result = tmp_sql_data.get_all_text();

	return result;
}

static void RestoreBindParamEditorGridCellWidth(CBindParamEditorDlg& dlg, int* m_bind_param_editor_grid_cell_width)
{
	if(m_bind_param_editor_grid_cell_width[0] < 50) m_bind_param_editor_grid_cell_width[0] = 50;
	if(m_bind_param_editor_grid_cell_width[1] < 50) m_bind_param_editor_grid_cell_width[1] = 50;

	dlg.m_bind_param_editor_grid_cell_width[0] = m_bind_param_editor_grid_cell_width[0];
	dlg.m_bind_param_editor_grid_cell_width[1] = m_bind_param_editor_grid_cell_width[1];
}

static void SaveBindParamEditorGridCellWidth(CBindParamEditorDlg& dlg, int* m_bind_param_editor_grid_cell_width)
{
	m_bind_param_editor_grid_cell_width[0] = dlg.m_bind_param_editor_grid_cell_width[0];
	m_bind_param_editor_grid_cell_width[1] = dlg.m_bind_param_editor_grid_cell_width[1];
}

int CPsqleditDoc::BindParamEdit()
{
	CBindParamEditorDlg	dlg;

	dlg.m_str_token = &g_sql_str_token;
	dlg.m_sql = GetSkipSpaceSql();
	dlg.m_bind_data_org = GetBindData();
	dlg.m_bind_data_tmp = GetBindDataTmp();

	RestoreBindParamEditorGridCellWidth(dlg, m_bind_param_editor_grid_cell_width);

	if(dlg.DoModal() != IDOK) {
		SaveBindParamEditorGridCellWidth(dlg, m_bind_param_editor_grid_cell_width);
		return 1;
	}

	SaveBindParamEditorGridCellWidth(dlg, m_bind_param_editor_grid_cell_width);
	return 0;
}

static int sql_run_main(CPsqleditDoc* pdoc, _thr_sql_run_st* sql_run_st, HWND hWnd)
{
	int ret_v;
	CMapStringToString* bind_data = pdoc->GetBindData();
	const TCHAR* psql = pdoc->GetSqlBuf();

	ret_v = check_bind_param(pdoc->GetSqlKind(), psql, &g_sql_str_token,
		bind_data, g_msg_buf);
	if(ret_v != 0) {
		ret_v = (int)SendMessage(hWnd, WM_DISP_BIND_PARAM_EDITOR, NULL, NULL);
		if(ret_v != 0) return ret_v;

		bind_data = pdoc->GetBindDataTmp();
	}

	CString bind_sql = do_bind_sql(psql, &g_sql_str_token, bind_data);
	if(!bind_sql.IsEmpty()) {
		psql = bind_sql.GetBuffer(0);
	}

	// autocommit offのとき、ここでbeginを発行する
	// beginを発行するか判定するため、src/bin/psql/common.cのcommand_no_beginを移植
	ret_v = 0;
	if(!g_option.sql.auto_commit &&
		pg_trans_is_idle(g_ss) &&
		!command_no_begin(psql)) {
		ret_v = pg_exec_sql(g_ss, _T("BEGIN"), g_msg_buf);
	}

	if(ret_v == 0) {
		if(sql_run_st->ar == NULL || pdoc->GetSqlKind() != SQL_SELECT) {
			// 検索SQL以外もget_datasetで実行する
			// データベース検索を実行
			HPgDataset dataset = NULL;

			ret_v = get_dataset(g_ss, psql, g_msg_buf,
				hWnd, sql_run_st->cancel_flg, &dataset);

			if(ret_v == 0) {
				if(pdoc->GetSqlKind() == SQL_DDL) {
					*(sql_run_st->obj_update_flg) = TRUE;
				}

				if(dataset != NULL) {
					pdoc->SetSqlKind(SQL_SELECT);
					pdoc->SetDataset(dataset, hWnd);
					SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_GRID_DATASET, 0);
					SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_GRID_SQL, (LPARAM)psql);
					SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_GRID_MSG, (LPARAM)g_msg_buf);
				} else {
					pdoc->SetSqlKind(SQL_NORMAL);
				}
			}
		} else {
			if(pdoc->GetMsg()->is_logging() == TRUE) {
				// SQLも保存するとき，SQLと検索結果の間に１行空ける
				sql_run_st->ar->WriteNextLine();
			} else if(pdoc->GetMsg()->is_logging() == FALSE && sql_run_st->sql_cnt != 0) {
				// 検索結果のみ保存するとき，検索結果の間に１行空ける
				if(sql_run_st->ar->GetPosition() != 0) {
					sql_run_st->ar->WriteNextLine();
				}
			}
			// FIXME: psqlをconst TCHAR*で渡せるようにする
			ret_v = download(g_ss, sql_run_st->ar, (TCHAR*)psql, g_msg_buf,
				g_option.download.put_column_name, hWnd, sql_run_st->cancel_flg);
		}
	}

	return ret_v;
}

static int sql_run(_thr_sql_run_st *sql_run_st)
{
	CPsqleditDoc *pdoc = sql_run_st->pdoc;
	int start_row = sql_run_st->start_row;
	int end_row = sql_run_st->end_row;
	HWND hWnd = sql_run_st->hWnd;

	int				ret_v = 0;
	int				row;
	int				row_offset;
	int				sql_cnt = 0;

	pg_notice_processor org_notice_processor = pg_set_notice_processor(g_ss, 
		sql_run_notice_processor, hWnd);

	if(start_row == 0 && end_row == 0) {
		start_row = 0;
		end_row = sql_run_st->edit_data->get_row_cnt();
	}

	for(row = start_row; row < end_row;) {
		row = pdoc->SkipNullRow(row, sql_run_st->edit_data);
		if(row >= end_row) break;

		row_offset = row;
		row = pdoc->GetSQL(row, sql_run_st->edit_data);

		const TCHAR	*skip_space_p;
		skip_space_p = g_sql_str_token.skipCommentAndSpace(pdoc->GetSqlBuf());

		// sqlが空のときは実行しない
		if(skip_space_p == NULL || *skip_space_p == '\0') continue;

		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_STATUS_BAR_MSG, (LPARAM)_T(""));

		{
			if(pdoc->GetSqlKind() == SQL_COMMAND) {
				ret_v = command_run(sql_run_st, skip_space_p);
				if(ret_v == 0) continue;
				if(ret_v == PGERR_CANCEL) break;
				if(g_option.sql.ignore_sql_error == TRUE) {
					continue;
				} else {
					break;
				}
			}

			if(pdoc->GetSqlKind() == SQL_FILE_RUN) {
				ret_v = file_sql_run(sql_run_st, (skip_space_p + 1));
				if(ret_v == 0) continue;
				if(ret_v == PGERR_CANCEL) break;
				// errorのとき
				if(g_option.sql.ignore_sql_error == TRUE) {
					continue;
				} else {
					break;
				}
			}
		}

		sql_cnt++;
		TCHAR sql_msg[512];
		_stprintf(sql_msg, _T("SQL実行中(%d)"), sql_cnt);
		SendMessage(hWnd, WM_OCI_DLG_STATIC, 0, (LPARAM)sql_msg);

		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SWITCH_VIEW, MSG_VIEW);
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)(pdoc->GetSqlBuf()));
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)_T("\n"));

//		int		sql_type;
		DWORD	tick_count;

		tick_count = GetTickCount();
		if(g_login_flg == FALSE) {
			ret_v = 1;
			_stprintf(g_msg_buf, _T("データベースに接続していません"));
		} else if(pdoc->GetSqlKind() == SQL_EXPLAIN) {
			CString plan = explain_plan(g_ss, pdoc->GetSqlBuf(), g_msg_buf);

			SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);
			SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)_T("----- 実行計画 -----\n"));
			SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)plan.GetBuffer(0));
		} else {
			ret_v = sql_run_main(pdoc, sql_run_st, hWnd);
		}
		tick_count = GetTickCount() - tick_count;
		(sql_run_st->sql_cnt)++;

		if(*(sql_run_st->cancel_flg) == 1) {
			ret_v = PGERR_CANCEL;
			_stprintf(g_msg_buf, PGERR_CANCEL_MSG);
		}

		if(ret_v != 0 && ret_v != PGERR_CANCEL) {
			ShowError(pdoc, hWnd, row_offset, ret_v);
			if(g_option.sql.ignore_sql_error == TRUE) {
				continue;
			} else {
				break;
			}
		}

		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SET_RESULT_CARET, 0);
		
		if(pdoc->GetSqlKind() != SQL_EXPLAIN) {
			CString		time_msg;
			time_msg.Format(_T("%s(%ld msec.)"), g_msg_buf, tick_count);
			SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_RESULT, (LPARAM)time_msg.GetBuffer(0));
		}
		SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_SEPALATOR, 0);

		if(pdoc->GetSqlKind() == SQL_SELECT && sql_run_st->ar == NULL) {
			SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_PUT_STATUS_BAR_MSG, (LPARAM)g_msg_buf);
			if(ret_v == 0) {
				SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_SWITCH_VIEW, GRID_VIEW);
			}
		}

		if(ret_v == PGERR_CANCEL) break;

		// SQLのログを保存
		g_sql_log_array->Add(pdoc->GetSqlBuf());
		if(g_option.sql_logging) {
			g_sql_logger->Save(pg_user(g_ss), pg_db(g_ss), pg_host(g_ss), pg_port(g_ss),
				pdoc->GetSqlBuf(), g_msg_buf);
		}
	}

	sql_run_st->ret_v = ret_v;

	// 非同期通知を受信
	if(g_option.sql.receive_async_notify) {
		pg_notice(g_ss, g_msg_buf);
	}

	pg_set_notice_processor(g_ss, org_notice_processor, NULL);

	return sql_run_st->ret_v;
}

static unsigned int _stdcall sql_run_thr(void *lpvThreadParam)
{
	_thr_sql_run_st *sql_run_st = (_thr_sql_run_st *)lpvThreadParam;

	sql_run_st->ret_v = sql_run(sql_run_st);

	if(sql_run_st->hWnd != NULL) {
		SendMessage(sql_run_st->hWnd, WM_OCI_DLG_ENABLE_CANCEL, FALSE, 0);
		PostMessage(sql_run_st->hWnd, WM_OCI_DLG_EXIT, sql_run_st->ret_v, 0);
	}

	return sql_run_st->ret_v;
}

void CPsqleditDoc::CheckModified()
{
	if(IsModified()) {
		if(m_sql_data.is_edit_data() == FALSE) {
			SetModifiedFlag(FALSE);
			SetTitle(GetTitle().Left(GetTitle().GetLength() - 2));
		}
	} else {
		if(m_sql_data.is_edit_data()) {
			SetModifiedFlag(TRUE);
			SetTitle(GetTitle() + " *");
		}
	}
}

int CPsqleditDoc::CopyClipboard(TCHAR * str)
{
	HGLOBAL hData = GlobalAlloc(GHND, (_tcslen(str) + 1) * sizeof(TCHAR));
	TCHAR *pstr = (TCHAR *)GlobalLock(hData);

	_tcscpy(pstr, str);

	GlobalUnlock(hData);

	if ( !OpenClipboard(NULL) ) {
		AfxMessageBox( _T("Cannot open the Clipboard") );
		return 1;
	}
	// Remove the current Clipboard contents
	if( !EmptyClipboard() ) {
		AfxMessageBox( _T("Cannot empty the Clipboard") );
		return 1;
	}
	// ...
	// Get the currently selected data
	// ...
	// For the appropriate data formats...
	if ( SetClipboardData( CF_UNICODETEXT, hData ) == NULL ) {
		AfxMessageBox( _T("Unable to set Clipboard data") );
		CloseClipboard();
		return 1;
	}
	// ...
	CloseClipboard();

	return 0;
}

BOOL CPsqleditDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	UpdateAllViews(NULL, UPD_REDRAW);

	g_tab_bar.InsertDoc(this);

	return TRUE;
}

void CPsqleditDoc::SetTitle(LPCTSTR lpszTitle) 
{
	CDocument::SetTitle(lpszTitle);

	g_tab_bar.SetTabName(this);
}

void CPsqleditDoc::OnCloseDocument() 
{
	g_tab_bar.DeleteDoc(this);
	
	CDocument::OnCloseDocument();
}

BOOL CPsqleditDoc::AllocSqlBuf(int buf_size)
{
	if(buf_size < m_sql_buf_size) return TRUE;

	FreeSqlBuf();

	int new_buf_size = 1024 * 4;
	for(; new_buf_size < buf_size;) {
		new_buf_size *= 2;
	}
	m_sql = (TCHAR *)malloc(new_buf_size);
	if(m_sql == NULL) {
		m_sql_buf_size = 0;
		return FALSE;
	}
	m_sql_buf_size = new_buf_size;

	return TRUE;
}

void CPsqleditDoc::FreeSqlBuf()
{
	if(m_sql != NULL) {
		free(m_sql);
		m_sql = NULL;
		m_sql_buf_size = 0;
	}
}

int CPsqleditDoc::SkipNullRow(int start_row, CEditData *edit_data)
{
	int		i;
	int		loop_cnt;
	int		row;

	if(edit_data == NULL) edit_data = GetSqlData();

	// 先頭の空行を読み飛ばす
	loop_cnt = edit_data->get_row_cnt() - start_row;
	for(i = 0, row = start_row; i < loop_cnt; i++, row++) {
		if(edit_data->get_row_len(row) != 0) break;
	}

	return row;
}

void CPsqleditDoc::DispFileType()
{
	g_file_type = CUnicodeArchive::MakeFileType(m_kanji_code, m_line_type);
}

int CPsqleditDoc::LoadFile(CArchive &ar, int kanji_code)
{
	m_kanji_code = kanji_code;
	m_sql_data.load_file(ar, m_kanji_code, m_line_type);

	DispFileType();

	return 0;
}

int CPsqleditDoc::SaveFile(CArchive &ar)
{
	m_sql_data.save_file(ar, m_kanji_code, m_line_type);

	return 0;
}

BOOL CPsqleditDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty()) {
		CDocTemplate* pTemplate = GetDocTemplate();
		ASSERT(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty()) {
			newName = m_strTitle;
#ifndef _MAC
			// check for dubious filename
			int iBad = newName.FindOneOf(_T(" #%;/\\"));
#else
			int iBad = newName.FindOneOf(_T(":"));
#endif
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

#ifndef _MAC
			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
			  !strExt.IsEmpty()) {
				ASSERT(strExt[0] == '.');
				newName += strExt;
			}
#endif
		}

		if (!((CPsqleditApp *)AfxGetApp())->DoPromptFileName(newName,
		  bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
		  OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, FALSE, pTemplate))
			return FALSE;       // don't even attempt to save
	}

	CWaitCursor wait;

	if (!OnSaveDocument(newName)) {
		if (lpszPathName == NULL) {
			if(_wunlink(newName.GetBuffer(0)) != 0) {
				TRACE0("Warning: failed to delete file after failed SaveAs.\n");
			}
		}
		return FALSE;
	}
	m_sql_data.clear_cur_operation();
	UpdateAllViews(NULL, UPD_INVALIDATE);

	// reset the title and change the document name
	if (bReplace)
		SetPathName(newName);

	return TRUE;        // success
}

int CPsqleditDoc::GetSQL(int start_row, CEditData *edit_data)
{
	POINT sql_start_pt;
	POINT sql_end_pt;

	if(edit_data == NULL) edit_data = GetSqlData();

	m_sql_kind = SQL_NORMAL;
	int loop_cnt = edit_data->get_row_cnt() - start_row;
	const TCHAR *sql_start_p = NULL;
	BOOL semicoron_flg = FALSE;
	BOOL in_quote = FALSE;
	BOOL in_comment = FALSE;
	BOOL in_row_comment = FALSE;
	BOOL sql_end_flg = FALSE;
	int row = start_row;
	TCHAR quote_end_str[100];
	int quote_end_str_len;
	int i;

	sql_start_pt.y = row;
	sql_start_pt.x = 0;
	sql_end_pt.y = -1;
	sql_end_pt.x = -1;

	for(i = 0, row = start_row; i < loop_cnt; i++, row++) {
		if(edit_data->get_row_len(row) == 0) continue;

		const TCHAR *row_start_p = edit_data->get_row_buf(row);
		const TCHAR *p = row_start_p;

		for(; *p != '\0'; p += get_char_len(p)) {
			unsigned int cur_ch = get_char(p);
			if(cur_ch == ' ' || cur_ch == '\t') continue;

			if(in_quote) {
				if(cur_ch == quote_end_str[0]) {
					if(quote_end_str_len == 1 || _tcsncmp(p, quote_end_str, quote_end_str_len) == NULL) {
						in_quote = FALSE;
						p += quote_end_str_len - 1;
						continue;
					}
				}
				continue;
			}

			if(in_comment) {
				if(cur_ch == '*' && p[1] == '/') {
					in_comment = FALSE;
					p += get_char_len(p);
					continue;
				}
				continue;
			}
			if(cur_ch == '/' && p[1] == '*') {
				in_comment = TRUE;
				p += get_char_len(p);
				continue;
			}
			if(cur_ch == '-' && p[1] == '-') {
				in_row_comment = TRUE;
				break;
			}

			if(sql_start_p == NULL) {
				sql_start_p = p;
				sql_start_pt.y = row;
				sql_start_pt.x = (p - row_start_p);

				// ファイルコマンド
				if(sql_start_p[0] == '@') {
					m_sql_kind = SQL_FILE_RUN;
					break;
				}
				// コマンド
				if(g_sql_str_token.isCommand(sql_start_p)) {
					if(_tcsncmp(sql_start_p, _T("\\set"), 4) == 0) sql_end_flg = TRUE;

					m_sql_kind = SQL_COMMAND;
					break;
				}
			}

			if(cur_ch == '\'' || cur_ch == '"') {
				in_quote = TRUE;
				quote_end_str[0] = cur_ch;
				quote_end_str[1] = '\0';
				quote_end_str_len = 1;
				continue;
			}
			if(cur_ch == '$' && !inline_isdigit(*(p + 1))) {
				const TCHAR *next_doller_p = _tcschr(p + 1, '$');
				if(next_doller_p) {
					quote_end_str_len = (next_doller_p - p) + 1;
					in_quote = TRUE;
					_tcsncpy(quote_end_str, p, quote_end_str_len);
					quote_end_str[quote_end_str_len] = '\0';
					p = next_doller_p;
					continue;
				}
			}

			if(cur_ch == ';') {
				sql_end_pt.y = row;
				sql_end_pt.x = (p - row_start_p);
				semicoron_flg = TRUE;
				break;
			}
		}
		if(in_row_comment) {
			in_row_comment = FALSE;
			continue;
		}
		if(semicoron_flg || sql_end_flg) break;
	}

	if(sql_end_pt.y == -1) {
		if(row == edit_data->get_row_cnt()) {
			sql_end_pt.y = row - 1;
		} else {
			sql_end_pt.y = row;
		}
		sql_end_pt.x = edit_data->get_row_len(sql_end_pt.y);
	}

	if(row != edit_data->get_row_cnt()) row++;

	int buf_size = edit_data->calc_buf_size(&sql_start_pt, &sql_end_pt);
	if(!AllocSqlBuf(buf_size)) return row;

	edit_data->copy_text_data(&sql_start_pt, &sql_end_pt, m_sql, buf_size);

	if(g_sql_str_token.isSelectSQL(m_sql)) {
		m_sql_kind = SQL_SELECT;
	} else if(g_sql_str_token.isExplainSQL(m_sql)) {
		m_sql_kind = SQL_EXPLAIN;
	} else if(g_sql_str_token.isDDLSQL(m_sql)) {
		m_sql_kind = SQL_DDL;
	}

	return row;
}

/*
NOTE: この処理は今後以下のようにしたい
	まず、SQLの終了位置を特定する, バッファサイズも計算
	メモリ確保
	SQLの開始位置から終了位置までバッファにコピー
#pragma intrinsic(strcpy, strcmp, strlen)
int CPsqleditDoc::GetSQL(int start_row, CEditData *edit_data)
{


	int		i;
	int		loop_cnt;
	int		row;
	int		alloc_cnt = 2048;
	int		sql_len = 0;
	TCHAR	*p;
	const TCHAR	*skip_space_p;
	int		semicolon_col;

	int		pl_sql_flg = 0;
	CString	pl_sql_sepa;
	BOOL	in_quote = FALSE;

	if(edit_data == NULL) edit_data = GetSqlData();

	// FIXME: 毎回free->mallocを実行しないようにする
	FreeSqlBuf();
	m_sql_kind = SQL_NORMAL;
	m_sql = (TCHAR *)malloc(alloc_cnt * sizeof(TCHAR));
	if(m_sql == NULL) {
		// FIXME: ERROR処理追加
		return start_row;
	}
	m_sql[0] = '\0';
	p = m_sql;

	loop_cnt = edit_data->get_row_cnt() - start_row;

	for(i = 0, row = start_row; i < loop_cnt; i++, row++) {
		// '/'だけの行がきたら，SQLの区切りと判断
		// FIXME: コメント中の'/'がSQLの区切りにならないようにする
		if(_tcscmp(edit_data->get_row_buf(row), _T("/")) == 0 &&
			g_sql_str_token.skipCommentAndSpace(m_sql) != NULL) break;

		// バッファサイズを計算
		if(i != 0) sql_len++;	// 改行分
		sql_len = sql_len + edit_data->get_row_len(row);

		// バッファを拡張
		if(sql_len >= alloc_cnt) {
			for(; sql_len >= alloc_cnt;) alloc_cnt = alloc_cnt * 2;
			
			// バグ回避のため，末尾にスペースを追加できるように，1バイト余分に取得する
			m_sql = (TCHAR *)realloc(m_sql, (alloc_cnt + 1) * sizeof(TCHAR));
			if(m_sql == NULL) {
				// FIXME: ERROR処理追加
				return row;
			}
			p = m_sql + _tcslen(m_sql);
		}

		// SQLを作成
		if(i != 0) {
			_tcscpy(p, _T("\n"));	// 改行を追加
			p++;
		}
		_tcscpy(p, edit_data->get_row_buf(row));
		p = p + edit_data->get_row_len(row);

		if(edit_data->get_row_len(row) == 0) continue;

		if(pl_sql_flg == 1) {
			if(_tcsstr(edit_data->get_row_buf(row), pl_sql_sepa) == NULL) {
				continue;
			}
			pl_sql_flg = 2;
		}

		// コメントを読み飛ばす
		// FIXME: ここの実装はあまり効率がよくない？ 毎回m_sqlを先頭からよんでいる O(n^2)
		skip_space_p = g_sql_str_token.skipCommentAndSpace(m_sql);
		if(skip_space_p == NULL) continue;

		// コマンドの処理
		{
			// ファイルコマンド
			if(*skip_space_p == '@') {
				m_sql_kind = SQL_FILE_RUN;
				break;
			}

			// コマンド
			if(g_sql_str_token.isCommand(skip_space_p)) {
				m_sql_kind = SQL_COMMAND;
				break;
			}
		}

		// 末尾に ';'がきたときの処理

		// FIXME: DOを使った無名functionの実行に対応する
		// ドル引用符による文字列の扱いを実装する
		if(is_sql_end(edit_data, row, &semicolon_col, in_quote)) {
			if(pl_sql_flg == 0 && g_sql_str_token.isPLSQL(skip_space_p, NULL, NULL)) {
				pl_sql_flg = 1;

				int			  word_len;
				const TCHAR *p = skip_space_p;
				for(; *p;) {
					p = g_sql_str_token.skipCommentAndSpace(p);
					word_len = g_sql_str_token.get_word_len(p);
					if(word_len != 2) {
						p += word_len;
						continue;
					}

					if(ostr_strncmp_nocase(p, _T("AS"), 2) == 0) {
						p += 2;
						p = g_sql_str_token.skipCommentAndSpace(p);
						if(*p == '\'') {
							pl_sql_sepa = _T("'");
						} else {
							word_len = g_sql_str_token.get_word_len(p);
							g_sql_str_token.get_word(p, 
								pl_sql_sepa.GetBuffer(word_len + 1),
								word_len + 1);
							pl_sql_sepa.ReleaseBuffer();
						}
						break;
					}
					p += word_len;
				}
				if(pl_sql_sepa == "'") {
					// 末尾の';'を取り除く
					m_sql[sql_len - (edit_data->get_row_len(row) - semicolon_col)] = ' ';
					if(m_sql[_tcslen(m_sql) - 1] == ' ') m_sql[_tcslen(m_sql) - 1] = '\0';
					break;
				}
				continue;
			} else {
				// 末尾の';'を取り除く
				m_sql[sql_len - (edit_data->get_row_len(row) - semicolon_col)] = ' ';
				if(m_sql[_tcslen(m_sql) - 1] == ' ') m_sql[_tcslen(m_sql) - 1] = '\0';
				break;
			}
		}

		// シングルクォートの中か調べる
		if((ostr_str_cnt(edit_data->get_row_buf(row), '\'') % 2) == 1) {
			in_quote = !in_quote;
		}

	}

	// コマンドのとき，末尾の';'と改行は削除する
	if(m_sql_kind != SQL_NORMAL) {
		if(m_sql[_tcslen(m_sql) - 1] == ';') m_sql[_tcslen(m_sql) - 1] = '\0';

		for(;;) {
			if(m_sql[_tcslen(m_sql) - 1] != '\n') break;
			m_sql[_tcslen(m_sql) - 1] = '\0';
		}
	}

	if(g_sql_str_token.isSelectSQL(skip_space_p)) {
		m_sql_kind = SQL_SELECT;
	}
	if(g_sql_str_token.isExplainSQL(skip_space_p)) {
		m_sql_kind = SQL_EXPLAIN;
	}

	// select * from tab"
	// 上のsqlのように，末尾が不要な"で終わるとき，oparse()が戻ってこないバグを回避する
	// NOTE: postgresqlでは不要
//	if(m_sql[_tcslen(m_sql) - 1] == '"') {
//		_tcscat(m_sql, _T(" "));
//	}

	if(row != edit_data->get_row_cnt()) row++;

	return row;
}
#pragma function(strcpy, strcmp, strlen)
*/

void CPsqleditDoc::SqlRun(int start_row, int end_row, CUnicodeArchive *ar)
{
	SqlRun(&m_sql_data, start_row, end_row, ar);
}

void CPsqleditDoc::SqlRun(CEditData *edit_data, int start_row, int end_row, CUnicodeArchive *ar)
{
	if(g_login_flg == FALSE) {
		DispNotConnectedMsg();
		return;
	}

	CWaitCursor		wait_cursor;
	CCancelDlg		dlg;
	_thr_sql_run_st st;
	UINT			thrdaddr;
	BOOL			obj_update_flg = FALSE;

	// 初期化
	st.pdoc = this;
	st.edit_data = edit_data;
	st.start_row = start_row;
	st.end_row = end_row;
	st.hWnd = NULL;
	st.ret_v = 0;
	st.obj_update_flg = &obj_update_flg;
	st.cancel_flg = &(dlg.m_cancel_flg);
	st.ar = ar;
	st.sql_cnt = 0;
	dlg.m_pdoc = this;
	dlg.m_cancel_flg = 0;
	dlg.m_p_hWnd = (HWND *)&(st.hWnd);

	dlg.m_disp_timer_flg = TRUE;

	dlg.m_hThread = _beginthreadex(NULL, 0, sql_run_thr,
		&st, CREATE_SUSPENDED, &thrdaddr);
	if(dlg.m_hThread == NULL) {
		_stprintf(g_msg_buf, _T("スレッド作成エラー"));
		return;
	}
	dlg.DoModal();
	WaitForSingleObject((void *)dlg.m_hThread, INFINITE);
	CloseHandle((void *)dlg.m_hThread);

	if(g_login_flg == TRUE && obj_update_flg == TRUE &&
		g_option.sql.refresh_table_list_after_ddl == TRUE) {
		((CPsqleditApp*)AfxGetApp())->OnRefreshTableList();
		CSqlListMaker::ClearCache();
	}
	if(g_option.sql.adjust_col_width_after_select == TRUE) {
		UpdateAllViews(NULL, UPD_ADJUST_COL_WIDTH);
	}

	// SQL実行の終了時にアプリケーションがアクティブでない場合は、flushwindowする
	if(GetActiveWindow() == NULL) {
		AfxGetMainWnd()->FlashWindow(TRUE);
	}

	return;
}

void CPsqleditDoc::OnSqlRun() 
{
	if(m_no_sql_run) {
		AfxGetMainWnd()->MessageBox(_T("ログファイルは実行できません"), 
			_T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	if(g_option.sql.sql_run_selected_area && m_sql_data.get_disp_data()->HaveSelected()) {
		OnSqlRunSelected();
	} else {
		SqlRun(0, 0);
	}
}

void CPsqleditDoc::OnSwitchView() 
{
	UpdateAllViews(NULL, UPD_SWITCH_VIEW, (CObject *)SWITCH_VIEW);
}

void CPsqleditDoc::OnClearResult() 
{
	UpdateAllViews(NULL, UPD_CLEAR_RESULT);
}

void CPsqleditDoc::SetDataset(HPgDataset dataset, HWND hWnd)
{
	if(m_dataset != NULL) {
		pg_free_dataset(m_dataset);
		m_dataset = NULL;
	}
	m_dataset = dataset;

	m_grid_swap_row_col_disp_flg = FALSE;

	// FilterはOFFにする
	if(m_grid_data.GetGridFilterMode()) {
		if(hWnd) {
			SendMessage(hWnd, WM_UPDATE_ALL_VIEWS, UPD_GRID_FILTER_DATA_OFF, GRID_VIEW);
		}
	}

	if(m_grid_swap_row_col_mode) {
		m_grid_data_swap_row_col.InitDispInfo();
		m_grid_swap_row_col_disp_flg = TRUE;
	}
}

CGridData *CPsqleditDoc::GetGridData()
{
	if(m_grid_swap_row_col_mode) {
		return &m_grid_data_swap_row_col;
	}

	if(m_grid_data.GetGridFilterMode()) {
		CGridData* g = m_grid_data.GetFilterGridData();
		return g;
	}

	return &m_grid_data;
}

void CPsqleditDoc::ActiveDoc()
{
	GetFirstView()->GetParentFrame()->ActivateFrame();
}

BOOL CPsqleditDoc::SaveModified() 
{
/*
	// ドキュメントを閉じる前に，変更を保存するか確認する
	if(g_option.save_modified && m_sql_data.is_edit_data()) {
		SetModifiedFlag(TRUE);
	} else {
		SetModifiedFlag(FALSE);
	}

	return CDocument::SaveModified();
*/
	if(!g_option.save_modified || !m_sql_data.is_edit_data() ||
		g_save_mode == SAVE_MODE_ALL_NOSAVE) {
		return TRUE;
	}

	ActiveDoc();
	// ドキュメントを閉じる前に，変更を保存するか確認する
	if(g_save_mode == SAVE_MODE_NONE) {
		return CDocument::SaveModified();
	}

	BOOL b_save = FALSE;

	if(g_save_mode == SAVE_MODE_ALL_QUERY) {
		CQueryCloseDlg	dlg;
		dlg.m_msg.Format(_T("%sへの変更を保存しますか？"),
			GetTitle().Left(GetTitle().GetLength() - 2));

		if(dlg.DoModal() == IDCANCEL) {
			g_save_mode = SAVE_MODE_NONE;
			return FALSE;
		}

		if(dlg.m_result == IDALLYES) g_save_mode = SAVE_MODE_ALL_SAVE;
		if(dlg.m_result == IDALLNO) g_save_mode = SAVE_MODE_ALL_NOSAVE;
		if(dlg.m_result == IDYES) b_save = TRUE;
	}

	if(b_save || g_save_mode == SAVE_MODE_ALL_SAVE) {
		if(!DoFileSave()) {
			g_save_mode = SAVE_MODE_NONE;
			return FALSE;
		}
	}

	return TRUE;
}

void CPsqleditDoc::OnSqlLog() 
{
	if(g_option.sql_logging) {
		CSqlLogFileDlg dlg;
		CString session_info;

		if(g_login_flg) {
			session_info.Format(_T("%s@%s:%s.%s"), pg_user(g_ss), pg_host(g_ss), pg_port(g_ss), pg_db(g_ss));
		}

		dlg.Init(&g_font, g_option.sql_log_dir, 
			CTime::GetCurrentTime(), session_info, _T(""));

		if(dlg.DoModal() != IDOK) return;

		m_sql_data.replace_all(dlg.GetSelectedSQL());

		UpdateAllViews(NULL, UPD_CLEAR_SELECTED);
		UpdateAllViews(NULL, UPD_REDRAW);
	} else {
		CSqlLogDlg	dlg;

		dlg.m_sql_log_array = g_sql_log_array;
		dlg.m_font = &g_font;

		if(dlg.DoModal() != IDOK) return;

		m_sql_data.replace_all(g_sql_log_array->GetAt(dlg.m_selected_row)->GetSql().GetBuffer(0));

		UpdateAllViews(NULL, UPD_CLEAR_SELECTED);
		UpdateAllViews(NULL, UPD_REDRAW);
	}
}

void CPsqleditDoc::OnFileSaveGridAs() 
{
	TCHAR file_name[_MAX_PATH];

	if(DoFileDlg(_T("検索結果表を保存"), FALSE, _T("csv"), NULL,
		OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("CSV Files (*.csv)|*.csv|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name) == FALSE) return;

	int ret_v;

	if(check_ext(file_name, _T(".txt")) == TRUE) {
		ret_v = pg_save_dataset_tsv(file_name, m_dataset,
			g_option.download.put_column_name, g_msg_buf);
	} else {
		ret_v = pg_save_dataset_csv(file_name, m_dataset,
			g_option.download.put_column_name, g_msg_buf);
	}

	if(ret_v != 0) {
		AfxGetMainWnd()->MessageBox(g_msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	}	
}

void CPsqleditDoc::OnSqlRunSelected() 
{
	SqlRunSelected();
}

void CPsqleditDoc::OnUpdateSqlRunSelected(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_sql_data.get_disp_data()->HaveSelected());
}

void CPsqleditDoc::OnUpdateSqlRun(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_no_sql_run);
}

void CPsqleditDoc::OnExplainRun() 
{
	if(g_login_flg == FALSE) {
		DispNotConnectedMsg();
		return;
	}

	CWaitCursor	wait_cursor;

	// 最初のSQLを取得(コマンドなどは読み飛ばす)
	CEditData *edit_data = GetSqlData();
	int	row_offset = 0;
	int row = 0;
	int start_row = 0;
	int end_row = edit_data->get_row_cnt();

	for(row = start_row; row < end_row;) {
		row = SkipNullRow(row, edit_data);
		if(row >= end_row) break;

		row_offset = row;
		row = GetSQL(row, edit_data);
		if(GetSqlKind() == SQL_NORMAL || GetSqlKind() == SQL_SELECT || GetSqlKind() == SQL_EXPLAIN) break;
	}

	UpdateAllViews(NULL, UPD_SWITCH_VIEW, (CObject *)MSG_VIEW);
	UpdateAllViews(NULL, UPD_SET_RESULT_CARET);
	UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)(GetSqlBuf()));
	UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)_T("\n"));

	CString plan;
	if(GetSqlKind() != SQL_EXPLAIN) {
		CString exp_sql;
		exp_sql.Format(_T("explain %s"), GetSqlBuf());
		plan = explain_plan(g_ss, exp_sql.GetBuffer(0), g_msg_buf);
	} else {
		plan = explain_plan(g_ss, GetSqlBuf(), g_msg_buf);
	}
	if(plan == "") goto ERR1;

	UpdateAllViews(NULL, UPD_SET_RESULT_CARET);
	UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)_T("----- 実行計画 -----\n"));
	UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)plan.GetBuffer(0));
	UpdateAllViews(NULL, UPD_PUT_SEPALATOR);

	UpdateAllViews(NULL, UPD_REDRAW);
	
	return;

ERR1:
	INT_PTR r = row_offset;
	UpdateAllViews(NULL, UPD_SET_ERROR_INFO, (CObject *)r);
	UpdateAllViews(NULL, UPD_SET_RESULT_CARET);
	UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)_T("Error\n"));
	UpdateAllViews(NULL, UPD_PUT_RESULT, (CObject *)g_msg_buf);
	UpdateAllViews(NULL, UPD_PUT_SEPALATOR);
	UpdateAllViews(NULL, UPD_REDRAW);
	return;
}

void CPsqleditDoc::OnUpdateExplainRun(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(!m_no_sql_run);
}

void CPsqleditDoc::DispNotConnectedMsg()
{
	AfxGetMainWnd()->MessageBox(_T("データベースに接続していません"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
}

void CPsqleditDoc::OnDownload() 
{
	if(g_login_flg == FALSE) {
		DispNotConnectedMsg();
		return;
	}

	CDownloadDlg	dlg;
	if(dlg.DoModal() == IDCANCEL) return;

	TCHAR *file_name = dlg.m_file_name.GetBuffer(0);

	// FIXME: 漢字コードなどを指定できるようにする
	CUnicodeArchive uni_ar;
	if(!uni_ar.OpenFile(file_name, _T("w"), KANJI_CODE_SJIS, LINE_TYPE_CR_LF)) {
		_stprintf(g_msg_buf, PGERR_OPEN_FILE_MSG, file_name);
		AfxGetMainWnd()->MessageBox(g_msg_buf, _T("Error"),
			MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	if(dlg.m_save_select_result_only == FALSE) {
		GetMsg()->log_start(&uni_ar);
	}

	SqlRun(0, 0, &uni_ar);

	if(dlg.m_save_select_result_only == FALSE) {
		GetMsg()->log_end();
	}
}

void CPsqleditDoc::OnUpdateMakeSql(CCmdUI* pCmdUI) 
{
	if(pCmdUI->m_pSubMenu != NULL) {
        // ポップアップ メニュー [SQL作成支援] 全体を選択可能にする
		BOOL bEnable = g_object_detail_bar.CanMakeSql();

		// この場合 CCmdUI::Enable は何もしないので
		//   代わりにここで適切な処理を行う
		pCmdUI->m_pMenu->EnableMenuItem(pCmdUI->m_nIndex, 
			MF_BYPOSITION | (bEnable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED)));
		return;
	}
	pCmdUI->Enable(g_object_detail_bar.CanMakeSql());
}

void CPsqleditDoc::OnMakeSelectSql() 
{
	CString paste_str = g_object_detail_bar.MakeSelectSql(FALSE, g_option.make_sql.select_sql_use_semicolon);
	UpdateAllViews(NULL, UPD_PASTE_SQL, (CObject *)paste_str.GetBuffer(0));
}

void CPsqleditDoc::OnMakeInsertSql() 
{
	g_object_detail_bar.MakeInsertSql();
}

void CPsqleditDoc::OnMakeUpdateSql() 
{
	g_object_detail_bar.MakeUpdateSql();
}

void CPsqleditDoc::OnMakeInsertSqlAll() 
{
	g_object_detail_bar.MakeInsertSql(TRUE);
}

void CPsqleditDoc::OnMakeSelectSqlAll() 
{
	CString paste_str = g_object_detail_bar.MakeSelectSql(TRUE, g_option.make_sql.select_sql_use_semicolon);
	UpdateAllViews(NULL, UPD_PASTE_SQL, (CObject *)paste_str.GetBuffer(0));
}

void CPsqleditDoc::OnShortCutSql(UINT nID)
{
	int		idx = nID - ID_SHORT_CUT_SQL1;
	CShortCutSql *sql_data = m_short_cut_sql_list.GetShortCutSql(idx);

	if(sql_data->GetName() == _T("") || sql_data->GetSql() == _T("")) return;

	if(sql_data->IsPasteToEditor()) {
		UpdateAllViews(NULL, UPD_PASTE_SQL2, (CObject *)sql_data->GetSql().GetBuffer(0));
		return;
	}

	if(g_login_flg == FALSE) {
		DispNotConnectedMsg();
		return;
	}

	if(sql_data->IsShowDlg()) {
		CString msg;
		msg.Format(_T("%sを実行します。\nよろしいですか？"), sql_data->GetName());
		if(AfxGetMainWnd()->MessageBox(msg, _T("確認"), MB_ICONQUESTION | MB_OKCANCEL) == IDCANCEL) return;
	}

	CEditData	edit_data;
	edit_data.set_str_token(&g_sql_str_token);
	edit_data.paste(sql_data->GetSql().GetBuffer(0));

	SqlRun(&edit_data);
}

// static data & function
CShortCutSqlList CPsqleditDoc::m_short_cut_sql_list;

BOOL CPsqleditDoc::LoadShortCutSqlList()
{
	return m_short_cut_sql_list.Load();
}

void CPsqleditDoc::SqlRunSelected()
{
	if(!m_sql_data.get_disp_data()->HaveSelected()) return;
	if(m_sql_data.get_disp_data()->GetSelectArea()->box_select) {
		AfxGetMainWnd()->MessageBox(_T("矩形選択のSQL実行はサポートしていません"), 
			_T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	POINT	pt1 = m_sql_data.get_disp_data()->GetSelectArea()->pos1;
	POINT	pt2 = m_sql_data.get_disp_data()->GetSelectArea()->pos2;

	// エラー位置がずれないように，空白で埋める
	int			i;
	CEditData	edit_data;
	edit_data.set_str_token(&g_sql_str_token);
	for(i = 0; i < pt1.y; i++) {
		edit_data.paste_no_undo(_T("\n"));
	}
	for(i = 0; i < pt1.x; i++) {
		edit_data.paste_no_undo(_T(" "));
	}

	// 選択範囲のテキストを貼り付ける
	edit_data.paste_no_undo(m_sql_data.get_point_text(pt1, pt2));
	edit_data.recalc_disp_size();

	SqlRun(&edit_data);
}

void CPsqleditDoc::OnPsqlgrid() 
{
	if(g_login_flg == FALSE) {
		DispNotConnectedMsg();
		return;
	}

	CWaitCursor	wait_cursor;

	// 最初のSQLを取得(コマンドなどは読み飛ばす)
	CEditData *edit_data = GetSqlData();
	int	row_offset = 0;
	int row = 0;
	int start_row = 0;
	int end_row = edit_data->get_row_cnt();

	for(row = start_row; row < end_row;) {
		row = SkipNullRow(row, edit_data);
		if(row >= end_row) break;

		row_offset = row;
		row = GetSQL(row, edit_data);
		if(row == -1) return;
		if(GetSqlKind() == SQL_NORMAL || GetSqlKind() == SQL_SELECT) break;
	}

	RunPsqlGrid(GetSqlBuf());
}

void CPsqleditDoc::OnUpdatePsqlgrid(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_login_flg);
}

BOOL CPsqleditDoc::RunPsqlGrid(TCHAR *sql)
{
	if(!g_sql_str_token.isSelectSQL(sql)) {
		AfxGetMainWnd()->MessageBox(_T("このSQLでは，PSqlGridを利用できません"), _T("Error"),
			MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
/*
	if(have_bind_param(sql, &g_sql_str_token)) {
		AfxGetMainWnd()->MessageBox("PSqlGridでは，バインド変数を使用できません", "Error",
			MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
*/
	CString file_name = GetAppPath() + _T("tmp_psqlgrid.psg");

	if(!CreatePsqlGridFile(sql, file_name)) {
		return FALSE;
	}

	{
		PROCESS_INFORMATION		pi;
		STARTUPINFO				si;

		CString cmd;
		cmd.Format(_T("%spsqlgrid.exe \"%s\" LOGON=%s"),
			GetAppPath(), file_name, g_connect_str);

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);

		if(CreateProcess(NULL, cmd.GetBuffer(0), NULL, NULL, FALSE,
			0, NULL, NULL, &si, &pi) == FALSE) {
			AfxMessageBox(_T("PSqlGridの起動に失敗しました"), MB_OK);
			return FALSE;
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	return TRUE;
}

static CString MakeLower1Byte(const TCHAR *str)
{
	TCHAR buf[1024];
	if(_tcslen(str) >= sizeof(buf) / sizeof(buf[0])) return str;

	_tcscpy(buf, str);
	my_mbslwr_1byte(buf);

	return buf;
}

BOOL CPsqleditDoc::ParseSelectSQL(const TCHAR *sql, TCHAR *msg_buf,
	CString &schema_name, CString &table_name1, CString &table_name2, 
	CString &alias_name, CString &where_clause)
{
	TCHAR buf[1024];
	const TCHAR *p = sql;

	for(;;) {
		if(p == NULL) return FALSE;
		p = g_sql_str_token.get_word(p, buf, sizeof(buf));
		if(ostr_strcmp_nocase(buf, _T("FROM")) == 0) break;
	}

	p = g_sql_str_token.skipCommentAndSpace(p);
	if(p == NULL) return FALSE;

	const TCHAR *p2 = p;
	for(; *p2 != '\0';) {
		if(*p2 == ',') {
			_stprintf(msg_buf, _T("FROM句に複数のテーブルがあります\n"));
			return FALSE;
		}
		if(*p2 == ' ' || *p2 == '\t' || *p2 == '\n' || get_char(p2) == '　') break;
		p2 += get_char_len(p2);
	}
	_stprintf(buf, _T("%.*s"), (int)(p2 - p), p);
	/*
			if(buf[0] == '"' && buf[_tcslen(buf) - 1] == '"') {
				_stprintf(buf, _T("%.*s"), (int)(p2 - p - 2), p + 1);
			} else {
				my_mbslwr_1byte(buf);
			}
			table_name1 = buf;
	*/
	table_name1 = buf;
	table_name1.TrimRight(L'\r');
	TRACE(_T("TABLE_NAME: %s\n"), table_name1);
	/*
			if(table_name.Find(".") != -1 || table_name.Find("@") != -1) {
				sprintf(msg_buf, "他スキーマのテーブルは指定できません\n");
				goto ERR1;
			}
	*/

	if(table_name1.Find(_T(".")) != -1) {
		schema_name = table_name1.Left(table_name1.Find(_T(".")));
		table_name2 = table_name1.Right(table_name1.GetLength() - table_name1.Find(_T(".")) - 1);
	} else {
		if(table_name1.GetAt(0) == '"') {
			table_name1.Replace(_T("\""), _T(""));
		} else {
			table_name1 = MakeLower1Byte(table_name1.GetString());
		}
		schema_name = get_table_schema_name(g_ss, table_name1, msg_buf);
		table_name2 = table_name1;
	}
	if(schema_name.GetAt(0) == '"') {
		schema_name.Replace(_T("\""), _T(""));
	} else {
		schema_name = MakeLower1Byte(schema_name.GetString());
	}
	if(table_name2.GetAt(0) == '"') {
		table_name2.Replace(_T("\""), _T(""));
	} else {
		table_name2 = MakeLower1Byte(table_name2.GetString());
	}

	schema_name.TrimRight(L'\r');
	table_name2.TrimRight(L'\r');

	table_name1.Format(_T("%s.%s"), schema_name, table_name2);

	p = p2;
	for(;;) {
		if(p == NULL) break;
		p2 = g_sql_str_token.get_word(p, buf, sizeof(buf));
		if(ostr_strcmp_nocase(buf, _T("WHERE")) == 0) break;
		if(ostr_strcmp_nocase(buf, _T("ORDER")) == 0) break;
		if(_tcscmp(buf, _T(",")) == 0) {
			_stprintf(msg_buf, _T("FROM句に複数のテーブルがあります\n"));
			return FALSE;
		}
		if(alias_name.IsEmpty()) alias_name = buf;
		TRACE(_T("ALIAS_NAME: %s\n"), alias_name);
		p = p2;
	}
	if(alias_name.IsEmpty()) alias_name = table_name2;

	where_clause = p;
	TRACE(_T("WHERE CLAUSE: %s\n"), where_clause);

	return TRUE;
}

BOOL CPsqleditDoc::CreatePsqlGridFile(TCHAR *sql, const TCHAR *file_name)
{
	TCHAR			msg_buf[1024] = _T("");
	CString			table_name1;
	CString			table_name2 = _T("");
	CString			schema_name = _T("");
	CString			alias_name;
	CString			where_clause;
	CStringArray	column_array;

	HPgDataset		tab_def_dataset = NULL;

	// テーブル名とwhere句を取得する
	if(!ParseSelectSQL(sql, msg_buf, schema_name, table_name1, table_name2, alias_name, where_clause)) goto ERR1;

	tab_def_dataset = get_column_list(g_ss, table_name2, schema_name, msg_buf);
	if(tab_def_dataset == NULL || pg_dataset_row_cnt(tab_def_dataset) == 0) {
//		char		msg_buf2[1024];
//		sprintf(msg_buf2, "テーブル定義の取得に失敗しました(%s)\n%s",
//			table_name1, msg_buf);
//		strcpy(msg_buf, msg_buf2);
		_stprintf(msg_buf, _T("テーブル定義の取得に失敗しました(%s)\n"), table_name1.GetBuffer(0));

		goto ERR1;
	}

	// ダブルクォートで囲まれている場合，取り除く
	//table_name1 = get_object_name(table_name1, TRUE);

	// create column list
	{
		TCHAR buf[1024];
		const TCHAR *p = sql;

		// selectをskip
		p = g_sql_str_token.get_word(p, buf, sizeof(buf));
		if(p == NULL) goto ERR1;

		for(;;) {
			// schemaname.tablename.columnnameのとき、columnnameを取得
			for(;;) {
				p = g_sql_str_token.skipCommentAndSpace(p);
				if(*p == '"') {
					p++;
					p = g_sql_str_token.get_word(p, buf, sizeof(buf));
					if(*p == '"') p++;
				} else {
					p = g_sql_str_token.get_word(p, buf, sizeof(buf));
					my_mbslwr_1byte(buf);
				}

				if(p == NULL) goto ERR1;
				if(*p != '.') break;
				p++;
			}

			if(_tcscmp(buf, _T("*")) == 0) {
				for(int idx = 0; idx < pg_dataset_row_cnt(tab_def_dataset); idx++) {
					column_array.Add(pg_dataset_data2(tab_def_dataset, idx, _T("ATTNAME")));
TRACE(_T("COLUMN_NAME: %s\n"), pg_dataset_data2(tab_def_dataset, idx, _T("ATTNAME")));
				}
			} else {
				//CString col_name = get_object_name((char *)buf, TRUE);
				CString col_name = buf;

				int idx;
				for(idx = 0; idx < pg_dataset_row_cnt(tab_def_dataset); idx++) {
					if(_tcscmp(col_name, pg_dataset_data2(tab_def_dataset, idx, _T("ATTNAME"))) == 0) break;
				}
				if(idx == pg_dataset_row_cnt(tab_def_dataset)) {
					_stprintf(msg_buf, _T("カラム名が不正です(%s)\n"), col_name.GetBuffer(0));
					goto ERR1;
				}
				column_array.Add(col_name);
TRACE(_T("COLUMN_NAME: %s\n"), col_name);
			}

			for(;;) {
				p = g_sql_str_token.get_word(p, buf, sizeof(buf));
				if(p == NULL) {
					_tcscpy(msg_buf, _T(""));
					goto ERR1;
				}
				if(_tcscmp(buf, _T(",")) == 0) break;
				if(ostr_strcmp_nocase(buf, _T("FROM")) == 0) break;
			}

			if(ostr_strcmp_nocase(buf, _T("FROM")) == 0) break;
		}
	}

	{
		FILE	*fp = NULL;
		fp = _tfopen(file_name, _T("w"));
		if(fp == NULL) {
			AfxGetMainWnd()->MessageBox(_T("ファイルが開けません"), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
			if(tab_def_dataset) pg_free_dataset(tab_def_dataset);
			return FALSE;
		}

		_ftprintf(fp, _T("PSqlGrid 検索条件設定ファイル\n"));
		_ftprintf(fp, _T("--- DO NOT EDIT THIS FILE ---\n"));
		_ftprintf(fp, _T("[TABLE_NAME]\n"));
		_ftprintf(fp, _T("%s\n"), table_name1.GetBuffer(0));
		_ftprintf(fp, _T("[ALIAS_NAME]\n"));
		_ftprintf(fp, _T("%s\n"), alias_name.GetBuffer(0));
		_ftprintf(fp, _T("[COLUMN_NAME]\n"));
		for(int idx = 0; idx < column_array.GetSize(); idx++) {
			CString str = column_array.GetAt(idx);
			_ftprintf(fp, _T("%s\n"), str.GetBuffer(0));
		}
		_ftprintf(fp, _T("[WHERE]\n"));
		_ftprintf(fp, _T("%s\n"), where_clause.GetBuffer(0));

		fclose(fp);
	}

	if(tab_def_dataset) pg_free_dataset(tab_def_dataset);

	return TRUE;

ERR1:
	if(tab_def_dataset) pg_free_dataset(tab_def_dataset);

	{
		TCHAR		msg_buf2[1024];
		_stprintf(msg_buf2, _T("このSQLでは，PSqlGridを利用できません\n%s"), msg_buf);
		AfxGetMainWnd()->MessageBox(msg_buf2, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	}
	
	return FALSE;
}


void CPsqleditDoc::CalcGridSelectedData()
{
	g_grid_calc = _T("");

	CGridData *grid_data = GetGridData();

	if(GetViewKind() != GRID_VIEW || !grid_data->HaveSelected() || 
		g_option.grid_calc_type == GRID_CALC_TYPE_NONE) return;

	CMainFrame	*pMainFrame = (CMainFrame *)AfxGetMainWnd();
	if(!pMainFrame->GetStatusBar()->IsWindowVisible()) return;

	CPoint pt1 = grid_data->GetSelectArea()->pos1;
	CPoint pt2 = grid_data->GetSelectArea()->pos2;

//	if(m_grid_swap_row_col_mode) {
//		pt1.x = grid_data->GetSelectArea()->pos1.y;
//		pt1.y = grid_data->GetSelectArea()->pos1.x;
//		pt2.x = grid_data->GetSelectArea()->pos2.y;
//		pt2.y = grid_data->GetSelectArea()->pos2.x;
//	}

	g_grid_calc = grid_data->CalcSelectedData(g_option.grid_calc_type, pt1, pt2);
}

void CPsqleditDoc::OnActiveDocument()
{
	DispFileType();
	UpdateAllViews(NULL, UPD_ACTIVE_DOC);
}

int CPsqleditDoc::GetViewKind()
{
	return ((CChildFrame *)GetFirstView()->GetParentFrame())->GetViewKind();
}

CView *CPsqleditDoc::GetFirstView()
{
	POSITION pos = GetFirstViewPosition();
	ASSERT(pos != NULL);
	CView *pview = GetNextView(pos);
	ASSERT(pview != NULL);

	return pview;
}

void CPsqleditDoc::OnPsqlgridObjectbar() 
{
	CString paste_str = g_object_detail_bar.MakeSelectSql(TRUE, FALSE);
	RunPsqlGrid(paste_str.GetBuffer(0));
}

void CPsqleditDoc::ToggleGridSwapRowColMode()
{
	if(m_grid_swap_row_col_mode) {
		m_grid_swap_row_col_mode = FALSE;
	} else {
		m_grid_swap_row_col_mode = TRUE;

		if(!m_grid_swap_row_col_disp_flg) {
			m_grid_data_swap_row_col.InitDispInfo();
			m_grid_swap_row_col_disp_flg = TRUE;
		}
	}
}

void CPsqleditDoc::PostGridFilterOnOff()
{
	if(m_grid_data.GetGridFilterMode()) {
		m_grid_data_swap_row_col.SetGridData(m_grid_data.GetFilterGridData());
	} else {
		m_grid_data_swap_row_col.SetGridData(&m_grid_data);
	}
}

void CPsqleditDoc::OnGridSwapRowCol() 
{
	UpdateAllViews(NULL, UPD_GRID_SWAP_ROW_COL, (CObject *)GRID_VIEW);
}

void CPsqleditDoc::OnUpdateGridSwapRowCol(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_grid_swap_row_col_mode);
	pCmdUI->Enable((GetViewKind() == GRID_VIEW));
}

void CPsqleditDoc::OnPsqlgrselected()
{
	if(g_login_flg == FALSE) {
		DispNotConnectedMsg();
		return;
	}

	CWaitCursor	wait_cursor;

	// 選択中のSQLを取得(コマンドなどは読み飛ばす)
	CString sql = m_sql_data.get_selected_text();
	sql.TrimRight(_T(" \t\r\n;"));
	sql.TrimRight(_T("/"));

	if(sql.IsEmpty()) {
		AfxGetMainWnd()->MessageBox(_T("SQLを選択してください"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	RunPsqlGrid(sql.GetBuffer());
}

void CPsqleditDoc::OnUpdatePsqlgrselected(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(g_login_flg && m_sql_data.get_disp_data()->HaveSelected());
}


void CPsqleditDoc::SetTabName(CString title)
{
	if(IsModified()) title += " *";

	SetTitle(title);
	g_tab_bar.SetTabName(this);
}

void CPsqleditDoc::OnTabName()
{
	CNameInputDlg	dlg;

	dlg.m_title = "タブに名前を付ける";
	dlg.m_data = g_tab_bar.GetCurrentTabName();

	if(dlg.DoModal() != IDOK) return;

	SetTabName(dlg.m_data);
}


BOOL CPsqleditDoc::IsShowDataDialogVisible()
{
	return g_show_clob_dlg.IsVisible();
}

void CPsqleditDoc::OnUpdateTabName(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!g_option.tab_title_is_path_name || GetPathName().IsEmpty());
}

void CPsqleditDoc::ShowGridData(POINT cell_pt)
{
	/*
		// モーダル表示
		CShowCLobDlg dlg;

		CEditData *edit_data = dlg.GetEditData();
		edit_data->del_all();
		edit_data->paste(m_grid_data.Get_ColData(cell_pt.y, cell_pt.x));

		dlg.SetFont(&g_font);
		dlg.DoModal2();
	*/
	// モードレス表示
	CShowCLobDlg* dlg = g_show_clob_dlg.GetDlg();
	CEditData* edit_data = dlg->GetEditData();
	edit_data->del_all();
	edit_data->paste(m_grid_data.Get_ColData(cell_pt.y, cell_pt.x));

	CString col_name = m_grid_data.Get_ColName(cell_pt.x);
	dlg->SetColName(col_name.GetString());

	if(g_show_clob_dlg.IsVisible()) {
		dlg->Invalidate();
		dlg->RedrawEditCtrl();
	} else {
		dlg->SetFont(&g_font);
		dlg->ShowDialog();
	}
}


void CPsqleditDoc::OnGrfilterOn()
{
	UpdateAllViews(NULL, UPD_GRID_FILTER_DATA_ON, (CObject*)GRID_VIEW);
}

void CPsqleditDoc::OnUpdateGrfilterOn(CCmdUI* pCmdUI)
{
	if(GetViewKind() != GRID_VIEW) {
		pCmdUI->Enable(FALSE);
		return;
	}
	if(m_grid_data.Get_RowCnt() == 0) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
}

void CPsqleditDoc::OnGrfilterOff()
{
	if(!m_grid_data.GetGridFilterMode()) return;

	UpdateAllViews(NULL, UPD_GRID_FILTER_DATA_OFF, (CObject*)GRID_VIEW);
}

void CPsqleditDoc::OnUpdateGrfilterOff(CCmdUI* pCmdUI)
{
	if(!m_grid_data.GetGridFilterMode()) {
		pCmdUI->Enable(FALSE);
		return;
	}
	pCmdUI->Enable(TRUE);
}

void CPsqleditDoc::HideSearchDlgs()
{
	if(CReplaceDlgSingleton::IsVisible()) {
		CReplaceDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
	if(CSearchDlgSingleton::IsVisible()) {
		CSearchDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
	if(CGridFilterDlgSingleton::IsVisible()) {
		CGridFilterDlgSingleton::GetDlg().ShowWindow(SW_HIDE);
	}
}

