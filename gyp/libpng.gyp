# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'libpng',
      'conditions': [
        [ 'skia_libpng_static',
          {
            'type': 'static_library',
            'include_dirs': [
              '../third_party/externals/libpng',
            ],
            'dependencies': [
              'zlib.gyp:zlib',
            ],
            'export_dependent_settings': [
              'zlib.gyp:zlib',
            ],
            'direct_dependent_settings': {
              'include_dirs': [
                '../third_party/externals/libpng',
              ],
            },
            'cflags': [
              '-w',
              '-fvisibility=hidden',
            ],
            'sources': [
              '../third_party/externals/libpng/png.c',
              '../third_party/externals/libpng/pngerror.c',
              '../third_party/externals/libpng/pnggccrd.c',
              '../third_party/externals/libpng/pngget.c',
              '../third_party/externals/libpng/pngmem.c',
              '../third_party/externals/libpng/pngpread.c',
              '../third_party/externals/libpng/pngread.c',
              '../third_party/externals/libpng/pngrio.c',
              '../third_party/externals/libpng/pngrtran.c',
              '../third_party/externals/libpng/pngrutil.c',
              '../third_party/externals/libpng/pngset.c',
              '../third_party/externals/libpng/pngtrans.c',
              '../third_party/externals/libpng/pngvcrd.c',
              '../third_party/externals/libpng/pngwio.c',
              '../third_party/externals/libpng/pngwrite.c',
              '../third_party/externals/libpng/pngwtran.c',
              '../third_party/externals/libpng/pngwutil.c',
            ],
          }, {  # not skia_libpng_static
            'type': 'none',
            'conditions': [
              [ 'skia_os == "android"',
                {
                  # TODO(halcanary): merge all png targets into this file.
                  'dependencies': [
                    'android_deps.gyp:png',
                  ],
                  'export_dependent_settings': [
                    'android_deps.gyp:png',
                  ],
                }, {  # skia_os != "android"
                  'dependencies': [
                    'zlib.gyp:zlib',
                    ],
                  'export_dependent_settings': [
                    'zlib.gyp:zlib',
                    ],
                  'direct_dependent_settings': {
                    'link_settings': {
                      'libraries': [
                        '-lpng',
                      ],
                    },
                  },
                }
              ]
            ]
          }
        ]
      ],
    },
  ]
}
