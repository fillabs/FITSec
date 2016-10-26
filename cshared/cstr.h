/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##
##  Copyleft (c) 2003 - 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/
#ifndef CSTR_H
#define CSTR_H
#include <string.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

int    cstrlen(const char * str);
int    cstrnlen(const char * str, int maxsize);
char*  cstrend(const char * str);
/* copy src to dst and return pointer to the next byte after the end */
char * cstrcpy(char * dst, const char * src);

/* copy up to maxsize characters from src to dst and return pointer to the next byte after the end */ 
char * cstrncpy(char * dst, int maxsize, const char * src);

/* copy up to maxsize characters from parameters to dst and return pointer to the next byte after the end */ 
char * cvstrncpy(char * dst, int maxsize, const char * ptr, ...);

int cstrequal(const char * s1, const char * s2);

/* allocate copy of the str */ 
char * cstralloc(int size);
char * cstrdup(const char * str);
char * cstrndup(const char * str, int max_size);

/* allocate new str and collect all paramaters */ 
char * cvstrdup(const char * ptr, ...);

int cstr_write(const char * const p, char ** const ptr, const char * const end, int * const error);
int cstr_read (char * const p, const char ** const ptr, const char * const end, int * const error);

int cstrn_write(const char * const p, int length, char ** const ptr, const char * const end, int * const error);
int cstrn_read (char * const p, int length, const char ** const ptr, const char * const end, int * const error);

char * cstr_hex2bin(char * bin, int blen, const char * hex, int hlen);

/* file access functions */
#ifndef PCHAR_T_DEFINED
#if defined(UNICODE) && ( defined(_WIN32) || defined(_WIN64) )
#define PCHAR16BITS
#include <wchar.h>
typedef wchar_t pchar_t;
#define _PCH(X) L##X
#else
typedef char pchar_t;
#define _PCH(X) X
#endif
#endif

/* load file content to the buffer and return pointer to the next byte after the buffer */
char * cstrnload(char * dst, int max_size, const pchar_t * path);
/* load file content to the new allocated buffer, assign it to p and return pointer to the next byte after the buffer */
char * cstraload(char ** p, const pchar_t * path);

/* return the last element of the path */
const pchar_t * cstrlastpathelement(const pchar_t * str);
pchar_t * cstrpathextension(const pchar_t * str);


#ifdef PCHAR16BITS
#define pchar_len(S)      wcslen(S)
#define pchar_ncpy(D,S,N) wcsncpy(D,S,N)
#define pchar_cpy(D,S)    wcscpy(D,S)
#define pchar_rchr(S,C)   wcsrchr(S,C)
#define pchar_fopen(P,M) _wfopen(P, M)
#define pchar_main(A,V)   wmain(A,V)
#else
#define pchar_len(S)      strlen(S)
#define pchar_ncpy(D,S,N) strncpy(D,S,N)
#define pchar_cpy(D,S)    strcpy(D,S)
#define pchar_rchr(S,C)   strrchr(S,C)
#define pchar_fopen(P,M)  fopen(P, M)
#define pchar_main(A,V)   main(A,V)
#endif
#define pchar_alloc(S) ((pchar_t*)malloc(sizeof(pchar_t)*((S)+1)))

#ifdef __cplusplus
}
#endif
#endif
