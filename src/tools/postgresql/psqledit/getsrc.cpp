/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include <stdafx.h>
#include "pglib.h"
#include "pgmsg.h"

#include "localsql.h"
#include "editdata.h"
#include "mbutil.h"
#include "getsrc.h"

// for trigger
#define TRIGGER_TYPE_ROW                (1 << 0)
#define TRIGGER_TYPE_BEFORE             (1 << 1)
#define TRIGGER_TYPE_INSERT             (1 << 2)
#define TRIGGER_TYPE_DELETE             (1 << 3)
#define TRIGGER_TYPE_UPDATE             (1 << 4)
#define TRIGGER_TYPE_TRUNCATE           (1 << 5)
#define TRIGGER_TYPE_INSTEAD            (1 << 6)

#define TRIGGER_TYPE_TIMING_MASK (TRIGGER_TYPE_BEFORE | TRIGGER_TYPE_INSTEAD)
#define TRIGGER_TYPE_AFTER              0

#define TRIGGER_FOR_ROW(type)           (type & TRIGGER_TYPE_ROW)

#define TRIGGER_FOR_BEFORE(type)        (((type) & TRIGGER_TYPE_TIMING_MASK) == TRIGGER_TYPE_BEFORE)
#define TRIGGER_FOR_AFTER(type)         (((type) & TRIGGER_TYPE_TIMING_MASK) == TRIGGER_TYPE_AFTER)
#define TRIGGER_FOR_INSTEAD(type)       (((type) & TRIGGER_TYPE_TIMING_MASK) == TRIGGER_TYPE_INSTEAD)

#define TRIGGER_FOR_INSERT(type)        (type & TRIGGER_TYPE_INSERT)
#define TRIGGER_FOR_DELETE(type)        (type & TRIGGER_TYPE_DELETE)
#define TRIGGER_FOR_UPDATE(type)        (type & TRIGGER_TYPE_UPDATE)
//

typedef enum _OidOptions
{
	zeroAsOpaque = 1,
	zeroAsAny = 2,
	zeroAsStar = 4,
	zeroAsNone = 8,
	useBaseTypeName = 1024
} OidOptions;

typedef enum _formatLiteralOptions
{
	CONV_ALL = 0,
	PASS_LFTAB = 3
	/* NOTE: 1 and 2 are reserved in case we * want to make a mask. */
	/* We could make this a bit mask for control chars, but I don't */
	/* see any value in making it more complex...the current code */
	/* only checks for 'opts == CONV_ALL' anyway. */
} formatLiteralOptions;

static int indent(CEditData *edit_data)
{
	if(edit_data->get_cur_row() == 0) return 0;

	TCHAR	*p;
	p = edit_data->get_row_buf(edit_data->get_cur_row() - 1);

	for(; *p == '\t'; p++) edit_data->paste(_T("\t"), FALSE);

	return 0;
}

static void add_char(TCHAR* p, TCHAR ch)
{
	int len = _tcslen(p);
	p[len] = ch;
	p[len+1] = '\0';
}

static void put_literalDQ(CEditData* edit_data, const TCHAR* p)
{
	// src/fe_utils/string_utils.cのappendStringLiteralDQを参考に作成
	static const TCHAR suffixes[] = _T("_XXXXXXX");
	int suffixes_len = _tcslen(suffixes);
	int		nextchar = 0;
	TCHAR	delim[100] = _T("");

	add_char(delim, '$');

	while(_tcsstr(p, delim) != NULL) {
		add_char(delim, suffixes[nextchar]);
		nextchar++;
		if(nextchar >= suffixes_len) nextchar = 0;

		if(_tcslen(delim) >= (sizeof(delim) / sizeof(delim[0])) - 2) break;
	}

	add_char(delim, '$');

	edit_data->paste(delim);
	edit_data->paste(p);
	edit_data->paste(delim);
}

static void put_literal(CEditData *edit_data, const TCHAR *p, formatLiteralOptions opts)
{
	edit_data->input_char('\'');

	for(; *p != '\0'; p++) {
		if(*p == '\\' || *p == '\'') {
			edit_data->input_char(*p);
			edit_data->input_char(*p);
		} else if(is_lead_byte(*p)) {
			edit_data->input_char(*p);
			p++;
			if(*p == '\0') return;
			edit_data->input_char(*p);
		} else if(*p == '\r') {
			if(*(p + 1) != '\n') {
				edit_data->input_char('\r');
			}
		} else if(*p < ' ' && (opts == CONV_ALL || (*p != '\n' && *p != '\t'))) {
            edit_data->input_char('\\');
            edit_data->input_char(((*p>> 6) & 3) + '0');
            edit_data->input_char(((*p >> 3) & 7) + '0');
            edit_data->input_char((*p & 7) + '0');
		} else if(*p == '\n') {
			edit_data->input_char('\r');
		} else {
			edit_data->input_char(*p);
		}
	}

	edit_data->input_char('\'');
}

static HPgDataset get_type_list(HPgSession ss, TCHAR *msg_buf)
{
	HPgDataset type_list = NULL;

	if(pg_get_remote_version(ss) >= 70300) {
		type_list = pg_create_dataset(ss,
			_T("SELECT pg_type.oid, typowner, typname, typlen, '0' as typprtlen, \n")
			_T("	typinput, typoutput, '' as typreceive, '' as typsend, typelem, typdelim, \n")
			_T("	typdefault, typrelid, typalign, typstorage, typbyval, typisdefined, \n")
			_T("	(select usename from pg_user where typowner = usesysid) as usename, \n")
			_T("	format_type(pg_type.oid, NULL) as typedefn \n")
			_T("from pg_type \n"),
			msg_buf);
	} else {
		type_list = pg_create_dataset(ss,
			_T("SELECT pg_type.oid, typowner, typname, typlen, typprtlen, ")
			_T("typinput, typoutput, typreceive, typsend, typelem, typdelim, ")
			_T("typdefault, typrelid, typalign, typstorage, typbyval, typisdefined, ")
			_T("(select usename from pg_user where typowner = usesysid) as usename, ")
			_T("format_type(pg_type.oid, NULL) as typedefn ")
			_T("from pg_type"),
			msg_buf);
	}

	return type_list;
}

