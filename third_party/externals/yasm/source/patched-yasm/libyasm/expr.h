/**
 * \file libyasm/expr.h
 * \brief YASM expression interface.
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
#ifndef YASM_EXPR_H
#define YASM_EXPR_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

/** Type of an expression item.  Types are listed in canonical sorting order.
 * See expr_order_terms().
 * Note #YASM_EXPR_PRECBC must be used carefully (in a-b pairs), as only
 * symrecs can become the relative term in a #yasm_value.
 */
typedef enum yasm_expr__type {
    YASM_EXPR_NONE = 0,     /**< Nothing */
    YASM_EXPR_REG = 1<<0,   /**< Register */
    YASM_EXPR_INT = 1<<1,   /**< Integer value */
    YASM_EXPR_SUBST = 1<<2, /**< Substitution placeholder */
    YASM_EXPR_FLOAT = 1<<3, /**< Floating point value */
    YASM_EXPR_SYM = 1<<4,   /**< Symbol */
    YASM_EXPR_PRECBC = 1<<5,/**< Direct bytecode ref (rather than via sym) */
    YASM_EXPR_EXPR = 1<<6   /**< Subexpression */
} yasm_expr__type;

/** Expression item. */
typedef struct yasm_expr__item {
    yasm_expr__type type;   /**< Type */

    /** Expression item data.  Correct value depends on type. */
    union {
        yasm_bytecode *precbc;  /**< Direct bytecode ref (YASM_EXPR_PRECBC) */
        yasm_symrec *sym;       /**< Symbol (YASM_EXPR_SYM) */
        yasm_expr *expn;        /**< Subexpression (YASM_EXPR_EXPR) */
        yasm_intnum *intn;      /**< Integer value (YASM_EXPR_INT) */
        yasm_floatnum *flt;     /**< Floating point value (YASM_EXPR_FLOAT) */
        uintptr_t reg;          /**< Register (YASM_EXPR_REG) */
        unsigned int subst;     /**< Subst placeholder (YASM_EXPR_SUBST) */
    } data;
} yasm_expr__item;

/** Expression. */
struct yasm_expr {
    yasm_expr_op op;    /**< Operation. */
    unsigned long line; /**< Line number where expression was defined. */
    int numterms;       /**< Number of terms in the expression. */

    /** Terms of the expression.  Structure may be extended to include more
     * terms, as some operations may allow more than two operand terms
     * (ADD, MUL, OR, AND, XOR).
     */
    yasm_expr__item terms[2];
};

/** Create a new expression e=a op b.
 * \param op        operation
 * \param a         expression item a
 * \param b         expression item b (optional depending on op)
 * \param line      virtual line (where expression defined)
 * \return Newly allocated expression.
 */
YASM_LIB_DECL
/*@only@*/ yasm_expr *yasm_expr_create
    (yasm_expr_op op, /*@only@*/ yasm_expr__item *a,
     /*@only@*/ /*@null@*/ yasm_expr__item *b, unsigned long line);

/** Create a new preceding-bytecode expression item.
 * \param precbc    preceding bytecode
 * \return Newly allocated expression item.
 */
YASM_LIB_DECL
/*@only@*/ yasm_expr__item *yasm_expr_precbc(/*@keep@*/ yasm_bytecode *precbc);

/** Create a new symbol expression item.
 * \param sym       symbol
 * \return Newly allocated expression item.
 */
YASM_LIB_DECL
/*@only@*/ yasm_expr__item *yasm_expr_sym(/*@keep@*/ yasm_symrec *sym);

/** Create a new expression expression item.
 * \param e         expression
 * \return Newly allocated expression item.
 */
YASM_LIB_DECL
/*@only@*/ yasm_expr__item *yasm_expr_expr(/*@keep@*/ yasm_expr *e);

/** Create a new intnum expression item.
 * \param intn      intnum
 * \return Newly allocated expression item.
 */
YASM_LIB_DECL
/*@only@*/ yasm_expr__item *yasm_expr_int(/*@keep@*/ yasm_intnum *intn);

/** Create a new floatnum expression item.
 * \param flt       floatnum
 * \return Newly allocated expression item.
 */
YASM_LIB_DECL
/*@only@*/ yasm_expr__item *yasm_expr_float(/*@keep@*/ yasm_floatnum *flt);

/** Create a new register expression item.
 * \param reg       register
 * \return Newly allocated expression item.
 */
YASM_LIB_DECL
/*@only@*/ yasm_expr__item *yasm_expr_reg(uintptr_t reg);

/** Create a new expression tree e=l op r.
 * \param l     expression for left side of new expression
 * \param o     operation
 * \param r     expression for right side of new expression
 * \param i     line index
 * \return Newly allocated expression.
 */
#define yasm_expr_create_tree(l,o,r,i) \
    yasm_expr_create ((o), yasm_expr_expr(l), yasm_expr_expr(r), i)

