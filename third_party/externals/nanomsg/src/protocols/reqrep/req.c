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

#include "req.h"
#include "xreq.h"

#include "../../nn.h"
#include "../../reqrep.h"

#include "../../aio/fsm.h"
#include "../../aio/timer.h"

#include "../../utils/err.h"
#include "../../utils/cont.h"
#include "../../utils/alloc.h"
#include "../../utils/random.h"
#include "../../utils/wire.h"
#include "../../utils/list.h"
#include "../../utils/int.h"
#include "../../utils/attr.h"

#include <stddef.h>
#include <string.h>

/*  Default re-send interval is 1 minute. */
#define NN_REQ_DEFAULT_RESEND_IVL 60000

#define NN_REQ_STATE_IDLE 1
#define NN_REQ_STATE_PASSIVE 2
#define NN_REQ_STATE_DELAYED 3
#define NN_REQ_STATE_ACTIVE 4
#define NN_REQ_STATE_TIMED_OUT 5
#define NN_REQ_STATE_CANCELLING 6
#define NN_REQ_STATE_STOPPING_TIMER 7
#define NN_REQ_STATE_DONE 8
#define NN_REQ_STATE_STOPPING 9

#define NN_REQ_ACTION_START 1
#define NN_REQ_ACTION_IN 2
#define NN_REQ_ACTION_OUT 3
#define NN_REQ_ACTION_SENT 4
#define NN_REQ_ACTION_RECEIVED 5
#define NN_REQ_ACTION_PIPE_RM 6

#define NN_REQ_SRC_RESEND_TIMER 1

struct nn_req {

    /*  The base class. Raw REQ socket. */
    struct nn_xreq xreq;

    /*  The state machine. */
    struct nn_fsm fsm;
    int state;

    /*  ID of the request being currently processed. Replies for different
        requests are considered stale and simply dropped. */
    uint32_t reqid;

    /*  Stored request, so that it can be re-sent if needed. */
    struct nn_msg request;

    /*  Stored reply, so that user can retrieve it later on. */
    struct nn_msg reply;

    /*  Timer used to wait while request should be re-sent. */
    struct nn_timer timer;

    /*  Protocol-specific socket options. */
    int resend_ivl;

    /*  Pipe the current request has been sent to. Non-null only in ACTIVE
        state  */
    struct nn_pipe *sent_to;
};

/*  Private functions. */
static void nn_req_init (struct nn_req *self,
    const struct nn_sockbase_vfptr *vfptr, void *hint);
static void nn_req_term (struct nn_req *self);
static int nn_req_inprogress (struct nn_req *self);
static void nn_req_handler (struct nn_fsm *self, int src, int type,
    void *srcptr);
static void nn_req_shutdown (struct nn_fsm *self, int src, int type,
    void *srcptr);
static void nn_req_action_send (struct nn_req *self, int allow_delay);

/*  Implementation of nn_sockbase's virtual functions. */
static void nn_req_stop (struct nn_sockbase *self);
static void nn_req_destroy (struct nn_sockbase *self);
static void nn_req_in (struct nn_sockbase *self, struct nn_pipe *pipe);
static void nn_req_out (struct nn_sockbase *self, struct nn_pipe *pipe);
static int nn_req_events (struct nn_sockbase *self);
static int nn_req_send (struct nn_sockbase *self, struct nn_msg *msg);
static void nn_req_rm (struct nn_sockbase *self, struct nn_pipe *pipe);
static int nn_req_recv (struct nn_sockbase *self, struct nn_msg *msg);
static int nn_req_setopt (struct nn_sockbase *self, int level, int option,
    const void *optval, size_t optvallen);
static int nn_req_getopt (struct nn_sockbase *self, int level, int option,
    void *optval, size_t *optvallen);
