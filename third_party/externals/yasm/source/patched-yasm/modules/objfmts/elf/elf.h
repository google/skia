/*
 * ELF object format helpers
 *
 *  Copyright (C) 2003-2007  Michael Urman
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

#ifndef ELF_H_INCLUDED
#define ELF_H_INCLUDED

typedef struct elf_reloc_entry elf_reloc_entry;
typedef struct elf_reloc_head elf_reloc_head;
typedef struct elf_secthead elf_secthead;
typedef struct elf_strtab_entry elf_strtab_entry;
typedef struct elf_strtab_head elf_strtab_head;
typedef struct elf_symtab_entry elf_symtab_entry;
typedef struct elf_symtab_head elf_symtab_head;

typedef struct elf_machine_handler elf_machine_handler;

typedef unsigned long   elf_address;
typedef unsigned long   elf_offset;
typedef unsigned long   elf_size;
typedef unsigned long   elf_section_info;

typedef enum {
    ET_NONE = 0,
    ET_REL = 1,                                 /* Relocatable */
    ET_EXEC = 2,                                /* Executable */
    ET_DYN = 3,                                 /* Shared object */
    ET_CORE = 4,                                /* Core */
    ET_LOOS = 0xfe00,                           /* Environment specific */
    ET_HIOS = 0xfeff,
    ET_LOPROC = 0xff00,                         /* Processor specific */
    ET_HIPROC = 0xffff
} elf_file_type;

typedef enum {
    EM_NONE = 0,
    EM_M32 = 1,                                 /* AT&T WE 32100 */
    EM_SPARC = 2,                               /* SPARC */
    EM_386 = 3,                                 /* Intel 80386 */
    EM_68K = 4,                                 /* Motorola 68000 */
    EM_88K = 5,                                 /* Motorola 88000 */
    EM_860 = 7,                                 /* Intel 80860 */
    EM_MIPS = 8,                                /* MIPS RS3000 */

    EM_S370 = 9,                                /* IBM System/370 */
    EM_MIPS_RS4_BE = 10,                        /* MIPS R4000 Big-Endian (dep)*/
    EM_PARISC = 15,                             /* HPPA */
    EM_SPARC32PLUS = 18,                        /* SPARC v8plus */
    EM_PPC = 20,                                /* PowerPC 32-bit */
    EM_PPC64 = 21,                              /* PowerPC 64-bit */
    EM_ARM = 40,                                /* ARM */
    EM_SPARCV9 = 43,                            /* SPARC v9 64-bit */
    EM_IA_64 = 50,                              /* Intel IA-64 */
    EM_X86_64 = 62,                             /* AMD x86-64 */
    EM_ALPHA = 0x9026                           /* Alpha (no ABI) */
} elf_machine;

typedef enum {
    ELFMAG0 = 0x7f,
    ELFMAG1 = 0x45,
    ELFMAG2 = 0x4c,
    ELFMAG3 = 0x46 
} elf_magic;

typedef enum {
    EV_NONE = 0,                                /* invalid */
    EV_CURRENT = 1                              /* current */
} elf_version;

typedef enum {
    EI_MAG0 = 0,                                /* File id */
    EI_MAG1 = 1,
    EI_MAG2 = 2,
    EI_MAG3 = 3,
    EI_CLASS = 4,                               /* File class */
    EI_DATA = 5,                                /* Data encoding */
    EI_VERSION = 6,                             /* File version */
    EI_OSABI = 7,                               /* OS and ABI */
    EI_ABIVERSION = 8,                          /* version of ABI */

    EI_PAD = 9,                                 /* Pad to end; start here */
    EI_NIDENT = 16                              /* Sizeof e_ident[] */
} elf_identification_index;

typedef enum {
    ELFOSABI_SYSV = 0,                          /* System V ABI */
    ELFOSABI_HPUX = 1,                          /* HP-UX os */
    ELFOSABI_STANDALONE = 255                   /* Standalone / embedded app */
} elf_osabi_index;

typedef enum {
    ELFCLASSNONE = 0,                           /* invalid */
    ELFCLASS32 = 1,                             /* 32-bit */
    ELFCLASS64 = 2                              /* 64-bit */
} elf_class;

