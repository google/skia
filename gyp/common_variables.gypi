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
    'angle_path%': '../',
    'variables': {  # level 2

      # Variables needed by conditions list within the level-2 variables dict.
      'variables': {  # level 3
        'variables': { # level 4
          # We use 'skia_os' instead of 'OS' throughout our gyp files, to allow
          # for cross-compilation (e.g. building for either MacOS or iOS on Mac).
          # We set it automatically based on 'OS' (the host OS), but allow the
          # user to override it via GYP_DEFINES if they like.
          'skia_os%': '<(OS)',
        },
        'skia_os%': '<(skia_os)',

        'skia_android_framework%': 0,
        'conditions' : [
          [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "mac"]', {
            'skia_arch_type%': 'x86_64',
          }, {
            'skia_arch_type%': 'x86',
          }],
        ],
        'arm_version%': 0,
        'arm_neon%': 0,
        'skia_egl%': 0,
      },

      # Re-define all variables defined within the level-3 'variables' dict,
      # so that siblings of the level-2 'variables' dict can see them.
      # (skia_os will depend on skia_android_framework.)
      'skia_android_framework%': '<(skia_android_framework)',
      'skia_arch_type%': '<(skia_arch_type)',
      'arm_version%': '<(arm_version)',
      'arm_neon%': '<(arm_neon)',
      'skia_egl%': '<(skia_egl)',

      'conditions': [
        [ 'skia_android_framework == 1', {
          'skia_os%': 'android',
          'skia_chrome_utils%': 0,
          'skia_use_android_framework_defines%': 1,
          'skia_use_system_json%': 1,
        }, {
          'skia_os%': '<(skia_os)',
          'skia_chrome_utils%': 1,
          'skia_use_android_framework_defines%': 0,
          'skia_use_system_json%': 0,
        }],
        [ 'skia_os == "win"', {
          'os_posix%': 0,
        }, {
          'os_posix%': 1,
        }],
        [ 'skia_os == "android"', {
          'skia_static_initializers%': 0,
          'skia_egl%': 1,
        }, {
          'skia_static_initializers%': 1,
        }],
        [ 'skia_os == "ios"', {
          'skia_arch_type%': 'arm',
          'arm_version%': 7,
          'arm_neon%': 0, # neon asm files known not to work with the ios build
        }],
        [ 'skia_os == "android" and not skia_android_framework',
          # skia_freetype_static - on OS variants that normally would
          #     dynamically link the system FreeType library, don't do
          #     that; instead statically link to the version in
          #     third_party/freetype and third_party/externals/freetype.
          {
            'skia_freetype_static%': '1',
          }, {
            'skia_freetype_static%': '0',
          }
        ],
      ],

      # skia_no_fontconfig - On POSIX systems that would normally use the
      #     SkFontHost_fontconfig interface; use the SkFontHost_linux
      #     version instead.
      'skia_no_fontconfig%': '0',
      'skia_embedded_fonts%': '0',

      'skia_sanitizer%': '',
      'skia_scalar%': 'float',
      'skia_mesa%': 0,
      'skia_gpu_extra_dependency_path%': '',
      'skia_stroke_path_rendering%': 0,
      'skia_android_path_rendering%': 0,
      'skia_resource_cache_mb_limit%': 0,
      'skia_resource_cache_count_limit%': 0,
      'skia_angle%': 0,
      'skia_gdi%': 0,
      'skia_gpu%': 1,
      'skia_osx_deployment_target%': '',
      'skia_profile_enabled%': 0,
      'skia_win_debuggers_path%': '',
      'skia_shared_lib%': 0,
      'skia_opencl%': 0,
      'skia_force_distance_field_text%': 0,

      # These variables determine the default optimization level for different
      # compilers.
      'skia_default_vs_optimization_level': 3, # full (/Ox)
      'skia_default_gcc_optimization_level': 3, # -O3
    },

    'conditions': [
      [ 'skia_os in ["mac", "linux", "freebsd", "openbsd", "solaris", "android", "win"] '
            'and skia_android_framework == 0', {
        'skia_warnings_as_errors%': 1,
      }, {
        'skia_warnings_as_errors%': 0,
      }],

      # This variable allows the user to customize the optimization level used
      # by the compiler.  The user should be aware that this has different
      # meanings for different compilers and should exercise caution when
      # overriding it.
      [ 'skia_os == "win"', {
        'skia_release_optimization_level%': '<(skia_default_vs_optimization_level)',
      }, {
        'skia_release_optimization_level%': '<(skia_default_gcc_optimization_level)',
      }],
      [ 'skia_sanitizer', {
        'skia_clang_build': 1,
        'skia_keep_frame_pointer': 1,
      }, {
        'skia_clang_build%': 0,
        'skia_keep_frame_pointer%': 0,
      }],
      [ 'skia_shared_lib or skia_sanitizer or skia_os == "android"', {
          'skia_pic%' : 1,
        }, {
          'skia_pic%' : 0,
        }
      ],
    ],

    # Re-define all variables defined within the level-2 'variables' dict,
    # so that siblings of the level-1 'variables' dict can see them.
    'arm_version%': '<(arm_version)',
    'arm_neon%': '<(arm_neon)',
    'arm_neon_optional%': 0,
    'mips_arch_variant%': 'mips32',
    'mips_dsp%': 0,
    'skia_os%': '<(skia_os)',
    'os_posix%': '<(os_posix)',

    'skia_freetype_static%': '<(skia_freetype_static)',
    'skia_no_fontconfig%': '<(skia_no_fontconfig)',
    'skia_embedded_fonts%': '<(skia_embedded_fonts)',
    'skia_sanitizer%': '<(skia_sanitizer)',
    'skia_scalar%': '<(skia_scalar)',
    'skia_mesa%': '<(skia_mesa)',
    'skia_gpu_extra_dependency_path%': '<(skia_gpu_extra_dependency_path)',
    'skia_stroke_path_rendering%': '<(skia_stroke_path_rendering)',
    'skia_android_framework%': '<(skia_android_framework)',
    'skia_use_android_framework_defines%': '<(skia_use_android_framework_defines)',
    'skia_use_system_json%': '<(skia_use_system_json)',
    'skia_android_path_rendering%': '<(skia_android_path_rendering)',
    'skia_resource_cache_mb_limit%': '<(skia_resource_cache_mb_limit)',
    'skia_resource_cache_count_limit%': '<(skia_resource_cache_count_limit)',
    'skia_angle%': '<(skia_angle)',
    'skia_arch_type%': '<(skia_arch_type)',
    'skia_chrome_utils%': '<(skia_chrome_utils)',
    'skia_gdi%': '<(skia_gdi)',
    'skia_gpu%': '<(skia_gpu)',
    'skia_win_exceptions%': 0,
    'skia_win_ltcg%': 1,
    'sknx_no_simd%': 0,
    'skia_osx_deployment_target%': '<(skia_osx_deployment_target)',
    'skia_profile_enabled%': '<(skia_profile_enabled)',
    'skia_shared_lib%': '<(skia_shared_lib)',
    'skia_opencl%': '<(skia_opencl)',
    'skia_force_distance_field_text%': '<(skia_force_distance_field_text)',
    'skia_static_initializers%': '<(skia_static_initializers)',
    'ios_sdk_version%': '6.0',
    'skia_win_debuggers_path%': '<(skia_win_debuggers_path)',
    'skia_disable_inlining%': 0,
    'skia_moz2d%': 0,
    'skia_is_bot%': '<!(python -c "import os; print os.environ.get(\'CHROME_HEADLESS\', 0)")',
    'skia_egl%': '<(skia_egl)',
    'skia_fast%': 0,
    'skia_fast_flags': [
        '-O3',                   # Even for Debug builds.
        '-march=native',         # Use all features of and optimize for THIS machine.
        '-fomit-frame-pointer',  # Sometimes an extra register is nice, and cuts a push/pop.
        '-ffast-math',           # Optimize float math even when it breaks IEEE compliance.
        #'-flto'                  # Enable link-time optimization.
    ],

    # These are referenced by our .gypi files that list files (e.g. core.gypi)
    #
    'skia_src_path%': '../src',
    'skia_include_path%': '../include',
  },
}
