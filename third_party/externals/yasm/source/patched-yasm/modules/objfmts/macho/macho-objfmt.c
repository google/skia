/*
 * Mac OS X ABI Mach-O File Format 
 *
 *  Copyright (C) 2007 Henryk Richter, built upon xdf objfmt (C) Peter Johnson
 *   
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
/*
  notes: This implementation is rather basic. There are several implementation
         issues to be sorted out for full compliance and error resilience.
         Some examples are given below (nasm syntax).

  1) section placement
     Mach-O requires BSS sections to be placed last in object files. This
     has to be done manually. 
     Example:

      section .text
       mov rax,[qword foo]
      section .data
       dw  0
      section .bss
      foo dw 0

  2) addressing issues

  2.1) symbol relative relocation (i.e. mov eax,[foo wrt bar])
       Not implemented yet.

  2.2) data referencing in 64 bit mode
       While ELF allows 32 bit absolute relocations in 64 bit mode, Mach-O
       does not. Therefore code like
        lea rbx,[_foo]  ;48 8d 1c 25 00 00 00 00
        mov rcx,[_bar]  ;48 8b 0c 25 00 00 00 00
       with a 32 bit address field cannot be relocated into an address >= 0x100000000 (OSX actually
       uses that). 
       
       Actually, the only register where a 64 bit displacement is allowed in x86-64, is rax
       as in the example 1).

       A plausible workaround is either classic PIC (like in C), which is in turn
       not implemented in this object format. The recommended was is PC relative 
       code (called RIP-relative in x86-64). So instead of the lines above, just write:
        lea rbx,[_foo wrt rip]
        mov rcx,[_bar wrt rip]

  2.3) section/data alignment
       Normally, you specify sections with a specific alignment
       and get your data layed out as desired. Unfortunately, the
       linker in MacOS X seems to ignore the section alignment requests.
       The workaround is an explicit alignment at the end of the text section.

       section .text
        movdqa xmm0,[_foo wrt rip]

        align 16
       section .data align=16
        _foo dw 32,32,32,32,32,32,32,32

       FIXME: perform that operation implicitly! 

  2.4) cross section symbol differences unsupported in current implementation
       [extern foo]
       [extern bar]
       section .data
        dq bar-foo

       Will currently produce an error though the necessary means are provided
       by the Mach-O specification.

*/

#include <util.h>

#include <libyasm.h>

/* MACH-O DEFINES */
/* Mach-O in-file header structure sizes (32 BIT, see below for 64 bit defs) */
#define MACHO_HEADER_SIZE       28
#define MACHO_SEGCMD_SIZE       56
#define MACHO_SECTCMD_SIZE      68
#define MACHO_SYMCMD_SIZE       24
#define MACHO_NLIST_SIZE        12
#define MACHO_RELINFO_SIZE      8

/* 64 bit sizes */
#define MACHO_HEADER64_SIZE     32
#define MACHO_SEGCMD64_SIZE     72
#define MACHO_SECTCMD64_SIZE    80
#define MACHO_NLIST64_SIZE      16
#define MACHO_RELINFO64_SIZE    8


/* Mach-O file header values */
#define MH_MAGIC                0xfeedface
#define MH_MAGIC_64             0xfeedfacf

/* CPU machine type */
#define CPU_TYPE_I386           7       /* x86 platform */
#define CPU_TYPE_X86_64         (CPU_TYPE_I386|CPU_ARCH_ABI64)
#define CPU_ARCH_ABI64          0x01000000      /* 64 bit ABI */

/* CPU machine subtype, e.g. processor */
#define CPU_SUBTYPE_I386_ALL    3       /* all-x86 compatible */
#define CPU_SUBTYPE_X86_64_ALL  CPU_SUBTYPE_I386_ALL
#define CPU_SUBTYPE_386         3
#define CPU_SUBTYPE_486         4
#define CPU_SUBTYPE_486SX       (4 + 128)
#define CPU_SUBTYPE_586         5
#define CPU_SUBTYPE_INTEL(f, m) ((f) + ((m) << 4))
#define CPU_SUBTYPE_PENT        CPU_SUBTYPE_INTEL(5, 0)
#define CPU_SUBTYPE_PENTPRO     CPU_SUBTYPE_INTEL(6, 1)
#define CPU_SUBTYPE_PENTII_M3   CPU_SUBTYPE_INTEL(6, 3)
#define CPU_SUBTYPE_PENTII_M5   CPU_SUBTYPE_INTEL(6, 5)
#define CPU_SUBTYPE_PENTIUM_4   CPU_SUBTYPE_INTEL(10, 0)

#define CPU_SUBTYPE_INTEL_FAMILY(x)     ((x) & 15)
#define CPU_SUBTYPE_INTEL_FAMILY_MAX    15

#define CPU_SUBTYPE_INTEL_MODEL(x)      ((x) >> 4)
#define CPU_SUBTYPE_INTEL_MODEL_ALL     0

#define MH_OBJECT               0x1     /* object file */

#define LC_SEGMENT              0x1     /* segment load command */
#define LC_SYMTAB               0x2     /* symbol table load command */
#define LC_SEGMENT_64           0x19    /* segment load command */


#define VM_PROT_NONE            0x00
#define VM_PROT_READ            0x01
#define VM_PROT_WRITE           0x02
#define VM_PROT_EXECUTE         0x04

#define VM_PROT_DEFAULT (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)
#define VM_PROT_ALL     (VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)

#define SECTION_TYPE        0x000000ff  /* section type mask */
#define SECTION_ATTRIBUTES  0xffffff00UL/* section attributes mask */

#define S_REGULAR           0x0         /* standard section */
#define S_ZEROFILL          0x1         /* zerofill, in-memory only */
#define S_CSTRING_LITERALS  0x2         /* literal C strings */
#define S_4BYTE_LITERALS    0x3         /* only 4-byte literals */
#define S_8BYTE_LITERALS    0x4         /* only 8-byte literals */
#define S_LITERAL_POINTERS  0x5         /* only pointers to literals */
#define S_NON_LAZY_SYMBOL_POINTERS  0x6 /* only non-lazy symbol pointers */
#define S_LAZY_SYMBOL_POINTERS      0x7 /* only lazy symbol pointers */
#define S_SYMBOL_STUBS      0x8         /* only symbol stubs; byte size of
                                         * stub in the reserved2 field */
#define S_MOD_INIT_FUNC_POINTERS    0x9 /* only function pointers for init */
#define S_MOD_TERM_FUNC_POINTERS    0xa /* only function pointers for term */
#define S_COALESCED         0xb         /* symbols that are to be coalesced */
#define S_GB_ZEROFILL       0xc         /* >4GB zero fill on demand section */
#define S_INTERPOSING       0xd         /* only pairs of function pointers for
                                         * interposing */
#define S_16BYTE_LITERALS   0xe         /* only 16 byte literals */

#define S_ATTR_DEBUG             0x02000000     /* a debug section */
#define SECTION_ATTRIBUTES_SYS   0x00ffff00     /* system setable attributes */
#define S_ATTR_SOME_INSTRUCTIONS 0x00000400     /* section contains some
                                                 * machine instructions */
#define S_ATTR_EXT_RELOC         0x00000200     /* section has external
                                                 * relocation entries */
#define S_ATTR_LOC_RELOC         0x00000100     /* section has local
                                                 * relocation entries */

#define SECTION_ATTRIBUTES_USR   0xff000000UL   /* User setable attributes */
#define S_ATTR_PURE_INSTRUCTIONS 0x80000000UL   /* only true machine insns */
#define S_ATTR_NO_TOC            0x40000000UL   /* coalesced symbols that are
                                                 * not to be in a ranlib table
                                                 * of contents */
#define S_ATTR_STRIP_STATIC_SYMS 0x20000000UL   /* ok to strip static symbols
                                                 * in this section in files
                                                 * with the MH_DYLDLINK flag */
