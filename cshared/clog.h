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
#ifndef clog_h
#define clog_h
enum{
    CLOG_FATAL=0,
    CLOG_ERROR,
    CLOG_WARNING,
    CLOG_INFO,
    CLOG_DEBUG,

    CLOG_LASTLEVEL
};

int          clog_level(void);
void         clog_set_level(int const level);
const char * clog_level_name(int const level);
void         clog_set_level_name(int const level, const char * const name);
void         clog_set_output(int const level, void * const out);

#define CLOG_DEFAULT_LEVEL_NAME ""

#define STRMODULE(M) #M
#define XSTRMODULE(M) STRMODULE(M)

#define clog_flm(F, SMODULE, LEVEL, FORMAT, ...) clog_fprintf(F, LEVEL, "%10.10s: %7.7s: " FORMAT, SMODULE, clog_level_name(LEVEL), ##__VA_ARGS__)
#define clog_lm(SMODULE,LEVEL,FORMAT, ...)       clog_flm((void*)0, SMODULE, LEVEL, FORMAT, ##__VA_ARGS__)
#define clog_fl(F,LEVEL,FORMAT, ...)             clog_flm(F, XSTRMODULE(__MODULE__), LEVEL, FORMAT, ##__VA_ARGS__)
#define clog_l(LEVEL,FORMAT, ...)                clog_flm((void*)0, XSTRMODULE(__MODULE__), LEVEL, FORMAT, ##__VA_ARGS__)

#define clog_fatal(FORMAT, ...)                 clog_l(CLOG_FATAL,   FORMAT, ##__VA_ARGS__)
#define clog_error(FORMAT, ...)                 clog_l(CLOG_ERROR,   FORMAT, ##__VA_ARGS__)
#define clog_warning(FORMAT, ...)               clog_l(CLOG_WARNING, FORMAT, ##__VA_ARGS__)
#define clog_info(FORMAT, ...)                  clog_l(CLOG_INFO,    FORMAT, ##__VA_ARGS__)
#define clog_debug(FORMAT, ...)                 clog_l(CLOG_DEBUG,   FORMAT, ##__VA_ARGS__)

#define fclog_fatal(F,FORMAT, ...)              clog_fl(F, CLOG_FATAL,   FORMAT, ##__VA_ARGS__)
#define fclog_error(F,FORMAT, ...)              clog_fl(F, CLOG_ERROR,   FORMAT, ##__VA_ARGS__)
#define fclog_warning(F,FORMAT, ...)            clog_fl(F, CLOG_WARNING, FORMAT, ##__VA_ARGS__)
#define fclog_info(F,FORMAT, ...)               clog_fl(F, CLOG_INFO,    FORMAT, ##__VA_ARGS__)
#define fclog_debug(F,FORMAT, ...)              clog_fl(F, CLOG_DEBUG,   FORMAT, ##__VA_ARGS__)

#define mclog_fatal(MODULE, FORMAT, ...)        clog_lm(#MODULE, CLOG_FATAL,   FORMAT, ##__VA_ARGS__)
#define mclog_error(MODULE, FORMAT, ...)        clog_lm(#MODULE, CLOG_ERROR,   FORMAT, ##__VA_ARGS__)
#define mclog_warning(MODULE, FORMAT, ...)      clog_lm(#MODULE, CLOG_WARNING, FORMAT, ##__VA_ARGS__)
#define mclog_info(MODULE, FORMAT, ...)         clog_lm(#MODULE, CLOG_INFO,    FORMAT, ##__VA_ARGS__)
#define mclog_debug(MODULE, FORMAT, ...)        clog_lm(#MODULE, CLOG_DEBUG,   FORMAT, ##__VA_ARGS__)

#define fmclog_fatal(F, MODULE, FORMAT, ...)    clog_flm(F, #MODULE, CLOG_FATAL,   FORMAT, ##__VA_ARGS__)
#define fmclog_error(F, MODULE, FORMAT, ...)    clog_flm(F, #MODULE, CLOG_ERROR,   FORMAT, ##__VA_ARGS__)
#define fmclog_warning(F, MODULE, FORMAT, ...)  clog_flm(F, #MODULE, CLOG_WARNING, FORMAT, ##__VA_ARGS__)
#define fmclog_info(F, MODULE, FORMAT, ...)     clog_flm(F, #MODULE, CLOG_INFO,    FORMAT, ##__VA_ARGS__)
#define fmclog_debug(F, MODULE, FORMAT, ...)    clog_flm(F, #MODULE, CLOG_DEBUG,   FORMAT, ##__VA_ARGS__)

void clog_fprintf(void * const f, int const level, const char * format, ...);

#undef __CLOG_MODULE

#endif
