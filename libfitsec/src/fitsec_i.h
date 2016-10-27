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

#ifndef fitsec_i_h
#define fitsec_i_h
#include "../fitsec.h"
#include "fitsec_types.h"
#include "fitsec_error.h"
#include "cring.h"
#include "cmem.h"
#include "cserialize.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_PROTOCOL_VERSION 2
#define FITSEC_AID_CAM  36
#define FITSEC_AID_DENM 37
#define MAX_AID_COUNT  16  // maximum 16 AID/SSP items per certificate

	HashedId3 HashedId8toId3(HashedId8 id8);
	HashedId8 toHashedId8(const unsigned char * buf);
	HashedId3 toHashedId3(const unsigned char * buf);

	enum {
		CERT_LOADED  = 1,
		CERT_INVALID = 2,
		CERT_TRUSTED = 4,
		CERT_LOCAL   = 8,
	};
#define FS_CERT_IS_INVALID(C) ((C) && ((C)->flags&CERT_INVALID)) 
#define FS_CERT_IS_TRUSTED(C) ((C) && ((C)->flags&CERT_TRUSTED)) 
#define FS_CERT_IS_NEW(C) ((C) && 0 == ((C)->flags&(CERT_TRUSTED|CERT_INVALID))) 

	/* Crypto functions */
	FSCryptEngine * FitSecCrypt_New(FitSec * e, void * param);
	void            FitSecCrypt_Free(FitSec * e, FSCryptEngine* c);

	typedef struct FitSecKey {
		FSCryptPKAlgorithm   alg;
		FSCryptSymmAlgorithm sym;
		int                  size;
		FSCryptKey         * key;
	} FitSecKey;	
	void  FN_THROW(RuntimeException) FitSecKey_Read(FitSec * e, FitSecKey * const k, const char ** const ptr, const char * const end, int * const error);
	void  FN_THROW(RuntimeException) FitSecKey_Write(FitSec * e, const FitSecKey * const k, char ** const ptr, const char * const end, int * const error);
	FSBOOL  FitSecKey_SetPrivateKey(FitSec * e, FitSecKey * k, const char * value, size_t vlength);
	void    FitSecKey_Cleanup(FitSec * e, FitSecKey * const k);

	typedef struct FitSecSignature {
		FSCryptPKAlgorithm   alg;
		int                 size;
		FSCryptSignature  * signature;
	} FitSecSignature;

	void FN_THROW(RuntimeException) FitSecSignature_Read(FitSec * e, FitSecSignature * const k, const char ** const ptr, const char * const end, int * const error);
	void FN_THROW(RuntimeException) FitSecSignature_Write(FitSec * e, const FitSecSignature * const k, char ** const ptr, const char * const end, int * const error);
	void FN_THROW(RuntimeException) FitSecSignature_Cleanup(FitSec * e, FitSecSignature * const k);
	FSBOOL FitSecSignature_Sign(FitSec * e, FitSecSignature * const s, const FitSecKey * k, const unsigned char * hash);
	FSBOOL FitSecSignature_Verify(FitSec * e, FitSecSignature * const s, const FitSecKey * k, const unsigned char * hash);

	typedef FSCryptHash FitSecHash;
	FSBOOL	FitSecHash_Calc(FitSec * e, FSCryptPKAlgorithm alg, unsigned char * hash, const void * data, size_t len);
	/*

	FSCryptHash *	FitSecHash_New(FitSec * e, FSCryptPKAlgorithm alg);
	void			FitSecHash_Free(FitSec * e, FitSecHash * h);
	int  			FitSecHash_Init(FitSec * e, FitSecHash * h);
	void			FitSecHash_Update(FitSec * e, FitSecHash * h, const void * data, size_t len);
	void			FitSecHash_Finalize(FitSec * e, FitSecHash * h, unsigned char * hash);
	*/
	struct FSCertificate {
		cring_t   ring;  // ring to be added to my_certs or unknown_certs
		int      _rcntr; // retain/release counter

		FitSec         * e;
		int              flags;

		uint8_t          hash[32];
		HashedId8        digest;
		FSCertificate  * signer;
		HashedId8        signer_digest;

		SubjectType		 subjectType;
		char			 subjectName[32];

		uint32_t         start_time;
		uint32_t         end_time;
		GeographicRegion region;

		EccPoint         recValue;
		SubjectAssurance assurance;
		int              aidsspcount;
		FSItsAidSsp      aidssp[MAX_AID_COUNT];

		FitSecKey        vkey;
		FitSecKey        ekey;

		FitSecSignature  signature;

		unsigned int     data_size;
		char *           data;
	};
	FSCertificate * Certificate_New(FitSec * e);
