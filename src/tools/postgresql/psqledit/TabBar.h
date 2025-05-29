/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_TABBAR_H__94D9BB21_13D4_11D6_850B_00E018A83B1B__INCLUDED_)
#define AFX_TABBAR_H__94D9BB21_13D4_11D6_850B_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabBar.h : ヘッダー ファイル
//

#include "sizecbar.h"

/////////////////////////////////////////////////////////////////////////////
// CTabBar ウィンドウ

#include "FileTabCtrl.h"


class CTabBar : public CSizingControlBar
{
// コンストラクション
public:
	CTabBar();

// アトリビュート
public:

private:
	CFileTabCtrl	m_tab_ctrl;
	int			m_font_height;

// オペレーション
public:
	void SetFont(CFont *font);
	void InsertDoc(CDocument *pdoc);
	void DeleteDoc(CDocument *pdoc);
	void SetTabName(CDocument *pdoc);
	void ActiveTab(CDocument *pdoc);

	void SetTabNameAll();

	void TabMoveLeft();
	void TabMoveRight();
	void TabMoveTop();
	void TabMoveLast();
	BOOL CanTabMoveLeft();
	BOOL CanTabMoveRight();

	void SortTab();
	BOOL CanSortTab();

	void NextDoc();
	void PrevDoc();
	BOOL CanNextPrevDoc();

	void SetOption();
	CDocument* GetDoc(int tab_idx);

	CString GetCurrentTabName();

private:
	void ActiveDoc(int tab_idx);
	void AdjustChildWindows();
	int SearchDoc(CDocument *pdoc);
	void SetTabName(int tab_idx);
	void CloseActiveDocument();

	int TabHitTest(CPoint point);
	void OnTabSelChanged();
	void OnTabMClicked();
	void OnTabRClicked();
	void OnTabToolTipName(NMHDR *p_nmhdr);

	void MoveTab(int old_pos, int new_pos);

	CString GetTabName(CDocument *pdoc);

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CTabBar)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CTabBar();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CTabBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TABBAR_H__94D9BB21_13D4_11D6_850B_00E018A83B1B__INCLUDED_)
