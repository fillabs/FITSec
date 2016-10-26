/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##
##  Copyleft (c) 2003 - 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/

#include "cserialize.h"
#include "cstr.h"
#include "e4c_lite.h"
#include <errno.h>
#include <string.h>

#ifdef __GNUC__
#define cint_cpy(D,N,S) __builtin_memcpy((char*)(D),N,S)
#else
#define cint_cpy(D,N,S) memcpy((char*)(D),N,S)
static const uint64_t __one64 = 1;
#if !defined (_MSC_VER)
uint64_t cint64_swap(uint64_t);
uint32_t cint32_swap(uint32_t);
uint16_t cint16_swap(uint16_t);
#endif
#endif

#define C_ERROR(E) \
	if (perror) *perror = E; \
	throw(RuntimeException, E, NULL)

int _cint64_write(const uint64_t value, char** const ptr, const char* const end, int * const perror)
{
	register unsigned char* p = (unsigned char*)*ptr;
	if (p + 8 > (unsigned char*)end){
		C_ERROR(ENOSPC);
		return -1;
	}
	if (0 == (((intptr_t)p) & 0x7)){
		*((uint64_t*)p) = cint64_hton(value);
		p+=8;
	}else{
		int i;
		for(i=7; i>=0; i--){
			*p++ = (value>>(8*i))&0xFF;
		}
	}
	*ptr = (char*)p;
	if (perror) *perror = 0;
	return 0;
}

int _cint32_write(const uint32_t value, char** const ptr, const char* const end, int * const perror)
{
	register unsigned char* p = (unsigned char*)*ptr;
	if(p + 4 > (unsigned char*)end){
		C_ERROR(ENOSPC);
		return -1;
	}
	if (0 == (((intptr_t)p) & 0x3)){
		*((uint32_t*)p) = cint32_hton(value);
		p+=4;
	}else{
		int i;
		for(i=3; i>=0; i--){
			*p++ = (value>>(8*i))&0xFF;
		}
	}
	*ptr = (char*)p;
	if (perror) *perror = 0;
	return 0;
}

int _cint16_write(const uint16_t value, char** const ptr, const char* const end, int * const perror)
{
	register unsigned char* p = (unsigned char*)*ptr;
	if (p + 2 > (unsigned char*)end){
		C_ERROR(ENOSPC);
		return -1;
	}
	*p++ = (value >> 8) & 0xFF;
	*p++ = value&0xFF;
	*ptr = (char*)p;
	if (perror) *perror = 0;
	return 0;
}

int _cint8_write(const uint8_t value, char** const ptr, const char* const end, int * const perror)
{
	if (*ptr >= end) {
		C_ERROR(ENOSPC);
		return -1;
	}
	if (perror) *perror = 0;
	*((uint8_t*)*ptr) = value;
	(*ptr) ++;
	return 0;
}

uint64_t cint64_read(const char** const ptr, const char* const end, int * const perror)
{
	uint64_t value;
	register const uint8_t * p = (const  uint8_t *)*ptr;
	if (p + 8 > (const uint8_t *)end) {
		C_ERROR(EFAULT);
		return (unsigned)-1;
	}
	if (0 == (((intptr_t)p) & 0x7)){
		value = *(uint64_t*)p;
		value = cint64_hton(value);
		*ptr = (char*)(p+8);
	}else{
		int i;
		value=0;
		for(i=0; i<8; i++){
			value  = (value<<8) | *(p++);
		}
		*ptr = (char*)p;
	}
	if (perror) *perror = 0;
	return value;
}

uint32_t cint32_read(const char** const ptr, const char* const end, int * const perror)
{
	uint32_t value;
	register const uint8_t * p = (const uint8_t*)*ptr;
	if(p + 4 > (const uint8_t *)end) {
		C_ERROR(EFAULT);
		return (unsigned)-1;
	}
	if (perror) *perror = 0;
	value = ((uint32_t)p[0]) << 24 | ((uint32_t)p[1]) << 16 | ((uint32_t)p[2]) << 8 | p[3];
	*ptr = (char*)(p+4);
	return value;
}

uint16_t cint16_read(const char** const ptr, const char* const end, int * const perror)
{
	uint32_t value;
	register const uint8_t * p = (const uint8_t*)*ptr;
	if (p + 2 > (const uint8_t *)end) {
		C_ERROR(EFAULT);
		return (uint16_t)-1;
	}
	if (perror) *perror = 0;
	value = ((uint16_t)p[0]) << 8 | p[1];
	*ptr = (const char*)(p+2);
	return value;
}

uint8_t cint8_read(const char** const ptr, const char* const end, int * const perror)
{
	if (*ptr >= end) {
		C_ERROR(EFAULT);
		return (uint8_t)-1;
	}
	if (perror) *perror = 0;
	return *(const uint8_t*)((*ptr)++);
}

int cintx_bytecount(uint64_t value)
{
	int num_bytes = 0;
#ifdef __GNUC__
	if(value){
		num_bytes = (64 + 6 - __builtin_clzll(value))/7;
	}else{
		num_bytes = 1;
	}
#else
	uint64_t overflow = 0;
	while(value >= overflow){
		num_bytes++;
		overflow = __one64 << (7*num_bytes);
	}
#endif
	return num_bytes;
}

