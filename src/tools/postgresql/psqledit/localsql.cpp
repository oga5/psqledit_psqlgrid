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

CString show_search_path(HPgSession ss, TCHAR *msg_buf)
{
	CString search_path;

	HPgDataset dataset = NULL;
	dataset = pg_create_dataset(ss, _T("show search_path"), msg_buf);
	if(dataset == NULL) {
		return _T("");
	}
	search_path = pg_dataset_data(dataset, 0, 0);
	pg_free_dataset(dataset);

	return search_path;
}

int set_search_path(HPgSession ss, const TCHAR *search_path, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	int		ret_v;

	_stprintf(sql_buf, _T("set search_path = %s"), search_path);

	ret_v = pg_exec_sql(ss, sql_buf, msg_buf);
	if(ret_v != 0) return ret_v;

	return 0;
}

static void add_search_path_array(HPgSession ss, CStringArray &schema_arr, CString &schema_name)
{
	schema_name.Trim(' ');
	if(schema_name.Compare(_T("$user")) == 0 || schema_name.Compare(_T("\"$user\"")) == 0) {
		schema_name = pg_user(ss);
	}
	schema_arr.Add(schema_name);
}

static void get_search_path_array(HPgSession ss, const TCHAR *search_path, CStringArray &schema_arr)
{
	const TCHAR *p;
	const TCHAR *prev_p;
	CString schema_name;

	prev_p = search_path;
	for(p = search_path;; p++) {
		if(*p == '\0') {
			schema_name = prev_p;
			add_search_path_array(ss, schema_arr, schema_name);
			break;
		}

		if(*p == ',') {
			schema_name.Format(_T("%.*s"), p - prev_p, prev_p);
			add_search_path_array(ss, schema_arr, schema_name);
			prev_p = p;
			prev_p++;
		}
	}
}

// 指定したschema_nameがsearch_pathに含まれるかチェックする
bool is_schema_in_search_path(HPgSession ss, const TCHAR* schema_name, TCHAR* msg_buf)
{
	if(_tcscmp(schema_name, _T("pg_catalog")) == 0) {
		// pg_catalogは常に参照可能
		return true;
	}

	CString search_path = show_search_path(ss, msg_buf);
	if (search_path.IsEmpty()) {
		return false;
	}

	CStringArray schema_arr;
	get_search_path_array(ss, search_path, schema_arr);

	CString target_schema = schema_name;
	target_schema.Trim(_T(" "));

	for (int i = 0; i < schema_arr.GetCount(); ++i) {
		CString sp_schema = schema_arr.GetAt(i);
		if (sp_schema.Compare(target_schema) == 0) {
			return true;
		}
	}
	return false;
}

// table名からschema名を取得
CString get_table_schema_name(HPgSession ss, const TCHAR *table_name, TCHAR *msg_buf)
{
	HPgDataset dataset = NULL;
	TCHAR sql_buf[4000];
	CString schema_name = _T("");
	CString user_name = pg_user(ss);
	CStringArray schema_arr;
	int i, j;

	if(_tcslen(table_name) == 0) {
		return _T("");
	}

	// search_pathの優先順位にする
	// show search_pathで取得できる ($user, publicのように取得できるので、カンマ区切りで分割、$userを自ユーザー名に置き換える
	CString search_path = show_search_path(ss, msg_buf);
	get_search_path_array(ss, search_path, schema_arr);

	if(pg_get_remote_version(ss) < 70300) {
		return schema_name;
	}

	_stprintf(sql_buf,
		_T("select schemaname \n")
		_T("from pg_tables \n")
		_T("where tablename = '%s' \n"),
		table_name
	);

	dataset = pg_create_dataset(ss, sql_buf, msg_buf);
	if(dataset == NULL) return schema_name;

	for(i = 0; i < schema_arr.GetCount(); i++) {
		CString tmp_schema = schema_arr.GetAt(i);

		for(j = 0; j < pg_dataset_row_cnt(dataset); j++) {
			CString table_schema = pg_dataset_data2(dataset, j, _T("SCHEMANAME"));
			if(table_schema.Compare(tmp_schema) == 0) {
				schema_name = table_schema;
				goto END_SEARCH_SCHEMA;
			}
		}
	}

END_SEARCH_SCHEMA:

	pg_free_dataset(dataset);

	return schema_name;
}


