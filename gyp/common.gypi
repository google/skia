# Copyright 2011 The Android Open Source Project
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This file is automatically included by gyp_skia when building any target.

{
  'includes': [
    'common_variables.gypi',
  ],

  'target_defaults': {
    'defines': [
      'SK_GAMMA_SRGB',
      'SK_GAMMA_APPLY_TO_A8',
    ],

    # Validate the 'skia_os' setting against 'OS', because only certain
    # combinations work.  You should only override 'skia_os' for certain
    # situations, like building for iOS on a Mac.
    'variables': {
      'conditions': [
        [ 'skia_os != OS and not ((skia_os == "ios" and OS == "mac") or \
                                  (skia_os == "nacl" and OS == "linux") or \
                                  (skia_os == "chromeos" and OS == "linux"))', {
          'error': '<!(Cannot build with skia_os=<(skia_os) on OS=<(OS))',
        }],
        [ 'skia_mesa and skia_os not in ["mac", "linux"]', {
          'error': '<!(skia_mesa=1 only supported with skia_os="mac" or "linux".)',
        }],
        [ 'skia_angle and not skia_os == "win"', {
          'error': '<!(skia_angle=1 only supported with skia_os="win".)',
        }],
        [ 'skia_arch_width != 32 and skia_arch_width != 64', {
          'error': '<!(skia_arch_width can only be 32 or 64 bits not <(skia_arch_width) bits)',
        }],
        [ 'skia_os == "nacl" and OS != "linux"', {
          'error': '<!(Skia NaCl build only currently supported on Linux.)',
        }],
        [ 'skia_os == "chromeos" and OS != "linux"', {
          'error': '<!(Skia ChromeOS build is only supported on Linux.)',
        }],
      ],
    },
    'includes': [
      'common_conditions.gypi',
    ],
    'conditions': [
      [ 'skia_scalar == "float"',
        {
          'defines': [
            'SK_SCALAR_IS_FLOAT',
            'SK_CAN_USE_FLOAT',
          ],
        }, { # else, skia_scalar != "float"
          'defines': [
            'SK_SCALAR_IS_FIXED',
            'SK_CAN_USE_FLOAT',  # we can still use floats along the way
          ],
        }
      ],
      [ 'skia_mesa', {
        'defines': [
          'SK_MESA',
        ],
        'direct_dependent_settings': {
          'defines': [
            'SK_MESA',
          ],
        },
      }],
      [ 'skia_angle', {
        'defines': [
          'SK_ANGLE',
        ],
        'direct_dependent_settings': {
          'defines': [
            'SK_ANGLE',
          ],
        },
      }],
      [ 'skia_win_debuggers_path and skia_os == "win"',
        {
          'defines': [
            'SK_USE_CDB',
          ],
        },
      ],
    ],
    'configurations': {
      'Debug': {
        'defines': [
          'SK_DEBUG',
          'SK_DEVELOPER=1',
        ],
      },
      'Release': {
        'defines': [
          'SK_RELEASE',
        ],
      },
      'Release_Developer': {
        'inherit_from': ['Release'],
        'defines': [
          'SK_DEVELOPER=1',
        ],
      },
    },
  }, # end 'target_defaults'
}
# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
