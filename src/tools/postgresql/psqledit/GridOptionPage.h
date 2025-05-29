/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_GRIDOPTIONPAGE_H__1F3DEE65_4156_4F17_A2E7_1AA75D91256D__INCLUDED_)
#define AFX_GRIDOPTIONPAGE_H__1F3DEE65_4156_4F17_A2E7_1AA75D91256D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridOptionPage.h : ヘッダー ファイル
//

#include "StrGridData.h"
#include "GridCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CGridOptionPage ダイアログ

class CGridOptionPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CGridOptionPage)

// コンストラクション
public:
	CGridOptionPage();
	~CGridOptionPage();

	COLORREF m_color[GRID_CTRL_COLOR_CNT];


// ダイアログ データ
	//{{AFX_DATA(CGridOptionPage)
	enum { IDD = IDD_OPT_GRID };
	BOOL	m_show_2byte_space;
	BOOL	m_show_space;
	BOOL	m_show_line_feed;
	BOOL	m_show_tab;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CGridOptionPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CGridOptionPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnBtnDefaultColor();
	afx_msg void OnCheckShow2byteSpace();
	afx_msg void OnCheckShowSpace();
	afx_msg void OnCheckShowLineFeed();
	afx_msg void OnCheckShowTab();
	//}}AFX_MSG
	afx_msg void OnColorBtn(unsigned int ctrl_id);
	DECLARE_MESSAGE_MAP()

private:
	CStrGridData	m_grid_data;
	CGridCtrl		m_grid_ctrl;

	int GetColorId(unsigned int ctrl_id);
	void CreateGridCtrl();
	void SetGridColor();
	void SetGridOption();
public:
	BOOL m_col_header_dbl_clk_paste;
	int m_cell_padding_top;
	int m_cell_padding_right;
	int m_cell_padding_left;
	int m_cell_padding_bottom;
	afx_msg void OnChangeEditCellPaddingBottom();
	afx_msg void OnChangeEditCellPaddingLeft();
	afx_msg void OnChangeEditCellPaddingRight();
	afx_msg void OnChangeEditCellPaddingTop();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_GRIDOPTIONPAGE_H__1F3DEE65_4156_4F17_A2E7_1AA75D91256D__INCLUDED_)
