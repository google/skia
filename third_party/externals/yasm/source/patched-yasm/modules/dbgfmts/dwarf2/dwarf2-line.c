/*
 * DWARF2 debugging format - line information
 *
 *  Copyright (C) 2006-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

#include "dwarf2-dbgfmt.h"

/* DWARF line number opcodes */
typedef enum {
    DW_LNS_extended_op = 0,
    DW_LNS_copy,
    DW_LNS_advance_pc,
    DW_LNS_advance_line,
    DW_LNS_set_file,
    DW_LNS_set_column,
    DW_LNS_negate_stmt,
    DW_LNS_set_basic_block,
    DW_LNS_const_add_pc,
    DW_LNS_fixed_advance_pc,
#ifdef WITH_DWARF3
    /* DWARF 3 extensions */
    DW_LNS_set_prologue_end,
    DW_LNS_set_epilogue_begin,
    DW_LNS_set_isa,
#endif
    DWARF2_LINE_OPCODE_BASE
} dwarf_line_number_op;

/* # of LEB128 operands needed for each of the above opcodes */
static unsigned char line_opcode_num_operands[DWARF2_LINE_OPCODE_BASE-1] = {
    0,  /* DW_LNS_copy */
    1,  /* DW_LNS_advance_pc */
    1,  /* DW_LNS_advance_line */
    1,  /* DW_LNS_set_file */
    1,  /* DW_LNS_set_column */
    0,  /* DW_LNS_negate_stmt */
    0,  /* DW_LNS_set_basic_block */
    0,  /* DW_LNS_const_add_pc */
    1,  /* DW_LNS_fixed_advance_pc */
#ifdef WITH_DWARF3
    0,  /* DW_LNS_set_prologue_end */
    0,  /* DW_LNS_set_epilogue_begin */
    1   /* DW_LNS_set_isa */
#endif
};

/* Line number extended opcodes */
typedef enum {
    DW_LNE_end_sequence = 1,
    DW_LNE_set_address,
    DW_LNE_define_file,
    DW_LNE_set_discriminator
} dwarf_line_number_ext_op;

/* Base and range for line offsets in special opcodes */
#define DWARF2_LINE_BASE                -5
#define DWARF2_LINE_RANGE               14

#define DWARF2_MAX_SPECIAL_ADDR_DELTA   \
    (((255-DWARF2_LINE_OPCODE_BASE)/DWARF2_LINE_RANGE)*\
     dbgfmt_dwarf2->min_insn_len)

/* Initial value of is_stmt register */
#define DWARF2_LINE_DEFAULT_IS_STMT     1

/* Line number state machine register state */
typedef struct dwarf2_line_state {
    /* static configuration */
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2;

    /* DWARF2 state machine registers */
    unsigned long address;
    unsigned long file;
    unsigned long line;
    unsigned long column;
    unsigned long isa;
    int is_stmt;

    /* other state information */
    /*@null@*/ yasm_bytecode *precbc;
} dwarf2_line_state;

typedef struct dwarf2_spp {
    yasm_bytecode *line_start_prevbc;
    yasm_bytecode *line_end_prevbc;
} dwarf2_spp;

typedef struct dwarf2_line_op {
    dwarf_line_number_op opcode;
    /*@owned@*/ /*@null@*/ yasm_intnum *operand;

    /* extended opcode */
    dwarf_line_number_ext_op ext_opcode;
    /*@null@*/ /*@dependent@*/ yasm_symrec *ext_operand;  /* unsigned */
    /*@null@*/ /*@dependent@*/ yasm_intnum *ext_operand_int; /* unsigned */
    unsigned long ext_operandsize;
} dwarf2_line_op;

/* Bytecode callback function prototypes */
static void dwarf2_spp_bc_destroy(void *contents);
static void dwarf2_spp_bc_print(const void *contents, FILE *f,
                                int indent_level);
static int dwarf2_spp_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int dwarf2_spp_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

static void dwarf2_line_op_bc_destroy(void *contents);
static void dwarf2_line_op_bc_print(const void *contents, FILE *f,
                                    int indent_level);
static int dwarf2_line_op_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int dwarf2_line_op_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

/* Bytecode callback structures */
static const yasm_bytecode_callback dwarf2_spp_bc_callback = {
    dwarf2_spp_bc_destroy,
    dwarf2_spp_bc_print,
    yasm_bc_finalize_common,
    NULL,
    dwarf2_spp_bc_calc_len,
    yasm_bc_expand_common,
    dwarf2_spp_bc_tobytes,
    0
};

static const yasm_bytecode_callback dwarf2_line_op_bc_callback = {
    dwarf2_line_op_bc_destroy,
    dwarf2_line_op_bc_print,
    yasm_bc_finalize_common,
    NULL,
    dwarf2_line_op_bc_calc_len,
    yasm_bc_expand_common,
    dwarf2_line_op_bc_tobytes,
    0
};