#define S_ATTR_NO_DEAD_STRIP     0x10000000UL   /* no dead stripping */
#define S_ATTR_LIVE_SUPPORT      0x08000000UL   /* blocks are live if they
                                                 * reference live blocks */
#define S_ATTR_SELF_MODIFYING_CODE 0x04000000UL /* Used with i386 code stubs
                                                 * written on by dyld */

/* macho references symbols in different ways whether they are linked at
 * runtime (LAZY, read library functions) or at link time (NON_LAZY, mostly
 * data)
 *
 * TODO: proper support for dynamically linkable modules would require the
 * __import sections as well as the dsymtab command
 */
#define REFERENCE_FLAG_UNDEFINED_NON_LAZY 0x0
#define REFERENCE_FLAG_UNDEFINED_LAZY     0x1

#define align(x, y) \
    (((x) + (y) - 1) & ~((y) - 1))      /* align x to multiple of y */

#define align32(x) \
    align(x, 4)                 /* align x to 32 bit boundary */

#define macho_MAGIC     0x87654322

/* Symbol table type field bit masks */
#define N_STAB  0xe0            /* mask indicating stab entry */
#define N_PEXT  0x10            /* private external bit */
#define N_TYPE  0x0e            /* mask for all the type bits */
#define N_EXT   0x01            /* external (global) bit */

/* Symbol table type field values */
#define N_UNDF  0x00            /* undefined */
#define N_ABS   0x02            /* absolute address */
#define N_SECT  0x0e            /* symbol is defined in a section */

#define NO_SECT 0               /* no section for symbol in nlist */

#define REGULAR_OUTBUF_SIZE     1024


typedef struct macho_reloc {
    yasm_reloc reloc;
    int pcrel;
    int length;
    int ext;
    enum reloc_type_x86_64 {
        /* x86 relocations */
        GENERIC_RELOC_VANILLA = 0,      /* generic relocation */
        GENERIC_RELOC_PAIR = 1,         /* Only follows a GENERIC_RELOC_SECTDIFF */
        GENERIC_RELOC_SECTDIFF = 2,
        GENERIC_RELOC_PB_LA_PTR = 3,    /* prebound lazy pointer */
        GENERIC_RELOC_LOCAL_SECTDIFF = 4,

        /* x86-64 relocations */
        X86_64_RELOC_UNSIGNED = 0,      /* for absolute addresses */
        X86_64_RELOC_SIGNED = 1,        /* for signed 32-bit displacement */
        X86_64_RELOC_BRANCH = 2,        /* a CALL/JMP insn with 32-bit disp */
        X86_64_RELOC_GOT_LOAD = 3,      /* a MOVQ load of a GOT entry */
        X86_64_RELOC_GOT = 4,           /* other GOT references */
        X86_64_RELOC_SUBTRACTOR = 5,    /* must be followed by a X86_64_RELOC_UNSIGNED */
        X86_64_RELOC_SIGNED_1 = 6,      /* signed 32-bit disp, -1 addend */
        X86_64_RELOC_SIGNED_2 = 7,      /* signed 32-bit disp, -2 addend */
        X86_64_RELOC_SIGNED_4 = 8       /* signed 32-bit disp, -4 addend */
    } type;
} macho_reloc;

typedef struct macho_section_data {
    /*@dependent@*/ yasm_symrec *sym; /* symbol created for this section */
    long scnum;                 /* section number (0=first section) */
    /*@only@*/ char *segname;   /* segment name in file */
    /*@only@*/ char *sectname;  /* section name in file */
    unsigned long flags;        /* S_* flags */
    unsigned long size;         /* size of raw data (section data) in bytes */
    unsigned long offset;       /* offset in raw data within file in bytes */
    unsigned long vmoff;        /* memory offset */
    unsigned long nreloc;       /* number of relocation entries */
    unsigned int extreloc;      /* external relocations present (0/1) */
} macho_section_data;


typedef struct macho_symrec_data {
    unsigned long index;        /* index in output order */
    yasm_intnum *value;         /* valid after writing symtable to file */
    unsigned long length;       /* length + 1 (plus auto underscore) */
} macho_symrec_data;


typedef struct yasm_objfmt_macho {
    yasm_objfmt_base objfmt;    /* base structure */

    long parse_scnum;           /* sect numbering in parser */
    int bits;                   /* 32 / 64 */

    yasm_symrec *gotpcrel_sym;  /* ..gotpcrel */
} yasm_objfmt_macho;


typedef struct macho_objfmt_output_info {
    yasm_object *object;
    yasm_objfmt_macho *objfmt_macho;
    yasm_errwarns *errwarns;
    /*@dependent@ */ FILE *f;
    /*@only@ */ unsigned char *buf;
    yasm_section *sect;
    /*@dependent@ */ macho_section_data *msd;

    unsigned int is_64;         /* write object in 64 bit mode */

    /* vmsize and filesize available after traversing section count routine */
    unsigned long vmsize;       /* raw size of all sections (including BSS) */
    unsigned long filesize;     /* size of sections in file (excluding BSS) */
    unsigned long offset;       /* offset within file */

    /* forward offset tracking */
    unsigned long rel_base;     /* first relocation in file */
    unsigned long s_reloff;     /* in-file offset to relocations */

    unsigned long indx;         /* current symbol size in bytes (name length+1) */
    unsigned long symindex;     /* current symbol index in output order */
    int all_syms;               /* outputting all symbols? */
    unsigned long strlength;    /* length of all strings */
} macho_objfmt_output_info;


static void macho_section_data_destroy(/*@only@*/ void *d);
static void macho_section_data_print(void *data, FILE *f, int indent_level);

static const yasm_assoc_data_callback macho_section_data_cb = {
    macho_section_data_destroy,
    macho_section_data_print
};

static void macho_symrec_data_destroy(/*@only@*/ void *d);
static void macho_symrec_data_print(void *data, FILE *f, int indent_level);

static const yasm_assoc_data_callback macho_symrec_data_cb = {
    macho_symrec_data_destroy,
    macho_symrec_data_print
};

yasm_objfmt_module yasm_macho_LTX_objfmt;
yasm_objfmt_module yasm_macho32_LTX_objfmt;
yasm_objfmt_module yasm_macho64_LTX_objfmt;

static yasm_objfmt *
macho_objfmt_create_common(yasm_object *object, yasm_objfmt_module *module,
                           int bits_pref)
{
    yasm_objfmt_macho *objfmt_macho = yasm_xmalloc(sizeof(yasm_objfmt_macho));

    objfmt_macho->objfmt.module = module;

    /* Only support x86 arch for now */
    if (yasm__strcasecmp(yasm_arch_keyword(object->arch), "x86") != 0) {
        yasm_xfree(objfmt_macho);
        return NULL;
    }

    /* Support x86 and amd64 machines of x86 arch */
    if (yasm__strcasecmp(yasm_arch_get_machine(object->arch), "x86") == 0 &&
        (bits_pref == 0 || bits_pref == 32)) {
        objfmt_macho->bits = 32;
        objfmt_macho->gotpcrel_sym = NULL;
    } else if (yasm__strcasecmp(yasm_arch_get_machine(object->arch),
                              "amd64") == 0 &&
             (bits_pref == 0 || bits_pref == 64)) {
        objfmt_macho->bits = 64;
        /* FIXME: misuse of NULL bytecode */
        objfmt_macho->gotpcrel_sym =
            yasm_symtab_define_label(object->symtab, "..gotpcrel", NULL, 0, 0);
    } else {
        yasm_xfree(objfmt_macho);
        return NULL;
    }

    objfmt_macho->parse_scnum = 0;      /* section numbering starts at 0 */
    return (yasm_objfmt *)objfmt_macho;
}

static yasm_objfmt *
macho_objfmt_create(yasm_object *object)
{
    yasm_objfmt *objfmt;
    yasm_objfmt_macho *objfmt_macho;

    objfmt = macho_objfmt_create_common(object, &yasm_macho_LTX_objfmt, 0);
    if (objfmt) {
        objfmt_macho = (yasm_objfmt_macho *)objfmt;
        /* Figure out which bitness of object format to use */
        if (objfmt_macho->bits == 32)
            objfmt_macho->objfmt.module = &yasm_macho32_LTX_objfmt;
        else if (objfmt_macho->bits == 64)
            objfmt_macho->objfmt.module = &yasm_macho64_LTX_objfmt;
    }
    return objfmt;
}

