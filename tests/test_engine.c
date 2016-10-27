/*********************************************************************
This file is a part of FItsSec project: Implementation of ETSI TS 103 097
Copyright (C) 2015  Denis Filatov (danya.filatov()gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed under GNU GPLv3 in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/gpl-3.0.txt>.
@license GPL-3.0+ <http://www.gnu.org/licenses/gpl-3.0.txt>

In particular cases this program can be distributed under other license
by simple request to the author. 
*********************************************************************/
#define _CRT_SECURE_NO_WARNINGS

#include "copts.h"
#include "cstr.h"
#include "fitsec.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

static FitSecConfig cfg1, cfg2;

static const pchar_t* cfgfile = NULL;

static FSLocation position = { 514743600, 56248900 };
static unsigned int _curTime = 0;
static unsigned int _beginTime = 0;

FILE * out;

const char * outpath = "msg.log";

static copt_t options [] = {
    { "h?", "help",     COPT_HELP,     NULL,              "Print this help page"},
    { "C",  "config",   COPT_CFGFILE,  &cfgfile,          "Config file"         },
	{ "s", "storage",   COPT_STR,      &cfg1.storage,     "Storage directory"   },
	{ "x", "hex",       COPT_BOOL,     &cfg1.hexadecimal, "Certificates are stored as hexadecimal stream" },
	{ "o", "out",       COPT_STR,      &outpath,          "Output path" },
	{ NULL, NULL, COPT_END, NULL, NULL }
};

static const char * _signer_types[] = {
	"self",
	"digest",
	"certificate",
	"chain",
	"other",
};


static int SendMsg(const char * tn, FitSec * e, char * buf, FSMessageInfo * info);
static int RecvMsg(const char * tn, FitSec * e, char * buf, int len, FSMessageInfo * info);

static void print_x(FILE * f, const char * const ptr, int len);

static const unsigned long _leap_moments[] = {
	1136073600,
	1230768000,
	1341100800,
	1435708800,
};

static time_t addleapseconds(time_t t)
{
	int i;
	for (i = 0; i < sizeof(_leap_moments) / sizeof(_leap_moments[0]); i++){
		if (t < _leap_moments[i]) break;
	}
	return t + i;
}

#define ITS_UTC_EPOCH 1072915200

static unsigned long unix2itstime32(time_t t)
{
	return ((unsigned long) addleapseconds(t)) - ITS_UTC_EPOCH;
}

static int loadCertificates(FitSec * e, const pchar_t * _path, int hexadecimal);

int main(int argc, char** argv)
{
	FitSec * es, *er;
	char buf[1024];
	int i, len;
	const char * testName;
	FSMessageInfo ms = { 0 }, mr = { 0 };
	
	FitSecConfig_InitDefault(&cfg1);
	cfg1.storage = "CERTS1";
	cfg1.hexadecimal = 1;

	int flags = COPT_DEFAULT | COPT_NOERR_UNKNOWN | COPT_NOAUTOHELP;
	argc = coptions(argc, argv, flags, options);
	if (COPT_ERC(argc)){
		coptions_help(stdout, argv[0], 0, options, "Test");
		return -1;
	}
	memcpy(&cfg2, &cfg1, sizeof(cfg1));
	cfg2.storage = "CERTS2";

	_curTime = unix2itstime32(time(NULL));

	if (outpath){
		out = fopen(outpath, "w");
		if (out == NULL){
			perror(outpath);
			return -1;
		}
	}
	else{
		out = stdout;
	}

	// setup crypto alg
	cfg1.crypt = FSCryptEngineConfig_Default();
	cfg2.crypt = FSCryptEngineConfig_Default();

	es = FitSec_New(&cfg1);
	er = FitSec_New(&cfg2);
	if(0 >= loadCertificates(es, cfg1.storage, cfg1.hexadecimal)){
		return -1;
	}
	if (0 >= loadCertificates(er, cfg2.storage, cfg2.hexadecimal)){
		FitSec_Free(es);
		return -1;
	}

	ms.ssp.aid = 36;
	ms.position = position;
	ms.ssp.u.ssp.version = 1;
	ms.payloadType = FS_PAYLOAD_SIGNED;
	
	_beginTime = _curTime;

	// test 1: Send 20 CAM and read it
	
	testName = "testCAM";
	ms.generationTime = ((Time64)_beginTime) * 1000;
	for (i = 0; i < 20; i++){
		ms.generationTime += 300000;
		len = SendMsg(testName, es, buf, &ms);
		if (len > 0){
			print_x(out, buf, len);
			RecvMsg(testName, er, buf, len, &mr);
		}
	}
	

	// test 2: Send 10 DENM and read it
	
	testName = "testDENM";
	ms.generationTime = ((Time64)_beginTime) * 1000;
	ms.ssp.aid = 37;
	for (i = 0; i < 20; i++){
		ms.generationTime += 300000;
		len = SendMsg(testName, es, buf, &ms);
		if (len > 0){
			print_x(out, buf, len);
			RecvMsg(testName, er, buf, len, &mr);
		}
	}
	
	// test 3: Check unknown cert sending
	testName = "testCAM3";
	ms.generationTime = ((Time64)_beginTime) * 1000;
	ms.ssp.aid = 36;
	len = SendMsg(testName, es, buf, &ms);
	len = SendMsg(testName, es, buf, &ms);
	if (len > 0){
		print_x(out, buf, len);
		RecvMsg(testName, er, buf, len, &mr);
		len = SendMsg(testName, er, buf, &mr);
		RecvMsg(testName, es, buf, len, &ms);
		len = SendMsg(testName, es, buf, &ms);
	}

	FitSec_Free(es);
	FitSec_Free(er);

    return 0;
}


