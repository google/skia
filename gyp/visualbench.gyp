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
        '<!@(python find.py ../tools/VisualBench "*.cpp")',
        '<!@(python find.py ../tools/VisualBench "*.h")',
        '<!@(python find.py ../bench "*.cpp")',
      ],
      'sources!': [
        '../bench/nanobench.cpp',
        '../bench/nanobenchAndroid.cpp',
      ],
      'dependencies': [
        'etc1.gyp:libetc1',
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'gputest.gyp:skgputest',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:proc_stats',
        'tools.gyp:sk_tool_utils',
        'tools.gyp:timer',
        'views.gyp:views',
      ],
      'conditions' : [
        [ 'skia_os == "android" and skia_use_sdl == 0', {
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
        [ 'skia_os == "android" and skia_use_sdl == 1', {
          'dependencies': [
            'android_deps.gyp:Android_VisualBenchSDL',
          ],
        }],
      ],
    },
  ],
}
