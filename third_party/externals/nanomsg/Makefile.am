# Copyright (c) 2013 Luca Barbato
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom
# the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

ACLOCAL_AMFLAGS = -I m4

DISTCLEANFILES = @DOLT_CLEANFILES@

EXTRA_DIST =

################################################################################
#  nanomsg library                                                             #
################################################################################

nnincludedir = $(includedir)/nanomsg
nninclude_HEADERS = \
    src/nn.h \
    src/inproc.h \
    src/ipc.h \
    src/tcp.h \
    src/pair.h \
    src/pubsub.h \
    src/reqrep.h \
    src/pipeline.h \
    src/survey.h \
    src/bus.h

lib_LTLIBRARIES = libnanomsg.la

NANOMSG_CORE = \
    src/core/ep.h \
    src/core/ep.c \
    src/core/epbase.c \
    src/core/global.h \
    src/core/global.c \
    src/core/pipe.c \
    src/core/poll.c \
    src/core/sock.h \
    src/core/sock.c \
    src/core/sockbase.c \
    src/core/device.c \
    src/core/symbol.c

NANOMSG_AIO = \
    src/aio/ctx.h \
    src/aio/ctx.c \
    src/aio/fsm.h \
    src/aio/fsm.c \
    src/aio/poller.h \
    src/aio/poller.c \
    src/aio/poller_epoll.h \
    src/aio/poller_epoll.inc \
    src/aio/poller_kqueue.h \
    src/aio/poller_kqueue.inc \
    src/aio/poller_poll.h \
    src/aio/poller_poll.inc \
    src/aio/pool.h \
    src/aio/pool.c \
    src/aio/timer.h \
    src/aio/timer.c \
    src/aio/timerset.h \
    src/aio/timerset.c \
    src/aio/usock.h \
    src/aio/usock.c \
    src/aio/usock_posix.h \
    src/aio/usock_posix.inc \
    src/aio/usock_win.h \
    src/aio/usock_win.inc \
    src/aio/worker.h \
    src/aio/worker.c \
    src/aio/worker_posix.h \
    src/aio/worker_posix.inc \
    src/aio/worker_win.h \
    src/aio/worker_win.inc

NANOMSG_UTILS = \
    src/utils/alloc.h \
    src/utils/alloc.c \
    src/utils/atomic.h \
    src/utils/atomic.c \
    src/utils/attr.h \
    src/utils/chunk.h \
    src/utils/chunk.c \
    src/utils/chunkref.h \
    src/utils/chunkref.c \
    src/utils/clock.h \
    src/utils/clock.c \
    src/utils/closefd.h \
    src/utils/closefd.c \
    src/utils/cont.h \
    src/utils/efd.h \
    src/utils/efd.c \
    src/utils/efd_eventfd.h \
    src/utils/efd_eventfd.inc \
    src/utils/efd_pipe.h \
    src/utils/efd_pipe.inc \
    src/utils/efd_socketpair.h \
    src/utils/efd_socketpair.inc \
    src/utils/efd_win.h \
    src/utils/efd_win.inc \
    src/utils/err.h \
    src/utils/err.c \
    src/utils/fast.h \
    src/utils/fd.h \
    src/utils/glock.h \
    src/utils/glock.c \
    src/utils/hash.h \
    src/utils/hash.c \
    src/utils/int.h \
    src/utils/list.h \
    src/utils/list.c \
    src/utils/msg.h \
    src/utils/msg.c \
    src/utils/mutex.h \
    src/utils/mutex.c \
    src/utils/queue.h \
    src/utils/queue.c \
    src/utils/random.h \
    src/utils/random.c \
    src/utils/sem.h \
    src/utils/sem.c \
    src/utils/sleep.h \
    src/utils/sleep.c \
    src/utils/stopwatch.h \
    src/utils/stopwatch.c \
    src/utils/thread.h \
    src/utils/thread.c \
    src/utils/thread_posix.h \
    src/utils/thread_posix.inc \
    src/utils/thread_win.h \
    src/utils/thread_win.inc \
    src/utils/win.h \
    src/utils/wire.h \
    src/utils/wire.c

PROTOCOLS_UTILS = \
    src/protocols/utils/dist.h \
    src/protocols/utils/dist.c \
    src/protocols/utils/excl.h \
    src/protocols/utils/excl.c \
    src/protocols/utils/fq.h \
    src/protocols/utils/fq.c \
    src/protocols/utils/lb.h \
    src/protocols/utils/lb.c \
    src/protocols/utils/priolist.h \
    src/protocols/utils/priolist.c

