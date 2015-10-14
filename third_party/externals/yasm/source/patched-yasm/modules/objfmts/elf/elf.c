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

#include <util.h>

#include <libyasm.h>
#define YASM_OBJFMT_ELF_INTERNAL
#include "elf.h"
#include "elf-machine.h"

static void elf_section_data_destroy(void *data);
static void elf_secthead_print(void *data, FILE *f, int indent_level);

const yasm_assoc_data_callback elf_section_data = {
    elf_section_data_destroy,
    elf_secthead_print
};

static void elf_symrec_data_destroy(/*@only@*/ void *d);
static void elf_symtab_entry_print(void *data, FILE *f, int indent_level);
static void elf_ssym_symtab_entry_print(void *data, FILE *f, int indent_level);

const yasm_assoc_data_callback elf_symrec_data = {
    elf_symrec_data_destroy,
    elf_symtab_entry_print
};

const yasm_assoc_data_callback elf_ssym_symrec_data = {
    elf_symrec_data_destroy,
    elf_ssym_symtab_entry_print
};

extern elf_machine_handler
    elf_machine_handler_x86_x86,
    elf_machine_handler_x86_amd64;

static const elf_machine_handler *elf_machine_handlers[] =
{
    &elf_machine_handler_x86_x86,
    &elf_machine_handler_x86_amd64,
    NULL
};
static const elf_machine_handler elf_null_machine = {0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0, 0, 0, 0, 0, 0,
                                                     0, 0, 0};
static elf_machine_handler const *elf_march = &elf_null_machine;
static yasm_symrec **elf_ssyms;

const elf_machine_handler *
elf_set_arch(yasm_arch *arch, yasm_symtab *symtab, int bits_pref)
{
    const char *machine = yasm_arch_get_machine(arch);
    int i;

    for (i=0, elf_march = elf_machine_handlers[0];
         elf_march != NULL;
         elf_march = elf_machine_handlers[++i])
    {
        if (yasm__strcasecmp(yasm_arch_keyword(arch), elf_march->arch)==0)
            if (yasm__strcasecmp(machine, elf_march->machine)==0)
                if (bits_pref == 0 || bits_pref == elf_march->bits)
                    break;
    }

    if (elf_march && elf_march->num_ssyms > 0)
    {
        /* Allocate "special" syms */
        elf_ssyms =
            yasm_xmalloc(elf_march->num_ssyms * sizeof(yasm_symrec *));
        for (i=0; (unsigned int)i<elf_march->num_ssyms; i++)
        {
            /* FIXME: misuse of NULL bytecode */
            elf_ssyms[i] = yasm_symtab_define_label(symtab,
                                                    elf_march->ssyms[i].name,
                                                    NULL, 0, 0);
            yasm_symrec_add_data(elf_ssyms[i], &elf_ssym_symrec_data,
                                 (void*)&elf_march->ssyms[i]);
        }
    }

    return elf_march;
}

yasm_symrec *
elf_get_special_sym(const char *name, const char *parser)
{
    int i;
    for (i=0; (unsigned int)i<elf_march->num_ssyms; i++) {
        if (yasm__strcasecmp(name, elf_march->ssyms[i].name) == 0)
            return elf_ssyms[i];
    }
    return NULL;
}

/* reloc functions */
int elf_ssym_has_flag(yasm_symrec *wrt, int flag);

int
elf_is_wrt_sym_relative(yasm_symrec *wrt)
{
    return elf_ssym_has_flag(wrt, ELF_SSYM_SYM_RELATIVE);
}

int
elf_is_wrt_pos_adjusted(yasm_symrec *wrt)
{
    return elf_ssym_has_flag(wrt, ELF_SSYM_CURPOS_ADJUST);
}

int
elf_ssym_has_flag(yasm_symrec *wrt, int flag)
{
    int i;
    for (i=0; (unsigned int)i<elf_march->num_ssyms; i++) {
        if (elf_ssyms[i] == wrt)
            return (elf_march->ssyms[i].sym_rel & flag) != 0;
    }
    return 0;
}

