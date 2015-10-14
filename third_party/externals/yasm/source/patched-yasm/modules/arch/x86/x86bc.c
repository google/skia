/*
 * x86 bytecode utility functions
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

#include "x86arch.h"


/* Bytecode callback function prototypes */

static void x86_bc_insn_destroy(void *contents);
static void x86_bc_insn_print(const void *contents, FILE *f,
                              int indent_level);
static int x86_bc_insn_calc_len(yasm_bytecode *bc,
                                yasm_bc_add_span_func add_span,
                                void *add_span_data);
static int x86_bc_insn_expand(yasm_bytecode *bc, int span, long old_val,
                              long new_val, /*@out@*/ long *neg_thres,
                              /*@out@*/ long *pos_thres);
static int x86_bc_insn_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                               unsigned char *bufstart,
                               void *d, yasm_output_value_func output_value,
                               /*@null@*/ yasm_output_reloc_func output_reloc);

static void x86_bc_jmp_destroy(void *contents);
static void x86_bc_jmp_print(const void *contents, FILE *f, int indent_level);
static int x86_bc_jmp_calc_len(yasm_bytecode *bc,
                               yasm_bc_add_span_func add_span,
                               void *add_span_data);
static int x86_bc_jmp_expand(yasm_bytecode *bc, int span, long old_val,
                             long new_val, /*@out@*/ long *neg_thres,
                             /*@out@*/ long *pos_thres);
static int x86_bc_jmp_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                              unsigned char *bufstart,
                              void *d, yasm_output_value_func output_value,
                              /*@null@*/ yasm_output_reloc_func output_reloc);

static void x86_bc_jmpfar_destroy(void *contents);
static void x86_bc_jmpfar_print(const void *contents, FILE *f,
                                int indent_level);
static int x86_bc_jmpfar_calc_len(yasm_bytecode *bc,
                                  yasm_bc_add_span_func add_span,
                                  void *add_span_data);
static int x86_bc_jmpfar_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

/* Bytecode callback structures */

static const yasm_bytecode_callback x86_bc_callback_insn = {
    x86_bc_insn_destroy,
    x86_bc_insn_print,
    yasm_bc_finalize_common,
    NULL,
    x86_bc_insn_calc_len,
    x86_bc_insn_expand,
    x86_bc_insn_tobytes,
    0
};

static const yasm_bytecode_callback x86_bc_callback_jmp = {
    x86_bc_jmp_destroy,
    x86_bc_jmp_print,
    yasm_bc_finalize_common,
    NULL,
    x86_bc_jmp_calc_len,
    x86_bc_jmp_expand,
    x86_bc_jmp_tobytes,
    0
};

static const yasm_bytecode_callback x86_bc_callback_jmpfar = {
    x86_bc_jmpfar_destroy,
    x86_bc_jmpfar_print,
    yasm_bc_finalize_common,
    NULL,
    x86_bc_jmpfar_calc_len,
    yasm_bc_expand_common,
    x86_bc_jmpfar_tobytes,
    0
};

int
yasm_x86__set_rex_from_reg(unsigned char *rex, unsigned char *low3,
                           uintptr_t reg, unsigned int bits,
                           x86_rex_bit_pos rexbit)
{
    *low3 = (unsigned char)(reg&7);

    if (bits == 64) {
        x86_expritem_reg_size size = (x86_expritem_reg_size)(reg & ~0xFUL);

        if (size == X86_REG8X || (reg & 0xF) >= 8) {
            /* Check to make sure we can set it */
            if (*rex == 0xff) {
                yasm_error_set(YASM_ERROR_TYPE,
                    N_("cannot use A/B/C/DH with instruction needing REX"));
                return 1;
            }
            *rex |= 0x40 | (((reg & 8) >> 3) << rexbit);
        } else if (size == X86_REG8 && (reg & 7) >= 4) {
            /* AH/BH/CH/DH, so no REX allowed */
            if (*rex != 0 && *rex != 0xff) {
                yasm_error_set(YASM_ERROR_TYPE,
                    N_("cannot use A/B/C/DH with instruction needing REX"));
                return 1;
            }
            *rex = 0xff;    /* Flag so we can NEVER set it (see above) */
        }
    }

    return 0;
}

void
yasm_x86__bc_transform_insn(yasm_bytecode *bc, x86_insn *insn)
{
    yasm_bc_transform(bc, &x86_bc_callback_insn, insn);
}

void
yasm_x86__bc_transform_jmp(yasm_bytecode *bc, x86_jmp *jmp)
{
    yasm_bc_transform(bc, &x86_bc_callback_jmp, jmp);
}

void
yasm_x86__bc_transform_jmpfar(yasm_bytecode *bc, x86_jmpfar *jmpfar)
{
    yasm_bc_transform(bc, &x86_bc_callback_jmpfar, jmpfar);
}

