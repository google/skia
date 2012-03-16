# Copyright 2011 The Android Open Source Project
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This file is automatically included by gyp_skia when building any target.

{
  # Define all variables, allowing for override in GYP_DEFINES.
  #
  # One such variable is 'skia_os', which we use instead of 'OS' throughout
  # our gyp files.  We set it automatically based on 'OS', but allow the
  # user to override it via GYP_DEFINES if they like.
  'variables': {
    'skia_scalar%': 'float',
    'skia_os%': '<(OS)',
    'skia_mesa%': 0,
    'skia_target_arch%': 'x86',
  },

  'target_defaults': {

    # Validate the 'skia_os' setting against 'OS', because only certain
    # combinations work.  You should only override 'skia_os' for certain
    # situations, like building for iOS on a Mac.
    'variables': {
      'conditions': [
        ['skia_os != OS and not (skia_os == "ios" and OS == "mac")',
          {'error': '<!(Cannot build with skia_os=<(skia_os) on OS=<(OS))'}],
        ['skia_mesa and skia_os not in ["mac", "linux"]',
          {'error': '<!(skia_mesa=1 only supported with skia_os="mac" or "linux".)'}],
      ],
    },
    'includes': [
      'common_conditions.gypi'
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
    ],
    'configurations': {
      'Debug': {
        'defines': [
          'SK_DEBUG',
          'GR_DEBUG=1',
        ],
      },
      'Release': {
        'defines': [
          'SK_RELEASE',
          'GR_RELEASE=1',
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