/* takes ownership of addr */
elf_reloc_entry *
elf_reloc_entry_create(yasm_symrec *sym,
                       yasm_symrec *wrt,
                       yasm_intnum *addr,
                       int rel,
                       size_t valsize,
                       int is_GOT_sym)
{
    elf_reloc_entry *entry;

    if (!elf_march->accepts_reloc)
        yasm_internal_error(N_("Unsupported machine for ELF output"));

    if (!elf_march->accepts_reloc(valsize, wrt))
    {
        if (addr)
            yasm_intnum_destroy(addr);
        return NULL;
    }

    if (sym == NULL)
        yasm_internal_error("sym is null");

    entry = yasm_xmalloc(sizeof(elf_reloc_entry));
    entry->reloc.sym = sym;
    entry->reloc.addr = addr;
    entry->rtype_rel = rel;
    entry->valsize = valsize;
    entry->addend = NULL;
    entry->wrt = wrt;
    entry->is_GOT_sym = is_GOT_sym;

    return entry;
}

void
elf_reloc_entry_destroy(void *entry)
{
    if (((elf_reloc_entry*)entry)->addend)
        yasm_intnum_destroy(((elf_reloc_entry*)entry)->addend);
    yasm_xfree(entry);
}

/* strtab functions */
elf_strtab_entry *
elf_strtab_entry_create(const char *str)
{
    elf_strtab_entry *entry = yasm_xmalloc(sizeof(elf_strtab_entry));
    entry->str = yasm__xstrdup(str);
    entry->index = 0;
    return entry;
}

void
elf_strtab_entry_set_str(elf_strtab_entry *entry, const char *str)
{
    elf_strtab_entry *last;
    if (entry->str)
        yasm_xfree(entry->str);
    entry->str = yasm__xstrdup(str);

    /* Update all following indices since string length probably changes */
    last = entry;
    entry = STAILQ_NEXT(last, qlink);
    while (entry) {
        entry->index = last->index + (unsigned long)strlen(last->str) + 1;
        last = entry;
        entry = STAILQ_NEXT(last, qlink);
    }
}

elf_strtab_head *
elf_strtab_create()
{
    elf_strtab_head *strtab = yasm_xmalloc(sizeof(elf_strtab_head));
    elf_strtab_entry *entry = yasm_xmalloc(sizeof(elf_strtab_entry));

    STAILQ_INIT(strtab);
    entry->index = 0;
    entry->str = yasm__xstrdup("");

    STAILQ_INSERT_TAIL(strtab, entry, qlink);
    return strtab;
}

elf_strtab_entry *
elf_strtab_append_str(elf_strtab_head *strtab, const char *str)
{
    elf_strtab_entry *last, *entry;

    if (strtab == NULL)
        yasm_internal_error("strtab is null");
    if (STAILQ_EMPTY(strtab))
        yasm_internal_error("strtab is missing initial dummy entry");

    last = STAILQ_LAST(strtab, elf_strtab_entry, qlink);

    entry = elf_strtab_entry_create(str);
    entry->index = last->index + (unsigned long)strlen(last->str) + 1;

    STAILQ_INSERT_TAIL(strtab, entry, qlink);
    return entry;
}

void
elf_strtab_destroy(elf_strtab_head *strtab)
{
    elf_strtab_entry *s1, *s2;

    if (strtab == NULL)
        yasm_internal_error("strtab is null");
    if (STAILQ_EMPTY(strtab))
        yasm_internal_error("strtab is missing initial dummy entry");

    s1 = STAILQ_FIRST(strtab);
    while (s1 != NULL) {
        s2 = STAILQ_NEXT(s1, qlink);
        yasm_xfree(s1->str);
        yasm_xfree(s1);
        s1 = s2;
    }
    yasm_xfree(strtab);
}

unsigned long
elf_strtab_output_to_file(FILE *f, elf_strtab_head *strtab)
{
    unsigned long size = 0;
    elf_strtab_entry *entry;

    if (strtab == NULL)
        yasm_internal_error("strtab is null");

    /* consider optimizing tables here */
    STAILQ_FOREACH(entry, strtab, qlink) {
        size_t len = 1 + strlen(entry->str);
        fwrite(entry->str, len, 1, f);
        size += (unsigned long)len;
    }
    return size;
}



