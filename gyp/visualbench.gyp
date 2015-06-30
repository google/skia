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
      'include_dirs' : [
        '../include/gpu',
        '../src/core',
        '../src/images',
      ],
      'sources': [
        '../tools/VisualBench/VisualBench.h',
        '../tools/VisualBench/VisualBench.cpp',
      ],
      'dependencies': [
        'flags.gyp:flags_common',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:proc_stats',
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
