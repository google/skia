/*
 * x86 identifier recognition and instruction handling
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
#include <ctype.h>
#include <util.h>

#include <libyasm.h>
#include <libyasm/phash.h>

#include "modules/arch/x86/x86arch.h"


static const char *cpu_find_reverse(unsigned int cpu0, unsigned int cpu1,
                                    unsigned int cpu2);

/* Opcode modifiers. */
#define MOD_Gap     0   /* Eats a parameter / does nothing */
#define MOD_PreAdd  1   /* Parameter adds to "special" prefix */
#define MOD_Op0Add  2   /* Parameter adds to opcode byte 0 */
#define MOD_Op1Add  3   /* Parameter adds to opcode byte 1 */
#define MOD_Op2Add  4   /* Parameter adds to opcode byte 2 */
#define MOD_SpAdd   5   /* Parameter adds to "spare" value */
#define MOD_OpSizeR 6   /* Parameter replaces opersize */
#define MOD_Imm8    7   /* Parameter is included as immediate byte */
#define MOD_AdSizeR 8   /* Parameter replaces addrsize (jmp only) */
#define MOD_DOpS64R 9   /* Parameter replaces default 64-bit opersize */
#define MOD_Op1AddSp 10 /* Parameter is added as "spare" to opcode byte 2 */
#define MOD_SetVEX  11  /* Parameter replaces internal VEX prefix value */

/* GAS suffix flags for instructions */
enum x86_gas_suffix_flags {
    SUF_Z = 1<<0,   /* no suffix */
    SUF_B = 1<<1,
    SUF_W = 1<<2,
    SUF_L = 1<<3,
    SUF_Q = 1<<4,
    SUF_S = 1<<5,
    SUF_MASK = SUF_Z|SUF_B|SUF_W|SUF_L|SUF_Q|SUF_S,

    /* Flags only used in x86_insn_info */
    GAS_ONLY = 1<<6,        /* Only available in GAS mode */
    GAS_ILLEGAL = 1<<7,     /* Illegal in GAS mode */
    GAS_NO_REV = 1<<8       /* Don't reverse operands in GAS mode */
};

/* Miscellaneous flag tests for instructions */
enum x86_misc_flags {
    /* These are tested against BITS==64. */
    ONLY_64 = 1<<0,         /* Only available in 64-bit mode */
    NOT_64 = 1<<1,          /* Not available (invalid) in 64-bit mode */
    /* These are tested against whether the base instruction is an AVX one. */
    ONLY_AVX = 1<<2,        /* Only available in AVX instruction */
    NOT_AVX = 1<<3          /* Not available (invalid) in AVX instruction */
};

enum x86_operand_type {
    OPT_Imm = 0,        /* immediate */
    OPT_Reg = 1,        /* any general purpose or FPU register */
    OPT_Mem = 2,        /* memory */
    OPT_RM = 3,         /* any general purpose or FPU register OR memory */
    OPT_SIMDReg = 4,    /* any MMX or XMM register */
    OPT_SIMDRM = 5,     /* any MMX or XMM register OR memory */
    OPT_SegReg = 6,     /* any segment register */
    OPT_CRReg = 7,      /* any CR register */
    OPT_DRReg = 8,      /* any DR register */
    OPT_TRReg = 9,      /* any TR register */
    OPT_ST0 = 10,       /* ST0 */
    OPT_Areg = 11,      /* AL/AX/EAX/RAX (depending on size) */
    OPT_Creg = 12,      /* CL/CX/ECX/RCX (depending on size) */
    OPT_Dreg = 13,      /* DL/DX/EDX/RDX (depending on size) */
    OPT_CS = 14,        /* CS */
    OPT_DS = 15,        /* DS */
    OPT_ES = 16,        /* ES */
    OPT_FS = 17,        /* FS */
    OPT_GS = 18,        /* GS */
    OPT_SS = 19,        /* SS */
    OPT_CR4 = 20,       /* CR4 */
    /* memory offset (an EA, but with no registers allowed)
     * [special case for MOV opcode]
     */
    OPT_MemOffs = 21,
    OPT_Imm1 = 22,      /* immediate, value=1 (for special-case shift) */
    /* immediate, does not contain SEG:OFF (for jmp/call) */
    OPT_ImmNotSegOff = 23,
    OPT_XMM0 = 24,      /* XMM0 */
    /* AX/EAX/RAX memory operand only (EA) [special case for SVM opcodes]
     */
    OPT_MemrAX = 25,
    /* EAX memory operand only (EA) [special case for SVM skinit opcode] */
    OPT_MemEAX = 26,
    /* XMM VSIB memory operand */
    OPT_MemXMMIndex = 27,
    /* YMM VSIB memory operand */
    OPT_MemYMMIndex = 28
};

enum x86_operand_size {
    /* any size acceptable/no size spec acceptable (dep. on strict) */
    OPS_Any = 0,
    /* 8/16/32/64/80/128/256 bits (from user or reg size) */
    OPS_8 = 1,
    OPS_16 = 2,
    OPS_32 = 3,
    OPS_64 = 4,
    OPS_80 = 5,
    OPS_128 = 6,
    OPS_256 = 7,
    /* current BITS setting; when this is used the size matched
     * gets stored into the opersize as well.
     */
    OPS_BITS = 8
};

enum x86_operand_targetmod {
    OPTM_None = 0,  /* no target mod acceptable */
    OPTM_Near = 1,  /* NEAR */
    OPTM_Short = 2, /* SHORT */
    OPTM_Far = 3,   /* FAR (or SEG:OFF immediate) */
    OPTM_To = 4     /* TO */
};

enum x86_operand_action {
    OPA_None = 0,   /* does nothing (operand data is discarded) */
    OPA_EA = 1,     /* operand data goes into ea field */
    OPA_Imm = 2,    /* operand data goes into imm field */
    OPA_SImm = 3,   /* operand data goes into sign-extended imm field */
    OPA_Spare = 4,  /* operand data goes into "spare" field */
    OPA_Op0Add = 5, /* operand data is added to opcode byte 0 */
    OPA_Op1Add = 6, /* operand data is added to opcode byte 1 */
    /* operand data goes into BOTH ea and spare
     * (special case for imul opcode)
     */
    OPA_SpareEA = 7,
    /* relative jump (outputs a jmp instead of normal insn) */
    OPA_JmpRel = 8,
    /* operand size goes into address size (jmp only) */
    OPA_AdSizeR = 9,
    /* far jump (outputs a farjmp instead of normal insn) */
    OPA_JmpFar = 10,
    /* ea operand only sets address size (no actual ea field) */
    OPA_AdSizeEA = 11,
    OPA_VEX = 12,   /* operand data goes into VEX/XOP "vvvv" field */
    /* operand data goes into BOTH VEX/XOP "vvvv" field and ea field */
    OPA_EAVEX = 13,
    /* operand data goes into BOTH VEX/XOP "vvvv" field and spare field */
    OPA_SpareVEX = 14,
    /* operand data goes into upper 4 bits of immediate byte (VEX is4 field) */
    OPA_VEXImmSrc = 15,
    /* operand data goes into bottom 4 bits of immediate byte
     * (currently only VEX imz2 field)
     */
    OPA_VEXImm = 16
};

enum x86_operand_post_action {
    OPAP_None = 0,
    /* sign-extended imm8 that could expand to a large imm16/32 */
    OPAP_SImm8 = 1,
    /* could become a short opcode mov with bits=64 and a32 prefix */
    OPAP_ShortMov = 2,
    /* forced 16-bit address size (override ignored, no prefix) */
    OPAP_A16 = 3,
    /* large imm64 that can become a sign-extended imm32 */
    OPAP_SImm32Avail = 4
};

typedef struct x86_info_operand {
    /* Operand types.  These are more detailed than the "general" types for all
     * architectures, as they include the size, for instance.
     */

    /* general type (must be exact match, except for RM types): */
    unsigned int type:5;

    /* size (user-specified, or from register size) */
    unsigned int size:4;

    /* size implicit or explicit ("strictness" of size matching on
     * non-registers -- registers are always strictly matched):
     * 0 = user size must exactly match size above.
     * 1 = user size either unspecified or exactly match size above.
     */
    unsigned int relaxed:1;

    /* effective address size
     * 0 = any address size allowed except for 64-bit
     * 1 = only 64-bit address size allowed
     */
    unsigned int eas64:1;

    /* target modification */
    unsigned int targetmod:3;

    /* Actions: what to do with the operand if the instruction matches.
     * Essentially describes what part of the output bytecode gets the
     * operand.  This may require conversion (e.g. a register going into
     * an ea field).  Naturally, only one of each of these may be contained
     * in the operands of a single insn_info structure.
     */
    unsigned int action:5;

    /* Postponed actions: actions which can't be completed at
     * parse-time due to possibly dependent expressions.  For these, some
     * additional data (stored in the second byte of the opcode with a
     * one-byte opcode) is passed to later stages of the assembler with
     * flags set to indicate postponed actions.
     */
    unsigned int post_action:3;
} x86_info_operand;

