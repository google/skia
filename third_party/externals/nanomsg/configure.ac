# Copyright (c) 2013 Luca Barbato
# Copyright (c) 2013 GoPivotal, Inc.  All rights reserved.
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

################################################################################
#  Start the configuration phase.                                              #
################################################################################

AC_PREREQ([2.53])

AC_INIT([nanomsg], [m4_esyscmd([./package_version.sh])],
   [nanomsg@freelists.org], [nanomsg], [http://nanomsg.org/])
AC_CONFIG_SRCDIR([src/nn.h])
AM_INIT_AUTOMAKE([1.6 foreign subdir-objects tar-ustar dist-zip])
m4_ifdef([AM_SILENT_RULES], [
    AM_SILENT_RULES([yes])
])

AC_CANONICAL_HOST

################################################################################
#  Retrieve the versions.                                                      #
################################################################################

AC_PROG_SED
AC_PROG_AWK

NN_ABI_VERSION=m4_esyscmd([./abi_version.sh])
NN_PACKAGE_VERSION=m4_esyscmd([./package_version.sh])
NN_LIBTOOL_VERSION=m4_esyscmd([./abi_version.sh -libtool])

AC_SUBST(NN_ABI_VERSION)
AC_SUBST(NN_PACKAGE_VERSION)
AC_SUBST(NN_LIBTOOL_VERSION)

AC_MSG_NOTICE([nanomsg package version: $NN_PACKAGE_VERSION])
AC_MSG_NOTICE([nanomsg ABI version: $NN_ABI_VERSION])

################################################################################
#  Check the compilers.                                                         #
################################################################################

AC_PROG_CC_C99
AM_PROG_CC_C_O

################################################################################
#  Check whether GCC compiler is used.                                         #
################################################################################
AS_IF([test x"$GCC" = xyes],
    [AC_DEFINE([NN_HAVE_GCC])]
)

################################################################################
#  Check whether ICC compiler is used.                                         #
################################################################################
AC_CACHE_CHECK([whether we are using ICC compiler],
    [ac_cv_icc_compiler],
    [AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([], [[
            #ifndef __INTEL_COMPILER
               #error
            #endif
            ]])],
        [ac_icc_compiler=yes],
        [ac_icc_compiler=no])
    ac_cv_icc_compiler=$ac_icc_compiler])
AS_IF([test x"$ac_cv_icc_compiler" = xyes],
    [AC_DEFINE([NN_HAVE_ICC])]
)

################################################################################
#  Check whether Sun Studio compiler is used.                                  #
################################################################################
AC_CACHE_CHECK([whether we are using Sun Studio compiler],
    [ac_cv_sun_studio_compiler],
    [AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([], [[
            #ifndef __SUNPRO_C
                #error
            #endif
            ]])],
        [ac_sun_studio_compiler=yes],
        [ac_sun_studio_compiler=no])
    ac_cv_sun_studio_compiler=$ac_sun_studio_compiler])
AS_IF([test x"$ac_cv_sun_studio_compiler" = xyes],
    [AC_DEFINE([NN_HAVE_SUN_STUDIO])]
)

################################################################################
#  Check whether clang compiler is used.                                       #
################################################################################
AC_CACHE_CHECK([whether we are using clang compiler],
    [ac_cv_clang_compiler],
    [AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([], [[
            #ifndef __clang__
                #error
            #endif
            ]])],
        [ac_clang_compiler=yes],
        [ac_clang_compiler=no])
    ac_cv_clang_compiler=$ac_clang_compiler])
AS_IF([test x"$ac_cv_clang_compiler" = xyes],
    [AC_DEFINE([NN_HAVE_CLANG])]
)

################################################################################
#  Check whether HP aCC compiler is used.                                      #
################################################################################
AC_CACHE_CHECK([whether we are using HP aCC compiler],
    [ac_cv_hpacc_compiler],
    [AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([], [[
            #ifndef __HP_cc
                #error
            #endif
            ]])],
        [ac_hpacc_compiler=yes],
        [ac_hpacc_compiler=no])
    ac_cv_hpacc_compiler=$ac_hpacc_compiler])