typedef enum {
    ELFDATANONE = 0,
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2
} elf_data_encoding;

/* elf section types - index of semantics */
typedef enum {
    SHT_NULL = 0,               /* inactive section - no associated data */
    SHT_PROGBITS = 1,           /* defined by program for its own meaning */
    SHT_SYMTAB = 2,             /* symbol table (primarily) for linking */
    SHT_STRTAB = 3,             /* string table - symbols need names */
    SHT_RELA = 4,               /* relocation entries w/ explicit addends */
    SHT_HASH = 5,               /* symbol hash table - for dynamic linking */
    SHT_DYNAMIC = 6,            /* information for dynamic linking */
    SHT_NOTE = 7,               /* extra data marking the file somehow */
    SHT_NOBITS = 8,             /* no stored data, but occupies runtime space */
    SHT_REL = 9,                /* relocations entries w/o explicit addends */
    SHT_SHLIB = 10,             /* reserved; unspecified semantics */
    SHT_DYNSYM = 11,            /* like symtab, but more for dynamic linking */

    SHT_LOOS = 0x60000000,      /* reserved for environment specific use */
    SHT_HIOS = 0x6fffffff,
    SHT_LOPROC = 0x70000000,    /* reserved for processor specific semantics */
    SHT_HIPROC = 0x7fffffff/*,
    SHT_LOUSER = 0x80000000,*/  /* reserved for applications; safe */
    /*SHT_HIUSER = 0xffffffff*/
} elf_section_type;

/* elf section flags - bitfield of attributes */
typedef enum {
    SHF_WRITE = 0x1,            /* data should be writable at runtime */
    SHF_ALLOC = 0x2,            /* occupies memory at runtime */
    SHF_EXECINSTR = 0x4,        /* contains machine instructions */
    SHF_MERGE = 0x10,           /* data can be merged */
    SHF_STRINGS = 0x20,         /* contains 0-terminated strings */
    SHF_GROUP = 0x200,          /* member of a section group */
    SHF_TLS = 0x400,            /* thread local storage */
    SHF_MASKOS = 0x0f000000/*,*//* environment specific use */
    /*SHF_MASKPROC = 0xf0000000*/       /* bits reserved for processor specific needs */
} elf_section_flags;

/* elf section index - just the special ones */
typedef enum {
    SHN_UNDEF = 0,              /* undefined symbol; requires other global */
    SHN_LORESERVE = 0xff00,     /* reserved for various semantics */
    SHN_LOPROC = 0xff00,        /* reserved for processor specific semantics */
    SHN_HIPROC = 0xff1f,
    SHN_LOOS = 0xff20,          /* reserved for environment specific use */
    SHN_HIOS = 0xff3f,
    SHN_ABS = 0xfff1,           /* associated symbols don't change on reloc */
    SHN_COMMON = 0xfff2,        /* associated symbols refer to unallocated */
    SHN_HIRESERVE = 0xffff
} elf_section_index;

/* elf symbol binding - index of visibility/behavior */
typedef enum {
    STB_LOCAL = 0,              /* invisible outside defining file */
    STB_GLOBAL = 1,             /* visible to all combined object files */
    STB_WEAK = 2,               /* global but lower precedence */

    STB_LOOS = 10,              /* Environment specific use */
    STB_HIOS = 12,
    STB_LOPROC = 13,            /* reserved for processor specific semantics */
    STB_HIPROC = 15
} elf_symbol_binding;

/* elf symbol type - index of classifications */
typedef enum {
    STT_NOTYPE = 0,             /* type not specified */
    STT_OBJECT = 1,             /* data object such as a variable, array, etc */
    STT_FUNC = 2,               /* a function or executable code */
    STT_SECTION = 3,            /* a section: often for relocation, STB_LOCAL */
    STT_FILE = 4,               /* often source filename: STB_LOCAL, SHN_ABS */
    STT_COMMON = 5,             /* Uninitialized common block. */
    STT_TLS = 6,                /* TLS object. */
    STT_NUM = 7,

    STT_LOOS = 10,              /* Environment specific use */
    STT_HIOS = 12,
    STT_LOPROC = 13,            /* reserved for processor specific semantics */
    STT_HIPROC = 15
} elf_symbol_type;

