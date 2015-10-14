/*
 * Expression handling
 *
 *  Copyright (C) 2001-2007  Michael Urman, Peter Johnson
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

#include "libyasm-stdint.h"
#include "coretype.h"
#include "bitvect.h"

#include "errwarn.h"
#include "intnum.h"
#include "floatnum.h"
#include "expr.h"
#include "symrec.h"

#include "bytecode.h"
#include "section.h"

#include "arch.h"


static /*@only@*/ yasm_expr *expr_level_op
    (/*@returned@*/ /*@only@*/ yasm_expr *e, int fold_const,
     int simplify_ident, int simplify_reg_mul);
static int expr_traverse_nodes_post(/*@null@*/ yasm_expr *e,
                                    /*@null@*/ void *d,
                                    int (*func) (/*@null@*/ yasm_expr *e,
                                                 /*@null@*/ void *d));
static void expr_delete_term(yasm_expr__item *term, int recurse);

/* Bitmap of used items.  We should really never need more than 2 at a time,
 * so 31 is pretty much overkill.
 */
static unsigned long itempool_used = 0;
static yasm_expr__item itempool[31];

/* allocate a new expression node, with children as defined.
 * If it's a unary operator, put the element in left and set right=NULL. */
/*@-compmempass@*/
yasm_expr *
yasm_expr_create(yasm_expr_op op, yasm_expr__item *left,
                 yasm_expr__item *right, unsigned long line)
{
    yasm_expr *ptr, *sube;
    unsigned long z;
    ptr = yasm_xmalloc(sizeof(yasm_expr));

    ptr->op = op;
    ptr->numterms = 0;
    ptr->terms[0].type = YASM_EXPR_NONE;
    ptr->terms[1].type = YASM_EXPR_NONE;
    if (left) {
        ptr->terms[0] = *left;  /* structure copy */
        z = (unsigned long)(left-itempool);
        if (z>=31)
            yasm_internal_error(N_("could not find expritem in pool"));
        itempool_used &= ~(1<<z);
        ptr->numterms++;

        /* Search downward until we find something *other* than an
         * IDENT, then bring it up to the current level.
         */
        while (ptr->terms[0].type == YASM_EXPR_EXPR &&
               ptr->terms[0].data.expn->op == YASM_EXPR_IDENT) {
            sube = ptr->terms[0].data.expn;
            ptr->terms[0] = sube->terms[0];     /* structure copy */
            /*@-usereleased@*/
            yasm_xfree(sube);
            /*@=usereleased@*/
        }
    } else {
        yasm_internal_error(N_("Right side of expression must exist"));
    }

    if (right) {
        ptr->terms[1] = *right; /* structure copy */
        z = (unsigned long)(right-itempool);
        if (z>=31)
            yasm_internal_error(N_("could not find expritem in pool"));
        itempool_used &= ~(1<<z);
        ptr->numterms++;

        /* Search downward until we find something *other* than an
         * IDENT, then bring it up to the current level.
         */
        while (ptr->terms[1].type == YASM_EXPR_EXPR &&
               ptr->terms[1].data.expn->op == YASM_EXPR_IDENT) {
            sube = ptr->terms[1].data.expn;
            ptr->terms[1] = sube->terms[0];     /* structure copy */
            /*@-usereleased@*/
            yasm_xfree(sube);
            /*@=usereleased@*/
        }
    }

    ptr->line = line;

    return expr_level_op(ptr, 1, 1, 0);
}
/*@=compmempass@*/

/* helpers */
static yasm_expr__item *
expr_get_item(void)
{
    int z = 0;
    unsigned long v = itempool_used & 0x7fffffff;

    while (v & 1) {
        v >>= 1;
        z++;
    }
    if (z>=31)
        yasm_internal_error(N_("too many expritems"));
    itempool_used |= 1<<z;
    return &itempool[z];
}

yasm_expr__item *
yasm_expr_precbc(yasm_bytecode *precbc)
{
    yasm_expr__item *e = expr_get_item();
    e->type = YASM_EXPR_PRECBC;
    e->data.precbc = precbc;
    return e;
}

yasm_expr__item *
yasm_expr_sym(yasm_symrec *s)
{
    yasm_expr__item *e = expr_get_item();
    e->type = YASM_EXPR_SYM;
    e->data.sym = s;
    return e;
}

yasm_expr__item *
yasm_expr_expr(yasm_expr *x)
{
    yasm_expr__item *e = expr_get_item();
    e->type = YASM_EXPR_EXPR;
    e->data.expn = x;
    return e;
}

yasm_expr__item *
yasm_expr_int(yasm_intnum *i)
{
    yasm_expr__item *e = expr_get_item();
    e->type = YASM_EXPR_INT;
    e->data.intn = i;
    return e;
}

yasm_expr__item *
yasm_expr_float(yasm_floatnum *f)
{
    yasm_expr__item *e = expr_get_item();
    e->type = YASM_EXPR_FLOAT;
    e->data.flt = f;
    return e;
}

yasm_expr__item *
yasm_expr_reg(uintptr_t reg)
{
    yasm_expr__item *e = expr_get_item();
    e->type = YASM_EXPR_REG;
    e->data.reg = reg;
    return e;
}

/* Transforms instances of symrec-symrec [symrec+(-1*symrec)] into single
 * expritems if possible.  Uses a simple n^2 algorithm because n is usually
 * quite small.  Also works for precbc-precbc (or symrec-precbc,
 * precbc-symrec).
 */
