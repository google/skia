/*
 * Bytecode utility functions
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

#include "errwarn.h"
#include "intnum.h"
#include "expr.h"
#include "value.h"
#include "symrec.h"

#include "bytecode.h"


void
yasm_bc_set_multiple(yasm_bytecode *bc, yasm_expr *e)
{
    if (bc->multiple)
        bc->multiple = yasm_expr_create_tree(bc->multiple, YASM_EXPR_MUL, e,
                                             e->line);
    else
        bc->multiple = e;
}

void
yasm_bc_finalize_common(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
}

int
yasm_bc_calc_len_common(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                        void *add_span_data)
{
    yasm_internal_error(N_("bytecode length cannot be calculated"));
    /*@unreached@*/
    return 0;
}

int
yasm_bc_expand_common(yasm_bytecode *bc, int span, long old_val, long new_val,
                      /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres)
{
    yasm_internal_error(N_("bytecode does not have any dependent spans"));
    /*@unreached@*/
    return 0;
}

int
yasm_bc_tobytes_common(yasm_bytecode *bc, unsigned char **buf,
                       unsigned char *bufstart, void *d,
                       yasm_output_value_func output_value,
                       /*@null@*/ yasm_output_reloc_func output_reloc)
{
    yasm_internal_error(N_("bytecode cannot be converted to bytes"));
    /*@unreached@*/
    return 0;
}

void
yasm_bc_transform(yasm_bytecode *bc, const yasm_bytecode_callback *callback,
                  void *contents)
{
    if (bc->callback)
        bc->callback->destroy(bc->contents);
    bc->callback = callback;
    bc->contents = contents;
}

yasm_bytecode *
yasm_bc_create_common(const yasm_bytecode_callback *callback, void *contents,
                      unsigned long line)
{
    yasm_bytecode *bc = yasm_xmalloc(sizeof(yasm_bytecode));

    bc->callback = callback;
    bc->section = NULL;
    bc->multiple = (yasm_expr *)NULL;
    bc->len = 0;
    bc->mult_int = 1;
    bc->line = line;
    bc->offset = ~0UL;  /* obviously incorrect / uninitialized value */
    bc->symrecs = NULL;
    bc->contents = contents;

    return bc;
}

yasm_section *
yasm_bc_get_section(yasm_bytecode *bc)
{
    return bc->section;
}

void
yasm_bc__add_symrec(yasm_bytecode *bc, yasm_symrec *sym)
{
    if (!bc->symrecs) {
        bc->symrecs = yasm_xmalloc(2*sizeof(yasm_symrec *));
        bc->symrecs[0] = sym;
        bc->symrecs[1] = NULL;
    } else {
        /* Very inefficient implementation for large numbers of symbols.  But
         * that would be very unusual, so use the simple algorithm instead.
         */
        size_t count = 1;
        while (bc->symrecs[count])
            count++;
        bc->symrecs = yasm_xrealloc(bc->symrecs,
                                    (count+2)*sizeof(yasm_symrec *));
        bc->symrecs[count] = sym;
        bc->symrecs[count+1] = NULL;
    }
}

void
yasm_bc_destroy(yasm_bytecode *bc)
{
    if (!bc)
        return;

    if (bc->callback)
        bc->callback->destroy(bc->contents);
    yasm_expr_destroy(bc->multiple);
    if (bc->symrecs)
        yasm_xfree(bc->symrecs);
    yasm_xfree(bc);
}

void
yasm_bc_print(const yasm_bytecode *bc, FILE *f, int indent_level)
{
    if (!bc->callback)
        fprintf(f, "%*s_Empty_\n", indent_level, "");
    else
        bc->callback->print(bc->contents, f, indent_level);
    fprintf(f, "%*sMultiple=", indent_level, "");
    if (!bc->multiple)
        fprintf(f, "nil (1)");
    else
        yasm_expr_print(bc->multiple, f);
    fprintf(f, "\n%*sLength=%lu\n", indent_level, "", bc->len);
    fprintf(f, "%*sLine Index=%lu\n", indent_level, "", bc->line);
    fprintf(f, "%*sOffset=%lx\n", indent_level, "", bc->offset);
}

void
yasm_bc_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    if (bc->callback)
        bc->callback->finalize(bc, prev_bc);
    if (bc->multiple) {
        yasm_value val;

        if (yasm_value_finalize_expr(&val, bc->multiple, prev_bc, 0))
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("multiple expression too complex"));
        else if (val.rel)
            yasm_error_set(YASM_ERROR_NOT_ABSOLUTE,
                           N_("multiple expression not absolute"));
        /* Finalize creates NULL output if value=0, but bc->multiple is NULL
         * if value=1 (this difference is to make the common case small).
         * However, this means we need to set bc->multiple explicitly to 0
         * here if val.abs is NULL.
         */
        if (val.abs)
            bc->multiple = val.abs;
        else
            bc->multiple = yasm_expr_create_ident(
                yasm_expr_int(yasm_intnum_create_uint(0)), bc->line);
    }
}

/*@null@*/ yasm_intnum *
yasm_calc_bc_dist(yasm_bytecode *precbc1, yasm_bytecode *precbc2)
{
    unsigned long dist2, dist1;
    yasm_intnum *intn;

    if (precbc1->section != precbc2->section)
        return NULL;

    dist1 = yasm_bc_next_offset(precbc1);
    dist2 = yasm_bc_next_offset(precbc2);
    if (dist2 < dist1) {
        intn = yasm_intnum_create_uint(dist1 - dist2);
        yasm_intnum_calc(intn, YASM_EXPR_NEG, NULL);
        return intn;
    }
    dist2 -= dist1;
    return yasm_intnum_create_uint(dist2);
}

