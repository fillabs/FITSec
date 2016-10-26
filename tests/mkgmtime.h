/*  lib/mkgmtime.h

    Copyright (C) 2010 DeSmuME team

    This file is part of DeSmuME

    DeSmuME is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DeSmuME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DeSmuME; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#ifndef _MKGMTIME_H_
#define _MKGMTIME_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

	extern time_t mkgmtime(struct tm *tim_p);
	extern time_t mktaitime(struct tm *tim_p);
	extern time_t addleapseconds(time_t t);
	extern unsigned long mkitstime32(struct tm *tim_p);
	extern unsigned long unix2itstime32(time_t t);
	extern const char * stritsdate32(time_t t);
	extern const char * strtaidate(time_t t);
	extern const char * strgmtdate(time_t t);

#ifdef __cplusplus
}
#endif

#endif
//_MKGMTIME_H_
