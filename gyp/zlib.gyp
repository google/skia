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
        [ 'skia_android_framework', {
          'type': 'none',
          'direct_dependent_settings': {
              'include_dirs': [ 'external/zlib' ],
              'link_settings': { 'libraries': [ '-lz' ] },
          },
        }, {
          'type': 'static_library',
          'variables': {
            'skia_zlib_flags' : [
              '-Wno-unused-value',
              '-Wno-shift-negative-value',
              '-Wno-unknown-warning-option',
            ],
          },
          'cflags': [ '<@(skia_zlib_flags)' ],
          'xcode_settings': { 'WARNING_CFLAGS': [ '<@(skia_zlib_flags)' ], },
          'sources': [
            '../third_party/externals/zlib/adler32.c',
            '../third_party/externals/zlib/compress.c',
            '../third_party/externals/zlib/crc32.c',
            '../third_party/externals/zlib/deflate.c',
            '../third_party/externals/zlib/gzclose.c',
            '../third_party/externals/zlib/gzlib.c',
            '../third_party/externals/zlib/gzread.c',
            '../third_party/externals/zlib/gzwrite.c',
            '../third_party/externals/zlib/infback.c',
            '../third_party/externals/zlib/inffast.c',
            '../third_party/externals/zlib/inflate.c',
            '../third_party/externals/zlib/inftrees.c',
            '../third_party/externals/zlib/simd_stub.c',
            '../third_party/externals/zlib/trees.c',
            '../third_party/externals/zlib/uncompr.c',
            '../third_party/externals/zlib/zutil.c',
          ],
          'include_dirs': [
            '../third_party/externals/zlib/',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/zlib',
            ],
          },
          'defines': [
            '_CRT_NONSTDC_NO_DEPRECATE',
          ],
        }],
      ],
  }],
}
