/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_GRIDVIEW_H__48B22260_491D_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_GRIDVIEW_H__48B22260_491D_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GridView.h : ヘッダー ファイル
//

#include "PSqlEditDoc.h"
#include "searchDlg.h"

#define WM_SEARCH_COLUMN	WM_USER + 201

/////////////////////////////////////////////////////////////////////////////
// CGridView ビュー

#include "GridCtrl.h"

class CGridView : public CView
{
protected:
	DECLARE_DYNCREATE(CGridView)

private:
	CGridCtrl		m_grid_ctrl;
	CString			m_grid_sql;
	CString			m_grid_msg;

	void SetGridOption();

// アトリビュート
public:

// オペレーション
public:
	CGridView();           // 動的生成に使用されるプロテクト コンストラクタ
	CPsqleditDoc* GetDocument() { return (CPsqleditDoc*)m_pDocument; };

	void AdjustAllColWidth();
	void EqualAllColWidth();

private:
	void SearchMsg(int ret_v, int dir, BOOL b_looped);
	void SetStatusBarInfo();

	void SwapRowCol();
	void GridFilterOn();
	void GridFilterOff();
	int GridFilterRun(int grid_filter_col_no);
	void PostGridFilterOnOff();

	void CellChanged(int zero_base_x, int zero_base_y);

	void InitSortData(int *sort_col_no, int *sort_order, int &sort_col_cnt, int order);
	void SortData(int *sort_col_no, int *sort_order, int sort_col_cnt);
	void SetGridSql(const TCHAR *sql);

	void SetHeaderStyle();
	void SetGridMsg(const TCHAR* msg);

	POINT GetCurCellOnDataset();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CGridView)
	protected:
	virtual void OnDraw(CDC* pDC);      // このビューを描画するためにオーバーライドしました。
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	virtual ~CGridView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CGridView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditCopy();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSearch();
	afx_msg void OnSearchNext();
	afx_msg void OnSearchPrev();
	afx_msg void OnSelectAll();
	afx_msg void OnGridCopyCsv();
	afx_msg void OnGridCopyCsvCname();
	afx_msg void OnGridCopyTab();
	afx_msg void OnGridCopyTabCname();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnGridCopyFixLen();
	afx_msg void OnGridCopyFixLenCname();
	afx_msg void OnEditPaste();
	afx_msg void OnClearSearchText();
	afx_msg void OnGridSort();
	//}}AFX_MSG
	afx_msg void OnUpdateDisableMenu(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateGrcreateInsertSql(CCmdUI *pCmdUI);
	afx_msg void OnGrcreateInsertSql();
	afx_msg void OnSearchColumn();
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditCopyColumnName();
	afx_msg void OnUpdateEditCopyColumnName(CCmdUI* pCmdUI);
	afx_msg void OnShowGridData();
	afx_msg void OnUpdateShowGridData(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnGrcolumnWidth();
	afx_msg void OnUpdateGrcolumnWidth(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_GRIDVIEW_H__48B22260_491D_11D4_B06E_00E018A83B1B__INCLUDED_)
