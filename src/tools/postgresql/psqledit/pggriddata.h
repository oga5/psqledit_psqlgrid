/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _ORA_GRID_DATA_H_INCLUDE
#define _ORA_GRID_DATA_H_INCLUDE

#include "griddata.h"
#include "pglib.h"

#include "pgtype.h"

class CPgGridData : public CGridData
{
public:
	CPgGridData() {
		m_null_dataset = NULL;
		m_dataset = m_null_dataset;
		m_row_idx = NULL;
	}
	virtual ~CPgGridData();

	// FIXME: null objectを使う
	virtual int Get_ColCnt() { 
		if(m_dataset == NULL) return 0;
		return pg_dataset_col_cnt(m_dataset);
	}
	virtual int Get_RowCnt() {
		if(m_dataset == NULL) return 0;
		return pg_dataset_row_cnt(m_dataset);
	}
	virtual const TCHAR *Get_ColName(int col) {
		if(m_dataset == NULL) return _T("");
		return pg_dataset_get_colname(m_dataset, col);
	}
	virtual const TCHAR *Get_ColData(int row, int col) {
		if(m_dataset == NULL) return _T("");
		return pg_dataset_data(m_dataset, GetRowIdx(row), col);
	}
	virtual int Get_ColMaxSize(int col) {
		if(m_dataset == NULL) return 0;
		return pg_dataset_get_colsize(m_dataset, col);
	}

	virtual int Get_ColDataType(int row, int col) {
		pg_oid oid = pg_dataset_get_coltype(m_dataset, col);
		if(oid == INT8OID || oid == INT4OID || oid == INT2OID ||
			oid == FLOAT4OID || oid == FLOAT8OID || oid == NUMERICOID) return GRID_DATA_NUMBER_TYPE;

		return GRID_DATA_CHAR_TYPE;
	}

	virtual BOOL IsColDataNull(int row, int col) {
		if(m_dataset == NULL) return FALSE;
		return pg_dataset_is_null(m_dataset, GetRowIdx(row), col);
	}

	virtual int GetMaxColLen(int col, int limit_len);

	void SetDataset(HPgDataset dataset);

protected:
	virtual void SwapRow(int r1, int r2);

private:
	HPgDataset	m_null_dataset;
	HPgDataset	m_dataset;
	int			*m_row_idx;

	int GetRowIdx(int row) {
		if(m_row_idx == NULL) return row;
		return m_row_idx[row];
	}

	void ClearRowIdx();
	void InitRowIdx();
};

#endif  _ORA_GRID_DATA_H_INCLUDE