static size_t
dwarf2_dbgfmt_add_file(yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2, unsigned long filenum,
                       const char *pathname)
{
    size_t dirlen;
    const char *filename;
    unsigned long i, dir;

    /* Put the directory into the directory table */
    dir = 0;
    dirlen = yasm__splitpath(pathname, &filename);
    if (dirlen > 0) {
        /* Look to see if we already have that dir in the table */
        for (dir=1; dir<dbgfmt_dwarf2->dirs_size+1; dir++) {
            if (strncmp(dbgfmt_dwarf2->dirs[dir-1], pathname, dirlen) == 0
                && dbgfmt_dwarf2->dirs[dir-1][dirlen] == '\0')
                break;
        }
        if (dir >= dbgfmt_dwarf2->dirs_size+1) {
            /* Not found in table, add to end, reallocing if necessary */
            if (dir >= dbgfmt_dwarf2->dirs_allocated+1) {
                dbgfmt_dwarf2->dirs_allocated = dir+32;
                dbgfmt_dwarf2->dirs = yasm_xrealloc(dbgfmt_dwarf2->dirs,
                    sizeof(char *)*dbgfmt_dwarf2->dirs_allocated);
            }
            dbgfmt_dwarf2->dirs[dir-1] = yasm__xstrndup(pathname, dirlen);
            dbgfmt_dwarf2->dirs_size = dir;
        }
    }

    /* Put the filename into the filename table */
    if (filenum == 0) {
        /* Look to see if we already have that filename in the table */
        for (; filenum<dbgfmt_dwarf2->filenames_size; filenum++) {
            if (!dbgfmt_dwarf2->filenames[filenum].filename ||
                (dbgfmt_dwarf2->filenames[filenum].dir == dir
                 && strcmp(dbgfmt_dwarf2->filenames[filenum].filename,
                           filename) == 0))
                break;
        }
    } else
        filenum--;      /* array index is 0-based */

    /* Realloc table if necessary */
    if (filenum >= dbgfmt_dwarf2->filenames_allocated) {
        unsigned long old_allocated = dbgfmt_dwarf2->filenames_allocated;
        dbgfmt_dwarf2->filenames_allocated = filenum+32;
        dbgfmt_dwarf2->filenames = yasm_xrealloc(dbgfmt_dwarf2->filenames,
            sizeof(dwarf2_filename)*dbgfmt_dwarf2->filenames_allocated);
        for (i=old_allocated; i<dbgfmt_dwarf2->filenames_allocated; i++) {
            dbgfmt_dwarf2->filenames[i].pathname = NULL;
            dbgfmt_dwarf2->filenames[i].filename = NULL;
            dbgfmt_dwarf2->filenames[i].dir = 0;
        }
    }

    /* Actually save in table */
    if (dbgfmt_dwarf2->filenames[filenum].pathname)
        yasm_xfree(dbgfmt_dwarf2->filenames[filenum].pathname);
    if (dbgfmt_dwarf2->filenames[filenum].filename)
        yasm_xfree(dbgfmt_dwarf2->filenames[filenum].filename);
    dbgfmt_dwarf2->filenames[filenum].pathname = yasm__xstrdup(pathname);
    dbgfmt_dwarf2->filenames[filenum].filename = yasm__xstrdup(filename);
    dbgfmt_dwarf2->filenames[filenum].dir = dir;

    /* Update table size */
    if (filenum >= dbgfmt_dwarf2->filenames_size)
        dbgfmt_dwarf2->filenames_size = filenum + 1;

    return filenum;
}

/* Create and add a new line opcode to a section, updating offset on insertion;
 * no optimization necessary.
 */
static yasm_bytecode *
dwarf2_dbgfmt_append_line_op(yasm_section *sect, dwarf_line_number_op opcode,
                             /*@only@*/ /*@null@*/ yasm_intnum *operand)
{
    dwarf2_line_op *line_op = yasm_xmalloc(sizeof(dwarf2_line_op));
    yasm_bytecode *bc;

    line_op->opcode = opcode;
    line_op->operand = operand;
    line_op->ext_opcode = 0;
    line_op->ext_operand = NULL;
    line_op->ext_operand_int = NULL;
    line_op->ext_operandsize = 0;

    bc = yasm_bc_create_common(&dwarf2_line_op_bc_callback, line_op, 0);
    bc->len = 1;
    if (operand)
        bc->len += yasm_intnum_size_leb128(operand,
                                           opcode == DW_LNS_advance_line);

    yasm_dwarf2__append_bc(sect, bc);
    return bc;
}

/* Create and add a new extended line opcode to a section, updating offset on
 * insertion; no optimization necessary.
 */
static yasm_bytecode *
dwarf2_dbgfmt_append_line_ext_op(yasm_section *sect,
                                 dwarf_line_number_ext_op ext_opcode,
                                 unsigned long ext_operandsize,
                                 /*@null@*/ yasm_symrec *ext_operand)
{
    dwarf2_line_op *line_op = yasm_xmalloc(sizeof(dwarf2_line_op));
    yasm_bytecode *bc;

    line_op->opcode = DW_LNS_extended_op;
    line_op->operand = yasm_intnum_create_uint(ext_operandsize+1);
    line_op->ext_opcode = ext_opcode;
    line_op->ext_operand = ext_operand;
    line_op->ext_operand_int = NULL;
    line_op->ext_operandsize = ext_operandsize;

    bc = yasm_bc_create_common(&dwarf2_line_op_bc_callback, line_op, 0);
    bc->len = 2 + yasm_intnum_size_leb128(line_op->operand, 0) +
        ext_operandsize;

    yasm_dwarf2__append_bc(sect, bc);
    return bc;
}

