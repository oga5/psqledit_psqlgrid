/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
// psqlgridDoc.cpp : CPsqlgridDoc クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "psqlgrid.h"

#include "psqlgridDoc.h"

#include "CommonCancelDlg.h"
#include "PlacesBarFileDlg.h"
#include "MainFrm.h"

#include "TableListDlg.h"
#include "fileutil.h"

#include "griddatafilter.h"

#include <process.h>
#include <float.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridDoc

IMPLEMENT_DYNCREATE(CPsqlgridDoc, CDocument)

BEGIN_MESSAGE_MAP(CPsqlgridDoc, CDocument)
	//{{AFX_MSG_MAP(CPsqlgridDoc)
	ON_COMMAND(ID_SAVE_CLOSE, OnSaveClose)
	ON_COMMAND(ID_SELECT_TABLE, OnSelectTable)
	ON_UPDATE_COMMAND_UI(ID_SAVE_CLOSE, OnUpdateSaveClose)
	ON_COMMAND(ID_FILE_SAVE_GRID_AS, OnFileSaveGridAs)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_GRID_AS, OnUpdateFileSaveGridAs)
	ON_UPDATE_COMMAND_UI(ID_SELECT_TABLE, OnUpdateSelectTable)
	ON_COMMAND(ID_GRID_SWAP_ROW_COL, OnGridSwapRowCol)
	ON_UPDATE_COMMAND_UI(ID_GRID_SWAP_ROW_COL, OnUpdateGridSwapRowCol)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_DATA_LOCK, OnUpdateDataLock)
	ON_COMMAND(ID_GRID_FILTER_ON, &CPsqlgridDoc::OnGrfilterOn)
	ON_UPDATE_COMMAND_UI(ID_GRID_FILTER_ON, &CPsqlgridDoc::OnUpdateGrfilterOn)
	ON_COMMAND(ID_GRID_FILTER_OFF, &CPsqlgridDoc::OnGrfilterOff)
	ON_UPDATE_COMMAND_UI(ID_GRID_FILTER_OFF, &CPsqlgridDoc::OnUpdateGrfilterOff)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridDoc クラスの構築/消滅

CPsqlgridDoc::CPsqlgridDoc()
{
	m_dataset = NULL;
	m_pkey_dataset = NULL;
	m_table_name = _T("");
	m_alias_name = _T("");
	m_where_clause = _T("");

	m_grid_swap_row_col_mode = FALSE;
	m_grid_swap_row_col_disp_flg = FALSE;
	m_grid_data_swap_row_col.SetGridData(&m_grid_data);
}

CPsqlgridDoc::~CPsqlgridDoc()
{
	FreeDataset();
}

void CPsqlgridDoc::FreeDataset()
{
	m_grid_data.SetDataset(NULL, NULL, NULL, _T(""), _T(""), _T(""), FALSE);

	if(m_dataset != NULL) pg_free_dataset(m_dataset);
	if(m_pkey_dataset != NULL) pg_free_dataset(m_pkey_dataset);

	m_dataset = NULL;
	m_pkey_dataset = NULL;
}

BOOL CPsqlgridDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: この位置に再初期化処理を追加してください。
	// (SDI ドキュメントはこのドキュメントを再利用します。)
	SetOciDataset(NULL, NULL, _T(""), _T(""), _T(""), TRUE);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridDoc シリアライゼーション

void CPsqlgridDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: この位置に保存用のコードを追加してください。
	}
	else
	{
		// TODO: この位置に読み込み用のコードを追加してください。
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridDoc クラスの診断

#ifdef _DEBUG
void CPsqlgridDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPsqlgridDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPsqlgridDoc コマンド

void CPsqlgridDoc::OnSaveClose() 
{
	CWaitCursor	wait_cursor;
	POINT	err_pt;
	int		ret_v;

	UpdateAllViews(NULL, UPD_FLUSH_DATA);

	if(AfxGetMainWnd()->MessageBox(_T("編集結果を保存します。\nよろしいですか？"), 
		_T("MESSAGE"), MB_ICONQUESTION | MB_YESNO) == IDNO) return;

	CCommonCancelDlg	dlg;

	struct _thr_save_grid_data_st st;
	st.grid_data = &m_grid_data;
	st.ss = g_ss;
	st.err_pt = &err_pt;
	st.msg_buf = m_msg_buf;
	st.cancel_flg = dlg.GetCancelFlgPt();
	st.ret_v = 0;

	// SQL実行用スレッドを作成
	UINT			thrdaddr;
	uintptr_t		hthread;
	hthread = _beginthreadex(NULL, 0, save_grid_data_thr, &st, CREATE_SUSPENDED, &thrdaddr);
	if(hthread == NULL) {
		AfxGetMainWnd()->MessageBox(_T("スレッド作成エラー"), _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
		return;
	}
	dlg.InitData(CND_USE_CANCEL_BTN | CND_USE_PROGRESS_BAR,
		(HWND *)&(st.hWnd), (HANDLE)hthread);
	dlg.DoModal();

	WaitForSingleObject((void *)hthread, INFINITE);
	CloseHandle((void *)hthread);

	ret_v = st.ret_v;

	if(ret_v != 0) {
		AfxGetMainWnd()->MessageBox(m_msg_buf, _T("ERROR"), MB_ICONEXCLAMATION | MB_OK);
		UpdateAllViews(NULL, UPD_ERROR, (CObject *)&err_pt);
		return;
	}

	CString msg;
	msg.Format(_T("%s\n再検索しますか？"), m_msg_buf);
	if(AfxGetMainWnd()->MessageBox(msg, _T("MESSAGE"), MB_ICONINFORMATION | MB_YESNO) == IDYES) {
		SelectTable(TRUE);
	} else {
		SetOciDataset(NULL, NULL, _T(""), _T(""), _T(""), TRUE);
	}
}

BOOL CPsqlgridDoc::CanCloseFrame(CFrameWnd* pFrame) 
{
	if(m_table_name == _T("")) return TRUE;

	UpdateAllViews(NULL, UPD_FLUSH_DATA);
	if(m_grid_data.IsModified() == FALSE) return TRUE;

	if(AfxGetMainWnd()->MessageBox(
		_T("アプリケーションを終了します。\n")
		_T("編集中のデータは保存されません。よろしいですか？"), 
		_T("MESSAGE"), MB_ICONQUESTION | MB_YESNO) == IDNO) return FALSE;

	return TRUE;
}

BOOL CPsqlgridDoc::ClearData()
{
	if(m_table_name != _T("") && m_grid_data.IsModified()) {
		if(AfxGetMainWnd()->MessageBox(_T("編集中のデータは保存されません。よろしいですか？"), 
			_T("MESSAGE"), MB_ICONQUESTION | MB_YESNO) == IDNO) return FALSE;
	}

	SetOciDataset(NULL, NULL, _T(""), _T(""), _T(""), TRUE);
	return TRUE;
}

void CPsqlgridDoc::OnUpdateSelectTable(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_login_flg);	
}

void CPsqlgridDoc::OnSelectTable() 
{
	CWaitCursor	wait_cursor;

	UpdateAllViews(NULL, UPD_FLUSH_DATA);

	if(m_table_name != _T("") && m_grid_data.IsModified()) {
		if(AfxGetMainWnd()->MessageBox(_T("編集中のデータは保存されません。よろしいですか？"), 
			_T("MESSAGE"), MB_ICONQUESTION | MB_YESNO) == IDNO) return;
	}

	SelectTable(FALSE);
}

void CopyCStringList(CStringList *dest, CStringList *src)
{
	dest->RemoveAll();

	POSITION	pos;
	for(pos = src->GetHeadPosition(); pos != NULL; src->GetNext(pos)) {
		dest->AddTail(src->GetAt(pos));
	}
}

int CompareCStringList(CStringList *l1, CStringList *l2)
{
	if(l1->GetCount() != l2->GetCount()) return 1;

	POSITION	pos;
	for(pos = l1->GetHeadPosition(); pos != NULL; l1->GetNext(pos)) {
		if(l1->GetAt(pos) != l2->GetAt(pos)) return 1;
	}

	return 0;
}

int CPsqlgridDoc::SelectTable(BOOL auto_start, const TCHAR *file_name)
{
	CTableListDlg	dlg;

	if(m_owner.IsEmpty()) m_owner = pg_user(g_ss);

	dlg.m_ss = g_ss;
	dlg.m_auto_start = auto_start;
	dlg.m_grid_swap_row_col_mode = m_grid_swap_row_col_mode;
	dlg.m_font = &g_font;
	dlg.m_owner = m_owner;
	dlg.m_table_name = m_table_name;
	dlg.m_alias_name = m_alias_name;
	dlg.m_where_clause = m_where_clause;
	dlg.m_data_lock = g_option.data_lock;
	dlg.m_str_token = &g_sql_str_token;
	if(file_name != NULL) dlg.m_osg_file_name = file_name;
	CopyCStringList(&(dlg.m_column_name_list), &m_column_name_list);

	if(dlg.DoModal() == IDCANCEL) {
		return 1;
	}

	BOOL adjust_col_width = TRUE;
	if(m_table_name == dlg.m_table_name && 
		CompareCStringList(&m_column_name_list, &(dlg.m_column_name_list)) == 0) {
		adjust_col_width = FALSE;
	}

	m_table_name = dlg.m_table_name;
	m_alias_name = dlg.m_alias_name;
	m_where_clause = dlg.m_where_clause;
	if(dlg.m_grid_swap_row_col_mode) {
		m_grid_swap_row_col_mode = dlg.m_grid_swap_row_col_mode;
		UpdateAllViews(NULL, UPD_GRID_SWAP_ROW_COL);
	}
	g_option.data_lock = dlg.m_data_lock;
	CopyCStringList(&m_column_name_list, &(dlg.m_column_name_list));

	return SetOciDataset(dlg.m_dataset, dlg.m_pkey_dataset, 
		dlg.m_owner, dlg.m_table_name.GetBuffer(0), dlg.m_sql, adjust_col_width);
}

int CPsqlgridDoc::SetOciDataset(HPgDataset dataset, HPgDataset pkey_dataset, const TCHAR *owner, 
	const TCHAR *table_name, const TCHAR *sql, BOOL adjust_col_width)
{
	CWaitCursor	wait_cursor;

	m_owner = owner;
	m_table_name = table_name;

	SetTitle(m_table_name);

	FreeDataset();
	m_dataset = dataset;
	m_pkey_dataset = pkey_dataset;

	if(m_table_name == _T("")) {
		UpdateAllViews(NULL, UPD_GRID_DATA);
		return 0;
	}

	m_grid_data.SetDataset(g_ss, m_dataset, m_pkey_dataset, m_owner, m_table_name, 
		sql, g_option.data_lock);

	// FilterはOFFにする
	if(m_grid_data.GetGridFilterMode()) {
		UpdateAllViews(NULL, UPD_GRID_FILTER_DATA_OFF);
	}

	m_grid_swap_row_col_disp_flg = FALSE;
	if(m_grid_swap_row_col_mode) {
		m_grid_data_swap_row_col.InitDispInfo();
		m_grid_swap_row_col_disp_flg = TRUE;
	}
	
	UINT_PTR param = adjust_col_width;
	UpdateAllViews(NULL, UPD_GRID_DATA, (CObject *)param);

	return 0;
}

CGridData *CPsqlgridDoc::GetGridData()
{
	if(m_grid_swap_row_col_mode) {
		return &m_grid_data_swap_row_col;
	}

	if(m_grid_data.GetGridFilterMode()) {
		CGridData* g = m_grid_data.GetFilterGridData();
		return g;
	}

	return &m_grid_data;
}

void CPsqlgridDoc::OnUpdateDataLock(CCmdUI *pCmdUI)
{
	if(m_table_name == _T("") || g_option.data_lock == FALSE) {
		pCmdUI->SetText(_T(""));
	} else {
		pCmdUI->SetText(_T("Data Lock"));
	}
}

void CPsqlgridDoc::OnUpdateSaveClose(CCmdUI* pCmdUI) 
{
	if(m_table_name == _T("")) {
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
	}
}

void CPsqlgridDoc::OnFileSaveGridAs() 
{
	TCHAR file_name[_MAX_PATH];
	TCHAR msg_buf[1024];

	if(DoFileDlg(_T("検索結果表を保存"), FALSE, _T("csv"), NULL,
		OFN_NOREADONLYRETURN | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("CSV Files (*.csv)|*.csv|Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"),
		AfxGetMainWnd(), file_name) == FALSE) return;

	int ret_v;

	// ROWIDなど，ツールで使っているフィールドは出力しないようにする
	TCHAR sepa = ',';
	if(check_ext(file_name, _T(".txt")) == 1) {
		sepa = '\t';
	}

	ret_v = pg_save_dataset_ex(file_name, m_dataset, 
		g_option.put_column_name, msg_buf, m_grid_data.Get_ColCnt(), sepa);

	if(ret_v != 0) {
		AfxGetMainWnd()->MessageBox(msg_buf, _T("Error"), MB_ICONEXCLAMATION | MB_OK);
	}
}

void CPsqlgridDoc::OnUpdateFileSaveGridAs(CCmdUI* pCmdUI) 
{
	if(m_table_name == _T("")) {
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
	}
}

BOOL CPsqlgridDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
//	if (!CDocument::OnOpenDocument(lpszPathName))
//		return FALSE;
	CDocument::OnNewDocument();

	// TODO: この位置に固有の作成用コードを追加してください
	SetOciDataset(NULL, NULL, _T(""), _T(""), _T(""), TRUE);
	if(SelectTable(TRUE, lpszPathName) != 0) return FALSE;

	if(::IsWindow(AfxGetMainWnd()->GetSafeHwnd())) {
		AfxGetMainWnd()->SetForegroundWindow();
	}

	return TRUE;
}

void CPsqlgridDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU) 
{
//	CDocument::SetPathName(lpszPathName, bAddToMRU);
}

void CPsqlgridDoc::CalcGridSelectedData()
{
	g_grid_calc = "";

	CGridData *grid_data = GetGridData();

	if(m_dataset == NULL || pg_dataset_row_cnt(m_dataset) == 0 || 
		!grid_data->HaveSelected() || g_option.grid_calc_type == GRID_CALC_TYPE_NONE) return;

	CMainFrame	*pMainFrame = (CMainFrame *)AfxGetMainWnd();
	if(!pMainFrame->GetStatusBar()->IsWindowVisible()) return;

	CPoint pt1 = grid_data->GetSelectArea()->pos1;
	CPoint pt2 = grid_data->GetSelectArea()->pos2;

//	if(m_grid_swap_row_col_mode) {
//		pt1.x = grid_data->GetSelectArea()->pos1.y;
//		pt1.y = grid_data->GetSelectArea()->pos1.x;
//		pt2.x = grid_data->GetSelectArea()->pos2.y;
//		pt2.y = grid_data->GetSelectArea()->pos2.x;
//	}

	g_grid_calc = grid_data->CalcSelectedData(g_option.grid_calc_type, pt1, pt2);
}

void CPsqlgridDoc::OnGridSwapRowCol() 
{
	if(m_grid_swap_row_col_mode) {
		m_grid_swap_row_col_mode = FALSE;
	} else {
		m_grid_swap_row_col_mode = TRUE;

		if(!m_grid_swap_row_col_disp_flg) {
			m_grid_data_swap_row_col.InitDispInfo();
			m_grid_swap_row_col_disp_flg = TRUE;
		}
	}
	UpdateAllViews(NULL, UPD_GRID_SWAP_ROW_COL);
}

