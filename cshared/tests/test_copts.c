/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##
##  Copyleft (c) 2003 - 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/

#include "../copts.h"
static const char*    logfile = NULL;
static int            use_daemon = 1;
static unsigned short port = 0;
static unsigned long  addr = 0;
static const char*    cfgfile = "not set";

static char*          strs[20];
static const char *   strenum[] = {
    "strenum0",
    "strenum1",
    "strenum2",
    "strenum3",
    "strenum4",
    NULL
};


static int str_callback(const copt_t * opt, const char * option, copt_value_t * value)
{
    printf("STR_CALLBACK: %s: %s\n", option, value->v_str);
    return 0;
}
static int long_callback(const copt_t * opt, const char * option, copt_value_t * value)
{
    printf("LONG_CALLBACK: %s %ld\n", option, value->v_long);
    return 0;
}

static int short_callback(const copt_t * opt, const char * option, copt_value_t * value)
{
    printf("SHORT_CALLBACK: %s %d\n", option, value->v_short);
    return 0;
}

static int arg_callback(const copt_t * opt, const char * option, copt_value_t * value)
{
    printf("ARGUMENT: %s \n", value->v_str);
    return 0;
}

static copt_t options [] = {
    {"h?", "help",      COPT_HELP,                 NULL        , "Print this help page"},
    {"C",  "config",    COPT_CFGFILE,              &cfgfile    , "Config file"         },
    {"b",  "boolvalue", COPT_BOOL   |COPT_CONFIG,  NULL        , "Bool test value"     },
    {"d",  "nodaemon",  COPT_BOOLI  |COPT_CONFIG,  &use_daemon , "Start as daemon"     },
    {"l",  "logfile",   COPT_STR    |COPT_CONFIG,  &logfile    , "Use this log file"   },
    {"e",  "strenum",   COPT_STRENUM|COPT_CONFIG,  &strenum[0] , "String enum values"  },
    {"pP", "port",      COPT_USHORT |COPT_CONFIG,  &port       , "Bind to port"        },
    {"aA", "addr",      COPT_HOST   |COPT_CONFIG,  &addr       , "Bind to ip-address"  },
    {"s",  "strings",   COPT_STRLIST|COPT_CONFIG,  &strs[0]    , "String list"         },
    {"S",  "str_cb",    COPT_STR    |COPT_CALLBACK,&str_callback,"String throw callback" },
    {"L",  "l_cb",      COPT_LONG   |COPT_CALLBACK,&long_callback,"Long throw callback" },
    {"H",  "s_cb",      COPT_SHORT  |COPT_CALLBACK,&short_callback,"Short throw callback" },
    {NULL, NULL,        COPT_CALLBACK,             &arg_callback,NULL },
    {NULL, NULL,        COPT_END,                  NULL        , NULL                  }
};

int main(int argc, char** argv)
{
    int i, j;
    int flags = COPT_DEFAULT|COPT_NOERR_UNKNOWN|COPT_NOAUTOHELP;

    i = coptions(argc, argv, flags, options);

    if(i<0){
        if(COPT_ERC(i)){
            if(i == COPT_EHELP){
                coptions_help(stdout, argv[0], options, "nonarg params");
            }
        }else{
            printf("Unknown option %s\n", argv[0-i]);
        }
    }

    printf("help = %s\n",         options[0].vptr?"yes":"no");
    printf("bool = %s\n",         options[1].vptr?"yes":"no");
    printf("use_daemon = %s\n",   use_daemon?"yes":"no");
    printf("logfile = %s\n",      logfile);
    printf("port = %d\n",         port);
    printf("addr = %08lX\n",      addr);
    printf("strenum=%d(%s)\n",
           (int)(((const char **)options[5].vptr) - &strenum[0]),
           *(const char **)options[5].vptr );
    {
        char ** b = &strs[0];
        char ** e = options[8].vptr;
        printf("strlist: count=%d\n", (int)(e - b));
        while(b < e){
            printf("\t%s\n", *b);
            b++;
        }
    }
    if(i>0){
        printf("\nNonoptions:\n");
        for(j=1; j<i; j++){
            printf("\targv[%d]=%s\n", j, argv[j]);
        }
        printf("\nOptions in order:\n");
        for(; j<argc; j++){
            printf("\targv[%d]=%s\n", j, argv[j]);
        }
    }
    return 0;
}
