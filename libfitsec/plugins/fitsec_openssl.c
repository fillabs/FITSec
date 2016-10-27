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

#include <openssl/evp.h>
//#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <assert.h>

#include "../fitsec_crypt.h"
#include <string.h>

#if defined(WIN32) && !defined(FITSEC_EXPORT)
#ifdef LIBFITSEC_EXPORTS
#define FITSEC_EXPORT __declspec(dllexport)
#else
#define FITSEC_EXPORT __declspec(dllimport)
#endif
#else
#define FITSEC_EXPORT
#endif


static	FSCryptEngine *  OpenSSL_New (const FSCryptEngineConfig * cfg, void * param);
static	void			 OpenSSL_Free(FSCryptEngine*);

static	FSCryptHash *     OpenSSL_HashNew(FSCryptHashAlgorithm alg);
static	void             OpenSSL_HashFree(FSCryptHash * c);
static	unsigned char *  OpenSSL_HashCalc(FSCryptHashAlgorithm alg, unsigned char * hash, const void * ptr, int length);
static	int              OpenSSL_HashInit(FSCryptHash * h);
static	void             OpenSSL_HashUpdate(FSCryptHash * h, const void * ptr, int length);
static	void             OpenSSL_HashFinalize(FSCryptHash * h, unsigned char * hash);

static FSCryptKey *	     OpenSSL_KeyRead(FSCryptEngine * c, FSCryptPKAlgorithm alg, FSCryptSymmAlgorithm sym, int type, const unsigned char * x, const unsigned char * y);
static int               OpenSSL_SetPrivateKey(FSCryptKey * k, const unsigned char * value, int vlength);
static void              OpenSSL_KeyFree(FSCryptKey * k);

static FSCryptSignature * OpenSSL_ReadSignature(FSCryptEngine * c, const unsigned char * s, const unsigned char * rx, FitSecEccPointType y);
static FSCryptSignature * OpenSSL_Sign(FSCryptEngine * c, const FSCryptKey * key, const unsigned char * hash);
static void               OpenSSL_WriteSignature(FSCryptSignature* t, unsigned char * s, unsigned char * rx, FitSecEccPointType * y);
static int                OpenSSL_VerifySignature(FSCryptSignature* t, const FSCryptKey * key, const unsigned char * hash);
static void               OpenSSL_FreeSignature(FSCryptSignature* t);

static 	FSCryptHashConfig _openssl_hash_config = {
	OpenSSL_HashNew,
	OpenSSL_HashFree,
	OpenSSL_HashCalc,
	OpenSSL_HashInit,
	OpenSSL_HashUpdate,
	OpenSSL_HashFinalize,
};

static FSCryptKeyConfig _openssl_key_config = {
	OpenSSL_KeyRead,
	OpenSSL_SetPrivateKey,
	OpenSSL_KeyFree
};

static FSCryptSignatureConfig _openssl_signature_config = {
	OpenSSL_ReadSignature,
	OpenSSL_Sign,
	OpenSSL_WriteSignature,
	OpenSSL_VerifySignature,
	OpenSSL_FreeSignature
};

static FSCryptEngineConfig _openssl_cfg = {
	OpenSSL_New, OpenSSL_Free,
	&_openssl_hash_config,
	&_openssl_key_config,
	&_openssl_signature_config
};

FITSEC_EXPORT const FSCryptEngineConfig * FSCryptEngineConfig_OpenSSL()
{
	return &_openssl_cfg;
}

/*************************************************************************************************/
typedef struct SHAConfig {
	unsigned char* (*Calc)  (const unsigned char *d, size_t n, unsigned char *md);
	int            (*Init)  (SHA256_CTX *c);
	int			   (*Update)(SHA256_CTX *c, const void *data, size_t len);
	int            (*Final) (unsigned char *md, SHA256_CTX *c);
}SHAConfig;

static SHAConfig _sha_cfg[FSCryptHashAlgorithm_Max] = {
	{
		SHA256,
		SHA256_Init,
		SHA256_Update,
		SHA256_Final
	}, {
		SHA224,
		SHA224_Init,
		SHA224_Update,
		SHA224_Final
	}
};

struct FSCryptHash {
	const SHAConfig * cfg;
	SHA256_CTX ctx;
};

static	FSCryptHash *    OpenSSL_HashNew(FSCryptHashAlgorithm alg) {
	FSCryptHash * h = malloc(sizeof(FSCryptHash));
	h->cfg = &_sha_cfg[alg];
	h->cfg->Init(&h->ctx);
	return h;
}
static	void            OpenSSL_HashFree(FSCryptHash * c) {
	free(c);
}
static	unsigned char *  OpenSSL_HashCalc(FSCryptHashAlgorithm alg, unsigned char * hash, const void * ptr, int length) {
	return _sha_cfg[alg].Calc(ptr, length, hash);
}
static	int             OpenSSL_HashInit(FSCryptHash * h) {
	return h->cfg->Init(&h->ctx);
}
static	void            OpenSSL_HashUpdate(FSCryptHash * h, const void * ptr, int length) {
	h->cfg->Update(&h->ctx, ptr, length);
}
static	void            OpenSSL_HashFinalize(FSCryptHash * h, unsigned char * hash) {
	SHA256_CTX c;
	memcpy(&c, &h->ctx, sizeof(SHA256_CTX));
	h->cfg->Final(hash, &c);
}