static HPgDataset get_type_list_by_oid(HPgSession ss, const TCHAR* oid, TCHAR* msg_buf)
{
	HPgDataset type_list = NULL;
	TCHAR		sql_buf[4096];

	if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("SELECT pg_type.oid, typowner, typname, typlen, '0' as typprtlen, \n")
			_T("	typinput, typoutput, '' as typreceive, '' as typsend, typelem, typdelim, \n")
			_T("	typdefault, typrelid, typalign, typstorage, typbyval, typisdefined, \n")
			_T("	(select usename from pg_user where typowner = usesysid) as usename, \n")
			_T("	format_type(pg_type.oid, NULL) as typedefn \n")
			_T("from pg_type \n")
			_T("where pg_type.oid = '%s'::oid \n"),
			oid);
	} else {
		_stprintf(sql_buf,
			_T("SELECT pg_type.oid, typowner, typname, typlen, typprtlen, ")
			_T("typinput, typoutput, typreceive, typsend, typelem, typdelim, ")
			_T("typdefault, typrelid, typalign, typstorage, typbyval, typisdefined, ")
			_T("(select usename from pg_user where typowner = usesysid) as usename, ")
			_T("format_type(pg_type.oid, NULL) as typedefn ")
			_T("from pg_type \n")
			_T("where pg_type.oid = '%s'::oid \n"),
			oid);
	}
	type_list = pg_create_dataset(ss, sql_buf, msg_buf);
	if(type_list == NULL || pg_dataset_row_cnt(type_list) == 0) goto ERR1;

	return type_list;

ERR1:
	pg_free_dataset(type_list);
	return NULL;
}

static int dump_comment(HPgSession ss, CEditData *edit_data, const TCHAR *object_type,
	const TCHAR *object_name, const TCHAR *oid, const TCHAR *class_name, int subid, TCHAR *msg_buf,
	BOOL use_lf = TRUE)
{
	TCHAR		sql_buf[4096];
	int			ret_v = 0;
	HPgDataset	comment_info = NULL;

	if(pg_has_objsubid(ss)) {
		_stprintf(sql_buf,
			_T("SELECT description FROM pg_description ")
			_T("WHERE objoid = '%s'::oid and classoid = ")
			_T("(SELECT oid FROM pg_class where relname = '%s') ")
			_T("and objsubid = %d"),
			oid, class_name, subid);
	} else {
		_stprintf(sql_buf,
			_T("SELECT description FROM pg_description ")
			_T("WHERE objoid = '%s'::oid and classoid = ")
			_T("(SELECT oid FROM pg_class where relname = '%s') "),
			oid, class_name);
	}

	comment_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(comment_info == NULL) goto ERR1;
	if(pg_dataset_row_cnt(comment_info) == 0) {
		pg_free_dataset(comment_info);
		return 0;
	}

	if(use_lf) edit_data->paste(_T("\n"), FALSE);

	edit_data->paste(_T("comment on "), FALSE);
	edit_data->paste(object_type, FALSE);
	edit_data->paste(_T(" "), FALSE);
	edit_data->paste(object_name, FALSE);
	edit_data->paste(_T(" is "), FALSE);

	put_literal(edit_data, 
		pg_dataset_data2(comment_info, 0, _T("description")),
		PASS_LFTAB);

	edit_data->paste(_T(";\n"), FALSE);

	pg_free_dataset(comment_info);
	return 0;

ERR1:
	pg_free_dataset(comment_info);
	return 1;
}

static const TCHAR *find_type_by_oid(HPgDataset type_list, const TCHAR *oid, OidOptions opts)
{
	int		i;
	int		oid_col_no;

	if (_tcscmp(oid, _T("0")) == 0) {
		if ((opts & zeroAsOpaque) != 0) return _T("opaque");
		else if ((opts & zeroAsAny) != 0) return _T("'any'");
		else if ((opts & zeroAsStar) != 0) return _T("*");
		else if ((opts & zeroAsNone) != 0) return _T("NONE");
		
		return NULL;
	}

	oid_col_no = pg_dataset_get_col_no(type_list, _T("oid"));
	for(i = 0; i < pg_dataset_row_cnt(type_list); i++) {
		if(_tcscmp(pg_dataset_data(type_list, i, oid_col_no), oid) == 0) {
			return pg_dataset_data2(type_list, i, _T("typedefn"));
		}
	}

	return NULL;
}

static BOOL find_type_by_oid2(HPgSession ss, const TCHAR* oid, OidOptions opts,
	CString *typedefn, CMapStringToString *type_map, TCHAR *msg_buf)
{
	HPgDataset type_list = NULL;

	*typedefn = _T("");

	if(_tcscmp(oid, _T("0")) == 0) {
		if((opts & zeroAsOpaque) != 0) {
			*typedefn = _T("opaque");
			return TRUE;
		}
		else if((opts & zeroAsAny) != 0) {
			*typedefn = _T("'any'");
			return TRUE;
		}
		else if((opts & zeroAsStar) != 0) {
			*typedefn = _T("*");
			return TRUE;
		}
		else if((opts & zeroAsNone) != 0) {
			*typedefn = _T("NONE");
			return TRUE;
		}

		return FALSE;
	}

	if(type_map != NULL && type_map->Lookup(oid, *typedefn)) {
		return TRUE;
	}

	type_list = get_type_list_by_oid(ss, oid, msg_buf);
	if(type_list == NULL || pg_dataset_row_cnt(type_list) == 0) goto ERR1;

	*typedefn = pg_dataset_data2(type_list, 0, _T("typedefn"));
	pg_free_dataset(type_list);

	if(type_map != NULL) {
		type_map->SetAt(oid, *typedefn);
	}

	return TRUE;

ERR1:
	pg_free_dataset(type_list);
	return FALSE;
}

static CString make_object_name(const TCHAR *object_name, BOOL force_quotes = FALSE)
{
	const TCHAR	*p;

	if(!force_quotes) {
		for(p = object_name; *p != '\0'; p++) {
			if(!(inline_islower(*p) || inline_isdigit(*p) || (*p == '_'))) {
				force_quotes = TRUE;
				break;
			}
		}
	}

	if(!force_quotes) return object_name;

	CString buf;
	int obj_len = (int)_tcslen(object_name);
	TCHAR	*bp = buf.GetBuffer(obj_len * 2 + 10);

	*bp = '"';
	bp++;
	for(p = object_name; *p != '\0'; p++) {
		if(*p == '"') {
			*bp = '"';
			bp++;
			*bp = '"';
			bp++;
			continue;
		}
		*bp = *p;
		bp++;
	}
	*bp = '"';
	bp++;
	*bp = '\0';

	buf.ReleaseBuffer();

	return buf;
}

static const TCHAR *parse_number_array(const TCHAR *array, CString &data)
{
	const TCHAR *p;

	p = _tcschr(array, ' ');
	if(p == NULL) {
		data = array;
		return NULL;
	}

	data.Format(_T("%.*s"), p - array, array);

	return p + 1;
}

static const TCHAR *parse_argnames_array(const TCHAR *array, CString &data)
{
	if(array == NULL || *array == '\0') {
		data = _T("");
		return NULL;
	}

	if(*array == '{') {
		array++;
	}

	const TCHAR *p;
	p = _tcschr(array, ',');
	if(p == NULL) {
		p = _tcschr(array, '}');
		if(p == NULL) {
			data = array;
		} else {
			data.Format(_T("%.*s"), p - array, array);
		}
		return NULL;
	}

	data.Format(_T("%.*s"), p - array, array);
	return p + 1;
}

