/*
 * ELF object format helpers - x86:amd64
 *
 *  Copyright (C) 2004-2007  Michael Urman
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
#define YASM_OBJFMT_ELF_INTERNAL
#include "elf.h"
#include "elf-machine.h"

static elf_machine_ssym elf_x86_amd64_ssyms[] = {
    {"pltoff",      ELF_SSYM_SYM_RELATIVE,  R_X86_64_PLTOFF64,  64},
    {"plt",         ELF_SSYM_SYM_RELATIVE,  R_X86_64_PLT32,     32},
    {"gotplt",      ELF_SSYM_SYM_RELATIVE,  R_X86_64_GOTPLT64,  64},
    {"gotoff",      ELF_SSYM_SYM_RELATIVE,  R_X86_64_GOTOFF64,  64},
    {"gotpcrel",    ELF_SSYM_SYM_RELATIVE,  R_X86_64_GOTPCREL,  32},
    {"tlsgd",       ELF_SSYM_SYM_RELATIVE|ELF_SSYM_THREAD_LOCAL,
                    R_X86_64_TLSGD,     32},
    {"tlsld",       ELF_SSYM_SYM_RELATIVE|ELF_SSYM_THREAD_LOCAL,
                    R_X86_64_TLSLD,     32},
    {"gottpoff",    ELF_SSYM_SYM_RELATIVE|ELF_SSYM_THREAD_LOCAL,
                    R_X86_64_GOTTPOFF,  32},
    {"tpoff",       ELF_SSYM_SYM_RELATIVE|ELF_SSYM_THREAD_LOCAL,
                    R_X86_64_TPOFF32,   32},
    {"dtpoff",      ELF_SSYM_SYM_RELATIVE|ELF_SSYM_THREAD_LOCAL,
                    R_X86_64_DTPOFF32,  32},
    {"got",         ELF_SSYM_SYM_RELATIVE,  R_X86_64_GOT32,     32},
    {"tlsdesc",     ELF_SSYM_SYM_RELATIVE|ELF_SSYM_THREAD_LOCAL,
                    R_X86_64_GOTPC32_TLSDESC,   32},
    {"tlscall",     ELF_SSYM_SYM_RELATIVE|ELF_SSYM_THREAD_LOCAL,
                    R_X86_64_TLSDESC_CALL,      32}
};

static int
elf_x86_amd64_accepts_reloc(size_t val, yasm_symrec *wrt)
{
    if (wrt) {
        const elf_machine_ssym *ssym = (elf_machine_ssym *)
            yasm_symrec_get_data(wrt, &elf_ssym_symrec_data);
        if (!ssym || val != ssym->size)
            return 0;
        return 1;
    }
    return (val&(val-1)) ? 0 : ((val & (8|16|32|64)) != 0);
}

static void
elf_x86_amd64_write_symtab_entry(unsigned char *bufp,
                                 elf_symtab_entry *entry,
                                 yasm_intnum *value_intn,
                                 yasm_intnum *size_intn)
{
    YASM_WRITE_32_L(bufp, entry->name ? entry->name->index : 0);
    YASM_WRITE_8(bufp, ELF64_ST_INFO(entry->bind, entry->type));
    YASM_WRITE_8(bufp, ELF64_ST_OTHER(entry->vis));
    if (entry->sect) {
        elf_secthead *shead =
            yasm_section_get_data(entry->sect, &elf_section_data);
        if (!shead)
            yasm_internal_error(N_("symbol references section without data"));
        YASM_WRITE_16_L(bufp, shead->index);
    } else {
        YASM_WRITE_16_L(bufp, entry->index);
    }
    YASM_WRITE_64I_L(bufp, value_intn);
    YASM_WRITE_64I_L(bufp, size_intn);
}

static void
elf_x86_amd64_write_secthead(unsigned char *bufp, elf_secthead *shead)
{
    YASM_WRITE_32_L(bufp, shead->name ? shead->name->index : 0);
    YASM_WRITE_32_L(bufp, shead->type);
    YASM_WRITE_64Z_L(bufp, shead->flags);
    YASM_WRITE_64Z_L(bufp, 0);          /* vmem address */
    YASM_WRITE_64Z_L(bufp, shead->offset);
    YASM_WRITE_64I_L(bufp, shead->size);

    YASM_WRITE_32_L(bufp, shead->link);
    YASM_WRITE_32_L(bufp, shead->info);

    YASM_WRITE_64Z_L(bufp, shead->align);
    YASM_WRITE_64Z_L(bufp, shead->entsize);
}

