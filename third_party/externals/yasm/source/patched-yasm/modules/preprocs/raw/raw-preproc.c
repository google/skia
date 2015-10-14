/*
 * Raw preprocessor (preforms NO preprocessing)
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

#include <libyasm.h>


#define BSIZE 512

typedef struct yasm_preproc_raw {
    yasm_preproc_base preproc;   /* base structure */

    FILE *in;
    yasm_linemap *cur_lm;
    yasm_errwarns *errwarns;
} yasm_preproc_raw;

yasm_preproc_module yasm_raw_LTX_preproc;

static yasm_preproc *
raw_preproc_create(const char *in_filename, yasm_symtab *symtab,
                   yasm_linemap *lm, yasm_errwarns *errwarns)
{
    FILE *f;
    yasm_preproc_raw *preproc_raw = yasm_xmalloc(sizeof(yasm_preproc_raw));

    if (strcmp(in_filename, "-") != 0) {
        f = fopen(in_filename, "r");
        if (!f)
            yasm__fatal( N_("Could not open input file") );
    }
    else
        f = stdin;

    preproc_raw->preproc.module = &yasm_raw_LTX_preproc;
    preproc_raw->in = f;
    preproc_raw->cur_lm = lm;
    preproc_raw->errwarns = errwarns;

    return (yasm_preproc *)preproc_raw;
}

static void
raw_preproc_destroy(yasm_preproc *preproc)
{
    yasm_xfree(preproc);
}

static char *
raw_preproc_get_line(yasm_preproc *preproc)
{
    yasm_preproc_raw *preproc_raw = (yasm_preproc_raw *)preproc;
    int bufsize = BSIZE;
    char *buf = yasm_xmalloc((size_t)bufsize);
    char *p;

    /* Loop to ensure entire line is read (don't want to limit line length). */
    p = buf;
    for (;;) {
        if (!fgets(p, bufsize-(p-buf), preproc_raw->in)) {
            if (ferror(preproc_raw->in)) {
                yasm_error_set(YASM_ERROR_IO,
                               N_("error when reading from file"));
                yasm_errwarn_propagate(preproc_raw->errwarns,
                    yasm_linemap_get_current(preproc_raw->cur_lm));
            }
            break;
        }
        p += strlen(p);
        if (p > buf && p[-1] == '\n')
            break;
        if ((p-buf)+1 >= bufsize) {
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
raw_preproc_get_included_file(yasm_preproc *preproc, char *buf,
                              size_t max_size)
{
    /* no included files */
    return 0;
}

static void
raw_preproc_add_include_file(yasm_preproc *preproc, const char *filename)
{
    /* no pre-include files */
}

static void
raw_preproc_predefine_macro(yasm_preproc *preproc, const char *macronameval)
{
    /* no pre-defining macros */
}

static void
raw_preproc_undefine_macro(yasm_preproc *preproc, const char *macroname)
{
    /* no undefining macros */
}

static void
raw_preproc_define_builtin(yasm_preproc *preproc, const char *macronameval)
{
    /* no builtin defines */
}

static void
raw_preproc_add_standard(yasm_preproc *preproc, const char **macros)
{
    /* no standard macros */
}


/* Define preproc structure -- see preproc.h for details */
yasm_preproc_module yasm_raw_LTX_preproc = {
    "Disable preprocessing",
    "raw",
    raw_preproc_create,
    raw_preproc_destroy,
    raw_preproc_get_line,
    raw_preproc_get_included_file,
    raw_preproc_add_include_file,
    raw_preproc_predefine_macro,
    raw_preproc_undefine_macro,
    raw_preproc_define_builtin,
    raw_preproc_add_standard
};
