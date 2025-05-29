/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // psqleditDoc.h : CPsqleditDoc クラスの宣言およびインターフェイスの定義をします。
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSQLEDITDOC_H__E320F3EE_EE38_4F8E_B5AA_1722D9C94831__INCLUDED_)
#define AFX_PSQLEDITDOC_H__E320F3EE_EE38_4F8E_B5AA_1722D9C94831__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EditData.h"
#include "PgGridData.h"
#include "LogEditData.h"
#include "pglib.h"
#include "UnicodeArchive.h"
#include "griddatafilter.h"

typedef enum {
	SQL_NORMAL,
	SQL_SELECT,
	SQL_EXPLAIN,
	SQL_COMMAND,
	SQL_FILE_RUN,
	SQL_DDL,
} SQL_KIND;

#include "shortcutsql.h"

class CPsqleditDoc : public CDocument
{
public:
	static BOOL LoadShortCutSqlList();
private:
	static CShortCutSqlList m_short_cut_sql_list;

protected: // シリアライズ機能のみから作成します。
	CPsqleditDoc();
	DECLARE_DYNCREATE(CPsqleditDoc)

// アトリビュート
public:

// オペレーション
public:

//オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CPsqleditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void SetTitle(LPCTSTR lpszTitle);
	virtual void OnCloseDocument();
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CPsqleditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CPsqleditDoc)
	afx_msg void OnSqlRun();
	afx_msg void OnSwitchView();
	afx_msg void OnClearResult();
	afx_msg void OnSqlLog();
	afx_msg void OnFileSaveGridAs();
	afx_msg void OnSqlRunSelected();
	afx_msg void OnUpdateSqlRunSelected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSqlRun(CCmdUI* pCmdUI);
	afx_msg void OnExplainRun();
	afx_msg void OnUpdateExplainRun(CCmdUI* pCmdUI);
	afx_msg void OnDownload();
	afx_msg void OnMakeInsertSql();
	afx_msg void OnMakeSelectSql();
	afx_msg void OnMakeUpdateSql();
	afx_msg void OnMakeInsertSqlAll();
	afx_msg void OnMakeSelectSqlAll();
	afx_msg void OnPsqlgrid();
	afx_msg void OnUpdatePsqlgrid(CCmdUI* pCmdUI);
	afx_msg void OnPsqlgridObjectbar();
	afx_msg void OnGridSwapRowCol();
	afx_msg void OnUpdateGridSwapRowCol(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateMakeSql(CCmdUI* pCmdUI);
	afx_msg void OnShortCutSql(UINT nID);
	DECLARE_MESSAGE_MAP()

private:
	CLogEditData	m_msg_data;
	CEditData	m_sql_data;
	HPgDataset	m_dataset;
	TCHAR			*m_sql;
	int				m_sql_buf_size;
	SQL_KIND		m_sql_kind;
	CGridData_SwapRowCol	m_grid_data_swap_row_col;
	CPgGridData		m_grid_data;

	CMapStringToString	m_bind_data;
	CMapStringToString	m_bind_data_tmp;
	int			m_bind_param_editor_grid_cell_width[2];

	int			m_kanji_code;
	int			m_line_type;

	BOOL		m_no_sql_run;

	BOOL		m_grid_swap_row_col_mode;
	BOOL		m_grid_swap_row_col_disp_flg;

	int			m_split_editor_prev_active_row;

// オペレーション
public:
	TCHAR *GetSqlBuf() {
		if(m_sql == NULL) return _T("");
		return m_sql;
	}
	const TCHAR* GetSkipSpaceSql() { 
		return g_sql_str_token.skipCommentAndSpace(GetSqlBuf());
	}
	SQL_KIND GetSqlKind() { return m_sql_kind; }
	void SetSqlKind(SQL_KIND kind) { m_sql_kind = kind; }

	CGridData *GetGridData();
	CPgGridData *GetOrigGridData() { return &m_grid_data; }
	CGridData_Filter* GetFilterGridData() { return m_grid_data.GetFilterGridData(); }
	BOOL GetGridFilterMode() { return m_grid_data.GetGridFilterMode(); }
	void PostGridFilterOnOff();

	int SkipNullRow(int start_row = 0, CEditData *edit_data = NULL);
	int GetSQL(int start_row = 0, CEditData *edit_data = NULL);

	CLogEditData	*GetMsg() { return &m_msg_data; }
	int			GetMsgMaxRow() { return g_option.max_msg_row; }
	HPgDataset	GetDataset() { return m_dataset; }
	void SetDataset(HPgDataset dataset, HWND hWnd);

	CEditData	*GetSqlData() { return &m_sql_data; }
	int CopyClipboard(TCHAR *str);
	void SqlRun(CEditData *edit_data, int start_row = 0, int end_row = 0, CUnicodeArchive *ar = NULL);
	void SqlRun(int start_row = 0, int end_row = 0, CUnicodeArchive *ar = NULL);
	void SetNoSqlRun() { m_no_sql_run = TRUE; }

	void CheckModified();
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);

	void DispFileType();

	void OnActiveDocument();
	void CalcGridSelectedData();
	
	void HideSearchDlgs();

	int GetViewKind();

	BOOL GetGridSwapRowColMode() { return m_grid_swap_row_col_mode; }
	void ToggleGridSwapRowColMode();

	BOOL CPsqleditDoc::ParseSelectSQL(const TCHAR *sql, TCHAR *msg_buf,
		CString &schema_name, CString &table_name1, CString &table_name2,
		CString &alias_name, CString &where_clause);

	void SetSplitEditorPrevActivePane(int row) { m_split_editor_prev_active_row = row; }
	int GetSplitEditorPrevActivePane() { return m_split_editor_prev_active_row; }

	BOOL IsShowDataDialogVisible();
	void ShowGridData(POINT cell_pt);

	CMapStringToString* GetBindData() { return &m_bind_data; }
	CMapStringToString* GetBindDataTmp() { return &m_bind_data_tmp; }
	int BindParamEdit();

private:
	BOOL AllocSqlBuf(int buf_size);
	void FreeSqlBuf();
	int LoadFile(CArchive &ar, int kanji_code = UnknownKanjiCode);
	int SaveFile(CArchive &ar);
	void ReloadFile(int kanji_code);
	void DispNotConnectedMsg();
	
	void SqlRunSelected();

	BOOL CreatePsqlGridFile(TCHAR *sql, const TCHAR *file_name);
	BOOL RunPsqlGrid(TCHAR *sql);

	CView *GetFirstView();

	void ActiveDoc();

	void SetTabName(CString title);

public:
	afx_msg void OnPsqlgrselected();
	afx_msg void OnUpdatePsqlgrselected(CCmdUI *pCmdUI);
	afx_msg void OnTabName();
	afx_msg void OnUpdateTabName(CCmdUI* pCmdUI);
	afx_msg void OnGrfilterOff();
	afx_msg void OnUpdateGrfilterOff(CCmdUI* pCmdUI);
	afx_msg void OnGrfilterOn();
	afx_msg void OnUpdateGrfilterOn(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_PSQLEDITDOC_H__E320F3EE_EE38_4F8E_B5AA_1722D9C94831__INCLUDED_)
