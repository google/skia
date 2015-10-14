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

#include "bytecode.h"


typedef struct bytecode_reserve {
    /*@only@*/ /*@null@*/ yasm_expr *numitems; /* number of items to reserve */
    unsigned int itemsize;          /* size of each item (in bytes) */
} bytecode_reserve;

static void bc_reserve_destroy(void *contents);
static void bc_reserve_print(const void *contents, FILE *f, int indent_level);
static void bc_reserve_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc);
static int bc_reserve_elem_size(yasm_bytecode *bc);
static int bc_reserve_calc_len(yasm_bytecode *bc,
                               yasm_bc_add_span_func add_span,
                               void *add_span_data);
static int bc_reserve_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                              unsigned char *bufstart, void *d,
                              yasm_output_value_func output_value,
                              /*@null@*/ yasm_output_reloc_func output_reloc);

static const yasm_bytecode_callback bc_reserve_callback = {
    bc_reserve_destroy,
    bc_reserve_print,
    bc_reserve_finalize,
    bc_reserve_elem_size,
    bc_reserve_calc_len,
    yasm_bc_expand_common,
    bc_reserve_tobytes,
    YASM_BC_SPECIAL_RESERVE
};


static void
bc_reserve_destroy(void *contents)
{
    bytecode_reserve *reserve = (bytecode_reserve *)contents;
    yasm_expr_destroy(reserve->numitems);
    yasm_xfree(contents);
}

static void
bc_reserve_print(const void *contents, FILE *f, int indent_level)
{
    const bytecode_reserve *reserve = (const bytecode_reserve *)contents;
    fprintf(f, "%*s_Reserve_\n", indent_level, "");
    fprintf(f, "%*sNum Items=", indent_level, "");
    yasm_expr_print(reserve->numitems, f);
    fprintf(f, "\n%*sItem Size=%u\n", indent_level, "", reserve->itemsize);
}

static void
bc_reserve_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    bytecode_reserve *reserve = (bytecode_reserve *)bc->contents;
    /* multiply reserve expression into multiple */
    if (!bc->multiple)
        bc->multiple = reserve->numitems;
    else
        bc->multiple = yasm_expr_create_tree(bc->multiple, YASM_EXPR_MUL,
                                             reserve->numitems, bc->line);
    reserve->numitems = NULL;
}

static int
bc_reserve_elem_size(yasm_bytecode *bc)
{
    bytecode_reserve *reserve = (bytecode_reserve *)bc->contents;
    return reserve->itemsize;
}

static int
bc_reserve_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                    void *add_span_data)
{
    bytecode_reserve *reserve = (bytecode_reserve *)bc->contents;
    bc->len += reserve->itemsize;
    return 0;
}

static int
bc_reserve_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                   unsigned char *bufstart, void *d,
                   yasm_output_value_func output_value,
                   /*@unused@*/ yasm_output_reloc_func output_reloc)
{
    yasm_internal_error(N_("bc_reserve_tobytes called"));
    /*@notreached@*/
    return 1;
}

yasm_bytecode *
yasm_bc_create_reserve(yasm_expr *numitems, unsigned int itemsize,
                       unsigned long line)
{
    bytecode_reserve *reserve = yasm_xmalloc(sizeof(bytecode_reserve));

    /*@-mustfree@*/
    reserve->numitems = numitems;
    /*@=mustfree@*/
    reserve->itemsize = itemsize;

    return yasm_bc_create_common(&bc_reserve_callback, reserve, line);
}

const yasm_expr *
yasm_bc_reserve_numitems(yasm_bytecode *bc, unsigned int *itemsize)
{
    bytecode_reserve *reserve;

    if (bc->callback != &bc_reserve_callback)
        return NULL;

    reserve = (bytecode_reserve *)bc->contents;
    *itemsize = reserve->itemsize;
    return reserve->numitems;
}
