/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include "ConnectList.h"
#include "ProfNameFolder.h"

CConnectInfo::CConnectInfo(TCHAR *user, TCHAR *passwd, TCHAR *dbname, TCHAR *host, TCHAR *port, TCHAR *option,
	TCHAR *connect_name)
{
	m_user = user;
	m_passwd = passwd;
	m_dbname = dbname;
	m_host = host;
	m_port = port;
	m_option = option;
	m_connect_name = connect_name;
}

CConnectList::CConnectList() :
	m_opt_prof_name(_T("")), m_opt_registry_key(_T(""))
{
}

CConnectList::CConnectList(const TCHAR *opt_prof_name, const TCHAR *opt_registry_key) :
	m_opt_prof_name(opt_prof_name), m_opt_registry_key(opt_registry_key)
{
}

CConnectList::~CConnectList()
{
	clear_list();
}

void CConnectList::clear_list() {
	while(m_connect_list.IsEmpty() == FALSE) {
		delete (CConnectInfo*)m_connect_list.GetHead();
		m_connect_list.RemoveHead();
	}
}

BOOL CConnectList::add_list(TCHAR *user, TCHAR *passwd, TCHAR *dbname, TCHAR *host, TCHAR *port, TCHAR *option,
	TCHAR *connect_name)
{
	CConnectInfo *p_info = new CConnectInfo(user, passwd, dbname, host, port, option, connect_name);
	m_connect_list.AddTail(p_info);

	return TRUE;
}

static void make_entry(TCHAR *user_entry, TCHAR *passwd_entry, TCHAR *dbname_entry,
	TCHAR *host_entry, TCHAR *port_entry, TCHAR *option_entry, TCHAR *connect_name_entry,
	int i)
{
	_stprintf(user_entry, _T("USER-%d"), i);
	_stprintf(passwd_entry, _T("PASSWD-%d"), i);
	_stprintf(dbname_entry, _T("DBNAME-%d"), i);
	_stprintf(host_entry, _T("HOST-%d"), i);
	_stprintf(port_entry, _T("PORT-%d"), i);
	_stprintf(option_entry, _T("OPTION-%d"), i);
	_stprintf(connect_name_entry, _T("CONNECT-NAME-%d"), i);
}

BOOL CConnectList::load_list()
{
	TCHAR		user_entry[20];
	TCHAR		passwd_entry[20];
	TCHAR		dbname_entry[20];
	TCHAR		host_entry[20];
	TCHAR		port_entry[20];
	TCHAR		option_entry[20];
	TCHAR		connect_name_entry[20];
	CString		user;
	CString		passwd;
	CString		dbname;
	CString		host;
	CString		port;
	CString		option;
	CString		connect_name;
	int			i, cnt;

	clear_list();

	CProfNameFolder		prof_name_folder(AfxGetApp(), m_opt_prof_name, m_opt_registry_key);

	cnt = AfxGetApp()->GetProfileInt(_T("CONNECT_INFO"), _T("COUNT"), 0);
	for(i = 0; i < cnt; i++) {
		make_entry(user_entry, passwd_entry, dbname_entry, host_entry, port_entry, option_entry,
			connect_name_entry, i);

		user = AfxGetApp()->GetProfileString(_T("CONNECT_INFO"), user_entry, _T(""));
		passwd = AfxGetApp()->GetProfileString(_T("CONNECT_INFO"), passwd_entry, _T(""));
		dbname = AfxGetApp()->GetProfileString(_T("CONNECT_INFO"), dbname_entry, _T(""));
		host = AfxGetApp()->GetProfileString(_T("CONNECT_INFO"), host_entry, _T(""));
		port = AfxGetApp()->GetProfileString(_T("CONNECT_INFO"), port_entry, _T(""));
		option = AfxGetApp()->GetProfileString(_T("CONNECT_INFO"), option_entry, _T(""));
		connect_name = AfxGetApp()->GetProfileString(_T("CONNECT_INFO"), connect_name_entry, _T(""));

		add_list(user.GetBuffer(0), passwd.GetBuffer(0), dbname.GetBuffer(0),
			host.GetBuffer(0), port.GetBuffer(0), option.GetBuffer(0),
			connect_name.GetBuffer(0));
	}

	return TRUE;
}

BOOL CConnectList::save_list()
{
	TCHAR		user_entry[20];
	TCHAR		passwd_entry[20];
	TCHAR		dbname_entry[20];
	TCHAR		host_entry[20];
	TCHAR		port_entry[20];
	TCHAR		option_entry[20];
	TCHAR		connect_name_entry[20];
	int			i;
	CConnectInfo *p_info;
	POSITION	pos;

	CProfNameFolder		prof_name_folder(AfxGetApp(), m_opt_prof_name, m_opt_registry_key);

	for(i = 0, pos = m_connect_list.GetHeadPosition(); pos != NULL; i++, m_connect_list.GetNext(pos)) {
		make_entry(user_entry, passwd_entry, dbname_entry, host_entry, port_entry, option_entry, 
			connect_name_entry, i);
		
		p_info = (CConnectInfo *)m_connect_list.GetAt(pos);

		AfxGetApp()->WriteProfileString(_T("CONNECT_INFO"), user_entry, p_info->m_user);
		AfxGetApp()->WriteProfileString(_T("CONNECT_INFO"), passwd_entry, p_info->m_passwd);
		AfxGetApp()->WriteProfileString(_T("CONNECT_INFO"), dbname_entry, p_info->m_dbname);
		AfxGetApp()->WriteProfileString(_T("CONNECT_INFO"), host_entry, p_info->m_host);
		AfxGetApp()->WriteProfileString(_T("CONNECT_INFO"), port_entry, p_info->m_port);
		AfxGetApp()->WriteProfileString(_T("CONNECT_INFO"), option_entry, p_info->m_option);
		AfxGetApp()->WriteProfileString(_T("CONNECT_INFO"), connect_name_entry, p_info->m_connect_name);
	}
	AfxGetApp()->WriteProfileInt(_T("CONNECT_INFO"), _T("COUNT"), (int)(m_connect_list.GetCount()));
	AfxGetApp()->WriteProfileInt(_T("CONNECT_INFO"), _T("ENCODE_VER"), 2);

	return TRUE;
}
