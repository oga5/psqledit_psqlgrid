/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __GLOBAL_H_INCLUDED__
#define __GLOBAL_H_INCLUDED__

#ifdef _MAIN
#define EXTERN
#else
#define EXTERN extern
#endif

#include "pglib.h"
#include "SqlStrToken.h"
#include "gridctrl.h"

EXTERN BOOL			g_license_ok;

EXTERN HPgSession	g_ss;				// oracle session handle
EXTERN BOOL			g_login_flg;
EXTERN TCHAR		g_msg_buf[1024];	// error message buffer

EXTERN CString		g_connect_str;
EXTERN CString		g_session_info;

EXTERN CFont		g_font;				// global font
EXTERN CSQLStrToken g_sql_str_token;	// string toknizer

EXTERN CString		g_grid_calc;

struct _st_option {
	struct _st_option_font {
		CString		face_name;
		int			point;
		int			weight;
		BOOL		italic;
	} font;

	struct _st_option_grid {
		BOOL	show_space;
		BOOL	show_2byte_space;
		BOOL	show_tab;
		BOOL	show_line_feed;
		BOOL	adjust_col_width_no_use_colname;
		BOOL	invert_select_text;
		BOOL	copy_escape_dblquote;
		BOOL	ime_caret_color;
		BOOL	end_key_like_excel;
		COLORREF	color[GRID_CTRL_COLOR_CNT];
		CString null_text;

		int		cell_padding_top{ 1 };
		int		cell_padding_bottom{ 1 };
		int		cell_padding_left{ 2 };
		int		cell_padding_right{ 2 };
	} grid;

	BOOL		put_column_name;
	BOOL		adjust_col_width;
	BOOL		edit_other_owner;

	BOOL		use_message_beep;

	BOOL		disp_all_delete_null_rows_warning;

	BOOL		use_user_updatable_columns;
	BOOL		share_connect_info;

	BOOL		data_lock;
	CString		nls_date_format;
	CString		nls_timestamp_format;
	CString		nls_timestamp_tz_format;
	CString		timezone;
	CString		initial_dir;

	GRID_CALC_TYPE	grid_calc_type;
};
EXTERN _st_option	g_option;

#define UPD_FONT		1001
#define UPD_ERROR		1002
#define UPD_FLUSH_DATA	1003
#define UPD_GRID_DATA	1004
#define UPD_GRID_OPTION	1010
#define UPD_CLEAR_DATA	1020
#define UPD_RECONNECT	1021

#define UPD_CALC_GRID_SELECTED_DATA	1008

#define UPD_GRID_SWAP_ROW_COL	1401
#define UPD_GRID_CTRL			1402
#define UPD_GRID_FILTER_DATA_ON		1403
#define UPD_GRID_FILTER_DATA_OFF	1404

#endif __GLOBAL_H_INCLUDED__
