/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SQLLOGFILEDLG_H__D0D9FDFB_6D45_447B_979E_9E64A4F7E685__INCLUDED_)
#define AFX_SQLLOGFILEDLG_H__D0D9FDFB_6D45_447B_979E_9E64A4F7E685__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SqlLogFileDlg.h : ヘッダー ファイル
//
#include "EditCtrl.h"
#include "AccelList.h"
#include <afxdtctl.h>

#include <vector>
#include <string>
#include <map>

#pragma warning( disable : 4786 )

/////////////////////////////////////////////////////////////////////////////
// CSqlLogFileDlg ダイアログ

class CSqlLogFileElement
{
public:
	CSqlLogFileElement(const TCHAR *file_name, 
		const TCHAR *user, const TCHAR *sql_time, 
		long offset, int row_cnt);

	const CString &GetUser() const { return m_user; }
	const CString &GetTime() const { return m_time; }
	const CString &GetFileName() const { return m_file_name; }

	void GetSql(CEditData *edit_data, BOOL no_delete = FALSE) const;
	void GetSql(FILE *fp, TCHAR *buf, int buf_size, CEditData *edit_data,
		BOOL no_delete = FALSE) const;
	const TCHAR *GetSummary(FILE *fp);
	const TCHAR *GetSummary() const { return m_summary; }

	void SetNo(int no) { _stprintf(m_no, _T("%d"), no); }
	const TCHAR *GetNo() const { return m_no; }

private:
	CString		m_file_name;
	CString		m_user;
	CString		m_time;
	long		m_offset;
	int			m_row_cnt;
	TCHAR		m_summary[100];
	TCHAR		m_no[10];
};

typedef std::vector<CSqlLogFileElement>			SqlLogFileElementContainer;
typedef SqlLogFileElementContainer::iterator	SLFEIterator;

class CSqlLogFileDlg : public CDialog
{
// コンストラクション
public:
	CSqlLogFileDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

	void Init(CFont *font, const TCHAR *sql_log_dir, 
		const CTime &date, const TCHAR *user, const TCHAR *filter);
	CString GetSelectedSQL();

// ダイアログ データ
	//{{AFX_DATA(CSqlLogFileDlg)
	enum { IDD = IDD_SQL_LOG_FILE_DLG };
	CEdit	m_edit_filter;
	CComboBox	m_combo_user;
	CButton	m_ok;
	CListCtrl	m_list;
	CString	m_filter;
	CString	m_user;
	CTime	m_date;
	//}}AFX_DATA

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CSqlLogFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CSqlLogFileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnDatetimechangeDatetimepicker1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeComboUser();
	afx_msg void OnButtonSearch();
	afx_msg void OnGetdispinfoList1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void RelayoutControl();
	int CreateEditCtrl();
	int SetLogList();

	// FIXME std::stringでいいか？
	typedef std::map<std::wstring, int> UserListContainer;
	typedef UserListContainer::iterator ul_it;

	BOOL ReadLogFile();
	BOOL ReadLogFileMain(const TCHAR* file_name, CString sql_date, TCHAR* buf);

	BOOL RefreshEditData();

	CString GetSelectedUser();

	UserListContainer				m_user_list;
	SqlLogFileElementContainer		m_sql_log_list;

	CFont		*m_font;
	CString		m_sql_log_dir;

	CEditData	m_edit_data;
	CEditCtrl	m_edit_ctrl;

	CTime		m_last_date;
	CTime		m_last_date2;

public:
	CTime m_date2;
	CDateTimeCtrl m_date_picker;
	CDateTimeCtrl m_date_picker2;
	afx_msg void OnCloseupDatetimepicker1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseupDatetimepicker2(NMHDR* pNMHDR, LRESULT* pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SQLLOGFILEDLG_H__D0D9FDFB_6D45_447B_979E_9E64A4F7E685__INCLUDED_)