static /*@only@*/ yasm_expr *
expr_xform_bc_dist_base(/*@returned@*/ /*@only@*/ yasm_expr *e,
                        /*@null@*/ void *cbd,
                        int (*callback) (yasm_expr__item *ei,
                                         yasm_bytecode *precbc,
                                         yasm_bytecode *precbc2,
                                         void *cbd))
{
    int i;
    /*@dependent@*/ yasm_section *sect;
    /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc;
    int numterms;

    /* Handle symrec-symrec in ADD exprs by looking for (-1*symrec) and
     * symrec term pairs (where both symrecs are in the same segment).
     */
    if (e->op != YASM_EXPR_ADD)
        return e;

    for (i=0; i<e->numterms; i++) {
        int j;
        yasm_expr *sube;
        yasm_intnum *intn;
        yasm_symrec *sym = NULL;
        /*@dependent@*/ yasm_section *sect2;
        /*@dependent@*/ /*@null@*/ yasm_bytecode *precbc2;

        /* First look for an (-1*symrec) term */
        if (e->terms[i].type != YASM_EXPR_EXPR)
            continue;
        sube = e->terms[i].data.expn;
        if (sube->op != YASM_EXPR_MUL || sube->numterms != 2)
            continue;

        if (sube->terms[0].type == YASM_EXPR_INT &&
            (sube->terms[1].type == YASM_EXPR_SYM ||
             sube->terms[1].type == YASM_EXPR_PRECBC)) {
            intn = sube->terms[0].data.intn;
            if (sube->terms[1].type == YASM_EXPR_PRECBC)
                precbc = sube->terms[1].data.precbc;
            else
                sym = sube->terms[1].data.sym;
        } else if ((sube->terms[0].type == YASM_EXPR_SYM ||
                    sube->terms[0].type == YASM_EXPR_PRECBC) &&
                   sube->terms[1].type == YASM_EXPR_INT) {
            if (sube->terms[0].type == YASM_EXPR_PRECBC)
                precbc = sube->terms[0].data.precbc;
            else
                sym = sube->terms[0].data.sym;
            intn = sube->terms[1].data.intn;
        } else
            continue;

        if (!yasm_intnum_is_neg1(intn))
            continue;

        if (sym && !yasm_symrec_get_label(sym, &precbc))
            continue;
        sect2 = yasm_bc_get_section(precbc);

        /* Now look for a symrec term in the same segment */
        for (j=0; j<e->numterms; j++) {
            if (((e->terms[j].type == YASM_EXPR_SYM &&
                  yasm_symrec_get_label(e->terms[j].data.sym, &precbc2)) ||
                 (e->terms[j].type == YASM_EXPR_PRECBC &&
                  (precbc2 = e->terms[j].data.precbc))) &&
                (sect = yasm_bc_get_section(precbc2)) &&
                sect == sect2 &&
                callback(&e->terms[j], precbc, precbc2, cbd)) {
                /* Delete the matching (-1*symrec) term */
                yasm_expr_destroy(sube);
                e->terms[i].type = YASM_EXPR_NONE;
                break;  /* stop looking for matching symrec term */
            }
        }
    }

    /* Clean up any deleted (EXPR_NONE) terms */
    numterms = 0;
    for (i=0; i<e->numterms; i++) {
        if (e->terms[i].type != YASM_EXPR_NONE)
            e->terms[numterms++] = e->terms[i]; /* structure copy */
    }
    if (e->numterms != numterms) {
        e->numterms = numterms;
        e = yasm_xrealloc(e, sizeof(yasm_expr)+((numterms<2) ? 0 :
                          sizeof(yasm_expr__item)*(numterms-2)));
        if (numterms == 1)
            e->op = YASM_EXPR_IDENT;
    }

    return e;
}

static int
expr_xform_bc_dist_cb(yasm_expr__item *ei, yasm_bytecode *precbc,
                      yasm_bytecode *precbc2, /*@null@*/ void *d)
{
    yasm_intnum *dist = yasm_calc_bc_dist(precbc, precbc2);
    if (!dist)
        return 0;
    /* Change the term to an integer */
    ei->type = YASM_EXPR_INT;
    ei->data.intn = dist;
    return 1;
}

/* Transforms instances of symrec-symrec [symrec+(-1*symrec)] into integers if
 * possible.
 */
static /*@only@*/ yasm_expr *
expr_xform_bc_dist(/*@returned@*/ /*@only@*/ yasm_expr *e)
{
    return expr_xform_bc_dist_base(e, NULL, expr_xform_bc_dist_cb);
}

typedef struct bc_dist_subst_cbd {
    void (*callback) (unsigned int subst, yasm_bytecode *precbc,
                      yasm_bytecode *precbc2, void *cbd);
    void *cbd;
    unsigned int subst;
} bc_dist_subst_cbd;

static int
expr_bc_dist_subst_cb(yasm_expr__item *ei, yasm_bytecode *precbc,
                      yasm_bytecode *precbc2, /*@null@*/ void *d)
{
    bc_dist_subst_cbd *my_cbd = d;
    assert(my_cbd != NULL);
    /* Call higher-level callback */
    my_cbd->callback(my_cbd->subst, precbc, precbc2, my_cbd->cbd);
    /* Change the term to an subst */
    ei->type = YASM_EXPR_SUBST;
    ei->data.subst = my_cbd->subst;
    my_cbd->subst++;
    return 1;
}

static yasm_expr *
expr_xform_bc_dist_subst(yasm_expr *e, void *d)
{
    return expr_xform_bc_dist_base(e, d, expr_bc_dist_subst_cb);
}

int
yasm_expr__bc_dist_subst(yasm_expr **ep, void *cbd,
                         void (*callback) (unsigned int subst,
                                           yasm_bytecode *precbc,
                                           yasm_bytecode *precbc2,
                                           void *cbd))
{
    bc_dist_subst_cbd my_cbd;   /* callback info for low-level callback */
    my_cbd.callback = callback;
    my_cbd.cbd = cbd;
    my_cbd.subst = 0;
    *ep = yasm_expr__level_tree(*ep, 1, 1, 1, 0, &expr_xform_bc_dist_subst,
                                &my_cbd);
    return my_cbd.subst;
}

/* Negate just a single ExprItem by building a -1*ei subexpression */
static void
expr_xform_neg_item(yasm_expr *e, yasm_expr__item *ei)
{
    yasm_expr *sube = yasm_xmalloc(sizeof(yasm_expr));

    /* Build -1*ei subexpression */
    sube->op = YASM_EXPR_MUL;
    sube->line = e->line;
    sube->numterms = 2;
    sube->terms[0].type = YASM_EXPR_INT;
    sube->terms[0].data.intn = yasm_intnum_create_int(-1);
    sube->terms[1] = *ei;       /* structure copy */

    /* Replace original ExprItem with subexp */
    ei->type = YASM_EXPR_EXPR;
    ei->data.expn = sube;
}

/* Negates e by multiplying by -1, with distribution over lower-precedence
 * operators (eg ADD) and special handling to simplify result w/ADD, NEG, and
 * others.
 *
 * Returns a possibly reallocated e.
 */