static yasm_objfmt *
macho32_objfmt_create(yasm_object *object)
{
    return macho_objfmt_create_common(object, &yasm_macho32_LTX_objfmt, 32);
}

static yasm_objfmt *
macho64_objfmt_create(yasm_object *object)
{
    return macho_objfmt_create_common(object, &yasm_macho64_LTX_objfmt, 64);
}

static int
macho_objfmt_output_value(yasm_value *value, unsigned char *buf,
                          unsigned int destsize, unsigned long offset,
                          yasm_bytecode *bc, int warn, /*@null@*/ void *d)
{
    /*@null@*/ macho_objfmt_output_info *info = (macho_objfmt_output_info *)d;
    yasm_objfmt_macho *objfmt_macho;
    /*@dependent@*/ /*@null@*/ yasm_intnum *intn;
    unsigned long intn_minus = 0, intn_plus = 0;
    int retval;
    unsigned int valsize = value->size;
    macho_reloc *reloc = NULL;

    assert(info != NULL);
    objfmt_macho = info->objfmt_macho;

    if (value->abs)
        value->abs = yasm_expr_simplify(value->abs, 1);

    /* Try to output constant and PC-relative section-local first.
     * Note this does NOT output any value with a SEG, WRT, external,
     * cross-section, or non-PC-relative reference (those are handled below).
     */
    switch (yasm_value_output_basic(value, buf, destsize, bc, warn,
                                    info->object->arch)) {
        case -1:
            return 1;
        case 0:
            break;
        default:
            return 0;
    }

    if (value->section_rel) {
        yasm_error_set(YASM_ERROR_TOO_COMPLEX,
            N_("macho: relocation too complex for current implementation"));
        return 1;
    }

    if (value->rel) {
        yasm_sym_vis vis = yasm_symrec_get_visibility(value->rel);

        reloc = yasm_xcalloc(sizeof(macho_reloc), 1);
        reloc->reloc.addr = yasm_intnum_create_uint(bc->offset + offset);
        reloc->reloc.sym = value->rel;
        switch (valsize) {
            case 64:
                reloc->length = 3;
                break;
            case 32:
                reloc->length = 2;
                break;
            case 16:
                reloc->length = 1;
                break;
            case 8:
                reloc->length = 0;
                break;
            default:
                yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                               N_("macho: relocation size unsupported"));
                yasm_xfree(reloc);
                return 1;
        }
        reloc->pcrel = 0;
        reloc->ext = 0;
        reloc->type = GENERIC_RELOC_VANILLA;
        /* R_ABS */

        if (value->rshift > 0) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("macho: shifted relocations not supported"));
            yasm_xfree(reloc);
            return 1;
        }

        if (value->seg_of) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("macho: SEG not supported"));
            yasm_xfree(reloc);
            return 1;
        }

        if (value->curpos_rel && objfmt_macho->gotpcrel_sym &&
            value->wrt == objfmt_macho->gotpcrel_sym) {
            reloc->type = X86_64_RELOC_GOT;
            value->wrt = NULL;
        } else if (value->wrt) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("macho: invalid WRT"));
            yasm_xfree(reloc);
            return 1;
        }

        if (value->curpos_rel) {
            reloc->pcrel = 1;
            if (!info->is_64) {
                /* Adjust to start of section, so subtract out the bytecode
                 * offset.
                 */
                intn_minus = bc->offset;
            } else {
                /* Add in the offset plus value size to end up with 0. */
                intn_plus = offset+destsize;
                if (reloc->type == X86_64_RELOC_GOT) {
                    /* XXX: This is a hack */
                    if (offset >= 2 && buf[-2] == 0x8B)
                        reloc->type = X86_64_RELOC_GOT_LOAD;
                } else if (value->jump_target)
                    reloc->type = X86_64_RELOC_BRANCH;
                else
                    reloc->type = X86_64_RELOC_SIGNED;
            }
        } else if (info->is_64) {
            if (valsize == 32) {
                yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                    N_("macho: sorry, cannot apply 32 bit absolute relocations in 64 bit mode, consider \"[_symbol wrt rip]\" for mem access, \"qword\" and \"dq _foo\" for pointers."));
                return 1;
            }
            reloc->type = X86_64_RELOC_UNSIGNED;
        }

        /* It seems that x86-64 objects need to have all extern relocs? */
        if (info->is_64)
            reloc->ext = 1;

        if ((vis & YASM_SYM_EXTERN) || (vis & YASM_SYM_COMMON)) {
            reloc->ext = 1;
            info->msd->extreloc = 1;    /* section has external relocations */
        } else if (!info->is_64) {
            /*@dependent@*/ /*@null@*/ yasm_bytecode *sym_precbc;

            /* Local symbols need valued to their actual address */
            if (yasm_symrec_get_label(value->rel, &sym_precbc)) {
                yasm_section *sym_sect = yasm_bc_get_section(sym_precbc);
                /*@null@*/ macho_section_data *msd;
                msd = yasm_section_get_data(sym_sect, &macho_section_data_cb);
                assert(msd != NULL);
                intn_plus += msd->vmoff + yasm_bc_next_offset(sym_precbc);
            }
        }

        info->msd->nreloc++;
        /*printf("reloc %s type %d ",yasm_symrec_get_name(reloc->reloc.sym),reloc->type);*/
        yasm_section_add_reloc(info->sect, (yasm_reloc *)reloc, yasm_xfree);
    }

    if (intn_minus <= intn_plus)
        intn = yasm_intnum_create_uint(intn_plus-intn_minus);
    else {
        intn = yasm_intnum_create_uint(intn_minus-intn_plus);
        yasm_intnum_calc(intn, YASM_EXPR_NEG, NULL);
    }

    if (value->abs) {
        yasm_intnum *intn2 = yasm_expr_get_intnum(&value->abs, 0);

        if (!intn2) {
            yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                           N_("macho: relocation too complex"));
            yasm_intnum_destroy(intn);
            return 1;
        }
        yasm_intnum_calc(intn, YASM_EXPR_ADD, intn2);
    }

    retval = yasm_arch_intnum_tobytes(info->object->arch, intn, buf, destsize,
                                      valsize, 0, bc, warn);
    /*printf("val %ld\n",yasm_intnum_get_int(intn));*/
    yasm_intnum_destroy(intn);
    return retval;
}

static int
macho_objfmt_output_bytecode(yasm_bytecode *bc, /*@null@*/ void *d)
{
    /*@null@*/ macho_objfmt_output_info *info = (macho_objfmt_output_info *)d;
    /*@null@*/ /*@only@*/ unsigned char *bigbuf;
    unsigned long size = REGULAR_OUTBUF_SIZE;
    int gap;

    assert(info != NULL);

    bigbuf = yasm_bc_tobytes(bc, info->buf, &size, &gap, info,
                             macho_objfmt_output_value, NULL);

    /* Don't bother doing anything else if size ended up being 0. */
    if (size == 0) {
        if (bigbuf)
            yasm_xfree(bigbuf);
        return 0;
    }

    /* Warn that gaps are converted to 0 and write out the 0's. */
    if (gap) {
        unsigned long left;

        yasm_warn_set(YASM_WARN_UNINIT_CONTENTS,
                      N_("uninitialized space: zeroing"));
        /* Write out in chunks */
        memset(info->buf, 0, REGULAR_OUTBUF_SIZE);
        left = size;
        while (left > REGULAR_OUTBUF_SIZE) {
            fwrite(info->buf, REGULAR_OUTBUF_SIZE, 1, info->f);
            left -= REGULAR_OUTBUF_SIZE;
        }
        fwrite(info->buf, left, 1, info->f);
    } else {
        /* Output buf (or bigbuf if non-NULL) to file */
        fwrite(bigbuf ? bigbuf : info->buf, (size_t) size, 1, info->f);
    }

    /* If bigbuf was allocated, free it */
    if (bigbuf)
        yasm_xfree(bigbuf);

    return 0;
}

