/*
    Copyright (c) 2012-2014 250bpm s.r.o.  All rights reserved.
    Copyright (c) 2013 GoPivotal, Inc.  All rights reserved.

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

#include "../nn.h"
#include "../transport.h"
#include "../protocol.h"

#include "global.h"
#include "sock.h"
#include "ep.h"

#include "../aio/pool.h"
#include "../aio/timer.h"

#include "../utils/err.h"
#include "../utils/alloc.h"
#include "../utils/mutex.h"
#include "../utils/list.h"
#include "../utils/cont.h"
#include "../utils/random.h"
#include "../utils/glock.h"
#include "../utils/chunk.h"
#include "../utils/msg.h"
#include "../utils/attr.h"

#include "../transports/inproc/inproc.h"
#include "../transports/ipc/ipc.h"
#include "../transports/tcp/tcp.h"

#include "../protocols/pair/pair.h"
#include "../protocols/pair/xpair.h"
#include "../protocols/pubsub/pub.h"
#include "../protocols/pubsub/sub.h"
#include "../protocols/pubsub/xpub.h"
#include "../protocols/pubsub/xsub.h"
#include "../protocols/reqrep/rep.h"
#include "../protocols/reqrep/req.h"
#include "../protocols/reqrep/xrep.h"
#include "../protocols/reqrep/xreq.h"
#include "../protocols/pipeline/push.h"
#include "../protocols/pipeline/pull.h"
#include "../protocols/pipeline/xpush.h"
#include "../protocols/pipeline/xpull.h"
#include "../protocols/survey/respondent.h"
#include "../protocols/survey/surveyor.h"
#include "../protocols/survey/xrespondent.h"
#include "../protocols/survey/xsurveyor.h"
#include "../protocols/bus/bus.h"
#include "../protocols/bus/xbus.h"

#include "../pubsub.h"
#include "../pipeline.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if defined NN_HAVE_MINGW
#include <pthread.h>
#elif defined NN_HAVE_WINDOWS
#define gmtime_r(ptr_numtime, ptr_strtime) gmtime_s(ptr_strtime, ptr_numtime)
#endif
#define NN_HAVE_GMTIME_R


#if defined NN_HAVE_WINDOWS
#include "../utils/win.h"
#else
#include <unistd.h>
#endif

/*  Max number of concurrent SP sockets. */
#define NN_MAX_SOCKETS 512

/*  To save some space, list of unused socket slots uses uint16_t integers to
    refer to individual sockets. If there's a need to more that 0x10000 sockets,
    the type should be changed to uint32_t or int. */
CT_ASSERT (NN_MAX_SOCKETS <= 0x10000);

/*  This check is performed at the beginning of each socket operation to make
    sure that the library was initialised and the socket actually exists. */
#define NN_BASIC_CHECKS \
    if (nn_slow (!self.socks || !self.socks [s])) {\
        errno = EBADF;\
        return -1;\
    }

#define NN_CTX_FLAG_ZOMBIE 1

#define NN_GLOBAL_SRC_STAT_TIMER 1

#define NN_GLOBAL_STATE_IDLE           1
#define NN_GLOBAL_STATE_ACTIVE         2
#define NN_GLOBAL_STATE_STOPPING_TIMER 3

struct nn_global {

    /*  The global table of existing sockets. The descriptor representing
        the socket is the index to this table. This pointer is also used to
        find out whether context is initialised. If it is NULL, context is
        uninitialised. */
    struct nn_sock **socks;

    /*  Stack of unused file descriptors. */
    uint16_t *unused;

    /*  Number of actual open sockets in the socket table. */
    size_t nsocks;

    /*  Combination of the flags listed above. */
    int flags;

    /*  List of all available transports. */
    struct nn_list transports;

    /*  List of all available socket types. */
    struct nn_list socktypes;

    /*  Pool of worker threads. */
    struct nn_pool pool;

    /*  Timer and other machinery for submitting statistics  */
    struct nn_ctx ctx;
    struct nn_fsm fsm;
    int state;
    struct nn_timer stat_timer;

    int print_errors;
    int print_statistics;

    /*  Special socket ids  */
    int statistics_socket;

    /*  Application name for statistics  */
    char hostname[64];
    char appname[64];
};

/*  Singleton object containing the global state of the library. */
static struct nn_global self = {0};

/*  Context creation- and termination-related private functions. */
static void nn_global_init (void);
static void nn_global_term (void);

/*  Transport-related private functions. */
static void nn_global_add_transport (struct nn_transport *transport);
static void nn_global_add_socktype (struct nn_socktype *socktype);

