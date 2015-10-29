# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'sdl',
      'product_name': 'sdl',
      'type': 'shared_library',
      'include_dirs': [
        '<(base_dir)/<(skia_os)',
        '<(src_dir)/include',
      ],
      'includes': [
        'sdl_sources.gypi',
      ],
      'sources': [
        '<@(sdl_sources)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(src_dir)/include',
        ]
      },
      'cflags': [
        '-g',
        '-w',
      ],
      'conditions': [
        ['skia_os == "linux"', {
          'includes': [
            'linux/sdl_linux.gypi',
          ]
        }],
      ],
    },
  ],
}