void
yasm_x86__ea_init(x86_effaddr *x86_ea, unsigned int spare,
                  yasm_bytecode *precbc)
{
    if (yasm_value_finalize(&x86_ea->ea.disp, precbc))
        yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                       N_("effective address too complex"));
    x86_ea->modrm &= 0xC7;                  /* zero spare/reg bits */
    x86_ea->modrm |= (spare << 3) & 0x38;   /* plug in provided bits */
}

void
yasm_x86__ea_set_disponly(x86_effaddr *x86_ea)
{
    x86_ea->valid_modrm = 0;
    x86_ea->need_modrm = 0;
    x86_ea->valid_sib = 0;
    x86_ea->need_sib = 0;
}

static x86_effaddr *
ea_create(void)
{
    x86_effaddr *x86_ea = yasm_xmalloc(sizeof(x86_effaddr));

    yasm_value_initialize(&x86_ea->ea.disp, NULL, 0);
    x86_ea->ea.need_nonzero_len = 0;
    x86_ea->ea.need_disp = 0;
    x86_ea->ea.nosplit = 0;
    x86_ea->ea.strong = 0;
    x86_ea->ea.segreg = 0;
    x86_ea->ea.pc_rel = 0;
    x86_ea->ea.not_pc_rel = 0;
    x86_ea->ea.data_len = 0;
    x86_ea->vsib_mode = 0;
    x86_ea->modrm = 0;
    x86_ea->valid_modrm = 0;
    x86_ea->need_modrm = 0;
    x86_ea->sib = 0;
    x86_ea->valid_sib = 0;
    x86_ea->need_sib = 0;

    return x86_ea;
}

x86_effaddr *
yasm_x86__ea_create_reg(x86_effaddr *x86_ea, unsigned long reg,
                        unsigned char *rex, unsigned int bits)
{
    unsigned char rm;

    if (yasm_x86__set_rex_from_reg(rex, &rm, reg, bits, X86_REX_B))
        return NULL;

    if (!x86_ea)
        x86_ea = ea_create();
    x86_ea->modrm = 0xC0 | rm;  /* Mod=11, R/M=Reg, Reg=0 */
    x86_ea->valid_modrm = 1;
    x86_ea->need_modrm = 1;

    return x86_ea;
}

yasm_effaddr *
yasm_x86__ea_create_expr(yasm_arch *arch, yasm_expr *e)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)arch;
    x86_effaddr *x86_ea;

    x86_ea = ea_create();

    if (arch_x86->parser == X86_PARSER_GAS) {
        /* Need to change foo+rip into foo wrt rip (even in .intel_syntax mode).
         * Note this assumes a particular ordering coming from the parser
         * to work (it's not very smart)!
         */
        if (e->op == YASM_EXPR_ADD && e->terms[0].type == YASM_EXPR_REG
            && e->terms[0].data.reg == X86_RIP) {
            /* replace register with 0 */
            e->terms[0].type = YASM_EXPR_INT;
            e->terms[0].data.intn = yasm_intnum_create_uint(0);
            /* build new wrt expression */
            e = yasm_expr_create(YASM_EXPR_WRT, yasm_expr_expr(e),
                                 yasm_expr_reg(X86_RIP), e->line);
        }
    }
    yasm_value_initialize(&x86_ea->ea.disp, e, 0);
    x86_ea->ea.need_disp = 1;
    x86_ea->need_modrm = 1;
    /* We won't know whether we need an SIB until we know more about expr and
     * the BITS/address override setting.
     */
    x86_ea->need_sib = 0xff;

    x86_ea->ea.data_len = 0;

    return (yasm_effaddr *)x86_ea;
}

/*@-compmempass@*/
x86_effaddr *
yasm_x86__ea_create_imm(x86_effaddr *x86_ea, yasm_expr *imm,
                        unsigned int im_len)
{
    if (!x86_ea)
        x86_ea = ea_create();
    yasm_value_initialize(&x86_ea->ea.disp, imm, im_len);
    x86_ea->ea.need_disp = 1;

    return x86_ea;
}
/*@=compmempass@*/

