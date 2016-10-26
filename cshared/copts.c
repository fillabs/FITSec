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
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#include "copts.h"

#ifdef _MSC_VER
#define inline __inline
#define strcasecmp _stricmp
#define strdup _strdup
#define _CRT_SECURE_NO_WARNINGS
#endif

#define  COPT_AFLAG_ERROR    0x10000
#define  COPT_AFLAG_PRESCAN  0x08000
#define  COPT_AFLAG_LOPT     0x04000
#define  COPT_AFLAG_AUTOHELP 0x02000

static int do_coptions     (int argc, char* argv[], int flags, copt_t* opts);
static int set_option_value(copt_t* opt, const char * option, char* val);
static int copt_addr_assign(unsigned long* addr,const char* val);
inline
static int call_callback(const copt_t * opt, const char * option, const copt_value_t * value);
static void print_opt_help(FILE* f, copt_t* opt, int err, const char* errval);

int  coptions(int argc, char* argv[], int flags, copt_t* opts)
{
    int rc = 1, ahflag = 0;
    copt_t *o;

    if(0 == (flags & COPT_NOAUTOHELP)){
        ahflag = COPT_AFLAG_AUTOHELP;
    }

    if(0 == (flags & COPT_NOCONFIG) || ahflag ){
        for(o=opts; o->type != COPT_END; o++){
            unsigned short t = o->type&COPT_TYPE_MASK;

            /* check for config files */
            if(0==(flags & COPT_NOCONFIG) && t == COPT_CFGFILE){
                /* check for config options.
                 * just options with type COPT_CFGFILE
                 * will be processed */
                rc = do_coptions(argc, argv, flags|COPT_AFLAG_PRESCAN|COPT_NOREORDER, opts);
                if(rc < 0){
                    if(rc == COPT_EHELP){
                        if(0==(flags&COPT_NOHELP_MSG)){
                            coptions_help_ex(stdout, argv[0], flags, opts, NULL, NULL, NULL);
                        }
                        return rc;
                    }
                    if(0==(flags&COPT_NOERR_UNKNOWN)){
                        return rc;
                    }
                }
                if(o->vptr){
                    const char * cfgname = *((const char **)o->vptr);
                    if(cfgname){
                        rc = coptions_load(cfgname, NULL, flags, opts);
                        if(rc < 0){
                            if(0 == (flags&COPT_NOERR_MSG)){
                                fprintf(stderr, "%s: config load error\n", cfgname);
                            }
                            if(flags & COPT_ERR_CONFIG){
                                return rc;
                            }
                        }
                    }
                }
            } else if(t == COPT_HELP && ahflag){
                /* check for autohelp */
                ahflag = 0;
            }
        }
    }
    /* process other options except of COPT_CFGFILE */
    return do_coptions(argc, argv, flags|ahflag, opts);
}

static void move_args(char * argv[], int to, int from, int count)
{
    if( from > to){
        while(from > to){
            int i;
            for(i=from; i<from+count; i++){
                char * p = argv[i];
                argv[i] = argv[i-1];
                argv[i-1] = p;
            }
            from --;
        }
    }else{
        while(from < to){
            int i;
            for(i=from+count-1; i>=from; i--){
                char * p = argv[i];
                argv[i] = argv[i+1];
                argv[i+1] = p;
            }
            from ++;
        }
    }
}

