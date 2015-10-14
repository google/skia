/*
 * Data (and LEB128) bytecode
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
#include "arch.h"


struct yasm_dataval {
    /*@reldef@*/ STAILQ_ENTRY(yasm_dataval) link;

    enum { DV_EMPTY, DV_VALUE, DV_RAW, DV_ULEB128, DV_SLEB128, DV_RESERVE }
        type;

    union {
        yasm_value val;
        struct {
            /*@only@*/ unsigned char *contents;
            unsigned long len;
        } raw;
    } data;

    /* number of times data is repeated, NULL=1. */
    /*@only@*/ /*@null@*/ yasm_expr *multiple;
};

typedef struct bytecode_data {
    /* converted data (linked list) */
    yasm_datavalhead datahead;

    int item_size;
} bytecode_data;

static void bc_data_destroy(void *contents);
static void bc_data_print(const void *contents, FILE *f, int indent_level);
static void bc_data_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc);
static int bc_data_item_size(yasm_bytecode *bc);
static int bc_data_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                            void *add_span_data);
static int bc_data_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                           unsigned char *bufstart, void *d,
                           yasm_output_value_func output_value,
                           /*@null@*/ yasm_output_reloc_func output_reloc);

static const yasm_bytecode_callback bc_data_callback = {
    bc_data_destroy,
    bc_data_print,
    bc_data_finalize,
    bc_data_item_size,
    bc_data_calc_len,
    yasm_bc_expand_common,
    bc_data_tobytes,
    0
};


static void
bc_data_destroy(void *contents)
{
    bytecode_data *bc_data = (bytecode_data *)contents;
    yasm_dvs_delete(&bc_data->datahead);
    yasm_xfree(contents);
}

static void
bc_data_print(const void *contents, FILE *f, int indent_level)
{
    const bytecode_data *bc_data = (const bytecode_data *)contents;
    fprintf(f, "%*s_Data_\n", indent_level, "");
    fprintf(f, "%*sElements:\n", indent_level+1, "");
    yasm_dvs_print(&bc_data->datahead, f, indent_level+2);
}

static void
bc_data_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    bytecode_data *bc_data = (bytecode_data *)bc->contents;
    yasm_dataval *dv;
    yasm_intnum *intn;

    /* Convert values from simple expr to value. */
    STAILQ_FOREACH(dv, &bc_data->datahead, link) {
        switch (dv->type) {
            case DV_VALUE:
                if (yasm_value_finalize(&dv->data.val, prev_bc)) {
                    yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                                   N_("data expression too complex"));
                    return;
                }
                break;
            case DV_ULEB128:
            case DV_SLEB128:
                intn = yasm_expr_get_intnum(&dv->data.val.abs, 0);
                if (!intn) {
                    yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                                   N_("LEB128 requires constant values"));
                    return;
                }
                /* Warn for negative values in unsigned environment.
                 * This could be an error instead: the likelihood this is
                 * desired is very low!
                 */
                if (yasm_intnum_sign(intn) == -1 && dv->type == DV_ULEB128)
                    yasm_warn_set(YASM_WARN_GENERAL,
                                  N_("negative value in unsigned LEB128"));
                break;
            default:
                break;
        }
        if (dv->multiple) {
            yasm_value val;
            if (yasm_value_finalize_expr(&val, dv->multiple, prev_bc, 0))
                yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                               N_("multiple expression too complex"));
            else if (val.rel)
                yasm_error_set(YASM_ERROR_NOT_ABSOLUTE,
                               N_("multiple expression not absolute"));
            dv->multiple = val.abs;
        }
    }
}

static int
bc_data_item_size(yasm_bytecode *bc)
{
    bytecode_data *bc_data = (bytecode_data *)bc->contents;
    return bc_data->item_size;
}

static int
bc_data_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                 void *add_span_data)
{
    bytecode_data *bc_data = (bytecode_data *)bc->contents;
    yasm_dataval *dv;
    yasm_intnum *intn;
    unsigned long len = 0;
    unsigned long multiple;

    /* Count up element sizes, rounding up string length. */
    STAILQ_FOREACH(dv, &bc_data->datahead, link) {
        switch (dv->type) {
            case DV_EMPTY:
                len = 0;
                break;
            case DV_VALUE:
                len = dv->data.val.size/8;
                break;
            case DV_RAW:
                len = dv->data.raw.len;
                break;
            case DV_ULEB128:
            case DV_SLEB128:
                intn = yasm_expr_get_intnum(&dv->data.val.abs, 0);
                if (!intn)
                    yasm_internal_error(N_("non-constant in data_tobytes"));
                len = yasm_intnum_size_leb128(intn, dv->type == DV_SLEB128);
                break;
            case DV_RESERVE:
                len = dv->data.val.size/8;
                break;
        }

        if (!yasm_dv_get_multiple(dv, &multiple))
            len *= multiple;

        bc->len += len;
    }

    return 0;
}