typedef struct x86_insn_info {
    /* GAS suffix flags */
    unsigned int gas_flags:9;      /* Enabled for these GAS suffixes */

    /* Tests against BITS==64, AVX, and XOP */
    unsigned int misc_flags:5;

    /* The CPU feature flags needed to execute this instruction.  This is OR'ed
     * with arch-specific data[2].  This combined value is compared with
     * cpu_enabled to see if all bits set here are set in cpu_enabled--if so,
     * the instruction is available on this CPU.
     */
    unsigned int cpu0:6;
    unsigned int cpu1:6;
    unsigned int cpu2:6;

    /* Opcode modifiers for variations of instruction.  As each modifier reads
     * its parameter in LSB->MSB order from the arch-specific data[1] from the
     * lexer data, and the LSB of the arch-specific data[1] is reserved for the
     * count of insn_info structures in the instruction grouping, there can
     * only be a maximum of 3 modifiers.
     */
    unsigned char modifiers[3];

    /* Operand Size */
    unsigned char opersize;

    /* Default operand size in 64-bit mode (0 = 32-bit for readability). */
    unsigned char def_opersize_64;

    /* A special instruction prefix, used for some of the Intel SSE and SSE2
     * instructions.  Intel calls these 3-byte opcodes, but in AMD64's 64-bit
     * mode, they're treated like normal prefixes (e.g. the REX prefix needs
     * to be *after* the F2/F3/66 "prefix").
     * (0=no special prefix)
     * 0xC0 - 0xCF indicate a VEX prefix, with the four LSBs holding "WLpp":
     *  W: VEX.W field (meaning depends on opcode)
     *  L: 0=128-bit, 1=256-bit
     *  pp: SIMD prefix designation:
     *      00: None
     *      01: 66
     *      10: F3
     *      11: F2
     * 0x80 - 0x8F indicate a XOP prefix, with the four LSBs holding "WLpp":
     *  same meanings as VEX prefix.
     */
    unsigned char special_prefix;

    /* The length of the basic opcode */
    unsigned char opcode_len;

    /* The basic 1-3 byte opcode (not including the special instruction
     * prefix).
     */
    unsigned char opcode[3];

    /* The 3-bit "spare" value (extended opcode) for the R/M byte field */
    unsigned char spare;

    /* The number of operands this form of the instruction takes */
    unsigned int num_operands:4;

    /* The index into the insn_operands array which contains the type of each
     * operand, see above
     */
    unsigned int operands_index:12;
} x86_insn_info;

typedef struct x86_id_insn {
    yasm_insn insn;     /* base structure */

    /* instruction parse group - NULL if empty instruction (just prefixes) */
    /*@null@*/ const x86_insn_info *group;

    /* CPU feature flags enabled at the time of parsing the instruction */
    wordptr cpu_enabled;

    /* Modifier data */
    unsigned char mod_data[3];

    /* Number of elements in the instruction parse group */
    unsigned int num_info:8;

    /* BITS setting active at the time of parsing the instruction */
    unsigned int mode_bits:8;

    /* Suffix flags */
    unsigned int suffix:9;

    /* Tests against BITS==64 and AVX */
    unsigned int misc_flags:5;

    /* Parser enabled at the time of parsing the instruction */
    unsigned int parser:2;

    /* Strict forced setting at the time of parsing the instruction */
    unsigned int force_strict:1;

    /* Default rel setting at the time of parsing the instruction */
    unsigned int default_rel:1;
} x86_id_insn;

static void x86_id_insn_destroy(void *contents);
static void x86_id_insn_print(const void *contents, FILE *f, int indent_level);
static void x86_id_insn_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc);

static const yasm_bytecode_callback x86_id_insn_callback = {
    x86_id_insn_destroy,
    x86_id_insn_print,
    x86_id_insn_finalize,
    NULL,
    yasm_bc_calc_len_common,
    yasm_bc_expand_common,
    yasm_bc_tobytes_common,
    YASM_BC_SPECIAL_INSN
};

#include "x86insns.c"

/* Looks for the first SIMD register match for the purposes of VSIB matching.
 * Full legality checking is performed in EA code.
 */
static int
x86_expr_contains_simd_cb(const yasm_expr__item *ei, void *d)
{
    int ymm = *((int *)d);
    if (ei->type != YASM_EXPR_REG)
        return 0;
    switch ((x86_expritem_reg_size)(ei->data.reg & ~0xFUL)) {
        case X86_XMMREG:
            if (!ymm)
                return 1;
            break;
        case X86_YMMREG:
            if (ymm)
                return 1;
            break;
        default:
            break;
    }
    return 0;
}

static int
x86_expr_contains_simd(const yasm_expr *e, int ymm)
{
    return yasm_expr__traverse_leaves_in_const(e, &ymm,
                                               x86_expr_contains_simd_cb);
}

static void
x86_finalize_common(x86_common *common, const x86_insn_info *info,
                    unsigned int mode_bits)
{
    common->addrsize = 0;
    common->opersize = info->opersize;
    common->lockrep_pre = 0;
    common->mode_bits = (unsigned char)mode_bits;
}

static void
x86_finalize_opcode(x86_opcode *opcode, const x86_insn_info *info)
{
    opcode->len = info->opcode_len;
    opcode->opcode[0] = info->opcode[0];
    opcode->opcode[1] = info->opcode[1];
    opcode->opcode[2] = info->opcode[2];
}

/* Clear operands so they don't get destroyed after we've copied references. */
static void
x86_id_insn_clear_operands(x86_id_insn *id_insn)
{
    yasm_insn_operand *op = yasm_insn_ops_first(&id_insn->insn);
    while (op) {
        op->type = YASM_INSN__OPERAND_REG;
        op = yasm_insn_op_next(op);
    }
}

static void
x86_finalize_jmpfar(yasm_bytecode *bc, yasm_bytecode *prev_bc,
                    const x86_insn_info *info)
{
    x86_id_insn *id_insn = (x86_id_insn *)bc->contents;
    unsigned char *mod_data = id_insn->mod_data;
    unsigned int mode_bits = id_insn->mode_bits;
    x86_jmpfar *jmpfar;
    yasm_insn_operand *op;
    unsigned int i;

    jmpfar = yasm_xmalloc(sizeof(x86_jmpfar));
    x86_finalize_common(&jmpfar->common, info, mode_bits);
    x86_finalize_opcode(&jmpfar->opcode, info);

    op = yasm_insn_ops_first(&id_insn->insn);

    if (op->type == YASM_INSN__OPERAND_IMM && op->seg) {
        /* SEG:OFF */
        if (yasm_value_finalize_expr(&jmpfar->segment, op->seg, prev_bc, 16))
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("jump target segment too complex"));
        if (yasm_value_finalize_expr(&jmpfar->offset, op->data.val, prev_bc,
                                     0))
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("jump target offset too complex"));
    } else if (op->targetmod == X86_FAR) {
        /* "FAR imm" target needs to become "seg imm:imm". */
        yasm_expr *e = yasm_expr_create_branch(YASM_EXPR_SEG,
                                               yasm_expr_copy(op->data.val),
                                               op->data.val->line);
        if (yasm_value_finalize_expr(&jmpfar->offset, op->data.val, prev_bc, 0)
            || yasm_value_finalize_expr(&jmpfar->segment, e, prev_bc, 16))
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("jump target expression too complex"));
    } else if (yasm_insn_op_next(op)) {
        /* Two operand form (gas) */
        yasm_insn_operand *op2 = yasm_insn_op_next(op);
        if (yasm_value_finalize_expr(&jmpfar->segment, op->data.val, prev_bc,
                                     16))
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("jump target segment too complex"));
        if (yasm_value_finalize_expr(&jmpfar->offset, op2->data.val, prev_bc,
                                     0))
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("jump target offset too complex"));
        if (op2->size == OPS_BITS)
            jmpfar->common.opersize = (unsigned char)mode_bits;
    } else
        yasm_internal_error(N_("didn't get FAR expression in jmpfar"));

    /* Apply modifiers */
    for (i=0; i<NELEMS(info->modifiers); i++) {
        switch (info->modifiers[i]) {
            case MOD_Gap:
                break;
            case MOD_Op0Add:
                jmpfar->opcode.opcode[0] += mod_data[i];
                break;
            case MOD_Op1Add:
                jmpfar->opcode.opcode[1] += mod_data[i];
                break;
            case MOD_Op2Add:
                jmpfar->opcode.opcode[2] += mod_data[i];
                break;
            case MOD_Op1AddSp:
                jmpfar->opcode.opcode[1] += mod_data[i]<<3;
                break;
            default:
                break;
        }
    }

    yasm_x86__bc_apply_prefixes((x86_common *)jmpfar, NULL,
                                info->def_opersize_64,
                                id_insn->insn.num_prefixes,
                                id_insn->insn.prefixes);

    x86_id_insn_clear_operands(id_insn);

    /* Transform the bytecode */
    yasm_x86__bc_transform_jmpfar(bc, jmpfar);
}