/* symtab functions */
elf_symtab_entry *
elf_symtab_entry_create(elf_strtab_entry *name,
                        yasm_symrec *sym)
{
    elf_symtab_entry *entry = yasm_xmalloc(sizeof(elf_symtab_entry));
    entry->in_table = 0;
    entry->sym = sym;
    entry->sect = NULL;
    entry->name = name;
    entry->value = 0;

    entry->xsize = NULL;
    entry->size = 0;
    entry->index = 0;
    entry->bind = 0;
    entry->type = STT_NOTYPE;
    entry->vis = STV_DEFAULT;

    return entry;
}

static void
elf_symtab_entry_destroy(elf_symtab_entry *entry)
{
    if (entry == NULL)
        yasm_internal_error("symtab entry is null");

    yasm_xfree(entry);
}

static void
elf_symrec_data_destroy(void *data)
{
    /* do nothing, as this stuff is in the symtab anyway...  this speaks of bad
     * design/use or this stuff, i fear */

    /* watch for double-free here ... */
    /*elf_symtab_entry_destroy((elf_symtab_entry *)data);*/
}

static void
elf_symtab_entry_print(void *data, FILE *f, int indent_level)
{
    elf_symtab_entry *entry = data;
    if (entry == NULL)
        yasm_internal_error("symtab entry is null");

    fprintf(f, "%*sbind=", indent_level, "");
    switch (entry->bind) {
        case STB_LOCAL:         fprintf(f, "local\n");  break;
        case STB_GLOBAL:        fprintf(f, "global\n"); break;
        case STB_WEAK:          fprintf(f, "weak\n");   break;
        default:                fprintf(f, "undef\n");  break;
    }
    fprintf(f, "%*stype=", indent_level, "");
    switch (entry->type) {
        case STT_NOTYPE:        fprintf(f, "notype\n"); break;
        case STT_OBJECT:        fprintf(f, "object\n"); break;
        case STT_FUNC:          fprintf(f, "func\n");   break;
        case STT_SECTION:       fprintf(f, "section\n");break;
        case STT_FILE:          fprintf(f, "file\n");   break;
        default:                fprintf(f, "undef\n");  break;
    }
    fprintf(f, "%*ssize=", indent_level, "");
    if (entry->xsize)
        yasm_expr_print(entry->xsize, f);
    else
        fprintf(f, "%ld", entry->size);
    fprintf(f, "\n");
}

static void
elf_ssym_symtab_entry_print(void *data, FILE *f, int indent_level)
{
    /* TODO */
}

elf_symtab_head *
elf_symtab_create()
{
    elf_symtab_head *symtab = yasm_xmalloc(sizeof(elf_symtab_head));
    elf_symtab_entry *entry = yasm_xmalloc(sizeof(elf_symtab_entry));

    STAILQ_INIT(symtab);
    entry->in_table = 1;
    entry->sym = NULL;
    entry->sect = NULL;
    entry->name = NULL;
    entry->value = 0;
    entry->xsize = NULL;
    entry->size = 0;
    entry->index = SHN_UNDEF;
    entry->bind = STB_LOCAL;
    entry->type = STT_NOTYPE;
    entry->vis = STV_DEFAULT;
    entry->symindex = 0;
    STAILQ_INSERT_TAIL(symtab, entry, qlink);
    return symtab;
}

void
elf_symtab_append_entry(elf_symtab_head *symtab, elf_symtab_entry *entry)
{
    if (symtab == NULL)
        yasm_internal_error("symtab is null");
    if (entry == NULL)
        yasm_internal_error("symtab entry is null");
    if (STAILQ_EMPTY(symtab))
        yasm_internal_error(N_("symtab is missing initial dummy entry"));

    STAILQ_INSERT_TAIL(symtab, entry, qlink);
    entry->in_table = 1;
}

void
elf_symtab_insert_local_sym(elf_symtab_head *symtab, elf_symtab_entry *entry)
{
    elf_symtab_entry *after = STAILQ_FIRST(symtab);
    elf_symtab_entry *before = NULL;

    while (after && (after->bind == STB_LOCAL)) {
        before = after;
        if (before->type == STT_FILE) break;
        after = STAILQ_NEXT(after, qlink);
    }
    STAILQ_INSERT_AFTER(symtab, before, entry, qlink);
    entry->in_table = 1;
}

