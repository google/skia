# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'conditions': [
      # Do not build QCMS on Android or iOS. (See http://crbug.com/577155)
      ['OS == "android" or OS == "ios"', {
        'disable_qcms%': 1,
      }, {
        'disable_qcms%': 0,
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'qcms',
      'product_name': 'qcms',
      'type': 'static_library',

      # Warning (sign-conversion) fixed upstream by large refactoring. Can be
      # removed on next roll.
      'msvs_disabled_warnings': [ 4018 ],

      'direct_dependent_settings': {
        'include_dirs': [
          './src',
        ],
      },

      'conditions': [
        ['disable_qcms == 1', {
          'sources': [
            'src/empty.c',
          ],
        }, { # disable_qcms == 0
          'sources': [
            'src/chain.c',
            'src/chain.h',
            'src/iccread.c',
            'src/matrix.c',
            'src/matrix.h',
            'src/qcms.h',
            'src/qcmsint.h',
            'src/qcmstypes.h',
            'src/qcms_util.c',
            'src/transform.c',
            'src/transform_util.c',
            'src/transform_util.h',
          ],
          'conditions': [
            ['target_arch=="ia32" or target_arch=="x64"', {
              'defines': [
                'SSE2_ENABLE',
              ],
              'sources': [
                'src/transform-sse2.c',
              ],
            }],
          ],
        }],
        ['OS == "win"', {
          'msvs_disabled_warnings': [
            4056,  # overflow in floating-point constant arithmetic (INFINITY)
            4756,  # overflow in constant arithmetic (INFINITY)
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['disable_qcms == 0', {
      'targets': [
        {
          'target_name': 'qcms_tests',
          'product_name': 'qcms_tests',
          'type': 'executable',
          'dependencies': [
            'qcms',
          ],
          'conditions': [
            ['OS != "win"', {
              'libraries': [
                '-lm',
              ],
            }],
            ['target_arch=="ia32" or target_arch=="x64"', {
              'defines': [
                'SSE2_ENABLE',
              ],
            }],
          ],
          'sources': [
            'src/tests/qcms_test_main.c',
            'src/tests/qcms_test_internal_srgb.c',
            'src/tests/qcms_test_munsell.c',
            'src/tests/qcms_test_ntsc_gamut.c',
            'src/tests/qcms_test_output_trc.c',
            'src/tests/qcms_test_tetra_clut_rgba.c',
            'src/tests/qcms_test_util.c',
          ],
        },
      ],
    }],
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
