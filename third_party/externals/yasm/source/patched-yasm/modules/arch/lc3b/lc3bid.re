/*
 * LC-3b identifier recognition and instruction handling
 *
 *  Copyright (C) 2003-2007  Peter Johnson
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

#include "modules/arch/lc3b/lc3barch.h"


/* Opcode modifiers.  The opcode bytes are in "reverse" order because the
 * parameters are read from the arch-specific data in LSB->MSB order.
 * (only for asthetic reasons in the lexer code below, no practical reason).
 */
#define MOD_OpHAdd  (1UL<<0)    /* Parameter adds to upper 8 bits of insn */
#define MOD_OpLAdd  (1UL<<1)    /* Parameter adds to lower 8 bits of insn */

/* Operand types.  These are more detailed than the "general" types for all
 * architectures, as they include the size, for instance.
 * Bit Breakdown (from LSB to MSB):
 *  - 1 bit = general type (must be exact match, except for =3):
 *            0 = immediate
 *            1 = register
 *
 * MSBs than the above are actions: what to do with the operand if the
 * instruction matches.  Essentially describes what part of the output bytecode
 * gets the operand.  This may require conversion (e.g. a register going into
 * an ea field).  Naturally, only one of each of these may be contained in the
 * operands of a single insn_info structure.
 *  - 2 bits = action:
 *             0 = does nothing (operand data is discarded)
 *             1 = DR field
 *             2 = SR field
 *             3 = immediate
 *
 * Immediate operands can have different sizes.
 *  - 3 bits = size:
 *             0 = no immediate
 *             1 = 4-bit immediate
 *             2 = 5-bit immediate
 *             3 = 6-bit index, word (16 bit)-multiple
 *             4 = 6-bit index, byte-multiple
 *             5 = 8-bit immediate, word-multiple
 *             6 = 9-bit signed immediate, word-multiple
 *             7 = 9-bit signed offset from next PC ($+2), word-multiple
 */
#define OPT_Imm         0x0
#define OPT_Reg         0x1
#define OPT_MASK        0x1

#define OPA_None        (0<<1)
#define OPA_DR          (1<<1)
#define OPA_SR          (2<<1)
#define OPA_Imm         (3<<1)
#define OPA_MASK        (3<<1)

#define OPI_None        (LC3B_IMM_NONE<<3)
#define OPI_4           (LC3B_IMM_4<<3)
#define OPI_5           (LC3B_IMM_5<<3)
#define OPI_6W          (LC3B_IMM_6_WORD<<3)
#define OPI_6B          (LC3B_IMM_6_BYTE<<3)
#define OPI_8           (LC3B_IMM_8<<3)
#define OPI_9           (LC3B_IMM_9<<3)
#define OPI_9PC         (LC3B_IMM_9_PC<<3)
#define OPI_MASK        (7<<3)

typedef struct lc3b_insn_info {
    /* Opcode modifiers for variations of instruction.  As each modifier reads
     * its parameter in LSB->MSB order from the arch-specific data[1] from the
     * lexer data, and the LSB of the arch-specific data[1] is reserved for the
     * count of insn_info structures in the instruction grouping, there can
     * only be a maximum of 3 modifiers.
     */
    unsigned int modifiers;

    /* The basic 2 byte opcode */
    unsigned int opcode;

    /* The number of operands this form of the instruction takes */
    unsigned char num_operands;

    /* The types of each operand, see above */
    unsigned int operands[3];
} lc3b_insn_info;

typedef struct lc3b_id_insn {
    yasm_insn insn;     /* base structure */

    /* instruction parse group - NULL if empty instruction (just prefixes) */
    /*@null@*/ const lc3b_insn_info *group;

    /* Modifier data */
    unsigned long mod_data;

    /* Number of elements in the instruction parse group */
    unsigned int num_info:8;
} lc3b_id_insn;

static void lc3b_id_insn_destroy(void *contents);
static void lc3b_id_insn_print(const void *contents, FILE *f, int indent_level);
static void lc3b_id_insn_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc);

static const yasm_bytecode_callback lc3b_id_insn_callback = {
    lc3b_id_insn_destroy,
    lc3b_id_insn_print,
    lc3b_id_insn_finalize,
    NULL,
    yasm_bc_calc_len_common,
    yasm_bc_expand_common,
    yasm_bc_tobytes_common,
    YASM_BC_SPECIAL_INSN
};

