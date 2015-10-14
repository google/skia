/*
 * File helper functions.
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
#include <util.h>

/* Need either unistd.h or direct.h to prototype getcwd() and mkdir() */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif

#ifdef _WIN32
#include <io.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <ctype.h>
#include <errno.h>

#include "errwarn.h"
#include "file.h"

#define BSIZE   8192        /* Fill block size */


void
yasm_scanner_initialize(yasm_scanner *s)
{
    s->bot = NULL;
    s->tok = NULL;
    s->ptr = NULL;
    s->cur = NULL;
    s->lim = NULL;
    s->top = NULL;
    s->eof = NULL;
}

void
yasm_scanner_delete(yasm_scanner *s)
{
    if (s->bot) {
        yasm_xfree(s->bot);
        s->bot = NULL;
    }
}

int
yasm_fill_helper(yasm_scanner *s, unsigned char **cursor,
                 size_t (*input_func) (void *d, unsigned char *buf,
                                       size_t max),
                 void *input_func_data)
{
    size_t cnt;
    int first = 0;

    if (s->eof)
        return 0;

    cnt = s->tok - s->bot;
    if (cnt > 0) {
        memmove(s->bot, s->tok, (size_t)(s->lim - s->tok));
        s->tok = s->bot;
        s->ptr -= cnt;
        *cursor -= cnt;
        s->lim -= cnt;
    }
    if (!s->bot)
        first = 1;
    if ((s->top - s->lim) < BSIZE) {
        unsigned char *buf = yasm_xmalloc((size_t)(s->lim - s->bot) + BSIZE);
        memcpy(buf, s->tok, (size_t)(s->lim - s->tok));
        s->tok = buf;
        s->ptr = &buf[s->ptr - s->bot];
        *cursor = &buf[*cursor - s->bot];
        s->lim = &buf[s->lim - s->bot];
        s->top = &s->lim[BSIZE];
        if (s->bot)
            yasm_xfree(s->bot);
        s->bot = buf;
    }
    if ((cnt = input_func(input_func_data, s->lim, BSIZE)) == 0) {
        s->eof = &s->lim[cnt];
        *s->eof++ = '\n';
    }
    s->lim += cnt;
    return first;
}

void
yasm_unescape_cstring(unsigned char *str, size_t *len)
{
    unsigned char *s = str;
    unsigned char *o = str;
    unsigned char t[4];

    while ((size_t)(s-str)<*len) {
        if (*s == '\\' && (size_t)(&s[1]-str)<*len) {
            s++;
            switch (*s) {
                case 'b': *o = '\b'; s++; break;
                case 'f': *o = '\f'; s++; break;
                case 'n': *o = '\n'; s++; break;
                case 'r': *o = '\r'; s++; break;
                case 't': *o = '\t'; s++; break;
                case 'x':
                    /* hex escape; grab last two digits */
                    s++;
                    while ((size_t)(&s[2]-str)<*len && isxdigit(s[0])
                           && isxdigit(s[1]) && isxdigit(s[2]))
                        s++;
                    if ((size_t)(s-str)<*len && isxdigit(*s)) {
                        t[0] = *s++;
                        t[1] = '\0';
                        t[2] = '\0';
                        if ((size_t)(s-str)<*len && isxdigit(*s))
                            t[1] = *s++;
                        *o = (unsigned char)strtoul((char *)t, NULL, 16);
                    } else
                        *o = '\0';
                    break;
                default:
                    if (isdigit(*s)) {
                        int warn = 0;
                        /* octal escape */
                        if (*s > '7')
                            warn = 1;
                        *o = *s++ - '0';
                        if ((size_t)(s-str)<*len && isdigit(*s)) {
                            if (*s > '7')
                                warn = 1;
                            *o <<= 3;
                            *o += *s++ - '0';
                            if ((size_t)(s-str)<*len && isdigit(*s)) {
                                if (*s > '7')
                                    warn = 1;
                                *o <<= 3;
                                *o += *s++ - '0';
                            }
                        }
                        if (warn)
                            yasm_warn_set(YASM_WARN_GENERAL,
                                          N_("octal value out of range"));
                    } else
                        *o = *s++;
                    break;
            }
            o++;
        } else
            *o++ = *s++;
    }
    *len = o-str;
}

