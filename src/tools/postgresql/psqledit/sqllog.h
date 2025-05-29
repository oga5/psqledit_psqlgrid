/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#if !defined(_SQL_LOG_INCLUDED_)
#define _SQL_LOG_INCLUDED_

#include <time.h>
#include <stdio.h>

class CSqlLog
{
private:
	int			m_number;
	CString		m_sql;
	CString		m_time;

public:
	CSqlLog(int number, const TCHAR *sql);

	int GetNumber() { return m_number; }
	CString GetSql() { return m_sql; }
	CString GetTime() { return m_time; }
};

class CSqlLogArray
{
private:
	int			m_next_number;
	int			m_max_log_cnt;
	CPtrArray	*m_sql_log_array;

public:
	CSqlLogArray(int max_log_cnt);
	~CSqlLogArray();

	int Add(const TCHAR *sql);
	INT_PTR GetSize() { return m_sql_log_array->GetSize(); }
	CSqlLog *GetAt(int index) { return (CSqlLog *)m_sql_log_array->GetAt(index); }
};

class CSqlLogger
{
private:
	HANDLE	m_mutex;
	CString	m_log_dir;

	int CreateNewLogFile(const TCHAR *log_file_name, TCHAR *msg_buf);
	FILE *OpenLogFile(const TCHAR *log_file_name, TCHAR *msg_buf);
	int SaveSql(FILE *fp, const TCHAR *user, const TCHAR *tns, const TCHAR *sql);
	CString MakeLogFileName(const TCHAR *log_dir_name);

public:
	CSqlLogger(const TCHAR *log_dir_name);
	~CSqlLogger();

	static CString GetTime();
	static CString GetDate(int format = 0);

	void SetLogDir(const TCHAR *log_dir_name);
	int Save(const TCHAR *user, const TCHAR *tns, const TCHAR *sql, TCHAR *msg_buf);
	int Save(const TCHAR *user, const TCHAR *db, const TCHAR *host, const TCHAR *port,
		const TCHAR *sql, TCHAR *msg_buf);
};

#endif _SQL_LOG_INCLUDED_