void
yasm_x86__bc_apply_prefixes(x86_common *common, unsigned char *rex,
                            unsigned int def_opersize_64,
                            unsigned int num_prefixes, uintptr_t *prefixes)
{
    unsigned int i;
    int first = 1;

    for (i=0; i<num_prefixes; i++) {
        switch ((x86_parse_insn_prefix)(prefixes[i] & 0xff00)) {
            case X86_LOCKREP:
                if (common->lockrep_pre != 0)
                    yasm_warn_set(YASM_WARN_GENERAL,
                        N_("multiple LOCK or REP prefixes, using leftmost"));
                common->lockrep_pre = (unsigned char)prefixes[i] & 0xff;
                break;
            case X86_ADDRSIZE:
                common->addrsize = (unsigned char)prefixes[i] & 0xff;
                break;
            case X86_OPERSIZE:
                common->opersize = (unsigned char)prefixes[i] & 0xff;
                if (common->mode_bits == 64 && common->opersize == 64 &&
                    def_opersize_64 != 64) {
                    if (!rex)
                        yasm_warn_set(YASM_WARN_GENERAL,
                                      N_("ignoring REX prefix on jump"));
                    else if (*rex == 0xff)
                        yasm_warn_set(YASM_WARN_GENERAL,
                            N_("REX prefix not allowed on this instruction, ignoring"));
                    else
                        *rex = 0x48;
                }
                break;
            case X86_SEGREG:
                /* This is a hack.. we should really be putting this in the
                 * the effective address!
                 */
                common->lockrep_pre = (unsigned char)prefixes[i] & 0xff;
                break;
            case X86_REX:
                if (!rex)
                    yasm_warn_set(YASM_WARN_GENERAL,
                                  N_("ignoring REX prefix on jump"));
                else if (*rex == 0xff)
                    yasm_warn_set(YASM_WARN_GENERAL,
                        N_("REX prefix not allowed on this instruction, ignoring"));
                else {
                    if (*rex != 0) {
                        if (first)
                            yasm_warn_set(YASM_WARN_GENERAL,
                                N_("overriding generated REX prefix"));
                        else
                            yasm_warn_set(YASM_WARN_GENERAL,
                                N_("multiple REX prefixes, using leftmost"));
                    }
                    /* Here we assume that we can't get this prefix in non
                     * 64 bit mode due to checks in parse_check_prefix().
                     */
                    common->mode_bits = 64;
                    *rex = (unsigned char)prefixes[i] & 0xff;
                }
                first = 0;
                break;
        }
    }
}

static void
x86_bc_insn_destroy(void *contents)
{
    x86_insn *insn = (x86_insn *)contents;
    if (insn->x86_ea)
        yasm_x86__ea_destroy((yasm_effaddr *)insn->x86_ea);
    if (insn->imm) {
        yasm_value_delete(insn->imm);
        yasm_xfree(insn->imm);
    }
    yasm_xfree(contents);
}

static void
x86_bc_jmp_destroy(void *contents)
{
    x86_jmp *jmp = (x86_jmp *)contents;
    yasm_value_delete(&jmp->target);
    yasm_xfree(contents);
}

static void
x86_bc_jmpfar_destroy(void *contents)
{
    x86_jmpfar *jmpfar = (x86_jmpfar *)contents;
    yasm_value_delete(&jmpfar->segment);
    yasm_value_delete(&jmpfar->offset);
    yasm_xfree(contents);
}

void
yasm_x86__ea_destroy(yasm_effaddr *ea)
{
    yasm_value_delete(&ea->disp);
    yasm_xfree(ea);
}

void
yasm_x86__ea_print(const yasm_effaddr *ea, FILE *f, int indent_level)
{
    const x86_effaddr *x86_ea = (const x86_effaddr *)ea;
    fprintf(f, "%*sDisp:\n", indent_level, "");
    yasm_value_print(&ea->disp, f, indent_level+1);
    fprintf(f, "%*sNoSplit=%u\n", indent_level, "", (unsigned int)ea->nosplit);
    fprintf(f, "%*sSegmentOv=%02x\n", indent_level, "",
            (unsigned int)x86_ea->ea.segreg);
    fprintf(f, "%*sVSIBMode=%u\n", indent_level, "",
            (unsigned int)x86_ea->vsib_mode);
    fprintf(f, "%*sModRM=%03o ValidRM=%u NeedRM=%u\n", indent_level, "",
            (unsigned int)x86_ea->modrm, (unsigned int)x86_ea->valid_modrm,
            (unsigned int)x86_ea->need_modrm);
    fprintf(f, "%*sSIB=%03o ValidSIB=%u NeedSIB=%u\n", indent_level, "",
            (unsigned int)x86_ea->sib, (unsigned int)x86_ea->valid_sib,
            (unsigned int)x86_ea->need_sib);
}

static void
x86_common_print(const x86_common *common, FILE *f, int indent_level)
{
    fprintf(f, "%*sAddrSize=%u OperSize=%u LockRepPre=%02x BITS=%u\n",
            indent_level, "",
            (unsigned int)common->addrsize,
            (unsigned int)common->opersize,
            (unsigned int)common->lockrep_pre,
            (unsigned int)common->mode_bits);
}

static void
x86_opcode_print(const x86_opcode *opcode, FILE *f, int indent_level)
{
    fprintf(f, "%*sOpcode: %02x %02x %02x OpLen=%u\n", indent_level, "",
            (unsigned int)opcode->opcode[0],
            (unsigned int)opcode->opcode[1],
            (unsigned int)opcode->opcode[2],
            (unsigned int)opcode->len);
}

