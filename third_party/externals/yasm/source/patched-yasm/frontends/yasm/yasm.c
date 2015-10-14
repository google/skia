/*
 * Program entry point, command line parsing
 *
 *  Copyright (C) 2001-2007  Peter Johnson
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

#include "yasm-options.h"

#ifdef CMAKE_BUILD
#include "yasm-plugin.h"
#endif

#include "license.c"

/* Preprocess-only buffer size */
#define PREPROC_BUF_SIZE    16384

/*@null@*/ /*@only@*/ static char *obj_filename = NULL, *in_filename = NULL;
/*@null@*/ /*@only@*/ static char *global_prefix = NULL, *global_suffix = NULL;
/*@null@*/ /*@only@*/ static char *list_filename = NULL, *map_filename = NULL;
/*@null@*/ /*@only@*/ static char *machine_name = NULL;
static int special_options = 0;
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
static int preproc_only = 0;
static unsigned int force_strict = 0;
static int generate_make_dependencies = 0;
static int warning_error = 0;   /* warnings being treated as errors */
static FILE *errfile;
/*@null@*/ /*@only@*/ static char *error_filename = NULL;
static enum {
    EWSTYLE_GNU = 0,
    EWSTYLE_VC
} ewmsg_style = EWSTYLE_GNU;

/*@null@*/ /*@dependent@*/ static FILE *open_file(const char *filename,
                                                  const char *mode);
static void check_errors(/*@only@*/ yasm_errwarns *errwarns,
                         /*@only@*/ yasm_object *object,
                         /*@only@*/ yasm_linemap *linemap);
static void cleanup(/*@null@*/ /*@only@*/ yasm_object *object);

/* Forward declarations: cmd line parser handlers */
static int opt_special_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_arch_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_parser_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_preproc_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_objfmt_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_dbgfmt_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_listfmt_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_listfile_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_objfile_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_mapfile_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_machine_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_strict_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_warning_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_error_file(char *cmd, /*@null@*/ char *param, int extra);
static int opt_error_stdout(char *cmd, /*@null@*/ char *param, int extra);
static int preproc_only_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_include_option(char *cmd, /*@null@*/ char *param, int extra);
static int opt_preproc_option(char *cmd, /*@null@*/ char *param, int extra);
static int opt_ewmsg_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_makedep_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_prefix_handler(char *cmd, /*@null@*/ char *param, int extra);
static int opt_suffix_handler(char *cmd, /*@null@*/ char *param, int extra);
#ifdef CMAKE_BUILD
static int opt_plugin_handler(char *cmd, /*@null@*/ char *param, int extra);
#endif

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
#define SPECIAL_LISTED 0x08

