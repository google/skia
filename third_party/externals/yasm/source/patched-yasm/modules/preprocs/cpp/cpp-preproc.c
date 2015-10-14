/*
 * Invoke an external C preprocessor
 *
 *  Copyright (C) 2007       Paul Barker
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
#include <libyasm.h>

/* TODO: Use autoconf to get the limit on the command line length. */
#define CMDLINE_SIZE 32770

#define BSIZE 512

/* Pre-declare the preprocessor module object. */
yasm_preproc_module yasm_cpp_LTX_preproc;

/*******************************************************************************
    Structures.
*******************************************************************************/

/* An entry in a list of arguments to pass to cpp. */
typedef struct cpp_arg_entry {
    TAILQ_ENTRY(cpp_arg_entry) entry;

    /*
        The operator (eg "-I") and the parameter (eg "include/"). op is expected
        to point to a string literal, whereas param is expected to be a copy of
        the parameter which is free'd when no-longer needed (in
        cpp_preproc_destroy()).
    */
    const char *op;
    char *param;
} cpp_arg_entry;

typedef struct yasm_preproc_cpp {
    yasm_preproc_base preproc;   /* base structure */

    /* List of arguments to pass to cpp. */
    TAILQ_HEAD(cpp_arg_head, cpp_arg_entry) cpp_args;

    char *filename;
    FILE *f, *f_deps;
    yasm_linemap *cur_lm;
    yasm_errwarns *errwarns;

    int flags;
} yasm_preproc_cpp;

/* Flag values for yasm_preproc_cpp->flags. */
#define CPP_HAS_BEEN_INVOKED        0x01
#define CPP_HAS_GENERATED_DEPS      0x02

/*******************************************************************************
    Internal functions and helpers.
*******************************************************************************/

/*
    Append a string to the command line, ensuring that we don't overflow the
    buffer.
*/
#define APPEND(s) do {                              \
    size_t _len = strlen(s);                        \
    if (p + _len >= limit)                          \
        yasm__fatal(N_("command line too long!"));  \
    strcpy(p, s);                                   \
    p += _len;                                      \
} while (0)

/*
    Put all the options together into a command line that can be used to invoke
    cpp.
*/
static char *
cpp_build_cmdline(yasm_preproc_cpp *pp, const char *extra)
{
    char *cmdline, *p, *limit;
    cpp_arg_entry *arg;

    /* Initialize command line. */
    cmdline = p = yasm_xmalloc(strlen(CPP_PROG)+CMDLINE_SIZE);
    limit = p + CMDLINE_SIZE;
    strcpy(p, CPP_PROG);
    p += strlen(CPP_PROG);

    arg = TAILQ_FIRST(&pp->cpp_args);

    /* Append arguments from the list. */
    while ( arg ) {
        APPEND(" ");
        APPEND(arg->op);
        APPEND(" ");
        APPEND(arg->param);

        arg = TAILQ_NEXT(arg, entry);
    }

    /* Append extra arguments. */
    if (extra) {
        APPEND(" ");
        APPEND(extra);
    }
    /* Append final arguments. */
    APPEND(" -x assembler-with-cpp ");
    APPEND(pp->filename);

    return cmdline;
}

/* Invoke the c preprocessor. */
static void
cpp_invoke(yasm_preproc_cpp *pp)
{
    char *cmdline;

    cmdline = cpp_build_cmdline(pp, NULL);

#ifdef HAVE_POPEN
    pp->f = popen(cmdline, "r");
    if (!pp->f)
        yasm__fatal( N_("Failed to execute preprocessor") );
#else
    yasm__fatal( N_("Cannot execute preprocessor, no popen available") );
#endif

    yasm_xfree(cmdline);
}

/* Free memory used by the list of arguments. */
static void
cpp_destroy_args(yasm_preproc_cpp *pp)
{
    cpp_arg_entry *arg;

    while ( (arg = TAILQ_FIRST(&pp->cpp_args)) ) {
        TAILQ_REMOVE(&pp->cpp_args, arg, entry);
        yasm_xfree(arg->param);
        yasm_xfree(arg);
    }
}

/* Invoke the c preprocessor to generate dependency info. */
static void
cpp_generate_deps(yasm_preproc_cpp *pp)
{
    char *cmdline;

    cmdline = cpp_build_cmdline(pp, "-M");

#ifdef HAVE_POPEN
    pp->f_deps = popen(cmdline, "r");
    if (!pp->f_deps)
        yasm__fatal( N_("Failed to execute preprocessor") );
#else
    yasm__fatal( N_("Cannot execute preprocessor, no popen available") );
#endif

    yasm_xfree(cmdline);
}

/*******************************************************************************
    Interface functions.
*******************************************************************************/
static yasm_preproc *
cpp_preproc_create(const char *in, yasm_symtab *symtab, yasm_linemap *lm,
                   yasm_errwarns *errwarns)
{
    yasm_preproc_cpp *pp = yasm_xmalloc(sizeof(yasm_preproc_cpp));
    void * iter;
    const char * inc_dir;

    pp->preproc.module = &yasm_cpp_LTX_preproc;
    pp->f = pp->f_deps = NULL;
    pp->cur_lm = lm;
    pp->errwarns = errwarns;
    pp->flags = 0;
    pp->filename = yasm__xstrdup(in);

    TAILQ_INIT(&pp->cpp_args);

    /* Iterate through the list of include dirs. */
    iter = NULL;
    while ((inc_dir = yasm_get_include_dir(&iter)) != NULL) {
        cpp_arg_entry *arg = yasm_xmalloc(sizeof(cpp_arg_entry));
        arg->op = "-I";
        arg->param = yasm__xstrdup(inc_dir);

        TAILQ_INSERT_TAIL(&pp->cpp_args, arg, entry);
    }

    return (yasm_preproc *)pp;
}

