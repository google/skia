# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP for "dm" (Diamond Master, a.k.a Dungeon master, a.k.a GM 2).
{
  'include_dirs': [
    '../dm',
    '../gm',
    '../include/private',
    '../src/core',
    '../src/effects',
    '../src/images',
    '../src/lazy',
    '../src/utils',
    '../tools/debugger',
    '../tests',
  ],
  'dependencies': [
    'etc1.gyp:libetc1',
    'flags.gyp:flags',
    'jsoncpp.gyp:jsoncpp',
    'libpng.gyp:libpng',
    'skia_lib.gyp:skia_lib',
    'svg.gyp:svg',
    'tools.gyp:crash_handler',
    'tools.gyp:picture_utils',
    'tools.gyp:proc_stats',
    'tools.gyp:sk_tool_utils',
    'tools.gyp:url_data_manager',
    'tools.gyp:timer',
    'xml.gyp:xml',
    'xps.gyp:xps',
  ],
  'includes': [
    'gmslides.gypi',
    'pathops_unittest.gypi',
    'tests.gypi',
  ],
  'sources': [
    '../dm/DM.cpp',
    '../dm/DMSrcSink.cpp',
    '../dm/DMJsonWriter.cpp',
    '../gm/gm.cpp',

    '../src/utils/SkMultiPictureDocumentReader.cpp',
    '../tools/debugger/SkDebugCanvas.cpp',
    '../tools/debugger/SkDrawCommand.cpp',
    '../tools/debugger/SkJsonWriteBuffer.cpp',
    '../tools/debugger/SkObjectParser.cpp',
    '../tools/debugger/SkOverdrawMode.h',
    '../tools/debugger/SkOverdrawMode.cpp',
  ],
  'conditions': [
    [ 'skia_gpu == 1', {
      'dependencies': [ 'gputest.gyp:skgputest' ],
    }],
  ],
}
