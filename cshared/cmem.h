/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##  Date      : 10.11.2005
##
##  Copyleft (c) 2003 - 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/
#ifndef cmem_h
#define cmem_h
#include <stdlib.h>
#include <string.h>
#define callocate(N)  malloc(N)
#define callocate0(N) calloc(1,N)
#define cfree(P)      free(P)
#define cnew(T)     (T*)callocate(sizeof(T))
#define cnew0(T)    (T*)callocate0(sizeof(T))
typedef void(cdestructor_fn)(void*);

#if defined(__GNUC__)
#define cfetch_and_add(P,X) __sync_fetch_and_add(P, X)
#define cfetch_and_sub(P,X) __sync_fetch_and_sub(P, X)
#define cadd_and_fetch(P,X) __sync_add_and_fetch(P, X)
#define csub_and_fetch(P,X) __sync_sub_and_fetch(P, X)
#define cfetch_and_inc(P)     cfetch_and_add(P,1)
#define cfetch_and_dec(P)     cfetch_and_sub(P,1)
#define cinc_and_fetch(P)     cadd_and_fetch(P,1)
#define cdec_and_fetch(P)     csub_and_fetch(P,1)

#elif defined (_MSC_VER)
#include <windows.h>
#define cfetch_and_add(P,X) (InterlockedAddNoFence(P,X)-X)
#define cfetch_and_sub(P,X) (InterlockedAddNoFence(P,-X)+X)
#define cadd_and_fetch(P,X) InterlockedAddNoFence(P,X)
#define csub_and_fetch(P,X) InterlockedAddNoFence(P,-X)
#define cfetch_and_inc(P)   (InterlockedIncrementNoFence(P)-1)
#define cfetch_and_dec(P)   (InterlockedDecrementNoFence(P)+1)
#define cinc_and_fetch(P)   InterlockedIncrementNoFence(P)
#define cdec_and_fetch(P)   InterlockedDecrementNoFence(P)
#ifndef __cplusplus
#define inline _inline
#endif
#endif

__inline static void * cmemdup(const void * const ptr, int size) {
	void * p;
	if (size > 0){
		p = callocate(size);
		if (p && ptr) {
			memcpy(p, ptr, size);
		}
	}
	else{
		p = NULL;
	}
	return p;
}

__inline static void * _cretain(void*p, int*prcntr) {
	if (*prcntr != 0){
		cinc_and_fetch(prcntr);
	}
	return p;
}
__inline static void  _crelease(void*p, int*prcntr, void * destr) {
	if (*prcntr != 0){
		if(0 == cdec_and_fetch(prcntr)){
			if (destr) ((cdestructor_fn *)destr)(p);
			else       free(p);
		}
	}
}

#if defined(__GNUC__)
#define cretain(S) (__typeof__(S)*)_cretain(S, &(S)->_rcntr)
#elif defined (_MSC_VER)
#define cretain(S) _cretain(S, &(S)->_rcntr)
#endif

#define crelease(S,D) _crelease(S, &(S)->_rcntr, D)

#endif
