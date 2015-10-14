/*
 * Win64 structured exception handling support
 *
 *  Copyright (C) 2007  Peter Johnson
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

#include "coff-objfmt.h"


#define UNW_FLAG_EHANDLER   0x01
#define UNW_FLAG_UHANDLER   0x02
#define UNW_FLAG_CHAININFO  0x04

/* Bytecode callback function prototypes */
static void win64_uwinfo_bc_destroy(void *contents);
static void win64_uwinfo_bc_print(const void *contents, FILE *f,
                                  int indent_level);
static void win64_uwinfo_bc_finalize(yasm_bytecode *bc,
                                     yasm_bytecode *prev_bc);
static int win64_uwinfo_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int win64_uwinfo_bc_expand(yasm_bytecode *bc, int span, long old_val,
                                  long new_val, /*@out@*/ long *neg_thres,
                                  /*@out@*/ long *pos_thres);
static int win64_uwinfo_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

static void win64_uwcode_bc_destroy(void *contents);
static void win64_uwcode_bc_print(const void *contents, FILE *f,
                                  int indent_level);
static void win64_uwcode_bc_finalize(yasm_bytecode *bc,
                                     yasm_bytecode *prev_bc);
static int win64_uwcode_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int win64_uwcode_bc_expand(yasm_bytecode *bc, int span, long old_val,
                                  long new_val, /*@out@*/ long *neg_thres,
                                  /*@out@*/ long *pos_thres);
static int win64_uwcode_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

/* Bytecode callback structures */
static const yasm_bytecode_callback win64_uwinfo_bc_callback = {
    win64_uwinfo_bc_destroy,
    win64_uwinfo_bc_print,
    win64_uwinfo_bc_finalize,
    NULL,
    win64_uwinfo_bc_calc_len,
    win64_uwinfo_bc_expand,
    win64_uwinfo_bc_tobytes,
    0
};

static const yasm_bytecode_callback win64_uwcode_bc_callback = {
    win64_uwcode_bc_destroy,
    win64_uwcode_bc_print,
    win64_uwcode_bc_finalize,
    NULL,
    win64_uwcode_bc_calc_len,
    win64_uwcode_bc_expand,
    win64_uwcode_bc_tobytes,
    0
};


coff_unwind_info *
yasm_win64__uwinfo_create(void)
{
    coff_unwind_info *info = yasm_xmalloc(sizeof(coff_unwind_info));
    info->proc = NULL;
    info->prolog = NULL;
    info->ehandler = NULL;
    info->framereg = 0;
    /* Frameoff is really a 4-bit value, scaled by 16 */
    yasm_value_initialize(&info->frameoff, NULL, 8);
    SLIST_INIT(&info->codes);
    yasm_value_initialize(&info->prolog_size, NULL, 8);
    yasm_value_initialize(&info->codes_count, NULL, 8);
    return info;
}

void
yasm_win64__uwinfo_destroy(coff_unwind_info *info)
{
    coff_unwind_code *code;

    yasm_value_delete(&info->frameoff);
    yasm_value_delete(&info->prolog_size);
    yasm_value_delete(&info->codes_count);

    while (!SLIST_EMPTY(&info->codes)) {
        code = SLIST_FIRST(&info->codes);
        SLIST_REMOVE_HEAD(&info->codes, link);
        yasm_value_delete(&code->off);
        yasm_xfree(code);
    }
    yasm_xfree(info);
}

