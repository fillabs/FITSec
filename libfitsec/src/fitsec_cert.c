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

#include "fitsec_i.h"
#include "cserialize.h"
#include "cmem.h"
#include "cstr.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void FN_THROW(RuntimeException) _Certificate_Read_V2(FSCertificate * c, const char** const ptr, const char* const end, int * const p_error);
static void FN_THROW(RuntimeException) _Certificate_Skip_V2(const char ** const ptr, const char * const end, int * const p_error);
static void FN_THROW(RuntimeException) _Certificate_ReadSubjectAttributes(FSCertificate * c, const char** const ptr, const char* const end, int * const p_error);
static void FN_THROW(RuntimeException) _Certificate_ReadValidityRestrictions(FSCertificate * c, const char** const ptr, const char* const end, int * const p_error);
static void FN_THROW(RuntimeException) _Certificate_ReadSubjectAttribute(FSCertificate * c, const char** const ptr, const char* const end, int * const p_error);
static void FN_THROW(RuntimeException) _Certificate_ReadAIDList(FSCertificate * c, const char** const ptr, const char* const end, int * const p_error);
static void FN_THROW(RuntimeException) _Certificate_ReadAIDSSPList(FSCertificate * c, const char** const ptr, const char* const end, int * const p_error);
static void FN_THROW(RuntimeException) _Certificate_ReadValidityRestriction(FSCertificate * c, const char** const ptr, const char* const end, int * const p_error);

//static FSItsAidSsp * _Certificate_GetItsAidSsp(FSCertificate * c, FSItsAid aid);

#define FSRethrow(E,M) e4c_throw(E4C_EXCEPTION.type, E4C_EXCEPTION.file, E4C_EXCEPTION.line,  ((E)|E4C_EXCEPTION.err), ((M)?(M):e4c.err.message))

static void _Certificate_Init(FitSec * e, FSCertificate * c)
{
	memset(c, 0, sizeof(FSCertificate));
	cring_init(&c->ring);
	c->_rcntr = 1;
	c->e = e;
}

FSCertificate * Certificate_New(FitSec * e)
{
	FSCertificate * c = (FSCertificate *)cring_erase(e->pool.next);
	if (&c->ring == &e->pool){
		c = cnew0(FSCertificate);
	}
	_Certificate_Init(e, c);
	return c;
}

static void _Certificate_Clean(FSCertificate * c)
{
	FitSec * e = c->e;
	Certificate_Release(c->signer);
	FitSecKey_Cleanup(c->e, &c->vkey);
	FitSecKey_Cleanup(c->e, &c->ekey);
	FitSecSignature_Cleanup(c->e, &c->signature);
	if (c->data) free(c->data);
	memset(c, 0, sizeof(FSCertificate));
	_Certificate_Init(e, c);
}

static void _Certificate_Free(FSCertificate * c)
{
	if (c){
		_Certificate_Clean(c);
		cring_enqueue(&c->e->pool, Certificate_Retain(c));
	}
}

void Certificate_Release(FSCertificate * c)
{
	if (c)crelease(c, _Certificate_Free);
}

FSCertificate*  Certificate_Retain(FSCertificate * c)
{
	if (c)cretain(c);
	return c;
}


void FN_THROW(RuntimeException) Certificate_Read(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	switch (**ptr){
	case 2:
		if (c){
			_Certificate_Read_V2(c, ptr, end, perror);
		}
		else{
			_Certificate_Skip_V2(ptr, end, perror);
		}
		break;
	default:
		throw(RuntimeException, FSERR_INVALID | FSERR_CERTIFICATE | FSERR_VERSION, NULL);
	}
}

void FN_THROW(RuntimeException) Certificate_SetData(FSCertificate * c, const char * data, int data_size, const unsigned char * hash, int * const perror)
{
	const char * ptr = data;
	if (c->data) free(c->data);
	c->data = cmemdup(data, data_size);
	ptr = (const char *)c->data;

	Certificate_Read(c, &ptr, ptr + data_size, perror);
	c->data_size = ptr - (const char *)c->data;
	if (hash){
		memcpy(c->hash, hash, 32);
	}
	else{
		// calculate certificate hash
		CalculateCertificateHash(c->e, FS_ECDSA_NISTP256, c->data, c->data_size, c->hash, perror);
	}
	c->digest = toHashedId8(c->hash + 24);
}

void Certificate_SetDigest(FSCertificate * c, HashedId8 digest)
{
	if (c->data){
		_Certificate_Clean(c);
	}
	c->digest = digest;
}