size_t
yasm__splitpath_unix(const char *path, /*@out@*/ const char **tail)
{
    const char *s;
    s = strrchr(path, '/');
    if (!s) {
        /* No head */
        *tail = path;
        return 0;
    }
    *tail = s+1;
    /* Strip trailing ./ on path */
    while ((s-1)>=path && *(s-1) == '.' && *s == '/'
           && !((s-2)>=path && *(s-2) == '.'))
        s -= 2;
    /* Strip trailing slashes on path (except leading) */
    while (s>path && *s == '/')
        s--;
    /* Return length of head */
    return s-path+1;
}

size_t
yasm__splitpath_win(const char *path, /*@out@*/ const char **tail)
{
    const char *basepath = path;
    const char *s;

    /* split off drive letter first, if any */
    if (isalpha(path[0]) && path[1] == ':')
        basepath += 2;

    s = basepath;
    while (*s != '\0')
        s++;
    while (s >= basepath && *s != '\\' && *s != '/')
        s--;
    if (s < basepath) {
        *tail = basepath;
        if (path == basepath)
            return 0;   /* No head */
        else
            return 2;   /* Drive letter is head */
    }
    *tail = s+1;
    /* Strip trailing .\ or ./ on path */
    while ((s-1)>=basepath && *(s-1) == '.' && (*s == '/' || *s == '\\')
           && !((s-2)>=basepath && *(s-2) == '.'))
        s -= 2;
    /* Strip trailing slashes on path (except leading) */
    while (s>basepath && (*s == '/' || *s == '\\'))
        s--;
    /* Return length of head */
    return s-path+1;
}

char *
yasm__getcwd(void)
{
    char *buf;
    size_t size;

    size = 1024;
    buf = yasm_xmalloc(size);

    if (getenv("YASM_TEST_SUITE")) {
        strcpy(buf, "./");
        return buf;
    }

    while (getcwd(buf, size-1) == NULL) {
        if (errno != ERANGE) {
            yasm__fatal(N_("could not determine current working directory"));
            yasm_xfree(buf);
            return NULL;
        }
        size *= 2;
        buf = yasm_xrealloc(buf, size);
    }

    /* append a '/' if not already present */
    size = strlen(buf);
    if (buf[size-1] != '\\' && buf[size-1] != '/') {
        buf[size] = '/';
        buf[size+1] = '\0';
    }
    return buf;
}

char *
yasm__abspath(const char *path)
{
    char *curdir, *abspath;

    curdir = yasm__getcwd();
    abspath = yasm__combpath(curdir, path);
    yasm_xfree(curdir);

    return abspath;
}

char *
yasm__combpath_unix(const char *from, const char *to)
{
    const char *tail;
    size_t pathlen, i, j;
    char *out;

    if (to[0] == '/') {
        /* absolute "to" */
        out = yasm_xmalloc(strlen(to)+1);
        /* Combine any double slashes when copying */
        for (j=0; *to; to++) {
            if (*to == '/' && *(to+1) == '/')
                continue;
            out[j++] = *to;
        }
        out[j++] = '\0';
        return out;
    }

    /* Get path component; note this strips trailing slash */
    pathlen = yasm__splitpath_unix(from, &tail);

    out = yasm_xmalloc(pathlen+strlen(to)+2);   /* worst case maximum len */

    /* Combine any double slashes when copying */
    for (i=0, j=0; i<pathlen; i++) {
        if (i<pathlen-1 && from[i] == '/' && from[i+1] == '/')
            continue;
        out[j++] = from[i];
    }
    pathlen = j;

    /* Add trailing slash back in */
    if (pathlen > 0 && out[pathlen-1] != '/')
        out[pathlen++] = '/';

    /* Now scan from left to right through "to", stripping off "." and "..";
     * if we see "..", back up one directory in out unless last directory in
     * out is also "..".
     *
     * Note this does NOT back through ..'s in the "from" path; this is just
     * as well as that could skip symlinks (e.g. "foo/bar/.." might not be
     * the same as "foo").
     */
    for (;;) {
        if (to[0] == '.' && to[1] == '/') {
            to += 2;        /* current directory */
            while (*to == '/')
                to++;           /* strip off any additional slashes */
        } else if (pathlen == 0)
            break;          /* no more "from" path left, we're done */
        else if (to[0] == '.' && to[1] == '.' && to[2] == '/') {
            if (pathlen >= 3 && out[pathlen-1] == '/' && out[pathlen-2] == '.'
                && out[pathlen-3] == '.') {
                /* can't ".." against a "..", so we're done. */
                break;
            }

            to += 3;    /* throw away "../" */
            while (*to == '/')
                to++;           /* strip off any additional slashes */

            /* and back out last directory in "out" if not already at root */
            if (pathlen > 1) {
                pathlen--;      /* strip off trailing '/' */
                while (pathlen > 0 && out[pathlen-1] != '/')
                    pathlen--;
            }
        } else
            break;
    }

    /* Copy "to" to tail of output, and we're done */
    /* Combine any double slashes when copying */
    for (j=pathlen; *to; to++) {
        if (*to == '/' && *(to+1) == '/')
            continue;
        out[j++] = *to;
    }
    out[j++] = '\0';

    return out;
}

