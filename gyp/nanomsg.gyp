# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [{
    # Draws pictures cross-process.
    'target_name': 'nanomsg_picture_demo',
    'type': 'executable',
    'dependencies': [
      'skia_lib.gyp:skia_lib',
      'flags.gyp:flags',
      'libnanomsg',
    ],
    'sources': [ '../experimental/nanomsg/picture_demo.cpp' ],
  },{
    'target_name': 'libnanomsg',
    'type': 'static_library',

    # Clients can include nanomsg public header foo.h with #include "nanomsg/src/foo.h"
    'direct_dependent_settings': {
      'include_dirs': [ '../third_party/externals' ]
    },

    'sources': [
      '<!@(python find.py ../third_party/externals/nanomsg/src "*.c")'
    ],

    # TODO(mtklein): Support Windows?
    # To refresh: cd third_party/externals/nanomsg; ./autogen.sh; ./configure; copy from Makefile.
    'conditions': [
      ['skia_os == "linux"', {
        'cflags': [ '-w' ],
        'libraries': [
            '-lpthread',
            '-lanl',  # Provides getaddrinfo_a and co.
        ],
        'direct_dependent_settings': {
            'libraries': [ '-lpthread', '-lanl' ],
        },
        'defines=': [             # equals sign throws away most Skia defines (just noise)
          'HAVE_ACCEPT4',
          'HAVE_ARPA_INET_H',
          'HAVE_CLOCK_GETTIME',
          'HAVE_DLFCN_H',
          'HAVE_EPOLL_CREATE',
          'HAVE_EVENTFD',
          'HAVE_GETIFADDRS',
          'HAVE_INTTYPES_H',
          'HAVE_MEMORY_H',
          'HAVE_NETDB_H',
          'HAVE_NETINET_IN_H',
          'HAVE_PIPE',
          'HAVE_PIPE2',
          'HAVE_POLL',
          'HAVE_PTHREAD_PRIO_INHERIT',
          'HAVE_STDINT_H',
          'HAVE_STDLIB_H',
          'HAVE_STRINGS_H',
          'HAVE_STRING_H',
          'HAVE_SYS_IOCTL_H',
          'HAVE_SYS_SOCKET_H',
          'HAVE_SYS_STAT_H',
          'HAVE_SYS_TYPES_H',
          'HAVE_UNISTD_H',
          'HAVE_UNISTD_H',
          'NN_HAVE_ACCEPT4',
          'NN_HAVE_CLANG',
          'NN_HAVE_EVENTFD',
          'NN_HAVE_GCC',
          'NN_HAVE_GETADDRINFO_A',
          'NN_HAVE_LINUX',
          'NN_HAVE_PIPE',
          'NN_HAVE_PIPE2',
          'NN_HAVE_POLL',
          'NN_HAVE_SEMAPHORE',
          'NN_HAVE_SOCKETPAIR',
          'NN_USE_EPOLL',
          'NN_USE_EVENTFD',
          'NN_USE_IFADDRS',
          'STDC_HEADERS',
          '_GNU_SOURCE',
        ],
      }],
      ['skia_os == "mac"', {
        'xcode_settings': {
            'WARNING_CFLAGS': [ '-w' ],
        },
        'defines=': [             # equals sign throws away most Skia defines (just noise)
          'HAVE_ARPA_INET_H',
          'HAVE_DLFCN_H',
          'HAVE_GETIFADDRS',
          'HAVE_INTTYPES_H',
          'HAVE_KQUEUE',
          'HAVE_MEMORY_H',
          'HAVE_NETDB_H',
          'HAVE_NETINET_IN_H',
          'HAVE_PIPE',
          'HAVE_POLL',
          'HAVE_PTHREAD_PRIO_INHERIT',
          'HAVE_STDINT_H',
          'HAVE_STDLIB_H',
          'HAVE_STRINGS_H',
          'HAVE_STRING_H',
          'HAVE_SYS_IOCTL_H',
          'HAVE_SYS_SOCKET_H',
          'HAVE_SYS_STAT_H',
          'HAVE_SYS_TYPES_H',
          'HAVE_UNISTD_H',
          'NN_HAVE_CLANG',
          'NN_HAVE_GCC',
          'NN_HAVE_OSX',
          'NN_HAVE_PIPE',
          'NN_HAVE_POLL',
          'NN_HAVE_SEMAPHORE',
          'NN_HAVE_SOCKETPAIR',
          'NN_USE_IFADDRS',
          'NN_USE_KQUEUE',
          'NN_USE_PIPE',
          'STDC_HEADERS',
          '_THREAD_SAFE',
        ],
      }],
    ]
  }]
}
