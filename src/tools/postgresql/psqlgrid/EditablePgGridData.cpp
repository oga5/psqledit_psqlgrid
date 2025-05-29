/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include <stdio.h>

#include "EditablePgGridData.h"

#include "get_char.h"
#include "octrl_msg.h"
#include "ostrutil.h"
#include "pgmsg.h"
#include "mbutil.h"

// ソースコード全体でユニークな文字列にする
// ""にするとコンパイラの共有プールオプションによって、他のソースコードの
// ポインタと共有してしまう
TCHAR *CEditablePgGridData::blank_str = _T("\b_blank_str");

CEditablePgGridData::CEditablePgGridData()
{
	m_null_dataset = NULL;
	m_dataset = m_null_dataset;
	m_reserve_columns = 1;
	m_pkey_dataset = NULL;

	SetCellDataSize(sizeof(BYTE));
}

CEditablePgGridData::~CEditablePgGridData()
{
}

unsigned int _stdcall save_grid_data_thr(void *lpvThreadParam)
{
	struct _thr_save_grid_data_st *p_st = (struct _thr_save_grid_data_st *)lpvThreadParam;

	// 保存を実行
	p_st->ret_v = p_st->grid_data->SaveData(p_st->ss, p_st->err_pt, 
		p_st->msg_buf, (HWND)p_st->hWnd, p_st->cancel_flg);

	if(p_st->hWnd != NULL) {
		PostMessage((HWND)p_st->hWnd, CND_WM_EXIT, p_st->ret_v, 0);
	}

	return p_st->ret_v;
}

static int query_cancel(HWND hWnd, TCHAR *msg_buf)
{
	for(; SendMessage(hWnd, CND_WM_QUERY_CANCEL, 0, 0) == 2; ) Sleep(500);
	if(SendMessage(hWnd, CND_WM_QUERY_CANCEL, 0, 0) == 1) {
		_stprintf(msg_buf, PGERR_CANCEL_MSG);
		return PGERR_CANCEL;
	}
	return 0;
}

int CEditablePgGridData::SetDataset(HPgSession ss, HPgDataset dataset, HPgDataset pkey_dataset, const TCHAR *owner, 
	const TCHAR *table_name, const TCHAR *sql, BOOL b_data_lock)
{
	m_ss = ss;
	m_sql = sql;

	m_b_data_lock = b_data_lock;
	m_owner = owner;
	m_table_name = table_name;

	FreeDataMemory();

	if(dataset == NULL) {
		m_can_insert_row = FALSE;
		m_can_delete_row = FALSE;
		m_dataset = m_null_dataset;
		m_pkey_dataset = NULL;
		m_reserve_columns = 1;
		if(AllocDataMemory(0, 0) != 0) return 1;
		return 0;
	}

	m_can_insert_row = TRUE;
	m_can_delete_row = TRUE;
	m_dataset = dataset;
	m_pkey_dataset = pkey_dataset;
	
	m_reserve_columns = pg_dataset_row_cnt(m_pkey_dataset);

	if(AllocDataMemory(pg_dataset_row_cnt(dataset), pg_dataset_col_cnt(dataset)) != 0) return 1;

	return 0;
}

const TCHAR *CEditablePgGridData::GetOriginalText(int row, int col)
{
	if(m_dataset == NULL) return _T("");
	if(IsInsertRow(row)) return _T("");

	return pg_dataset_data(m_dataset, GetRowIdx(row), col);
}

