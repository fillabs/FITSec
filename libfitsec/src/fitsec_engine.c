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

#include "fitsec_i.h"
#include "cserialize.h"
//#include "cdir.h"
#include "cstr.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/*

typedef struct FitSecProfile {
	FSItsAid aid;

	int    certPeriod; // max delay between two cert sending
}FitSecProfile;
static FitSecProfile _profiles[] = {
	{   FITSEC_AID_CAM, 990 },
	{	0, 0	} // default
};

*/

static FSBOOL _dummyOnEvent(FitSec * e, void * user, int event, void * params)
{
	return FSTRUE;
}

FITSEC_EXPORT void  FitSecConfig_InitDefault(FitSecConfig * cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	cfg->version = DEFAULT_PROTOCOL_VERSION;
	cfg->certPeriod = 990;
	cfg->flags = FS_USE_CERT_REQUEST;
	cfg->crypt = FSCryptEngineConfig_Default();
	cfg->cbOnEvent = _dummyOnEvent;
}

FITSEC_EXPORT FitSec * FitSec_New(const FitSecConfig * config)
{
	FitSec * e = cnew0(FitSec);
	e->cfg = config;
	e->crypt = FitSecCrypt_New(e, NULL);
	e->certs = CertificateHash_New();
	e->version = config->version ? config->version : DEFAULT_PROTOCOL_VERSION;
	cring_init(&e->pool);
	cring_init(&e->mycerts);
	cring_init(&e->unknown);
	cring_init(&e->trusted);
	return e;
}

FITSEC_EXPORT void FitSec_Clean(FitSec * e, int clean_local)
{
	// remove foreign at certificates
	if(clean_local){
		cring_cleanup(&e->mycerts, Certificate_Release);
	}
	cring_cleanup(&e->trusted, Certificate_Release);
	cring_cleanup(&e->unknown, Certificate_Release);
	CertificateHash_Purge(e->certs);
	cring_cleanup(&e->pool, free);
	if(!clean_local){
		// add all local certificates to the hash
		FSCertificate * at = cring_next_cast(&e->mycerts, FSCertificate);
		FSCertificate *aa = NULL, *root = NULL;
		for(;&at->ring != &e->mycerts; at=cring_next_cast(at, FSCertificate)){
			if(at->signer != aa){
				aa = at->signer;
				CertificateHash_Add(e->certs, aa);
				if(aa->signer != root){
					root = aa->signer;
					CertificateHash_Add(e->certs, root);
				}
			}
		}
	}
}

FITSEC_EXPORT void FitSec_Free(FitSec * e)
{
	FitSec_Clean(e, 1);
	FitSecCrypt_Free(e, e->crypt);
	free(e);
}

FITSEC_EXPORT const FSCertificate * FitSec_InstallCertificate(FitSec * e,
	const char * cert, int cert_length,
	const char * vkey, int vkey_length,
	const char * ekey, int ekey_length,
	int * p_error)
{
	const char * ptr = cert;
	const char * end = cert + cert_length;
	unsigned char hash[32];
	HashedId8 digest;
	FSCertificate *c, *o;
	int error;
	
	c = Certificate_New(e);
	// calculate digest
	try {
		Certificate_Read(c, &ptr, end, &error);
		CalculateCertificateHash(e, FS_ECDSA_NISTP256, cert, ptr - cert, hash, &error);
		digest = toHashedId8(hash + 24);
		// search for FSCertificate
		o = CertificateHash_Find(e->certs, digest);
		if (o){
			Certificate_Release(c);
			c = Certificate_Retain(o);
		}
		if (c->data == NULL){
			Certificate_SetData(c, cert, ptr - cert, hash, &error);
		}

		if (vkey){
			if (!Certificate_SetPrivateKey(c, SA_VERIFICATION_KEY, vkey, vkey_length)){
				throw(RuntimeException, 0, "Invalid private verification key");
			}
			if (c->subjectType == AUTHORIZATION_TICKET){
				Certificate_Retain(c);
				cring_enqueue(&e->mycerts, &c->ring);
			}
		}
		if (ekey){
			if (!Certificate_SetPrivateKey(c, SA_ENCRYPTION_KEY, ekey, ekey_length)){
				throw(RuntimeException, 0, "Invalid private encryption key");
			}
		}
		if (NULL == o){
			// new certificate
			CertificateHash_Add(e->certs, c);
		}
		Certificate_Release(c); // decrease pointer

		CertificateHash_RelinkSigners(e->certs);
		if (e->currentCert == NULL && !cring_is_empty(&e->mycerts)){
			e->currentCert = (FSCertificate*)e->mycerts.next;
		}
	}
	catch (RuntimeException){
		Certificate_Release(c);
		c = NULL;
		if (p_error) *p_error = E4C_EXCEPTION.err;
	}
	return c;
}

