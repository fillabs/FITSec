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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "clog.h"

static int _clog_level = CLOG_INFO;
static FILE * _clog_out[CLOG_LASTLEVEL];
static const char * _clog_lnames[CLOG_LASTLEVEL] = {
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
};

static int _clog_out_initialized = 0;

static void _clog_out_initialize(void)
{
    int i;
    _clog_out[CLOG_FATAL]   = stderr;
    _clog_out[CLOG_ERROR]   = stderr;
    _clog_out[CLOG_WARNING] = stderr;
    for(i=CLOG_INFO; i<CLOG_LASTLEVEL; i++){
        _clog_out[i] = stdout;
    }
    _clog_out_initialized = 1;
}

int  clog_level(void)
{
    return _clog_level;
}

void clog_set_level(int const level)
{
    if(level >= 0 && level <= CLOG_DEBUG){
        _clog_level = level;
    }
}
const char * clog_level_name(int const level)
{
    const char * ret = NULL;
    if(level < CLOG_LASTLEVEL)
        ret = _clog_lnames[level];
    return ret ? ret : CLOG_DEFAULT_LEVEL_NAME;
}

void clog_set_level_name(int const level, const char * const name)
{
    if(level < CLOG_LASTLEVEL)
        _clog_lnames[level] = name;
}

void clog_set_output(int const level, void * const out)
{
    if(0 == _clog_out_initialized){
        _clog_out_initialize();
    }
    if(level < 0){
        int i;
        for(i=0; i<sizeof(_clog_out)/sizeof(_clog_out[0]); i++){
            _clog_out[i] = (FILE*)out;
        }
    }else if(level < sizeof(_clog_out)/sizeof(_clog_out[0])){
        _clog_out[level] = (FILE*)out;
    }
}

void clog_fprintf(void * const f, int const level, const char * format, ...)
{
    if(level <= _clog_level){
        FILE * out;
        va_list ap;
        if(f){
            out = (FILE*)f;
        }else{
            if(0 == _clog_out_initialized){
                _clog_out_initialize();
            }
            if(level >= sizeof(_clog_out)/sizeof(_clog_out[0])){
                out = _clog_out[sizeof(_clog_out)/sizeof(_clog_out[0]) - 1];
            }else{
                out = _clog_out[level];
            }
        }
        if(out){
            va_start(ap, format);
            vfprintf(out, format, ap);
            va_end(ap);
            {
                int l = strlen(format);
                if(l == 0 || format[l-1]!= '\n'){
                    fprintf(out, "\n");
                }
            }
        }
    }
}
