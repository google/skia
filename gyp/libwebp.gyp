# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'skia_warnings_as_errors': 0,
    'conditions':[
      ['skia_android_framework == 1', {
        'use_system_libwebp': 1,
      }, {
        'use_system_libwebp%': 0,
      }],
    ],
  },
  'conditions': [
    ['use_system_libwebp==0', {
      'targets': [
        {
          'target_name': 'libwebp_dec',
          'type': 'static_library',
          'includes': [
            'libwebp_skia.gypi',
          ],
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/dec/alpha.c',
            '../third_party/externals/libwebp/src/dec/buffer.c',
            '../third_party/externals/libwebp/src/dec/frame.c',
            '../third_party/externals/libwebp/src/dec/idec.c',
            '../third_party/externals/libwebp/src/dec/io.c',
            '../third_party/externals/libwebp/src/dec/quant.c',
            '../third_party/externals/libwebp/src/dec/tree.c',
            '../third_party/externals/libwebp/src/dec/vp8.c',
            '../third_party/externals/libwebp/src/dec/vp8l.c',
            '../third_party/externals/libwebp/src/dec/webp.c',
          ],
          'cflags': [ '-w' ],
          'xcode_settings': { 'WARNING_CFLAGS': [ '-w' ] },
        },
        {
          'target_name': 'libwebp_demux',
          'type': 'static_library',
          'includes': [
            'libwebp_skia.gypi',
          ],
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/demux/demux.c',
          ],
          'cflags': [ '-w' ],
          'xcode_settings': { 'WARNING_CFLAGS': [ '-w' ] },
        },
        {
          'target_name': 'libwebp_dsp',
          'type': 'static_library',
          'includes': [
            'libwebp_skia.gypi',
          ],
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/dsp/alpha_processing.c',
            '../third_party/externals/libwebp/src/dsp/alpha_processing_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/alpha_processing_sse2.c',
            '../third_party/externals/libwebp/src/dsp/alpha_processing_sse41.c',
            '../third_party/externals/libwebp/src/dsp/cpu.c',
            '../third_party/externals/libwebp/src/dsp/dec.c',
            '../third_party/externals/libwebp/src/dsp/dec_clip_tables.c',
            '../third_party/externals/libwebp/src/dsp/dec_mips32.c',
            '../third_party/externals/libwebp/src/dsp/dec_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/dec_sse2.c',
            '../third_party/externals/libwebp/src/dsp/dec_sse41.c',
            '../third_party/externals/libwebp/src/dsp/enc.c',
            '../third_party/externals/libwebp/src/dsp/enc_sse2.c',
            '../third_party/externals/libwebp/src/dsp/filters.c',
            '../third_party/externals/libwebp/src/dsp/filters_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/filters_sse2.c',
            '../third_party/externals/libwebp/src/dsp/lossless.c',
            '../third_party/externals/libwebp/src/dsp/lossless_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/lossless_sse2.c',
            '../third_party/externals/libwebp/src/dsp/rescaler.c',
            '../third_party/externals/libwebp/src/dsp/rescaler_mips32.c',
            '../third_party/externals/libwebp/src/dsp/rescaler_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/upsampling.c',
            '../third_party/externals/libwebp/src/dsp/upsampling_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/upsampling_sse2.c',
            '../third_party/externals/libwebp/src/dsp/yuv.c',
            '../third_party/externals/libwebp/src/dsp/yuv_mips32.c',
            '../third_party/externals/libwebp/src/dsp/yuv_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/yuv_sse2.c',
          ],
          'cflags': [ '-w' ],
          'xcode_settings': { 'WARNING_CFLAGS': [ '-w' ] },
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
          'includes': [
            'libwebp_skia.gypi',
          ],
          'conditions': [
            ['arm_version == 7', {
              'cflags': [ '-mfpu=neon' ],
            }],
            ['arm_version >= 7', {
              'type': 'static_library',
              'include_dirs': [
                  '../third_party/externals/libwebp',
              ],
              'sources': [
                '../third_party/externals/libwebp/src/dsp/dec_neon.c',
                '../third_party/externals/libwebp/src/dsp/enc_neon.c',
                '../third_party/externals/libwebp/src/dsp/lossless_neon.c',
                '../third_party/externals/libwebp/src/dsp/lossless_enc_neon.c',
                '../third_party/externals/libwebp/src/dsp/upsampling_neon.c',
              ],
              # behavior similar dsp_neon.c.neon in an Android.mk
              'cflags!': [
                '-mfpu=vfpv3-d16',
              ],
              'cflags': [ '-w' ],
            },{  # !(arm_version >= 7)
              'type': 'none',
            }],
          ],
        },
        {
          'target_name': 'libwebp_dsp_enc',
          'type': 'static_library',
          'includes': [
            'libwebp_skia.gypi',
          ],
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/dsp/argb.c',
            '../third_party/externals/libwebp/src/dsp/argb_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/argb_sse2.c',
            '../third_party/externals/libwebp/src/dsp/cost.c',
            '../third_party/externals/libwebp/src/dsp/cost_mips32.c',
            '../third_party/externals/libwebp/src/dsp/cost_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/cost_sse2.c',
            '../third_party/externals/libwebp/src/dsp/enc_avx2.c',
            '../third_party/externals/libwebp/src/dsp/enc_mips32.c',
            '../third_party/externals/libwebp/src/dsp/enc_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/enc_sse41.c',
            '../third_party/externals/libwebp/src/dsp/lossless_enc.c',
            '../third_party/externals/libwebp/src/dsp/lossless_enc_mips32.c',
            '../third_party/externals/libwebp/src/dsp/lossless_enc_mips_dsp_r2.c',
            '../third_party/externals/libwebp/src/dsp/lossless_enc_sse2.c',
            '../third_party/externals/libwebp/src/dsp/lossless_enc_sse41.c',
          ],
        },
        {
          'target_name': 'libwebp_enc',
          'type': 'static_library',
          'includes': [
            'libwebp_skia.gypi',
          ],
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'dependencies' : [
            'libwebp_dsp_enc',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/enc/alpha.c',
            '../third_party/externals/libwebp/src/enc/analysis.c',
            '../third_party/externals/libwebp/src/enc/backward_references.c',
            '../third_party/externals/libwebp/src/enc/config.c',
            '../third_party/externals/libwebp/src/enc/cost.c',
            '../third_party/externals/libwebp/src/enc/filter.c',
            '../third_party/externals/libwebp/src/enc/frame.c',
            '../third_party/externals/libwebp/src/enc/histogram.c',
            '../third_party/externals/libwebp/src/enc/iterator.c',
            '../third_party/externals/libwebp/src/enc/near_lossless.c',
            '../third_party/externals/libwebp/src/enc/picture.c',
            '../third_party/externals/libwebp/src/enc/picture_csp.c',
            '../third_party/externals/libwebp/src/enc/picture_psnr.c',
            '../third_party/externals/libwebp/src/enc/picture_rescale.c',
            '../third_party/externals/libwebp/src/enc/picture_tools.c',
            '../third_party/externals/libwebp/src/enc/quant.c',
            '../third_party/externals/libwebp/src/enc/syntax.c',
            '../third_party/externals/libwebp/src/enc/token.c',
            '../third_party/externals/libwebp/src/enc/tree.c',
            '../third_party/externals/libwebp/src/enc/vp8l.c',
            '../third_party/externals/libwebp/src/enc/webpenc.c',
          ],
          'cflags': [ '-w' ],
          'xcode_settings': { 'WARNING_CFLAGS': [ '-w' ] },
        },
        {
          'target_name': 'libwebp_utils',
          'type': 'static_library',
          'includes': [
            'libwebp_skia.gypi',
          ],
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/utils/bit_reader.c',
            '../third_party/externals/libwebp/src/utils/bit_writer.c',
            '../third_party/externals/libwebp/src/utils/color_cache.c',
            '../third_party/externals/libwebp/src/utils/filters.c',
            '../third_party/externals/libwebp/src/utils/huffman.c',
            '../third_party/externals/libwebp/src/utils/huffman_encode.c',
            '../third_party/externals/libwebp/src/utils/quant_levels.c',
            '../third_party/externals/libwebp/src/utils/quant_levels_dec.c',
            '../third_party/externals/libwebp/src/utils/random.c',
            '../third_party/externals/libwebp/src/utils/rescaler.c',
            '../third_party/externals/libwebp/src/utils/thread.c',
            '../third_party/externals/libwebp/src/utils/utils.c',
          ],
          'cflags': [ '-w' ],
          'xcode_settings': { 'WARNING_CFLAGS': [ '-w' ] },
        },
        {
          'target_name': 'libwebp',
          'type': 'none',
          'dependencies' : [
            'libwebp_dec',
            'libwebp_demux',
            'libwebp_dsp',
            'libwebp_dsp_neon',
            'libwebp_enc',
            'libwebp_utils',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/libwebp/src',
            ],
          },
          'conditions': [
            ['OS!="win"', {'product_name': 'webp'}],
          ],
        },
      ],
    }, {
      # use_system_libwebp == 1
      'targets': [
        {
          'target_name': 'libwebp',
          'type': 'none',
          'conditions': [
            [ 'skia_android_framework', {
              'direct_dependent_settings': {
                'libraries': [
                  'libwebp-decode.a',
                  'libwebp-encode.a',
                ],
              'include_dirs': [
                'external/webp/include',
              ],
              },
            }, { # skia_android_framework == 0
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
              },
            ],
          ],
        }
      ],
    }],
  ],
}
