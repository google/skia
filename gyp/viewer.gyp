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
        'samples.gypi',
      ],
      'include_dirs': [
        '../bench',
        '../gm',
        '../include/private',
        '../src/core',
        '../src/effects',
        '../src/gpu',
        '../src/image',
        '../src/images',
        '../src/pathops',
        '../src/views/unix',
        '../tools/timer',
      ],
      'sources': [
        '../gm/gm.cpp',
        '<!@(python find.py ../tools/viewer "*.cpp")',

        # views (subset of files for the Android build)
        '../src/views/SkEvent.cpp',
        '../src/views/SkEventSink.cpp',
        '../src/views/SkOSMenu.cpp',
        '../src/views/SkTagList.cpp',
        '../src/views/SkTagList.h',
        '../src/views/SkTouchGesture.cpp',
        '../src/views/SkView.cpp',
        '../src/views/SkViewPriv.cpp',
        '../src/views/SkViewPriv.h',
        '../src/views/unix/keysym2ucs.c',
      ],
      'sources!': [
        '../samplecode/SampleSkLayer.cpp', #relies on SkMatrix44 which doesn't compile
        '../samplecode/SampleFontCache.cpp', #relies on pthread.h
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
          # views depends on SkOSWindow_android, which we don't want to include
          # so we only include the minimum set of views files in sources
          'dependencies!': [
            'views.gyp:views',
          ],
          'link_settings': {
            'libraries': [
              '-landroid',
            ],
          },
        }],
        [ 'skia_os == "linux" and skia_vulkan == 1', {
          'link_settings': {
            'libraries': [
              '-lX11-xcb',
            ],
          },
        }],
        ['skia_os != "android"', {
          'sources/': [
            ['exclude', '_android.(h|cpp)$'],
            ['exclude', 'src/views'],
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