static int do_coptions(int argc, char* argv[], int flags, copt_t* opts)
{
    int i, rc;
    int searchopts = 1;
    copt_t* o=NULL;
    copt_t* copt  = NULL;
    copt_t* onopt = NULL;
    coptype_t t;
    int nargc=argc;

    assert(opts);

    for(i=1; i<nargc; i++){
        if(NULL == copt){
            char * aopt = argv[i];
            /* check for option */
            if(searchopts && aopt[0]=='-' && aopt[1] != 0){
                if(aopt[1] == '-'){

                    /* this is long option */
                    if(aopt[2] == 0){
                        /* -- - stop parsing options */
                        searchopts = 0;
                        continue;
                    }

                    /* check autohelp */
                    if(flags & COPT_AFLAG_AUTOHELP){
                        if(0==strcmp(aopt+2, "help")){
                            return COPT_EHELP;
                        }
                    }

                    /* search given option */
                    for(o = opts; COPT_END != o->type; o++){
                        int ol;
                        if(NULL == o->lopt) continue;
                        t = o->type&COPT_TYPE_MASK;
                        ol = (int)strlen(o->lopt);
                        if(   (aopt[2+ol]==0 || aopt[2+ol]=='=')
                           && 0==strncmp(o->lopt, aopt+2, ol)){

                            /* option found */
                            if(t == COPT_HELP){
                                return COPT_EHELP;
                            }
                            if(aopt[2+ol]=='='){
                                if(0==(flags&COPT_AFLAG_PRESCAN) || t == COPT_CFGFILE){
                                    rc = set_option_value(o, argv[i], aopt+ol+3);
                                    if(rc<0){
                                        if(0==(flags&COPT_NOERR_MSG)){
                                            print_opt_help(stderr, o, flags|COPT_AFLAG_ERROR, argv[i]);
                                        }
                                        if(0==(flags&COPT_NOERR_ARG)){
                                            return -i;
                                        }
                                    }
                                }
                            }else if(t == COPT_BOOL || t == COPT_BOOLI){
                                if(0==(flags & COPT_AFLAG_PRESCAN)){
                                    set_option_value(o, argv[i], NULL);
                                }
                            }else{
                                copt = o;
                            }
                            /* move this option argument to the end of list */
                            move_args(argv, argc-1, i, 1);
                            i--;
                            nargc--;
                            break;
                        }
                    }
                    if(COPT_END == o->type){
                        if(0==(flags&COPT_NOERR_MSG)){
                            fprintf(stderr, "Unknown option '%s'\n", argv[i]);
                        }
                        if(0==(flags & COPT_NOERR_UNKNOWN)){
                            return 0-i;
                        }
                    }
                }else{
                    /* short options */
                    aopt++; /* skip first '-' */
                    while(*aopt){

                        /* check autohelp */
                        if(flags & COPT_AFLAG_AUTOHELP){
                            if(*aopt == 'h' || *aopt == '?'){
                                return COPT_EHELP;
                            }
                        }

                        for(o = opts; COPT_END != o->type; o++){
                            if(NULL == o->sopts) continue;
                            t = o->type&COPT_TYPE_MASK;
                            if(strchr(o->sopts, *aopt)){
                                /* option found */
                                char szopt[3]={'-',0,0};
                                szopt[1] = *aopt;

                                if(t == COPT_HELP){
                                    return COPT_EHELP;
                                }
                                if(aopt[1] == 0){       /* -p -b 13 */
                                    if(t == COPT_BOOL || t == COPT_BOOLI){
                                        if(0 == (flags&COPT_AFLAG_PRESCAN)){
                                            set_option_value(o, szopt, NULL);
                                        }
                                    }else{
                                        copt = o;
                                    }
                                    aopt++;
                                }else if(aopt[1] == '='){   /* -p=123 */
                                    if(0==(flags&COPT_AFLAG_PRESCAN) || t == COPT_CFGFILE){
                                        rc = set_option_value(o, argv[i], aopt+2);
                                        if(rc<0){
                                            if(0==(flags&COPT_NOERR_MSG)){
                                                print_opt_help(stderr, o, flags|COPT_AFLAG_ERROR, argv[i]);
                                            }
                                            if(0==(flags&COPT_NOERR_ARG)){
                                                return -i;
                                            }
                                        }
                                    }
                                    while(*aopt) aopt++;
                                }else{                  /* -p123*/
                                    if(0==(flags&COPT_AFLAG_PRESCAN) || t == COPT_CFGFILE){
                                        if( t == COPT_BOOL || t == COPT_BOOLI ){
                                            set_option_value(o, szopt, NULL);
                                        }else{
                                            rc = set_option_value(o, szopt, aopt+1);
                                            if(rc<0){
                                                if(0==(flags&COPT_NOERR_MSG)){
													print_opt_help(stderr, o, flags | COPT_AFLAG_ERROR, argv[i]);
                                                }
                                                if(0==(flags&COPT_NOERR_ARG)){
                                                    return -i;
                                                }
                                            }
                                        }
                                    }
                                    while(*aopt) aopt++;
                                }
                                break;
                            }
                        }
                        if(COPT_END == o->type){
                            if(0==(flags&COPT_NOERR_MSG)){
                                fprintf(stderr, "Unknown option '%s'\n", argv[i]);
                            }
                            if(0==(flags & COPT_NOERR_UNKNOWN)){
                                return 0-i;
                            }
                            aopt++;
                        }else{
                            /* move this option argument to the end of list */
                            move_args(argv, argc-1, i, 1);
                            i--;
                            nargc--;
                        }
                    }
                }
            }else{
                /* check for nonoption callback */
                if(0 == (flags & COPT_AFLAG_PRESCAN)){
                    if(onopt == NULL){
                        onopt = (void*)-1;
                        for(o = opts; COPT_END != o->type; o++){
                            if(o->type & COPT_CALLBACK){
                                if(o->lopt == NULL && o->sopts == NULL && o->vptr){
                                    onopt = o;
                                    break;
                                }
                            }
                        }
                    }
                    if(onopt != (void*)-1){
                        copt_value_t v;
                        v.v_str = argv[i];
                        rc = call_callback(onopt, NULL, &v);
                        if(rc < 0){
                            if(rc == COPT_EHELP) return rc;
                            if(0==(flags&COPT_NOERR_ARG)){
                                return -i;
                            }
                        }
                    }
                }
            }
        }else{ /* prev is option */
            t = copt->type & COPT_TYPE_MASK;
            if(0==(flags&COPT_AFLAG_PRESCAN) || t == COPT_CFGFILE){
                rc = set_option_value(copt, argv[i-1], argv[i]);
                if(rc < 0){
                    if(0==(flags&COPT_NOERR_MSG)){
						print_opt_help(stderr, o, flags | COPT_AFLAG_ERROR, argv[i - 1]);
                    }
                    if(0==(flags&COPT_NOERR_ARG)){
                        return 0-i-1;
                    }
                }
            }
            copt = NULL;
            /* move this option argument to the end of list */
            move_args(argv, argc-1, i, 1);
            nargc--;
            i--;
        }
    }
    if(copt){
        /* last option was not completed */
        if(0==(flags&COPT_NOERR_MSG)){
			print_opt_help(stderr, o, flags | COPT_AFLAG_ERROR, argv[i - 1]);
        }
        if(0==(flags&COPT_NOERR_ARG)){
            return 0-i-1;
        }
    }
    return nargc;
}

