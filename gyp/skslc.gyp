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
        '../include/gpu',
        '../include/private',
        '../include/utils',
        '../src/core',
        '../src/gpu',
        '../src/utils',
      ],
      'sources': [
        '<!@(python read_gni.py ../gn/sksl.gni skia_sksl_sources)',
        '../src/sksl/SkSLMain.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
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
