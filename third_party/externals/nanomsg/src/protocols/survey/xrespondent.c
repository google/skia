/*
    Copyright (c) 2012-2013 250bpm s.r.o.  All rights reserved.

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

#include "xrespondent.h"

#include "../../nn.h"
#include "../../survey.h"

#include "../../utils/err.h"
#include "../../utils/cont.h"
#include "../../utils/fast.h"
#include "../../utils/alloc.h"
#include "../../utils/list.h"
#include "../../utils/attr.h"

/*  Private functions. */
static void nn_xrespondent_destroy (struct nn_sockbase *self);

/*  Implementation of nn_sockbase's virtual functions. */
static const struct nn_sockbase_vfptr nn_xrespondent_sockbase_vfptr = {
    NULL,
    nn_xrespondent_destroy,
    nn_xrespondent_add,
    nn_xrespondent_rm,
    nn_xrespondent_in,
    nn_xrespondent_out,
    nn_xrespondent_events,
    nn_xrespondent_send,
    nn_xrespondent_recv,
    nn_xrespondent_setopt,
    nn_xrespondent_getopt
};

void nn_xrespondent_init (struct nn_xrespondent *self,
    const struct nn_sockbase_vfptr *vfptr, void *hint)
{
    nn_sockbase_init (&self->sockbase, vfptr, hint);
    nn_excl_init (&self->excl);
}

void nn_xrespondent_term (struct nn_xrespondent *self)
{
    nn_excl_term (&self->excl);
    nn_sockbase_term (&self->sockbase);
}

static void nn_xrespondent_destroy (struct nn_sockbase *self)
{
    struct nn_xrespondent *xrespondent;

    xrespondent = nn_cont (self, struct nn_xrespondent, sockbase);

    nn_xrespondent_term (xrespondent);
    nn_free (xrespondent);
}

int nn_xrespondent_add (struct nn_sockbase *self, struct nn_pipe *pipe)
{
    return nn_excl_add (&nn_cont (self, struct nn_xrespondent, sockbase)->excl,
        pipe);
}

void nn_xrespondent_rm (struct nn_sockbase *self, struct nn_pipe *pipe)
{
    nn_excl_rm (&nn_cont (self, struct nn_xrespondent, sockbase)->excl, pipe);
}

void nn_xrespondent_in (struct nn_sockbase *self, struct nn_pipe *pipe)
{
    nn_excl_in (&nn_cont (self, struct nn_xrespondent, sockbase)->excl, pipe);
}

void nn_xrespondent_out (struct nn_sockbase *self, struct nn_pipe *pipe)
{
    nn_excl_out (&nn_cont (self, struct nn_xrespondent, sockbase)->excl, pipe);
}

int nn_xrespondent_events (struct nn_sockbase *self)
{
    struct nn_xrespondent *xrespondent;
    int events;

    xrespondent = nn_cont (self, struct nn_xrespondent, sockbase);

    events = 0;
    if (nn_excl_can_recv (&xrespondent->excl))
        events |= NN_SOCKBASE_EVENT_IN;
    if (nn_excl_can_send (&xrespondent->excl))
        events |= NN_SOCKBASE_EVENT_OUT;
    return events;
}

int nn_xrespondent_send (struct nn_sockbase *self, struct nn_msg *msg)
{
    struct nn_xrespondent *xrespondent;

    xrespondent = nn_cont (self, struct nn_xrespondent, sockbase);

    return nn_excl_send (&xrespondent->excl, msg);
}

int nn_xrespondent_recv (struct nn_sockbase *self, struct nn_msg *msg)
{
    int rc;
    struct nn_xrespondent *xrespondent;

    xrespondent = nn_cont (self, struct nn_xrespondent, sockbase);

    /*  Get the survey. */
    rc = nn_excl_recv (&xrespondent->excl, msg);
    if (rc == -EAGAIN)
        return -EAGAIN;
    errnum_assert (rc >= 0, -rc);

    /*  Split the survey ID from the body, if needed. */
    if (!(rc & NN_PIPE_PARSED)) {
        if (nn_slow (nn_chunkref_size (&msg->body) < sizeof (uint32_t))) {
            nn_msg_term (msg);
            return -EAGAIN;
        }
        nn_chunkref_term (&msg->hdr);
        nn_chunkref_init (&msg->hdr, sizeof (uint32_t));
        memcpy (nn_chunkref_data (&msg->hdr), nn_chunkref_data (&msg->body),
            sizeof (uint32_t));
        nn_chunkref_trim (&msg->body, sizeof (uint32_t));
    }

    return 0;
}

int nn_xrespondent_setopt (NN_UNUSED struct nn_sockbase *self,
    NN_UNUSED int level, NN_UNUSED int option,
    NN_UNUSED const void *optval, NN_UNUSED size_t optvallen)
{
    return -ENOPROTOOPT;
}

int nn_xrespondent_getopt (NN_UNUSED struct nn_sockbase *self,
    NN_UNUSED int level, NN_UNUSED int option,
    NN_UNUSED void *optval, NN_UNUSED size_t *optvallen)
{
    return -ENOPROTOOPT;
}

static int nn_xrespondent_create (void *hint, struct nn_sockbase **sockbase)
{
    struct nn_xrespondent *self;

    self = nn_alloc (sizeof (struct nn_xrespondent), "socket (xrespondent)");
    alloc_assert (self);
    nn_xrespondent_init (self, &nn_xrespondent_sockbase_vfptr, hint);
    *sockbase = &self->sockbase;

    return 0;
}

int nn_xrespondent_ispeer (int socktype)
{
    return socktype == NN_SURVEYOR ? 1 : 0;
}

static struct nn_socktype nn_xrespondent_socktype_struct = {
    AF_SP_RAW,
    NN_RESPONDENT,
    0,
    nn_xrespondent_create,
    nn_xrespondent_ispeer,
    NN_LIST_ITEM_INITIALIZER
};

struct nn_socktype *nn_xrespondent_socktype = &nn_xrespondent_socktype_struct;

