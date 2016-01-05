# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build pathops unit tests.
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'pathops_unittest',
      'type': 'executable',
      'includes': [
        'pathops_unittest.gypi',
      ],
      'dependencies': [
        'flags.gyp:flags_common',
        'tools.gyp:crash_handler',
      ],
      'sources': [
        '../tests/PathOpsAngleIdeas.cpp',
        '../tests/PathOpsBattles.cpp',
        '../tests/PathOpsCubicLineIntersectionIdeas.cpp',
        '../tests/PathOpsDebug.cpp',
        '../tests/PathOpsOpLoopThreadedTest.cpp',
        '../tests/PathOpsTSectDebug.h',
        '../tests/skia_test.cpp',
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
	  'sources': [
            '../src/gpu/GrContextFactory.cpp',
            '../src/gpu/GrContextFactory.h',
          ]
        }],
      ],
    },
  ],
}
