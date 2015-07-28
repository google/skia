# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build visual bench tool
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'visualbench',
      'type': 'executable',
      'includes' : [
        'gmslides.gypi',
      ],
      'include_dirs' : [
        '../bench',
        '../include/gpu',
        '../include/private',
        '../src/core',
        '../src/effects',
        '../src/images',
      ],
      'sources': [
        '../gm/gm.cpp',
        '../tools/VisualBench/VisualBench.h',
        '../tools/VisualBench/VisualBench.cpp',
        '../tools/VisualBench/VisualBenchmarkStream.h',
        '../tools/VisualBench/VisualBenchmarkStream.cpp',
        '../tools/VisualBench/VisualSKPBench.h',
        '../tools/VisualBench/VisualSKPBench.cpp',
        '<!@(python find.py ../bench "*.cpp")',
      ],
      'sources!': [
        '../bench/nanobench.cpp',
        '../bench/nanobenchAndroid.cpp',
      ],
      'dependencies': [
        'etc1.gyp:libetc1',
        'flags.gyp:flags',
        'gputest.gyp:skgputest',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:proc_stats',
        'tools.gyp:sk_tool_utils',
        'tools.gyp:timer',
        'views.gyp:views',
      ],
      'conditions' : [
        [ 'skia_os == "android"', {
          'dependencies': [
            'android_deps.gyp:Android_VisualBench',
            'android_deps.gyp:native_app_glue',
          ],
         'link_settings': {
            'libraries': [
              '-landroid',
              '-lGLESv2',
              '-lEGL',
            ],
          },
        }],
      ],
    },
  ],
}
