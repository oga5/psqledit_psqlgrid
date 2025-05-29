/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SHORTCUTSQLLISTDLG_H__1BEB40EC_6259_4744_9CA6_44D061957AE5__INCLUDED_)
#define AFX_SHORTCUTSQLLISTDLG_H__1BEB40EC_6259_4744_9CA6_44D061957AE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShortCutSqlListDlg.h : ヘッダー ファイル
//

#include "shortcutsql.h"
#include "AccelList.h"

/////////////////////////////////////////////////////////////////////////////
// CShortCutSqlListDlg ダイアログ

class CShortCutSqlListDlg : public CDialog
{
// コンストラクション
public:
	CShortCutSqlListDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	CAccelList			m_accel_list;

// ダイアログ データ
	//{{AFX_DATA(CShortCutSqlListDlg)
	enum { IDD = IDD_SHORTCUT_SQL_LIST_DLG };
	CButton	m_btn_modify;
	CButton	m_btn_delete;
	CButton	m_btn_add;
	CListCtrl	m_list_view;
	CString	m_max_cnt;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CShortCutSqlListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CShortCutSqlListDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnOk2();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnBtnModify();
	afx_msg void OnItemchangedSqlList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL SaveData();

	BOOL InitSqlListView();
	BOOL InitSqlList();

	BOOL AddList(CString name, CString sql, WORD cmd, BOOL show_dlg, 
		BOOL paste_to_editor, int idx);
	BOOL SetList(CString name, CString sql, WORD cmd, BOOL show_dlg, 
		BOOL paste_to_editor, int idx);

	void SetListKey();

	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SHORTCUTSQLLISTDLG_H__1BEB40EC_6259_4744_9CA6_44D061957AE5__INCLUDED_)
