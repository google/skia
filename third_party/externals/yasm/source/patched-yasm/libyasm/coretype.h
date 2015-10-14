/**
 * \file libyasm/coretype.h
 * \brief YASM core types and utility functions.
 *
 * \license
 *  Copyright (C) 2001-2007  Peter Johnson
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
#ifndef YASM_CORETYPE_H
#define YASM_CORETYPE_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Architecture instance (mostly opaque type).  \see arch.h for details. */
typedef struct yasm_arch yasm_arch;
/** Preprocessor interface.  \see preproc.h for details. */
typedef struct yasm_preproc yasm_preproc;
/** Parser instance (mostly opaque type).  \see parser.h for details. */
typedef struct yasm_parser yasm_parser;
/** Object format interface.  \see objfmt.h for details. */
typedef struct yasm_objfmt yasm_objfmt;
/** Debug format interface.  \see dbgfmt.h for details. */
typedef struct yasm_dbgfmt yasm_dbgfmt;
/** List format interface.  \see listfmt.h for details. */
typedef struct yasm_listfmt yasm_listfmt;

/** Object format module interface.  \see objfmt.h for details. */
typedef struct yasm_objfmt_module yasm_objfmt_module;
/** Debug format module interface.  \see dbgfmt.h for details. */
typedef struct yasm_dbgfmt_module yasm_dbgfmt_module;

/** Standard macro structure for modules that allows association of a set of
 * standard macros with a parser/preprocessor combination.
 * A NULL-terminated array of these structures is used in a number of module
 * interfaces.
 */
typedef struct yasm_stdmac {
    const char *parser;         /**< Parser keyword */
    const char *preproc;        /**< Preprocessor keyword */

    /** NULL-terminated array of standard macros.  May be NULL if no standard
     * macros should be added for this preprocessor.
     */
    const char **macros;
} yasm_stdmac;

/** YASM associated data callback structure.  Many data structures can have
 * arbitrary data associated with them.
 */
typedef struct yasm_assoc_data_callback {
    /** Free memory allocated for associated data.
     * \param data      associated data
     */
    void (*destroy) (/*@only@*/ void *data);

    /** Print a description of allocated data.  For debugging purposes.
     * \param data              associated data
     * \param f                 output file
     * \param indent_level      indentation level
     */
    void (*print) (void *data, FILE *f, int indent_level);
} yasm_assoc_data_callback;

/** Set of collected error/warnings (opaque type).
 * \see errwarn.h for details.
 */
typedef struct yasm_errwarns yasm_errwarns;

/** Bytecode.  \see bytecode.h for details and related functions. */
typedef struct yasm_bytecode yasm_bytecode;

/** Object.  \see section.h for details and related functions. */
typedef struct yasm_object yasm_object;

/** Section (opaque type).  \see section.h for related functions. */
typedef struct yasm_section yasm_section;

/** Symbol table (opaque type).  \see symrec.h for related functions. */
typedef struct yasm_symtab yasm_symtab;

/** Symbol record (opaque type).  \see symrec.h for related functions. */
typedef struct yasm_symrec yasm_symrec;

/** Expression.  \see expr.h for details and related functions. */
typedef struct yasm_expr yasm_expr;
/** Integer value (opaque type).  \see intnum.h for related functions. */
typedef struct yasm_intnum yasm_intnum;
/** Floating point value (opaque type).
 * \see floatnum.h for related functions.
 */
typedef struct yasm_floatnum yasm_floatnum;

/** A value.  May be absolute or relative.  Outside the parser, yasm_expr
 * should only be used for absolute exprs.  Anything that could contain
 * a relocatable value should use this structure instead.
 * \see value.h for related functions.
 */
typedef struct yasm_value {
    /** The absolute portion of the value.  May contain *differences* between
     * symrecs but not standalone symrecs.  May be NULL if there is no
     * absolute portion (e.g. the absolute portion is 0).
     */
    /*@null@*/ /*@only@*/ yasm_expr *abs;

    /** The relative portion of the value.  This is the portion that may
     * need to generate a relocation.  May be NULL if no relative portion.
     */
    /*@null@*/ /*@dependent@*/ yasm_symrec *rel;

    /** What the relative portion is in reference to.  NULL if the default. */
    /*@null@*/ /*@dependent@*/ yasm_symrec *wrt;

    /** If the segment of the relative portion should be used, not the
     * relative portion itself.  Boolean.
     */
    unsigned int seg_of : 1;

    /** If the relative portion of the value should be shifted right
     * (supported only by a few object formats).  If just the absolute portion
     * should be shifted, that must be in the abs expr, not here!
     */
    unsigned int rshift : 7;

    /** Indicates the relative portion of the value should be relocated
     * relative to the current assembly position rather than relative to the
     * section start.  "Current assembly position" here refers to the starting
     * address of the bytecode containing this value.  Boolean.
     */
    unsigned int curpos_rel : 1;

    /** Indicates that curpos_rel was set due to IP-relative relocation;
     * in some objfmt/arch combinations (e.g. win64/x86-amd64) this info
     * is needed to generate special relocations.
     */
    unsigned int ip_rel : 1;

    /** Indicates the value is a jump target address (rather than a simple
     * data address).  In some objfmt/arch combinations (e.g. macho/amd64)
     * this info is needed to generate special relocations.
     */
    unsigned int jump_target : 1;

    /** Indicates the relative portion of the value should be relocated
     * relative to its own section start rather than relative to the
     * section start of the bytecode containing this value.  E.g. the value
     * resulting from the relative portion should be the offset from its
     * section start.  Boolean.
     */
    unsigned int section_rel : 1;

    /** Indicates overflow warnings have been disabled for this value. */
    unsigned int no_warn : 1;

    /** Sign of the value.  Nonzero if the final value should be treated as
     * signed, 0 if it should be treated as signed.
     */
    unsigned int sign : 1;

    /** Size of the value, in bits. */
    unsigned int size : 8;
} yasm_value;