static int _nids[FSCryptPKAlgorithm_Max] = {
	NID_X9_62_prime256v1,
	NID_X9_62_prime256v1
};

struct FSCryptEngine {
	int _rcntr;
	EC_GROUP *    groups[FSCryptPKAlgorithm_Max];
};

static FSCryptEngine _engine = { 0 };
static	FSCryptEngine * OpenSSL_New(const FSCryptEngineConfig * cfg, void * param)
{
	int f = _engine._rcntr++;
	if (f == 0){
		int i;
		for (i = 0; i < FSCryptPKAlgorithm_Max; i++){
			_engine.groups[i] = EC_GROUP_new_by_curve_name(_nids[i]);
		}
	}
	return &_engine;
}

static	void OpenSSL_Free(FSCryptEngine* e)
{
	int f = --_engine._rcntr;
	if (f == 0){
		int i;
		for (i = 0; i < FSCryptPKAlgorithm_Max; i++){
			EC_GROUP_free(_engine.groups[i]);
		}
	}
}

static FSCryptKey *	OpenSSL_KeyRead(FSCryptEngine * c, FSCryptPKAlgorithm alg, FSCryptSymmAlgorithm sym, int type, const unsigned char * x, const unsigned char * y)
{
	EC_KEY * k;
	const EC_GROUP * group;
	EC_POINT * pnt;
	BIGNUM *bnx, *bny;

	group = c->groups[alg];

	k = EC_KEY_new();
	EC_KEY_set_group(k, group);

	pnt = EC_POINT_new(group);

	bnx = BN_new(); BN_bin2bn(x, 32, bnx);

	switch (type){
	case FS_X_COORDINATE_ONLY:
	case FS_COMPRESSED_LSB_Y_0:
	case FS_COMPRESSED_LSB_Y_1:
		EC_POINT_set_compressed_coordinates_GFp(group, pnt, bnx, ((type & 2) ? 1 : 0), NULL);
		break;
	case FS_UNCOMPRESSED:
		bny = BN_new(); BN_bin2bn(y, 32, bny);
		EC_POINT_set_affine_coordinates_GFp(group, pnt, bnx, bny, NULL);
		BN_clear_free(bny);
		break;
	}
	BN_clear_free(bnx);
	EC_KEY_set_public_key(k, pnt);
	EC_POINT_free(pnt);
	return (FSCryptKey*)k;
}

static int  OpenSSL_SetPrivateKey(FSCryptKey * _k, const unsigned char * value, int vlength)
{
	EC_KEY * k = (EC_KEY *)_k;
	BIGNUM * p = BN_new();
	BN_bin2bn(value, vlength, p);
	EC_KEY_set_private_key(k, p);
	BN_clear_free(p);
	return EC_KEY_check_key(k);
}

static void OpenSSL_KeyFree(FSCryptKey * k)
{
	EC_KEY_free((EC_KEY *)k);
}


static FSCryptSignature * OpenSSL_ReadSignature(FSCryptEngine * c, const unsigned char * s, const unsigned char * rx, FitSecEccPointType type)
{
	// ignore type
	ECDSA_SIG * sg = ECDSA_SIG_new();
	BN_bin2bn(s, 32, sg->s);
	BN_bin2bn(rx, 32, sg->r);
	return (FSCryptSignature*)sg;
}

static FSCryptSignature * OpenSSL_Sign(FSCryptEngine * c, const FSCryptKey * key, const unsigned char * hash)
{
	return (FSCryptSignature*)ECDSA_do_sign(hash, 32, (EC_KEY*)key);
}

static void OpenSSL_FreeSignature(FSCryptSignature* t)
{
	ECDSA_SIG_free((ECDSA_SIG*)t);
}

static void OpenSSL_WriteSignature(FSCryptSignature* t, unsigned char * s, unsigned char * rx, FitSecEccPointType * type)
{
	ECDSA_SIG * ecdsa = (ECDSA_SIG*)t;
	BN_bn2bin(ecdsa->r, rx);
	BN_bn2bin(ecdsa->s, s);
	if (type)*type = 0; // uncompressed
}

static int OpenSSL_VerifySignature(FSCryptSignature* t, const FSCryptKey * key, const unsigned char * hash)
{
	return ECDSA_do_verify(hash, 32, (ECDSA_SIG*)t, (EC_KEY*)key);
}