static yasm_bytecode *
dwarf2_dbgfmt_append_line_ext_op_int(yasm_section *sect,
                                     dwarf_line_number_ext_op ext_opcode,
                                     /*@only@*/ yasm_intnum *ext_operand)
{
    dwarf2_line_op *line_op = yasm_xmalloc(sizeof(dwarf2_line_op));
    unsigned long ext_operandsize = yasm_intnum_size_leb128(ext_operand, 0);
    yasm_bytecode *bc;

    line_op->opcode = DW_LNS_extended_op;
    line_op->operand = yasm_intnum_create_uint(ext_operandsize+1);
    line_op->ext_opcode = ext_opcode;
    line_op->ext_operand = NULL;
    line_op->ext_operand_int = ext_operand;
    line_op->ext_operandsize = ext_operandsize;

    bc = yasm_bc_create_common(&dwarf2_line_op_bc_callback, line_op, 0);
    bc->len = 2 + yasm_intnum_size_leb128(line_op->operand, 0) +
        ext_operandsize;

    yasm_dwarf2__append_bc(sect, bc);
    return bc;
}

static void
dwarf2_dbgfmt_finalize_locs(yasm_section *sect, dwarf2_section_data *dsd)
{
    /*@dependent@*/ yasm_symrec *lastsym = NULL;
    /*@null@*/ yasm_bytecode *bc;
    /*@null@*/ dwarf2_loc *loc;

    bc = yasm_section_bcs_first(sect);
    STAILQ_FOREACH(loc, &dsd->locs, link) {
        /* Find the first bytecode following this loc by looking at
         * the virtual line numbers.  XXX: this assumes the source file
         * order will be the same as the actual section order.  If we ever
         * implement subsegs this will NOT necessarily be true and this logic
         * will need to be fixed to handle it!
         *
         * Keep track of last symbol seen prior to the loc.
         */
        while (bc && bc->line <= loc->vline) {
            if (bc->symrecs) {
                int i = 0;
                while (bc->symrecs[i]) {
                    lastsym = bc->symrecs[i];
                    i++;
                }
            }
            bc = yasm_bc__next(bc);
        }
        loc->sym = lastsym;
        loc->bc = bc;
    }
}

static int
dwarf2_dbgfmt_gen_line_op(yasm_section *debug_line, dwarf2_line_state *state,
                          const dwarf2_loc *loc,
                          /*@null@*/ const dwarf2_loc *nextloc)
{
    unsigned long addr_delta;
    long line_delta;
    int opcode1, opcode2;
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = state->dbgfmt_dwarf2;

    if (state->file != loc->file) {
        state->file = loc->file;
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_set_file,
                                     yasm_intnum_create_uint(state->file));
    }
    if (state->column != loc->column) {
        state->column = loc->column;
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_set_column,
                                     yasm_intnum_create_uint(state->column));
    }
    if (loc->discriminator != 0) {
        dwarf2_dbgfmt_append_line_ext_op_int(debug_line,
            DW_LNE_set_discriminator,
            yasm_intnum_create_uint(loc->discriminator));
    }
#ifdef WITH_DWARF3
    if (loc->isa_change) {
        state->isa = loc->isa;
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_set_isa,
                                     yasm_intnum_create_uint(state->isa));
    }
#endif
    if (state->is_stmt == 0 && loc->is_stmt == IS_STMT_SET) {
        state->is_stmt = 1;
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_negate_stmt, NULL);
    } else if (state->is_stmt == 1 && loc->is_stmt == IS_STMT_CLEAR) {
        state->is_stmt = 0;
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_negate_stmt, NULL);
    }
    if (loc->basic_block) {
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_set_basic_block, NULL);
    }
#ifdef WITH_DWARF3
    if (loc->prologue_end) {
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_set_prologue_end, NULL);
    }
    if (loc->epilogue_begin) {
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_set_epilogue_begin,
                                     NULL);
    }
