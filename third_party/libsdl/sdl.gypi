# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'sdl',
      'product_name': 'sdl',
      'type': 'static_library',
      'include_dirs': [
        '<(base_dir)/<(skia_os)',
        '<(src_dir)/include',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(src_dir)/include',
          '<(src_dir)/src',
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
        ['skia_os == "android"', {
          'includes': [
            'android/sdl_android.gypi',
          ]
        }],
      ],
    },
  ],
}
