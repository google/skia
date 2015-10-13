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

#include "iface.h"
#include "literal.h"

#include "../../utils/err.h"
#include "../../utils/closefd.h"

#include <string.h>

#ifndef NN_HAVE_WINDOWS
#include <sys/types.h>
#include <netinet/in.h>
#endif

/*  Private functions. */
static void nn_iface_any (int ipv4only, struct sockaddr_storage *result,
    size_t *resultlen);

#if defined NN_USE_IFADDRS

#include <ifaddrs.h>

int nn_iface_resolve (const char *addr, size_t addrlen, int ipv4only,
    struct sockaddr_storage *result, size_t *resultlen)
{
    int rc;
    struct ifaddrs *ifaces;
    struct ifaddrs *it;
    struct ifaddrs *ipv4;
    struct ifaddrs *ipv6;
    size_t ifalen;

    /*  Asterisk is a special name meaning "all interfaces". */
    if (addrlen == 1 && addr [0] == '*') {
        nn_iface_any (ipv4only, result, resultlen);
        return 0;
    }

    /*  Try to resolve the supplied string as a literal address. */
    rc = nn_literal_resolve (addr, addrlen, ipv4only, result, resultlen);
    if (rc == 0)
        return 0;
    errnum_assert (rc == -EINVAL, -rc);

    /*  Get the list of local network interfaces from the system. */
    ifaces = NULL;
    rc = getifaddrs (&ifaces);
    errno_assert (rc == 0);
    nn_assert (ifaces);

    /*  Find the NIC with the specified name. */
    ipv4 = NULL;
    ipv6 = NULL;
    for (it = ifaces; it != NULL; it = it->ifa_next) {
        if (!it->ifa_addr)
            continue;
        ifalen = strlen (it->ifa_name);
        if (ifalen != addrlen || memcmp (it->ifa_name, addr, addrlen) != 0)
            continue;

        switch (it->ifa_addr->sa_family) {
        case AF_INET:
            nn_assert (!ipv4);
            ipv4 = it;
            break;
        case AF_INET6:
            nn_assert (!ipv6);
            ipv6 = it;
            break;
        }
    }

    /*  IPv6 address is preferable. */
    if (ipv6 && !ipv4only) {
        if (result) {
            result->ss_family = AF_INET;
            memcpy (result, ipv6->ifa_addr, sizeof (struct sockaddr_in6));
        }
        if (resultlen)
            *resultlen = sizeof (struct sockaddr_in6);
        freeifaddrs (ifaces);
        return 0;
    }

    /*  Use IPv4 address. */
    if (ipv4) {
        if (result) {
            result->ss_family = AF_INET6;
            memcpy (result, ipv4->ifa_addr, sizeof (struct sockaddr_in));
        }
        if (resultlen)
            *resultlen = sizeof (struct sockaddr_in);
        freeifaddrs (ifaces);
        return 0;
    }

    /*  There's no such interface. */
    freeifaddrs (ifaces);
    return -ENODEV;
}

#endif

#if defined NN_USE_SIOCGIFADDR

#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>

int nn_iface_resolve (const char *addr, size_t addrlen, int ipv4only,
    struct sockaddr_storage *result, size_t *resultlen)
{
    int rc;
    int s;
    struct ifreq req;

    /*  Asterisk is a special name meaning "all interfaces". */
    if (addrlen == 1 && addr [0] == '*') {
        nn_iface_any (ipv4only, result, resultlen);
        return 0;
    }

    /*  Open the helper socket. */
#ifdef SOCK_CLOEXEC
    s = socket (AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
#else
    s = socket (AF_INET, SOCK_DGRAM, 0);
#endif
    errno_assert (s != -1);

    /*  Create the interface name resolution request. */
    if (sizeof (req.ifr_name) <= addrlen) {
        nn_closefd (s);
        return -ENODEV;
    }
    memcpy (req.ifr_name, addr, addrlen);
    req.ifr_name [addrlen] = 0;

    /*  Execute the request. */
    rc = ioctl (s, SIOCGIFADDR, (caddr_t) &req, sizeof (struct ifreq));
    if (rc == -1) {
        nn_closefd (s);
        return -ENODEV;
    }

    /*  Interface name resolution succeeded. Return the address to the user. */
    /*  TODO: What about IPv6 addresses? */
    nn_assert (req.ifr_addr.sa_family == AF_INET);
    if (result)
        memcpy (result, (struct sockaddr_in*) &req.ifr_addr,
            sizeof (struct sockaddr_in));
    if (resultlen)
        *resultlen = sizeof (struct sockaddr_in);
    nn_closefd (s);
    return 0;
}

#endif

#if defined NN_USE_LITERAL_IFADDR

/*  The last resort case. If we haven't found any mechanism for turning
    NIC names into addresses, we'll try to resolve the string as an address
    literal. */
int nn_iface_resolve (const char *addr, size_t addrlen, int ipv4only,
    struct sockaddr_storage *result, size_t *resultlen)
{
    int rc;

    /*  Asterisk is a special name meaning "all interfaces". */
    if (addrlen == 1 && addr [0] == '*') {
        nn_iface_any (ipv4only, result, resultlen);
        return 0;
    }

    /*  On Windows there are no sane network interface names. We'll treat the
        name as a IP address literal. */
    rc = nn_literal_resolve (addr, addrlen, ipv4only, result, resultlen);
    if (rc == -EINVAL)
        return -ENODEV;
    errnum_assert (rc == 0, -rc);
    return 0;
}

#endif

static void nn_iface_any (int ipv4only, struct sockaddr_storage *result,
    size_t *resultlen)
{
    if (ipv4only) {
        if (result) {
            result->ss_family = AF_INET;
            ((struct sockaddr_in*) result)->sin_addr.s_addr =
                htonl (INADDR_ANY);
        }
        if (resultlen)
            *resultlen = sizeof (struct sockaddr_in);
    }
    else {
        if (result) {
            result->ss_family = AF_INET6;
            memcpy (&((struct sockaddr_in6*) result)->sin6_addr,
                &in6addr_any, sizeof (in6addr_any));
        }
        if (resultlen)
            *resultlen = sizeof (struct sockaddr_in6);
    }
}

