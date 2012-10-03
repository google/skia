# Copyright 2012 The Android Open Source Project
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  # Get ready for the ugly...
  #
  # - We have to nest our variables dictionaries multiple levels deep, so that
  #   this and other gyp files can rely on previously-set variable values in
  #   their 'variables': { 'conditions': [] } clauses.
  #
  #   Example 1:
  #   Within this file, we use the value of variable 'skia_os' to set the
  #   value of variable 'os_posix', so 'skia_os' must be defined within a
  #   "more inner" (enclosed) scope than 'os_posix'.
  #
  #   Example 2:
  #   http://src.chromium.org/viewvc/chrome/trunk/src/third_party/libjpeg/libjpeg.gyp?revision=102306 ,
  #   which we currently import into our build, uses the value of 'os_posix'
  #   within the 'conditions' list in its 'variables' dict.
  #   In order for that to work, it needs the value of 'os_posix' to have been
  #   set within a "more inner" (enclosed) scope than its own 'variables' dict.
  #
  # - On the other hand, key/value pairs of a given 'variable' dict are only
  #   inherited by:
  #   1. directly enclosing 'variable' dicts, and
  #   2. "sibling" 'variable' dicts (which, I guess, are combined into a single
  #      'variable' dict during gyp processing)
  #   and NOT inherited by "uncles" (siblings of directly enclosing 'variable'
  #   dicts), so we have to re-define every variable at every enclosure level
  #   within our ridiculous matryoshka doll of 'variable' dicts.  That's why
  #   we have variable definitions like this:  'skia_os%': '<(skia_os)',
  #
  # See http://src.chromium.org/viewvc/chrome/trunk/src/build/common.gypi?revision=127004 ,
  # which deals with these same constraints in a similar manner.
  #
  'variables': {  # level 1
    'variables': {  # level 2

      # Variables needed by conditions list within the level-2 variables dict.
      'variables': {  # level 3
        # We use 'skia_os' instead of 'OS' throughout our gyp files, to allow
        # for cross-compilation (e.g. building for either MacOS or iOS on Mac).
        # We set it automatically based on 'OS' (the host OS), but allow the
        # user to override it via GYP_DEFINES if they like.
        'skia_os%': '<(OS)',
      },

      # Re-define all variables defined within the level-3 'variables' dict,
      # so that siblings of the level-2 'variables' dict can see them.
      'skia_os%': '<(skia_os)',

      'conditions': [
        ['skia_os == "win"', {
          'os_posix%': 0,
        }, {
          'os_posix%': 1,
        }],
        ['skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
          'skia_arch_width%': 64,
        }, {
          'skia_arch_width%': 32,
        }],
        ['skia_os == "android"', {
          'skia_static_initializers%': 0,
        }, {
          'skia_static_initializers%': 1,
        }],
      ],

      'skia_scalar%': 'float',
      'skia_mesa%': 0,
      'skia_nv_path_rendering%': 0,
      'skia_angle%': 0,
      'skia_directwrite%': 0,
      'android_make_apk%': 1,
      'skia_nacl%': 0,
      'skia_gpu%': 1,
      'ios_sdk_dir%': '/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS',
      'ios_sdk_version%': '6.0',
      'skia_osx_sdkroot%': 'macosx10.6',
    },

    # Re-define all variables defined within the level-2 'variables' dict,
    # so that siblings of the level-1 'variables' dict can see them.
    'skia_os%': '<(skia_os)',
    'os_posix%': '<(os_posix)',
    'skia_scalar%': '<(skia_scalar)',
    'skia_mesa%': '<(skia_mesa)',
    'skia_nv_path_rendering%': '<(skia_nv_path_rendering)',
    'skia_angle%': '<(skia_angle)',
    'skia_arch_width%': '<(skia_arch_width)',
    'skia_directwrite%': '<(skia_directwrite)',
    'android_make_apk%': '<(android_make_apk)',
    'skia_nacl%': '<(skia_nacl)',
    'skia_gpu%': '<(skia_gpu)',
    'skia_osx_sdkroot%': '<(skia_osx_sdkroot)',
    'skia_static_initializers%': '<(skia_static_initializers)',
    'ios_sdk_version%': '<(ios_sdk_version)',
    'ios_sdk_dir%': '<(ios_sdk_dir)',

    'conditions': [
      ['skia_os == "ios"', {
        'skia_arch_type%': 'arm',
        'armv7%': 1,
        'arm_neon%': 0, # neon asm files known not to work with the ios build
      },{ # skia_os is not ios
        'skia_arch_type%': 'x86',
      }],
    ],
    # These are referenced by our .gypi files that list files (e.g. core.gypi)
    #
    'skia_src_path%': '../src',
    'skia_include_path%': '../include',
  },
}
# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
