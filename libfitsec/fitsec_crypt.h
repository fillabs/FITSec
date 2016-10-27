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

#ifndef fitsec_crypt_h
#define fitsec_crypt_h

#ifdef __cplusplus
extern "C" {
#endif
	typedef struct FSCryptKey FSCryptKey;
	typedef struct FSCryptSignature FSCryptSignature;
	typedef struct FSCryptHash FSCryptHash;
	typedef struct FSCryptEngine FSCryptEngine;
	typedef struct FSCryptEngineConfig FSCryptEngineConfig;

	typedef enum {
		FS_SHA256,
		FS_SHA224,
		
		FSCryptHashAlgorithm_Max
	}FSCryptHashAlgorithm;

	typedef enum {
		FS_ECDSA_NISTP256,
		FS_ECIES_NISTP256,
		
		FSCryptPKAlgorithm_Max
	}FSCryptPKAlgorithm;

	typedef enum {
		FS_AES_128_CCM
	}FSCryptSymmAlgorithm;

	typedef enum {
		FS_X_COORDINATE_ONLY = 0,
		FS_COMPRESSED_LSB_Y_0 = 2,
		FS_COMPRESSED_LSB_Y_1 = 3,
		FS_UNCOMPRESSED = 4
	}FitSecEccPointType;

	typedef struct FSCryptHashConfig {
		FSCryptHash*    (*New)     (FSCryptHashAlgorithm alg);
		void            (*Free)    (FSCryptHash * c);
		unsigned char * (*Calc)    (FSCryptHashAlgorithm alg, unsigned char * hash, const void * ptr, int length);
		int             (*Init)    (FSCryptHash * h);
		void            (*Update)  (FSCryptHash * h, const void * ptr, int length);
		void            (*Finalize)(FSCryptHash * h, unsigned char * hash);
	}FSCryptHashConfig;

	typedef struct FSCryptKeyConfig {
		FSCryptKey *    (*Read)      (FSCryptEngine * c, FSCryptPKAlgorithm alg, FSCryptSymmAlgorithm sym, int type, const unsigned char * x, const unsigned char * y);
		int             (*SetPrivate)(FSCryptKey * k, const unsigned char * value, int vlength);
		void            (*Free)      (FSCryptKey * k);
	}FSCryptKeyConfig;

	typedef struct FSCryptSignatureConfig {
		FSCryptSignature *	(*Read)   (FSCryptEngine * c, const unsigned char * s, const unsigned char * rx, FitSecEccPointType type);
		FSCryptSignature *	(*Sign)   (FSCryptEngine * c, const FSCryptKey * key, const unsigned char * hash);
		void                (*Write)  (FSCryptSignature* t, unsigned char * s, unsigned char * rx, FitSecEccPointType * type);
		int                 (*Verify) (FSCryptSignature* t, const FSCryptKey * key, const unsigned char * hash);
		void                (*Free)   (FSCryptSignature* t);
	}FSCryptSignatureConfig;

	struct FSCryptEngineConfig {
		FSCryptEngine * (*New) (const FSCryptEngineConfig * cfg, void * param);
		void            (*Free)(FSCryptEngine*);
		const FSCryptHashConfig      * Hash;
		const FSCryptKeyConfig       * Key;
		const FSCryptSignatureConfig * Signature;
	};

#ifdef FITSEC_HAVE_OPENSSL
	FITSEC_EXPORT const FSCryptEngineConfig * FSCryptEngineConfig_OpenSSL();
	#define FSCryptEngineConfig_Default FSCryptEngineConfig_OpenSSL
#endif

#ifdef __cplusplus
}
#endif
#endif