void
elf_symtab_destroy(elf_symtab_head *symtab)
{
    elf_symtab_entry *s1, *s2;

    if (symtab == NULL)
        yasm_internal_error("symtab is null");
    if (STAILQ_EMPTY(symtab))
        yasm_internal_error(N_("symtab is missing initial dummy entry"));

    s1 = STAILQ_FIRST(symtab);
    while (s1 != NULL) {
        s2 = STAILQ_NEXT(s1, qlink);
        elf_symtab_entry_destroy(s1);
        s1 = s2;
    }
    yasm_xfree(symtab);
}

unsigned long
elf_symtab_assign_indices(elf_symtab_head *symtab)
{
    elf_symtab_entry *entry, *prev=NULL;
    unsigned long last_local=0;

    if (symtab == NULL)
        yasm_internal_error("symtab is null");
    if (STAILQ_EMPTY(symtab))
        yasm_internal_error(N_("symtab is missing initial dummy entry"));

    STAILQ_FOREACH(entry, symtab, qlink) {
        if (prev)
            entry->symindex = prev->symindex + 1;
        if (entry->bind == STB_LOCAL)
            last_local = entry->symindex;
        prev = entry;
    }
    return last_local + 1;
}

unsigned long
elf_symtab_write_to_file(FILE *f, elf_symtab_head *symtab,
                         yasm_errwarns *errwarns)
{
    unsigned char buf[SYMTAB_MAXSIZE], *bufp;
    elf_symtab_entry *entry, *prev;
    unsigned long size = 0;

    if (!symtab)
        yasm_internal_error(N_("symtab is null"));

    prev = NULL;
    STAILQ_FOREACH(entry, symtab, qlink) {

        yasm_intnum *size_intn=NULL, *value_intn=NULL;
        bufp = buf;

        /* get size (if specified); expr overrides stored integer */
        if (entry->xsize) {
            size_intn = yasm_intnum_copy(
                yasm_expr_get_intnum(&entry->xsize, 1));
            if (!size_intn) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("size specifier not an integer expression"));
                yasm_errwarn_propagate(errwarns, entry->xsize->line);
            }
        }
        else
            size_intn = yasm_intnum_create_uint(entry->size);

        /* get EQU value for constants */
        if (entry->sym) {
            const yasm_expr *equ_expr_c;
            equ_expr_c = yasm_symrec_get_equ(entry->sym);

            if (equ_expr_c != NULL) {
                const yasm_intnum *equ_intn;
                yasm_expr *equ_expr = yasm_expr_copy(equ_expr_c);
                equ_intn = yasm_expr_get_intnum(&equ_expr, 1);

                if (equ_intn == NULL) {
                    yasm_error_set(YASM_ERROR_VALUE,
                                   N_("EQU value not an integer expression"));
                    yasm_errwarn_propagate(errwarns, equ_expr->line);
                } else
                    value_intn = yasm_intnum_copy(equ_intn);
                entry->index = SHN_ABS;
                yasm_expr_destroy(equ_expr);
            }
        }
        if (value_intn == NULL)
            value_intn = yasm_intnum_create_uint(entry->value);

        /* If symbol is in a TLS section, force its type to TLS. */
        if (entry->sym) {
            yasm_bytecode *precbc;
            yasm_section *sect;
            elf_secthead *shead;
            if (yasm_symrec_get_label(entry->sym, &precbc) &&
                (sect = yasm_bc_get_section(precbc)) &&
                (shead = yasm_section_get_data(sect, &elf_section_data)) &&
                shead->flags & SHF_TLS) {
                entry->type = STT_TLS;
            }
        }

        if (!elf_march->write_symtab_entry || !elf_march->symtab_entry_size)
            yasm_internal_error(N_("Unsupported machine for ELF output"));
        elf_march->write_symtab_entry(bufp, entry, value_intn, size_intn);
        fwrite(buf, elf_march->symtab_entry_size, 1, f);
        size += elf_march->symtab_entry_size;

        yasm_intnum_destroy(size_intn);
        yasm_intnum_destroy(value_intn);

        prev = entry;
    }
    return size;
}

void elf_symtab_set_nonzero(elf_symtab_entry *entry,
                            yasm_section *sect,
                            elf_section_index sectidx,
                            elf_symbol_binding bind,
                            elf_symbol_type type,
                            yasm_expr *xsize,
                            elf_address *value)
{
    if (!entry)
        yasm_internal_error("NULL entry");
    if (sect) entry->sect = sect;
    if (sectidx) entry->index = sectidx;
    if (bind) entry->bind = bind;
    if (type) entry->type = type;
    if (xsize) entry->xsize = xsize;
    if (value) entry->value = *value;
}

