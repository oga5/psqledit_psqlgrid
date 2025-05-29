/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // ChildFrm.h : CChildFrame クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__439F0FC3_E168_42ED_AB5B_20B12A360DBC__INCLUDED_)
#define AFX_CHILDFRM_H__439F0FC3_E168_42ED_AB5B_20B12A360DBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wheelSplit.h"

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();
	
	void SwitchView(int view_kind);
	int GetViewKind() { return m_view_kind; }

	void SetPaneHeight(int pane_height = -1);
	int GetPaneHeight();

private:
	CWheelSplitterWnd	m_wnd_splitter;
	CWheelSplitterWnd	m_wnd_grid_splitter;
	CWheelSplitterWnd	m_wnd_edit_splitter;

	CView	*m_msg_view;
	int		m_view_kind;
	int		m_sql_editor_height;

	void SetTabBar();
	void RestorePain1();

// アトリビュート
public:

// オペレーション
public:

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CChildFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 生成したメッセージ マップ関数
protected:
	//{{AFX_MSG(CChildFrame)
	afx_msg void OnAdjustAllColWidth();
	afx_msg void OnUpdateAdjustAllColWidth(CCmdUI* pCmdUI);
	afx_msg void OnEqualAllColWidth();
	afx_msg void OnUpdateEqualAllColWidth(CCmdUI* pCmdUI);
	afx_msg void OnActiveResultGrid();
	afx_msg void OnActiveSqlEditor();
	afx_msg void OnActiveSqlResult();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSqlEditorMaximize();
	afx_msg void OnResultGridMaximize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_CHILDFRM_H__439F0FC3_E168_42ED_AB5B_20B12A360DBC__INCLUDED_)
