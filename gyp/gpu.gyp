{
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
        ],
      }],
      # nullify the targets in this gyp file if skia_gpu is 0
      [ 'skia_gpu == 0', {
        'sources/': [
          ['exclude', '.*'],
        ],
        'defines/': [
          ['exclude', '.*'],
        ],
        'include_dirs/': [
           ['exclude', '.*'],
        ],
        'link_settings': {
          'libraries/': [
            ['exclude', '.*'],
          ],
        },
        'direct_dependent_settings': {
          'defines/': [
            ['exclude', '.*'],
          ],
          'include_dirs/': [
            ['exclude', '.*'],
          ],
        },
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
        '../src/gpu',
      ],
      'dependencies': [
        'angle.gyp:*',
      ],
      'export_dependent_settings': [
        'angle.gyp:*',
      ],
      'sources': [
        '../include/gpu/SkGpuCanvas.h',
        '../include/gpu/SkGpuDevice.h',
        '../include/gpu/SkGr.h',
        '../include/gpu/SkGrPixelRef.h',
        '../include/gpu/SkGrTexturePixelRef.h',

        '../include/gpu/gl/SkGLContext.h',
        '../include/gpu/gl/SkMesaGLContext.h',
        '../include/gpu/gl/SkANGLEGLContext.h',
        '../include/gpu/gl/SkNativeGLContext.h',
        '../include/gpu/gl/SkNullGLContext.h',
        '../include/gpu/gl/SkDebugGLContext.h',

        '../src/gpu/SkGpuCanvas.cpp',
        '../src/gpu/SkGpuDevice.cpp',
        '../src/gpu/SkGr.cpp',
        '../src/gpu/SkGrFontScaler.cpp',
        '../src/gpu/SkGrPixelRef.cpp',
        '../src/gpu/SkGrTexturePixelRef.cpp',

        '../src/gpu/gl/SkGLContext.cpp',
        '../src/gpu/gl/SkNullGLContext.cpp',

        '../src/gpu/gl/debug/SkDebugGLContext.cpp',

        '../src/gpu/gl/mac/SkNativeGLContext_mac.cpp',

        '../src/gpu/gl/win/SkNativeGLContext_win.cpp',

        '../src/gpu/gl/unix/SkNativeGLContext_unix.cpp',

        '../src/gpu/gl/mesa/SkMesaGLContext.cpp',
        '../src/gpu/gl/angle/SkANGLEGLContext.cpp',
        '../src/gpu/gl/angle/GrGLCreateANGLEInterface.cpp',

        '../src/gpu/android/SkNativeGLContext_android.cpp',
      ],
      'conditions': [
        [ 'not skia_mesa', {
          'sources!': [
            '../src/gpu/gl/mesa/SkMesaGLContext.cpp',
          ],
        }],
        [ 'skia_mesa and skia_os == "mac"', {
          'include_dirs': [
             '$(SDKROOT)/usr/X11/include/',
          ],
        }],
        [ 'not skia_angle', {
          'sources!': [
            '../include/gpu/gl/SkANGLEGLContext.h',
            '../src/gpu/gl/angle/SkANGLEGLContext.cpp',
            '../src/gpu/gl/angle/GrGLCreateANGLEInterface.cpp',
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
        '../src/gpu'
      ],
      'dependencies': [
        'angle.gyp:*',
      ],
      'export_dependent_settings': [
        'angle.gyp:*',
      ],
      'sources': [
        '../include/gpu/GrAARectRenderer.h',
        '../include/gpu/GrCacheID.h',
        '../include/gpu/GrClipData.h',
        '../include/gpu/GrColor.h',
        '../include/gpu/GrConfig.h',
        '../include/gpu/GrContext.h',
        '../include/gpu/GrContextFactory.h',
        '../include/gpu/GrCustomStage.h',
        '../include/gpu/GrCustomStageUnitTest.h',
        '../include/gpu/GrFontScaler.h',
        '../include/gpu/GrGlyph.h',
        '../include/gpu/GrInstanceCounter.h',
        '../include/gpu/GrKey.h',
        '../include/gpu/GrMatrix.h',
        '../include/gpu/GrNoncopyable.h',
        '../include/gpu/GrPaint.h',
        '../include/gpu/GrPoint.h',
        '../include/gpu/GrProgramStageFactory.h',
        '../include/gpu/GrRect.h',
        '../include/gpu/GrRefCnt.h',
        '../include/gpu/GrRenderTarget.h',
        '../include/gpu/GrResource.h',
        '../include/gpu/GrSamplerState.h',
        '../include/gpu/GrScalar.h',
        '../include/gpu/GrSurface.h',
        '../include/gpu/GrTextContext.h',
        '../include/gpu/GrTexture.h',
        '../include/gpu/GrTypes.h',
        '../include/gpu/GrUserConfig.h',

        '../include/gpu/gl/GrGLConfig.h',
        '../include/gpu/gl/GrGLConfig_chrome.h',
        '../include/gpu/gl/GrGLFunctions.h',
        '../include/gpu/gl/GrGLInterface.h',

        '../src/gpu/GrAAHairLinePathRenderer.cpp',
        '../src/gpu/GrAAHairLinePathRenderer.h',
        '../src/gpu/GrAAConvexPathRenderer.cpp',
        '../src/gpu/GrAAConvexPathRenderer.h',
        '../src/gpu/GrAARectRenderer.cpp',
        '../src/gpu/GrAddPathRenderers_default.cpp',
        '../src/gpu/GrAllocator.h',
        '../src/gpu/GrAllocPool.h',
        '../src/gpu/GrAllocPool.cpp',
        '../src/gpu/GrAtlas.cpp',
        '../src/gpu/GrAtlas.h',
        '../src/gpu/GrBinHashKey.h',
        '../src/gpu/GrBufferAllocPool.cpp',
        '../src/gpu/GrBufferAllocPool.h',
        '../src/gpu/GrCacheID.cpp',
        '../src/gpu/GrClipData.cpp',
        '../src/gpu/GrContext.cpp',
        '../src/gpu/GrCustomStage.cpp',
        '../src/gpu/GrDefaultPathRenderer.cpp',
        '../src/gpu/GrDefaultPathRenderer.h',
        '../src/gpu/GrDrawState.h',
        '../src/gpu/GrDrawTarget.cpp',
        '../src/gpu/GrDrawTarget.h',
        '../src/gpu/GrGeometryBuffer.h',
        '../src/gpu/GrClipMaskManager.h',
        '../src/gpu/GrClipMaskManager.cpp',
        '../src/gpu/GrGpu.cpp',
        '../src/gpu/GrGpu.h',
        '../src/gpu/GrGpuFactory.cpp',
        '../src/gpu/GrGpuVertex.h',
        '../src/gpu/GrIndexBuffer.h',
        '../src/gpu/GrInOrderDrawBuffer.cpp',
        '../src/gpu/GrInOrderDrawBuffer.h',
        '../src/gpu/GrMatrix.cpp',
        '../src/gpu/GrMemory.cpp',
        '../src/gpu/GrMemoryPool.cpp',
        '../src/gpu/GrMemoryPool.h',
        '../src/gpu/GrPath.h',
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
        '../src/gpu/GrStencilAndCoverPathRenderer.cpp',
        '../src/gpu/GrStencilAndCoverPathRenderer.h',
        '../src/gpu/GrStencilBuffer.cpp',
        '../src/gpu/GrStencilBuffer.h',
        '../src/gpu/GrTBSearch.h',
        '../src/gpu/GrTDArray.h',
        '../src/gpu/GrSWMaskHelper.cpp',
        '../src/gpu/GrSWMaskHelper.h',
        '../src/gpu/GrSoftwarePathRenderer.cpp',
        '../src/gpu/GrSoftwarePathRenderer.h',
        '../src/gpu/GrSurface.cpp',
        '../src/gpu/GrTemplates.h',
        '../src/gpu/GrTextContext.cpp',
        '../src/gpu/GrTextStrike.cpp',
        '../src/gpu/GrTextStrike.h',
        '../src/gpu/GrTextStrike_impl.h',
        '../src/gpu/GrTexture.cpp',
        '../src/gpu/GrTHashCache.h',
        '../src/gpu/GrTLList.h',
        '../src/gpu/GrVertexBuffer.h',
        '../src/gpu/gr_unittests.cpp',

        '../src/gpu/effects/Gr1DKernelEffect.h',
        '../src/gpu/effects/GrTextureStripAtlas.h',
        '../src/gpu/effects/GrTextureStripAtlas.cpp',
        '../src/gpu/effects/GrColorTableEffect.cpp',
        '../src/gpu/effects/GrColorTableEffect.h',
        '../src/gpu/effects/GrConvolutionEffect.cpp',
        '../src/gpu/effects/GrConvolutionEffect.h',
        '../src/gpu/effects/GrMorphologyEffect.cpp',
        '../src/gpu/effects/GrMorphologyEffect.h',
        '../src/gpu/effects/GrSingleTextureEffect.cpp',
        '../src/gpu/effects/GrSingleTextureEffect.h',
        '../src/gpu/effects/GrTextureDomainEffect.cpp',
        '../src/gpu/effects/GrTextureDomainEffect.h',

        '../src/gpu/gl/GrGLCaps.cpp',
        '../src/gpu/gl/GrGLCaps.h',
        '../src/gpu/gl/GrGLContextInfo.cpp',
        '../src/gpu/gl/GrGLContextInfo.h',
        '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
        '../src/gpu/gl/GrGLCreateNullInterface.cpp',
        '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
        '../src/gpu/gl/GrGLDefaultInterface_native.cpp',
        '../src/gpu/gl/GrGLDefines.h',
        '../src/gpu/gl/GrGLIndexBuffer.cpp',
        '../src/gpu/gl/GrGLIndexBuffer.h',
        '../src/gpu/gl/GrGLInterface.cpp',
        '../src/gpu/gl/GrGLIRect.h',
        '../src/gpu/gl/GrGLPath.cpp',
        '../src/gpu/gl/GrGLPath.h',
        '../src/gpu/gl/GrGLProgram.cpp',
        '../src/gpu/gl/GrGLProgram.h',
        '../src/gpu/gl/GrGLProgramStage.cpp',
        '../src/gpu/gl/GrGLProgramStage.h',
        '../src/gpu/gl/GrGLRenderTarget.cpp',
        '../src/gpu/gl/GrGLRenderTarget.h',
        '../src/gpu/gl/GrGLShaderBuilder.cpp',
        '../src/gpu/gl/GrGLShaderBuilder.h',
        '../src/gpu/gl/GrGLShaderVar.h',
        '../src/gpu/gl/GrGLSL.cpp',
        '../src/gpu/gl/GrGLSL.h',
        '../src/gpu/gl/GrGLStencilBuffer.cpp',
        '../src/gpu/gl/GrGLStencilBuffer.h',
        '../src/gpu/gl/GrGLTexture.cpp',
        '../src/gpu/gl/GrGLTexture.h',
        '../src/gpu/gl/GrGLUtil.cpp',
        '../src/gpu/gl/GrGLUtil.h',
        '../src/gpu/gl/GrGLUniformManager.cpp',
        '../src/gpu/gl/GrGLUniformManager.h',
        '../src/gpu/gl/GrGLUniformHandle.h',
        '../src/gpu/gl/GrGLVertexBuffer.cpp',
        '../src/gpu/gl/GrGLVertexBuffer.h',
        '../src/gpu/gl/GrGpuGL.cpp',
        '../src/gpu/gl/GrGpuGL.h',
        '../src/gpu/gl/GrGpuGL_program.cpp',

        '../src/gpu/gl/debug/GrGLCreateDebugInterface.cpp',
        '../src/gpu/gl/debug/GrFakeRefObj.h',
        '../src/gpu/gl/debug/GrBufferObj.h',
        '../src/gpu/gl/debug/GrBufferObj.cpp',
        '../src/gpu/gl/debug/GrFBBindableObj.h',
        '../src/gpu/gl/debug/GrRenderBufferObj.h',
        '../src/gpu/gl/debug/GrTextureObj.h',
        '../src/gpu/gl/debug/GrTextureObj.cpp',
        '../src/gpu/gl/debug/GrTextureUnitObj.h',
        '../src/gpu/gl/debug/GrTextureUnitObj.cpp',
        '../src/gpu/gl/debug/GrFrameBufferObj.h',
        '../src/gpu/gl/debug/GrFrameBufferObj.cpp',
        '../src/gpu/gl/debug/GrShaderObj.h',
        '../src/gpu/gl/debug/GrShaderObj.cpp',
        '../src/gpu/gl/debug/GrProgramObj.h',
        '../src/gpu/gl/debug/GrProgramObj.cpp',
        '../src/gpu/gl/debug/GrDebugGL.h',
        '../src/gpu/gl/debug/GrDebugGL.cpp',

        '../src/gpu/gl/mac/GrGLCreateNativeInterface_mac.cpp',

        '../src/gpu/gl/win/GrGLCreateNativeInterface_win.cpp',

        '../src/gpu/gl/unix/GrGLCreateNativeInterface_unix.cpp',

        '../src/gpu/gl/mesa/GrGLCreateMesaInterface.cpp',
        '../src/gpu/gl/angle/GrGLCreateANGLEInterface.cpp',

        '../src/gpu/android/GrGLCreateNativeInterface_android.cpp',
      ],
      'defines': [
        'GR_IMPLEMENTATION=1',
      ],
      'conditions': [
        [ 'skia_nv_path_rendering', {
          'defines': [
            'GR_GL_USE_NV_PATH_RENDERING=1',
          ],
        }],
        [ 'skia_os == "linux"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
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
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
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
            '../src/gpu/gl/mesa/GrGLCreateMesaInterface.cpp',
          ],
        }],
        [ 'skia_os == "win"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ 'not skia_angle', {
          'sources!': [
            '../include/gpu/gl/SkANGLEGLContext.h',

            '../src/gpu/gl/angle/GrGLCreateANGLEInterface.cpp',
            '../src/gpu/gl/angle/SkANGLEGLContext.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
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
