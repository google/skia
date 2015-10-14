/*
 * Program entry point, command line parsing
 *
 *  Copyright (C) 2001-2008  Peter Johnson
 *  Copyright (C) 2007-2008  Samuel Thibault
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <util.h>

#include <ctype.h>
#include <libyasm/compat-queue.h>
#include <libyasm/bitvect.h>
#include <libyasm.h>

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include "tasm-options.h"

#ifdef CMAKE_BUILD
#include "yasm-plugin.h"
#endif

#include "license.c"

#define DEFAULT_OBJFMT_MODULE   "bin"

/*@null@*/ /*@only@*/ static char *obj_filename = NULL, *in_filename = NULL;
/*@null@*/ /*@only@*/ static char *list_filename = NULL, *xref_filename = NULL;
/*@null@*/ /*@only@*/ static char *machine_name = NULL;
static int special_options = 0;
static int segment_ordering = 0;
static int cross_reference = 0;
static int floating_point = 0;
static int listing = 0;
static int expanded_listing = 0;
static int case_sensitivity = 0;
static int valid_length = -1;
/*@null@*/ /*@dependent@*/ static yasm_arch *cur_arch = NULL;
/*@null@*/ /*@dependent@*/ static const yasm_arch_module *
    cur_arch_module = NULL;
/*@null@*/ /*@dependent@*/ static const yasm_parser_module *
    cur_parser_module = NULL;
/*@null@*/ /*@dependent@*/ static yasm_preproc *cur_preproc = NULL;
/*@null@*/ /*@dependent@*/ static const yasm_preproc_module *
    cur_preproc_module = NULL;
/*@null@*/ static char *objfmt_keyword = NULL;
/*@null@*/ /*@dependent@*/ static const yasm_objfmt_module *
    cur_objfmt_module = NULL;
/*@null@*/ /*@dependent@*/ static const yasm_dbgfmt_module *
    cur_dbgfmt_module = NULL;
/*@null@*/ /*@dependent@*/ static yasm_listfmt *cur_listfmt = NULL;
/*@null@*/ /*@dependent@*/ static const yasm_listfmt_module *
    cur_listfmt_module = NULL;
static int warning_error = 0;   /* warnings being treated as errors */
static FILE *errfile;
/*@null@*/ /*@only@*/ static char *error_filename = NULL;

/*@null@*/ /*@dependent@*/ static FILE *open_file(const char *filename,
                                                  const char *mode);
static void check_errors(/*@only@*/ yasm_errwarns *errwarns,
                         /*@only@*/ yasm_object *object,
                         /*@only@*/ yasm_linemap *linemap);
static void cleanup(/*@null@*/ /*@only@*/ yasm_object *object);

/* Forward declarations: cmd line parser handlers */
static int opt_special_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_segment_ordering_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_cross_reference_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_floating_point_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_ignore(char *cmd, /*@null@*/ char *param, int extra);
static int opt_listing_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_case_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_valid_length_handler(char *cmd, /*@null@*/ char *param, int extra);

static int opt_warning_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_preproc_option(char *cmd, /*@null@*/ char *param, int extra);
static int opt_exe_handler(char *cmd, /*@null@*/ char *param, int extra);

static /*@only@*/ char *replace_extension(const char *orig, /*@null@*/
                                          const char *ext, const char *def);
static void print_error(const char *fmt, ...);

static /*@exits@*/ void handle_yasm_int_error(const char *file,
                                              unsigned int line,
                                              const char *message);
static /*@exits@*/ void handle_yasm_fatal(const char *message, va_list va);
static const char *handle_yasm_gettext(const char *msgid);
static void print_yasm_error(const char *filename, unsigned long line,
                             const char *msg, /*@null@*/ const char *xref_fn,
                             unsigned long xref_line,
                             /*@null@*/ const char *xref_msg);
static void print_yasm_warning(const char *filename, unsigned long line,
                               const char *msg);

static void apply_preproc_builtins(void);
static void apply_preproc_standard_macros(const yasm_stdmac *stdmacs);
static void apply_preproc_saved_options(void);
static void print_list_keyword_desc(const char *name, const char *keyword);

