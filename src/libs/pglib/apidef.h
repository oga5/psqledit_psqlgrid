/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#define LOAD_LIBLARY_MODE

#ifdef LOAD_LIBLARY_MODE

typedef PGconn *(*FP_PQconnectdb)(const char *connect_info);
typedef PGconn *(*FP_PQconnectStart)(const char *conninfo);
typedef void (*FP_PQfinish)(PGconn *conn);
typedef ConnStatusType (*FP_PQstatus)(const PGconn *conn);
typedef PGTransactionStatusType (*FP_PQtransactionStatus)(const PGconn *conn);

typedef int (*FP_PQserverVersion)(const PGconn *conn);

typedef char *(*FP_PQdb)(const PGconn *conn);
typedef char *(*FP_PQuser)(const PGconn *conn);
typedef char *(*FP_PQpass)(const PGconn *conn);
typedef char *(*FP_PQhost)(const PGconn *conn);
typedef char *(*FP_PQport)(const PGconn *conn);

typedef PQnoticeProcessor (*FP_PQsetNoticeProcessor)(PGconn *conn, 
	PQnoticeProcessor proc, void *arg);

typedef int (*FP_PQsendQuery)(PGconn *conn, const char *query);
typedef PGresult *(*FP_PQgetResult)(PGconn *conn);
typedef int (*FP_PQconsumeInput)(PGconn *conn);

typedef PGnotify *(*FP_PQnotifies)(PGconn *conn);
typedef void (*FP_PQfreeNotify)(PGnotify *notify);

typedef ExecStatusType (*FP_PQresultStatus)(const PGresult *res);
typedef void (*FP_PQclear)(PGresult *res);
typedef int (*FP_PQrequestCancel)(PGconn *conn);
typedef int (*FP_PQisBusy)(PGconn *conn);
typedef int (*FP_PQsocket)(PGconn *conn);

typedef char *(*FP_PQcmdStatus)(PGresult *res);
typedef char *(*FP_PQcmdTuples)(PGresult *res);

typedef int (*FP_PQntuples)(const PGresult *res);
typedef int (*FP_PQnfields)(const PGresult *res);
typedef char *(*FP_PQgetvalue)(const PGresult *res, int tup_num, int field_num);
typedef int (*FP_PQgetlength)(const PGresult *res, int tup_num, int field_num);
typedef int (*FP_PQfnumber)(const PGresult *res, const char *field_name);
typedef char *(*FP_PQfname)(const PGresult *res, int field_index);
typedef int (*FP_PQfsize)(const PGresult *res, int field_index);
typedef int (*FP_PQgetisnull)(const PGresult *res, int tup_num, int field_num);
typedef Oid (*FP_PQftype)(const PGresult *res, int field_index);

typedef char *(*FP_PQerrorMessage)(const PGconn *conn);
typedef char *(*FP_PQresultErrorMessage)(const PGresult *res);

typedef int (*FP_PQcurrentNtuples)(const PGconn *conn);
typedef void (*FP_PQsetErrIgnoreFlg)(const PGconn *conn, char flg);

typedef void *(*FP_PQgetssl)(PGconn *conn);

typedef const char *(*FP_PQparameterStatus)(const PGconn *conn, const char *paramName);

#ifdef DLL_MAIN
	FP_PQconnectdb	fp_pqconnectdb = NULL;
	FP_PQconnectStart	fp_pqconnectStart = NULL;
	FP_PQfinish		fp_pqfinish = NULL;
	FP_PQstatus		fp_pqstatus = NULL;
	FP_PQtransactionStatus	fp_pqtransactionstatus = NULL;
	FP_PQserverVersion	fp_pqserverversion = NULL;
	FP_PQdb			fp_pqdb = NULL;
	FP_PQuser		fp_pquser = NULL;
	FP_PQpass		fp_pqpass = NULL;
	FP_PQhost		fp_pqhost = NULL;
	FP_PQport		fp_pqport = NULL;
	FP_PQsetNoticeProcessor	fp_pqsetNoticeProcessor = NULL;
	FP_PQsendQuery	fp_pqsendQuery = NULL;
	FP_PQgetResult	fp_pqgetResult = NULL;
	FP_PQconsumeInput	fp_pqconsumeInput = NULL;
	FP_PQresultStatus	fp_pqresultStatus = NULL;
	FP_PQclear		fp_pqclear = NULL;
	FP_PQrequestCancel	fp_pqrequestCancel = NULL;
	FP_PQisBusy		fp_pqisBusy = NULL;
	FP_PQcmdStatus	fp_pqcmdStatus = NULL;
	FP_PQcmdTuples	fp_pqcmdTuples = NULL;
	FP_PQntuples	fp_pqntuples = NULL;
	FP_PQnfields	fp_pqnfields = NULL;
	FP_PQgetvalue	fp_pqgetvalue = NULL;
	FP_PQgetlength	fp_pqgetlength = NULL;
	FP_PQfnumber	fp_pqfnumber = NULL;
	FP_PQfname		fp_pqfname = NULL;
	FP_PQfsize		fp_pqfsize = NULL;
	FP_PQgetisnull	fp_pqgetisnull = NULL;
	FP_PQerrorMessage	fp_pqerrorMessage = NULL;
	FP_PQresultErrorMessage	fp_pqresultErrorMessage = NULL;
	FP_PQsocket		fp_pqsocket = NULL;
	FP_PQftype		fp_pqftype = NULL;
	FP_PQnotifies	fp_pqnotifies = NULL;
	FP_PQfreeNotify	fp_pqfreeNotify = NULL;
	FP_PQcurrentNtuples	fp_pqcurrentNtuples = NULL;
	FP_PQsetErrIgnoreFlg	fp_pqsetErrIgnoreFlg = NULL;
	FP_PQgetssl		fp_pqgetssl = NULL;
	FP_PQparameterStatus		fp_pqparameterstatus = NULL;