static void
elf_x86_amd64_write_secthead_rel(unsigned char *bufp,
                                 elf_secthead *shead,
                                 elf_section_index symtab_idx,
                                 elf_section_index sindex)
{
    yasm_intnum *nreloc;
    yasm_intnum *relocsize;

    YASM_WRITE_32_L(bufp, shead->rel_name ? shead->rel_name->index : 0);
    YASM_WRITE_32_L(bufp, SHT_RELA);
    YASM_WRITE_64Z_L(bufp, 0);
    YASM_WRITE_64Z_L(bufp, 0);
    YASM_WRITE_64Z_L(bufp, shead->rel_offset);

    nreloc = yasm_intnum_create_uint(shead->nreloc);
    relocsize = yasm_intnum_create_uint(RELOC64A_SIZE);
    yasm_intnum_calc(relocsize, YASM_EXPR_MUL, nreloc);
    YASM_WRITE_64I_L(bufp, relocsize);          /* size */
    yasm_intnum_destroy(nreloc);
    yasm_intnum_destroy(relocsize);

    YASM_WRITE_32_L(bufp, symtab_idx);          /* link: symtab index */
    YASM_WRITE_32_L(bufp, shead->index);        /* info: relocated's index */
    YASM_WRITE_64Z_L(bufp, RELOC64_ALIGN);      /* align */
    YASM_WRITE_64Z_L(bufp, RELOC64A_SIZE);      /* entity size */
}

static void
elf_x86_amd64_handle_reloc_addend(yasm_intnum *intn,
                                  elf_reloc_entry *reloc,
                                  unsigned long offset)
{
    /* .rela: copy value out as addend, replace original with 0 */
    reloc->addend = yasm_intnum_copy(intn);
    yasm_intnum_zero(intn);
}

static unsigned int
elf_x86_amd64_map_reloc_info_to_type(elf_reloc_entry *reloc)
{
    if (reloc->wrt) {
        const elf_machine_ssym *ssym = (elf_machine_ssym *)
            yasm_symrec_get_data(reloc->wrt, &elf_ssym_symrec_data);
        if (!ssym || reloc->valsize != ssym->size)
            yasm_internal_error(N_("Unsupported WRT"));

        /* Force TLS type; this is required by the linker. */
        if (ssym->sym_rel & ELF_SSYM_THREAD_LOCAL) {
            elf_symtab_entry *esym;

            esym = yasm_symrec_get_data(reloc->reloc.sym, &elf_symrec_data);
            if (esym)
                esym->type = STT_TLS;
        }
        /* Map PC-relative GOT to appropriate relocation */
        if (reloc->rtype_rel && ssym->reloc == R_X86_64_GOT32)
            return (unsigned char) R_X86_64_GOTPCREL;
        return (unsigned char) ssym->reloc;
    } else if (reloc->is_GOT_sym && reloc->valsize == 32) {
        return (unsigned char) R_X86_64_GOTPC32;
    } else if (reloc->is_GOT_sym && reloc->valsize == 64) {
        return (unsigned char) R_X86_64_GOTPC64;
    } else if (reloc->rtype_rel) {
        switch (reloc->valsize) {
            case 8: return (unsigned char) R_X86_64_PC8;
            case 16: return (unsigned char) R_X86_64_PC16;
            case 32: return (unsigned char) R_X86_64_PC32;
            case 64: return (unsigned char) R_X86_64_PC64;
            default: yasm_internal_error(N_("Unsupported relocation size"));
        }
    } else {
        switch (reloc->valsize) {
            case 8: return (unsigned char) R_X86_64_8;
            case 16: return (unsigned char) R_X86_64_16;
            case 32: return (unsigned char) R_X86_64_32;
            case 64: return (unsigned char) R_X86_64_64;
            default: yasm_internal_error(N_("Unsupported relocation size"));
        }
    }
    return 0;
}

