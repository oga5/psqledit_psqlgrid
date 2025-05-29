/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_OBJECTDETAILBAR_H__E33446C2_8AFD_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_OBJECTDETAILBAR_H__E33446C2_8AFD_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ObjectDetailBar.h : ヘッダー ファイル
//

#include "sizecbar.h"
#include "FileTabCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CObjectDetailBar ウィンドウ

class CObjectDetailBar : public CSizingControlBar
{
// コンストラクション
public:
	CObjectDetailBar();

// アトリビュート
private:
	CString		m_owner;
	CString		m_object_name;
	CString		m_oid;
	CString		m_schema;
	CString		m_object_type_name;
	int			m_object_type;
	HPgDataset	m_dataset;

	CFileTabCtrl	m_tab_ctrl;
	int			m_tab_ctrl_type;

	CListCtrl	m_list;
	int			m_list_type;

	int			m_font_height;
	CStatic		m_static_keyword;
	CEdit		m_edit_keyword;
	CButton		m_btn_keyword;
	BOOL		m_keyword_is_ok;

// オペレーション
private:
	CString GetSelectedItemString();
	int OnTabSelChanged();
	void OnDblClkListView();
	void OnListCtrlColumnClick(NM_LISTVIEW *p_nmhdr, CListCtrl *list_ctrl);
	void OnRClickListView();
	void OnRClickListView_Index();
	void OnRClickListView_Property();
	void OnKeywordChanged();

	int SetColumnList();
	int SetIndexList();
	int SetTriggerList();
	int SetPropertyList();

	void SetTabCtrl(const TCHAR *object_type);
	void AdjustChildWindows();
	void SetListColumns(int type);

	void SaveCurColumnWidth();

	int SetObjectList(HPgDataset dataset);

	CString MakeColumnList(int row_len, BOOL update, CString *insert_values, BOOL b_alias);
	CString MakeColumnListAll(int row_len, CString *insert_values, BOOL b_alias);
	CString	MakeObjectName();

	void SetKeywordOK(BOOL b_ok);

public:
	void FreeObjectList();

	void SetObjectType(TCHAR *object_type);
	int SetObjectData(const TCHAR *owner, const TCHAR *object_name,
		const TCHAR *oid, const TCHAR *schema, const TCHAR *object_type);
	void SetFont(CFont *font);
	void ClearColumnList();

	void LoadColumnWidth();
	void SaveColumnWidth();

	CString MakeSelectSql(BOOL b_all, BOOL b_use_semicolon);
	void MakeInsertSql(BOOL b_all = FALSE);
	void MakeUpdateSql();
	BOOL CanMakeSql();
	BOOL CanGetSource();

	void CopyValue();

	BOOL isKeywordActive();
	void PasteToKeyword();
	void CopyFromKeyword();
	void CutKeyword();
	void SelectAllKeyword();

	int GetSelectedObjectList(CStringArray* object_name_list, CStringArray* object_type_list,
		CStringArray* oid_list, CStringArray* schema_list);

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CObjectDetailBar)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CObjectDetailBar();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CObjectDetailBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_OBJECTDETAILBAR_H__E33446C2_8AFD_11D4_B06E_00E018A83B1B__INCLUDED_)