/*
 * Instruction groupings
 */

static const lc3b_insn_info empty_insn[] = {
    { 0, 0, 0, {0, 0, 0} }
};

static const lc3b_insn_info addand_insn[] = {
    { MOD_OpHAdd, 0x1000, 3,
      {OPT_Reg|OPA_DR, OPT_Reg|OPA_SR, OPT_Reg|OPA_Imm|OPI_5} },
    { MOD_OpHAdd, 0x1020, 3,
      {OPT_Reg|OPA_DR, OPT_Reg|OPA_SR, OPT_Imm|OPA_Imm|OPI_5} }
};

static const lc3b_insn_info br_insn[] = {
    { MOD_OpHAdd, 0x0000, 1, {OPT_Imm|OPA_Imm|OPI_9PC, 0, 0} }
};

static const lc3b_insn_info jmp_insn[] = {
    { 0, 0xC000, 2, {OPT_Reg|OPA_DR, OPT_Imm|OPA_Imm|OPI_9, 0} }
};

static const lc3b_insn_info lea_insn[] = {
    { 0, 0xE000, 2, {OPT_Reg|OPA_DR, OPT_Imm|OPA_Imm|OPI_9PC, 0} }
};

static const lc3b_insn_info ldst_insn[] = {
    { MOD_OpHAdd, 0x0000, 3,
      {OPT_Reg|OPA_DR, OPT_Reg|OPA_SR, OPT_Imm|OPA_Imm|OPI_6W} }
};

static const lc3b_insn_info ldstb_insn[] = {
    { MOD_OpHAdd, 0x0000, 3,
      {OPT_Reg|OPA_DR, OPT_Reg|OPA_SR, OPT_Imm|OPA_Imm|OPI_6B} }
};

static const lc3b_insn_info not_insn[] = {
    { 0, 0x903F, 2, {OPT_Reg|OPA_DR, OPT_Reg|OPA_SR, 0} }
};

static const lc3b_insn_info nooperand_insn[] = {
    { MOD_OpHAdd, 0x0000, 0, {0, 0, 0} }
};

static const lc3b_insn_info shift_insn[] = {
    { MOD_OpLAdd, 0xD000, 3,
      {OPT_Reg|OPA_DR, OPT_Reg|OPA_SR, OPT_Imm|OPA_Imm|OPI_4} }
};

static const lc3b_insn_info trap_insn[] = {
    { 0, 0xF000, 1, {OPT_Imm|OPA_Imm|OPI_8, 0, 0} }
};

