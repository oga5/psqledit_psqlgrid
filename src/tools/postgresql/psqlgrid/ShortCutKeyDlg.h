/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SHORTCUTKEYDLG_H__4E39A183_9642_47D2_AD47_AA6876CC5F34__INCLUDED_)
#define AFX_SHORTCUTKEYDLG_H__4E39A183_9642_47D2_AD47_AA6876CC5F34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShortCutKeyDlg.h : ヘッダー ファイル
//

#include "AccelList.h"

/////////////////////////////////////////////////////////////////////////////
// CShortCutKeyDlg ダイアログ

class CShortCutKeyDlg : public CDialog
{
// コンストラクション
public:
	CShortCutKeyDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	CAccelList	m_accel_list;
	WORD		m_command;

// ダイアログ データ
	//{{AFX_DATA(CShortCutKeyDlg)
	enum { IDD = IDD_SHORTCUT_KEY_DLG };
	CListCtrl	m_list_accel;
	CEdit	m_edit_new_key;
	CButton	m_btn_delete;
	CButton	m_btn_add;
	CString	m_new_key;
	CString	m_current_menu;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CShortCutKeyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CShortCutKeyDlg)
	afx_msg void OnItemchangedListAccel(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnOk2();
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnChangeEditNewKey();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ResetAccelList();
	void SetAccelList(int menu_id);
	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SHORTCUTKEYDLG_H__4E39A183_9642_47D2_AD47_AA6876CC5F34__INCLUDED_)
