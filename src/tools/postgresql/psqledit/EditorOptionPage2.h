/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_EDITOROPTIONPAGE2_H__B963AAA8_8099_43B1_80D0_C68E7EBA2405__INCLUDED_)
#define AFX_EDITOROPTIONPAGE2_H__B963AAA8_8099_43B1_80D0_C68E7EBA2405__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorOptionPage2.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CEditorOptionPage2 ダイアログ

class CEditorOptionPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CEditorOptionPage2)

// コンストラクション
public:
	CEditorOptionPage2();
	~CEditorOptionPage2();

// ダイアログ データ
	//{{AFX_DATA(CEditorOptionPage2)
	enum { IDD = IDD_OPT_EDITOR2 };
	BOOL	m_auto_indent;
	BOOL	m_copy_lower_name;
	BOOL	m_drag_drop_edit;
	BOOL	m_tab_as_space;
	BOOL	m_row_copy_at_no_sel;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CEditorOptionPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CEditorOptionPage2)
		// メモ: ClassWizard はこの位置にメンバ関数を追加します。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_EDITOROPTIONPAGE2_H__B963AAA8_8099_43B1_80D0_C68E7EBA2405__INCLUDED_)
