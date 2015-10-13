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

#ifndef NN_STOPWATCH_INCLUDED
#define NN_STOPWATCH_INCLUDED

#include "err.h"
#include "int.h"

/*  Check whether measured time is the expected time (in microseconds).
    The upper tolerance is 50ms so that the test doesn't fail even on
    very slow or very loaded systems. */
#define time_assert(actual,expected) \
    nn_assert (actual > ((expected) - 5000) && actual < ((expected) + 50000)); 

/*  Measures time interval in microseconds. */

struct nn_stopwatch {
    uint64_t start;
};

void nn_stopwatch_init (struct nn_stopwatch *self);
uint64_t nn_stopwatch_term (struct nn_stopwatch *self);

#endif
