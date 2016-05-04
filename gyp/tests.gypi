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
    '../tools/debugger',
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
    [ 'skia_os not in ["linux", "freebsd", "openbsd", "solaris", "android"]', {
        'sources!': [ '../tests/FontMgrAndroidParserTest.cpp', ],
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
    '../tools/debugger/SkDrawCommand.h',
    '../tools/debugger/SkDrawCommand.cpp',
    '../tools/debugger/SkDebugCanvas.h',
    '../tools/debugger/SkDebugCanvas.cpp',
    '../tools/debugger/SkJsonWriteBuffer.h',
    '../tools/debugger/SkJsonWriteBuffer.cpp',
    '../tools/debugger/SkObjectParser.h',
    '../tools/debugger/SkObjectParser.cpp',
    '../tools/debugger/SkOverdrawMode.h',
    '../tools/debugger/SkOverdrawMode.cpp',
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