#endif

    /* If multiple loc for the same location, use last */
    if (nextloc && nextloc->bc->offset == loc->bc->offset)
        return 0;

    if (!state->precbc) {
        /* Set the starting address for the section */
        if (!loc->sym) {
            /* shouldn't happen! */
            yasm_error_set(YASM_ERROR_GENERAL,
                           N_("could not find label prior to loc"));
            return 1;
        }
        dwarf2_dbgfmt_append_line_ext_op(debug_line, DW_LNE_set_address,
            dbgfmt_dwarf2->sizeof_address, loc->sym);
        addr_delta = 0;
    } else if (loc->bc) {
        if (state->precbc->offset > loc->bc->offset)
            yasm_internal_error(N_("dwarf2 address went backwards?"));
        addr_delta = loc->bc->offset - state->precbc->offset;
    } else
        return 0;       /* ran out of bytecodes!  XXX: do something? */

    /* Generate appropriate opcode(s).  Address can only increment,
     * whereas line number can go backwards.
     */
    line_delta = loc->line - state->line;
    state->line = loc->line;

    /* First handle the line delta */
    if (line_delta < DWARF2_LINE_BASE
        || line_delta >= DWARF2_LINE_BASE+DWARF2_LINE_RANGE) {
        /* Won't fit in special opcode, use (signed) line advance */
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_advance_line,
                                     yasm_intnum_create_int(line_delta));
        line_delta = 0;
    }

    /* Next handle the address delta */
    opcode1 = DWARF2_LINE_OPCODE_BASE + line_delta - DWARF2_LINE_BASE +
        DWARF2_LINE_RANGE * (addr_delta / dbgfmt_dwarf2->min_insn_len);
    opcode2 = DWARF2_LINE_OPCODE_BASE + line_delta - DWARF2_LINE_BASE +
        DWARF2_LINE_RANGE * ((addr_delta - DWARF2_MAX_SPECIAL_ADDR_DELTA) /
                             dbgfmt_dwarf2->min_insn_len);
    if (line_delta == 0 && addr_delta == 0) {
        /* Both line and addr deltas are 0: do DW_LNS_copy */
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_copy, NULL);
    } else if (addr_delta <= DWARF2_MAX_SPECIAL_ADDR_DELTA && opcode1 <= 255) {
        /* Addr delta in range of special opcode */
        dwarf2_dbgfmt_append_line_op(debug_line, opcode1, NULL);
    } else if (addr_delta <= 2*DWARF2_MAX_SPECIAL_ADDR_DELTA
               && opcode2 <= 255) {
        /* Addr delta in range of const_add_pc + special */
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_const_add_pc, NULL);
        dwarf2_dbgfmt_append_line_op(debug_line, opcode2, NULL);
    } else {
        /* Need advance_pc */
        dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_advance_pc,
                                     yasm_intnum_create_uint(addr_delta));
        /* Take care of any remaining line_delta and add entry to matrix */
        if (line_delta == 0)
            dwarf2_dbgfmt_append_line_op(debug_line, DW_LNS_copy, NULL);
        else {
            unsigned int opcode;
            opcode = DWARF2_LINE_OPCODE_BASE + line_delta - DWARF2_LINE_BASE;
            dwarf2_dbgfmt_append_line_op(debug_line, opcode, NULL);
        }
    }
    state->precbc = loc->bc;
    return 0;
}

typedef struct dwarf2_line_bc_info {
    yasm_section *debug_line;
    yasm_object *object;
    yasm_linemap *linemap;
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2;
    dwarf2_line_state *state;
    dwarf2_loc loc;
    unsigned long lastfile;
} dwarf2_line_bc_info;

static int
dwarf2_filename_equals(const dwarf2_filename *fn,
                       char **dirs,
                       const char *pathname,
                       unsigned long dirlen,
                       const char *filename)
{
    /* check directory */
    if (fn->dir == 0) {
        if (dirlen != 0)
            return 0;
    } else {
        if (strncmp(dirs[fn->dir-1], pathname, dirlen) != 0 ||
            dirs[fn->dir-1][dirlen] != '\0')
            return 0;
    }

    /* check filename */
    return strcmp(fn->filename, filename) == 0;
}

static int
dwarf2_generate_line_bc(yasm_bytecode *bc, /*@null@*/ void *d)
{
    dwarf2_line_bc_info *info = (dwarf2_line_bc_info *)d;
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = info->dbgfmt_dwarf2;
    unsigned long i;
    size_t dirlen;
    const char *pathname, *filename;
    /*@null@*/ yasm_bytecode *nextbc = yasm_bc__next(bc);

    if (nextbc && bc->offset == nextbc->offset)
        return 0;

    info->loc.vline = bc->line;
    info->loc.bc = bc;

    /* Keep track of last symbol seen */
    if (bc->symrecs) {
        i = 0;
        while (bc->symrecs[i]) {
            info->loc.sym = bc->symrecs[i];
            i++;
        }
    }

    yasm_linemap_lookup(info->linemap, bc->line, &pathname, &info->loc.line);
    dirlen = yasm__splitpath(pathname, &filename);

    /* Find file index; just linear search it unless it was the last used */
    if (info->lastfile > 0
        && dwarf2_filename_equals(&dbgfmt_dwarf2->filenames[info->lastfile-1],
                                  dbgfmt_dwarf2->dirs, pathname, dirlen,
                                  filename))
        info->loc.file = info->lastfile;
    else {
        for (i=0; i<dbgfmt_dwarf2->filenames_size; i++) {
            if (dwarf2_filename_equals(&dbgfmt_dwarf2->filenames[i],
                                       dbgfmt_dwarf2->dirs, pathname, dirlen,
                                       filename))
                break;
        }
        if (i >= dbgfmt_dwarf2->filenames_size)
            yasm_internal_error(N_("could not find filename in table"));
        info->loc.file = i+1;
        info->lastfile = i+1;
    }
    if (dwarf2_dbgfmt_gen_line_op(info->debug_line, info->state, &info->loc,
                                  NULL))
        return 1;
    return 0;
}

typedef struct dwarf2_line_info {
    yasm_section *debug_line;   /* section to which line number info goes */
    yasm_object *object;
    yasm_linemap *linemap;
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2;
    yasm_errwarns *errwarns;

    /* Generate based on bytecodes (1) or locs (0)?  Use bytecodes if we're
     * generating line numbers for the actual assembly source file.
     */
    int asm_source;

    /* number of sections line number info generated for */
    size_t num_sections;
    /* last section line number info generated for */
    /*@null@*/ yasm_section *last_code;
} dwarf2_line_info;

