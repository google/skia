/*
 * Imported NASM preprocessor - glue code
 *
 *  Copyright (C) 2002-2007  Peter Johnson
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

#include "nasm.h"
#include "nasmlib.h"
#include "nasm-pp.h"
#include "nasm-eval.h"

typedef struct yasm_preproc_nasm {
    yasm_preproc_base preproc;   /* Base structure */

    FILE *in;
    char *line;
    char *file_name;
    long prior_linnum;
    int lineinc;
} yasm_preproc_nasm;
yasm_symtab *nasm_symtab;
static yasm_linemap *cur_lm;
static yasm_errwarns *cur_errwarns;
int tasm_compatible_mode = 0;
int tasm_locals;
const char *tasm_segment;

#include "nasm-version.c"

typedef struct preproc_dep {
    STAILQ_ENTRY(preproc_dep) link;
    char *name;
} preproc_dep;

static STAILQ_HEAD(preproc_dep_head, preproc_dep) *preproc_deps;
static int done_dep_preproc;

yasm_preproc_module yasm_nasm_LTX_preproc;

static void
nil_listgen_init(char *p, efunc e)
{
}

static void
nil_listgen_cleanup(void)
{
}

static void
nil_listgen_output(long v, const void *d, unsigned long v2)
{
}

static void
nil_listgen_line(int v, char *p)
{
}

static void
nil_listgen_uplevel(int v)
{
}

static void
nil_listgen_downlevel(int v)
{
}

static ListGen nil_list = {
    nil_listgen_init,
    nil_listgen_cleanup,
    nil_listgen_output,
    nil_listgen_line,
    nil_listgen_uplevel,
    nil_listgen_downlevel
};


static void
nasm_efunc(int severity, const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    switch (severity & ERR_MASK) {
        case ERR_WARNING:
            yasm_warn_set_va(YASM_WARN_PREPROC, fmt, va);
            break;
        case ERR_NONFATAL:
            yasm_error_set_va(YASM_ERROR_GENERAL, fmt, va);
            break;
        case ERR_FATAL:
            yasm_fatal(fmt, va);
            /*@notreached@*/
            break;
        case ERR_PANIC:
            yasm_internal_error(fmt);   /* FIXME */
            break;
        case ERR_DEBUG:
            break;
    }
    va_end(va);
    yasm_errwarn_propagate(cur_errwarns,
        yasm_linemap_poke(cur_lm, nasm_src_get_fname(),
                          (unsigned long)nasm_src_get_linnum()));
}

static yasm_preproc *
nasm_preproc_create(const char *in_filename, yasm_symtab *symtab,
                    yasm_linemap *lm, yasm_errwarns *errwarns)
{
    FILE *f;
    yasm_preproc_nasm *preproc_nasm = yasm_xmalloc(sizeof(yasm_preproc_nasm));

    preproc_nasm->preproc.module = &yasm_nasm_LTX_preproc;

    if (strcmp(in_filename, "-") != 0) {
        f = fopen(in_filename, "r");
        if (!f)
            yasm__fatal( N_("Could not open input file") );
    }
    else
        f = stdin;

    preproc_nasm->in = f;
    nasm_symtab = symtab;
    cur_lm = lm;
    cur_errwarns = errwarns;
    preproc_deps = NULL;
    done_dep_preproc = 0;
    preproc_nasm->line = NULL;
    preproc_nasm->file_name = NULL;
    preproc_nasm->prior_linnum = 0;
    preproc_nasm->lineinc = 0;
    nasmpp.reset(f, in_filename, 2, nasm_efunc, nasm_evaluate, &nil_list);

    pp_extra_stdmac(nasm_version_mac);

    return (yasm_preproc *)preproc_nasm;
}

static void
nasm_preproc_destroy(yasm_preproc *preproc)
{
    yasm_preproc_nasm *preproc_nasm = (yasm_preproc_nasm *)preproc;
    nasmpp.cleanup(0);
    if (preproc_nasm->line)
        yasm_xfree(preproc_nasm->line);
    if (preproc_nasm->file_name)
        yasm_xfree(preproc_nasm->file_name);
    yasm_xfree(preproc);
    if (preproc_deps)
        yasm_xfree(preproc_deps);
}

