/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */
 
 // グローバル変数
#ifdef MAIN_PROG
#define	EXTERN
#else
#define EXTERN extern
#endif

#include "sqlstrtoken.h"
#include "EditCtrl.h"
#include "GridCtrl.h"
#include "sqllog.h"
#include "tabbar.h"

#include "ObjectBar.h"
#include "ObjectDetailBar.h"
#include "ShowCLobDlg.h"
#include "SearchDlgData.h"

#define PSQLEDIT	1

EXTERN HACCEL		g_bind_dlg_accel;
EXTERN CSearchDlgData	g_search_data;

EXTERN BOOL			g_login_flg;
EXTERN HPgSession	g_ss;				// oracle session handle
EXTERN TCHAR		g_msg_buf[4096];	// error message buffer

EXTERN CString		g_connect_str;
EXTERN CString		g_session_info;

EXTERN CShowCLobDlgHandler g_show_clob_dlg;

EXTERN CString		g_file_type;
EXTERN CString		g_grid_calc;

EXTERN CSqlLogArray	*g_sql_log_array;	// sql log list
EXTERN CSqlLogger	*g_sql_logger;
EXTERN int			g_max_sql_log;		// max sql log count

EXTERN CFont		g_font;				// global font
EXTERN CFont		g_dlg_bar_font;		// global font(object list)

EXTERN POINT		g_cur_pos;			// cursor position of current textctrl
EXTERN CSQLStrToken	g_sql_str_token;	// string toknizer

EXTERN CObjectBar		g_object_bar;
EXTERN CObjectDetailBar	g_object_detail_bar;
EXTERN CTabBar		g_tab_bar;

EXTERN int			g_escape_cmd;		// escapeキーに割り当てられたコマンド

enum SaveMode {
	SAVE_MODE_NONE,
	SAVE_MODE_ALL_QUERY,
	SAVE_MODE_ALL_SAVE,
	SAVE_MODE_ALL_NOSAVE,
};
EXTERN SaveMode		g_save_mode;

enum CompletionObjectType {
	COT_TABLE,
	COT_VIEW,
	COT_INDEX,
	COT_SEQUENCE,
	COT_COUNT,
};
extern TCHAR *completion_object_type_list[COT_COUNT];

enum ObjectListFilterColumn {
	OFC_FIRST_COLUMN,
	OFC_FIRST_AND_COMMENT_COLUMN,
	OFC_ALL_COLUMN
};

struct _st_option {
	struct {
		int		tabstop;				// tabstop count of textctrl
		int		row_space;
		int		char_space;
		int		top_space;
		int		left_space;
		int		line_mode;
		int		line_len;
		BOOL	show_line_feed;
		BOOL	show_tab;
		BOOL	show_row_num;
		BOOL	show_col_num;
		BOOL	show_space;
		BOOL	show_2byte_space;
		BOOL	show_row_line;
		BOOL	show_edit_row;
		BOOL	show_brackets_bold;
		COLORREF	color[EDIT_CTRL_COLOR_CNT];
		BOOL	table_name_completion;
		BOOL	column_name_completion;
		BOOL	copy_lower_name;
		BOOL	auto_indent;
		BOOL	tab_as_space;
		BOOL	drag_drop_edit;
		BOOL	row_copy_at_no_sel;
		BOOL	ime_caret_color;
	} text_editor;

	BOOL completion_object_type[COT_COUNT];

	struct {
		BOOL	show_space;
		BOOL	show_2byte_space;
		BOOL	show_line_feed;
		BOOL	show_tab;
		BOOL	col_header_dbl_clk_paste;
		COLORREF	color[GRID_CTRL_COLOR_CNT];
		int		cell_padding_top{ 1 };
		int		cell_padding_bottom{ 1 };
		int		cell_padding_left{ 2 };
		int		cell_padding_right{ 2 };
	} grid;

	struct {
		BOOL	put_column_name;
	} download;

	struct {
		BOOL	ignore_sql_error;
		BOOL	refresh_table_list_after_ddl;
		BOOL	adjust_col_width_after_select;
		BOOL	receive_async_notify;
		BOOL	auto_commit;
		BOOL	sql_run_selected_area;
	} sql;

	struct {
		BOOL	select_sql_use_semicolon;
		BOOL	add_alias_name;
		CString	alias_name;
	} make_sql;

	struct {
		BOOL	use_keyword_window;
		BOOL	enable_code_assist;
		int		sort_column_key;
	} code_assist;

	struct {
		BOOL	comma_position_is_before_cname;
		BOOL	word_set_right;
		BOOL	put_semicolon;
		UINT	next_line_pos;
	} sql_lint;

	struct _st_option_font {
		CString		face_name;
		int			point;
		int			weight;
		BOOL		italic;
	};
	struct _st_option_font font;
	struct _st_option_font dlg_bar_font;

	int			max_msg_row;
	BOOL		save_modified;
	CString		initial_dir;
	CString		sql_lib_dir;

	BOOL		search_loop_msg;
	int			max_mru;
	BOOL		tab_title_is_path_name;
	BOOL		tab_close_at_mclick;
	BOOL		tab_create_at_dblclick;
	BOOL		tab_drag_move;
	BOOL		close_btn_on_tab;
	BOOL		show_tab_tooltip;

	BOOL		sql_logging;
	CString		sql_log_dir;

	// object list
	CString		init_object_list_type;
	int			object_list_filter_column{ 0 };

	int			init_table_list_user;

	CString		sql_stmt_str;

	GRID_CALC_TYPE	grid_calc_type;
};
EXTERN _st_option	g_option;


// View ID
#define EDIT_VIEW				101
#define MSG_VIEW				102
#define GRID_VIEW				103
#define SWITCH_VIEW				199

// View Messages
#define UPD_FONT				1001
#define UPD_REDRAW				1002
#define UPD_INVALIDATE			1003
#define UPD_SWITCH				1004
#define UPD_REFRESH_OBJECT_INFO	1005
#define UPD_CLEAR_SELECTED		1006
#define UPD_CALC_GRID_SELECTED_DATA	1008
#define UPD_ACTIVE_DOC			1011
#define UPD_MDI_ACTIVATE		1022

#define UPD_PASTE_SQL			1102
#define UPD_PASTE_SQL2			1103
#define UPD_PASTE_OBJECT_NAME	1110

#define UPD_SWITCH_VIEW			1200
#define UPD_SET_ERROR_INFO		1201
#define UPD_ACTIVE_VIEW			1202

#define UPD_PUT_RESULT			1301
#define UPD_SET_RESULT_CARET	1302
#define UPD_CLEAR_RESULT		1303
#define UPD_PUT_SEPALATOR		1304
#define UPD_PUT_STATUS_BAR_MSG	1305

#define UPD_ADJUST_COL_WIDTH	1400
#define UPD_EQUAL_COL_WIDTH		1401
#define UPD_GRID_DATASET		1402
#define UPD_GRID_SQL			1403
#define UPD_SET_GRID_MSG		1408
#define UPD_GRID_SWAP_ROW_COL	1410
#define UPD_GRID_CTRL			1412
#define UPD_GRID_FILTER_DATA_ON		1413
#define UPD_GRID_FILTER_DATA_OFF	1414

#define UPD_CHANGE_EDITOR_OPTION	1500
#define UPD_CHANGE_LINE_MODE		1501

#define UPD_SET_HEADER_STYLE		2000
#define UPD_DELETE_PANE				2001


//
#define CODE_ASSIST_SORT_COLUMN_NAME		0
#define CODE_ASSIST_SORT_COLUMN_ID			1

