/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(AFX_SQLEDITVIEW_H__97EA593E_62EF_11D3_8FED_444553540000__INCLUDED_)
#define AFX_SQLEDITVIEW_H__97EA593E_62EF_11D3_8FED_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SQLEditView.h : ヘッダー ファイル
//

#include "PsqleditDoc.h"
#include "PsqlEditCtrl.h"

#include "searchDlg.h"
#include "replaceDlg.h"
//#include "printDlg.h"

#define IdScrollTimer	100

#define NO_DRAG		0
#define PRE_DRAG	1
#define DO_DRAG		2

/////////////////////////////////////////////////////////////////////////////
// CSQLEditView ビュー

class CSQLEditView : public CView
{
protected:
	CSQLEditView();           // 動的生成に使用されるプロテクト コンストラクタ
	DECLARE_DYNCREATE(CSQLEditView)

// アトリビュート
public:
	CPsqleditDoc* GetDocument() { return (CPsqleditDoc*)m_pDocument; };

private:
	void PasteObjectName(TCHAR *object_name);
	CPSqlEditCtrl		m_edit_ctrl;

	void SetErrorInfo(int row_offset);
	void SetErrorPos(int row_offset);
	void ErrorHighlight(CPoint pt);

	void SetEditorOption();
	
	void ReplaceText(TCHAR *search_text, TCHAR *replace_text, BOOL b_regexp);
	void SearchMsg(int ret_v, int dir, BOOL b_looped);
	
	void SearchString(const TCHAR* search_text);

	void DoLintSql(const TCHAR *sql);

	BOOL IsPrimaryView();
	BOOL IsPrevActiveView();

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。

	//{{AFX_VIRTUAL(CSQLEditView)
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnDraw(CDC* pDC);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	virtual ~CSQLEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// 生成されたメッセージ マップ関数
protected:
	void Invalidate();
	//{{AFX_MSG(CSQLEditView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditPaste();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnSelectAll();
	afx_msg void OnSearchNext();
	afx_msg void OnSearchPrev();
	afx_msg void OnSearch();
	afx_msg void OnReplace();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnToLower();
	afx_msg void OnToUpper();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnJumpBookMark();
	afx_msg void OnSetBookMark();
	afx_msg void OnJumpBookMarkBack();
	afx_msg void OnConvertToC();
	afx_msg void OnConvertToHtml();
	afx_msg void OnConvertToJava();
	afx_msg void OnConvertToPerl();
	afx_msg void OnConvertFromC();
	afx_msg void OnConvertFromHtml();
	afx_msg void OnConvertFromJava();
	afx_msg void OnConvertFromPerl();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnConvertToVb();
	afx_msg void OnConvertFromVb();
	afx_msg void OnClearAllBookMark();
	afx_msg void OnEditReverseRows();
	afx_msg void OnEditSortRowsAsc();
	afx_msg void OnEditSortRowsDesc();
	afx_msg void OnSetSqlBookmark();
	afx_msg void OnClearSearchText();
	afx_msg void OnFilePrint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnInsertComment();
	afx_msg void OnDeleteComment();
	afx_msg void OnEditDelete();
	afx_msg void OnEditIndent();
	afx_msg void OnEditIndentRev();
	afx_msg void OnCharLeft();
	afx_msg void OnCharLeftExtend();
	afx_msg void OnCharRight();
	afx_msg void OnCharRightExtend();
	afx_msg void OnLineUp();
	afx_msg void OnLineUpExtend();
	afx_msg void OnLineDown();
	afx_msg void OnLineDownExtend();
	afx_msg void OnWordLeft();
	afx_msg void OnWordLeftExtend();
	afx_msg void OnWordRight();
	afx_msg void OnWordRightExtend();
	afx_msg void OnLineStart();
	afx_msg void OnLineStartExtend();
	afx_msg void OnLineEnd();
	afx_msg void OnLineEndExtend();
	afx_msg void OnDocumentEnd();
	afx_msg void OnDocumentEndExtend();
	afx_msg void OnDocumentStart();
	afx_msg void OnDocumentStartExtend();
	afx_msg void OnEditBackspace();
	afx_msg void OnKeywordCompletion();
	afx_msg void OnKeywordCompletionRev();
	afx_msg void OnScrollUp();
	afx_msg void OnScrollDown();
	afx_msg void OnScrollPageUp();
	afx_msg void OnScrollPageDown();
	afx_msg void OnPageUp();
	afx_msg void OnPageUpExtend();
	afx_msg void OnPageDown();
	afx_msg void OnPageDownExtend();
	afx_msg void OnSelectRow();
	afx_msg void OnBoxSelect();
	afx_msg void OnToggleOverwrite();
	afx_msg void OnTabToSpace();
	afx_msg void OnSpaceToTab();
	afx_msg void OnJumpBookMarkExtend();
	afx_msg void OnJumpBookMarkBackExtend();
	afx_msg void OnLineJump();
	afx_msg void OnEditDeleteRow();
	afx_msg void OnEditDeleteAfterCursor();
	afx_msg void OnLastEditPos();
	afx_msg void OnEditCopyRow();
	afx_msg void OnEditCutRow();
	afx_msg void OnEditPasteRow();
	afx_msg void OnEditPasteRowUp();
	afx_msg void OnSplitStart();
	afx_msg void OnSplitStartExtend();
	afx_msg void OnSplitEnd();
	afx_msg void OnSplitEndExtend();
	afx_msg void OnSelectWord();
	afx_msg void OnEditJoinRow();
	afx_msg void OnSearchSelected();
	afx_msg void OnUpdateSearchSelected(CCmdUI* pCmdUI);
	afx_msg void OnCodeAssistOn();
	afx_msg void OnSqlLint();
	afx_msg void OnUpdateSqlLint(CCmdUI* pCmdUI);
	afx_msg void OnConvertToCUnicode();
	//}}AFX_MSG
	afx_msg void OnUpdateConvertMenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLowerUpper(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMultiLineEdit(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDisableMenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEnableMenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOverwrite(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	afx_msg void OnConvertFromPython();
	afx_msg void OnConvertToPython();
	afx_msg void OnConvertFromJavascript();
	afx_msg void OnConvertToJavascript();
	afx_msg void OnToLowerWithoutCommentLiteral();
	afx_msg void OnToUpperWithoutCommentLiteral();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_SQLEDITVIEW_H__97EA593E_62EF_11D3_8FED_444553540000__INCLUDED_)