static void
x86_finalize_jmp(yasm_bytecode *bc, yasm_bytecode *prev_bc,
                 const x86_insn_info *jinfo)
{
    x86_id_insn *id_insn = (x86_id_insn *)bc->contents;
    x86_jmp *jmp;
    int num_info = id_insn->num_info;
    const x86_insn_info *info = id_insn->group;
    unsigned char *mod_data = id_insn->mod_data;
    unsigned int mode_bits = id_insn->mode_bits;
    /*unsigned char suffix = id_insn->suffix;*/
    yasm_insn_operand *op;
    static const unsigned char size_lookup[] =
        {0, 8, 16, 32, 64, 80, 128, 0, 0};  /* 256 not needed */
    unsigned int i;

    /* We know the target is in operand 0, but sanity check for Imm. */
    op = yasm_insn_ops_first(&id_insn->insn);
    if (op->type != YASM_INSN__OPERAND_IMM)
        yasm_internal_error(N_("invalid operand conversion"));

    jmp = yasm_xmalloc(sizeof(x86_jmp));
    x86_finalize_common(&jmp->common, jinfo, mode_bits);
    if (yasm_value_finalize_expr(&jmp->target, op->data.val, prev_bc, 0))
        yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                       N_("jump target expression too complex"));
    if (jmp->target.seg_of || jmp->target.rshift || jmp->target.curpos_rel)
        yasm_error_set(YASM_ERROR_VALUE, N_("invalid jump target"));
    yasm_value_set_curpos_rel(&jmp->target, bc, 0);
    jmp->target.jump_target = 1;

    /* See if the user explicitly specified short/near/far. */
    switch (insn_operands[jinfo->operands_index+0].targetmod) {
        case OPTM_Short:
            jmp->op_sel = JMP_SHORT_FORCED;
            break;
        case OPTM_Near:
            jmp->op_sel = JMP_NEAR_FORCED;
            break;
        default:
            jmp->op_sel = JMP_NONE;
    }

    /* Check for address size setting in second operand, if present */
    if (jinfo->num_operands > 1 &&
        insn_operands[jinfo->operands_index+1].action == OPA_AdSizeR)
        jmp->common.addrsize = (unsigned char)
            size_lookup[insn_operands[jinfo->operands_index+1].size];

    /* Check for address size override */
    for (i=0; i<NELEMS(jinfo->modifiers); i++) {
        if (jinfo->modifiers[i] == MOD_AdSizeR)
            jmp->common.addrsize = mod_data[i];
    }

    /* Scan through other infos for this insn looking for short/near versions.
     * Needs to match opersize and number of operands, also be within CPU.
     */
    jmp->shortop.len = 0;
    jmp->nearop.len = 0;
    for (; num_info>0 && (jmp->shortop.len == 0 || jmp->nearop.len == 0);
         num_info--, info++) {
        /* Match CPU */
        if (mode_bits != 64 && (info->misc_flags & ONLY_64))
            continue;
        if (mode_bits == 64 && (info->misc_flags & NOT_64))
            continue;

        if (!BitVector_bit_test(id_insn->cpu_enabled, info->cpu0) ||
            !BitVector_bit_test(id_insn->cpu_enabled, info->cpu1) ||
            !BitVector_bit_test(id_insn->cpu_enabled, info->cpu2))
            continue;

        if (info->num_operands == 0)
            continue;

        if (insn_operands[info->operands_index+0].action != OPA_JmpRel)
            continue;

        if (info->opersize != jmp->common.opersize)
            continue;

        switch (insn_operands[info->operands_index+0].targetmod) {
            case OPTM_Short:
                x86_finalize_opcode(&jmp->shortop, info);
                for (i=0; i<NELEMS(info->modifiers); i++) {
                    if (info->modifiers[i] == MOD_Op0Add)
                        jmp->shortop.opcode[0] += mod_data[i];
                }
                break;
            case OPTM_Near:
                x86_finalize_opcode(&jmp->nearop, info);
                for (i=0; i<NELEMS(info->modifiers); i++) {
                    if (info->modifiers[i] == MOD_Op1Add)
                        jmp->nearop.opcode[1] += mod_data[i];
                }
                break;
        }
    }

    if ((jmp->op_sel == JMP_SHORT_FORCED) && (jmp->shortop.len == 0))
        yasm_error_set(YASM_ERROR_TYPE,
                       N_("no SHORT form of that jump instruction exists"));
    if ((jmp->op_sel == JMP_NEAR_FORCED) && (jmp->nearop.len == 0))
        yasm_error_set(YASM_ERROR_TYPE,
                       N_("no NEAR form of that jump instruction exists"));

    if (jmp->op_sel == JMP_NONE) {
        if (jmp->nearop.len == 0)
            jmp->op_sel = JMP_SHORT_FORCED;
        if (jmp->shortop.len == 0)
            jmp->op_sel = JMP_NEAR_FORCED;
    }

    yasm_x86__bc_apply_prefixes((x86_common *)jmp, NULL,
                                jinfo->def_opersize_64,
                                id_insn->insn.num_prefixes,
                                id_insn->insn.prefixes);

    x86_id_insn_clear_operands(id_insn);

    /* Transform the bytecode */
    yasm_x86__bc_transform_jmp(bc, jmp);
}