static int make_arg_list(HPgSession ss, HPgDataset proc_info, HPgDataset type_list, CString &arg_list, TCHAR *msg_buf)
{
	arg_list = _T("");

	int		arg_cnt = _ttoi(pg_dataset_data2(proc_info, 0, _T("pronargs")));
	if(arg_cnt == 0) {
		return 0;
	}

	int		i;
	CString arg_oid = _T("");
	CString arg_name = _T("");
	const TCHAR	*arg_types = pg_dataset_data2(proc_info, 0, _T("proargtypes"));
	const TCHAR	*arg_names = pg_dataset_data2(proc_info, 0, _T("proargnames"));

	for(i = 0; i < arg_cnt; i++) {
		const TCHAR *type_name;

		if(arg_types == NULL) {
			_stprintf(msg_buf, _T("error proargtypes"));
			return 1;
		}
		arg_types = parse_number_array(arg_types, arg_oid);
		arg_names = parse_argnames_array(arg_names, arg_name);

		type_name = find_type_by_oid(type_list, arg_oid.GetBuffer(0), zeroAsOpaque);
		if(type_name == NULL) {
			_stprintf(msg_buf, _T("error typename"));
			return 1;
		}

		if(i != 0) arg_list += _T(", ");
		arg_list += arg_name;
		arg_list += _T(" ");
		arg_list += type_name;
	}

	return 0;
}

static CString make_function_name(HPgSession ss, HPgDataset proc_info, HPgDataset type_list, TCHAR *msg_buf)
{
	CString	object_name;
	CString arg_list;

	if(make_arg_list(ss, proc_info, type_list, arg_list, msg_buf) != 0) return _T("");
	object_name.Format(_T("%s(%s)"),
		make_object_name(pg_dataset_data2(proc_info, 0, _T("proname"))).GetString(),
		arg_list.GetString());

	return object_name;
}

static int create_function_source_main(HPgSession ss,
	const TCHAR *func_name, const TCHAR *object_type,
	HPgDataset proc_info, HPgDataset lang_info,
	const TCHAR *schema, CEditData *edit_data, TCHAR *msg_buf)
{
	CString ret_type;

	if(_tcscmp(object_type, _T("FUNCTION")) == 0) {
		if(find_type_by_oid2(ss, pg_dataset_data2(proc_info, 0, _T("prorettype")),
			zeroAsOpaque, &ret_type, NULL, msg_buf) == FALSE) {
			_stprintf(msg_buf, _T("error prorettype"));
			return 1;
		}
	}

	if(_tcscmp(object_type, _T("FUNCTION")) == 0) {
		edit_data->paste(_T("create or replace function "), FALSE);
	} else if(_tcscmp(object_type, _T("PROCEDURE")) == 0) {
		edit_data->paste(_T("create or replace procedure "), FALSE);
	}
	edit_data->paste(make_object_name(schema).GetBuffer(0), FALSE);
	edit_data->paste(_T("."), FALSE);
	edit_data->paste(func_name, FALSE);
	edit_data->paste(_T("\n"), FALSE);

	if(_tcscmp(object_type, _T("FUNCTION")) == 0) {
		edit_data->paste(_T("returns "), FALSE);
		if(_tcscmp(pg_dataset_data2(proc_info, 0, _T("proretset")), _T("t")) == 0) {
			edit_data->paste(_T("setof "), FALSE);
		}
		edit_data->paste(ret_type.GetBuffer(0), FALSE);
		edit_data->paste(_T("\n"), FALSE);
	}

	if(!pg_dataset_is_null(proc_info, 0, pg_dataset_get_col_no(proc_info, _T("probin"))) &&
		_tcscmp(pg_dataset_data2(proc_info, 0, _T("probin")), _T("-")) != 0) {
		edit_data->paste(_T("AS "), FALSE);
		put_literal(edit_data, pg_dataset_data2(proc_info, 0, _T("probin")), CONV_ALL);

		if(_tcscmp(pg_dataset_data2(proc_info, 0, _T("prosrc")), _T("-")) != 0) {
			edit_data->paste(_T(", "), FALSE);
//			put_literal(edit_data, pg_dataset_data2(proc_info, 0, _T("prosrc")), PASS_LFTAB);
			put_literalDQ(edit_data, pg_dataset_data2(proc_info, 0, _T("prosrc")));
		}
	} else {
		if(_tcscmp(pg_dataset_data2(proc_info, 0, _T("prosrc")), _T("-")) != 0) {
			edit_data->paste(_T("AS "), FALSE);
//			put_literal(edit_data, pg_dataset_data2(proc_info, 0, _T("prosrc")), PASS_LFTAB);
			put_literalDQ(edit_data, pg_dataset_data2(proc_info, 0, _T("prosrc")));
		}
	}
	edit_data->paste(_T("\n"), FALSE);

	edit_data->paste(_T("language "), FALSE);
	put_literal(edit_data, pg_dataset_data2(lang_info, 0, _T("lanname")), CONV_ALL);

	if (pg_get_remote_version(ss) >= 100000) {
		if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("provolatile")), _T("i")) == 0) {
			edit_data->paste(_T("\nIMMUTABLE"), FALSE);
		}
		if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("provolatile")), _T("s")) == 0) {
			edit_data->paste(_T("\nSTABLE"), FALSE);
		}
		if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("proisstrict")), _T("t")) == 0) {
			edit_data->paste(_T("\nSTRICT"), FALSE);
		}
		if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("prosecdef")), _T("t")) == 0) {
			edit_data->paste(_T("\nSECURITY DEFINER"), FALSE);
		}
		if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("procost")), _T("100")) != 0) {
			edit_data->paste(_T("\nCOST "), FALSE);
			edit_data->paste(pg_dataset_data2(proc_info, 0, _T("procost")), FALSE);
		}
	}
	else {
		if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("proiscachable")), _T("t")) == 0 ||
			_tcscmp(pg_dataset_data2(proc_info, 0, _T("proisstrict")), _T("t")) == 0) {
			CString sep = _T("");
			edit_data->paste(_T("\n"), FALSE);
			edit_data->paste(_T("with("), FALSE);

			if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("proiscachable")), _T("t")) == 0) {
				edit_data->paste(sep.GetBuffer(0), FALSE);
				edit_data->paste(_T("iscachable"), FALSE);
				sep = _T(", ");
			}
			if (_tcscmp(pg_dataset_data2(proc_info, 0, _T("proisstrict")), _T("t")) == 0) {
				edit_data->paste(sep.GetBuffer(0), FALSE);
				edit_data->paste(_T("isstrict"), FALSE);
				sep = ", ";
			}
			edit_data->paste(_T(")"), FALSE);
		}
	}
	edit_data->paste(_T(";\n"), FALSE);

	return 0;
}

