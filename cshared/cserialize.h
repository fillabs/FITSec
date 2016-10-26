/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##
##  Copyleft (c) 2003 - 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/

#ifndef cint_h
#define cint_h
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "e4c_lite.h"

#if defined(__GNUC__)
# define cint64_swap(X) __builtin_bswap64(X)
# define cint32_swap(X) __builtin_bswap32(X)
# define cint16_swap(X) __builtin_bswap16(X)
#elif defined (_MSC_VER)
# define cint64_swap(X) _byteswap_uint64(X)
# define cint32_swap(X) _byteswap_ulong (X)
# define cint16_swap(X) _byteswap_ushort(X)
#define __ORDER_LITTLE_ENDIAN__ 1
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#else
uint64_t cint64_swap(const uint64_t);
uint32_t cint32_swap(const uint32_t);
uint16_t cint16_swap(const uint16_t);
#endif
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define cint64_hton(X) cint64_swap(X)
# define cint32_hton(X) cint32_swap(X)
# define cint16_hton(X) cint16_swap(X)
# define cint64_lsb(X)  ((uint8_t)(((uint64_t)(X))>>56))
# define cint32_lsb(X)  ((uint8_t)(((uint32_t)(X))>24))
# define cint24_lsb(X)  ((uint8_t)(((uint32_t)(X))>16))
# define cint64_lsb3(X) ((uint32_t)(((uint64_t)(X))>>40))
# define cint32_lsb3(X) ((uint32_t)(((uint64_t)(X))>>8))
#else
# define cint64_hton(X) (X)
# define cint32_hton(X) (X)
# define cint16_hton(X) (X)
# define cint64_lsb(X)  ((uint8_t)((X)&0xFF))
# define cint32_lsb(X)  ((uint8_t)((X)&0xFF))
# define cint24_lsb(X)  ((uint8_t)((X)&0xFF))
# define cint64_lsb3(X) ((uint32_t)((X)&0xFFFFFF))
# define cint32_lsb3(X) ((uint32_t)((X)&0xFFFFFF))
#endif /* __BYTE_ORDER__ */

/* serialisation */
int  _cint64_write(const uint64_t n, char ** const ptr, const char * const end, int * const error);
int  _cint32_write(const uint32_t n, char ** const ptr, const char * const end, int * const error);
int  _cint16_write(const uint16_t n, char ** const ptr, const char * const end, int * const error);
int  _cint8_write (const uint8_t  n, char ** const ptr, const char * const end, int * const error);
int  _cintx_write (const uint64_t n, char ** const ptr, const char * const end, int * const error);

#define cint64_write(N,P,S,E) _cint64_write((uint64_t)(N), P, S, E)
#define cint32_write(N,P,S,E) _cint32_write((uint32_t)(N), P, S, E)
#define cint16_write(N,P,S,E) _cint16_write((uint16_t)(N), P, S, E)
#define cint8_write(N,P,S,E)  _cint8_write ((uint8_t)(N), P, S, E)
#define cintx_write(N,P,S,E)  _cintx_write ((uint32_t)(N), P, S, E)
        
uint64_t cint64_read (const char ** const ptr, const char * const end, int * const error);
uint32_t cint32_read (const char ** const ptr, const char * const end, int * const error);
uint16_t cint16_read (const char ** const ptr, const char * const end, int * const error);
uint8_t  cint8_read  (const char ** const ptr, const char * const end, int * const error);
uint64_t cintx_read  (const char ** const ptr, const char * const end, int * const error);
uint32_t cxsize_read(const char ** const ptr, const char * const end, int * const error);
int cintx_bytecount(uint64_t);

int cbuf_write(const void * const p, int length, char ** const ptr, const char * const end, int * const error);
int cbuf_read (void * const p, int length, const char ** const ptr, const char * const end, int * const error);

typedef struct {
	int    idx;
	void * ptrs[6];
}cbookmark;
int cbookmark_store(cbookmark * bm, char ** const ptr, const char * const end, int * const error);
int cbookmark_apply(cbookmark * bm, char ** const ptr, const char * const end, int * const error);

E4C_DECLARE_EXCEPTION(cexc_readbuf);
E4C_DECLARE_EXCEPTION(RuntimeException);

#ifdef __cplusplus
}
#endif

#endif
