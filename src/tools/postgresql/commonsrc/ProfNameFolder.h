/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef PROF_NAME_FOLDER_H
#define PROF_NAME_FOLDER_H

class CProfNameFolder
{
public:
	CProfNameFolder(CWinApp *papp, const TCHAR *prof_name, const TCHAR *registry_key) : 
		m_papp(papp), m_back_prof_name(NULL)
	{
		if(prof_name == NULL || _tcslen(prof_name) == 0) return;

		m_prof_name = prof_name;
		m_back_prof_name = m_papp->m_pszProfileName;
		m_papp->m_pszProfileName = m_prof_name;

		m_registry_key = registry_key;
		m_back_registry_key = m_papp->m_pszRegistryKey;
		if(m_registry_key.IsEmpty()) {
			m_papp->m_pszRegistryKey = NULL;
		} else {
			m_papp->m_pszRegistryKey = m_registry_key;
		}
	}
	~CProfNameFolder()
	{
		if(m_back_prof_name) {
			m_papp->m_pszProfileName = m_back_prof_name;
			m_papp->m_pszRegistryKey = m_back_registry_key;
		}
	}

private:
	CWinApp *m_papp;
	CString		m_prof_name;
	CString		m_registry_key;
	const TCHAR *m_back_prof_name;
	const TCHAR *m_back_registry_key;
};

#endif PROF_NAME_FOLDER_H
