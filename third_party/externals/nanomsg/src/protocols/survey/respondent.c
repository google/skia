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

#include "respondent.h"
#include "xrespondent.h"

#include "../../nn.h"
#include "../../survey.h"

#include "../../utils/err.h"
#include "../../utils/cont.h"
#include "../../utils/fast.h"
#include "../../utils/alloc.h"
#include "../../utils/wire.h"
#include "../../utils/list.h"
#include "../../utils/int.h"

#include <string.h>

#define NN_RESPONDENT_INPROGRESS 1

struct nn_respondent {
    struct nn_xrespondent xrespondent;
    uint32_t surveyid;
    uint32_t flags;
};

/*  Private functions. */
static void nn_respondent_init (struct nn_respondent *self,
    const struct nn_sockbase_vfptr *vfptr, void *hint);
static void nn_respondent_term (struct nn_respondent *self);

/*  Implementation of nn_sockbase's virtual functions. */
static void nn_respondent_destroy (struct nn_sockbase *self);
static int nn_respondent_events (struct nn_sockbase *self);
static int nn_respondent_send (struct nn_sockbase *self, struct nn_msg *msg);
static int nn_respondent_recv (struct nn_sockbase *self, struct nn_msg *msg);
static const struct nn_sockbase_vfptr nn_respondent_sockbase_vfptr = {
    NULL,
    nn_respondent_destroy,
    nn_xrespondent_add,
    nn_xrespondent_rm,
    nn_xrespondent_in,
    nn_xrespondent_out,
    nn_respondent_events,
    nn_respondent_send,
    nn_respondent_recv,
    nn_xrespondent_setopt,
    nn_xrespondent_getopt
};

static void nn_respondent_init (struct nn_respondent *self,
    const struct nn_sockbase_vfptr *vfptr, void *hint)
{
    nn_xrespondent_init (&self->xrespondent, vfptr, hint);
    self->flags = 0;
}

static void nn_respondent_term (struct nn_respondent *self)
{
    nn_xrespondent_term (&self->xrespondent);
}

void nn_respondent_destroy (struct nn_sockbase *self)
{
    struct nn_respondent *respondent;

    respondent = nn_cont (self, struct nn_respondent, xrespondent.sockbase);

    nn_respondent_term (respondent);
    nn_free (respondent);
}

static int nn_respondent_events (struct nn_sockbase *self)
{
    int events;
    struct nn_respondent *respondent;

    respondent = nn_cont (self, struct nn_respondent, xrespondent.sockbase);

    events = nn_xrespondent_events (&respondent->xrespondent.sockbase);
    if (!(respondent->flags & NN_RESPONDENT_INPROGRESS))
        events &= ~NN_SOCKBASE_EVENT_OUT;
    return events;
}

static int nn_respondent_send (struct nn_sockbase *self, struct nn_msg *msg)
{
    int rc;
    struct nn_respondent *respondent;

    respondent = nn_cont (self, struct nn_respondent, xrespondent.sockbase);

    /*  If there's no survey going on, report EFSM error. */
    if (nn_slow (!(respondent->flags & NN_RESPONDENT_INPROGRESS)))
        return -EFSM;

    /*  Tag the message with survey ID. */
    nn_assert (nn_chunkref_size (&msg->hdr) == 0);
    nn_chunkref_term (&msg->hdr);
    nn_chunkref_init (&msg->hdr, 4);
    nn_putl (nn_chunkref_data (&msg->hdr), respondent->surveyid);

    /*  Try to send the message. If it cannot be sent due to pushback, drop it
        silently. */
    rc = nn_xrespondent_send (&respondent->xrespondent.sockbase, msg);
    if (nn_slow (rc == -EAGAIN)) {
        nn_msg_term (msg);
        return -EAGAIN;
    }
    errnum_assert (rc == 0, -rc);

    /*  Remember that no survey is being processed. */
    respondent->flags &= ~NN_RESPONDENT_INPROGRESS;

    return 0;
}

static int nn_respondent_recv (struct nn_sockbase *self, struct nn_msg *msg)
{
    int rc;
    struct nn_respondent *respondent;

    respondent = nn_cont (self, struct nn_respondent, xrespondent.sockbase);

    /*  Cancel current survey, if it exists. */
    respondent->flags &= ~NN_RESPONDENT_INPROGRESS;

    /*  Get next survey. */
    rc = nn_xrespondent_recv (&respondent->xrespondent.sockbase, msg);
    if (nn_slow (rc == -EAGAIN))
        return -EAGAIN;
    errnum_assert (rc == 0, -rc);

    /*  Remember the survey ID. */
    nn_assert (nn_chunkref_size (&msg->hdr) == sizeof (uint32_t));
    respondent->surveyid = nn_getl (nn_chunkref_data (&msg->hdr));
    nn_chunkref_term (&msg->hdr);
    nn_chunkref_init (&msg->hdr, 0);

    /*  Remember that survey is being processed. */
    respondent->flags |= NN_RESPONDENT_INPROGRESS;

    return 0;
}

static int nn_respondent_create (void *hint, struct nn_sockbase **sockbase)
{
    struct nn_respondent *self;

    self = nn_alloc (sizeof (struct nn_respondent), "socket (respondent)");
    alloc_assert (self);
    nn_respondent_init (self, &nn_respondent_sockbase_vfptr, hint);
    *sockbase = &self->xrespondent.sockbase;

    return 0;
}

static struct nn_socktype nn_respondent_socktype_struct = {
    AF_SP,
    NN_RESPONDENT,
    0,
    nn_respondent_create,
    nn_xrespondent_ispeer,
    NN_LIST_ITEM_INITIALIZER
};

struct nn_socktype *nn_respondent_socktype = &nn_respondent_socktype_struct;