static int
bc_data_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                unsigned char *bufstart, void *d,
                yasm_output_value_func output_value,
                /*@unused@*/ yasm_output_reloc_func output_reloc)
{
    bytecode_data *bc_data = (bytecode_data *)bc->contents;
    yasm_dataval *dv;
    yasm_intnum *intn;
    unsigned int val_len;
    unsigned long multiple, i;

    STAILQ_FOREACH(dv, &bc_data->datahead, link) {
        if (yasm_dv_get_multiple(dv, &multiple) || multiple == 0)
            continue;
        switch (dv->type) {
            case DV_EMPTY:
                break;
            case DV_VALUE:
                val_len = dv->data.val.size/8;
                for (i=0; i<multiple; i++) {
                    if (output_value(&dv->data.val, *bufp, val_len,
                                     (unsigned long)(*bufp-bufstart), bc, 1,
                                     d))
                        return 1;
                    *bufp += val_len;
                }
                break;
            case DV_RAW:
                for (i=0; i<multiple; i++) {
                    memcpy(*bufp, dv->data.raw.contents, dv->data.raw.len);
                    *bufp += dv->data.raw.len;
                }
                break;
            case DV_ULEB128:
            case DV_SLEB128:
                intn = yasm_expr_get_intnum(&dv->data.val.abs, 234);
                if (!intn)
                    yasm_internal_error(N_("non-constant in data_tobytes"));
                for (i=0; i<multiple; i++) {
                    *bufp +=
                        yasm_intnum_get_leb128(intn, *bufp,
                                dv->type == DV_SLEB128);
                }
            case DV_RESERVE:
                val_len = dv->data.val.size/8;
                for (i=0; i<multiple; i++) {
                    memset(*bufp, 0, val_len);
                    *bufp += val_len;
                }
                break;
        }
    }

    return 0;
}

