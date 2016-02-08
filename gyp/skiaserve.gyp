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
        '../src/ports',
        '../src/utils/debugger',
        '../tools/json',
      ],
      'sources': [ 
        # Stuff for the debug canvas
        '../src/utils/debugger/SkDrawCommand.h',
        '../src/utils/debugger/SkDrawCommand.cpp',
        '../src/utils/debugger/SkDebugCanvas.h',
        '../src/utils/debugger/SkDebugCanvas.cpp',
        '../src/utils/debugger/SkObjectParser.h',
        '../src/utils/debugger/SkObjectParser.cpp',
        '../src/utils/debugger/SkOverdrawMode.h',
        '../src/utils/debugger/SkOverdrawMode.cpp',
        '<!@(python find.py ../tools/skiaserve "*.cpp")',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'gputest.gyp:skgputest',
        'json.gyp:json',
        'jsoncpp.gyp:jsoncpp',
        'microhttpd.gyp:microhttpd',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:proc_stats',
        'tools.gyp:resources',
        'tools.gyp:url_data_manager',
      ],
    },
  ],
}
