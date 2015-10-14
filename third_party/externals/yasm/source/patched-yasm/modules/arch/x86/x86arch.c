/*
 * x86 architecture description
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
#include <util.h>

#include <libyasm.h>

#include "x86arch.h"


yasm_arch_module yasm_x86_LTX_arch;


static /*@only@*/ yasm_arch *
x86_create(const char *machine, const char *parser,
           /*@out@*/ yasm_arch_create_error *error)
{
    yasm_arch_x86 *arch_x86;
    unsigned int amd64_machine;

    *error = YASM_ARCH_CREATE_OK;

    if (yasm__strcasecmp(machine, "x86") == 0)
        amd64_machine = 0;
    else if (yasm__strcasecmp(machine, "amd64") == 0)
        amd64_machine = 1;
    else {
        *error = YASM_ARCH_CREATE_BAD_MACHINE;
        return NULL;
    }

    arch_x86 = yasm_xmalloc(sizeof(yasm_arch_x86));

    arch_x86->arch.module = &yasm_x86_LTX_arch;

    /* default to all instructions/features enabled */
    arch_x86->active_cpu = 0;
    arch_x86->cpu_enables_size = 1;
    arch_x86->cpu_enables = yasm_xmalloc(sizeof(wordptr));
    arch_x86->cpu_enables[0] = BitVector_Create(64, FALSE);
    BitVector_Fill(arch_x86->cpu_enables[0]);

    arch_x86->amd64_machine = amd64_machine;
    arch_x86->mode_bits = 0;
    arch_x86->force_strict = 0;
    arch_x86->default_rel = 0;
    arch_x86->gas_intel_mode = 0;
    arch_x86->nop = X86_NOP_BASIC;

    if (yasm__strcasecmp(parser, "nasm") == 0)
        arch_x86->parser = X86_PARSER_NASM;
    else if (yasm__strcasecmp(parser, "tasm") == 0)
        arch_x86->parser = X86_PARSER_TASM;
    else if (yasm__strcasecmp(parser, "gas") == 0
             || yasm__strcasecmp(parser, "gnu") == 0)
        arch_x86->parser = X86_PARSER_GAS;
    else {
        yasm_xfree(arch_x86);
        *error = YASM_ARCH_CREATE_BAD_PARSER;
        return NULL;
    }

    return (yasm_arch *)arch_x86;
}

static void
x86_destroy(/*@only@*/ yasm_arch *arch)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)arch;
    unsigned int i;
    for (i=0; i<arch_x86->cpu_enables_size; i++)
        BitVector_Destroy(arch_x86->cpu_enables[i]);
    yasm_xfree(arch_x86->cpu_enables);
    yasm_xfree(arch);
}

static const char *
x86_get_machine(const yasm_arch *arch)
{
    const yasm_arch_x86 *arch_x86 = (const yasm_arch_x86 *)arch;
    if (arch_x86->amd64_machine)
        return "amd64";
    else
        return "x86";
}

static unsigned int
x86_get_address_size(const yasm_arch *arch)
{
    const yasm_arch_x86 *arch_x86 = (const yasm_arch_x86 *)arch;
    if (arch_x86->mode_bits != 0)
        return arch_x86->mode_bits;
    if (arch_x86->amd64_machine)
        return 64;
    else
        return 32;
}

static int
x86_set_var(yasm_arch *arch, const char *var, unsigned long val)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)arch;
    if (yasm__strcasecmp(var, "mode_bits") == 0)
        arch_x86->mode_bits = (unsigned int)val;
    else if (yasm__strcasecmp(var, "force_strict") == 0)
        arch_x86->force_strict = (unsigned int)val;
    else if (yasm__strcasecmp(var, "default_rel") == 0) {
        if (arch_x86->mode_bits != 64)
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("ignoring default rel in non-64-bit mode"));
        else
            arch_x86->default_rel = (unsigned int)val;
    } else if (yasm__strcasecmp(var, "gas_intel_mode") == 0) {
        arch_x86->gas_intel_mode = (unsigned int)val;
    } else
        return 1;
    return 0;
}

