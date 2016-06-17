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
      'target_name': 'kilobench',
      'type': 'executable',
      'include_dirs': [
        '../bench',
        '../gm',
        '../include/private',
        '../src/core',
        '../src/gpu',
        '../tools/VisualBench',
      ],
      'sources': [ 
        '<!@(python find.py ../tools/kilobench "*.cpp")',
        '../bench/Benchmark.cpp',
        '../tools/VisualBench/VisualSKPBench.cpp',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'gpu.gyp:skgpu',
        'gputest.gyp:skgputest',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:proc_stats',
        'tools.gyp:resources',
        'tools.gyp:timer',
      ],
    },
  ],
}
