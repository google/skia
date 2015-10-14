/*
 * Align bytecode
 *
 *  Copyright (C) 2005-2007  Peter Johnson
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

#include "errwarn.h"
#include "intnum.h"
#include "expr.h"

#include "bytecode.h"


typedef struct bytecode_align {
    /*@only@*/ yasm_expr *boundary;     /* alignment boundary */

    /* What to fill intervening locations with, NULL if using code_fill */
    /*@only@*/ /*@null@*/ yasm_expr *fill;

    /* Maximum number of bytes to skip, NULL if no maximum. */
    /*@only@*/ /*@null@*/ yasm_expr *maxskip;

    /* Code fill, NULL if using 0 fill */
    /*@null@*/ const unsigned char **code_fill;
} bytecode_align;

static void bc_align_destroy(void *contents);
static void bc_align_print(const void *contents, FILE *f, int indent_level);
static void bc_align_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc);
static int bc_align_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                             void *add_span_data);
static int bc_align_expand(yasm_bytecode *bc, int span, long old_val,
                           long new_val, /*@out@*/ long *neg_thres,
                           /*@out@*/ long *pos_thres);
static int bc_align_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                            unsigned char *bufstart, void *d,
                            yasm_output_value_func output_value,
                            /*@null@*/ yasm_output_reloc_func output_reloc);

static const yasm_bytecode_callback bc_align_callback = {
    bc_align_destroy,
    bc_align_print,
    bc_align_finalize,
    NULL,
    bc_align_calc_len,
    bc_align_expand,
    bc_align_tobytes,
    YASM_BC_SPECIAL_OFFSET
};


static void
bc_align_destroy(void *contents)
{
    bytecode_align *align = (bytecode_align *)contents;
    if (align->boundary)
        yasm_expr_destroy(align->boundary);
    if (align->fill)
        yasm_expr_destroy(align->fill);
    if (align->maxskip)
        yasm_expr_destroy(align->maxskip);
    yasm_xfree(contents);
}

static void
bc_align_print(const void *contents, FILE *f, int indent_level)
{
    const bytecode_align *align = (const bytecode_align *)contents;
    fprintf(f, "%*s_Align_\n", indent_level, "");
    fprintf(f, "%*sBoundary=", indent_level, "");
    yasm_expr_print(align->boundary, f);
    fprintf(f, "\n%*sFill=", indent_level, "");
    yasm_expr_print(align->fill, f);
    fprintf(f, "\n%*sMax Skip=", indent_level, "");
    yasm_expr_print(align->maxskip, f);
    fprintf(f, "\n");
}

static void
bc_align_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    bytecode_align *align = (bytecode_align *)bc->contents;
    if (!yasm_expr_get_intnum(&align->boundary, 0))
        yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                       N_("align boundary must be a constant"));
    if (align->fill && !yasm_expr_get_intnum(&align->fill, 0))
        yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                       N_("align fill must be a constant"));
    if (align->maxskip && !yasm_expr_get_intnum(&align->maxskip, 0))
        yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                       N_("align maximum skip must be a constant"));
}

static int
bc_align_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                  void *add_span_data)
{
    long neg_thres = 0;
    long pos_thres = 0;

    if (bc_align_expand(bc, 0, 0, (long)bc->offset, &neg_thres,
                        &pos_thres) < 0)
        return -1;

    return 0;
}

static int
bc_align_expand(yasm_bytecode *bc, int span, long old_val, long new_val,
                /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres)
{
    bytecode_align *align = (bytecode_align *)bc->contents;
    unsigned long end;
    unsigned long boundary =
        yasm_intnum_get_uint(yasm_expr_get_intnum(&align->boundary, 0));

    if (boundary == 0) {
        bc->len = 0;
        *pos_thres = new_val;
        return 0;
    }

    end = (unsigned long)new_val;
    if ((unsigned long)new_val & (boundary-1))
        end = ((unsigned long)new_val & ~(boundary-1)) + boundary;

    *pos_thres = (long)end;
    bc->len = end - (unsigned long)new_val;

    if (align->maxskip) {
        unsigned long maxskip =
            yasm_intnum_get_uint(yasm_expr_get_intnum(&align->maxskip, 0));
        if (bc->len > maxskip) {
            *pos_thres = (long)end-maxskip-1;
            bc->len = 0;
        }
    }
    return 1;
}

static int
bc_align_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                 unsigned char *bufstart, void *d,
                 yasm_output_value_func output_value,
                 /*@unused@*/ yasm_output_reloc_func output_reloc)
{
    bytecode_align *align = (bytecode_align *)bc->contents;
    unsigned long len;
    unsigned long boundary =
        yasm_intnum_get_uint(yasm_expr_get_intnum(&align->boundary, 0));

    if (boundary == 0)
        return 0;
    else {
        unsigned long end = bc->offset;
        if (bc->offset & (boundary-1))
            end = (bc->offset & ~(boundary-1)) + boundary;
        len = end - bc->offset;
        if (len == 0)
            return 0;
        if (align->maxskip) {
            unsigned long maxskip =
                yasm_intnum_get_uint(yasm_expr_get_intnum(&align->maxskip, 0));
            if (len > maxskip)
                return 0;
        }
    }

    if (align->fill) {
        unsigned long v;
        v = yasm_intnum_get_uint(yasm_expr_get_intnum(&align->fill, 0));
        memset(*bufp, (int)v, len);
        *bufp += len;
    } else if (align->code_fill) {
        unsigned long maxlen = 15;
        while (!align->code_fill[maxlen] && maxlen>0)
            maxlen--;
        if (maxlen == 0) {
            yasm_error_set(YASM_ERROR_GENERAL,
                           N_("could not find any code alignment size"));
            return 1;
        }

        /* Fill with maximum code fill as much as possible */
        while (len > maxlen) {
            memcpy(*bufp, align->code_fill[maxlen], maxlen);
            *bufp += maxlen;
            len -= maxlen;
        }

        if (!align->code_fill[len]) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("invalid alignment size %d"), len);
            return 1;
        }
        /* Handle rest of code fill */
        memcpy(*bufp, align->code_fill[len], len);
        *bufp += len;
    } else {
        /* Just fill with 0 */
        memset(*bufp, 0, len);
        *bufp += len;
    }
    return 0;
}

yasm_bytecode *
yasm_bc_create_align(yasm_expr *boundary, yasm_expr *fill,
                     yasm_expr *maxskip, const unsigned char **code_fill,
                     unsigned long line)
{
    bytecode_align *align = yasm_xmalloc(sizeof(bytecode_align));

    align->boundary = boundary;
    align->fill = fill;
    align->maxskip = maxskip;
    align->code_fill = code_fill;

    return yasm_bc_create_common(&bc_align_callback, align, line);
}
