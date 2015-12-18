# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'nanobench',
      'type': 'executable',
      'sources': [
        '../gm/gm.cpp',
      ],
      'includes': [
        'bench.gypi',
        'gmslides.gypi',
      ],
      'dependencies': [
        'flags.gyp:flags_common',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:proc_stats',
        'tools.gyp:timer',
      ],
      'conditions': [
        ['skia_android_framework', {
          'libraries': [
            '-lskia',
            '-landroid',
            '-lhwui',
            '-lutils',
          ],
          'include_dirs': [
            '../../../frameworks/base/libs/hwui/',
          ],
          'dependencies': [
            'tools.gyp:android_utils',
          ],
        }],
      ],
    },
  ],
}