static void
x86_bc_insn_print(const void *contents, FILE *f, int indent_level)
{
    const x86_insn *insn = (const x86_insn *)contents;

    fprintf(f, "%*s_Instruction_\n", indent_level, "");
    fprintf(f, "%*sEffective Address:", indent_level, "");
    if (insn->x86_ea) {
        fprintf(f, "\n");
        yasm_x86__ea_print((yasm_effaddr *)insn->x86_ea, f, indent_level+1);
    } else
        fprintf(f, " (nil)\n");
    fprintf(f, "%*sImmediate Value:", indent_level, "");
    if (!insn->imm)
        fprintf(f, " (nil)\n");
    else {
        indent_level++;
        fprintf(f, "\n");
        yasm_value_print(insn->imm, f, indent_level);
        indent_level--;
    }
    x86_opcode_print(&insn->opcode, f, indent_level);
    x86_common_print(&insn->common, f, indent_level);
    fprintf(f, "%*sSpPre=%02x REX=%03o PostOp=%u\n", indent_level, "",
            (unsigned int)insn->special_prefix,
            (unsigned int)insn->rex,
            (unsigned int)insn->postop);
}

static void
x86_bc_jmp_print(const void *contents, FILE *f, int indent_level)
{
    const x86_jmp *jmp = (const x86_jmp *)contents;

    fprintf(f, "%*s_Jump_\n", indent_level, "");
    fprintf(f, "%*sTarget:\n", indent_level, "");
    yasm_value_print(&jmp->target, f, indent_level+1);
    /* FIXME
    fprintf(f, "%*sOrigin=\n", indent_level, "");
    yasm_symrec_print(jmp->origin, f, indent_level+1);
    */
    fprintf(f, "\n%*sShort Form:\n", indent_level, "");
    if (jmp->shortop.len == 0)
        fprintf(f, "%*sNone\n", indent_level+1, "");
    else
        x86_opcode_print(&jmp->shortop, f, indent_level+1);
    fprintf(f, "%*sNear Form:\n", indent_level, "");
    if (jmp->nearop.len == 0)
        fprintf(f, "%*sNone\n", indent_level+1, "");
    else
        x86_opcode_print(&jmp->nearop, f, indent_level+1);
    fprintf(f, "%*sOpSel=", indent_level, "");
    switch (jmp->op_sel) {
        case JMP_NONE:
            fprintf(f, "None");
            break;
        case JMP_SHORT:
            fprintf(f, "Short");
            break;
        case JMP_NEAR:
            fprintf(f, "Near");
            break;
        case JMP_SHORT_FORCED:
            fprintf(f, "Forced Short");
            break;
        case JMP_NEAR_FORCED:
            fprintf(f, "Forced Near");
            break;
        default:
            fprintf(f, "UNKNOWN!!");
            break;
    }
    x86_common_print(&jmp->common, f, indent_level);
}

static void
x86_bc_jmpfar_print(const void *contents, FILE *f, int indent_level)
{
    const x86_jmpfar *jmpfar = (const x86_jmpfar *)contents;

    fprintf(f, "%*s_Far_Jump_\n", indent_level, "");
    fprintf(f, "%*sSegment:\n", indent_level, "");
    yasm_value_print(&jmpfar->segment, f, indent_level+1);
    fprintf(f, "%*sOffset:\n", indent_level, "");
    yasm_value_print(&jmpfar->offset, f, indent_level+1);
    x86_opcode_print(&jmpfar->opcode, f, indent_level);
    x86_common_print(&jmpfar->common, f, indent_level);
}

static unsigned int
x86_common_calc_len(const x86_common *common)
{
    unsigned int len = 0;

    if (common->addrsize != 0 && common->addrsize != common->mode_bits)
        len++;
    if (common->opersize != 0 &&
        ((common->mode_bits != 64 && common->opersize != common->mode_bits) ||
         (common->mode_bits == 64 && common->opersize == 16)))
        len++;
    if (common->lockrep_pre != 0)
        len++;

    return len;
}

