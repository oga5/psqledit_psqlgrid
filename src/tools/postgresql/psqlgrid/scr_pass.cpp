/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"
#include <stdio.h>

static char *scr_data[10] = {
	{"3uqSv9iNBJIj+y8M0VE2omKLH4nzPphc5d7ADgsUWk/xw6tGClQeabfOXYrZFTR1"},
	{"iLW9g0lwKQ34/SPzdYj6Zonyu2eq1pABICN5mJT8MFEsRx+hUtaGvrDHcXfkO7Vb"},
	{"1lIcJYGgE+a4oSsv0e8yf6N5DndQuFxwX/CkMOTzK7i2qB9At3mLjhVbHpRrPWZU"},
	{"MqTyOYdfsCHx+SlKW7hDnJ4zbZup1mR5tEFGwr9L6v8A2PeI/0QUgVjX3kcNiaBo"},
	{"ItBn9/VhKoFx3ZLG7Ss8AwjXkDTf6q5UbrPdWlmRYO42CgM1Ju0pQEHvzNya+cie"},
	{"1aDOEiU0YTPrJGcFRNMWLjl/ZtC+ewHIBAVg3KXq4dyQpsSb72vn5fzoxuhkm698"},
	{"9lu7MvHWehi+SwJ1KQ8DOGj4npxANarF32qToIbLd6f/VZPyBszYgtXkmCcU0R5E"},
	{"Tj0zyn7M8eUHLhpuPxOlsF94r+Bqt2AR5/idabQD3kZSYNKg1IvJVfwCXmEWoc6G"},
	{"9PjkHXCh2ZI1fS+RzKmDTeu0wv3FUJnMGBd8qsa6c5QtNgVEbrOpWyoLli4YA7x/"},
	{"5saogw80WTz3SJnPxjAL2/K6GMb1rdIXVkptYHE7B9+yfFNmDe4chQvURuilCOqZ"},
};

static int scr_data_len = 27+27+10+2;

static const char *make_encode_data(const char *src, char *data)
{
	data[0] = '\0';
	data[1] = '\0';
	data[2] = '\0';
	data[3] = '\0';

	data[0] = src[0] >> 2;
	if(src[1] == '\0') {
		data[1] = ((src[0] & 0x03) << 4);
		return src + 1;
	}
	data[1] = ((src[0] & 0x03) << 4) + (src[1] >> 4) ;
	if(src[2] == '\0') {
		data[2] = ((src[1] & 0x0f) << 2);
		return src + 2;
	}
	data[2] = ((src[1] & 0x0f) << 2) + (src[2] >> 6) ;
	data[3] = (src[2] & 0x3f);

	return src + 3;
}

static int find_scr_data(int ch, int solt)
{
	if(solt > sizeof(scr_data)/sizeof(scr_data[0])) return 0;

	int		i;
	for(i = 0; i < scr_data_len; i++) {
		if(scr_data[solt][i] == ch) {
			return i;
		}
	}

	ASSERT(0);
	return 0;	// error
}

static const char *make_decode_data(const char *src, char *data,
	int solt)
{
	int		cd;
	data[0] = '\0';
	data[1] = '\0';
	data[2] = '\0';

	cd = find_scr_data(src[0], solt);
	data[0] += cd << 2;

	if(src[1] == '\0') return src + 1;
	cd = find_scr_data(src[1], solt);
	data[0] += cd >> 4;
	data[1] += cd << 4;

	if(src[2] == '\0') return src + 2;
	cd = find_scr_data(src[2], solt);
	data[1] += cd >> 2;
	data[2] += cd << 6;

	if(src[3] == '\0') return src + 3;
	cd = find_scr_data(src[3], solt);
	data[2] += cd;

	return src + 4;
}

static int scr_encode_main(const char *src, char *dst, int solt)
{
	char	b[4];
	int				i;

	for(; *src != '\0';) {
		src = make_encode_data(src, b);
		for(i = 0; i < 4; i++) {
			*dst = scr_data[solt][b[i]];
			dst++;
		}
	}
	*dst = '\0';

	return 0;
}

static int scr_decode_main(const char *src, char *dst, int solt)
{
	char	b[3];
	int		i;

	for(; *src != '\0';) {
		src = make_decode_data(src, b, solt);
		for(i = 0; i < 3; i++) {
			*dst = b[i];
			dst++;
		}
	}
	*dst = '\0';

	return 0;
}

int scr_encode(const TCHAR *src, TCHAR *dst, int solt)
{
	char c_src[256];
	char c_dst[256];

	WideCharToMultiByte(CP_ACP, 0, src, -1, c_src, 256, NULL, NULL);
	scr_encode_main(c_src, c_dst, solt);
	MultiByteToWideChar(CP_ACP, 0, c_dst, -1, dst, 256);

	return 0;
}

int scr_decode(const TCHAR *src, TCHAR *dst, int solt)
{
	char c_src[256];
	char c_dst[256];

	WideCharToMultiByte(CP_ACP, 0, src, -1, c_src, 256, NULL, NULL);
	scr_decode_main(c_src, c_dst, solt);
	MultiByteToWideChar(CP_ACP, 0, c_dst, -1, dst, 256);

	return 0;
}