/* values for special_options */
#define SPECIAL_SHOW_HELP 0x01
#define SPECIAL_SHOW_VERSION 0x02
#define SPECIAL_SHOW_LICENSE 0x04

#define SEGMENT_ORDERING_ALPHABETIC 0x01
#define SEGMENT_ORDERING_SOURCE 0x02

#define FP_EMULATED 0x01
#define FP_REAL 0x02

#define CASE_ALL 0x01
#define CASE_GLOBALS 0x02
#define CASE_NONE 0x04

#define DEBUG_FULL 0x01
#define DEBUG_LINES 0x02
#define DEBUG_NONE 0x04

/* command line options */
static opt_option options[] =
{
    { "version", 0, opt_special_handler, SPECIAL_SHOW_VERSION,
      N_("show version text"), NULL },
    { "license", 0, opt_special_handler, SPECIAL_SHOW_LICENSE,
      N_("show license text"), NULL },
    { "a", 0, opt_segment_ordering_handler, SEGMENT_ORDERING_ALPHABETIC,
      N_("Alphabetic segment ordering"), NULL },
    { "s", 0, opt_segment_ordering_handler, SEGMENT_ORDERING_SOURCE,
      N_("Source segment ordering"), NULL },

    { "c", 0, opt_cross_reference_handler, 0,
      N_("Generate cross-reference in listing"), NULL },

    { "d", 1, opt_preproc_option, 2,
      N_("pre-define a macro, optionally to value"), N_("macro[=value]") },

    { "e", 0, opt_floating_point_handler, FP_EMULATED,
      N_("Emulated floating-point instructions (not supported)"), NULL },
    { "r", 0, opt_floating_point_handler, FP_REAL,
      N_("Real floating-point instructions"), NULL },

    { "h", 0, opt_special_handler, SPECIAL_SHOW_HELP,
      N_("show help text"), NULL },
    { "?", 0, opt_special_handler, SPECIAL_SHOW_HELP,
      N_("show help text"), NULL },

    { "i", 1, opt_preproc_option, 0,
      N_("add include path"), N_("path") },

    { "j", 1, opt_ignore, 0,
      N_("Jam in an assemble directive CMD (eg. /jIDEAL) (not supported)"), NULL },

    { "k", 1, opt_ignore, 0,
      N_("Hash table capacity (ignored)"), N_("# symbols") },

    { "l", 0, opt_listing_handler, 0,
      N_("Generate listing"), N_("l=normal listing, la=expanded listing") },

    { "ml", 0, opt_case_handler, CASE_ALL,
      N_("Case sensitivity on all symbols"), NULL },
    { "mx", 0, opt_case_handler, CASE_GLOBALS,
      N_("Case sensitivity on global symbols"), NULL },
    { "mu", 0, opt_case_handler, CASE_NONE,
      N_("No case sensitivity on symbols"), NULL },
    { "mv", 0, opt_valid_length_handler, 0,
      N_("Set maximum valid length for symbols"), N_("length") },

    { "m", 1, opt_ignore, 0,
      N_("Allow multiple passes to resolve forward reference (ignored)"), N_("number of passes") },

    { "n", 0, opt_ignore, 0,
      N_("Suppress symbol tables in listing"), NULL },

    { "o", 0, opt_ignore, 0,
      N_("Object code"), N_("os: standard, o: standard w/overlays, op: Phar Lap, oi: IBM") },

    { "p", 0, opt_ignore, 0,
      N_("Check for code segment overrides in protected mode"), NULL },
    { "q", 0, opt_ignore, 0,
      N_("Suppress OBJ records not needed for linking (ignored)"), NULL },
    { "t", 0, opt_ignore, 0,
      N_("Suppress messages if successful assembly"), NULL },
    { "u", 0, opt_ignore, 0,
      N_("Set version emulation"), N_("Version") },
    { "w", 1, opt_warning_handler, 0,
      N_("Set warning level"), N_("w0=none, w1=w2=warnings on, w-xxx/w+xxx=disable/enable warning xxx") },
    { "x", 0, opt_ignore, 0,
      N_("Include false conditionals in listing"), NULL },
    { "zi", 0, opt_ignore, DEBUG_FULL,
      N_("Full debug info"), NULL },
    { "zd", 0, opt_ignore, DEBUG_LINES,
      N_("Line numbers debug info"), NULL },
    { "zn", 0, opt_ignore, DEBUG_NONE,
      N_("No debug info"), NULL },
    { "z", 0, opt_ignore, 0,
      N_("Display source line with error message (ignored)"), NULL },

    { "b", 0, opt_exe_handler, 0,
      N_("Build a (very) basic .exe file"), NULL },
};

