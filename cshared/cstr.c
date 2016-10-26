/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##
##  Copyleft (c) 2003 - 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include "cstr.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

int    cstrlen(const char * str)
{
	return str ? strlen(str) : 0;
}
int    cstrnlen(const char * str, int maxsize)
{
	int len = str ? strlen(str) : 0;
	return  len > maxsize ? maxsize : len;
}

char*  cstrend(const char * str)
{
	return (char*)(str ? str + strlen(str) : NULL);
}

int cstrequal(const char * s1, const char * s2)
{
	return (s1 == s2) || (s1 && s2 && 0 == strcmp(s1, s2));
}

char * cstrcpy(char * dst, const char * src)
{
	if(!dst) return (char*)0;
	int len = 0;
	if(src){
		len = strlen(src);
		if(len){
			memcpy(dst, src, len);
		}
	}
	dst[len]=0;
	return dst + len;
}

/* copy up to maxsize characters from src to dst and return pointer to the next byte after the end */ 
char * cstrncpy(char * dst, int maxsize, const char * src)
{
	if(!dst) return (char*)0;
	unsigned int ms = maxsize;
	unsigned int len = 0;
	if(src && ms > 0){
		len = strlen(src);
		if (len > ms){
			len = ms;
		}
		if(len){
			memcpy(dst, src, len);
		}
	}
	dst[len]=0;
	return dst + len;
}

/* copy up to maxsize characters from src to dst and return pointer to the next byte after the end */ 
char * cvstrncpy(char * dst, int maxsize, const char * ptr, ...)
{
	va_list ap;
	char * p = dst;
	const char * r = ptr;
	unsigned int ms = maxsize;
	if(ms > 0){
		va_start(ap, ptr);
		while(r){
			unsigned int len = strlen(r);
			if(len > ms) len = ms;
			memcpy(p, r, len);
			p  += len;
			ms -= len;
			r = va_arg(ap, const char*);
		}
		va_end(ap);
		*p = 0;
	}
	return p;
}

char * cstralloc(int size)
{
	return (char*)malloc((size+0xF)&(~0xF));
}
char * cstrdup(const char * str)
{
	char * ret = NULL;
	if(str){
		int len = strlen(str);
		if(len){
			ret = cstralloc(len);
			memcpy(ret, str, len+1);
		}
	}
	return ret;
}

char * cstrndup(const char * str, int max_size)
{
	char * ret = NULL;
	unsigned int ms = max_size;
	if(str){
		unsigned int len = strlen(str);
		if(len>ms) len=ms;
		if(len){
			ret = cstralloc(len);
			memcpy(ret, str, len);
			ret[len] = 0;
		}
	}
	return ret;
}

char * cvstrdup(const char * ptr, ...)
{
	va_list ap;
	int len = 0;
	char *dst, *p;
	const char * r;
	
	if(!ptr) return (char*)ptr;

	// calculate size
	r = ptr;
	va_start(ap, ptr);
	while(r){
		len += strlen(r);
		r = va_arg(ap, const char*);
	}
	va_end(ap);
	
	p = dst = cstralloc(len+1);
	if(dst){
		r = ptr;
		va_start(ap, ptr);
		while(r){
			len = strlen(r);
			memcpy(p, r, len);
			p += len;
			r = va_arg(ap, const char*);
		}
		va_end(ap);
		*p = 0;
	}
	return dst;
}

char * cstrnload(char * dst, int max_size, const pchar_t * path)
{
	FILE * f = pchar_fopen(path, _PCH("rb"));
	unsigned long len, rl;
	unsigned long ms = max_size;
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (len > ms) len = ms;
	rl = fread(dst, 1, len, f);
	fclose(f);
	if ((int)rl < 0){
		return NULL;
	}
	if (len < ms)dst[len] = 0;
	return dst + len;
}

char * cstraload(char ** p, const pchar_t * path)
{
	char * ret = NULL;
	FILE * f = pchar_fopen(path, _PCH("rb"));
	int len;
	if (f){
		fseek(f, 0, SEEK_END);
		len = ftell(f);
		fseek(f, 0, SEEK_SET);
		if (len > 0){
			ret = malloc(len);
			if (ret){
				int ms = fread(ret, 1, len, f);
				if (ms < len){
					free(ret);
					*p = ret = NULL;
				}
				else{
					*p = ret;
					ret += len;
				}
			}
		}
		fclose(f);
	}
	return ret;
}

const pchar_t * cstrlastpathelement(const pchar_t * str)
{
	const pchar_t * p = pchar_rchr(str, '/');
	const pchar_t * p2 = pchar_rchr(str, '/');
	if(p<p2)p=p2;
	else if(p == NULL) p = str;
	return p;
}

pchar_t * cstrpathextension(const pchar_t * str)
{
	const pchar_t * p;
	if (str){
		p = pchar_rchr(str, '.');
		if (!p) p = str + pchar_len(str);
	}
	else{
		p = str;
	}
	return (pchar_t*)p;
}

char * cstr_hex2bin(char * bin, int blen, const char * hex, int hlen)
{
	// check
	const char * h = hex;
	const char * e = hex+hlen;
	char * b = bin;
	int n = 0;
	while (h<e){
		char ch = *h++;
		if (isspace((int)(ch))) continue;
		if (ch >= '0' && ch <= '9') continue;
		if (ch >= 'A' && ch <= 'F') continue;
		if (ch >= 'a' && ch <= 'f') continue;
		return NULL;
	}
	h = hex;
	while (h < e){
		char ch = *h++;
		if (ch >= '0' && ch <= '9') ch -= '0';
		else if (ch >= 'A' && ch <= 'F') ch -= 'A' - 0x0A;
		else if (ch >= 'a' && ch <= 'f') ch -= 'a' - 0x0A;
		else continue;
		if (!n){
			*b = ch;
		}
		else{
			char ch1 = *b;
			*b++ = (ch1 << 4) | ch;
		}
		n = !n;
	}
	if (n){
		char ch1 = *b;
		*b++ = (ch1 << 4);
		n = 0;
	}
	return b;
}

int cstr_write(const char * const p, char ** const ptr, const char * const end, int * const error);
int cstr_read(char * const p, const char ** const ptr, const char * const end, int * const error);

int cstrn_write(const char * const p, int length, char ** const ptr, const char * const end, int * const error);
int cstrn_read(char * const p, int length, const char ** const ptr, const char * const end, int * const error);