static void FN_THROW(RuntimeException) _Certificate_Skip_V2(const char ** const ptr, const char * const end, int * const perror)
{
	// signer info
	(*ptr)++; // skip version
	switch (*ptr[0]){
	case FS_SI_SELF:
		(*ptr)++;
		break;
	case FS_SI_DIGEST:
		// skip digest
		cbuf_read(NULL, 9, ptr, end, perror);
		break;
	default:
		*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_SIGNER_INFO | FSERR_TYPE;
		throw(RuntimeException, *perror, NULL);
	}
	
	(*ptr)++; // skip subject type;
	
	cstrn_read(NULL, 32, ptr, end, perror); // skip subject name
	cstr_read(NULL, ptr, end, perror); // skip subject attributes
	cstr_read(NULL, ptr, end, perror); // skip validity restrictions
	FitSecSignature_Read(NULL, NULL, ptr, end, perror); // skip signaure
}

static void FN_THROW(RuntimeException) _Certificate_Read_V2(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	SignerInfo si;

	if (c == NULL){
		_Certificate_Skip_V2(ptr, end, perror);
		return;
	}

	(*ptr)++; // skip version

	//signer info
	SignerInfo_Read(&si, ptr, end, perror);
	switch (si.type){
	case FS_SI_SELF:
		c->signer = Certificate_Retain(c);
		break;
	case FS_SI_DIGEST:
		c->signer_digest = si.u.digest;
		break;
	default:
		*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_SIGNER_INFO | FSERR_TYPE;
		throw(RuntimeException, *perror, NULL);
	}

	// subject info
	e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_TYPE;
	c->subjectType = cint8_read(ptr, end, perror);
	// subject name
	e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_NAME;
	cstrn_read(c->subjectName, sizeof(c->subjectName), ptr, end, perror);
	// subject attributes
	_Certificate_ReadSubjectAttributes(c, ptr, end, perror);
	// validity restrictions
	_Certificate_ReadValidityRestrictions(c, ptr, end, perror);
	// signature
	e4c.err.err = FSERR_CERTIFICATE | FSERR_SIGNATURE;
	FitSecSignature_Read(c->e, &c->signature, ptr, end, perror);
}

static void FN_THROW(RuntimeException) _Certificate_ReadSubjectAttributes(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	const char * sa_end;
	uint32_t size;
	e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE|FSERR_SIZE;
	size = (uint32_t)cintx_read(ptr, end, perror);
	sa_end = *ptr + size;
	while (*ptr < sa_end) {
		_Certificate_ReadSubjectAttribute(c, ptr, sa_end, perror);
	}
}

static void FN_THROW(RuntimeException) _Certificate_ReadValidityRestrictions(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	const char * sa_end;
	uint32_t size;

	e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION;
	size = (uint32_t)cintx_read(ptr, end, perror);
	sa_end = *ptr + size;
	while (*ptr < sa_end){
		_Certificate_ReadValidityRestriction(c, ptr, sa_end, perror);
	}
}

static void FN_THROW(RuntimeException) _Certificate_ReadSubjectAttribute(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	uint8_t type;
	int len;

	e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_TYPE;
	type = cint8_read(ptr, end, perror);
	switch (type){
	case SA_VERIFICATION_KEY:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_VERIFICATION_KEY;
		FitSecKey_Read(c->e, &c->vkey, ptr, end, perror);
		break;
	case SA_ENCRYPTION_KEY:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_ENCRYPTION_KEY;
		FitSecKey_Read(c->e, &c->ekey, ptr, end, perror);
		break;
	case SA_RECONSTRUCTION_VALUE:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_RECONSTRUCTION_VALUE;
		EccPoint_Read(&c->recValue, ptr, end, perror);
		break;
	case SA_ASSURANCE_LEVEL:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_ASSURANCE_LEVEL;
		c->assurance = cint8_read(ptr, end, perror);
		break;
	case SA_ITS_AID_LIST:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_AID_LIST | FSERR_SIZE;
		len = (int)cintx_read(ptr, end, perror);
		_Certificate_ReadAIDList(c, ptr, *ptr + len, perror);
		break;
	case SA_ITS_AID_SSP_LIST:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_AID_SSP_LIST | FSERR_SIZE;
		len = (int)cintx_read(ptr, end, perror);
		_Certificate_ReadAIDSSPList(c, ptr, *ptr + len, perror);
		break;
	default:
		// skip unknown attribute for the moment
		e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_SIZE;
		cstr_read(NULL, ptr, end, perror);
	}
}

