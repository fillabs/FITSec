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
#ifndef cdir_h
#define cdir_h
#include "cstr.h"
#include "cserialize.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct cdir_t cdir_t;
enum {
	e_cdir_recursive = 1,
	e_cdir_nofiles   = 4,
	e_cdir_nodirs    = 8,
};

typedef struct {
	const char * path;
	const char * fname;
	int          flags;
	uint64_t     size;
}cdir_stat_t;

cdir_t *            cdir_open(const pchar_t * path, const char * mask, int flags);
void                cdir_close(cdir_t * dir);
cdir_t *            cdir_rewind(cdir_t * dir);
const cdir_stat_t * cdir_next(cdir_t * dir);

int             cdir_glob(const char * mask, const char * fname);
#ifdef __cplusplus
}
#endif
#endif

