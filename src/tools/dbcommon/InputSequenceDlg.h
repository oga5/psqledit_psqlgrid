/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_INPUTSEQUENCEDLG_H__ADE25C28_251D_4CAC_A23A_E1728FD9B7CB__INCLUDED_)
#define AFX_INPUTSEQUENCEDLG_H__ADE25C28_251D_4CAC_A23A_E1728FD9B7CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InputSequenceDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CInputSequenceDlg ダイアログ

class CInputSequenceDlg : public CDialog
{
// コンストラクション
public:
	CInputSequenceDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	__int64		m_incremental;
	__int64		m_start_num;

// ダイアログ データ
	//{{AFX_DATA(CInputSequenceDlg)
	enum { IDD = IDD_INPUT_SEQUENCE_DLG };
	CButton	m_ok;
	CString	m_incremental_str;
	CString	m_start_num_str;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CInputSequenceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CInputSequenceDlg)
	afx_msg void OnChangeEditStartNum();
	afx_msg void OnChangeEditIncremental();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CheckBtn();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_INPUTSEQUENCEDLG_H__ADE25C28_251D_4CAC_A23A_E1728FD9B7CB__INCLUDED_)
