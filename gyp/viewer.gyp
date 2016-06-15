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
      'target_name': 'viewer',
      'type': 'executable',
      'includes' : [
        'gmslides.gypi',
      ],
      'include_dirs': [
        '../bench',
        '../gm',
        '../include/views',
        '../include/private',
        '../src/core',
        '../src/effects',
        '../src/gpu',
        '../src/images',
        '../src/image',
        '../src/views/unix',
        '../tools/timer',
      ],
      'sources': [
        '../gm/gm.cpp',
        '../src/views/SkTouchGesture.cpp',
        '../src/views/unix/keysym2ucs.c',
        '<!@(python find.py ../tools/viewer "*.cpp")',
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
      'conditions' : [
        [ 'skia_os == "android"', {
          'dependencies': [
            'android_deps.gyp:Android_EntryPoint',
            'android_deps.gyp:native_app_glue',
          ],
          'link_settings': {
            'libraries': [
              '-landroid',
            ],
          },
        }],
        [ 'skia_os == "linux"', {
          'link_settings': {
            'libraries': [
              '-lX11-xcb',
            ],
          },
        }],
        ['skia_os != "android"', {
          'sources/': [ ['exclude', '_android.(h|cpp)$'],
          ],
        }],
        ['skia_os != "linux"', {
          'sources/': [ 
            ['exclude', '_unix.(h|cpp)$'],
            ['exclude', 'keysym2ucs.c'],
          ],
        }],
        ['skia_os != "win"', {
          'sources/': [ ['exclude', '_win.(h|cpp)$'],
          ],
        }],
	['skia_vulkan == 0', {
	  'sources/': [ ['exclude', 'Vulkan']
	  ],
	}],
      ],
    },
  ],
}