static int create_function_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name,
	const TCHAR *schema, CEditData *edit_data, TCHAR *msg_buf)
{
	TCHAR		sql_buf[4096];
	int			ret_v = 0;
	HPgDataset	proc_info = NULL;
	HPgDataset	lang_info = NULL;
	HPgDataset	type_list = NULL;
	CString		func_name = "";

	if (pg_get_remote_version(ss) >= 100000) {
		_stprintf(sql_buf,
			_T("SELECT pg_proc.oid, proname, prolang, pronargs, prorettype, ")
			_T("proretset, proargtypes, prosrc, probin, proargnames, ")
			_T("procost, prosecdef, provolatile, ")
			_T("(select usename from pg_user where proowner = usesysid) as usename, ")
			_T("'f' as proiscachable, proisstrict, ")
			_T("pg_catalog.pg_get_function_arguments(oid) AS funcargs,\n")
			_T("pg_catalog.pg_get_function_identity_arguments(oid) AS funciargs,\n")
			_T("pg_catalog.pg_get_function_result(oid) AS funcresult \n")
			_T("from pg_proc ")
			_T("where pg_proc.oid = '%s'::oid"),
			oid);
	} else if (pg_get_remote_version(ss) >= 80400) {
		_stprintf(sql_buf,
			_T("SELECT pg_proc.oid, proname, prolang, pronargs, prorettype, ")
			_T("proretset, proargtypes, prosrc, probin, proargnames, ")
			_T("(select usename from pg_user where proowner = usesysid) as usename, ")
			_T("'f' as proiscachable, proisstrict, ")
			_T("pg_catalog.pg_get_function_arguments(oid) AS funcargs,\n")
			_T("pg_catalog.pg_get_function_identity_arguments(oid) AS funciargs,\n")
			_T("pg_catalog.pg_get_function_result(oid) AS funcresult \n")
			_T("from pg_proc ")
			_T("where pg_proc.oid = '%s'::oid"),
			oid);
	} else if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("SELECT pg_proc.oid, proname, prolang, pronargs, prorettype, ")
			_T("proretset, proargtypes, prosrc, probin, proargnames, ")
			_T("(select usename from pg_user where proowner = usesysid) as usename, ")
			_T("'f' as proiscachable, proisstrict ")
			_T("from pg_proc ")
			_T("where pg_proc.oid = '%s'::oid"),
			oid);
	} else {
		_stprintf(sql_buf,
			_T("SELECT pg_proc.oid, proname, prolang, pronargs, prorettype, ")
			_T("proretset, proargtypes, prosrc, probin, proargnames, ")
			_T("(select usename from pg_user where proowner = usesysid) as usename, ")
			_T("proiscachable, proisstrict ")
			_T("from pg_proc ")
			_T("where pg_proc.oid = '%s'::oid"),
			oid);
	}

	proc_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(proc_info == NULL || pg_dataset_row_cnt(proc_info) == 0) goto ERR1;

	_stprintf(sql_buf,
		_T("SELECT lanname FROM pg_language WHERE oid = '%s'::oid"),
		pg_dataset_data2(proc_info, 0, _T("prolang")));
	
	lang_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(lang_info == NULL || pg_dataset_row_cnt(lang_info) == 0) goto ERR1;

	if(pg_get_remote_version(ss) >= 80400) {
		func_name.Format(_T("%s(%s)"),
			make_object_name(pg_dataset_data2(proc_info, 0, _T("proname"))).GetString(),
			pg_dataset_data2(proc_info, 0, _T("funcargs")));
	} else {
		type_list = get_type_list(ss, msg_buf);
		if(type_list == NULL) goto ERR1;

		func_name = make_function_name(ss, proc_info, type_list, msg_buf);
		if(func_name == _T("")) goto ERR1;
	}

	ret_v = create_function_source_main(ss, func_name.GetBuffer(0), object_type, proc_info, lang_info, 
		schema, edit_data, msg_buf);
	if(ret_v != 0) goto ERR1;

	ret_v = dump_comment(ss, edit_data, _T("function"), func_name.GetBuffer(0), oid, _T("pg_proc"), 0, NULL);
	if(ret_v != 0) goto ERR1;

	pg_free_dataset(proc_info);
	pg_free_dataset(lang_info);
	pg_free_dataset(type_list);

	return 0;

ERR1:
	pg_free_dataset(proc_info);
	pg_free_dataset(lang_info);
	pg_free_dataset(type_list);
	return 1;
}

#define INT64CONST(x)  (x##L)
#define UINT64CONST(x) (x##UL)
#define PG_INT16_MIN    (-0x7FFF-1)
#define PG_INT16_MAX    (0x7FFF)
#define PG_INT32_MIN    (-0x7FFFFFFF-1)
#define PG_INT32_MAX    (0x7FFFFFFF)
#define PG_INT64_MIN    (-INT64CONST(0x7FFFFFFFFFFFFFFF) - 1)
#define PG_INT64_MAX    INT64CONST(0x7FFFFFFFFFFFFFFF)
#define INT64_FORMAT	_T("%I64d")