static int
dwarf2_generate_line_section(yasm_section *sect, /*@null@*/ void *d)
{
    dwarf2_line_info *info = (dwarf2_line_info *)d;
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = info->dbgfmt_dwarf2;
    /*@null@*/ dwarf2_section_data *dsd;
    /*@null@*/ yasm_bytecode *bc;
    dwarf2_line_state state;
    unsigned long addr_delta;

    dsd = yasm_section_get_data(sect, &yasm_dwarf2__section_data_cb);
    if (!dsd) {
        if (info->asm_source && yasm_section_is_code(sect)) {
            /* Create line data for asm code sections */
            dsd = yasm_xmalloc(sizeof(dwarf2_section_data));
            STAILQ_INIT(&dsd->locs);
            yasm_section_add_data(sect, &yasm_dwarf2__section_data_cb, dsd);
        } else
            return 0;   /* no line data for this section */
    }

    info->num_sections++;
    info->last_code = sect;

    /* initialize state machine registers for each sequence */
    state.dbgfmt_dwarf2 = dbgfmt_dwarf2;
    state.address = 0;
    state.file = 1;
    state.line = 1;
    state.column = 0;
    state.isa = 0;
    state.is_stmt = DWARF2_LINE_DEFAULT_IS_STMT;
    state.precbc = NULL;

    if (info->asm_source) {
        dwarf2_line_bc_info bcinfo;

        bcinfo.debug_line = info->debug_line;
        bcinfo.object = info->object;
        bcinfo.linemap = info->linemap;
        bcinfo.dbgfmt_dwarf2 = dbgfmt_dwarf2;
        bcinfo.state = &state;
        bcinfo.lastfile = 0;
        bcinfo.loc.isa_change = 0;
        bcinfo.loc.column = 0;
        bcinfo.loc.discriminator = 0;
        bcinfo.loc.is_stmt = IS_STMT_NOCHANGE;
        bcinfo.loc.basic_block = 0;
        bcinfo.loc.prologue_end = 0;
        bcinfo.loc.epilogue_begin = 0;
        bcinfo.loc.sym = NULL;

        /* bcs_traverse() skips first "dummy" bytecode, so look at it
         * separately to determine the initial symrec.
         */
        bc = yasm_section_bcs_first(sect);
        if (bc->symrecs) {
            size_t i = 0;
            while (bc->symrecs[i]) {
                bcinfo.loc.sym = bc->symrecs[i];
                i++;
            }
        }

        yasm_section_bcs_traverse(sect, info->errwarns, &bcinfo,
                                  dwarf2_generate_line_bc);
    } else {
        /*@null@*/ dwarf2_loc *loc;

        dwarf2_dbgfmt_finalize_locs(sect, dsd);

        STAILQ_FOREACH(loc, &dsd->locs, link) {
            if (dwarf2_dbgfmt_gen_line_op(info->debug_line, &state, loc,
                                          STAILQ_NEXT(loc, link)))
                return 1;
        }
    }

    /* End sequence: bring address to end of section, then output end
     * sequence opcode.  Don't use a special opcode to do this as we don't
     * want an extra entry in the line matrix.
     */
    if (!state.precbc)
        state.precbc = yasm_section_bcs_first(sect);
    bc = yasm_section_bcs_last(sect);
    addr_delta = yasm_bc_next_offset(bc) - state.precbc->offset;
    if (addr_delta == DWARF2_MAX_SPECIAL_ADDR_DELTA)
        dwarf2_dbgfmt_append_line_op(info->debug_line, DW_LNS_const_add_pc,
                                     NULL);
    else if (addr_delta > 0)
        dwarf2_dbgfmt_append_line_op(info->debug_line, DW_LNS_advance_pc,
                                     yasm_intnum_create_uint(addr_delta));
    dwarf2_dbgfmt_append_line_ext_op(info->debug_line, DW_LNE_end_sequence, 0,
                                     NULL);

    return 0;
}

static int
dwarf2_generate_filename(const char *filename, void *d)
{
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = (yasm_dbgfmt_dwarf2 *)d;
    dwarf2_dbgfmt_add_file(dbgfmt_dwarf2, 0, filename);
    return 0;
}

