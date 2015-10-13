/*
    Copyright (c) 201-2013 250bpm s.r.o.  All rights reserved.

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

#ifndef NN_XRESPONDENT_INCLUDED
#define NN_XRESPONDENT_INCLUDED

#include "../../protocol.h"

#include "../utils/excl.h"

extern struct nn_socktype *nn_xrespondent_socktype;

struct nn_xrespondent {
    struct nn_sockbase sockbase;
    struct nn_excl excl;
};

void nn_xrespondent_init (struct nn_xrespondent *self,
    const struct nn_sockbase_vfptr *vfptr, void *hint);
void nn_xrespondent_term (struct nn_xrespondent *self);

int nn_xrespondent_add (struct nn_sockbase *self, struct nn_pipe *pipe);
void nn_xrespondent_rm (struct nn_sockbase *self, struct nn_pipe *pipe);
void nn_xrespondent_in (struct nn_sockbase *self, struct nn_pipe *pipe);
void nn_xrespondent_out (struct nn_sockbase *self, struct nn_pipe *pipe);
int nn_xrespondent_events (struct nn_sockbase *self);
int nn_xrespondent_send (struct nn_sockbase *self, struct nn_msg *msg);
int nn_xrespondent_recv (struct nn_sockbase *self, struct nn_msg *msg);
int nn_xrespondent_setopt (struct nn_sockbase *self, int level, int option,
    const void *optval, size_t optvallen);
int nn_xrespondent_getopt (struct nn_sockbase *self, int level, int option,
    void *optval, size_t *optvallen);

int nn_xrespondent_ispeer (int socktype);

#endif