static const x86_insn_info *
x86_find_match(x86_id_insn *id_insn, yasm_insn_operand **ops,
               yasm_insn_operand **rev_ops, const unsigned int *size_lookup,
               int bypass)
{
    const x86_insn_info *info = id_insn->group;
    unsigned int num_info = id_insn->num_info;
    unsigned int suffix = id_insn->suffix;
    unsigned int mode_bits = id_insn->mode_bits;
    int found = 0;

    /* Just do a simple linear search through the info array for a match.
     * First match wins.
     */
    for (; num_info>0 && !found; num_info--, info++) {
        yasm_insn_operand *op, **use_ops;
        const x86_info_operand *info_ops =
            &insn_operands[info->operands_index];
        unsigned int gas_flags = info->gas_flags;
        unsigned int misc_flags = info->misc_flags;
        unsigned int size;
        int mismatch = 0;
        unsigned int i;

        /* Match CPU */
        if (mode_bits != 64 && (misc_flags & ONLY_64))
            continue;
        if (mode_bits == 64 && (misc_flags & NOT_64))
            continue;

        if (bypass != 8 &&
            (!BitVector_bit_test(id_insn->cpu_enabled, info->cpu0) ||
             !BitVector_bit_test(id_insn->cpu_enabled, info->cpu1) ||
             !BitVector_bit_test(id_insn->cpu_enabled, info->cpu2)))
            continue;

        /* Match # of operands */
        if (id_insn->insn.num_operands != info->num_operands)
            continue;

        /* Match AVX */
        if (!(id_insn->misc_flags & ONLY_AVX) && (misc_flags & ONLY_AVX))
            continue;
        if ((id_insn->misc_flags & ONLY_AVX) && (misc_flags & NOT_AVX))
            continue;

        /* Match parser mode */
        if ((gas_flags & GAS_ONLY) && id_insn->parser != X86_PARSER_GAS)
            continue;
        if ((gas_flags & GAS_ILLEGAL) && id_insn->parser == X86_PARSER_GAS)
            continue;

        /* Match suffix (if required) */
        if (id_insn->parser == X86_PARSER_GAS
            && ((suffix & SUF_MASK) & (gas_flags & SUF_MASK)) == 0)
            continue;

        /* Use reversed operands in GAS mode if not otherwise specified */
        use_ops = ops;
        if (id_insn->parser == X86_PARSER_GAS && !(gas_flags & GAS_NO_REV))
            use_ops = rev_ops;

        if (id_insn->insn.num_operands == 0) {
            found = 1;      /* no operands -> must have a match here. */
            break;
        }

        /* Match each operand type and size */
        for (i = 0, op = use_ops[0]; op && i<info->num_operands && !mismatch;
             op = use_ops[++i]) {
            /* Check operand type */
            switch (info_ops[i].type) {
                case OPT_Imm:
                    if (op->type != YASM_INSN__OPERAND_IMM)
                        mismatch = 1;
                    break;
                case OPT_RM:
                    if (op->type == YASM_INSN__OPERAND_MEMORY)
                        break;
                    /*@fallthrough@*/
                case OPT_Reg:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        mismatch = 1;
                    else {
                        switch ((x86_expritem_reg_size)(op->data.reg&~0xFUL)) {
                            case X86_REG8:
                            case X86_REG8X:
                            case X86_REG16:
                            case X86_REG32:
                            case X86_REG64:
                            case X86_FPUREG:
                                break;
                            default:
                                mismatch = 1;
                                break;
                        }
                    }
                    break;
                case OPT_Mem:
                    if (op->type != YASM_INSN__OPERAND_MEMORY)
                        mismatch = 1;
                    break;
                case OPT_SIMDRM:
                    if (op->type == YASM_INSN__OPERAND_MEMORY)
                        break;
                    /*@fallthrough@*/
                case OPT_SIMDReg:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        mismatch = 1;
                    else {
                        switch ((x86_expritem_reg_size)(op->data.reg&~0xFUL)) {
                            case X86_MMXREG:
                            case X86_XMMREG:
                            case X86_YMMREG:
                                break;
                            default:
                                mismatch = 1;
                                break;
                        }
                    }
                    break;
                case OPT_SegReg:
                    if (op->type != YASM_INSN__OPERAND_SEGREG)
                        mismatch = 1;
                    break;
                case OPT_CRReg:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        (op->data.reg & ~0xFUL) != X86_CRREG)
                        mismatch = 1;
                    break;
                case OPT_DRReg:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        (op->data.reg & ~0xFUL) != X86_DRREG)
                        mismatch = 1;
                    break;
                case OPT_TRReg:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        (op->data.reg & ~0xFUL) != X86_TRREG)
                        mismatch = 1;
                    break;
                case OPT_ST0:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        op->data.reg != X86_FPUREG)
                        mismatch = 1;
                    break;
                case OPT_Areg:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        (info_ops[i].size == OPS_8 &&
                         op->data.reg != (X86_REG8 | 0) &&
                         op->data.reg != (X86_REG8X | 0)) ||
                        (info_ops[i].size == OPS_16 &&
                         op->data.reg != (X86_REG16 | 0)) ||
                        (info_ops[i].size == OPS_32 &&
                         op->data.reg != (X86_REG32 | 0)) ||
                        (info_ops[i].size == OPS_64 &&
                         op->data.reg != (X86_REG64 | 0)))
                        mismatch = 1;
                    break;
                case OPT_Creg:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        (info_ops[i].size == OPS_8 &&
                         op->data.reg != (X86_REG8 | 1) &&
                         op->data.reg != (X86_REG8X | 1)) ||
                        (info_ops[i].size == OPS_16 &&
                         op->data.reg != (X86_REG16 | 1)) ||
                        (info_ops[i].size == OPS_32 &&
                         op->data.reg != (X86_REG32 | 1)) ||
                        (info_ops[i].size == OPS_64 &&
                         op->data.reg != (X86_REG64 | 1)))
                        mismatch = 1;
                    break;
                case OPT_Dreg:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        (info_ops[i].size == OPS_8 &&
                         op->data.reg != (X86_REG8 | 2) &&
                         op->data.reg != (X86_REG8X | 2)) ||
                        (info_ops[i].size == OPS_16 &&
                         op->data.reg != (X86_REG16 | 2)) ||
                        (info_ops[i].size == OPS_32 &&
                         op->data.reg != (X86_REG32 | 2)) ||
                        (info_ops[i].size == OPS_64 &&
                         op->data.reg != (X86_REG64 | 2)))
                        mismatch = 1;
                    break;
                case OPT_CS:
                    if (op->type != YASM_INSN__OPERAND_SEGREG ||
                        (op->data.reg & 0xF) != 1)
                        mismatch = 1;
                    break;
                case OPT_DS:
                    if (op->type != YASM_INSN__OPERAND_SEGREG ||
                        (op->data.reg & 0xF) != 3)
                        mismatch = 1;
                    break;
                case OPT_ES:
                    if (op->type != YASM_INSN__OPERAND_SEGREG ||
                        (op->data.reg & 0xF) != 0)
                        mismatch = 1;
                    break;
                case OPT_FS:
                    if (op->type != YASM_INSN__OPERAND_SEGREG ||
                        (op->data.reg & 0xF) != 4)
                        mismatch = 1;
                    break;
                case OPT_GS:
                    if (op->type != YASM_INSN__OPERAND_SEGREG ||
                        (op->data.reg & 0xF) != 5)
                        mismatch = 1;
                    break;
                case OPT_SS:
                    if (op->type != YASM_INSN__OPERAND_SEGREG ||
                        (op->data.reg & 0xF) != 2)
                        mismatch = 1;
                    break;
                case OPT_CR4:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        op->data.reg != (X86_CRREG | 4))
                        mismatch = 1;
                    break;
                case OPT_MemOffs:
                    if (op->type != YASM_INSN__OPERAND_MEMORY ||
                        yasm_expr__contains(op->data.ea->disp.abs,
                                            YASM_EXPR_REG) ||
                        op->data.ea->pc_rel ||
                        (!op->data.ea->not_pc_rel && id_insn->default_rel &&
                         op->data.ea->disp.size != 64))
                        mismatch = 1;
                    break;
                case OPT_Imm1:
                    if (op->type == YASM_INSN__OPERAND_IMM) {
                        const yasm_intnum *num;
                        num = yasm_expr_get_intnum(&op->data.val, 0);
                        if (!num || !yasm_intnum_is_pos1(num))
                            mismatch = 1;
                    } else
                        mismatch = 1;
                    break;
                case OPT_ImmNotSegOff:
                    if (op->type != YASM_INSN__OPERAND_IMM ||
                        op->targetmod != 0 || op->seg)
                        mismatch = 1;
                    break;
                case OPT_XMM0:
                    if (op->type != YASM_INSN__OPERAND_REG ||
                        op->data.reg != X86_XMMREG)
                        mismatch = 1;
                    break;
                case OPT_MemrAX: {
                    const uintptr_t *regp;
                    if (op->type != YASM_INSN__OPERAND_MEMORY ||
                        !(regp = yasm_expr_get_reg(&op->data.ea->disp.abs, 0)) ||
                        (*regp != (X86_REG16 | 0) &&
                         *regp != (X86_REG32 | 0) &&
                         *regp != (X86_REG64 | 0)))
                        mismatch = 1;
                    break;
                }
                case OPT_MemEAX: {
                    const uintptr_t *regp;
                    if (op->type != YASM_INSN__OPERAND_MEMORY ||
                        !(regp = yasm_expr_get_reg(&op->data.ea->disp.abs, 0)) ||
                        *regp != (X86_REG32 | 0))
                        mismatch = 1;
                    break;
                }
                case OPT_MemXMMIndex:
                    if (op->type != YASM_INSN__OPERAND_MEMORY ||
                        !x86_expr_contains_simd(op->data.ea->disp.abs, 0))
                        mismatch = 1;
                    break;
                case OPT_MemYMMIndex:
                    if (op->type != YASM_INSN__OPERAND_MEMORY ||
                        !x86_expr_contains_simd(op->data.ea->disp.abs, 1))
                        mismatch = 1;
                    break;
                default:
                    yasm_internal_error(N_("invalid operand type"));
            }

            if (mismatch)
                break;

            /* Check operand size */
            size = size_lookup[info_ops[i].size];
            if (id_insn->parser == X86_PARSER_GAS) {
                /* Require relaxed operands for GAS mode (don't allow
                 * per-operand sizing).
                 */
                if (op->type == YASM_INSN__OPERAND_REG && op->size == 0) {
                    /* Register size must exactly match */
                    if (yasm_x86__get_reg_size(op->data.reg) != size)
                        mismatch = 1;
                } else if ((info_ops[i].type == OPT_Imm
                            || info_ops[i].type == OPT_ImmNotSegOff
                            || info_ops[i].type == OPT_Imm1)
                    && !info_ops[i].relaxed
                    && info_ops[i].action != OPA_JmpRel)
                    mismatch = 1;
            } else {
                if (op->type == YASM_INSN__OPERAND_REG && op->size == 0) {
                    /* Register size must exactly match */
                    if ((bypass == 4 && i == 0) || (bypass == 5 && i == 1)
                        || (bypass == 6 && i == 2))
                        ;
                    else if (yasm_x86__get_reg_size(op->data.reg) != size)
                        mismatch = 1;
                } else {
                    if ((bypass == 1 && i == 0) || (bypass == 2 && i == 1)
                        || (bypass == 3 && i == 2))
                        ;
                    else if (info_ops[i].relaxed) {
                        /* Relaxed checking */
                        if (size != 0 && op->size != size && op->size != 0)
                            mismatch = 1;
                    } else {
                        /* Strict checking */
                        if (op->size != size)
                            mismatch = 1;
                    }
                }
            }

            if (mismatch)
                break;

            /* Check for 64-bit effective address size in NASM mode */
            if (id_insn->parser != X86_PARSER_GAS &&
                op->type == YASM_INSN__OPERAND_MEMORY) {
                if (info_ops[i].eas64) {
                    if (op->data.ea->disp.size != 64)
                        mismatch = 1;
                } else if (op->data.ea->disp.size == 64)
                    mismatch = 1;
            }

            if (mismatch)
                break;

            /* Check target modifier */
            switch (info_ops[i].targetmod) {
                case OPTM_None:
                    if (op->targetmod != 0)
                        mismatch = 1;
                    break;
                case OPTM_Near:
                    if (op->targetmod != X86_NEAR)
                        mismatch = 1;
                    break;
                case OPTM_Short:
                    if (op->targetmod != X86_SHORT)
                        mismatch = 1;
                    break;
                case OPTM_Far:
                    if (op->targetmod != X86_FAR)
                        mismatch = 1;
                    break;
                case OPTM_To:
                    if (op->targetmod != X86_TO)
                        mismatch = 1;
                    break;
                default:
                    yasm_internal_error(N_("invalid target modifier type"));
            }
        }

        if (!mismatch) {
            found = 1;
            break;
        }
    }

    if (!found)
        return NULL;
    return info;
}