static /*@only@*/ yasm_expr *
expr_xform_neg_helper(/*@returned@*/ /*@only@*/ yasm_expr *e)
{
    yasm_expr *ne;
    int i;

    switch (e->op) {
        case YASM_EXPR_ADD:
            /* distribute (recursively if expr) over terms */
            for (i=0; i<e->numterms; i++) {
                if (e->terms[i].type == YASM_EXPR_EXPR)
                    e->terms[i].data.expn =
                        expr_xform_neg_helper(e->terms[i].data.expn);
                else
                    expr_xform_neg_item(e, &e->terms[i]);
            }
            break;
        case YASM_EXPR_SUB:
            /* change op to ADD, and recursively negate left side (if expr) */
            e->op = YASM_EXPR_ADD;
            if (e->terms[0].type == YASM_EXPR_EXPR)
                e->terms[0].data.expn =
                    expr_xform_neg_helper(e->terms[0].data.expn);
            else
                expr_xform_neg_item(e, &e->terms[0]);
            break;
        case YASM_EXPR_NEG:
            /* Negating a negated value?  Make it an IDENT. */
            e->op = YASM_EXPR_IDENT;
            break;
        case YASM_EXPR_IDENT:
            /* Negating an ident?  Change it into a MUL w/ -1 if there's no
             * floatnums present below; if there ARE floatnums, recurse.
             */
            if (e->terms[0].type == YASM_EXPR_FLOAT)
                yasm_floatnum_calc(e->terms[0].data.flt, YASM_EXPR_NEG, NULL);
            else if (e->terms[0].type == YASM_EXPR_INT)
                yasm_intnum_calc(e->terms[0].data.intn, YASM_EXPR_NEG, NULL);
            else if (e->terms[0].type == YASM_EXPR_EXPR &&
                yasm_expr__contains(e->terms[0].data.expn, YASM_EXPR_FLOAT))
                    expr_xform_neg_helper(e->terms[0].data.expn);
            else {
                e->op = YASM_EXPR_MUL;
                e->numterms = 2;
                e->terms[1].type = YASM_EXPR_INT;
                e->terms[1].data.intn = yasm_intnum_create_int(-1);
            }
            break;
        default:
            /* Everything else.  MUL will be combined when it's leveled.
             * Make a new expr (to replace e) with -1*e.
             */
            ne = yasm_xmalloc(sizeof(yasm_expr));
            ne->op = YASM_EXPR_MUL;
            ne->line = e->line;
            ne->numterms = 2;
            ne->terms[0].type = YASM_EXPR_INT;
            ne->terms[0].data.intn = yasm_intnum_create_int(-1);
            ne->terms[1].type = YASM_EXPR_EXPR;
            ne->terms[1].data.expn = e;
            return ne;
    }
    return e;
}

/* Transforms negatives into expressions that are easier to combine:
 * -x -> -1*x
 * a-b -> a+(-1*b)
 *
 * Call post-order on an expression tree to transform the entire tree.
 *
 * Returns a possibly reallocated e.
 */
static /*@only@*/ yasm_expr *
expr_xform_neg(/*@returned@*/ /*@only@*/ yasm_expr *e)
{
    switch (e->op) {
        case YASM_EXPR_NEG:
            /* Turn -x into -1*x */
            e->op = YASM_EXPR_IDENT;
            return expr_xform_neg_helper(e);
        case YASM_EXPR_SUB:
            /* Turn a-b into a+(-1*b) */

            /* change op to ADD, and recursively negate right side (if expr) */
            e->op = YASM_EXPR_ADD;
            if (e->terms[1].type == YASM_EXPR_EXPR)
                e->terms[1].data.expn =
                    expr_xform_neg_helper(e->terms[1].data.expn);
            else
                expr_xform_neg_item(e, &e->terms[1]);
            break;
        default:
            break;
    }

    return e;
}

/* Look for simple identities that make the entire result constant:
 * 0*&x, -1|x, etc.
 */
static int
expr_is_constant(yasm_expr_op op, yasm_intnum *intn)
{
    int iszero = yasm_intnum_is_zero(intn);
    return ((iszero && op == YASM_EXPR_MUL) ||
            (iszero && op == YASM_EXPR_AND) ||
            (iszero && op == YASM_EXPR_LAND) ||
            (yasm_intnum_is_neg1(intn) && op == YASM_EXPR_OR));
}

/* Look for simple "left" identities like 0+x, 1*x, etc. */
static int
expr_can_destroy_int_left(yasm_expr_op op, yasm_intnum *intn)
{
    int iszero = yasm_intnum_is_zero(intn);
    return ((yasm_intnum_is_pos1(intn) && op == YASM_EXPR_MUL) ||
            (iszero && op == YASM_EXPR_ADD) ||
            (yasm_intnum_is_neg1(intn) && op == YASM_EXPR_AND) ||
            (!iszero && op == YASM_EXPR_LAND) ||
            (iszero && op == YASM_EXPR_OR) ||
            (iszero && op == YASM_EXPR_LOR));
}

/* Look for simple "right" identities like x+|-0, x*&/1 */
static int
expr_can_destroy_int_right(yasm_expr_op op, yasm_intnum *intn)
{
    int iszero = yasm_intnum_is_zero(intn);
    int ispos1 = yasm_intnum_is_pos1(intn);
    return ((ispos1 && op == YASM_EXPR_MUL) ||
            (ispos1 && op == YASM_EXPR_DIV) ||
            (iszero && op == YASM_EXPR_ADD) ||
            (iszero && op == YASM_EXPR_SUB) ||
            (yasm_intnum_is_neg1(intn) && op == YASM_EXPR_AND) ||
            (!iszero && op == YASM_EXPR_LAND) ||
            (iszero && op == YASM_EXPR_OR) ||
            (iszero && op == YASM_EXPR_LOR) ||
            (iszero && op == YASM_EXPR_SHL) ||
            (iszero && op == YASM_EXPR_SHR));
}

/* Check for and simplify identities.  Returns new number of expr terms.
 * Sets e->op = EXPR_IDENT if numterms ends up being 1.
 * Uses numterms parameter instead of e->numterms for basis of "new" number
 * of terms.
 * Assumes int_term is *only* integer term in e.
 * NOTE: Really designed to only be used by expr_level_op().
 */