/*
FITSEC_EXPORT FSBOOL FitSec_Initialize(FitSec * e)
{
	// load all stored certificates, skipping expired
	int error;
	size_t dsize = 4096;
	char *vkey, *ekey, *fn;
	int cert_len = 0, vkey_len = 0, ekey_len = 0;

	cdir_t * dir = cdir_open(e->cfg->storage, "*.crt", e_cdir_nodirs);
	if (dir){
		const cdir_stat_t *st;
		char * data = malloc(4096); // it must not be more
		while ((st = cdir_next(dir))){
			char * end = cstrnload(data, (int)dsize, st->path);
			if (end > data){
				if (e->cfg->hexadecimal){
					end = cstr_hex2bin(data, end - data, data, end - data);
				}
				cert_len = end - data;

				// look for keys
				fn = strrchr(st->fname, '.'); if (!fn) fn = cstrend(st->fname);

				cstrcpy(fn, ".vkey");
				vkey = end;
				end = cstrnload(vkey, dsize - (vkey - data), st->path);
				if (end <= vkey){
					end = vkey; vkey = NULL;
				}
				else{
					if (e->cfg->hexadecimal){
						end = cstr_hex2bin(vkey, end - vkey, vkey, end - vkey);
					}
					vkey_len = end - vkey;
				}

				cstrcpy(fn, ".ekey");
				ekey = end;
				end = cstrnload(ekey, dsize - (ekey - data), st->path);
				if (end <= ekey){
					end = ekey; ekey = NULL;
				}
				else{
					if (e->cfg->hexadecimal){
						end = cstr_hex2bin(ekey, end - ekey, ekey, end - ekey);
					}
					ekey_len = end - vkey;
				}
				if (NULL == FitSec_InstallCertificate(e, data, cert_len, vkey, vkey_len, ekey, ekey_len, &error)){
					printf("%s: %s\n", st->fname, FitSec_ErrorMessage(error));

				}
			}
		}
		if (data){
			free(data);
		}
		cdir_close(dir);

		// go throw the certificate table and link all certs and its signers
		CertificateHash_RelinkSigners(e->certs);
		e->currentCert = (FSCertificate*)e->mycerts.next;
	}

	return FS_CERT_IS_TRUSTED(e->currentCert)? FSTRUE : FSFALSE;
}
*/
HashedId3 HashedId8toId3(HashedId8 id8)
{
	return ((HashedId3*)&id8)[1]>>8;
}

HashedId3 toHashedId3(const unsigned char * buf)
{
	HashedId3 ret;
	int n = ((unsigned long)buf) & 3;
	if (n == 1){
		HashedId3 * p = (HashedId3 *)(buf - n);
		ret = *p >> 8;
	}
	else{
		unsigned char * d = (unsigned char *)&ret;
		d[0] = buf[0];
		d[1] = buf[1];
		d[2] = buf[2];
		d[3] = 0;
	}
	return ret;
}

HashedId8 toHashedId8(const unsigned char * buf)
{
	return *(HashedId8*)(buf);
}

