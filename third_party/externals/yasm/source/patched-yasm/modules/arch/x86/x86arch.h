/*
 * x86 Architecture header file
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
#ifndef YASM_X86ARCH_H
#define YASM_X86ARCH_H

#include <libyasm/bitvect.h>

/* Available CPU feature flags */
#define CPU_Any     0       /* Any old cpu will do */
#define CPU_086     CPU_Any
#define CPU_186     1       /* i186 or better required */
#define CPU_286     2       /* i286 or better required */
#define CPU_386     3       /* i386 or better required */
#define CPU_486     4       /* i486 or better required */
#define CPU_586     5       /* i585 or better required */
#define CPU_686     6       /* i686 or better required */
#define CPU_P3      7       /* Pentium3 or better required */
#define CPU_P4      8       /* Pentium4 or better required */
#define CPU_IA64    9       /* IA-64 or better required */
#define CPU_K6      10      /* AMD K6 or better required */
#define CPU_Athlon  11      /* AMD Athlon or better required */
#define CPU_Hammer  12      /* AMD Sledgehammer or better required */
#define CPU_FPU     13      /* FPU support required */
#define CPU_MMX     14      /* MMX support required */
#define CPU_SSE     15      /* Streaming SIMD extensions required */
#define CPU_SSE2    16      /* Streaming SIMD extensions 2 required */
#define CPU_SSE3    17      /* Streaming SIMD extensions 3 required */
#define CPU_3DNow   18      /* 3DNow! support required */
#define CPU_Cyrix   19      /* Cyrix-specific instruction */
#define CPU_AMD     20      /* AMD-specific inst. (older than K6) */
#define CPU_SMM     21      /* System Management Mode instruction */
#define CPU_Prot    22      /* Protected mode only instruction */
#define CPU_Undoc   23      /* Undocumented instruction */
#define CPU_Obs     24      /* Obsolete instruction */
#define CPU_Priv    25      /* Priveleged instruction */
#define CPU_SVM     26      /* Secure Virtual Machine instruction */
#define CPU_PadLock 27      /* VIA PadLock instruction */
#define CPU_EM64T   28      /* Intel EM64T or better */
#define CPU_SSSE3   29      /* Streaming SIMD extensions 3 required */
#define CPU_SSE41   30      /* Streaming SIMD extensions 4.1 required */
#define CPU_SSE42   31      /* Streaming SIMD extensions 4.2 required */
#define CPU_SSE4a   32      /* AMD Streaming SIMD extensions 4a required */
#define CPU_XSAVE   33      /* Intel XSAVE instructions */
#define CPU_AVX     34      /* Intel Advanced Vector Extensions */
#define CPU_FMA     35      /* Intel Fused-Multiply-Add Extensions */
#define CPU_AES     36      /* AES instruction */
#define CPU_CLMUL   37      /* PCLMULQDQ instruction */
#define CPU_MOVBE   38      /* MOVBE instruction */
#define CPU_XOP     39      /* AMD XOP extensions */
#define CPU_FMA4    40      /* AMD Fused-Multiply-Add extensions */
#define CPU_F16C    41      /* Intel float-16 instructions */
#define CPU_FSGSBASE 42     /* Intel FSGSBASE instructions */
#define CPU_RDRAND  43      /* Intel RDRAND instruction */
#define CPU_XSAVEOPT 44     /* Intel XSAVEOPT instruction */
#define CPU_EPTVPID 45      /* Intel INVEPT, INVVPID instructions */
#define CPU_SMX     46      /* Intel SMX instruction (GETSEC) */
#define CPU_AVX2    47      /* Intel AVX2 instructions */
#define CPU_BMI1    48      /* Intel BMI1 instructions */
#define CPU_BMI2    49      /* Intel BMI2 instructions */
#define CPU_INVPCID 50      /* Intel INVPCID instruction */
#define CPU_LZCNT   51      /* Intel LZCNT instruction */

enum x86_parser_type {
    X86_PARSER_NASM = 0,
    X86_PARSER_TASM = 1,
    X86_PARSER_GAS = 2
};

#define PARSER(arch) (((arch)->parser == X86_PARSER_GAS && (arch)->gas_intel_mode) ? X86_PARSER_NASM : (arch)->parser)

typedef struct yasm_arch_x86 {
    yasm_arch_base arch;        /* base structure */

    /* What instructions/features are enabled? */
    unsigned int active_cpu;        /* active index into cpu_enables table */
    unsigned int cpu_enables_size;  /* size of cpu_enables table */
    wordptr *cpu_enables;

    unsigned int amd64_machine;
    enum x86_parser_type parser;
    unsigned int mode_bits;
    unsigned int force_strict;
    unsigned int default_rel;
    unsigned int gas_intel_mode;

    enum {
        X86_NOP_BASIC = 0,
        X86_NOP_INTEL = 1,
        X86_NOP_AMD = 2
    } nop;
} yasm_arch_x86;

