/*
    Copyright (c) 2013 250bpm s.r.o.  All rights reserved.

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

#include "../nn.h"

#include "../utils/err.h"
#include "../utils/fast.h"
#include "../utils/fd.h"
#include "../utils/attr.h"

#include <string.h>

#if defined NN_HAVE_WINDOWS
#include "../utils/win.h"
#elif defined NN_HAVE_POLL
#include <poll.h>
#else
#error
#endif

/*  Private functions. */
static int nn_device_loopback (int s);
static int nn_device_twoway (int s1, nn_fd s1rcv, nn_fd s1snd, int s2, nn_fd s2rcv, nn_fd s2snd);
static int nn_device_oneway (int s1, nn_fd s1rcv, int s2, nn_fd s2snd);
static int nn_device_mvmsg (int from, int to, int flags);

int nn_device (int s1, int s2)
{
    int rc;
    int op1;
    int op2;
    nn_fd s1rcv;
    nn_fd s1snd;
    nn_fd s2rcv;
    nn_fd s2snd;
    size_t opsz;

    /*  At least one socket must be specified. */
    if (s1 < 0 && s2 < 0) {
        errno = EBADF;
        return -1;
    }

    /*  Handle the case when there's only one socket in the device. */
    if (s2 < 0)
        return nn_device_loopback (s1);
    if (s1 < 0)
        return nn_device_loopback (s2);

    /*  Check whether both sockets are "raw" sockets. */
    opsz = sizeof (op1);
    rc = nn_getsockopt (s1, NN_SOL_SOCKET, NN_DOMAIN, &op1, &opsz);
    errno_assert (rc == 0);
    nn_assert (opsz == sizeof (op1));
    opsz = sizeof (op2);
    rc = nn_getsockopt (s2, NN_SOL_SOCKET, NN_DOMAIN, &op2, &opsz);
    errno_assert (rc == 0);
    nn_assert (opsz == sizeof (op2));
    if (op1 != AF_SP_RAW || op2 != AF_SP_RAW) {
        errno = EINVAL;
        return -1;
    }

    /*  Check whether both sockets are from the same protocol. */
    opsz = sizeof (op1);
    rc = nn_getsockopt (s1, NN_SOL_SOCKET, NN_PROTOCOL, &op1, &opsz);
    errno_assert (rc == 0);
    nn_assert (opsz == sizeof (op1));
    opsz = sizeof (op2);
    rc = nn_getsockopt (s2, NN_SOL_SOCKET, NN_PROTOCOL, &op2, &opsz);
    errno_assert (rc == 0);
    nn_assert (opsz == sizeof (op2));
    if (op1 / 16 != op2 / 16) {
        errno = EINVAL;
        return -1;
    }

    /*  Get the file descriptors for polling. */
    opsz = sizeof (s1rcv);
    rc = nn_getsockopt (s1, NN_SOL_SOCKET, NN_RCVFD, &s1rcv, &opsz);
    if (rc < 0 && nn_errno () == ENOPROTOOPT)
        s1rcv = -1;
    else {
        nn_assert (rc == 0);
        nn_assert (opsz == sizeof (s1rcv));
        nn_assert (s1rcv >= 0);
    }
    opsz = sizeof (s1snd);
    rc = nn_getsockopt (s1, NN_SOL_SOCKET, NN_SNDFD, &s1snd, &opsz);
    if (rc < 0 && nn_errno () == ENOPROTOOPT)
        s1snd = -1;
    else {
        nn_assert (rc == 0);
        nn_assert (opsz == sizeof (s1snd));
        nn_assert (s1snd >= 0);
    }
    opsz = sizeof (s2rcv);
    rc = nn_getsockopt (s2, NN_SOL_SOCKET, NN_RCVFD, &s2rcv, &opsz);
    if (rc < 0 && nn_errno () == ENOPROTOOPT)
        s2rcv = -1;
    else {
        nn_assert (rc == 0);
        nn_assert (opsz == sizeof (s2rcv));
        nn_assert (s2rcv >= 0);
    }
    opsz = sizeof (s2snd);
    rc = nn_getsockopt (s2, NN_SOL_SOCKET, NN_SNDFD, &s2snd, &opsz);
    if (rc < 0 && nn_errno () == ENOPROTOOPT)
        s2snd = -1;
    else {
        nn_assert (rc == 0);
        nn_assert (opsz == sizeof (s2snd));
        nn_assert (s2snd >= 0);
    }

    /*  Check the directionality of the sockets. */
    if (s1rcv != -1 && s2snd == -1) {
        errno = EINVAL;
        return -1;
    }
    if (s1snd != -1 && s2rcv == -1) {
        errno = EINVAL;
        return -1;
    }
    if (s2rcv != -1 && s1snd == -1) {
        errno = EINVAL;
        return -1;
    }
    if (s2snd != -1 && s1rcv == -1) {
        errno = EINVAL;
        return -1;
    }

    /*  Two-directional device. */
    if (s1rcv != -1 && s1snd != -1 && s2rcv != -1 && s2snd != -1)
        return nn_device_twoway (s1, s1rcv, s1snd, s2, s2rcv, s2snd);

    /*  Single-directional device passing messages from s1 to s2. */
    if (s1rcv != -1 && s1snd == -1 && s2rcv == -1 && s2snd != -1)
        return nn_device_oneway (s1, s1rcv, s2, s2snd);

    /*  Single-directional device passing messages from s2 to s1. */
    if (s1rcv == -1 && s1snd != -1 && s2rcv != -1 && s2snd == -1)
        return nn_device_oneway (s2, s2rcv, s1, s1snd);

    /*  This should never happen. */
    nn_assert (0);
}