FSCertificate * FitSec_GetMyCertificate(FitSec * e, HashedId8 digest)
{
	FSCertificate * a = cring_next_cast(&e->mycerts, FSCertificate);
	for (; &a->ring != &e->mycerts; a = cring_next_cast(a, FSCertificate)){
		if (a->digest == digest){
			return a;
		}
	}
	return NULL;
}

static FSSignerInfoType _FitSec_GetCAMSignerInfo(FitSec * e, Time64 t)
{
	FSSignerInfoType ret = FS_SI_DIGEST;
	if (e->requestedCert == e->currentCert){
		ret = FS_SI_CERTIFICATE;
	}
	else if (e->requestedCert == e->currentCert->signer){
		ret = FS_SI_CERTIFICATE_CHAIN;
	}
	else if (e->newNeighbourFlag){
		ret = FS_SI_CERTIFICATE;
	}
	else if (t >= e->nextCertTime){
		ret = FS_SI_CERTIFICATE;
	}
	if (ret == FS_SI_CERTIFICATE){
		e->nextCertTime = t + (e->cfg->certPeriod * 1000);
	}
	e->requestedCert = NULL;
	e->newNeighbourFlag = 0;
	return ret;
}

FSCertificate * FitSec_SelectMyCertificate(FitSec * e, const FSItsAidSsp * ssp, const FSLocation * position, Time64 time)
{
	int error;
	if (Certificate_IsValidFor(e->currentCert, ssp, position, time, &error)){
		return e->currentCert;
	}
	
	FSCertificate * c = cring_next_cast(&e->mycerts, FSCertificate);
	while (&c->ring != &e->mycerts){
		if (c != e->currentCert){
			if (Certificate_IsValidFor(c, ssp, position, time, &error)){
				// TODO: switch to the new certificate
				e->currentCert = c;
				return c;
			}
		}
		c = cring_next_cast(c, FSCertificate);
	}
	return NULL;
}