static int
x86_bc_insn_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                     void *add_span_data)
{
    x86_insn *insn = (x86_insn *)bc->contents;
    x86_effaddr *x86_ea = insn->x86_ea;
    yasm_value *imm = insn->imm;

    if (x86_ea) {
        /* Check validity of effective address and calc R/M bits of
         * Mod/RM byte and SIB byte.  We won't know the Mod field
         * of the Mod/RM byte until we know more about the
         * displacement.
         */
        if (yasm_x86__expr_checkea(x86_ea, &insn->common.addrsize,
                insn->common.mode_bits, insn->postop == X86_POSTOP_ADDRESS16,
                &insn->rex, bc))
            /* failed, don't bother checking rest of insn */
            return -1;

        if (x86_ea->ea.disp.size == 0 && x86_ea->ea.need_nonzero_len) {
            /* Handle unknown case, default to byte-sized and set as
             * critical expression.
             */
            x86_ea->ea.disp.size = 8;
            add_span(add_span_data, bc, 1, &x86_ea->ea.disp, -128, 127);
        }
        bc->len += x86_ea->ea.disp.size/8;

        /* Handle address16 postop case */
        if (insn->postop == X86_POSTOP_ADDRESS16)
            insn->common.addrsize = 0;

        /* Compute length of ea and add to total */
        bc->len += x86_ea->need_modrm + (x86_ea->need_sib ? 1:0);
        bc->len += (x86_ea->ea.segreg != 0) ? 1 : 0;
    }

    if (imm) {
        unsigned int immlen = imm->size;

        /* TODO: check imm->len vs. sized len from expr? */

        /* Handle signext_imm8 postop special-casing */
        if (insn->postop == X86_POSTOP_SIGNEXT_IMM8) {
            /*@null@*/ /*@only@*/ yasm_intnum *num;
            num = yasm_value_get_intnum(imm, NULL, 0);

            if (!num) {
                /* Unknown; default to byte form and set as critical
                 * expression.
                 */
                immlen = 8;
                add_span(add_span_data, bc, 2, imm, -128, 127);
            } else {
                if (yasm_intnum_in_range(num, -128, 127)) {
                    /* We can use the sign-extended byte form: shorten
                     * the immediate length to 1 and make the byte form
                     * permanent.
                     */
                    imm->size = 8;
                    imm->sign = 1;
                    immlen = 8;
                } else {
                    /* We can't.  Copy over the word-sized opcode. */
                    insn->opcode.opcode[0] =
                        insn->opcode.opcode[insn->opcode.len];
                    insn->opcode.len = 1;
                }
                insn->postop = X86_POSTOP_NONE;
                yasm_intnum_destroy(num);
            }
        }

        bc->len += immlen/8;
    }

    /* VEX and XOP prefixes never have REX (it's embedded in the opcode).
     * For VEX, we can come into this function with the three byte form,
     * so we need to see if we can optimize to the two byte form.
     * We can't do it earlier, as we don't know all of the REX byte until now.
     */
    if (insn->special_prefix == 0xC4) {
        /* See if we can shorten the VEX prefix to its two byte form.
         * In order to do this, REX.X, REX.B, and REX.W/VEX.W must all be 0,
         * and the VEX mmmmm field must be 1.
         */
        if ((insn->opcode.opcode[0] & 0x1F) == 1 &&
            (insn->opcode.opcode[1] & 0x80) == 0 &&
            (insn->rex == 0xff || (insn->rex & 0x0B) == 0)) {
            insn->opcode.opcode[0] = insn->opcode.opcode[1];
            insn->opcode.opcode[1] = insn->opcode.opcode[2];
            insn->opcode.opcode[2] = 0; /* sanity */
            insn->opcode.len = 2;
            insn->special_prefix = 0xC5;    /* mark as two-byte VEX */
        }
    } else if (insn->rex != 0xff && insn->rex != 0 &&
               insn->special_prefix != 0xC5 && insn->special_prefix != 0x8F)
        bc->len++;

    bc->len += insn->opcode.len;
    bc->len += x86_common_calc_len(&insn->common);
    bc->len += (insn->special_prefix != 0) ? 1:0;
    return 0;
}

static int
x86_bc_insn_expand(yasm_bytecode *bc, int span, long old_val, long new_val,
                   /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres)
{
    x86_insn *insn = (x86_insn *)bc->contents;
    x86_effaddr *x86_ea = insn->x86_ea;
    yasm_effaddr *ea = &x86_ea->ea;
    yasm_value *imm = insn->imm;

    if (ea && span == 1) {
        /* Change displacement length into word-sized */
        if (ea->disp.size == 8) {
            ea->disp.size = (insn->common.addrsize == 16) ? 16 : 32;
            x86_ea->modrm &= ~0300;
            x86_ea->modrm |= 0200;
            bc->len--;
            bc->len += ea->disp.size/8;
        }
    }

    if (imm && span == 2) {
        if (insn->postop == X86_POSTOP_SIGNEXT_IMM8) {
            /* Update bc->len for new opcode and immediate size */
            bc->len -= insn->opcode.len;
            bc->len += imm->size/8;

            /* Change to the word-sized opcode */
            insn->opcode.opcode[0] = insn->opcode.opcode[insn->opcode.len];
            insn->opcode.len = 1;
            insn->postop = X86_POSTOP_NONE;
        }
    }

    return 0;
}

