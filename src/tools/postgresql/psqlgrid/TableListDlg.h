/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_TABLELISTDLG_H__023053D5_FC28_11D4_8504_00E018A83B1B__INCLUDED_)
#define AFX_TABLELISTDLG_H__023053D5_FC28_11D4_8504_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TableListDlg.h : ヘッダー ファイル
//

#include "pglib.h"
#include "EditCtrl.h"
#include "EditData.h"

/////////////////////////////////////////////////////////////////////////////
// CTableListDlg ダイアログ

class CTableListDlg : public CDialog
{
// コンストラクション
public:
	CTableListDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ
	~CTableListDlg();

	HPgSession	m_ss;
	HPgDataset	m_dataset;
	HPgDataset	m_pkey_dataset;
	CString		m_owner;
	CString		m_table_name;
	//CString		m_schema_name;
	CStringList	m_column_name_list;
	CString		m_where_clause;
	CString		m_alias_name;
	BOOL		m_auto_start;
	BOOL		m_grid_swap_row_col_mode;
	CFont		*m_font;
	CStrToken	*m_str_token;

	CString		m_osg_file_name;
	CString		m_sql;

// ダイアログ データ
	//{{AFX_DATA(CTableListDlg)
	enum { IDD = IDD_TABLE_LIST_DLG };
	CSpinButtonCtrl	m_spin_selected_column;
	CButton	m_btn_clear_column;
	CButton	m_btn_del;
	CButton	m_btn_add;
	CListCtrl	m_selected_column_list;
	CStatic	m_static_filter;
	CEdit	m_edit_filter;
	CButton	m_btn_filter;
	CButton	m_btn_load;
	CButton	m_btn_save;
	CButton	m_check_data_lock;
	CStatic	m_static_where;
	CStatic	m_static_table;
	CStatic	m_static_column;
	CButton	m_sel_all_btn;
	CButton	m_cancel;
	CListCtrl	m_table_list;
	CListCtrl	m_column_list;
	CButton	m_ok;
	BOOL	m_data_lock;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CTableListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	UINT_PTR	m_auto_start_timer;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CTableListDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnItemchangedTableList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedColumnList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonSelectAllColumn();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnBtnLoad();
	afx_msg void OnBtnSave();
	afx_msg void OnBtnFilter();
	afx_msg void OnChangeEditFilter();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonClearColumnList();
	afx_msg void OnButtonDel();
	afx_msg void OnItemchangedSelectedColumnList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkColumnList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaposSpinSelectedColumn(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void CreateEditCtrl();
	CString GetAllTableName();
	CString GetOwner();
	CString GetObjectType();
	CString GetTableName();
	CString GetSchemaName();

	CString GetColumnDataType(const TCHAR *col_name);

	void SplitSchemaAndTable(const TCHAR *name, CString &schema_name, CString &table_name);
	CString GetTableOwner(const TCHAR *name);

	BOOL MakeColumnNameList(BOOL b_check);
	int SetUserList(const TCHAR *user_name);
	int SetTableList(const TCHAR *owner);
	int SetTableList(HPgDataset dataset);

	int SetColumnList(const TCHAR *owner, const TCHAR *schema_name, 
		const TCHAR *table_name, const TCHAR *type, BOOL b_all_select = FALSE);

	int SetColumnList_Table(const TCHAR *owner, const TCHAR *schema_name, 
		const TCHAR *table_name, BOOL b_all_select = FALSE);
	int SetColumnList_Table(HPgDataset dataset, BOOL b_all_select);

	int CreateOciDataset();

	void CheckButtons();
	void CheckOKButton();
	void CheckAddDelBtn();
	void CheckSpinBtn();

	void SwapSelectedColumn(int iDelta);

	void RelayoutControl();
	BOOL LoadFile(TCHAR *file_name);

	BOOL SelectListData(CListCtrl *list, TCHAR *key, BOOL b_visible);
	void ClearListSelected(CListCtrl *list);

	BOOL SelectTable(TCHAR *table_name);
	
	void SetKeywordOK(BOOL b_ok);

	BOOL AddColumn(const TCHAR *column_name, BOOL b_check);

	TCHAR m_msg_buf[1024];
	CStringList	m_null_column_name_list;

	CEditData	m_edit_data;
	CEditCtrl	m_edit_ctrl;

	BOOL		m_init;

	HPgDataset	m_table_dataset;
	BOOL		m_keyword_is_ok;
public:
	CComboBox m_combo_owner;
	CStatic m_static_owner;
	afx_msg void OnSelchangeComboOwner();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TABLELISTDLG_H__023053D5_FC28_11D4_8504_00E018A83B1B__INCLUDED_)
