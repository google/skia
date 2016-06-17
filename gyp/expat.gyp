# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Build expat from source.
# Used on Linux bots for testing the Android FontMgr xml parsing.
# This is mostly important for the MSAN bot's instrumentation.

{
    'targets': [{
        'target_name': 'expat',
        'type': 'static_library',
        'cflags': [ '-w' ],
        'defines': [ 'HAVE_MEMMOVE' ],
        'sources': [
            '../third_party/externals/expat/lib/xmlparse.c',
            '../third_party/externals/expat/lib/xmlrole.c',
            '../third_party/externals/expat/lib/xmltok.c',
        ],
        'direct_dependent_settings': {
            'include_dirs': [ '../third_party/externals/expat/lib' ],
        },
    }]
}
