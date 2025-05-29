#include <stdio.h>
#include <stdlib.h>

#include "localdef.h"
#include "pgapi.h"
#include "pgmsg.h"

static int csv_fputs(FILE *stream, const TCHAR *string, TCHAR sepa)
{
	if(string == NULL) return EOF;

	if(putwc('\"', stream) == EOF) return EOF;
	for(;*string != '\0'; string++) {
		if(putwc(*string, stream) == EOF) return EOF;
		if(*string == '\"') {
			if(putwc('\"', stream) == EOF) return EOF;
		}
	}
	if(putwc('\"', stream) == EOF) return EOF;

	putwc(sepa, stream);

	return 1;
}

static int pg_save_dataset_main_fp(FILE *fp, HPgDataset dataset, int put_colname, TCHAR *msg_buf,
	TCHAR sepa)
{
	int		r, c;

	if(put_colname == 1) {
		for(c = 0; c < dataset->col_cnt; c++) {
			csv_fputs(fp, pg_dataset_get_colname(dataset, c), sepa);
		}
		_fputts(_T("\n"), fp);
	}

	for(r = 0; r < dataset->row_cnt; r++) {
		for(c = 0; c < dataset->col_cnt; c++) {
			csv_fputs(fp, pg_dataset_data(dataset, r, c), sepa);
		}
		_fputts(_T("\n"), fp);
	}

	return 0;
}

static int pg_save_dataset_main(const TCHAR *path, HPgDataset dataset, int put_colname, TCHAR *msg_buf,
	TCHAR sepa)
{
	FILE	*fp;
	int		ret_v;

	if ( (fp = _tfopen(path, _T("w"))) == NULL ) {
		if(msg_buf != NULL) _stprintf(msg_buf, PGERR_OPEN_FILE_MSG, path);
		return PGERR_OPEN_FILE;
	}

	ret_v = pg_save_dataset_main_fp(fp, dataset, put_colname, msg_buf, sepa);
	if(ret_v != 0) {
		fclose(fp);
		return ret_v;
	}

	if ( fclose(fp) ) {
		if(msg_buf != NULL) _tcscpy(msg_buf, PGERR_CLOSE_FILE_MSG);
		return PGERR_CLOSE_FILE;
	}

	return 0;
}

int pg_save_dataset_csv(const TCHAR *path, HPgDataset dataset, int put_colname, TCHAR *msg_buf)
{
	return pg_save_dataset_main(path, dataset, put_colname, msg_buf, ',');
}

int pg_save_dataset_tsv(const TCHAR *path, HPgDataset dataset, int put_colname, TCHAR *msg_buf)
{
	return pg_save_dataset_main(path, dataset, put_colname, msg_buf, '\t');
}

int pg_save_dataset_csv_fp(FILE *fp, HPgDataset dataset, int put_colname, TCHAR *msg_buf)
{
	return pg_save_dataset_main_fp(fp, dataset, put_colname, msg_buf, ',');
}

int pg_save_dataset_ex(const TCHAR *path, HPgDataset dataset, int put_colname, TCHAR *msg_buf,
	int col_cnt, TCHAR sepa)
{
	int	ret_v;
	int org_col_cnt = dataset->col_cnt;
	dataset->col_cnt = col_cnt;
	ret_v = pg_save_dataset_main(path, dataset, put_colname, msg_buf, sepa);
	dataset->col_cnt = org_col_cnt;
	return ret_v;
}