static int create_sequence_source_pg10(HPgSession ss, const TCHAR* object_type, const TCHAR* oid, const TCHAR* object_name,
	const TCHAR* schema, CEditData* edit_data, TCHAR* msg_buf)
{
	TCHAR		sql_buf[4096];
	int			ret_v = 0;
	HPgDataset	seq_info = NULL;

	const TCHAR		*startv, *incby, *maxv, *minv, *cache, *seqtype;
	bool            cycled;
	bool            is_ascending;
	INT64           default_minv,default_maxv;
	TCHAR           bufm[32], bufx[32];

	_stprintf(sql_buf,
		_T("SELECT format_type(seqtypid, NULL) seqtype, \n")
		_T("seqstart, seqincrement, \n")
		_T("seqmax, seqmin, \n")
		_T("seqcache, seqcycle \n")
		_T("FROM pg_catalog.pg_sequence \n")
		_T("WHERE seqrelid = '%s'::oid \n"),
		oid);
	seq_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(seq_info == NULL || pg_dataset_row_cnt(seq_info) == 0) goto ERR1;

	seqtype = pg_dataset_data(seq_info, 0, 0);
	startv = pg_dataset_data(seq_info, 0, 1);
	incby = pg_dataset_data(seq_info, 0, 2);
	maxv = pg_dataset_data(seq_info, 0, 3);
	minv = pg_dataset_data(seq_info, 0, 4);
	cache = pg_dataset_data(seq_info, 0, 5);
	cycled = (_tcscmp(pg_dataset_data(seq_info, 0, 6), _T("t")) == 0);

	/* Calculate default limits for a sequence of this type */
	is_ascending = (incby[0] != '-');
	if(_tcscmp(seqtype, _T("smallint")) == 0)
	{
		default_minv = is_ascending ? 1 : PG_INT16_MIN;
		default_maxv = is_ascending ? PG_INT16_MAX : -1;
	} else if(_tcscmp(seqtype, _T("integer")) == 0)
	{
		default_minv = is_ascending ? 1 : PG_INT32_MIN;
		default_maxv = is_ascending ? PG_INT32_MAX : -1;
	} else if(_tcscmp(seqtype, _T("bigint")) == 0)
	{
		default_minv = is_ascending ? 1 : PG_INT64_MIN;
		default_maxv = is_ascending ? PG_INT64_MAX : -1;
	}

	/*
	 * 64-bit strtol() isn't very portable, so convert the limits to strings
	 * and compare that way.
	 */
	_stprintf(bufm, INT64_FORMAT, default_minv);
	_stprintf(bufx, INT64_FORMAT, default_maxv);

	/* Don't print minv/maxv if they match the respective default limit */
	if(_tcscmp(minv, bufm) == 0)
		minv = NULL;
	if(_tcscmp(maxv, bufx) == 0)
		maxv = NULL;

	edit_data->paste(_T("create sequence "), FALSE);
	edit_data->paste(make_object_name(schema).GetBuffer(0), FALSE);
	edit_data->paste(_T("."), FALSE);
	edit_data->paste(make_object_name(object_name).GetBuffer(0), FALSE);
	edit_data->paste(_T("\n"), FALSE);

	if(_tcscmp(seqtype, _T("bigint")) != 0) {
		edit_data->paste(_T("    AS "), FALSE);
		edit_data->paste(seqtype, FALSE);
		edit_data->paste(_T("\n"), FALSE);
	}
	edit_data->paste(_T("    START WITH "), FALSE);
	edit_data->paste(startv, FALSE);
	edit_data->paste(_T("\n"), FALSE);

	edit_data->paste(_T("    INCREMENT BY "), FALSE);
	edit_data->paste(incby, FALSE);
	edit_data->paste(_T("\n"), FALSE);

	if(minv) {
		edit_data->paste(_T("    MINVALUE "), FALSE);
		edit_data->paste(minv, FALSE);
		edit_data->paste(_T("\n"), FALSE);
	} else {
		edit_data->paste(_T("    NO MINVALUE\n"), FALSE);
	}

	if(maxv) {
		edit_data->paste(_T("    MAXVALUE "), FALSE);
		edit_data->paste(maxv, FALSE);
		edit_data->paste(_T("\n"), FALSE);
	} else {
		edit_data->paste(_T("    NO MAXVALUE\n"), FALSE);
	}

	edit_data->paste(_T("    CACHE "), FALSE);
	edit_data->paste(cache, FALSE);

	if(cycled) {
		edit_data->paste(_T("\n    CYCLE"), FALSE);
	}
	edit_data->paste(_T(";\n"), FALSE);

	pg_free_dataset(seq_info);
	return 0;

ERR1:
	pg_free_dataset(seq_info);
	return 1;
}


static int create_sequence_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name,
	const TCHAR *schema, CEditData *edit_data, TCHAR *msg_buf)
{
	TCHAR		sql_buf[4096];
	int			ret_v = 0;
	HPgDataset	seq_info = NULL;

	if(pg_get_remote_version(ss) >= 100000) {
		return create_sequence_source_pg10(ss, object_type, oid, object_name, schema, edit_data, msg_buf);
	}

	_stprintf(sql_buf,
		_T("SELECT sequence_name, last_value, increment_by, max_value, \n")
		_T("min_value, cache_value, is_cycled, is_called \n")
		_T("from %s"),
		object_name);
	seq_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(seq_info == NULL || pg_dataset_row_cnt(seq_info) == 0) goto ERR1;

	// drop sequence
//	edit_data->paste((unsigned char *)"drop sequence ", FALSE);
//	edit_data->paste((unsigned char *)make_object_name(object_name).GetBuffer(0), FALSE);
//	edit_data->paste((unsigned char *)";\n", FALSE);
//	edit_data->paste((unsigned char *)"\n", FALSE);

	// create sequence
	edit_data->paste(_T("create sequence "), FALSE);
	edit_data->paste(make_object_name(object_name).GetBuffer(0), FALSE);
	edit_data->paste(_T("\n"), FALSE);
	edit_data->paste(_T("start "), FALSE);
	if(_tcscmp(pg_dataset_data2(seq_info, 0, _T("is_called")), _T("t")) == 0) {
		edit_data->paste(pg_dataset_data2(seq_info, 0, _T("min_value")), FALSE);
	} else {
		edit_data->paste(pg_dataset_data2(seq_info, 0, _T("last_value")), FALSE);
	}
	edit_data->paste(_T(" increment "), FALSE);
	edit_data->paste(pg_dataset_data2(seq_info, 0, _T("increment_by")), FALSE);

	edit_data->paste(_T("\n"), FALSE);
	edit_data->paste(_T("maxvalue "), FALSE);
	edit_data->paste(pg_dataset_data2(seq_info, 0, _T("max_value")), FALSE);
	edit_data->paste(_T(" minvalue "), FALSE);
	edit_data->paste(pg_dataset_data2(seq_info, 0, _T("min_value")), FALSE);
	edit_data->paste(_T("\n"), FALSE);
	edit_data->paste(_T("cache "), FALSE);
	edit_data->paste(pg_dataset_data2(seq_info, 0, _T("cache_value")), FALSE);
	if(_tcscmp(pg_dataset_data2(seq_info, 0, _T("is_cycled")), _T("t")) == 0) {
		edit_data->paste(_T("cycle"), FALSE);
	}
	edit_data->paste(_T(";\n"), FALSE);

	// sequence set value
	edit_data->paste(_T("\n"), FALSE);
	edit_data->paste(_T("select setval("), FALSE);
	put_literal(edit_data, make_object_name(object_name).GetBuffer(0), CONV_ALL);
	edit_data->paste(_T(", "), FALSE);
	edit_data->paste(pg_dataset_data2(seq_info, 0, _T("last_value")), FALSE);
	edit_data->paste(_T(", "), FALSE);
	if(_tcscmp(pg_dataset_data2(seq_info, 0, _T("is_called")), _T("t")) == 0) {
		edit_data->paste(_T("true"), FALSE);
	} else {
		edit_data->paste(_T("false"), FALSE);
	}
	edit_data->paste(_T(");\n"), FALSE);

	ret_v = dump_comment(ss, edit_data, _T("sequence"), 
		make_object_name(object_name).GetBuffer(0),
		oid, _T("pg_class"), 0, NULL);
	if(ret_v != 0) goto ERR1;

	pg_free_dataset(seq_info);

	return 0;

ERR1:
	pg_free_dataset(seq_info);
	return 1;
}