/* command line options */
static opt_option options[] =
{
    { 0, "version", 0, opt_special_handler, SPECIAL_SHOW_VERSION,
      N_("show version text"), NULL },
    { 0, "license", 0, opt_special_handler, SPECIAL_SHOW_LICENSE,
      N_("show license text"), NULL },
    { 'h', "help", 0, opt_special_handler, SPECIAL_SHOW_HELP,
      N_("show help text"), NULL },
    { 'a', "arch", 1, opt_arch_handler, 0,
      N_("select architecture (list with -a help)"), N_("arch") },
    { 'p', "parser", 1, opt_parser_handler, 0,
      N_("select parser (list with -p help)"), N_("parser") },
    { 'r', "preproc", 1, opt_preproc_handler, 0,
      N_("select preprocessor (list with -r help)"), N_("preproc") },
    { 'f', "oformat", 1, opt_objfmt_handler, 0,
      N_("select object format (list with -f help)"), N_("format") },
    { 'g', "dformat", 1, opt_dbgfmt_handler, 0,
      N_("select debugging format (list with -g help)"), N_("debug") },
    { 'L', "lformat", 1, opt_listfmt_handler, 0,
      N_("select list format (list with -L help)"), N_("list") },
    { 'l', "list", 1, opt_listfile_handler, 0,
      N_("name of list-file output"), N_("listfile") },
    { 'o', "objfile", 1, opt_objfile_handler, 0,
      N_("name of object-file output"), N_("filename") },
    { 0, "mapfile", 1, opt_mapfile_handler, 0,
      N_("name of map-file output"), N_("filename") },
    { 'm', "machine", 1, opt_machine_handler, 0,
      N_("select machine (list with -m help)"), N_("machine") },
    { 0, "force-strict", 0, opt_strict_handler, 0,
      N_("treat all sized operands as if `strict' was used"), NULL },
    { 'w', NULL, 0, opt_warning_handler, 1,
      N_("inhibits warning messages"), NULL },
    { 'W', NULL, 0, opt_warning_handler, 0,
      N_("enables/disables warning"), NULL },
    { 'M', NULL, 0, opt_makedep_handler, 0,
      N_("generate Makefile dependencies on stdout"), NULL },
    { 'E', NULL, 1, opt_error_file, 0,
      N_("redirect error messages to file"), N_("file") },
    { 's', NULL, 0, opt_error_stdout, 0,
      N_("redirect error messages to stdout"), NULL },
    { 'e', "preproc-only", 0, preproc_only_handler, 0,
      N_("preprocess only (writes output to stdout by default)"), NULL },
    { 'i', NULL, 1, opt_include_option, 0,
      N_("add include path"), N_("path") },
    { 'I', NULL, 1, opt_include_option, 0,
      N_("add include path"), N_("path") },
    { 'P', NULL, 1, opt_preproc_option, 0,
      N_("pre-include file"), N_("filename") },
    { 'd', NULL, 1, opt_preproc_option, 1,
      N_("pre-define a macro, optionally to value"), N_("macro[=value]") },
    { 'D', NULL, 1, opt_preproc_option, 1,
      N_("pre-define a macro, optionally to value"), N_("macro[=value]") },
    { 'u', NULL, 1, opt_preproc_option, 2,
      N_("undefine a macro"), N_("macro") },
    { 'U', NULL, 1, opt_preproc_option, 2,
      N_("undefine a macro"), N_("macro") },
    { 'X', NULL, 1, opt_ewmsg_handler, 0,
      N_("select error/warning message style (`gnu' or `vc')"), N_("style") },
    { 0, "prefix", 1, opt_prefix_handler, 0,
      N_("prepend argument to name of all external symbols"), N_("prefix") },
    { 0, "suffix", 1, opt_suffix_handler, 0,
      N_("append argument to name of all external symbols"), N_("suffix") },
    { 0, "postfix", 1, opt_suffix_handler, 0,
      N_("append argument to name of all external symbols"), N_("suffix") },
#ifdef CMAKE_BUILD
    { 'N', "plugin", 1, opt_plugin_handler, 0,
      N_("load plugin module"), N_("plugin") },
#endif
};

/* version message */
/*@observer@*/ static const char *version_msg[] = {
    PACKAGE_STRING,
#if !defined(DONT_EMBED_BUILD_METADATA) || defined(OFFICIAL_BUILD)
    "Compiled on " __DATE__ ".",
#endif
    "Copyright (c) 2001-2011 Peter Johnson and other Yasm developers.",
    "Run yasm --license for licensing overview and summary."
};

/* help messages */
/*@observer@*/ static const char *help_head = N_(
    "usage: yasm [option]* file\n"
    "Options:\n");
