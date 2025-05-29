/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

// psqlgridDoc.h : CPsqlgridDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSQLGRIDDOC_H__8473B417_B891_4739_8761_CCEDF3B42F26__INCLUDED_)
#define AFX_PSQLGRIDDOC_H__8473B417_B891_4739_8761_CCEDF3B42F26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EditablePgGridData.h"

#define UPD_WINDOW_MOVED			115
#define UPD_SET_HEADER_STYLE		202
#define UPD_INVALIDATE_CUR_CELL		203

class CPsqlgridDoc : public CDocument
{
protected: // シリアライズ機能のみから作成します。
	CPsqlgridDoc();
	DECLARE_DYNCREATE(CPsqlgridDoc)

// アトリビュート
public:

// オペレーション
public:
	CString GetTableName() { return m_table_name; }

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(COgridDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CPsqlgridDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HPgDataset				m_dataset;
	HPgDataset				m_pkey_dataset;
	CEditablePgGridData		m_grid_data;
	CGridData_SwapRowCol	m_grid_data_swap_row_col;
	CString					m_owner;
	CString					m_table_name;
	CStringList				m_column_name_list;
	CString					m_where_clause;
	CString					m_alias_name;
	TCHAR					m_msg_buf[1024];

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CPsqlgridDoc)
	afx_msg void OnSaveClose();
	afx_msg void OnSelectTable();
	afx_msg void OnUpdateSaveClose(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveGridAs();
	afx_msg void OnUpdateFileSaveGridAs(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectTable(CCmdUI* pCmdUI);
	afx_msg void OnGridSwapRowCol();
	afx_msg void OnUpdateGridSwapRowCol(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateDataLock(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	CGridData *GetGridData();
	CEditablePgGridData *GetPgGridData() { return &m_grid_data; }
	CGridData_Filter* GetFilterGridData() { return m_grid_data.GetFilterGridData(); }
	BOOL GetGridFilterMode() { return m_grid_data.GetGridFilterMode(); }
	void PostGridFilterOnOff();

	BOOL ClearData();

	void CalcGridSelectedData();
	BOOL GetGridSwapRowColMode() { return m_grid_swap_row_col_mode; }

private:
	int		SelectTable(BOOL auto_start, const TCHAR *file_name = NULL);
	int		SetOciDataset(HPgDataset dataset, HPgDataset pkey_dataset, const TCHAR *owner, 
		const TCHAR *table_name, const TCHAR *sql, BOOL adjust_col_width);
	void	FreeDataset();

	BOOL		m_grid_swap_row_col_mode;
	BOOL		m_grid_swap_row_col_disp_flg;
public:
	afx_msg void OnGrfilterOn();
	afx_msg void OnUpdateGrfilterOn(CCmdUI* pCmdUI);
	afx_msg void OnGrfilterOff();
	afx_msg void OnUpdateGrfilterOff(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PSQLGRIDDOC_H__8473B417_B891_4739_8761_CCEDF3B42F26__INCLUDED_)