int _cintx_write (const uint64_t value, char ** const ptr, const char * const end, int * const perror)
{
	int num_bytes = 0;
	uint8_t c;
	uint8_t *out = (uint8_t*)(*ptr);
	num_bytes = cintx_bytecount(value);
	if(num_bytes > 8 || out+num_bytes > ((const uint8_t*)end)){
		C_ERROR(ENOSPC);
		return (unsigned)-1;
	}
	num_bytes--;
	c  = ~((1<<(8-num_bytes))-1);
	c |= (value >> (num_bytes*8)) & 0xFF;
	*out++ = c;
	while(num_bytes){
		num_bytes--;
		c = (value >> (num_bytes*8)) & 0xFF;
		*out++ = c;
	}
	*ptr = (char*)out;
	if (perror) *perror = 0;
	return 0;
}

static int countof1(int c)
{
	int r = 0;
	while(c & 0x80){
#if defined(__GNUC__) && ((__GNUC__ * 100 + __GNUC_MINOR__) >= 407) && !defined(__INTEL_COMPILER)
		return 1 + __builtin_clrsb(c<<24);
#else
		r++;
		c<<=1;
#endif		
	}
	return r;
}

uint64_t cintx_read(const char** const ptr, const char* const end, int * const perror)
{
	uint8_t c;
	const uint8_t* in;
	int i, lead_ones;
	in = (const uint8_t*)*ptr;
	if(in <= (const uint8_t*)end){
		c = *in;
		lead_ones = countof1(c);
		if(in + 1 + lead_ones <= (const uint8_t*)end) {
			uint64_t value;
			value = c & ((1<<(7-lead_ones))-1);
			for(i=1; i<=lead_ones; i++){
				value  = (value<<8) | in[i];
			}
			*ptr = (const char*)(in + 1 + lead_ones);
			if (perror) *perror = 0;
			return value;
		}
	}
	C_ERROR(EFAULT);
	return (unsigned)-1;
}

uint32_t cxsize_read(const char ** const ptr, const char * const end, int * const perror)
{
	uint32_t len = (uint32_t)cintx_read(ptr, end, perror);
	if (perror == 0){
		if (*ptr + len > end){
			C_ERROR(EFAULT);
			return (unsigned)-1;
		}
	}
	if (perror) *perror = 0;
	return len;
}

int cbuf_write(const void * const p, int length, char ** const ptr, const char * const end, int * const perror)
{
	if((*ptr) + length > end) {
		C_ERROR(ENOSPC);
		return (unsigned)-1;
	}
	cint_cpy(*ptr, p, length);
	*ptr = (*ptr) + length;
	if (perror) *perror = 0;
	return 0;
}

int cbuf_read (void * const p, int length, const char ** const ptr, const char * const end, int * const perror)
{
	if((*ptr) + length > end) {
		C_ERROR(EFAULT);
		return -1;
	}
	if (p){
		cint_cpy(p, *ptr, length);
	}
	*ptr = (*ptr) + length;
	if (perror) *perror = 0;
	return 0;
}

int cstr_write(const char * const p, char ** const ptr, const char * const end, int * const perror)
{
	int ret;
	int len = cstrlen(p);
	// write size
	ret = cintx_write(len, ptr, end, perror);
	if (ret == 0 && len > 0 ) {
		ret = cbuf_write(p, len, ptr, end, perror);
	}
	return ret;
}

int cstrn_write(const char * const p, int length, char ** const ptr, const char * const end, int * const perror)
{
	int ret;
	int len = cstrnlen(p, length);
	// write size
	ret = cintx_write(len, ptr, end, perror);
	if (ret == 0 && len > 0) {
		ret = cbuf_write(p, len, ptr, end, perror);
	}
	return ret;
}

int cstr_read(char * const p, const char ** const ptr, const char * const end, int * const perror)
{
	// read size
	int len = (int)cintx_read(ptr, end, perror);
	if (*perror == 0){
		return cbuf_read(p, len, ptr, end, perror);
	}
	return -1;
}

int cstrn_read(char * const p, int length, const char ** const ptr, const char * const end, int * const perror)
{
	// read size
	int len = (int)cintx_read(ptr, end, perror);
	if (len <= length){
		// read buf
		return cbuf_read(p, len, ptr, end, perror);
	}
	C_ERROR(EFAULT);
	return -1;
}

int cbookmark_store(cbookmark * bm, char ** const ptr, const char * const end, int * const perror)
{
	char * p = *ptr;
	if (bm->idx >= sizeof(bm->ptrs) / sizeof(bm->ptrs[0])){
		C_ERROR(E2BIG);
		return -1;
	}
	if (p >= end){
		C_ERROR(ENOSPC);
		return -1;
	}
	bm->ptrs[bm->idx] = p;
	bm->idx++;
	*ptr = p + 1;
	if (perror) *perror = 0;
	return 0;
}

int cbookmark_apply(cbookmark * bm, char ** const ptr, const char * const end, int * const perror)
{
	int size, bcount;
	char *p, * psize;

	p = *ptr;
	if (bm->idx == 0){
		C_ERROR(E2BIG);
		return -1;
	}
	psize = bm->ptrs[--bm->idx];
	size = p - psize - 1;
	bcount = cintx_bytecount(size);
	if (bcount == 1){
		*(unsigned char*)psize = size;
		size = 0; // return value;
	}
	else{
		if (p + bcount - 1 > end){
			C_ERROR(ENOSPC);
			return (unsigned)-1;
		}
		memmove(psize + bcount, psize + 1, p - psize - 1);
		*ptr = p + bcount - 1;
		size = cintx_write(size, &psize, psize + bcount, perror);
	}
	if (perror) *perror = 0;
	return size; // size is overridden to be 0 or -1
}