static const struct nn_sockbase_vfptr nn_req_sockbase_vfptr = {
    nn_req_stop,
    nn_req_destroy,
    nn_xreq_add,
    nn_req_rm,
    nn_req_in,
    nn_req_out,
    nn_req_events,
    nn_req_send,
    nn_req_recv,
    nn_req_setopt,
    nn_req_getopt
};

static void nn_req_init (struct nn_req *self,
    const struct nn_sockbase_vfptr *vfptr, void *hint)
{
    nn_xreq_init (&self->xreq, vfptr, hint);
    nn_fsm_init_root (&self->fsm, nn_req_handler, nn_req_shutdown,
        nn_sockbase_getctx (&self->xreq.sockbase));
    self->state = NN_REQ_STATE_IDLE;
    self->sent_to = NULL;

    /*  Start assigning request IDs beginning with a random number. This way
        there should be no key clashes even if the executable is re-started. */
    nn_random_generate (&self->reqid, sizeof (self->reqid));

    nn_msg_init (&self->request, 0);
    nn_msg_init (&self->reply, 0);
    nn_timer_init (&self->timer, NN_REQ_SRC_RESEND_TIMER, &self->fsm);
    self->resend_ivl = NN_REQ_DEFAULT_RESEND_IVL;

    /*  Start the state machine. */
    nn_fsm_start (&self->fsm);
}

static void nn_req_term (struct nn_req *self)
{
    nn_timer_term (&self->timer);
    nn_msg_term (&self->reply);
    nn_msg_term (&self->request);
    nn_fsm_term (&self->fsm);
    nn_xreq_term (&self->xreq);
}

static void nn_req_stop (struct nn_sockbase *self)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    nn_fsm_stop (&req->fsm);
}

static void nn_req_destroy (struct nn_sockbase *self)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    nn_req_term (req);
    nn_free (req);
}

static int nn_req_inprogress (struct nn_req *self)
{
    /*  Return 1 if there's a request submitted. 0 otherwise. */
    return self->state == NN_REQ_STATE_IDLE ||
        self->state == NN_REQ_STATE_PASSIVE ||
        self->state == NN_REQ_STATE_STOPPING ? 0 : 1;
}

static void nn_req_in (struct nn_sockbase *self, struct nn_pipe *pipe)
{
    int rc;
    struct nn_req *req;
    uint32_t reqid;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    /*  Pass the pipe to the raw REQ socket. */
    nn_xreq_in (&req->xreq.sockbase, pipe);

    while (1) {

        /*  Get new reply. */
        rc = nn_xreq_recv (&req->xreq.sockbase, &req->reply);
        if (nn_slow (rc == -EAGAIN))
            return;
        errnum_assert (rc == 0, -rc);

        /*  No request was sent. Getting a reply doesn't make sense. */
        if (nn_slow (!nn_req_inprogress (req))) {
            nn_msg_term (&req->reply);
            continue;
        }

        /*  Ignore malformed replies. */
        if (nn_slow (nn_chunkref_size (&req->reply.hdr) != sizeof (uint32_t))) {
            nn_msg_term (&req->reply);
            continue;
        }

        /*  Ignore replies with incorrect request IDs. */
        reqid = nn_getl (nn_chunkref_data (&req->reply.hdr));
        if (nn_slow (!(reqid & 0x80000000))) {
            nn_msg_term (&req->reply);
            continue;
        }
        if (nn_slow (reqid != (req->reqid | 0x80000000))) {
            nn_msg_term (&req->reply);
            continue;
        }

        /*  Trim the request ID. */
        nn_chunkref_term (&req->reply.hdr);
        nn_chunkref_init (&req->reply.hdr, 0);

        /*  TODO: Deallocate the request here? */

        /*  Notify the state machine. */
        if (req->state == NN_REQ_STATE_ACTIVE)
            nn_fsm_action (&req->fsm, NN_REQ_ACTION_IN);

        return;
    }
}

