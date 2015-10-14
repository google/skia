/*
 * Incbin bytecode
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
#include "util.h"

#include "libyasm-stdint.h"
#include "coretype.h"

#include "linemap.h"

#include "errwarn.h"
#include "intnum.h"
#include "expr.h"
#include "value.h"

#include "bytecode.h"

#include "file.h"


typedef struct bytecode_incbin {
    /*@only@*/ char *filename;          /* file to include data from */
    const char *from;           /* filename of what contained incbin */

    /* starting offset to read from (NULL=0) */
    /*@only@*/ /*@null@*/ yasm_expr *start;

    /* maximum number of bytes to read (NULL=no limit) */
    /*@only@*/ /*@null@*/ yasm_expr *maxlen;
} bytecode_incbin;

static void bc_incbin_destroy(void *contents);
static void bc_incbin_print(const void *contents, FILE *f, int indent_level);
static void bc_incbin_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc);
static int bc_incbin_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                              void *add_span_data);
static int bc_incbin_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                             unsigned char *bufstart, void *d,
                             yasm_output_value_func output_value,
                             /*@null@*/ yasm_output_reloc_func output_reloc);

static const yasm_bytecode_callback bc_incbin_callback = {
    bc_incbin_destroy,
    bc_incbin_print,
    bc_incbin_finalize,
    NULL,
    bc_incbin_calc_len,
    yasm_bc_expand_common,
    bc_incbin_tobytes,
    0
};


static void
bc_incbin_destroy(void *contents)
{
    bytecode_incbin *incbin = (bytecode_incbin *)contents;
    yasm_xfree(incbin->filename);
    yasm_expr_destroy(incbin->start);
    yasm_expr_destroy(incbin->maxlen);
    yasm_xfree(contents);
}

static void
bc_incbin_print(const void *contents, FILE *f, int indent_level)
{
    const bytecode_incbin *incbin = (const bytecode_incbin *)contents;
    fprintf(f, "%*s_IncBin_\n", indent_level, "");
    fprintf(f, "%*sFilename=`%s'\n", indent_level, "",
            incbin->filename);
    fprintf(f, "%*sStart=", indent_level, "");
    if (!incbin->start)
        fprintf(f, "nil (0)");
    else
        yasm_expr_print(incbin->start, f);
    fprintf(f, "%*sMax Len=", indent_level, "");
    if (!incbin->maxlen)
        fprintf(f, "nil (unlimited)");
    else
        yasm_expr_print(incbin->maxlen, f);
    fprintf(f, "\n");
}

static void
bc_incbin_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    bytecode_incbin *incbin = (bytecode_incbin *)bc->contents;
    yasm_value val;

    if (yasm_value_finalize_expr(&val, incbin->start, prev_bc, 0))
        yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                       N_("start expression too complex"));
    else if (val.rel)
        yasm_error_set(YASM_ERROR_NOT_ABSOLUTE,
                       N_("start expression not absolute"));
    incbin->start = val.abs;

    if (yasm_value_finalize_expr(&val, incbin->maxlen, prev_bc, 0))
        yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                       N_("maximum length expression too complex"));
    else if (val.rel)
        yasm_error_set(YASM_ERROR_NOT_ABSOLUTE,
                       N_("maximum length expression not absolute"));
    incbin->maxlen = val.abs;
}