static int
macho_objfmt_output_section(yasm_section *sect, /*@null@ */ void *d)
{
    /*@null@ */ macho_objfmt_output_info *info =
        (macho_objfmt_output_info *) d;
    /*@dependent@ *//*@null@ */ macho_section_data *msd;

    assert(info != NULL);
    msd = yasm_section_get_data(sect, &macho_section_data_cb);
    assert(msd != NULL);

    if (!(msd->flags & S_ZEROFILL)) {
        /* Output non-BSS sections */
        info->sect = sect;
        info->msd = msd;
        yasm_section_bcs_traverse(sect, info->errwarns, info,
                                  macho_objfmt_output_bytecode);
    }
    return 0;
}

static int
macho_objfmt_output_relocs(yasm_section *sect, /*@null@*/ void *d)
{
    /*@null@*/ macho_objfmt_output_info *info = (macho_objfmt_output_info *)d;
    /*@dependent@*/ /*@null@*/ macho_section_data *msd;
    macho_reloc *reloc;

    reloc = (macho_reloc *)yasm_section_relocs_first(sect);
    while (reloc) {
        unsigned char *localbuf = info->buf;
        /*@null@*/ macho_symrec_data *xsymd;
        unsigned long symnum;

        xsymd = yasm_symrec_get_data(reloc->reloc.sym, &macho_symrec_data_cb);
        yasm_intnum_get_sized(reloc->reloc.addr, localbuf, 4, 32, 0, 0, 0);
        localbuf += 4;          /* address of relocation */

        if (reloc->ext)
            symnum = xsymd->index;
        else {
            /* find section where the symbol relates to */
            /*@dependent@*/ /*@null@*/ yasm_section *dsect;
            /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc;
            symnum = 0; /* default to absolute */
            if (yasm_symrec_get_label(reloc->reloc.sym, &precbc) &&
                (dsect = yasm_bc_get_section(precbc)) &&
                (msd = yasm_section_get_data(dsect, &macho_section_data_cb)))
                symnum = msd->scnum+1;
        }
        YASM_WRITE_32_L(localbuf,
                        (symnum & 0x00ffffff) |
                        (((unsigned long)reloc->pcrel & 1) << 24) |
                        (((unsigned long)reloc->length & 3) << 25) |
                        (((unsigned long)reloc->ext & 1) << 27) |
                        (((unsigned long)reloc->type & 0xf) << 28));
        fwrite(info->buf, 8, 1, info->f);
        reloc = (macho_reloc *)yasm_section_reloc_next((yasm_reloc *)reloc);
    }

    return 0;
}

static int
exp2_to_bits(unsigned long val)
{
    int ret = 0;

    while (val) {
        val >>= 1;
        ret++;
    }
    ret = (ret > 0) ? ret - 1 : 0;

    return ret;
}

static int
macho_objfmt_is_section_label(yasm_symrec *sym)
{
    /*@dependent@*/ /*@null@*/ yasm_section *sect;
    /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc;

    /* Look at symrec for value/scnum/etc. */
    if (yasm_symrec_get_label(sym, &precbc)) {
        if (precbc)
            sect = yasm_bc_get_section(precbc);
        else
            sect = NULL;
        /* it's a label: get value and offset.
         * If there is not a section, leave as debugging symbol.
         */
        if (sect) {
            /*@dependent@*/ /*@null@*/ macho_section_data *msd;

            msd = yasm_section_get_data(sect, &macho_section_data_cb);
            if (msd) {
                if (msd->sym == sym)
                    return 1;   /* don't store section names */
            }
        }
    }
    return 0;
}

static int
macho_objfmt_output_secthead(yasm_section *sect, /*@null@*/ void *d)
{
    /*@null@*/ macho_objfmt_output_info *info = (macho_objfmt_output_info *)d;
    yasm_objfmt_macho *objfmt_macho;
    /*@dependent@*/ /*@null@*/ macho_section_data *msd;
    unsigned char *localbuf;

    assert(info != NULL);
    objfmt_macho = info->objfmt_macho;
    msd = yasm_section_get_data(sect, &macho_section_data_cb);
    assert(msd != NULL);

    localbuf = info->buf;

    memset(localbuf, 0, 16);
    strncpy((char *)localbuf, msd->sectname, 16);
    localbuf += 16;
    memset(localbuf, 0, 16);
    strncpy((char *)localbuf, msd->segname, 16);
    localbuf += 16;
    /* section address, size depend on 32/64 bit mode */
    YASM_WRITE_32_L(localbuf, msd->vmoff);      /* address in memory */
    if (info->is_64)
        YASM_WRITE_32_L(localbuf, 0);   /* 64-bit mode: upper 32 bits = 0 */
    YASM_WRITE_32_L(localbuf, msd->size);       /* size in memory */
    if (info->is_64)
        YASM_WRITE_32_L(localbuf, 0);   /* 64-bit mode: upper 32 bits = 0 */

    /* offset,align,reloff,nreloc,flags,reserved1,reserved2 are 32 bit */
    if ((msd->flags & SECTION_TYPE) != S_ZEROFILL) {
        YASM_WRITE_32_L(localbuf, msd->offset);
        YASM_WRITE_32_L(localbuf, exp2_to_bits(yasm_section_get_align(sect)));
        if (msd->nreloc) {
            msd->flags |= S_ATTR_LOC_RELOC;
            if (msd->extreloc)
                msd->flags |= S_ATTR_EXT_RELOC;
            YASM_WRITE_32_L(localbuf,
                            align32((long)(info->rel_base + info->s_reloff)));
            YASM_WRITE_32_L(localbuf, msd->nreloc);     /* nreloc */
        } else {
            YASM_WRITE_32_L(localbuf, 0);
            YASM_WRITE_32_L(localbuf, 0);
        }

        info->s_reloff += msd->nreloc * MACHO_RELINFO_SIZE;     /* nreloc */
    } else {
        YASM_WRITE_32_L(localbuf, 0);   /* these are zero in BSS */
        YASM_WRITE_32_L(localbuf, 0);
        YASM_WRITE_32_L(localbuf, 0);
        YASM_WRITE_32_L(localbuf, 0);
    }

    YASM_WRITE_32_L(localbuf, msd->flags);      /* flags */
    YASM_WRITE_32_L(localbuf, 0);       /* reserved 1 */
    YASM_WRITE_32_L(localbuf, 0);       /* reserved 2 */

    if (info->is_64)
        fwrite(info->buf, MACHO_SECTCMD64_SIZE, 1, info->f);
    else
        fwrite(info->buf, MACHO_SECTCMD_SIZE, 1, info->f);

    return 0;
}


static int
macho_objfmt_count_sym(yasm_symrec *sym, /*@null@*/ void *d)
{
    /*@null@*/ macho_objfmt_output_info *info = (macho_objfmt_output_info *)d;
    /*@only@*/ char *name;
    yasm_sym_vis vis = yasm_symrec_get_visibility(sym);

    assert(info != NULL);
    if (info->all_syms ||
        vis & (YASM_SYM_GLOBAL | YASM_SYM_COMMON | YASM_SYM_EXTERN)) {
        if (0 == macho_objfmt_is_section_label(sym)) {
            /* Save index in symrec data */
            macho_symrec_data *sym_data =
                yasm_symrec_get_data(sym, &macho_symrec_data_cb);
            if (!sym_data) {
                sym_data = yasm_xcalloc(sizeof(macho_symrec_data), 1);
                yasm_symrec_add_data(sym, &macho_symrec_data_cb, sym_data);
            }
            sym_data->index = info->symindex;
            info->symindex++;

            name = yasm_symrec_get_global_name(sym, info->object);
            /*printf("%s\n",name); */
            /* name length + delimiter */
            sym_data->length = (unsigned long)strlen(name) + 1;
            info->strlength += sym_data->length;
            info->indx++;
            yasm_xfree(name);
        }
    }
    return 0;
}


