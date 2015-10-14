/*
 * Error and warning reporting and related functions.
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

#include <ctype.h>
#include <stdarg.h>

#include "coretype.h"

#include "linemap.h"
#include "errwarn.h"


#define MSG_MAXSIZE     1024

#if !defined(HAVE_TOASCII) || defined(lint)
# define toascii(c) ((c) & 0x7F)
#endif

/* Default handlers for replacable functions */
static /*@exits@*/ void def_internal_error_
    (const char *file, unsigned int line, const char *message);
static /*@exits@*/ void def_fatal(const char *message, va_list va);
static const char *def_gettext_hook(const char *msgid);

/* Storage for errwarn's "extern" functions */
/*@exits@*/ void (*yasm_internal_error_)
    (const char *file, unsigned int line, const char *message)
    = def_internal_error_;
/*@exits@*/ void (*yasm_fatal) (const char *message, va_list va) = def_fatal;
const char * (*yasm_gettext_hook) (const char *msgid) = def_gettext_hook;

/* Error indicator */
/* yasm_eclass is not static so that yasm_error_occurred macro can access it */
yasm_error_class yasm_eclass;
static /*@only@*/ /*@null@*/ char *yasm_estr;
static unsigned long yasm_exrefline;
static /*@only@*/ /*@null@*/ char *yasm_exrefstr;

/* Warning indicator */
typedef struct warn {
    /*@reldef@*/ STAILQ_ENTRY(warn) link;

    yasm_warn_class wclass;
    /*@owned@*/ /*@null@*/ char *wstr;
} warn;
static STAILQ_HEAD(warn_head, warn) yasm_warns;

/* Enabled warnings.  See errwarn.h for a list. */
static unsigned long warn_class_enabled;

typedef struct errwarn_data {
    /*@reldef@*/ SLIST_ENTRY(errwarn_data) link;

    enum { WE_UNKNOWN, WE_ERROR, WE_WARNING, WE_PARSERERROR } type;

    unsigned long line;
    unsigned long xrefline;
    /*@owned@*/ char *msg;
    /*@owned@*/ char *xrefmsg;
} errwarn_data;

struct yasm_errwarns {
    /*@reldef@*/ SLIST_HEAD(errwarn_head, errwarn_data) errwarns;

    /* Total error count */
    unsigned int ecount;

    /* Total warning count */
    unsigned int wcount;

    /* Last inserted error/warning.  Used to speed up insertions. */
    /*@null@*/ errwarn_data *previous_we;
};

/* Static buffer for use by conv_unprint(). */
static char unprint[5];


static const char *
def_gettext_hook(const char *msgid)
{
    return msgid;
}

void
yasm_errwarn_initialize(void)
{
    /* Default enabled warnings.  See errwarn.h for a list. */
    warn_class_enabled = 
        (1UL<<YASM_WARN_GENERAL) | (1UL<<YASM_WARN_UNREC_CHAR) |
        (1UL<<YASM_WARN_PREPROC) | (0UL<<YASM_WARN_ORPHAN_LABEL) |
        (1UL<<YASM_WARN_UNINIT_CONTENTS) | (0UL<<YASM_WARN_SIZE_OVERRIDE) |
        (1UL<<YASM_WARN_IMPLICIT_SIZE_OVERRIDE);

    yasm_eclass = YASM_ERROR_NONE;
    yasm_estr = NULL;
    yasm_exrefline = 0;
    yasm_exrefstr = NULL;

    STAILQ_INIT(&yasm_warns);
}

void
yasm_errwarn_cleanup(void)
{
    yasm_error_clear();
    yasm_warn_clear();
}

/* Convert a possibly unprintable character into a printable string, using
 * standard cat(1) convention for unprintable characters.
 */
char *
yasm__conv_unprint(int ch)
{
    int pos = 0;

    if (((ch & ~0x7F) != 0) /*!isascii(ch)*/ && !isprint(ch)) {
        unprint[pos++] = 'M';
        unprint[pos++] = '-';
        ch &= toascii(ch);
    }
    if (iscntrl(ch)) {
        unprint[pos++] = '^';
        unprint[pos++] = (ch == '\177') ? '?' : ch | 0100;
    } else
        unprint[pos++] = ch;
    unprint[pos] = '\0';

    return unprint;
}

/* Report an internal error.  Essentially a fatal error with trace info.
 * Exit immediately because it's essentially an assert() trap.
 */
static void
def_internal_error_(const char *file, unsigned int line, const char *message)
{
    fprintf(stderr,
            yasm_gettext_hook(N_("INTERNAL ERROR at %s, line %u: %s\n")),
            file, line, yasm_gettext_hook(message));
#ifdef HAVE_ABORT
    abort();
#else
    exit(EXIT_FAILURE);
#endif
}

/* Report a fatal error.  These are unrecoverable (such as running out of
 * memory), so just exit immediately.
 */
