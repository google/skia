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
      'target_name': 'vulkanviewer',
      'type': 'executable',
      'includes' : [
        'gmslides.gypi',
      ],
      'include_dirs': [
        '../bench',
        '../gm',
        '../include/private',
        '../src/core',
        '../src/effects',
        '../src/gpu',
        '../src/images',
        '../src/image',
        '../tools/timer',
      ],
      'sources': [
        '../gm/gm.cpp',
        '<!@(python find.py ../tools/vulkan "*.cpp")',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'gputest.gyp:skgputest',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:proc_stats',
        'tools.gyp:resources',
        'tools.gyp:sk_tool_utils',
        'tools.gyp:timer',
        'tools.gyp:url_data_manager',
      ],
    },
  ],
}
