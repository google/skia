# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'skslc',
      'type': 'executable',
      'include_dirs': [
         '../include/config',
         '../include/core',
         '../include/private',
         '../src/sksl',
      ],
      'sources': [
        '<!@(python read_gni.py ../gn/sksl.gni skia_sksl_sources)',
        '../src/sksl/SkSLMain.cpp',
      ],
      'configurations': {
        'Debug': {
          'defines': [
            'DEBUG',
          ],
        },
      },
    },
  ],
}
