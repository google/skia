/*
 * strcasecmp() implementation for systems that don't have it or stricmp()
 * or strcmpi().
 *
 * Copyright (c) 1987, 1993
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

#ifndef USE_OUR_OWN_STRCASECMP
#undef yasm__strcasecmp
#undef yasm__strncasecmp
#endif

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)strcasecmp.c        8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <ctype.h>

int
yasm__strcasecmp(const char *s1, const char *s2)
{
#ifdef HAVE_STRCASECMP
    return strcasecmp(s1, s2);
#elif HAVE_STRICMP
    return stricmp(s1, s2);
#elif HAVE__STRICMP
    return _stricmp(s1, s2);
#elif HAVE_STRCMPI
    return strcmpi(s1, s2);
#else
        const unsigned char
                        *us1 = (const unsigned char *)s1,
                        *us2 = (const unsigned char *)s2;

        while (tolower(*us1) == tolower(*us2++))
                if (*us1++ == '\0')
                        return (0);
        return (tolower(*us1) - tolower(*--us2));
#endif
}

int
yasm__strncasecmp(const char *s1, const char *s2, size_t n)
{
#ifdef HAVE_STRCASECMP
    return strncasecmp(s1, s2, n);
#elif HAVE_STRICMP
    return strnicmp(s1, s2, n);
#elif HAVE__STRNICMP
    return _strnicmp(s1, s2, n);
#elif HAVE_STRCMPI
    return strncmpi(s1, s2, n);
#else
        const unsigned char
                        *us1 = (const unsigned char *)s1,
                        *us2 = (const unsigned char *)s2;

        if (n != 0) {
                do {
                        if (tolower(*us1) != tolower(*us2++))
                                return (tolower(*us1) - tolower(*--us2));
                        if (*us1++ == '\0')
                                break;
                } while (--n != 0);
        }
        return (0);
#endif
}
