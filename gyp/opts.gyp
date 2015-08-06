# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Gyp file for building opts target.
{
  # Source lists live in opts.gypi.  This makes it easier to maintain our Chrome GYP/GN setup.
  # (To be honest, I'm not sure why we need to include common.gypi.  I thought it was automatic.)
  'variables': {
    'includes': [ 'common.gypi', 'opts.gypi' ],
  },

  # Generally we shove things into one 'opts' target conditioned on platform.
  # If a particular platform needs some files built with different flags,
  # those become separate targets: opts_ssse3, opts_sse41, opts_neon.

  'targets': [
    {
      'target_name': 'opts',
      'product_name': 'skia_opts',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'effects.gyp:*'
      ],
      'include_dirs': [
        '../include/private',
        '../src/core',
        '../src/opts',
        '../src/utils',
        '../include/utils',
      ],
      'conditions': [
        [ '"x86" in skia_arch_type and skia_os != "ios"', {
          'cflags': [ '-msse2' ],
          'dependencies': [ 'opts_ssse3', 'opts_sse41' ],
          'sources': [ '<@(sse2_sources)' ],
        }],

        [ 'skia_arch_type == "mips"', {
          'conditions': [
            [ '(mips_arch_variant == "mips32r2") and (mips_dsp == 1 or mips_dsp == 2)', {
                'sources': [ '<@(mips_dsp_sources)' ],
            },{
                'sources': [ '<@(none_sources)' ],
            }],
          ]
        }],

        [ '(skia_arch_type == "arm" and arm_version < 7) \
            or (skia_os == "ios") \
            or (skia_os == "android" \
                and skia_arch_type not in ["x86", "x86_64", "arm", "mips", \
                                           "arm64"])', {
          'sources': [ '<@(none_sources)' ],
        }],

        [ 'skia_arch_type == "arm" and arm_version >= 7', {
          # The assembly uses the frame pointer register (r7 in Thumb/r11 in
          # ARM), the compiler doesn't like that.
          'cflags!': [ '-fno-omit-frame-pointer', '-mapcs-frame', '-mapcs' ],
          'cflags':  [ '-fomit-frame-pointer' ],
          'variables': { 'arm_neon_optional%': '<(arm_neon_optional>' },
          'sources': [ '<@(armv7_sources)' ],
          'conditions': [
            [ 'arm_neon == 1 or arm_neon_optional == 1', {
              'dependencies': [ 'opts_neon' ]
            }],
          ],
        }],

        [ 'skia_arch_type == "arm64"', {
          'sources': [ '<@(arm64_sources)' ],
        }],

        [ 'skia_android_framework', {
          'cflags!': [
            '-msse2',
            '-mfpu=neon',
            '-fomit-frame-pointer',
          ]
        }],
      ],
    },
    {
      'target_name': 'opts_ssse3',
      'product_name': 'skia_opts_ssse3',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [ 'core.gyp:*' ],
      'include_dirs': [
          '../include/private',
          '../src/core',
          '../src/utils',
      ],
      'sources': [ '<@(ssse3_sources)' ],
      'conditions': [
        [ 'skia_os == "win"', {
            'defines' : [ 'SK_CPU_SSE_LEVEL=31' ],
        }],
        [ 'not skia_android_framework', {
          'cflags': [ '-mssse3' ],
        }],
      ],
    },
    {
      'target_name': 'opts_sse41',
      'product_name': 'skia_opts_sse41',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [ 'core.gyp:*' ],
      'include_dirs': [
          '../include/private',
          '../src/core',
          '../src/utils',
      ],
      'sources': [ '<@(sse41_sources)' ],
      'conditions': [
        [ 'skia_os == "win"', {
            'defines' : [ 'SK_CPU_SSE_LEVEL=41' ],
        }],
        [ 'not skia_android_framework', {
          'cflags': [ '-msse4.1' ],
        }],
        [ 'skia_os == "mac"', {
          'xcode_settings': { 'GCC_ENABLE_SSE41_EXTENSIONS': 'YES' },
        }],
      ],
    },
    {
      'target_name': 'opts_neon',
      'product_name': 'skia_opts_neon',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'effects.gyp:*'
      ],
      'include_dirs': [
        '../include/private',
        '../src/core',
        '../src/opts',
        '../src/utils',
      ],
      'sources': [ '<@(neon_sources)' ],
      'cflags!': [
        '-fno-omit-frame-pointer',
        '-mfpu=vfp',  # remove them all, just in case.
        '-mfpu=vfpv3',
        '-mfpu=vfpv3-d16',
      ],
      'conditions': [
        [ 'not skia_android_framework', {
          'cflags': [
            '-mfpu=neon',
            '-fomit-frame-pointer',
          ],
        }],
      ],
      'ldflags': [
        '-march=armv7-a',
        '-Wl,--fix-cortex-a8',
      ],
    },
  ],
}
