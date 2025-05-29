/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _SQL_PARSE_H_INCLUDE
#define _SQL_PARSE_H_INCLUDE

#include "EditData.h"

BOOL is_sql_end(CEditData *edit_data, int row, int *semicolon_col = NULL, BOOL in_quote = FALSE);

#endif _SQL_PARSE_H_INCLUDE
