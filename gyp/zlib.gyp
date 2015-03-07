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
    'target_name' : 'zlib_x86_simd',
    'type': 'static_library',
    'cflags' : ['-msse4.2', '-mpclmul'],
    'xcode_settings': {
        'OTHER_CFLAGS': ['-msse4.2', '-mpclmul'],
    },
    'sources' : [
      '../third_party/externals/zlib/x86.h',
      '../third_party/externals/zlib/x86.c',
      '../third_party/externals/zlib/crc_folding.c',
      '../third_party/externals/zlib/fill_window_sse.c',
    ],
    'conditions': [
      ['skia_clang_build==1', {
        'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': [ '-msse4.2', '-mpclmul' ],
          },
        },
      }],
      ['skia_os in ["linux", "chromeos"]', {
        'all_dependent_settings': {
          'libraries': [ '-lpthread' ],
        },
      }],
    ],
  },
  {
      'target_name': 'zlib',
      'conditions': [
        [ 'skia_android_framework', {
            'type': 'none',
            'direct_dependent-settings': {
                'include_dirs': [ 'external/zlib' ]
            },
        },{
          'type': 'static_library',
          'sources': [
            '../third_party/externals/zlib/adler32.c',
            '../third_party/externals/zlib/compress.c',
            '../third_party/externals/zlib/crc32.c',
            '../third_party/externals/zlib/crc32.h',
            '../third_party/externals/zlib/deflate.c',
            '../third_party/externals/zlib/deflate.h',
            '../third_party/externals/zlib/gzclose.c',
            '../third_party/externals/zlib/gzguts.h',
            '../third_party/externals/zlib/gzlib.c',
            '../third_party/externals/zlib/gzread.c',
            '../third_party/externals/zlib/gzwrite.c',
            '../third_party/externals/zlib/infback.c',
            '../third_party/externals/zlib/inffast.c',
            '../third_party/externals/zlib/inffast.h',
            '../third_party/externals/zlib/inffixed.h',
            '../third_party/externals/zlib/inflate.c',
            '../third_party/externals/zlib/inflate.h',
            '../third_party/externals/zlib/inftrees.c',
            '../third_party/externals/zlib/inftrees.h',
            '../third_party/externals/zlib/mozzconf.h',
            '../third_party/externals/zlib/trees.c',
            '../third_party/externals/zlib/trees.h',
            '../third_party/externals/zlib/uncompr.c',
            '../third_party/externals/zlib/zconf.h',
            '../third_party/externals/zlib/zlib.h',
            '../third_party/externals/zlib/zutil.c',
            '../third_party/externals/zlib/zutil.h',
          ],
          'include_dirs': [
            '../third_party/externals/zlib/',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/zlib',
            ],
          },
          'conditions': [
            [ '"x86" in skia_arch_type', {
              'dependencies': [ 'zlib_x86_simd' ],
            },{
              'sources': ['../third_party/externals/zlib/simd_stub.c'],
            }]
          ],
          'defines': [
            '_CRT_NONSTDC_NO_DEPRECATE',
          ],
        }],
      ],
  }],
}