/*  Private function that unifies nn_bind and nn_connect functionality.
    It returns the ID of the newly created endpoint. */
static int nn_global_create_ep (int s, const char *addr, int bind);

/*  Private socket creator which doesn't initialize global state and
    does no locking by itself */
static int nn_global_create_socket (int domain, int protocol);

/*  FSM callbacks  */
static void nn_global_handler (struct nn_fsm *self,
    int src, int type, void *srcptr);
static void nn_global_shutdown (struct nn_fsm *self,
    int src, int type, void *srcptr);


int nn_errno (void)
{
    return nn_err_errno ();
}

const char *nn_strerror (int errnum)
{
    return nn_err_strerror (errnum);
}

static void nn_global_init (void)
{
    int i;
    char *envvar;
    int rc;
    char *addr;

#if defined NN_HAVE_WINDOWS
    WSADATA data;
#endif

    /*  Check whether the library was already initialised. If so, do nothing. */
    if (self.socks)
        return;

    /*  On Windows, initialise the socket library. */
#if defined NN_HAVE_WINDOWS
    rc = WSAStartup (MAKEWORD (2, 2), &data);
    nn_assert (rc == 0);
    nn_assert (LOBYTE (data.wVersion) == 2 &&
        HIBYTE (data.wVersion) == 2);
#endif

    /*  Initialise the memory allocation subsystem. */
    nn_alloc_init ();

    /*  Seed the pseudo-random number generator. */
    nn_random_seed ();

    /*  Allocate the global table of SP sockets. */
    self.socks = nn_alloc ((sizeof (struct nn_sock*) * NN_MAX_SOCKETS) +
        (sizeof (uint16_t) * NN_MAX_SOCKETS), "socket table");
    alloc_assert (self.socks);
    for (i = 0; i != NN_MAX_SOCKETS; ++i)
        self.socks [i] = NULL;
    self.nsocks = 0;
    self.flags = 0;

    /*  Print connection and accepting errors to the stderr  */
    envvar = getenv("NN_PRINT_ERRORS");
    /*  any non-empty string is true */
    self.print_errors = envvar && *envvar;

    /*  Print socket statistics to stderr  */
    envvar = getenv("NN_PRINT_STATISTICS");
    self.print_statistics = envvar && *envvar;

    /*  Allocate the stack of unused file descriptors. */
    self.unused = (uint16_t*) (self.socks + NN_MAX_SOCKETS);
    alloc_assert (self.unused);
    for (i = 0; i != NN_MAX_SOCKETS; ++i)
        self.unused [i] = NN_MAX_SOCKETS - i - 1;

    /*  Initialise other parts of the global state. */
    nn_list_init (&self.transports);
    nn_list_init (&self.socktypes);

    /*  Plug in individual transports. */
    nn_global_add_transport (nn_inproc);
    nn_global_add_transport (nn_ipc);
    nn_global_add_transport (nn_tcp);

    /*  Plug in individual socktypes. */
    nn_global_add_socktype (nn_pair_socktype);
    nn_global_add_socktype (nn_xpair_socktype);
    nn_global_add_socktype (nn_pub_socktype);
    nn_global_add_socktype (nn_sub_socktype);
    nn_global_add_socktype (nn_xpub_socktype);
    nn_global_add_socktype (nn_xsub_socktype);
    nn_global_add_socktype (nn_rep_socktype);
    nn_global_add_socktype (nn_req_socktype);
    nn_global_add_socktype (nn_xrep_socktype);
    nn_global_add_socktype (nn_xreq_socktype);
    nn_global_add_socktype (nn_push_socktype);
    nn_global_add_socktype (nn_xpush_socktype);
    nn_global_add_socktype (nn_pull_socktype);
    nn_global_add_socktype (nn_xpull_socktype);
    nn_global_add_socktype (nn_respondent_socktype);
    nn_global_add_socktype (nn_surveyor_socktype);
    nn_global_add_socktype (nn_xrespondent_socktype);
    nn_global_add_socktype (nn_xsurveyor_socktype);
    nn_global_add_socktype (nn_bus_socktype);
    nn_global_add_socktype (nn_xbus_socktype);

    /*  Start the worker threads. */
    nn_pool_init (&self.pool);

    /*  Start FSM  */
    nn_fsm_init_root (&self.fsm, nn_global_handler, nn_global_shutdown,
        &self.ctx);
    self.state = NN_GLOBAL_STATE_IDLE;

    nn_ctx_init (&self.ctx, nn_global_getpool (), NULL);
    nn_timer_init (&self.stat_timer, NN_GLOBAL_SRC_STAT_TIMER, &self.fsm);
    nn_fsm_start (&self.fsm);

    /*   Initializing special sockets.  */
    addr = getenv ("NN_STATISTICS_SOCKET");
    if (addr) {
        self.statistics_socket = nn_global_create_socket (AF_SP, NN_PUB);
        errno_assert (self.statistics_socket >= 0);

        rc = nn_global_create_ep (self.statistics_socket, addr, 0);
        errno_assert (rc >= 0);
    } else {
        self.statistics_socket = -1;
    }

    addr = getenv ("NN_APPLICATION_NAME");
    if (addr) {
        strncpy (self.appname, addr, 63);
        self.appname[63] = '\0';
    } else {
        /*  No cross-platform way to find out application binary.
            Also, MSVC suggests using _getpid() instead of getpid(),
            however, it's not clear whether the former is supported
            by older versions of Windows/MSVC. */
#if defined _MSC_VER
#pragma warning (push)
#pragma warning (disable:4996)
#endif
        sprintf (self.appname, "nanomsg.%d", getpid());
#if defined _MSC_VER
#pragma warning (pop)
#endif
    }

    addr = getenv ("NN_HOSTNAME");
    if (addr) {
        strncpy (self.hostname, addr, 63);
        self.hostname[63] = '\0';
    } else {
        /*  No cross-platform way to find out application binary  */
        rc = gethostname (self.hostname, 63);
        errno_assert (rc == 0);
        self.hostname[63] = '\0';
    }
}

