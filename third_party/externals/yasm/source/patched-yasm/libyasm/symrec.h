/**
 * \file libyasm/symrec.h
 * \brief YASM symbol table interface.
 *
 * \license
 *  Copyright (C) 2001-2007  Michael Urman, Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
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
 * \endlicense
 */
#ifndef YASM_SYMREC_H
#define YASM_SYMREC_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Symbol status.  YASM_SYM_DEFINED is set by yasm_symtab_define_label(),
 * yasm_symtab_define_equ(), or yasm_symtab_declare()/yasm_symrec_declare()
 * with a visibility of #YASM_SYM_EXTERN or #YASM_SYM_COMMON.
 */
typedef enum yasm_sym_status {
    YASM_SYM_NOSTATUS = 0,          /**< no status */
    YASM_SYM_USED = 1 << 0,         /**< for use before definition */
    YASM_SYM_DEFINED = 1 << 1,      /**< once it's been defined in the file */
    YASM_SYM_VALUED = 1 << 2,       /**< once its value has been determined */
    YASM_SYM_NOTINTABLE = 1 << 3    /**< if it's not in sym_table (ex. '$') */
} yasm_sym_status;

/** Symbol record visibility.
 * \note YASM_SYM_EXTERN and YASM_SYM_COMMON are mutually exclusive.
 */
typedef enum yasm_sym_vis {
    YASM_SYM_LOCAL = 0,         /**< Default, local only */
    YASM_SYM_GLOBAL = 1 << 0,   /**< If symbol is declared GLOBAL */
    YASM_SYM_COMMON = 1 << 1,   /**< If symbol is declared COMMON */
    YASM_SYM_EXTERN = 1 << 2,   /**< If symbol is declared EXTERN */
    YASM_SYM_DLOCAL = 1 << 3    /**< If symbol is explicitly declared LOCAL */
} yasm_sym_vis;

/** Create a new symbol table. */
YASM_LIB_DECL
yasm_symtab *yasm_symtab_create(void);

/** Destroy a symbol table and all internal symbols.
 * \param symtab    symbol table
 * \warning All yasm_symrec *'s into this symbol table become invalid after
 * this is called!
 */
YASM_LIB_DECL
void yasm_symtab_destroy(/*@only@*/ yasm_symtab *symtab);

/** Set the symbol table to be case sensitive or not.
 * Should be called before adding any symbol.
 * \param symtab    symbol table
 * \param sensitive whether the symbol table should be case sensitive.
 */
YASM_LIB_DECL
void yasm_symtab_set_case_sensitive(yasm_symtab *symtab, int sensitive);

/** Get a reference to the symbol table's "absolute" symbol.  This is
 * essentially an EQU with no name and value 0, and is used for relocating
 * absolute current-position-relative values.
 * \see yasm_value_set_curpos_rel().
 * \param symtab    symbol table
 * \return Absolute symbol (dependent pointer, do not free).
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_symrec *yasm_symtab_abs_sym(yasm_symtab *symtab);

/** Get a reference to (use) a symbol.  The symbol does not necessarily need to
 * be defined before it is used.
 * \param symtab    symbol table
 * \param name      symbol name
 * \param line      virtual line where referenced
 * \return Symbol (dependent pointer, do not free).
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_symrec *yasm_symtab_use
    (yasm_symtab *symtab, const char *name, unsigned long line);

/** Get a reference to a symbol, without "using" it.  Should be used for cases
 * when an internal assembler usage of a symbol shouldn't be treated like a
 * normal user symbol usage.
 * \param symtab    symbol table
 * \param name      symbol name
 * \return Symbol (dependent pointer, do not free).  May be NULL if symbol
 *         doesn't exist.
 */
YASM_LIB_DECL
/*@null@*/ /*@dependent@*/ yasm_symrec *yasm_symtab_get
    (yasm_symtab *symtab, const char *name);

/** Define a symbol as an EQU value.
 * \param symtab    symbol table
 * \param name      symbol (EQU) name
 * \param e         EQU value (expression)
 * \param line      virtual line of EQU
 * \return Symbol (dependent pointer, do not free).
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_symrec *yasm_symtab_define_equ
    (yasm_symtab *symtab, const char *name, /*@keep@*/ yasm_expr *e,
     unsigned long line);

/** Define a symbol as a label.
 * \param symtab    symbol table
 * \param name      symbol (label) name
 * \param precbc    bytecode preceding label
 * \param in_table  nonzero if the label should be inserted into the symbol
 *                  table (some specially-generated ones should not be)
 * \param line      virtual line of label
 * \return Symbol (dependent pointer, do not free).
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_symrec *yasm_symtab_define_label
    (yasm_symtab *symtab, const char *name,
     /*@dependent@*/ yasm_bytecode *precbc, int in_table, unsigned long line);

/** Define a symbol as a label representing the current assembly position.
 * This should be used for this purpose instead of yasm_symtab_define_label()
 * as value_finalize_scan() looks for usage of this symbol type for special
 * handling.  The symbol created is not inserted into the symbol table.
 * \param symtab    symbol table
 * \param name      symbol (label) name
 * \param precbc    bytecode preceding label
 * \param line      virtual line of label
 * \return Symbol (dependent pointer, do not free).
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_symrec *yasm_symtab_define_curpos
    (yasm_symtab *symtab, const char *name,
     /*@dependent@*/ yasm_bytecode *precbc, unsigned long line);

