/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FILTEREDITCTRL_H__56BBE870_0878_47DE_8802_E3CBDA67E954__INCLUDED_)
#define AFX_FILTEREDITCTRL_H__56BBE870_0878_47DE_8802_E3CBDA67E954__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FilterEditCtrl.h : ヘッダー ファイル
//

#include "oregexp.h"

/////////////////////////////////////////////////////////////////////////////
// CFilterEditCtrl ウィンドウ

class CFilterEditCtrl : public CComboBox
{
// コンストラクション
public:
	CFilterEditCtrl();

// アトリビュート
public:

// オペレーション
public:
	BOOL Create(CString keyword_file_name, CRect rect, CWnd *parent, UINT nID);

	BOOL isKeywordOK() { return m_keyword_is_ok; }

	void CutKeyword();
	void SelectAllKeyword()	{ SetEditSel(0, -1); }
	void PasteToKeyword() { Paste(); }
	void CopyFromKeyword() { Copy(); }
	void DeleteKeyword() { Clear(); }
	void UndoKeyword() { GetEdit()->Undo(); }

	void AddFilterText(const TCHAR *add_text);
	void DeleteFilterText(const TCHAR *del_text);

	CEdit *GetEdit() { return (CEdit *)GetWindow(GW_CHILD); }

	CString GetFilterText();

	HREG_DATA GetRegexpData();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CFilterEditCtrl)
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CFilterEditCtrl();

	// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CFilterEditCtrl)
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	void SetKeywordOK(BOOL b_ok);
	void LoadFilterText();
	void SaveFilterText();

	BOOL m_keyword_is_ok;
	BOOL m_keyword_list_is_modify;
	CWnd *m_child_wnd;

	CString m_keyword_file_name;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FILTEREDITCTRL_H__56BBE870_0878_47DE_8802_E3CBDA67E954__INCLUDED_)
