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
#include "cstr.h"
#include "cmem.h"
#include <errno.h>
#include <assert.h>

static int _point_sizes[FSCryptPKAlgorithm_Max] = {
	32, 32
};

int FitSecPoint_Size(int alg) {
	return _point_sizes[alg];
}

void FN_THROW(RuntimeException) FitSecKey_Read(FitSec * e, FitSecKey * const k, const char ** const ptr, const char * const end, int * const perror)
{
	int type, alg, sym=0, psize;
	try{
		alg = cint8_read(ptr, end, perror);
		psize = FitSecPoint_Size(alg);
		switch (alg) {
		case FS_ECIES_NISTP256:
			sym = cint8_read(ptr, end, perror);
		case FS_ECDSA_NISTP256:
			type = cint8_read(ptr, end, perror);
			if (end - *ptr < psize || (type == FS_UNCOMPRESSED &&  end - *ptr < (psize * 2))){
				throw(RuntimeException, FSERR_PARSEERROR, NULL);
			}
			if (k){
				k->alg = alg;
				k->sym = sym;
				k->size = 0;
				k->key = e->cfg->crypt->Key->Read(e->crypt, alg, sym, type, (const unsigned char*)*ptr, (const unsigned char*)((type == FS_UNCOMPRESSED) ? *ptr + psize : NULL));
			}
			*ptr += (type == FS_UNCOMPRESSED) ? 2 * psize : psize;
			break;
		default:
			psize = (int)cintx_read(ptr, end, perror);
			if (end - *ptr < psize){
				throw(RuntimeException, FSERR_PARSEERROR, NULL);
			}
			// it is enough to point to existing certificate buffer
			if (k){
				k->alg = alg;
				k->sym = 0;
				k->size = psize;
				k->key = (FSCryptKey*)*ptr;
			}
			*ptr = *ptr + psize;
		}
	}
	catch (RuntimeException){
		*perror = FSERR_PUBLIC_KEY | E4C_EXCEPTION.err;
	}
	if (*perror){
		throw(RuntimeException, *perror, "Public key read error");
	}
}

void FN_THROW(RuntimeException) FitSecKey_Write(FitSec * e, const FitSecKey * const k, char ** const ptr, const char * const end, int * const perror)
{
	// TODO: to be implemented
	throw(RuntimeException, 0, "FitSecKey_Write not yet implemented");
}

void FitSecKey_Cleanup(FitSec * e, FitSecKey * const k)
{
	switch (k->alg){
	case FS_ECIES_NISTP256:
	case FS_ECDSA_NISTP256:
		if (k->key){
			e->cfg->crypt->Key->Free(k->key);
		}
	default:
		break;	
	};
}

FSBOOL FitSecKey_SetPrivateKey(FitSec * e, FitSecKey * k, const char * value, size_t vlength)
{
	FSBOOL ret = FSFALSE;
	switch (k->alg){
	case FS_ECIES_NISTP256:
	case FS_ECDSA_NISTP256:
		if (k->key){
			ret = e->cfg->crypt->Key->SetPrivate(k->key, (const unsigned char*)value, vlength);
		}
	default:
		break;	
	};
	return ret;
}

void FN_THROW(RuntimeException) FitSecSignature_Read(FitSec * e, FitSecSignature * const k, const char ** const ptr, const char * const end, int * const perror)
{
	int type, psize, alg;
	const char *r, *s;
	try {
		alg = cint8_read(ptr, end, perror);
		psize = FitSecPoint_Size(alg);
		switch (alg) {
		case FS_ECDSA_NISTP256:
			type = cint8_read(ptr, end, perror);
			r = *ptr;
			if (end - (*ptr) < psize){
				throw(RuntimeException, FSERR_PARSEERROR, NULL);
			}
			*ptr = *ptr + psize;
			if (type == FS_UNCOMPRESSED) { // skip y
				if (end - (*ptr) < psize){
					throw(RuntimeException, FSERR_PARSEERROR, NULL);
					break;
				}
				*ptr = *ptr + psize;
			}

			s = *ptr;
			if (end - (*ptr) < psize){
				throw(RuntimeException, FSERR_PARSEERROR, NULL);
				break;
			}
			*ptr = *ptr + psize;
			if (k){
				k->alg = alg;
				k->signature = e->cfg->crypt->Signature->Read(e->crypt, (const unsigned char*)s, (const unsigned char*)r, type);
			}
			break;
		default:
			psize = (int)cintx_read(ptr, end, perror);
			if (end - *ptr < psize){
				throw(RuntimeException, FSERR_PARSEERROR, NULL);
			}
			else{
				// it is enough to point to existing certificate buffer
				if (k){
					k->alg = alg;
					k->signature = (FSCryptSignature*)*ptr;
				}
				*ptr = *ptr + psize;
			}
		}; // switch
		*perror = 0;
	}
	catch (RuntimeException){
		*perror = E4C_EXCEPTION.err | FSERR_SIGNATURE;
	}
	if (*perror){
		throw(RuntimeException, *perror, "Signature read error");
	}
}

