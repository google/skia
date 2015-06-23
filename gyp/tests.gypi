# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Common gypi for unit tests.
{
  'include_dirs': [
    '../src/codec',
    '../src/core',
    '../src/effects',
    '../src/image',
    '../src/lazy',
    '../src/images',
    '../src/pathops',
    '../src/pdf',
    '../src/pipe/utils',
    '../src/utils',
    '../src/utils/debugger',

    # Needed for TDStackNesterTest.
    '../experimental/PdfViewer',
    '../experimental/PdfViewer/src',
  ],
  'dependencies': [
    'experimental.gyp:experimental',
    'flags.gyp:flags_common',
    'pdf.gyp:pdf',
    'skia_lib.gyp:skia_lib',
    'tools.gyp:picture_utils',
    'tools.gyp:resources',
    'tools.gyp:sk_tool_utils',
  ],
  'conditions': [
    [ 'skia_os == "android"',
      {
        'include_dirs': [ '../src/ports', ],
      }, {
        'sources!': [ '../tests/FontMgrAndroidParserTest.cpp', ],
      }
    ],
    [ 'skia_android_framework == 1', {
      'libraries': [
        '-ldl',
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
    '../src/pipe/utils/SamplePipeControllers.cpp',
    '../experimental/PdfViewer/src/SkTDStackNester.h',
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
