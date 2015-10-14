/*
 * ELF object machine specific format helpers
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

#ifndef ELF_MACHINE_H_INCLUDED
#define ELF_MACHINE_H_INCLUDED

#define YASM_WRITE_32I_L(p, i) do {\
    assert(yasm_intnum_check_size(i, 32, 0, 2)); \
    yasm_intnum_get_sized(i, p, 4, 32, 0, 0, 0); \
    p += 4; } while (0)

#define YASM_WRITE_64I_L(p, i) do {\
    assert(yasm_intnum_check_size(i, 64, 0, 2)); \
    yasm_intnum_get_sized(i, p, 8, 64, 0, 0, 0); \
    p += 8; } while (0)

#define YASM_WRITE_64C_L(p, hi, lo) do {\
    YASM_WRITE_32_L(p, lo); \
    YASM_WRITE_32_L(p, hi); } while (0)

#define YASM_WRITE_64Z_L(p, i)          YASM_WRITE_64C_L(p, 0, i)

typedef int(*func_accepts_reloc)(size_t val, yasm_symrec *wrt);
typedef void(*func_write_symtab_entry)(unsigned char *bufp,
                                       elf_symtab_entry *entry,
                                       yasm_intnum *value_intn,
                                       yasm_intnum *size_intn);
typedef void(*func_write_secthead)(unsigned char *bufp, elf_secthead *shead);
typedef void(*func_write_secthead_rel)(unsigned char *bufp,
                                       elf_secthead *shead,
                                       elf_section_index symtab_idx,
                                       elf_section_index sindex);

typedef void(*func_handle_reloc_addend)(yasm_intnum *intn,
                                        elf_reloc_entry *reloc,
                                        unsigned long offset);
typedef unsigned int(*func_map_reloc_info_to_type)(elf_reloc_entry *reloc);
typedef void(*func_write_reloc)(unsigned char *bufp,
                                elf_reloc_entry *reloc,
                                unsigned int r_type,
                                unsigned int r_sym);
typedef void (*func_write_proghead)(unsigned char **bufpp,
                                    elf_offset secthead_addr,
                                    unsigned long secthead_count,
                                    elf_section_index shstrtab_index);

enum {
    ELF_SSYM_SYM_RELATIVE = 1 << 0,
    ELF_SSYM_CURPOS_ADJUST = 1 << 1,
    ELF_SSYM_THREAD_LOCAL = 1 << 2
};

typedef struct {
    const char *name;       /* should be something like ..name */
    const int sym_rel;      /* symbol or section-relative? */
    const unsigned int reloc;   /* relocation type */
    const unsigned int size;    /* legal data size */
} elf_machine_ssym;

struct elf_machine_handler {
    const char *arch;
    const char *machine;
    const char *reloc_section_prefix;
    const unsigned long symtab_entry_size;
    const unsigned long symtab_entry_align;
    const unsigned long reloc_entry_size;
    const unsigned long secthead_size;
    const unsigned long proghead_size;
    func_accepts_reloc accepts_reloc;
    func_write_symtab_entry write_symtab_entry;
    func_write_secthead write_secthead;
    func_write_secthead_rel write_secthead_rel;
    func_handle_reloc_addend handle_reloc_addend;
    func_map_reloc_info_to_type map_reloc_info_to_type;
    func_write_reloc write_reloc;
    func_write_proghead write_proghead;

    elf_machine_ssym *ssyms;            /* array of "special" syms */
    const size_t num_ssyms;             /* size of array */

    const int bits;                     /* usually 32 or 64 */
};

#endif /* ELF_MACHINE_H_INCLUDED */