static void FN_THROW(RuntimeException) _Certificate_ReadAIDList(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	int i;
	e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_AID_LIST;
	for (i = 0; *ptr < end; i++){
		if (i >= MAX_AID_COUNT){
			*perror = e4c.err.err;
			throw(RuntimeException, *perror, NULL);
		}
		c->aidssp[i].aid = (FSItsAid)cintx_read(ptr, end, perror);
		// fill the SSP with 0xFF to permit any services
		memset(c->aidssp[i].u.data, 0xFF, sizeof(c->aidssp[i].u.data));
	}
	c->aidsspcount = i;
}

static void FN_THROW(RuntimeException) _Certificate_ReadAIDSSPList(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	int i;
	e4c.err.err = FSERR_CERTIFICATE | FSERR_SUBJECT_ATTRIBUTE | FSERR_AID_SSP_LIST;
	for (i = 0; *ptr < end; i++){
		if (i >= MAX_AID_COUNT){
			*perror = e4c.err.err;
			throw(RuntimeException, *perror, NULL);
		}
		c->aidssp[i].aid = (FSItsAid)cintx_read(ptr, end, perror);
		cstrn_read((char*)&c->aidssp[i].u.data[0], 31, ptr, end, perror);
	}
	c->aidsspcount = i;
}

static void FN_THROW(RuntimeException) _Certificate_ReadValidityRestriction(FSCertificate * c, const char** const ptr, const char* const end, int * const perror)
{
	uint8_t type;
	//vr type
	e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_TYPE;
	type = cint8_read(ptr, end, perror);
	switch (type){
	case VR_TIME_START_AND_DURATION:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_START_TIME;
		c->start_time = cint32_read(ptr, end, perror);
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_DURATION;
		c->end_time = c->start_time + cint16_read(ptr, end, perror);
		break;
	case VR_TIME_START_AND_END:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_START_TIME;
		c->start_time = cint32_read(ptr, end, perror);
	case VR_TIME_END:
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION | FSERR_END_TIME;
		c->end_time = cint32_read(ptr, end, perror);
		break;
	case VR_REGION:
		GeographicRegion_Read(&c->region, ptr, end, perror);
		break;
	default:
		// skip unknown VR for the moment
		e4c.err.err = FSERR_CERTIFICATE | FSERR_VALIDITY_RESTRICTION;
		cstr_read(NULL, ptr, end, perror);
	}
}

static FSBOOL _Certificate_CalculateSignatureHash(FSCertificate * c, unsigned char * hash, int * perror)
{
	if (c->data){
		int csize;
		switch (c->signature.alg){
		case FS_ECDSA_NISTP256:
		case FS_ECIES_NISTP256:
			csize = 66;
			break;
		default:
			*perror = FSERR_SIGNATURE | FSERR_CERTIFICATE | FSERR_TYPE;
			return FSFALSE;
		};
		FitSecHash_Calc(c->e, c->signature.alg, hash, c->data, c->data_size - csize);
		return FSTRUE;
	}
	*perror = FSERR_SIGNATURE | FSERR_CERTIFICATE | FSERR_TYPE;
	return FSFALSE;
}

FSBOOL CalculateCertificateHash(FitSec * e, FSCryptPKAlgorithm alg, const char * ptr, int size, unsigned char * hash, int * const perror)
{
	int n, ptype_shift = 65;
	switch (alg){
	case FS_ECDSA_NISTP256:
	case FS_ECIES_NISTP256:
		ptype_shift = 65;
		break;
	default:
		if (perror) *perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_PK_ALGORITHM;
		return FSFALSE;
	};
	n = ptr[size - ptype_shift];
	((char*)ptr)[size - ptype_shift] = 0;
	FitSecHash_Calc(e, alg, hash, ptr, size);
	((char*)ptr)[size - ptype_shift] = n;
	return FSTRUE;
}

FSBOOL Certificate_SetPrivateKey(FSCertificate * c, int type, const char * data, int dsize)
{
	FitSecKey * k;
	switch (type){
	case SA_VERIFICATION_KEY:
		k = &c->vkey;
		break;
	case SA_ENCRYPTION_KEY:
		k = &c->ekey;
		break;
	default:
		return FSFALSE;
	}
	return FitSecKey_SetPrivateKey(c->e, k, data, dsize);
}