void
yasm_win64__unwind_generate(yasm_section *xdata, coff_unwind_info *info,
                            unsigned long line)
{
    yasm_bytecode *infobc, *codebc = NULL;
    coff_unwind_code *code;

    /* 4-byte align the start of unwind info */
    yasm_section_bcs_append(xdata, yasm_bc_create_align(
        yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_uint(4)),
                               line),
        NULL, NULL, NULL, line));

    /* Prolog size = end of prolog - start of procedure */
    yasm_value_initialize(&info->prolog_size,
        yasm_expr_create(YASM_EXPR_SUB, yasm_expr_sym(info->prolog),
                         yasm_expr_sym(info->proc), line),
        8);

    /* Unwind info */
    infobc = yasm_bc_create_common(&win64_uwinfo_bc_callback, info, line);
    yasm_section_bcs_append(xdata, infobc);

    /* Code array */
    SLIST_FOREACH(code, &info->codes, link) {
        codebc = yasm_bc_create_common(&win64_uwcode_bc_callback, code,
                                       yasm_symrec_get_def_line(code->loc));
        yasm_section_bcs_append(xdata, codebc);
    }

    /* Avoid double-free (by code destroy and uwinfo destroy). */
    SLIST_INIT(&info->codes);

    /* Number of codes = (Last code - end of info) >> 1 */
    if (!codebc) {
        yasm_value_initialize(&info->codes_count,
            yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_uint(0)),
                                   line),
            8);
    } else {
        yasm_value_initialize(&info->codes_count,
            yasm_expr_create(YASM_EXPR_SHR, yasm_expr_expr(
                yasm_expr_create(YASM_EXPR_SUB, yasm_expr_precbc(codebc),
                                 yasm_expr_precbc(infobc), line)),
                yasm_expr_int(yasm_intnum_create_uint(1)), line),
            8);
    }

    /* 4-byte align */
    yasm_section_bcs_append(xdata, yasm_bc_create_align(
        yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_uint(4)),
                               line),
        NULL, NULL, NULL, line));

    /* Exception handler, if present.  Use data bytecode. */
    if (info->ehandler) {
        yasm_datavalhead dvs;

        yasm_dvs_initialize(&dvs);
        yasm_dvs_append(&dvs, yasm_dv_create_expr(
            yasm_expr_create_ident(yasm_expr_sym(info->ehandler), line)));
        yasm_section_bcs_append(xdata,
                                yasm_bc_create_data(&dvs, 4, 0, NULL, line));
    }
}

static void
win64_uwinfo_bc_destroy(void *contents)
{
    yasm_win64__uwinfo_destroy((coff_unwind_info *)contents);
}

static void
win64_uwinfo_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static void
win64_uwinfo_bc_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    coff_unwind_info *info = (coff_unwind_info *)bc->contents;

    if (yasm_value_finalize(&info->prolog_size, prev_bc))
        yasm_internal_error(N_("prolog size expression too complex"));

    if (yasm_value_finalize(&info->codes_count, prev_bc))
        yasm_internal_error(N_("codes count expression too complex"));

    if (yasm_value_finalize(&info->frameoff, prev_bc))
        yasm_error_set(YASM_ERROR_VALUE,
                       N_("frame offset expression too complex"));
}

static int
win64_uwinfo_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                         void *add_span_data)
{
    coff_unwind_info *info = (coff_unwind_info *)bc->contents;
    /*@only@*/ /*@null@*/ yasm_intnum *intn;
    long intv;

    /* Want to make sure prolog size and codes count doesn't exceed
     * byte-size, and scaled frame offset doesn't exceed 4 bits.
     */
    add_span(add_span_data, bc, 1, &info->prolog_size, 0, 255);
    add_span(add_span_data, bc, 2, &info->codes_count, 0, 255);

    intn = yasm_value_get_intnum(&info->frameoff, bc, 0);
    if (intn) {
        intv = yasm_intnum_get_int(intn);
        if (intv < 0 || intv > 240)
            yasm_error_set(YASM_ERROR_VALUE,
                N_("frame offset of %ld bytes, must be between 0 and 240"),
                intv);
        else if ((intv & 0xF) != 0)
            yasm_error_set(YASM_ERROR_VALUE,
                N_("frame offset of %ld is not a multiple of 16"), intv);
        yasm_intnum_destroy(intn);
    } else
        add_span(add_span_data, bc, 3, &info->frameoff, 0, 240);

    bc->len += 4;
    return 0;
}

static int
win64_uwinfo_bc_expand(yasm_bytecode *bc, int span, long old_val, long new_val,
                       /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres)
{
    coff_unwind_info *info = (coff_unwind_info *)bc->contents;
    switch (span) {
        case 1:
            yasm_error_set_xref(yasm_symrec_get_def_line(info->prolog),
                                N_("prologue ended here"));
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("prologue %ld bytes, must be <256"), new_val);
            return -1;
        case 2:
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("%ld unwind codes, maximum of 255"), new_val);
            return -1;
        case 3:
            yasm_error_set(YASM_ERROR_VALUE,
                N_("frame offset of %ld bytes, must be between 0 and 240"),
                new_val);
            return -1;
        default:
            yasm_internal_error(N_("unrecognized span id"));
    }
    return 0;
}

