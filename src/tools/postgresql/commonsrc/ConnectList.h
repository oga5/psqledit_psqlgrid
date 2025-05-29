/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef __CONNECT_INFO_H_INCLUDED__
#define __CONNECT_INFO_H_INCLUDED__

enum connect_info_list {
	LIST_USER,
	LIST_PASSWD,
	LIST_DBNAME,
	LIST_HOST,
	LIST_PORT,
	LIST_OPTION,
	LIST_CONNECT_NAME,
};

class CConnectInfo
{
public:
	CConnectInfo(TCHAR *user, TCHAR *passwd, TCHAR *dbname, TCHAR *host, TCHAR *port, TCHAR *option,
		TCHAR *connect_name);

	CString		m_user;
	CString		m_passwd;
	CString		m_dbname;
	CString		m_host;
	CString		m_port;
	CString		m_option;
	CString		m_connect_name;
};

class CConnectList
{
public:
	CConnectList();
	CConnectList(const TCHAR *opt_prof_name, const TCHAR *opt_registry_key);
	~CConnectList();

	BOOL load_list();
	BOOL save_list();
	BOOL add_list(TCHAR *user, TCHAR *passwd, TCHAR *dbname, TCHAR *host, TCHAR *port, TCHAR *option,
		TCHAR *connect_name);
	CPtrList	*GetList() { return &m_connect_list; }

private:
	CPtrList	m_connect_list;
	CString		m_opt_prof_name;
	CString		m_opt_registry_key;

	void clear_list();
};

#endif __CONNECT_INFO_H_INCLUDED__B