unsigned long
yasm_bc_next_offset(yasm_bytecode *precbc)
{
    return precbc->offset + precbc->len*precbc->mult_int;
}

int
yasm_bc_elem_size(yasm_bytecode *bc)
{
    if (!bc->callback) {
        yasm_internal_error(N_("got empty bytecode in yasm_bc_elem_size"));
        return 0;
    } else if (!bc->callback->elem_size)
        return 0;
    else
        return bc->callback->elem_size(bc);
}

int
yasm_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                 void *add_span_data)
{
    int retval = 0;

    bc->len = 0;

    if (!bc->callback)
        yasm_internal_error(N_("got empty bytecode in yasm_bc_calc_len"));
    else
        retval = bc->callback->calc_len(bc, add_span, add_span_data);

    /* Check for multiples */
    bc->mult_int = 1;
    if (bc->multiple) {
        /*@dependent@*/ /*@null@*/ const yasm_intnum *num;

        num = yasm_expr_get_intnum(&bc->multiple, 0);
        if (num) {
            if (yasm_intnum_sign(num) < 0) {
                yasm_error_set(YASM_ERROR_VALUE, N_("multiple is negative"));
                retval = -1;
            } else
                bc->mult_int = yasm_intnum_get_int(num);
        } else {
            if (yasm_expr__contains(bc->multiple, YASM_EXPR_FLOAT)) {
                yasm_error_set(YASM_ERROR_VALUE,
                    N_("expression must not contain floating point value"));
                retval = -1;
            } else {
                yasm_value value;
                yasm_value_initialize(&value, bc->multiple, 0);
                add_span(add_span_data, bc, 0, &value, 0, 0);
                bc->mult_int = 0;   /* assume 0 to start */
            }
        }
    }

    /* If we got an error somewhere along the line, clear out any calc len */
    if (retval < 0)
        bc->len = 0;

    return retval;
}

int
yasm_bc_expand(yasm_bytecode *bc, int span, long old_val, long new_val,
               /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres)
{
    if (span == 0) {
        bc->mult_int = new_val;
        return 1;
    }
    if (!bc->callback) {
        yasm_internal_error(N_("got empty bytecode in yasm_bc_expand"));
        /*@unreached@*/
        return -1;
    } else
        return bc->callback->expand(bc, span, old_val, new_val, neg_thres,
                                    pos_thres);
}

/*@null@*/ /*@only@*/ unsigned char *
yasm_bc_tobytes(yasm_bytecode *bc, unsigned char *buf, unsigned long *bufsize,
                /*@out@*/ int *gap, void *d,
                yasm_output_value_func output_value,
                /*@null@*/ yasm_output_reloc_func output_reloc)
    /*@sets *buf@*/
{
    /*@only@*/ /*@null@*/ unsigned char *mybuf = NULL;
    unsigned char *bufstart;
    unsigned char *origbuf, *destbuf;
    long i;
    int error = 0;

    long mult;
    if (yasm_bc_get_multiple(bc, &mult, 1) || mult == 0) {
        *bufsize = 0;
        return NULL;
    }
    bc->mult_int = mult;

    /* special case for reserve bytecodes */
    if (bc->callback->special == YASM_BC_SPECIAL_RESERVE) {
        *bufsize = bc->len*bc->mult_int;
        *gap = 1;
        return NULL;    /* we didn't allocate a buffer */
    }
    *gap = 0;

    if (*bufsize < bc->len*bc->mult_int) {
        mybuf = yasm_xmalloc(bc->len*bc->mult_int);
        destbuf = mybuf;
    } else
        destbuf = buf;
    bufstart = destbuf;

    *bufsize = bc->len*bc->mult_int;

    if (!bc->callback)
        yasm_internal_error(N_("got empty bytecode in bc_tobytes"));
    else for (i=0; i<bc->mult_int; i++) {
        origbuf = destbuf;
        error = bc->callback->tobytes(bc, &destbuf, bufstart, d, output_value,
                                      output_reloc);

        if (!error && ((unsigned long)(destbuf - origbuf) != bc->len))
            yasm_internal_error(
                N_("written length does not match optimized length"));
    }

    return mybuf;
}

int
yasm_bc_get_multiple(yasm_bytecode *bc, long *multiple, int calc_bc_dist)
{
    /*@dependent@*/ /*@null@*/ const yasm_intnum *num;

    *multiple = 1;
    if (bc->multiple) {
        num = yasm_expr_get_intnum(&bc->multiple, calc_bc_dist);
        if (!num) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("could not determine multiple"));
            return 1;
        }
        if (yasm_intnum_sign(num) < 0) {
            yasm_error_set(YASM_ERROR_VALUE, N_("multiple is negative"));
            return 1;
        }
        *multiple = yasm_intnum_get_int(num);
    }
    return 0;
}

const yasm_expr *
yasm_bc_get_multiple_expr(const yasm_bytecode *bc)
{
    return bc->multiple;
}

yasm_insn *
yasm_bc_get_insn(yasm_bytecode *bc)
{
    if (bc->callback->special != YASM_BC_SPECIAL_INSN)
        return NULL;
    return (yasm_insn *)bc->contents;
}