FSBOOL Certificate_Validate(FSCertificate * c, int * perror)
{
	int i;
	// signer
	if (c->signer == NULL){
		*perror = FSERR_UNKNOWN | FSERR_CERTIFICATE | FSERR_SIGNER;
		return FSFALSE;
	}
	if (c->signer->flags & CERT_INVALID) {
		*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_SIGNER;
		return FSFALSE;
	}

	// verify time validity
	if (c->signer->end_time < c->end_time) {
		*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_END_TIME;
		return FSFALSE;
	}
	if (c->signer->start_time > c->start_time) {
		*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_START_TIME;
		return FSFALSE;
	}

	//verify aid
	for (i = 0; i < c->aidsspcount; i++) {
		if (NULL == Certificate_GetAidSsp(c->signer, c->aidssp[i].aid)){
			*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_AID;
			return FSFALSE;
		}
	}

	//verify region
	if (c->signer->region.type != REGION_NONE){
		if (c->region.type == REGION_NONE) {
			*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_REGION;
			return FSFALSE;
		}
		if (!GeographicRegion_IsRegionInside(&c->signer->region, &c->region)){
			*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_REGION;
			return FSFALSE;
		}
	}

	// verify Signature
	{
		// calculate hash
		unsigned char hash[32];
		if (!_Certificate_CalculateSignatureHash(c, hash, perror)){
			return FSFALSE;
		}
		if(!FitSecSignature_Verify(c->e, &c->signature, &c->signer->vkey, hash)){
			*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_REGION;
			return FSFALSE;
		}
	}
	return FSTRUE;
}

FSBOOL Certificate_ValidateChain(FSCertificate * c, int *perror)
{
	if (c->flags & CERT_TRUSTED){
		*perror = 0;
		return FSTRUE;
	}
	if (c->flags & CERT_INVALID){
		*perror = FSERR_INVALID | FSERR_CERTIFICATE;
		return FSFALSE;
	}

	if (c->signer == c){
		c->flags |= CERT_TRUSTED;
		return FSTRUE;
	}
	
	if (!c->signer){
		*perror = FSERR_UNKNOWN | FSERR_CERTIFICATE | FSERR_SIGNER;
		return FSFALSE;
	}
	
	if (Certificate_ValidateChain(c->signer, perror)){
		if (Certificate_Validate(c, perror)){
			c->flags |= CERT_TRUSTED;
			return FSTRUE;
		}
	}
	if (*perror & FSERR_INVALID){
		c->flags |= CERT_INVALID;
	}
	return FSFALSE;
}

FSBOOL Certificate_IsValidForTime(const FSCertificate * c, Time64 time, int * const perror)
{
	uint32_t t = (uint32_t)(time / 1000);
	// check time
	if (c->start_time > t || c->end_time < t){
		*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_TIME;
		return FSFALSE;
	}
	return FSTRUE;
}

FSBOOL AIDSSP_Match(const FSItsAidSsp * tpl, const FSItsAidSsp * ssp)
{
	if (tpl->aid == ssp->aid){
		if (tpl->u.ssp.version > 0) {
			if (ssp->u.ssp.version == 0){
				int i;
				for (i = 0; i < (sizeof(tpl->u.data) / sizeof(int)); i++){
					if ((tpl->u.ssp.flags[i] & ssp->u.ssp.flags[i]) != ssp->u.ssp.flags[i]){
						return FSFALSE;
					}
				}
				return FSTRUE;
			}
		}
	}
	return FSFALSE;
}

FSBOOL Certificate_IsValidForSSP(const FSCertificate * c, const FSItsAidSsp * ssp, int * const perror)
{
	int i;
	for (i = 0; i < c->aidsspcount; i++){
		if (c->aidssp[i].aid == ssp->aid &&
			AIDSSP_Match(&c->aidssp[i], ssp)) {
			return FSTRUE;
		}
	}
	*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_AID;
	return FSFALSE;
}

FSBOOL Certificate_IsValidForPosition(const FSCertificate * c, const FSLocation * position, int * const perror)
{
	if (position){
		if (!GeographicRegion_IsPointInside(&c->region, (const TwoDLocation*)position)){
			*perror = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_REGION;
			return FSFALSE;
		}
	}
	return FSTRUE;
}

FSBOOL Certificate_IsValidFor(const FSCertificate * c, const FSItsAidSsp * ssp, const FSLocation * position, Time64 time, int * const perror)
{
	return  (
		Certificate_IsValidForTime(c, time, perror) &&
		Certificate_IsValidForSSP(c, ssp, perror) &&
		Certificate_IsValidForPosition(c, position, perror)
	);
}

const FSItsAidSsp *   Certificate_GetAidSsp(const FSCertificate * c, FSItsAid aid)
{
	int i;
	for (i = 0; i < c->aidsspcount; i++){
		if (c->aidssp[i].aid == aid){
			return &c->aidssp[i];
		}
	}
	return NULL;
}
