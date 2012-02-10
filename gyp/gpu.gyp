{
  'variables': {
    'skia_gpu_disable_osaa%': 0,
  },
  'skia_gpu_disable_osaa': '<(skia_gpu_disable_osaa)',

  'includes': [
    'common.gypi',
  ],
  'target_defaults': {
    'conditions': [
      ['skia_os != "win"', {
        'sources/': [ ['exclude', '_win.(h|cpp)$'],
        ],
      }],
      ['skia_os != "mac"', {
        'sources/': [ ['exclude', '_mac.(h|cpp)$'],
        ],
      }],
      ['skia_os != "linux"', {
        'sources/': [ ['exclude', '_unix.(h|cpp)$'],
        ],
      }],
      ['skia_os != "ios"', {
        'sources/': [ ['exclude', '_iOS.(h|cpp)$'],
        ],
      }],
      ['skia_os != "android"', {
        'sources/': [ ['exclude', '_android.(h|cpp)$'],
        ],
      }],
      [ 'skia_os == "android"', {
        'defines': [
          'GR_ANDROID_BUILD=1',
        ],
      }],
      [ 'skia_os == "mac"', {
        'defines': [
          'GR_MAC_BUILD=1',
        ],
      }],
      [ 'skia_os == "linux"', {
        'defines': [
          'GR_LINUX_BUILD=1',
        ],
      }],
      [ 'skia_os == "ios"', {
        'defines': [
          'GR_IOS_BUILD=1',
        ],
      }],
      [ 'skia_os == "win"', {
        'defines': [
          'GR_WIN32_BUILD=1',
          'GR_GL_FUNCTION_TYPE=__stdcall',
        ],
      }],
    ],
    'direct_dependent_settings': {
      'conditions': [
        [ 'skia_os == "android"', {
          'defines': [
            'GR_ANDROID_BUILD=1',
          ],
        }],
        [ 'skia_os == "mac"', {
          'defines': [
            'GR_MAC_BUILD=1',
          ],
        }],
        [ 'skia_os == "linux"', {
          'defines': [
            'GR_LINUX_BUILD=1',
          ],
        }],
        [ 'skia_os == "ios"', {
          'defines': [
            'GR_IOS_BUILD=1',
          ],
        }],
        [ 'skia_os == "win"', {
          'defines': [
            'GR_WIN32_BUILD=1',
            'GR_GL_FUNCTION_TYPE=__stdcall',
          ],
        }],
      ],
      'include_dirs': [
        '../include/gpu',
      ],
    },
  },
  'targets': [
    {
      'target_name': 'skgr',
      'type': 'static_library',
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../src/core',
        '../include/gpu',
      ],
      'sources': [
        '../include/gpu/SkGLContext.h',
        '../include/gpu/SkMesaGLContext.h',
        '../include/gpu/SkNativeGLContext.h',
        '../include/gpu/SkNullGLContext.h',
        '../include/gpu/SkGpuCanvas.h',
        '../include/gpu/SkGpuDevice.h',
        '../include/gpu/SkGr.h',
        '../include/gpu/SkGrTexturePixelRef.h',

        '../src/gpu/GrPrintf_skia.cpp',
        '../src/gpu/SkGLContext.cpp',
        '../src/gpu/SkGpuCanvas.cpp',
        '../src/gpu/SkGpuDevice.cpp',
        '../src/gpu/SkGr.cpp',
        '../src/gpu/SkGrFontScaler.cpp',
        '../src/gpu/SkGrTexturePixelRef.cpp',
        '../src/gpu/SkNullGLContext.cpp',

        '../src/gpu/android/SkNativeGLContext_android.cpp',

        '../src/gpu/mac/SkNativeGLContext_mac.cpp',

        '../src/gpu/win/SkNativeGLContext_win.cpp',

        '../src/gpu/unix/SkNativeGLContext_unix.cpp',

        '../src/gpu/mesa/SkMesaGLContext.cpp',
      ],
      'conditions': [
        [ 'not skia_mesa', {
          'sources!': [
            '../src/gpu/mesa/SkMesaGLContext.cpp',
          ],
        }],
        [ 'skia_mesa and skia_os == "mac"', {
          'include_dirs': [
             '$(SDKROOT)/usr/X11/include/',
          ],
        }],
      ],
    },
    {
      'target_name': 'gr',
      'type': 'static_library',
      'include_dirs': [
        '../include/core',
        '../include/config',
        '../include/gpu',
        '../src/core', # SkRasterClip.h
      ],
      'dependencies': [
        'libtess.gyp:libtess',
      ],
      'sources': [
        '../include/gpu/GrClip.h',
        '../include/gpu/GrClipIterator.h',
        '../include/gpu/GrColor.h',
        '../include/gpu/GrConfig.h',
        '../include/gpu/GrContext.h',
        '../include/gpu/GrFontScaler.h',
        '../include/gpu/GrGLConfig.h',
        '../include/gpu/GrGLConfig_chrome.h',
        '../include/gpu/GrGLDefines.h',
        '../include/gpu/GrGLInterface.h',
        '../include/gpu/GrGlyph.h',
        '../include/gpu/GrInstanceCounter.h',
        '../include/gpu/GrKey.h',
        '../include/gpu/GrMatrix.h',
        '../include/gpu/GrNoncopyable.h',
        '../include/gpu/GrPaint.h',
        '../include/gpu/GrPath.h',
        '../include/gpu/GrPoint.h',
        '../include/gpu/GrRect.h',
        '../include/gpu/GrRefCnt.h',
        '../include/gpu/GrRenderTarget.h',
        '../include/gpu/GrResource.h',
        '../include/gpu/GrSamplerState.h',
        '../include/gpu/GrScalar.h',
        '../include/gpu/GrTextContext.h',
        '../include/gpu/GrTexture.h',
        '../include/gpu/GrTypes.h',
        '../include/gpu/GrUserConfig.h',

        '../src/gpu/GrAAHairLinePathRenderer.cpp',
        '../src/gpu/GrAAHairLinePathRenderer.h',
        '../src/gpu/GrAAConvexPathRenderer.cpp',
        '../src/gpu/GrAAConvexPathRenderer.h',
        '../src/gpu/GrAddPathRenderers_default.cpp',
        '../src/gpu/GrAllocator.h',
        '../src/gpu/GrAllocPool.h',
        '../src/gpu/GrAllocPool.cpp',
        '../src/gpu/GrAtlas.cpp',
        '../src/gpu/GrAtlas.h',
        '../src/gpu/GrBinHashKey.h',
        '../src/gpu/GrBufferAllocPool.cpp',
        '../src/gpu/GrBufferAllocPool.h',
        '../src/gpu/GrClip.cpp',
        '../src/gpu/GrContext.cpp',
        '../src/gpu/GrDefaultPathRenderer.cpp',
        '../src/gpu/GrDefaultPathRenderer.h',
        '../src/gpu/GrDrawState.h',
        '../src/gpu/GrDrawTarget.cpp',
        '../src/gpu/GrDrawTarget.h',
        '../src/gpu/GrGeometryBuffer.h',
        '../src/gpu/GrGLContextInfo.cpp',
        '../src/gpu/GrGLContextInfo.h',
        '../src/gpu/GrGLCreateNativeInterface_none.cpp',
        '../src/gpu/GrGLCreateNullInterface.cpp',
        '../src/gpu/GrGLDefaultInterface_none.cpp',
        '../src/gpu/GrGLDefaultInterface_native.cpp',
        '../src/gpu/GrGLIndexBuffer.cpp',
        '../src/gpu/GrGLIndexBuffer.h',
        '../src/gpu/GrGLInterface.cpp',
        '../src/gpu/GrGLIRect.h',
        '../src/gpu/GrGLProgram.cpp',
        '../src/gpu/GrGLProgram.h',
        '../src/gpu/GrGLRenderTarget.cpp',
        '../src/gpu/GrGLRenderTarget.h',
        '../src/gpu/GrGLShaderVar.h',
        '../src/gpu/GrGLSL.cpp',
        '../src/gpu/GrGLSL.h',
        '../src/gpu/GrGLStencilBuffer.cpp',
        '../src/gpu/GrGLStencilBuffer.h',
        '../src/gpu/GrGLTexture.cpp',
        '../src/gpu/GrGLTexture.h',
        '../src/gpu/GrGLUtil.cpp',
        '../src/gpu/GrGLVertexBuffer.cpp',
        '../src/gpu/GrGLVertexBuffer.h',
        '../src/gpu/GrGpu.cpp',
        '../src/gpu/GrGpu.h',
        '../src/gpu/GrGpuFactory.cpp',
        '../src/gpu/GrGpuGL.cpp',
        '../src/gpu/GrGpuGL.h',
        '../src/gpu/GrGpuGLShaders.cpp',
        '../src/gpu/GrGpuGLShaders.h',
        '../src/gpu/GrGpuVertex.h',
        '../src/gpu/GrIndexBuffer.h',
        '../src/gpu/GrInOrderDrawBuffer.cpp',
        '../src/gpu/GrInOrderDrawBuffer.h',
        '../src/gpu/GrMatrix.cpp',
        '../src/gpu/GrMemory.cpp',
        '../src/gpu/GrPathRendererChain.cpp',
        '../src/gpu/GrPathRendererChain.h',
        '../src/gpu/GrPathRenderer.cpp',
        '../src/gpu/GrPathRenderer.h',
        '../src/gpu/GrPathUtils.cpp',
        '../src/gpu/GrPathUtils.h',
        '../src/gpu/GrPlotMgr.h',
        '../src/gpu/GrRandom.h',
        '../src/gpu/GrRectanizer.cpp',
        '../src/gpu/GrRectanizer.h',
        '../src/gpu/GrRedBlackTree.h',
        '../src/gpu/GrRenderTarget.cpp',
        '../src/gpu/GrResource.cpp',
        '../src/gpu/GrResourceCache.cpp',
        '../src/gpu/GrResourceCache.h',
        '../src/gpu/GrStencil.cpp',
        '../src/gpu/GrStencil.h',
        '../src/gpu/GrStencilBuffer.cpp',
        '../src/gpu/GrStencilBuffer.h',
        '../src/gpu/GrStringBuilder.h',
        '../src/gpu/GrTBSearch.h',
        '../src/gpu/GrTDArray.h',
        '../src/gpu/GrTesselatedPathRenderer.cpp',
        '../src/gpu/GrTesselatedPathRenderer.h',
        '../src/gpu/GrTextContext.cpp',
        '../src/gpu/GrTextStrike.cpp',
        '../src/gpu/GrTextStrike.h',
        '../src/gpu/GrTextStrike_impl.h',
        '../src/gpu/GrTexture.cpp',
        '../src/gpu/GrTHashCache.h',
        '../src/gpu/GrTLList.h',
        '../src/gpu/GrVertexBuffer.h',
        '../src/gpu/gr_unittests.cpp',


        '../src/gpu/mac/GrGLCreateNativeInterface_mac.cpp',

        '../src/gpu/win/GrGLCreateNativeInterface_win.cpp',

        '../src/gpu/unix/GrGLCreateNativeInterface_unix.cpp',

        '../src/gpu/android/GrGLCreateNativeInterface_android.cpp',

        '../src/gpu/mesa/GrGLCreateMesaInterface.cpp',
      ],
      'defines': [
        'GR_IMPLEMENTATION=1',
      ],
      'conditions': [
        [ 'skia_gpu_disable_osaa', {
          'defines': [
            'GR_USE_OFFSCREEN_AA=0',
          ],
        }],
        [ 'skia_os == "linux"', {
          'sources!': [
            '../src/gpu/GrGLDefaultInterface_none.cpp',
            '../src/gpu/GrGLCreateNativeInterface_none.cpp',
          ],
          'link_settings': {
            'libraries': [
              '-lGL',
              '-lX11',
            ],
          },
        }],
        [ 'skia_mesa and skia_os == "linux"', {
          'link_settings': {
            'libraries': [
              '-lOSMesa',
            ],
          },
        }],
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
          },
          'sources!': [
            '../src/gpu/GrGLDefaultInterface_none.cpp',
            '../src/gpu/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ 'skia_mesa and skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/usr/X11/lib/libOSMesa.dylib',
            ],
          },
          'include_dirs': [
             '$(SDKROOT)/usr/X11/include/',
          ],
        }],
        [ 'not skia_mesa', {
          'sources!': [
            '../src/gpu/mesa/GrGLCreateMesaInterface.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
          'sources!': [
            '../src/gpu/GrGLDefaultInterface_none.cpp',
            '../src/gpu/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'sources!': [
            '../src/gpu/GrGLDefaultInterface_none.cpp',
            '../src/gpu/GrGLCreateNativeInterface_none.cpp',
          ],
          'link_settings': {
            'libraries': [
              '-lGLESv2',
              '-lEGL',
            ],
          },
        }],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
