/*
 * YASM assembler virtual line mapping handling (for parse stage)
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
#include "util.h"

#include "coretype.h"
#include "hamt.h"

#include "errwarn.h"
#include "linemap.h"


typedef struct line_mapping {
    /* monotonically increasing virtual line */
    unsigned long line;

    /* related info */
    /* "original" source filename */
    /*@null@*/ /*@dependent@*/ const char *filename;
    /* "original" source base line number */
    unsigned long file_line;
    /* "original" source line number increment (for following lines) */
    unsigned long line_inc;
} line_mapping;

typedef struct line_source_info {
    /* first bytecode on line; NULL if no bytecodes on line */
    /*@null@*/ /*@dependent@*/ yasm_bytecode *bc;

    /* source code line */
    /*@owned@*/ char *source;
} line_source_info;

struct yasm_linemap {
    /* Shared storage for filenames */
    /*@only@*/ /*@null@*/ HAMT *filenames;

    /* Current virtual line number. */
    unsigned long current;

    /* Mappings from virtual to physical line numbers */
    struct line_mapping *map_vector;
    unsigned long map_size;
    unsigned long map_allocated;

    /* Bytecode and source line information */
    /*@only@*/ line_source_info *source_info;
    size_t source_info_size;
};

static void
filename_delete_one(/*@only@*/ void *d)
{
    yasm_xfree(d);
}

void
yasm_linemap_set(yasm_linemap *linemap, const char *filename,
                 unsigned long virtual_line, unsigned long file_line,
                 unsigned long line_inc)
{
    char *copy;
    unsigned long i;
    int replace = 0;
    line_mapping *mapping = NULL;

    if (virtual_line == 0) {
        virtual_line = linemap->current;
    }

    /* Replace all existing mappings that have line numbers >= this one. */
    for (i = linemap->map_size; i > 0; i--) {
        if (linemap->map_vector[i-1].line < virtual_line) {
            if (i < linemap->map_size) {
                mapping = &linemap->map_vector[i];
                linemap->map_size = i + 1;
            }
            break;
        }
    }

    if (mapping == NULL) {
        /* Create a new mapping in the map */
        if (linemap->map_size >= linemap->map_allocated) {
            /* allocate another size bins when full for 2x space */
            linemap->map_vector = yasm_xrealloc(linemap->map_vector,
                2*linemap->map_allocated*sizeof(line_mapping));
            linemap->map_allocated *= 2;
        }
        mapping = &linemap->map_vector[linemap->map_size];
        linemap->map_size++;
    }

    /* Fill it */

    if (!filename) {
        if (linemap->map_size >= 2)
            mapping->filename =
                linemap->map_vector[linemap->map_size-2].filename;
        else
            filename = "unknown";
    }
    if (filename) {
        /* Copy the filename (via shared storage) */
        copy = yasm__xstrdup(filename);
        /*@-aliasunique@*/
        mapping->filename = HAMT_insert(linemap->filenames, copy, copy,
                                        &replace, filename_delete_one);
        /*@=aliasunique@*/
    }

    mapping->line = virtual_line;
    mapping->file_line = file_line;
    mapping->line_inc = line_inc;
}

unsigned long
yasm_linemap_poke(yasm_linemap *linemap, const char *filename,
                  unsigned long file_line)
{
    unsigned long line;
    line_mapping *mapping;

    linemap->current++;
    yasm_linemap_set(linemap, filename, 0, file_line, 0);

    mapping = &linemap->map_vector[linemap->map_size-1];

    line = linemap->current;

    linemap->current++;
    yasm_linemap_set(linemap, mapping->filename, 0,
                     mapping->file_line +
                     mapping->line_inc*(linemap->current-2-mapping->line),
                     mapping->line_inc);

    return line;
}

yasm_linemap *
yasm_linemap_create(void)
{
    size_t i;
    yasm_linemap *linemap = yasm_xmalloc(sizeof(yasm_linemap));

    linemap->filenames = HAMT_create(0, yasm_internal_error_);

    linemap->current = 1;

    /* initialize mapping vector */
    linemap->map_vector = yasm_xmalloc(8*sizeof(line_mapping));
    linemap->map_size = 0;
    linemap->map_allocated = 8;
    
    /* initialize source line information array */
    linemap->source_info_size = 2;
    linemap->source_info = yasm_xmalloc(linemap->source_info_size *
                                        sizeof(line_source_info));
    for (i=0; i<linemap->source_info_size; i++) {
        linemap->source_info[i].bc = NULL;
        linemap->source_info[i].source = NULL;
    }

    return linemap;
}

void
yasm_linemap_destroy(yasm_linemap *linemap)
{
    size_t i;
    for (i=0; i<linemap->source_info_size; i++) {
        if (linemap->source_info[i].source)
            yasm_xfree(linemap->source_info[i].source);
    }
    yasm_xfree(linemap->source_info);

    yasm_xfree(linemap->map_vector);

    if (linemap->filenames)
        HAMT_destroy(linemap->filenames, filename_delete_one);

    yasm_xfree(linemap);
}

unsigned long
yasm_linemap_get_current(yasm_linemap *linemap)
{
    return linemap->current;
}

void
yasm_linemap_add_source(yasm_linemap *linemap, yasm_bytecode *bc,
                        const char *source)
{
    size_t i;

    while (linemap->current > linemap->source_info_size) {
        /* allocate another size bins when full for 2x space */
        linemap->source_info = yasm_xrealloc(linemap->source_info,
            2*linemap->source_info_size*sizeof(line_source_info));
        for (i=linemap->source_info_size; i<linemap->source_info_size*2; i++) {
            linemap->source_info[i].bc = NULL;
            linemap->source_info[i].source = NULL;
        }
        linemap->source_info_size *= 2;
    }

    /* Delete existing info for that line (if any) */
    if (linemap->source_info[linemap->current-1].source)
        yasm_xfree(linemap->source_info[linemap->current-1].source);

    linemap->source_info[linemap->current-1].bc = bc;
    linemap->source_info[linemap->current-1].source = yasm__xstrdup(source);
}

unsigned long
yasm_linemap_goto_next(yasm_linemap *linemap)
{
    return ++(linemap->current);
}

void
yasm_linemap_lookup(yasm_linemap *linemap, unsigned long line,
                    const char **filename, unsigned long *file_line)
{
    line_mapping *mapping;
    unsigned long vindex, step;

    assert(line <= linemap->current);

    /* Binary search through map to find highest line_index <= index */
    vindex = 0;
    /* start step as the greatest power of 2 <= size */
    step = 1;
    while (step*2<=linemap->map_size)
        step*=2;
    while (step>0) {
        if (vindex+step < linemap->map_size
                && linemap->map_vector[vindex+step].line <= line)
            vindex += step;
        step /= 2;
    }
    mapping = &linemap->map_vector[vindex];

    *filename = mapping->filename;
    *file_line = (line ? mapping->file_line + mapping->line_inc*(line-mapping->line) : 0);
}

int
yasm_linemap_traverse_filenames(yasm_linemap *linemap, /*@null@*/ void *d,
                                int (*func) (const char *filename, void *d))
{
    return HAMT_traverse(linemap->filenames, d, (int (*) (void *, void *))func);
}

int
yasm_linemap_get_source(yasm_linemap *linemap, unsigned long line,
                        yasm_bytecode **bcp, const char **sourcep)
{
    if (line > linemap->source_info_size) {
        *bcp = NULL;
        *sourcep = NULL;
        return 1;
    }

    *bcp = linemap->source_info[line-1].bc;
    *sourcep = linemap->source_info[line-1].source;

    return (!(*sourcep));
}