yasm_bytecode *
yasm_bc_create_data(yasm_datavalhead *datahead, unsigned int size,
                    int append_zero, yasm_arch *arch, unsigned long line)
{
    bytecode_data *data = yasm_xmalloc(sizeof(bytecode_data));
    yasm_bytecode *bc = yasm_bc_create_common(&bc_data_callback, data, line);
    yasm_dataval *dv, *dv2, *dvo;
    yasm_intnum *intn;
    unsigned long len = 0, rlen, i;


    yasm_dvs_initialize(&data->datahead);
    data->item_size = size;

    /* Prescan input data for length, etc.  Careful: this needs to be
     * precisely paired with the second loop.
     */
    STAILQ_FOREACH(dv, datahead, link) {
        if (dv->multiple && dv->type != DV_EMPTY && len > 0) {
            /* Flush previous data */
            dvo = yasm_dv_create_raw(yasm_xmalloc(len), len);
            STAILQ_INSERT_TAIL(&data->datahead, dvo, link);
            len = 0;
        }
        switch (dv->type) {
            case DV_EMPTY:
                break;
            case DV_VALUE:
            case DV_ULEB128:
            case DV_SLEB128:
                intn = yasm_expr_get_intnum(&dv->data.val.abs, 0);
                if (intn && dv->type == DV_VALUE && (arch || size == 1))
                    len += size;
                else if (intn && dv->type == DV_ULEB128)
                    len += yasm_intnum_size_leb128(intn, 0);
                else if (intn && dv->type == DV_SLEB128)
                    len += yasm_intnum_size_leb128(intn, 1);
                else {
                    if (len > 0) {
                        /* Create bytecode for all previous len */
                        dvo = yasm_dv_create_raw(yasm_xmalloc(len), len);
                        STAILQ_INSERT_TAIL(&data->datahead, dvo, link);
                        len = 0;
                    }

                    /* Create bytecode for this value */
                    dvo = yasm_xmalloc(sizeof(yasm_dataval));
                    STAILQ_INSERT_TAIL(&data->datahead, dvo, link);
                    dvo->multiple = dv->multiple;
                }
                break;
            case DV_RAW:
                rlen = dv->data.raw.len;
                /* find count, rounding up to nearest multiple of size */
                rlen = (rlen + size - 1) / size;
                len += rlen*size;
                break;
            case DV_RESERVE:
                len += size;
                break;
        }

        if (dv->multiple && dv->type != DV_EMPTY && len > 0) {
            /* Flush this data */
            dvo = yasm_dv_create_raw(yasm_xmalloc(len), len);
            STAILQ_INSERT_TAIL(&data->datahead, dvo, link);
            dvo->multiple = dv->multiple;
            len = 0;
        }

        if (append_zero)
            len++;
    }

    /* Create final dataval for any trailing length */
    if (len > 0) {
        dvo = yasm_dv_create_raw(yasm_xmalloc(len), len);
        STAILQ_INSERT_TAIL(&data->datahead, dvo, link);
    }

    /* Second iteration: copy data and delete input datavals. */
    dv = STAILQ_FIRST(datahead);
    dvo = STAILQ_FIRST(&data->datahead);
    len = 0;
    while (dv && dvo) {
        if (dv->multiple && dv->type != DV_EMPTY && len > 0) {
            dvo = STAILQ_NEXT(dvo, link);
            len = 0;
        }
        switch (dv->type) {
            case DV_EMPTY:
                break;
            case DV_VALUE:
            case DV_ULEB128:
            case DV_SLEB128:
                intn = yasm_expr_get_intnum(&dv->data.val.abs, 0);
                if (intn && dv->type == DV_VALUE && (arch || size == 1)) {
                    if (size == 1)
                        yasm_intnum_get_sized(intn,
                                              &dvo->data.raw.contents[len],
                                              1, 8, 0, 0, 1);
                    else
                        yasm_arch_intnum_tobytes(arch, intn,
                                                 &dvo->data.raw.contents[len],
                                                 size, size*8, 0, bc, 1);
                    yasm_value_delete(&dv->data.val);
                    len += size;
                } else if (intn && dv->type == DV_ULEB128) {
                    len += yasm_intnum_get_leb128(intn,
                                                  &dvo->data.raw.contents[len],
                                                  0);
                    yasm_value_delete(&dv->data.val);
                } else if (intn && dv->type == DV_SLEB128) {
                    len += yasm_intnum_get_leb128(intn,
                                                  &dvo->data.raw.contents[len],
                                                  1);
                    yasm_value_delete(&dv->data.val);
                } else {
                    if (len > 0)
                        dvo = STAILQ_NEXT(dvo, link);
                    dvo->type = dv->type;
                    dvo->data.val = dv->data.val;   /* structure copy */
                    dvo->data.val.size = size*8;    /* remember size */
                    dvo = STAILQ_NEXT(dvo, link);
                    len = 0;
                }
                break;
            case DV_RAW:
                rlen = dv->data.raw.len;
                memcpy(&dvo->data.raw.contents[len], dv->data.raw.contents,
                       rlen);
                yasm_xfree(dv->data.raw.contents);
                len += rlen;
                /* pad with 0's to nearest multiple of size */
                rlen %= size;
                if (rlen > 0) {
                    rlen = size-rlen;
                    for (i=0; i<rlen; i++)
                        dvo->data.raw.contents[len++] = 0;
                }
                break;
            case DV_RESERVE:
                memset(&dvo->data.raw.contents[len], 0, size);
                len += size;
                break;
        }

        if (dv->multiple && dv->type != DV_EMPTY && len > 0) {
            dvo = STAILQ_NEXT(dvo, link);
            len = 0;
        }

        if (append_zero)
            dvo->data.raw.contents[len++] = 0;
        dv2 = STAILQ_NEXT(dv, link);
        yasm_xfree(dv);
        dv = dv2;
    }

    return bc;
}

yasm_bytecode *
yasm_bc_create_leb128(yasm_datavalhead *datahead, int sign, unsigned long line)
{
    yasm_dataval *dv;

    /* Convert all values into LEB type, error on strings/raws */
    STAILQ_FOREACH(dv, datahead, link) {
        switch (dv->type) {
            case DV_VALUE:
                dv->type = sign ? DV_SLEB128 : DV_ULEB128;
                break;
            case DV_RAW:
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("LEB128 does not allow string constants"));
                break;
            default:
                break;
        }
    }

    return yasm_bc_create_data(datahead, 0, 0, 0, line);
}

