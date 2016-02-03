# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'includes': [ 'apptype_console.gypi' ],
    'targets': [{
        'target_name': 'fuzz',
        'type': 'executable',
        'sources': [ '<!@(python find.py ../fuzz "*.cpp")' ],
        'dependencies': [ 'skia_lib.gyp:skia_lib' ],
        'xcode_settings': {
            'DEAD_CODE_STRIPPING': 'YES',
        },
    }],
}