static int
macho_objfmt_output_symtable(yasm_symrec *sym, /*@null@*/ void *d)
{
    /*@null@*/ macho_objfmt_output_info *info = (macho_objfmt_output_info *)d;
    yasm_sym_vis vis = yasm_symrec_get_visibility(sym);

    assert(info != NULL);

    if (info->all_syms ||
        vis & (YASM_SYM_GLOBAL | YASM_SYM_COMMON | YASM_SYM_EXTERN)) {
        const yasm_expr *equ_val;
        const yasm_intnum *intn;
        unsigned long value = 0;
        long scnum = -3;        /* -3 = debugging symbol */
        /*@dependent@*/ /*@null@*/ yasm_section *sect;
        /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc;
        unsigned char *localbuf;
        yasm_intnum *val;
        unsigned int long_int_bytes = (info->is_64) ? 8 : 4;
        unsigned int n_type = 0, n_sect = 0, n_desc = 0;
        macho_symrec_data *symd;

        val = yasm_intnum_create_uint(0);

        symd = yasm_symrec_get_data(sym, &macho_symrec_data_cb);

        /* Look at symrec for value/scnum/etc. */
        if (yasm_symrec_get_label(sym, &precbc)) {
            if (precbc)
                sect = yasm_bc_get_section(precbc);
            else
                sect = NULL;
            /* it's a label: get value and offset.
             * If there is not a section, leave as debugging symbol.
             */
            if (sect) {
                /*@dependent@*/ /*@null@*/ macho_section_data *msd;

                msd = yasm_section_get_data(sect, &macho_section_data_cb);
                if (msd) {
                    if (msd->sym == sym) {
                        /* don't store section names */
                        yasm_intnum_destroy(val);
                        return 0;
                    }
                    scnum = msd->scnum;
                    n_type = N_SECT;
                } else
                    yasm_internal_error(N_("didn't understand section"));
                if (precbc)
                    value += yasm_bc_next_offset(precbc);
                /* all values are subject to correction: base offset is first
                 * raw section, therefore add section offset
                 */
                if (msd)
                    value += msd->vmoff;
                yasm_intnum_set_uint(val, value);
                /*printf("%s offset %lx\n",name,value);*/
            }
        } else if ((equ_val = yasm_symrec_get_equ(sym))) {
            yasm_expr *equ_val_copy = yasm_expr_copy(equ_val);

            intn = yasm_expr_get_intnum(&equ_val_copy, 1);
            if (!intn) {
                if (vis & YASM_SYM_GLOBAL) {
                    yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                        N_("global EQU value not an integer expression"));
                    yasm_errwarn_propagate(info->errwarns, equ_val->line);
                }
            } else
                value = yasm_intnum_get_uint(intn);
            yasm_expr_destroy(equ_val_copy);
            yasm_intnum_set_uint(val, value);
            n_type = N_ABS;
            scnum = -2;         /* -2 = absolute symbol */
        }

        if (vis & YASM_SYM_EXTERN) {
            n_type = N_EXT;
            scnum = -1;
            /*n_desc = REFERENCE_FLAG_UNDEFINED_LAZY;   * FIXME: see definition of REFERENCE_FLAG_* above */
        } else if (vis & YASM_SYM_COMMON) {
            yasm_expr **csize = yasm_symrec_get_common_size(sym);
            n_type = N_UNDF | N_EXT;
            if (csize) {
                intn = yasm_expr_get_intnum(csize, 1);
                if (!intn) {
                    yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                                   N_("COMMON data size not an integer expression"));
                    yasm_errwarn_propagate(info->errwarns, (*csize)->line);
                } else
                    yasm_intnum_set_uint(val, yasm_intnum_get_uint(intn));
            }
            /*printf("common symbol %s val %lu\n", name, yasm_intnum_get_uint(val));*/
        } else if (vis & YASM_SYM_GLOBAL) {
            yasm_valparamhead *valparams =
                yasm_symrec_get_objext_valparams(sym);

            struct macho_global_data {
                unsigned long flag; /* N_PEXT */
            } data;

            data.flag = 0;

            if (valparams) {
                static const yasm_dir_help help[] = {
                    { "private_extern", 0, yasm_dir_helper_flag_set,
                      offsetof(struct macho_global_data, flag), N_PEXT },
                };
                yasm_dir_helper(sym, yasm_vps_first(valparams),
                                yasm_symrec_get_decl_line(sym), help, NELEMS(help),
                                &data, yasm_dir_helper_valparam_warn);
            }

            n_type |= N_EXT | data.flag;
        }

        localbuf = info->buf;
        YASM_WRITE_32_L(localbuf, info->indx);  /* offset in string table */
        YASM_WRITE_8(localbuf, n_type); /* type of symbol entry */
        n_sect = (scnum >= 0) ? scnum + 1 : NO_SECT;
        YASM_WRITE_8(localbuf, n_sect); /* referring section where symbol is found */
        YASM_WRITE_16_L(localbuf, n_desc);      /* extra description */
        yasm_intnum_get_sized(val, localbuf, long_int_bytes, ((long_int_bytes) << 3), 0, 0, 0); /* value/argument */
        localbuf += long_int_bytes;
        if (symd)
            symd->value = val;
        else
            yasm_intnum_destroy(val);

        info->indx += symd->length;

        fwrite(info->buf, 8 + long_int_bytes, 1, info->f);
    }

    return 0;
}


static int
macho_objfmt_output_str(yasm_symrec *sym, /*@null@*/ void *d)
{
    /*@null@*/ macho_objfmt_output_info *info = (macho_objfmt_output_info *)d;
    yasm_sym_vis vis = yasm_symrec_get_visibility(sym);
    /*@null@*/ macho_symrec_data *xsymd;


    assert(info != NULL);

    if (info->all_syms ||
        vis & (YASM_SYM_GLOBAL | YASM_SYM_COMMON | YASM_SYM_EXTERN)) {
        if (0 == macho_objfmt_is_section_label(sym)) {
            /*@only@*/ char *name =
                yasm_symrec_get_global_name(sym, info->object);
            size_t len = strlen(name);

            xsymd = yasm_symrec_get_data(sym, &macho_symrec_data_cb);
            fwrite(name, len + 1, 1, info->f);
            yasm_xfree(name);
        }
    }
    return 0;
}

static int
macho_objfmt_calc_sectsize(yasm_section *sect, /*@null@ */ void *d)
{
    /*@null@ */ macho_objfmt_output_info *info =
        (macho_objfmt_output_info *) d;
    /*@dependent@ *//*@null@ */ macho_section_data *msd;
    unsigned long align;

    assert(info != NULL);
    msd = yasm_section_get_data(sect, &macho_section_data_cb);
    assert(msd != NULL);

    msd->size = yasm_bc_next_offset(yasm_section_bcs_last(sect));
    if (!(msd->flags & S_ZEROFILL)) {
        msd->offset = info->offset;
        info->offset += msd->size;
        info->filesize += msd->size;
    }

    /* accumulate size in memory */
    msd->vmoff = info->vmsize;
    info->vmsize += msd->size;

    /* align both start and end of section */
    align = yasm_section_get_align(sect);
    if (align != 0) {
        unsigned long delta = msd->vmoff % align;
        if (delta > 0) {
            msd->vmoff += align - delta;
            info->vmsize += align - delta;
        }
    }

    return 0;
}

