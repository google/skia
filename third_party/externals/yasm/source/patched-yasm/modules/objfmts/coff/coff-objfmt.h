/*
 * COFF (DJGPP) object format
 *
 *  Copyright (C) 2007  Peter Johnson
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
#ifndef COFF_OBJFMT_H
#define COFF_OBJFMT_H

typedef struct coff_unwind_code {
    SLIST_ENTRY(coff_unwind_code) link;

    /*@dependent@*/ yasm_symrec *proc;      /* Start of procedure */
    /*@dependent@*/ yasm_symrec *loc;       /* Location of operation */
    /* Unwind operation code */
    enum {
        UWOP_PUSH_NONVOL = 0,
        UWOP_ALLOC_LARGE = 1,
        UWOP_ALLOC_SMALL = 2,
        UWOP_SET_FPREG = 3,
        UWOP_SAVE_NONVOL = 4,
        UWOP_SAVE_NONVOL_FAR = 5,
        UWOP_SAVE_XMM128 = 8,
        UWOP_SAVE_XMM128_FAR = 9,
        UWOP_PUSH_MACHFRAME = 10
    } opcode;
    unsigned int info;          /* Operation info */
    yasm_value off;             /* Offset expression (used for some codes) */
} coff_unwind_code;

typedef struct coff_unwind_info {
    /*@dependent@*/ yasm_symrec *proc;      /* Start of procedure */
    /*@dependent@*/ yasm_symrec *prolog;    /* End of prologue */

    /*@null@*/ /*@dependent@*/ yasm_symrec *ehandler;   /* Error handler */

    unsigned long framereg;     /* Frame register */
    yasm_value frameoff;        /* Frame offset */

    /* Linked list of codes, in decreasing location offset order.
     * Inserting at the head of this list during assembly naturally results
     * in this sorting.
     */
    SLIST_HEAD(coff_unwind_code_head, coff_unwind_code) codes;

    /* These aren't used until inside of generate. */
    yasm_value prolog_size;
    yasm_value codes_count;
} coff_unwind_info;

coff_unwind_info *yasm_win64__uwinfo_create(void);
void yasm_win64__uwinfo_destroy(coff_unwind_info *info);
void yasm_win64__unwind_generate(yasm_section *xdata,
                                 /*@only@*/ coff_unwind_info *info,
                                 unsigned long line);

#endif
