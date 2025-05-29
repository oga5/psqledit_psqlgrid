/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_GRIDFILTERDLG_H__8F6F9980_626D_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_GRIDFILTERDLG_H__8F6F9980_626D_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GridFilterDlg.h : ヘッダー ファイル
//


#include "resource.h"
#include "SearchDlg.h"
#include "SearchDlgData.h"
#include "DlgSingletonTmpl.h"
#include "GridData.h"
#include "afxwin.h"

// NOTE: SearchDlg用の値と重複しないようにする
#define WM_GRID_FILTER_DLG_BASE	WM_SEARCH_DLG_BASE + 100
#define WM_GRID_FILTER_RUN		WM_GRID_FILTER_DLG_BASE + 1
#define WM_GRID_FILTER_CLEAR	WM_GRID_FILTER_DLG_BASE + 2

/////////////////////////////////////////////////////////////////////////////
// CGridFilterDlg ダイアログ

class CGridFilterDlg : public CDialog
{
// コンストラクション
public:
	CGridFilterDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ
	~CGridFilterDlg();

	void ShowDialog(CWnd *wnd, CSearchDlgData* search_data, 
		CGridData* grid_data, int filter_col_no, BOOL b_close_at_found);
	INT_PTR DoModal(CWnd *wnd, CSearchDlgData *search_data, 
		CGridData* grid_data, int filter_col_no, BOOL b_close_at_found);

	CWnd *GetWnd() { return m_wnd; }
	void SetWnd(CWnd *wnd) { m_wnd = wnd; }

	CString		m_title;

	int m_filter_col_no { 0 };

// ダイアログ データ
	//{{AFX_DATA(CGridFilterDlg)
	enum { IDD = IDD_GRID_FILTER_DLG };
	CButton	m_btn_check_regexp;
	CButton	m_btn_cancel;
	CComboBox	m_combo_search_text;
	CButton	m_btn_filter_run;
	BOOL	m_distinct_lwr_upr;
	BOOL	m_regexp;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CGridFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CGridFilterDlg)
	afx_msg void OnBtnFilterRun();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeComboSearchText();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnOK();
	afx_msg void OnSelendokComboSearchText();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnCancel();
	//}}AFX_MSG
	afx_msg void OnGetMinMaxInfo( MINMAXINFO FAR* lpMMI );
	DECLARE_MESSAGE_MAP()
private:
	BOOL		m_initialized;
	BOOL		m_b_modal;
	BOOL		m_close_at_found;
	CWnd		*m_wnd;
	CSearchDlgData* m_data{ NULL };
	int			*m_search_status;
	
	CString		m_search_text;
	int			m_dlg_height;
	CButton m_btn_bottom;

	CGridData* m_grid_data{ NULL };

	void InitDialog();
	void InitColumnComboBox(CComboBox* combo, int sel);
	void GetData();

	void CheckBtn();
	void SaveGridFilterDlgData();

	void SaveSearchTextList();
	void LoadSearchTextList();

	void LoadDlgSize();
	void SaveDlgSize();
	void RelayoutControls();

	BOOL UpdateData(BOOL bSaveAndValidate = TRUE);
public:
	BOOL m_distinct_width_ascii;
private:
	CComboBox m_combo_grid_column;
public:
	CButton m_btn_filter_clear;
	afx_msg void OnClickedFilterClear();
};

typedef DlgSingletonTmpl<CGridFilterDlg, IDD_GRID_FILTER_DLG> CGridFilterDlgSingleton;


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_GRIDFILTERDLG_H__8F6F9980_626D_11D4_B06E_00E018A83B1B__INCLUDED_)