/* version message */
/*@observer@*/ static const char *version_msg[] = {
    PACKAGE_STRING,
#if !defined(DONT_EMBED_BUILD_METADATA) || defined(OFFICIAL_BUILD)
    "Compiled on " __DATE__ ".",
#endif
    "Copyright (c) 2001-2010 Peter Johnson and other Yasm developers.",
    "Run yasm --license for licensing overview and summary."
};

/* help messages */
/*@observer@*/ static const char *help_head = N_(
    "usage: tasm [option]* source [,object] [,listing] [,xref] \n"
    "Options:\n");
/*@observer@*/ static const char *help_tail = N_(
    "\n"
    "source is asm source to be assembled.\n"
    "\n"
    "Sample invocation:\n"
    "   tasm /zi source.asm\n"
    "\n"
    "Report bugs to bug-yasm@tortall.net\n");

/* parsed command line storage until appropriate modules have been loaded */
typedef STAILQ_HEAD(constcharparam_head, constcharparam) constcharparam_head;

typedef struct constcharparam {
    STAILQ_ENTRY(constcharparam) link;
    const char *param;
    int id;
} constcharparam;

static constcharparam_head preproc_options;

static int
do_assemble(void)
{
    yasm_object *object;
    const char *base_filename;
    /*@null@*/ FILE *obj = NULL;
    yasm_arch_create_error arch_error;
    yasm_linemap *linemap;
    yasm_errwarns *errwarns = yasm_errwarns_create();
    int i, matched;

    /* Initialize line map */
    linemap = yasm_linemap_create();
    yasm_linemap_set(linemap, in_filename, 0, 1, 1);

    /* determine the object filename if not specified */
    if (!obj_filename) {
        if (in_filename == NULL)
            /* Default to yasm.out if no obj filename specified */
            obj_filename = yasm__xstrdup("yasm.out");
        else {
            /* replace (or add) extension to base filename */
            yasm__splitpath(in_filename, &base_filename);
            if (base_filename[0] == '\0')
                obj_filename = yasm__xstrdup("yasm.out");
            else
                obj_filename = replace_extension(base_filename,
                                                 "obj",
                                                 "yasm.out");
        }
    }

    cur_arch = yasm_arch_create(cur_arch_module, machine_name,
                                cur_parser_module->keyword, &arch_error);
    if (!cur_arch) {
        switch (arch_error) {
            case YASM_ARCH_CREATE_BAD_MACHINE:
                print_error(_("%s: `%s' is not a valid %s for %s `%s'"),
                            _("FATAL"), machine_name, _("machine"),
                            _("architecture"), cur_arch_module->keyword);
                break;
            case YASM_ARCH_CREATE_BAD_PARSER:
                print_error(_("%s: `%s' is not a valid %s for %s `%s'"),
                            _("FATAL"), cur_parser_module->keyword,
                            _("parser"), _("architecture"),
                            cur_arch_module->keyword);
                break;
            default:
                print_error(_("%s: unknown architecture error"), _("FATAL"));
        }

        return EXIT_FAILURE;
    }

    /* Create object */
    object = yasm_object_create(in_filename, obj_filename, cur_arch,
                                cur_objfmt_module, cur_dbgfmt_module);
    if (!object) {
        yasm_error_class eclass;
        unsigned long xrefline;
        /*@only@*/ /*@null@*/ char *estr, *xrefstr;

        yasm_error_fetch(&eclass, &estr, &xrefline, &xrefstr);
        print_error("%s: %s", _("FATAL"), estr);
        yasm_xfree(estr);
        yasm_xfree(xrefstr);

        cleanup(object);
        return EXIT_FAILURE;
    }

    /* Get a fresh copy of objfmt_module as it may have changed. */
    cur_objfmt_module = ((yasm_objfmt_base *)object->objfmt)->module;

    /* Check to see if the requested preprocessor is in the allowed list
     * for the active parser.
     */
    matched = 0;
    for (i=0; cur_parser_module->preproc_keywords[i]; i++)
        if (yasm__strcasecmp(cur_parser_module->preproc_keywords[i],
                             cur_preproc_module->keyword) == 0)
            matched = 1;
    if (!matched) {
        print_error(_("%s: `%s' is not a valid %s for %s `%s'"), _("FATAL"),
                    cur_preproc_module->keyword, _("preprocessor"),
                    _("parser"), cur_parser_module->keyword);
        cleanup(object);
        return EXIT_FAILURE;
    }

    cur_preproc = yasm_preproc_create(cur_preproc_module, in_filename,
                                      object->symtab, linemap, errwarns);

    apply_preproc_builtins();
    apply_preproc_standard_macros(cur_parser_module->stdmacs);
    apply_preproc_standard_macros(cur_objfmt_module->stdmacs);
    apply_preproc_saved_options();

    /* Get initial x86 BITS setting from object format */
    if (strcmp(cur_arch_module->keyword, "x86") == 0) {
        yasm_arch_set_var(cur_arch, "mode_bits",
                          cur_objfmt_module->default_x86_mode_bits);
    }

    /* Parse! */
    cur_parser_module->do_parse(object, cur_preproc, list_filename != NULL,
                                linemap, errwarns);

    check_errors(errwarns, object, linemap);

    /* Finalize parse */
    yasm_object_finalize(object, errwarns);
    check_errors(errwarns, object, linemap);

    /* Optimize */
    yasm_object_optimize(object, errwarns);
    check_errors(errwarns, object, linemap);

    /* generate any debugging information */
    yasm_dbgfmt_generate(object, linemap, errwarns);
    check_errors(errwarns, object, linemap);

    /* open the object file for output (if not already opened by dbg objfmt) */
    if (!obj && strcmp(cur_objfmt_module->keyword, "dbg") != 0) {
        obj = open_file(obj_filename, "wb");
        if (!obj) {
            cleanup(object);
            return EXIT_FAILURE;
        }
    }

    /* Write the object file */
    yasm_objfmt_output(object, obj?obj:stderr,
                       strcmp(cur_dbgfmt_module->keyword, "null"), errwarns);

    /* Close object file */
    if (obj)
        fclose(obj);

    /* If we had an error at this point, we also need to delete the output
     * object file (to make sure it's not left newer than the source).
     */
    if (yasm_errwarns_num_errors(errwarns, warning_error) > 0)
        remove(obj_filename);
    check_errors(errwarns, object, linemap);

    /* Open and write the list file */
    if (list_filename) {
        FILE *list = open_file(list_filename, "wt");
        if (!list) {
            cleanup(object);
            return EXIT_FAILURE;
        }
        /* Initialize the list format */
        cur_listfmt = yasm_listfmt_create(cur_listfmt_module, in_filename,
                                          obj_filename);
        yasm_listfmt_output(cur_listfmt, list, linemap, cur_arch);
        fclose(list);
    }

    yasm_errwarns_output_all(errwarns, linemap, warning_error,
                             print_yasm_error, print_yasm_warning);

    yasm_linemap_destroy(linemap);
    yasm_errwarns_destroy(errwarns);
    cleanup(object);
    return EXIT_SUCCESS;
}

