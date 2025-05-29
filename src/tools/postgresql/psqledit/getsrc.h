/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */


#ifndef __GET_SRC_H_INCLUDED__
#define __GET_SRC_H_INCLUDED__

#include "editdata.h"

int create_object_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name, const TCHAR *schema,
	CEditData *edit_data, TCHAR *msg_buf, BOOL recalc_disp_size = TRUE);

// getsrc2.cpp
int dumpTableSchema(HPgSession ss, const TCHAR *oid, CEditData *edit_data, TCHAR *msg_buf);
int dumpIndexSchema(HPgSession ss, const TCHAR* table_oid, const TCHAR* index_oid,
	CEditData* edit_data, TCHAR* msg_buf);

#endif  // __GET_SRC_H_INCLUDED__