/*------------------------------------------------------------------------------
 データベースの検索
------------------------------------------------------------------------------*/
int get_dataset(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf, 
	void *hWnd, volatile int *cancel_flg, HPgDataset *result)
{
	return pg_create_dataset_ex(ss, sql, msg_buf, cancel_flg, hWnd, result);
}

/*------------------------------------------------------------------------------
 SQL実行
------------------------------------------------------------------------------*/
int execute_sql(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf,
	volatile int *cancel_flg)
{
	return pg_exec_sql_ex(ss, sql, msg_buf, cancel_flg);
}


/*------------------------------------------------------------------------------
 SQL実行計画の取得
------------------------------------------------------------------------------*/
CString explain_plan(HPgSession ss, const TCHAR *sql, TCHAR *msg_buf)
{
	const TCHAR	*plan;

	plan = pg_explain_plan(ss, sql, msg_buf);
	if(plan == NULL) return _T("");

	CString exp_plan = plan;

	free((void *)plan);

	return exp_plan;
}

static int pg_save_dataset_main_ar(CUnicodeArchive *ar, HPgDataset dataset, int put_colname, TCHAR *msg_buf)
{
	int		r, c;
	int		col_cnt = pg_dataset_col_cnt(dataset);
	int		row_cnt = pg_dataset_row_cnt(dataset);
	const TCHAR	*sepa = _T(",");

	if(put_colname == 1) {
		for(c = 0; c < col_cnt; c++) {
			if(c != 0) {
				ar->WriteString(sepa);
			}
			ar->CsvOut(pg_dataset_get_colname(dataset, c), TRUE);
		}
		ar->WriteNextLine();
	}

	for(r = 0; r < row_cnt; r++) {
		for(c = 0; c < col_cnt; c++) {
			if(c != 0) {
				ar->WriteString(sepa);
			}
			ar->CsvOut(pg_dataset_data(dataset, r, c), TRUE);
		}
		ar->WriteNextLine();
	}

	return 0;
}

/*------------------------------------------------------------------------------
 SQL実行結果の保存
------------------------------------------------------------------------------*/
int download(HPgSession ss, CUnicodeArchive *ar, const TCHAR *sql, TCHAR *msg_buf,
	BOOL put_column_name, void *hWnd, volatile int *cancel_flg)
{
	HPgDataset dataset = NULL;
	int			ret_v;

	ret_v = pg_create_dataset_ex(ss, sql, msg_buf, cancel_flg, hWnd, &dataset);
	if(ret_v != 0) return ret_v;

	ret_v = pg_save_dataset_main_ar(ar, dataset, put_column_name, msg_buf);
	if(ret_v != 0) return ret_v;

	pg_free_dataset(dataset);

	return ret_v;
}

HPgDataset get_user_list(HPgSession ss, TCHAR *msg_buf)
{
	return pg_create_dataset(ss, 
		_T("select usename \n")
		_T("from pg_user \n")
		_T("order by usename"),
		msg_buf);
}