yasm_section *
yasm_dwarf2__generate_line(yasm_object *object, yasm_linemap *linemap,
                           yasm_errwarns *errwarns, int asm_source,
                           /*@out@*/ yasm_section **main_code,
                           /*@out@*/ size_t *num_line_sections)
{
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = (yasm_dbgfmt_dwarf2 *)object->dbgfmt;
    dwarf2_line_info info;
    int new;
    size_t i;
    yasm_bytecode *last, *sppbc;
    dwarf2_spp *spp;
    dwarf2_head *head;

    if (asm_source) {
        /* Generate dirs and filenames based on linemap */
        yasm_linemap_traverse_filenames(linemap, dbgfmt_dwarf2,
                                        dwarf2_generate_filename);
    }

    info.num_sections = 0;
    info.last_code = NULL;
    info.asm_source = asm_source;
    info.object = object;
    info.linemap = linemap;
    info.dbgfmt_dwarf2 = dbgfmt_dwarf2;
    info.debug_line = yasm_object_get_general(object, ".debug_line", 1, 0, 0,
                                              &new, 0);
    last = yasm_section_bcs_last(info.debug_line);

    /* header */
    head = yasm_dwarf2__add_head(dbgfmt_dwarf2, info.debug_line, NULL, 0, 0);

    /* statement program prologue */
    spp = yasm_xmalloc(sizeof(dwarf2_spp));
    sppbc = yasm_bc_create_common(&dwarf2_spp_bc_callback, spp, 0);
    sppbc->len = dbgfmt_dwarf2->sizeof_offset + 5 +
        NELEMS(line_opcode_num_operands);

    /* directory list */
    for (i=0; i<dbgfmt_dwarf2->dirs_size; i++)
        sppbc->len += (unsigned long)strlen(dbgfmt_dwarf2->dirs[i])+1;
    sppbc->len++;

    /* filename list */
    for (i=0; i<dbgfmt_dwarf2->filenames_size; i++) {
        if (!dbgfmt_dwarf2->filenames[i].filename) {
            yasm_error_set(YASM_ERROR_GENERAL,
                           N_("dwarf2 file number %d unassigned"), i+1);
            yasm_errwarn_propagate(errwarns, 0);
            continue;
        }
        sppbc->len +=
            (unsigned long)strlen(dbgfmt_dwarf2->filenames[i].filename) + 1 +
            yasm_size_uleb128(dbgfmt_dwarf2->filenames[i].dir) + 2;
    }
    sppbc->len++;
    yasm_dwarf2__append_bc(info.debug_line, sppbc);

    /* statement program */
    yasm_object_sections_traverse(object, (void *)&info,
                                  dwarf2_generate_line_section);

    /* mark end of line information */
    yasm_dwarf2__set_head_end(head, yasm_section_bcs_last(info.debug_line));

    *num_line_sections = info.num_sections;
    if (info.num_sections == 1)
        *main_code = info.last_code;
    else
        *main_code = NULL;
    return info.debug_line;
}

static void
dwarf2_spp_bc_destroy(void *contents)
{
    yasm_xfree(contents);
}

static void
dwarf2_spp_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
dwarf2_spp_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                       void *add_span_data)
{
    yasm_internal_error(N_("tried to calc_len a dwarf2 spp bytecode"));
    /*@notreached@*/
    return 0;
}

static int
dwarf2_spp_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                      unsigned char *bufstart, void *d,
                      yasm_output_value_func output_value,
                      yasm_output_reloc_func output_reloc)
{
    yasm_object *object = yasm_section_get_object(bc->section);
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = (yasm_dbgfmt_dwarf2 *)object->dbgfmt;
    unsigned char *buf = *bufp;
    yasm_intnum *cval;
    size_t i, len;

    /* Prologue length (following this field) */
    cval = yasm_intnum_create_uint(bc->len - (unsigned long)(buf-*bufp) -
                                   dbgfmt_dwarf2->sizeof_offset);
    yasm_arch_intnum_tobytes(object->arch, cval, buf,
                             dbgfmt_dwarf2->sizeof_offset,
                             dbgfmt_dwarf2->sizeof_offset*8, 0, bc, 0);
    buf += dbgfmt_dwarf2->sizeof_offset;

    YASM_WRITE_8(buf, dbgfmt_dwarf2->min_insn_len);     /* minimum_instr_len */
    YASM_WRITE_8(buf, DWARF2_LINE_DEFAULT_IS_STMT);     /* default_is_stmt */
    YASM_WRITE_8(buf, DWARF2_LINE_BASE);                /* line_base */
    YASM_WRITE_8(buf, DWARF2_LINE_RANGE);               /* line_range */
    YASM_WRITE_8(buf, DWARF2_LINE_OPCODE_BASE);         /* opcode_base */

    /* Standard opcode # operands array */
    for (i=0; i<NELEMS(line_opcode_num_operands); i++)
        YASM_WRITE_8(buf, line_opcode_num_operands[i]);

    /* directory list */
    for (i=0; i<dbgfmt_dwarf2->dirs_size; i++) {
        len = strlen(dbgfmt_dwarf2->dirs[i])+1;
        memcpy(buf, dbgfmt_dwarf2->dirs[i], len);
        buf += len;
    }
    /* finish with single 0 byte */
    YASM_WRITE_8(buf, 0);

    /* filename list */
    for (i=0; i<dbgfmt_dwarf2->filenames_size; i++) {
        len = strlen(dbgfmt_dwarf2->filenames[i].filename)+1;
        memcpy(buf, dbgfmt_dwarf2->filenames[i].filename, len);
        buf += len;

        /* dir */
        buf += yasm_get_uleb128(dbgfmt_dwarf2->filenames[i].dir, buf);
        YASM_WRITE_8(buf, 0);   /* time */
        YASM_WRITE_8(buf, 0);   /* length */
    }
    /* finish with single 0 byte */
    YASM_WRITE_8(buf, 0);

    *bufp = buf;

    yasm_intnum_destroy(cval);
    return 0;
}

static void
dwarf2_line_op_bc_destroy(void *contents)
{
    dwarf2_line_op *line_op = (dwarf2_line_op *)contents;
    if (line_op->operand)
        yasm_intnum_destroy(line_op->operand);
    if (line_op->ext_operand_int)
        yasm_intnum_destroy(line_op->ext_operand_int);
    yasm_xfree(contents);
}