/** Maximum value of #yasm_value.rshift */
#define YASM_VALUE_RSHIFT_MAX   127

/** Line number mapping repository (opaque type).  \see linemap.h for related
 * functions.
 */
typedef struct yasm_linemap yasm_linemap;

/** Value/parameter pair (opaque type).
 * \see valparam.h for related functions.
 */
typedef struct yasm_valparam yasm_valparam;
/** List of value/parameters (opaque type).
 * \see valparam.h for related functions.
 */
typedef struct yasm_valparamhead yasm_valparamhead;
/** Directive list entry.
 * \see valparam.h for details and related functions.
 */
typedef struct yasm_directive yasm_directive;

/** An effective address.
 * \see insn.h for related functions.
 */
typedef struct yasm_effaddr yasm_effaddr;

/** An instruction.
 * \see insn.h for related functions.
 */
typedef struct yasm_insn yasm_insn;

/** Expression operators usable in #yasm_expr expressions. */
typedef enum yasm_expr_op {
    YASM_EXPR_IDENT,    /**< No operation, just a value. */
    YASM_EXPR_ADD,      /**< Arithmetic addition (+). */
    YASM_EXPR_SUB,      /**< Arithmetic subtraction (-). */
    YASM_EXPR_MUL,      /**< Arithmetic multiplication (*). */
    YASM_EXPR_DIV,      /**< Arithmetic unsigned division. */
    YASM_EXPR_SIGNDIV,  /**< Arithmetic signed division. */
    YASM_EXPR_MOD,      /**< Arithmetic unsigned modulus. */
    YASM_EXPR_SIGNMOD,  /**< Arithmetic signed modulus. */
    YASM_EXPR_NEG,      /**< Arithmetic negation (-). */
    YASM_EXPR_NOT,      /**< Bitwise negation. */
    YASM_EXPR_OR,       /**< Bitwise OR. */
    YASM_EXPR_AND,      /**< Bitwise AND. */
    YASM_EXPR_XOR,      /**< Bitwise XOR. */
    YASM_EXPR_XNOR,     /**< Bitwise XNOR. */
    YASM_EXPR_NOR,      /**< Bitwise NOR. */
    YASM_EXPR_SHL,      /**< Shift left (logical). */
    YASM_EXPR_SHR,      /**< Shift right (logical). */
    YASM_EXPR_LOR,      /**< Logical OR. */
    YASM_EXPR_LAND,     /**< Logical AND. */
    YASM_EXPR_LNOT,     /**< Logical negation. */
    YASM_EXPR_LXOR,     /**< Logical XOR. */
    YASM_EXPR_LXNOR,    /**< Logical XNOR. */
    YASM_EXPR_LNOR,     /**< Logical NOR. */
    YASM_EXPR_LT,       /**< Less than comparison. */
    YASM_EXPR_GT,       /**< Greater than comparison. */
    YASM_EXPR_EQ,       /**< Equality comparison. */
    YASM_EXPR_LE,       /**< Less than or equal to comparison. */
    YASM_EXPR_GE,       /**< Greater than or equal to comparison. */
    YASM_EXPR_NE,       /**< Not equal comparison. */
    YASM_EXPR_NONNUM,   /**< Start of non-numeric operations (not an op). */
    YASM_EXPR_SEG,      /**< SEG operator (gets segment portion of address). */
    YASM_EXPR_WRT,      /**< WRT operator (gets offset of address relative to
                         *   some other segment). */
    YASM_EXPR_SEGOFF    /**< The ':' in segment:offset. */
} yasm_expr_op;

/** Convert yasm_value to its byte representation.  Usually implemented by
 * object formats to keep track of relocations and verify legal expressions.
 * Must put the value into the least significant bits of the destination,
 * unless shifted into more significant bits by the shift parameter.  The
 * destination bits must be cleared before being set.
 * \param value         value
 * \param buf           buffer for byte representation
 * \param destsize      destination size (in bytes)
 * \param offset        offset (in bytes) of the expr contents from the start
 *                      of the bytecode (needed for relative)
 * \param bc            current bytecode (usually passed into higher-level
 *                      calling function)
 * \param warn          enables standard warnings: zero for none;
 *                      nonzero for overflow/underflow floating point warnings
 * \param d             objfmt-specific data (passed into higher-level calling
 *                      function)
 * \return Nonzero if an error occurred, 0 otherwise.
 */
