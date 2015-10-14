/*
 * Value/Parameter type functions
 *
 *  Copyright (C) 2001-2007  Peter Johnson
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
#include "valparam.h"

#include "errwarn.h"
#include "intnum.h"
#include "expr.h"
#include "symrec.h"

#include "section.h"

void
yasm_call_directive(const yasm_directive *directive, yasm_object *object,
                    yasm_valparamhead *valparams,
                    yasm_valparamhead *objext_valparams, unsigned long line)
{
    yasm_valparam *vp;

    if ((directive->flags & (YASM_DIR_ARG_REQUIRED|YASM_DIR_ID_REQUIRED)) &&
        (!valparams || !yasm_vps_first(valparams))) {
        yasm_error_set(YASM_ERROR_SYNTAX,
                       N_("directive `%s' requires an argument"),
                       directive->name);
        return;
    }
    if (valparams) {
        vp = yasm_vps_first(valparams);
        if ((directive->flags & YASM_DIR_ID_REQUIRED) &&
            vp->type != YASM_PARAM_ID) {
            yasm_error_set(YASM_ERROR_SYNTAX,
                N_("directive `%s' requires an identifier parameter"),
                directive->name);
            return;
        }
    }
    directive->handler(object, valparams, objext_valparams, line);
}

yasm_valparam *
yasm_vp_create_id(/*@keep@*/ char *v, /*@keep@*/ char *p, int id_prefix)
{
    yasm_valparam *r = yasm_xmalloc(sizeof(yasm_valparam));
    r->val = v;
    r->type = YASM_PARAM_ID;
    r->param.id = p;
    r->id_prefix = (char)id_prefix;
    return r;
}

yasm_valparam *
yasm_vp_create_string(/*@keep@*/ char *v, /*@keep@*/ char *p)
{
    yasm_valparam *r = yasm_xmalloc(sizeof(yasm_valparam));
    r->val = v;
    r->type = YASM_PARAM_STRING;
    r->param.str = p;
    r->id_prefix = '\0';
    return r;
}

yasm_valparam *
yasm_vp_create_expr(/*@keep@*/ char *v, /*@keep@*/ yasm_expr *p)
{
    yasm_valparam *r = yasm_xmalloc(sizeof(yasm_valparam));
    r->val = v;
    r->type = YASM_PARAM_EXPR;
    r->param.e = p;
    r->id_prefix = '\0';
    return r;
}

/*@null@*/ /*@only@*/ yasm_expr *
yasm_vp_expr(const yasm_valparam *vp, yasm_symtab *symtab, unsigned long line)
{
    if (!vp)
        return NULL;
    switch (vp->type) {
        case YASM_PARAM_ID:
            return yasm_expr_create_ident(yasm_expr_sym(
                yasm_symtab_use(symtab, yasm_vp_id(vp), line)), line);
        case YASM_PARAM_EXPR:
            return yasm_expr_copy(vp->param.e);
        default:
            return NULL;
    }
}

/*@null@*/ /*@dependent@*/ const char *
yasm_vp_string(const yasm_valparam *vp)
{
    if (!vp)
        return NULL;
    switch (vp->type) {
        case YASM_PARAM_ID:
            return vp->param.id;
        case YASM_PARAM_STRING:
            return vp->param.str;
        default:
            return NULL;
    }
}

/*@null@*/ /*@dependent@*/ const char *
yasm_vp_id(const yasm_valparam *vp)
{
    if (!vp)
        return NULL;
    if (vp->type == YASM_PARAM_ID) {
        if (vp->param.id[0] == vp->id_prefix)
            return &vp->param.id[1];
        else
            return vp->param.id;
    }
    return NULL;
}

void
yasm_vps_delete(yasm_valparamhead *headp)
{
    yasm_valparam *cur, *next;

    cur = STAILQ_FIRST(headp);
    while (cur) {
        next = STAILQ_NEXT(cur, link);
        if (cur->val)
            yasm_xfree(cur->val);
        switch (cur->type) {
            case YASM_PARAM_ID:
                yasm_xfree(cur->param.id);
                break;
            case YASM_PARAM_STRING:
                yasm_xfree(cur->param.str);
                break;
            case YASM_PARAM_EXPR:
                yasm_expr_destroy(cur->param.e);
                break;
        }
        yasm_xfree(cur);
        cur = next;
    }
    STAILQ_INIT(headp);
}

void
yasm_vps_print(const yasm_valparamhead *headp, FILE *f)
{
    const yasm_valparam *vp;

    if(!headp) {
        fprintf(f, "(none)");
        return;
    }

    yasm_vps_foreach(vp, headp) {
        if (vp->val)
            fprintf(f, "(\"%s\",", vp->val);
        else
            fprintf(f, "((nil),");
        switch (vp->type) {
            case YASM_PARAM_ID:
                fprintf(f, "%s", vp->param.id);
                break;
            case YASM_PARAM_STRING:
                fprintf(f, "\"%s\"", vp->param.str);
                break;
            case YASM_PARAM_EXPR:
                yasm_expr_print(vp->param.e, f);
                break;
        }
        fprintf(f, ")");
        if (yasm_vps_next(vp))
            fprintf(f, ",");
    }
}

yasm_valparamhead *
yasm_vps_create(void)
{
    yasm_valparamhead *headp = yasm_xmalloc(sizeof(yasm_valparamhead));
    yasm_vps_initialize(headp);
    return headp;
}