static void
dwarf2_line_op_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
dwarf2_line_op_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                           void *add_span_data)
{
    yasm_internal_error(N_("tried to calc_len a dwarf2 line_op bytecode"));
    /*@notreached@*/
    return 0;
}

static int
dwarf2_line_op_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                          unsigned char *bufstart, void *d,
                          yasm_output_value_func output_value,
                          yasm_output_reloc_func output_reloc)
{
    dwarf2_line_op *line_op = (dwarf2_line_op *)bc->contents;
    unsigned char *buf = *bufp;

    YASM_WRITE_8(buf, line_op->opcode);
    if (line_op->operand)
        buf += yasm_intnum_get_leb128(line_op->operand, buf,
                                      line_op->opcode == DW_LNS_advance_line);
    if (line_op->ext_opcode > 0) {
        YASM_WRITE_8(buf, line_op->ext_opcode);
        if (line_op->ext_operand) {
            yasm_value value;
            yasm_value_init_sym(&value, line_op->ext_operand,
                                line_op->ext_operandsize*8);
            output_value(&value, buf, line_op->ext_operandsize,
                         (unsigned long)(buf-bufstart), bc, 0, d);
            buf += line_op->ext_operandsize;
        }
        if (line_op->ext_operand_int) {
            buf += yasm_intnum_get_leb128(line_op->ext_operand_int, buf, 0);
        }
    }

    *bufp = buf;
    return 0;
}

