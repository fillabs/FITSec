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
#include "khash.h"
#include "cmem.h"

#include<stdio.h>
#define __STDC_FORMAT_MACROS
#include<inttypes.h>

KHASH_MAP_INIT_INT64(certs, FSCertificate*)

struct CertificateHash {
	khash_t(certs) h;
};

CertificateHash * CertificateHash_New() {
	return (CertificateHash *)kh_init(certs);
}

void CertificateHash_Free(CertificateHash * ch)
{
	khash_t(certs) * h = (khash_t(certs)*)ch;
	if (h) {
		// release all certificates
		FSCertificate * c;
		kh_foreach_value(h, c, Certificate_Release(c));
		kh_destroy(certs, h);
	}
}

void CertificateHash_Clear(CertificateHash * ch)
{
	khash_t(certs) * h = (khash_t(certs)*)ch;
	if (h) {
		FSCertificate * c;
		kh_foreach_value(h, c, Certificate_Release(c));
		kh_clear(certs, h);
	}
}

void CertificateHash_Purge(CertificateHash * ch)
{
	khash_t(certs) * h = (khash_t(certs)*)ch;
	if (h) {
		khint_t k;
		FSCertificate * c;
		for (k = kh_begin(h); k != kh_end(h); ++k) {
			if (!kh_exist(h, k)) continue;
			c = kh_val(h, k);
			if (0 == (c->flags & CERT_LOCAL)){
				kh_del(certs, h, k);
				Certificate_Release(c);
			}
		}
	}
}

FSCertificate *     CertificateHash_Add    (CertificateHash *ch, FSCertificate *c)
{
	khash_t(certs) * h = (khash_t(certs)*)ch;
	if (h) {
		khiter_t k;
		int ret;
		k = kh_put(certs, h, c->digest, &ret);
		if(ret >= 0){
			if(ret > 0){
				kh_value(h, k) = Certificate_Retain(c);
			}
			return kh_value(h, k);
		}
	}
	return NULL;
}

FSCertificate *     CertificateHash_Delete (CertificateHash * ch, HashedId8 id8)
{
	FSCertificate * c = NULL;
	khash_t(certs) * h = (khash_t(certs)*)ch;
	if (h) {
		khiter_t k;
		k = kh_get(certs, h, id8);
		if(k != kh_end(h)){
			c = kh_value(h, k);
			kh_del(certs, h, k);
		}
	}
	return c;
}

FSCertificate *     CertificateHash_Find   (CertificateHash * ch, HashedId8 id8)
{
	FSCertificate * c = NULL;
	khash_t(certs) * h = (khash_t(certs)*)ch;
	if (h) {
		khiter_t k;
		k = kh_get(certs, h, id8);
		if(k != kh_end(h)){
			c = kh_value(h, k);
		}
	}
	return c;
}

FSBOOL CertificateHash_Relink(CertificateHash * ch, FSCertificate * c)
{
	if (NULL == c->signer){
		if (NULL == c->data) return FSFALSE;
		c->signer = Certificate_Retain(CertificateHash_Find(ch, c->signer_digest));
		if (NULL == c->signer) return FSFALSE;
	}
	if (c->signer == c) return FSTRUE;
	return CertificateHash_Relink(ch, c->signer);
}

void CertificateHash_RelinkSigners(CertificateHash * ch)
{
	int error;
	khash_t(certs) * h = (khash_t(certs)*)ch;
	if (h) {
		khint_t k;
		for (k = kh_begin(h); k != kh_end(h); ++k) {
			if (kh_exist(h, k)) {
				FSCertificate * c = kh_value(h, k);
				if (!c->signer){
					khint_t j = kh_get(certs, h, c->signer_digest);
					if (j != kh_end(h)){
						c->signer = kh_value(h, j);
					}
					else{
						fprintf(stderr, "%016" PRIX64 ":\t Signer %016" PRIX64 " not found\n", c->digest, c->signer_digest);
					}
				}
			}
		}
		for (k = kh_begin(h); k != kh_end(h); ++k) {
			if (kh_exist(h, k)) {
				FSCertificate * c = kh_value(h, k);
				if (!Certificate_ValidateChain(c, &error)){
					fprintf(stderr, "%016" PRIX64 ":\t Certificate Chain is not valid\n", c->digest);
				}
			}
		}
	}
}
