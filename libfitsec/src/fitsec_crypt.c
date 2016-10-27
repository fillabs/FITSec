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
FSCryptEngine * FitSecCrypt_New (FitSec * e, void * param)
{
	return e->cfg->crypt->New(e->cfg->crypt, param);
}

void FitSecCrypt_Free(FitSec * e, FSCryptEngine* c)
{
	e->cfg->crypt->Free(c);
}

FSBOOL FitSecHash_Calc(FitSec * e, FSCryptPKAlgorithm alg, unsigned char * hash, const void * data, size_t len)
{
	if (e->cfg->crypt->Hash->Calc)
		return e->cfg->crypt->Hash->Calc(alg, hash, data, len) ? FSTRUE : FSFALSE;
	else {
		FSCryptHash * h = e->cfg->crypt->Hash->New(alg);
		if (h){
			e->cfg->crypt->Hash->Update(h, data, len);
			e->cfg->crypt->Hash->Finalize(h, hash);
			e->cfg->crypt->Hash->Free(h);
			return FSTRUE;
		}
	}
	return FSFALSE;
}

/*
FSCryptHash *	FitSecHash_New(FitSec * e, FSCryptPKAlgorithm alg)
{
	return e->cfg->crypt->Hash->New(alg);
}
void FitSecHash_Free(FitSec * e, FitSecHash * h)
{
	e->cfg->crypt->Hash->Free(h);
}
int FitSecHash_Init(FitSec * e, FitSecHash * h)
{
	return e->cfg->crypt->Hash->Init(h);
}
void FitSecHash_Update(FitSec * e, FitSecHash * h, const void * data, size_t len)
{
	e->cfg->crypt->Hash->Update(h, data, len);
}
void FitSecHash_Finalize(FitSec * e, FitSecHash * h, unsigned char * hash)
{
	e->cfg->crypt->Hash->Finalize(h, hash);
}
*/