static int put_trigger_func_args(CEditData *edit_data, HPgDataset tr_info, TCHAR *msg_buf)
{
	int		i;
	int		args = _ttoi(pg_dataset_data2(tr_info, 0, _T("tgnargs")));
	const TCHAR	*tgargs;
	const TCHAR	*p;
	const TCHAR	*s;

	edit_data->paste(_T("("), FALSE);

	tgargs = pg_dataset_data2(tr_info, 0, _T("tgargs"));
	for(i = 0; i < args; i++) {
		for(p = tgargs;;) {
			p = _tcschr(p, '\\');
			if(p == NULL) {
				_stprintf(msg_buf, _T("bad argument string"));
				return 1;
			}
			p++;
			if(*p == '\\') {
				p++;
				continue;
			}
			if(p[0] == '0' && p[1] == '0' && p[2] == '0') break;
		}
		p--;

		if(i != 0) edit_data->paste(_T(","), FALSE);

		edit_data->paste(_T("'"), FALSE);
		for (s = tgargs; s < p; s++) {
			if(*s == '\'') edit_data->input_char('\\');
			edit_data->input_char(*s);
		}
		edit_data->paste(_T("'"), FALSE);
		tgargs = p + 4;
	}

	edit_data->paste(_T(");\n"), FALSE);

	return 0;
}

static int create_trigger_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name,
	const TCHAR *schema, CEditData *edit_data, TCHAR *msg_buf)
{
	TCHAR		sql_buf[4096];
	int			ret_v = 0;
	HPgDataset	tr_info = NULL;
	int			tgtype = 0;
	int			findx = 0;
	CString		trigger_name = "";
	int			tgisconstraint_flg = 0;

	if(pg_get_remote_version(ss) >= 90000) {
		_stprintf(sql_buf,
			_T("select tgname, tgfoid, tgtype, tgnargs, tgargs, tgdeferrable, tgconstrrelid, tginitdeferred, tgrelid, \n")
			_T("(select relname from pg_class where oid = tgrelid) as relname, \n")
			_T("(select n.nspname from pg_class c, pg_namespace n where c.oid = tgrelid and n.oid = c.relnamespace) as relnspname, \n")
			_T("(select relname from pg_class where oid = tgconstrrelid) as tgconstrrelname, \n")
			_T("(select n.nspname from pg_proc p, pg_namespace n where p.oid = tgfoid and n.oid = p.pronamespace) as pronspname, \n")
			_T("(select proname from pg_proc where pg_proc.oid = tgfoid) as proname \n")
			_T("from pg_trigger \n")
			_T("where oid = '%s'::oid \n"),
			oid);
	} else {
		_stprintf(sql_buf,
			_T("select tgname, tgfoid, tgtype, tgnargs, tgargs, \n")
			_T("tgisconstraint, tgconstrname, tgdeferrable, \n")
			_T("tgconstrrelid, tginitdeferred, tgrelid, \n")
			_T("(select relname from pg_class where oid = tgrelid) \n")
			_T("	as relname, \n")
			_T("(select relname from pg_class where oid = tgconstrrelid) \n")
			_T("	as tgconstrrelname, \n")
			_T("(select proname from pg_proc where pg_proc.oid = tgfoid) \n")
			_T("	as proname \n")
			_T("from pg_trigger \n")
			_T("where oid = '%s'::oid \n"),
			oid);
		tgisconstraint_flg = 1;
	}
	tr_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(tr_info == NULL || pg_dataset_row_cnt(tr_info) == 0) goto ERR1;

	trigger_name.Format(_T("%s on %s"),
		make_object_name(object_name).GetString(),
		make_object_name(pg_dataset_data2(tr_info, 0, _T("relname"))).GetString());
		
	// drop trigger
//	edit_data->paste((unsigned char *)"drop trigger ", FALSE);
//	edit_data->paste((unsigned char *)trigger_name, FALSE);
//	edit_data->paste((unsigned char *)";\n", FALSE);
//	edit_data->paste((unsigned char *)"\n", FALSE);

	if(tgisconstraint_flg && _tcscmp(pg_dataset_data2(tr_info, 0, _T("tgisconstraint")), _T("t")) == 0) {
		edit_data->paste(_T("create constraint trigger "), FALSE);
		edit_data->paste(
			make_object_name(pg_dataset_data2(tr_info, 0, _T("tgconstrname"))).GetBuffer(0), FALSE);
	} else {
		edit_data->paste(_T("create trigger "), FALSE);
		edit_data->paste(make_object_name(object_name).GetBuffer(0), FALSE);
	}
	edit_data->paste(_T("\n"), FALSE);

	tgtype = _ttoi(pg_dataset_data2(tr_info, 0, _T("tgtype")));
	if(TRIGGER_FOR_BEFORE(tgtype)) {
		edit_data->paste(_T("before "), FALSE);
	} else if(TRIGGER_FOR_AFTER(tgtype)) {
		edit_data->paste(_T("after "), FALSE);
	} else if(TRIGGER_FOR_INSTEAD(tgtype)) {
		edit_data->paste(_T("instead of "), FALSE);
	}

	findx = 0;
	if(TRIGGER_FOR_INSERT(tgtype)) {
		edit_data->paste(_T("insert"), FALSE);
		findx++;
	}
	if(TRIGGER_FOR_DELETE(tgtype)) {
		if(findx > 0) edit_data->paste(_T(" or "), FALSE);
		edit_data->paste(_T("delete"), FALSE);
		findx++;
	}
	if(TRIGGER_FOR_UPDATE(tgtype)) {
		if(findx > 0) edit_data->paste(_T(" or "), FALSE);
		edit_data->paste(_T("update"), FALSE);
		findx++;
	}
	edit_data->paste(_T(" on "), FALSE);
	edit_data->paste(make_object_name(pg_dataset_data2(tr_info, 0, _T("relnspname"))).GetBuffer(0), FALSE);
	edit_data->paste(_T("."), FALSE);
	edit_data->paste(make_object_name(pg_dataset_data2(tr_info, 0, _T("relname"))).GetBuffer(0), FALSE);

	if(tgisconstraint_flg && _tcscmp(pg_dataset_data2(tr_info, 0, _T("tgisconstraint")), _T("t")) == 0) {
		if(_tcscmp(pg_dataset_data2(tr_info, 0, _T("tgconstrrelid")), _T("0")) != 0) {
			if(pg_dataset_is_null(tr_info, 0, pg_dataset_get_col_no(tr_info, _T("tgconstrrelname")))) {
				_stprintf(msg_buf, 
					_T("query produced NULL referenced table name for foreign key trigger \"%s\" on table \"%s\""),
					object_name, pg_dataset_data2(tr_info, 0, _T("relname")));
				goto ERR1;
			}
			edit_data->paste(_T(" from "), FALSE);
			edit_data->paste(
				make_object_name(pg_dataset_data2(tr_info, 0, _T("tgconstrrelname"))).GetBuffer(0), FALSE);
		}

		edit_data->paste(_T("\n"), FALSE);

		if(_tcscmp(pg_dataset_data2(tr_info, 0, _T("tgdeferrable")), _T("f")) == 0) {
			edit_data->paste(_T("not "), FALSE);
		}
		edit_data->paste(_T("deferrable initially "), FALSE);
		if(_tcscmp(pg_dataset_data2(tr_info, 0, _T("tginitdeferred")), _T("t")) == 0) {
			edit_data->paste(_T("deferred"), FALSE);
		} else {
			edit_data->paste(_T("immediate"), FALSE);
		}
	}
	edit_data->paste(_T("\n"), FALSE);
	edit_data->paste(_T("for each row\n"), FALSE);
	if(pg_get_remote_version(ss) < 110000) {
		edit_data->paste(_T("execute procedure "), FALSE);
	} else {
		edit_data->paste(_T("execute function "), FALSE);
	}
	edit_data->paste(make_object_name(pg_dataset_data2(tr_info, 0, _T("pronspname"))).GetBuffer(0), FALSE);
	edit_data->paste(_T("."), FALSE);
	edit_data->paste(make_object_name(pg_dataset_data2(tr_info, 0, _T("proname"))).GetBuffer(0), FALSE);

	if(put_trigger_func_args(edit_data, tr_info, msg_buf) != 0) goto ERR1;

	if(dump_comment(ss, edit_data, _T("trigger"), trigger_name.GetBuffer(0),
		oid, _T("pg_trigger"), 0, NULL) != 0) goto ERR1;

	pg_free_dataset(tr_info);
	return 0;

ERR1:
	pg_free_dataset(tr_info);
	return 1;
}