static void
lc3b_id_insn_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    lc3b_id_insn *id_insn = (lc3b_id_insn *)bc->contents;
    lc3b_insn *insn;
    int num_info = id_insn->num_info;
    const lc3b_insn_info *info = id_insn->group;
    unsigned long mod_data = id_insn->mod_data;
    int found = 0;
    yasm_insn_operand *op;
    int i;

    yasm_insn_finalize(&id_insn->insn);

    /* Just do a simple linear search through the info array for a match.
     * First match wins.
     */
    for (; num_info>0 && !found; num_info--, info++) {
        int mismatch = 0;

        /* Match # of operands */
        if (id_insn->insn.num_operands != info->num_operands)
            continue;

        if (id_insn->insn.num_operands == 0) {
            found = 1;      /* no operands -> must have a match here. */
            break;
        }

        /* Match each operand type and size */
        for(i = 0, op = yasm_insn_ops_first(&id_insn->insn);
            op && i<info->num_operands && !mismatch;
            op = yasm_insn_op_next(op), i++) {
            /* Check operand type */
            switch ((int)(info->operands[i] & OPT_MASK)) {
                case OPT_Imm:
                    if (op->type != YASM_INSN__OPERAND_IMM)
                        mismatch = 1;
                    break;
                case OPT_Reg:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        mismatch = 1;
                    break;
                default:
                    yasm_internal_error(N_("invalid operand type"));
            }

            if (mismatch)
                break;
        }

        if (!mismatch) {
            found = 1;
            break;
        }
    }

    if (!found) {
        /* Didn't find a matching one */
        yasm_error_set(YASM_ERROR_TYPE,
                       N_("invalid combination of opcode and operands"));
        return;
    }

    /* Copy what we can from info */
    insn = yasm_xmalloc(sizeof(lc3b_insn));
    yasm_value_initialize(&insn->imm, NULL, 0);
    insn->imm_type = LC3B_IMM_NONE;
    insn->opcode = info->opcode;

    /* Apply modifiers */
    if (info->modifiers & MOD_OpHAdd) {
        insn->opcode += ((unsigned int)(mod_data & 0xFF))<<8;
        mod_data >>= 8;
    }
    if (info->modifiers & MOD_OpLAdd) {
        insn->opcode += (unsigned int)(mod_data & 0xFF);
        /*mod_data >>= 8;*/
    }

    /* Go through operands and assign */
    if (id_insn->insn.num_operands > 0) {
        for(i = 0, op = yasm_insn_ops_first(&id_insn->insn);
            op && i<info->num_operands; op = yasm_insn_op_next(op), i++) {

            switch ((int)(info->operands[i] & OPA_MASK)) {
                case OPA_None:
                    /* Throw away the operand contents */
                    if (op->type == YASM_INSN__OPERAND_IMM)
                        yasm_expr_destroy(op->data.val);
                    break;
                case OPA_DR:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        yasm_internal_error(N_("invalid operand conversion"));
                    insn->opcode |= ((unsigned int)(op->data.reg & 0x7)) << 9;
                    break;
                case OPA_SR:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        yasm_internal_error(N_("invalid operand conversion"));
                    insn->opcode |= ((unsigned int)(op->data.reg & 0x7)) << 6;
                    break;
                case OPA_Imm:
                    insn->imm_type = (info->operands[i] & OPI_MASK)>>3;
                    switch (op->type) {
                        case YASM_INSN__OPERAND_IMM:
                            if (insn->imm_type == LC3B_IMM_6_WORD
                                || insn->imm_type == LC3B_IMM_8
                                || insn->imm_type == LC3B_IMM_9
                                || insn->imm_type == LC3B_IMM_9_PC)
                                op->data.val = yasm_expr_create(YASM_EXPR_SHR,
                                    yasm_expr_expr(op->data.val),
                                    yasm_expr_int(yasm_intnum_create_uint(1)),
                                    op->data.val->line);
                            if (yasm_value_finalize_expr(&insn->imm,
                                                         op->data.val,
                                                         prev_bc, 0))
                                yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                                    N_("immediate expression too complex"));
                            break;
                        case YASM_INSN__OPERAND_REG:
                            if (yasm_value_finalize_expr(&insn->imm,
                                    yasm_expr_create_ident(yasm_expr_int(
                                    yasm_intnum_create_uint(op->data.reg & 0x7)),
                                    bc->line), prev_bc, 0))
                                yasm_internal_error(N_("reg expr too complex?"));
                            break;
                        default:
                            yasm_internal_error(N_("invalid operand conversion"));
                    }
                    break;
                default:
                    yasm_internal_error(N_("unknown operand action"));
            }

            /* Clear so it doesn't get destroyed */
            op->type = YASM_INSN__OPERAND_REG;
        }

        if (insn->imm_type == LC3B_IMM_9_PC) {
            if (insn->imm.seg_of || insn->imm.rshift > 1
                || insn->imm.curpos_rel)
                yasm_error_set(YASM_ERROR_VALUE, N_("invalid jump target"));
            insn->imm.curpos_rel = 1;
        }
    }

    /* Transform the bytecode */
    yasm_lc3b__bc_transform_insn(bc, insn);
}


#define YYCTYPE         unsigned char
#define YYCURSOR        id
#define YYLIMIT         id
#define YYMARKER        marker
#define YYFILL(n)       (void)(n)

yasm_arch_regtmod
yasm_lc3b__parse_check_regtmod(yasm_arch *arch, const char *oid, size_t id_len,
                               uintptr_t *data)
{
    const YYCTYPE *id = (const YYCTYPE *)oid;
    /*const char *marker;*/
    /*!re2c
        /* integer registers */
        'r' [0-7]       {
            *data = (oid[1]-'0');
            return YASM_ARCH_REG;
        }

        /* catchalls */
        [\001-\377]+    {
            return YASM_ARCH_NOTREGTMOD;
        }
        [\000]  {
            return YASM_ARCH_NOTREGTMOD;
        }
    */
}