typedef enum {
    STN_UNDEF = 0
} elf_symbol_index;

/* elf symbol visibility - lower two bits of OTHER field */
typedef enum {
    STV_DEFAULT = 0,            /* Default symbol visibility rules */
    STV_INTERNAL = 1,           /* Processor specific hidden class */
    STV_HIDDEN = 2,             /* Sym unavailable in other modules */
    STV_PROTECTED = 3           /* Not preemptable, not exported */
} elf_symbol_vis;


/* internal only object definitions */
#ifdef YASM_OBJFMT_ELF_INTERNAL

#define ELF_VISIBILITY_MASK             0x03
#define ELF_ST_VISIBILITY(v)            ((v) & ELF_VISIBILITY_MASK)

#define ELF32_ST_INFO(bind, type)       (((bind) << 4) + ((type) & 0xf))
#define ELF32_R_INFO(s,t)               (((s)<<8)+(unsigned char)(t))
#define ELF32_ST_OTHER(vis)             ELF_ST_VISIBILITY(vis)

#define ELF64_ST_INFO(bind, type)       (((bind) << 4) + ((type) & 0xf))
#define ELF64_R_INFO(s,t)               (((s)<<32) + ((t) & 0xffffffffL))
#define ELF64_ST_OTHER(vis)             ELF_ST_VISIBILITY(vis)

#define EHDR32_SIZE 52
#define EHDR64_SIZE 64
#define EHDR_MAXSIZE 64

#define SHDR32_SIZE 40
#define SHDR64_SIZE 64
#define SHDR_MAXSIZE 64

#define SYMTAB32_SIZE 16
#define SYMTAB64_SIZE 24
#define SYMTAB_MAXSIZE 24

#define SYMTAB32_ALIGN 4
#define SYMTAB64_ALIGN 8

#define RELOC32_SIZE 8
#define RELOC32A_SIZE 12
#define RELOC64_SIZE 16
#define RELOC64A_SIZE 24
#define RELOC_MAXSIZE 24

#define RELOC32_ALIGN 4
#define RELOC64_ALIGN 8


/* elf relocation type - index of semantics
 *
 * A = Addend (r_addend for RELA, value at location for REL)
 * B = Base address
 * G = Offset into global offset table (GOT)
 * GOT = Address of the global offset table (GOT)
 * L = Location of procedure linkage table (PLT)
 * P = Location of location being relocated (r_offset)
 * S = Value of symbol
 */
typedef enum {
    R_386_NONE = 0,             /* none */
    R_386_32 = 1,               /* word32, S + A */
    R_386_PC32 = 2,             /* word32, S + A - P */
    R_386_GOT32 = 3,            /* word32, G + A - P */
    R_386_PLT32 = 4,            /* word32, L + A - P */
    R_386_COPY = 5,             /* none */
    R_386_GLOB_DAT = 6,         /* word32, S */
    R_386_JMP_SLOT = 7,         /* word32, S */
    R_386_RELATIVE = 8,         /* word32, B + A */
    R_386_GOTOFF = 9,           /* word32, S + A - GOT */
    R_386_GOTPC = 10,           /* word32, GOT + A - P */
    R_386_TLS_TPOFF = 14,       /* Negative offset in static TLS block (GNU
                                   version) */
    R_386_TLS_IE = 15,          /* Absolute address of GOT entry for negative
                                   static TLS block offset */
    R_386_TLS_GOTIE = 16,       /* GOT entry for negative static TLS block
                                   offset */
    R_386_TLS_LE = 17,          /* Negative offset relative to static TLS
                                   (GNU version) */
    R_386_TLS_GD = 18,          /* Direct 32 bit for GNU version of GD TLS */
    R_386_TLS_LDM = 19,         /* Direct 32 bit for GNU version of LD TLS
                                   in LE code */
    R_386_16 = 20,              /* word16, S + A (GNU extension) */
    R_386_PC16 = 21,            /* word16, S + A - P (GNU extension) */
    R_386_8 = 22,               /* word8, S + A (GNU extension) */
    R_386_PC8 = 23,             /* word8, S + A - P (GNU extension) */
    R_386_TLS_GD_32 = 24,       /* Direct 32 bit for GD TLS */
    R_386_TLS_GD_PUSH = 25,     /* Tag for pushl in GD TLS code */
    R_386_TLS_GD_CALL = 26,     /* Relocation for call to */
    R_386_TLS_GD_POP = 27,      /* Tag for popl in GD TLS code */
    R_386_TLS_LDM_32 = 28,      /* Direct 32 bit for local dynamic code */
    R_386_TLS_LDM_PUSH = 29,    /* Tag for pushl in LDM TLS code */
    R_386_TLS_LDM_CALL = 30,    /* Relocation for call to */
    R_386_TLS_LDM_POP = 31,     /* Tag for popl in LDM TLS code */
    R_386_TLS_LDO_32 = 32,      /* Offset relative to TLS block */
    R_386_TLS_IE_32 = 33,       /* GOT entry for static TLS block */
    R_386_TLS_LE_32 = 34,       /* Offset relative to static TLS block */
    R_386_TLS_DTPMOD32 = 35,    /* ID of module containing symbol */
    R_386_TLS_DTPOFF32 = 36,    /* Offset in TLS block */
    R_386_TLS_TPOFF32 = 37,     /* Offset in static TLS block */
    R_386_TLS_GOTDESC = 39,
    R_386_TLS_DESC_CALL = 40,
    R_386_TLS_DESC = 41
} elf_386_relocation_type;