/** Create a new expression branch e=op r.
 * \param o     operation
 * \param r     expression for right side of new expression
 * \param i     line index
 * \return Newly allocated expression.
 */
#define yasm_expr_create_branch(o,r,i) \
    yasm_expr_create ((o), yasm_expr_expr(r), (yasm_expr__item *)NULL, i)

/** Create a new expression identity e=r.
 * \param r     expression for identity within new expression
 * \param i     line index
 * \return Newly allocated expression.
 */
#define yasm_expr_create_ident(r,i) \
    yasm_expr_create (YASM_EXPR_IDENT, (r), (yasm_expr__item *)NULL, i)

/** Duplicate an expression.
 * \param e     expression
 * \return Newly allocated expression identical to e.
 */
yasm_expr *yasm_expr_copy(const yasm_expr *e);
#ifndef YASM_DOXYGEN
#define yasm_expr_copy(e)   yasm_expr__copy_except(e, -1)
#endif

/** Destroy (free allocated memory for) an expression.
 * \param e     expression
 */
YASM_LIB_DECL
void yasm_expr_destroy(/*@only@*/ /*@null@*/ yasm_expr *e);

/** Determine if an expression is a specified operation (at the top level).
 * \param e             expression
 * \param op            operator
 * \return Nonzero if the expression was the specified operation at the top
 *         level, zero otherwise.
 */
YASM_LIB_DECL
int yasm_expr_is_op(const yasm_expr *e, yasm_expr_op op);

/** Extra transformation function for yasm_expr__level_tree().
 * \param e     expression being simplified
 * \param d     data provided as expr_xform_extra_data to
 *              yasm_expr__level_tree()
 * \return Transformed e.
 */
typedef /*@only@*/ yasm_expr * (*yasm_expr_xform_func)
    (/*@returned@*/ /*@only@*/ yasm_expr *e, /*@null@*/ void *d);

/** Level an entire expression tree.
 * \internal
 * \param e                 expression
 * \param fold_const        enable constant folding if nonzero
 * \param simplify_ident    simplify identities
 * \param simplify_reg_mul  simplify REG*1 identities
 * \param calc_bc_dist      nonzero if distances between bytecodes should be
 *                          calculated, 0 if they should be left intact
 * \param expr_xform_extra  extra transformation function
 * \param expr_xform_extra_data data to pass to expr_xform_extra
 * \return Leveled expression.
 */
YASM_LIB_DECL
/*@only@*/ /*@null@*/ yasm_expr *yasm_expr__level_tree
    (/*@returned@*/ /*@only@*/ /*@null@*/ yasm_expr *e, int fold_const,
     int simplify_ident, int simplify_reg_mul, int calc_bc_dist,
     /*@null@*/ yasm_expr_xform_func expr_xform_extra,
     /*@null@*/ void *expr_xform_extra_data);

/** Simplify an expression as much as possible.  Eliminates extraneous
 * branches and simplifies integer-only subexpressions.  Simplified version
 * of yasm_expr__level_tree().
 * \param e     expression
 * \param cbd   if distance between bytecodes should be calculated
 * \return Simplified expression.
 */
#define yasm_expr_simplify(e, cbd) \
    yasm_expr__level_tree(e, 1, 1, 1, cbd, NULL, NULL)

/** Extract the segment portion of an expression containing SEG:OFF, leaving
 * the offset.
 * \param ep            expression (pointer to)
 * \return NULL if unable to extract a segment (expr does not contain a
 *         YASM_EXPR_SEGOFF operator), otherwise the segment expression.
 *         The input expression is modified such that on return, it's the
 *         offset expression.
 */
YASM_LIB_DECL
/*@only@*/ /*@null@*/ yasm_expr *yasm_expr_extract_deep_segoff(yasm_expr **ep);

/** Extract the segment portion of a SEG:OFF expression, leaving the offset.
 * \param ep            expression (pointer to)
 * \return NULL if unable to extract a segment (YASM_EXPR_SEGOFF not the
 *         top-level operator), otherwise the segment expression.  The input
 *         expression is modified such that on return, it's the offset
 *         expression.
 */
YASM_LIB_DECL
/*@only@*/ /*@null@*/ yasm_expr *yasm_expr_extract_segoff(yasm_expr **ep);

/** Extract the right portion (y) of a x WRT y expression, leaving the left
 * portion (x).
 * \param ep            expression (pointer to)
 * \return NULL if unable to extract (YASM_EXPR_WRT not the top-level
 *         operator), otherwise the right side of the WRT expression.  The
 *         input expression is modified such that on return, it's the left side
 *         of the WRT expression.
 */
YASM_LIB_DECL
/*@only@*/ /*@null@*/ yasm_expr *yasm_expr_extract_wrt(yasm_expr **ep);