/*@observer@*/ static const char *help_tail = N_(
    "\n"
    "Files are asm sources to be assembled.\n"
    "\n"
    "Sample invocation:\n"
    "   yasm -f elf -o object.o source.asm\n"
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
do_preproc_only(void)
{
    yasm_linemap *linemap;
    char *preproc_buf;
    size_t got;
    const char *base_filename;
    FILE *out = NULL;
    yasm_errwarns *errwarns = yasm_errwarns_create();

    /* Initialize line map */
    linemap = yasm_linemap_create();
    yasm_linemap_set(linemap, in_filename, 0, 1, 1);

    /* Default output to stdout if not specified or generating dependency
       makefiles */
    if (!obj_filename || generate_make_dependencies) {
        out = stdout;

        /* determine the object filename if not specified, but we need a
            file name for the makefile rule */
        if (generate_make_dependencies && !obj_filename) {
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
                        cur_objfmt_module->extension, "yasm.out");
            }
        }
    } else {
        /* Open output (object) file */
        out = open_file(obj_filename, "wt");
        if (!out)
            return EXIT_FAILURE;
    }

    /* Create preprocessor */
    cur_preproc = yasm_preproc_create(cur_preproc_module, in_filename, NULL,
                                      linemap, errwarns);

    /* Apply macros */
    apply_preproc_builtins();
    apply_preproc_standard_macros(cur_parser_module->stdmacs);
    apply_preproc_standard_macros(cur_objfmt_module->stdmacs);
    apply_preproc_saved_options();

    /* Pre-process until done */
    if (generate_make_dependencies) {
        size_t totlen;

        preproc_buf = yasm_xmalloc(PREPROC_BUF_SIZE);

        fprintf(stdout, "%s: %s", obj_filename, in_filename);
        totlen = strlen(obj_filename)+2+strlen(in_filename);

        while ((got = yasm_preproc_get_included_file(cur_preproc, preproc_buf,
                                                     PREPROC_BUF_SIZE)) != 0) {
            totlen += got;
            if (totlen > 72) {
                fputs(" \\\n  ", stdout);
                totlen = 2;
            }
            fputc(' ', stdout);
            fwrite(preproc_buf, got, 1, stdout);
        }
        fputc('\n', stdout);
        yasm_xfree(preproc_buf);
    } else {
        while ((preproc_buf = yasm_preproc_get_line(cur_preproc)) != NULL) {
            fputs(preproc_buf, out);
            fputc('\n', out);
            yasm_xfree(preproc_buf);
        }
    }

    if (out != stdout)
        fclose(out);

    if (yasm_errwarns_num_errors(errwarns, warning_error) > 0) {
        yasm_errwarns_output_all(errwarns, linemap, warning_error,
                                 print_yasm_error, print_yasm_warning);
        if (out != stdout)
            remove(obj_filename);
        yasm_linemap_destroy(linemap);
        yasm_errwarns_destroy(errwarns);
        cleanup(NULL);
        return EXIT_FAILURE;
    }

    yasm_errwarns_output_all(errwarns, linemap, warning_error,
                             print_yasm_error, print_yasm_warning);
    yasm_linemap_destroy(linemap);
    yasm_errwarns_destroy(errwarns);
    cleanup(NULL);
    return EXIT_SUCCESS;
}

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
                                                 cur_objfmt_module->extension,
                                                 "yasm.out");
        }
    }

    /* Set up architecture using machine and parser. */
    if (!machine_name) {
        /* If we're using x86 and the default objfmt bits is 64, default the
         * machine to amd64.  When we get more arches with multiple machines,
         * we should do this in a more modular fashion.
         */
        if (strcmp(cur_arch_module->keyword, "x86") == 0 &&
            cur_objfmt_module->default_x86_mode_bits == 64)
            machine_name = yasm__xstrdup("amd64");
        else
            machine_name =
                yasm__xstrdup(cur_arch_module->default_machine_keyword);
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

    if (global_prefix)
        yasm_object_set_global_prefix(object, global_prefix);
    if (global_suffix)
        yasm_object_set_global_suffix(object, global_suffix);

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

    yasm_arch_set_var(cur_arch, "force_strict", force_strict);

    /* Try to enable the map file via a map NASM directive.  This is
     * somewhat of a hack.
     */
    if (map_filename) {
        const yasm_directive *dir = &cur_objfmt_module->directives[0];
        matched = 0;
        for (; dir && dir->name; dir++) {
            if (yasm__strcasecmp(dir->name, "map") == 0 &&
                yasm__strcasecmp(dir->parser, "nasm") == 0) {
                yasm_valparamhead vps;
                yasm_valparam *vp;
                matched = 1;
                yasm_vps_initialize(&vps);
                vp = yasm_vp_create_string(NULL, yasm__xstrdup(map_filename));
                yasm_vps_append(&vps, vp);
                dir->handler(object, &vps, NULL, 0);
                yasm_vps_delete(&vps);
            }
        }
        if (!matched) {
            print_error(
                _("warning: object format `%s' does not support map files"),
                cur_objfmt_module->keyword);
        }
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
        case SPECIAL_LISTED:
            /* Printed out earlier */
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
            objfmt_keyword = yasm__xstrdup("bin");
        cur_objfmt_module = yasm_load_objfmt(objfmt_keyword);
        if (!cur_objfmt_module) {
            print_error(_("%s: could not load default %s"), _("FATAL"),
                        _("object format"));
            return EXIT_FAILURE;
        }
    }

    /* Default to x86 as the architecture */
    if (!cur_arch_module) {
        cur_arch_module = yasm_load_arch("x86");
        if (!cur_arch_module) {
            print_error(_("%s: could not load default %s"), _("FATAL"),
                        _("architecture"));
            return EXIT_FAILURE;
        }
    }

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

    /* Default to NASM as the parser */
    if (!cur_parser_module) {
        cur_parser_module = yasm_load_parser("nasm");
        if (!cur_parser_module) {
            print_error(_("%s: could not load default %s"), _("FATAL"),
                        _("parser"));
            cleanup(NULL);
            return EXIT_FAILURE;
        }
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

    /* handle preproc-only case here */
    if (preproc_only)
        return do_preproc_only();

    /* If list file enabled, make sure we have a list format loaded. */
    if (list_filename) {
        /* If not already specified, default to nasm as the list format. */
        if (!cur_listfmt_module) {
            cur_listfmt_module = yasm_load_listfmt("nasm");
            if (!cur_listfmt_module) {
                print_error(_("%s: could not load default %s"), _("FATAL"),
                            _("list format"));
                return EXIT_FAILURE;
            }
        }
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
#define DO_FREE         1

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
        if (map_filename)
            yasm_xfree(map_filename);
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
int
not_an_option_handler(char *param)
{
    if (in_filename) {
        print_error(
            _("warning: can open only one input file, only the last file will be processed"));
        yasm_xfree(in_filename);
    }

    in_filename = yasm__xstrdup(param);

    return 0;
}

int
other_option_handler(char *option)
{
    /* Accept, but ignore, -O and -Onnn, for compatibility with NASM. */
    if (option[0] == '-' && option[1] == 'O') {
        int n = 2;
        for (;;) {
            if (option[n] == '\0')
                return 0;
            if (!isdigit(option[n]))
                return 1;
            n++;
        }
    }
    return 1;
}

static int
opt_special_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param, int extra)
{
    if (special_options == 0)
        special_options = extra;
    return 0;
}

static int
opt_arch_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    assert(param != NULL);
    cur_arch_module = yasm_load_arch(param);
    if (!cur_arch_module) {
        if (!strcmp("help", param)) {
            printf(_("Available yasm %s:\n"), _("architectures"));
            yasm_list_arch(print_list_keyword_desc);
            special_options = SPECIAL_LISTED;
            return 0;
        }
        print_error(_("%s: unrecognized %s `%s'"), _("FATAL"),
                    _("architecture"), param);
        exit(EXIT_FAILURE);
    }
    return 0;
}

static int
opt_parser_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    assert(param != NULL);
    cur_parser_module = yasm_load_parser(param);
    if (!cur_parser_module) {
        if (!strcmp("help", param)) {
            printf(_("Available yasm %s:\n"), _("parsers"));
            yasm_list_parser(print_list_keyword_desc);
            special_options = SPECIAL_LISTED;
            return 0;
        }
        print_error(_("%s: unrecognized %s `%s'"), _("FATAL"), _("parser"),
                    param);
        exit(EXIT_FAILURE);
    }
    return 0;
}