/* main function */
/*@-globstate -unrecog@*/
int
main(int argc, char *argv[])
{
    size_t i;

    errfile = stderr;

#if defined(HAVE_SETLOCALE) && defined(HAVE_LC_MESSAGES)
    setlocale(LC_MESSAGES, "");
#endif
#if defined(LOCALEDIR)
    bindtextdomain(PACKAGE, LOCALEDIR);
#endif
    textdomain(PACKAGE);

    /* Initialize errwarn handling */
    yasm_internal_error_ = handle_yasm_int_error;
    yasm_fatal = handle_yasm_fatal;
    yasm_gettext_hook = handle_yasm_gettext;
    yasm_errwarn_initialize();

    /* Initialize BitVector (needed for intnum/floatnum). */
    if (BitVector_Boot() != ErrCode_Ok) {
        print_error(_("%s: could not initialize BitVector"), _("FATAL"));
        return EXIT_FAILURE;
    }

    /* Initialize intnum and floatnum */
    yasm_intnum_initialize();
    yasm_floatnum_initialize();

#ifdef CMAKE_BUILD
    /* Load standard modules */
    if (!load_plugin("yasmstd")) {
        print_error(_("%s: could not load standard modules"), _("FATAL"));
        return EXIT_FAILURE;
    }
#endif

    /* Initialize parameter storage */
    STAILQ_INIT(&preproc_options);

    if (parse_cmdline(argc, argv, options, NELEMS(options), print_error))
        return EXIT_FAILURE;

    switch (special_options) {
        case SPECIAL_SHOW_HELP:
            /* Does gettext calls internally */
            help_msg(help_head, help_tail, options, NELEMS(options));
            return EXIT_SUCCESS;
        case SPECIAL_SHOW_VERSION:
            for (i=0; i<NELEMS(version_msg); i++)
                printf("%s\n", version_msg[i]);
            return EXIT_SUCCESS;
        case SPECIAL_SHOW_LICENSE:
            for (i=0; i<NELEMS(license_msg); i++)
                printf("%s\n", license_msg[i]);
            return EXIT_SUCCESS;
    }

    /* Open error file if specified. */
    if (error_filename) {
        errfile = open_file(error_filename, "wt");
        if (!errfile)
            return EXIT_FAILURE;
    }

    /* If not already specified, default to bin as the object format. */
    if (!cur_objfmt_module) {
        if (!objfmt_keyword)
            objfmt_keyword = yasm__xstrdup(DEFAULT_OBJFMT_MODULE);
        cur_objfmt_module = yasm_load_objfmt(objfmt_keyword);
        if (!cur_objfmt_module) {
            print_error(_("%s: could not load default %s"), _("FATAL"),
                        _("object format"));
            return EXIT_FAILURE;
        }
    }

    /* TASM's architecture is x86 */
    cur_arch_module = yasm_load_arch("x86");
    if (!cur_arch_module) {
        print_error(_("%s: could not load %s"), _("FATAL"),
                    _("architecture"));
        return EXIT_FAILURE;
    }
    machine_name =
        yasm__xstrdup(cur_arch_module->default_machine_keyword);

    /* Check for arch help */
    if (machine_name && strcmp(machine_name, "help") == 0) {
        const yasm_arch_machine *m = cur_arch_module->machines;
        printf(_("Available %s for %s `%s':\n"), _("machines"),
               _("architecture"), cur_arch_module->keyword);
        while (m->keyword && m->name) {
            print_list_keyword_desc(m->name, m->keyword);
            m++;
        }
        return EXIT_SUCCESS;
    }

    cur_parser_module = yasm_load_parser("tasm");
    if (!cur_parser_module) {
        print_error(_("%s: could not load %s"), _("FATAL"),
                    _("parser"));
        cleanup(NULL);
        return EXIT_FAILURE;
    }

    /* If not already specified, default to the parser's default preproc. */
    if (!cur_preproc_module) {
        cur_preproc_module =
            yasm_load_preproc(cur_parser_module->default_preproc_keyword);
        if (!cur_preproc_module) {
            print_error(_("%s: could not load default %s"), _("FATAL"),
                        _("preprocessor"));
            cleanup(NULL);
            return EXIT_FAILURE;
        }
    }

    /* Determine input filename and open input file. */
    if (!in_filename) {
        print_error(_("No input files specified"));
        return EXIT_FAILURE;
    }

    /* If list file enabled, make sure we have a list format loaded. */
    if (list_filename) {
        /* use nasm as the list format. */
        cur_listfmt_module = yasm_load_listfmt("nasm");
    }

    /* If not already specified, default to null as the debug format. */
    if (!cur_dbgfmt_module) {
        cur_dbgfmt_module = yasm_load_dbgfmt("null");
        if (!cur_dbgfmt_module) {
            print_error(_("%s: could not load default %s"), _("FATAL"),
                        _("debug format"));
            return EXIT_FAILURE;
        }
    }

    return do_assemble();
}
/*@=globstate =unrecog@*/