static void nn_global_term (void)
{
#if defined NN_HAVE_WINDOWS
    int rc;
#endif
    struct nn_list_item *it;
    struct nn_transport *tp;

    /*  If there are no sockets remaining, uninitialise the global context. */
    nn_assert (self.socks);
    if (self.nsocks > 0)
        return;

    /*  Stop the FSM  */
    nn_ctx_enter (&self.ctx);
    nn_fsm_stop (&self.fsm);
    nn_ctx_leave (&self.ctx);

    /*  Shut down the worker threads. */
    nn_pool_term (&self.pool);

    /* Terminate ctx mutex */
    nn_ctx_term (&self.ctx);

    /*  Ask all the transport to deallocate their global resources. */
    while (!nn_list_empty (&self.transports)) {
        it = nn_list_begin (&self.transports);
        tp = nn_cont (it, struct nn_transport, item);
        if (tp->term)
            tp->term ();
        nn_list_erase (&self.transports, it);
    }

    /*  For now there's nothing to deallocate about socket types, however,
        let's remove them from the list anyway. */
    while (!nn_list_empty (&self.socktypes))
        nn_list_erase (&self.socktypes, nn_list_begin (&self.socktypes));

    /*  Final deallocation of the nn_global object itself. */
    nn_list_term (&self.socktypes);
    nn_list_term (&self.transports);
    nn_free (self.socks);

    /*  This marks the global state as uninitialised. */
    self.socks = NULL;

    /*  Shut down the memory allocation subsystem. */
    nn_alloc_term ();

    /*  On Windows, uninitialise the socket library. */
#if defined NN_HAVE_WINDOWS
    rc = WSACleanup ();
    nn_assert (rc == 0);
#endif
}

void nn_term (void)
{
    int i;

    nn_glock_lock ();

    /*  Switch the global state into the zombie state. */
    self.flags |= NN_CTX_FLAG_ZOMBIE;

    /*  Mark all open sockets as terminating. */
    if (self.socks && self.nsocks) {
        for (i = 0; i != NN_MAX_SOCKETS; ++i)
            if (self.socks [i])
                nn_sock_zombify (self.socks [i]);
    }

    nn_glock_unlock ();
}

void *nn_allocmsg (size_t size, int type)
{
    int rc;
    void *result;

    rc = nn_chunk_alloc (size, type, &result);
    if (rc == 0)
        return result;
    errno = -rc;
    return NULL;
}

void *nn_reallocmsg (void *msg, size_t size)
{
    int rc;

    rc = nn_chunk_realloc (size, &msg);
    if (rc == 0)
        return msg;
    errno = -rc;
    return NULL;
}

int nn_freemsg (void *msg)
{
    nn_chunk_free (msg);
    return 0;
}

