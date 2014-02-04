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
            '../third_party/externals/libwebp/src/dec/alpha.c',
            '../third_party/externals/libwebp/src/dec/buffer.c',
            '../third_party/externals/libwebp/src/dec/frame.c',
            '../third_party/externals/libwebp/src/dec/idec.c',
            '../third_party/externals/libwebp/src/dec/io.c',
            '../third_party/externals/libwebp/src/dec/layer.c',
            '../third_party/externals/libwebp/src/dec/quant.c',
            '../third_party/externals/libwebp/src/dec/tree.c',
            '../third_party/externals/libwebp/src/dec/vp8.c',
            '../third_party/externals/libwebp/src/dec/vp8l.c',
            '../third_party/externals/libwebp/src/dec/webp.c',
          ],
        },
        {
          'target_name': 'libwebp_demux',
          'type': 'static_library',
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/demux/demux.c',
          ],
        },
        {
          'target_name': 'libwebp_dsp',
          'type': 'static_library',
          'include_dirs': [
              '../third_party/externals/libwebp',
          ],
          'sources': [
            '../third_party/externals/libwebp/src/dsp/cpu.c',
            '../third_party/externals/libwebp/src/dsp/dec.c',
            '../third_party/externals/libwebp/src/dsp/dec_sse2.c',
            '../third_party/externals/libwebp/src/dsp/enc.c',
            '../third_party/externals/libwebp/src/dsp/enc_sse2.c',
            '../third_party/externals/libwebp/src/dsp/lossless.c',
            '../third_party/externals/libwebp/src/dsp/upsampling.c',
            '../third_party/externals/libwebp/src/dsp/upsampling_sse2.c',
            '../third_party/externals/libwebp/src/dsp/yuv.c',
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
            ['arm_version >= 7', {
              'type': 'static_library',
              'include_dirs': [
                  '../third_party/externals/libwebp',
              ],
              'sources': [
                '../third_party/externals/libwebp/src/dsp/dec_neon.c',
                '../third_party/externals/libwebp/src/dsp/enc_neon.c',
                '../third_party/externals/libwebp/src/dsp/upsampling_neon.c',
              ],
              # behavior similar dsp_neon.c.neon in an Android.mk
              'cflags!': [
                '-mfpu=vfpv3-d16',
              ],
              'cflags': [ '-mfpu=neon' ],
            },{  # !(arm_version >= 7)
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
            '../third_party/externals/libwebp/src/enc/alpha.c',
            '../third_party/externals/libwebp/src/enc/analysis.c',
            '../third_party/externals/libwebp/src/enc/backward_references.c',
            '../third_party/externals/libwebp/src/enc/config.c',
            '../third_party/externals/libwebp/src/enc/cost.c',
            '../third_party/externals/libwebp/src/enc/filter.c',
            '../third_party/externals/libwebp/src/enc/frame.c',
            '../third_party/externals/libwebp/src/enc/histogram.c',
            '../third_party/externals/libwebp/src/enc/iterator.c',
            '../third_party/externals/libwebp/src/enc/layer.c',
            '../third_party/externals/libwebp/src/enc/picture.c',
            '../third_party/externals/libwebp/src/enc/quant.c',
            '../third_party/externals/libwebp/src/enc/syntax.c',
            '../third_party/externals/libwebp/src/enc/token.c',
            '../third_party/externals/libwebp/src/enc/tree.c',
            '../third_party/externals/libwebp/src/enc/vp8l.c',
            '../third_party/externals/libwebp/src/enc/webpenc.c',
          ],
          'cflags': [ '-w' ],
        },
        {
          'target_name': 'libwebp_utils',
          'type': 'static_library',
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
            '../third_party/externals/libwebp/src/utils/rescaler.c',
            '../third_party/externals/libwebp/src/utils/thread.c',
            '../third_party/externals/libwebp/src/utils/utils.c',
          ],
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