static void
x86_match_error(x86_id_insn *id_insn, yasm_insn_operand **ops,
                yasm_insn_operand **rev_ops, const unsigned int *size_lookup)
{
    const x86_insn_info *i;
    int ni;
    int found;
    int bypass;

    /* Check for matching # of operands */
    found = 0;
    for (ni=id_insn->num_info, i=id_insn->group; ni>0; ni--, i++) {
        if (id_insn->insn.num_operands == i->num_operands) {
            found = 1;
            break;
        }
    }
    if (!found) {
        yasm_error_set(YASM_ERROR_TYPE, N_("invalid number of operands"));
        return;
    }

    for (bypass=1; bypass<9; bypass++) {
        i = x86_find_match(id_insn, ops, rev_ops, size_lookup, bypass);
        if (i)
            break;
    }

    switch (bypass) {
        case 1:
        case 4:
            yasm_error_set(YASM_ERROR_TYPE,
                           N_("invalid size for operand %d"), 1);
            break;
        case 2:
        case 5:
            yasm_error_set(YASM_ERROR_TYPE,
                           N_("invalid size for operand %d"), 2);
            break;
        case 3:
        case 6:
            yasm_error_set(YASM_ERROR_TYPE,
                           N_("invalid size for operand %d"), 3);
            break;
        case 7:
            yasm_error_set(YASM_ERROR_TYPE,
                N_("one of source operand 1 or 3 must match dest operand"));
            break;
        case 8:
        {
            unsigned int cpu0 = i->cpu0, cpu1 = i->cpu1, cpu2 = i->cpu2;
            yasm_error_set(YASM_ERROR_TYPE,
                          N_("requires CPU%s"),
                          cpu_find_reverse(cpu0, cpu1, cpu2));
            break;
        }
        default:
            yasm_error_set(YASM_ERROR_TYPE,
                           N_("invalid combination of opcode and operands"));
    }
}