#define RET_INSN(g, m) \
    do { \
        group = g##_insn; \
        mod = m; \
        nelems = NELEMS(g##_insn); \
        goto done; \
    } while(0)

yasm_arch_insnprefix
yasm_lc3b__parse_check_insnprefix(yasm_arch *arch, const char *oid,
                                  size_t id_len, unsigned long line,
                                  yasm_bytecode **bc, uintptr_t *prefix)
{
    const YYCTYPE *id = (const YYCTYPE *)oid;
    const lc3b_insn_info *group = empty_insn;
    unsigned long mod = 0;
    unsigned int nelems = NELEMS(empty_insn);
    lc3b_id_insn *id_insn;

    *bc = (yasm_bytecode *)NULL;
    *prefix = 0;

    /*const char *marker;*/
    /*!re2c
        /* instructions */

        'add' { RET_INSN(addand, 0x00); }
        'and' { RET_INSN(addand, 0x40); }

        'br' { RET_INSN(br, 0x00); }
        'brn' { RET_INSN(br, 0x08); }
        'brz' { RET_INSN(br, 0x04); }
        'brp' { RET_INSN(br, 0x02); }
        'brnz' { RET_INSN(br, 0x0C); }
        'brnp' { RET_INSN(br, 0x0A); }
        'brzp' { RET_INSN(br, 0x06); }
        'brnzp' { RET_INSN(br, 0x0E); }
        'jsr' { RET_INSN(br, 0x40); }

        'jmp' { RET_INSN(jmp, 0); }

        'lea' { RET_INSN(lea, 0); }

        'ld' { RET_INSN(ldst, 0x20); }
        'ldi' { RET_INSN(ldst, 0xA0); }
        'st' { RET_INSN(ldst, 0x30); }
        'sti' { RET_INSN(ldst, 0xB0); }

        'ldb' { RET_INSN(ldstb, 0x60); }
        'stb' { RET_INSN(ldstb, 0x70); }

        'not' { RET_INSN(not, 0); }

        'ret' { RET_INSN(nooperand, 0xCE); }
        'rti' { RET_INSN(nooperand, 0x80); }
        'nop' { RET_INSN(nooperand, 0); }

        'lshf' { RET_INSN(shift, 0x00); }
        'rshfl' { RET_INSN(shift, 0x10); }
        'rshfa' { RET_INSN(shift, 0x30); }

        'trap' { RET_INSN(trap, 0); }

        /* catchalls */
        [\001-\377]+    {
            return YASM_ARCH_NOTINSNPREFIX;
        }
        [\000]  {
            return YASM_ARCH_NOTINSNPREFIX;
        }
    */

done:
    id_insn = yasm_xmalloc(sizeof(lc3b_id_insn));
    yasm_insn_initialize(&id_insn->insn);
    id_insn->group = group;
    id_insn->mod_data = mod;
    id_insn->num_info = nelems;
    *bc = yasm_bc_create_common(&lc3b_id_insn_callback, id_insn, line);
    return YASM_ARCH_INSN;
}

static void
lc3b_id_insn_destroy(void *contents)
{
    lc3b_id_insn *id_insn = (lc3b_id_insn *)contents;
    yasm_insn_delete(&id_insn->insn, yasm_lc3b__ea_destroy);
    yasm_xfree(contents);
}

static void
lc3b_id_insn_print(const void *contents, FILE *f, int indent_level)
{
    const lc3b_id_insn *id_insn = (const lc3b_id_insn *)contents;
    yasm_insn_print(&id_insn->insn, f, indent_level);
    /*TODO*/
}

/*@only@*/ yasm_bytecode *
yasm_lc3b__create_empty_insn(yasm_arch *arch, unsigned long line)
{
    lc3b_id_insn *id_insn = yasm_xmalloc(sizeof(lc3b_id_insn));

    yasm_insn_initialize(&id_insn->insn);
    id_insn->group = empty_insn;
    id_insn->mod_data = 0;
    id_insn->num_info = NELEMS(empty_insn);

    return yasm_bc_create_common(&lc3b_id_insn_callback, id_insn, line);
}