static int set_option_value(copt_t* opt, const char * arg, char* val)
{
    char* np;
    copt_value_t value;

    value.v_boolean = 0;

    switch(opt->type & COPT_TYPE_MASK){
    case COPT_HELP:
    case COPT_BOOL:
        value.v_boolean = 1;
    case COPT_BOOLI:
        if(val){
            if(  val[0] == '0'
               ||0==strcasecmp(val, "false")
               ||0==strcasecmp(val, "no")
              ){
                value.v_boolean = !value.v_boolean;
            }else if( !isdigit((int)(val[0]))
                     && strcasecmp(val, "true")
                     && strcasecmp(val, "yes")
                    ){
                return -1;
            }
        }
        break;
    case COPT_LONG:
        if(!val) return -1;
        value.v_long = strtol(val, &np, 0);
        if(*np != 0) return -1;
        break;
    case COPT_ULONG:
        if(!val) return -1;
        value.v_ulong = strtoul(val, &np, 0);
        if(*np != 0) return -1;
        break;
    case COPT_SHORT:
        if(!val || !opt->vptr) return -1;
        value.v_long = strtol(val, &np, 0);
        if(*np != 0) return -1;
        if(value.v_long>SHRT_MAX || value.v_long<SHRT_MIN){
            return -1;
        }
        value.v_short = (short) value.v_long;
        break;
    case COPT_USHORT:
        if(!val || !opt->vptr) return -1;
        value.v_ulong = strtoul(val, &np, 0);
        if(*np != 0) return -1;
        if(value.v_ulong>USHRT_MAX){
            return -1;
        }
        value.v_ushort = (unsigned short) value.v_ulong;
        break;

    case COPT_CHAR:
        if(!val || !opt->vptr) return -1;
        if(0==val[1]){
            value.v_char = val[0];
        }else{
            value.v_ulong = strtoul(val, &np, 0);
            if(*np != 0) return -1;
            if(value.v_ulong>UCHAR_MAX){
                return -1;
            }
            value.v_char = (unsigned char)value.v_long;
        }
        break;
    case COPT_STRENUM:
        {
            const char ** p = (const char**)opt->vptr;
            for(; *p && strcmp(*p, val); p++);
            if(NULL == *p){
                return -1;
            }
            value.v_long = p - (const char**)opt->vptr;
        }
        break;
    case COPT_STRLIST:
        if(!opt->vptr || (opt->type & COPT_CALLBACK)) return -1;
    case COPT_CFGFILE:
    case COPT_PATH:
    case COPT_STR:
        value.v_str = val;
        break;
    case COPT_HOST:
        if(0 > copt_addr_assign(&value.v_ulong, val)){
            return -1;
        }
        break;
    default:
        return -1;
    }

    if(opt->type & COPT_CALLBACK){
        return call_callback(opt, arg, &value);
    }

    switch(opt->type & COPT_TYPE_MASK){
    case COPT_HELP:
    case COPT_BOOL:
    case COPT_BOOLI:
        if(opt->vptr && opt->vptr != (void*)1){
            *(int*)opt->vptr = value.v_boolean;
        }else{
            opt->vptr = ((char*)NULL) + value.v_boolean;
        }
        break;
    case COPT_LONG:
    case COPT_ULONG:
    case COPT_CFGFILE:
    case COPT_STR:
    case COPT_PATH:
    case COPT_HOST:
        if(opt->vptr){
            *(void**)opt->vptr = value.v_str;
        }else{
            opt->vptr = (void*)value.v_str;
        }
        break;
    case COPT_STRLIST:
        {
            char ** p = opt->vptr;
            *p = value.v_str;
            opt->vptr = p + 1;
        }
        break;
    case COPT_STRENUM:
        opt->vptr = (void*)(((const char**)opt->vptr) + value.v_long);
        break;
    case COPT_SHORT:
    case COPT_USHORT:
        *(short*)opt->vptr = value.v_short;
        break;
    case COPT_CHAR:
        *(char*)opt->vptr = value.v_char;
        break;
    default:
        return -1;
    };
    return 0;
}

