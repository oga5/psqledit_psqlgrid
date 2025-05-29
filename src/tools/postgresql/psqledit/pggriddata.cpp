/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "pggriddata.h"
#include "ostrutil.h"
#include "str_inline.h"

int CPgGridData::GetMaxColLen(int col, int limit_len)
{
	int		i, len, max_len;
	int		row_cnt = Get_RowCnt();
// 数値データなどでは，Get_ColMaxSize()は使えない
//	int		col_max_size = Get_ColMaxSize(col) - 1;
//	if(col_max_size < 0) col_max_size = INT_MAX;

	for(i = 0, max_len = 0; i < row_cnt; i++) {
		len = (int)inline_strlen_first_line(pg_dataset_data(m_dataset, i, col), 2);
		if(len > max_len) {
			max_len = len;
			// ウィンドウ幅を超えたときは，これ以上最大幅を求めない
			if(max_len > limit_len) break;
			// カラムの最大幅になったときは，これ以上調べる必要がない
//			if(col_max_size == max_len) break;
		}
	}

	return max_len;
}

CPgGridData::~CPgGridData()
{
	ClearRowIdx();
}

void CPgGridData::ClearRowIdx()
{
	if(m_row_idx != NULL) {
		free(m_row_idx);
		m_row_idx = NULL;
	}
}

void CPgGridData::InitRowIdx()
{
	int		i;
	int		row_cnt = Get_RowCnt();

	m_row_idx = (int *)malloc(row_cnt * sizeof(int));
	if(m_row_idx == NULL) return;

	for(i = 0; i < row_cnt; i++) {
		m_row_idx[i] = i;
	}
}

void CPgGridData::SetDataset(HPgDataset dataset) 
{
	ClearRowIdx();
	m_dataset = dataset;
	AllocDispInfo();
}

void CPgGridData::SwapRow(int r1, int r2)
{
	if(m_row_idx == NULL) InitRowIdx();

	// FIXME: メモリが確保できない場合、SortDataComb11で無限ループになる
	if(m_row_idx == NULL) return;

	int tmp_r = m_row_idx[r1];
	m_row_idx[r1] = m_row_idx[r2];
	m_row_idx[r2] = tmp_r;
}

