/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _LOG_EDIT_DATA_H_INCLUDE
#define _LOG_EDIT_DATA_H_INCLUDE

#include "editdata.h"
#include "UnicodeArchive.h"

class CLogEditData : public CEditData
{
public:
	CLogEditData();
	~CLogEditData();

	virtual int paste(const TCHAR *pstr, BOOL recalc = TRUE);

	void log_start(CUnicodeArchive *ar);
	void log_end();
	BOOL is_logging() { return m_logging; }

private:
	BOOL	m_logging;
	CUnicodeArchive	*m_ar;
	TCHAR	*m_buf;
	int		m_buf_size;

	void puts_logfile(const TCHAR *pstr, CUnicodeArchive *ar);
};

#endif _LOG_EDIT_DATA_H_INCLUDE
