#include "localdef.h"
#include "pgapi.h"
#include "pgmsg.h"

#define DLL_MAIN
#include "apidef.h"

#ifdef LOAD_LIBLARY_MODE

#include <tchar.h>
#include <windows.h>
#include <wchar.h>

HINSTANCE	pg_dll = NULL;

int pg_init_library(TCHAR *msg_buf)
{
	pg_dll = LoadLibrary(_T("LIBPQ.DLL"));
	if(pg_dll == NULL) {
		if(msg_buf != NULL) _stprintf(msg_buf, PGERR_LOAD_LIBPQ_MSG);
		return PGERR_LOAD_LIBPQ;
	}

	fp_pqconnectdb = (FP_PQconnectdb)GetProcAddress(pg_dll, "PQconnectdb");
	if(fp_pqconnectdb == NULL) goto ERR1;

	fp_pqconnectStart = (FP_PQconnectdb)GetProcAddress(pg_dll, "PQconnectStart");
	if(fp_pqconnectStart == NULL) goto ERR1;

	fp_pqstatus = (FP_PQstatus)GetProcAddress(pg_dll, "PQstatus");
	if(fp_pqstatus == NULL) goto ERR1;
	fp_pqtransactionstatus = (FP_PQstatus)GetProcAddress(pg_dll, "PQtransactionStatus");
	if(fp_pqtransactionstatus == NULL) goto ERR1;
	fp_pqfinish = (FP_PQfinish)GetProcAddress(pg_dll, "PQfinish");
	if(fp_pqfinish == NULL) goto ERR1;

	fp_pqserverversion = (FP_PQserverVersion)GetProcAddress(pg_dll, "PQserverVersion");

	fp_pqdb = (FP_PQdb)GetProcAddress(pg_dll, "PQdb");
	if(fp_pqdb == NULL) goto ERR1;
	fp_pquser = (FP_PQuser)GetProcAddress(pg_dll, "PQuser");
	if(fp_pquser == NULL) goto ERR1;
	fp_pqpass = (FP_PQpass)GetProcAddress(pg_dll, "PQpass");
	if(fp_pqpass == NULL) goto ERR1;
	fp_pqhost = (FP_PQhost)GetProcAddress(pg_dll, "PQhost");
	if(fp_pqhost == NULL) goto ERR1;
	fp_pqport = (FP_PQport)GetProcAddress(pg_dll, "PQport");
	if(fp_pqport == NULL) goto ERR1;

	fp_pqsetNoticeProcessor = (FP_PQsetNoticeProcessor)GetProcAddress(pg_dll, "PQsetNoticeProcessor");
	if(fp_pqsetNoticeProcessor == NULL) goto ERR1;

	fp_pqsendQuery = (FP_PQsendQuery)GetProcAddress(pg_dll, "PQsendQuery");
	if(fp_pqsendQuery == NULL) goto ERR1;
	fp_pqgetResult = (FP_PQgetResult)GetProcAddress(pg_dll, "PQgetResult");
	if(fp_pqgetResult == NULL) goto ERR1;
	fp_pqconsumeInput = (FP_PQconsumeInput)GetProcAddress(pg_dll, "PQconsumeInput");
	if(fp_pqconsumeInput == NULL) goto ERR1;

	fp_pqnotifies = (FP_PQnotifies)GetProcAddress(pg_dll, "PQnotifies");
	if(fp_pqnotifies == NULL) goto ERR1;
	fp_pqfreeNotify = (FP_PQfreeNotify)GetProcAddress(pg_dll, "PQfreeNotify");
	if(fp_pqfreeNotify == NULL) goto ERR1;

	fp_pqresultStatus = (FP_PQresultStatus)GetProcAddress(pg_dll, "PQresultStatus");
	if(fp_pqresultStatus == NULL) goto ERR1;
	fp_pqclear = (FP_PQclear)GetProcAddress(pg_dll, "PQclear");
	if(fp_pqclear == NULL) goto ERR1;

	fp_pqrequestCancel = (FP_PQrequestCancel)GetProcAddress(pg_dll, "PQrequestCancel");
	if(fp_pqrequestCancel == NULL) goto ERR1;
	fp_pqisBusy = (FP_PQisBusy)GetProcAddress(pg_dll, "PQisBusy");
	if(fp_pqisBusy == NULL) goto ERR1;

	fp_pqcmdStatus = (FP_PQcmdStatus)GetProcAddress(pg_dll, "PQcmdStatus");
	if(fp_pqcmdStatus == NULL) goto ERR1;
	fp_pqcmdTuples = (FP_PQcmdTuples)GetProcAddress(pg_dll, "PQcmdTuples");
	if(fp_pqcmdTuples == NULL) goto ERR1;

	fp_pqntuples = (FP_PQntuples)GetProcAddress(pg_dll, "PQntuples");
	if(fp_pqntuples == NULL) goto ERR1;
	fp_pqnfields = (FP_PQnfields)GetProcAddress(pg_dll, "PQnfields");
	if(fp_pqnfields == NULL) goto ERR1;
	fp_pqgetvalue = (FP_PQgetvalue)GetProcAddress(pg_dll, "PQgetvalue");
	if(fp_pqgetvalue == NULL) goto ERR1;
	fp_pqgetlength = (FP_PQgetlength)GetProcAddress(pg_dll, "PQgetlength");
	if(fp_pqgetlength == NULL) goto ERR1;

	fp_pqfnumber = (FP_PQfnumber)GetProcAddress(pg_dll, "PQfnumber");
	if(fp_pqfnumber == NULL) goto ERR1;
	fp_pqfname = (FP_PQfname)GetProcAddress(pg_dll, "PQfname");
	if(fp_pqfname == NULL) goto ERR1;
	fp_pqfsize = (FP_PQfsize)GetProcAddress(pg_dll, "PQfsize");
	if(fp_pqfsize == NULL) goto ERR1;
	fp_pqftype = (FP_PQfsize)GetProcAddress(pg_dll, "PQftype");
	if(fp_pqftype == NULL) goto ERR1;
	fp_pqgetisnull = (FP_PQgetisnull)GetProcAddress(pg_dll, "PQgetisnull");
	if(fp_pqgetisnull == NULL) goto ERR1;

	fp_pqerrorMessage = (FP_PQerrorMessage)GetProcAddress(pg_dll, "PQerrorMessage");
	if(fp_pqerrorMessage == NULL) goto ERR1;
	fp_pqresultErrorMessage = (FP_PQresultErrorMessage)GetProcAddress(pg_dll, "PQresultErrorMessage");
	if(fp_pqresultErrorMessage == NULL) goto ERR1;

	fp_pqsocket = (FP_PQsocket)GetProcAddress(pg_dll, "PQsocket");
	if(fp_pqsocket == NULL) goto ERR1;

	fp_pqparameterstatus = (FP_PQparameterStatus)GetProcAddress(pg_dll, "PQparameterStatus");
	if(fp_pqparameterstatus == NULL) goto ERR1;

	fp_pqcurrentNtuples = (FP_PQcurrentNtuples)GetProcAddress(pg_dll, "PQcurrentNtuples");
	fp_pqsetErrIgnoreFlg = (FP_PQsetErrIgnoreFlg)GetProcAddress(pg_dll, "PQsetErrIgnoreFlg");

	fp_pqgetssl = (FP_PQgetssl)GetProcAddress(pg_dll, "PQgetssl");

	return 0;   // OK

ERR1:
	if(pg_dll != NULL) {
		FreeLibrary(pg_dll);
		pg_dll = NULL;
	}

	if(msg_buf != NULL) _stprintf(msg_buf, PGERR_INIT_LIBPQ_MSG);
	return PGERR_INIT_LIBPQ;
}

int pg_free_library(TCHAR *msg_buf)
{
	BOOL	ret;

	if(pg_dll != NULL) {
		ret = FreeLibrary(pg_dll);
		if(ret == FALSE) {
			pg_dll = NULL;
			if(msg_buf != NULL) _stprintf(msg_buf, PGERR_FREE_LIBPQ_MSG);
			return PGERR_FREE_LIBPQ;
		}
	}

	pg_dll = NULL;

	return 0;
}

BOOL pg_library_is_ok()
{
	if(pg_dll == NULL) return FALSE;
	return TRUE;
}

#else LOAD_LIBRARY_MODE

int pg_init_library(TCHAR *msg_buf)
{
	return 0;
}

int pg_free_library(TCHAR *msg_buf)
{
	return 0;
}

BOOL pg_library_is_ok()
{
	return TRUE;
}

#endif LOAD_LIBRARY_MODE