/* 0-15 (low 4 bits) used for register number, stored in same data area.
 * Note 8-15 are only valid for some registers, and only in 64-bit mode.
 */
typedef enum {
    X86_REG8 = 0x1<<4,
    X86_REG8X = 0x2<<4,     /* 64-bit mode only, REX prefix version of REG8 */
    X86_REG16 = 0x3<<4,
    X86_REG32 = 0x4<<4,
    X86_REG64 = 0x5<<4,     /* 64-bit mode only */
    X86_FPUREG = 0x6<<4,
    X86_MMXREG = 0x7<<4,
    X86_XMMREG = 0x8<<4,
    X86_YMMREG = 0x9<<4,
    X86_CRREG = 0xA<<4,
    X86_DRREG = 0xB<<4,
    X86_TRREG = 0xC<<4,
    X86_RIP = 0xD<<4        /* 64-bit mode only, always RIP (regnum ignored) */
} x86_expritem_reg_size;

/* Low 8 bits are used for the prefix value, stored in same data area. */
typedef enum {
    X86_LOCKREP = 1<<8,
    X86_ADDRSIZE = 2<<8,
    X86_OPERSIZE = 3<<8,
    X86_SEGREG = 4<<8,
    X86_REX = 5<<8
} x86_parse_insn_prefix;

typedef enum {
    X86_NEAR = 1,
    X86_SHORT,
    X86_FAR,
    X86_TO
} x86_parse_targetmod;

typedef enum {
    JMP_NONE,
    JMP_SHORT,
    JMP_NEAR,
    JMP_SHORT_FORCED,
    JMP_NEAR_FORCED
} x86_jmp_opcode_sel;

typedef enum {
    X86_REX_W = 3,
    X86_REX_R = 2,
    X86_REX_X = 1,
    X86_REX_B = 0
} x86_rex_bit_pos;

/* Sets REX (4th bit) and 3 LS bits from register size/number.  Returns 1 if
 * impossible to fit reg into REX, otherwise returns 0.  Input parameter rexbit
 * indicates bit of REX to use if REX is needed.  Will not modify REX if not
 * in 64-bit mode or if it wasn't needed to express reg.
 */
int yasm_x86__set_rex_from_reg(unsigned char *rex, unsigned char *low3,
                               uintptr_t reg, unsigned int bits,
                               x86_rex_bit_pos rexbit);

/* Effective address type */
typedef struct x86_effaddr {
    yasm_effaddr ea;            /* base structure */

    /* VSIB uses the normal SIB byte, but this flag enables it. */
    unsigned char vsib_mode;    /* 0 if not, 1 if XMM, 2 if YMM */

    /* How the spare (register) bits in Mod/RM are handled:
     * Even if valid_modrm=0, the spare bits are still valid (don't overwrite!)
     * They're set in bytecode_create_insn().
     */
    unsigned char modrm;
    unsigned char valid_modrm;  /* 1 if Mod/RM byte currently valid, 0 if not */
    unsigned char need_modrm;   /* 1 if Mod/RM byte needed, 0 if not */

    unsigned char sib;
    unsigned char valid_sib;    /* 1 if SIB byte currently valid, 0 if not */
    unsigned char need_sib;     /* 1 if SIB byte needed, 0 if not,
                                   0xff if unknown */
} x86_effaddr;

void yasm_x86__ea_init(x86_effaddr *x86_ea, unsigned int spare,
                       yasm_bytecode *precbc);

void yasm_x86__ea_set_disponly(x86_effaddr *x86_ea);
x86_effaddr *yasm_x86__ea_create_reg(x86_effaddr *x86_ea, unsigned long reg,
                                     unsigned char *rex, unsigned int bits);
x86_effaddr *yasm_x86__ea_create_imm
    (x86_effaddr *x86_ea, /*@keep@*/ yasm_expr *imm, unsigned int im_len);
yasm_effaddr *yasm_x86__ea_create_expr(yasm_arch *arch,
                                       /*@keep@*/ yasm_expr *e);
void yasm_x86__ea_destroy(yasm_effaddr *ea);
void yasm_x86__ea_print(const yasm_effaddr *ea, FILE *f, int indent_level);

void yasm_x86__bc_insn_opersize_override(yasm_bytecode *bc,
                                         unsigned int opersize);
void yasm_x86__bc_insn_addrsize_override(yasm_bytecode *bc,
                                         unsigned int addrsize);