FITSEC_EXPORT int FitSec_PrepareMessage(FitSec * e, FSMessageInfo* m, char * buf, int bufsize)
{
	int ret = -1;
	int error;
	char * ptr = buf;
	const char * end = buf + bufsize;
	FSSignerInfoType sitype;

	// looking for the certificate correspondent to current situation
	// system must change identity inside this function if current certificate is not valid for the situation
	m->cert = FitSec_SelectMyCertificate(e, &m->ssp, &m->position, m->generationTime);
	if (m->cert == NULL){
		m->status = FSERR_UNKNOWN | FSERR_CERTIFICATE;
		return -1;
	}
	
	cbookmark * bm = (cbookmark *)&m->_ptrs[0];
	bm->idx = 0;

	if (m->ssp.aid == FITSEC_AID_CAM || m->ssp.aid == FITSEC_AID_DENM){
		if (m->payloadType != FS_PAYLOAD_SIGNED){
			// TODO: raise a warning here
			m->payloadType = FS_PAYLOAD_SIGNED;
		}
	}

	try{
		// protocol version
		cint8_write(e->version, &ptr, end, &error);
		
		// Message headers bookmark
		cbookmark_store(bm, &ptr, end, &error);
		
		//signer info
		if (m->ssp.aid == FITSEC_AID_CAM){
			sitype = _FitSec_GetCAMSignerInfo(e, m->generationTime);
		}
		else{
			switch (m->payloadType){
			case FS_PAYLOAD_SIGNED:
			case FS_PAYLOAD_SIGNED_AND_ENCRYPTED:
			case FS_PAYLOAD_SIGNED_EXTERNAL:
				sitype = FS_SI_CERTIFICATE;
				break;
			default:
				sitype = FS_SI_NO_SIGNATURE;
			};
		}
		m->si_type = sitype;
		if (sitype != FS_SI_NO_SIGNATURE){
			cint8_write(FT_SIGNER_INFO, &ptr, end, &error);
			cint8_write(sitype, &ptr, end, &error);
			if (sitype == FS_SI_DIGEST){
				cbuf_write(&m->cert->digest, 8, &ptr, end, &error);
			}
			else if (sitype == FS_SI_CERTIFICATE){
				cbuf_write(m->cert->data, m->cert->data_size, &ptr, end, &error);
			}
			else if (sitype == FS_SI_CERTIFICATE_CHAIN){
				// bookmark position 
				cbookmark_store(bm, &ptr, end, &error);
				cbuf_write(m->cert->signer->data, m->cert->signer->data_size, &ptr, end, &error);
				cbuf_write(m->cert->data, m->cert->data_size, &ptr, end, &error);
				// apply bookmark 
				cbookmark_apply(bm, &ptr, end, &error);
			}
			else{
				throw(RuntimeException, FSERR_INVALID, "Invalid Signer info type");
			}
		}

		// generation time
		cint8_write(FT_GENERATION_TIME, &ptr, end, &error);
		cint64_write(m->generationTime, &ptr, end, &error);

		// generation location
		if (m->ssp.aid != FITSEC_AID_CAM){
			cint8_write(FT_GENERATION_LOCATION, &ptr, end, &error);
			ThreeDLocation_Write(&m->position, &ptr, end, &error);
		}

		// request_unrecognized_certificate
		if (m->ssp.aid == FITSEC_AID_CAM){
			// add all unknown certificates in the list
			FSCertificate * c = (FSCertificate *)e->unknown.next;
			if (&c->ring != &e->unknown){
				cint8_write(FT_REQUEST_UNRECOGNIZED_CERTIFICATE, &ptr, end, &error);
				cbookmark_store(bm, &ptr, end, &error);
				do {
					HashedId3_Write(HashedId8toId3(c->digest), &ptr, end, &error);
					c = (FSCertificate *)cring_erase(e->unknown.next);
				} while (&c->ring != &e->unknown);
				cbookmark_apply(bm, &ptr, end, &error);
			}
		}

		//ITS AID
		cint8_write(FT_ITS_AID, &ptr, end, &error);
		cintx_write(m->ssp.aid, &ptr, end, &error);

		// TODO: support encryption

		cbookmark_apply(bm, &ptr, end, &error);

		// payload
		cint8_write(m->payloadType, &ptr, end, &error);
		// bookmark current position of the payload. this bookmark will be applied in Signature function
		cbookmark_store(bm, &ptr, end, &error);

		m->payload = ptr;
		m->status = 0;
		ret = ptr - buf;
	}
	catch (RuntimeException){
		m->status = E4C_EXCEPTION.err;
	}
	return ret;
}

FITSEC_EXPORT int FitSec_SignMessage(FitSec * e, FSMessageInfo* m, char * buf, int bufsize)
{
	int ret = -1;
	int error;
	char * ptr = m->payload + m->payloadSize;
	const char * end = buf + bufsize;
	unsigned char hash[32];
	FitSecSignature  s = { FS_ECDSA_NISTP256 };

	cbookmark * bm = (cbookmark *)&m->_ptrs[0];
	int trailer_size = 0;
	if (m->si_type != FS_SI_NO_SIGNATURE){
		trailer_size += 67; // signature size;
	}


	try {
		// apply payload size bookmark
		cbookmark_apply(bm, &ptr, end, &error);

		// trailer size 
		cintx_write(trailer_size, &ptr, end, &error);
		if (m->si_type != FS_SI_NO_SIGNATURE){

			cint8_write(FT_SIGNATURE, &ptr, end, &error);

			if (!FitSecHash_Calc(e, s.alg, hash, buf, ptr - buf)){
				error = FSERR_INVALID | FSERR_SIGNATURE;
				throw(RuntimeException, error, "Hash calculation error");
			}

			if (!FitSecSignature_Sign(e, &s, &m->cert->vkey, hash)){
				error = FSERR_INVALID | FSERR_SIGNATURE;
				throw(RuntimeException, error, "Hash calculation error");
			}
			FitSecSignature_Write(e, &s, &ptr, end, &error);
		}
		m->status = 0;
		ret = ptr - buf;
	}
	catch (RuntimeException){
		m->status = E4C_EXCEPTION.err;
	}
	FitSecSignature_Cleanup(e, &s);
	return ret;
}
static FSBOOL FN_THROW(RuntimeException) _FitSec_Verify_V2(FitSec * e, FSMessageInfo * m, const char ** const ptr, const char * end, int * const error);
static FSCertificate * FN_THROW(RuntimeException) _CertificateChain_FromData(FitSec * e, const char * b, const char * end, int * p_error);
static FSCertificate * FN_THROW(RuntimeException) _Certificate_FromData(FitSec * e, const char * b, const char * end, int * p_error);
static FSCertificate * _CheckMessageSigner(FitSec * e, SignerInfo * si, int * const p_error);