int nn_device_loopback (int s)
{
    int rc;
    int op;
    size_t opsz;

    /*  Check whether the socket is a "raw" socket. */
    opsz = sizeof (op);
    rc = nn_getsockopt (s, NN_SOL_SOCKET, NN_DOMAIN, &op, &opsz);
    errno_assert (rc == 0);
    nn_assert (opsz == sizeof (op));
    if (op != AF_SP_RAW) {
        errno = EINVAL;
        return -1;
    }

    while (1) {
        rc = nn_device_mvmsg (s, s, 0);
        if (nn_slow (rc < 0))
            return -1;
    }
}

#if defined NN_HAVE_WINDOWS

static int nn_device_twoway (int s1, nn_fd s1rcv, nn_fd s1snd,
    int s2, nn_fd s2rcv, nn_fd s2snd)
{
    int rc;
    fd_set fds;
    int s1rcv_isready = 0;
    int s1snd_isready = 0;
    int s2rcv_isready = 0;
    int s2snd_isready = 0;

    /*  Initialise the pollset. */
    FD_ZERO (&fds);

    while (1) {

        /*  Wait for network events. Adjust the 'ready' events based
            on the result. */
        if (s1rcv_isready)
            FD_CLR (s1rcv, &fds);
        else
            FD_SET (s1rcv, &fds);
        if (s1snd_isready)
            FD_CLR (s1snd, &fds);
        else
            FD_SET (s1snd, &fds);
        if (s2rcv_isready)
            FD_CLR (s2rcv, &fds);
        else
            FD_SET (s2rcv, &fds);
        if (s2snd_isready)
            FD_CLR (s2snd, &fds);
        else
            FD_SET (s2snd, &fds);
        rc = select (0, &fds, NULL, NULL, NULL);
        wsa_assert (rc != SOCKET_ERROR);
        if (FD_ISSET (s1rcv, &fds))
            s1rcv_isready = 1;
        if (FD_ISSET (s1snd, &fds))
            s1snd_isready = 1;
        if (FD_ISSET (s2rcv, &fds))
            s2rcv_isready = 1;
        if (FD_ISSET (s2snd, &fds))
            s2snd_isready = 1;

        /*  If possible, pass the message from s1 to s2. */
        if (s1rcv_isready && s2snd_isready) {
            rc = nn_device_mvmsg (s1, s2, NN_DONTWAIT);
            if (nn_slow (rc < 0))
                return -1;
            s1rcv_isready = 0;
            s2snd_isready = 0;
        }

        /*  If possible, pass the message from s2 to s1. */
        if (s2rcv_isready && s1snd_isready) {
            rc = nn_device_mvmsg (s2, s1, NN_DONTWAIT);
            if (nn_slow (rc < 0))
                return -1;
            s2rcv_isready = 0;
            s1snd_isready = 0;
        }
    }
}

