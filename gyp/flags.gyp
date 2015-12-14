# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build flag parser
#
{
  'targets': [
    {
      'target_name': 'flags',
      'type': 'static_library',
      'include_dirs': [
          '../include/private',
          '../src/core',
      ],
      'sources': [
        '../tools/flags/SkCommandLineFlags.cpp',
        '../tools/flags/SkCommandLineFlags.h',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../tools/flags',
        ],
      }
    },
    {
      'target_name': 'flags_common',
      'type': 'static_library',
      'include_dirs': [
          '../include/gpu',
          '../src/gpu',
      ],
      'sources': [
        '../tools/flags/SkCommonFlags.cpp',
        '../tools/flags/SkCommonFlags.h',
        '../tools/flags/SkCommonFlagsConfig.cpp',
        '../tools/flags/SkCommonFlagsConfig.h',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'flags.gyp:flags',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/gpu',
          '../src/gpu',
          '../tools/flags',
        ],
      }
    },
  ],
}