char *
yasm__combpath_win(const char *from, const char *to)
{
    const char *tail;
    size_t pathlen, i, j;
    char *out;

    if ((isalpha(to[0]) && to[1] == ':') || (to[0] == '/' || to[0] == '\\')) {
        /* absolute or drive letter "to" */
        out = yasm_xmalloc(strlen(to)+1);
        /* Combine any double slashes when copying */
        for (j=0; *to; to++) {
            if ((*to == '/' || *to == '\\')
                && (*(to+1) == '/' || *(to+1) == '\\'))
                continue;
            if (*to == '/')
                out[j++] = '\\';
            else
                out[j++] = *to;
        }
        out[j++] = '\0';
        return out;
    }

    /* Get path component; note this strips trailing slash */
    pathlen = yasm__splitpath_win(from, &tail);

    out = yasm_xmalloc(pathlen+strlen(to)+2);   /* worst case maximum len */

    /* Combine any double slashes when copying */
    for (i=0, j=0; i<pathlen; i++) {
        if (i<pathlen-1 && (from[i] == '/' || from[i] == '\\')
            && (from[i+1] == '/' || from[i+1] == '\\'))
            continue;
        if (from[i] == '/')
            out[j++] = '\\';
        else
            out[j++] = from[i];
    }
    pathlen = j;

    /* Add trailing slash back in, unless it's only a raw drive letter */
    if (pathlen > 0 && out[pathlen-1] != '\\'
        && !(pathlen == 2 && isalpha(out[0]) && out[1] == ':'))
        out[pathlen++] = '\\';

    /* Now scan from left to right through "to", stripping off "." and "..";
     * if we see "..", back up one directory in out unless last directory in
     * out is also "..".
     *
     * Note this does NOT back through ..'s in the "from" path; this is just
     * as well as that could skip symlinks (e.g. "foo/bar/.." might not be
     * the same as "foo").
     */
    for (;;) {
        if (to[0] == '.' && (to[1] == '/' || to[1] == '\\')) {
            to += 2;        /* current directory */
            while (*to == '/' || *to == '\\')
                to++;           /* strip off any additional slashes */
        } else if (pathlen == 0
                 || (pathlen == 2 && isalpha(out[0]) && out[1] == ':'))
            break;          /* no more "from" path left, we're done */
        else if (to[0] == '.' && to[1] == '.'
                 && (to[2] == '/' || to[2] == '\\')) {
            if (pathlen >= 3 && out[pathlen-1] == '\\'
                && out[pathlen-2] == '.' && out[pathlen-3] == '.') {
                /* can't ".." against a "..", so we're done. */
                break;
            }

            to += 3;    /* throw away "../" (or "..\") */
            while (*to == '/' || *to == '\\')
                to++;           /* strip off any additional slashes */

            /* and back out last directory in "out" if not already at root */
            if (pathlen > 1) {
                pathlen--;      /* strip off trailing '/' */
                while (pathlen > 0 && out[pathlen-1] != '\\')
                    pathlen--;
            }
        } else
            break;
    }

    /* Copy "to" to tail of output, and we're done */
    /* Combine any double slashes when copying */
    for (j=pathlen; *to; to++) {
        if ((*to == '/' || *to == '\\') && (*(to+1) == '/' || *(to+1) == '\\'))
            continue;
        if (*to == '/')
            out[j++] = '\\';
        else
            out[j++] = *to;
    }
    out[j++] = '\0';

    return out;
}