static int
x86_bc_jmp_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                    void *add_span_data)
{
    x86_jmp *jmp = (x86_jmp *)bc->contents;
    yasm_bytecode *target_prevbc;
    unsigned char opersize;

    /* As opersize may be 0, figure out its "real" value. */
    opersize = (jmp->common.opersize == 0) ?
        jmp->common.mode_bits : jmp->common.opersize;

    bc->len += x86_common_calc_len(&jmp->common);

    if (jmp->op_sel == JMP_NEAR_FORCED || jmp->shortop.len == 0) {
        if (jmp->nearop.len == 0) {
            yasm_error_set(YASM_ERROR_TYPE, N_("near jump does not exist"));
            return -1;
        }

        /* Near jump, no spans needed */
        if (jmp->shortop.len == 0)
            jmp->op_sel = JMP_NEAR;
        bc->len += jmp->nearop.len;
        bc->len += (opersize == 16) ? 2 : 4;
        return 0;
    }

    if (jmp->op_sel == JMP_SHORT_FORCED || jmp->nearop.len == 0) {
        if (jmp->shortop.len == 0) {
            yasm_error_set(YASM_ERROR_TYPE, N_("short jump does not exist"));
            return -1;
        }

        /* We want to be sure to error if we exceed short length, so
         * put it in as a dependent expression (falling through).
         */
    }

    if (jmp->target.rel
        && (!yasm_symrec_get_label(jmp->target.rel, &target_prevbc)
            || target_prevbc->section != bc->section)) {
        /* External or out of segment, so we can't check distance.
         * Allowing short jumps depends on the objfmt supporting
         * 8-bit relocs.  While most don't, some might, so allow it here.
         * Otherwise default to word-sized.
         * The objfmt will error if not supported.
         */
        if (jmp->op_sel == JMP_SHORT_FORCED || jmp->nearop.len == 0) {
            if (jmp->op_sel == JMP_NONE)
                jmp->op_sel = JMP_SHORT;
            bc->len += jmp->shortop.len + 1;
        } else {
            jmp->op_sel = JMP_NEAR;
            bc->len += jmp->nearop.len;
            bc->len += (opersize == 16) ? 2 : 4;
        }
        return 0;
    }

    /* Default to short jump and generate span */
    if (jmp->op_sel == JMP_NONE)
        jmp->op_sel = JMP_SHORT;
    bc->len += jmp->shortop.len + 1;
    add_span(add_span_data, bc, 1, &jmp->target, -128+(long)bc->len,
             127+(long)bc->len);
    return 0;
}

static int
x86_bc_jmp_expand(yasm_bytecode *bc, int span, long old_val, long new_val,
                  /*@out@*/ long *neg_thres, /*@out@*/ long *pos_thres)
{
    x86_jmp *jmp = (x86_jmp *)bc->contents;
    unsigned char opersize;

    if (span != 1)
        yasm_internal_error(N_("unrecognized span id"));

    /* As opersize may be 0, figure out its "real" value. */
    opersize = (jmp->common.opersize == 0) ?
        jmp->common.mode_bits : jmp->common.opersize;

    if (jmp->op_sel == JMP_SHORT_FORCED || jmp->nearop.len == 0) {
        yasm_error_set(YASM_ERROR_VALUE, N_("short jump out of range"));
        return -1;
    }

    if (jmp->op_sel == JMP_NEAR)
        yasm_internal_error(N_("trying to expand an already-near jump"));

    /* Upgrade to a near jump */
    jmp->op_sel = JMP_NEAR;
    bc->len -= jmp->shortop.len + 1;
    bc->len += jmp->nearop.len;
    bc->len += (opersize == 16) ? 2 : 4;

    return 0;
}

static int
x86_bc_jmpfar_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                       void *add_span_data)
{
    x86_jmpfar *jmpfar = (x86_jmpfar *)bc->contents;
    unsigned char opersize;
   
    opersize = (jmpfar->common.opersize == 0) ?
        jmpfar->common.mode_bits : jmpfar->common.opersize;

    bc->len += jmpfar->opcode.len;
    bc->len += 2;       /* segment */
    bc->len += (opersize == 16) ? 2 : 4;
    bc->len += x86_common_calc_len(&jmpfar->common);

    return 0;
}

static void
x86_common_tobytes(const x86_common *common, unsigned char **bufp,
                   unsigned int segreg)
{
    if (segreg != 0)
        YASM_WRITE_8(*bufp, (unsigned char)segreg);
    if (common->addrsize != 0 && common->addrsize != common->mode_bits)
        YASM_WRITE_8(*bufp, 0x67);
    if (common->opersize != 0 &&
        ((common->mode_bits != 64 && common->opersize != common->mode_bits) ||
         (common->mode_bits == 64 && common->opersize == 16)))
        YASM_WRITE_8(*bufp, 0x66);
    if (common->lockrep_pre != 0)
        YASM_WRITE_8(*bufp, common->lockrep_pre);
}