static void
x86_dir_cpu(yasm_object *object, yasm_valparamhead *valparams,
            yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)object->arch;

    yasm_valparam *vp;
    yasm_vps_foreach(vp, valparams) {
        /*@null@*/ /*@dependent@*/ const char *s = yasm_vp_string(vp);
        if (s)
            yasm_x86__parse_cpu(arch_x86, s, strlen(s));
        else if (vp->type == YASM_PARAM_EXPR) {
            const yasm_intnum *intcpu;
            intcpu = yasm_expr_get_intnum(&vp->param.e, 0);
            if (!intcpu)
                yasm_error_set(YASM_ERROR_SYNTAX,
                               N_("invalid argument to [%s]"), "CPU");
            else {
                char strcpu[16];
                sprintf(strcpu, "%lu", yasm_intnum_get_uint(intcpu));
                yasm_x86__parse_cpu(arch_x86, strcpu, strlen(strcpu));
            }
        } else
            yasm_error_set(YASM_ERROR_SYNTAX, N_("invalid argument to [%s]"),
                           "CPU");
    }
}

static void
x86_dir_bits(yasm_object *object, yasm_valparamhead *valparams,
             yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)object->arch;
    yasm_valparam *vp;
    /*@only@*/ /*@null@*/ yasm_expr *e = NULL;
    const yasm_intnum *intn;
    long lval;

    if ((vp = yasm_vps_first(valparams)) && !vp->val &&
        (e = yasm_vp_expr(vp, object->symtab, line)) != NULL &&
        (intn = yasm_expr_get_intnum(&e, 0)) != NULL &&
        (lval = yasm_intnum_get_int(intn)) &&
        (lval == 16 || lval == 32 || lval == 64))
        arch_x86->mode_bits = (unsigned char)lval;
    else
        yasm_error_set(YASM_ERROR_VALUE, N_("invalid argument to [%s]"),
                       "BITS");
    if (e)
        yasm_expr_destroy(e);
}

static void
x86_dir_code16(yasm_object *object, yasm_valparamhead *valparams,
               yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)object->arch;
    arch_x86->mode_bits = 16;
}

static void
x86_dir_code32(yasm_object *object, yasm_valparamhead *valparams,
               yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)object->arch;
    arch_x86->mode_bits = 32;
}

static void
x86_dir_code64(yasm_object *object, yasm_valparamhead *valparams,
               yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)object->arch;
    arch_x86->mode_bits = 64;
}