static int
expr_simplify_identity(yasm_expr *e, int numterms, int *int_term,
                       int simplify_reg_mul)
{
    int i;
    int save_numterms;

    /* Don't do this step if it's 1*REG.  Save and restore numterms so
     * yasm_expr__contains() works correctly.
     */
    save_numterms = e->numterms;
    e->numterms = numterms;
    if (simplify_reg_mul || e->op != YASM_EXPR_MUL
        || !yasm_intnum_is_pos1(e->terms[*int_term].data.intn)
        || !yasm_expr__contains(e, YASM_EXPR_REG)) {
        /* Check for simple identities that delete the intnum.
         * Don't delete if the intnum is the only thing in the expn.
         */
        if ((*int_term == 0 && numterms > 1 &&
             expr_can_destroy_int_left(e->op, e->terms[0].data.intn)) ||
            (*int_term > 0 &&
             expr_can_destroy_int_right(e->op,
                                        e->terms[*int_term].data.intn))) {
            /* Delete the intnum */
            yasm_intnum_destroy(e->terms[*int_term].data.intn);

            /* Slide everything to its right over by 1 */
            if (*int_term != numterms-1) /* if it wasn't last.. */
                memmove(&e->terms[*int_term], &e->terms[*int_term+1],
                        (numterms-1-*int_term)*sizeof(yasm_expr__item));

            /* Update numterms */
            numterms--;
            *int_term = -1;     /* no longer an int term */
        }
    }
    e->numterms = save_numterms;

    /* Check for simple identites that delete everything BUT the intnum.
     * Don't bother if the intnum is the only thing in the expn.
     */
    if (numterms > 1 && *int_term != -1 &&
        expr_is_constant(e->op, e->terms[*int_term].data.intn)) {
        /* Loop through, deleting everything but the integer term */
        for (i=0; i<e->numterms; i++)
            if (i != *int_term)
                expr_delete_term(&e->terms[i], 1);

        /* Move integer term to the first term (if not already there) */
        if (*int_term != 0)
            e->terms[0] = e->terms[*int_term];  /* structure copy */

        /* Set numterms to 1 */
        numterms = 1;
    }

    /* Compute NOT, NEG, and LNOT on single intnum. */
    if (numterms == 1 && *int_term == 0 &&
        (e->op == YASM_EXPR_NOT || e->op == YASM_EXPR_NEG ||
         e->op == YASM_EXPR_LNOT))
        yasm_intnum_calc(e->terms[0].data.intn, e->op, NULL);

    /* Change expression to IDENT if possible. */
    if (numterms == 1)
        e->op = YASM_EXPR_IDENT;

    /* Return the updated numterms */
    return numterms;
}

/* Levels the expression tree starting at e.  Eg:
 * a+(b+c) -> a+b+c
 * (a+b)+(c+d) -> a+b+c+d
 * Naturally, only levels operators that allow more than two operand terms.
 * NOTE: only does *one* level of leveling (no recursion).  Should be called
 *  post-order on a tree to combine deeper levels.
 * Also brings up any IDENT values into the current level (for ALL operators).
 * Folds (combines by evaluation) *integer* constant values if fold_const != 0.
 *
 * Returns a possibly reallocated e.
 */
/*@-mustfree@*/
static /*@only@*/ yasm_expr *
expr_level_op(/*@returned@*/ /*@only@*/ yasm_expr *e, int fold_const,
              int simplify_ident, int simplify_reg_mul)
{
    int i, j, o, fold_numterms, level_numterms, level_fold_numterms;
    int first_int_term = -1;

    /* Determine how many operands will need to be brought up (for leveling).
     * Go ahead and bring up any IDENT'ed values.
     */
    while (e->op == YASM_EXPR_IDENT && e->terms[0].type == YASM_EXPR_EXPR) {
        yasm_expr *sube = e->terms[0].data.expn;
        yasm_xfree(e);
        e = sube;
    }

    /* If non-numeric expression, don't fold constants. */
    if (e->op > YASM_EXPR_NONNUM)
        fold_const = 0;

    level_numterms = e->numterms;
    level_fold_numterms = 0;
    for (i=0; i<e->numterms; i++) {
        /* Search downward until we find something *other* than an
         * IDENT, then bring it up to the current level.
         */
        while (e->terms[i].type == YASM_EXPR_EXPR &&
               e->terms[i].data.expn->op == YASM_EXPR_IDENT) {
            yasm_expr *sube = e->terms[i].data.expn;
            e->terms[i] = sube->terms[0];
            yasm_xfree(sube);
        }

        if (e->terms[i].type == YASM_EXPR_EXPR &&
            e->terms[i].data.expn->op == e->op) {
                /* It's an expression w/the same operator, add in its numterms.
                 * But don't forget to subtract one for the expr itself!
                 */
                level_numterms += e->terms[i].data.expn->numterms - 1;

                /* If we're folding constants, count up the number of constants
                 * that will be merged in.
                 */
                if (fold_const)
                    for (j=0; j<e->terms[i].data.expn->numterms; j++)
                        if (e->terms[i].data.expn->terms[j].type ==
                            YASM_EXPR_INT)
                            level_fold_numterms++;
        }

        /* Find the first integer term (if one is present) if we're folding
         * constants.
         */
        if (fold_const && first_int_term == -1 &&
            e->terms[i].type == YASM_EXPR_INT)
            first_int_term = i;
    }

    /* Look for other integer terms if there's one and combine.
     * Also eliminate empty spaces when combining and adjust numterms
     * variables.
     */
    fold_numterms = e->numterms;
    if (first_int_term != -1) {
        for (i=first_int_term+1, o=first_int_term+1; i<e->numterms; i++) {
            if (e->terms[i].type == YASM_EXPR_INT) {
                yasm_intnum_calc(e->terms[first_int_term].data.intn, e->op,
                                 e->terms[i].data.intn);
                fold_numterms--;
                level_numterms--;
                /* make sure to delete folded intnum */
                yasm_intnum_destroy(e->terms[i].data.intn);
            } else if (o != i) {
                /* copy term if it changed places */
                e->terms[o++] = e->terms[i];
            } else
                o++;
        }

        if (simplify_ident) {
            int new_fold_numterms;
            /* Simplify identities and make IDENT if possible. */
            new_fold_numterms =
                expr_simplify_identity(e, fold_numterms, &first_int_term,
                                       simplify_reg_mul);
            level_numterms -= fold_numterms-new_fold_numterms;
            fold_numterms = new_fold_numterms;
        }
        if (fold_numterms == 1)
            e->op = YASM_EXPR_IDENT;
    }

    /* Only level operators that allow more than two operand terms.
     * Also don't bother leveling if it's not necessary to bring up any terms.
     */
    if ((e->op != YASM_EXPR_ADD && e->op != YASM_EXPR_MUL &&
         e->op != YASM_EXPR_OR && e->op != YASM_EXPR_AND &&
         e->op != YASM_EXPR_LOR && e->op != YASM_EXPR_LAND &&
         e->op != YASM_EXPR_LXOR && e->op != YASM_EXPR_XOR) ||
        level_numterms <= fold_numterms) {
        /* Downsize e if necessary */
        if (fold_numterms < e->numterms && e->numterms > 2)
            e = yasm_xrealloc(e, sizeof(yasm_expr)+((fold_numterms<2) ? 0 :
                              sizeof(yasm_expr__item)*(fold_numterms-2)));
        /* Update numterms */
        e->numterms = fold_numterms;
        return e;
    }

    /* Adjust numterms for constant folding from terms being "pulled up".
     * Careful: if there's no integer term in e, then save space for it.
     */
    if (fold_const) {
        level_numterms -= level_fold_numterms;
        if (first_int_term == -1 && level_fold_numterms != 0)
            level_numterms++;
    }

    /* Alloc more (or conceivably less, but not usually) space for e */
    e = yasm_xrealloc(e, sizeof(yasm_expr)+((level_numterms<2) ? 0 :
                      sizeof(yasm_expr__item)*(level_numterms-2)));

    /* Copy up ExprItem's.  Iterate from right to left to keep the same
     * ordering as was present originally.
     * Combine integer terms as necessary.
     */
    for (i=fold_numterms-1, o=level_numterms-1; i>=0; i--) {
        if (e->terms[i].type == YASM_EXPR_EXPR &&
            e->terms[i].data.expn->op == e->op) {
            /* bring up subexpression */
            yasm_expr *sube = e->terms[i].data.expn;

            /* copy terms right to left */
            for (j=sube->numterms-1; j>=0; j--) {
                if (fold_const && sube->terms[j].type == YASM_EXPR_INT) {
                    /* Need to fold it in.. but if there's no int term already,
                     * just copy into a new one.
                     */
                    if (first_int_term == -1) {
                        first_int_term = o--;
                        e->terms[first_int_term] = sube->terms[j];  /* struc */
                    } else {
                        yasm_intnum_calc(e->terms[first_int_term].data.intn,
                                         e->op, sube->terms[j].data.intn);
                        /* make sure to delete folded intnum */
                        yasm_intnum_destroy(sube->terms[j].data.intn);
                    }
                } else {
                    if (o == first_int_term)
                        o--;
                    e->terms[o--] = sube->terms[j];     /* structure copy */
                }
            }

            /* delete subexpression, but *don't delete nodes* (as we've just
             * copied them!)
             */
            yasm_xfree(sube);
        } else if (o != i) {
            /* copy operand if it changed places */
            if (o == first_int_term)
                o--;
            e->terms[o] = e->terms[i];
            /* If we moved the first_int_term, change first_int_num too */
            if (i == first_int_term)
                first_int_term = o;
            o--;
        } else
            o--;
    }

    /* Simplify identities, make IDENT if possible, and save to e->numterms. */
    if (simplify_ident && first_int_term != -1) {
        e->numterms = expr_simplify_identity(e, level_numterms,
                                             &first_int_term, simplify_reg_mul);
    } else {
        e->numterms = level_numterms;
        if (level_numterms == 1)
            e->op = YASM_EXPR_IDENT;
    }

    return e;
}
/*@=mustfree@*/

