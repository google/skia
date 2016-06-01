# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This build file has been adapted for use in Skia.  The contents of third_party/qcms
# are copied directly from Chromium.
{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'qcms',
      'type': 'static_library',

      # Warning (sign-conversion) fixed upstream by large refactoring. Can be
      # removed on next roll.
      'msvs_disabled_warnings': [ 4018 ],

      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/qcms/src/',
        ],
      },

      'sources': [
        '../third_party/qcms/src/chain.c',
        '../third_party/qcms/src/chain.h',
        '../third_party/qcms/src/iccread.c',
        '../third_party/qcms/src/matrix.c',
        '../third_party/qcms/src/matrix.h',
        '../third_party/qcms/src/qcms.h',
        '../third_party/qcms/src/qcmsint.h',
        '../third_party/qcms/src/qcmstypes.h',
        '../third_party/qcms/src/qcms_util.c',
        '../third_party/qcms/src/transform.c',
        '../third_party/qcms/src/transform_util.c',
        '../third_party/qcms/src/transform_util.h',
      ],
      'conditions': [
        ['"x86" in skia_arch_type', {
          'defines': [
            'SSE2_ENABLE',
          ],
          'sources': [
            '../third_party/qcms/src/transform-sse2.c',
          ],
        }],
        ['skia_os == "win"', {
          'msvs_disabled_warnings': [
            4056,  # overflow in floating-point constant arithmetic (INFINITY)
            4756,  # overflow in constant arithmetic (INFINITY)
          ],
        }],
      ],
      
      # Disable warnings
      'cflags': [
        '-w',
      ],
      'xcode_settings': {
        'WARNING_CFLAGS': [
          '-w'
        ],
      },
      'msvs_settings': {
        'VCCLCompilerTool': {
          'WarningLevel': '0',
        },
      },
      
    },
  ],
}
