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

#if !defined(AFX_MAINFRM_H__62502B69_24F6_459B_9C8A_E23172BDE778__INCLUDED_)
#define AFX_MAINFRM_H__62502B69_24F6_459B_9C8A_E23172BDE778__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "accellist.h"

#define ACCEL_KIND_FRAME	0
#define ACCEL_KIND_DOC		1

#define INDICATOR_FILE_TYPE		1
#define INDICATOR_SESSION_INFO	2

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

	virtual void GetMessageString(UINT nID, CString& rMessage) const;

	CStatusBar	*GetStatusBar() { return &m_wndStatusBar; };
	void SetIdleMessage(const TCHAR* msg) { m_idle_msg = msg; }

	void InitAccelerator();
	void SetAccelerator(int accel_kind);

// アトリビュート
public:

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
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

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnNextDoc();
	afx_msg void OnPrevDoc();
	afx_msg void OnAccelerator();
	afx_msg void OnTabMoveLast();
	afx_msg void OnTabMoveLeft();
	afx_msg void OnTabMoveRight();
	afx_msg void OnTabMoveTop();
	afx_msg void OnUpdateTabMoveLast(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTabMoveLeft(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTabMoveRight(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTabMoveTop(CCmdUI* pCmdUI);
	afx_msg void OnSortTab();
	afx_msg void OnUpdateSortTab(CCmdUI* pCmdUI);
	afx_msg void OnCopyValue();
	afx_msg void OnSetupShortcutSql();
	afx_msg void OnViewObjectBar();
	afx_msg void OnUpdateViewObjectBar(CCmdUI* pCmdUI);
	afx_msg void OnViewObjectDetailBar();
	afx_msg void OnUpdateViewObjectDetailBar(CCmdUI* pCmdUI);
	afx_msg void OnViewTabBar();
	afx_msg void OnUpdateViewTabBar(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClose();
	//}}AFX_MSG
	afx_msg void OnUpdateCursorPos(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSessionInfo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateNextPrevDoc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridCalc(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGridCalcType(CCmdUI* pCmdUI);
	afx_msg void OnGridCalcType(UINT nID);
	afx_msg void OnUpdateFileType(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	CAccelList m_accel_list;
	int	   m_accel_kind;
	HACCEL m_doc_accel;
	HACCEL m_frame_accel;

	CString m_idle_msg;

	void SetBarState();

	void CreateAccelerator();
	void DestroyAccelerator();
	void SetAcceleratorToMenu(CMenu *pMenu);
	void SetWindowPosition(LPCREATESTRUCT lpCreateStruct);


	BOOL HitTestStatusBar(CPoint pt);
public:
	afx_msg void OnObjectbarFilterAdd();
	afx_msg void OnObjectbarFilterClear();
	afx_msg void OnObjectbarFilterDel();
	afx_msg void OnGrcalcCopy();
	afx_msg void OnUpdateGrcalcCopy(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MAINFRM_H__62502B69_24F6_459B_9C8A_E23172BDE778__INCLUDED_)
