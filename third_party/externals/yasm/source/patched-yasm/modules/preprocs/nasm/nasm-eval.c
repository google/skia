/* eval.c    expression evaluator for the Netwide Assembler
 *
 * The Netwide Assembler is copyright (C) 1996 Simon Tatham and
 * Julian Hall. All rights reserved. The software is
 * redistributable under the licence given in the file "Licence"
 * distributed in the NASM archive.
 *
 * initial version 27/iii/95 by Simon Tatham
 */
#include <util.h>
#include <libyasm-stdint.h>
#include <libyasm/coretype.h>
#include <libyasm/intnum.h>
#include <libyasm/expr.h>
#include <libyasm/symrec.h>
#include <ctype.h>

#include "nasm.h"
#include "nasmlib.h"
#include "nasm-eval.h"

/* The assembler symbol table. */
extern yasm_symtab *nasm_symtab;

static scanner scan;    /* Address of scanner routine */
static efunc error;     /* Address of error reporting routine */

static struct tokenval *tokval;   /* The current token */
static int i;                     /* The t_type of tokval */

static void *scpriv;

/*
 * Recursive-descent parser. Called with a single boolean operand,
 * which is TRUE if the evaluation is critical (i.e. unresolved
 * symbols are an error condition). Must update the global `i' to
 * reflect the token after the parsed string. May return NULL.
 *
 * evaluate() should report its own errors: on return it is assumed
 * that if NULL has been returned, the error has already been
 * reported.
 */

/*
 * Grammar parsed is:
 *
 * expr  : bexpr [ WRT expr6 ]
 * bexpr : rexp0 or expr0 depending on relative-mode setting
 * rexp0 : rexp1 [ {||} rexp1...]
 * rexp1 : rexp2 [ {^^} rexp2...]
 * rexp2 : rexp3 [ {&&} rexp3...]
 * rexp3 : expr0 [ {=,==,<>,!=,<,>,<=,>=} expr0 ]
 * expr0 : expr1 [ {|} expr1...]
 * expr1 : expr2 [ {^} expr2...]
 * expr2 : expr3 [ {&} expr3...]
 * expr3 : expr4 [ {<<,>>} expr4...]
 * expr4 : expr5 [ {+,-} expr5...]
 * expr5 : expr6 [ {*,/,%,//,%%} expr6...]
 * expr6 : { ~,+,-,SEG } expr6
 *       | (bexpr)
 *       | symbol
 *       | $
 *       | number
 */

static yasm_expr *rexp0(void), *rexp1(void), *rexp2(void), *rexp3(void);

static yasm_expr *expr0(void), *expr1(void), *expr2(void), *expr3(void);
static yasm_expr *expr4(void), *expr5(void), *expr6(void);

static yasm_expr *(*bexpr)(void);

static yasm_expr *rexp0(void) 
{
    yasm_expr *e, *f;

    e = rexp1();
    if (!e)
        return NULL;

    while (i == TOKEN_DBL_OR) 
    {   
        i = scan(scpriv, tokval);
        f = rexp1();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        e = yasm_expr_create_tree(e, YASM_EXPR_LOR, f, 0);
    }
    return e;
}

static yasm_expr *rexp1(void)
{
    yasm_expr *e, *f;

    e = rexp2();
    if (!e)
        return NULL;
    
    while (i == TOKEN_DBL_XOR) 
    {
        i = scan(scpriv, tokval);
        f = rexp2();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        e = yasm_expr_create_tree(e, YASM_EXPR_LXOR, f, 0);
    }
    return e;
}

static yasm_expr *rexp2(void) 
{
    yasm_expr *e, *f;

    e = rexp3();
    if (!e)
        return NULL;
    while (i == TOKEN_DBL_AND) 
    {
        i = scan(scpriv, tokval);
        f = rexp3();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        e = yasm_expr_create_tree(e, YASM_EXPR_LAND, f, 0);
    }
    return e;
}

static yasm_expr *rexp3(void) 
{
    yasm_expr *e, *f;

    e = expr0();
    if (!e)
        return NULL;

    while (i == TOKEN_EQ || i == TOKEN_LT || i == TOKEN_GT ||
           i == TOKEN_NE || i == TOKEN_LE || i == TOKEN_GE) 
    {
        int j = i;
        i = scan(scpriv, tokval);
        f = expr0();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        switch (j) 
        {
            case TOKEN_EQ:
                e = yasm_expr_create_tree(e, YASM_EXPR_EQ, f, 0);
                break;
            case TOKEN_LT:
                e = yasm_expr_create_tree(e, YASM_EXPR_LT, f, 0);
                break;
            case TOKEN_GT:
                e = yasm_expr_create_tree(e, YASM_EXPR_GT, f, 0);
                break;
            case TOKEN_NE:
                e = yasm_expr_create_tree(e, YASM_EXPR_NE, f, 0);
                break;
            case TOKEN_LE:
                e = yasm_expr_create_tree(e, YASM_EXPR_LE, f, 0);
                break;
            case TOKEN_GE:
                e = yasm_expr_create_tree(e, YASM_EXPR_GE, f, 0);
                break;
        }
    }
    return e;
}

static yasm_expr *expr0(void) 
{
    yasm_expr *e, *f;

    e = expr1();
    if (!e)
        return NULL;

    while (i == '|') 
    {
        i = scan(scpriv, tokval);
        f = expr1();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        e = yasm_expr_create_tree(e, YASM_EXPR_OR, f, 0);
    }
    return e;
}

static yasm_expr *expr1(void) 
{
    yasm_expr *e, *f;

    e = expr2();
    if (!e)
        return NULL;

    while (i == '^') {
        i = scan(scpriv, tokval);
        f = expr2();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        e = yasm_expr_create_tree(e, YASM_EXPR_XOR, f, 0);
    }
    return e;
}

