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

#ifndef NN_ALLOC_INCLUDED
#define NN_ALLOC_INCLUDED

#include <stddef.h>

/*  These functions allows for interception of memory allocation-related
    functionality. */

void nn_alloc_init (void);
void nn_alloc_term (void);
void *nn_realloc (void *ptr, size_t size);
void nn_free (void *ptr);

#if defined NN_ALLOC_MONITOR
#define nn_alloc(size, name) nn_alloc_ (size, name)
void *nn_alloc_ (size_t size, const char *name);
#else
#define nn_alloc(size, name) nn_alloc_(size)
void *nn_alloc_ (size_t size);
#endif

#endif