static int create_view_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid,
	const TCHAR *object_name, const TCHAR *schema, CEditData *edit_data, TCHAR *msg_buf)
{
	TCHAR		sql_buf[4096];
	HPgDataset	view_info = NULL;

	_stprintf(sql_buf,
		_T("select viewname, definition from pg_views \n")
		_T("where viewname = '%s' \n")
		_T("and schemaname = '%s' \n"),
		object_name, schema);
	view_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(view_info == NULL || pg_dataset_row_cnt(view_info) == 0) goto ERR1;

	// create view
	edit_data->paste(_T("create or replace view "), FALSE);
	edit_data->paste(make_object_name(schema).GetBuffer(0), FALSE);
	edit_data->paste(_T("."), FALSE);
	edit_data->paste(make_object_name(object_name).GetBuffer(0), FALSE);
	edit_data->paste(_T(" as\n"), FALSE);
	edit_data->paste(pg_dataset_data2(view_info, 0, _T("definition")), FALSE);
	edit_data->paste(_T("\n"), FALSE);

	if(dump_comment(ss, edit_data, _T("view"),
		make_object_name(object_name).GetBuffer(0),
		oid, _T("pg_class"), 0, NULL) != 0) goto ERR1;

	pg_free_dataset(view_info);
	return 0;

ERR1:
	pg_free_dataset(view_info);
	return 1;
}

static int create_materialized_view_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid,
	const TCHAR *object_name, const TCHAR *schema, CEditData *edit_data, TCHAR *msg_buf)
{
	TCHAR		sql_buf[4096];
	HPgDataset	view_info = NULL;

	_stprintf(sql_buf,
		_T("select matviewname, definition from pg_matviews \n")
		_T("where matviewname = '%s' \n")
		_T("and schemaname = '%s' \n"),
		object_name, schema);
	view_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(view_info == NULL || pg_dataset_row_cnt(view_info) == 0) goto ERR1;

	// create view
	edit_data->paste(_T("create materialized view "), FALSE);
	edit_data->paste(make_object_name(object_name).GetBuffer(0), FALSE);
	edit_data->paste(_T(" as\n"), FALSE);
	edit_data->paste(pg_dataset_data2(view_info, 0, _T("definition")), FALSE);
	edit_data->paste(_T("\n"), FALSE);

	if(dump_comment(ss, edit_data, _T("view"),
		make_object_name(object_name).GetBuffer(0),
		oid, _T("pg_class"), 0, NULL) != 0) goto ERR1;

	pg_free_dataset(view_info);
	return 0;

ERR1:
	pg_free_dataset(view_info);
	return 1;
}