static void nn_req_out (struct nn_sockbase *self, struct nn_pipe *pipe)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    /*  Add the pipe to the underlying raw socket. */
    nn_xreq_out (&req->xreq.sockbase, pipe);

    /*  Notify the state machine. */
    if (req->state == NN_REQ_STATE_DELAYED)
        nn_fsm_action (&req->fsm, NN_REQ_ACTION_OUT);
}

static int nn_req_events (struct nn_sockbase *self)
{
    int rc;
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    /*  OUT is signalled all the time because sending a request while
        another one is being processed cancels the old one. */
    rc = NN_SOCKBASE_EVENT_OUT;

    /*  In DONE state the reply is stored in 'reply' field. */
    if (req->state == NN_REQ_STATE_DONE)
        rc |= NN_SOCKBASE_EVENT_IN;

    return rc;
}

static int nn_req_send (struct nn_sockbase *self, struct nn_msg *msg)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    /*  Generate new request ID for the new request and put it into message
        header. The most important bit is set to 1 to indicate that this is
        the bottom of the backtrace stack. */
    ++req->reqid;
    nn_assert (nn_chunkref_size (&msg->hdr) == 0);
    nn_chunkref_term (&msg->hdr);
    nn_chunkref_init (&msg->hdr, 4);
    nn_putl (nn_chunkref_data (&msg->hdr), req->reqid | 0x80000000);

    /*  Store the message so that it can be re-sent if there's no reply. */
    nn_msg_term (&req->request);
    nn_msg_mv (&req->request, msg);

    /*  Notify the state machine. */
    nn_fsm_action (&req->fsm, NN_REQ_ACTION_SENT);

    return 0;
}

static int nn_req_recv (struct nn_sockbase *self, struct nn_msg *msg)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    /*  No request was sent. Waiting for a reply doesn't make sense. */
    if (nn_slow (!nn_req_inprogress (req)))
        return -EFSM;

    /*  If reply was not yet recieved, wait further. */
    if (nn_slow (req->state != NN_REQ_STATE_DONE))
        return -EAGAIN;

    /*  If the reply was already received, just pass it to the caller. */
    nn_msg_mv (msg, &req->reply);
    nn_msg_init (&req->reply, 0);

    /*  Notify the state machine. */
    nn_fsm_action (&req->fsm, NN_REQ_ACTION_RECEIVED);

    return 0;
}

static int nn_req_setopt (struct nn_sockbase *self, int level, int option,
        const void *optval, size_t optvallen)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    if (level != NN_REQ)
        return -ENOPROTOOPT;

    if (option == NN_REQ_RESEND_IVL) {
        if (nn_slow (optvallen != sizeof (int)))
            return -EINVAL;
        req->resend_ivl = *(int*) optval;
        return 0;
    }

    return -ENOPROTOOPT;
}

static int nn_req_getopt (struct nn_sockbase *self, int level, int option,
        void *optval, size_t *optvallen)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    if (level != NN_REQ)
        return -ENOPROTOOPT;

    if (option == NN_REQ_RESEND_IVL) {
        if (nn_slow (*optvallen < sizeof (int)))
            return -EINVAL;
        *(int*) optval = req->resend_ivl;
        *optvallen = sizeof (int);
        return 0;
    }

    return -ENOPROTOOPT;
}

static void nn_req_shutdown (struct nn_fsm *self, int src, int type,
    NN_UNUSED void *srcptr)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, fsm);

    if (nn_slow (src == NN_FSM_ACTION && type == NN_FSM_STOP)) {
        nn_timer_stop (&req->timer);
        req->state = NN_REQ_STATE_STOPPING;
    }
    if (nn_slow (req->state == NN_REQ_STATE_STOPPING)) {
        if (!nn_timer_isidle (&req->timer))
            return;
        req->state = NN_REQ_STATE_IDLE;
        nn_fsm_stopped_noevent (&req->fsm);
        nn_sockbase_stopped (&req->xreq.sockbase);
        return;
    }

    nn_fsm_bad_state(req->state, src, type);
}