int nn_global_create_socket (int domain, int protocol)
{
    int rc;
    int s;
    struct nn_list_item *it;
    struct nn_socktype *socktype;
    struct nn_sock *sock;
    /* The function is called with nn_glock held */

    /*  Only AF_SP and AF_SP_RAW domains are supported. */
    if (nn_slow (domain != AF_SP && domain != AF_SP_RAW)) {
        return -EAFNOSUPPORT;
    }

    /*  If socket limit was reached, report error. */
    if (nn_slow (self.nsocks >= NN_MAX_SOCKETS)) {
        return -EMFILE;
    }

    /*  Find an empty socket slot. */
    s = self.unused [NN_MAX_SOCKETS - self.nsocks - 1];

    /*  Find the appropriate socket type. */
    for (it = nn_list_begin (&self.socktypes);
          it != nn_list_end (&self.socktypes);
          it = nn_list_next (&self.socktypes, it)) {
        socktype = nn_cont (it, struct nn_socktype, item);
        if (socktype->domain == domain && socktype->protocol == protocol) {

            /*  Instantiate the socket. */
            sock = nn_alloc (sizeof (struct nn_sock), "sock");
            alloc_assert (sock);
            rc = nn_sock_init (sock, socktype, s);
            if (rc < 0)
                return rc;

            /*  Adjust the global socket table. */
            self.socks [s] = sock;
            ++self.nsocks;
            return s;
        }
    }
    /*  Specified socket type wasn't found. */
    return -EINVAL;
}

int nn_socket (int domain, int protocol)
{
    int rc;

    nn_glock_lock ();

    /*  If nn_term() was already called, return ETERM. */
    if (nn_slow (self.flags & NN_CTX_FLAG_ZOMBIE)) {
        nn_glock_unlock ();
        errno = ETERM;
        return -1;
    }

    /*  Make sure that global state is initialised. */
    nn_global_init ();

    rc = nn_global_create_socket (domain, protocol);

    if(rc < 0) {
        nn_global_term ();
        nn_glock_unlock ();
        errno = -rc;
        return -1;
    }

    nn_glock_unlock();

    return rc;
}

int nn_close (int s)
{
    int rc;

    NN_BASIC_CHECKS;

    /*  TODO: nn_sock_term can take a long time to accomplish. It should not be
        performed under global critical section. */
    nn_glock_lock ();

    /*  Deallocate the socket object. */
    rc = nn_sock_term (self.socks [s]);
    if (nn_slow (rc == -EINTR)) {
        nn_glock_unlock ();
        errno = EINTR;
        return -1;
    }

    /*  Remove the socket from the socket table, add it to unused socket
        table. */
    nn_free (self.socks [s]);
    self.socks [s] = NULL;
    self.unused [NN_MAX_SOCKETS - self.nsocks] = s;
    --self.nsocks;

    /*  Destroy the global context if there's no socket remaining. */
    nn_global_term ();

    nn_glock_unlock ();

    return 0;
}

int nn_setsockopt (int s, int level, int option, const void *optval,
    size_t optvallen)
{
    int rc;

    NN_BASIC_CHECKS;

    if (nn_slow (!optval && optvallen)) {
        errno = EFAULT;
        return -1;
    }

    rc = nn_sock_setopt (self.socks [s], level, option, optval, optvallen);
    if (nn_slow (rc < 0)) {
        errno = -rc;
        return -1;
    }
    errnum_assert (rc == 0, -rc);

    return 0;
}

int nn_getsockopt (int s, int level, int option, void *optval,
    size_t *optvallen)
{
    int rc;

    NN_BASIC_CHECKS;

    if (nn_slow (!optval && optvallen)) {
        errno = EFAULT;
        return -1;
    }

    rc = nn_sock_getopt (self.socks [s], level, option, optval, optvallen);
    if (nn_slow (rc < 0)) {
        errno = -rc;
        return -1;
    }
    errnum_assert (rc == 0, -rc);

    return 0;
}

int nn_bind (int s, const char *addr)
{
    int rc;

    NN_BASIC_CHECKS;

    nn_glock_lock();
    rc = nn_global_create_ep (s, addr, 1);
    nn_glock_unlock();
    if (rc < 0) {
        errno = -rc;
        return -1;
    }

    return rc;
}

int nn_connect (int s, const char *addr)
{
    int rc;

    NN_BASIC_CHECKS;

    nn_glock_lock();
    rc = nn_global_create_ep (s, addr, 0);
    nn_glock_unlock();
    if (rc < 0) {
        errno = -rc;
        return -1;
    }

    return rc;
}

int nn_shutdown (int s, int how)
{
    int rc;

    NN_BASIC_CHECKS;

    rc = nn_sock_rm_ep (self.socks [s], how);
    if (nn_slow (rc < 0)) {
        errno = -rc;
        return -1;
    }
    nn_assert (rc == 0);

    return 0;
}