int CEditablePgGridData::SaveData(HPgSession ss, POINT *err_pt, TCHAR *msg_buf, 
	HWND hWnd, volatile int *cancel_flg)
{
	int		ret_v;
	int		del_row = 0;
	int		ins_row = 0;
	int		upd_row = 0;

	if(hWnd != NULL) {
		SendMessage(hWnd, CND_WM_STATIC, 1, (LPARAM)_T("データ保存開始"));
		SendMessage(hWnd, CND_WM_SETRANGE, 1, (Get_RowCnt() / 100 + 1) * 3);
	}

	err_pt->y = -1;
	err_pt->x = -1;

	if(hWnd != NULL) {
		SendMessage(hWnd, CND_WM_STATIC, 1, (LPARAM)_T("deleteを実行中"));
	}
	ret_v = DeleteData(ss, err_pt, &del_row, msg_buf, hWnd, cancel_flg);
	if(ret_v != 0) goto ERR1;

	if(hWnd != NULL) {
		SendMessage(hWnd, CND_WM_STATIC, 1, (LPARAM)_T("updateを実行中"));
	}
	ret_v = UpdateData(ss, err_pt, &upd_row, msg_buf, hWnd, cancel_flg);
	if(ret_v != 0) goto ERR1;

	if(hWnd != NULL) {
		SendMessage(hWnd, CND_WM_STATIC, 1, (LPARAM)_T("insertを実行中"));
	}
	ret_v = InsertData(ss, err_pt, &ins_row, msg_buf, hWnd, cancel_flg);
	if(ret_v != 0) goto ERR1;

	if(hWnd != NULL) {
		ret_v = query_cancel(hWnd, msg_buf);
		if(ret_v != 0) goto ERR1;
	}

	ret_v = pg_commit(ss, msg_buf);
	if(ret_v != 0) goto ERR1;

	_stprintf(msg_buf, _T("編集結果を保存しました。\n")
			_T("    delete: %d\n")
			_T("    insert %d\n")
			_T("    update %d"),
		del_row, ins_row, upd_row);

	return 0;

ERR1:
	int ret_v2 = pg_rollback(ss, NULL);
	if(ret_v2 != 0) {
		// 致命的なエラー
		_stprintf(msg_buf, _T("fatal error: rollback error"));
		ret_v = PG_ERR_FATAL;
	}

	if(m_b_data_lock) {
		// データをロックするために，再検索する
		HPgDataset dset_dummy = pg_create_dataset(m_ss, m_sql.GetBuffer(0), NULL);
		if(dset_dummy == 0) {
			// 致命的なエラー
			_stprintf(msg_buf, _T("fatal error: lock data error"));
			ret_v = PG_ERR_FATAL;
		} else {
			pg_free_dataset(dset_dummy);
		}
	}

	return ret_v;
}

#pragma intrinsic(memcpy)
CString CEditablePgGridData::GetSqlColData(int row, int col)
{
	if(IsBlank(row, col)) return _T("''");
	if(IsColDataNull(row, col)) return _T("null");

	// SQLのパラメータは，シングルクォートで囲む
	// データの中にシングルクォートがある場合，シングルクォート2つにする

	const TCHAR *data = Get_ColData(row, col);
	const TCHAR *p;
	int			ch_len;
	int			len = 0;
	BOOL		have_single_quote = FALSE;

	for(p = data; *p != '\0';) {
		if(*p == '\'') {
			len++;
			have_single_quote = TRUE;
		}
		ch_len = get_char_len(p);
		len += ch_len;
		p += ch_len;
	}

	CString result;
	// +3はシングルクォートが2つと'\0'の分
	TCHAR *d = result.GetBuffer(len + 3);

	*d = '\''; d++;

	if(have_single_quote) {
		for(p = data; *p != '\0';) {
			if(*p == '\'') {
				*d = '\''; d++;
			}
			ch_len = get_char_len(p);
			do {
				if(*p == '\0') break;
				*d = *p; d++; p++;
				ch_len--;
			} while(ch_len);
		}
	} else {
		memcpy(d, data, len * sizeof(TCHAR));
		d += len;
	}

	*d = '\''; d++;
	*d = '\0';

	result.ReleaseBuffer();
	return result;
}
#pragma function(memcpy)

CString CEditablePgGridData::GetUpdateWhereClause(int row)
{
	CString where_clause;
	CString tmp;
	int		i;
	int		col_offset;

	where_clause = _T("\nwhere ");
	col_offset = pg_dataset_col_cnt(m_dataset) - m_reserve_columns;

	for(i = 0; i < pg_dataset_row_cnt(m_pkey_dataset); i++) {
		if(i > 0) where_clause += _T(" and ");
		tmp.Format(_T(" \"%s\" = '%s' "),
			pg_dataset_data(m_pkey_dataset, i, 0),
			Get_ColData(row, col_offset));

		where_clause += tmp;
		col_offset++;
	}

	return where_clause;
}

int CEditablePgGridData::DeleteData(HPgSession ss, POINT *err_pt, int *row_cnt, TCHAR *msg_buf, 
	HWND hWnd, volatile int *cancel_flg)
{
	int			ret_v;
	int			row;
	CString		sql;

	for(row = 0; row < Get_RowCnt(); row++) {
		if(hWnd != NULL && (row % 100) == 0) {
			SendMessage(hWnd, CND_WM_STEPIT, 0, 0);
			ret_v = query_cancel(hWnd, msg_buf);
			if(ret_v != 0) goto ERR1;
		}

		if(IsDeleteRow(row)) {
			sql.Format(_T("delete from %s %s"),
				m_table_name,
				GetUpdateWhereClause(row));

			ret_v = pg_exec_sql(ss, sql.GetBuffer(0), msg_buf);
			if(ret_v != 0) goto ERR1;
			(*row_cnt)++;
		}
	}

	return 0;

ERR1:
	err_pt->y = row;
	err_pt->x = 0;
	return ret_v;
}