void
yasm_dwarf2__dir_loc(yasm_object *object, yasm_valparamhead *valparams,
                     yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp;
    int in_is_stmt = 0, in_isa = 0, in_discriminator = 0;

    /*@dependent@*/ /*@null@*/ const yasm_intnum *intn;
    dwarf2_section_data *dsd;
    dwarf2_loc *loc = yasm_xmalloc(sizeof(dwarf2_loc));

    /* File number (required) */
    if (!valparams || !(vp = yasm_vps_first(valparams)) ||
        vp->val || vp->type != YASM_PARAM_EXPR) {
        yasm_error_set(YASM_ERROR_SYNTAX, N_("file number required"));
        yasm_xfree(loc);
        return;
    }
    intn = yasm_expr_get_intnum(&vp->param.e, 0);
    if (!intn) {
        yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                       N_("file number is not a constant"));
        yasm_xfree(loc);
        return;
    }
    if (yasm_intnum_sign(intn) != 1) {
        yasm_error_set(YASM_ERROR_VALUE, N_("file number less than one"));
        yasm_xfree(loc);
        return;
    }
    loc->file = yasm_intnum_get_uint(intn);

    /* Line number (required) */
    vp = yasm_vps_next(vp);
    if (!vp || vp->val || vp->type != YASM_PARAM_EXPR) {
        yasm_error_set(YASM_ERROR_SYNTAX, N_("line number required"));
        yasm_xfree(loc);
        return;
    }
    intn = yasm_expr_get_intnum(&vp->param.e, 0);
    if (!intn) {
        yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                       N_("line number is not a constant"));
        yasm_xfree(loc);
        return;
    }
    loc->line = yasm_intnum_get_uint(intn);

    /* Generate new section data if it doesn't already exist */
    if (!object->cur_section) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("[%s] can only be used inside of a section"), "loc");
        yasm_xfree(loc);
        return;
    }
    dsd = yasm_section_get_data(object->cur_section,
                                &yasm_dwarf2__section_data_cb);
    if (!dsd) {
        dsd = yasm_xmalloc(sizeof(dwarf2_section_data));
        STAILQ_INIT(&dsd->locs);
        yasm_section_add_data(object->cur_section,
                              &yasm_dwarf2__section_data_cb, dsd);
    }

    /* Defaults for optional settings */
    loc->column = 0;
    loc->discriminator = 0;
    loc->isa_change = 0;
    loc->isa = 0;
    loc->is_stmt = IS_STMT_NOCHANGE;
    loc->basic_block = 0;
    loc->prologue_end = 0;
    loc->epilogue_begin = 0;

    /* Optional column number */
    vp = yasm_vps_next(vp);
    if (vp && !vp->val && vp->type == YASM_PARAM_EXPR) {
        intn = yasm_expr_get_intnum(&vp->param.e, 0);
        if (!intn) {
            yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                           N_("column number is not a constant"));
            yasm_xfree(loc);
            return;
        }
        loc->column = yasm_intnum_get_uint(intn);
        vp = yasm_vps_next(vp);
    }

    /* Other options; note for GAS compatibility we need to support both:
     * is_stmt=1 (NASM) and
     * is_stmt 1 (GAS)
     */
    while (vp) {
        /*@null@*/ /*@dependent@*/ const char *s;
        /*@null@*/ /*@only@*/ yasm_expr *e;

restart:
        if (in_is_stmt) {
            in_is_stmt = 0;
            if (!(e = yasm_vp_expr(vp, object->symtab, line)) ||
                !(intn = yasm_expr_get_intnum(&e, 0))) {
                yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                               N_("is_stmt value is not a constant"));
                yasm_xfree(loc);
                if (e)
                    yasm_expr_destroy(e);
                return;
            }
            if (yasm_intnum_is_zero(intn))
                loc->is_stmt = IS_STMT_SET;
            else if (yasm_intnum_is_pos1(intn))
                loc->is_stmt = IS_STMT_CLEAR;
            else {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("is_stmt value not 0 or 1"));
                yasm_xfree(loc);
                yasm_expr_destroy(e);
                return;
            }
            yasm_expr_destroy(e);
        } else if (in_isa) {
            in_isa = 0;
            if (!(e = yasm_vp_expr(vp, object->symtab, line)) ||
                !(intn = yasm_expr_get_intnum(&e, 0))) {
                yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                               N_("isa value is not a constant"));
                yasm_xfree(loc);
                if (e)
                    yasm_expr_destroy(e);
                return;
            }
            if (yasm_intnum_sign(intn) < 0) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("isa value less than zero"));
                yasm_xfree(loc);
                yasm_expr_destroy(e);
                return;
            }
            loc->isa_change = 1;
            loc->isa = yasm_intnum_get_uint(intn);
            yasm_expr_destroy(e);
        } else if (in_discriminator) {
            in_discriminator = 0;
            if (!(e = yasm_vp_expr(vp, object->symtab, line)) ||
                !(intn = yasm_expr_get_intnum(&e, 0))) {
                yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                               N_("discriminator value is not a constant"));
                yasm_xfree(loc);
                if (e)
                    yasm_expr_destroy(e);
                return;
            }
            if (yasm_intnum_sign(intn) < 0) {
                yasm_error_set(YASM_ERROR_VALUE,
                               N_("discriminator value less than zero"));
                yasm_xfree(loc);
                yasm_expr_destroy(e);
                return;
            }
            loc->discriminator = yasm_intnum_get_uint(intn);
            yasm_expr_destroy(e);
        } else if (!vp->val && (s = yasm_vp_id(vp))) {
            if (yasm__strcasecmp(s, "is_stmt") == 0)
                in_is_stmt = 1;
            else if (yasm__strcasecmp(s, "isa") == 0)
                in_isa = 1;
            else if (yasm__strcasecmp(s, "discriminator") == 0)
                in_discriminator = 1;
            else if (yasm__strcasecmp(s, "basic_block") == 0)
                loc->basic_block = 1;
            else if (yasm__strcasecmp(s, "prologue_end") == 0)
                loc->prologue_end = 1;
            else if (yasm__strcasecmp(s, "epilogue_begin") == 0)
                loc->epilogue_begin = 1;
            else
                yasm_warn_set(YASM_WARN_GENERAL,
                              N_("unrecognized loc option `%s'"), s);
        } else if (!vp->val) {
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("unrecognized numeric qualifier"));
        } else if (yasm__strcasecmp(vp->val, "is_stmt") == 0) {
            in_is_stmt = 1;
            goto restart; /* don't go to the next valparam */
        } else if (yasm__strcasecmp(vp->val, "isa") == 0) {
            in_isa = 1;
            goto restart; /* don't go to the next valparam */
        } else if (yasm__strcasecmp(vp->val, "discriminator") == 0) {
            in_discriminator = 1;
            goto restart; /* don't go to the next valparam */
        } else
            yasm_warn_set(YASM_WARN_GENERAL,
                          N_("unrecognized loc option `%s'"), vp->val);
        vp = yasm_vps_next(vp);
    }

    if (in_is_stmt || in_isa || in_discriminator) {
        yasm_error_set(YASM_ERROR_SYNTAX, N_("%s requires value"),
                       in_is_stmt ? "is_stmt" :
                       (in_isa ? "isa" : "discriminator"));
        yasm_xfree(loc);
        return;
    }

    /* Append new location */
    loc->vline = line;
    loc->bc = NULL;
    loc->sym = NULL;
    STAILQ_INSERT_TAIL(&dsd->locs, loc, link);
}

void
yasm_dwarf2__dir_file(yasm_object *object, yasm_valparamhead *valparams,
                      yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_dbgfmt_dwarf2 *dbgfmt_dwarf2 = (yasm_dbgfmt_dwarf2 *)object->dbgfmt;
    yasm_valparam *vp;
    /*@dependent@*/ /*@null@*/ const yasm_intnum *file_intn;
    unsigned long filenum;

    if (!valparams) {
        yasm_error_set(YASM_ERROR_SYNTAX, N_("[%s] requires an argument"),
                       "FILE");
        return;
    }

    vp = yasm_vps_first(valparams);
    if (yasm_vp_string(vp)) {
        /* Just a bare filename */
        yasm_object_set_source_fn(object, yasm_vp_string(vp));
        return;
    }

    /* Otherwise.. first vp is the file number */
    if (vp->type != YASM_PARAM_EXPR ||
        !(file_intn = yasm_expr_get_intnum(&vp->param.e, 0))) {
        yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                       N_("file number is not a constant"));
        return;
    }
    filenum = yasm_intnum_get_uint(file_intn);

    vp = yasm_vps_next(vp);
    if (!yasm_vp_string(vp)) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("file number given but no filename"));
        return;
    }

    dwarf2_dbgfmt_add_file(dbgfmt_dwarf2, filenum, yasm_vp_string(vp));
}