int nn_send (int s, const void *buf, size_t len, int flags)
{
    struct nn_iovec iov;
    struct nn_msghdr hdr;

    iov.iov_base = (void*) buf;
    iov.iov_len = len;

    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;
    hdr.msg_control = NULL;
    hdr.msg_controllen = 0;

    return nn_sendmsg (s, &hdr, flags);
}

int nn_recv (int s, void *buf, size_t len, int flags)
{
    struct nn_iovec iov;
    struct nn_msghdr hdr;

    iov.iov_base = buf;
    iov.iov_len = len;

    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;
    hdr.msg_control = NULL;
    hdr.msg_controllen = 0;

    return nn_recvmsg (s, &hdr, flags);
}

int nn_sendmsg (int s, const struct nn_msghdr *msghdr, int flags)
{
    int rc;
    size_t sz;
    int i;
    struct nn_iovec *iov;
    struct nn_msg msg;
    void *chunk;
    int nnmsg;

    NN_BASIC_CHECKS;

    if (nn_slow (!msghdr)) {
        errno = EINVAL;
        return -1;
    }

    if (nn_slow (msghdr->msg_iovlen < 0)) {
        errno = EMSGSIZE;
        return -1;
    }

    if (msghdr->msg_iovlen == 1 && msghdr->msg_iov [0].iov_len == NN_MSG) {
        chunk = *(void**) msghdr->msg_iov [0].iov_base;
        if (nn_slow (chunk == NULL)) {
            errno = EFAULT;
            return -1;
        }
        sz = nn_chunk_size (chunk);
        nn_msg_init_chunk (&msg, chunk);
        nnmsg = 1;
    }
    else {

        /*  Compute the total size of the message. */
        sz = 0;
        for (i = 0; i != msghdr->msg_iovlen; ++i) {
            iov = &msghdr->msg_iov [i];
            if (nn_slow (iov->iov_len == NN_MSG)) {
               errno = EINVAL;
               return -1;
            }
            if (nn_slow (!iov->iov_base && iov->iov_len)) {
                errno = EFAULT;
                return -1;
            }
            if (nn_slow (sz + iov->iov_len < sz)) {
                errno = EINVAL;
                return -1;
            }
            sz += iov->iov_len;
        }

        /*  Create a message object from the supplied scatter array. */
        nn_msg_init (&msg, sz);
        sz = 0;
        for (i = 0; i != msghdr->msg_iovlen; ++i) {
            iov = &msghdr->msg_iov [i];
            memcpy (((uint8_t*) nn_chunkref_data (&msg.body)) + sz,
                iov->iov_base, iov->iov_len);
            sz += iov->iov_len;
        }

        nnmsg = 0;
    }

    /*  Add ancillary data to the message. */
    if (msghdr->msg_control) {
        if (msghdr->msg_controllen == NN_MSG) {
            chunk = *((void**) msghdr->msg_control);
            nn_chunkref_term (&msg.hdr);
            nn_chunkref_init_chunk (&msg.hdr, chunk);
        }
        else {

            /*  TODO: Copy the control data to the message. */
            nn_assert (0);
        }
    }

    /*  Send it further down the stack. */
    rc = nn_sock_send (self.socks [s], &msg, flags);
    if (nn_slow (rc < 0)) {

        /*  If we are dealing with user-supplied buffer, detach it from
            the message object. */
        if (nnmsg)
            nn_chunkref_init (&msg.body, 0);

        nn_msg_term (&msg);
        errno = -rc;
        return -1;
    }

    /*  Adjust the statistics. */
    nn_sock_stat_increment (self.socks [s], NN_STAT_MESSAGES_SENT, 1);
    nn_sock_stat_increment (self.socks [s], NN_STAT_BYTES_SENT, sz);

    return (int) sz;
}

