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
        'angle.gyp:*',
        'xml.gyp:*',
      ],
      'include_dirs': [
        '../include/views',
        '../include/views/unix',
      ],
      'sources': [
        '../include/views/SkApplication.h',
        '../include/views/SkBGViewArtist.h',
        '../include/views/SkEvent.h',
        '../include/views/SkEventSink.h',
        '../include/views/SkKey.h',
        '../include/views/SkOSMenu.h',
        '../include/views/SkOSWindow_Mac.h',
        '../include/views/SkOSWindow_NaCl.h',
        '../include/views/SkOSWindow_SDL.h',
        '../include/views/SkOSWindow_Unix.h',
        '../include/views/SkOSWindow_Win.h',
        '../include/views/SkStackViewLayout.h',
        '../include/views/SkSystemEventTypes.h',
        '../include/views/SkTextBox.h',
        '../include/views/SkTouchGesture.h',
        '../include/views/SkView.h',
        '../include/views/SkViewInflate.h',
        '../include/views/SkWidget.h',
        '../include/views/SkWindow.h',

        '../src/views/SkBGViewArtist.cpp',
        '../src/views/SkEvent.cpp',
        '../src/views/SkEventSink.cpp',
        '../src/views/SkOSMenu.cpp',
        '../src/views/SkParsePaint.cpp',
        '../src/views/SkProgressView.cpp',
        '../src/views/SkStackViewLayout.cpp',
        '../src/views/SkTagList.cpp',
        '../src/views/SkTagList.h',
        '../src/views/SkTextBox.cpp',
        '../src/views/SkTouchGesture.cpp',
        '../src/views/SkView.cpp',
        '../src/views/SkViewInflate.cpp',
        '../src/views/SkViewPriv.cpp',
        '../src/views/SkViewPriv.h',
        '../src/views/SkWidgets.cpp',
        '../src/views/SkWindow.cpp',

        # Mac
        '../src/views/mac/SkOSWindow_Mac.mm',
        '../src/views/mac/skia_mac.mm',

        # SDL
        '../src/views/SDL/SkOSWindow_SDL.cpp',

        # *nix
        '../src/views/unix/SkOSWindow_Unix.cpp',
        '../src/views/unix/keysym2ucs.c',
        '../src/views/unix/skia_unix.cpp',

        # Windows
        '../src/views/win/SkOSWindow_win.cpp',
        '../src/views/win/skia_win.cpp',

      ],
      'sources!' : [
        '../src/views/SDL/SkOSWindow_SDL.cpp',
      ],
      'conditions': [
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
            ],
          },
        },{
          'sources!': [
            '../src/views/mac/SkOSWindow_Mac.mm',
            '../src/views/mac/skia_mac.mm',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
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
        [ 'skia_os == "nacl"', {
          'sources!': [
            '../src/views/unix/SkOSWindow_Unix.cpp',
            '../src/views/unix/keysym2ucs.c',
            '../src/views/unix/skia_unix.cpp',
          ],
        }, {
          'sources!': [
            '../src/views/nacl/SkOSWindow_NaCl.cpp',
          ],
        }],
        [ 'skia_gpu == 1', {
          'include_dirs': [
            '../include/gpu',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/views',
        ],
      },
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
