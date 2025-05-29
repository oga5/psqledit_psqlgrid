/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
// psqlgridView.h : CPsqlgridView クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSQLGRIDVIEW_H__EB7B91DC_0859_434F_86D4_92163D2C07C3__INCLUDED_)
#define AFX_PSQLGRIDVIEW_H__EB7B91DC_0859_434F_86D4_92163D2C07C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "gridctrl.h"
#include "searchDlg.h"

class CPsqlgridView : public CScrollView
{
protected: // シリアライズ機能のみから作成します。
	CPsqlgridView();
	DECLARE_DYNCREATE(CPsqlgridView)

	CGridCtrl			m_grid_ctrl;
	CSearchDlgData		m_search_data;

// アトリビュート
public:
	CPsqlgridDoc* GetDocument();

// オペレーション
public:

private:
	void SearchMsg(int ret_v, int dir, BOOL b_looped);
	void SetHeaderStyle();
	void SetGridOption();

	void DeleteNullRows(BOOL all_flg);

	BOOL OnUpdateDataType(CCmdUI *pCmdUI, int type);

	void InitSortData(int *sort_col_no, int *sort_order, int &sort_col_cnt, int order);
	void SortData(int *sort_col_no, int *sort_order, int sort_col_cnt);

	void GridFilterOn();
	void GridFilterOff();
	int GridFilterRun(int grid_filter_col_no);

	void SetStatusBarInfo();

	void PostGridFilterOnOff();

	void HideSearchDlgs();


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CPsqlgridView)
	public:
	virtual void OnDraw(CDC* pDC);  // このビューを描画する際にオーバーライドされます。
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CPsqlgridView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CPsqlgridView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDeleteRow();
	afx_msg void OnInsertRow();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnUpdateDeleteRow(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsertRow(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditCut();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEnterEdit();
	afx_msg void OnInputEnter();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnSearch();
	afx_msg void OnSearchNext();
	afx_msg void OnSearchPrev();
	afx_msg void OnUpdateInsertRows(CCmdUI* pCmdUI);
	afx_msg void OnInsertRows();
	afx_msg void OnGridCopyCsv();
	afx_msg void OnGridCopyCsvCname();
	afx_msg void OnGridCopyTab();
	afx_msg void OnGridCopyTabCname();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnReplace();
	afx_msg void OnUpdateSearch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSearchNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSearchPrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReplace(CCmdUI* pCmdUI);
	afx_msg void OnGridCopyFixLen();
	afx_msg void OnGridCopyFixLenCname();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnClearSearchText();
	afx_msg void OnDeleteNullRows();
	afx_msg void OnEditPasteToCell();
	afx_msg void OnUpdateEditPasteToCell(CCmdUI* pCmdUI);
	afx_msg void OnDataEdit();
	afx_msg void OnUpdateDataEdit(CCmdUI* pCmdUI);
	afx_msg void OnDeleteAllNullRows();
	afx_msg void OnUpdateDeleteAllNullRows(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDeleteNullRows(CCmdUI* pCmdUI);
	afx_msg void OnToLower();
	afx_msg void OnToUpper();
	afx_msg void OnZenkakuToHankaku();
	afx_msg void OnZenkakuToHankakuAlpha();
	afx_msg void OnZenkakuToHankakuKata();
	afx_msg void OnHankakuToZenkaku();
	afx_msg void OnHankakuToZenkakuAlpha();
	afx_msg void OnHankakuToZenkakuKata();
	afx_msg void OnInputSequence();
	afx_msg void OnUpdateInputSequence(CCmdUI* pCmdUI);
	afx_msg void OnAdjustAllColWidthDataonly();
	afx_msg void OnAdjustAllColWidthUseColname();
	afx_msg void OnGridSort();
	afx_msg void OnSearchColumn();
	afx_msg void OnEqualAllColWidth();
	afx_msg void OnDataSetBlank();
	//}}AFX_MSG
	afx_msg void OnUpdateEnd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateValidCurPt(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnGrcolumnWidth();
	afx_msg void OnUpdateGrcolumnWidth(CCmdUI* pCmdUI);
};


#ifndef _DEBUG  // psqlgridView.cpp ファイルがデバッグ環境の時使用されます。
inline CPsqlgridDoc* CPsqlgridView::GetDocument()
   { return (CPsqlgridDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PSQLGRIDVIEW_H__EB7B91DC_0859_434F_86D4_92163D2C07C3__INCLUDED_)
