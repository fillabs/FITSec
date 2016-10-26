/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##  Date      : 10.11.2005
##
##      C command line arguments and simple config file parser
##
##  Copyleft (c) 2003 - 2007
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/
#ifndef copts_h
#define copts_h
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup COPTS copts - Programm line option processing.
 * @{ */

#ifndef DOXYGEN
    typedef struct copt_t   copt_t;
    typedef enum coptflag_t coptflag_t;
    typedef enum coptype_t  coptype_t;
    typedef enum copterr_t  copterr_t;
#endif
    /** @enum coptype_t
     * Option types definitions.
     */
    enum coptype_t{
        COPT_BOOL ,   /**< Boolean option. Doesn't require arguments.*/
        COPT_BOOLI,   /**< Inverted Boolean option. Doesn't require arguments. */
        COPT_LONG ,   /**< Requires for long argument */
        COPT_ULONG,   /**< Requires for unsigned long argument */
        COPT_SHORT,   /**< Requires for short (16 bit) argument */
        COPT_USHORT,  /**< Requires for unsigned short argument */
        COPT_CHAR ,   /**< Requires for char or unsigned char argument */
        COPT_STR  ,   /**< Requires for string (const char *) arguments */
        COPT_HOST,    /**< Requires for string (const char *) arguments. Checks url syntax. */
        COPT_PATH,    /**< Requires for string (const pchar_t *) arguments. */
        COPT_STRLIST, /**< Requires for string list argument (const char *[])
                       *   Every time when this opion will be occuren in argv
                       *   the value will be assigned to given pointer and
                       *   this pointer will be incremented. */
        COPT_STRENUM, /**< Requires for one of the string in the array given by vptr)
                       *   Array of strings must be terminated by NULL pointer.
                       *   After option found the vptr pointer will point
                       *   to the element corresponding to the argument */
        COPT_CFGFILE, /**< Requires for string (const pchar_t *) arguments.
                       *    Treat it as config file name and load if found.
                       *    If one or more config file options are exists in copt_t list
                       *    this options will be executed before any other options parsing */
        COPT_HELP,    /**< Does't require argument.
                       *    If this option is occured in command line parsing will be
                       *    terminated imediate and COPT_EHELP will be returned */


        COPT_TYPE_MASK = 0x00FF, /**< Option type mask. For internal usage. */
        COPT_CALLBACK  = 0x4000, /**< Mask. Can be or-ed with any other option.
                                  *    That's mean treat vptr as a callback addres to call
                                  *    when option is occured */
        COPT_CONFIG    = 0x8000, /**< Mask. Can be or-ed with any other option.
                                  *    That's mean this option can be reached from config file
                                  *    and have to be writen to.*/

        COPT_END       = 0xFFFF, /**< End of options.
                                  *   If vptr is not NULL, treat it as callback to call for unknown
                                  *   options and non-option values */
    };

#define COPT_INT      COPT_LONG
#define COPT_UINT     COPT_ULONG
#define COPT_IBOOL    COPT_BOOLI

    /** Main options item.
     *   Have to be used to define options items.
     *   Short and long options can be defined.
     *   Possible options notations:
     *     - Boolean options:
     *       - -o
     *       - --option
     *     - Other types except of boolean:
     *       - -o value
     *       - -o=value
     *       - --option=value
     */
    struct copt_t
    {
        const    char*  sopts;   /**< Short options. */
        const    char*  lopt;    /**< Long option.   */
        const coptype_t type;    /**< Option type ( see @ref coptype_t ). */
                 void*  vptr;    /**< Option variable pointer. */
        const    char*  helpstr; /**< Option help string. */
    };

    /**
     * Execute option parser.
     * @param argc  Command line parameters count (from arguments of main() for example).
     * @param argv  Array of command line parameters.
     * @param flags Configuration flags ( @ref coptflag_t ).
     * @param opts  Array of possible options. Must be finished by item with COPT_END type.
     * @return      <ul><li>On success returns the index of the first option argument in the arguments array.<li>On error returns negative index of the invalid option in the arguments array.</ul>
     */
    int  coptions(int argc, char* argv[], int flags, copt_t* opts);

    /** Get enum index from the option variable.
      * @param opts @ref copt_t array.
      * @param idx  Index of the enum option in the array.
      * @param ptr  The initial value of the @a vptr field of the opion array item.
      * @return the integer enum value of the selected item.
     */
#define copts_enum_value(opts,idx,ptr) \
    ((const char **)((opts)[idx]).vptr) - ((const char **)(ptr))

    /**
     * Load options config file.
     * @param filename  File path to load.
     * @param section   If not NULL then try to find the last occurance of the
     *                  given section or load the file complet.
     * @param flags     Configuration flags ( see @ref coptflag_t ).
     * @param opts      The Array of possible option records. Must be finished
     *                  by the item with COPT_END type.
     * @return
                      <ul><li>On success returns 0.<li>On error returns negative line number of the invalid expression.</ul>
     */
    int  coptions_load(const char* filename, const char * section, int flags, copt_t* const opts);

    /**
     * Save current options to the file
     */
    int  coptions_save(const char* filename, const copt_t* const opts);

    /**
     * Save current options to already opened file
     */
    int  coptions_fsave(FILE * fd,  const copt_t* const opts);

    /**
     * Generate and print the help page.
     * @param fd       File descriptor to print the resulting help page.
     * @param prgname  Application name. Can be taken from argv[0].
     * @param opt      Options array.
     * @param usagestr The string to print before option list.
     * @param header   Help page header.
     * @param footer   Help page footer.

     */
    void coptions_help_ex(FILE * fd, const char * prgname, int flags, copt_t* opt, const char* usagestr,
                          const char* header, const char* footer);
    /** The lite version of the @ref coptions_help_ex.
     * @param fd       File descriptor to print the resulting help page.
     * @param prgname  Application name. Can be taken from argv[0].
     * @param opt      Options array.
     * @param usagestr The string to print before option list.
     */
#define coptions_help(fd,prgname,flags,opt,usagestr) \
    coptions_help_ex(fd,prgname,flags,opt,usagestr,NULL,NULL)

    /** Wild value definition */
    typedef union{
        int            v_boolean;
        signed   short v_short;
        unsigned short v_ushort;
        signed   long  v_long;
        unsigned long  v_ulong;
        char           v_char;
        char *         v_str;
    }copt_value_t;

    /** The type of callback function to be called for the option having
        @ref COPT_CALLBACK bit set in the @e type field of the @ref copt_t structure.

        These functions must return zero if option was successfully processed,
        @ref COPT_EHELP to generate option help string or negative value when
        some error was occured.
        @param opt The current item of the options array.
        @param option String option given.
        @param value Pointer to the option value.
     */

    typedef int copt_callback(const copt_t * opt, const char * option, const copt_value_t * value);

/** Inverted Boolean True. */
#define IBOOL_YES ((void*)-1)

/** @enum coptflag_t
    Option flag mask values.
*/
    enum coptflag_t
    {
        COPT_DEFAULT       = 0x0000, /**< No special flags given. */
        COPT_NOAUTOHELP    = 0x0001, /**< Does not provide automatic help messages. */
        COPT_NOCONFIG      = 0x0002, /**< Does not search for config files. */
        COPT_NOREORDER     = 0x0004, /**< Does not reorder command line array. */
        COPT_NOERR_MSG     = 0x0010, /**< Be silent. */
        COPT_NOERR_UNKNOWN = 0x0020, /**< Treat unknown options as non-option args.*/
        COPT_NOERR_ARG     = 0x0040, /**< Does not produce an error if the required
                                          option's argument is omited or have
                                          incompatible type. */
        COPT_NOERR         = 0x0070, /**< Does not produce any errors. */
        COPT_ERR_CONFIG    = 0x0080, /**< Does not produce config errors. */
		COPT_NOHELP_MSG    = 0x0100, /**< Does not print help messages. */
		COPT_HELP_NOVALUES = 0x0200, /**< Does not print default values. */
	};

    /** @{
        @ref coptions return codes.
     */
    /** Help option (-h or --help) vaw invoked. Need to print help page.*/
#define COPT_EHELP   ((int)(0x80000001))
    /** Some error was occured.*/
#define COPT_ERROR   ((int)(0x80000002))
#define COPT_ERC(rc) (rc < 0 && 0==(rc & 0x8000000))
    /**@}*/

/** @} */

/**
 *   @example test_copts.c
 */

#ifdef __cplusplus
}
#endif

#endif