static int copt_addr_assign(unsigned long* addr,const char* val)
{
    unsigned long n = 0;
    unsigned long s = 0;
    int cnt = 0;
    while(*val){
        if(*val == '.'){
            if(cnt == 3){
                return -1;
            }
            n = (n<<8) | s;
            s = 0;
            cnt++;
        }else{
            unsigned int c = *val - '0';
            if(c > 9){
                return -1;
            }
            s = s*10 + c;
        }
        val++;
    }
    if(cnt != 3){
        return -1;
    }
    n = (n<<8) | s;

    *addr = n;
    return 0;
}

inline static int call_callback(const copt_t * opt, const char * option, const copt_value_t * value)
{
    if(opt->vptr){
        return ((copt_callback *)opt->vptr)(opt, option, value);
    }
    return -1;
}

void coptions_help_ex(FILE * f, const char * prgname, int flags, copt_t* opt, const char* usagestr,
                      const char* header, const char* footer)
{
    const char * progname;
    if(header){
        fputs(header, f);
    }
    if(prgname){
        progname = strrchr(prgname, '/');
        if(progname) progname++;
        else         progname = prgname;
        if(usagestr){
            fprintf(f, "Usage: %s <options> %s\n", progname, usagestr);
        }
        fprintf(f, "Options:\n");
    }else{
        if(usagestr){
            fprintf(f, "%s\n", usagestr);
        }
    }
    while(opt->type != COPT_END){
        if(opt->sopts || opt->lopt){
            fprintf(f, "  ");
			print_opt_help(f, opt, flags, NULL);
        }
        opt++;
    }
    if(footer){
        fputs(footer, f);
    }
}

