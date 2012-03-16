{
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
        '../include/utils/win',
        '../include/views',
        '../include/effects',
        '../include/xml',
      ],
      'sources': [
        '../include/utils/SkBoundaryPatch.h',
        '../include/utils/SkCamera.h',
        '../include/utils/SkCubicInterval.h',
        '../include/utils/SkCullPoints.h',
        '../include/utils/SkDeferredCanvas.h',
        '../include/utils/SkDumpCanvas.h',
        '../include/utils/SkInterpolator.h',
        '../include/utils/SkLayer.h',
        '../include/utils/SkMatrix44.h',
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
        '../include/utils/SkWGL.h',

        '../src/utils/SkBase64.cpp',
        '../src/utils/SkBase64.h',
        '../src/utils/SkBoundaryPatch.cpp',
        '../src/utils/SkCamera.cpp',
        '../src/utils/SkColorMatrix.cpp',
        '../src/utils/SkCubicInterval.cpp',
        '../src/utils/SkCullPoints.cpp',
        '../src/utils/SkDeferredCanvas.cpp',
        '../src/utils/SkDumpCanvas.cpp',
        '../src/utils/SkInterpolator.cpp',
        '../src/utils/SkLayer.cpp',
        '../src/utils/SkMatrix44.cpp',
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

        #mac
        '../include/utils/mac/SkCGUtils.h',
        '../src/utils/mac/SkCreateCGImageRef.cpp',

        #sdl
        '../src/utils/SDL/SkOSWindow_SDL.cpp',

        #*nix
        '../src/utils/unix/keysym2ucs.c',
        '../src/utils/unix/SkOSWindow_Unix.cpp',
        
        #windows
        '../include/utils/win/SkAutoCoInitialize.h',
        '../include/utils/win/SkHRESULT.h',
        '../include/utils/win/SkIStream.h',
        '../include/utils/win/SkTScopedComPtr.h',
        '../src/utils/win/SkAutoCoInitialize.cpp',
        '../src/utils/win/skia_win.cpp',
        '../src/utils/win/SkHRESULT.cpp',
        '../src/utils/win/SkIStream.cpp',
        '../src/utils/win/SkOSWindow_win.cpp',
        '../src/utils/win/SkWGL_win.cpp',
      ],
      'sources!': [
          '../src/utils/SDL/SkOSWindow_SDL.cpp',
      ],
      'conditions': [
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/AGL.framework',
            ],
          },
          'direct_dependent_settings': {
            'include_dirs': [
              '../include/utils/mac',
            ],
          },
        },{ #else if 'skia_os != "mac"'
          'include_dirs!': [
            '../include/utils/mac',
          ],
          'sources!': [
            '../include/utils/mac/SkCGUtils.h',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
            '../src/utils/mac/skia_mac.mm',
            '../src/utils/mac/SkOSWindow_Mac.mm',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
          'link_settings': {
            'libraries': [
              '-lGL',
              '-lGLU',
            ],
          },
        },{ #else if 'skia_os not in ["linux", "freebsd", "openbsd", "solaris"]'
          'include_dirs!': [
            '../include/utils/unix',
          ],
          'sources!': [
            '../src/utils/unix/keysym2ucs.c',
            '../src/utils/unix/SkOSWindow_Unix.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../include/utils/win',
            ],
          },
        },{ #else if 'skia_os != "win"'
          'include_dirs!': [
            '../include/utils/win',
          ],
          'sources/': [ ['exclude', '_win.(h|cpp)$'],],
          'sources!': [
            '../include/utils/win/SkAutoCoInitialize.h',
            '../include/utils/win/SkHRESULT.h',
            '../include/utils/win/SkIStream.h',
            '../include/utils/win/SkTScopedComPtr.h',
            '../src/utils/win/SkAutoCoInitialize.cpp',
            '../src/utils/win/SkHRESULT.cpp',
            '../src/utils/win/SkIStream.cpp',
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
