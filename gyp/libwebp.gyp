# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'use_system_libwebp%': 0,
  },
  'conditions': [
    ['use_system_libwebp==0', {
      'targets': [
        {
          'target_name': 'libwebp_dec',
          'type': 'static_library',
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/dec/alpha.c',
            '../third_party/externals/libwebp/dec/buffer.c',
            '../third_party/externals/libwebp/dec/frame.c',
            '../third_party/externals/libwebp/dec/idec.c',
            '../third_party/externals/libwebp/dec/io.c',
            '../third_party/externals/libwebp/dec/layer.c',
            '../third_party/externals/libwebp/dec/quant.c',
            '../third_party/externals/libwebp/dec/tree.c',
            '../third_party/externals/libwebp/dec/vp8.c',
            '../third_party/externals/libwebp/dec/vp8l.c',
            '../third_party/externals/libwebp/dec/webp.c',
          ],
          'cflags!': [
            '-fno-rtti', # supresses warnings about invalid option of non-C++ code
          ],
        },
        {
          'target_name': 'libwebp_dsp',
          'type': 'static_library',
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/dsp/cpu.c',
            '../third_party/externals/libwebp/dsp/dec.c',
            '../third_party/externals/libwebp/dsp/dec_sse2.c',
            '../third_party/externals/libwebp/dsp/enc.c',
            '../third_party/externals/libwebp/dsp/enc_sse2.c',
            '../third_party/externals/libwebp/dsp/lossless.c',
            '../third_party/externals/libwebp/dsp/upsampling.c',
            '../third_party/externals/libwebp/dsp/upsampling_sse2.c',
            '../third_party/externals/libwebp/dsp/yuv.c',
          ],
          'cflags!': [
            '-fno-rtti', # supresses warnings about invalid option of non-C++ code
          ],
          'conditions': [
            ['skia_os == "android"', {
              'dependencies' : [
                'android_deps.gyp:cpu_features',
              ],
            }],
          ],
        },
        {
          'target_name': 'libwebp_dsp_neon',
          'conditions': [
            ['armv7 == 1', {
              'type': 'static_library',
              'include_dirs': [
                  '../third_party/externals/libwebp',
              ],
              'sources': [
                '../third_party/externals/libwebp/dsp/dec_neon.c',
              ],
              # behavior similar dsp_neon.c.neon in an Android.mk
              'cflags!': [ 
                '-mfpu=vfpv3-d16',
                '-fno-rtti', # supresses warnings about invalid option of non-C++ code
              ],
              'cflags': [ '-mfpu=neon' ],
            },{  # "armv7 != 1"
              'type': 'none',
            }],
          ],
        },
        {
          'target_name': 'libwebp_enc',
          'type': 'static_library',
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/enc/alpha.c',
            '../third_party/externals/libwebp/enc/analysis.c',
            '../third_party/externals/libwebp/enc/backward_references.c',
            '../third_party/externals/libwebp/enc/config.c',
            '../third_party/externals/libwebp/enc/cost.c',
            '../third_party/externals/libwebp/enc/filter.c',
            '../third_party/externals/libwebp/enc/frame.c',
            '../third_party/externals/libwebp/enc/histogram.c',
            '../third_party/externals/libwebp/enc/iterator.c',
            '../third_party/externals/libwebp/enc/layer.c',
            '../third_party/externals/libwebp/enc/picture.c',
            '../third_party/externals/libwebp/enc/quant.c',
            '../third_party/externals/libwebp/enc/syntax.c',
            '../third_party/externals/libwebp/enc/tree.c',
            '../third_party/externals/libwebp/enc/vp8l.c',
            '../third_party/externals/libwebp/enc/webpenc.c',
          ],
          'cflags!': [
            '-fno-rtti', # supresses warnings about invalid option of non-C++ code
          ],
        },
        {
          'target_name': 'libwebp_utils',
          'type': 'static_library',
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/utils/bit_reader.c',
            '../third_party/externals/libwebp/utils/bit_writer.c',
            '../third_party/externals/libwebp/utils/color_cache.c',
            '../third_party/externals/libwebp/utils/filters.c',
            '../third_party/externals/libwebp/utils/huffman.c',
            '../third_party/externals/libwebp/utils/huffman_encode.c',
            '../third_party/externals/libwebp/utils/quant_levels.c',
            '../third_party/externals/libwebp/utils/rescaler.c',
            '../third_party/externals/libwebp/utils/thread.c',
            '../third_party/externals/libwebp/utils/utils.c',
          ],
          'cflags!': [
            '-fno-rtti', # supresses warnings about invalid option of non-C++ code
          ],
        },
        {
          'target_name': 'libwebp',
          'type': 'none',
          'dependencies' : [
            'libwebp_dec',
            'libwebp_dsp',
            'libwebp_dsp_neon',
            'libwebp_enc',
            'libwebp_utils',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/libwebp',
            ],
          },
          'conditions': [
            ['OS!="win"', {'product_name': 'webp'}],
          ],
        },
      ],
    }, {
      'targets': [
        {
          'target_name': 'libwebp',
          'type': 'none',
          'direct_dependent_settings': {
            'defines': [
              'ENABLE_WEBP',
            ],
          },
          'link_settings': {
            'libraries': [
              '-lwebp',
            ],
          },
        }
      ],
    }],
  ],
}
