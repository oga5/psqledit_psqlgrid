/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>

#include "LogEditData.h"

CLogEditData::CLogEditData()
{
	m_logging = FALSE;
	m_ar = NULL;
	m_buf = NULL;
}

CLogEditData::~CLogEditData()
{
}

// 改行コードを変換しながら出力する
void CLogEditData::puts_logfile(const TCHAR *pstr, CUnicodeArchive *ar)
{
	const TCHAR *p1 = pstr;
	const TCHAR *p2;

	for(;;) {
		p2 = inline_strchr2(p1, '\r', '\n');

		if(p2 != NULL) {
			if(p2 != p1) {
				int need_size = (int)((p2 - p1) + 1) * sizeof(TCHAR);

				if(m_buf_size < need_size) {
					need_size = need_size + (1024 - need_size % 1024);
					m_buf = (TCHAR *)realloc(m_buf, need_size);
					m_buf_size = need_size;
				}

				_stprintf(m_buf, _T("%.*s"), (int)(p2 - p1), p1);
				ar->WriteString(m_buf);
			}
			ar->WriteNextLine();
			
			if(*p2 == '\r' && *(p2 + 1) == '\n') {
				p1 = p2 + 2;
			} else if(*p2 == '\n') {
				p1 = p2 + 1;
			}
		} else {
			if(*p1 != '\0') {
				ar->WriteString(p1);
			}
			break;
		}
	}
}

int CLogEditData::paste(const TCHAR *pstr, BOOL recalc)
{
	if(m_logging) {
		puts_logfile(pstr, m_ar);
	}

	return CEditData::paste(pstr, recalc);
}

void CLogEditData::log_start(CUnicodeArchive *ar)
{
	if(is_logging()) log_end();

	if(ar == NULL) {
		m_logging = FALSE;
		m_ar = NULL;
		return;
	}

	m_logging = TRUE;
	m_ar = ar;
}

void CLogEditData::log_end()
{
	m_logging = FALSE;
	m_ar = NULL;
}