static void
elf_x86_amd64_write_reloc(unsigned char *bufp, elf_reloc_entry *reloc,
                          unsigned int r_type, unsigned int r_sym)
{
    YASM_WRITE_64I_L(bufp, reloc->reloc.addr);
    /*YASM_WRITE_64_L(bufp, ELF64_R_INFO(r_sym, r_type));*/
    YASM_WRITE_64C_L(bufp, r_sym, r_type);
    if (reloc->addend)
        YASM_WRITE_64I_L(bufp, reloc->addend);
    else {
        YASM_WRITE_32_L(bufp, 0);
        YASM_WRITE_32_L(bufp, 0);
    }
}

static void
elf_x86_amd64_write_proghead(unsigned char **bufpp,
                             elf_offset secthead_addr,
                             unsigned long secthead_count,
                             elf_section_index shstrtab_index)
{
    unsigned char *bufp = *bufpp;
    unsigned char *buf = bufp-4;
    YASM_WRITE_8(bufp, ELFCLASS64);         /* elf class */
    YASM_WRITE_8(bufp, ELFDATA2LSB);        /* data encoding :: MSB? */
    YASM_WRITE_8(bufp, EV_CURRENT);         /* elf version */
    YASM_WRITE_8(bufp, ELFOSABI_SYSV);      /* os/abi */
    YASM_WRITE_8(bufp, 0);                  /* SYSV v3 ABI=0 */
    while (bufp-buf < EI_NIDENT)            /* e_ident padding */
        YASM_WRITE_8(bufp, 0);

    YASM_WRITE_16_L(bufp, ET_REL);          /* e_type - object file */
    YASM_WRITE_16_L(bufp, EM_X86_64);       /* e_machine - or others */
    YASM_WRITE_32_L(bufp, EV_CURRENT);      /* elf version */
    YASM_WRITE_64Z_L(bufp, 0);              /* e_entry */
    YASM_WRITE_64Z_L(bufp, 0);              /* e_phoff */
    YASM_WRITE_64Z_L(bufp, secthead_addr);  /* e_shoff secthead off */

    YASM_WRITE_32_L(bufp, 0);               /* e_flags */
    YASM_WRITE_16_L(bufp, EHDR64_SIZE);     /* e_ehsize */
    YASM_WRITE_16_L(bufp, 0);               /* e_phentsize */
    YASM_WRITE_16_L(bufp, 0);               /* e_phnum */
    YASM_WRITE_16_L(bufp, SHDR64_SIZE);     /* e_shentsize */
    YASM_WRITE_16_L(bufp, secthead_count);  /* e_shnum */
    YASM_WRITE_16_L(bufp, shstrtab_index);  /* e_shstrndx */
    *bufpp = bufp;
}

const elf_machine_handler
elf_machine_handler_x86_amd64 = {
    "x86", "amd64", ".rela",
    SYMTAB64_SIZE, SYMTAB64_ALIGN, RELOC64A_SIZE, SHDR64_SIZE, EHDR64_SIZE,
    elf_x86_amd64_accepts_reloc,
    elf_x86_amd64_write_symtab_entry,
    elf_x86_amd64_write_secthead,
    elf_x86_amd64_write_secthead_rel,
    elf_x86_amd64_handle_reloc_addend,
    elf_x86_amd64_map_reloc_info_to_type,
    elf_x86_amd64_write_reloc,
    elf_x86_amd64_write_proghead,
    elf_x86_amd64_ssyms,
    sizeof(elf_x86_amd64_ssyms)/sizeof(elf_x86_amd64_ssyms[0]),
    64
};