int nn_recvmsg (int s, struct nn_msghdr *msghdr, int flags)
{
    int rc;
    struct nn_msg msg;
    uint8_t *data;
    size_t sz;
    int i;
    struct nn_iovec *iov;
    void *chunk;

    NN_BASIC_CHECKS;

    if (nn_slow (!msghdr)) {
        errno = EINVAL;
        return -1;
    }

    if (nn_slow (msghdr->msg_iovlen < 0)) {
        errno = EMSGSIZE;
        return -1;
    }

    /*  Get a message. */
    rc = nn_sock_recv (self.socks [s], &msg, flags);
    if (nn_slow (rc < 0)) {
        errno = -rc;
        return -1;
    }

    if (msghdr->msg_iovlen == 1 && msghdr->msg_iov [0].iov_len == NN_MSG) {
        chunk = nn_chunkref_getchunk (&msg.body);
        *(void**) (msghdr->msg_iov [0].iov_base) = chunk;
        sz = nn_chunk_size (chunk);
    }
    else {

        /*  Copy the message content into the supplied gather array. */
        data = nn_chunkref_data (&msg.body);
        sz = nn_chunkref_size (&msg.body);
        for (i = 0; i != msghdr->msg_iovlen; ++i) {
            iov = &msghdr->msg_iov [i];
            if (nn_slow (iov->iov_len == NN_MSG)) {
                nn_msg_term (&msg);
                errno = EINVAL;
                return -1;
            }
            if (iov->iov_len > sz) {
                memcpy (iov->iov_base, data, sz);
                break;
            }
            memcpy (iov->iov_base, data, iov->iov_len);
            data += iov->iov_len;
            sz -= iov->iov_len;
        }
        sz = nn_chunkref_size (&msg.body);
    }

    /*  Retrieve the ancillary data from the message. */
    if (msghdr->msg_control) {
        if (msghdr->msg_controllen == NN_MSG) {
            chunk = nn_chunkref_getchunk (&msg.hdr);
            *((void**) msghdr->msg_control) = chunk;
        }
        else {

            /*  TODO: Copy the data to the supplied buffer, prefix them
                with size. */
            nn_assert (0);
        }
    }

    nn_msg_term (&msg);

    return (int) sz;
}

static void nn_global_add_transport (struct nn_transport *transport)
{
    if (transport->init)
        transport->init ();
    nn_list_insert (&self.transports, &transport->item,
        nn_list_end (&self.transports));

}

static void nn_global_add_socktype (struct nn_socktype *socktype)
{
    nn_list_insert (&self.socktypes, &socktype->item,
        nn_list_end (&self.socktypes));
}

static void nn_global_submit_counter (int i, struct nn_sock *s,
    char *name, uint64_t value)
{
    /* Length of buffer is:
       len(hostname) + len(appname) + len(socket_name) + len(timebuf)
       + len(str(value)) + len(static characters)
       63 + 63 + 63 + 20 + 20 + 60 = 289 */
    char buf[512];
    char timebuf[20];
    time_t numtime;
    struct tm strtime;
    int len;

    if(self.print_statistics) {
        fprintf(stderr, "nanomsg: socket.%s: %s: %llu\n",
            s->socket_name, name, (long long unsigned int)value);
    }

    if (self.statistics_socket >= 0) {
        /*  TODO(tailhook) add HAVE_GMTIME_R ifdef  */
        time(&numtime);
#ifdef NN_HAVE_GMTIME_R
        gmtime_r (&numtime, &strtime);
#else
#error
#endif
        strftime (timebuf, 20, "%Y-%m-%dT%H:%M:%S", &strtime);
        if(*s->socket_name) {
            len = sprintf (buf, "ESTP:%s:%s:socket.%s:%s: %sZ 10 %llu:c",
                self.hostname, self.appname, s->socket_name, name,
                timebuf, (long long unsigned int)value);
        } else {
            len = sprintf (buf, "ESTP:%s:%s:socket.%d:%s: %sZ 10 %llu:c",
                self.hostname, self.appname, i, name,
                timebuf, (long long unsigned int)value);
        }
        nn_assert (len < (int)sizeof(buf));
        (void) nn_send (self.statistics_socket, buf, len, NN_DONTWAIT);
    }
}

static void nn_global_submit_level (int i, struct nn_sock *s,
    char *name, int value)
{
    /* Length of buffer is:
       len(hostname) + len(appname) + len(socket_name) + len(timebuf)
       + len(str(value)) + len(static characters)
       63 + 63 + 63 + 20 + 20 + 60 = 289 */
    char buf[512];
    char timebuf[20];
    time_t numtime;
    struct tm strtime;
    int len;

    if(self.print_statistics) {
        fprintf(stderr, "nanomsg: socket.%s: %s: %d\n",
            s->socket_name, name, value);
    }

    if (self.statistics_socket >= 0) {
        /*  TODO(tailhook) add HAVE_GMTIME_R ifdef  */
        time(&numtime);
#ifdef NN_HAVE_GMTIME_R
        gmtime_r (&numtime, &strtime);
#else
#error
#endif
        strftime (timebuf, 20, "%Y-%m-%dT%H:%M:%S", &strtime);
        if(*s->socket_name) {
            len = sprintf (buf, "ESTP:%s:%s:socket.%s:%s: %sZ 10 %d",
                self.hostname, self.appname, s->socket_name, name,
                timebuf, value);
        } else {
            len = sprintf (buf, "ESTP:%s:%s:socket.%d:%s: %sZ 10 %d",
                self.hostname, self.appname, i, name,
                timebuf, value);
        }
        nn_assert (len < (int)sizeof(buf));
        (void) nn_send (self.statistics_socket, buf, len, NN_DONTWAIT);
    }
}

