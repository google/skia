{
  'includes': [
    'target_defaults.gypi',
  ],
  'targets': [
    {
      'target_name': 'utils',
      'type': 'static_library',
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../include/utils',
        '../include/utils/mac',
        '../include/utils/unix',
        '../include/views',
        '../include/effects',
        '../include/xml',
      ],
      'sources': [
        '../include/utils/SkBoundaryPatch.h',
        '../include/utils/SkCamera.h',
        '../include/utils/SkCubicInterval.h',
        '../include/utils/SkCullPoints.h',
        '../include/utils/SkDumpCanvas.h',
        '../include/utils/SkEGLContext.h',
        '../include/utils/SkGLCanvas.h',
        '../include/utils/SkInterpolator.h',
        '../include/utils/SkLayer.h',
        '../include/utils/SkMeshUtils.h',
        '../include/utils/SkNinePatch.h',
        '../include/utils/SkNWayCanvas.h',
        '../include/utils/SkParse.h',
        '../include/utils/SkParsePaint.h',
        '../include/utils/SkParsePath.h',
        '../include/utils/SkProxyCanvas.h',
        '../include/utils/SkSfntUtils.h',
        '../include/utils/SkTextBox.h',
        '../include/utils/SkUnitMappers.h',

        '../src/utils/SkBoundaryPatch.cpp',
        '../src/utils/SkCamera.cpp',
        '../src/utils/SkColorMatrix.cpp',
        '../src/utils/SkCubicInterval.cpp',
        '../src/utils/SkCullPoints.cpp',
        '../src/utils/SkDumpCanvas.cpp',
        '../src/utils/SkEGLContext_none.cpp',
        '../src/utils/SkInterpolator.cpp',
        '../src/utils/SkLayer.cpp',
        '../src/utils/SkMeshUtils.cpp',
        '../src/utils/SkNinePatch.cpp',
        '../src/utils/SkNWayCanvas.cpp',
        '../src/utils/SkOSFile.cpp',
        '../src/utils/SkParse.cpp',
        '../src/utils/SkParseColor.cpp',
        '../src/utils/SkParsePath.cpp',
        '../src/utils/SkProxyCanvas.cpp',
        '../src/utils/SkSfntUtils.cpp',
        '../src/utils/SkUnitMappers.cpp',

        '../include/utils/mac/SkCGUtils.h',
        '../src/utils/mac/SkCreateCGImageRef.cpp',
        '../src/utils/mac/SkEGLContext_mac.cpp',
        '../src/utils/mac/skia_mac.cpp',
        '../src/utils/mac/SkOSWindow_Mac.cpp',

        '../src/utils/mesa/SkEGLContext_Mesa.cpp',

        '../src/utils/SDL/SkOSWindow_SDL.cpp',

        '../src/utils/unix/keysym2ucs.c',
        '../src/utils/unix/SkEGLContext_Unix.cpp',
        '../src/utils/unix/SkOSWindow_Unix.cpp',
        
        '../src/utils/win/skia_win.cpp',
        '../src/utils/win/SkEGLContext_Win.cpp',
        '../src/utils/win/SkOSWindow_Win.cpp',
      ],
      'sources!': [
          '../src/utils/mesa/SkEGLContext_Mesa.cpp',
          '../src/utils/SDL/SkOSWindow_SDL.cpp',
      ],
      'conditions': [
        [ 'OS != "mac"', {
          'sources!': [
            '../include/utils/mac/SkCGUtils.h',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
            '../src/utils/mac/SkEGLContext_mac.cpp',
            '../src/utils/mac/skia_mac.cpp',
            '../src/utils/mac/SkOSWindow_Mac.cpp',
          ],
          'include_dirs!': [
            '../include/utils/mac',
          ],
        }],
        [ 'OS == "mac"', {
          'sources!': [
            '../src/utils/SkEGLContext_none.cpp',
          ],
        }],
        [ 'OS != "linux" and OS != "freebsd" and OS != "openbsd" and OS != "solaris"', {
          'sources!': [
            '../src/utils/unix/keysym2ucs.c',
            '../src/utils/unix/SkEGLContext_Unix.cpp',
            '../src/utils/unix/SkOSWindow_Unix.cpp',
          ],
          'include_dirs!': [
            '../include/utils/unix',
          ],
        }],
        [ 'OS == "linux" or OS == "freebsd" or OS == "openbsd" or OS == "solaris"', {
          'sources!': [
            '../src/utils/SkEGLContext_none.cpp',
          ],
        }],
        [ 'OS != "win"', {
          'sources!': [
            '../src/utils/win/skia_win.cpp',
            '../src/utils/win/SkEGLContext_Win.cpp',
            '../src/utils/win/SkOSWindow_Win.cpp',
          ],
        }],
        [ 'OS == "win"', {
          'sources!': [
            '../src/utils/SkEGLContext_none.cpp',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/utils',
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
