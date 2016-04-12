# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'includes': [ 'apptype_console.gypi' ],
    'targets': [{
        'target_name': 'fuzz',
        'type': 'executable',
        'defines': [
          'SK_FUZZ_LOGGING',
        ],
        'sources': [ '<!@(python find.py ../fuzz "*.cpp")' ],
        'dependencies': [
            'flags.gyp:flags',
            'skia_lib.gyp:skia_lib',
        ],
        'include_dirs': [
            '../src/core',
        ],
    }],
}