/* write object */
static void
macho_objfmt_output(yasm_object *object, FILE *f, int all_syms,
                    yasm_errwarns *errwarns)
{
    yasm_objfmt_macho *objfmt_macho = (yasm_objfmt_macho *)object->objfmt;
    macho_objfmt_output_info info;
    unsigned char *localbuf;
    unsigned long symtab_count = 0;
    unsigned long headsize;
    unsigned int macho_segcmdsize, macho_sectcmdsize, macho_nlistsize;
    unsigned int macho_relinfosize, macho_segcmd;
    unsigned int head_ncmds, head_sizeofcmds;
    unsigned long fileoffset, fileoff_sections;
    yasm_intnum *val;
    unsigned long long_int_bytes;
    const char pad_data[3] = "\0\0\0";

    info.object = object;
    info.objfmt_macho = objfmt_macho;
    info.errwarns = errwarns;
    info.f = f;
    info.buf = yasm_xmalloc(REGULAR_OUTBUF_SIZE);

    if (objfmt_macho->parse_scnum == 0) {
        yasm_internal_error(N_("no sections defined"));
        /*@notreached@*/
        return;
    }

    val = yasm_intnum_create_uint(0);

    /*
     * MACH-O Header, Seg CMD, Sect CMDs, Sym Tab, Reloc Data
     */
    info.is_64 = (objfmt_macho->bits == 32) ? 0 : 1;
    if (info.is_64) {
        /* this works only when SYMBOLS and SECTIONS present */
        headsize =
            MACHO_HEADER64_SIZE + MACHO_SEGCMD64_SIZE +
            (MACHO_SECTCMD64_SIZE * (objfmt_macho->parse_scnum)) +
            MACHO_SYMCMD_SIZE;
        macho_segcmd = LC_SEGMENT_64;
        macho_segcmdsize = MACHO_SEGCMD64_SIZE;
        macho_sectcmdsize = MACHO_SECTCMD64_SIZE;
        macho_nlistsize = MACHO_NLIST64_SIZE;
        macho_relinfosize = MACHO_RELINFO64_SIZE;
        long_int_bytes = 8;
    } else {
        headsize =
            MACHO_HEADER_SIZE + MACHO_SEGCMD_SIZE +
            (MACHO_SECTCMD_SIZE * (objfmt_macho->parse_scnum)) +
            MACHO_SYMCMD_SIZE;
        macho_segcmd = LC_SEGMENT;
        macho_segcmdsize = MACHO_SEGCMD_SIZE;
        macho_sectcmdsize = MACHO_SECTCMD_SIZE;
        macho_nlistsize = MACHO_NLIST_SIZE;
        macho_relinfosize = MACHO_RELINFO_SIZE;
        long_int_bytes = 4;
    }

    /* Get number of symbols */
    info.symindex = 0;
    info.indx = 0;
    info.strlength = 1;         /* string table starts with a zero byte */
    info.all_syms = all_syms || info.is_64;
    /*info.all_syms = 1;                * force all syms into symbol table */
    yasm_symtab_traverse(object->symtab, &info, macho_objfmt_count_sym);
    symtab_count = info.indx;

    /* write raw section data first */
    if (fseek(f, (long)headsize, SEEK_SET) < 0) {
        yasm__fatal(N_("could not seek on output file"));
        /*@notreached@ */
        return;
    }

    /* get size of sections in memory (including BSS) and size of sections
     * in file (without BSS)
     */
    info.vmsize = 0;
    info.filesize = 0;
    info.offset = headsize;
    yasm_object_sections_traverse(object, &info, macho_objfmt_calc_sectsize);

    /* output sections to file */
    yasm_object_sections_traverse(object, &info, macho_objfmt_output_section);

    fileoff_sections = ftell(f);

    /* Write headers */
    if (fseek(f, 0, SEEK_SET) < 0) {
        yasm__fatal(N_("could not seek on output file"));
        /*@notreached@*/
        return;
    }

    localbuf = info.buf;

    /* header size is common to 32 bit and 64 bit variants */
    if (info.is_64) {
        YASM_WRITE_32_L(localbuf, MH_MAGIC_64); /* magic number */
        /* i386 64-bit ABI */
        YASM_WRITE_32_L(localbuf, CPU_ARCH_ABI64 | CPU_TYPE_I386);
    } else {
        YASM_WRITE_32_L(localbuf, MH_MAGIC);    /* magic number */
        YASM_WRITE_32_L(localbuf, CPU_TYPE_I386);       /* i386 32-bit ABI */
    }
    /* i386 all cpu subtype compatible */
    YASM_WRITE_32_L(localbuf, CPU_SUBTYPE_I386_ALL);
    YASM_WRITE_32_L(localbuf, MH_OBJECT);       /* MACH file type */

    /* calculate number of commands and their size, put to stream */
    head_ncmds = 0;
    head_sizeofcmds = 0;
    if (objfmt_macho->parse_scnum > 0) {
        head_ncmds++;
        head_sizeofcmds +=
            macho_segcmdsize + macho_sectcmdsize * objfmt_macho->parse_scnum;
    }
    if (symtab_count > 0) {
        head_ncmds++;
        head_sizeofcmds += MACHO_SYMCMD_SIZE;
    }

    YASM_WRITE_32_L(localbuf, head_ncmds);
    YASM_WRITE_32_L(localbuf, head_sizeofcmds);
    YASM_WRITE_32_L(localbuf, 0);       /* no flags (yet) */
    if (info.is_64) {
        YASM_WRITE_32_L(localbuf, 0);   /* reserved in 64 bit */
        fileoffset = MACHO_HEADER64_SIZE + head_sizeofcmds;
    } else {
        /* initial offset to first section */
        fileoffset = MACHO_HEADER_SIZE + head_sizeofcmds;
    }

    /* --------------- write segment header command ---------------- */
    YASM_WRITE_32_L(localbuf, macho_segcmd);    /* command LC_SEGMENT */
    /* size of load command including section load commands */
    YASM_WRITE_32_L(localbuf,
                    macho_segcmdsize +
                    macho_sectcmdsize * objfmt_macho->parse_scnum);
    /* in an MH_OBJECT file all sections are in one unnamed (name all zeros)
     * segment (16x0)
     */
    YASM_WRITE_32_L(localbuf, 0);
    YASM_WRITE_32_L(localbuf, 0);
    YASM_WRITE_32_L(localbuf, 0);
    YASM_WRITE_32_L(localbuf, 0);

    /* in-memory offset, in-memory size */
    yasm_intnum_set_uint(val, 0);       /* offset in memory (vmaddr) */
    yasm_intnum_get_sized(val, localbuf, long_int_bytes,
                          ((long_int_bytes) << 3), 0, 0, 0);
    localbuf += long_int_bytes;
    yasm_intnum_set_uint(val, info.vmsize);     /* size in memory (vmsize) */
    yasm_intnum_get_sized(val, localbuf, long_int_bytes,
                          ((long_int_bytes) << 3), 0, 0, 0);
    localbuf += long_int_bytes;
    /* offset in file to first section */
    yasm_intnum_set_uint(val, fileoffset);
    yasm_intnum_get_sized(val, localbuf, long_int_bytes,
                          ((long_int_bytes) << 3), 0, 0, 0);
    localbuf += long_int_bytes;
    yasm_intnum_set_uint(val, info.filesize);   /* overall size in file */
    yasm_intnum_get_sized(val, localbuf, long_int_bytes,
                          ((long_int_bytes) << 3), 0, 0, 0);
    localbuf += long_int_bytes;

    YASM_WRITE_32_L(localbuf, VM_PROT_DEFAULT); /* VM protection, maximum */
    YASM_WRITE_32_L(localbuf, VM_PROT_DEFAULT); /* VM protection, initial */
    /* number of sections */
    YASM_WRITE_32_L(localbuf, objfmt_macho->parse_scnum);
    YASM_WRITE_32_L(localbuf, 0);       /* no flags */

    /* write MACH-O header and segment command to outfile */
    fwrite(info.buf, (size_t) (localbuf - info.buf), 1, f);

    /* next: section headers */
    /* offset to relocs for first section */
    info.rel_base = align32((long)fileoff_sections);
    info.s_reloff = 0;          /* offset for relocs of following sections */
    yasm_object_sections_traverse(object, &info, macho_objfmt_output_secthead);

    localbuf = info.buf;
    /* write out symbol command */
    YASM_WRITE_32_L(localbuf, LC_SYMTAB);       /* cmd == LC_SYMTAB */
    YASM_WRITE_32_L(localbuf, MACHO_SYMCMD_SIZE);
    /* symbol table offset */
    YASM_WRITE_32_L(localbuf, info.rel_base + info.s_reloff);
    YASM_WRITE_32_L(localbuf, symtab_count);    /* number of symbols */

    YASM_WRITE_32_L(localbuf, macho_nlistsize * symtab_count + info.rel_base +
                    info.s_reloff);     /* string table offset */
    YASM_WRITE_32_L(localbuf, info.strlength);  /* string table size */
    /* write symbol command */
    fwrite(info.buf, (size_t)(localbuf - info.buf), 1, f);

    /*printf("num symbols %d, vmsize %d, filesize %d\n",symtab_count,
      info.vmsize, info.filesize ); */

    /* get back to end of raw section data */
    if (fseek(f, (long)fileoff_sections, SEEK_SET) < 0) {
        yasm__fatal(N_("could not seek on output file"));
        /*@notreached@*/
        return;
    }

    /* padding to long boundary */
    if ((info.rel_base - fileoff_sections) > 0) {
        fwrite(pad_data, info.rel_base - fileoff_sections, 1, f);
    }

    /* relocation data */
    yasm_object_sections_traverse(object, &info, macho_objfmt_output_relocs);

    /* symbol table (NLIST) */
    info.indx = 1;              /* restart symbol table indices */
    yasm_symtab_traverse(object->symtab, &info, macho_objfmt_output_symtable);

    /* symbol strings */
    fwrite(pad_data, 1, 1, f);
    yasm_symtab_traverse(object->symtab, &info, macho_objfmt_output_str);

    yasm_intnum_destroy(val);
    yasm_xfree(info.buf);
}