typedef SLIST_HEAD(yasm__exprhead, yasm__exprentry) yasm__exprhead;
typedef struct yasm__exprentry {
    /*@reldef@*/ SLIST_ENTRY(yasm__exprentry) next;
    /*@null@*/ const yasm_expr *e;
} yasm__exprentry;

static yasm_expr *
expr_expand_equ(yasm_expr *e, yasm__exprhead *eh)
{
    int i;
    yasm__exprentry ee;

    /* traverse terms */
    for (i=0; i<e->numterms; i++) {
        const yasm_expr *equ_expr;

        /* Expand equ's. */
        if (e->terms[i].type == YASM_EXPR_SYM &&
            (equ_expr = yasm_symrec_get_equ(e->terms[i].data.sym))) {
            yasm__exprentry *np;

            /* Check for circular reference */
            SLIST_FOREACH(np, eh, next) {
                if (np->e == equ_expr) {
                    yasm_error_set(YASM_ERROR_TOO_COMPLEX,
                                   N_("circular reference detected"));
                    return e;
                }
            }

            e->terms[i].type = YASM_EXPR_EXPR;
            e->terms[i].data.expn = yasm_expr_copy(equ_expr);

            /* Remember we saw this equ and recurse */
            ee.e = equ_expr;
            SLIST_INSERT_HEAD(eh, &ee, next);
            e->terms[i].data.expn = expr_expand_equ(e->terms[i].data.expn, eh);
            SLIST_REMOVE_HEAD(eh, next);
        } else if (e->terms[i].type == YASM_EXPR_EXPR)
            /* Recurse */
            e->terms[i].data.expn = expr_expand_equ(e->terms[i].data.expn, eh);
    }

    return e;
}

static yasm_expr *
expr_level_tree(yasm_expr *e, int fold_const, int simplify_ident,
                int simplify_reg_mul, int calc_bc_dist,
                yasm_expr_xform_func expr_xform_extra,
                void *expr_xform_extra_data)
{
    int i;

    e = expr_xform_neg(e);

    /* traverse terms */
    for (i=0; i<e->numterms; i++) {
        /* Recurse */
        if (e->terms[i].type == YASM_EXPR_EXPR)
            e->terms[i].data.expn =
                expr_level_tree(e->terms[i].data.expn, fold_const,
                                simplify_ident, simplify_reg_mul, calc_bc_dist,
                                expr_xform_extra, expr_xform_extra_data);
    }

    /* Check for SEG of SEG:OFF, if we match, simplify to just the segment */
    if (e->op == YASM_EXPR_SEG && e->terms[0].type == YASM_EXPR_EXPR &&
        e->terms[0].data.expn->op == YASM_EXPR_SEGOFF) {
        e->op = YASM_EXPR_IDENT;
        e->terms[0].data.expn->op = YASM_EXPR_IDENT;
        /* Destroy the second (offset) term */
        e->terms[0].data.expn->numterms = 1;
        expr_delete_term(&e->terms[0].data.expn->terms[1], 1);
    }

    /* do callback */
    e = expr_level_op(e, fold_const, simplify_ident, simplify_reg_mul);
    if (calc_bc_dist || expr_xform_extra) {
        if (calc_bc_dist)
            e = expr_xform_bc_dist(e);
        if (expr_xform_extra)
            e = expr_xform_extra(e, expr_xform_extra_data);
        e = expr_level_tree(e, fold_const, simplify_ident, simplify_reg_mul,
                            0, NULL, NULL);
    }
    return e;
}

