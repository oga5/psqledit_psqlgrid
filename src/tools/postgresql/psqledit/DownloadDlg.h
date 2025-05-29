/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_DOWNLOADDLG_H__A6DC1C81_C4B7_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_DOWNLOADDLG_H__A6DC1C81_C4B7_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DownloadDlg.h : ヘッダー ファイル
//


/////////////////////////////////////////////////////////////////////////////
// CDownloadDlg ダイアログ

class CDownloadDlg : public CDialog
{
// コンストラクション
public:
	CDownloadDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CDownloadDlg)
	enum { IDD = IDD_DOWNLOAD_DLG };
	CButton	m_ok;
	CString	m_file_name;
	BOOL	m_save_select_result_only;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CDownloadDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CDownloadDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnFileSelect();
	afx_msg void OnChangeEditFileName();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void CheckBtn();
	void LoadSetup();
	void SaveSetup();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_DOWNLOADDLG_H__A6DC1C81_C4B7_11D5_8505_00E018A83B1B__INCLUDED_)