static int
bc_incbin_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                   void *add_span_data)
{
    bytecode_incbin *incbin = (bytecode_incbin *)bc->contents;
    FILE *f;
    /*@dependent@*/ /*@null@*/ const yasm_intnum *num;
    unsigned long start = 0, maxlen = 0xFFFFFFFFUL, flen;

    /* Try to convert start to integer value */
    if (incbin->start) {
        num = yasm_expr_get_intnum(&incbin->start, 0);
        if (num)
            start = yasm_intnum_get_uint(num);
        if (!num) {
            /* FIXME */
            yasm_error_set(YASM_ERROR_NOT_IMPLEMENTED,
                           N_("incbin does not yet understand non-constant"));
            return -1;
        }
    }

    /* Try to convert maxlen to integer value */
    if (incbin->maxlen) {
        num = yasm_expr_get_intnum(&incbin->maxlen, 0);
        if (num)
            maxlen = yasm_intnum_get_uint(num);
        if (!num) {
            /* FIXME */
            yasm_error_set(YASM_ERROR_NOT_IMPLEMENTED,
                           N_("incbin does not yet understand non-constant"));
            return -1;
        }
    }

    /* Open file and determine its length */
    f = yasm_fopen_include(incbin->filename, incbin->from, "rb", NULL);
    if (!f) {
        yasm_error_set(YASM_ERROR_IO,
                       N_("`incbin': unable to open file `%s'"),
                       incbin->filename);
        return -1;
    }
    if (fseek(f, 0L, SEEK_END) < 0) {
        yasm_error_set(YASM_ERROR_IO,
                       N_("`incbin': unable to seek on file `%s'"),
                       incbin->filename);
        return -1;
    }
    flen = (unsigned long)ftell(f);
    fclose(f);

    /* Compute length of incbin from start, maxlen, and len */
    if (start > flen) {
        yasm_warn_set(YASM_WARN_GENERAL,
                      N_("`incbin': start past end of file `%s'"),
                      incbin->filename);
        start = flen;
    }
    flen -= start;
    if (incbin->maxlen)
        if (maxlen < flen)
            flen = maxlen;
    bc->len += flen;
    return 0;
}

static int
bc_incbin_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                  unsigned char *bufstart, void *d,
                  yasm_output_value_func output_value,
                  /*@unused@*/ yasm_output_reloc_func output_reloc)
{
    bytecode_incbin *incbin = (bytecode_incbin *)bc->contents;
    FILE *f;
    /*@dependent@*/ /*@null@*/ const yasm_intnum *num;
    unsigned long start = 0;

    /* Convert start to integer value */
    if (incbin->start) {
        num = yasm_expr_get_intnum(&incbin->start, 0);
        if (!num)
            yasm_internal_error(
                N_("could not determine start in bc_tobytes_incbin"));
        start = yasm_intnum_get_uint(num);
    }

    /* Open file */
    f = yasm_fopen_include(incbin->filename, incbin->from, "rb", NULL);
    if (!f) {
        yasm_error_set(YASM_ERROR_IO, N_("`incbin': unable to open file `%s'"),
                       incbin->filename);
        return 1;
    }

    /* Seek to start of data */
    if (fseek(f, (long)start, SEEK_SET) < 0) {
        yasm_error_set(YASM_ERROR_IO,
                       N_("`incbin': unable to seek on file `%s'"),
                       incbin->filename);
        fclose(f);
        return 1;
    }

    /* Read len bytes */
    if (fread(*bufp, 1, (size_t)bc->len, f) < (size_t)bc->len) {
        yasm_error_set(YASM_ERROR_IO,
                       N_("`incbin': unable to read %lu bytes from file `%s'"),
                       bc->len, incbin->filename);
        fclose(f);
        return 1;
    }

    *bufp += bc->len;
    fclose(f);
    return 0;
}

yasm_bytecode *
yasm_bc_create_incbin(char *filename, yasm_expr *start, yasm_expr *maxlen,
                      yasm_linemap *linemap, unsigned long line)
{
    bytecode_incbin *incbin = yasm_xmalloc(sizeof(bytecode_incbin));
    unsigned long xline;

    /* Find from filename based on line number */
    yasm_linemap_lookup(linemap, line, &incbin->from, &xline);

    /*@-mustfree@*/
    incbin->filename = filename;
    incbin->start = start;
    incbin->maxlen = maxlen;
    /*@=mustfree@*/

    return yasm_bc_create_common(&bc_incbin_callback, incbin, line);
}