void FN_THROW(RuntimeException) FitSecSignature_Write(FitSec * e, const FitSecSignature * const k, char ** const ptr, const char * const end, int * const error)
{
	int psize = FitSecPoint_Size(k->alg);
	FitSecEccPointType ptype;
	char * p = *ptr;

	assert(k->signature);

	switch (k->alg){
	case FS_ECIES_NISTP256:
	case FS_ECDSA_NISTP256:
		if (end - p < 2 + psize * 2){
			*error = FSERR_PARSEERROR;
			throw(RuntimeException, *error, NULL);
		}
		p[0] = k->alg;
		e->cfg->crypt->Signature->Write(k->signature, (unsigned char*)p + 2 + psize, (unsigned char*)p + 2, &ptype);
		p[1] = ptype;
		*ptr = p + 2 + psize * 2;
		*error = 0;
		break;
	default:
		cstrn_write((const char*)k->signature, k->size, ptr, end, error);
		break;
	};
}

FSBOOL FitSecSignature_Sign(FitSec * e, FitSecSignature * const s, const FitSecKey * k, const unsigned char * hash)
{
	if (s->signature){
		switch (s->alg){
		case FS_ECDSA_NISTP256:
			e->cfg->crypt->Signature->Free(s->signature);
			break;
		default:
			break;
		};
	}

	s->alg = k->alg;
	switch (s->alg){
	case FS_ECDSA_NISTP256:
		s->signature = e->cfg->crypt->Signature->Sign(e->crypt, k->key, hash);
		break;
	default:
		s->size = 0;
		s->signature = NULL;
		break;
	}
	return s->signature ? FSTRUE : FSFALSE;
}


FSBOOL FitSecSignature_Verify(FitSec * e, FitSecSignature * const s, const FitSecKey * k, const unsigned char * hash)
{
	if (s->alg != k->alg){
		return FSFALSE;
	}
	switch (s->alg){
	case FS_ECDSA_NISTP256:
		if (s->signature == NULL || k->key == NULL){
			return FSFALSE;
		}
		return e->cfg->crypt->Signature->Verify(s->signature, k->key, hash);
	default:
		break;
	}
	return FSFALSE;
}

void FitSecSignature_Cleanup(FitSec * e, FitSecSignature * const k)
{
	switch (k->alg){
	case FS_ECDSA_NISTP256:
		if (k->signature){
			e->cfg->crypt->Signature->Free(k->signature);
		}
	default:
		break;
	}
}

void FN_THROW(RuntimeException) EccPoint_Read(EccPoint * p, const char** const ptr, const char* const end, int * const perror)
{
	p->type = cint8_read(ptr, end, perror);
	cbuf_read(&p->x[0], 32, ptr, end, perror);
	switch (p->type){
	case FS_X_COORDINATE_ONLY:
	case FS_COMPRESSED_LSB_Y_0:
	case FS_COMPRESSED_LSB_Y_1:
		break;
	case FS_UNCOMPRESSED:
		cbuf_read(&p->y[0], 32, ptr, end, perror);
		break;
	default:
		cstr_read(NULL, ptr, end, perror); // skip it
	}
}

void FN_THROW(RuntimeException) TwoDLocation_Read(TwoDLocation * l, const char** const ptr, const char* const end, int * const perror)
{
	l->latitude = (int32_t)cint32_read(ptr, end, perror);
	l->longitude = (int32_t)cint32_read(ptr, end, perror);
}

void FN_THROW(RuntimeException) FitSecLocation_Read(FSLocation * l, const char** const ptr, const char* const end, int * const perror)
{
	l->latitude = (int32_t)cint32_read(ptr, end, perror);
	l->longitude = (int32_t)cint32_read(ptr, end, perror);
	l->elevation = (uint16_t)cint16_read(ptr, end, perror);
}

void FN_THROW(RuntimeException) ThreeDLocation_Read(ThreeDLocation * l, const char** const ptr, const char* const end, int * const perror)
{
	FitSecLocation_Read(l, ptr, end, perror);
}

void FN_THROW(RuntimeException) ThreeDLocation_Write(FSLocation * l, char** const ptr, const char* const end, int * const perror)
{
	try {
		cint32_write(l->latitude, ptr, end, perror);
		cint32_write(l->longitude, ptr, end, perror);
		cint16_write(l->elevation, ptr, end, perror);
	}
	catch (RuntimeException){
	}
	if (*perror){
		throw(RuntimeException, FSERR_PARSEERROR, "No more space to write ThreeDLocation");
	}
}