/* Level an entire expn tree, expanding equ's as we go */
yasm_expr *
yasm_expr__level_tree(yasm_expr *e, int fold_const, int simplify_ident,
                      int simplify_reg_mul, int calc_bc_dist,
                      yasm_expr_xform_func expr_xform_extra,
                      void *expr_xform_extra_data)
{
    yasm__exprhead eh;
    SLIST_INIT(&eh);

    if (!e)
        return 0;

    e = expr_expand_equ(e, &eh);
    e = expr_level_tree(e, fold_const, simplify_ident, simplify_reg_mul,
                        calc_bc_dist, expr_xform_extra, expr_xform_extra_data);

    return e;
}

/* Comparison function for expr_order_terms().
 * Assumes ExprType enum is in canonical order.
 */
static int
expr_order_terms_compare(const void *va, const void *vb)
{
    const yasm_expr__item *a = va, *b = vb;
    return (a->type - b->type);
}

/* Reorder terms of e into canonical order.  Only reorders if reordering
 * doesn't change meaning of expression.  (eg, doesn't reorder SUB).
 * Canonical order: REG, INT, FLOAT, SYM, EXPR.
 * Multiple terms of a single type are kept in the same order as in
 * the original expression.
 * NOTE: Only performs reordering on *one* level (no recursion).
 */
void
yasm_expr__order_terms(yasm_expr *e)
{
    /* don't bother reordering if only one element */
    if (e->numterms == 1)
        return;

    /* only reorder some types of operations */
    switch (e->op) {
        case YASM_EXPR_ADD:
        case YASM_EXPR_MUL:
        case YASM_EXPR_OR:
        case YASM_EXPR_AND:
        case YASM_EXPR_XOR:
        case YASM_EXPR_LOR:
        case YASM_EXPR_LAND:
        case YASM_EXPR_LXOR:
            /* Use mergesort to sort.  It's fast on already sorted values and a
             * stable sort (multiple terms of same type are kept in the same
             * order).
             */
            yasm__mergesort(e->terms, (size_t)e->numterms,
                            sizeof(yasm_expr__item), expr_order_terms_compare);
            break;
        default:
            break;
    }
}

static void
expr_item_copy(yasm_expr__item *dest, const yasm_expr__item *src)
{
    dest->type = src->type;
    switch (src->type) {
        case YASM_EXPR_SYM:
            /* Symbols don't need to be copied */
            dest->data.sym = src->data.sym;
            break;
        case YASM_EXPR_PRECBC:
            /* Nor do direct bytecode references */
            dest->data.precbc = src->data.precbc;
            break;
        case YASM_EXPR_EXPR:
            dest->data.expn = yasm_expr__copy_except(src->data.expn, -1);
            break;
        case YASM_EXPR_INT:
            dest->data.intn = yasm_intnum_copy(src->data.intn);
            break;
        case YASM_EXPR_FLOAT:
            dest->data.flt = yasm_floatnum_copy(src->data.flt);
            break;
        case YASM_EXPR_REG:
            dest->data.reg = src->data.reg;
            break;
        case YASM_EXPR_SUBST:
            dest->data.subst = src->data.subst;
            break;
        default:
            break;
    }
}

/* Copy entire expression EXCEPT for index "except" at *top level only*. */
yasm_expr *
yasm_expr__copy_except(const yasm_expr *e, int except)
{
    yasm_expr *n;
    int i;
    
    n = yasm_xmalloc(sizeof(yasm_expr) +
                     sizeof(yasm_expr__item)*(e->numterms<2?0:e->numterms-2));

    n->op = e->op;
    n->line = e->line;
    n->numterms = e->numterms;
    for (i=0; i<e->numterms; i++) {
        if (i != except)
            expr_item_copy(&n->terms[i], &e->terms[i]);
    }

    return n;
}

static void
expr_delete_term(yasm_expr__item *term, int recurse)
{
    switch (term->type) {
        case YASM_EXPR_INT:
            yasm_intnum_destroy(term->data.intn);
            break;
        case YASM_EXPR_FLOAT:
            yasm_floatnum_destroy(term->data.flt);
            break;
        case YASM_EXPR_EXPR:
            if (recurse)
                yasm_expr_destroy(term->data.expn);
            break;
        default:
            break;
    }
}

static int
expr_destroy_each(/*@only@*/ yasm_expr *e, /*@unused@*/ void *d)
{
    int i;
    for (i=0; i<e->numterms; i++)
        expr_delete_term(&e->terms[i], 0);
    yasm_xfree(e);      /* free ourselves */
    return 0;   /* don't stop recursion */
}

/*@-mustfree@*/
void
yasm_expr_destroy(yasm_expr *e)
{
    expr_traverse_nodes_post(e, NULL, expr_destroy_each);
}
/*@=mustfree@*/

int
yasm_expr_is_op(const yasm_expr *e, yasm_expr_op op)
{
    return (e->op == op);
}

static int
expr_contains_callback(const yasm_expr__item *ei, void *d)
{
    yasm_expr__type *t = d;
    return (ei->type & *t);
}

int
yasm_expr__contains(const yasm_expr *e, yasm_expr__type t)
{
    return yasm_expr__traverse_leaves_in_const(e, &t, expr_contains_callback);
}

typedef struct subst_cbd {
    unsigned int num_items;
    const yasm_expr__item *items;
} subst_cbd;

static int
expr_subst_callback(yasm_expr__item *ei, void *d)
{
    subst_cbd *cbd = d;
    if (ei->type != YASM_EXPR_SUBST)
        return 0;
    if (ei->data.subst >= cbd->num_items)
        return 1;   /* error */
    expr_item_copy(ei, &cbd->items[ei->data.subst]);
    return 0;
}

int
yasm_expr__subst(yasm_expr *e, unsigned int num_items,
                 const yasm_expr__item *items)
{
    subst_cbd cbd;
    cbd.num_items = num_items;
    cbd.items = items;
    return yasm_expr__traverse_leaves_in(e, &cbd, expr_subst_callback);
}

