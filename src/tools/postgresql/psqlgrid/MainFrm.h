/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

 // MainFrm.h : CMainFrame クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__6B3C037A_6F93_48C2_B344_4239748AF36F__INCLUDED_)
#define AFX_MAINFRM_H__6B3C037A_6F93_48C2_B344_4239748AF36F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wheelsplit.h"

#include "accellist.h"

class CMainFrame : public CFrameWnd
{
protected: // シリアライズ機能のみから作成します。
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// アトリビュート
protected:

public:

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMainFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // コントロール バー用メンバ
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

	CWheelSplitterWnd		m_wndSplitter;

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnAccelerator();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	afx_msg void OnUpdateSessionInfo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGridCalc(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGridCalcType(CCmdUI* pCmdUI);
	afx_msg void OnGridCalcType(UINT nID);
	afx_msg void OnUpdateEnd(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

public:
	void InitAccelerator();
	CStatusBar	*GetStatusBar() { return &m_wndStatusBar; }

	BOOL SetIndicators();

private:
	CAccelList m_accel_list;
	HACCEL m_accel;

	void CreateAccelerator();
	void DestroyAccelerator();
	void SetAcceleratorToMenu(CMenu *pMenu);
	BOOL HitTestStatusBar(CPoint pt);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MAINFRM_H__6B3C037A_6F93_48C2_B344_4239748AF36F__INCLUDED_)
