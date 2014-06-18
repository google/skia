# Copyright 2014 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'resources',
      'type': 'static_library',
      'sources': [
        '../tools/Resources.cpp',
        '../tools/Resources.h',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../tools/',
        ],
      },
    },
  ]
}