static void
x86_id_insn_finalize(yasm_bytecode *bc, yasm_bytecode *prev_bc)
{
    x86_id_insn *id_insn = (x86_id_insn *)bc->contents;
    x86_insn *insn;
    const x86_insn_info *info = id_insn->group;
    unsigned int mode_bits = id_insn->mode_bits;
    unsigned char *mod_data = id_insn->mod_data;
    yasm_insn_operand *op, *ops[5], *rev_ops[5];
    /*@null@*/ yasm_expr *imm;
    unsigned char im_len;
    unsigned char im_sign;
    unsigned char spare;
    unsigned char vexdata, vexreg;
    unsigned int i;
    unsigned int size_lookup[] = {0, 8, 16, 32, 64, 80, 128, 256, 0};
    unsigned long do_postop = 0;

    size_lookup[OPS_BITS] = mode_bits;

    yasm_insn_finalize(&id_insn->insn);

    /* Build local array of operands from list, since we know we have a max
     * of 5 operands.
     */
    if (id_insn->insn.num_operands > 5) {
        yasm_error_set(YASM_ERROR_TYPE, N_("too many operands"));
        return;
    }
    ops[0] = ops[1] = ops[2] = ops[3] = ops[4] = NULL;
    for (i = 0, op = yasm_insn_ops_first(&id_insn->insn);
         op && i < id_insn->insn.num_operands;
         op = yasm_insn_op_next(op), i++)
        ops[i] = op;

    /* If we're running in GAS mode, build a reverse array of the operands
     * as most GAS instructions have reversed operands from Intel style.
     */
    if (id_insn->parser == X86_PARSER_GAS) {
        rev_ops[0] = rev_ops[1] = rev_ops[2] = rev_ops[3] = rev_ops[4] = NULL;
        for (i = id_insn->insn.num_operands-1,
             op = yasm_insn_ops_first(&id_insn->insn);
             op; op = yasm_insn_op_next(op), i--)
            rev_ops[i] = op;
    }

    /* If we're running in GAS mode, look at the first insn_info to see
     * if this is a relative jump (OPA_JmpRel).  If so, run through the
     * operands and adjust for dereferences / lack thereof.
     */
    if (id_insn->parser == X86_PARSER_GAS
        && insn_operands[info->operands_index+0].action == OPA_JmpRel) {
        for (i = 0, op = ops[0]; op; op = ops[++i]) {
            if (!op->deref && (op->type == YASM_INSN__OPERAND_REG
                               || (op->type == YASM_INSN__OPERAND_MEMORY
                                   && op->data.ea->strong)))
                yasm_warn_set(YASM_WARN_GENERAL,
                              N_("indirect call without `*'"));
            if (!op->deref && op->type == YASM_INSN__OPERAND_MEMORY
                && !op->data.ea->strong) {
                /* Memory that is not dereferenced, and not strong, is
                 * actually an immediate for the purposes of relative jumps.
                 */
                if (op->data.ea->segreg != 0)
                    yasm_warn_set(YASM_WARN_GENERAL,
                                  N_("skipping prefixes on this instruction"));
                imm = op->data.ea->disp.abs;
                op->data.ea->disp.abs = NULL;
                yasm_x86__ea_destroy(op->data.ea);
                op->type = YASM_INSN__OPERAND_IMM;
                op->data.val = imm;
            }
        }
    }

    info = x86_find_match(id_insn, ops, rev_ops, size_lookup, 0);

    if (!info) {
        /* Didn't find a match */
        x86_match_error(id_insn, ops, rev_ops, size_lookup);
        return;
    }

    if (id_insn->insn.num_operands > 0) {
        switch (insn_operands[info->operands_index+0].action) {
            case OPA_JmpRel:
                /* Shortcut to JmpRel */
                x86_finalize_jmp(bc, prev_bc, info);
                return;
            case OPA_JmpFar:
                /* Shortcut to JmpFar */
                x86_finalize_jmpfar(bc, prev_bc, info);
                return;
        }
    }

    /* Copy what we can from info */
    insn = yasm_xmalloc(sizeof(x86_insn));
    x86_finalize_common(&insn->common, info, mode_bits);
    x86_finalize_opcode(&insn->opcode, info);
    insn->x86_ea = NULL;
    imm = NULL;
    insn->def_opersize_64 = info->def_opersize_64;
    insn->special_prefix = info->special_prefix;
    spare = info->spare;
    vexdata = 0;
    vexreg = 0;
    im_len = 0;
    im_sign = 0;
    insn->postop = X86_POSTOP_NONE;
    insn->rex = 0;

    /* Move VEX/XOP data (stored in special prefix) to separate location to
     * allow overriding of special prefix by modifiers.
     */
    if ((insn->special_prefix & 0xF0) == 0xC0 ||
        (insn->special_prefix & 0xF0) == 0x80) {
        vexdata = insn->special_prefix;
        insn->special_prefix = 0;
    }

    /* Apply modifiers */
    for (i=0; i<NELEMS(info->modifiers); i++) {
        switch (info->modifiers[i]) {
            case MOD_Gap:
                break;
            case MOD_PreAdd:
                insn->special_prefix += mod_data[i];
                break;
            case MOD_Op0Add:
                insn->opcode.opcode[0] += mod_data[i];
                break;
            case MOD_Op1Add:
                insn->opcode.opcode[1] += mod_data[i];
                break;
            case MOD_Op2Add:
                insn->opcode.opcode[2] += mod_data[i];
                break;
            case MOD_SpAdd:
                spare += mod_data[i];
                break;
            case MOD_OpSizeR:
                insn->common.opersize = mod_data[i];
                break;
            case MOD_Imm8:
                imm = yasm_expr_create_ident(yasm_expr_int(
                    yasm_intnum_create_uint(mod_data[i])), bc->line);
                im_len = 8;
                break;
            case MOD_DOpS64R:
                insn->def_opersize_64 = mod_data[i];
                break;
            case MOD_Op1AddSp:
                insn->opcode.opcode[1] += mod_data[i]<<3;
                break;
            case MOD_SetVEX:
                vexdata = mod_data[i];
                break;
            default:
                break;
        }
    }

    /* In 64-bit mode, if opersize is 64 and default is not 64,
     * force REX byte.
     */
    if (mode_bits == 64 && insn->common.opersize == 64 &&
        insn->def_opersize_64 != 64)
        insn->rex = 0x48;

    /* Go through operands and assign */
    if (id_insn->insn.num_operands > 0) {
        yasm_insn_operand **use_ops = ops;
        const x86_info_operand *info_ops =
            &insn_operands[info->operands_index];

        /* Use reversed operands in GAS mode if not otherwise specified */
        if (id_insn->parser == X86_PARSER_GAS
            && !(info->gas_flags & GAS_NO_REV))
            use_ops = rev_ops;

        for (i = 0, op = use_ops[0]; op && i<info->num_operands;
             op = use_ops[++i]) {
            switch (info_ops[i].action) {
                case OPA_None:
                    /* Throw away the operand contents */
                    switch (op->type) {
                        case YASM_INSN__OPERAND_REG:
                        case YASM_INSN__OPERAND_SEGREG:
                            break;
                        case YASM_INSN__OPERAND_MEMORY:
                            yasm_x86__ea_destroy(op->data.ea);
                            break;
                        case YASM_INSN__OPERAND_IMM:
                            yasm_expr_destroy(op->data.val);
                            break;
                    }
                    break;
                case OPA_EA:
                    switch (op->type) {
                        case YASM_INSN__OPERAND_REG:
                            insn->x86_ea =
                                yasm_x86__ea_create_reg(insn->x86_ea,
                                    (unsigned long)op->data.reg, &insn->rex,
                                    mode_bits);
                            break;
                        case YASM_INSN__OPERAND_SEGREG:
                            yasm_internal_error(
                                N_("invalid operand conversion"));
                        case YASM_INSN__OPERAND_MEMORY:
                            if (op->seg)
                                yasm_error_set(YASM_ERROR_VALUE,
                                    N_("invalid segment in effective address"));
                            insn->x86_ea = (x86_effaddr *)op->data.ea;
                            if (info_ops[i].type == OPT_MemOffs)
                                /* Special-case for MOV MemOffs instruction */
                                yasm_x86__ea_set_disponly(insn->x86_ea);
                            else if (info_ops[i].type == OPT_MemXMMIndex) {
                                /* Remember VSIB mode */
                                insn->x86_ea->vsib_mode = 1;
                                insn->x86_ea->need_sib = 1;
                            } else if (info_ops[i].type == OPT_MemYMMIndex) {
                                /* Remember VSIB mode */
                                insn->x86_ea->vsib_mode = 2;
                                insn->x86_ea->need_sib = 1;
                            } else if (id_insn->default_rel &&
                                       !op->data.ea->not_pc_rel &&
                                       op->data.ea->segreg != 0x6404 &&
                                       op->data.ea->segreg != 0x6505 &&
                                       !yasm_expr__contains(
                                          op->data.ea->disp.abs, YASM_EXPR_REG))
                                /* Enable default PC-rel if no regs and segreg
                                 * is not FS or GS.
                                 */
                                insn->x86_ea->ea.pc_rel = 1;
                            break;
                        case YASM_INSN__OPERAND_IMM:
                            insn->x86_ea =
                                yasm_x86__ea_create_imm(insn->x86_ea,
                                    op->data.val,
                                    size_lookup[info_ops[i].size]);
                            break;
                    }
                    break;
                case OPA_EAVEX:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        yasm_internal_error(N_("invalid operand conversion"));
                    insn->x86_ea =
                        yasm_x86__ea_create_reg(insn->x86_ea,
                            (unsigned long)op->data.reg, &insn->rex, mode_bits);
                    vexreg = op->data.reg & 0xF;
                    break;
                case OPA_Imm:
                    if (op->seg)
                        yasm_error_set(YASM_ERROR_VALUE,
                                       N_("immediate does not support segment"));
                    if (op->type == YASM_INSN__OPERAND_IMM) {
                        imm = op->data.val;
                        im_len = size_lookup[info_ops[i].size];
                    } else
                        yasm_internal_error(N_("invalid operand conversion"));
                    break;
                case OPA_SImm:
                    if (op->seg)
                        yasm_error_set(YASM_ERROR_VALUE,
                                       N_("immediate does not support segment"));
                    if (op->type == YASM_INSN__OPERAND_IMM) {
                        imm = op->data.val;
                        im_len = size_lookup[info_ops[i].size];
                        im_sign = 1;
                    } else
                        yasm_internal_error(N_("invalid operand conversion"));
                    break;
                case OPA_Spare:
                    if (op->type == YASM_INSN__OPERAND_SEGREG)
                        spare = (unsigned char)(op->data.reg&7);
                    else if (op->type == YASM_INSN__OPERAND_REG) {
                        if (yasm_x86__set_rex_from_reg(&insn->rex, &spare,
                                op->data.reg, mode_bits, X86_REX_R))
                            return;
                    } else
                        yasm_internal_error(N_("invalid operand conversion"));
                    break;
                case OPA_SpareVEX:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        yasm_internal_error(N_("invalid operand conversion"));
                    if (yasm_x86__set_rex_from_reg(&insn->rex, &spare,
                            op->data.reg, mode_bits, X86_REX_R))
                        return;
                    vexreg = op->data.reg & 0xF;
                    break;
                case OPA_Op0Add:
                    if (op->type == YASM_INSN__OPERAND_REG) {
                        unsigned char opadd;
                        if (yasm_x86__set_rex_from_reg(&insn->rex, &opadd,
                                op->data.reg, mode_bits, X86_REX_B))
                            return;
                        insn->opcode.opcode[0] += opadd;
                    } else
                        yasm_internal_error(N_("invalid operand conversion"));
                    break;
                case OPA_Op1Add:
                    if (op->type == YASM_INSN__OPERAND_REG) {
                        unsigned char opadd;
                        if (yasm_x86__set_rex_from_reg(&insn->rex, &opadd,
                                op->data.reg, mode_bits, X86_REX_B))
                            return;
                        insn->opcode.opcode[1] += opadd;
                    } else
                        yasm_internal_error(N_("invalid operand conversion"));
                    break;
                case OPA_SpareEA:
                    if (op->type == YASM_INSN__OPERAND_REG) {
                        insn->x86_ea =
                            yasm_x86__ea_create_reg(insn->x86_ea,
                                (unsigned long)op->data.reg, &insn->rex,
                                mode_bits);
                        if (!insn->x86_ea ||
                            yasm_x86__set_rex_from_reg(&insn->rex, &spare,
                                op->data.reg, mode_bits, X86_REX_R)) {
                            if (insn->x86_ea)
                                yasm_xfree(insn->x86_ea);
                            yasm_xfree(insn);
                            return;
                        }
                    } else
                        yasm_internal_error(N_("invalid operand conversion"));
                    break;
                case OPA_AdSizeEA: {
                    const uintptr_t *regp = NULL;
                    /* Only implement this for OPT_MemrAX and OPT_MemEAX
                     * for now.
                     */
                    if (op->type != YASM_INSN__OPERAND_MEMORY ||
                        !(regp = yasm_expr_get_reg(&op->data.ea->disp.abs, 0)))
                        yasm_internal_error(N_("invalid operand conversion"));
                    /* 64-bit mode does not allow 16-bit addresses */
                    if (mode_bits == 64 && *regp == (X86_REG16 | 0))
                        yasm_error_set(YASM_ERROR_TYPE,
                            N_("16-bit addresses not supported in 64-bit mode"));
                    else if (*regp == (X86_REG16 | 0))
                        insn->common.addrsize = 16;
                    else if (*regp == (X86_REG32 | 0))
                        insn->common.addrsize = 32;
                    else if (mode_bits == 64 && *regp == (X86_REG64 | 0))
                        insn->common.addrsize = 64;
                    else
                        yasm_error_set(YASM_ERROR_TYPE,
                            N_("unsupported address size"));
                    yasm_x86__ea_destroy(op->data.ea);
                    break;
                }
                case OPA_VEX:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        yasm_internal_error(N_("invalid operand conversion"));
                    vexreg = op->data.reg & 0xF;
                    break;
                case OPA_VEXImmSrc:
                    if (op->type != YASM_INSN__OPERAND_REG)
                        yasm_internal_error(N_("invalid operand conversion"));

                    if (!imm) {
                        imm = yasm_expr_create_ident(
                            yasm_expr_int(
                                yasm_intnum_create_uint((op->data.reg << 4)
                                                        & 0xF0)),
                            bc->line);
                    } else {
                        imm = yasm_expr_create(
                            YASM_EXPR_OR,
                            yasm_expr_expr(yasm_expr_create(
                                YASM_EXPR_AND,
                                yasm_expr_expr(imm),
                                yasm_expr_int(yasm_intnum_create_uint(0x0F)),
                                bc->line)),
                            yasm_expr_int(
                                yasm_intnum_create_uint((op->data.reg << 4)
                                                        & 0xF0)),
                            bc->line);
                    }
                    im_len = 8;
                    break;
                case OPA_VEXImm:
                    if (op->type != YASM_INSN__OPERAND_IMM)
                        yasm_internal_error(N_("invalid operand conversion"));

                    if (!imm)
                        imm = op->data.val;
                    else {
                        imm = yasm_expr_create(
                            YASM_EXPR_OR,
                            yasm_expr_expr(yasm_expr_create(
                                YASM_EXPR_AND,
                                yasm_expr_expr(op->data.val),
                                yasm_expr_int(yasm_intnum_create_uint(0x0F)),
                                bc->line)),
                            yasm_expr_expr(yasm_expr_create(
                                YASM_EXPR_AND,
                                yasm_expr_expr(imm),
                                yasm_expr_int(yasm_intnum_create_uint(0xF0)),
                                bc->line)),
                            bc->line);
                    }
                    im_len = 8;
                    break;
                default:
                    yasm_internal_error(N_("unknown operand action"));
            }

            if (info_ops[i].size == OPS_BITS)
                insn->common.opersize = (unsigned char)mode_bits;

            switch (info_ops[i].post_action) {
                case OPAP_None:
                    break;
                case OPAP_SImm8:
                    /* Check operand strictness; if strict and non-8-bit,
                     * pre-emptively expand to full size.
                     * For unspecified size case, still optimize.
                     */
                    if (!(id_insn->force_strict || op->strict)
                        || op->size == 0)
                        insn->postop = X86_POSTOP_SIGNEXT_IMM8;
                    else if (op->size != 8) {
                        insn->opcode.opcode[0] =
                            insn->opcode.opcode[insn->opcode.len];
                        insn->opcode.len = 1;
                    }
                    break;
                case OPAP_ShortMov:
                    do_postop = OPAP_ShortMov;
                    break;
                case OPAP_A16:
                    insn->postop = X86_POSTOP_ADDRESS16;
                    break;
                case OPAP_SImm32Avail:
                    do_postop = OPAP_SImm32Avail;
                    break;
                default:
                    yasm_internal_error(
                        N_("unknown operand postponed action"));
            }
        }
    }

    if (insn->x86_ea) {
        yasm_x86__ea_init(insn->x86_ea, spare, prev_bc);
        for (i=0; i<id_insn->insn.num_segregs; i++)
            yasm_ea_set_segreg(&insn->x86_ea->ea, id_insn->insn.segregs[i]);
    } else if (id_insn->insn.num_segregs > 0 && insn->special_prefix == 0) {
        if (id_insn->insn.num_segregs > 1)
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("multiple segment overrides, using leftmost"));
        insn->special_prefix = (unsigned char)
            (id_insn->insn.segregs[id_insn->insn.num_segregs-1]>>8);
    } else if (id_insn->insn.num_segregs > 0)
        yasm_internal_error(N_("unhandled segment prefix"));

    if (imm) {
        insn->imm = yasm_xmalloc(sizeof(yasm_value));
        if (yasm_value_finalize_expr(insn->imm, imm, prev_bc, im_len))
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("immediate expression too complex"));
        insn->imm->sign = im_sign;
    } else
        insn->imm = NULL;

    yasm_x86__bc_apply_prefixes((x86_common *)insn, &insn->rex,
                                insn->def_opersize_64,
                                id_insn->insn.num_prefixes,
                                id_insn->insn.prefixes);

    if (insn->postop == X86_POSTOP_ADDRESS16 && insn->common.addrsize) {
        yasm_warn_set(YASM_WARN_GENERAL, N_("address size override ignored"));
        insn->common.addrsize = 0;
    }

    /* Handle non-span-dependent post-ops here */
    switch (do_postop) {
        case OPAP_ShortMov:
            /* Long (modrm+sib) mov instructions in amd64 can be optimized into
             * short mov instructions if a 32-bit address override is applied in
             * 64-bit mode to an EA of just an offset (no registers) and the
             * target register is al/ax/eax/rax.
             *
             * We don't want to do this if we're in default rel mode.
             */
            if (!id_insn->default_rel &&
                insn->common.mode_bits == 64 &&
                insn->common.addrsize == 32 &&
                (!insn->x86_ea->ea.disp.abs ||
                 !yasm_expr__contains(insn->x86_ea->ea.disp.abs,
                                      YASM_EXPR_REG))) {
                yasm_x86__ea_set_disponly(insn->x86_ea);
                /* Make the short form permanent. */
                insn->opcode.opcode[0] = insn->opcode.opcode[1];
            }
            insn->opcode.opcode[1] = 0; /* avoid possible confusion */
            break;
        case OPAP_SImm32Avail:
            /* Used for 64-bit mov immediate, which can take a sign-extended
             * imm32 as well as imm64 values.  The imm32 form is put in the
             * second byte of the opcode and its ModRM byte is put in the third
             * byte of the opcode.
             */
            if (!insn->imm->abs ||
                (yasm_expr_get_intnum(&insn->imm->abs, 0) &&
                 yasm_intnum_check_size(
                    yasm_expr_get_intnum(&insn->imm->abs, 0), 32, 0, 1))) {
                /* Throwaway REX byte */
                unsigned char rex_temp = 0;

                /* Build ModRM EA - CAUTION: this depends on
                 * opcode 0 being a mov instruction!
                 */
                insn->x86_ea = yasm_x86__ea_create_reg(insn->x86_ea,
                    (unsigned long)insn->opcode.opcode[0]-0xB8, &rex_temp, 64);

                /* Make the imm32s form permanent. */
                insn->opcode.opcode[0] = insn->opcode.opcode[1];
                insn->imm->size = 32;
            }
            insn->opcode.opcode[1] = 0; /* avoid possible confusion */
            break;
        default:
            break;
    }

    /* Convert to VEX/XOP prefixes if requested.
     * To save space in the insn structure, the VEX/XOP prefix is written into
     * special_prefix and the first 2 bytes of the instruction are set to
     * the second two VEX/XOP bytes.  During calc_len() it may be shortened to
     * one VEX byte (this can only be done after knowledge of REX value); this
     * further optimization is not possible for XOP.
     */
    if (vexdata) {
        int xop = ((vexdata & 0xF0) == 0x80);
        unsigned char vex1 = 0xE0;  /* R=X=B=1, mmmmm=0 */
        unsigned char vex2;

        if (xop) {
            /* Look at the first bytes of the opcode for the XOP mmmmm field.
             * Leave R=X=B=1 for now.
             */
            if (insn->opcode.opcode[0] != 0x08 &&
                insn->opcode.opcode[0] != 0x09)
                yasm_internal_error(N_("first opcode byte of XOP must be 0x08 or 0x09"));
            vex1 |= insn->opcode.opcode[0];
            /* Move opcode byte back one byte to make room for XOP prefix. */
            insn->opcode.opcode[2] = insn->opcode.opcode[1];
        } else {
            /* Look at the first bytes of the opcode to see what leading bytes
             * to encode in the VEX mmmmm field.  Leave R=X=B=1 for now.
             */
            if (insn->opcode.opcode[0] != 0x0F)
                yasm_internal_error(N_("first opcode byte of VEX must be 0x0F"));

            if (insn->opcode.opcode[1] == 0x38)
                vex1 |= 0x02;       /* implied 0x0F 0x38 */
            else if (insn->opcode.opcode[1] == 0x3A)
                vex1 |= 0x03;       /* implied 0x0F 0x3A */
            else {
                /* Originally a 0F-only opcode; move opcode byte back one
                 * position to make room for VEX prefix.
                 */
                insn->opcode.opcode[2] = insn->opcode.opcode[1];
                vex1 |= 0x01;       /* implied 0x0F */
            }
        }

        /* Check for update of special prefix by modifiers */
        if (insn->special_prefix != 0) {
            vexdata &= ~0x03;
            switch (insn->special_prefix) {
                case 0x66:
                    vexdata |= 0x01;
                    break;
                case 0xF3:
                    vexdata |= 0x02;
                    break;
                case 0xF2:
                    vexdata |= 0x03;
                    break;
                default:
                    yasm_internal_error(N_("unrecognized special prefix"));
            }
        }

        /* 2nd VEX byte is WvvvvLpp.
         * W, L, pp come from vexdata
         * vvvv comes from 1s complement of vexreg
         */
        vex2 = (((vexdata & 0x8) << 4) |                /* W */
                ((15 - (vexreg & 0xF)) << 3) |          /* vvvv */
                (vexdata & 0x7));                       /* Lpp */

        /* Save to special_prefix and opcode */
        insn->special_prefix = xop ? 0x8F : 0xC4;   /* VEX/XOP prefix */
        insn->opcode.opcode[0] = vex1;
        insn->opcode.opcode[1] = vex2;
        insn->opcode.len = 3;   /* two prefix bytes and 1 opcode byte */
    }

    x86_id_insn_clear_operands(id_insn);

    /* Transform the bytecode */
    yasm_x86__bc_transform_insn(bc, insn);
}