static void
macho_objfmt_destroy(yasm_objfmt *objfmt)
{
    yasm_xfree(objfmt);
}

static void
macho_objfmt_init_new_section(yasm_section *sect, unsigned long line)
{
    yasm_object *object = yasm_section_get_object(sect);
    const char *sectname = yasm_section_get_name(sect);
    yasm_objfmt_macho *objfmt_macho = (yasm_objfmt_macho *)object->objfmt;
    macho_section_data *data;
    yasm_symrec *sym;

    data = yasm_xmalloc(sizeof(macho_section_data));
    data->scnum = objfmt_macho->parse_scnum++;
    data->segname = NULL;
    data->sectname = NULL;
    data->flags = S_REGULAR;
    data->size = 0;
    data->offset = 0;
    data->vmoff = 0;
    data->nreloc = 0;
    data->extreloc = 0;
    yasm_section_add_data(sect, &macho_section_data_cb, data);

    sym = yasm_symtab_define_label(object->symtab, sectname,
                                   yasm_section_bcs_first(sect), 1, line);
    data->sym = sym;
}

static yasm_section *
macho_objfmt_add_default_section(yasm_object *object)
{
    yasm_section *retval;
    macho_section_data *msd;
    int isnew;

    retval = yasm_object_get_general(object, "LC_SEGMENT.__TEXT.__text", 0, 1,
                                     0, &isnew, 0);
    if (isnew) {
        msd = yasm_section_get_data(retval, &macho_section_data_cb);
        msd->segname = yasm__xstrdup("__TEXT");
        msd->sectname = yasm__xstrdup("__text");
        msd->flags = S_ATTR_PURE_INSTRUCTIONS;
        yasm_section_set_align(retval, 0, 0);
        yasm_section_set_default(retval, 1);
    }
    return retval;
}

static /*@observer@*/ /*@null@*/ yasm_section *
macho_objfmt_section_switch(yasm_object *object, yasm_valparamhead *valparams,
                            /*@unused@*/ /*@null@*/
                            yasm_valparamhead *objext_valparams,
                            unsigned long line)
{
    yasm_valparam *vp;
    yasm_section *retval;
    int isnew;
    /*@only@*/ char *f_sectname;
    unsigned long flags;
    unsigned long align;
    int flags_override = 0;
    const char *sectname;
    char *realname;
    int resonly = 0;
    macho_section_data *msd;
    size_t i;

    static const struct {
        const char *in;
        const char *seg;
        const char *sect;
        unsigned long flags;
        unsigned long align;
    } section_name_translation[] = {
        {".text",           "__TEXT", "__text", S_ATTR_PURE_INSTRUCTIONS, 0},
        {".const",          "__TEXT", "__const",        S_REGULAR, 0},
        {".static_const",   "__TEXT", "__static_const", S_REGULAR, 0},
        {".cstring",        "__TEXT", "__cstring",      S_CSTRING_LITERALS, 0},
        {".literal4",       "__TEXT", "__literal4",     S_4BYTE_LITERALS, 4},
        {".literal8",       "__TEXT", "__literal8",     S_8BYTE_LITERALS, 8},
        {".literal16",      "__TEXT", "__literal16",    S_16BYTE_LITERALS, 16},
        {".constructor",    "__TEXT", "__constructor",  S_REGULAR, 0},
        {".destructor",     "__TEXT", "__destructor",   S_REGULAR, 0},
        {".fvmlib_init0",   "__TEXT", "__fvmlib_init0", S_REGULAR, 0},
        {".fvmlib_init1",   "__TEXT", "__fvmlib_init1", S_REGULAR, 0},
        {".mod_init_func",  "__DATA", "__mod_init_func",
            S_MOD_INIT_FUNC_POINTERS, 4},
        {".mod_term_func",  "__DATA", "__mod_term_func",
            S_MOD_TERM_FUNC_POINTERS, 4},
        {".dyld",           "__DATA", "__dyld",         S_REGULAR, 0},
        {".data",           "__DATA", "__data",         S_REGULAR, 0},
        {".static_data",    "__DATA", "__static_data",  S_REGULAR, 0},
        {".const_data",     "__DATA", "__const",        S_REGULAR, 0},
        {".rodata",         "__DATA", "__const",        S_REGULAR, 0},
        {".bss",            "__DATA", "__bss",          S_ZEROFILL, 0},
        {".objc_class_names",   "__TEXT", "__cstring",  S_CSTRING_LITERALS, 0},
        {".objc_meth_var_types","__TEXT", "__cstring",  S_CSTRING_LITERALS, 0},
        {".objc_meth_var_names","__TEXT", "__cstring",  S_CSTRING_LITERALS, 0},
        {".objc_selector_strs", "__OBJC", "__selector_strs",
            S_CSTRING_LITERALS, 0},
        {".objc_class",         "__OBJC", "__class",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_meta_class",    "__OBJC", "__meta_class",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_string_object", "__OBJC", "__string_object",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_protocol",      "__OBJC", "__protocol",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_cat_cls_meth",  "__OBJC", "__cat_cls_meth",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_cat_inst_meth", "__OBJC", "__cat_inst_meth",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_cls_meth",      "__OBJC", "__cls_meth",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_inst_meth",     "__OBJC", "__inst_meth",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_message_refs",  "__OBJC", "__message_refs",
            S_LITERAL_POINTERS|S_ATTR_NO_DEAD_STRIP, 4},
        {".objc_cls_refs",      "__OBJC", "__cls_refs",
            S_LITERAL_POINTERS|S_ATTR_NO_DEAD_STRIP, 4},
        {".objc_module_info",   "__OBJC", "__module_info",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_symbols",       "__OBJC", "__symbols",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_category",      "__OBJC", "__category",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_class_vars",    "__OBJC", "__class_vars",
            S_ATTR_NO_DEAD_STRIP, 0},
        {".objc_instance_vars", "__OBJC", "__instance_vars",
            S_ATTR_NO_DEAD_STRIP, 0}
    };

    struct macho_section_switch_data {
        /*@only@*/ /*@null@*/ char *f_segname;
        /*@only@*/ /*@null@*/ yasm_intnum *align_intn;
    } data;

    static const yasm_dir_help help[] = {
        { "segname", 1, yasm_dir_helper_string,
          offsetof(struct macho_section_switch_data, f_segname), 0 },
        { "align", 1, yasm_dir_helper_intn,
          offsetof(struct macho_section_switch_data, align_intn), 0 }
    };

    data.f_segname = NULL;
    data.align_intn = NULL;

    vp = yasm_vps_first(valparams);
    sectname = yasm_vp_string(vp);
    if (!sectname)
        return NULL;
    vp = yasm_vps_next(vp);

    /* translate .text,.data,.bss to __text,__data,__bss... */
    for (i=0; i<NELEMS(section_name_translation); i++) {
        if (yasm__strcasecmp(sectname, section_name_translation[i].in) == 0)
            break;
    }

    if (i == NELEMS(section_name_translation)) {
        const char *s;
        if (vp && !vp->val && (s = yasm_vp_string(vp))) {
            /* Treat as SEGNAME, SECTNAME */
            if (strlen(sectname) > 16)
                yasm_warn_set(YASM_WARN_GENERAL,
                    N_("segment name is too long, max 16 chars; truncating"));
            data.f_segname = yasm__xstrndup(sectname, 16);
            if (strlen(s) > 16)
                yasm_warn_set(YASM_WARN_GENERAL,
                    N_("section name is too long, max 16 chars; truncating"));
            f_sectname = yasm__xstrndup(s, 16);
            flags = S_REGULAR;
            align = 0;

            sectname = s;
            vp = yasm_vps_next(vp);
        } else {
            data.f_segname = NULL;
            if (strlen(sectname) > 16)
                yasm_warn_set(YASM_WARN_GENERAL,
                    N_("section name is too long, max 16 chars; truncating"));
            f_sectname = yasm__xstrndup(sectname, 16);
            flags = S_ATTR_SOME_INSTRUCTIONS;
            align = 0;
        }
    } else {
        data.f_segname = yasm__xstrdup(section_name_translation[i].seg);
        f_sectname = yasm__xstrdup(section_name_translation[i].sect);
        flags = section_name_translation[i].flags;
        align = section_name_translation[i].align;
    }

    flags_override = yasm_dir_helper(object, vp, line, help, NELEMS(help),
                                     &data, yasm_dir_helper_valparam_warn);
    if (flags_override < 0)
        return NULL;    /* error occurred */

    if (data.align_intn) {
        align = yasm_intnum_get_uint(data.align_intn);
        yasm_intnum_destroy(data.align_intn);

        /* Alignments must be a power of two. */
        if (!is_exp2(align)) {
            yasm_error_set(YASM_ERROR_VALUE,
                           N_("argument to `%s' is not a power of two"),
                           vp->val);
            return NULL;
        }

        /* Check to see if alignment is supported size */
        if (align > 16384) {
            yasm_error_set(YASM_ERROR_VALUE,
                N_("macho implementation does not support alignments > 16384"));
            return NULL;
        }
    }

    if (!data.f_segname) {
        yasm_warn_set(YASM_WARN_GENERAL,
                      N_("Unknown section name, defaulting to __TEXT segment"));
        data.f_segname = yasm__xstrdup("__TEXT");
    }

    /* Build a unique sectname from f_segname and f_sectname. */
    realname = yasm_xmalloc(strlen("LC_SEGMENT") + 1 + strlen(data.f_segname) + 1 +
                            strlen(f_sectname) + 1);
    sprintf(realname, "LC_SEGMENT.%s.%s", data.f_segname, f_sectname);
    retval = yasm_object_get_general(object, realname, align, 1, resonly,
                                     &isnew, line);
    yasm_xfree(realname);

    msd = yasm_section_get_data(retval, &macho_section_data_cb);

    if (isnew || yasm_section_is_default(retval)) {
        yasm_section_set_default(retval, 0);
        msd->segname = data.f_segname;
        msd->sectname = f_sectname;
        msd->flags = flags;
        yasm_section_set_align(retval, align, line);
    } else if (flags_override) {
        /* align is the only value used from overrides. */
        if (yasm_section_get_align(retval) != align) {
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("section flags ignored on section redeclaration"));
        }
    }
    return retval;
}