/* Open the object file.  Returns 0 on failure. */
static FILE *
open_file(const char *filename, const char *mode)
{
    FILE *f;

    f = fopen(filename, mode);
    if (!f)
        print_error(_("could not open file `%s'"), filename);
    return f;
}

static void
check_errors(yasm_errwarns *errwarns, yasm_object *object,
             yasm_linemap *linemap)
{
    if (yasm_errwarns_num_errors(errwarns, warning_error) > 0) {
        yasm_errwarns_output_all(errwarns, linemap, warning_error,
                                 print_yasm_error, print_yasm_warning);
        yasm_linemap_destroy(linemap);
        yasm_errwarns_destroy(errwarns);
        cleanup(object);
        exit(EXIT_FAILURE);
    }
}

/* Define DO_FREE to 1 to enable deallocation of all data structures.
 * Useful for detecting memory leaks, but slows down execution unnecessarily
 * (as the OS will free everything we miss here).
 */
#define DO_FREE 1

/* Cleans up all allocated structures. */
static void
cleanup(yasm_object *object)
{
    if (DO_FREE) {
        if (cur_listfmt)
            yasm_listfmt_destroy(cur_listfmt);
        if (cur_preproc)
            yasm_preproc_destroy(cur_preproc);
        if (object)
            yasm_object_destroy(object);

        yasm_floatnum_cleanup();
        yasm_intnum_cleanup();

        yasm_errwarn_cleanup();

        BitVector_Shutdown();
    }

    if (DO_FREE) {
        if (in_filename)
            yasm_xfree(in_filename);
        if (obj_filename)
            yasm_xfree(obj_filename);
        if (list_filename)
            yasm_xfree(list_filename);
        if (xref_filename)
            yasm_xfree(xref_filename);
        if (machine_name)
            yasm_xfree(machine_name);
        if (objfmt_keyword)
            yasm_xfree(objfmt_keyword);
    }

    if (errfile != stderr && errfile != stdout)
        fclose(errfile);
#ifdef CMAKE_BUILD
    unload_plugins();
#endif
}

