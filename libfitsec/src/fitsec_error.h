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

#ifndef fitsec_error_h
#define fitsec_error_h

#define FSISERR(X) ((0xFF&(X))?1:0)
#define FSISOK(X)  ((0xFF&(X))?0:1)

// events (0..256)
#define FSERR_OK		0x0
#define FSERR_PARSEERROR	EFAULT
#define FSERR_NOSPACE		ENOSPC
#define FSERR_INVALID		EINVAL
#define FSERR_UNKNOWN		ENOENT
#define FSERR_ORDER		EBADF


// elementes
#define FSERR_ELEMENTS_SHIFT 8

enum {
	FSERR_VERSION = (1 << FSERR_ELEMENTS_SHIFT),
	FSERR_SIZE,
	FSERR_TYPE,
	FSERR_DIGEST,
	FSERR_AID,
	FSERR_PK_ALGORITHM,
	FSERR_SYM_ALGORITHM,
	FSERR_SUBJECT_TYPE,
	FSERR_SUBJECT_NAME,
	FSERR_ASSURANCE_LEVEL,
	FSERR_AID_LIST,
	FSERR_AID_SSP_LIST,
	FSERR_TIME,
	FSERR_START_TIME,
	FSERR_END_TIME,
	FSERR_PUBLIC_KEY,
	FSERR_VERIFICATION_KEY,
	FSERR_ENCRYPTION_KEY,
	FSERR_RECONSTRUCTION_VALUE,
	FSERR_OTHER_ATTRIBUTE,
	FSERR_REQUEST_UNRECOGNIZED_CERTIFICATE,
	FSERR_DURATION,
	FSERR_TWODLOCATION,
	FSERR_THREEDLOCATION,
	FSERR_ECC_POINT,
};

// big elements
#define FSERR_TRAILER                   (1<<15)
#define FSERR_ENCRYPTED_KEY             (1<<16)
#define FSERR_RECIPIENT_INFO			(1<<17)
#define FSERR_ENC_PARAMETERS            (1<<18)
#define FSERR_IDENTIFIED                (1<<19)
#define FSERR_CIRCLE                    (1<<20)
#define FSERR_POLYGON                   (1<<21)
#define FSERR_REGION                    (1<<22)
#define FSERR_SIGNER					(1<<23)
#define FSERR_HEADERS                   (1<<24)

#define FSERR_SUBJECT_ATTRIBUTE		    (1<<25)
#define FSERR_VALIDITY_RESTRICTION		(1<<26)
#define FSERR_SIGNER_INFO				(1<<27)
#define FSERR_SIGNATURE					(1<<28)
#define FSERR_PAYLOAD					(1<<29)
#define FSERR_CERTIFICATE				(1<<30)
#define FSERR_CHAIN						(1<<31)

#define FSERR_CONTAINERS_SHIFT 14
#define FSERR_ELEMENT(err) (((err)>>FSERR_ELEMENTS_SHIFT) & ((1<<(FSERR_CONTAINERS_SHIFT-FSERR_ELEMENTS_SHIFT))-1))
#define FSERR_SET_ELEMENT(E,e) (((E) & (((-1)<<FSERR_CONTAINERS_SHIFT) | ((1<<FSERR_ELEMENTS_SHIFT)-1))) | (e))
#define FSERR_ERROR(err)   ((err)  & ((1<<(FSERR_ELEMENTS_SHIFT))-1))
#endif
