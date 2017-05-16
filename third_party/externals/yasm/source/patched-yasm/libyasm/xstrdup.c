/*
 * strdup() implementation with error checking (using xmalloc).
 *
 * Copyright (c) 1988, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "util.h"

#include "coretype.h"

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strdup.c    8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */


#ifdef WITH_DMALLOC
#undef yasm__xstrdup
#endif

char *
yasm__xstrdup(const char *str)
{
        size_t len;
        char *copy;

        len = strlen(str) + 1;
        copy = yasm_xmalloc(len);
        memcpy(copy, str, len);
        return (copy);
}

char *
yasm__xstrndup(const char *str, size_t max)
{
        size_t len = 0;
        char *copy;

        while (len < max && str[len] != '\0')
            len++;
        copy = yasm_xmalloc(len+1);
        memcpy(copy, str, len);
        copy[len] = '\0';
        return (copy);
}