static char *
nasm_preproc_get_line(yasm_preproc *preproc)
{
    yasm_preproc_nasm *preproc_nasm = (yasm_preproc_nasm *)preproc;
    long linnum;
    int altline;
    char *line;

    if (preproc_nasm->line) {
        char *retval = preproc_nasm->line;
        preproc_nasm->line = NULL;
        return retval;
    }

    line = nasmpp.getline();
    if (!line)
    {
        nasmpp.cleanup(1);
        return NULL;    /* EOF */
    }

    linnum = preproc_nasm->prior_linnum += preproc_nasm->lineinc;
    altline = nasm_src_get(&linnum, &preproc_nasm->file_name);
    if (altline != 0) {
        preproc_nasm->lineinc =
            (altline != -1 || preproc_nasm->lineinc != 1);
        preproc_nasm->line = line;
        line = yasm_xmalloc(40+strlen(preproc_nasm->file_name));
        sprintf(line, "%%line %ld+%d %s", linnum,
                preproc_nasm->lineinc, preproc_nasm->file_name);
        preproc_nasm->prior_linnum = linnum;
    }

    return line;
}

void
nasm_preproc_add_dep(char *name)
{
    preproc_dep *dep;

    /* If not processing dependencies, simply return */
    if (!preproc_deps)
        return;

    /* Save in preproc_deps */
    dep = yasm_xmalloc(sizeof(preproc_dep));
    dep->name = yasm__xstrdup(name);
    STAILQ_INSERT_TAIL(preproc_deps, dep, link);
}

static size_t
nasm_preproc_get_included_file(yasm_preproc *preproc, /*@out@*/ char *buf,
                               size_t max_size)
{
    if (!preproc_deps) {
        preproc_deps = yasm_xmalloc(sizeof(struct preproc_dep_head));
        STAILQ_INIT(preproc_deps);
    }

    for (;;) {
        char *line;

        /* Pull first dep out of preproc_deps and return it if there is one */
        if (!STAILQ_EMPTY(preproc_deps)) {
            char *name;
            preproc_dep *dep = STAILQ_FIRST(preproc_deps);
            STAILQ_REMOVE_HEAD(preproc_deps, link);
            name = dep->name;
            yasm_xfree(dep);
            strncpy(buf, name, max_size);
            buf[max_size-1] = '\0';
            yasm_xfree(name);
            return strlen(buf);
        }

        /* No more preprocessing to do */
        if (done_dep_preproc) {
            return 0;
        }

        /* Preprocess some more, throwing away the result */
        line = nasmpp.getline();
        if (line)
            yasm_xfree(line);
        else
            done_dep_preproc = 1;
    }
}

static void
nasm_preproc_add_include_file(yasm_preproc *preproc, const char *filename)
{
    pp_pre_include(filename);
}

static void
nasm_preproc_predefine_macro(yasm_preproc *preproc, const char *macronameval)
{
    char *mnv = yasm__xstrdup(macronameval);
    pp_pre_define(mnv);
    yasm_xfree(mnv);
}

static void
nasm_preproc_undefine_macro(yasm_preproc *preproc, const char *macroname)
{
    char *mn = yasm__xstrdup(macroname);
    pp_pre_undefine(mn);
    yasm_xfree(mn);
}

static void
nasm_preproc_define_builtin(yasm_preproc *preproc, const char *macronameval)
{
    char *mnv = yasm__xstrdup(macronameval);
    pp_builtin_define(mnv);
    yasm_xfree(mnv);
}

static void
nasm_preproc_add_standard(yasm_preproc *preproc, const char **macros)
{
    pp_extra_stdmac(macros);
}

/* Define preproc structure -- see preproc.h for details */
yasm_preproc_module yasm_nasm_LTX_preproc = {
    "Real NASM Preprocessor",
    "nasm",
    nasm_preproc_create,
    nasm_preproc_destroy,
    nasm_preproc_get_line,
    nasm_preproc_get_included_file,
    nasm_preproc_add_include_file,
    nasm_preproc_predefine_macro,
    nasm_preproc_undefine_macro,
    nasm_preproc_define_builtin,
    nasm_preproc_add_standard
};

static yasm_preproc *
tasm_preproc_create(const char *in_filename, yasm_symtab *symtab,
                    yasm_linemap *lm, yasm_errwarns *errwarns)
{
    tasm_compatible_mode = 1;
    return nasm_preproc_create(in_filename, symtab, lm, errwarns);
}

yasm_preproc_module yasm_tasm_LTX_preproc = {
    "Real TASM Preprocessor",
    "tasm",
    tasm_preproc_create,
    nasm_preproc_destroy,
    nasm_preproc_get_line,
    nasm_preproc_get_included_file,
    nasm_preproc_add_include_file,
    nasm_preproc_predefine_macro,
    nasm_preproc_undefine_macro,
    nasm_preproc_define_builtin,
    nasm_preproc_add_standard
};