void yasm_x86__bc_insn_set_lockrep_prefix(yasm_bytecode *bc,
                                          unsigned int prefix);

/* Bytecode types */
typedef struct x86_common {
    unsigned char addrsize;         /* 0 or =mode_bits => no override */
    unsigned char opersize;         /* 0 or =mode_bits => no override */
    unsigned char lockrep_pre;      /* 0 indicates no prefix */

    unsigned char mode_bits;
} x86_common;

typedef struct x86_opcode {
    unsigned char opcode[3];        /* opcode */
    unsigned char len;
} x86_opcode;

typedef struct x86_insn {
    x86_common common;              /* common x86 information */
    x86_opcode opcode;

    /*@null@*/ x86_effaddr *x86_ea; /* effective address */

    /*@null@*/ yasm_value *imm;     /* immediate or relative value */

    unsigned char def_opersize_64;  /* default operand size in 64-bit mode */
    unsigned char special_prefix;   /* "special" prefix (0=none) */

    unsigned char rex;          /* REX AMD64 extension, 0 if none,
                                   0xff if not allowed (high 8 bit reg used) */

    /* Postponed (from parsing to later binding) action options. */
    enum {
        /* None */
        X86_POSTOP_NONE = 0,

        /* Instructions that take a sign-extended imm8 as well as imm values
         * (eg, the arith instructions and a subset of the imul instructions)
         * should set this and put the imm8 form as the "normal" opcode (in
         * the first one or two bytes) and non-imm8 form in the second or
         * third byte of the opcode.
         */
        X86_POSTOP_SIGNEXT_IMM8,

        /* Override any attempt at address-size override to 16 bits, and never
         * generate a prefix.  This is used for the ENTER opcode.
         */
        X86_POSTOP_ADDRESS16
    } postop;
} x86_insn;

typedef struct x86_jmp {
    x86_common common;          /* common x86 information */
    x86_opcode shortop, nearop;

    yasm_value target;          /* jump target */

    /* which opcode are we using? */
    /* The *FORCED forms are specified in the source as such */
    x86_jmp_opcode_sel op_sel;
} x86_jmp;

/* Direct (immediate) FAR jumps ONLY; indirect FAR jumps get turned into
 * x86_insn bytecodes; relative jumps turn into x86_jmp bytecodes.
 * This bytecode is not legal in 64-bit mode.
 */
typedef struct x86_jmpfar {
    x86_common common;          /* common x86 information */
    x86_opcode opcode;

    yasm_value segment;         /* target segment */
    yasm_value offset;          /* target offset */
} x86_jmpfar;

void yasm_x86__bc_transform_insn(yasm_bytecode *bc, x86_insn *insn);
void yasm_x86__bc_transform_jmp(yasm_bytecode *bc, x86_jmp *jmp);
void yasm_x86__bc_transform_jmpfar(yasm_bytecode *bc, x86_jmpfar *jmpfar);

void yasm_x86__bc_apply_prefixes
    (x86_common *common, unsigned char *rex, unsigned int def_opersize_64,
     unsigned int num_prefixes, uintptr_t *prefixes);

/* Check an effective address.  Returns 0 if EA was successfully determined,
 * 1 if invalid EA, or 2 if indeterminate EA.
 */
int yasm_x86__expr_checkea
    (x86_effaddr *x86_ea, unsigned char *addrsize, unsigned int bits,
     int address16_op, unsigned char *rex, yasm_bytecode *bc);

void yasm_x86__parse_cpu(yasm_arch_x86 *arch_x86, const char *cpuid,
                         size_t cpuid_len);

yasm_arch_insnprefix yasm_x86__parse_check_insnprefix
    (yasm_arch *arch, const char *id, size_t id_len, unsigned long line,
     /*@out@*/ yasm_bytecode **bc, /*@out@*/ uintptr_t *prefix);
yasm_arch_regtmod yasm_x86__parse_check_regtmod
    (yasm_arch *arch, const char *id, size_t id_len,
     /*@out@*/ uintptr_t *data);

int yasm_x86__floatnum_tobytes
    (yasm_arch *arch, const yasm_floatnum *flt, unsigned char *buf,
     size_t destsize, size_t valsize, size_t shift, int warn);
int yasm_x86__intnum_tobytes
    (yasm_arch *arch, const yasm_intnum *intn, unsigned char *buf,
     size_t destsize, size_t valsize, int shift, const yasm_bytecode *bc,
     int warn);

unsigned int yasm_x86__get_reg_size(uintptr_t reg);

/*@only@*/ yasm_bytecode *yasm_x86__create_empty_insn(yasm_arch *arch,
                                                      unsigned long line);
#endif
