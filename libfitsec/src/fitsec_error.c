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

You should have received a copy of the GNU Lesser General Public License
along with Fitsec.  If not, see <http://www.gnu.org/licenses/lgpl-3.0.txt>.
@license LGPL-3.0+ <http://www.gnu.org/licenses/lgpl-3.0.txt>

In particular cases this program can be distributed under other license
by simple request to the author.
*********************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include "fitsec_i.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "cstr.h"

static struct {
	int id;
	const char * msg;
}_fitsec_msg_actions[] = {
	{ FSERR_OK, "OK" },
	{ FSERR_PARSEERROR, "parsing error" },
	{ FSERR_NOSPACE, "buffer is full" },
	{ FSERR_INVALID, "invalid" },
	{ FSERR_UNKNOWN, "unknown" },
	{ FSERR_ORDER, "wrong order" } //FSERR_ORDER
};

static const char * _fitsec_msg_element[] = {
	NULL,
	"version",  // FSERR_VERSION
	"size",     //FSERR_SIZE
	"type",     // FSERR_TYPE
	"digest",   // FSERR_DIGEST
	"AID",      // FSERR_AID
	"pk algorithm",  // FSERR_PK_ALGORITHM
	"sym algorithm", // FSERR_SYM_ALGORITHM
	"subject type",  // FSERR_SUBJECT_TYPE
	"subject name",  // FSERR_SUBJECT_NAME
	"assurance",     // FSERR_ASSURANCE_LEVE
	"aid list",      // FSERR_AID_LIST
	"aid ssp list",  // FSERR_AID_SSP_LIST
	"time",          // FSERR_TIME
	"start time",    // FSERR_START_TIME
	"end time",      // FSERR_END_TIME
	"publik key",    // FSERR_PUBLIC_KEY
	"verification key", // FSERR_VERIFICATION_KEY
	"encryption key", // FSERR_ENCRYPTION_KEY
	"reconstruction value", // FSERR_RECONSTRUCTION_VALUE
	"other attribute", // FSERR_OTHER_ATTRIBUTE
	"unrec cert request",  //FSERR_REQUEST_UNRECOGNIZED_CERTIFICATE
	"duration",  // FSERR_DURATION
	"2dlocation", // FSERR_TWODLOCATION
	"3dlocation", // FSERR_THREEDLOCATION
	"polygon",    // FSERR_POLYGON
	"ecc point",
};

static const char * _fitsec_msg_containers[] = {
	/* 14 */ "",
	/* 15 */ "trailer",
	/* 16 */ "encrypted key",
	/* 17 */ "receipient info",
	/* 18 */ "encryption parameters",
	/* 19 */ "identified",
	/* 20 */ "circle",
	/* 21 */ "polygon",
	/* 22 */ "region",
	/* 23 */ "signer",  // FSERR_SIGNER					0x004000 //1<<14
	/* 24 */ "headers", // FSERR_HEADERS                   0x008000 //1<<15
	/* 25 */ "subject attributes", // FSERR_SUBJECT_ATTRIBUTE		    0x010000 //1<<16
	/* 26 */ "validity restrictions", // FSERR_VALIDITY_RESTRICTION		0x020000 //1<<17
	/* 27 */ "signer info", // FSERR_SIGNER_INFO				0x040000 //1<<18
	/* 28 */ "signature", // FSERR_SIGNATURE					0x080000 //1<<19
	/* 29 */ "payload", // FSERR_PAYLOAD					0x100000 //1<<20
	/* 30 */ "certificate", // FSERR_CERTIFICATE				0x200000 //1<<21
	/* 31 */ "certificate chain" // FSERR_CHAIN						0x400000 //1<<22
};

static char _err[1024];
FITSEC_EXPORT const char * FitSec_ErrorMessage(int err)
{
	int i;
	char * ptr = &_err[0];
	const char * end = ptr + sizeof(_err);
	for (i = sizeof(_fitsec_msg_containers) / sizeof(_fitsec_msg_containers[0])-1; i >= 0; i--){
		if (err & (1 << (FSERR_CONTAINERS_SHIFT+i))){
			ptr = cstrncpy(ptr, end - ptr, _fitsec_msg_containers[i]);
			ptr = cstrncpy(ptr, end - ptr, ":");
		}
	}
	i = FSERR_ELEMENT(err);
	if (i > 0 && i < sizeof(_fitsec_msg_element) / sizeof(_fitsec_msg_element[0])){
		ptr = cstrncpy(ptr, end - ptr, _fitsec_msg_element[i]);
		ptr = cstrncpy(ptr, end - ptr, ":");
	}
	
	err = FSERR_ERROR(err);
	for (i = 0; i < sizeof(_fitsec_msg_actions) / sizeof(_fitsec_msg_actions[0]); i++){
		if (_fitsec_msg_actions[i].id == err){
			*ptr++ = ' ';
			ptr = cstrncpy(ptr, end - ptr, _fitsec_msg_actions[i].msg);
			*ptr = 0;
			break;
		}
	}
	if (i == sizeof(_fitsec_msg_actions) / sizeof(_fitsec_msg_actions[0])){
		ptr += sprintf(ptr, "ERR_%d", err);
	}
	return _err;
}
