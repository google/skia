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
      'SK_INTERNAL',
      'SK_GAMMA_SRGB',
      'SK_GAMMA_APPLY_TO_A8',
      'QT_NO_KEYWORDS',
      # The discardable resource cache is tested via a special bot. This is 
      # disabled here so we test the non-discardable use case.
      # 'SK_USE_DISCARDABLE_SCALEDIMAGECACHE',
    ],

    # Validate the 'skia_os' setting against 'OS', because only certain
    # combinations work.  You should only override 'skia_os' for certain
    # situations, like building for iOS on a Mac.
    'variables': {
      'conditions': [
        [ 'skia_os != OS and not (skia_os == "ios" and OS == "mac")', {
          'error': '<!(Cannot build with skia_os=<(skia_os) on OS=<(OS))',
        }],
        [ 'skia_mesa and skia_os not in ["mac", "linux"]', {
          'error': '<!(skia_mesa=1 only supported with skia_os="mac" or "linux".)',
        }],
        [ 'skia_angle and not (skia_os == "win" or skia_os == "linux" or skia_os == "mac")', {
          'error': '<!(skia_angle=1 only supported with skia_os="win" or skia_os="linux" or skia_os="mac".)',
        }],
      ],
    },
    'includes': [
      'common_conditions.gypi',
    ],
    'conditions': [
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
      [ 'skia_vulkan', {
        'defines': [
          'SK_VULKAN',
        ],
        'direct_dependent_settings': {
          'defines': [
            'SK_VULKAN',
          ],
        },
      }],
      [ 'skia_command_buffer', {
        'defines': [
          'SK_COMMAND_BUFFER',
        ],
        'direct_dependent_settings': {
          'defines': [
            'SK_COMMAND_BUFFER',
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
      [ 'skia_android_framework==0', {
        # These defines are not used for skia_android_framework, where we build
        # one makefile and allow someone to add SK_DEBUG etc for their own
        # debugging purposes.
        'configurations': {
          'Debug':   { 'defines': [ 'SK_DEBUG=1' ] },
          'Release': { 'defines': [ 'NDEBUG' ] },
          'Release_Developer': {
            'inherit_from': ['Release'],
            'defines': [ 'SK_DEBUG=1' ],
            'conditions': [
              [ 'skia_clang_build == 0', {
                # gcc has problems providing useful warnings of these types for
                # optimized builds.
                'cflags': [
                  '-Wno-array-bounds',
                  '-Wno-maybe-uninitialized',
                  '-Wno-strict-overflow',
                ],
              }],
            ],
          },
        },
      }],
    ],
  }, # end 'target_defaults'
}