PROTOCOLS_BUS = \
    src/protocols/bus/bus.h \
    src/protocols/bus/bus.c \
    src/protocols/bus/xbus.h \
    src/protocols/bus/xbus.c

PROTOCOLS_PIPELINE = \
    src/protocols/pipeline/push.h \
    src/protocols/pipeline/push.c \
    src/protocols/pipeline/pull.h \
    src/protocols/pipeline/pull.c \
    src/protocols/pipeline/xpull.h \
    src/protocols/pipeline/xpull.c \
    src/protocols/pipeline/xpush.h \
    src/protocols/pipeline/xpush.c

PROTOCOLS_PAIR = \
    src/protocols/pair/pair.h \
    src/protocols/pair/pair.c \
    src/protocols/pair/xpair.h \
    src/protocols/pair/xpair.c

PROTOCOLS_PUBSUB = \
    src/protocols/pubsub/pub.h \
    src/protocols/pubsub/pub.c \
    src/protocols/pubsub/sub.h \
    src/protocols/pubsub/sub.c \
    src/protocols/pubsub/trie.h \
    src/protocols/pubsub/trie.c \
    src/protocols/pubsub/xpub.h \
    src/protocols/pubsub/xpub.c \
    src/protocols/pubsub/xsub.h \
    src/protocols/pubsub/xsub.c

PROTOCOLS_REQREP = \
    src/protocols/reqrep/req.h \
    src/protocols/reqrep/req.c \
    src/protocols/reqrep/rep.h \
    src/protocols/reqrep/rep.c \
    src/protocols/reqrep/xrep.h \
    src/protocols/reqrep/xrep.c \
    src/protocols/reqrep/xreq.h \
    src/protocols/reqrep/xreq.c

PROTOCOLS_SURVEY = \
    src/protocols/survey/respondent.h \
    src/protocols/survey/respondent.c \
    src/protocols/survey/surveyor.h \
    src/protocols/survey/surveyor.c \
    src/protocols/survey/xrespondent.h \
    src/protocols/survey/xrespondent.c \
    src/protocols/survey/xsurveyor.h \
    src/protocols/survey/xsurveyor.c

NANOMSG_PROTOCOLS = \
    $(PROTOCOLS_BUS) \
    $(PROTOCOLS_PIPELINE) \
    $(PROTOCOLS_PAIR) \
    $(PROTOCOLS_PUBSUB) \
    $(PROTOCOLS_REQREP) \
    $(PROTOCOLS_SURVEY) \
    $(PROTOCOLS_UTILS) \
    $(PROTOCOLS_INPROC) \
    $(PROTOCOLS_IPC) \
    $(PROTOCOLS_TCP)


TRANSPORTS_UTILS = \
    src/transports/utils/backoff.h \
    src/transports/utils/backoff.c \
    src/transports/utils/dns.h \
    src/transports/utils/dns.c \
    src/transports/utils/dns_getaddrinfo.h \
    src/transports/utils/dns_getaddrinfo.inc \
    src/transports/utils/dns_getaddrinfo_a.h \
    src/transports/utils/dns_getaddrinfo_a.inc \
    src/transports/utils/iface.h \
    src/transports/utils/iface.c \
    src/transports/utils/literal.h \
    src/transports/utils/literal.c \
    src/transports/utils/port.h \
    src/transports/utils/port.c \
    src/transports/utils/streamhdr.h \
    src/transports/utils/streamhdr.c

TRANSPORTS_INPROC = \
    src/transports/inproc/binproc.h \
    src/transports/inproc/binproc.c \
    src/transports/inproc/cinproc.h \
    src/transports/inproc/cinproc.c \
    src/transports/inproc/inproc.h \
    src/transports/inproc/inproc.c \
    src/transports/inproc/ins.h \
    src/transports/inproc/ins.c \
    src/transports/inproc/msgqueue.h \
    src/transports/inproc/msgqueue.c \
    src/transports/inproc/sinproc.h \
    src/transports/inproc/sinproc.c

TRANSPORTS_IPC = \
    src/transports/ipc/aipc.h \
    src/transports/ipc/aipc.c \
    src/transports/ipc/bipc.h \
    src/transports/ipc/bipc.c \
    src/transports/ipc/cipc.h \
    src/transports/ipc/cipc.c \
    src/transports/ipc/ipc.h \
    src/transports/ipc/ipc.c \
    src/transports/ipc/sipc.h \
    src/transports/ipc/sipc.c

TRANSPORTS_TCP = \
    src/transports/tcp/atcp.h \
    src/transports/tcp/atcp.c \
    src/transports/tcp/btcp.h \
    src/transports/tcp/btcp.c \
    src/transports/tcp/ctcp.h \
    src/transports/tcp/ctcp.c \
    src/transports/tcp/stcp.h \
    src/transports/tcp/stcp.c \
    src/transports/tcp/tcp.h \
    src/transports/tcp/tcp.c

