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
      'target_name': 'zlib',
      'conditions': [
        [ 'skia_zlib_static',
          {
            'type': 'static_library',
            'include_dirs': [
              '../third_party/externals/zlib',
            ],
            'direct_dependent_settings': {
              'defines': [
                'SK_ZLIB_INCLUDE="zlib.h"',
              ],
              'include_dirs': [
                '../third_party/externals/zlib',
              ],
            },
            'sources': [
              '../third_party/externals/zlib/src/adler32.c',
              '../third_party/externals/zlib/src/compress.c',
              '../third_party/externals/zlib/src/crc32.c',
              '../third_party/externals/zlib/src/deflate.c',
              '../third_party/externals/zlib/src/gzclose.c',
              '../third_party/externals/zlib/src/gzlib.c',
              '../third_party/externals/zlib/src/gzread.c',
              '../third_party/externals/zlib/src/gzwrite.c',
              '../third_party/externals/zlib/src/infback.c',
              '../third_party/externals/zlib/src/inffast.c',
              '../third_party/externals/zlib/src/inflate.c',
              '../third_party/externals/zlib/src/inftrees.c',
              '../third_party/externals/zlib/src/trees.c',
              '../third_party/externals/zlib/src/uncompr.c',
              '../third_party/externals/zlib/src/zutil.c',
            ],
          }, {  # not skia_zlib_static
            'type': 'none',
            'direct_dependent_settings': {
              'conditions': [
                [ 'skia_android_framework', {
                  'include_dirs': [
                    'external/zlib',
                  ],
                }, {
                  'defines': [
                    'SK_SYSTEM_ZLIB=1',
                  ],
                }]
              ],
              'link_settings': {
                'conditions': [
                  [ 'skia_os == "mac" or skia_os == "ios"', {
                    'libraries': [
                      '$(SDKROOT)/usr/lib/libz.dylib',
                    ]
                  }, {  # skia_os != "mac" and skia_os != "ios"
                    'libraries': [
                      '-lz',
                    ]
                  }],
                ],
              }
            },
          }
        ]
      ]
    }
  ]
}
