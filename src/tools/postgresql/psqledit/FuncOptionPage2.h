/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_FUNCOPTIONPAGE2_H__299B0AEB_72B1_4E29_9649_DAC371E43639__INCLUDED_)
#define AFX_FUNCOPTIONPAGE2_H__299B0AEB_72B1_4E29_9649_DAC371E43639__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FuncOptionPage2.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CFuncOptionPage2 ダイアログ

class CFuncOptionPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CFuncOptionPage2)

// コンストラクション
public:
	CFuncOptionPage2();
	~CFuncOptionPage2();

	BOOL m_completion_object_type[COT_COUNT];

// ダイアログ データ
	//{{AFX_DATA(CFuncOptionPage2)
	enum { IDD = IDD_OPT_FUNC2 };
	CListCtrl	m_list_completion_type;
	BOOL	m_column_name_completion;
	BOOL	m_table_name_completion;
	BOOL	m_use_keyword_window;
	BOOL	m_enable_code_assist;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CFuncOptionPage2)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CFuncOptionPage2)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitCompletionObjectTypeList();
public:
	int m_code_assist_sort_column;
	int m_object_list_filter_column;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_FUNCOPTIONPAGE2_H__299B0AEB_72B1_4E29_9649_DAC371E43639__INCLUDED_)