/* Traverse over expression tree, calling func for each operation AFTER the
 * branches (if expressions) have been traversed (eg, postorder
 * traversal).  The data pointer d is passed to each func call.
 *
 * Stops early (and returns 1) if func returns 1.  Otherwise returns 0.
 */
static int
expr_traverse_nodes_post(yasm_expr *e, void *d,
                         int (*func) (/*@null@*/ yasm_expr *e,
                                      /*@null@*/ void *d))
{
    int i;

    if (!e)
        return 0;

    /* traverse terms */
    for (i=0; i<e->numterms; i++) {
        if (e->terms[i].type == YASM_EXPR_EXPR &&
            expr_traverse_nodes_post(e->terms[i].data.expn, d, func))
            return 1;
    }

    /* do callback */
    return func(e, d);
}

/* Traverse over expression tree in order, calling func for each leaf
 * (non-operation).  The data pointer d is passed to each func call.
 *
 * Stops early (and returns 1) if func returns 1.  Otherwise returns 0.
 */
int
yasm_expr__traverse_leaves_in_const(const yasm_expr *e, void *d,
    int (*func) (/*@null@*/ const yasm_expr__item *ei, /*@null@*/ void *d))
{
    int i;

    if (!e)
        return 0;

    for (i=0; i<e->numterms; i++) {
        if (e->terms[i].type == YASM_EXPR_EXPR) {
            if (yasm_expr__traverse_leaves_in_const(e->terms[i].data.expn, d,
                                                    func))
                return 1;
        } else {
            if (func(&e->terms[i], d))
                return 1;
        }
    }
    return 0;
}

/* Traverse over expression tree in order, calling func for each leaf
 * (non-operation).  The data pointer d is passed to each func call.
 *
 * Stops early (and returns 1) if func returns 1.  Otherwise returns 0.
 */
int
yasm_expr__traverse_leaves_in(yasm_expr *e, void *d,
    int (*func) (/*@null@*/ yasm_expr__item *ei, /*@null@*/ void *d))
{
    int i;

    if (!e)
        return 0;

    for (i=0; i<e->numterms; i++) {
        if (e->terms[i].type == YASM_EXPR_EXPR) {
            if (yasm_expr__traverse_leaves_in(e->terms[i].data.expn, d, func))
                return 1;
        } else {
            if (func(&e->terms[i], d))
                return 1;
        }
    }
    return 0;
}

yasm_expr *
yasm_expr_extract_deep_segoff(yasm_expr **ep)
{
    yasm_expr *retval;
    yasm_expr *e = *ep;
    int i;

    /* Try to extract at this level */
    retval = yasm_expr_extract_segoff(ep);
    if (retval)
        return retval;

    /* Not at this level?  Search any expr children. */
    for (i=0; i<e->numterms; i++) {
        if (e->terms[i].type == YASM_EXPR_EXPR) {
            retval = yasm_expr_extract_deep_segoff(&e->terms[i].data.expn);
            if (retval)
                return retval;
        }
    }

    /* Didn't find one */
    return NULL;
}

yasm_expr *
yasm_expr_extract_segoff(yasm_expr **ep)
{
    yasm_expr *retval;
    yasm_expr *e = *ep;

    /* If not SEG:OFF, we can't do this transformation */
    if (e->op != YASM_EXPR_SEGOFF)
        return NULL;

    /* Extract the SEG portion out to its own expression */
    if (e->terms[0].type == YASM_EXPR_EXPR)
        retval = e->terms[0].data.expn;
    else {
        /* Need to build IDENT expression to hold non-expression contents */
        retval = yasm_xmalloc(sizeof(yasm_expr));
        retval->op = YASM_EXPR_IDENT;
        retval->numterms = 1;
        retval->terms[0] = e->terms[0]; /* structure copy */
    }

    /* Delete the SEG: portion by changing the expression into an IDENT */
    e->op = YASM_EXPR_IDENT;
    e->numterms = 1;
    e->terms[0] = e->terms[1];  /* structure copy */

    return retval;
}

yasm_expr *
yasm_expr_extract_wrt(yasm_expr **ep)
{
    yasm_expr *retval;
    yasm_expr *e = *ep;

    /* If not WRT, we can't do this transformation */
    if (e->op != YASM_EXPR_WRT)
        return NULL;

    /* Extract the right side portion out to its own expression */
    if (e->terms[1].type == YASM_EXPR_EXPR)
        retval = e->terms[1].data.expn;
    else {
        /* Need to build IDENT expression to hold non-expression contents */
        retval = yasm_xmalloc(sizeof(yasm_expr));
        retval->op = YASM_EXPR_IDENT;
        retval->numterms = 1;
        retval->terms[0] = e->terms[1]; /* structure copy */
    }

    /* Delete the right side portion by changing the expr into an IDENT */
    e->op = YASM_EXPR_IDENT;
    e->numterms = 1;

    return retval;
}

/*@-unqualifiedtrans -nullderef -nullstate -onlytrans@*/
yasm_intnum *
yasm_expr_get_intnum(yasm_expr **ep, int calc_bc_dist)
{
    *ep = yasm_expr_simplify(*ep, calc_bc_dist);

    if ((*ep)->op == YASM_EXPR_IDENT && (*ep)->terms[0].type == YASM_EXPR_INT)
        return (*ep)->terms[0].data.intn;
    else
        return (yasm_intnum *)NULL;
}
/*@=unqualifiedtrans =nullderef -nullstate -onlytrans@*/

/*@-unqualifiedtrans -nullderef -nullstate -onlytrans@*/
const yasm_symrec *
yasm_expr_get_symrec(yasm_expr **ep, int simplify)
{
    if (simplify)
        *ep = yasm_expr_simplify(*ep, 0);

    if ((*ep)->op == YASM_EXPR_IDENT && (*ep)->terms[0].type == YASM_EXPR_SYM)
        return (*ep)->terms[0].data.sym;
    else
        return (yasm_symrec *)NULL;
}
/*@=unqualifiedtrans =nullderef -nullstate -onlytrans@*/

/*@-unqualifiedtrans -nullderef -nullstate -onlytrans@*/
const uintptr_t *
yasm_expr_get_reg(yasm_expr **ep, int simplify)
{
    if (simplify)
        *ep = yasm_expr_simplify(*ep, 0);

    if ((*ep)->op == YASM_EXPR_IDENT && (*ep)->terms[0].type == YASM_EXPR_REG)
        return &((*ep)->terms[0].data.reg);
    else
        return NULL;
}
/*@=unqualifiedtrans =nullderef -nullstate -onlytrans@*/

