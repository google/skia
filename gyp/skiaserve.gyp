# Copyright 2016 Google Inc.
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
      'target_name': 'skiaserve',
      'type': 'executable',
      'include_dirs': [
        '../src/core',
        '../src/gpu',
        '../src/ports',
        '../tools/debugger',
        '../tools/json',
      ],
      'sources': [ 
        # Stuff for the debug canvas
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
        '<!@(python find.py ../tools/skiaserve "*.cpp")',
        '<!@(python find.py ../tools/skiaserve/urlhandlers "*.cpp")',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'gputest.gyp:skgputest',
        'jsoncpp.gyp:jsoncpp',
        'libpng.gyp:libpng',
        'microhttpd.gyp:microhttpd',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:picture_utils',
        'tools.gyp:proc_stats',
        'tools.gyp:resources',
        'tools.gyp:url_data_manager',
      ],
    },
  ],
}
