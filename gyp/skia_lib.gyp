# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# The minimal set of static libraries for basic Skia functionality.

{
  'variables': {
    'component_libs': [
      'core.gyp:core',
      'codec.gyp:codec',
      'codec_android.gyp:codec_android',
      'effects.gyp:effects',
      'images.gyp:images',
      'opts.gyp:opts',
      'ports.gyp:ports',
      'sfnt.gyp:sfnt',
      'utils.gyp:utils',
    ],
    'conditions': [
      [ '"x86" in skia_arch_type and skia_os != "android"', {
        'component_libs': [
          'opts.gyp:opts_ssse3',
          'opts.gyp:opts_sse41',
        ],
      }],
      [ 'arm_neon == 1', {
        'component_libs': [
          'opts.gyp:opts_neon',
        ],
      }],
      [ 'skia_gpu', {
        'component_libs': [
          'gpu.gyp:skgpu',
        ],
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'skia_lib',
      'sources': [ '<(skia_src_path)/core/SkForceCPlusPlusLinking.cpp', ],
      'conditions': [
        [ 'skia_shared_lib', {
          'conditions': [
            [ 'skia_os == "android"', {
              # The name skia will confuse the linker on android into using the system's libskia.so
              # instead of the one packaged with the apk. We simply choose a different name to fix
              # this.
              'product_name': 'skia_android',
            }, {
              'product_name': 'skia',
            }],
          ],
          'type': 'shared_library',
        }, {
          'type': 'none',
        }],
      ],
      'dependencies': [
        '<@(component_libs)',
      ],
      'export_dependent_settings': [
        '<@(component_libs)',
      ],
    },
  ],
}