AS_IF([test x"$ac_cv_hpacc_compiler" = xyes],
    [AC_DEFINE([NN_HAVE_HPACC])]
)

################################################################################
#  If --enable-debug is specified set appropriate compiler options.            #
################################################################################

AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug],
    [Enable debugging information [default=no]])])

if test "x$enable_debug" = "xyes"; then
    if test "x$GCC" = "xyes" -o \
            "x$ac_cv_icc_compiler" = "xyes" -o \
            "x$ac_cv_clang_compiler" = "xyes"; then
        CFLAGS="-g -O0 "
    elif test "x$ac_cv_sun_studio_compiler" = "xyes"; then
        CFLAGS="-g0 "
    elif test "x$ac_cv_hpacc_compiler" = "xyes"; then
        CFLAGS="-g +O0 "
    fi
    ORIG_CFLAGS="${CFLAGS:-none}"
    if test "x$ORIG_CFLAGS" != "xnone"; then
        CFLAGS="${CFLAGS} ${ORIG_CFLAGS}"
    fi
    AC_DEFINE([NN_DEBUG])
fi

################################################################################
#  Build the documentation.                                                    #
################################################################################

AC_ARG_ENABLE([doc],
    AS_HELP_STRING([--enable-doc], [Build documentation [default=no]])
)

AS_IF([test x"$enable_doc" = "xyes"], [
    AC_CHECK_PROGS([ASCIIDOC], [asciidoc])
    AS_IF([test x"$ASCIIDOC" = "x"],[
        AC_MSG_ERROR([Please install asciidoc to build the documentation])
    ])
    AC_CHECK_PROGS([XMLTO], [xmlto])
    AS_IF([test x"$XMLTO" = "x"], [
        AC_MSG_ERROR([Please install xmlto to build the documentation])
    ])
    AS_IF([test ! -d "doc"], [
        AC_PROG_MKDIR_P
        AS_MKDIR_P("doc")
    ])
])
AM_CONDITIONAL([DOC], [test x"$ASCIIDOC" != "x" -a x"$XMLTO" != "x" ])

################################################################################
#  Build the nanocat
################################################################################

AC_ARG_ENABLE([nanocat],
    AS_HELP_STRING([--enable-nanocat], [Build nanocat executable [default=yes]])
)
AC_ARG_ENABLE([symlinks],
    AS_HELP_STRING([--enable-symlinks],
        [Make nn_xxx symlinks for nanocat [default=yes]])
)

AS_IF([test x"$enable_nanocat" != "xno" -a x"$enable_symlinks" != x"no"], [
    AC_PROG_LN_S
])
AM_CONDITIONAL([NANOCAT], [test x"$enable_nanocat" != "xno" ])
AM_CONDITIONAL([SYMLINKS], [test x"$enable_symlinks" != "xno" ])

################################################################################
#  Check which OS we are building for.                                         #
################################################################################

AS_CASE([${host_os}],
    [*linux*],   [AC_DEFINE([NN_HAVE_LINUX])],
    [*mingw32*], [
        AC_DEFINE([NN_HAVE_WINDOWS])
        AC_DEFINE([NN_HAVE_MINGW])
        AC_DEFINE([_WIN32_WINNT],0x0600)
        LIBS="$LIBS -lws2_32 -lMswsock"
    ],
    [*darwin*],  [AC_DEFINE([NN_HAVE_OSX])],
    [*freebsd*], [AC_DEFINE([NN_HAVE_FREEBSD])],
    [*netbsd*],  [AC_DEFINE([NN_HAVE_NETBSD])],
    [*openbsd*], [AC_DEFINE([NN_HAVE_OPENBSD])],
    [*hpux*],    [AC_DEFINE([NN_HAVE_HPUX])],
    [*solaris*], [
        AC_DEFINE([NN_HAVE_SOLARIS])
        AC_CHECK_LIB([socket], [socket])
        AC_CHECK_LIB([nsl], [gethostbyname])
    ],
    [*nto-qnx*], [
        AC_DEFINE([NN_HAVE_QNX])
        AC_CHECK_LIB([socket], [socket])
    ]
)