//	void          Certificate_Free(FSCertificate * c);
	void          Certificate_Release(FSCertificate * c);
	FSCertificate*  Certificate_Retain(FSCertificate * c);

	void FN_THROW(RuntimeException) Certificate_Read(FSCertificate * c, const char** const ptr, const char* const end, int * const perror);
	void FN_THROW(RuntimeException) Certificate_SetData(FSCertificate * c, const char * data, int data_size, const unsigned char * hash, int * const perror);
	void         Certificate_SetDigest(FSCertificate * c, HashedId8 digest);
	FSBOOL        Certificate_SetPrivateKey(FSCertificate * c, int type, const char * pkdata, int pksize);

	FSBOOL        Certificate_Validate(FSCertificate * c, int *error);
	FSBOOL        Certificate_ValidateChain(FSCertificate * c, int *error);

	FSBOOL        CalculateCertificateHash(FitSec * e, FSCryptPKAlgorithm alg, const char * ptr, int size, unsigned char * hash, int * const perror);
	FSBOOL        Certificate_IsValidForTime(const FSCertificate * c, Time64 time, int * const perror);
	FSBOOL        Certificate_IsValidForPosition(const FSCertificate * c, const FSLocation * position, int * const perror);
	FSBOOL        Certificate_IsValidForSSP(const FSCertificate * c, const FSItsAidSsp * ssp, int * const perror);
	FSBOOL        Certificate_IsValidFor(const FSCertificate * c, const FSItsAidSsp * ssp, const FSLocation * position, Time64 time, int * const perror);
	const FSItsAidSsp *   Certificate_GetAidSsp(const FSCertificate * c, FSItsAid aid);
	
	FSBOOL        AIDSSP_Match(const FSItsAidSsp * mask, const FSItsAidSsp * ssp);

	typedef struct CertificateHash CertificateHash;
	CertificateHash * CertificateHash_New    ();
	void              CertificateHash_Free   (CertificateHash *);
	void              CertificateHash_Clear  (CertificateHash *);
	void              CertificateHash_Purge  (CertificateHash *);
	FSCertificate *   CertificateHash_Add    (CertificateHash *, FSCertificate *);
	FSCertificate *   CertificateHash_Delete (CertificateHash *, HashedId8);
	FSCertificate *   CertificateHash_Find   (CertificateHash *, HashedId8);
	FSBOOL            CertificateHash_Relink (CertificateHash *, FSCertificate *);
	void              CertificateHash_RelinkSigners(CertificateHash *);

	struct FitSec
	{
		const FitSecConfig * cfg;
		FSCryptEngine      * crypt;
		unsigned int         version;
		unsigned int         error;
		CertificateHash *    certs;
		FSCertificate *      currentCert;

		cring_t              mycerts;
		cring_t              trusted;
		cring_t              unknown;
		cring_t              pool;

		FSCertificate *      requestedCert;
		int                  newNeighbourFlag;
		Time64               nextCertTime;
	};

	FSCertificate * FitSec_SelectMyCertificate(FitSec * e, const FSItsAidSsp * ssp, const FSLocation * position, Time64 time);

	void FN_THROW(RuntimeException) SignerInfo_Read(SignerInfo * si, const char** const ptr, const char* end, int * const p_error);
	void FN_THROW(RuntimeException) EccPoint_Read(EccPoint * p, const char** const ptr, const char* const end, int * const p_error);
	void FN_THROW(RuntimeException) GeographicRegion_Read(GeographicRegion * r, const char** const ptr, const char* const end, int * const p_error);
	void FN_THROW(RuntimeException) TwoDLocation_Read(TwoDLocation * l, const char** const ptr, const char* const end, int * const p_error);
	void FN_THROW(RuntimeException) ThreeDLocation_Write(ThreeDLocation * l, char** const ptr, const char* const end, int * const p_error);
	void FN_THROW(RuntimeException) ThreeDLocation_Read(ThreeDLocation * l, const char** const ptr, const char* const end, int * const p_error);
	HashedId3 FN_THROW(RuntimeException) HashedId3_Read(const char** const ptr, const char* const end, int * const p_error);
	void FN_THROW(RuntimeException)  HashedId3_Write(HashedId3 id3, char** const ptr, const char* const end, int * const p_error);
	void FN_THROW(RuntimeException)  EncryptionParameters_Read(EncryptionParameters * ep, const char** const ptr, const char* end, int * const p_error);
	void FN_THROW(RuntimeException)  RecipientInfo_Read(RecipientInfo * ri, FSCryptSymmAlgorithm sym, const char** const ptr, const char* end, int * const p_error);
	void FN_THROW(RuntimeException)  EciesEncryptedKey_Read(EciesEncryptedKey * k, FSCryptSymmAlgorithm sym, const char** const ptr, const char* end, int * const p_error);


	FSBOOL GeographicRegion_IsRegionInside(const GeographicRegion * r, const GeographicRegion * s);
	FSBOOL GeographicRegion_IsPointInside(const GeographicRegion * r, const TwoDLocation * l);


#ifdef __cplusplus
}
#endif
#endif