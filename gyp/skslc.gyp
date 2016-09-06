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
      'includes' : [
        'sksl.gypi',
      ],
      'sources': [
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
