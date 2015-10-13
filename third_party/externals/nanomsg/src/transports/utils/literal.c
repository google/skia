/*
    Copyright (c) 2012-2013 250bpm s.r.o.  All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "literal.h"

#include "../../utils/err.h"
#include "../../utils/fast.h"

#include <string.h>

#ifndef NN_HAVE_WINDOWS
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

/*  On Windows XS there's no inet_pton() function. */
#if defined NN_HAVE_WINDOWS && ((_WIN32_WINNT <= 0x0501) || (WINVER <= 0x0501))

static int nn_inet_pton(int family, const char *src, void *dst)
{
    int rc;
    struct sockaddr_storage addr;
    int addr_len = sizeof(addr);

    if (nn_slow (family != AF_INET && family != AF_INET6)) {
        errno = EAFNOSUPPORT;
        return -1;
    }

    addr.ss_family = family;
    rc = WSAStringToAddressA ((char*) src, family, NULL,
        (struct sockaddr*) &addr, &addr_len);
    if (rc != 0)
        return 0;

    if (family == AF_INET) {
        memcpy(dst, &((struct sockaddr_in *) &addr)->sin_addr,
            sizeof(struct in_addr));
    } else if (family == AF_INET6) {
        memcpy(dst, &((struct sockaddr_in6 *)&addr)->sin6_addr,
            sizeof(struct in6_addr));
    }

    return 1;
}

#else

static int nn_inet_pton(int family, const char *src, void *dst)
{
    return inet_pton (family, src, dst);
}

#endif

int nn_literal_resolve (const char *addr, size_t addrlen,
    int ipv4only, struct sockaddr_storage *result, size_t *resultlen)
{
    int rc;
    char addrz [INET6_ADDRSTRLEN > INET_ADDRSTRLEN ?
        INET6_ADDRSTRLEN :  INET_ADDRSTRLEN];
    struct in_addr inaddr;
    struct in6_addr in6addr;

    /*  Try to treat the address as a literal string. If the size of
        the address is larger than longest possible literal, skip the step.
        If the literal in enclosed in square brackets ignore them. */
    if (addrlen > 0 && addr [0] == '[') {
        if (addr [addrlen - 1] != ']')
            return -EINVAL;
        if (addrlen - 2 + 1 > sizeof (addrz))
            return -EINVAL;
        memcpy (addrz, addr + 1, addrlen - 2);
        addrz [addrlen - 2] = 0;
    }
    else {
        if (addrlen + 1 > sizeof (addrz))
            return -EINVAL;
        memcpy (addrz, addr, addrlen);
        addrz [addrlen] = 0;
    }

    /*  Try to interpret the literal as an IPv6 address. */
    if (!ipv4only) {
        rc = nn_inet_pton (AF_INET6, addrz, &in6addr);
        if (rc == 1) {
            if (result) {
                result->ss_family = AF_INET6;
                ((struct sockaddr_in6*) result)->sin6_addr = in6addr;
             }
             if (resultlen)
                *resultlen = sizeof (struct sockaddr_in6);
            return 0;
        }
        errno_assert (rc == 0);
    }

    /*  Try to interpret the literal as an IPv4 address. */
    rc = nn_inet_pton (AF_INET, addrz, &inaddr);
    if (rc == 1) {
        if (result) {
           result->ss_family = AF_INET;
           ((struct sockaddr_in*) result)->sin_addr = inaddr;
        }
        if (resultlen)
            *resultlen = sizeof (struct sockaddr_in);
        return 0;
    }
    errno_assert (rc == 0);

    /*  The supplied string is not a valid literal address. */
    return -EINVAL;
}