FITSEC_EXPORT FSBOOL FitSec_Verify(FitSec * e, FSMessageInfo * m, const void * msg, int msgsize)
{
	FSBOOL ret = FSFALSE;
	const char * ptr = (const char *)msg;
	const char * end = ptr + msgsize;
	switch (ptr[0]){
	case 2:
		ret = _FitSec_Verify_V2(e, m, &ptr, end, &m->status);
		break;
	default:
		m->status = FSERR_INVALID|FSERR_VERSION;
	}
	return ret;
}

static FSBOOL FN_THROW(RuntimeException) _FitSec_Verify_V2(FitSec * e, FSMessageInfo * m, const char ** const ptr, const char * end, int * const p_error)
{
	SignerInfo si;
	RecipientInfo ri;
	FitSecSignature sig;
	EncryptionParameters eparams;
	ThreeDLocation * position = NULL;
	int prev = -1, n;
	const char * m_begin = *ptr;
	const char * m_end;
	const char * h_end, *n_end;
	// skip version
	(*ptr)++;
	try {
		// load headers length
		e4c.err.err = FSERR_HEADERS | FSERR_SIZE;
		n = (int)cintx_read(ptr, end, p_error);
		h_end = *ptr + n;

		// load signer info
		e4c.err.err = FSERR_HEADERS | FSERR_TYPE;
		n = (int)cint8_read(ptr, h_end, p_error);
		if (n != FT_SIGNER_INFO){
			*p_error = FSERR_ORDER | FSERR_HEADERS;
			throw(RuntimeException, *p_error, "Signer info is not the first header");
		}

		e4c.err.err = FSERR_HEADERS ;
		SignerInfo_Read(&si, ptr, h_end, p_error);
		m->si_type = si.type;

		// load other headers
		prev = -1;
		while (*ptr < h_end) {
			e4c.err.err = FSERR_HEADERS;
			n = (int)cint8_read(ptr, h_end, p_error);
			if (n <= prev){
				throw(RuntimeException, FSERR_ORDER, "Headers are duplicated or not in acceding order");
			}
			switch (n){
			case FT_GENERATION_TIME:
				e4c.err.err = FSERR_HEADERS | FSERR_TIME;
				m->generationTime = cint64_read(ptr, h_end, p_error);
				break;
			case FT_GENERATION_TIME_STANDARD_DEVIATION:
				e4c.err.err = FSERR_HEADERS | FSERR_TIME;
				if (prev == FT_GENERATION_TIME){
					throw(RuntimeException, FSERR_ORDER, "Generation time and generation time with standard deviation both exists in the message");
				}
				m->generationTime = cint64_read(ptr, h_end, p_error);
				cint8_read(ptr, h_end, p_error); // skip deviation
				break;
			case FT_EXPIRATION:
				e4c.err.err = FSERR_HEADERS | FSERR_TIME;
				m->expirationTime = cint32_read(ptr, h_end, p_error);
				break;
			case FT_GENERATION_LOCATION:
				e4c.err.err = FSERR_HEADERS | FSERR_THREEDLOCATION;
				ThreeDLocation_Read(&m->position, ptr, h_end, p_error);
				position = &m->position;
				break;
			case FT_REQUEST_UNRECOGNIZED_CERTIFICATE:
				e4c.err.err = FSERR_HEADERS | FSERR_REQUEST_UNRECOGNIZED_CERTIFICATE;
				n = (int)cintx_read(ptr, h_end, p_error);
				HashedId3 id3;
				n_end = *ptr + n;
				if (n_end > h_end){
					throw(RuntimeException, FSERR_PARSEERROR, "Request for unrec cert list is too big");
				}
				// check that my AT cert is in the list
				if (e->currentCert){
					int flags = 0;
					while (*ptr < n_end){
						id3 = HashedId3_Read(ptr, n_end, p_error);
						if (id3 == HashedId8toId3(e->currentCert->digest)){
							flags |= 1;
						}
						else if (id3 == HashedId8toId3(e->currentCert->signer->digest)){
							flags |= 2;
						}
					}
					switch (flags){
					case 1: // AT cert only
						e->requestedCert = e->currentCert;
						break;
					case 2: // AA CERT only
						if (e->cfg->flags & FS_SEND_AA_CERT_IF_BOTH_REQUESTED){
							break;
						}
					case 3: // BOTH
						e->requestedCert = e->currentCert->signer;
						break;
					default:
						break;
					}
				}
				else{
					*ptr = n_end;
				}
				break;
			case FT_ITS_AID:
				e4c.err.err = FSERR_HEADERS | FSERR_AID;
				m->ssp.aid = (FSItsAid)cintx_read(ptr, h_end, p_error);
				memset(m->ssp.u.data, 0, sizeof(m->ssp.u.data));
				break;
			case FT_SIGNER_INFO:
				e4c.err.err = FSERR_HEADERS | FSERR_SIGNER_INFO;
				throw(RuntimeException, FSERR_ORDER, "Signer info is duplicated");
				break;
			case FT_ENCRYPTION_PARAMETERS:
				e4c.err.err = FSERR_HEADERS | FSERR_ENC_PARAMETERS;
				EncryptionParameters_Read(&eparams, ptr, h_end, p_error);
				break;
			case FT_RECIPIENT_INFO:
				e4c.err.err = FSERR_HEADERS | FSERR_RECIPIENT_INFO;
				if (prev != FT_ENCRYPTION_PARAMETERS){
					throw(RuntimeException, FSERR_ORDER , "Encryption parameters must be preceeded before the receipient info");
					break;
				}
				n = (int)cintx_read(ptr, h_end, p_error);
				n_end = *ptr + n;
				if (n_end > h_end){
					throw(RuntimeException, FSERR_PARSEERROR, "Recipient info records size is too long");
				}
				while (*ptr < n_end){
					RecipientInfo_Read(&ri, eparams.sym_alg, ptr, n_end, p_error);
					FSCertificate * c = FitSec_GetMyCertificate(e, ri.cert_id);
					if (c){
						// OK. it is for us. So, Done
						*ptr = n_end;
					}
				}
				break;
			default:
				// skip unknown header
				e4c.err.err = FSERR_HEADERS;
				cstr_read(NULL, ptr, end, p_error);
				break;
			};
		}

		// payload
		e4c.err.err = FSERR_PAYLOAD | FSERR_TYPE;
		m->payloadType = cint8_read(ptr, end, p_error);

		if (m->payloadType == FS_PAYLOAD_SIGNED_EXTERNAL){
			m->payloadSize = 0;
			m->payload = NULL;
		}
		else{
			e4c.err.err = FSERR_PAYLOAD | FSERR_SIZE;
			m->payloadSize = (int)cintx_read(ptr, end, p_error);
			m->payload = (void*)*ptr;
			if (m->payload + m->payloadSize > end){
				throw(RuntimeException, FSERR_INVALID, "Payload size is too big");
			}
			*ptr += m->payloadSize;
		}

		// trailer
		e4c.err.err = FSERR_TRAILER | FSERR_SIZE;
		n = (int)cintx_read(ptr, end, p_error);
		n_end = *ptr + n;
		if (n_end > end){
			throw(RuntimeException, FSERR_INVALID, "Payload size is too big");
		}
		while (*ptr < n_end){
			n = *(*ptr)++;
			switch (n){
			case FT_SIGNATURE:
				// read signature type;
				m_end = *ptr;
				e4c.err.err = FSERR_SIGNATURE;
				FitSecSignature_Read(e, &sig, ptr, end, p_error);
				break;
			default:
				// skip other trailers
				e4c.err.err = FSERR_TRAILER;
				cstr_read(NULL, ptr, end, p_error);
			}
		}
		// message has been read
		*p_error = 0;
	}
	catch (RuntimeException){
		*p_error = E4C_EXCEPTION.err;
	}

	if (*p_error) return FSFALSE;

	// try to find signer certificate, request for the cert if unknown
	m->cert = _CheckMessageSigner(e, &si, p_error);
	if (NULL == m->cert)
		return FSFALSE;

	// check that message is conformed to the certificate
	if (!Certificate_IsValidFor(m->cert, &m->ssp, position, m->generationTime, p_error))
		return FSFALSE;

	// return certificate ssp flags
	const FSItsAidSsp * certSSP = Certificate_GetAidSsp(m->cert, m->ssp.aid);
	memcpy(m->ssp.u.data, certSSP->u.data, sizeof(certSSP->u.data));

	// check Signature
	switch (m->payloadType){
	case FS_PAYLOAD_SIGNED:
	case FS_PAYLOAD_SIGNED_EXTERNAL:
	case FS_PAYLOAD_SIGNED_AND_ENCRYPTED:
		// calculate message hash
		if (sig.signature == NULL){
			// signature was not provided!!!
			*p_error = FSERR_INVALID| FSERR_SIGNATURE;
			return FSFALSE;
		} else{
			unsigned char hash[32];
			FitSecHash_Calc(e, m->cert->vkey.alg, hash, m_begin, m_end - m_begin);
			if (!FitSecSignature_Verify(e, &sig, &m->cert->vkey, hash)){
				*p_error = FSERR_INVALID | FSERR_SIGNATURE;
				return FSFALSE;
			}
		}
	default:
		// do not need to check signature in other cases
		break;
	}
	return FSTRUE;
}