static int sprintf_option_value(const copt_t* const opt, char * const buf)
{
    int ret = 0;
    switch(opt->type&COPT_TYPE_MASK){
    case COPT_BOOL:
        if(opt->vptr && opt->vptr != (void*)1){
            ret = sprintf(buf, "%s", ((*(int*)opt->vptr)?"true":"false"));
        }else{
            ret = sprintf(buf, "%s", ((opt->vptr)?"true":"false"));
        }
        break;
    case COPT_BOOLI:
        if(opt->vptr && opt->vptr != (void*)1){
            ret = sprintf(buf, "%s", (*(int*)opt->vptr)?"false":"true");
        }else{
            ret = sprintf(buf, "%s", (opt->vptr)?"false":"true");
        }
        break;
    case COPT_LONG:
        ret = sprintf(buf, "%ld", *((long*)opt->vptr));
        break;
    case COPT_ULONG:
        ret = sprintf(buf, "%lu", *((unsigned long*)opt->vptr));
        break;
    case COPT_SHORT:
        ret = sprintf(buf, "%u", *((short*)opt->vptr));
        break;
    case COPT_USHORT:
        ret = sprintf(buf, "%u", *((unsigned short*)opt->vptr));
        break;
    case COPT_CHAR:
        ret = sprintf(buf, "%c", *((char*)opt->vptr));
        break;
    case COPT_STR:
    case COPT_CFGFILE:
    case COPT_STRENUM:
        if(*(char**)opt->vptr)
	    ret = sprintf(buf, "%s", *(char**)opt->vptr);
        else
	    ret = sprintf(buf, "NULL");
	break;
    case COPT_HOST:
        {
            unsigned int n = *(unsigned int*)opt->vptr;
            sprintf(buf, "%u.%u.%u.%u",
                    n>>24, (n>>16)&0xFF, (n>>8)&0xFF, n&0xFF);
        }
        break;
    }
    return ret;
}

static const char* valnames[] = {
    NULL,  /* COPT_BOOL   */
    NULL,  /* COPT_BOOLI  */
    "num", /* COPT_LONG   */
    "num", /* COPT_ULONG  */
    "num", /* COPT_SHORT  */
    "num", /* COPT_USHORT */
    "chr", /* COPT_CHAR   */
    "str", /* COPT_STR    */
    "addr",/* COPT_HOST   */
    "str", /* COPT_STRLIST*/
    "str", /* COPT_STRENUM*/
    "file",/* COPT_CFGFILE*/
    NULL,  /* COPT_HELP   */
};

