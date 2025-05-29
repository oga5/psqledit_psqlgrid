/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

struct {
	int		category_id;
	TCHAR	*category_name;
} menu_category_list[] = {
	{0, _T("ファイル")},
	{1, _T("編集")},
	{3, _T("データ")},
	{4, _T("表示")},
	{5, _T("ツール")},
	{7, _T("ヘルプ")},
};

struct {
	int		category_id;
	int		menu_id;
} menu_list[] = {
	{0, ID_SELECT_TABLE},
	{0, ID_SAVE_CLOSE},
	{0, ID_FILE_SAVE_GRID_AS},
	{0, ID_RECONNECT},
	{0, ID_APP_EXIT},

	{1, ID_EDIT_UNDO},
	{1, ID_EDIT_REDO},
	{1, ID_EDIT_CUT},
	{1, ID_EDIT_COPY},
	{1, ID_EDIT_SELECT_ALL},

	{1, ID_GRID_COPY_TAB},
	{1, ID_GRID_COPY_TAB_CNAME},
	{1, ID_GRID_COPY_CSV},
	{1, ID_GRID_COPY_CSV_CNAME},
	{1, ID_GRID_COPY_FIX_LEN},
	{1, ID_GRID_COPY_FIX_LEN_CNAME},
	{1, ID_EDIT_PASTE},
	{1, ID_EDIT_PASTE_TO_CELL},

	{1, ID_SEARCH},
	{1, ID_SEARCH_NEXT},
	{1, ID_SEARCH_PREV},
	{1, ID_REPLACE},
	{1, ID_CLEAR_SEARCH_TEXT},

	{1, ID_ENTER_EDIT},
	{1, ID_DATA_EDIT},

	{1, ID_INPUT_ENTER},

	{1, ID_TO_LOWER},
	{1, ID_TO_UPPER},
	{1, ID_INPUT_SEQUENCE},

	{1, ID_HANKAKU_TO_ZENKAKU},
	{1, ID_HANKAKU_TO_ZENKAKU_KATA},
	{1, ID_HANKAKU_TO_ZENKAKU_ALPHA},
	{1, ID_ZENKAKU_TO_HANKAKU},
	{1, ID_ZENKAKU_TO_HANKAKU_KATA},
	{1, ID_ZENKAKU_TO_HANKAKU_ALPHA},

	{3, ID_INSERT_ROW},
	{3, ID_INSERT_ROWS},
	{3, ID_DELETE_ROW},
	{3, ID_DELETE_NULL_ROWS},
	{3, ID_DELETE_ALL_NULL_ROWS},

	{3, ID_ADJUST_ALL_COL_WIDTH_USE_COLNAME},
	{3, ID_ADJUST_ALL_COL_WIDTH_DATAONLY},
	{3, ID_EQUAL_ALL_COL_WIDTH},

	{3, ID_SEARCH_COLUMN},
	{3, ID_GRID_SORT},
	{3, ID_GRID_SWAP_ROW_COL},

	{ 3, ID_GRID_FILTER_ON },
	{ 3, ID_GRID_FILTER_OFF },

	{4, ID_VIEW_TOOLBAR},
	{4, ID_VIEW_STATUS_BAR},
	{4, ID_SET_FONT},

	{5, ID_SET_LOGIN_INFO},
	{5, ID_OPTION},
	{5, ID_ACCELERATOR},	

	{7, ID_APP_ABOUT},
};