yasm_dataval *
yasm_dv_create_expr(yasm_expr *e)
{
    yasm_dataval *retval = yasm_xmalloc(sizeof(yasm_dataval));

    retval->type = DV_VALUE;
    yasm_value_initialize(&retval->data.val, e, 0);
    retval->multiple = NULL;

    return retval;
}

yasm_dataval *
yasm_dv_create_raw(unsigned char *contents, unsigned long len)
{
    yasm_dataval *retval = yasm_xmalloc(sizeof(yasm_dataval));

    retval->type = DV_RAW;
    retval->data.raw.contents = contents;
    retval->data.raw.len = len;
    retval->multiple = NULL;

    return retval;
}

yasm_dataval *
yasm_dv_create_reserve(void)
{
    yasm_dataval *retval = yasm_xmalloc(sizeof(yasm_dataval));

    retval->type = DV_RESERVE;
    retval->multiple = NULL;

    return retval;
}

yasm_value *
yasm_dv_get_value(yasm_dataval *dv)
{
    if (dv->type != DV_VALUE)
        return NULL;
    return &dv->data.val;
}

void
yasm_dv_set_multiple(yasm_dataval *dv, yasm_expr *e)
{
    if (dv->multiple)
        dv->multiple = yasm_expr_create_tree( dv->multiple, YASM_EXPR_MUL, e,
                                             e->line);
    else
        dv->multiple = e;
}

int
yasm_dv_get_multiple(yasm_dataval *dv, unsigned long *multiple)
{
    /*@dependent@*/ /*@null@*/ const yasm_intnum *num;

    *multiple = 1;
    if (dv->multiple) {
        num = yasm_expr_get_intnum(&dv->multiple, 0);
        if (!num) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("could not determine multiple"));
            return 1;
        }
        if (yasm_intnum_sign(num) < 0) {
            yasm_error_set(YASM_ERROR_VALUE, N_("multiple is negative"));
            return 1;
        }
        *multiple = yasm_intnum_get_uint(num);
    }
    return 0;
}

void
yasm_dvs_delete(yasm_datavalhead *headp)
{
    yasm_dataval *cur, *next;

    cur = STAILQ_FIRST(headp);
    while (cur) {
        next = STAILQ_NEXT(cur, link);
        switch (cur->type) {
            case DV_VALUE:
                yasm_value_delete(&cur->data.val);
                break;
            case DV_RAW:
                yasm_xfree(cur->data.raw.contents);
                break;
            default:
                break;
        }
        if (cur->multiple)
            yasm_expr_destroy(cur->multiple);
        yasm_xfree(cur);
        cur = next;
    }
    STAILQ_INIT(headp);
}

yasm_dataval *
yasm_dvs_append(yasm_datavalhead *headp, yasm_dataval *dv)
{
    if (dv) {
        STAILQ_INSERT_TAIL(headp, dv, link);
        return dv;
    }
    return (yasm_dataval *)NULL;
}

void
yasm_dvs_print(const yasm_datavalhead *head, FILE *f, int indent_level)
{
    yasm_dataval *cur;
    unsigned long i;

    STAILQ_FOREACH(cur, head, link) {
        fprintf(f, "%*sMultiple=", indent_level, "");
        if (!cur->multiple)
            fprintf(f, "nil (1)");
        else
            yasm_expr_print(cur->multiple, f);
        switch (cur->type) {
            case DV_EMPTY:
                fprintf(f, "%*sEmpty\n", indent_level, "");
                break;
            case DV_VALUE:
                fprintf(f, "%*sValue:\n", indent_level, "");
                yasm_value_print(&cur->data.val, f, indent_level+1);
                break;
            case DV_RAW:
                fprintf(f, "%*sLength=%lu\n", indent_level, "",
                        cur->data.raw.len);
                fprintf(f, "%*sBytes=[", indent_level, "");
                for (i=0; i<cur->data.raw.len; i++)
                    fprintf(f, "0x%02x, ", cur->data.raw.contents[i]);
                fprintf(f, "]\n");
                break;
            case DV_ULEB128:
                fprintf(f, "%*sULEB128 value:\n", indent_level, "");
                yasm_value_print(&cur->data.val, f, indent_level+1);
                break;
            case DV_SLEB128:
                fprintf(f, "%*sSLEB128 value:\n", indent_level, "");
                yasm_value_print(&cur->data.val, f, indent_level+1);
                break;
            case DV_RESERVE:
                fprintf(f, "%*sReserved\n", indent_level, "");
                break;
        }
    }
}