static int
opt_preproc_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    assert(param != NULL);
    cur_preproc_module = yasm_load_preproc(param);
    if (!cur_preproc_module) {
        if (!strcmp("help", param)) {
            printf(_("Available yasm %s:\n"), _("preprocessors"));
            yasm_list_preproc(print_list_keyword_desc);
            special_options = SPECIAL_LISTED;
            return 0;
        }
        print_error(_("%s: unrecognized %s `%s'"), _("FATAL"),
                    _("preprocessor"), param);
        exit(EXIT_FAILURE);
    }
    return 0;
}

static int
opt_objfmt_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    size_t i;
    assert(param != NULL);
    cur_objfmt_module = yasm_load_objfmt(param);
    if (!cur_objfmt_module) {
        if (!strcmp("help", param)) {
            printf(_("Available yasm %s:\n"), _("object formats"));
            yasm_list_objfmt(print_list_keyword_desc);
            special_options = SPECIAL_LISTED;
            return 0;
        }
        print_error(_("%s: unrecognized %s `%s'"), _("FATAL"),
                    _("object format"), param);
        exit(EXIT_FAILURE);
    }
    if (objfmt_keyword)
        yasm_xfree(objfmt_keyword);
    objfmt_keyword = yasm__xstrdup(param);
    for (i=0; i<strlen(objfmt_keyword); i++)
        objfmt_keyword[i] = tolower(objfmt_keyword[i]);
    return 0;
}

