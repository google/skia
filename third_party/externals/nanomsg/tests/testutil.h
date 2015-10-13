/*
    Copyright (c) 2013 Insollo Entertainment, LLC. All rights reserved.

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

#ifndef TESTUTIL_H_INCLUDED
#define TESTUTIL_H_INCLUDED

#include "../src/utils/attr.h"
#include "../src/utils/err.c"
#include "../src/utils/sleep.c"

static int test_socket_impl (char *file, int line, int family, int protocol);
static int test_connect_impl (char *file, int line, int sock, char *address);
static int test_bind_impl (char *file, int line, int sock, char *address);
static void test_close_impl (char *file, int line, int sock);
static void test_send_impl (char *file, int line, int sock, char *data);
static void test_recv_impl (char *file, int line, int sock, char *data);

#define test_socket(f, p) test_socket_impl (__FILE__, __LINE__, (f), (p))
#define test_connect(s, a) test_connect_impl (__FILE__, __LINE__, (s), (a))
#define test_bind(s, a) test_bind_impl (__FILE__, __LINE__, (s), (a))
#define test_send(s, d) test_send_impl (__FILE__, __LINE__, (s), (d))
#define test_recv(s, d) test_recv_impl (__FILE__, __LINE__, (s), (d))
#define test_close(s) test_close_impl (__FILE__, __LINE__, (s))

static int test_socket_impl (char *file, int line, int family, int protocol)
{
    int sock;

    sock = nn_socket (family, protocol);
    if (sock == -1) {
        fprintf (stderr, "Failed create socket: %s [%d] (%s:%d)\n",
            nn_err_strerror (errno),
            (int) errno, file, line);
        nn_err_abort ();
    }

    return sock;
}

static int NN_UNUSED test_connect_impl (char *file, int line,
    int sock, char *address)
{
    int rc;

    rc = nn_connect (sock, address);
    if(rc < 0) {
        fprintf (stderr, "Failed connect to \"%s\": %s [%d] (%s:%d)\n",
            address,
            nn_err_strerror (errno),
            (int) errno, file, line);
        nn_err_abort ();
    }
    return rc;
}

static int NN_UNUSED test_bind_impl (char *file, int line,
    int sock, char *address)
{
    int rc;

    rc = nn_bind (sock, address);
    if(rc < 0) {
        fprintf (stderr, "Failed bind to \"%s\": %s [%d] (%s:%d)\n",
            address,
            nn_err_strerror (errno),
            (int) errno, file, line);
        nn_err_abort ();
    }
    return rc;
}

static void test_close_impl (char *file, int line, int sock)
{
    int rc;

    rc = nn_close (sock);
    if (rc != 0) {
        fprintf (stderr, "Failed to close socket: %s [%d] (%s:%d)\n",
            nn_err_strerror (errno),
            (int) errno, file, line);
        nn_err_abort ();
    }
}

static void NN_UNUSED test_send_impl (char *file, int line,
    int sock, char *data)
{
    size_t data_len;
    int rc;

    data_len = strlen (data);

    rc = nn_send (sock, data, data_len, 0);
    if (rc < 0) {
        fprintf (stderr, "Failed to send: %s [%d] (%s:%d)\n",
            nn_err_strerror (errno),
            (int) errno, file, line);
        nn_err_abort ();
    }
    if (rc != (int)data_len) {
        fprintf (stderr, "Data to send is truncated: %d != %d (%s:%d)\n",
            rc, (int) data_len,
            file, line);
        nn_err_abort ();
    }
}

static void NN_UNUSED test_recv_impl (char *file, int line, int sock, char *data)
{
    size_t data_len;
    int rc;
    char *buf;

    data_len = strlen (data);
    /*  We allocate plus one byte so that we are sure that message received
        has corrent length and not truncated  */
    buf = malloc (data_len+1);
    alloc_assert (buf);

    rc = nn_recv (sock, buf, data_len+1, 0);
    if (rc < 0) {
        fprintf (stderr, "Failed to recv: %s [%d] (%s:%d)\n",
            nn_err_strerror (errno),
            (int) errno, file, line);
        nn_err_abort ();
    }
    if (rc != (int)data_len) {
        fprintf (stderr, "Received data has wrong length: %d != %d (%s:%d)\n",
            rc, (int) data_len,
            file, line);
        nn_err_abort ();
    }
    if (memcmp (data, buf, data_len) != 0) {
        /*  We don't print the data as it may have binary garbage  */
        fprintf (stderr, "Received data is wrong (%s:%d)\n", file, line);
        nn_err_abort ();
    }

    free (buf);
}

#endif