void
elf_sym_set_visibility(elf_symtab_entry *entry,
                       elf_symbol_vis    vis)
{
    entry->vis = ELF_ST_VISIBILITY(vis);
}                            

void
elf_sym_set_type(elf_symtab_entry *entry,
                 elf_symbol_type   type)
{
    entry->type = type;
}                            

void
elf_sym_set_size(elf_symtab_entry *entry,
                 struct yasm_expr *size)
{
    if (entry->xsize)
        yasm_expr_destroy(entry->xsize);
    entry->xsize = size;
}                            

int
elf_sym_in_table(elf_symtab_entry *entry)
{
    return entry->in_table;
}

elf_secthead *
elf_secthead_create(elf_strtab_entry    *name,
                    elf_section_type     type,
                    elf_section_flags    flags,
                    elf_address          offset,
                    elf_size             size)
{
    elf_secthead *esd = yasm_xmalloc(sizeof(elf_secthead));

    esd->type = type;
    esd->flags = flags;
    esd->offset = offset;
    esd->size = yasm_intnum_create_uint(size);
    esd->link = 0;
    esd->info = 0;
    esd->align = 0;
    esd->entsize = 0;
    esd->index = 0;

    esd->sym = NULL;
    esd->name = name;
    esd->index = 0;
    esd->rel_name = NULL;
    esd->rel_index = 0;
    esd->rel_offset = 0;
    esd->nreloc = 0;

    if (name && (strcmp(name->str, ".symtab") == 0)) {
        if (!elf_march->symtab_entry_size || !elf_march->symtab_entry_align)
            yasm_internal_error(N_("unsupported ELF format"));
        esd->entsize = elf_march->symtab_entry_size;
        esd->align = elf_march->symtab_entry_align;
    }

    return esd;
}

void
elf_secthead_destroy(elf_secthead *shead)
{
    if (shead == NULL)
        yasm_internal_error(N_("shead is null"));

    yasm_intnum_destroy(shead->size);

    yasm_xfree(shead);
}

static void
elf_section_data_destroy(void *data)
{
    elf_secthead_destroy((elf_secthead *)data);
}

static void
elf_secthead_print(void *data, FILE *f, int indent_level)
{
    elf_secthead *sect = data;
    fprintf(f, "%*sname=%s\n", indent_level, "",
            sect->name ? sect->name->str : "<undef>");
    fprintf(f, "%*ssym=\n", indent_level, "");
    yasm_symrec_print(sect->sym, f, indent_level+1);
    fprintf(f, "%*sindex=0x%x\n", indent_level, "", sect->index);
    fprintf(f, "%*sflags=", indent_level, "");
    if (sect->flags & SHF_WRITE)
        fprintf(f, "WRITE ");
    if (sect->flags & SHF_ALLOC)
        fprintf(f, "ALLOC ");
    if (sect->flags & SHF_EXECINSTR)
        fprintf(f, "EXEC ");
    /*if (sect->flags & SHF_MASKPROC)
        fprintf(f, "PROC-SPECIFIC"); */
    fprintf(f, "%*soffset=0x%lx\n", indent_level, "", sect->offset);
    fprintf(f, "%*ssize=0x%lx\n", indent_level, "",
            yasm_intnum_get_uint(sect->size));
    fprintf(f, "%*slink=0x%x\n", indent_level, "", sect->link);
    fprintf(f, "%*salign=%lu\n", indent_level, "", sect->align);
    fprintf(f, "%*snreloc=%ld\n", indent_level, "", sect->nreloc);
}

unsigned long
elf_secthead_write_to_file(FILE *f, elf_secthead *shead,
                           elf_section_index sindex)
{
    unsigned char buf[SHDR_MAXSIZE], *bufp = buf;
    shead->index = sindex;

    if (shead == NULL)
        yasm_internal_error("shead is null");

    if (!elf_march->write_secthead || !elf_march->secthead_size)
        yasm_internal_error(N_("Unsupported machine for ELF output"));
    elf_march->write_secthead(bufp, shead);
    if (fwrite(buf, elf_march->secthead_size, 1, f))
        return elf_march->secthead_size;
    yasm_internal_error(N_("Failed to write an elf section header"));
    return 0;
}

