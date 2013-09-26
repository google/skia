# Gyp for utils.
{
  'targets': [
    {
      'target_name': 'utils',
      'product_name': 'skia_utils',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
      ],
      'include_dirs': [
        '../include/effects',
        '../include/images',
        '../include/lazy',
        '../include/pathops',
        '../include/pipe',
        '../include/utils',
        '../include/utils/mac',
        '../include/utils/unix',
        '../include/utils/win',
        '../include/xml',
        '../src/core',
        '../src/utils',
      ],
      'sources': [
        # Classes for a threadpool.
        '../include/utils/SkCondVar.h',
        '../include/utils/SkCountdown.h',
        '../include/utils/SkRunnable.h',
        '../include/utils/SkThreadPool.h',
        '../src/utils/SkCondVar.cpp',
        '../src/utils/SkCountdown.cpp',
        '../src/utils/SkThreadPool.cpp',

        '../include/utils/SkBoundaryPatch.h',
        '../include/utils/SkFrontBufferedStream.h',
        '../include/utils/SkCamera.h',
        '../include/utils/SkCanvasStateUtils.h',
        '../include/utils/SkCubicInterval.h',
        '../include/utils/SkCullPoints.h',
        '../include/utils/SkDebugUtils.h',
        '../include/utils/SkDeferredCanvas.h',
        '../include/utils/SkDumpCanvas.h',
        '../include/utils/SkInterpolator.h',
        '../include/utils/SkLayer.h',
        '../include/utils/SkMatrix44.h',
        '../include/utils/SkMeshUtils.h',
        '../include/utils/SkNinePatch.h',
        '../include/utils/SkNWayCanvas.h',
        '../include/utils/SkNullCanvas.h',
        '../include/utils/SkParse.h',
        '../include/utils/SkParsePaint.h',
        '../include/utils/SkParsePath.h',
        '../include/utils/SkPictureUtils.h',
        '../include/utils/SkRandom.h',
        '../include/utils/SkRTConf.h',
        '../include/utils/SkProxyCanvas.h',
        '../include/utils/SkUnitMappers.h',
        '../include/utils/SkWGL.h',

        '../src/utils/SkBase64.cpp',
        '../src/utils/SkBase64.h',
        '../src/utils/SkBitmapHasher.cpp',
        '../src/utils/SkBitmapHasher.h',
        '../src/utils/SkBitSet.cpp',
        '../src/utils/SkBitSet.h',
        '../src/utils/SkBoundaryPatch.cpp',
        '../src/utils/SkFrontBufferedStream.cpp',
        '../src/utils/SkCamera.cpp',
        '../src/utils/SkCanvasStack.h',
        '../src/utils/SkCanvasStack.cpp',
        '../src/utils/SkCanvasStateUtils.cpp',
        '../src/utils/SkCubicInterval.cpp',
        '../src/utils/SkCullPoints.cpp',
        '../src/utils/SkDeferredCanvas.cpp',
        '../src/utils/SkDumpCanvas.cpp',
        '../src/utils/SkFloatUtils.h',
        '../src/utils/SkInterpolator.cpp',
        '../src/utils/SkLayer.cpp',
        '../src/utils/SkMatrix44.cpp',
        '../src/utils/SkMD5.cpp',
        '../src/utils/SkMD5.h',
        '../src/utils/SkMeshUtils.cpp',
        '../src/utils/SkNinePatch.cpp',
        '../src/utils/SkNWayCanvas.cpp',
        '../src/utils/SkNullCanvas.cpp',
        '../src/utils/SkOSFile.cpp',
        '../src/utils/SkParse.cpp',
        '../src/utils/SkParseColor.cpp',
        '../src/utils/SkParsePath.cpp',
        '../src/utils/SkPictureUtils.cpp',
        '../src/utils/SkPathUtils.cpp',
        '../src/utils/SkProxyCanvas.cpp',
        '../src/utils/SkSHA1.cpp',
        '../src/utils/SkSHA1.h',
        '../src/utils/SkRTConf.cpp',
        '../src/utils/SkThreadUtils.h',
        '../src/utils/SkThreadUtils_pthread.cpp',
        '../src/utils/SkThreadUtils_pthread.h',
        '../src/utils/SkThreadUtils_pthread_linux.cpp',
        '../src/utils/SkThreadUtils_pthread_mach.cpp',
        '../src/utils/SkThreadUtils_pthread_other.cpp',
        '../src/utils/SkThreadUtils_win.cpp',
        '../src/utils/SkThreadUtils_win.h',
        '../src/utils/SkTFitsIn.h',
        '../src/utils/SkTLogic.h',
        '../src/utils/SkUnitMappers.cpp',

        #mac
        '../include/utils/mac/SkCGUtils.h',
        '../src/utils/mac/SkCreateCGImageRef.cpp',

        #windows
        '../include/utils/win/SkAutoCoInitialize.h',
        '../include/utils/win/SkHRESULT.h',
        '../include/utils/win/SkIStream.h',
        '../include/utils/win/SkTScopedComPtr.h',
        '../src/utils/win/SkAutoCoInitialize.cpp',
        '../src/utils/win/SkDWriteFontFileStream.cpp',
        '../src/utils/win/SkDWriteFontFileStream.h',
        '../src/utils/win/SkDWriteGeometrySink.cpp',
        '../src/utils/win/SkDWriteGeometrySink.h',
        '../src/utils/win/SkHRESULT.cpp',
        '../src/utils/win/SkIStream.cpp',
        '../src/utils/win/SkWGL_win.cpp',

        #testing
        '../src/fonts/SkGScalerContext.cpp',
        '../src/fonts/SkGScalerContext.h',
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
        }],
        [ 'skia_os in ["mac", "ios"]', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../include/utils/mac',
            ],
          },
          'sources!': [
            '../src/utils/SkThreadUtils_pthread_other.cpp',
          ],
        },{ #else if 'skia_os != "mac"'
          'include_dirs!': [
            '../include/utils/mac',
          ],
          'sources!': [
            '../include/utils/mac/SkCGUtils.h',
            '../src/utils/mac/SkCreateCGImageRef.cpp',
            '../src/utils/SkThreadUtils_pthread_mach.cpp',
          ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
          'sources!': [
            '../src/utils/SkThreadUtils_pthread_other.cpp',
          ],
        },{ #else if 'skia_os not in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]'
          'include_dirs!': [
            '../include/utils/unix',
          ],
          'sources!': [
            '../src/utils/SkThreadUtils_pthread_linux.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../include/utils/win',
            ],
          },
          'sources!': [
            '../src/utils/SkThreadUtils_pthread.cpp',
            '../src/utils/SkThreadUtils_pthread.h',
            '../src/utils/SkThreadUtils_pthread_other.cpp',
          ],
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
            '../src/utils/win/SkDWriteFontFileStream.cpp',
            '../src/utils/win/SkDWriteFontFileStream.h',
            '../src/utils/win/SkDWriteGeometrySink.cpp',
            '../src/utils/win/SkDWriteGeometrySink.h',
            '../src/utils/win/SkHRESULT.cpp',
            '../src/utils/win/SkIStream.cpp',
          ],
        }],
        [ 'skia_os == "nacl"', {
          'sources': [
            '../src/utils/SkThreadUtils_pthread_other.cpp',
          ],
          'sources!': [
            '../src/utils/SkThreadUtils_pthread_linux.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'sources': [
            '../src/utils/android/ashmem.cpp',
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