typedef enum {
    R_X86_64_NONE = 0,          /* none */
    R_X86_64_64 = 1,            /* word64, S + A */
    R_X86_64_PC32 = 2,          /* word32, S + A - P */
    R_X86_64_GOT32 = 3,         /* word32, G + A */
    R_X86_64_PLT32 = 4,         /* word32, L + A - P */
    R_X86_64_COPY = 5,          /* none */
    R_X86_64_GLOB_DAT = 6,      /* word64, S, set GOT entry to data address */
    R_X86_64_JMP_SLOT = 7,      /* word64, S, set GOT entry to code address */
    R_X86_64_RELATIVE = 8,      /* word64, B + A */
    R_X86_64_GOTPCREL = 9,      /* word32, G + GOT + A - P */
    R_X86_64_32 = 10,           /* word32 (zero extend), S + A */
    R_X86_64_32S = 11,          /* word32 (sign extend), S + A */
    R_X86_64_16 = 12,           /* word16, S + A */
    R_X86_64_PC16 = 13,         /* word16, S + A - P */
    R_X86_64_8 = 14,            /* word8, S + A */
    R_X86_64_PC8 = 15,          /* word8, S + A - P */
    R_X86_64_DPTMOD64 = 16,     /* word64, ID of module containing symbol */
    R_X86_64_DTPOFF64 = 17,     /* word64, offset in TLS block */
    R_X86_64_TPOFF64 = 18,      /* word64, offset in initial TLS block */
    R_X86_64_TLSGD = 19,        /* word32, PC-rel offset to GD GOT block */
    R_X86_64_TLSLD = 20,        /* word32, PC-rel offset to LD GOT block */
    R_X86_64_DTPOFF32 = 21,     /* word32, offset to TLS block */
    R_X86_64_GOTTPOFF = 22,     /* word32, PC-rel offset to IE GOT entry */
    R_X86_64_TPOFF32 = 23,      /* word32, offset in initial TLS block */
    R_X86_64_PC64 = 24,         /* word64, PC relative */
    R_X86_64_GOTOFF64 = 25,     /* word64, offset to GOT */
    R_X86_64_GOTPC32 = 26,      /* word32, signed pc relative to GOT */
    R_X86_64_GOT64 = 27,        /* word64, GOT entry offset */
    R_X86_64_GOTPCREL64 = 28,   /* word64, signed pc relative to GOT entry */
    R_X86_64_GOTPC64 = 29,      /* word64, signed pc relative to GOT */
    R_X86_64_GOTPLT64 = 30,     /* like GOT64, but indicates PLT entry needed */
    R_X86_64_PLTOFF64 = 31,     /* word64, GOT relative offset to PLT entry */
    R_X86_64_GOTPC32_TLSDESC = 34, /* GOT offset for TLS descriptor */
    R_X86_64_TLSDESC_CALL = 35, /* Marker for call through TLS descriptor */
    R_X86_64_TLSDESC = 36       /* TLS descriptor */
} elf_x86_64_relocation_type;