static const unsigned char **
x86_get_fill(const yasm_arch *arch)
{
    const yasm_arch_x86 *arch_x86 = (const yasm_arch_x86 *)arch;

    /* Fill patterns that GAS uses. */
    static const unsigned char fill16_1[1] =
        {0x90};                                 /* 1 - nop */
    static const unsigned char fill16_2[2] =
        {0x89, 0xf6};                           /* 2 - mov si, si */
    static const unsigned char fill16_3[3] =
        {0x8d, 0x74, 0x00};                     /* 3 - lea si, [si+byte 0] */
    static const unsigned char fill16_4[4] =
        {0x8d, 0xb4, 0x00, 0x00};               /* 4 - lea si, [si+word 0] */
    static const unsigned char fill16_5[5] =
        {0x90,                                  /* 5 - nop */
         0x8d, 0xb4, 0x00, 0x00};               /*     lea si, [si+word 0] */
    static const unsigned char fill16_6[6] =
        {0x89, 0xf6,                            /* 6 - mov si, si */
         0x8d, 0xbd, 0x00, 0x00};               /*     lea di, [di+word 0] */
    static const unsigned char fill16_7[7] =
        {0x8d, 0x74, 0x00,                      /* 7 - lea si, [si+byte 0] */
         0x8d, 0xbd, 0x00, 0x00};               /*     lea di, [di+word 0] */
    static const unsigned char fill16_8[8] =
        {0x8d, 0xb4, 0x00, 0x00,                /* 8 - lea si, [si+word 0] */
         0x8d, 0xbd, 0x00, 0x00};               /*     lea di, [di+word 0] */
    static const unsigned char fill16_9[9] =
        {0xeb, 0x07, 0x90, 0x90, 0x90, 0x90,    /* 9 - jmp $+9; nop fill */
         0x90, 0x90, 0x90};
    static const unsigned char fill16_10[10] =
        {0xeb, 0x08, 0x90, 0x90, 0x90, 0x90,    /* 10 - jmp $+10; nop fill */
         0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill16_11[11] =
        {0xeb, 0x09, 0x90, 0x90, 0x90, 0x90,    /* 11 - jmp $+11; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill16_12[12] =
        {0xeb, 0x0a, 0x90, 0x90, 0x90, 0x90,    /* 12 - jmp $+12; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill16_13[13] =
        {0xeb, 0x0b, 0x90, 0x90, 0x90, 0x90,    /* 13 - jmp $+13; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill16_14[14] =
        {0xeb, 0x0c, 0x90, 0x90, 0x90, 0x90,    /* 14 - jmp $+14; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill16_15[15] =
        {0xeb, 0x0d, 0x90, 0x90, 0x90, 0x90,    /* 15 - jmp $+15; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char *fill16[16] =
    {
        NULL,      fill16_1,  fill16_2,  fill16_3,
        fill16_4,  fill16_5,  fill16_6,  fill16_7,
        fill16_8,  fill16_9,  fill16_10, fill16_11,
        fill16_12, fill16_13, fill16_14, fill16_15
    };

    static const unsigned char fill32_1[1] =
        {0x90};                              /* 1 - nop */
    static const unsigned char fill32_2[2] =
        {0x66, 0x90};                        /* 2 - xchg ax, ax (o16 nop) */
    static const unsigned char fill32_3[3] =
        {0x8d, 0x76, 0x00};                  /* 3 - lea esi, [esi+byte 0] */
    static const unsigned char fill32_4[4] =
        {0x8d, 0x74, 0x26, 0x00};            /* 4 - lea esi, [esi*1+byte 0] */
    static const unsigned char fill32_5[5] =
        {0x90,                               /* 5 - nop */
         0x8d, 0x74, 0x26, 0x00};            /*     lea esi, [esi*1+byte 0] */
    static const unsigned char fill32_6[6] =
        {0x8d, 0xb6, 0x00, 0x00, 0x00, 0x00};/* 6 - lea esi, [esi+dword 0] */
    static const unsigned char fill32_7[7] =
        {0x8d, 0xb4, 0x26, 0x00, 0x00, 0x00, /* 7 - lea esi, [esi*1+dword 0] */
         0x00};
    static const unsigned char fill32_8[8] =
        {0x90,                               /* 8 - nop */
         0x8d, 0xb4, 0x26, 0x00, 0x00, 0x00, /*     lea esi, [esi*1+dword 0] */
         0x00};
#if 0
    /* GAS uses these */
    static const unsigned char fill32_9[9] =
        {0x89, 0xf6,                         /* 9 - mov esi, esi */
         0x8d, 0xbc, 0x27, 0x00, 0x00, 0x00, /*     lea edi, [edi*1+dword 0] */
         0x00};
    static const unsigned char fill32_10[10] =
        {0x8d, 0x76, 0x00,                   /* 10 - lea esi, [esi+byte 0] */
         0x8d, 0xbc, 0x27, 0x00, 0x00, 0x00, /*      lea edi, [edi+dword 0] */
         0x00};
    static const unsigned char fill32_11[11] =
        {0x8d, 0x74, 0x26, 0x00,             /* 11 - lea esi, [esi*1+byte 0] */
         0x8d, 0xbc, 0x27, 0x00, 0x00, 0x00, /*      lea edi, [edi*1+dword 0] */
         0x00};
    static const unsigned char fill32_12[12] =
        {0x8d, 0xb6, 0x00, 0x00, 0x00, 0x00, /* 12 - lea esi, [esi+dword 0] */
         0x8d, 0xbf, 0x00, 0x00, 0x00, 0x00};/*      lea edi, [edi+dword 0] */
    static const unsigned char fill32_13[13] =
        {0x8d, 0xb6, 0x00, 0x00, 0x00, 0x00, /* 13 - lea esi, [esi+dword 0] */
         0x8d, 0xbc, 0x27, 0x00, 0x00, 0x00, /*      lea edi, [edi*1+dword 0] */
         0x00};
    static const unsigned char fill32_14[14] =
        {0x8d, 0xb4, 0x26, 0x00, 0x00, 0x00, /* 14 - lea esi, [esi*1+dword 0] */
         0x00,
         0x8d, 0xbc, 0x27, 0x00, 0x00, 0x00, /*      lea edi, [edi*1+dword 0] */
         0x00};
#else
    /* But on newer processors, these are recommended */
    static const unsigned char fill32_9[9] =
        {0xeb, 0x07, 0x90, 0x90, 0x90, 0x90, /* 9 - jmp $+9; nop fill */
         0x90, 0x90, 0x90};
    static const unsigned char fill32_10[10] =
        {0xeb, 0x08, 0x90, 0x90, 0x90, 0x90, /* 10 - jmp $+10; nop fill */
         0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill32_11[11] =
        {0xeb, 0x09, 0x90, 0x90, 0x90, 0x90, /* 11 - jmp $+11; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill32_12[12] =
        {0xeb, 0x0a, 0x90, 0x90, 0x90, 0x90, /* 12 - jmp $+12; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill32_13[13] =
        {0xeb, 0x0b, 0x90, 0x90, 0x90, 0x90, /* 13 - jmp $+13; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char fill32_14[14] =
        {0xeb, 0x0c, 0x90, 0x90, 0x90, 0x90, /* 14 - jmp $+14; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
#endif
    static const unsigned char fill32_15[15] =
        {0xeb, 0x0d, 0x90, 0x90, 0x90, 0x90, /* 15 - jmp $+15; nop fill */
         0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
    static const unsigned char *fill32[16] =
    {
        NULL,      fill32_1,  fill32_2,  fill32_3,
        fill32_4,  fill32_5,  fill32_6,  fill32_7,
        fill32_8,  fill32_9,  fill32_10, fill32_11,
        fill32_12, fill32_13, fill32_14, fill32_15
    };

    /* Long form nops available on more recent Intel and AMD processors */
    static const unsigned char fill32new_3[3] =
        {0x0f, 0x1f, 0x00};                         /* 3 - nop(3) */
    static const unsigned char fill32new_4[4] =
        {0x0f, 0x1f, 0x40, 0x00};                   /* 4 - nop(4) */
    static const unsigned char fill32new_5[5] =
        {0x0f, 0x1f, 0x44, 0x00, 0x00};             /* 5 - nop(5) */
    static const unsigned char fill32new_6[6] =
        {0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00};       /* 6 - nop(6) */
    static const unsigned char fill32new_7[7] =
        {0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00}; /* 7 - nop(7) */
    static const unsigned char fill32new_8[8] =
        {0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00,  /* 8 - nop(8) */
         0x00};
    static const unsigned char fill32new_9[9] =
        {0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,  /* 9 - nop(9) */
         0x00, 0x00};

    /* Longer forms preferred by Intel use repeated o16 prefixes */
    static const unsigned char fill32intel_10[10] =
        {0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00,  /* 10 - o16; cs; nop */
         0x00, 0x00, 0x00};
    static const unsigned char fill32intel_11[11] =
        {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00,  /* 11 - 2x o16; cs; nop */
         0x00, 0x00, 0x00, 0x00};
    static const unsigned char fill32intel_12[12] =
        {0x66, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84,  /* 12 - 3x o16; cs; nop */
         0x00, 0x00, 0x00, 0x00, 0x00};
    static const unsigned char fill32intel_13[13] =
        {0x66, 0x66, 0x66, 0x66, 0x2e, 0x0f, 0x1f,  /* 13 - 4x o16; cs; nop */
         0x84, 0x00, 0x00, 0x00, 0x00, 0x00};
    static const unsigned char fill32intel_14[14] =
        {0x66, 0x66, 0x66, 0x66, 0x66, 0x2e, 0x0f,  /* 14 - 5x o16; cs; nop */
         0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00};
    static const unsigned char fill32intel_15[15] =
        {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x2e,  /* 15 - 6x o16; cs; nop */
         0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* Longer forms preferred by AMD use fewer o16 prefixes and no CS prefix;
     * Source: Software Optimisation Guide for AMD Family 10h
     * Processors 40546 revision 3.10 February 2009
     */
    static const unsigned char fill32amd_10[10] =
        {0x66, 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00,  /* 10 - nop(10) */
         0x00, 0x00, 0x00};
    static const unsigned char fill32amd_11[11] =
        {0x0f, 0x1f, 0x44, 0x00, 0x00,              /* 11 - nop(5) */
         0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00};       /*      nop(6) */
    static const unsigned char fill32amd_12[12] =
        {0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00,        /* 12 - nop(6) */
         0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00};       /*      nop(6) */
    static const unsigned char fill32amd_13[13] =
        {0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00,        /* 13 - nop(6) */
         0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00}; /*      nop(7) */
    static const unsigned char fill32amd_14[14] =
        {0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,  /* 14 - nop(7) */
         0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00}; /*      nop(7) */
    static const unsigned char fill32amd_15[15] =
        {0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00,        /* 15 - nop(7) */
         0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00}; /*      nop(8) */

    static const unsigned char *fill32_intel[16] =
    {
        NULL,           fill32_1,       fill32_2,       fill32new_3,
        fill32new_4,    fill32new_5,    fill32new_6,    fill32new_7,
        fill32new_8,    fill32new_9,    fill32intel_10, fill32intel_11,
        fill32intel_12, fill32intel_13, fill32intel_14, fill32intel_15
    };
    static const unsigned char *fill32_amd[16] =
    {
        NULL,           fill32_1,       fill32_2,       fill32new_3,
        fill32new_4,    fill32new_5,    fill32new_6,    fill32new_7,
        fill32new_8,    fill32new_9,    fill32amd_10,   fill32amd_11,
        fill32amd_12,   fill32amd_13,   fill32amd_14,   fill32amd_15
    };

    switch (arch_x86->mode_bits) {
        case 16:
            return fill16;
        case 32:
            if (arch_x86->nop == X86_NOP_INTEL)
                return fill32_intel;
            else if (arch_x86->nop == X86_NOP_AMD)
                return fill32_amd;
            else
                return fill32;
        case 64:
            /* We know long nops are available in 64-bit mode; default to Intel
             * ones if unspecified (to match GAS behavior).
             */
            if (arch_x86->nop == X86_NOP_AMD)
                return fill32_amd;
            else
                return fill32_intel;
        default:
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("Invalid mode_bits in x86_get_fill"));
            return NULL;
    }
}

unsigned int
yasm_x86__get_reg_size(uintptr_t reg)
{
    switch ((x86_expritem_reg_size)(reg & ~0xFUL)) {
        case X86_REG8:
        case X86_REG8X:
            return 8;
        case X86_REG16:
            return 16;
        case X86_REG32:
        case X86_CRREG:
        case X86_DRREG:
        case X86_TRREG:
            return 32;
        case X86_REG64:
        case X86_MMXREG:
            return 64;
        case X86_XMMREG:
            return 128;
        case X86_YMMREG:
            return 256;
        case X86_FPUREG:
            return 80;
        default:
            yasm_error_set(YASM_ERROR_VALUE, N_("unknown register size"));
    }
    return 0;
}

static unsigned int
x86_get_reg_size(yasm_arch *arch, uintptr_t reg)
{
    return yasm_x86__get_reg_size(reg);
}

static uintptr_t
x86_reggroup_get_reg(yasm_arch *arch, uintptr_t reggroup,
                     unsigned long regindex)
{
    yasm_arch_x86 *arch_x86 = (yasm_arch_x86 *)arch;
    switch ((x86_expritem_reg_size)(reggroup & ~0xFUL)) {
        case X86_XMMREG:
        case X86_YMMREG:
            if (arch_x86->mode_bits == 64) {
                if (regindex > 15)
                    return 0;
                return reggroup | (regindex & 15);
            }
            /*@fallthrough@*/
        case X86_MMXREG:
        case X86_FPUREG:
            if (regindex > 7)
                return 0;
            return reggroup | (regindex & 7);
        default:
            yasm_error_set(YASM_ERROR_VALUE, N_("bad register group"));
    }
    return 0;
}

static void
x86_reg_print(yasm_arch *arch, uintptr_t reg, FILE *f)
{
    static const char *name8[] = {"al","cl","dl","bl","ah","ch","dh","bh"};
    static const char *name8x[] = {
        "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil",
        "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
    };
    static const char *name16[] = {
        "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
        "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
    };
    static const char *name32[] = {
        "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
        "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
    };
    static const char *name64[] = {
        "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    };

    switch ((x86_expritem_reg_size)(reg & ~0xFUL)) {
        case X86_REG8:
            fprintf(f, "%s", name8[reg&0xF]);
            break;
        case X86_REG8X:
            fprintf(f, "%s", name8x[reg&0xF]);
            break;
        case X86_REG16:
            fprintf(f, "%s", name16[reg&0xF]);
            break;
        case X86_REG32:
            fprintf(f, "%s", name32[reg&0xF]);
            break;
        case X86_REG64:
            fprintf(f, "%s", name64[reg&0xF]);
            break;
        case X86_MMXREG:
            fprintf(f, "mm%d", (int)(reg&0xF));
            break;
        case X86_XMMREG:
            fprintf(f, "xmm%d", (int)(reg&0xF));
            break;
        case X86_YMMREG:
            fprintf(f, "ymm%d", (int)(reg&0xF));
            break;
        case X86_CRREG:
            fprintf(f, "cr%d", (int)(reg&0xF));
            break;
        case X86_DRREG:
            fprintf(f, "dr%d", (int)(reg&0xF));
            break;
        case X86_TRREG:
            fprintf(f, "tr%d", (int)(reg&0xF));
            break;
        case X86_FPUREG:
            fprintf(f, "st%d", (int)(reg&0xF));
            break;
        default:
            yasm_error_set(YASM_ERROR_VALUE, N_("unknown register size"));
    }
}

static void
x86_segreg_print(yasm_arch *arch, uintptr_t segreg, FILE *f)
{
    static const char *name[] = {"es","cs","ss","ds","fs","gs"};
    fprintf(f, "%s", name[segreg&7]);
}

/* Define x86 machines -- see arch.h for details */
static const yasm_arch_machine x86_machines[] = {
    { "IA-32 and derivatives", "x86" },
    { "AMD64", "amd64" },
    { NULL, NULL }
};

static const yasm_directive x86_directives[] = {
    { "cpu",            "nasm", x86_dir_cpu,    YASM_DIR_ARG_REQUIRED },
    { "bits",           "nasm", x86_dir_bits,   YASM_DIR_ARG_REQUIRED },
    { ".code16",        "gas",  x86_dir_code16, YASM_DIR_ANY },
    { ".code32",        "gas",  x86_dir_code32, YASM_DIR_ANY },
    { ".code64",        "gas",  x86_dir_code64, YASM_DIR_ANY },
    { NULL, NULL, NULL, 0 }
};

/* Define arch structure -- see arch.h for details */
yasm_arch_module yasm_x86_LTX_arch = {
    "x86 (IA-32 and derivatives), AMD64",
    "x86",
    x86_directives,
    x86_create,
    x86_destroy,
    x86_get_machine,
    x86_get_address_size,
    x86_set_var,
    yasm_x86__parse_check_insnprefix,
    yasm_x86__parse_check_regtmod,
    x86_get_fill,
    yasm_x86__floatnum_tobytes,
    yasm_x86__intnum_tobytes,
    x86_get_reg_size,
    x86_reggroup_get_reg,
    x86_reg_print,
    x86_segreg_print,
    yasm_x86__ea_create_expr,
    yasm_x86__ea_destroy,
    yasm_x86__ea_print,
    yasm_x86__create_empty_insn,
    x86_machines,
    "x86",
    16,
    1
};