static FSCertificate * _CheckMessageSigner(FitSec * e, SignerInfo * si, int * const p_error)
{
	FSCertificate * at = NULL;

	switch (si->type){
	case FS_SI_SELF:
		*p_error = FSERR_INVALID | FSERR_SIGNER_INFO | FSERR_TYPE;
		break;
	case FS_SI_DIGEST:
		// looking for the cert
		at = CertificateHash_Find(e->certs, si->u.digest);
		if (at == NULL){
			// unknown certificate
			// message from unknown station has been received
			at = Certificate_New(e);
			at->subjectType = AUTHORIZATION_TICKET;
			Certificate_SetDigest(at, si->u.digest);
			CertificateHash_Add(e->certs, at);
			// add this certificate to the list for request
			if (e->cfg->flags & FS_USE_CERT_REQUEST) {
				cring_enqueue(&e->unknown, Certificate_Retain(at));
			}
			Certificate_Release(at);
			e->newNeighbourFlag = 1; // set to send our cert in next cam
			*p_error = FSERR_UNKNOWN | FSERR_SIGNER;
			return NULL;
		}
		else if (at->data == NULL) {
			// request it again
			if (e->cfg->flags & FS_USE_CERT_REQUEST) {
				cring_enqueue(&e->unknown, Certificate_Retain(at));
			}
			*p_error = FSERR_UNKNOWN | FSERR_SIGNER;
			return NULL;
		}
		else if (FS_CERT_IS_INVALID(at)){
			*p_error = FSERR_INVALID | FSERR_SIGNER;
			return NULL;
		}
		break;
	case FS_SI_CERTIFICATE:
		// calc cert hash
		at = _Certificate_FromData(e, si->u.data.beg, si->u.data.end, p_error);
		if (at == NULL) return NULL;
		if (NULL == at->signer){
			// unknown AA cert
			FSCertificate * aa = Certificate_New(e);
			aa->subjectType = AUTHORIZATION_AUTHORITY;
			Certificate_SetDigest(aa, at->signer_digest);
			CertificateHash_Add(e->certs, aa);
			at->signer = aa;
		}
		if (NULL == at->signer->data){
			// request for unrecognized aa
			if (e->cfg->flags & FS_REQUEST_AT_CERT_WITH_AA) {
				cring_enqueue(&e->unknown, Certificate_Retain(at));
			}
			cring_enqueue(&e->unknown, Certificate_Retain(at->signer));
		}
		if (!Certificate_ValidateChain(at, p_error))
			return NULL;
		break;
	
	case FS_SI_CERTIFICATE_CHAIN:
		// Check AA cert
		at = _CertificateChain_FromData(e, si->u.data.beg, si->u.data.end, p_error);
		break;
	default:
		*p_error = FSERR_INVALID| FSERR_SIGNER_INFO | FSERR_TYPE;
	}
	return at;
}

