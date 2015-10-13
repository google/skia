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

#include "mutex.h"
#include "err.h"

#ifdef NN_HAVE_WINDOWS

void nn_mutex_init (struct nn_mutex *self)
{
    InitializeCriticalSection (&self->mutex);
}

void nn_mutex_term (struct nn_mutex *self)
{
    DeleteCriticalSection (&self->mutex);
}

void nn_mutex_lock (struct nn_mutex *self)
{
    EnterCriticalSection (&self->mutex);
}

void nn_mutex_unlock (struct nn_mutex *self)
{
    LeaveCriticalSection (&self->mutex);
}

#else

void nn_mutex_init (struct nn_mutex *self)
{
    int rc;

    rc = pthread_mutex_init (&self->mutex, NULL);
    errnum_assert (rc == 0, rc);
}

void nn_mutex_term (struct nn_mutex *self)
{
    int rc;

    rc = pthread_mutex_destroy (&self->mutex);
    errnum_assert (rc == 0, rc);
}

void nn_mutex_lock (struct nn_mutex *self)
{
    int rc;

    rc = pthread_mutex_lock (&self->mutex);
    errnum_assert (rc == 0, rc);
}

void nn_mutex_unlock (struct nn_mutex *self)
{
    int rc;

    rc = pthread_mutex_unlock (&self->mutex);
    errnum_assert (rc == 0, rc);
}

#endif