/** Define a special symbol that will appear in the symbol table and have a
 * defined name, but have no other data associated with it within the
 * standard symrec.
 * \param symtab    symbol table
 * \param name      symbol name
 * \param vis       symbol visibility
 * \return Symbol (dependent pointer, do not free).
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_symrec *yasm_symtab_define_special
    (yasm_symtab *symtab, const char *name, yasm_sym_vis vis);

/** Declare external visibility of a symbol.
 * \note Not all visibility combinations are allowed.
 * \param symtab    symbol table
 * \param name      symbol name
 * \param vis       visibility
 * \param line      virtual line of visibility-setting
 * \return Symbol (dependent pointer, do not free).
 */
YASM_LIB_DECL
/*@dependent@*/ yasm_symrec *yasm_symtab_declare
    (yasm_symtab *symtab, const char *name, yasm_sym_vis vis,
     unsigned long line);

/** Declare external visibility of a symbol.
 * \note Not all visibility combinations are allowed.
 * \param symrec    symbol
 * \param vis       visibility
 * \param line      virtual line of visibility-setting
 */
YASM_LIB_DECL
void yasm_symrec_declare(yasm_symrec *symrec, yasm_sym_vis vis,
                         unsigned long line);

/** Callback function for yasm_symrec_traverse().
 * \param sym       symbol
 * \param d         data passed into yasm_symrec_traverse()
 * \return Nonzero to stop symbol traversal.
 */
typedef int (*yasm_symtab_traverse_callback)
    (yasm_symrec *sym, /*@null@*/ void *d);

/** Traverse all symbols in the symbol table.
 * \param symtab    symbol table
 * \param d         data to pass to each call of callback function
 * \param func      callback function called on each symbol
 * \return Nonzero value returned by callback function if it ever returned
 *         nonzero.
 */
YASM_LIB_DECL
int /*@alt void@*/ yasm_symtab_traverse
    (yasm_symtab *symtab, /*@null@*/ void *d,
     yasm_symtab_traverse_callback func);

/** Symbol table iterator (opaque type). */
typedef struct yasm_symtab_iter yasm_symtab_iter;

/** Get an iterator pointing to the first symbol in the symbol table.
 * \param symtab    symbol table
 * \return Iterator for the symbol table.
 */
YASM_LIB_DECL
const yasm_symtab_iter *yasm_symtab_first(const yasm_symtab *symtab);

/** Move a symbol table iterator to the next symbol in the symbol table.
 * \param prev          Previous iterator value
 * \return Next iterator value, or NULL if no more symbols in the table.
 */
YASM_LIB_DECL
/*@null@*/ const yasm_symtab_iter *yasm_symtab_next
    (const yasm_symtab_iter *prev);

/** Get the symbol corresponding to the current symbol table iterator value.
 * \param cur           iterator value
 * \return Corresponding symbol.
 */
YASM_LIB_DECL
yasm_symrec *yasm_symtab_iter_value(const yasm_symtab_iter *cur);

/** Finalize symbol table after parsing stage.  Checks for symbols that are
 * used but never defined or declared #YASM_SYM_EXTERN or #YASM_SYM_COMMON.
 * \param symtab        symbol table
 * \param undef_extern  if nonzero, all undef syms should be declared extern
 * \param errwarns      error/warning set
 * \note Errors/warnings are stored into errwarns.
 */
YASM_LIB_DECL
void yasm_symtab_parser_finalize(yasm_symtab *symtab, int undef_extern,
                                 yasm_errwarns *errwarns);

/** Print the symbol table.  For debugging purposes.
 * \param symtab        symbol table
 * \param f             file
 * \param indent_level  indentation level
 */
YASM_LIB_DECL
void yasm_symtab_print(yasm_symtab *symtab, FILE *f, int indent_level);

/** Get the name of a symbol.
 * \param sym       symbol
 * \return Symbol name.
 */
YASM_LIB_DECL
/*@observer@*/ const char *yasm_symrec_get_name(const yasm_symrec *sym);

/** Get the externally-visible (global) name of a symbol.
 * \param sym       symbol
 * \param object    object
 * \return Externally-visible symbol name (allocated, caller must free).
 */
YASM_LIB_DECL
/*@only@*/ char *yasm_symrec_get_global_name(const yasm_symrec *sym,
                                             const yasm_object *object);

/** Get the visibility of a symbol.
 * \param sym       symbol
 * \return Symbol visibility.
 */
YASM_LIB_DECL
yasm_sym_vis yasm_symrec_get_visibility(const yasm_symrec *sym);

/** Get the status of a symbol.
 * \param sym       symbol
 * \return Symbol status.
 */
YASM_LIB_DECL
yasm_sym_status yasm_symrec_get_status(const yasm_symrec *sym);

/** Get the virtual line of where a symbol was first defined.
 * \param sym       symbol
 * \return line     virtual line
 */