static void
cpp_preproc_destroy(yasm_preproc *preproc)
{
    yasm_preproc_cpp *pp = (yasm_preproc_cpp *)preproc;

    if (pp->f) {
#ifdef HAVE_POPEN
        if (pclose(pp->f) != 0)
            yasm__fatal( N_("Preprocessor exited with failure") );
#endif
    }

    cpp_destroy_args(pp);

    yasm_xfree(pp->filename);
    yasm_xfree(pp);
}

static char *
cpp_preproc_get_line(yasm_preproc *preproc)
{
    yasm_preproc_cpp *pp = (yasm_preproc_cpp *)preproc;
    int bufsize = BSIZE;
    char *buf, *p;

    if (! (pp->flags & CPP_HAS_BEEN_INVOKED) ) {
        pp->flags |= CPP_HAS_BEEN_INVOKED;

        cpp_invoke(pp);
    }

    /*
        Once the preprocessor has been run, we're just dealing with a normal
        file.
    */

    /* Loop to ensure entire line is read (don't want to limit line length). */
    buf = yasm_xmalloc((size_t)bufsize);
    p = buf;
    for (;;) {
        if (!fgets(p, bufsize-(p-buf), pp->f)) {
            if (ferror(pp->f)) {
                yasm_error_set(YASM_ERROR_IO,
                               N_("error when reading from file"));
                yasm_errwarn_propagate(pp->errwarns,
                    yasm_linemap_get_current(pp->cur_lm));
            }
            break;
        }
        p += strlen(p);
        if (p > buf && p[-1] == '\n')
            break;
        if ((p-buf) >= bufsize) {
            /* Increase size of buffer */
            char *oldbuf = buf;
            bufsize *= 2;
            buf = yasm_xrealloc(buf, (size_t)bufsize);
            p = buf + (p-oldbuf);
        }
    }

    if (p == buf) {
        /* No data; must be at EOF */
        yasm_xfree(buf);
        return NULL;
    }

    /* Strip the line ending */
    buf[strcspn(buf, "\r\n")] = '\0';

    return buf;
}

static size_t
cpp_preproc_get_included_file(yasm_preproc *preproc, char *buf,
                              size_t max_size)
{
    char *p = buf;
    int ch = '\0';
    size_t n = 0;
    yasm_preproc_cpp *pp = (yasm_preproc_cpp *)preproc;

    if (! (pp->flags & CPP_HAS_GENERATED_DEPS) ) {
        pp->flags |= CPP_HAS_GENERATED_DEPS;

        cpp_generate_deps(pp);

        /* Skip target name and first dependency. */
        while (ch != ':')
            ch = fgetc(pp->f_deps);

        fgetc(pp->f_deps);      /* Discard space after colon. */

        while (ch != ' ' && ch != EOF)
            ch = fgetc(pp->f_deps);

        if (ch == EOF)
            return 0;
    }

    while (n < max_size) {
        ch = fgetc(pp->f_deps);

        if (ch == ' ' || ch == EOF) {
            *p = '\0';
            return n;
        }

        /* Eat any silly characters. */
        if (ch < ' ')
            continue;

        *p++ = ch;
        n++;
    }

    /* Ensure the buffer is null-terminated. */
    *(p - 1) = '\0';
    return n;
}

static void
cpp_preproc_add_include_file(yasm_preproc *preproc, const char *filename)
{
    yasm_preproc_cpp *pp = (yasm_preproc_cpp *)preproc;

    cpp_arg_entry *arg = yasm_xmalloc(sizeof(cpp_arg_entry));
    arg->op = "-include";
    arg->param = yasm__xstrdup(filename);

    TAILQ_INSERT_TAIL(&pp->cpp_args, arg, entry);
}

static void
cpp_preproc_predefine_macro(yasm_preproc *preproc, const char *macronameval)
{
    yasm_preproc_cpp *pp = (yasm_preproc_cpp *)preproc;

    cpp_arg_entry *arg = yasm_xmalloc(sizeof(cpp_arg_entry));
    arg->op = "-D";
    arg->param = yasm__xstrdup(macronameval);

    TAILQ_INSERT_TAIL(&pp->cpp_args, arg, entry);
}

static void
cpp_preproc_undefine_macro(yasm_preproc *preproc, const char *macroname)
{
    yasm_preproc_cpp *pp = (yasm_preproc_cpp *)preproc;

    cpp_arg_entry *arg = yasm_xmalloc(sizeof(cpp_arg_entry));
    arg->op = "-U";
    arg->param = yasm__xstrdup(macroname);

    TAILQ_INSERT_TAIL(&pp->cpp_args, arg, entry);
}

static void
cpp_preproc_define_builtin(yasm_preproc *preproc, const char *macronameval)
{
    /* Handle a builtin as if it were a predefine. */
    cpp_preproc_predefine_macro(preproc, macronameval);
}

static void
cpp_preproc_add_standard(yasm_preproc *preproc, const char **macros)
{
    /* TODO */
}

/*******************************************************************************
    Preprocessor module object.
*******************************************************************************/

yasm_preproc_module yasm_cpp_LTX_preproc = {
    "Run input through external C preprocessor",
    "cpp",
    cpp_preproc_create,
    cpp_preproc_destroy,
    cpp_preproc_get_line,
    cpp_preproc_get_included_file,
    cpp_preproc_add_include_file,
    cpp_preproc_predefine_macro,
    cpp_preproc_undefine_macro,
    cpp_preproc_define_builtin,
    cpp_preproc_add_standard
};
