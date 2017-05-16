/*
    Copyright (c) 2013 250bpm s.r.o.  All rights reserved.
    Copyright (c) 2013 GoPivotal, Inc.  All rights reserved.

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

#include "../src/nn.h"
#include "../src/pair.h"
#include "../src/pipeline.h"
#include "../src/inproc.h"
#include "../src/ipc.h"
#include "../src/tcp.h"
#include "testutil.h"

#define SOCKET_ADDRESS_INPROC "inproc://a"
#define SOCKET_ADDRESS_IPC "ipc://test-separation.ipc"
#define SOCKET_ADDRESS_TCP "tcp://127.0.0.1:5556"

/*  This test checks whether the library prevents interconnecting sockets
    between different non-compatible protocols. */

int main ()
{
    int rc;
    int pair;
    int pull;
    int timeo;

    /*  Inproc: Bind first, connect second. */
    pair = test_socket (AF_SP, NN_PAIR);
    test_bind (pair, SOCKET_ADDRESS_INPROC);
    pull = test_socket (AF_SP, NN_PULL);
    test_connect (pull, SOCKET_ADDRESS_INPROC);
    timeo = 100;
    rc = nn_setsockopt (pair, NN_SOL_SOCKET, NN_SNDTIMEO,
        &timeo, sizeof (timeo));
    rc = nn_send (pair, "ABC", 3, 0);
    errno_assert (rc < 0 && nn_errno () == EAGAIN);
    test_close (pull);
    test_close (pair);

    /*  Inproc: Connect first, bind second. */
    pull = test_socket (AF_SP, NN_PULL);
    test_connect (pull, SOCKET_ADDRESS_INPROC);
    pair = test_socket (AF_SP, NN_PAIR);
    test_bind (pair, SOCKET_ADDRESS_INPROC);
    timeo = 100;
    rc = nn_setsockopt (pair, NN_SOL_SOCKET, NN_SNDTIMEO,
        &timeo, sizeof (timeo));
    rc = nn_send (pair, "ABC", 3, 0);
    errno_assert (rc < 0 && nn_errno () == EAGAIN);
    test_close (pull);
    test_close (pair);

#if !defined NN_HAVE_WINDOWS

    /*  IPC */
    pair = test_socket (AF_SP, NN_PAIR);
    test_bind (pair, SOCKET_ADDRESS_IPC);
    pull = test_socket (AF_SP, NN_PULL);
    test_connect (pull, SOCKET_ADDRESS_IPC);
    timeo = 100;
    rc = nn_setsockopt (pair, NN_SOL_SOCKET, NN_SNDTIMEO,
        &timeo, sizeof (timeo));
    rc = nn_send (pair, "ABC", 3, 0);
    errno_assert (rc < 0 && nn_errno () == EAGAIN);
    test_close (pull);
    test_close (pair);

#endif

    /*  TCP */
    pair = test_socket (AF_SP, NN_PAIR);
    test_bind (pair, SOCKET_ADDRESS_TCP);
    pull = test_socket (AF_SP, NN_PULL);
    test_connect (pull, SOCKET_ADDRESS_TCP);
    timeo = 100;
    rc = nn_setsockopt (pair, NN_SOL_SOCKET, NN_SNDTIMEO,
        &timeo, sizeof (timeo));
    rc = nn_send (pair, "ABC", 3, 0);
    errno_assert (rc < 0 && nn_errno () == EAGAIN);
    test_close (pull);
    test_close (pair);

    return 0;
}

