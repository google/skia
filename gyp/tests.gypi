# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Common gypi for unit tests.
{
  'include_dirs': [
    '../include/private',
    '../src/codec',
    '../src/core',
    '../src/effects',
    '../src/image',
    '../src/lazy',
    '../src/images',
    '../src/pathops',
    '../src/pdf',
    '../src/ports',
    '../src/utils',
    '../src/utils/debugger',
  ],
  'dependencies': [
    'experimental.gyp:experimental',
    'flags.gyp:flags_common',
    'pdf.gyp:pdf',
    'skia_lib.gyp:skia_lib',
    'tools.gyp:picture_utils',
    'tools.gyp:resources',
    'tools.gyp:sk_tool_utils',
    'zlib.gyp:zlib',
  ],
  'conditions': [
    [ 'skia_os not in ["linux", "freebsd", "openbsd", "solaris", "chromeos", "android"]', {
        'sources!': [ '../tests/FontMgrAndroidParserTest.cpp', ],
    }],
    [ 'skia_android_framework == 1', {
      'libraries': [
        '-ldl',
      ],
    }],
    [ 'not skia_pdf', {
      'dependencies!': [ 'pdf.gyp:pdf', 'zlib.gyp:zlib' ],
      'dependencies': [ 'pdf.gyp:nopdf' ],
      'sources!': [ '<!@(python find.py ../tests "PDF*.c*")', ],
    }],
    [ 'skia_gpu_extra_tests_path', {
      'sources': [
        '<!@(python find.py <(skia_gpu_extra_tests_path) "*.c*")',
      ],
    }],
  ],
  'sources': [
    '../tests/Test.h',
    '<!@(python find.py ../tests "*.c*")',
    '../src/utils/debugger/SkDrawCommand.h',
    '../src/utils/debugger/SkDrawCommand.cpp',
    '../src/utils/debugger/SkDebugCanvas.h',
    '../src/utils/debugger/SkDebugCanvas.cpp',
    '../src/utils/debugger/SkObjectParser.h',
    '../src/utils/debugger/SkObjectParser.cpp',
  ],
  'sources!': [
    '../tests/SkpSkGrTest.cpp',
    '../tests/skia_test.cpp',
    '../tests/PathOpsAngleIdeas.cpp',
    '../tests/PathOpsBattles.cpp',
    '../tests/PathOpsCubicLineIntersectionIdeas.cpp',
    '../tests/PathOpsDebug.cpp',
    '../tests/PathOpsOpLoopThreadedTest.cpp',
    '../tests/PathOpsSkpClipTest.cpp',
  ],
}