size_t
yasm__createpath_common(const char *path, int win)
{
    const char *pp = path, *pe;
    char *ts, *tp;
    size_t len, lth;

    lth = len = strlen(path);
    ts = tp = (char *) malloc(len + 1);
    pe = pp + len;
    while (pe > pp) {
        if ((win && *pe == '\\') || *pe == '/')
            break;
        --pe;
        --lth;
    }

    while (pp <= pe) {
        if (pp == pe || (win && *pp == '\\') || *pp == '/') {
#ifdef _WIN32
            struct _finddata_t fi; 
            intptr_t h;
#elif defined(HAVE_SYS_STAT_H) 
            struct stat fi;
#endif
            *tp = '\0';

#ifdef _WIN32
            h = _findfirst(ts, &fi);
            if (h != -1) {
                if (fi.attrib != _A_SUBDIR) {
                    _findclose(h);
                    break;
                }
            } else if (errno == ENOENT) {
                if (_mkdir(ts) == -1) {
                    _findclose(h);
                    lth = -1;
                    break;
                }
            }
            _findclose(h);
#elif defined(HAVE_SYS_STAT_H)
            if (stat(ts, &fi) != -1) {
                if (!S_ISDIR(fi.st_mode))
                    break;
            } else if (errno == ENOENT) {
                if (mkdir(ts, 0755) == -1) {
                    lth = 0;
                    break;
                }
            }
#else
            break;
#endif
        }
        *tp++ = *pp++;
    }
    free(ts);
    return lth;
}

typedef struct incpath {
    STAILQ_ENTRY(incpath) link;
    /*@owned@*/ char *path;
} incpath;

STAILQ_HEAD(incpath_head, incpath) incpaths = STAILQ_HEAD_INITIALIZER(incpaths);

FILE *
yasm_fopen_include(const char *iname, const char *from, const char *mode,
                   char **oname)
{
    FILE *f;
    char *combine;
    incpath *np;

    /* Try directly relative to from first, then each of the include paths */
    if (from) {
        combine = yasm__combpath(from, iname);
        f = fopen(combine, mode);
        if (f) {
            if (oname)
                *oname = combine;
            else
                yasm_xfree(combine);
            return f;
        }
        yasm_xfree(combine);
    }

    STAILQ_FOREACH(np, &incpaths, link) {
        combine = yasm__combpath(np->path, iname);
        f = fopen(combine, mode);
        if (f) {
            if (oname)
                *oname = combine;
            else
                yasm_xfree(combine);
            return f;
        }
        yasm_xfree(combine);
    }

    if (oname)
        *oname = NULL;
    return NULL;
}

void
yasm_delete_include_paths(void)
{
    incpath *n1, *n2;

    n1 = STAILQ_FIRST(&incpaths);
    while (n1) {
        n2 = STAILQ_NEXT(n1, link);
        yasm_xfree(n1->path);
        yasm_xfree(n1);
        n1 = n2;
    }
    STAILQ_INIT(&incpaths);
}

const char *
yasm_get_include_dir(void **iter)
{
    incpath *p = (incpath *)*iter;

    if (!p)
        p = STAILQ_FIRST(&incpaths);
    else
        p = STAILQ_NEXT(p, link);

    *iter = p;
    if (p)
        return p->path;
    else
        return NULL;
}

void
yasm_add_include_path(const char *path)
{
    incpath *np = yasm_xmalloc(sizeof(incpath));
    size_t len = strlen(path);

    np->path = yasm_xmalloc(len+2);
    memcpy(np->path, path, len+1);
    /* Add trailing slash if it is missing */
    if (path[len-1] != '\\' && path[len-1] != '/') {
        np->path[len] = '/';
        np->path[len+1] = '\0';
    }

    STAILQ_INSERT_TAIL(&incpaths, np, link);
}

size_t
yasm_fwrite_16_l(unsigned short val, FILE *f)
{
    if (fputc(val & 0xFF, f) == EOF)
        return 0;
    if (fputc((val >> 8) & 0xFF, f) == EOF)
        return 0;
    return 1;
}

size_t
yasm_fwrite_32_l(unsigned long val, FILE *f)
{
    if (fputc((int)(val & 0xFF), f) == EOF)
        return 0;
    if (fputc((int)((val >> 8) & 0xFF), f) == EOF)
        return 0;
    if (fputc((int)((val >> 16) & 0xFF), f) == EOF)
        return 0;
    if (fputc((int)((val >> 24) & 0xFF), f) == EOF)
        return 0;
    return 1;
}

size_t
yasm_fwrite_16_b(unsigned short val, FILE *f)
{
    if (fputc((val >> 8) & 0xFF, f) == EOF)
        return 0;
    if (fputc(val & 0xFF, f) == EOF)
        return 0;
    return 1;
}

size_t
yasm_fwrite_32_b(unsigned long val, FILE *f)
{
    if (fputc((int)((val >> 24) & 0xFF), f) == EOF)
        return 0;
    if (fputc((int)((val >> 16) & 0xFF), f) == EOF)
        return 0;
    if (fputc((int)((val >> 8) & 0xFF), f) == EOF)
        return 0;
    if (fputc((int)(val & 0xFF), f) == EOF)
        return 0;
    return 1;
}