/** Get the integer value of an expression if it's just an integer.
 * \param ep            expression (pointer to)
 * \param calc_bc_dist  nonzero if distances between bytecodes should be
 *                      calculated, 0 if NULL should be returned in this case
 * \return NULL if the expression is too complex (contains anything other than
 *         integers, ie floats, non-valued labels, registers); otherwise the
 *         intnum value of the expression.
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ yasm_intnum *yasm_expr_get_intnum
    (yasm_expr **ep, int calc_bc_dist);

/** Get the symbol value of an expression if it's just a symbol.
 * \param ep            expression (pointer to)
 * \param simplify      if nonzero, simplify the expression first
 * \return NULL if the expression is too complex; otherwise the symbol value of
 *         the expression.
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ const yasm_symrec *yasm_expr_get_symrec
    (yasm_expr **ep, int simplify);

/** Get the register value of an expression if it's just a register.
 * \param ep            expression (pointer to)
 * \param simplify      if nonzero, simplify the expression first
 * \return NULL if the expression is too complex; otherwise the register value
 *         of the expression.
 */
YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ const uintptr_t *yasm_expr_get_reg
    (yasm_expr **ep, int simplify);

/** Print an expression.  For debugging purposes.
 * \param e     expression
 * \param f     file
 */
YASM_LIB_DECL
void yasm_expr_print(/*@null@*/ const yasm_expr *e, FILE *f);

/** Return the size of an expression, if the user provided it
 * \param e     expression
 */
unsigned int yasm_expr_size(const yasm_expr *e);

/** Return the segment of an expression, if the user provided it
 * \param e     expression
 */
const char *yasm_expr_segment(const yasm_expr *e);

/** Traverse over expression tree in order (const version).
 * Calls func for each leaf (non-operation).
 * \param e     expression
 * \param d     data passed to each call to func
 * \param func  callback function
 * \return Stops early (and returns 1) if func returns 1.
 *         Otherwise returns 0.
 */
YASM_LIB_DECL
int yasm_expr__traverse_leaves_in_const
    (const yasm_expr *e, /*@null@*/ void *d,
     int (*func) (/*@null@*/ const yasm_expr__item *ei, /*@null@*/ void *d));

/** Traverse over expression tree in order.
 * Calls func for each leaf (non-operation).
 * \param e     expression
 * \param d     data passed to each call to func
 * \param func  callback function
 * \return Stops early (and returns 1) if func returns 1.
 *         Otherwise returns 0.
 */
YASM_LIB_DECL
int yasm_expr__traverse_leaves_in
    (yasm_expr *e, /*@null@*/ void *d,
     int (*func) (/*@null@*/ yasm_expr__item *ei, /*@null@*/ void *d));

/** Reorder terms of e into canonical order.  Only reorders if reordering
 * doesn't change meaning of expression.  (eg, doesn't reorder SUB).
 * Canonical order: REG, INT, FLOAT, SYM, EXPR.
 * Multiple terms of a single type are kept in the same order as in
 * the original expression.
 * \param e     expression
 * \note Only performs reordering on *one* level (no recursion).
 */
YASM_LIB_DECL
void yasm_expr__order_terms(yasm_expr *e);

/** Copy entire expression EXCEPT for index "except" at *top level only*.
 * \param e         expression
 * \param except    term index not to copy; -1 to copy all terms
 * \return Newly allocated copy of expression.
 */
YASM_LIB_DECL
yasm_expr *yasm_expr__copy_except(const yasm_expr *e, int except);

/** Test if expression contains an item.  Searches recursively into
 * subexpressions.
 * \param e     expression
 * \param t     type of item to look for
 * \return Nonzero if expression contains an item of type t, zero if not.
 */
YASM_LIB_DECL
int yasm_expr__contains(const yasm_expr *e, yasm_expr__type t);

/** Transform symrec-symrec terms in expression into #YASM_EXPR_SUBST items.
 * Calls the callback function for each symrec-symrec term.
 * \param ep            expression (pointer to)
 * \param cbd           callback data passed to callback function
 * \param callback      callback function: given subst index for bytecode
 *                      pair, bytecode pair (bc2-bc1), and cbd (callback data)
 * \return Number of transformations made.
 */
YASM_LIB_DECL
int yasm_expr__bc_dist_subst(yasm_expr **ep, void *cbd,
                             void (*callback) (unsigned int subst,
                                               yasm_bytecode *precbc,
                                               yasm_bytecode *precbc2,
                                               void *cbd));

/** Substitute items into expr YASM_EXPR_SUBST items (by index).  Items are
 * copied, so caller is responsible for freeing array of items.
 * \param e             expression
 * \param num_items     number of items in items array
 * \param items         items array
 * \return 1 on error (index out of range).
 */
YASM_LIB_DECL
int yasm_expr__subst(yasm_expr *e, unsigned int num_items,
                     const yasm_expr__item *items);

#endif
