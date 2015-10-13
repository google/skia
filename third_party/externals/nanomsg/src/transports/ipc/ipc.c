/*
    Copyright (c) 2012-2013 250bpm s.r.o.  All rights reserved.
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

#include "ipc.h"
#include "bipc.h"
#include "cipc.h"

#include "../../ipc.h"

#include "../../utils/err.h"
#include "../../utils/alloc.h"
#include "../../utils/fast.h"
#include "../../utils/list.h"

#include <string.h>
#if defined NN_HAVE_WINDOWS
#include "../../utils/win.h"
#else
#include <sys/un.h>
#include <unistd.h>
#endif

/*  nn_transport interface. */
static int nn_ipc_bind (void *hint, struct nn_epbase **epbase);
static int nn_ipc_connect (void *hint, struct nn_epbase **epbase);

static struct nn_transport nn_ipc_vfptr = {
    "ipc",
    NN_IPC,
    NULL,
    NULL,
    nn_ipc_bind,
    nn_ipc_connect,
    NULL,
    NN_LIST_ITEM_INITIALIZER
};

struct nn_transport *nn_ipc = &nn_ipc_vfptr;

static int nn_ipc_bind (void *hint, struct nn_epbase **epbase)
{
    return nn_bipc_create (hint, epbase);
}

static int nn_ipc_connect (void *hint, struct nn_epbase **epbase)
{
    return nn_cipc_create (hint, epbase);
}