void
elf_secthead_append_reloc(yasm_section *sect, elf_secthead *shead,
                          elf_reloc_entry *reloc)
{
    if (sect == NULL)
        yasm_internal_error("sect is null");
    if (shead == NULL)
        yasm_internal_error("shead is null");
    if (reloc == NULL)
        yasm_internal_error("reloc is null");

    shead->nreloc++;
    yasm_section_add_reloc(sect, (yasm_reloc *)reloc, elf_reloc_entry_destroy);
}

char *
elf_secthead_name_reloc_section(const char *basesect)
{
    if (!elf_march->reloc_section_prefix)
    {
        yasm_internal_error(N_("Unsupported machine for ELF output"));
        return NULL;
    }
    else
    {
        size_t prepend_length = strlen(elf_march->reloc_section_prefix);
        char *sectname = yasm_xmalloc(prepend_length + strlen(basesect) + 1);
        strcpy(sectname, elf_march->reloc_section_prefix);
        strcat(sectname, basesect);
        return sectname;
    }
}

void
elf_handle_reloc_addend(yasm_intnum *intn,
                        elf_reloc_entry *reloc,
                        unsigned long offset)
{
    if (!elf_march->handle_reloc_addend)
        yasm_internal_error(N_("Unsupported machine for ELF output"));
    elf_march->handle_reloc_addend(intn, reloc, offset);
}

unsigned long
elf_secthead_write_rel_to_file(FILE *f, elf_section_index symtab_idx,
                               yasm_section *sect, elf_secthead *shead,
                               elf_section_index sindex)
{
    unsigned char buf[SHDR_MAXSIZE], *bufp = buf;

    if (shead == NULL)
        yasm_internal_error("shead is null");

    if (!yasm_section_relocs_first(sect))
        return 0;       /* no relocations, no .rel.* section header */

    shead->rel_index = sindex;

    if (!elf_march->write_secthead_rel || !elf_march->secthead_size)
        yasm_internal_error(N_("Unsupported machine for ELF output"));
    elf_march->write_secthead_rel(bufp, shead, symtab_idx, sindex);
    if (fwrite(buf, elf_march->secthead_size, 1, f))
        return elf_march->secthead_size;
    yasm_internal_error(N_("Failed to write an elf section header"));
    return 0;
}

unsigned long
elf_secthead_write_relocs_to_file(FILE *f, yasm_section *sect,
                                  elf_secthead *shead, yasm_errwarns *errwarns)
{
    elf_reloc_entry *reloc;
    unsigned char buf[RELOC_MAXSIZE], *bufp;
    unsigned long size = 0;
    long pos;

    if (shead == NULL)
        yasm_internal_error("shead is null");

    reloc = (elf_reloc_entry *)yasm_section_relocs_first(sect);
    if (!reloc)
        return 0;

    /* first align section to multiple of 4 */
    pos = ftell(f);
    if (pos == -1) {
        yasm_error_set(YASM_ERROR_IO,
                       N_("couldn't read position on output stream"));
        yasm_errwarn_propagate(errwarns, 0);
    }
    pos = (pos + 3) & ~3;
    if (fseek(f, pos, SEEK_SET) < 0) {
        yasm_error_set(YASM_ERROR_IO, N_("couldn't seek on output stream"));
        yasm_errwarn_propagate(errwarns, 0);
    }
    shead->rel_offset = (unsigned long)pos;


    while (reloc) {
        yasm_sym_vis vis;
        unsigned int r_type=0, r_sym;
        elf_symtab_entry *esym;

        esym = yasm_symrec_get_data(reloc->reloc.sym, &elf_symrec_data);
        if (esym)
            r_sym = esym->symindex;
        else
            r_sym = STN_UNDEF;

        vis = yasm_symrec_get_visibility(reloc->reloc.sym);
        if (!elf_march->map_reloc_info_to_type)
            yasm_internal_error(N_("Unsupported arch/machine for elf output"));
        r_type = elf_march->map_reloc_info_to_type(reloc);

        bufp = buf;
        if (!elf_march->write_reloc || !elf_march->reloc_entry_size)
            yasm_internal_error(N_("Unsupported arch/machine for elf output"));
        elf_march->write_reloc(bufp, reloc, r_type, r_sym);
        fwrite(buf, elf_march->reloc_entry_size, 1, f);
        size += elf_march->reloc_entry_size;

        reloc = (elf_reloc_entry *)
            yasm_section_reloc_next((yasm_reloc *)reloc);
    }
    return size;
}