YASM_LIB_DECL
unsigned long yasm_symrec_get_def_line(const yasm_symrec *sym);

/** Get the virtual line of where a symbol was first declared.
 * \param sym       symbol
 * \return line     virtual line
 */
YASM_LIB_DECL
unsigned long yasm_symrec_get_decl_line(const yasm_symrec *sym);

/** Get the virtual line of where a symbol was first used.
 * \param sym       symbol
 * \return line     virtual line
 */
YASM_LIB_DECL
unsigned long yasm_symrec_get_use_line(const yasm_symrec *sym);

/** Get EQU value of a symbol.
 * \param sym       symbol
 * \return EQU value, or NULL if symbol is not an EQU or is not defined.
 */
YASM_LIB_DECL
/*@observer@*/ /*@null@*/ const yasm_expr *yasm_symrec_get_equ
    (const yasm_symrec *sym);

/** Dependent pointer to a bytecode. */
typedef /*@dependent@*/ yasm_bytecode *yasm_symrec_get_label_bytecodep;

/** Get the label location of a symbol.
 * \param sym       symbol
 * \param precbc    bytecode preceding label (output)
 * \return 0 if not symbol is not a label or if the symbol's visibility is
 *         #YASM_SYM_EXTERN or #YASM_SYM_COMMON (not defined in the file).
 */
YASM_LIB_DECL
int yasm_symrec_get_label(const yasm_symrec *sym,
                          /*@out@*/ yasm_symrec_get_label_bytecodep *precbc);

/** Set the size of a symbol.
 * \param sym       symbol
 * \param size      size to be set
 */
void yasm_symrec_set_size(yasm_symrec *sym, int size);

/** Get the size of a symbol.
 * \param sym       symbol
 * \return size of the symbol, 0 if none specified by the user.
 */
int yasm_symrec_get_size(const yasm_symrec *sym);

/** Set the segment of a symbol.
 * \param sym       symbol
 * \param segment   segment to be set
 */
void yasm_symrec_set_segment(yasm_symrec *sym, const char *segment);

/** Get the segment of a symbol.
 * \param sym       symbol
 * \return segment of the symbol, NULL if none specified by the user.
 */
const char *yasm_symrec_get_segment(const yasm_symrec *sym);

/** Determine if symbol is the "absolute" symbol created by
 * yasm_symtab_abs_sym().
 * \param sym       symbol
 * \return 0 if symbol is not the "absolute" symbol, nonzero otherwise.
 */
YASM_LIB_DECL
int yasm_symrec_is_abs(const yasm_symrec *sym);

/** Determine if symbol is a special symbol.
 * \param sym       symbol
 * \return 0 if symbol is not a special symbol, nonzero otherwise.
 */
YASM_LIB_DECL
int yasm_symrec_is_special(const yasm_symrec *sym);

/** Determine if symbol is a label representing the current assembly position.
 * \param sym       symbol
 * \return 0 if symbol is not a current position label, nonzero otherwise.
 */
YASM_LIB_DECL
int yasm_symrec_is_curpos(const yasm_symrec *sym);

/** Set object-extended valparams.
 * \param sym                   symbol
 * \param objext_valparams      object-extended valparams
 */
YASM_LIB_DECL
void yasm_symrec_set_objext_valparams
    (yasm_symrec *sym, /*@only@*/ yasm_valparamhead *objext_valparams);

/** Get object-extended valparams, if any, associated with symbol's
 * declaration.
 * \param sym       symbol
 * \return Object-extended valparams (NULL if none).
 */
YASM_LIB_DECL
/*@null@*/ /*@dependent@*/ yasm_valparamhead *yasm_symrec_get_objext_valparams
    (yasm_symrec *sym);

/** Set common size of symbol.
 * \param sym           symbol
 * \param common_size   common size expression
 */
YASM_LIB_DECL
void yasm_symrec_set_common_size
    (yasm_symrec *sym, /*@only@*/ yasm_expr *common_size);

/** Get common size of symbol, if symbol is declared COMMON and a size was set
 * for it.
 * \param sym       symbol
 * \return Common size (NULL if none).
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ yasm_expr **yasm_symrec_get_common_size
    (yasm_symrec *sym);

/** Get associated data for a symbol and data callback.
 * \param sym       symbol
 * \param callback  callback used when adding data
 * \return Associated data (NULL if none).
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ void *yasm_symrec_get_data
    (yasm_symrec *sym, const yasm_assoc_data_callback *callback);

/** Add associated data to a symbol.
 * \attention Deletes any existing associated data for that data callback.
 * \param sym       symbol
 * \param callback  callback
 * \param data      data to associate
 */
YASM_LIB_DECL
void yasm_symrec_add_data(yasm_symrec *sym,
                          const yasm_assoc_data_callback *callback,
                          /*@only@*/ /*@null@*/ void *data);

/** Print a symbol.  For debugging purposes.
 * \param f             file
 * \param indent_level  indentation level
 * \param sym           symbol
 */
YASM_LIB_DECL
void yasm_symrec_print(const yasm_symrec *sym, FILE *f, int indent_level);

#endif