static yasm_expr *expr2(void) 
{
    yasm_expr *e, *f;

    e = expr3();
    if (!e)
        return NULL;

    while (i == '&') {
        i = scan(scpriv, tokval);
        f = expr3();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        e = yasm_expr_create_tree(e, YASM_EXPR_AND, f, 0);
    }
    return e;
}

static yasm_expr *expr3(void) 
{
    yasm_expr *e, *f;

    e = expr4();
    if (!e)
        return NULL;

    while (i == TOKEN_SHL || i == TOKEN_SHR) 
    {
        int j = i;
        i = scan(scpriv, tokval);
        f = expr4();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }

        switch (j) {
            case TOKEN_SHL:
                e = yasm_expr_create_tree(e, YASM_EXPR_SHL, f, 0);
                break;
            case TOKEN_SHR:
                e = yasm_expr_create_tree(e, YASM_EXPR_SHR, f, 0);
                break;
        }
    }
    return e;
}

static yasm_expr *expr4(void)
{
    yasm_expr *e, *f;

    e = expr5();
    if (!e)
        return NULL;
    while (i == '+' || i == '-') 
    {
        int j = i;
        i = scan(scpriv, tokval);
        f = expr5();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }
        switch (j) {
          case '+':
            e = yasm_expr_create_tree(e, YASM_EXPR_ADD, f, 0);
            break;
          case '-':
            e = yasm_expr_create_tree(e, YASM_EXPR_SUB, f, 0);
            break;
        }
    }
    return e;
}

static yasm_expr *expr5(void)
{
    yasm_expr *e, *f;

    e = expr6();
    if (!e)
        return NULL;
    while (i == '*' || i == '/' || i == '%' ||
           i == TOKEN_SDIV || i == TOKEN_SMOD) 
    {
        int j = i;
        i = scan(scpriv, tokval);
        f = expr6();
        if (!f) {
            yasm_expr_destroy(e);
            return NULL;
        }
        switch (j) {
          case '*':
            e = yasm_expr_create_tree(e, YASM_EXPR_MUL, f, 0);
            break;
          case '/':
            e = yasm_expr_create_tree(e, YASM_EXPR_DIV, f, 0);
            break;
          case '%':
            e = yasm_expr_create_tree(e, YASM_EXPR_MOD, f, 0);
            break;
          case TOKEN_SDIV:
            e = yasm_expr_create_tree(e, YASM_EXPR_SIGNDIV, f, 0);
            break;
          case TOKEN_SMOD:
            e = yasm_expr_create_tree(e, YASM_EXPR_SIGNMOD, f, 0);
            break;
        }
    }
    return e;
}

static yasm_expr *expr6(void)
{
    yasm_expr *e = NULL;

    if (i == '-') {
        i = scan(scpriv, tokval);
        e = expr6();
        if (!e)
            return NULL;
        return yasm_expr_create_branch(YASM_EXPR_NEG, e, 0);
    } else if (i == '+') {
        i = scan(scpriv, tokval);
        return expr6();
    } else if (i == '~') {
        i = scan(scpriv, tokval);
        e = expr6();
        if (!e)
            return NULL;
        return yasm_expr_create_branch(YASM_EXPR_NOT, e, 0);
    } else if (i == TOKEN_SEG) {
        i = scan(scpriv, tokval);
        e = expr6();
        if (!e)
            return NULL;
        error(ERR_NONFATAL, "%s not supported", "SEG");
        return e;
    } else if (i == '(') {
        i = scan(scpriv, tokval);
        e = bexpr();
        if (!e)
            return NULL;
        if (i != ')') {
            error(ERR_NONFATAL, "expecting `)'");
            return NULL;
        }
        i = scan(scpriv, tokval);
        return e;
    } 
    else if (i == TOKEN_NUM || i == TOKEN_ID ||
             i == TOKEN_HERE || i == TOKEN_BASE) 
    {
        switch (i) {
          case TOKEN_NUM:
            e = yasm_expr_create_ident(yasm_expr_int(tokval->t_integer), 0);
            break;
          case TOKEN_ID:
            if (nasm_symtab) {
                yasm_symrec *sym =
                    yasm_symtab_get(nasm_symtab, tokval->t_charptr);
                if (sym) {
                    e = yasm_expr_create_ident(yasm_expr_sym(sym), 0);
                } else {
                    error(ERR_NONFATAL,
                          "undefined symbol `%s' in preprocessor",
                          tokval->t_charptr);
                    e = yasm_expr_create_ident(yasm_expr_int(
                        yasm_intnum_create_int(1)), 0);
                }
                break;
            }
            /*fallthrough*/
          case TOKEN_HERE:
          case TOKEN_BASE:
            error(ERR_NONFATAL,
                  "cannot reference symbol `%s' in preprocessor",
                  (i == TOKEN_ID ? tokval->t_charptr :
                   i == TOKEN_HERE ? "$" : "$$"));
            e = yasm_expr_create_ident(yasm_expr_int(yasm_intnum_create_int(1)),
                                       0);
            break;
        }
        i = scan(scpriv, tokval);
        return e;
    } else {
        error(ERR_NONFATAL, "expression syntax error");
        return NULL;
    }
}

yasm_expr *nasm_evaluate (scanner sc, void *scprivate, struct tokenval *tv,
                          int critical, efunc report_error)
{
    if (critical & CRITICAL) {
        critical &= ~CRITICAL;
        bexpr = rexp0;
    } else
        bexpr = expr0;

    scan = sc;
    scpriv = scprivate;
    tokval = tv;
    error = report_error;

    if (tokval->t_type == TOKEN_INVALID)
        i = scan(scpriv, tokval);
    else
        i = tokval->t_type;

    return bexpr ();
}