static int SendMsg(const char * tn, FitSec * e, char * buf, FSMessageInfo * info)
{
	int ret;
	ret = FitSec_PrepareMessage(e, info, buf, 1024);
	if (ret > 0){
		memcpy(&buf[ret], "0123456789", 10);
		info->payloadSize = 10;
		ret = FitSec_SignMessage(e, info, buf, 1024);
	}
	if (ret < 0){
		fprintf(stderr, "SEND %s:\t ERROR: 0x%08X %s\n", tn, info->status, FitSec_ErrorMessage(info->status));
	}
	else{
		fprintf(stderr, "SEND %s:\t OK %s\n", tn, _signer_types[info->si_type]);
	}
	return ret;
}

static int RecvMsg(const char * tn, FitSec * e, char * buf, int len, FSMessageInfo * info)
{
	int ret = FitSec_Verify(e, info, buf, len);
	if (!ret){
		fprintf(stderr, "RECV %s:\t ERROR: 0x%08X %s\n", tn, info->status, FitSec_ErrorMessage(info->status));
	}
	else{
#ifdef _MSC_VER
		fprintf(stderr, "RECV %s:\t %u ", tn, (unsigned int)(info->generationTime / 1000 - _beginTime));
		fprintf(stderr, _signer_types[info->si_type]);
		fprintf(stderr, "\n");
#else
		fprintf(stderr, "RECV %s:\t %u %s\n", 
				tn, (unsigned int)(info->generationTime / 1000 - _beginTime),
				_signer_types[info->si_type]);
#endif
	}
	return ret;
}

static void print_x(FILE * f, const char * const ptr, int len)
{
	const unsigned char * p = (const unsigned char *)ptr;
	const unsigned char * e = p + len;
	for (; p < e; p++){
		fprintf(f, "%02X", *p);
	}
}

static char * _data;
static int    _dsize = 4096;
static int _load_certificate(FitSec * e, pchar_t * path, pchar_t * fname, int hexadecimal)
{
	char *data, *end;
	char *vkey = NULL, *ekey = NULL;
	int   cert_len = 0, vkey_len = 0, ekey_len = 0;
	pchar_t *ext;
	int error = 0;

	data = _data;
	end = cstrnload(data, _dsize, path);
	if (end > data){
		if (hexadecimal){
			end = cstr_hex2bin(data, end - data, data, end - data);
		}
		cert_len = end - data;

		printf("%s:", fname);
		
		// look for keys
		ext = cstrpathextension(fname);
		pchar_cpy(ext, ".vkey");
		vkey = end;
		end = cstrnload(vkey, _dsize - (vkey - data), path);
		if (end <= vkey){
			end = vkey; vkey = NULL;
		}
		else{
			if (hexadecimal){
				end = cstr_hex2bin(vkey, end - vkey, vkey, end - vkey);
			}
			vkey_len = end - vkey;
		}

		pchar_cpy(ext, ".ekey");
		ekey = end;
		end = cstrnload(ekey, _dsize - (ekey - data), path);
		if (end <= ekey){
			end = ekey; ekey = NULL;
		}
		else{
			if (hexadecimal){
				end = cstr_hex2bin(ekey, end - ekey, ekey, end - ekey);
			}
			ekey_len = end - vkey;
		}
		FitSec_InstallCertificate(e, data, cert_len, vkey, vkey_len, ekey, ekey_len, &error);
		printf("%s\n", FitSec_ErrorMessage(error));;
	}
	else{
		error = -1;
		printf("%s: Empty File\n", fname);
	}
	return error;
}

static int loadCertificates(FitSec * e, const pchar_t * _path, int hexadecimal)
{
	size_t plen;
	pchar_t *path;
	int ccount = 0;

	plen = _path ? pchar_len(_path) : 0;
	path = malloc((plen + 256) * sizeof(pchar_t));
	if (plen){
		memcpy(path, _path, plen * sizeof(pchar_t));
		while (plen && (path[plen - 1] == '/' || path[plen - 1] == '\\')) plen--;
	}
	if (plen == 0) path[plen++] = '.';
	path[plen++] = '/';
	path[plen] = 0;

	_data = malloc(_dsize); // it must not be more

#ifdef WIN32
	{
		WIN32_FIND_DATA fd;
		HANDLE h;

		pchar_cpy(path + plen, "*.crt");

		h = FindFirstFile(path, &fd);
		if (INVALID_HANDLE_VALUE != h){
			do{
				if (!(fd.dwFileAttributes& FILE_ATTRIBUTE_DIRECTORY)) {
					pchar_cpy(path + plen, fd.cFileName);
					if (0 <= _load_certificate(e, path, path + plen, hexadecimal)){
						ccount++;
					}
				}
			} while (FindNextFile(h, &fd));
			FindClose(h);
		}
	}
#else
	{
		DIR * d;
		struct dirent * de;
		d = opendir(path);
		if(d){
			while((de = readdir(d))){
				pchar_t * ext = pchar_rchr(de->d_name, '.');
				if(ext && 0 == strcmp(ext, ".crt")){
					pchar_cpy(path + plen, de->d_name);
					if (0 <= _load_certificate(e, path, path + plen, hexadecimal)){
						ccount++;
					}
				}
			}
			closedir(d);
		}
	}
#endif
	free(path);
	free(_data);
	return ccount;
}