static FSCertificate * _Certificate_FromData(FitSec * e, const char * b, const char * end, int * p_error)
{
	FSCertificate *c, *cnew = NULL;
	HashedId8 digest;
	unsigned char hash[32];

	// calc cert hash
	CalculateCertificateHash(e, FS_ECDSA_NISTP256, b, end - b, hash, p_error);
	digest = toHashedId8(hash + 24);
	c = CertificateHash_Find(e->certs, digest);
	if (c == NULL){
		// message from unknown station has been received
		c = cnew = Certificate_New(e);
		e->newNeighbourFlag = 1;
	}
	if (c->data == NULL){
		// message from unknown station has been received
		try {
			Certificate_SetData(c, b, end - b, hash, p_error);
			if (cnew) CertificateHash_Add(e->certs, cnew);
			CertificateHash_Relink(e->certs, c);
		}
		catch (RuntimeException){
			// oups. certificate loading error.
			c = NULL;
		}
	}
	Certificate_Release(cnew);
	return c;
}

static FSCertificate * _CertificateChain_FromData(FitSec * e, const char * b, const char * end, int * p_error)
{

	const char * ptr = b;
	const char * p_at;
	FSCertificate *at;

	try {
		// skip AA cert
		Certificate_Read(NULL, &ptr, end, p_error);
		p_at = ptr;

		// read AT cert first
		at = _Certificate_FromData(e, p_at, ptr, p_error);
		if (at){
			if (!Certificate_ValidateChain(at, p_error)){
				// read AA cert
				FSCertificate * aa = _Certificate_FromData(e, b, p_at, p_error);
				if (aa == NULL){
					*p_error = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_SIGNER;
					throw(RuntimeException, *p_error, "AA FSCertificate read error");
				}
				if (!Certificate_ValidateChain(aa, p_error)){
					*p_error = FSERR_INVALID | FSERR_CERTIFICATE | FSERR_SIGNER;
					throw(RuntimeException, *p_error, "Invalid AA certificate");
				}
				if (at->signer_digest == aa->digest){
					if (at->signer != aa){
						Certificate_Release(at->signer);
						at->signer = Certificate_Retain(aa);
					}
					if (!Certificate_ValidateChain(at, p_error)){
						at = NULL;
					}
				}
			}
		}
	}
	catch (RuntimeException){
		at = NULL;
	}
	return at;
}