static int
opt_dbgfmt_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    assert(param != NULL);
    cur_dbgfmt_module = yasm_load_dbgfmt(param);
    if (!cur_dbgfmt_module) {
        if (!strcmp("help", param)) {
            printf(_("Available yasm %s:\n"), _("debug formats"));
            yasm_list_dbgfmt(print_list_keyword_desc);
            special_options = SPECIAL_LISTED;
            return 0;
        }
        print_error(_("%s: unrecognized %s `%s'"), _("FATAL"),
                    _("debug format"), param);
        exit(EXIT_FAILURE);
    }
    return 0;
}

static int
opt_listfmt_handler(/*@unused@*/ char *cmd, char *param,
                    /*@unused@*/ int extra)
{
    assert(param != NULL);
    cur_listfmt_module = yasm_load_listfmt(param);
    if (!cur_listfmt_module) {
        if (!strcmp("help", param)) {
            printf(_("Available yasm %s:\n"), _("list formats"));
            yasm_list_listfmt(print_list_keyword_desc);
            special_options = SPECIAL_LISTED;
            return 0;
        }
        print_error(_("%s: unrecognized %s `%s'"), _("FATAL"),
                    _("list format"), param);
        exit(EXIT_FAILURE);
    }
    return 0;
}

static int
opt_listfile_handler(/*@unused@*/ char *cmd, char *param,
                     /*@unused@*/ int extra)
{
    if (list_filename) {
        print_error(
            _("warning: can output to only one list file, last specified used"));
        yasm_xfree(list_filename);
    }

    assert(param != NULL);
    list_filename = yasm__xstrdup(param);

    return 0;
}

static int
opt_objfile_handler(/*@unused@*/ char *cmd, char *param,
                    /*@unused@*/ int extra)
{
    if (obj_filename) {
        print_error(
            _("warning: can output to only one object file, last specified used"));
        yasm_xfree(obj_filename);
    }

    assert(param != NULL);
    obj_filename = yasm__xstrdup(param);

    return 0;
}

static int
opt_mapfile_handler(/*@unused@*/ char *cmd, char *param,
                    /*@unused@*/ int extra)
{
    if (map_filename) {
        print_error(
            _("warning: can output to only one map file, last specified used"));
        yasm_xfree(map_filename);
    }

    assert(param != NULL);
    map_filename = yasm__xstrdup(param);

    return 0;
}

static int
opt_machine_handler(/*@unused@*/ char *cmd, char *param,
                    /*@unused@*/ int extra)
{
    if (machine_name)
        yasm_xfree(machine_name);

    assert(param != NULL);
    machine_name = yasm__xstrdup(param);

    return 0;
}

static int
opt_strict_handler(/*@unused@*/ char *cmd,
                   /*@unused@*/ /*@null@*/ char *param,
                   /*@unused@*/ int extra)
{
    force_strict = 1;
    return 0;
}

