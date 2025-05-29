/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "sqllog.h"

#include "fileutil.h"

#include <time.h>
#include <stdio.h>

CSqlLog::CSqlLog(int number, const TCHAR *sql)
{
	m_number = number;
	m_sql = sql;

	m_time = CSqlLogger::GetTime();
}

CSqlLogArray::CSqlLogArray(int max_log_cnt)
{
	m_next_number = 0;
	m_max_log_cnt = max_log_cnt;
	m_sql_log_array = new CPtrArray();
}

CSqlLogArray::~CSqlLogArray()
{
	for(; m_sql_log_array->GetSize() != 0;) {
		delete GetAt(0);
		m_sql_log_array->RemoveAt(0);
	}
	delete m_sql_log_array;
}

int CSqlLogArray::Add(const TCHAR *sql)
{
	CSqlLog *plog = new CSqlLog(m_next_number, sql);
	m_sql_log_array->Add(plog);

	if(m_sql_log_array->GetSize() > m_max_log_cnt) {
		delete GetAt(0);
		m_sql_log_array->RemoveAt(0);
	}

	m_next_number++;

	return 0;
}

// SQLLogger

int CSqlLogger::CreateNewLogFile(const TCHAR *log_file_name, TCHAR *msg_buf)
{
	FILE	*fp;

	fp = _tfopen(log_file_name, _T("w"));
	if(fp == NULL) {
		_stprintf(msg_buf, _T("ファイルが開けませんでした(%s)"), log_file_name);
		return 1;
	}

	CString date_str = GetDate();
	_ftprintf(fp, _T("-- psqledit log file\n"));
	_ftprintf(fp, _T("-- date: %s\n"), date_str.GetBuffer(0));
	_ftprintf(fp, _T("\n"));

	fclose(fp);

	return 0;
}

FILE *CSqlLogger::OpenLogFile(const TCHAR *log_file_name, TCHAR *msg_buf)
{
	FILE	*fp = NULL;

	if(is_file_exist(log_file_name) == FALSE) {
		if(CreateNewLogFile(log_file_name, msg_buf) != 0) return NULL;
	}

	fp = _tfopen(log_file_name, _T("r+"));
	if(fp == NULL) {
		_stprintf(msg_buf, _T("ファイルが開けませんでした(%s)"), log_file_name);
		return NULL;
	}

	return fp;
}

CString CSqlLogger::GetTime()
{
	time_t		_time;
	time(&_time);                /* long 整数として時刻を取得 */

	struct tm	*nowtime;
	nowtime = localtime(&_time); /* 現地時刻に変換 */

	CString	time_str;
	time_str.Format(_T("%.2d:%.2d:%.2d"),
		nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec);

	return time_str;
}

CString CSqlLogger::GetDate(int format)
{
	time_t		_time;
	time(&_time);                /* long 整数として時刻を取得 */

	struct tm	*nowtime;
	nowtime = localtime(&_time); /* 現地時刻に変換 */

	CString	time_str;

	if(format == 0) {
		time_str.Format(_T("%.4d-%.2d-%.2d"),
			nowtime->tm_year + 1900, nowtime->tm_mon + 1, nowtime->tm_mday);
	} else {
		time_str.Format(_T("%.4d%.2d%.2d"),
			nowtime->tm_year + 1900, nowtime->tm_mon + 1, nowtime->tm_mday);
	}

	return time_str;
}

int CSqlLogger::SaveSql(FILE *fp, const  TCHAR *user, const TCHAR *tns, const TCHAR *sql)
{
	CString time = GetTime();

	// ファイルの最後に移動
	fseek(fp, 0, SEEK_END);

	_ftprintf(fp, _T("--%s@%s %s\n"), user, tns, time.GetBuffer(0));
	_ftprintf(fp, _T("%s\n/\n\n"), sql);

	return 0;
}

CString CSqlLogger::MakeLogFileName(const TCHAR *log_dir_name)
{
	CString date_str = GetDate(1);

	CString log_file_name;
	log_file_name.Format(_T("%s%s.sql"), log_dir_name, date_str.GetBuffer(0));

	return log_file_name;
}

int CSqlLogger::Save(const TCHAR *user, const TCHAR *tns, const TCHAR *sql, TCHAR *msg_buf)
{
	DWORD dw = WaitForSingleObject(m_mutex, INFINITE);
	if(dw != WAIT_OBJECT_0) return 1;

	FILE	*fp = NULL;
	CString	log_file_name = MakeLogFileName(m_log_dir.GetBuffer(0));

	if(is_directory_exist(m_log_dir.GetBuffer(0)) == FALSE) {
		if(make_directory(m_log_dir.GetBuffer(0), msg_buf) != 0) goto ERR;
	}

	fp = OpenLogFile(log_file_name.GetBuffer(0), msg_buf);
	if(fp == NULL) goto ERR;

	SaveSql(fp, user, tns, sql);

	fclose(fp);

	ReleaseMutex(m_mutex);

	return 0;

ERR:
	ReleaseMutex(m_mutex);

	return 1;
}

CSqlLogger::CSqlLogger(const TCHAR *log_dir_name)
{
	m_mutex = CreateMutex(NULL, FALSE, _T("PSqlEdit_CSqlLogger_Mutex"));

	SetLogDir(log_dir_name);
}

void CSqlLogger::SetLogDir(const TCHAR *log_dir_name)
{
	m_log_dir = log_dir_name;

	make_dirname(m_log_dir.GetBuffer(_MAX_PATH));
	m_log_dir.ReleaseBuffer();
}

CSqlLogger::~CSqlLogger()
{
	CloseHandle(m_mutex);
}

// for postgresql
int CSqlLogger::Save(const TCHAR *user, const TCHAR *db, const TCHAR *host, const TCHAR *port, 
	const TCHAR *sql, TCHAR *msg_buf)
{
	DWORD dw = WaitForSingleObject(m_mutex, INFINITE);
	if(dw != WAIT_OBJECT_0) return 1;

	FILE	*fp = NULL;
	CString	log_file_name = MakeLogFileName(m_log_dir.GetBuffer(0));

	if(is_directory_exist(m_log_dir.GetBuffer(0)) == FALSE) {
		if(make_directory(m_log_dir.GetBuffer(0), msg_buf) != 0) goto ERR;
	}

	fp = OpenLogFile(log_file_name.GetBuffer(0), msg_buf);
	if(fp == NULL) goto ERR;

	{
		CString time = GetTime();

		// ファイルの最後に移動
		fseek(fp, 0, SEEK_END);

		_ftprintf(fp, _T("--%s@%s:%s.%s %s\n"),
			user, host, port, db, time.GetBuffer(0));
		_ftprintf(fp, _T("%s;\n--/\n\n"), sql);
	}

	fclose(fp);

	ReleaseMutex(m_mutex);

	return 0;

ERR:
	ReleaseMutex(m_mutex);

	return 1;
}