#elif defined NN_HAVE_POLL

static int nn_device_twoway (int s1, nn_fd s1rcv, nn_fd s1snd,
    int s2, nn_fd s2rcv, nn_fd s2snd)
{
    int rc;
    struct pollfd pfd [4];

    /*  Initialise the pollset. */
    pfd [0].fd = s1rcv;
    pfd [0].events = POLLIN;
    pfd [1].fd = s1snd;
    pfd [1].events = POLLIN;
    pfd [2].fd = s2rcv;
    pfd [2].events = POLLIN;
    pfd [3].fd = s2snd;
    pfd [3].events = POLLIN;

    while (1) {

        /*  Wait for network events. */
        rc = poll (pfd, 4, -1);
        errno_assert (rc >= 0);
        if (nn_slow (rc < 0 && errno == EINTR))
            return -1;
        nn_assert (rc != 0);

        /*  Process the events. When the event is received, we cease polling
            for it. */
        if (pfd [0].revents & POLLIN)
            pfd [0].events = 0;
        if (pfd [1].revents & POLLIN)
            pfd [1].events = 0;
        if (pfd [2].revents & POLLIN)
            pfd [2].events = 0;
        if (pfd [3].revents & POLLIN)
            pfd [3].events = 0;

        /*  If possible, pass the message from s1 to s2. */
        if (pfd [0].events == 0 && pfd [3].events == 0) {
            rc = nn_device_mvmsg (s1, s2, NN_DONTWAIT);
            if (nn_slow (rc < 0))
                return -1;
            pfd [0].events = POLLIN;
            pfd [3].events = POLLIN;
        }

        /*  If possible, pass the message from s2 to s1. */
        if (pfd [2].events == 0 && pfd [1].events == 0) {
            rc = nn_device_mvmsg (s2, s1, NN_DONTWAIT);
            if (nn_slow (rc < 0))
                return -1;
            pfd [2].events = POLLIN;
            pfd [1].events = POLLIN;
        }
    }
}

#else
#error
#endif

static int nn_device_oneway (int s1, NN_UNUSED nn_fd s1rcv,
                             int s2, NN_UNUSED nn_fd s2snd)
{
    int rc;

    while (1) {
        rc = nn_device_mvmsg (s1, s2, 0);
        if (nn_slow (rc < 0))
            return -1;
    }
}

static int nn_device_mvmsg (int from, int to, int flags)
{
    int rc;
    void *body;
    void *control;
    struct nn_iovec iov;
    struct nn_msghdr hdr;

    iov.iov_base = &body;
    iov.iov_len = NN_MSG;
    memset (&hdr, 0, sizeof (hdr));
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;
    hdr.msg_control = &control;
    hdr.msg_controllen = NN_MSG;
    rc = nn_recvmsg (from, &hdr, flags);
    if (nn_slow (rc < 0 && nn_errno () == ETERM))
        return -1;
    errno_assert (rc >= 0);
    rc = nn_sendmsg (to, &hdr, flags);
    if (nn_slow (rc < 0 && nn_errno () == ETERM))
        return -1;
    errno_assert (rc >= 0);
    return 0;
}
