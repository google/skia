# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'svg',
      'product_name': 'skia_svg',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'xml.gyp:*',
      ],
      'include_dirs': [
        '../include/private',
        '../include/svg',
        '../src/core',
      ],
      'sources': [
        '<(skia_include_path)/svg/SkSVGCanvas.h',

        '<(skia_src_path)/svg/SkSVGCanvas.cpp',
        '<(skia_src_path)/svg/SkSVGDevice.cpp',
        '<(skia_src_path)/svg/SkSVGDevice.h',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/svg',
        ],
      },
    },
  ],
}
