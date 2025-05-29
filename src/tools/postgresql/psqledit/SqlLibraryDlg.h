/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SQLLIBRARYDLG_H__7EC5CC60_13B8_11D5_8505_00E018A83B1B__INCLUDED_)
#define AFX_SQLLIBRARYDLG_H__7EC5CC60_13B8_11D5_8505_00E018A83B1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SqlLibraryDlg.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CSqlLibraryDlg ダイアログ

class CSqlLibraryDlg : public CDialog
{
// コンストラクション
public:
	CSqlLibraryDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	CString		m_root_dir;
	CFont		*m_font;

	CString		m_file_name;

// ダイアログ データ
	//{{AFX_DATA(CSqlLibraryDlg)
	enum { IDD = IDD_SQL_LIBRARY_DLG };
	CButton	m_ok;
	CListCtrl	m_list;
	CString	m_category;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSqlLibraryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	CImageList	m_image_list;
	CString		m_current_dir;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSqlLibraryDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkSqlList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedSqlList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickSqlList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditIndex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int SetFileData(int row, TCHAR *file_name, CStringList &str_list);
	int CreateIndex(CString dir_name, CStringList &str_list);
	int SetSqlList(CString dir_name);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SQLLIBRARYDLG_H__7EC5CC60_13B8_11D5_8505_00E018A83B1B__INCLUDED_)
