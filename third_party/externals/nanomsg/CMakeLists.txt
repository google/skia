#
#   Copyright (c) 2012 250bpm s.r.o.  All rights reserved.
#   Copyright (c) 2013 GoPivotal, Inc.  All rights reserved.
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom
#   the Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
#   IN THE SOFTWARE.
#

cmake_minimum_required (VERSION 2.8)
include (CheckIncludeFiles)
include (CheckSymbolExists)
include (CheckCSourceCompiles)

project (nanomsg C)
enable_testing ()

#  Platform checks.

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions (-DNN_HAVE_WINDOWS)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS)

    if (MINGW)
        add_definitions (-DNN_HAVE_MINGW -DNN_HAVE_STDINT -D_WIN32_WINNT=0x0600)
    endif ()
else ()
    message (FATAL_ERROR "ERROR: CMake build system is intended only to generate MSVC solution files.\nUse autotools build system for any other purpose." )
endif ()

#  Build the library itself.

add_subdirectory (src)

#  Build the tools

add_executable (nanocat tools/nanocat.c tools/options.c)
target_link_libraries (nanocat nanomsg)

#  Build unit tests.

set (all_tests "")

macro (add_libnanomsg_test NAME)
    list (APPEND all_tests ${NAME})
    add_executable (${NAME} tests/${NAME}.c)
    target_link_libraries (${NAME} nanomsg)
    add_test (NAME ${NAME} COMMAND ${NAME})
endmacro (add_libnanomsg_test)

#  Transport tests.
add_libnanomsg_test (inproc)
add_libnanomsg_test (inproc_shutdown)
add_libnanomsg_test (ipc)
add_libnanomsg_test (ipc_shutdown)
add_libnanomsg_test (tcp)
add_libnanomsg_test (tcp_shutdown)

#  Protocol tests.
add_libnanomsg_test (pair)
add_libnanomsg_test (pubsub)
add_libnanomsg_test (reqrep)
add_libnanomsg_test (pipeline)
add_libnanomsg_test (survey)
add_libnanomsg_test (bus)

#  Feature tests.
add_libnanomsg_test (block)
add_libnanomsg_test (term)
add_libnanomsg_test (timeo)
add_libnanomsg_test (iovec)
add_libnanomsg_test (msg)
add_libnanomsg_test (prio)
add_libnanomsg_test (poll)
add_libnanomsg_test (device)
add_libnanomsg_test (emfile)
add_libnanomsg_test (domain)
add_libnanomsg_test (trie)
add_libnanomsg_test (list)
add_libnanomsg_test (hash)
add_libnanomsg_test (symbol)
add_libnanomsg_test (separation)
add_libnanomsg_test (zerocopy)
add_libnanomsg_test (shutdown)

#  Build the performance tests.

macro (add_libnanomsg_perf NAME)
    add_executable (${NAME} perf/${NAME}.c)
    target_link_libraries (${NAME} nanomsg)
endmacro (add_libnanomsg_perf)

add_libnanomsg_perf (inproc_lat)
add_libnanomsg_perf (inproc_thr)
add_libnanomsg_perf (local_lat)
add_libnanomsg_perf (remote_lat)
add_libnanomsg_perf (local_thr)
add_libnanomsg_perf (remote_thr)

#  NSIS package

install (FILES src/nn.h DESTINATION include/nanomsg)
install (FILES src/inproc.h DESTINATION include/nanomsg)
install (FILES src/ipc.h DESTINATION include/nanomsg)
install (FILES src/tcp.h DESTINATION include/nanomsg)
install (FILES src/pair.h DESTINATION include/nanomsg)
install (FILES src/pubsub.h DESTINATION include/nanomsg)
install (FILES src/reqrep.h DESTINATION include/nanomsg)
install (FILES src/pipeline.h DESTINATION include/nanomsg)
install (FILES src/survey.h DESTINATION include/nanomsg)
install (FILES src/bus.h DESTINATION include/nanomsg)

install (TARGETS nanocat RUNTIME DESTINATION bin)

set (CPACK_GENERATOR "NSIS")
set (CPACK_PACKAGE_NAME "nanomsg")
include (CPack)