static int create_type_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name,
	CEditData *edit_data, TCHAR *msg_buf)
{
	int			row_no = -1;
	HPgDataset	type_list = NULL;

	type_list = get_type_list_by_oid(ss, oid, msg_buf);
	if(type_list == NULL || pg_dataset_row_cnt(type_list) == 0) {
		_stprintf(msg_buf, _T("type %s not found"), object_name);
		goto ERR1;
	}

	row_no = 0;

	// create type
	edit_data->paste(_T("create type "), FALSE);
	edit_data->paste(make_object_name(object_name).GetBuffer(0), FALSE);
	edit_data->paste(_T("(\n\tinternallength = "), FALSE);
	if(_tcscmp(pg_dataset_data2(type_list, row_no, _T("typlen")), _T("-1")) == 0) {
		edit_data->paste(_T("variable"), FALSE);
	} else {
		edit_data->paste(pg_dataset_data2(type_list, row_no, _T("typlen")), FALSE);
	}

	edit_data->paste(_T(", externallength = "), FALSE);
	if(pg_get_remote_version(ss) < 70300) {
		if(_tcscmp(pg_dataset_data2(type_list, row_no, _T("typprtlen")), _T("-1")) == 0) {
			edit_data->paste(_T("variable"), FALSE);
		} else {
			edit_data->paste(pg_dataset_data2(type_list, row_no, _T("typprtlen")), FALSE);
		}
	}

	edit_data->paste(_T(",\n\tinput = "), FALSE);
	edit_data->paste(
		make_object_name(pg_dataset_data2(type_list, row_no, _T("typinput"))).GetBuffer(0), FALSE);
	edit_data->paste(_T(",output = "), FALSE);
	edit_data->paste(
		make_object_name(pg_dataset_data2(type_list, row_no, _T("typoutput"))).GetBuffer(0), FALSE);

	if(pg_get_remote_version(ss) < 70300) {
		edit_data->paste(_T(",\n\tsend = "), FALSE);
		edit_data->paste(
			make_object_name(pg_dataset_data2(type_list, row_no, _T("typsend"))).GetBuffer(0), FALSE);
		edit_data->paste(_T(", receive = "), FALSE);
		edit_data->paste(
			make_object_name(pg_dataset_data2(type_list, row_no, _T("typreceive"))).GetBuffer(0), FALSE);
	}

	if(!pg_dataset_is_null2(type_list, row_no, _T("typdefault"))) {
		edit_data->paste(_T(",\n\tdefault = "), FALSE);
		put_literal(edit_data, pg_dataset_data2(type_list, row_no, _T("typdefault")), CONV_ALL);
	}

	if((_tcscmp(pg_dataset_data2(type_list, row_no, _T("typelem")), _T("0")) != 0) &&
		pg_dataset_data2(type_list, row_no, _T("typname"))[0] != '_') {

		const TCHAR       *elemType;
		elemType = find_type_by_oid(type_list, pg_dataset_data2(type_list, row_no, _T("typelem")), zeroAsOpaque);

		if(elemType == NULL) {
			_stprintf(msg_buf, _T("fatal error in create_type_source()"));
			goto ERR1;
		}

		edit_data->paste(_T(",\n\telement = "), FALSE);
		edit_data->paste(elemType, FALSE);
		edit_data->paste(_T(", delimiter = "), FALSE);
		put_literal(edit_data, pg_dataset_data2(type_list, row_no, _T("typdelim")), CONV_ALL);
	}

	edit_data->paste(_T(",\n\talignment = "), FALSE);
	switch(pg_dataset_data2(type_list, row_no, _T("typalign"))[0]) {
	case 'c':
		edit_data->paste(_T("char"), FALSE);
		break;
	case 's':
		edit_data->paste(_T("int2"), FALSE);
		break;
	case 'i':
		edit_data->paste(_T("int4"), FALSE);
		break;
	case 'd':
		edit_data->paste(_T("double"), FALSE);
		break;
	}

	edit_data->paste(_T(", storage = "), FALSE);
	switch(pg_dataset_data2(type_list, row_no, _T("typstorage"))[0]) {
	case 'p':
		edit_data->paste(_T("plain"), FALSE);
		break;
	case 'e':
		edit_data->paste(_T("external"), FALSE);
		break;
	case 'x':
		edit_data->paste(_T("extended"), FALSE);
		break;
	case 'm':
		edit_data->paste(_T("main"), FALSE);
		break;
	}

	if(_tcscmp(pg_dataset_data2(type_list, row_no, _T("typbyval")), _T("f")) != 0) {
		edit_data->paste(_T(", passedbyvalue"), FALSE);
	}
	edit_data->paste(_T(");\n"), FALSE);

	if(dump_comment(ss, edit_data, _T("type"),
		make_object_name(object_name).GetBuffer(0),
		oid, _T("pg_type"), 0, NULL) != 0) goto ERR1;

	pg_free_dataset(type_list);
	return 0;

ERR1:
	pg_free_dataset(type_list);
	return 1;
}

static int create_index_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name,
	CEditData *edit_data, TCHAR *msg_buf)
{
	TCHAR		sql_buf[4096];
	HPgDataset	index_info = NULL;

	_stprintf(sql_buf,
		_T("select i.indexrelid as indexreloid, \n")
		_T("i.indrelid as indreloid, \n")
		_T("t1.relname as indexrelname, t2.relname as indrelname, \n")
		_T("pg_get_indexdef(i.indexrelid) as indexdef, \n")
		_T("i.indisprimary, i.indkey \n")
		_T("from pg_index i, pg_class t1, pg_class t2 \n")
		_T("where t1.oid = i.indexrelid and t2.oid = i.indrelid \n")
		_T("and i.indexrelid = '%s' \n"),
		oid);
	index_info = pg_create_dataset(ss, sql_buf, msg_buf);
	if(index_info == NULL) goto ERR1;
	if(pg_dataset_row_cnt(index_info) == 0) {
		_stprintf(msg_buf, _T("index %s not found"), object_name);
		goto ERR1;
	}

	// create index
	edit_data->paste(pg_dataset_data2(index_info, 0, _T("indexdef")), FALSE);
	edit_data->paste(_T(";\n"), FALSE);

	if(dump_comment(ss, edit_data, _T("index"),
		make_object_name(object_name).GetBuffer(0),
		oid, _T("pg_class"), 0, NULL) != 0) goto ERR1;

	pg_free_dataset(index_info);
	return 0;

ERR1:
	pg_free_dataset(index_info);
	return 1;
}

int create_object_source(HPgSession ss, const TCHAR *object_type, const TCHAR *oid, const TCHAR *object_name, const TCHAR *schema,
	CEditData *edit_data, TCHAR *msg_buf, BOOL recalc_disp_size)
{
	int		ret_v = 0;

	// auto indentが有効のとき、ソース取得結果のインデントがおかしくなるバグを回避する
	INDENT_MODE		indent_mode = edit_data->get_indent_mode();
	edit_data->set_indent_mode(INDENT_MODE_NONE);

	if(_tcscmp(object_type, _T("FUNCTION")) == 0 || _tcscmp(object_type, _T("PROCEDURE")) == 0) {
		ret_v = create_function_source(ss, object_type, oid, object_name, schema, edit_data, msg_buf);
	} else if(_tcscmp(object_type, _T("SEQUENCE")) == 0) {
		ret_v = create_sequence_source(ss, object_type, oid, object_name, schema, edit_data, msg_buf);
	} else if(_tcscmp(object_type, _T("TRIGGER")) == 0) {
		ret_v = create_trigger_source(ss, object_type, oid, object_name, schema, edit_data, msg_buf);
	} else if(_tcscmp(object_type, _T("VIEW")) == 0) {
		ret_v = create_view_source(ss, object_type, oid, object_name, schema, edit_data, msg_buf);
	} else if(_tcscmp(object_type, _T("MATERIALIZED VIEW")) == 0) {
		ret_v = create_materialized_view_source(ss, object_type, oid, object_name, schema, edit_data, msg_buf);
	} else if(_tcscmp(object_type, _T("TYPE")) == 0) {
		ret_v = create_type_source(ss, object_type, oid, object_name, edit_data, msg_buf);
	} else if(_tcscmp(object_type, _T("INDEX")) == 0) {
		ret_v = create_index_source(ss, object_type, oid, object_name, edit_data, msg_buf);
	} else if(_tcscmp(object_type, _T("TABLE")) == 0) {
		ret_v = dumpTableSchema(ss, oid, edit_data, msg_buf);
	}
	edit_data->set_indent_mode(indent_mode);

	if(recalc_disp_size) {
		edit_data->recalc_disp_size();
		edit_data->set_cur(0, 0);
		edit_data->reset_undo();
	}

	return ret_v;
}