################################################################################
#  Feature checks.                                                             #
################################################################################

AX_PTHREAD([
  LIBS="$PTHREAD_LIBS $LIBS"
  CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
  CC="$PTHREAD_CC"
])

AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([stdint.h], [AC_DEFINE([NN_HAVE_STDINT])])

AC_CHECK_FUNCS([eventfd], [AC_DEFINE([NN_HAVE_EVENTFD])])
AC_CHECK_FUNCS([pipe], [AC_DEFINE([NN_HAVE_PIPE])])
AC_CHECK_FUNCS([pipe2], [
    AC_DEFINE([NN_HAVE_PIPE2])
    CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
])
AC_CHECK_FUNCS([gethrtime], [AC_DEFINE([NN_HAVE_GETHRTIME])])
AC_CHECK_FUNCS([clock_gettime], [
    AC_EGREP_HEADER([_POSIX_MONOTONIC_CLOCK], [unistd.h], [
        AC_DEFINE([NN_HAVE_CLOCK_MONOTONIC])
    ])
])

AC_CHECK_FUNCS([poll], [AC_DEFINE([NN_HAVE_POLL])])

AC_CHECK_FUNCS([epoll_create], [AC_DEFINE([NN_USE_EPOLL])], [
    AC_CHECK_FUNCS([kqueue], [AC_DEFINE([NN_USE_KQUEUE])], [
        AC_DEFINE([NN_USE_POLL])
    ])
])

AC_CHECK_FUNCS([getifaddrs], [AC_DEFINE([NN_USE_IFADDRS])], [
    AC_EGREP_HEADER([SIOCGIFADDR], [sys/ioctl.h], [
        AC_DEFINE([NN_USE_SIOCGIFADDR])
    ], [
        AC_DEFINE([NN_USE_LITERAL_IFADDR])
    ])
])

AC_CHECK_FUNCS([accept4], [
    AC_DEFINE([NN_HAVE_ACCEPT4])
    CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
])
AC_SEARCH_LIBS([getaddrinfo_a], [anl], [
    AC_DEFINE([NN_HAVE_GETADDRINFO_A])
    CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"
])
AC_SEARCH_LIBS([socketpair], [], [
    AC_DEFINE([NN_HAVE_SOCKETPAIR])
])
AC_SEARCH_LIBS([sem_wait], [rt pthread], [
    AC_DEFINE([NN_HAVE_SEMAPHORE])
])

AC_LINK_IFELSE([AC_LANG_PROGRAM([], [[
        #include <stdint.h>
        int main()
        {
            volatile uint32_t n = 0;
            __sync_fetch_and_add (&n, 1);
            __sync_fetch_and_sub (&n, 1);
            return 0;
        }
]])], [
    AC_DEFINE([NN_HAVE_GCC_ATOMIC_BUILTINS])
])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
    [[#include <atomic.h>]],
    [[uint32_t value; atomic_cas_32 (&value, 0, 0); return 0;]])],[
    AC_DEFINE([NN_HAVE_ATOMIC_SOLARIS])],[])

AS_IF([test x"$ac_cv_func_eventfd" = xyes], [
    AC_DEFINE([NN_USE_EVENTFD])], [
    AS_IF([test x"$ac_cv_func_pipe" = xyes], [
        AC_DEFINE([NN_USE_PIPE])], [
        AS_IF([test x"$ac_cv_func_socketpair" = xyes], [
            AC_DEFINE([NN_USE_SOCKETPAIR])]
        )
    ])
])

LT_INIT

DOLT

################################################################################
#  Finish the configuration phase.                                             #
################################################################################

AC_CONFIG_MACRO_DIR([m4])

AC_OUTPUT([
Makefile
libnanomsg.pc
])