static void nn_req_handler (struct nn_fsm *self, int src, int type,
    NN_UNUSED void *srcptr)
{
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, fsm);

    switch (req->state) {

/******************************************************************************/
/*  IDLE state.                                                               */
/*  The socket was created recently. Intermediate state.                      */
/*  Pass straight to the PASSIVE state.                                       */
/******************************************************************************/
    case NN_REQ_STATE_IDLE:
        switch (src) {

        case NN_FSM_ACTION:
            switch (type) {
            case NN_FSM_START:
                req->state = NN_REQ_STATE_PASSIVE;
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  PASSIVE state.                                                            */
/*  No request is submitted.                                                  */
/******************************************************************************/
    case NN_REQ_STATE_PASSIVE:
        switch (src) {

        case NN_FSM_ACTION:
            switch (type) {
            case NN_REQ_ACTION_SENT:
                nn_req_action_send (req, 1);
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  DELAYED state.                                                            */
/*  Request was submitted but it could not be sent to the network because     */
/*  there was no peer available at the moment. Now we are waiting for the     */
/*  peer to arrive to send the request to it.                                 */
/******************************************************************************/
    case NN_REQ_STATE_DELAYED:
        switch (src) {

        case NN_FSM_ACTION:
            switch (type) {
            case NN_REQ_ACTION_OUT:
                nn_req_action_send (req, 0);
                return;
            case NN_REQ_ACTION_SENT:
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  ACTIVE state.                                                             */
/*  Request was submitted. Waiting for reply.                                 */
/******************************************************************************/
    case NN_REQ_STATE_ACTIVE:
        switch (src) {

        case NN_FSM_ACTION:
            switch (type) {
            case NN_REQ_ACTION_IN:

                /*  Reply arrived. */
                nn_timer_stop (&req->timer);
                req->sent_to = NULL;
                req->state = NN_REQ_STATE_STOPPING_TIMER;
                return;

            case NN_REQ_ACTION_SENT:

                /*  New request was sent while the old one was still being
                    processed. Cancel the old request first. */
                nn_timer_stop (&req->timer);
                req->sent_to = NULL;
                req->state = NN_REQ_STATE_CANCELLING;
                return;

            case NN_REQ_ACTION_PIPE_RM:
                /*  Pipe that we sent request to is removed  */
                nn_timer_stop (&req->timer);
                req->sent_to = NULL;
                /*  Pretend we timed out so request resent immediately  */
                req->state = NN_REQ_STATE_TIMED_OUT;
                return;

            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        case NN_REQ_SRC_RESEND_TIMER:
            switch (type) {
            case NN_TIMER_TIMEOUT:
                nn_timer_stop (&req->timer);
                req->sent_to = NULL;
                req->state = NN_REQ_STATE_TIMED_OUT;
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  TIMED_OUT state.                                                          */
/*  Waiting for reply has timed out. Stopping the timer. Afterwards, we'll    */
/*  re-send the request.                                                      */
/******************************************************************************/
    case NN_REQ_STATE_TIMED_OUT:
        switch (src) {

        case NN_REQ_SRC_RESEND_TIMER:
            switch (type) {
            case NN_TIMER_STOPPED:
                nn_req_action_send (req, 1);
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        case NN_FSM_ACTION:
            switch (type) {
            case NN_REQ_ACTION_SENT:
                req->state = NN_REQ_STATE_CANCELLING;
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  CANCELLING state.                                                         */
/*  Request was canceled. Waiting till the timer is stopped. Note that        */
/*  cancelling is done by sending a new request. Thus there's already         */
/*  a request waiting to be sent in this state.                               */
/******************************************************************************/
    case NN_REQ_STATE_CANCELLING:
        switch (src) {

        case NN_REQ_SRC_RESEND_TIMER:
            switch (type) {
            case NN_TIMER_STOPPED:

                /*  Timer is stopped. Now we can send the delayed request. */
                nn_req_action_send (req, 1);
                return;

            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        case NN_FSM_ACTION:
             switch (type) {
             case NN_REQ_ACTION_SENT:

                 /*  No need to do anything here. Old delayed request is just
                     replaced by the new one that will be sent once the timer
                     is closed. */
                 return;

             default:
                 nn_fsm_bad_action (req->state, src, type);
             }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  STOPPING_TIMER state.                                                     */
/*  Reply was delivered. Waiting till the timer is stopped.                   */
/******************************************************************************/
    case NN_REQ_STATE_STOPPING_TIMER:
        switch (src) {

        case NN_REQ_SRC_RESEND_TIMER:

            switch (type) {
            case NN_TIMER_STOPPED:
                req->state = NN_REQ_STATE_DONE;
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        case NN_FSM_ACTION:
            switch (type) {
            case NN_REQ_ACTION_SENT:
                req->state = NN_REQ_STATE_CANCELLING;
                return;
            default:
                nn_fsm_bad_action (req->state, src, type);
            }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  DONE state.                                                               */
/*  Reply was received but not yet retrieved by the user.                     */
/******************************************************************************/
    case NN_REQ_STATE_DONE:
        switch (src) {

        case NN_FSM_ACTION:
             switch (type) {
             case NN_REQ_ACTION_RECEIVED:
                 req->state = NN_REQ_STATE_PASSIVE;
                 return;
             case NN_REQ_ACTION_SENT:
                 nn_req_action_send (req, 1);
                 return;
             default:
                 nn_fsm_bad_action (req->state, src, type);
             }

        default:
            nn_fsm_bad_source (req->state, src, type);
        }

/******************************************************************************/
/*  Invalid state.                                                            */
/******************************************************************************/
    default:
        nn_fsm_bad_state (req->state, src, type);
    }
}

/******************************************************************************/
/*  State machine actions.                                                    */
/******************************************************************************/

static void nn_req_action_send (struct nn_req *self, int allow_delay)
{
    int rc;
    struct nn_msg msg;
    struct nn_pipe *to;

    /*  Send the request. */
    nn_msg_cp (&msg, &self->request);
    rc = nn_xreq_send_to (&self->xreq.sockbase, &msg, &to);

    /*  If the request cannot be sent at the moment wait till
        new outbound pipe arrives. */
    if (nn_slow (rc == -EAGAIN)) {
        nn_assert (allow_delay == 1);
        nn_msg_term (&msg);
        self->state = NN_REQ_STATE_DELAYED;
        return;
    }

    /*  Request was successfully sent. Set up the re-send timer
        in case the request gets lost somewhere further out
        in the topology. */
    if (nn_fast (rc == 0)) {
        nn_timer_start (&self->timer, self->resend_ivl);
        nn_assert (to);
        self->sent_to = to;
        self->state = NN_REQ_STATE_ACTIVE;
        return;
    }

    /*  Unexpected error. */
    errnum_assert (0, -rc);
}

static int nn_req_create (void *hint, struct nn_sockbase **sockbase)
{
    struct nn_req *self;

    self = nn_alloc (sizeof (struct nn_req), "socket (req)");
    alloc_assert (self);
    nn_req_init (self, &nn_req_sockbase_vfptr, hint);
    *sockbase = &self->xreq.sockbase;

    return 0;
}

void nn_req_rm (struct nn_sockbase *self, struct nn_pipe *pipe) {
    struct nn_req *req;

    req = nn_cont (self, struct nn_req, xreq.sockbase);

    nn_xreq_rm (self, pipe);
    if (nn_slow (pipe == req->sent_to)) {
        nn_fsm_action (&req->fsm, NN_REQ_ACTION_PIPE_RM);
    }
}

static struct nn_socktype nn_req_socktype_struct = {
    AF_SP,
    NN_REQ,
    0,
    nn_req_create,
    nn_xreq_ispeer,
    NN_LIST_ITEM_INITIALIZER
};

struct nn_socktype *nn_req_socktype = &nn_req_socktype_struct;

