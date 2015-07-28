# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build pathops skp clip test.
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'pathops_skpclip',
      'type': 'executable',
      'include_dirs': [
        '../include/private',
        '../src/core',
        '../src/effects',
        '../src/lazy',
        '../src/pathops',
        '../src/pipe/utils',
        '../src/utils',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:resources',
      ],
      'sources': [
        '../tests/PathOpsDebug.cpp',
        '../tests/PathOpsSkpClipTest.cpp',
      ],
      'conditions': [
        [ 'skia_android_framework == 1', {
          'libraries': [
            '-lskia',
          ],
          'libraries!': [
            '-lz',
            '-llog',
          ],
        }],
        [ 'skia_gpu == 1', {
          'include_dirs': [
            '../src/gpu',
          ],
        }],
      ],
    },
  ],
}