elf_section_type
elf_secthead_get_type(elf_secthead *shead)
{
    return shead->type;
}

void
elf_secthead_set_typeflags(elf_secthead *shead, elf_section_type type,
                           elf_section_flags flags)
{
    shead->type = type;
    shead->flags = flags;
}

int
elf_secthead_is_empty(elf_secthead *shead)
{
    return yasm_intnum_is_zero(shead->size);
}

yasm_symrec *
elf_secthead_get_sym(elf_secthead *shead)
{
    return shead->sym;
}

elf_section_index
elf_secthead_get_index(elf_secthead *shead)
{
    return shead->index;
}

unsigned long
elf_secthead_get_align(const elf_secthead *shead)
{
    return shead->align;
}

unsigned long
elf_secthead_set_align(elf_secthead *shead, unsigned long align)
{
    return shead->align = align;
}

elf_section_info
elf_secthead_set_info(elf_secthead *shead, elf_section_info info)
{
    return shead->info = info;
}

elf_section_index
elf_secthead_set_index(elf_secthead *shead, elf_section_index sectidx)
{
    return shead->index = sectidx;
}

elf_section_index
elf_secthead_set_link(elf_secthead *shead, elf_section_index link)
{
    return shead->link = link;
}

elf_section_index
elf_secthead_set_rel_index(elf_secthead *shead, elf_section_index sectidx)
{
    return shead->rel_index = sectidx;
}

elf_strtab_entry *
elf_secthead_set_rel_name(elf_secthead *shead, elf_strtab_entry *entry)
{
    return shead->rel_name = entry;
}

elf_size
elf_secthead_set_entsize(elf_secthead *shead, elf_size size)
{
    return shead->entsize = size;
}

yasm_symrec *
elf_secthead_set_sym(elf_secthead *shead, yasm_symrec *sym)
{
    return shead->sym = sym;
}

void
elf_secthead_add_size(elf_secthead *shead, yasm_intnum *size)
{
    if (size) {
        yasm_intnum_calc(shead->size, YASM_EXPR_ADD, size);
    }
}

long
elf_secthead_set_file_offset(elf_secthead *shead, long pos)
{
    unsigned long align = shead->align;

    if (align == 0 || align == 1) {
        shead->offset = (unsigned long)pos;
        return pos;
    }
    else if (align & (align - 1))
        yasm_internal_error(
            N_("alignment %d for section `%s' is not a power of 2"));
            /*, align, sect->name->str);*/

    shead->offset = (unsigned long)((pos + align - 1) & ~(align - 1));
    return (long)shead->offset;
}

unsigned long
elf_proghead_get_size(void)
{
    if (!elf_march->proghead_size)
        yasm_internal_error(N_("Unsupported ELF format for output"));
    return elf_march->proghead_size;
}

unsigned long
elf_proghead_write_to_file(FILE *f,
                           elf_offset secthead_addr,
                           unsigned long secthead_count,
                           elf_section_index shstrtab_index)
{
    unsigned char buf[EHDR_MAXSIZE], *bufp = buf;

    YASM_WRITE_8(bufp, ELFMAG0);                /* ELF magic number */
    YASM_WRITE_8(bufp, ELFMAG1);
    YASM_WRITE_8(bufp, ELFMAG2);
    YASM_WRITE_8(bufp, ELFMAG3);

    if (!elf_march->write_proghead || !elf_march->proghead_size)
        yasm_internal_error(N_("Unsupported ELF format for output"));
    elf_march->write_proghead(&bufp, secthead_addr, secthead_count, shstrtab_index);

    if (((unsigned)(bufp - buf)) != elf_march->proghead_size)
        yasm_internal_error(N_("ELF program header is not proper length"));

    if (fwrite(buf, elf_march->proghead_size, 1, f))
        return elf_march->proghead_size;

    yasm_internal_error(N_("Failed to write ELF program header"));
    return 0;
}