struct elf_secthead {
    elf_section_type     type;
    elf_section_flags    flags;
    elf_address          offset;
    yasm_intnum         *size;
    elf_section_index    link;
    elf_section_info     info;      /* see note ESD1 */
    unsigned long        align;
    elf_size             entsize;

    yasm_symrec         *sym;
    elf_strtab_entry    *name;
    elf_section_index    index;

    elf_strtab_entry    *rel_name;
    elf_section_index    rel_index;
    elf_address          rel_offset;
    unsigned long        nreloc;
};

/* Note ESD1:
 *   for section types SHT_REL, SHT_RELA:
 *     link -> index of associated symbol table
 *     info -> index of relocated section
 *   for section types SHT_SYMTAB, SHT_DYNSYM:
 *     link -> index of associated string table
 *     info -> 1+index of last "local symbol" (bind == STB_LOCAL)
 *  (for section type SHT_DNAMIC:
 *     link -> index of string table
 *     info -> 0 )
 *  (for section type SHT_HASH:
 *     link -> index of symbol table to which hash applies
 *     info -> 0 )
 *   for all others:
 *     link -> SHN_UNDEF
 *     info -> 0
 */

struct elf_reloc_entry {
    yasm_reloc           reloc;
    int                  rtype_rel;
    size_t               valsize;
    yasm_intnum         *addend;
    /*@null@*/ yasm_symrec *wrt;
    int                  is_GOT_sym;
};

STAILQ_HEAD(elf_strtab_head, elf_strtab_entry);
struct elf_strtab_entry {
    STAILQ_ENTRY(elf_strtab_entry) qlink;
    unsigned long        index;
    char                *str;
};

STAILQ_HEAD(elf_symtab_head, elf_symtab_entry);
struct elf_symtab_entry {
    STAILQ_ENTRY(elf_symtab_entry) qlink;
    int                 in_table;
    yasm_symrec         *sym;
    yasm_section        *sect;
    elf_strtab_entry    *name;
    elf_address          value;
    /*@dependent@*/ yasm_expr *xsize;
    elf_size             size;
    elf_section_index    index;
    elf_symbol_binding   bind;
    elf_symbol_type      type;
    elf_symbol_vis       vis;
    elf_symbol_index     symindex;
};

#endif /* defined(YASM_OBJFMT_ELF_INTERNAL) */

extern const yasm_assoc_data_callback elf_section_data;
extern const yasm_assoc_data_callback elf_symrec_data;
extern const yasm_assoc_data_callback elf_ssym_symrec_data;


const elf_machine_handler *elf_set_arch(struct yasm_arch *arch,
                                        yasm_symtab *symtab,
                                        int bits_pref);

yasm_symrec *elf_get_special_sym(const char *name, const char *parser);

/* reloc functions */
int elf_is_wrt_sym_relative(yasm_symrec *wrt);
int elf_is_wrt_pos_adjusted(yasm_symrec *wrt);
elf_reloc_entry *elf_reloc_entry_create(yasm_symrec *sym,
                                        /*@null@*/ yasm_symrec *wrt,
                                        yasm_intnum *addr,
                                        int rel,
                                        size_t valsize,
                                        int is_GOT_sym);
void elf_reloc_entry_destroy(void *entry);

/* strtab functions */
elf_strtab_entry *elf_strtab_entry_create(const char *str);
void elf_strtab_entry_set_str(elf_strtab_entry *entry, const char *str);
elf_strtab_head *elf_strtab_create(void);
elf_strtab_entry *elf_strtab_append_str(elf_strtab_head *head, const char *str);
void elf_strtab_destroy(elf_strtab_head *head);
unsigned long elf_strtab_output_to_file(FILE *f, elf_strtab_head *head);

/* symtab functions */
elf_symtab_entry *elf_symtab_entry_create(elf_strtab_entry *name,
                                          struct yasm_symrec *sym);