static void
x86_opcode_tobytes(const x86_opcode *opcode, unsigned char **bufp)
{
    unsigned int i;
    for (i=0; i<opcode->len; i++)
        YASM_WRITE_8(*bufp, opcode->opcode[i]);
}

static int
x86_bc_insn_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                    unsigned char *bufstart, void *d,
                    yasm_output_value_func output_value,
                    /*@unused@*/ yasm_output_reloc_func output_reloc)
{
    x86_insn *insn = (x86_insn *)bc->contents;
    /*@null@*/ x86_effaddr *x86_ea = (x86_effaddr *)insn->x86_ea;
    yasm_value *imm = insn->imm;

    /* Prefixes */
    x86_common_tobytes(&insn->common, bufp,
                       x86_ea ? (unsigned int)(x86_ea->ea.segreg>>8) : 0);
    if (insn->special_prefix != 0)
        YASM_WRITE_8(*bufp, insn->special_prefix);
    if (insn->special_prefix == 0xC4 || insn->special_prefix == 0x8F) {
        /* 3-byte VEX/XOP; merge in 1s complement of REX.R, REX.X, REX.B */
        insn->opcode.opcode[0] &= 0x1F;
        if (insn->rex != 0xff)
            insn->opcode.opcode[0] |= ((~insn->rex) & 0x07) << 5;
        /* merge REX.W via ORing; there should never be a case in which REX.W
         * is important when VEX.W is already set by the instruction.
         */
        if (insn->rex != 0xff && (insn->rex & 0x8) != 0)
            insn->opcode.opcode[1] |= 0x80;
    } else if (insn->special_prefix == 0xC5) {
        /* 2-byte VEX; merge in 1s complement of REX.R */
        insn->opcode.opcode[0] &= 0x7F;
        if (insn->rex != 0xff && (insn->rex & 0x4) == 0)
            insn->opcode.opcode[0] |= 0x80;
        /* No other REX bits should be set */
        if (insn->rex != 0xff && (insn->rex & 0xB) != 0)
            yasm_internal_error(N_("x86: REX.WXB set, but 2-byte VEX"));
    } else if (insn->rex != 0xff && insn->rex != 0) {
        if (insn->common.mode_bits != 64)
            yasm_internal_error(N_("x86: got a REX prefix in non-64-bit mode"));
        YASM_WRITE_8(*bufp, insn->rex);
    }

    /* Opcode */
    x86_opcode_tobytes(&insn->opcode, bufp);

    /* Effective address: ModR/M (if required), SIB (if required), and
     * displacement (if required).
     */
    if (x86_ea) {
        if (x86_ea->need_modrm) {
            if (!x86_ea->valid_modrm)
                yasm_internal_error(N_("invalid Mod/RM in x86 tobytes_insn"));
            YASM_WRITE_8(*bufp, x86_ea->modrm);
        }

        if (x86_ea->need_sib) {
            if (!x86_ea->valid_sib)
                yasm_internal_error(N_("invalid SIB in x86 tobytes_insn"));
            YASM_WRITE_8(*bufp, x86_ea->sib);
        }

        if (x86_ea->ea.need_disp) {
            unsigned int disp_len = x86_ea->ea.disp.size/8;

            if (x86_ea->ea.disp.ip_rel) {
                /* Adjust relative displacement to end of bytecode */
                /*@only@*/ yasm_intnum *delta;
                delta = yasm_intnum_create_int(-(long)bc->len);
                if (!x86_ea->ea.disp.abs)
                    x86_ea->ea.disp.abs =
                        yasm_expr_create_ident(yasm_expr_int(delta), bc->line);
                else
                    x86_ea->ea.disp.abs =
                        yasm_expr_create(YASM_EXPR_ADD,
                                         yasm_expr_expr(x86_ea->ea.disp.abs),
                                         yasm_expr_int(delta), bc->line);
            }
            if (output_value(&x86_ea->ea.disp, *bufp, disp_len,
                             (unsigned long)(*bufp-bufstart), bc, 1, d))
                return 1;
            *bufp += disp_len;
        }
    }

    /* Immediate (if required) */
    if (imm) {
        unsigned int imm_len;
        if (insn->postop == X86_POSTOP_SIGNEXT_IMM8) {
            /* If we got here with this postop still set, we need to force
             * imm size to 8 here.
             */
            imm->size = 8;
            imm->sign = 1;
            imm_len = 1;
        } else
            imm_len = imm->size/8;
        if (output_value(imm, *bufp, imm_len, (unsigned long)(*bufp-bufstart),
                         bc, 1, d))
            return 1;
        *bufp += imm_len;
    }

    return 0;
}

