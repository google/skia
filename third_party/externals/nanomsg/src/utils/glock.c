/*
    Copyright (c) 2012 250bpm s.r.o.  All rights reserved.

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

#include "glock.h"

#if defined NN_HAVE_WINDOWS

#include "win.h"
#include "err.h"

static LONG nn_glock_initialised = 0;
static CRITICAL_SECTION nn_glock_cs;

static void nn_glock_init (void)
{
    if (InterlockedCompareExchange (&nn_glock_initialised, 1, 0) == 0)
        InitializeCriticalSection (&nn_glock_cs);
}

void nn_glock_lock (void)
{
    nn_glock_init ();
    EnterCriticalSection (&nn_glock_cs);
}

void nn_glock_unlock (void)
{
    nn_glock_init ();
    LeaveCriticalSection (&nn_glock_cs);
}

#else

#include "err.h"

#include <pthread.h>

static pthread_mutex_t nn_glock_mutex = PTHREAD_MUTEX_INITIALIZER;

void nn_glock_lock (void)
{
    int rc;

    rc = pthread_mutex_lock (&nn_glock_mutex);
    errnum_assert (rc == 0, rc);
}

void nn_glock_unlock (void)
{
    int rc;

    rc = pthread_mutex_unlock (&nn_glock_mutex);
    errnum_assert (rc == 0, rc);
}

#endif