static int
opt_warning_handler(char *cmd, /*@unused@*/ char *param, int extra)
{
    /* is it disabling the warning instead of enabling? */
    void (*action)(yasm_warn_class wclass) = yasm_warn_enable;

    if (extra == 1) {
        /* -w, disable warnings */
        yasm_warn_disable_all();
        return 0;
    }

    /* skip past 'W' */
    cmd++;

    /* detect no- prefix to disable the warning */
    if (cmd[0] == 'n' && cmd[1] == 'o' && cmd[2] == '-') {
        action = yasm_warn_disable;
        cmd += 3;   /* skip past it to get to the warning name */
    }

    if (cmd[0] == '\0')
        /* just -W or -Wno-, so definitely not valid */
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
opt_error_file(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    if (error_filename) {
        print_error(
            _("warning: can output to only one error file, last specified used"));
        yasm_xfree(error_filename);
    }

    assert(param != NULL);
    error_filename = yasm__xstrdup(param);

    return 0;
}

static int
opt_error_stdout(/*@unused@*/ char *cmd, /*@unused@*/ char *param,
                 /*@unused@*/ int extra)
{
    /* Clear any specified error filename */
    if (error_filename) {
        yasm_xfree(error_filename);
        error_filename = NULL;
    }
    errfile = stdout;
    return 0;
}

static int
preproc_only_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param,
                     /*@unused@*/ int extra)
{
    preproc_only = 1;
    return 0;
}

static int
opt_include_option(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    yasm_add_include_path(param);
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
opt_ewmsg_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    if (yasm__strcasecmp(param, "gnu") == 0 ||
        yasm__strcasecmp(param, "gcc") == 0) {
        ewmsg_style = EWSTYLE_GNU;
    } else if (yasm__strcasecmp(param, "vc") == 0) {
        ewmsg_style = EWSTYLE_VC;
    } else
        print_error(_("warning: unrecognized message style `%s'"), param);

    return 0;
}

static int
opt_makedep_handler(/*@unused@*/ char *cmd, /*@unused@*/ char *param,
                    /*@unused@*/ int extra)
{
    /* Also set preproc_only to 1, we don't want to generate code */
    preproc_only = 1;
    generate_make_dependencies = 1;

    return 0;
}

static int
opt_prefix_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    if (global_prefix)
        yasm_xfree(global_prefix);

    assert(param != NULL);
    global_prefix = yasm__xstrdup(param);

    return 0;
}

static int
opt_suffix_handler(/*@unused@*/ char *cmd, char *param, /*@unused@*/ int extra)
{
    if (global_suffix)
        yasm_xfree(global_suffix);

    assert(param != NULL);
    global_suffix = yasm__xstrdup(param);

    return 0;
}

#ifdef CMAKE_BUILD
static int
opt_plugin_handler(/*@unused@*/ char *cmd, char *param,
                   /*@unused@*/ int extra)
{
    if (!load_plugin(param))
        print_error(_("warning: could not load plugin `%s'"), param);
    return 0;
}
#endif

static void
apply_preproc_builtins()
{
    char *predef;

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
            *outext++ = '.';                /* append '.' */
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
    fprintf(errfile, "yasm: ");
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
    fprintf(errfile, "yasm: %s: ", _("FATAL"));
    vfprintf(errfile, gettext(fmt), va);
    fputc('\n', errfile);
    exit(EXIT_FAILURE);
}

static const char *
handle_yasm_gettext(const char *msgid)
{
    return gettext(msgid);
}

static const char *fmt[2] = {
        "%s:%lu: %s%s\n",       /* GNU */
        "%s(%lu) : %s%s\n"      /* VC */
};

static const char *fmt_noline[2] = {
        "%s: %s%s\n",   /* GNU */
        "%s : %s%s\n"   /* VC */
};

static void
print_yasm_error(const char *filename, unsigned long line, const char *msg,
                 const char *xref_fn, unsigned long xref_line,
                 const char *xref_msg)
{
    if (line)
        fprintf(errfile, fmt[ewmsg_style], filename, line, _("error: "), msg);
    else
        fprintf(errfile, fmt_noline[ewmsg_style], filename, _("error: "), msg);

    if (xref_fn && xref_msg) {
        if (xref_line)
            fprintf(errfile, fmt[ewmsg_style], xref_fn, xref_line, _("error: "),
                    xref_msg);
        else
            fprintf(errfile, fmt_noline[ewmsg_style], xref_fn, _("error: "),
                    xref_msg);
    }
}

static void
print_yasm_warning(const char *filename, unsigned long line, const char *msg)
{
    if (line)
        fprintf(errfile, fmt[ewmsg_style], filename, line, _("warning: "),
                msg);
    else
        fprintf(errfile, fmt_noline[ewmsg_style], filename, _("warning: "),
                msg);
}
