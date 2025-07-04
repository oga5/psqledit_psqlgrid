/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(_SQL_STR_TOKEN_INCLUDED_)
#define _SQL_STR_TOKEN_INCLUDED_

#include "strtoken.h"
#include "pglib.h"

class CSQLStrToken : public CStrToken
{
private:
	CRITICAL_SECTION m_critical_section;
	volatile bool m_is_init_keyword;
	struct _key_word_st	*m_completion_words;
	int m_completion_word_cnt;

	HPgDataset m_dset_table;
	HPgDataset m_dset_column;

	const TCHAR *getObjectName(const TCHAR *p, TCHAR *name, int name_buf_size);

	void initCharTable();
	void freeCompletionKeyword();
	int addDatasetToKeywordList(HPgDataset dataset, const TCHAR *owner, TCHAR *msg_buf,
		int column_idx, int org_column_idx, int type_idx);

	BOOL CheckSQL(const TCHAR *sql, const TCHAR *keyword);

public:
	BOOL isCommand(const TCHAR *sql);
	BOOL isExecuteCommand(const TCHAR *sql);
	BOOL isSelectSQL(const TCHAR *sql);
	BOOL isExplainSQL(const TCHAR *sql);
	BOOL isPLSQL(const TCHAR *sql, TCHAR *object_type, TCHAR *object_name);
	CSQLStrToken();
	virtual ~CSQLStrToken();

	virtual const TCHAR *skipCommentAndSpace(const TCHAR *p);

	int initCompletionWord(HPgSession ss, 
		const TCHAR *owner, TCHAR *msg_buf, BOOL b_object_name, BOOL b_column_name,
		CString object_type_list = "");

	void FreeDataset();
};

#endif _SQL_STR_TOKEN_INCLUDED_
