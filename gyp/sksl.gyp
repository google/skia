# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'sksl',
      'type': 'static_library',
      'standalone_static_library': 1,
      'includes': [
        'sksl.gypi',
      ],
      'defines': [
        'SKIA'
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '../src/sksl',
        ],
      },
    },
  ],
}