void CPsqlgridDoc::OnUpdateGridSwapRowCol(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_grid_swap_row_col_mode);
	pCmdUI->Enable(!m_table_name.IsEmpty());
}

void CPsqlgridDoc::PostGridFilterOnOff()
{
	if(m_grid_data.GetGridFilterMode()) {
		m_grid_data_swap_row_col.SetGridData(m_grid_data.GetFilterGridData());
	} else {
		m_grid_data_swap_row_col.SetGridData(&m_grid_data);
	}
}

void CPsqlgridDoc::OnGrfilterOn()
{
	UpdateAllViews(NULL, UPD_GRID_FILTER_DATA_ON);
}

void CPsqlgridDoc::OnUpdateGrfilterOn(CCmdUI* pCmdUI)
{
	if(m_grid_data.Get_RowCnt() == 0) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);
}

void CPsqlgridDoc::OnGrfilterOff()
{
	if(!m_grid_data.GetGridFilterMode()) return;

	UpdateAllViews(NULL, UPD_GRID_FILTER_DATA_OFF);
}

void CPsqlgridDoc::OnUpdateGrfilterOff(CCmdUI* pCmdUI)
{
	if(!m_grid_data.GetGridFilterMode()) {
		pCmdUI->Enable(FALSE);
		return;
	}
	pCmdUI->Enable(TRUE);
}