#else DLL_MAIN
	extern FP_PQconnectdb	fp_pqconnectdb;
	extern FP_PQconnectStart	fp_pqconnectStart;
	extern FP_PQfinish		fp_pqfinish;
	extern FP_PQstatus		fp_pqstatus;
	extern FP_PQtransactionStatus	fp_pqtransactionstatus;
	extern FP_PQserverVersion	fp_pqserverversion;
	extern FP_PQdb			fp_pqdb;
	extern FP_PQuser		fp_pquser;
	extern FP_PQpass		fp_pqpass;
	extern FP_PQhost		fp_pqhost;
	extern FP_PQport		fp_pqport;
	extern FP_PQsetNoticeProcessor	fp_pqsetNoticeProcessor;
	extern FP_PQsendQuery	fp_pqsendQuery;
	extern FP_PQgetResult	fp_pqgetResult;
	extern FP_PQconsumeInput	fp_pqconsumeInput;
	extern FP_PQresultStatus	fp_pqresultStatus;
	extern FP_PQclear		fp_pqclear;
	extern FP_PQrequestCancel	fp_pqrequestCancel;
	extern FP_PQisBusy		fp_pqisBusy;
	extern FP_PQcmdStatus	fp_pqcmdStatus;
	extern FP_PQcmdTuples	fp_pqcmdTuples;
	extern FP_PQntuples		fp_pqntuples;
	extern FP_PQnfields		fp_pqnfields;
	extern FP_PQgetvalue	fp_pqgetvalue;
	extern FP_PQgetlength	fp_pqgetlength;
	extern FP_PQfnumber		fp_pqfnumber;
	extern FP_PQfname		fp_pqfname;
	extern FP_PQfsize		fp_pqfsize;
	extern FP_PQgetisnull	fp_pqgetisnull;
	extern FP_PQerrorMessage	fp_pqerrorMessage;
	extern FP_PQresultErrorMessage	fp_pqresultErrorMessage;
	extern FP_PQsocket		fp_pqsocket;
	extern FP_PQftype		fp_pqftype;
	extern FP_PQnotifies	fp_pqnotifies;
	extern FP_PQfreeNotify	fp_pqfreeNotify;
	extern FP_PQcurrentNtuples	fp_pqcurrentNtuples;
	extern FP_PQsetErrIgnoreFlg	fp_pqsetErrIgnoreFlg;
	extern FP_PQgetssl		fp_pqgetssl;
	extern FP_PQparameterStatus		fp_pqparameterstatus;
#endif DLL_MAIN

#else LOAD_LIBLARY_MODE

#define fp_pqconnectdb(c)		PQconnectdb(c)
#define fp_pqfinish(c)			PQfinish(c)
#define fp_pqstatus(c)			PQstatus(c)
#define fp_pqtransactionstatus(c)			PQtransactionStatus(c)
#define fp_pqdb(c)				PQdb(c)
#define fp_pquser(c)			PQuser(c)
#define fp_pqpass(c)			PQpass(c)
#define fp_pqhost(c)			PQhost(c)
#define fp_pqport(c)			PQport(c)
#define fp_pqsetNoticeProcessor(c, p, a)	PQsetNoticeProcessor(c, p, a)
#define fp_pqsendQuery(c, s)		PQsendQuery(c, s)
#define fp_pqgetResult(c)		PQgetResult(c)
#define fp_pqconsumeInput(c)	PQconsumeInput(c)
#define fp_pqresultStatus(c)	PQresultStatus(c)
#define fp_pqclear(c)			PQclear(c)
#define fp_pqrequestCancel(c)	PQrequestCancel(c)
#define fp_pqisBusy(c)			PQisBusy(c)
#define fp_pqcmdStatus(c)		PQcmdStatus(c)
#define fp_pqcmdTuples(c)		PQcmdTuples(c)
#define fp_pqntuples(c)			PQntuples(c)
#define fp_pqnfields(c)			PQnfields(c)
#define fp_pqgetvalue(res, r, c)		PQgetvalue(res, r, c)
#define fp_pqgetlength(res, r, c)		PQgetlength(res, r, c)
#define fp_pqfnumber(res, c)	PQfnumber(res, c)
#define fp_pqfname(res, c)		PQfname(res, c)
#define fp_pqfsize(res, c)		PQfsize(res, c)
#define fp_pqftype(res, c)		PQftype(res, c)
#define fp_pqgetisnull(res, r, c)		PQgetisnull(res, r, c)
#define fp_pqerrorMessage(c)	PQerrorMessage(c)
#define fp_pqresultErrorMessage(c)	PQresultErrorMessage(c)
#define fp_pqsocket(c)			PQsocket(c)
#define fp_pqnotifies(c)		PQnotifies(c)
#define fp_pqfreeNotify(n)		PQfreeNotify(n)
#define fp_pqcurrentNtuples(c)	PQcurrentNtuples(c)
#define fp_pqsetErrIgnoreFlg(c, f)	PQsetErrIgnoreFlg(c, f)
#define fp_pqgetssl(c)			PQgetssl(c)
#define fp_pqparameterstatus(c, p) PQParameterStatus(c, p)

#endif LOAD_LIBLARY_MODE