static void print_opt_help(FILE* f, copt_t* opt, int flags, const char* errval)
{
    char *p, sz_help[256];
    p=sz_help;
	int err = flags&COPT_AFLAG_ERROR;
    int t=opt->type&COPT_TYPE_MASK;
    if(opt->sopts){
        const char* po = opt->sopts;
        while(*po){
            if(po != opt->sopts) (*p++) = '|';
            *(p++)='-'; *(p++)=(*po);
            po++;
        }
        if(valnames[t]){
            p+=sprintf(p, " <%s>", valnames[t]);
        }
    }
    if(opt->lopt){
        if(opt->sopts) p+=sprintf(p, " | ");
        p+=sprintf(p, "--%s", opt->lopt);
        if(valnames[t]){
            p+=sprintf(p, "=%s", valnames[t]);
        }
    }
    if(opt->helpstr){
        while(p-sz_help<35){
            *(p++) = ' ';
        }
        p+=sprintf(p, " : %s", opt->helpstr);
    }
    if(err){
        *(p++)='\n'; *(p++)='\t';
        if(errval){
            p+=sprintf(p, "%s ", errval);
        }
        p+=sprintf(p, "Unsupported value");
    }else{
		if (0 == (flags & COPT_HELP_NOVALUES)){
			if (t != COPT_HELP && t != COPT_STRLIST && 0 == (opt->type&COPT_CALLBACK)){
				/* print default value */
				*(p++) = '[';
				p += sprintf_option_value(opt, p);
				*(p++) = ']';
				*p = 0;
			}
		}
    }
    fprintf(f, "%s\n", sz_help);
}

int  coptions_load(const char* filename, const char * section, int flags, copt_t* const opts)
{
    char* buf;
    int line = 0;
    int err = 0;
    int in_sect = 0;
    FILE* f = fopen(filename, "r");

    if(f == NULL){
        if(0 == (flags & COPT_NOERR_MSG)){
            perror(filename);
        }
        return COPT_ERROR;
    }

    buf = (char*)malloc(1024);
    while(fgets(buf, 1024, f)){
        char *e, *val=NULL, *key = buf;
        copt_t * o;
        unsigned short t;

        err = 1;
        line ++;
        while(*key && isspace((int)(*key))) key++;
        if(0 == *key)continue;
        /* search for section */
        if(*key == '['){
            if(section){
                if(in_sect){
                    /* section loading finished */
                    in_sect =0;
                    break;
                }
                key++;
                while(*key && isspace((int)(*key))) key++;
                e = strchr(key, ']');
                if(e == NULL)           goto error;
                *e = 0;
                if(0==strcmp(section, key)){
                    in_sect = 1;
                }
            }
            continue;
        }

        if(section && 0== in_sect)      continue;
        if(*key == '#' || *key == ';')  continue;

        e = strchr(key, '=');
        if(NULL == e)                   goto error;
        val = e+1;
        while(e>key && isspace((int)(*(e-1))))e--;
        if(e == key)                    goto error;
        *e = 0;

        while(*val && isspace((int)(*val))) val ++;

        for(o=opts; o->type != COPT_END; o++){
            if(o->lopt && 0==strcmp(o->lopt, key))
                break;
        }
        if(COPT_END == o->type) {
            err = 2;
            goto error;
        }
        t = o->type & COPT_TYPE_MASK;

        e = val + strlen(val);
        while(e>val && isspace((int)(*(e-1)))) *(--e) = 0;

        if(   t == COPT_STR
           || t == COPT_STRLIST
          ){
			if(0==strcmp("NULL", val) || 0==strcmp("null", val)) val = NULL;
            else                                                 val = strdup(val);
        }

        if(set_option_value(o, key, val)){
            err = 3;
            goto error;
        }

        continue;

    error:
        if(0 == (flags & COPT_NOERR_MSG)){
            switch(err){
            case 1:
                fprintf(stderr, "%s:%d: Syntax error in config file\n", filename, line);
                break;
            case 2:
                fprintf(stderr, "%s:%d: Option is unknown\n", filename, line);
                break;
            case 3:
                fprintf(stderr, "%s:%d: Unsupported value: %s\n", filename, line, val);
                break;
            }
        }
        if(flags & COPT_ERR_CONFIG){
            break;
        }
    }
    free(buf);
    fclose(f);
    return 0;
}