static void
def_fatal(const char *fmt, va_list va)
{
    fprintf(stderr, "%s: ", yasm_gettext_hook(N_("FATAL")));
    vfprintf(stderr, yasm_gettext_hook(fmt), va);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

/* Create an errwarn structure in the correct linked list location.
 * If replace_parser_error is nonzero, overwrites the last error if its
 * type is WE_PARSERERROR.
 */
static errwarn_data *
errwarn_data_new(yasm_errwarns *errwarns, unsigned long line,
                 int replace_parser_error)
{
    errwarn_data *first, *next, *ins_we, *we;
    enum { INS_NONE, INS_HEAD, INS_AFTER } action = INS_NONE;

    /* Find the entry with either line=line or the last one with line<line.
     * Start with the last entry added to speed the search.
     */
    ins_we = errwarns->previous_we;
    first = SLIST_FIRST(&errwarns->errwarns);
    if (!ins_we || !first)
        action = INS_HEAD;
    while (action == INS_NONE) {
        next = SLIST_NEXT(ins_we, link);
        if (line < ins_we->line) {
            if (ins_we == first)
                action = INS_HEAD;
            else
                ins_we = first;
        } else if (!next)
            action = INS_AFTER;
        else if (line >= ins_we->line && line < next->line)
            action = INS_AFTER;
        else
            ins_we = next;
    }

    if (replace_parser_error && ins_we && ins_we->type == WE_PARSERERROR) {
        /* overwrite last error */      
        we = ins_we;
    } else {
        /* add a new error */
        we = yasm_xmalloc(sizeof(errwarn_data));

        we->type = WE_UNKNOWN;
        we->line = line;
        we->xrefline = 0;
        we->msg = NULL;
        we->xrefmsg = NULL;

        if (action == INS_HEAD)
            SLIST_INSERT_HEAD(&errwarns->errwarns, we, link);
        else if (action == INS_AFTER) {
            assert(ins_we != NULL);
            SLIST_INSERT_AFTER(ins_we, we, link);
        } else
            yasm_internal_error(N_("Unexpected errwarn insert action"));
    }

    /* Remember previous err/warn */
    errwarns->previous_we = we;

    return we;
}

void
yasm_error_clear(void)
{
    if (yasm_estr)
        yasm_xfree(yasm_estr);
    if (yasm_exrefstr)
        yasm_xfree(yasm_exrefstr);
    yasm_eclass = YASM_ERROR_NONE;
    yasm_estr = NULL;
    yasm_exrefline = 0;
    yasm_exrefstr = NULL;
}

int
yasm_error_matches(yasm_error_class eclass)
{
    if (yasm_eclass == YASM_ERROR_NONE)
        return eclass == YASM_ERROR_NONE;
    if (yasm_eclass == YASM_ERROR_GENERAL)
        return eclass == YASM_ERROR_GENERAL;
    return (yasm_eclass & eclass) == eclass;
}

void
yasm_error_set_va(yasm_error_class eclass, const char *format, va_list va)
{
    if (yasm_eclass != YASM_ERROR_NONE)
        return;

    yasm_eclass = eclass;
    yasm_estr = yasm_xmalloc(MSG_MAXSIZE+1);
#ifdef HAVE_VSNPRINTF
    vsnprintf(yasm_estr, MSG_MAXSIZE, yasm_gettext_hook(format), va);
#else
    vsprintf(yasm_estr, yasm_gettext_hook(format), va);
#endif
}

void
yasm_error_set(yasm_error_class eclass, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    yasm_error_set_va(eclass, format, va);
    va_end(va);
}

void
yasm_error_set_xref_va(unsigned long xrefline, const char *format, va_list va)
{
    if (yasm_eclass != YASM_ERROR_NONE)
        return;

    yasm_exrefline = xrefline;

    yasm_exrefstr = yasm_xmalloc(MSG_MAXSIZE+1);
#ifdef HAVE_VSNPRINTF
    vsnprintf(yasm_exrefstr, MSG_MAXSIZE, yasm_gettext_hook(format), va);
#else
    vsprintf(yasm_exrefstr, yasm_gettext_hook(format), va);
#endif
}

void
yasm_error_set_xref(unsigned long xrefline, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    yasm_error_set_xref_va(xrefline, format, va);
    va_end(va);
}

void
yasm_error_fetch(yasm_error_class *eclass, char **str, unsigned long *xrefline,
                 char **xrefstr)
{
    *eclass = yasm_eclass;
    *str = yasm_estr;
    *xrefline = yasm_exrefline;
    *xrefstr = yasm_exrefstr;
    yasm_eclass = YASM_ERROR_NONE;
    yasm_estr = NULL;
    yasm_exrefline = 0;
    yasm_exrefstr = NULL;
}

void yasm_warn_clear(void)
{
    /* Delete all error/warnings */
    while (!STAILQ_EMPTY(&yasm_warns)) {
        warn *w = STAILQ_FIRST(&yasm_warns);

        if (w->wstr)
            yasm_xfree(w->wstr);

        STAILQ_REMOVE_HEAD(&yasm_warns, link);
        yasm_xfree(w);
    }
}

yasm_warn_class
yasm_warn_occurred(void)
{
    if (STAILQ_EMPTY(&yasm_warns))
        return YASM_WARN_NONE;
    return STAILQ_FIRST(&yasm_warns)->wclass;
}

void
yasm_warn_set_va(yasm_warn_class wclass, const char *format, va_list va)
{
    warn *w;

    if (!(warn_class_enabled & (1UL<<wclass)))
        return;     /* warning is part of disabled class */

    w = yasm_xmalloc(sizeof(warn));
    w->wclass = wclass;
    w->wstr = yasm_xmalloc(MSG_MAXSIZE+1);
#ifdef HAVE_VSNPRINTF
    vsnprintf(w->wstr, MSG_MAXSIZE, yasm_gettext_hook(format), va);
#else
    vsprintf(w->wstr, yasm_gettext_hook(format), va);
#endif
    STAILQ_INSERT_TAIL(&yasm_warns, w, link);
}

void
yasm_warn_set(yasm_warn_class wclass, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    yasm_warn_set_va(wclass, format, va);
    va_end(va);
}

void
yasm_warn_fetch(yasm_warn_class *wclass, char **str)
{
    warn *w = STAILQ_FIRST(&yasm_warns);

    if (!w) {
        *wclass = YASM_WARN_NONE;
        *str = NULL;
        return;
    }

    *wclass = w->wclass;
    *str = w->wstr;

    STAILQ_REMOVE_HEAD(&yasm_warns, link);
    yasm_xfree(w);
}

void
yasm_warn_enable(yasm_warn_class num)
{
    warn_class_enabled |= (1UL<<num);
}

void
yasm_warn_disable(yasm_warn_class num)
{
    warn_class_enabled &= ~(1UL<<num);
}

void
yasm_warn_disable_all(void)
{
    warn_class_enabled = 0;
}

yasm_errwarns *
yasm_errwarns_create(void)
{
    yasm_errwarns *errwarns = yasm_xmalloc(sizeof(yasm_errwarns));
    SLIST_INIT(&errwarns->errwarns);
    errwarns->ecount = 0;
    errwarns->wcount = 0;
    errwarns->previous_we = NULL;
    return errwarns;
}

void
yasm_errwarns_destroy(yasm_errwarns *errwarns)
{
    errwarn_data *we;

    /* Delete all error/warnings */
    while (!SLIST_EMPTY(&errwarns->errwarns)) {
        we = SLIST_FIRST(&errwarns->errwarns);
        if (we->msg)
            yasm_xfree(we->msg);
        if (we->xrefmsg)
            yasm_xfree(we->xrefmsg);

        SLIST_REMOVE_HEAD(&errwarns->errwarns, link);
        yasm_xfree(we);
    }

    yasm_xfree(errwarns);
}

void
yasm_errwarn_propagate(yasm_errwarns *errwarns, unsigned long line)
{
    if (yasm_eclass != YASM_ERROR_NONE) {
        errwarn_data *we = errwarn_data_new(errwarns, line, 1);
        yasm_error_class eclass;

        yasm_error_fetch(&eclass, &we->msg, &we->xrefline, &we->xrefmsg);
        if (eclass != YASM_ERROR_GENERAL
            && (eclass & YASM_ERROR_PARSE) == YASM_ERROR_PARSE)
            we->type = WE_PARSERERROR;
        else
            we->type = WE_ERROR;
        errwarns->ecount++;
    }

    while (!STAILQ_EMPTY(&yasm_warns)) {
        errwarn_data *we = errwarn_data_new(errwarns, line, 0);
        yasm_warn_class wclass;

        yasm_warn_fetch(&wclass, &we->msg);
        we->type = WE_WARNING;
        errwarns->wcount++;
    }
}

unsigned int
yasm_errwarns_num_errors(yasm_errwarns *errwarns, int warning_as_error)
{
    if (warning_as_error)
        return errwarns->ecount+errwarns->wcount;
    else
        return errwarns->ecount;
}

void
yasm_errwarns_output_all(yasm_errwarns *errwarns, yasm_linemap *lm,
                         int warning_as_error,
                         yasm_print_error_func print_error,
                         yasm_print_warning_func print_warning)
{
    errwarn_data *we;
    const char *filename, *xref_filename;
    unsigned long line, xref_line;

    /* If we're treating warnings as errors, tell the user about it. */
    if (warning_as_error && warning_as_error != 2) {
        print_error("", 0,
                    yasm_gettext_hook(N_("warnings being treated as errors")),
                    NULL, 0, NULL);
        warning_as_error = 2;
    }

    /* Output error/warnings. */
    SLIST_FOREACH(we, &errwarns->errwarns, link) {
        /* Output error/warning */
        yasm_linemap_lookup(lm, we->line, &filename, &line);
        if (we->xrefline)
            yasm_linemap_lookup(lm, we->xrefline, &xref_filename, &xref_line);
        else {
            xref_filename = NULL;
            xref_line = 0;
        }
        if (we->type == WE_ERROR || we->type == WE_PARSERERROR)
            print_error(filename, line, we->msg, xref_filename, xref_line,
                        we->xrefmsg);
        else
            print_warning(filename, line, we->msg);
    }
}

void
yasm__fatal(const char *message, ...)
{
    va_list va;
    va_start(va, message);
    yasm_fatal(message, va);
    /*@notreached@*/
    va_end(va);
}
