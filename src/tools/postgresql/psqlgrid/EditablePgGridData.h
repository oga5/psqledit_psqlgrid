/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __EDITABLE_PG_EDIT_DATA_H_INCLUDED__
#define __EDITABLE_PG_EDIT_DATA_H_INCLUDED__

#include "EditableGridData.h"
#include "pglib.h"

#include "pgtype.h"


class CEditablePgGridData : public CEditableGridData
{
public:
	static TCHAR *blank_str;

	CEditablePgGridData();
	virtual ~CEditablePgGridData();

	// FIXME: null objectを使う
	virtual int Get_ColCnt() { 
		if(m_dataset == NULL) return 0;
		return pg_dataset_col_cnt(m_dataset) - m_reserve_columns;
	}

	virtual const TCHAR *Get_ColName(int col) {
		if(m_dataset == NULL) return _T("");
		return pg_dataset_get_colname(m_dataset, col);
	}
	virtual const TCHAR *GetOriginalText(int row, int col);

	virtual int Get_ColMaxSize(int col) {
		if(m_dataset == NULL) return 0;
		return pg_dataset_get_colsize(m_dataset, col);
	}

	virtual int Get_ColDataType(int row, int col) {
		pg_oid oid = pg_dataset_get_coltype(m_dataset, col);
		if(oid == INT8OID || oid == INT4OID || oid == INT2OID ||
			oid == FLOAT4OID || oid == FLOAT8OID) return GRID_DATA_NUMBER_TYPE;

		return GRID_DATA_CHAR_TYPE;
	}

	virtual BOOL IsColDataNull(int row, int col) {
		if(m_dataset == NULL) return FALSE;
		if(IsUpdateCell(row, col) || IsInsertRow(row)) {
			if(IsBlank(row, col)) return FALSE;
			return (Get_ColData(row, col)[0] == '\0');
		}
		return pg_dataset_is_null(m_dataset, GetRowIdx(row), col);
	}

	int SetDataset(HPgSession ss, HPgDataset dataset, HPgDataset pkey_dataset, const TCHAR *owner, 
		const TCHAR *table_name, const TCHAR *sql, BOOL b_data_lock);

	virtual int GetSystemReserveColumnCnt() { return m_reserve_columns; }

	int SaveData(HPgSession ss, POINT *err_pt, TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);

	virtual int UpdateCell(int row, int col, const TCHAR *data, int len);

protected:

private:
	HPgSession	m_ss;
	HPgDataset	m_null_dataset;
	HPgDataset	m_dataset;
	HPgDataset	m_pkey_dataset;

	CString		m_owner;
	CString		m_table_name;
	CString		m_sql;

	BOOL		m_b_data_lock;

	int			m_reserve_columns;

	CString GetUpdateWhereClause(int row);
	CString GetSqlColData(int row, int col);

	int DeleteData(HPgSession ss, POINT *err_pt, int *row_cnt, 
		TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);
	int InsertData(HPgSession ss, POINT *err_pt, int *row_cnt, 
		TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);
	int UpdateData(HPgSession ss, POINT *err_pt, int *row_cnt, 
		TCHAR *msg_buf, HWND hWnd, volatile int *cancel_flg);

	void SetBlank(int row, int col);
	void UnSetBlank(int row, int col);
	BOOL IsBlank(int row, int col);
};


unsigned int _stdcall save_grid_data_thr(void *lpvThreadParam);

struct _thr_save_grid_data_st {
	CEditablePgGridData *grid_data;
	HPgSession	ss;
	POINT		*err_pt;
	TCHAR		*msg_buf;
	void		*hWnd;
	volatile int *cancel_flg;
	int			ret_v;
};

#endif __EDITABLE_PG_EDIT_DATA_H_INCLUDED__