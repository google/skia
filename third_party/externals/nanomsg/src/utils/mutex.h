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

#ifndef NN_MUTEX_INCLUDED
#define NN_MUTEX_INCLUDED

#ifdef NN_HAVE_WINDOWS
#include "win.h"
#else
#include <pthread.h>
#endif

struct nn_mutex {
#ifdef NN_HAVE_WINDOWS
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
};

/*  Initialise the mutex. */
void nn_mutex_init (struct nn_mutex *self);

/*  Terminate the mutex. */
void nn_mutex_term (struct nn_mutex *self);

/*  Lock the mutex. Behaviour of multiple locks from the same thread is
    undefined. */
void nn_mutex_lock (struct nn_mutex *self);

/*  Unlock the mutex. Behaviour of unlocking an unlocked mutex is undefined */
void nn_mutex_unlock (struct nn_mutex *self);

#endif