static HPgDataset get_object_list_common(HPgSession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	TCHAR	type_cond[1024] = _T("");
	TCHAR	where_cond[1024] = _T("");

	if(_tcscmp(type, _T("TABLE")) == 0) {
		if(pg_get_remote_version(ss) >= 80100) {
			// pertition tableの親テーブル relkind=pを追加
			_tcscpy(type_cond, _T("where ((c.relkind = 'r'::\"char\") or (c.relkind = 's'::\"char\") or (c.relkind = 'p'::\"char\")) "));
		} else {
			_tcscpy(type_cond, _T("where ((c.relkind = 'r'::\"char\") or (c.relkind = 's'::\"char\")) "));
		}
	} else if(_tcscmp(type, _T("FOREIGN TABLE")) == 0) {
		_tcscpy(type_cond, _T("where (c.relkind = 'f'::\"char\") "));
	} else if(_tcscmp(type, _T("VIEW")) == 0) {
		_tcscpy(type_cond, _T("where (c.relkind = 'v'::\"char\") "));
	} else if(_tcscmp(type, _T("MATERIALIZED VIEW")) == 0) {
		_tcscpy(type_cond, _T("where (c.relkind = 'm'::\"char\") "));
	} else if(_tcscmp(type, _T("SEQUENCE")) == 0) {
		_tcscpy(type_cond, _T("where (c.relkind = 'S'::\"char\") "));
	} else {
		_stprintf(msg_buf, _T("not implement"));
		return NULL;
	}

	if(_tcscmp(owner, ALL_USERS) != 0) {
		_stprintf(where_cond,
			_T("%s \n and pg_get_userbyid(c.relowner) = '%s' "),
			type_cond, owner);
	} else {
		_stprintf(where_cond,
			_T("%s"),
			type_cond);
	}

	/*
		7.2.6, 7.3.8, 7.4.6より前のバージョンでは，大きな外部結合で正しい検索結果が
		得られないバグがあるので，以下のようなSQLに置き換える
		(left outer joinをunionに置き換える)

		select c.relname as name, n.nspname, 
			pg_get_userbyid(c.relowner) as owner, 
			c.oid, d.description as comment
		from pg_class c, pg_namespace n, pg_description d
		where c.relnamespace = n.oid
		and c.oid = d.objoid
		and (d.objsubid = 0 or d.objsubid is null)
		union
		select c.relname as name, n.nspname, 
			pg_get_userbyid(c.relowner) as owner, 
			c.oid as objoid, ''::text as comment
		from pg_class c, pg_namespace n
		where c.relnamespace = n.oid
		and not exists(select objoid from pg_description d
			where c.oid = d.objoid)
	*/
	if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("select c.relname as name, n.nspname,  \n")
			_T("	pg_get_userbyid(c.relowner) as owner,  \n")
			_T("	d.description as comment, c.oid \n")
			_T("from pg_class c, pg_namespace n, pg_description d \n")
			_T("%s \n")
			_T("and c.relnamespace = n.oid \n")
			_T("and c.oid = d.objoid \n")
			_T("and (d.objsubid = 0 or d.objsubid is null) \n")
			_T("union \n")
			_T("select c.relname as name, n.nspname,  \n")
			_T("	pg_get_userbyid(c.relowner) as owner,  \n")
			_T("	null::text as comment, c.oid \n")
			_T("from pg_class c, pg_namespace n \n")
			_T("%s \n")
			_T("and c.relnamespace = n.oid \n")
			_T("and not exists(select objoid from pg_description d \n")
			_T("	where c.oid = d.objoid and (d.objsubid = 0 or d.objsubid is null)) \n")
			_T("order by name \n"),
			where_cond, where_cond);
	} else {
	if(pg_has_objsubid(ss)) {
		_stprintf(sql_buf,
			_T("select c.relname as name, ''::\"char\" as nspname, \n")
			_T("	pg_get_userbyid(c.relowner) as owner,  \n")
			_T("	d.description as comment, c.oid \n")
			_T("from pg_class c, pg_description d \n")
			_T("%s \n")
			_T("and c.oid = d.objoid \n")
			_T("and (d.objsubid = 0 or d.objsubid is null) \n")
			_T("union \n")
			_T("select c.relname as name, ''::\"char\" as nspname, \n")
			_T("	pg_get_userbyid(c.relowner) as owner,  \n")
			_T("	null::text as comment, c.oid \n")
			_T("from pg_class c \n")
			_T("%s \n")
			_T("and not exists(select objoid from pg_description d \n")
			_T("	where c.oid = d.objoid and (d.objsubid = 0 or d.objsubid is null)) \n")
			_T("order by name \n"),
			where_cond, where_cond);
	} else {
		_stprintf(sql_buf,
			_T("select c.relname as name, ''::\"char\" as nspname, \n")
			_T("	pg_get_userbyid(c.relowner) as owner,  \n")
			_T("	d.description as comment, c.oid \n")
			_T("from pg_class c, pg_description d \n")
			_T("%s \n")
			_T("and c.oid = d.objoid \n")
			_T("union \n")
			_T("select c.relname as name, ''::\"char\" as nspname, \n")
			_T("	pg_get_userbyid(c.relowner) as owner,  \n")
			_T("	null::text as comment, c.oid \n")
			_T("from pg_class c \n")
			_T("%s \n")
			_T("and not exists(select objoid from pg_description d \n")
			_T("	where c.oid = d.objoid) \n")
			_T("order by name \n"),
			where_cond, where_cond);
	}
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

static HPgDataset get_object_list_foreign_table(HPgSession ss, const TCHAR* owner, const TCHAR* type, TCHAR* msg_buf)
{
	TCHAR	sql_buf[4096];
	TCHAR	where_cond[1024] = _T("");

	if(pg_get_remote_version(ss) < 90100) {
		return get_object_list_common(ss, owner, type, msg_buf);
	}

	if(_tcscmp(owner, ALL_USERS) != 0) {
		_stprintf(where_cond,
			_T("\n and pg_get_userbyid(c.relowner) = '%s' "),
			owner);
	}

	_stprintf(sql_buf,
		_T("select c.relname, n.nspname, pg_get_userbyid(c.relowner) as owner, s.srvname, fdw.fdwname, ft.ftrelid as oid  \n")
		_T("from pg_foreign_table ft, pg_class c, pg_foreign_server s, pg_foreign_data_wrapper fdw, pg_namespace n \n")
		_T("where ft.ftrelid = c.oid \n")
		_T("and ft.ftserver = s.oid \n")
		_T("and s.srvfdw = fdw.oid \n")
		_T("and c.relnamespace = n.oid \n")
		_T("%s")
		_T("order by c.relname"),
		where_cond);

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

static HPgDataset get_object_list_index(HPgSession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	TCHAR	where_cond[1024] = _T("");

	if(_tcscmp(owner, ALL_USERS) != 0) {
		_stprintf(where_cond,
			_T("\n and pg_get_userbyid(c.relowner) = '%s' "),
			owner);
	}

	if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("select i.indexname, i.nspname, i.tablename, i.owner, d.description, i.objoid as oid \n")
			_T("from (select n.nspname, c.relname as tablename, i.relname as indexname, \n")
			_T("		pg_get_userbyid(c.relowner) as owner, \n")
			_T("		i.oid as objoid \n")
			_T("	from pg_index x, pg_class c, pg_class i, pg_namespace n \n")
			_T("	where ((((c.relkind = 'r'::\"char\") \n")
			_T("	and (i.relkind = 'i'::\"char\")) \n")
			_T("	and (c.oid = x.indrelid)) \n")
			_T("	and (i.oid = x.indexrelid)) \n")
			_T("	and (c.relnamespace = n.oid) %s) i \n")
			_T("		left outer join pg_description d using(objoid) \n")
			_T("order by i.tablename, i.indexname \n"),
			where_cond);
	} else {
		_stprintf(sql_buf,
			_T("select i.indexname, ''::\"char\" as nspname, i.tablename, i.owner, d.description, i.objoid as oid \n")
			_T("from (select c.relname as tablename, i.relname as indexname,  \n")
			_T("		pg_get_userbyid(c.relowner) as owner, \n")
			_T("		i.oid as objoid \n")
			_T("	from pg_index x, pg_class c, pg_class i  \n")
			_T("	where ((((c.relkind = 'r'::\"char\")  \n")
			_T("	and (i.relkind = 'i'::\"char\"))  \n")
			_T("	and (c.oid = x.indrelid))  \n")
			_T("	and (i.oid = x.indexrelid)) %s) i \n")
			_T("		left outer join pg_description d using(objoid) \n")
			_T("order by i.tablename, i.indexname \n"),
			where_cond);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

static HPgDataset get_object_list_proc(HPgSession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	TCHAR	where_cond[1024] = _T("");

	if(pg_get_remote_version(ss) >= 110000) {
		if(_tcscmp(owner, ALL_USERS) != 0) {
			_stprintf(where_cond, _T("\n and pg_get_userbyid(p.proowner) = '%s' "), owner);
		}

		TCHAR prokind[10] = _T("");
		if(_tcscmp(type, _T("FUNCTION")) == 0) {
			_tcscpy(prokind, _T("f"));
		}
		if(_tcscmp(type, _T("PROCEDURE")) == 0) {
			_tcscpy(prokind, _T("p"));
		}

		_stprintf(sql_buf,
			_T("select p.name, p.nspname, p.owner, d.description, p.objoid as oid \n")
			_T("from(select n.nspname, \n")
			_T("	p.proname || '(' || oidvectortypes(p.proargtypes) || ')' as name, \n")
			_T("	pg_get_userbyid(p.proowner) as owner, \n")
			_T("		p.oid as objoid \n")
			_T("	from pg_proc p, pg_namespace n \n")
			_T("	where p.pronamespace = n.oid \n")
			_T("	and p.prokind = '%s' \n")
			_T("	%s) p \n")
			_T("		left outer join pg_description d using(objoid) \n")
			_T("order by name \n"),
			prokind, where_cond);
	} else if(pg_get_remote_version(ss) >= 70300) {
		if(_tcscmp(owner, ALL_USERS) != 0) {
			_stprintf(where_cond, _T("\n and pg_get_userbyid(p.proowner) = '%s' "), owner);
		}
		_stprintf(sql_buf,
			_T("select p.name, p.nspname, p.owner, d.description, p.objoid as oid \n")
			_T("from(select n.nspname, \n")
			_T("	p.proname || '(' || oidvectortypes(p.proargtypes) || ')' as name, \n")
			_T("	pg_get_userbyid(p.proowner) as owner, \n")
			_T("		p.oid as objoid \n")
			_T("	from pg_proc p, pg_namespace n \n")
			_T("	where p.pronamespace = n.oid %s) p \n")
			_T("		left outer join pg_description d using(objoid) \n")
			_T("order by name \n"),
			where_cond);
	} else {
		if(_tcscmp(owner, ALL_USERS) != 0) {
			_stprintf(where_cond, _T("\n where pg_get_userbyid(p.proowner) = '%s' "), owner);
		}
		_stprintf(sql_buf,
			_T("select p.name, ''::\"char\" as nspname, p.owner, d.description, p.objoid as oid \n")
			_T("from(select p.proname || '(' || oidvectortypes(p.proargtypes) || ')' as name, \n")
			_T("	pg_get_userbyid(p.proowner) as owner, \n")
			_T("		p.oid as objoid \n")
			_T("	from pg_proc p %s) p \n")
			_T("		left outer join pg_description d using(objoid) \n")
			_T("order by name \n"),
			where_cond);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

static HPgDataset get_object_list_trigger(HPgSession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	TCHAR	where_cond[1024] = _T("");

	if(_tcscmp(owner, ALL_USERS) != 0) {
		_stprintf(where_cond,
			_T("\n and pg_get_userbyid(p.proowner) = '%s' "),
			owner);
	}

	if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("select t.name, t.nspname, t.tablename, t.function, t.tgtype, t.owner,  \n")
			_T("	d.description, t.objoid as oid \n")
			_T("from (select n.nspname, t.tgname as name, c.relname as tablename,  \n")
			_T("		p.proname as function, \n")
			_T("		t.tgtype, pg_get_userbyid(c.relowner) as owner, \n")
			_T("		t.oid as objoid \n")
			_T("	from pg_trigger t, pg_class c, pg_proc p, pg_namespace n \n")
			_T("	where c.oid = t.tgrelid \n")
			_T("	and p.oid = t.tgfoid \n")
			_T("	and c.relnamespace = n.oid %s) t \n")
			_T("	left outer join pg_description d using(objoid) \n")
			_T("order by name \n"),
			where_cond);
	} else {
		_stprintf(sql_buf,
			_T("select t.name, ''::\"char\" as nspname, t.tablename, t.function, t.tgtype, t.owner, d.description, \n")
			_T("	t.objoid as oid \n")
			_T("from (select t.tgname as name, c.relname as tablename, p.proname as function,  \n")
			_T("		t.tgtype, pg_get_userbyid(c.relowner) as owner,  \n")
			_T("		t.oid as objoid \n")
			_T("	from pg_trigger t, pg_class c, pg_proc p \n")
			_T("	where c.oid = t.tgrelid \n")
			_T("	and p.oid = t.tgfoid %s) t \n")
			_T("	left outer join pg_description d using(objoid) \n")
			_T("order by name \n"),
			where_cond);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

static HPgDataset get_object_list_type(HPgSession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	TCHAR	where_cond[1024] = _T("");

	if(_tcscmp(owner, ALL_USERS) != 0) {
		_stprintf(where_cond,
			_T("\n and pg_get_userbyid(t.typowner) = '%s' "),
			owner);
	}

	if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("select t.name, t.nspname, t.owner, t.typlen as len,  \n")
			_T("	d.description, t.objoid as oid \n")
			_T("from (select n.nspname, typname as name, \n")
			_T("		pg_get_userbyid(t.typowner) as owner, typlen, \n")
			_T("		t.oid as objoid \n")
			_T("	from pg_type t, pg_namespace n \n")
			_T("	where t.typtype = 'b' \n")
			_T("	and t.typnamespace = n.oid %s) t \n")
			_T("	left outer join pg_description d using(objoid) \n")
			_T("order by t.name \n"),
			where_cond);
	} else {
		_stprintf(sql_buf,
			_T("select t.name, ''::\"char\" as nspname, t.owner, t.typlen as len, d.description, t.objoid as oid \n")
			_T("from (select typname as name,  \n")
			_T("		pg_get_userbyid(t.typowner) as owner, typlen, \n")
			_T("		t.oid as objoid \n")
			_T("	from pg_type t \n")
			_T("	where t.typtype = 'b' %s) t \n")
			_T("	left outer join pg_description d using(objoid) \n")
			_T("order by t.name \n"),
			where_cond);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

HPgDataset get_object_list(HPgSession ss, const TCHAR *owner, const TCHAR *type, TCHAR *msg_buf)
{
	if(_tcscmp(type, _T("TABLE")) == 0 || _tcscmp(type, _T("VIEW")) == 0 || 
			_tcscmp(type, _T("MATERIALIZED VIEW")) == 0 || _tcscmp(type, _T("SEQUENCE")) == 0) {
		return get_object_list_common(ss, owner, type, msg_buf);
	}
	if(_tcscmp(type, _T("FOREIGN TABLE")) == 0) {
		return get_object_list_foreign_table (ss, owner, type, msg_buf);
	}
	if(_tcscmp(type, _T("INDEX")) == 0) {
		return get_object_list_index(ss, owner, type, msg_buf);
	}
	if(_tcscmp(type, _T("FUNCTION")) == 0) {
		return get_object_list_proc(ss, owner, type, msg_buf);
	}
	if(_tcscmp(type, _T("PROCEDURE")) == 0) {
		return get_object_list_proc(ss, owner, type, msg_buf);
	}
	if(_tcscmp(type, _T("TRIGGER")) == 0) {
		return get_object_list_trigger(ss, owner, type, msg_buf);
	}
	if(_tcscmp(type, _T("TYPE")) == 0) {
		return get_object_list_type(ss, owner, type, msg_buf);
	}

	_stprintf(msg_buf, _T("not implement"));
	return NULL;
}

HPgDataset get_column_list(HPgSession ss, const TCHAR *relname, const TCHAR *schema, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	if(pg_get_remote_version(ss) >= 70300 && schema != NULL && _tcslen(schema) > 0) {
		_stprintf(sql_buf,
			_T("select att.attname, format_type(att.atttypid, NULL) as typname, att.attlen, \n")
			_T("	att.attnotnull, d.description, att.objsubid \n")
			_T("from (select a.attname, t.typname,  \n")
			_T("	(case when a.atttypmod > 4 then a.atttypmod - 4 else a.attlen end) as attlen, \n")
			_T("	a.attnotnull, a.attrelid as objoid, a.attnum as objsubid, a.atttypid \n")
			_T("	from pg_attribute a, pg_class c, pg_type t \n")
			_T("	where c.oid = a.attrelid \n")
			_T("	and t.oid = a.atttypid \n")
			_T("	and c.relname = '%s' \n")
			_T("	and c.relnamespace = (select oid from pg_namespace n where n.nspname = '%s') \n")
			_T("	and a.attnum > 0) att \n")
			_T("		left outer join pg_description d using(objoid, objsubid) \n")
			_T("order by att.objsubid \n"),
			relname, schema);
	} else if(pg_get_remote_version(ss) >= 70200) {
		if(pg_has_objsubid(ss)) {
			_stprintf(sql_buf,
				_T("select att.attname, att.typname, att.attlen, \n")
				_T("	att.attnotnull, d.description, att.objsubid \n")
				_T("from (select a.attname, t.typname,  \n")
				_T("	(case when a.atttypmod > 4 then a.atttypmod - 4 else a.attlen::int4 end) as attlen, \n")
				_T("	a.attnotnull, a.attrelid as objoid, a.attnum as objsubid \n")
				_T("	from pg_attribute a, pg_class c, pg_type t \n")
				_T("	where c.oid = a.attrelid \n")
				_T("	and t.oid = a.atttypid \n")
				_T("	and c.relname = '%s' \n")
				_T("	and a.attnum > 0) att  \n")
				_T("		left outer join pg_description d using(objoid, objsubid) \n")
				_T("order by att.objsubid \n"),
				relname);
		} else {
			_stprintf(sql_buf,
				_T("select att.attname, att.typname, att.attlen, \n")
				_T("	att.attnotnull, ''::\"text\" as description, att.objsubid \n")
				_T("from (select a.attname, t.typname,  \n")
				_T("	(case when a.atttypmod > 4 then a.atttypmod - 4 else a.attlen::int4 end) as attlen, \n")
				_T("	a.attnotnull, a.attrelid as objoid, a.attnum as objsubid \n")
				_T("	from pg_attribute a, pg_class c, pg_type t \n")
				_T("	where c.oid = a.attrelid \n")
				_T("	and t.oid = a.atttypid \n")
				_T("	and c.relname = '%s' \n")
				_T("	and a.attnum > 0) att  \n")
				_T("order by att.objsubid \n"),
				relname);
		}
	} else {
		_stprintf(sql_buf,
			_T("select att.attname, att.typname, att.attlen, \n")
			_T("	att.attnotnull, d.description, att.attnum \n")
			_T("from (select a.attname, t.typname, a.attlen, \n")
			_T("	a.attnotnull, a.attnum, a.oid as objoid \n")
			_T("	from pg_attribute a, pg_class c, pg_type t \n")
			_T("	where c.oid = a.attrelid \n")
			_T("	and t.oid = a.atttypid \n")
			_T("	and c.relname = '%s' \n")
			_T("	and a.attnum > 0) att \n")
			_T("		left outer join pg_description d using(objoid) \n")
			_T("order by att.objoid \n"),
			relname);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

HPgDataset get_index_list_by_table(HPgSession ss, const TCHAR *relname, 
	const TCHAR *schema, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("select i.relname as indexname, a.attname as columnname, \n")
			_T("	x.indisunique, x.indisprimary, \n")
			_T("	i.oid as indexoid, c.oid as table_oid \n")
			_T("from pg_index x, pg_class c, pg_class i, pg_attribute a \n")
			_T("where ((((c.relkind = 'r'::\"char\")  \n")
			_T("and (i.relkind = 'i'::\"char\"))  \n")
			_T("and (c.oid = x.indrelid))  \n")
			_T("and (i.oid = x.indexrelid)) \n")
			_T("and c.relname = '%s' \n")
			_T("and i.oid = a.attrelid \n")
			_T("and c.relnamespace = (select oid from pg_namespace n where n.nspname = '%s') \n")
			_T("order by i.relname, a.attnum \n"),
			relname, schema);
	} else {
		_stprintf(sql_buf,
			_T("select i.relname as indexname, a.attname as columnname, \n")
			_T("	x.indisunique, x.indisprimary, \n")
			_T("	i.oid as indexoid, c.oid as table_oid \n")
			_T("from pg_index x, pg_class c, pg_class i, pg_attribute a \n")
			_T("where ((((c.relkind = 'r'::\"char\")  \n")
			_T("and (i.relkind = 'i'::\"char\"))  \n")
			_T("and (c.oid = x.indrelid))  \n")
			_T("and (i.oid = x.indexrelid)) \n")
			_T("and c.relname = '%s' \n")
			_T("and i.oid = a.attrelid \n")
			_T("order by i.relname, a.attnum \n"),
			relname);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

HPgDataset get_trigger_list_by_table(HPgSession ss, const TCHAR *relname,
	const TCHAR *schema, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];

	if(pg_get_remote_version(ss) >= 70300) {
		_stprintf(sql_buf,
			_T("select t.name, t.tablename, t.function, t.tgtype, d.description as comment \n")
			_T("from (select t.tgname as name, c.relname as tablename, p.proname as function, \n")
			_T("		t.tgtype, t.oid as objoid \n")
			_T("	from pg_trigger t, pg_class c, pg_proc p \n")
			_T("	where c.oid = t.tgrelid \n")
			_T("	and p.oid = t.tgfoid \n")
			_T("	and c.relname = '%s' \n")
			_T("	and c.relnamespace = (select oid from pg_namespace n where n.nspname = '%s')) t \n")
			_T("	left outer join pg_description d using(objoid) \n")
			_T("order by name \n"),
			relname, schema);
	} else {
		_stprintf(sql_buf,
			_T("select t.name, t.tablename, t.function, t.tgtype, d.description as comment \n")
			_T("from (select t.tgname as name, c.relname as tablename, p.proname as function, \n")
			_T("		t.tgtype, t.oid as objoid \n")
			_T("	from pg_trigger t, pg_class c, pg_proc p \n")
			_T("	where c.oid = t.tgrelid \n")
			_T("	and p.oid = t.tgfoid \n")
			_T("	and c.relname = '%s') t \n")
			_T("	left outer join pg_description d using(objoid) \n")
			_T("order by name \n"),
			relname);
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}

HPgDataset get_object_properties(HPgSession ss, const TCHAR *type, const TCHAR *name, 
	const TCHAR *schema, TCHAR *msg_buf)
{
	TCHAR	sql_buf[4096];
	
	if(_tcscmp(type, _T("TABLE")) == 0) {
		if(pg_get_remote_version(ss) >= 70300) {
			_stprintf(sql_buf,
				_T("select * from pg_tables \n")
				_T("where tablename = '%s' and schemaname = '%s' \n"),
				name, schema);
		} else {
			_stprintf(sql_buf,
				_T("select * from pg_tables \n")
				_T("where tablename = '%s' \n"),
				name);
		}
	} else if(_tcscmp(type, _T("FOREIGN TABLE")) == 0) {
		_stprintf(sql_buf,
			_T("select ft.ftrelid, c.relname, n.nspname, r.rolname, s.srvname, fdw.fdwname, ft.ftoptions \n")
			_T("from pg_foreign_table ft, pg_class c, pg_foreign_server s, pg_foreign_data_wrapper fdw, pg_namespace n, pg_roles r \n")
			_T("where ft.ftrelid = c.oid \n")
			_T("and ft.ftserver = s.oid \n")
			_T("and s.srvfdw = fdw.oid \n")
			_T("and c.relnamespace = n.oid \n")
			_T("and c.relowner = r.oid \n")
			_T("and c.relname = '%s' \n")
			_T("and n.nspname = '%s' \n"),
			name, schema);
	} else if(_tcscmp(type, _T("VIEW")) == 0) {
		if(pg_get_remote_version(ss) >= 70300) {
			_stprintf(sql_buf,
				_T("select * from pg_views \n")
				_T("where viewname = '%s' and schemaname = '%s' \n"),
				name, schema);
		} else {
			_stprintf(sql_buf,
				_T("select * from pg_views \n")
				_T("where viewname = '%s' \n"),
				name);
		}
	} else if(_tcscmp(type, _T("MATERIALIZED VIEW")) == 0) {
		_stprintf(sql_buf,
			_T("select * from pg_matviews \n")
			_T("where matviewname = '%s' and schemaname = '%s' \n"),
			name, schema);
	} else if(_tcscmp(type, _T("INDEX")) == 0) {
		if(pg_get_remote_version(ss) >= 70300) {
			_stprintf(sql_buf,
				_T("select * from pg_indexes \n")
				_T("where indexname = '%s' and schemaname = '%s' \n"),
				name, schema);
		} else {
			_stprintf(sql_buf,
				_T("select * from pg_indexes \n")
				_T("where indexname = '%s' \n"),
				name);
		}
	} else if(_tcscmp(type, _T("SEQUENCE")) == 0) {
		if(pg_get_remote_version(ss) >= 70300) {
			_stprintf(sql_buf,
				_T("select * from %s.%s \n"),
				schema, name);
		} else {
			_stprintf(sql_buf,
				_T("select * from %s \n"),
				name);
		}
	} else if(_tcscmp(type, _T("FUNCTION")) == 0) {
		_stprintf(sql_buf,
			_T("select * from pg_proc \n")
			_T("where oid = '%s' \n"),
			name);
	} else if(_tcscmp(type, _T("PROCEDURE")) == 0) {
		_stprintf(sql_buf,
			_T("select * from pg_proc \n")
			_T("where oid = '%s' \n"),
			name);
	} else if(_tcscmp(type, _T("TRIGGER")) == 0) {
		if(pg_get_remote_version(ss) >= 70300) {
			_stprintf(sql_buf,
				_T("select t.* from pg_trigger t, pg_class c \n")
				_T("where t.tgname = '%s' \n")
				_T("and t.tgrelid = c.oid \n")
				_T("and c.relnamespace = (select oid from pg_namespace where nspname = '%s') \n"),
				name, schema);
		} else {
			_stprintf(sql_buf,
				_T("select * from pg_trigger \n")
				_T("where tgname = '%s' \n"),
				name);
		}
	} else if(_tcscmp(type, _T("TYPE")) == 0) {
		if(pg_get_remote_version(ss) >= 70300) {
			_stprintf(sql_buf,
				_T("select * from pg_type \n")
				_T("where typname = '%s' \n")
				_T("and typnamespace = (select oid from pg_namespace where nspname = '%s') \n"),
				name, schema);
		} else {
			_stprintf(sql_buf,
				_T("select * from pg_type \n")
				_T("where typname = '%s' \n"),
				name);
		}
	} else {
		_stprintf(msg_buf, _T("not implement"));
		return NULL;
	}

	return pg_create_dataset(ss, sql_buf, msg_buf);
}
