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

#ifndef fitsec_types_h
#define fitsec_types_h
#include "cserialize.h"

typedef uint64_t HashedId8;
typedef uint32_t HashedId3;

typedef struct {
	FitSecEccPointType type;
	uint8_t	x[32];
	uint8_t	y[32];
} EccPoint;

typedef struct SignerInfo {
	FSSignerInfoType type;
	union {
		HashedId8 digest;
		struct {
			const char * beg;
			const char * end;
		}data;
	}u;
}SignerInfo;

typedef struct {
	FSCryptSymmAlgorithm	sym_alg;
	union {
		struct {
			uint8_t  nonce[12];
		}aes128ccm;
	}u;
} EncryptionParameters;

typedef struct {
	FSCryptPKAlgorithm    alg;
	FSCryptSymmAlgorithm  symm_alg;
	EccPoint              public_key;
} PublicKey;

typedef struct EciesEncryptedKey {
	EccPoint      v;
	unsigned char c[16];
	unsigned char t[16];
} EciesEncryptedKey;

typedef struct RecipientInfo {
	HashedId8			cert_id;
	FSCryptPKAlgorithm	pk_encryption;
	union {
		EciesEncryptedKey	ecies_key;
	}u;
}RecipientInfo;

typedef struct {
	EccPoint r;
	uint8_t  s[32];
} EcdsaSignature;

typedef struct {
	FSCryptPKAlgorithm	algorithm;
	EcdsaSignature ecdsa;
} Signature;

typedef struct {
	int32_t	 latitude;
	int32_t	 longitude;
} TwoDLocation;

typedef FSLocation  ThreeDLocation;
typedef struct {
	TwoDLocation center;
	uint32_t     radius;
} CircularRegion;

typedef struct {
	int count;
	struct {
		TwoDLocation nw;
		TwoDLocation se;
	}r[6];
} RectangularRegion;

typedef struct {
	int count;
	TwoDLocation points[12];
} PolygonalRegion;

typedef enum {
	RDICT_ISO_3166_1,
	RDICT_UN_STATS,
} RegionDictionary;

typedef struct {
	RegionDictionary dictionary;
	uint16_t         identifier;
	int	             local;
} IdentifiedRegion;

typedef enum {
	REGION_NONE,
	REGION_CIRCLE,
	REGION_RECTANGLE,
	REGION_POLYGON,
	REGION_ID,
} RegionType;

typedef struct {
	RegionType				  type;
	union {
		CircularRegion		  circular;
		RectangularRegion     rectangular;
		PolygonalRegion       polygonal;
		IdentifiedRegion	  identified;
	}u;
} GeographicRegion;

typedef enum {
	FT_GENERATION_TIME = 0,
	FT_GENERATION_TIME_STANDARD_DEVIATION = 1,
	FT_EXPIRATION = 2,
	FT_GENERATION_LOCATION = 3,
	FT_REQUEST_UNRECOGNIZED_CERTIFICATE = 4,
	FT_ITS_AID = 5,
	FT_SIGNER_INFO = 128,
	FT_ENCRYPTION_PARAMETERS = 129,
	FT_RECIPIENT_INFO = 130,
} HeaderFieldType;

typedef enum {
	FT_SIGNATURE = 1,
} TrailerFieldType;

typedef enum {
	ENROLLMENT_CREDENTIAL = 0,
	AUTHORIZATION_TICKET = 1,
	AUTHORIZATION_AUTHORITY = 2,
	ENROLLMENT_AUTHORITY = 3,
	ROOT_CA = 4,
	CRL_SIGNER = 5,
} SubjectType;

typedef enum {
	SA_VERIFICATION_KEY = 0,
	SA_ENCRYPTION_KEY = 1,
	SA_ASSURANCE_LEVEL = 2,
	SA_RECONSTRUCTION_VALUE = 3,
	SA_ITS_AID_LIST = 32,
	SA_ITS_AID_SSP_LIST = 33,
} SubjectAttributeType;

typedef uint8_t SubjectAssurance;

typedef enum {
	VR_TIME_END = 0,
	VR_TIME_START_AND_END = 1,
	VR_TIME_START_AND_DURATION = 2,
	VR_REGION = 3,
} ValidityRestrictionType;

#endif