/*
 *  Command line options handlers
 */
static char ** const filenames[] = {
    &in_filename, &obj_filename, &list_filename, &xref_filename, NULL
}, ** const * cur_filename = &filenames[0];

static int filename_handler(char *param) {
    if (!*cur_filename) {
        print_error(_("error: too many files on command line."));
        return 1;
    }

    if (*param)
            **cur_filename = yasm__xstrdup(param);

    return 0;
}
int
not_an_option_handler(char *param) {
    char *c, *d = param;

    while ((c = strchr(d, ','))) {
        *c = '\0';
        if (filename_handler(d))
            return 1;
        d = c + 1;
        cur_filename++;
    }
    filename_handler(d);
    return 0;
}

static int
opt_special_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    if (special_options == 0)
        special_options = extra;
    return 0;
}

static int
opt_segment_ordering_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    segment_ordering = extra;
    return 0;
}

static int
opt_cross_reference_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    cross_reference = 1;
    return 0;
}

static int
opt_floating_point_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    floating_point = extra;
    return 0;
}

static int
opt_ignore(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    return 0;
}

static int
opt_listing_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    if (param && param[0]) {
        if (param[0] != 'a')
            return 1;
        expanded_listing = 1;
    }
    listing = 1;
    return 0;
}

static int
opt_case_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    case_sensitivity = extra;
    return 0;
}

static int
opt_valid_length_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    valid_length = atoi(param);
    return 0;
}