void FN_THROW(RuntimeException) HashedId3_Write(HashedId3 id3, char** const ptr, const char* const end, int * const p_error)
{
	cbuf_write(&id3, 3, ptr, end, p_error);
}

HashedId3 FN_THROW(RuntimeException) HashedId3_Read(const char** const ptr, const char* const end, int * const p_error)
{
	HashedId3 id = 0;
	const char * p = *ptr;
	if (end - p >= 3){
		id = toHashedId3((const unsigned char*)p);
		*ptr = p + 3;
		if (p_error) *p_error = 0;
		return id;
	}
	if (p_error) *p_error = FSERR_PARSEERROR;
	throw(RuntimeException, FSERR_PARSEERROR, NULL);
	return 0;
}

void FN_THROW(RuntimeException) SignerInfo_Read(SignerInfo * si, const char** const ptr, const char* end, int * const perror)
{
	int n;
	unsigned int edef = e4c.err.err;
	e4c.err.err |= FSERR_SIGNER_INFO | FSERR_TYPE;
	si->type = cint8_read(ptr, end, perror);
	switch (si->type){
	case FS_SI_SELF:
		break;
	case FS_SI_DIGEST:
		e4c.err.err = edef | FSERR_SIGNER_INFO | FSERR_DIGEST;
		cbuf_read(&si->u.digest, 8, ptr, end, perror);
		break;
	case FS_SI_CERTIFICATE:
		e4c.err.err = edef | FSERR_SIGNER_INFO;
		si->u.data.beg = *ptr;
		Certificate_Read(NULL, ptr, end, perror);
		si->u.data.end = *ptr;
		break;
	case FS_SI_CERTIFICATE_CHAIN:
	case FS_SI_OTHER_DIGEST:
		n = (int)cintx_read(ptr, end, perror);
		if (*ptr + n > end){
			throw(RuntimeException, FSERR_PARSEERROR, NULL);
		}
		si->u.data.beg = *ptr;
		si->u.data.end = *ptr = si->u.data.beg + n;
		break;
	default:
		throw(RuntimeException, FSERR_INVALID, "Invalid signer info type");
	}
	e4c.err.err = edef;
}

void FN_THROW(RuntimeException) EncryptionParameters_Read(EncryptionParameters * ep, const char** const ptr, const char* end, int * const perror)
{
	unsigned int edef = e4c.err.err;
	e4c.err.err |= FSERR_ENC_PARAMETERS | FSERR_SYM_ALGORITHM;
	ep->sym_alg = cint8_read(ptr, end, perror);
	switch (ep->sym_alg){
	case FS_AES_128_CCM:
		e4c.err.err |= edef | FSERR_ENC_PARAMETERS;
		cbuf_read(&ep->u.aes128ccm.nonce, sizeof(ep->u.aes128ccm.nonce), ptr, end, perror);
		break;
	default:
		throw(RuntimeException, FSERR_INVALID, "Invalid Symmetric algorithm");
	}
	e4c.err.err = edef;
}

void FN_THROW(RuntimeException) RecipientInfo_Read(RecipientInfo * ri, FSCryptSymmAlgorithm sym, const char** const ptr, const char* end, int * const perror)
{
	unsigned int edef;
	edef = e4c.err.err |= FSERR_RECIPIENT_INFO;
	e4c.err.err |= FSERR_DIGEST;
	ri->cert_id = HashedId3_Read(ptr, end, perror);
	e4c.err.err = edef | FSERR_PK_ALGORITHM;
	ri->pk_encryption = cint8_read(ptr, end, perror);
	switch (ri->pk_encryption){
	case FS_ECIES_NISTP256:
		e4c.err.err = edef | FSERR_ENCRYPTION_KEY;
		EciesEncryptedKey_Read(&ri->u.ecies_key, sym, ptr, end, perror);
		break;
	default:
		e4c.err.err = edef;
		cstr_read(NULL, ptr, end, perror);
	}
	e4c.err.err = edef;
}

void FN_THROW(RuntimeException) EciesEncryptedKey_Read(EciesEncryptedKey * k, FSCryptSymmAlgorithm sym, const char** const ptr, const char* end, int * const perror)
{
	unsigned int edef;
	edef = e4c.err.err |= FSERR_ENCRYPTED_KEY;
	e4c.err.err |= FSERR_ECC_POINT;
	EccPoint_Read(&k->v, ptr, end, perror);
	e4c.err.err = edef | FSERR_SYM_ALGORITHM;
	switch (sym){
	case FS_AES_128_CCM:
		e4c.err.err = edef;
		cbuf_read(k->c, 16, ptr, end, perror);
	default:
		throw(RuntimeException, FSERR_INVALID, "Invalid Symmetric algorithm");
	};
	e4c.err.err = edef;
	cbuf_read(k->t, 16, ptr, end, perror);
}
