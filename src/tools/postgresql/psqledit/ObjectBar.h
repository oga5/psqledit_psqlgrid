/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_OBJECTBAR_H__E3ABD2A0_8AFB_11D4_B06E_00E018A83B1B__INCLUDED_)
#define AFX_OBJECTBAR_H__E3ABD2A0_8AFB_11D4_B06E_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ObjectBar.h : ヘッダー ファイル
//

#include "sizecbar.h"
#include "pglib.h"

/////////////////////////////////////////////////////////////////////////////
// CObjectBar ウィンドウ

class CObjectBar : public CSizingControlBar
{
// コンストラクション
public:
	CObjectBar();
	virtual ~CObjectBar();

// アトリビュート
private:
	CListCtrl	m_list;
	CComboBox	m_combo_owner;
	CComboBox	m_combo_type;
	CStatic		m_static_owner;
	CStatic		m_static_type;
	int			m_font_height;
	CString		m_cur_type;
	CStatic		m_static_keyword;
	CEdit		m_edit_keyword;
	CButton		m_btn_keyword;
	BOOL		m_keyword_is_ok;

	HPgDataset	m_object_list;

// オペレーション
private:
	int SetUserList(const TCHAR *user_name);
	int SetObjectList(const TCHAR *object_name);
	int SetObjectList(HPgDataset dataset, const TCHAR *object_name);
	
	void OnDblClkListView();
	void OnRClickListView();
	void OnKeywordChanged();
	void OnListViewItemChanged(NM_LISTVIEW *p_nmhdr);
	void OnListViewColumnClick(NM_LISTVIEW *p_nmhdr);

	void OnOwnerChanged();
	void OnTypeChanged();

	void SetControlPosition();

	void SaveCurColumnWidth();

	void SetKeywordOK(BOOL b_ok);

	int GetColumnIdx(const TCHAR *colname);

	CString GetSelectedData(const TCHAR *colname);

	int GetSelectedRow();
	CString GetSelectedType2(int selected_row);

public:
	int InitializeList(const TCHAR *user_name);
	void FreeObjectList();

	int RefreshList();
	void SetFont(CFont *font);
	CString GetSelectedUser();
	CString GetSelectedType();
	CString GetSelectedObject();
	CString GetSelectedOid();
	CString GetSelectedSchema();

	int GetSelectedObjectList(CStringArray* object_name_list, CStringArray* object_type_list,
		CStringArray* oid_list, CStringArray* schema_list);

	void LoadColumnWidth();
	void SaveColumnWidth();

	void ClearData();

	BOOL isKeywordActive();
	void PasteToKeyword();
	void CopyFromKeyword();
	void CutKeyword();
	void SelectAllKeyword();

	BOOL CanGetSource();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CObjectBar)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

	// 生成されたメッセージ マップ関数
protected:
	int SetListColumns(HPgDataset dataset);
	//{{AFX_MSG(CObjectBar)
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

#endif // !defined(AFX_OJECTBAR_H__E3ABD2A0_8AFB_11D4_B06E_00E018A83B1B__INCLUDED_)