int CEditablePgGridData::InsertData(HPgSession ss, POINT *err_pt, int *row_cnt, TCHAR *msg_buf, 
	HWND hWnd, volatile int *cancel_flg)
{
	int			ret_v;
	int			row, col;
	CString		insert_sql;
	CString		sql;

	insert_sql.Format(_T("insert into %s (\n"), m_table_name);

	for(col = 0; col < Get_ColCnt(); col++) {
		if(col != 0) insert_sql += ",";
		insert_sql += "\"";
		insert_sql += Get_ColName(col);
		insert_sql += "\"";
	}
	insert_sql += ") values (\n";

	for(row = 0; row < Get_RowCnt(); row++) {
		if(hWnd != NULL && (row % 100) == 0) {
			SendMessage(hWnd, CND_WM_STEPIT, 0, 0);
			ret_v = query_cancel(hWnd, msg_buf);
			if(ret_v != 0) goto ERR1;
		}

		if(IsInsertRow(row)) {
			sql = insert_sql;

			for(col = 0; col < Get_ColCnt(); col++) {
				if(col != 0) sql += _T(",");
				sql += GetSqlColData(row, col);
			}
			sql += ')';

			ret_v = pg_exec_sql(ss, sql.GetBuffer(0), msg_buf);
			if(ret_v != 0) goto ERR1;
			(*row_cnt)++;
		}
	}

	return 0;

ERR1:
	err_pt->y = row;
	err_pt->x = 0;
	return ret_v;
}

int CEditablePgGridData::UpdateData(HPgSession ss, POINT *err_pt, int *row_cnt, TCHAR *msg_buf, 
	HWND hWnd, volatile int *cancel_flg)
{
	int			ret_v;
	int			row, col;
	CString		sql;

	for(row = 0; row < Get_RowCnt(); row++) {
		if(hWnd != NULL && (row % 100) == 0) {
			SendMessage(hWnd, CND_WM_STEPIT, 0, 0);
			ret_v = query_cancel(hWnd, msg_buf);
			if(ret_v != 0) goto ERR1;
		}

		if(IsUpdateRow(row)) {
			sql.Format(_T("update %s set "), m_table_name);
			BOOL	comma_flg = FALSE;
			for(col = 0; col < Get_ColCnt(); col++) {
				if(IsUpdateCell(row, col)) {
					if(comma_flg) {
						sql += _T(",");
					} else {
						comma_flg = TRUE;
					}
					sql += _T("\"");
					sql += Get_ColName(col);
					sql += _T("\"");
					sql += _T(" = ");
					sql += GetSqlColData(row, col);
				}
			}

			sql += GetUpdateWhereClause(row);

			ret_v = pg_exec_sql(ss, sql.GetBuffer(0), msg_buf);
			if(ret_v != 0) goto ERR1;
			(*row_cnt)++;
		}
	}

	return 0;

ERR1:
	err_pt->y = row;
	err_pt->x = 0;
	return ret_v;
}

void CEditablePgGridData::SetBlank(int row, int col)
{
	BYTE flg = 1;
	SetCellData(row, col, &flg);
}

void CEditablePgGridData::UnSetBlank(int row, int col)
{
	BYTE flg = 0;
	SetCellData(row, col, &flg);
}

BOOL CEditablePgGridData::IsBlank(int row, int col)
{
	if(IsUpdateCell(row, col) || IsInsertRow(row)) {
		BYTE *flg = (BYTE *)GetCellData(row, col);
		return (*flg != 0);
	}

	if(pg_dataset_is_null(m_dataset, GetRowIdx(row), col)) return FALSE;

	const TCHAR *str = pg_dataset_data(m_dataset, GetRowIdx(row), col);
	if(str[0] == '\0') return TRUE;

	return FALSE;
}

int CEditablePgGridData::UpdateCell(int row, int col, const TCHAR *data, int len)
{
	CUndoSetMode undo_set;

	if(data == blank_str) {
		if(!IsBlank(row, col)) {
			undo_set.Start(GetUndo(), TRUE);
			CEditableGridData::UpdateCell(row, col, _T("\b"), 1);
			SetBlank(row, col);
		}
		data = _T("");
	} else {
		if(IsBlank(row, col)) {
			undo_set.Start(GetUndo(), TRUE);
			CEditableGridData::UpdateCell(row, col, _T("\b"), 1);
			UnSetBlank(row, col);
		}
	}
	return CEditableGridData::UpdateCell(row, col, data, len);
}