elf_symtab_head *elf_symtab_create(void);
void elf_symtab_append_entry(elf_symtab_head *symtab, elf_symtab_entry *entry);
void elf_symtab_insert_local_sym(elf_symtab_head *symtab,
                                 elf_symtab_entry *entry);
void elf_symtab_destroy(elf_symtab_head *head);
unsigned long elf_symtab_assign_indices(elf_symtab_head *symtab);
unsigned long elf_symtab_write_to_file(FILE *f, elf_symtab_head *symtab,
                                       yasm_errwarns *errwarns);
void elf_symtab_set_nonzero(elf_symtab_entry    *entry,
                            struct yasm_section *sect,
                            elf_section_index    sectidx,
                            elf_symbol_binding   bind,
                            elf_symbol_type      type,
                            struct yasm_expr    *size,
                            elf_address         *value);
void elf_sym_set_visibility(elf_symtab_entry    *entry,
                            elf_symbol_vis       vis);
void elf_sym_set_type(elf_symtab_entry *entry, elf_symbol_type type);
void elf_sym_set_size(elf_symtab_entry *entry, struct yasm_expr *size);
int elf_sym_in_table(elf_symtab_entry *entry);

/* section header functions */
elf_secthead *elf_secthead_create(elf_strtab_entry      *name,
                                  elf_section_type      type,
                                  elf_section_flags     flags,
                                  elf_address           offset,
                                  elf_size              size);
void elf_secthead_destroy(elf_secthead *esd);
unsigned long elf_secthead_write_to_file(FILE *f, elf_secthead *esd,
                                         elf_section_index sindex);
void elf_secthead_append_reloc(yasm_section *sect, elf_secthead *shead,
                               elf_reloc_entry *reloc);
elf_section_type elf_secthead_get_type(elf_secthead *shead);
void elf_secthead_set_typeflags(elf_secthead *shead, elf_section_type type,
                                elf_section_flags flags);
int elf_secthead_is_empty(elf_secthead *shead);
struct yasm_symrec *elf_secthead_get_sym(elf_secthead *shead);
unsigned long elf_secthead_get_align(const elf_secthead *shead);
unsigned long elf_secthead_set_align(elf_secthead *shead, unsigned long align);
elf_section_index elf_secthead_get_index(elf_secthead *shead);
elf_section_info elf_secthead_set_info(elf_secthead *shead,
                                       elf_section_info info);
elf_section_index elf_secthead_set_index(elf_secthead *shead,
                                         elf_section_index sectidx);
elf_section_index elf_secthead_set_link(elf_secthead *shead,
                                        elf_section_index link);
elf_section_index elf_secthead_set_rel_index(elf_secthead *shead,
                                             elf_section_index sectidx);
elf_strtab_entry *elf_secthead_set_rel_name(elf_secthead *shead,
                                            elf_strtab_entry *entry);
elf_size elf_secthead_set_entsize(elf_secthead *shead, elf_size size);
struct yasm_symrec *elf_secthead_set_sym(elf_secthead *shead,
                                         struct yasm_symrec *sym);
void elf_secthead_add_size(elf_secthead *shead, yasm_intnum *size);
char *elf_secthead_name_reloc_section(const char *basesect);
void elf_handle_reloc_addend(yasm_intnum *intn,
                             elf_reloc_entry *reloc,
                             unsigned long offset);
unsigned long elf_secthead_write_rel_to_file(FILE *f, elf_section_index symtab,
                                             yasm_section *sect,
                                             elf_secthead *esd,
                                             elf_section_index sindex);
unsigned long elf_secthead_write_relocs_to_file(FILE *f, yasm_section *sect,
                                                elf_secthead *shead,
                                                yasm_errwarns *errwarns);
long elf_secthead_set_file_offset(elf_secthead *shead, long pos);

/* program header function */
unsigned long
elf_proghead_get_size(void);
unsigned long
elf_proghead_write_to_file(FILE *f,
                           elf_offset secthead_addr,
                           unsigned long secthead_count,
                           elf_section_index shstrtab_index);

#endif /* ELF_H_INCLUDED */