static void nn_global_submit_errors (int i, struct nn_sock *s,
    char *name, int value)
{
    /*  TODO(tailhook) dynamically allocate buffer  */
    char buf[4096];
    char *curbuf;
    int buf_left;
    char timebuf[20];
    time_t numtime;
    struct tm strtime;
    int len;
    struct nn_list_item *it;
    struct nn_ep *ep;

    if (self.statistics_socket >= 0) {
        /*  TODO(tailhook) add HAVE_GMTIME_R ifdef  */
        time(&numtime);
#ifdef NN_HAVE_GMTIME_R
        gmtime_r (&numtime, &strtime);
#else
#error
#endif
        strftime (timebuf, 20, "%Y-%m-%dT%H:%M:%S", &strtime);
        if(*s->socket_name) {
            len = sprintf (buf, "ESTP:%s:%s:socket.%s:%s: %sZ 10 %d\n",
                self.hostname, self.appname, s->socket_name, name,
                timebuf, value);
        } else {
            len = sprintf (buf, "ESTP:%s:%s:socket.%d:%s: %sZ 10 %d\n",
                self.hostname, self.appname, i, name,
                timebuf, value);
        }
        buf_left = sizeof(buf) - len;
        curbuf = buf + len;


        for (it = nn_list_begin (&s->eps);
              it != nn_list_end (&s->eps);
              it = nn_list_next (&s->eps, it)) {
            ep = nn_cont (it, struct nn_ep, item);

            if (ep->last_errno) {
#ifdef NN_HAVE_WINDOWS
                len = _snprintf_s (curbuf, buf_left, _TRUNCATE,
                    " nanomsg: Endpoint %d [%s] error: %s\n",
                    ep->eid, nn_ep_getaddr (ep), nn_strerror (ep->last_errno));
#else
                 len = snprintf (curbuf, buf_left,
                     " nanomsg: Endpoint %d [%s] error: %s\n",
                     ep->eid, nn_ep_getaddr (ep), nn_strerror (ep->last_errno));
#endif
                if (buf_left < len)
                    break;
                curbuf += len;
                buf_left -= len;
            }

        }

        (void) nn_send (self.statistics_socket,
            buf, sizeof(buf) - buf_left, NN_DONTWAIT);
    }
}

static void nn_global_submit_statistics () {
    int i;

    /*  TODO(tailhook)  optimized it to use nsocks and unused  */
    for(i = 0; i < NN_MAX_SOCKETS; ++i) {
        struct nn_sock *s = self.socks [i];
        if (!s)
            continue;
        if (i == self.statistics_socket)
            continue;
        nn_ctx_enter (&s->ctx);
        nn_global_submit_counter (i, s,
            "established_connections", s->statistics.established_connections);
        nn_global_submit_counter (i, s,
            "accepted_connections", s->statistics.accepted_connections);
        nn_global_submit_counter (i, s,
            "dropped_connections", s->statistics.dropped_connections);
        nn_global_submit_counter (i, s,
            "broken_connections", s->statistics.broken_connections);
        nn_global_submit_counter (i, s,
            "connect_errors", s->statistics.connect_errors);
        nn_global_submit_counter (i, s,
            "bind_errors", s->statistics.bind_errors);
        nn_global_submit_counter (i, s,
            "accept_errors", s->statistics.accept_errors);
        nn_global_submit_counter (i, s,
            "messages_sent", s->statistics.messages_sent);
        nn_global_submit_counter (i, s,
            "messages_received", s->statistics.messages_received);
        nn_global_submit_counter (i, s,
            "bytes_sent", s->statistics.bytes_sent);
        nn_global_submit_counter (i, s,
            "bytes_received", s->statistics.bytes_received);
        nn_global_submit_level (i, s,
            "current_connections", s->statistics.current_connections);
        nn_global_submit_level (i, s,
            "inprogress_connections", s->statistics.inprogress_connections);
        nn_global_submit_level (i, s,
            "current_snd_priority", s->statistics.current_snd_priority);
        nn_global_submit_errors (i, s,
            "current_ep_errors", s->statistics.current_ep_errors);
        nn_ctx_leave (&s->ctx);
    }
}

