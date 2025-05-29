/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SQLEXPLAINVIEW_H__97EA593E_62EF_11D3_8FED_444553540000__INCLUDED_)
#define AFX_SQLEXPLAINVIEW_H__97EA593E_62EF_11D3_8FED_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SQLExplainView.h : ヘッダー ファイル
//

#include "PSQLEditDoc.h"
#include "editctrl.h"

#include "searchDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSQLExplainView ビュー

class CSQLExplainView : public CView
{
protected:
	DECLARE_DYNCREATE(CSQLExplainView)

// アトリビュート
public:
	CSQLExplainView();           // 動的生成に使用されるプロテクト コンストラクタ
	CPsqleditDoc* GetDocument();

private:
	CEditCtrl	m_edit_ctrl;
	void MoveCaretEndData();
	void PutResult(TCHAR *msg);
	void PutStatusBarMsg(TCHAR *msg);
	void PutSepalator();
	void SearchMsg(int ret_v, int dir, BOOL b_looped);

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSQLExplainView)
	protected:
	virtual void OnDraw(CDC* pDC);      // このビューを描画するためにオーバーライドしました。
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	virtual ~CSQLExplainView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CSQLExplainView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditCopy();
	afx_msg void OnSelectAll();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSearch();
	afx_msg void OnSearchNext();
	afx_msg void OnSearchPrev();
	afx_msg void OnClearSearchText();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnCharLeft();
	afx_msg void OnCharLeftExtend();
	afx_msg void OnCharRight();
	afx_msg void OnCharRightExtend();
	afx_msg void OnLineUp();
	afx_msg void OnLineUpExtend();
	afx_msg void OnLineDown();
	afx_msg void OnLineDownExtend();
	afx_msg void OnWordLeft();
	afx_msg void OnWordLeftExtend();
	afx_msg void OnWordRight();
	afx_msg void OnWordRightExtend();
	afx_msg void OnLineStart();
	afx_msg void OnLineStartExtend();
	afx_msg void OnLineEnd();
	afx_msg void OnLineEndExtend();
	afx_msg void OnDocumentEnd();
	afx_msg void OnDocumentEndExtend();
	afx_msg void OnDocumentStart();
	afx_msg void OnDocumentStartExtend();
	afx_msg void OnScrollUp();
	afx_msg void OnScrollDown();
	afx_msg void OnScrollPageUp();
	afx_msg void OnScrollPageDown();
	afx_msg void OnPageUp();
	afx_msg void OnPageDown();
	afx_msg void OnPageDownExtend();
	afx_msg void OnPageUpExtend();
	afx_msg void OnSelectRow();
	afx_msg void OnBoxSelect();
	afx_msg void OnEditPaste();
	//}}AFX_MSG
	afx_msg void OnUpdateDisableMenu(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	void SetEditorOption();
public:
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

inline CPsqleditDoc* CSQLExplainView::GetDocument()
   { return (CPsqleditDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SQLEXPLAINVIEW_H__97EA593E_62EF_11D3_8FED_444553540000__INCLUDED_)