void
yasm_expr_print(const yasm_expr *e, FILE *f)
{
    char opstr[8];
    int i;

    if (!e) {
        fprintf(f, "(nil)");
        return;
    }

    switch (e->op) {
        case YASM_EXPR_ADD:
            strcpy(opstr, "+");
            break;
        case YASM_EXPR_SUB:
            strcpy(opstr, "-");
            break;
        case YASM_EXPR_MUL:
            strcpy(opstr, "*");
            break;
        case YASM_EXPR_DIV:
            strcpy(opstr, "/");
            break;
        case YASM_EXPR_SIGNDIV:
            strcpy(opstr, "//");
            break;
        case YASM_EXPR_MOD:
            strcpy(opstr, "%");
            break;
        case YASM_EXPR_SIGNMOD:
            strcpy(opstr, "%%");
            break;
        case YASM_EXPR_NEG:
            fprintf(f, "-");
            opstr[0] = 0;
            break;
        case YASM_EXPR_NOT:
            fprintf(f, "~");
            opstr[0] = 0;
            break;
        case YASM_EXPR_OR:
            strcpy(opstr, "|");
            break;
        case YASM_EXPR_AND:
            strcpy(opstr, "&");
            break;
        case YASM_EXPR_XOR:
            strcpy(opstr, "^");
            break;
        case YASM_EXPR_XNOR:
            strcpy(opstr, "XNOR");
            break;
        case YASM_EXPR_NOR:
            strcpy(opstr, "NOR");
            break;
        case YASM_EXPR_SHL:
            strcpy(opstr, "<<");
            break;
        case YASM_EXPR_SHR:
            strcpy(opstr, ">>");
            break;
        case YASM_EXPR_LOR:
            strcpy(opstr, "||");
            break;
        case YASM_EXPR_LAND:
            strcpy(opstr, "&&");
            break;
        case YASM_EXPR_LNOT:
            strcpy(opstr, "!");
            break;
        case YASM_EXPR_LXOR:
            strcpy(opstr, "^^");
            break;
        case YASM_EXPR_LXNOR:
            strcpy(opstr, "LXNOR");
            break;
        case YASM_EXPR_LNOR:
            strcpy(opstr, "LNOR");
            break;
        case YASM_EXPR_LT:
            strcpy(opstr, "<");
            break;
        case YASM_EXPR_GT:
            strcpy(opstr, ">");
            break;
        case YASM_EXPR_LE:
            strcpy(opstr, "<=");
            break;
        case YASM_EXPR_GE:
            strcpy(opstr, ">=");
            break;
        case YASM_EXPR_NE:
            strcpy(opstr, "!=");
            break;
        case YASM_EXPR_EQ:
            strcpy(opstr, "==");
            break;
        case YASM_EXPR_SEG:
            fprintf(f, "SEG ");
            opstr[0] = 0;
            break;
        case YASM_EXPR_WRT:
            strcpy(opstr, " WRT ");
            break;
        case YASM_EXPR_SEGOFF:
            strcpy(opstr, ":");
            break;
        case YASM_EXPR_IDENT:
            opstr[0] = 0;
            break;
        default:
            strcpy(opstr, " !UNK! ");
            break;
    }
    for (i=0; i<e->numterms; i++) {
        switch (e->terms[i].type) {
            case YASM_EXPR_PRECBC:
                fprintf(f, "{%lx}",
                        yasm_bc_next_offset(e->terms[i].data.precbc));
                break;
            case YASM_EXPR_SYM:
                fprintf(f, "%s", yasm_symrec_get_name(e->terms[i].data.sym));
                break;
            case YASM_EXPR_EXPR:
                fprintf(f, "(");
                yasm_expr_print(e->terms[i].data.expn, f);
                fprintf(f, ")");
                break;
            case YASM_EXPR_INT:
                yasm_intnum_print(e->terms[i].data.intn, f);
                break;
            case YASM_EXPR_FLOAT:
                yasm_floatnum_print(e->terms[i].data.flt, f);
                break;
            case YASM_EXPR_REG:
                /* FIXME */
                /*yasm_arch_reg_print(arch, e->terms[i].data.reg, f);*/
                break;
            case YASM_EXPR_SUBST:
                fprintf(f, "[%u]", e->terms[i].data.subst);
                break;
            case YASM_EXPR_NONE:
                break;
        }
        if (i < e->numterms-1)
            fprintf(f, "%s", opstr);
    }
}

unsigned int
yasm_expr_size(const yasm_expr *e)
{
    int i;
    int seen = 0;
    unsigned int size = 0, newsize;

    if (e->op == YASM_EXPR_IDENT) {
        if (e->terms[0].type == YASM_EXPR_SYM)
            return yasm_symrec_get_size(e->terms[0].data.sym);
        return 0;
    }
    if (e->op != YASM_EXPR_ADD && e->op != YASM_EXPR_SUB)
        return 0;

    for (i=0; i<e->numterms; i++) {
        newsize = 0;
        switch (e->terms[i].type) {
        case YASM_EXPR_EXPR:
            newsize = yasm_expr_size(e->terms[i].data.expn);
            break;
        case YASM_EXPR_SYM:
            newsize = yasm_symrec_get_size(e->terms[i].data.sym);
            break;
        default:
            break;
        }
        if (newsize) {
            size = newsize;
            if (seen)
                /* either sum of idents (?!) or substract of idents */
                return 0;
            seen = 1;
        }
    }
    /* exactly one offset */
    return size;
}

const char *
yasm_expr_segment(const yasm_expr *e)
{
    int i;
    int seen = 0;
    const char *segment = NULL;

    if (e->op == YASM_EXPR_IDENT) {
        if (e->terms[0].type == YASM_EXPR_SYM)
            return yasm_symrec_get_segment(e->terms[0].data.sym);
        return NULL;
    }
    if (e->op != YASM_EXPR_ADD && e->op != YASM_EXPR_SUB)
        return NULL;

    for (i=0; i<e->numterms; i++) {
        if ((e->op == YASM_EXPR_ADD || !i) &&
                e->terms[i].type == YASM_EXPR_EXPR) {
            if ((segment = yasm_expr_segment(e->terms[i].data.expn))) {
                if (seen) {
                    /* either sum of idents (?!) or substract of idents */
                    return NULL;
                }
                seen = 1;
            }
        }
    }
    /* exactly one offset */
    return segment;
}