typedef int (*yasm_output_value_func)
    (yasm_value *value, /*@out@*/ unsigned char *buf, unsigned int destsize,
     unsigned long offset, yasm_bytecode *bc, int warn, /*@null@*/ void *d);

/** Convert a symbol reference to its byte representation.  Usually implemented
 * by object formats and debug formats to keep track of relocations generated
 * by themselves.
 * \param sym           symbol
 * \param bc            current bytecode (usually passed into higher-level
 *                      calling function)
 * \param buf           buffer for byte representation
 * \param destsize      destination size (in bytes)
 * \param valsize       size (in bits)
 * \param warn          enables standard warnings: zero for none;
 *                      nonzero for overflow/underflow floating point warnings;
 *                      negative for signed integer warnings,
 *                      positive for unsigned integer warnings
 * \param d             objfmt-specific data (passed into higher-level calling
 *                      function)
 * \return Nonzero if an error occurred, 0 otherwise.
 */
typedef int (*yasm_output_reloc_func)
    (yasm_symrec *sym, yasm_bytecode *bc, unsigned char *buf,
     unsigned int destsize, unsigned int valsize, int warn, void *d);

/** Sort an array using merge sort algorithm.
 * \internal
 * \param base      base of array
 * \param nmemb     number of elements in array
 * \param size      size of each array element
 * \param compar    element comparison function
 */
YASM_LIB_DECL
int yasm__mergesort(void *base, size_t nmemb, size_t size,
                    int (*compar)(const void *, const void *));

/** Separate string by delimiters.
 * \internal
 * \param stringp   string
 * \param delim     set of 1 or more delimiters
 * \return First/next substring.
 */
YASM_LIB_DECL
/*@null@*/ char *yasm__strsep(char **stringp, const char *delim);

/** Compare two strings, ignoring case differences.
 * \internal
 * \param s1    string 1
 * \param s2    string 2
 * \return 0 if strings are equal, -1 if s1<s2, 1 if s1>s2.
 */
YASM_LIB_DECL
int yasm__strcasecmp(const char *s1, const char *s2);

/** Compare portion of two strings, ignoring case differences.
 * \internal
 * \param s1    string 1
 * \param s2    string 2
 * \param n     maximum number of characters to compare
 * \return 0 if strings are equal, -1 if s1<s2, 1 if s1>s2.
 */
YASM_LIB_DECL
int yasm__strncasecmp(const char *s1, const char *s2, size_t n);

/** strdup() implementation using yasm_xmalloc().
 * \internal
 * \param str   string
 * \return Newly allocated duplicate string.
 */
YASM_LIB_DECL
/*@only@*/ char *yasm__xstrdup(const char *str);

/** strndup() implementation using yasm_xmalloc().
 * \internal
 * \param str   string
 * \param max   maximum number of characters to copy
 * \return Newly allocated duplicate string.
 */
YASM_LIB_DECL
/*@only@*/ char *yasm__xstrndup(const char *str, size_t max);

/** Error-checking memory allocation.  A default implementation is provided
 * that calls yasm_fatal() on allocation errors.
 * A replacement should \em never return NULL.
 * \param size      number of bytes to allocate
 * \return Allocated memory block.
 */
YASM_LIB_DECL
extern /*@only@*/ /*@out@*/ void * (*yasm_xmalloc) (size_t size);

/** Error-checking memory allocation (with clear-to-0).  A default
 * implementation is provided that calls yasm_fatal() on allocation errors.
 * A replacement should \em never return NULL.
 * \param size      number of elements to allocate
 * \param elsize    size (in bytes) of each element
 * \return Allocated and cleared memory block.
 */
YASM_LIB_DECL
extern /*@only@*/ void * (*yasm_xcalloc) (size_t nelem, size_t elsize);

/** Error-checking memory reallocation.  A default implementation is provided
 * that calls yasm_fatal() on allocation errors.  A replacement should
 * \em never return NULL.
 * \param oldmem    memory block to resize
 * \param elsize    new size, in bytes
 * \return Re-allocated memory block.
 */
YASM_LIB_DECL
extern /*@only@*/ void * (*yasm_xrealloc)
    (/*@only@*/ /*@out@*/ /*@returned@*/ /*@null@*/ void *oldmem, size_t size)
    /*@modifies oldmem@*/;

/** Error-checking memory deallocation.  A default implementation is provided
 * that calls yasm_fatal() on allocation errors.
 * \param p     memory block to free
 */
YASM_LIB_DECL
extern void (*yasm_xfree) (/*@only@*/ /*@out@*/ /*@null@*/ void *p)
    /*@modifies p@*/;

#endif
