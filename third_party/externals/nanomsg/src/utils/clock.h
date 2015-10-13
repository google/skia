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

#ifndef NN_CLOCK_INCLUDED
#define NN_CLOCK_INCLUDED

#include "int.h"

/*  Optimised retrieval of the current time. The clock object is not
    thread-safe. */

struct nn_clock
{
    uint64_t last_tsc;
    uint64_t last_time;
};

/*  Initialise the clock object. */
void nn_clock_init (struct nn_clock *self);

/*  Terminate the clock object. */
void nn_clock_term (struct nn_clock *self);

/*  Returns current time in milliseconds. */
uint64_t nn_clock_now (struct nn_clock *self);

/*  Returns an unique timestamp. If the system doesn't support producing
    timestamps the return value is zero. */
uint64_t nn_clock_timestamp ();

#endif

