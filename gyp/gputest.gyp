# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'skgputest',
      'product_name': 'skia_skgputest',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'include_dirs': [
        '../include/gpu',
        '../include/private',
        '../include/utils',
        '../src/core',
        '../src/gpu',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../src/gpu',
        ],
      },
      'sources': [
        '<(skia_src_path)/gpu/GrContextFactory.cpp',
        '<(skia_src_path)/gpu/GrContextFactory.h',
        '<(skia_src_path)/gpu/GrTest.cpp',
        '<(skia_src_path)/gpu/GrTest.h',
      ],
    },
  ],
}