static int
opt_warning_handler(char *cmd, /*@unused@*/ char *param, int extra)
{
    /* is it disabling the warning instead of enabling? */
    void (*action)(yasm_warn_class wclass) = NULL;

    if (cmd[0] == '0') {
        /* /w0, disable warnings */
        yasm_warn_disable_all();
        return 0;
    }

    if (cmd[0] == '1' || cmd[0] == '2') {
        /* /w[12], enable warnings */
        yasm_warn_enable(YASM_WARN_UNREC_CHAR);
        yasm_warn_enable(YASM_WARN_ORPHAN_LABEL);
        yasm_warn_enable(YASM_WARN_UNINIT_CONTENTS);
        return 0;
    }

    /* detect no- prefix to disable the warning */
    if (cmd[0] == '-') {
        action = yasm_warn_disable;
    } else if (cmd[0] == '+') {
        action = yasm_warn_enable;
    } else return 1;

    /* skip past '+/-' */
    cmd++;

    if (cmd[0] == '\0')
        /* just /w- or /w+, so definitely not valid */
        return 1;
    else if (strcmp(cmd, "error") == 0)
        warning_error = (action == yasm_warn_enable);
    else if (strcmp(cmd, "unrecognized-char") == 0)
        action(YASM_WARN_UNREC_CHAR);
    else if (strcmp(cmd, "orphan-labels") == 0)
        action(YASM_WARN_ORPHAN_LABEL);
    else if (strcmp(cmd, "uninit-contents") == 0)
        action(YASM_WARN_UNINIT_CONTENTS);
    else if (strcmp(cmd, "size-override") == 0)
        action(YASM_WARN_SIZE_OVERRIDE);
    else
        return 1;

    return 0;
}

static int
opt_preproc_option(/*@unused@*/ char *cmd, char *param, int extra)
{
    constcharparam *cp;
    cp = yasm_xmalloc(sizeof(constcharparam));
    cp->param = param;
    cp->id = extra;
    STAILQ_INSERT_TAIL(&preproc_options, cp, link);
    return 0;
}

static int
opt_exe_handler(char *cmd, /*@unused@*/ char *param, int extra)
{
    objfmt_keyword = yasm__xstrdup("dosexe");
    return 0;
}

static void
apply_preproc_builtins()
{
    char *predef;

    if (!objfmt_keyword)
        objfmt_keyword = yasm__xstrdup(DEFAULT_OBJFMT_MODULE);

    /* Define standard YASM assembly-time macro constants */
    predef = yasm_xmalloc(strlen("__YASM_OBJFMT__=")
                          + strlen(objfmt_keyword) + 1);
    strcpy(predef, "__YASM_OBJFMT__=");
    strcat(predef, objfmt_keyword);
    yasm_preproc_define_builtin(cur_preproc, predef);
    yasm_xfree(predef);
}

static void
apply_preproc_standard_macros(const yasm_stdmac *stdmacs)
{
    int i, matched;

    if (!stdmacs)
        return;

    matched = -1;
    for (i=0; stdmacs[i].parser; i++)
        if (yasm__strcasecmp(stdmacs[i].parser,
                             cur_parser_module->keyword) == 0 &&
            yasm__strcasecmp(stdmacs[i].preproc,
                             cur_preproc_module->keyword) == 0)
            matched = i;
    if (matched >= 0 && stdmacs[matched].macros)
        yasm_preproc_add_standard(cur_preproc, stdmacs[matched].macros);
}

static void
apply_preproc_saved_options()
{
    constcharparam *cp, *cpnext;

    void (*funcs[3])(yasm_preproc *, const char *);
    funcs[0] = cur_preproc_module->add_include_file;
    funcs[1] = cur_preproc_module->predefine_macro;
    funcs[2] = cur_preproc_module->undefine_macro;

    STAILQ_FOREACH(cp, &preproc_options, link) {
        if (0 <= cp->id && cp->id < 3 && funcs[cp->id])
            funcs[cp->id](cur_preproc, cp->param);
    }

    cp = STAILQ_FIRST(&preproc_options);
    while (cp != NULL) {
        cpnext = STAILQ_NEXT(cp, link);
        yasm_xfree(cp);
        cp = cpnext;
    }
    STAILQ_INIT(&preproc_options);
}