void
yasm_vps_destroy(yasm_valparamhead *headp)
{
    yasm_vps_delete(headp);
    yasm_xfree(headp);
}

int
yasm_dir_helper(void *obj, yasm_valparam *vp_first, unsigned long line,
                const yasm_dir_help *help, size_t nhelp, void *data,
                int (*helper_valparam) (void *obj, yasm_valparam *vp,
                                        unsigned long line, void *data))
{
    yasm_valparam *vp = vp_first;
    int anymatched = 0;
    int matched;

    if (!vp)
        return 0;

    do {
        const char *s;
        size_t i;

        matched = 0;
        if (!vp->val && (s = yasm_vp_id(vp))) {
            for (i=0; i<nhelp; i++) {
                if (help[i].needsparam == 0 &&
                    yasm__strcasecmp(s, help[i].name) == 0) {
                    if (help[i].helper(obj, vp, line,
                                       ((char *)data)+help[i].off,
                                       help[i].arg) != 0)
                        return -1;
                    matched = 1;
                    anymatched = 1;
                    break;
                }
            }
        } else if (vp->val) {
            for (i=0; i<nhelp; i++) {
                if (help[i].needsparam == 1 &&
                    yasm__strcasecmp(vp->val, help[i].name) == 0) {
                    if (help[i].helper(obj, vp, line,
                                       ((char *)data)+help[i].off,
                                       help[i].arg) != 0)
                        return -1;
                    matched = 1;
                    anymatched = 1;
                    break;
                }
            }
        }

        if (!matched) {
            int final = helper_valparam(obj, vp, line, data);
            if (final < 0)
                return -1;
            if (final > 0)
                anymatched = 1;
        }
    } while((vp = yasm_vps_next(vp)));

    return anymatched;
}

int
yasm_dir_helper_flag_or(void *obj, yasm_valparam *vp, unsigned long line,
                        void *d, uintptr_t flag)
{
    unsigned long *flags = (unsigned long *)d;
    *flags |= flag;
    return 0;
}

int
yasm_dir_helper_flag_and(void *obj, yasm_valparam *vp, unsigned long line,
                         void *d, uintptr_t flag)
{
    unsigned long *flags = (unsigned long *)d;
    *flags &= ~flag;
    return 0;
}

int
yasm_dir_helper_flag_set(void *obj, yasm_valparam *vp, unsigned long line,
                         void *d, uintptr_t flag)
{
    unsigned long *flags = (unsigned long *)d;
    *flags = flag;
    return 0;
}

int
yasm_dir_helper_expr(void *obj, yasm_valparam *vp, unsigned long line,
                     void *data, uintptr_t arg)
{
    yasm_object *object = (yasm_object *)obj;
    yasm_expr **expr = (yasm_expr **)data;

    if (*expr)
        yasm_expr_destroy(*expr);
    if (!(*expr = yasm_vp_expr(vp, object->symtab, line))) {
        yasm_error_set(YASM_ERROR_VALUE, N_("argument to `%s' is not an expression"),
                       vp->val);
        return -1;
    }
    return 0;
}

int
yasm_dir_helper_intn(void *obj, yasm_valparam *vp, unsigned long line,
                     void *data, uintptr_t arg)
{
    yasm_object *object = (yasm_object *)obj;
    /*@only@*/ /*@null@*/ yasm_expr *e;
    /*@dependent@*/ /*@null@*/ yasm_intnum *local;
    yasm_intnum **intn = (yasm_intnum **)data;

    if (*intn)
        yasm_intnum_destroy(*intn);
    if (!(e = yasm_vp_expr(vp, object->symtab, line)) ||
        !(local = yasm_expr_get_intnum(&e, 0))) {
        yasm_error_set(YASM_ERROR_NOT_CONSTANT,
                       N_("argument to `%s' is not an integer"),
                       vp->val);
        if (e)
            yasm_expr_destroy(e);
        return -1;
    }
    *intn = yasm_intnum_copy(local);
    yasm_expr_destroy(e);
    return 0;
}

int
yasm_dir_helper_string(void *obj, yasm_valparam *vp, unsigned long line,
                       void *data, uintptr_t arg)
{
    /*@dependent@*/ /*@null@*/ const char *local;
    char **s = (char **)data;

    if (*s)
        yasm_xfree(*s);
    if (!(local = yasm_vp_string(vp))) {
        yasm_error_set(YASM_ERROR_VALUE,
                       N_("argument to `%s' is not a string or identifier"),
                       vp->val);
        return -1;
    }
    *s = yasm__xstrdup(local);
    return 0;
}

int
yasm_dir_helper_valparam_warn(void *obj, yasm_valparam *vp,
                              unsigned long line, void *data)
{
    const char *s;

    if (vp->val) {
        yasm_warn_set(YASM_WARN_GENERAL, N_("Unrecognized qualifier `%s'"),
                      vp->val);
        return 0;
    }

    if ((s = yasm_vp_id(vp)))
        yasm_warn_set(YASM_WARN_GENERAL, N_("Unrecognized qualifier `%s'"), s);
    else if (vp->type == YASM_PARAM_STRING)
        yasm_warn_set(YASM_WARN_GENERAL, N_("Unrecognized string qualifier"));
    else
        yasm_warn_set(YASM_WARN_GENERAL, N_("Unrecognized numeric qualifier"));

    return 0;
}
