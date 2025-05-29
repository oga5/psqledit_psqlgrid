/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_GRIDSORTDLG_H__C375A67B_575E_46AF_B422_9F58347C2BF2__INCLUDED_)
#define AFX_GRIDSORTDLG_H__C375A67B_575E_46AF_B422_9F58347C2BF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GridSortDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CGridSortDlg ダイアログ
#include "resource.h"
#include "griddata.h"

class CGridSortDlg : public CDialog
{
// コンストラクション
public:
	CGridSortDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void InitData(CGridData *grid_data, int *sort_col_no, int *sort_order,
		int *sort_cnt);

private:
	void InitComboBox(CComboBox *combo, int sel);
	void CheckBtn();
	void GetData();

	CGridData *m_grid_data;
	int *m_sort_col_no;
	int *m_sort_order;
	int *m_sort_cnt;

// ダイアログ データ
	//{{AFX_DATA(CGridSortDlg)
	enum { IDD = IDD_GRID_SORT_DLG };
	CButton	m_ok;
	CComboBox	m_combo_sort_key3;
	CComboBox	m_combo_sort_key2;
	CComboBox	m_combo_sort_key1;
	int		m_sort_order1;
	int		m_sort_order2;
	int		m_sort_order3;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CGridSortDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CGridSortDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboSortKey1();
	afx_msg void OnSelchangeComboSortKey2();
	afx_msg void OnSelchangeComboSortKey3();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_GRIDSORTDLG_H__C375A67B_575E_46AF_B422_9F58347C2BF2__INCLUDED_)