static /*@observer@*/ /*@null@*/ yasm_symrec *
macho_objfmt_get_special_sym(yasm_object *object, const char *name,
                             const char *parser)
{
    yasm_objfmt_macho *objfmt_macho = (yasm_objfmt_macho *)object->objfmt;
    if (yasm__strcasecmp(name, "gotpcrel") == 0) {
        return objfmt_macho->gotpcrel_sym;
    }
    return NULL;
}

static void
macho_section_data_destroy(void *data)
{
    macho_section_data *msd = (macho_section_data *) data;
    yasm_xfree(msd->segname);
    yasm_xfree(msd->sectname);
    yasm_xfree(data);
}

static void
macho_section_data_print(void *data, FILE *f, int indent_level)
{
    macho_section_data *msd = (macho_section_data *) data;

    fprintf(f, "%*ssym=\n", indent_level, "");
    yasm_symrec_print(msd->sym, f, indent_level + 1);
    fprintf(f, "%*sscnum=%ld\n", indent_level, "", msd->scnum);
    fprintf(f, "%*sflags=0x%lx\n", indent_level, "", msd->flags);
    fprintf(f, "%*ssize=%lu\n", indent_level, "", msd->size);
    fprintf(f, "%*snreloc=%lu\n", indent_level, "", msd->nreloc);
    fprintf(f, "%*soffset=%lu\n", indent_level, "", msd->offset);
    fprintf(f, "%*sextreloc=%u\n", indent_level, "", msd->extreloc);
}

static void
macho_symrec_data_destroy(void *data)
{
    yasm_xfree(data);
}

static void
macho_symrec_data_print(void *data, FILE *f, int indent_level)
{
    macho_symrec_data *msd = (macho_symrec_data *)data;

    fprintf(f, "%*sindex=%ld\n", indent_level, "", msd->index);
    fprintf(f, "%*svalue=", indent_level, "");
    if (msd->value)
        fprintf(f, "%ld\n", yasm_intnum_get_int(msd->value));
    else
        fprintf(f, "nil\n");
}


/* Define valid debug formats to use with this object format */
static const char *macho_objfmt_dbgfmt_keywords[] = {
    "null",
    NULL
};

/* Define objfmt structure -- see objfmt.h for details */
yasm_objfmt_module yasm_macho_LTX_objfmt = {
    "Mac OS X ABI Mach-O File Format",
    "macho",
    "o",
    32,
    0,
    macho_objfmt_dbgfmt_keywords,
    "null",
    NULL,   /* no directives */
    NULL,   /* no standard macros */
    macho_objfmt_create,
    macho_objfmt_output,
    macho_objfmt_destroy,
    macho_objfmt_add_default_section,
    macho_objfmt_init_new_section,
    macho_objfmt_section_switch,
    macho_objfmt_get_special_sym
};

yasm_objfmt_module yasm_macho32_LTX_objfmt = {
    "Mac OS X ABI Mach-O File Format (32-bit)",
    "macho32",
    "o",
    32,
    0,
    macho_objfmt_dbgfmt_keywords,
    "null",
    NULL,   /* no directives */
    NULL,   /* no standard macros */
    macho32_objfmt_create,
    macho_objfmt_output,
    macho_objfmt_destroy,
    macho_objfmt_add_default_section,
    macho_objfmt_init_new_section,
    macho_objfmt_section_switch,
    macho_objfmt_get_special_sym
};

yasm_objfmt_module yasm_macho64_LTX_objfmt = {
    "Mac OS X ABI Mach-O File Format (64-bit)",
    "macho64",
    "o",
    64,
    0,
    macho_objfmt_dbgfmt_keywords,
    "null",
    NULL,   /* no directives */
    NULL,   /* no standard macros */
    macho64_objfmt_create,
    macho_objfmt_output,
    macho_objfmt_destroy,
    macho_objfmt_add_default_section,
    macho_objfmt_init_new_section,
    macho_objfmt_section_switch,
    macho_objfmt_get_special_sym
};