static int
x86_bc_jmp_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                   unsigned char *bufstart, void *d,
                   yasm_output_value_func output_value,
                   /*@unused@*/ yasm_output_reloc_func output_reloc)
{
    x86_jmp *jmp = (x86_jmp *)bc->contents;
    unsigned char opersize;
    unsigned int i;
    /*@only@*/ yasm_intnum *delta;

    /* Prefixes */
    x86_common_tobytes(&jmp->common, bufp, 0);

    /* As opersize may be 0, figure out its "real" value. */
    opersize = (jmp->common.opersize == 0) ?
        jmp->common.mode_bits : jmp->common.opersize;

    /* Check here again to see if forms are actually legal. */
    switch (jmp->op_sel) {
        case JMP_SHORT_FORCED:
        case JMP_SHORT:
            /* 1 byte relative displacement */
            if (jmp->shortop.len == 0)
                yasm_internal_error(N_("short jump does not exist"));

            /* Opcode */
            x86_opcode_tobytes(&jmp->shortop, bufp);

            /* Adjust relative displacement to end of bytecode */
            delta = yasm_intnum_create_int(-(long)bc->len);
            if (!jmp->target.abs)
                jmp->target.abs = yasm_expr_create_ident(yasm_expr_int(delta),
                                                         bc->line);
            else
                jmp->target.abs =
                    yasm_expr_create(YASM_EXPR_ADD,
                                     yasm_expr_expr(jmp->target.abs),
                                     yasm_expr_int(delta), bc->line);

            jmp->target.size = 8;
            jmp->target.sign = 1;
            if (output_value(&jmp->target, *bufp, 1,
                             (unsigned long)(*bufp-bufstart), bc, 1, d))
                return 1;
            *bufp += 1;
            break;
        case JMP_NEAR_FORCED:
        case JMP_NEAR:
            /* 2/4 byte relative displacement (depending on operand size) */
            if (jmp->nearop.len == 0) {
                yasm_error_set(YASM_ERROR_TYPE,
                               N_("near jump does not exist"));
                return 1;
            }

            /* Opcode */
            x86_opcode_tobytes(&jmp->nearop, bufp);

            i = (opersize == 16) ? 2 : 4;

            /* Adjust relative displacement to end of bytecode */
            delta = yasm_intnum_create_int(-(long)bc->len);
            if (!jmp->target.abs)
                jmp->target.abs = yasm_expr_create_ident(yasm_expr_int(delta),
                                                         bc->line);
            else
                jmp->target.abs =
                    yasm_expr_create(YASM_EXPR_ADD,
                                     yasm_expr_expr(jmp->target.abs),
                                     yasm_expr_int(delta), bc->line);

            jmp->target.size = i*8;
            jmp->target.sign = 1;
            if (output_value(&jmp->target, *bufp, i,
                             (unsigned long)(*bufp-bufstart), bc, 1, d))
                return 1;
            *bufp += i;
            break;
        case JMP_NONE:
            yasm_internal_error(N_("jump op_sel cannot be JMP_NONE in tobytes"));
        default:
            yasm_internal_error(N_("unrecognized relative jump op_sel"));
    }
    return 0;
}

static int
x86_bc_jmpfar_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                      unsigned char *bufstart, void *d,
                      yasm_output_value_func output_value,
                      /*@unused@*/ yasm_output_reloc_func output_reloc)
{
    x86_jmpfar *jmpfar = (x86_jmpfar *)bc->contents;
    unsigned int i;
    unsigned char opersize;

    x86_common_tobytes(&jmpfar->common, bufp, 0);
    x86_opcode_tobytes(&jmpfar->opcode, bufp);

    /* As opersize may be 0, figure out its "real" value. */
    opersize = (jmpfar->common.opersize == 0) ?
        jmpfar->common.mode_bits : jmpfar->common.opersize;

    /* Absolute displacement: segment and offset */
    i = (opersize == 16) ? 2 : 4;
    jmpfar->offset.size = i*8;
    if (output_value(&jmpfar->offset, *bufp, i,
                     (unsigned long)(*bufp-bufstart), bc, 1, d))
        return 1;
    *bufp += i;
    jmpfar->segment.size = 16;
    if (output_value(&jmpfar->segment, *bufp, 2,
                     (unsigned long)(*bufp-bufstart), bc, 1, d))
        return 1;
    *bufp += 2;

    return 0;
}

int
yasm_x86__intnum_tobytes(yasm_arch *arch, const yasm_intnum *intn,
                         unsigned char *buf, size_t destsize, size_t valsize,
                         int shift, const yasm_bytecode *bc, int warn)
{
    /* Write value out. */
    yasm_intnum_get_sized(intn, buf, destsize, valsize, shift, 0, warn);
    return 0;
}