static int
win64_uwinfo_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                        unsigned char *bufstart, void *d,
                        yasm_output_value_func output_value,
                        yasm_output_reloc_func output_reloc)
{
    coff_unwind_info *info = (coff_unwind_info *)bc->contents;
    unsigned char *buf = *bufp;
    /*@only@*/ /*@null@*/ yasm_intnum *frameoff;
    long intv;

    /* Version and flags */
    if (info->ehandler)
        YASM_WRITE_8(buf, 1 | (UNW_FLAG_EHANDLER << 3));
    else
        YASM_WRITE_8(buf, 1);

    /* Size of prolog */
    output_value(&info->prolog_size, buf, 1, (unsigned long)(buf-bufstart),
                 bc, 1, d);
    buf += 1;

    /* Count of codes */
    output_value(&info->codes_count, buf, 1, (unsigned long)(buf-bufstart),
                 bc, 1, d);
    buf += 1;

    /* Frame register and offset */
    frameoff = yasm_value_get_intnum(&info->frameoff, bc, 1);
    if (!frameoff) {
        yasm_error_set(YASM_ERROR_VALUE,
                       N_("frame offset expression too complex"));
        return 1;
    }
    intv = yasm_intnum_get_int(frameoff);
    if (intv < 0 || intv > 240)
        yasm_error_set(YASM_ERROR_VALUE,
            N_("frame offset of %ld bytes, must be between 0 and 240"), intv);
    else if ((intv & 0xF) != 0)
        yasm_error_set(YASM_ERROR_VALUE,
            N_("frame offset of %ld is not a multiple of 16"), intv);

    YASM_WRITE_8(buf, ((unsigned long)intv & 0xF0) | (info->framereg & 0x0F));
    yasm_intnum_destroy(frameoff);

    *bufp = buf;
    return 0;
}

static void
win64_uwcode_bc_destroy(void *contents)
{
    coff_unwind_code *code = (coff_unwind_code *)contents;
    yasm_value_delete(&code->off);
    yasm_xfree(contents);
}

static void
win64_uwcode_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static void
win64_uwcode_bc_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    coff_unwind_code *code = (coff_unwind_code *)bc->contents;
    if (yasm_value_finalize(&code->off, prev_bc))
        yasm_error_set(YASM_ERROR_VALUE, N_("offset expression too complex"));
}

static int
win64_uwcode_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                         void *add_span_data)
{
    coff_unwind_code *code = (coff_unwind_code *)bc->contents;
    int span = 0;
    /*@only@*/ /*@null@*/ yasm_intnum *intn;
    long intv;
    long low, high, mask;

    bc->len += 2;   /* Prolog offset, code, and info */

    switch (code->opcode) {
        case UWOP_PUSH_NONVOL:
        case UWOP_SET_FPREG:
        case UWOP_PUSH_MACHFRAME:
            /* always 1 node */
            return 0;
        case UWOP_ALLOC_SMALL:
        case UWOP_ALLOC_LARGE:
            /* Start with smallest, then work our way up as necessary */
            code->opcode = UWOP_ALLOC_SMALL;
            code->info = 0;
            span = 1; low = 8; high = 128; mask = 0x7;
            break;
        case UWOP_SAVE_NONVOL:
        case UWOP_SAVE_NONVOL_FAR:
            /* Start with smallest, then work our way up as necessary */
            code->opcode = UWOP_SAVE_NONVOL;
            bc->len += 2;   /* Scaled offset */
            span = 2;
            low = 0;
            high = 8*64*1024-8;         /* 16-bit field, *8 scaling */
            mask = 0x7;
            break;
        case UWOP_SAVE_XMM128:
        case UWOP_SAVE_XMM128_FAR:
            /* Start with smallest, then work our way up as necessary */
            code->opcode = UWOP_SAVE_XMM128;
            bc->len += 2;   /* Scaled offset */
            span = 3;
            low = 0;
            high = 16*64*1024-16;       /* 16-bit field, *16 scaling */
            mask = 0xF;
            break;
        default:
            yasm_internal_error(N_("unrecognied unwind opcode"));
            /*@unreached@*/
            return 0;
    }

    intn = yasm_value_get_intnum(&code->off, bc, 0);
    if (intn) {
        intv = yasm_intnum_get_int(intn);
        if (intv > high) {
            /* Expand it ourselves here if we can and we're already larger */
            if (win64_uwcode_bc_expand(bc, span, intv, intv, &low, &high) > 0)
                add_span(add_span_data, bc, span, &code->off, low, high);
        }
        if (intv < low)
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("negative offset not allowed"));
        if ((intv & mask) != 0)
            yasm_error_set(YASM_ERROR_VALUE,
                N_("offset of %ld is not a multiple of %ld"), intv, mask+1);
        yasm_intnum_destroy(intn);
    } else
        add_span(add_span_data, bc, span, &code->off, low, high);
    return 0;
}