static int nn_global_create_ep (int s, const char *addr, int bind)
{
    int rc;
    const char *proto;
    const char *delim;
    size_t protosz;
    struct nn_transport *tp;
    struct nn_list_item *it;

    /*  Check whether address is valid. */
    if (!addr)
        return -EINVAL;
    if (strlen (addr) >= NN_SOCKADDR_MAX)
        return -ENAMETOOLONG;

    /*  Separate the protocol and the actual address. */
    proto = addr;
    delim = strchr (addr, ':');
    if (!delim)
        return -EINVAL;
    if (delim [1] != '/' || delim [2] != '/')
        return -EINVAL;
    protosz = delim - addr;
    addr += protosz + 3;

    /*  Find the specified protocol. */
    tp = NULL;
    for (it = nn_list_begin (&self.transports);
          it != nn_list_end (&self.transports);
          it = nn_list_next (&self.transports, it)) {
        tp = nn_cont (it, struct nn_transport, item);
        if (strlen (tp->name) == protosz &&
              memcmp (tp->name, proto, protosz) == 0)
            break;
        tp = NULL;
    }

    /*  The protocol specified doesn't match any known protocol. */
    if (!tp) {
        return -EPROTONOSUPPORT;
    }

    /*  Ask the socket to create the endpoint. */
    rc = nn_sock_add_ep (self.socks [s], tp, bind, addr);
    return rc;
}

struct nn_transport *nn_global_transport (int id)
{
    struct nn_transport *tp;
    struct nn_list_item *it;

    /*  Find the specified protocol. */
    tp = NULL;
    nn_glock_lock ();
    for (it = nn_list_begin (&self.transports);
          it != nn_list_end (&self.transports);
          it = nn_list_next (&self.transports, it)) {
        tp = nn_cont (it, struct nn_transport, item);
        if (tp->id == id)
            break;
        tp = NULL;
    }
    nn_glock_unlock ();

    return tp;
}

struct nn_pool *nn_global_getpool ()
{
    return &self.pool;
}

static void nn_global_handler (struct nn_fsm *self,
    int src, int type, NN_UNUSED void *srcptr)
{

    struct nn_global *global;

    global = nn_cont (self, struct nn_global, fsm);

    switch (global->state) {

/******************************************************************************/
/*  IDLE state.                                                               */
/*  The state machine wasn't yet started.                                     */
/******************************************************************************/
    case NN_GLOBAL_STATE_IDLE:
        switch (src) {

        case NN_FSM_ACTION:
            switch (type) {
            case NN_FSM_START:
                global->state = NN_GLOBAL_STATE_ACTIVE;
                if (global->print_statistics || global->statistics_socket >= 0)
                {
                    /*  Start statistics collection timer. */
                    nn_timer_start (&global->stat_timer, 10000);
                }
                return;
            default:
                nn_fsm_bad_action (global->state, src, type);
            }

        default:
            nn_fsm_bad_source (global->state, src, type);
        }

/******************************************************************************/
/*  ACTIVE state.                                                             */
/*  Normal lifetime for global object.                                        */
/******************************************************************************/
    case NN_GLOBAL_STATE_ACTIVE:
        switch (src) {

        case NN_GLOBAL_SRC_STAT_TIMER:
            switch (type) {
            case NN_TIMER_TIMEOUT:
                nn_global_submit_statistics ();
                /*  No need to change state  */
                nn_timer_stop (&global->stat_timer);
                return;
            case NN_TIMER_STOPPED:
                nn_timer_start (&global->stat_timer, 10000);
                return;
            default:
                nn_fsm_bad_action (global->state, src, type);
            }

        default:
            nn_fsm_bad_source (global->state, src, type);
        }

/******************************************************************************/
/*  Invalid state.                                                            */
/******************************************************************************/
    default:
        nn_fsm_bad_state (global->state, src, type);
    }
}

static void nn_global_shutdown (struct nn_fsm *self,
    NN_UNUSED int src, NN_UNUSED int type, NN_UNUSED void *srcptr)
{

    struct nn_global *global;

    global = nn_cont (self, struct nn_global, fsm);

    nn_assert (global->state == NN_GLOBAL_STATE_ACTIVE
        || global->state == NN_GLOBAL_STATE_IDLE);
    if (global->state == NN_GLOBAL_STATE_ACTIVE) {
        if (!nn_timer_isidle (&global->stat_timer)) {
            nn_timer_stop (&global->stat_timer);
            return;
        }
    }
}

int nn_global_print_errors () {
    return self.print_errors;
}