/* Static parse data structure for instructions */
typedef struct insnprefix_parse_data {
    const char *name;

    /* instruction parse group - NULL if prefix */
    /*@null@*/ const x86_insn_info *group;

    /* For instruction, number of elements in group.
     * For prefix, prefix type shifted right by 8.
     */
    unsigned int num_info:8;

    /* For instruction, GAS suffix flags.
     * For prefix, prefix value.
     */
    unsigned int flags:8;

    /* Instruction modifier data. */
    unsigned int mod_data0:8;
    unsigned int mod_data1:8;
    unsigned int mod_data2:8;

    /* Tests against BITS==64 and AVX */
    unsigned int misc_flags:6;

    /* CPU flags */
    unsigned int cpu0:6;
    unsigned int cpu1:6;
    unsigned int cpu2:6;
} insnprefix_parse_data;

/* Pull in all parse data */
#include "x86insn_nasm.c"
#include "x86insn_gas.c"

static const char *
cpu_find_reverse(unsigned int cpu0, unsigned int cpu1, unsigned int cpu2)
{
    static char cpuname[200];
    wordptr cpu = BitVector_Create(128, TRUE);

    if (cpu0 != CPU_Any)
        BitVector_Bit_On(cpu, cpu0);
    if (cpu1 != CPU_Any)
        BitVector_Bit_On(cpu, cpu1);
    if (cpu2 != CPU_Any)
        BitVector_Bit_On(cpu, cpu2);

    cpuname[0] = '\0';

    if (BitVector_bit_test(cpu, CPU_Prot))
        strcat(cpuname, " Protected");
    if (BitVector_bit_test(cpu, CPU_Undoc))
        strcat(cpuname, " Undocumented");
    if (BitVector_bit_test(cpu, CPU_Obs))
        strcat(cpuname, " Obsolete");
    if (BitVector_bit_test(cpu, CPU_Priv))
        strcat(cpuname, " Privileged");

    if (BitVector_bit_test(cpu, CPU_FPU))
        strcat(cpuname, " FPU");
    if (BitVector_bit_test(cpu, CPU_MMX))
        strcat(cpuname, " MMX");
    if (BitVector_bit_test(cpu, CPU_SSE))
        strcat(cpuname, " SSE");
    if (BitVector_bit_test(cpu, CPU_SSE2))
        strcat(cpuname, " SSE2");
    if (BitVector_bit_test(cpu, CPU_SSE3))
        strcat(cpuname, " SSE3");
    if (BitVector_bit_test(cpu, CPU_3DNow))
        strcat(cpuname, " 3DNow");
    if (BitVector_bit_test(cpu, CPU_Cyrix))
        strcat(cpuname, " Cyrix");
    if (BitVector_bit_test(cpu, CPU_AMD))
        strcat(cpuname, " AMD");
    if (BitVector_bit_test(cpu, CPU_SMM))
        strcat(cpuname, " SMM");
    if (BitVector_bit_test(cpu, CPU_SVM))
        strcat(cpuname, " SVM");
    if (BitVector_bit_test(cpu, CPU_PadLock))
        strcat(cpuname, " PadLock");
    if (BitVector_bit_test(cpu, CPU_EM64T))
        strcat(cpuname, " EM64T");
    if (BitVector_bit_test(cpu, CPU_SSSE3))
        strcat(cpuname, " SSSE3");
    if (BitVector_bit_test(cpu, CPU_SSE41))
        strcat(cpuname, " SSE4.1");
    if (BitVector_bit_test(cpu, CPU_SSE42))
        strcat(cpuname, " SSE4.2");

    if (BitVector_bit_test(cpu, CPU_186))
        strcat(cpuname, " 186");
    if (BitVector_bit_test(cpu, CPU_286))
        strcat(cpuname, " 286");
    if (BitVector_bit_test(cpu, CPU_386))
        strcat(cpuname, " 386");
    if (BitVector_bit_test(cpu, CPU_486))
        strcat(cpuname, " 486");
    if (BitVector_bit_test(cpu, CPU_586))
        strcat(cpuname, " 586");
    if (BitVector_bit_test(cpu, CPU_686))
        strcat(cpuname, " 686");
    if (BitVector_bit_test(cpu, CPU_P3))
        strcat(cpuname, " P3");
    if (BitVector_bit_test(cpu, CPU_P4))
        strcat(cpuname, " P4");
    if (BitVector_bit_test(cpu, CPU_IA64))
        strcat(cpuname, " IA64");
    if (BitVector_bit_test(cpu, CPU_K6))
        strcat(cpuname, " K6");
    if (BitVector_bit_test(cpu, CPU_Athlon))
        strcat(cpuname, " Athlon");
    if (BitVector_bit_test(cpu, CPU_Hammer))
        strcat(cpuname, " Hammer");

    BitVector_Destroy(cpu);
    return cpuname;
}