static int
win64_uwcode_bc_expand(yasm_bytecode *bc, int span, long old_val, long new_val,
                       /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres)
{
    coff_unwind_code *code = (coff_unwind_code *)bc->contents;

    if (new_val < 0) {
        yasm_error_set(YASM_ERROR_VALUE, N_("negative offset not allowed"));
        return -1;
    }

    if (span == 1) {
        /* 3 stages: SMALL, LARGE and info=0, LARGE and info=1 */
        if (code->opcode == UWOP_ALLOC_LARGE && code->info == 1)
            yasm_internal_error(N_("expansion on already largest alloc"));

        if (code->opcode == UWOP_ALLOC_SMALL && new_val > 128) {
            /* Overflowed small size */
            code->opcode = UWOP_ALLOC_LARGE;
            bc->len += 2;
        }
        if (new_val <= 8*64*1024-8) {
            /* Still can grow one more size */
            *pos_thres = 8*64*1024-8;
            return 1;
        }
        /* We're into the largest size */
        code->info = 1;
        bc->len += 2;
    } else if (code->opcode == UWOP_SAVE_NONVOL && span == 2) {
        code->opcode = UWOP_SAVE_NONVOL_FAR;
        bc->len += 2;
    } else if (code->opcode == UWOP_SAVE_XMM128 && span == 3) {
        code->opcode = UWOP_SAVE_XMM128_FAR;
        bc->len += 2;
    }
    return 0;
}

static int
win64_uwcode_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                        unsigned char *bufstart, void *d,
                        yasm_output_value_func output_value,
                        yasm_output_reloc_func output_reloc)
{
    coff_unwind_code *code = (coff_unwind_code *)bc->contents;
    unsigned char *buf = *bufp;
    yasm_value val;
    unsigned int size;
    int shift;
    long intv, low, high, mask;
    yasm_intnum *intn;

    /* Offset in prolog */
    yasm_value_initialize(&val,
        yasm_expr_create(YASM_EXPR_SUB, yasm_expr_sym(code->loc),
                         yasm_expr_sym(code->proc), bc->line),
        8);
    output_value(&val, buf, 1, (unsigned long)(buf-bufstart), bc, 1, d);
    buf += 1;
    yasm_value_delete(&val);

    /* Offset value */
    switch (code->opcode) {
        case UWOP_PUSH_NONVOL:
        case UWOP_SET_FPREG:
        case UWOP_PUSH_MACHFRAME:
            /* just 1 node, no offset; write opcode and info and we're done */
            YASM_WRITE_8(buf, (code->info << 4) | (code->opcode & 0xF));
            *bufp = buf;
            return 0;
        case UWOP_ALLOC_SMALL:
            /* 1 node, but offset stored in info */
            size = 0; low = 8; high = 128; shift = 3; mask = 0x7;
            break;
        case UWOP_ALLOC_LARGE:
            if (code->info == 0) {
                size = 2; low = 136; high = 8*64*1024-8; shift = 3;
            } else {
                size = 4; low = high = 0; shift = 0;
            }
            mask = 0x7;
            break;
        case UWOP_SAVE_NONVOL:
            size = 2; low = 0; high = 8*64*1024-8; shift = 3; mask = 0x7;
            break;
        case UWOP_SAVE_XMM128:
            size = 2; low = 0; high = 16*64*1024-16; shift = 4; mask = 0xF;
            break;
        case UWOP_SAVE_NONVOL_FAR:
            size = 4; low = high = 0; shift = 0; mask = 0x7;
            break;
        case UWOP_SAVE_XMM128_FAR:
            size = 4; low = high = 0; shift = 0; mask = 0xF;
            break;
        default:
            yasm_internal_error(N_("unrecognied unwind opcode"));
            /*@unreached@*/
            return 1;
    }

    /* Check for overflow */
    intn = yasm_value_get_intnum(&code->off, bc, 1);
    if (!intn) {
        yasm_error_set(YASM_ERROR_VALUE, N_("offset expression too complex"));
        return 1;
    }
    intv = yasm_intnum_get_int(intn);
    if (size != 4 && (intv < low || intv > high)) {
        yasm_error_set(YASM_ERROR_VALUE,
            N_("offset of %ld bytes, must be between %ld and %ld"),
            intv, low, high);
        return 1;
    }
    if ((intv & mask) != 0) {
        yasm_error_set(YASM_ERROR_VALUE,
                       N_("offset of %ld is not a multiple of %ld"),
                       intv, mask+1);
        return 1;
    }

    /* Stored value in info instead of extra code space */
    if (size == 0)
        code->info = (yasm_intnum_get_uint(intn) >> shift)-1;

    /* Opcode and info */
    YASM_WRITE_8(buf, (code->info << 4) | (code->opcode & 0xF));

    if (size != 0) {
        yasm_intnum_get_sized(intn, buf, size, size*8, -shift, 0, 1);
        buf += size;
    }

    yasm_intnum_destroy(intn);

    *bufp = buf;
    return 0;
}
