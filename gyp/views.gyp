# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Views is the Skia windowing toolkit.
# It provides:
#  * A portable means of creating native windows.
#  * Events.
#  * Basic widgets and controls.

{
  'targets': [
    {
      'target_name': 'views',
      'product_name': 'skia_views',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'xml.gyp:xml',
      ],
      'include_dirs': [
        '../include/private',
        '../include/views',
        '../include/views/unix',
        '../src/core',
        '../src/gpu',
      ],
      'sources': [
        '../include/views/SkApplication.h',
        '../include/views/SkEvent.h',
        '../include/views/SkEventSink.h',
        '../include/views/SkKey.h',
        '../include/views/SkOSMenu.h',
        '../include/views/SkOSWindow_Mac.h',
        '../include/views/SkOSWindow_SDL.h',
        '../include/views/SkOSWindow_Unix.h',
        '../include/views/SkOSWindow_Win.h',
        '../include/views/SkSystemEventTypes.h',
        '../include/views/SkTouchGesture.h',
        '../include/views/SkView.h',
        '../include/views/SkWindow.h',

        '../src/views/SkEvent.cpp',
        '../src/views/SkEventSink.cpp',
        '../src/views/SkOSMenu.cpp',
        '../src/views/SkTagList.cpp',
        '../src/views/SkTagList.h',
        '../src/views/SkTouchGesture.cpp',
        '../src/views/SkView.cpp',
        '../src/views/SkViewPriv.cpp',
        '../src/views/SkViewPriv.h',
        '../src/views/SkWindow.cpp',
            
        # Unix
        '../src/views/unix/SkOSWindow_Unix.cpp',
        '../src/views/unix/keysym2ucs.c',
        '../src/views/unix/skia_unix.cpp',

        # Mac
        '../src/views/mac/SkEventNotifier.h',
        '../src/views/mac/SkEventNotifier.mm',
        '../src/views/mac/SkTextFieldCell.h',
        '../src/views/mac/SkTextFieldCell.m',
        '../src/views/mac/SkNSView.h',
        '../src/views/mac/SkNSView.mm',
        '../src/views/mac/SkOSWindow_Mac.mm',
        '../src/views/mac/skia_mac.mm',

        # Windows
        '../src/views/win/SkOSWindow_win.cpp',
        '../src/views/win/skia_win.cpp',
      ],
      'conditions': [
        [ 'skia_gpu == 1', {
          'include_dirs' : [
            '../src/gpu',
          ],
        }],
        [ 'skia_os == "mac" and skia_use_sdl == 0', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
            ],
          },
        },{
          'sources!': [
          '../src/views/mac/SkEventNotifier.h',
          '../src/views/mac/SkEventNotifier.mm',
          '../src/views/mac/SkTextFieldCell.h',
          '../src/views/mac/SkTextFieldCell.m',
          '../src/views/mac/SkNSView.h',
          '../src/views/mac/SkNSView.mm',
          '../src/views/mac/SkOSWindow_Mac.mm',
          '../src/views/mac/skia_mac.mm',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"] and skia_use_sdl == 0', {
          'link_settings': {
            'libraries': [
              '-lGL',
              '-lGLU',
              '-lX11',
            ],
          },
        },{
          'sources!': [
            '../src/views/unix/SkOSWindow_Unix.cpp',
            '../src/views/unix/keysym2ucs.c',
            '../src/views/unix/skia_unix.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
        },{
          'sources!': [
            '../src/views/win/SkOSWindow_win.cpp',
            '../src/views/win/skia_win.cpp',
          ],
        }],
        [ 'skia_gpu == 1', {
          'include_dirs': [
            '../include/gpu',
          ],
        }],
        [ 'skia_angle', {
          'dependencies': [
            'angle.gyp:*',
          ],
        }],
        [ 'skia_use_sdl == 1', {
          'defines': [
            'SK_USE_SDL',
          ],
          'dependencies': [
            'sdl.gyp:sdl',
          ],
          'sources!': [
             # linux sources
             '../src/views/unix/SkOSWindow_Unix.cpp',
             '../src/views/unix/keysym2ucs.c',
             '../src/views/unix/skia_unix.cpp',

             # mac sources
             '../src/views/mac/SkEventNotifier.h',
             '../src/views/mac/SkEventNotifier.mm',
             '../src/views/mac/SkTextFieldCell.h',
             '../src/views/mac/SkTextFieldCell.m',
             '../src/views/mac/SkNSView.h',
             '../src/views/mac/SkNSView.mm',
             '../src/views/mac/SkOSWindow_Mac.mm',
             '../src/views/mac/skia_mac.mm',

             # win sources
             '../src/views/win/SkOSWindow_win.cpp',
             '../src/views/win/skia_win.cpp',
          ],
          'sources': [
            '../src/views/sdl/SkOSWindow_SDL.cpp',
          ],
          'export_dependent_settings': [
            'sdl.gyp:sdl',
          ],
          'conditions': [
            [ 'skia_os == "mac"', {
              'include_dirs': [
                  '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework/Headers',
              ],
              'link_settings': {
                'libraries': [
                  '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
                ],
              }
            }],
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/views',
        ],
      },
      'export_dependent_settings': [
        'xml.gyp:xml',
      ],
    },
  ],
}