/* Replace extension on a filename (or append one if none is present).
 * If output filename would be identical to input (same extension out as in),
 * returns (copy of) def.
 * A NULL ext means the trailing '.' should NOT be included, whereas a "" ext
 * means the trailing '.' should be included.
 */
static char *
replace_extension(const char *orig, /*@null@*/ const char *ext,
                  const char *def)
{
    char *out, *outext;
    size_t deflen, outlen;

    /* allocate enough space for full existing name + extension */
    outlen = strlen(orig) + 2;
    if (ext)
        outlen += strlen(ext) + 1;
    deflen = strlen(def) + 1;
    if (outlen < deflen)
        outlen = deflen;
    out = yasm_xmalloc(outlen);

    strcpy(out, orig);
    outext = strrchr(out, '.');
    if (outext) {
        /* Existing extension: make sure it's not the same as the replacement
         * (as we don't want to overwrite the source file).
         */
        outext++;   /* advance past '.' */
        if (ext && strcmp(outext, ext) == 0) {
            outext = NULL;  /* indicate default should be used */
            print_error(
                _("file name already ends in `.%s': output will be in `%s'"),
                ext, def);
        }
    } else {
        /* No extension: make sure the output extension is not empty
         * (again, we don't want to overwrite the source file).
         */
        if (!ext)
            print_error(
                _("file name already has no extension: output will be in `%s'"),
                def);
        else {
            outext = strrchr(out, '\0');    /* point to end of the string */
            *outext++ = '.';                    /* append '.' */
        }
    }

    /* replace extension or use default name */
    if (outext) {
        if (!ext) {
            /* Back up and replace '.' with string terminator */
            outext--;
            *outext = '\0';
        } else
            strcpy(outext, ext);
    } else
        strcpy(out, def);

    return out;
}

void
print_list_keyword_desc(const char *name, const char *keyword)
{
    printf("%4s%-12s%s\n", "", keyword, name);
}

static void
print_error(const char *fmt, ...)
{
    va_list va;
    fprintf(errfile, "tasm: ");
    va_start(va, fmt);
    vfprintf(errfile, fmt, va);
    va_end(va);
    fputc('\n', errfile);
}

static /*@exits@*/ void
handle_yasm_int_error(const char *file, unsigned int line, const char *message)
{
    fprintf(stderr, _("INTERNAL ERROR at %s, line %u: %s\n"), file, line,
            gettext(message));
#ifdef HAVE_ABORT
    abort();
#else
    exit(EXIT_FAILURE);
#endif
}

static /*@exits@*/ void
handle_yasm_fatal(const char *fmt, va_list va)
{
    fprintf(errfile, "**%s**: ", _("Fatal"));
    vfprintf(errfile, gettext(fmt), va);
    fputc('\n', errfile);
    exit(EXIT_FAILURE);
}

static const char *
handle_yasm_gettext(const char *msgid)
{
    return gettext(msgid);
}

static void
print_yasm_error(const char *filename, unsigned long line, const char *msg,
                 const char *xref_fn, unsigned long xref_line,
                 const char *xref_msg)
{
    if (line)
        fprintf(errfile, "**%s** %s(%lu) %s\n", _("Error"), filename, line, msg);
    else
        fprintf(errfile, "**%s** %s %s\n", _("Error"), filename, msg);

    if (/* xref_fn && */ xref_msg) {
        if (xref_line)
            fprintf(errfile, "**%s** %s(%lu) %s\n", _("Error"), filename, xref_line, xref_msg);
        else
            fprintf(errfile, "**%s** %s %s\n", _("Error"), filename, xref_msg);
    }
}

static void
print_yasm_warning(const char *filename, unsigned long line, const char *msg)
{
    if (line)
        fprintf(errfile, "*%s* %s(%lu) %s\n", _("Warning"), filename, line, msg);
    else
        fprintf(errfile, "*%s* %s %s\n", _("Warning"), filename, msg);
}