NANOMSG_TRANSPORTS = \
    $(TRANSPORTS_UTILS) \
    $(TRANSPORTS_INPROC) \
    $(TRANSPORTS_IPC) \
    $(TRANSPORTS_TCP)

libnanomsg_la_SOURCES = \
    src/transport.h \
    src/protocol.h \
    $(NANOMSG_CORE) \
    $(NANOMSG_AIO) \
    $(NANOMSG_UTILS) \
    $(NANOMSG_PROTOCOLS) \
    $(NANOMSG_TRANSPORTS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnanomsg.pc

libnanomsg_la_LDFLAGS = -no-undefined -version-info @NN_LIBTOOL_VERSION@
libnanomsg_la_CFLAGS = -DNN_EXPORTS

################################################################################
#  documentation                                                               #
################################################################################

#  Source files.

MAN7 = \
    doc/nanomsg.txt \
    doc/nn_pair.txt \
    doc/nn_reqrep.txt \
    doc/nn_pubsub.txt \
    doc/nn_survey.txt \
    doc/nn_pipeline.txt \
    doc/nn_bus.txt \
    doc/nn_inproc.txt \
    doc/nn_ipc.txt \
    doc/nn_tcp.txt \
    doc/nn_env.txt

MAN3 = \
    doc/nn_errno.txt \
    doc/nn_strerror.txt \
    doc/nn_symbol.txt \
    doc/nn_symbol_info.txt \
    doc/nn_term.txt \
    doc/nn_allocmsg.txt \
    doc/nn_reallocmsg.txt \
    doc/nn_freemsg.txt \
    doc/nn_socket.txt \
    doc/nn_close.txt \
    doc/nn_getsockopt.txt \
    doc/nn_setsockopt.txt \
    doc/nn_bind.txt \
    doc/nn_connect.txt \
    doc/nn_shutdown.txt \
    doc/nn_send.txt \
    doc/nn_recv.txt \
    doc/nn_sendmsg.txt \
    doc/nn_recvmsg.txt \
    doc/nn_device.txt \
    doc/nn_cmsg.txt \
    doc/nn_poll.txt

MAN1 = \
    doc/nanocat.txt

if DOC

#  Build instructions.

.txt.xml:
	$(AM_V_GEN)$(ASCIIDOC) -d manpage -b docbook \
        -f $(abs_srcdir)/doc/asciidoc.conf \
        -aversion=@NN_PACKAGE_VERSION@ -o $@ $<

SUFFIXES = .1 .3 .7 .1.html .3.html .7.html
.xml.1:
	$(AM_V_GEN)$(XMLTO) -o doc man $<
.xml.3:
	$(AM_V_GEN)$(XMLTO) -o doc man $<
.xml.7:
	$(AM_V_GEN)$(XMLTO) -o doc man $<

.txt.1.html:
	$(AM_V_GEN)$(ASCIIDOC) -d manpage -b xhtml11 \
        -a stylesheet=$(abs_srcdir)/doc/htmldoc.css \
        -f $(abs_srcdir)/doc/asciidoc.conf \
        -aversion=@NN_PACKAGE_VERSION@ -o $@ $<

.txt.3.html:
	$(AM_V_GEN)$(ASCIIDOC) -d manpage -b xhtml11 \
        -a stylesheet=$(abs_srcdir)/doc/htmldoc.css \
        -f $(abs_srcdir)/doc/asciidoc.conf \
        -aversion=@NN_PACKAGE_VERSION@ -o $@ $<

.txt.7.html:
	$(AM_V_GEN)$(ASCIIDOC) -d manpage -b xhtml11 \
        -a stylesheet=$(abs_srcdir)/doc/htmldoc.css \
        -f $(abs_srcdir)/doc/asciidoc.conf \
        -aversion=@NN_PACKAGE_VERSION@ -o $@ $<

#  Cause man pages to be generated and installed.

dist_man1_MANS = $(MAN1:%.txt=%.1)
dist_man3_MANS = $(MAN3:%.txt=%.3)
dist_man7_MANS = $(MAN7:%.txt=%.7)

#  Cause HTML docs to be generated.

doc_DATA = $(MAN1:%.txt=%.1.html) $(MAN3:%.txt=%.3.html) $(MAN7:%.txt=%.7.html)

#  Extra files to be cleaned.

CLEANFILES = $(MAN1:%.txt=%.1) $(MAN3:%.txt=%.3) $(MAN7:%.txt=%.7) \
    $(MAN1:%.txt=%.1.html) $(MAN3:%.txt=%.3.html) $(MAN7:%.txt=%.7.html)

endif

#  Extra files to be included into the source package.

EXTRA_DIST += doc/asciidoc.conf doc/htmldoc.css $(MAN1) $(MAN3) $(MAN7)

################################################################################
#  performance tests                                                           #
################################################################################

noinst_PROGRAMS = \
    perf/inproc_lat \
    perf/inproc_thr \
    perf/local_lat \
    perf/remote_lat \
    perf/local_thr \
    perf/remote_thr

LDADD = libnanomsg.la

################################################################################
#  automated tests                                                             #
################################################################################

TRANSPORT_TESTS = \
    tests/inproc \
    tests/inproc_shutdown \
    tests/ipc \
    tests/ipc_shutdown \
    tests/tcp \
    tests/tcp_shutdown

PROTOCOL_TESTS = \
    tests/pair \
    tests/pubsub \
    tests/reqrep \
    tests/pipeline \
    tests/survey \
    tests/bus

FEATURE_TESTS = \
    tests/block \
    tests/term \
    tests/timeo \
    tests/iovec \
    tests/msg \
    tests/prio \
    tests/poll \
    tests/device \
    tests/emfile \
    tests/domain \
    tests/trie \
    tests/list \
    tests/hash \
    tests/symbol \
    tests/separation \
    tests/zerocopy \
    tests/shutdown

EXTRA_DIST += tests/testutil.h

check_PROGRAMS = \
    $(TRANSPORT_TESTS) \
    $(PROTOCOL_TESTS) \
    $(FEATURE_TESTS)

TESTS = $(check_PROGRAMS)


################################################################################
#  tools                                                                       #
################################################################################

bin_PROGRAMS =

nanocat_SOURCES = \
	tools/nanocat.c \
	tools/options.c \
	tools/options.h

if NANOCAT
bin_PROGRAMS += nanocat
endif NANOCAT

################################################################################
#  RFCs                                                                        #
################################################################################

EXTRA_DIST += \
    ./rfc/sp-protocol-ids-01.txt \
    ./rfc/sp-tcp-mapping-01.txt \
    ./rfc/sp-udp-mapping-01.txt \
    ./rfc/sp-tls-mapping-01.txt \
    ./rfc/sp-request-reply-01.txt

################################################################################
#  additional packaging-related stuff                                          #
################################################################################

#  Generate ChangeLog file from git.
#  Also, there's no git availabe when building from the source package and
#  thus no way to obtain package version. Therefore, package version is
#  saved into a file when building a source package.
dist-hook:
	@if test -d "$(srcdir)/.git"; \
	then \
		echo Creating ChangeLog; \
		cd "$(top_srcdir)"; \
		(echo '# Generated by Makefile. Do not edit.'; echo; \
		  ./missing --run git log --decorate ) > ChangeLog.tmp; \
		mv -f ChangeLog.tmp $(top_distdir)/ChangeLog; \
		rm -f ChangeLog.tmp; \
    else \
        cp -f ChangeLog $(top_distdir)/ChangeLog || \
        echo Failed to generate ChangeLog >&2; \
	fi; \
	$(srcdir)/package_version.sh > $(distdir)/.version


if NANOCAT
if SYMLINKS

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
          $(LN_S) -f nanocat nn_push$(EXEEXT) && \
          $(LN_S) -f nanocat nn_pull$(EXEEXT) && \
          $(LN_S) -f nanocat nn_pub$(EXEEXT) && \
          $(LN_S) -f nanocat nn_sub$(EXEEXT) && \
          $(LN_S) -f nanocat nn_req$(EXEEXT) && \
          $(LN_S) -f nanocat nn_rep$(EXEEXT) && \
          $(LN_S) -f nanocat nn_surveyor$(EXEEXT) && \
          $(LN_S) -f nanocat nn_respondent$(EXEEXT) && \
          $(LN_S) -f nanocat nn_bus$(EXEEXT) && \
          $(LN_S) -f nanocat nn_pair$(EXEEXT)

endif SYMLINKS
endif NANOCAT


EXTRA_DIST += \
    ./abi_version.sh \
    ./package_version.sh \
    ./src/CMakeLists.txt \
    ./CMakeLists.txt

diagrams:
	-mkdir -p doc/diagrams
	./doc/diag.py $(DEFS) $(CPPFLAGS) \
		$(shell clang -E - -v < /dev/null 2>&1 | \
			sed -n '/#include </{:x;n;/^End/q;s/^ */-I/;p;bx}')