yasm_arch_insnprefix
yasm_x86__parse_check_insnprefix(yasm_arch *arch, const char *id,
                                 size_t id_len, unsigned long line,
                                 yasm_bytecode **bc, uintptr_t *prefix)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)arch;
    /*@null@*/ const insnprefix_parse_data *pdata;
    size_t i;
    static char lcaseid[17];

    *bc = (yasm_bytecode *)NULL;
    *prefix = 0;

    if (id_len > 16)
        return YASM_ARCH_NOTINSNPREFIX;
    for (i=0; i<id_len; i++)
        lcaseid[i] = tolower(id[i]);
    lcaseid[id_len] = '\0';

    switch (PARSER(arch_x86)) {
        case X86_PARSER_NASM:
            pdata = insnprefix_nasm_find(lcaseid, id_len);
            break;
        case X86_PARSER_TASM:
            pdata = insnprefix_nasm_find(lcaseid, id_len);
            break;
        case X86_PARSER_GAS:
            pdata = insnprefix_gas_find(lcaseid, id_len);
            break;
        default:
            pdata = NULL;
    }
    if (!pdata)
        return YASM_ARCH_NOTINSNPREFIX;

    if (pdata->group) {
        x86_id_insn *id_insn;
        wordptr cpu_enabled = arch_x86->cpu_enables[arch_x86->active_cpu];
        unsigned int cpu0, cpu1, cpu2;

        if (arch_x86->mode_bits != 64 && (pdata->misc_flags & ONLY_64)) {
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("`%s' is an instruction in 64-bit mode"), id);
            return YASM_ARCH_NOTINSNPREFIX;
        }
        if (arch_x86->mode_bits == 64 && (pdata->misc_flags & NOT_64)) {
            yasm_error_set(YASM_ERROR_GENERAL,
                           N_("`%s' invalid in 64-bit mode"), id);
            id_insn = yasm_xmalloc(sizeof(x86_id_insn));
            yasm_insn_initialize(&id_insn->insn);
            id_insn->group = not64_insn;
            id_insn->cpu_enabled = cpu_enabled;
            id_insn->mod_data[0] = 0;
            id_insn->mod_data[1] = 0;
            id_insn->mod_data[2] = 0;
            id_insn->num_info = NELEMS(not64_insn);
            id_insn->mode_bits = arch_x86->mode_bits;
            id_insn->suffix = 0;
            id_insn->misc_flags = 0;
            id_insn->parser = PARSER(arch_x86);
	
            id_insn->force_strict = arch_x86->force_strict != 0;
            id_insn->default_rel = arch_x86->default_rel != 0;
            *bc = yasm_bc_create_common(&x86_id_insn_callback, id_insn, line);
            return YASM_ARCH_INSN;
        }

        cpu0 = pdata->cpu0;
        cpu1 = pdata->cpu1;
        cpu2 = pdata->cpu2;

        if (!BitVector_bit_test(cpu_enabled, cpu0) ||
            !BitVector_bit_test(cpu_enabled, cpu1) ||
            !BitVector_bit_test(cpu_enabled, cpu2)) {
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("`%s' is an instruction in CPU%s"), id,
                          cpu_find_reverse(cpu0, cpu1, cpu2));
            return YASM_ARCH_NOTINSNPREFIX;
        }

        id_insn = yasm_xmalloc(sizeof(x86_id_insn));
        yasm_insn_initialize(&id_insn->insn);
        id_insn->group = pdata->group;
        id_insn->cpu_enabled = cpu_enabled;
        id_insn->mod_data[0] = pdata->mod_data0;
        id_insn->mod_data[1] = pdata->mod_data1;
        id_insn->mod_data[2] = pdata->mod_data2;
        id_insn->num_info = pdata->num_info;
        id_insn->mode_bits = arch_x86->mode_bits;
        id_insn->suffix = pdata->flags;
        id_insn->misc_flags = pdata->misc_flags;
        id_insn->parser = PARSER(arch_x86);
        id_insn->force_strict = arch_x86->force_strict != 0;
        id_insn->default_rel = arch_x86->default_rel != 0;
        *bc = yasm_bc_create_common(&x86_id_insn_callback, id_insn, line);
        return YASM_ARCH_INSN;
    } else {
        unsigned long type = pdata->num_info<<8;
        unsigned long value = pdata->flags;

        if (arch_x86->mode_bits == 64 && type == X86_OPERSIZE && value == 32) {
            yasm_error_set(YASM_ERROR_GENERAL,
                N_("Cannot override data size to 32 bits in 64-bit mode"));
            return YASM_ARCH_NOTINSNPREFIX;
        }

        if (arch_x86->mode_bits == 64 && type == X86_ADDRSIZE && value == 16) {
            yasm_error_set(YASM_ERROR_GENERAL,
                N_("Cannot override address size to 16 bits in 64-bit mode"));
            return YASM_ARCH_NOTINSNPREFIX;
        }

        if (arch_x86->mode_bits != 64 && (pdata->misc_flags & ONLY_64)) {
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("`%s' is a prefix in 64-bit mode"), id);
            return YASM_ARCH_NOTINSNPREFIX;
        }
        *prefix = type|value;
        return YASM_ARCH_PREFIX;
    }
}

static void
x86_id_insn_destroy(void *contents)
{
    x86_id_insn *id_insn = (x86_id_insn *)contents;
    yasm_insn_delete(&id_insn->insn, yasm_x86__ea_destroy);
    yasm_xfree(contents);
}

static void
x86_id_insn_print(const void *contents, FILE *f, int indent_level)
{
    const x86_id_insn *id_insn = (const x86_id_insn *)contents;
    yasm_insn_print(&id_insn->insn, f, indent_level);
    /*TODO*/
}

/*@only@*/ yasm_bytecode *
yasm_x86__create_empty_insn(yasm_arch *arch, unsigned long line)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)arch;
    x86_id_insn *id_insn = yasm_xmalloc(sizeof(x86_id_insn));

    yasm_insn_initialize(&id_insn->insn);
    id_insn->group = empty_insn;
    id_insn->cpu_enabled = arch_x86->cpu_enables[arch_x86->active_cpu];
    id_insn->mod_data[0] = 0;
    id_insn->mod_data[1] = 0;
    id_insn->mod_data[2] = 0;
    id_insn->num_info = NELEMS(empty_insn);
    id_insn->mode_bits = arch_x86->mode_bits;
    id_insn->suffix = (PARSER(arch_x86) == X86_PARSER_GAS) ? SUF_Z : 0;
    id_insn->misc_flags = 0;
    id_insn->parser = PARSER(arch_x86);
    id_insn->force_strict = arch_x86->force_strict != 0;
    id_insn->default_rel = arch_x86->default_rel != 0;

    return yasm_bc_create_common(&x86_id_insn_callback, id_insn, line);
}

