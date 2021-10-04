/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLCaps.h"

#include <memory>

#include "include/gpu/GrContextOptions.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkTSearch.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/gl/GrGLContext.h"
#include "src/gpu/gl/GrGLRenderTarget.h"
#include "src/gpu/gl/GrGLTexture.h"
#include "src/utils/SkJSONWriter.h"

#if defined(SK_BUILD_FOR_IOS)
#include <TargetConditionals.h>
#endif

GrGLCaps::GrGLCaps(const GrContextOptions& contextOptions,
                   const GrGLContextInfo& ctxInfo,
                   const GrGLInterface* glInterface) : INHERITED(contextOptions) {
    fStandard = ctxInfo.standard();

    fPackFlipYSupport = false;
    fTextureUsageSupport = false;
    fImagingSupport = false;
    fVertexArrayObjectSupport = false;
    fDebugSupport = false;
    fES2CompatibilitySupport = false;
    fDrawRangeElementsSupport = false;
    fBaseVertexBaseInstanceSupport = false;
    fIsCoreProfile = false;
    fBindFragDataLocationSupport = false;
    fRectangleTextureSupport = false;
    fBindUniformLocationSupport = false;
    fMipmapLevelControlSupport = false;
    fMipmapLodControlSupport = false;
    fUseBufferDataNullHint = false;
    fDoManualMipmapping = false;
    fClearToBoundaryValuesIsBroken = false;
    fClearTextureSupport = false;
    fDrawArraysBaseVertexIsBroken = false;
    fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO = false;
    fUseDrawInsteadOfAllRenderTargetWrites = false;
    fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines = false;
    fDontSetBaseOrMaxLevelForExternalTextures = false;
    fNeverDisableColorWrites = false;
    fMustSetAnyTexParameterToEnableMipmapping = false;
    fAllowBGRA8CopyTexSubImage = false;
    fDisallowDynamicMSAA = false;
    fMustResetBlendFuncBetweenDualSourceAndDisable = false;
    fBindTexture0WhenChangingTextureFBOMultisampleCount = false;
    fProgramBinarySupport = false;
    fProgramParameterSupport = false;
    fSamplerObjectSupport = false;
    fUseSamplerObjects = false;
    fTextureSwizzleSupport = false;
    fTiledRenderingSupport = false;
    fFBFetchRequiresEnablePerSample = false;
    fSRGBWriteControl = false;
    fSkipErrorChecks = false;

    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    // All of Skia's automated testing of ANGLE and all related tuning of performance and driver
    // workarounds is oriented around the D3D backends of ANGLE. Chrome has started using Skia
    // on top of ANGLE's GL backend. In this case ANGLE is still interfacing the same underlying
    // GL driver that our performance and correctness tuning was performed on. To avoid losing
    // that we strip the ANGLE info and for the rest of caps setup pretend we're directly on top of
    // the GL driver. Note that this means that some driver workarounds are likely implemented at
    // two levels of the stack (Skia and ANGLE) but we haven't determined which.
    if (ctxInfo.angleBackend() == GrGLANGLEBackend::kOpenGL) {
        this->init(contextOptions, ctxInfo.makeNonAngle(), glInterface);
        // A major caveat is that ANGLE does not allow client side arrays.
        fPreferClientSideDynamicBuffers = false;
    } else {
        this->init(contextOptions, ctxInfo, glInterface);
    }
}

void GrGLCaps::init(const GrContextOptions& contextOptions,
                    const GrGLContextInfo& ctxInfo,
                    const GrGLInterface* gli) {
    GrGLStandard standard = ctxInfo.standard();
    // standard can be unused (optimized away) if SK_ASSUME_GL_ES is set
    sk_ignore_unused_variable(standard);
    GrGLVersion version = ctxInfo.version();

    if (GR_IS_GR_GL(standard)) {
        GrGLint max;
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max);
        fMaxFragmentUniformVectors = max / 4;
        if (version >= GR_GL_VER(3, 2)) {
            GrGLint profileMask;
            GR_GL_GetIntegerv(gli, GR_GL_CONTEXT_PROFILE_MASK, &profileMask);
            fIsCoreProfile = SkToBool(profileMask & GR_GL_CONTEXT_CORE_PROFILE_BIT);
        }
    } else if (GR_IS_GR_GL_ES(standard) || GR_IS_GR_WEBGL(standard)) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS,
                          &fMaxFragmentUniformVectors);
    }

    if (fDriverBugWorkarounds.max_fragment_uniform_vectors_32) {
        fMaxFragmentUniformVectors = std::min(fMaxFragmentUniformVectors, 32);
    }
    GR_GL_GetIntegerv(gli, GR_GL_MAX_VERTEX_ATTRIBS, &fMaxVertexAttributes);

    if (GR_IS_GR_GL(standard)) {
        fWritePixelsRowBytesSupport = true;
        fReadPixelsRowBytesSupport = true;
        fPackFlipYSupport = false;
    } else if (GR_IS_GR_GL_ES(standard)) {
        fWritePixelsRowBytesSupport =
                version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_EXT_unpack_subimage");
        fReadPixelsRowBytesSupport =
                version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_NV_pack_subimage");
        fPackFlipYSupport =
            ctxInfo.hasExtension("GL_ANGLE_pack_reverse_row_order");
    } else if (GR_IS_GR_WEBGL(standard)) {
        // WebGL 2.0 has these
        fWritePixelsRowBytesSupport = version >= GR_GL_VER(2, 0);
        fReadPixelsRowBytesSupport = version >= GR_GL_VER(2, 0);
    }
    fTransferPixelsToRowBytesSupport = fWritePixelsRowBytesSupport;

    if (fDriverBugWorkarounds.pack_parameters_workaround_with_pack_buffer) {
        // In some cases drivers handle copying the last row incorrectly
        // when using GL_PACK_ROW_LENGTH.  Chromium handles this by iterating
        // through every row and conditionally clobbering that value, but
        // Skia already has a scratch buffer workaround when pack row length
        // is not supported, so just use that.
        fReadPixelsRowBytesSupport = false;
    }

    fTextureUsageSupport = GR_IS_GR_GL_ES(standard) &&
                           ctxInfo.hasExtension("GL_ANGLE_texture_usage");

    if (GR_IS_GR_GL(standard)) {
        fTextureBarrierSupport = version >= GR_GL_VER(4,5) ||
                                 ctxInfo.hasExtension("GL_ARB_texture_barrier") ||
                                 ctxInfo.hasExtension("GL_NV_texture_barrier");
    } else if (GR_IS_GR_GL_ES(standard)) {
        fTextureBarrierSupport = ctxInfo.hasExtension("GL_NV_texture_barrier");
    } // no WebGL support

    if (GR_IS_GR_GL(standard)) {
        fSampleLocationsSupport = version >= GR_GL_VER(3,2) ||
                                  ctxInfo.hasExtension("GL_ARB_texture_multisample");
    } else if (GR_IS_GR_GL_ES(standard)) {
        fSampleLocationsSupport = version >= GR_GL_VER(3,1);
    }  // no WebGL support

    fImagingSupport = GR_IS_GR_GL(standard) &&
                      ctxInfo.hasExtension("GL_ARB_imaging");

    if (((GR_IS_GR_GL(standard) && version >= GR_GL_VER(4,3)) ||
         (GR_IS_GR_GL_ES(standard) && version >= GR_GL_VER(3,0)) ||
         ctxInfo.hasExtension("GL_ARB_invalidate_subdata"))) {
        fInvalidateFBType = kInvalidate_InvalidateFBType;
    } else if (ctxInfo.hasExtension("GL_EXT_discard_framebuffer")) {
        fInvalidateFBType = kDiscard_InvalidateFBType;
    }

    // For future reference on Desktop GL, GL_PRIMITIVE_RESTART_FIXED_INDEX appears in 4.3, and
    // GL_PRIMITIVE_RESTART (where the client must call glPrimitiveRestartIndex) appears in 3.1.
    if (GR_IS_GR_GL_ES(standard)) {
        // Primitive restart can cause a 3x slowdown on Adreno. Enable conservatively.
        // FIXME: Primitive restart would likely be a win on iOS if we had an enum value for it.
        if (ctxInfo.vendor() == GrGLVendor::kARM) {
            fUsePrimitiveRestart = version >= GR_GL_VER(3,0);
        }
    }

    if (ctxInfo.vendor() == GrGLVendor::kARM         ||
        ctxInfo.vendor() == GrGLVendor::kImagination ||
        ctxInfo.vendor() == GrGLVendor::kQualcomm ) {
        fPreferFullscreenClears = true;
    }

    if (GR_IS_GR_GL(standard)) {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_ARB_vertex_array_object") ||
                                    ctxInfo.hasExtension("GL_APPLE_vertex_array_object");
    } else if (GR_IS_GR_GL_ES(standard)) {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_OES_vertex_array_object");
    } else if (GR_IS_GR_WEBGL(standard)) {
        fVertexArrayObjectSupport = version >= GR_GL_VER(2, 0) ||
                                    ctxInfo.hasExtension("GL_OES_vertex_array_object") ||
                                    ctxInfo.hasExtension("OES_vertex_array_object");
    }

    if (GR_IS_GR_GL(standard) && version >= GR_GL_VER(4,3)) {
        fDebugSupport = true;
    } else if (GR_IS_GR_GL_ES(standard)) {
        fDebugSupport = ctxInfo.hasExtension("GL_KHR_debug");
    } // no WebGL support

    if (GR_IS_GR_GL(standard)) {
        fES2CompatibilitySupport = ctxInfo.hasExtension("GL_ARB_ES2_compatibility");
    }
    else if (GR_IS_GR_GL_ES(standard)) {
        fES2CompatibilitySupport = true;
    } else if (GR_IS_GR_WEBGL(standard)) {
        fES2CompatibilitySupport = true;
    }

    if (GR_IS_GR_GL(standard)) {
        fClientCanDisableMultisample = true;
    } else if (GR_IS_GR_GL_ES(standard)) {
        fClientCanDisableMultisample = ctxInfo.hasExtension("GL_EXT_multisample_compatibility");
    } // no WebGL support

    if (GR_IS_GR_GL(standard)) {
        // 3.1 has draw_instanced but not instanced_arrays, for the time being we only care about
        // instanced arrays, but we could make this more granular if we wanted
        fDrawInstancedSupport =
                version >= GR_GL_VER(3, 2) ||
                (ctxInfo.hasExtension("GL_ARB_draw_instanced") &&
                 ctxInfo.hasExtension("GL_ARB_instanced_arrays"));
    } else if (GR_IS_GR_GL_ES(standard)) {
        fDrawInstancedSupport =
                version >= GR_GL_VER(3, 0) ||
                (ctxInfo.hasExtension("GL_EXT_draw_instanced") &&
                 ctxInfo.hasExtension("GL_EXT_instanced_arrays")) ||
                ctxInfo.hasExtension("GL_ANGLE_instanced_arrays");
    }  else if (GR_IS_GR_WEBGL(standard)) {
        // WebGL 2.0 has DrawArraysInstanced and drawElementsInstanced
        fDrawInstancedSupport = version >= GR_GL_VER(2, 0);
    }

#ifdef GR_DISABLE_TESSELLATION_ON_ES2
    if (GR_IS_GR_GL_ES(standard) && version < GR_GL_VER(3, 0)) {
        // Temporarily disable the tessellation path renderer on Chrome ES2 while we roll the
        // necessary Skia changes.
        fDisableTessellationPathRenderer = true;
    }
#else
    if (GR_IS_GR_GL_ES(standard) && ctxInfo.isOverCommandBuffer() && version < GR_GL_VER(3, 0)) {
        // Temporarily disable the tessellation path renderer over the ES2 command buffer. This is
        // an attempt to lower impact while we roll out tessellation in Chrome.
        fDisableTessellationPathRenderer = true;
    }
#endif

    if (GR_IS_GR_GL(standard)) {
        if (version >= GR_GL_VER(3, 0)) {
            fBindFragDataLocationSupport = true;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (version >= GR_GL_VER(3, 0) && ctxInfo.hasExtension("GL_EXT_blend_func_extended")) {
            fBindFragDataLocationSupport = true;
        }
    } // no WebGL support

    fBindUniformLocationSupport = ctxInfo.hasExtension("GL_CHROMIUM_bind_uniform_location");

    if (GR_IS_GR_GL(standard)) {
        if (version >= GR_GL_VER(3, 1) || ctxInfo.hasExtension("GL_ARB_texture_rectangle") ||
            ctxInfo.hasExtension("GL_ANGLE_texture_rectangle")) {
            fRectangleTextureSupport = true;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        fRectangleTextureSupport = ctxInfo.hasExtension("GL_ARB_texture_rectangle") ||
                                   ctxInfo.hasExtension("GL_ANGLE_texture_rectangle");
    } // no WebGL support

    // GrCaps defaults fClampToBorderSupport to true, so disable when unsupported
    if (GR_IS_GR_GL(standard)) {
        // Clamp to border added in 1.3
        if (version < GR_GL_VER(1, 3) && !ctxInfo.hasExtension("GL_ARB_texture_border_clamp")) {
            fClampToBorderSupport = false;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        // GLES didn't have clamp to border until 3.2, but provides several alternative extensions
        if (version < GR_GL_VER(3, 2) && !ctxInfo.hasExtension("GL_EXT_texture_border_clamp") &&
            !ctxInfo.hasExtension("GL_NV_texture_border_clamp") &&
            !ctxInfo.hasExtension("GL_OES_texture_border_clamp")) {
            fClampToBorderSupport = false;
        }
    } else if (GR_IS_GR_WEBGL(standard)) {
        // WebGL appears to only have REPEAT, CLAMP_TO_EDGE and MIRRORED_REPEAT
        fClampToBorderSupport = false;
    }

    if (GR_IS_GR_GL(standard)) {
        if (version >= GR_GL_VER(3,3) || ctxInfo.hasExtension("GL_ARB_texture_swizzle")) {
            fTextureSwizzleSupport = true;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (version >= GR_GL_VER(3,0)) {
            fTextureSwizzleSupport = true;
        }
    } // no WebGL support

    if (GR_IS_GR_GL(standard)) {
        fMipmapLevelControlSupport = true;
        fMipmapLodControlSupport = true;
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (version >= GR_GL_VER(3,0)) {
            fMipmapLevelControlSupport = true;
            fMipmapLodControlSupport = true;
        }
    } // no WebGL support

    // Chrome's command buffer will zero out a buffer if null is passed to glBufferData to avoid
    // letting an application see uninitialized memory. WebGL spec explicitly disallows null values.
    fUseBufferDataNullHint = !GR_IS_GR_WEBGL(standard) && !ctxInfo.isOverCommandBuffer();

    if (GR_IS_GR_GL(standard)) {
        fClearTextureSupport = (version >= GR_GL_VER(4,4) ||
                                ctxInfo.hasExtension("GL_ARB_clear_texture"));
    } else if (GR_IS_GR_GL_ES(standard)) {
        fClearTextureSupport = ctxInfo.hasExtension("GL_EXT_clear_texture");
    }  // no WebGL support

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
    fSupportsAHardwareBufferImages = true;
#endif

    if (GR_IS_GR_GL(standard)) {
        fSRGBWriteControl = version >= GR_GL_VER(3, 0) ||
                            ctxInfo.hasExtension("GL_ARB_framebuffer_sRGB") ||
                            ctxInfo.hasExtension("GL_EXT_framebuffer_sRGB");
    } else if (GR_IS_GR_GL_ES(standard)) {
        // ES through 3.2 requires EXT_srgb_write_control to support toggling
        // sRGB writing for destinations.
        fSRGBWriteControl = ctxInfo.hasExtension("GL_EXT_sRGB_write_control");
    }  // No WebGL support

    fSkipErrorChecks = ctxInfo.isOverCommandBuffer();
    if (GR_IS_GR_WEBGL(standard)) {
        // Error checks are quite costly in webgl, especially in Chrome.
        fSkipErrorChecks = true;
    }

    // When we are abandoning the context we cannot call into GL thus we should skip any sync work.
    fMustSyncGpuDuringAbandon = false;

    /**************************************************************************
    * GrShaderCaps fields
    **************************************************************************/

    // This must be called after fCoreProfile is set on the GrGLCaps
    this->initGLSL(ctxInfo, gli);
    GrShaderCaps* shaderCaps = fShaderCaps.get();

    // Enable supported shader-related caps
    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fDualSourceBlendingSupport =
                (version >= GR_GL_VER(3, 3) ||
                 ctxInfo.hasExtension("GL_ARB_blend_func_extended")) &&
                ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;

        shaderCaps->fShaderDerivativeSupport = true;

        shaderCaps->fIntegerSupport = version >= GR_GL_VER(3, 0) &&
            ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;

        shaderCaps->fNonsquareMatrixSupport = ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
        shaderCaps->fInverseHyperbolicSupport = ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
    } else if (GR_IS_GR_GL_ES(standard)) {
        shaderCaps->fDualSourceBlendingSupport = ctxInfo.hasExtension("GL_EXT_blend_func_extended");

        shaderCaps->fShaderDerivativeSupport = version >= GR_GL_VER(3, 0) ||
            ctxInfo.hasExtension("GL_OES_standard_derivatives");

        shaderCaps->fIntegerSupport = version >= GR_GL_VER(3, 0) &&
            ctxInfo.glslGeneration() >= k330_GrGLSLGeneration; // We use this value for GLSL ES 3.0.
        shaderCaps->fNonsquareMatrixSupport = ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
        shaderCaps->fInverseHyperbolicSupport = ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
    } else if (GR_IS_GR_WEBGL(standard)) {
        shaderCaps->fShaderDerivativeSupport = version >= GR_GL_VER(2, 0) ||
                                               ctxInfo.hasExtension("GL_OES_standard_derivatives") ||
                                               ctxInfo.hasExtension("OES_standard_derivatives");
        shaderCaps->fIntegerSupport = (version >= GR_GL_VER(2, 0));
        shaderCaps->fNonsquareMatrixSupport = ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
        shaderCaps->fInverseHyperbolicSupport = ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
    }

    if (ctxInfo.hasExtension("GL_NV_conservative_raster")) {
        fConservativeRasterSupport = true;
    }

    if (GR_IS_GR_GL(standard)) {
        fWireframeSupport = true;
    }

    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fRewriteSwitchStatements =
                ctxInfo.glslGeneration() < k130_GrGLSLGeneration;  // introduced in GLSL 1.3
    } else if (GR_IS_GR_GL_ES(standard)) {
        shaderCaps->fRewriteSwitchStatements =
                ctxInfo.glslGeneration() < k330_GrGLSLGeneration;  // introduced in GLSL ES3
    } else if (GR_IS_GR_WEBGL(standard)) {
        shaderCaps->fRewriteSwitchStatements = version < GR_GL_VER(2, 0);  // introduced in WebGL 2
    }

    // Protect ourselves against tracking huge amounts of texture state.
    static const uint8_t kMaxSaneSamplers = 32;
    GrGLint maxSamplers;
    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_IMAGE_UNITS, &maxSamplers);
    shaderCaps->fMaxFragmentSamplers = std::min<GrGLint>(kMaxSaneSamplers, maxSamplers);

    // SGX and Mali GPUs have tiled architectures that have trouble with frequently changing VBOs.
    // We've measured a performance increase using non-VBO vertex data for dynamic content on these
    // GPUs. Perhaps we should read the renderer string and limit this decision to specific GPU
    // families rather than basing it on the vendor alone.
    // The Chrome command buffer blocks the use of client side buffers (but may emulate VBOs with
    // them). Client side buffers are not allowed in core profiles.
    if (GR_IS_GR_GL(standard) || GR_IS_GR_GL_ES(standard)) {
        if (!ctxInfo.isOverCommandBuffer() && !fIsCoreProfile &&
            (ctxInfo.vendor() == GrGLVendor::kARM         ||
             ctxInfo.vendor() == GrGLVendor::kImagination ||
             ctxInfo.vendor() == GrGLVendor::kQualcomm)) {
            fPreferClientSideDynamicBuffers = true;
        }
    } // No client side arrays in WebGL https://www.khronos.org/registry/webgl/specs/1.0/#6.2

    if (!contextOptions.fAvoidStencilBuffers) {
        // To reduce surface area, if we avoid stencil buffers, we also disable MSAA.
        this->initFSAASupport(contextOptions, ctxInfo, gli);
        this->initStencilSupport(ctxInfo);
    }

    // Setup blit framebuffer
    if (GR_IS_GR_GL(standard)) {
        if (version >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object") ||
            ctxInfo.hasExtension("GL_EXT_framebuffer_blit")) {
            fBlitFramebufferFlags = 0;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (version >= GR_GL_VER(3, 0) ||
            ctxInfo.hasExtension("GL_NV_framebuffer_blit")) {
            fBlitFramebufferFlags = kNoFormatConversionForMSAASrc_BlitFramebufferFlag |
                                    kNoMSAADst_BlitFramebufferFlag |
                                    kRectsMustMatchForMSAASrc_BlitFramebufferFlag;
        } else if (ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_multisample") ||
                   ctxInfo.hasExtension("GL_ANGLE_framebuffer_blit")) {
            // The CHROMIUM extension uses the ANGLE version of glBlitFramebuffer and includes its
            // limitations.
            fBlitFramebufferFlags = kNoScalingOrMirroring_BlitFramebufferFlag |
                                    kResolveMustBeFull_BlitFrambufferFlag |
                                    kNoMSAADst_BlitFramebufferFlag |
                                    kNoFormatConversion_BlitFramebufferFlag |
                                    kRectsMustMatchForMSAASrc_BlitFramebufferFlag;
        }
    } // No WebGL 1.0 support for BlitFramebuffer

    this->initBlendEqationSupport(ctxInfo);

    if (GR_IS_GR_GL(standard)) {
        fMapBufferFlags = kCanMap_MapFlag; // we require VBO support and the desktop VBO
                                            // extension includes glMapBuffer.
        if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_map_buffer_range")) {
            fMapBufferFlags |= kSubset_MapFlag;
            fMapBufferType = kMapBufferRange_MapBufferType;
        } else {
            fMapBufferType = kMapBuffer_MapBufferType;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        // Unextended GLES2 doesn't have any buffer mapping.
        fMapBufferFlags = kNone_MapFlags;
        if (ctxInfo.hasExtension("GL_CHROMIUM_map_sub")) {
            fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag;
            fMapBufferType = kChromium_MapBufferType;
        } else if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_EXT_map_buffer_range")) {
            fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag;
            fMapBufferType = kMapBufferRange_MapBufferType;
        } else if (ctxInfo.hasExtension("GL_OES_mapbuffer")) {
            fMapBufferFlags = kCanMap_MapFlag;
            fMapBufferType = kMapBuffer_MapBufferType;
        }
    } else if (GR_IS_GR_WEBGL(standard)) {
        // explicitly removed https://www.khronos.org/registry/webgl/specs/2.0/#5.14
        fMapBufferFlags = kNone_MapFlags;
    }

    if (GR_IS_GR_GL(standard)) {
        if (version >= GR_GL_VER(2, 1) || ctxInfo.hasExtension("GL_ARB_pixel_buffer_object") ||
            ctxInfo.hasExtension("GL_EXT_pixel_buffer_object")) {
            fTransferFromBufferToTextureSupport = true;
            fTransferFromSurfaceToBufferSupport = true;
            fTransferBufferType = TransferBufferType::kARB_PBO;
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (version >= GR_GL_VER(3, 0) ||
            (ctxInfo.hasExtension("GL_NV_pixel_buffer_object") &&
             // GL_EXT_unpack_subimage needed to support subtexture rectangles
             ctxInfo.hasExtension("GL_EXT_unpack_subimage"))) {
            fTransferFromBufferToTextureSupport = true;
            fTransferFromSurfaceToBufferSupport = true;
            if (version < GR_GL_VER(3, 0)) {
                fTransferBufferType = TransferBufferType::kNV_PBO;
            } else {
                fTransferBufferType = TransferBufferType::kARB_PBO;
            }
// TODO: get transfer buffers working in Chrome
//        } else if (ctxInfo.hasExtension("GL_CHROMIUM_pixel_transfer_buffer_object")) {
//            fTransferFromBufferToTextureSupport = false;
//            fTransferFromSurfaceToBufferSupport = false;
//            fTransferBufferType = TransferBufferType::kChromium;
        }
    } // no WebGL support

    // On many GPUs, map memory is very expensive, so we effectively disable it here by setting the
    // threshold to the maximum unless the client gives us a hint that map memory is cheap.
    if (fBufferMapThreshold < 0) {
#if 0
        // We think mapping on Chromium will be cheaper once we know ahead of time how much space
        // we will use for all GrMeshDrawOps. Right now we might wind up mapping a large buffer and
        // using a small subset.
        fBufferMapThreshold = ctxInfo.isOverCommandBuffer() ? 0 : SK_MaxS32;
#else
        fBufferMapThreshold = SK_MaxS32;
#endif
    }

    if (GR_IS_GR_GL(standard)) {
        fNPOTTextureTileSupport = true;
        fMipmapSupport = true;
    } else if (GR_IS_GR_GL_ES(standard)) {
        // Unextended ES2 supports NPOT textures with clamp_to_edge and non-mip filters only
        // ES3 has no limitations.
        fNPOTTextureTileSupport = version >= GR_GL_VER(3,0) ||
                                  ctxInfo.hasExtension("GL_OES_texture_npot");
        // ES2 supports MIP mapping for POT textures but our caps don't allow for limited MIP
        // support. The OES extension or ES 3.0 allow for MIPS on NPOT textures. So, apparently,
        // does the undocumented GL_IMG_texture_npot extension. This extension does not seem to
        // to alllow arbitrary wrap modes, however.
        fMipmapSupport = fNPOTTextureTileSupport || ctxInfo.hasExtension("GL_IMG_texture_npot");
    } else if (GR_IS_GR_WEBGL(standard)) {
        // Texture access works in the WebGL 2.0 API as in the OpenGL ES 3.0 API
        fNPOTTextureTileSupport = version >= GR_GL_VER(2,0);
        // All mipmapping and all wrapping modes are supported for non-power-of-
        // two images [in WebGL 2.0].
        fMipmapSupport = fNPOTTextureTileSupport;
    }

    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_SIZE, &fMaxTextureSize);

    if (fDriverBugWorkarounds.max_texture_size_limit_4096) {
        fMaxTextureSize = std::min(fMaxTextureSize, 4096);
    }

    GR_GL_GetIntegerv(gli, GR_GL_MAX_RENDERBUFFER_SIZE, &fMaxRenderTargetSize);
    fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;

    if (ctxInfo.vendor() == GrGLVendor::kARM) {
        // On Mali G71, RT's above 4k have been observed to incur a performance cost.
        fMaxPreferredRenderTargetSize = std::min(4096, fMaxPreferredRenderTargetSize);
    }

    fGpuTracingSupport = ctxInfo.hasExtension("GL_EXT_debug_marker");

    // Disable scratch texture reuse on Mali and Adreno devices
    fReuseScratchTextures = (ctxInfo.vendor() != GrGLVendor::kARM);

#if 0
    fReuseScratchBuffers = ctxInfo.vendor() != GrGLVendor::kARM
                           ctxInfo.vendor() != GrGLVendor::kQualcomm;
#endif

    if (ctxInfo.hasExtension("GL_EXT_window_rectangles")) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_WINDOW_RECTANGLES, &fMaxWindowRectangles);
    }

#ifdef SK_BUILD_FOR_WIN
    // We're assuming that on Windows Chromium we're using ANGLE.
    bool isANGLE = ctxInfo.angleBackend() != GrGLANGLEBackend::kUnknown ||
                   ctxInfo.isOverCommandBuffer();
    // On ANGLE deferring flushes can lead to GPU starvation
    fPreferVRAMUseOverFlushes = !isANGLE;
#endif

    if (ctxInfo.isOverCommandBuffer()) {
        fMustClearUploadedBufferData = true;
    }

    // In a WASM build on Firefox, we see warnings like
    // WebGL warning: texSubImage2D: This operation requires zeroing texture data. This is slow.
    // WebGL warning: texSubImage2D: Texture has not been initialized prior to a partial upload,
    //                forcing the browser to clear it. This may be slow.
    // Setting the initial clear seems to make those warnings go away and offers a substantial
    // boost in performance in Firefox. Chrome sees a more modest increase.
    if (GR_IS_GR_WEBGL(standard)) {
        fShouldInitializeTextures = true;
    }

    if (GR_IS_GR_GL(standard)) {
        // ARB allows mixed size FBO attachments, EXT does not.
        if (version >= GR_GL_VER(3, 0) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fOversizedStencilSupport = true;
        } else {
            SkASSERT(ctxInfo.hasExtension("GL_EXT_framebuffer_object"));
        }
    } else if (GR_IS_GR_GL_ES(standard)) {
        // ES 3.0 supports mixed size FBO attachments, 2.0 does not.
        fOversizedStencilSupport = version >= GR_GL_VER(3, 0);
    } else if (GR_IS_GR_WEBGL(standard)) {
        // WebGL 1.0 has some constraints for FBO attachments:
        // https://www.khronos.org/registry/webgl/specs/1.0/index.html#6.6
        // These constraints "no longer apply in WebGL 2"
        fOversizedStencilSupport = version >= GR_GL_VER(2, 0);
    }

    if (GR_IS_GR_GL(standard)) {
        fBaseVertexBaseInstanceSupport = version >= GR_GL_VER(4,2) ||
                                         ctxInfo.hasExtension("GL_ARB_base_instance");
        if (fBaseVertexBaseInstanceSupport) {
            fNativeDrawIndirectSupport = version >= GR_GL_VER(4,0) ||
                                         ctxInfo.hasExtension("GL_ARB_draw_indirect");
            if (version >= GR_GL_VER(4,3) || ctxInfo.hasExtension("GL_ARB_multi_draw_indirect")) {
                fMultiDrawType = MultiDrawType::kMultiDrawIndirect;
            }
        }
        fDrawRangeElementsSupport = version >= GR_GL_VER(2,0);
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (ctxInfo.hasExtension("GL_ANGLE_base_vertex_base_instance")) {
            fBaseVertexBaseInstanceSupport = true;
            fNativeDrawIndirectSupport = true;
            fMultiDrawType = MultiDrawType::kANGLEOrWebGL;
            // The indirect structs need to reside in CPU memory for the ANGLE version.
            fUseClientSideIndirectBuffers = true;
        } else {
            fBaseVertexBaseInstanceSupport = ctxInfo.hasExtension("GL_EXT_base_instance");
            if (fBaseVertexBaseInstanceSupport) {
                fNativeDrawIndirectSupport = (version >= GR_GL_VER(3,1));
                if (ctxInfo.hasExtension("GL_EXT_multi_draw_indirect")) {
                    fMultiDrawType = MultiDrawType::kMultiDrawIndirect;
                }
            }
        }
        fDrawRangeElementsSupport = version >= GR_GL_VER(3,0);
    } else if (GR_IS_GR_WEBGL(standard)) {
        fBaseVertexBaseInstanceSupport = ctxInfo.hasExtension(
                "WEBGL_draw_instanced_base_vertex_base_instance");
        if (fBaseVertexBaseInstanceSupport && ctxInfo.hasExtension(
                "GL_WEBGL_multi_draw_instanced_base_vertex_base_instance")) {
            fNativeDrawIndirectSupport = true;
            fMultiDrawType = MultiDrawType::kANGLEOrWebGL;
        }
        // The indirect structs need to reside in CPU memory for the WebGL version.
        fUseClientSideIndirectBuffers = true;
        fDrawRangeElementsSupport = version >= GR_GL_VER(2,0);
    }
    // We used to disable this as a correctness workaround (http://anglebug.com/4536). Now it is
    // disabled because of poor performance (http://skbug.com/11998).
    if (ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D11) {
        fBaseVertexBaseInstanceSupport = false;
        fNativeDrawIndirectSupport = false;
        fMultiDrawType = MultiDrawType::kNone;
    }

    // We prefer GL sync objects but also support NV_fence_sync. The former can be
    // used to implements GrFence and GrSemaphore. The latter only implements GrFence.
    // TODO: support CHROMIUM_sync_point and maybe KHR_fence_sync
    if (GR_IS_GR_WEBGL(standard)) {
        // Only in WebGL 2.0
        fSemaphoreSupport = fFenceSyncSupport = version >= GR_GL_VER(2, 0);
        fFenceType = FenceType::kSyncObject;
    } else if (GR_IS_GR_GL(standard) &&
               (version >= GR_GL_VER(3, 2) || ctxInfo.hasExtension("GL_ARB_sync"))) {
        fSemaphoreSupport = fFenceSyncSupport = true;
        fFenceType = FenceType::kSyncObject;
    } else if (GR_IS_GR_GL_ES(standard) &&
               (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_APPLE_sync"))) {
        fSemaphoreSupport = fFenceSyncSupport = true;
        fFenceType = FenceType::kSyncObject;
    } else if (ctxInfo.hasExtension("GL_NV_fence")) {
        // This extension can exist in GL and GL ES. We have it last because we prefer the
        // standard GLsync object implementation which also supports GPU semaphore semantics.
        fFenceSyncSupport = true;
        fFenceType = FenceType::kNVFence;
    }

    // Safely moving textures between contexts requires semaphores.
    fCrossContextTextureSupport = fSemaphoreSupport;

    // Half float vertex attributes requires GL3 or ES3
    // It can also work with OES_VERTEX_HALF_FLOAT, but that requires a different enum.
    if (GR_IS_GR_GL(standard)) {
        fHalfFloatVertexAttributeSupport = (version >= GR_GL_VER(3, 0));
    } else if (GR_IS_GR_GL_ES(standard)) {
        fHalfFloatVertexAttributeSupport = (version >= GR_GL_VER(3, 0));
    } else if (GR_IS_GR_WEBGL(standard)) {
        // This appears to be supported in 2.0, looking at the spec.
        fHalfFloatVertexAttributeSupport = (version >= GR_GL_VER(2, 0));
    }

    fDynamicStateArrayGeometryProcessorTextureSupport = true;

    if (GR_IS_GR_GL(standard)) {
        fProgramBinarySupport = (version >= GR_GL_VER(4, 1));
        fProgramParameterSupport = (version >= GR_GL_VER(4, 1));
    } else if (GR_IS_GR_GL_ES(standard)) {
        fProgramBinarySupport =
                (version >= GR_GL_VER(3, 0)) || ctxInfo.hasExtension("GL_OES_get_program_binary");
        fProgramParameterSupport = (version >= GR_GL_VER(3, 0));
    } // Explicitly not supported in WebGL 2.0
      // https://www.khronos.org/registry/webgl/specs/2.0/#5.4
    if (fProgramBinarySupport) {
        GrGLint count;
        GR_GL_GetIntegerv(gli, GR_GL_NUM_PROGRAM_BINARY_FORMATS, &count);
        fProgramBinarySupport = count > 0;
    }
    if (GR_IS_GR_GL(standard)) {
        fSamplerObjectSupport =
                version >= GR_GL_VER(3,3) || ctxInfo.hasExtension("GL_ARB_sampler_objects");
    } else if (GR_IS_GR_GL_ES(standard)) {
        fSamplerObjectSupport = version >= GR_GL_VER(3,0);
    } else if (GR_IS_GR_WEBGL(standard)) {
        fSamplerObjectSupport = version >= GR_GL_VER(2,0);
    }
    // We currently use sampler objects whenever they are available.
    fUseSamplerObjects = fSamplerObjectSupport;

    if (GR_IS_GR_GL_ES(standard)) {
        fTiledRenderingSupport = ctxInfo.hasExtension("GL_QCOM_tiled_rendering");
    }

    if (ctxInfo.vendor() == GrGLVendor::kARM) {
        fShouldCollapseSrcOverToSrcWhenAble = true;
    }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    if (ctxInfo.renderer() == GrGLRenderer::kPowerVRRogue) {
        // https://b/195281495
        // The TecnoSpark 3 Pro with a PowerVR GE8300 seems to have a steep dithering performance
        // cliff in the Android Framework
        fAvoidDithering = true;
    }
#endif

    FormatWorkarounds formatWorkarounds;

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(ctxInfo, contextOptions, gli, shaderCaps,
                                                &formatWorkarounds);
    }

    // Requires msaa support, ES compatibility have already been detected.
    this->initFormatTable(ctxInfo, gli, formatWorkarounds);

    this->finishInitialization(contextOptions);

    // For now these two are equivalent but we could have dst read in shader via some other method.
    shaderCaps->fDstReadInShaderSupport = shaderCaps->fFBFetchSupport;
}

const char* get_glsl_version_decl_string(GrGLStandard standard, GrGLSLGeneration generation,
                                         bool isCoreProfile) {
    if (GR_IS_GR_GL(standard)) {
        switch (generation) {
            case k110_GrGLSLGeneration:
                return "#version 110\n";
            case k130_GrGLSLGeneration:
                return "#version 130\n";
            case k140_GrGLSLGeneration:
                return "#version 140\n";
            case k150_GrGLSLGeneration:
                if (isCoreProfile) {
                    return "#version 150\n";
                } else {
                    return "#version 150 compatibility\n";
                }
            case k330_GrGLSLGeneration:
                if (isCoreProfile) {
                    return "#version 330\n";
                } else {
                    return "#version 330 compatibility\n";
                }
            case k400_GrGLSLGeneration:
                if (isCoreProfile) {
                    return "#version 400\n";
                } else {
                    return "#version 400 compatibility\n";
                }
            case k420_GrGLSLGeneration:
                if (isCoreProfile) {
                    return "#version 420\n";
                } else {
                    return "#version 420 compatibility\n";
                }
            default:
                break;
        }
    } else if (GR_IS_GR_GL_ES(standard) || GR_IS_GR_WEBGL(standard)) {
        switch (generation) {
            case k110_GrGLSLGeneration:
                // ES2s shader language is based on version 1.20 but is version
                // 1.00 of the ES language.
                return "#version 100\n";
            case k330_GrGLSLGeneration:
                return "#version 300 es\n";
            case k310es_GrGLSLGeneration:
                return "#version 310 es\n";
            case k320es_GrGLSLGeneration:
                return "#version 320 es\n";
            default:
                break;
        }
    }
    return "<no version>";
}

bool is_float_fp32(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli, GrGLenum precision) {
    if (GR_IS_GR_GL(ctxInfo.standard()) &&
        ctxInfo.version() < GR_GL_VER(4,1) &&
        !ctxInfo.hasExtension("GL_ARB_ES2_compatibility")) {
        // We're on a desktop GL that doesn't have precision info. Assume they're all 32bit float.
        return true;
    }
    // glGetShaderPrecisionFormat doesn't accept GL_GEOMETRY_SHADER as a shader type. Hopefully the
    // geometry shaders don't have lower precision than vertex and fragment.
    for (GrGLenum shader : {GR_GL_FRAGMENT_SHADER, GR_GL_VERTEX_SHADER}) {
        GrGLint range[2];
        GrGLint bits;
        GR_GL_GetShaderPrecisionFormat(gli, shader, precision, range, &bits);
        if (range[0] < 127 || range[1] < 127 || bits < 23) {
            return false;
        }
    }
    return true;
}

void GrGLCaps::initGLSL(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli) {
    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

    /**************************************************************************
    * Caps specific to GrShaderCaps
    **************************************************************************/

    GrShaderCaps* shaderCaps = fShaderCaps.get();
    shaderCaps->fGLSLGeneration = ctxInfo.glslGeneration();
    if (GR_IS_GR_GL_ES(standard)) {
        // fFBFetchRequiresEnablePerSample is not a shader cap but is initialized below to keep it
        // with related FB fetch logic.
        if (ctxInfo.hasExtension("GL_EXT_shader_framebuffer_fetch")) {
            shaderCaps->fFBFetchNeedsCustomOutput = (version >= GR_GL_VER(3, 0));
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "gl_LastFragData[0]";
            shaderCaps->fFBFetchExtensionString = "GL_EXT_shader_framebuffer_fetch";
            fFBFetchRequiresEnablePerSample = false;
        } else if (ctxInfo.hasExtension("GL_NV_shader_framebuffer_fetch")) {
            // Actually, we haven't seen an ES3.0 device with this extension yet, so we don't know.
            shaderCaps->fFBFetchNeedsCustomOutput = false;
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "gl_LastFragData[0]";
            shaderCaps->fFBFetchExtensionString = "GL_NV_shader_framebuffer_fetch";
            fFBFetchRequiresEnablePerSample = false;
        } else if (ctxInfo.hasExtension("GL_ARM_shader_framebuffer_fetch")) {
            // The arm extension also requires an additional flag which we will set onResetContext.
            shaderCaps->fFBFetchNeedsCustomOutput = false;
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "gl_LastFragColorARM";
            shaderCaps->fFBFetchExtensionString = "GL_ARM_shader_framebuffer_fetch";
            fFBFetchRequiresEnablePerSample = true;
        }
        shaderCaps->fUsesPrecisionModifiers = true;
    } else if (GR_IS_GR_GL(standard)) {
        if (ctxInfo.hasExtension("GL_EXT_shader_framebuffer_fetch")) {
            shaderCaps->fFBFetchNeedsCustomOutput = (version >= GR_GL_VER(3, 0));
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "gl_LastFragData[0]";
            shaderCaps->fFBFetchExtensionString = "GL_EXT_shader_framebuffer_fetch";
            fFBFetchRequiresEnablePerSample = false;
        }
    } else if (GR_IS_GR_WEBGL(standard)) {
        shaderCaps->fUsesPrecisionModifiers = true;
    }

    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fFlatInterpolationSupport = ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
    } else if (GR_IS_GR_GL_ES(standard) || GR_IS_GR_WEBGL(standard)) {
        shaderCaps->fFlatInterpolationSupport =
            ctxInfo.glslGeneration() >= k330_GrGLSLGeneration; // This is the value for GLSL ES 3.0.
    } // not sure for WebGL

    // Flat interpolation appears to be slow on Qualcomm GPUs (tested Adreno 405 and 530).
    // Avoid on ANGLE too, it inserts a geometry shader into the pipeline to implement flat interp.
    // Is this only true on ANGLE's D3D backends or also on the GL backend?
    shaderCaps->fPreferFlatInterpolation = shaderCaps->fFlatInterpolationSupport &&
                                           ctxInfo.vendor() != GrGLVendor::kQualcomm &&
                                           ctxInfo.angleBackend() == GrGLANGLEBackend::kUnknown;
    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fNoPerspectiveInterpolationSupport =
            ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (ctxInfo.hasExtension("GL_NV_shader_noperspective_interpolation") &&
            ctxInfo.glslGeneration() >= k330_GrGLSLGeneration /* GLSL ES 3.0 */) {
            shaderCaps->fNoPerspectiveInterpolationSupport = true;
            shaderCaps->fNoPerspectiveInterpolationExtensionString =
                "GL_NV_shader_noperspective_interpolation";
        }
    }  // Not sure for WebGL

    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fSampleMaskSupport = ctxInfo.glslGeneration() >= k400_GrGLSLGeneration;
    } else if (GR_IS_GR_GL_ES(standard)) {
        if (ctxInfo.glslGeneration() >= k320es_GrGLSLGeneration) {
            shaderCaps->fSampleMaskSupport = true;
        } else if (ctxInfo.hasExtension("GL_OES_sample_variables")) {
            shaderCaps->fSampleMaskSupport = true;
            shaderCaps->fSampleVariablesExtensionString = "GL_OES_sample_variables";
        }
    }

    bool hasTessellationSupport = false;
    if (GR_IS_GR_GL(standard)) {
        hasTessellationSupport = version >= GR_GL_VER(4,0) ||
                                 ctxInfo.hasExtension("GL_ARB_tessellation_shader");
    } else if (version >= GR_GL_VER(3,2)) {
        hasTessellationSupport = true;
    } else if (ctxInfo.hasExtension("GL_OES_tessellation_shader")) {
        hasTessellationSupport = true;
        shaderCaps->fTessellationExtensionString = "GL_OES_tessellation_shader";
    }
    if (hasTessellationSupport) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_TESS_GEN_LEVEL_OES,
                          &shaderCaps->fMaxTessellationSegments);
        // Just in case a driver returns a negative number?
        shaderCaps->fMaxTessellationSegments = std::max(0, shaderCaps->fMaxTessellationSegments);
    }

    shaderCaps->fVersionDeclString = get_glsl_version_decl_string(standard,
                                                                  shaderCaps->fGLSLGeneration,
                                                                  fIsCoreProfile);

    if (GR_IS_GR_GL_ES(standard) || GR_IS_GR_WEBGL(standard)) {
        if (k110_GrGLSLGeneration == shaderCaps->fGLSLGeneration) {
            shaderCaps->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
        }
    } // WebGL might have to check for OES_standard_derivatives

    if (GR_IS_GR_GL_ES(standard)) {
        shaderCaps->fSecondaryOutputExtensionString = "GL_EXT_blend_func_extended";
    }

    if (ctxInfo.hasExtension("GL_OES_EGL_image_external")) {
        if (ctxInfo.glslGeneration() == k110_GrGLSLGeneration) {
            shaderCaps->fExternalTextureSupport = true;
            shaderCaps->fExternalTextureExtensionString = "GL_OES_EGL_image_external";
        } else if (ctxInfo.hasExtension("GL_OES_EGL_image_external_essl3") ||
                   ctxInfo.hasExtension("OES_EGL_image_external_essl3")) {
            // At least one driver has been found that has this extension without the "GL_" prefix.
            shaderCaps->fExternalTextureSupport = true;
            shaderCaps->fExternalTextureExtensionString = "GL_OES_EGL_image_external_essl3";
        }
    }

    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fVertexIDSupport = true;
    } else if (GR_IS_GR_GL_ES(standard) || GR_IS_GR_WEBGL(standard)) {
        // Desktop GLSL 3.30 == ES GLSL 3.00.
        shaderCaps->fVertexIDSupport = ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
    }

    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fInfinitySupport = true;
    } else if (GR_IS_GR_GL_ES(standard) || GR_IS_GR_WEBGL(standard)) {
        // Desktop GLSL 3.30 == ES GLSL 3.00.
        shaderCaps->fInfinitySupport = ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
    }

    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fBitManipulationSupport = ctxInfo.glslGeneration() >= k400_GrGLSLGeneration;
    } else if (GR_IS_GR_GL_ES(standard) || GR_IS_GR_WEBGL(standard)) {
        shaderCaps->fBitManipulationSupport = ctxInfo.glslGeneration() >= k310es_GrGLSLGeneration;
    }

    shaderCaps->fFloatIs32Bits = is_float_fp32(ctxInfo, gli, GR_GL_HIGH_FLOAT);
    shaderCaps->fHalfIs32Bits = is_float_fp32(ctxInfo, gli, GR_GL_MEDIUM_FLOAT);
    shaderCaps->fHasLowFragmentPrecision = ctxInfo.renderer() == GrGLRenderer::kMali4xx;

    if (GR_IS_GR_GL(standard)) {
        shaderCaps->fBuiltinFMASupport = ctxInfo.glslGeneration() >= k400_GrGLSLGeneration;
    } else if (GR_IS_GR_GL_ES(standard)) {
        shaderCaps->fBuiltinFMASupport = ctxInfo.glslGeneration() >= k320es_GrGLSLGeneration;
    }

    shaderCaps->fBuiltinDeterminantSupport = ctxInfo.glslGeneration() >= k150_GrGLSLGeneration;

    if (GR_IS_GR_WEBGL(standard)) {
      // WebGL 1.0 doesn't support do-while loops.
      shaderCaps->fCanUseDoLoops = version >= GR_GL_VER(2, 0);
    }
}

void GrGLCaps::initFSAASupport(const GrContextOptions& contextOptions,
                               const GrGLContextInfo& ctxInfo, const GrGLInterface* gli) {
    if (GR_IS_GR_GL(ctxInfo.standard())) {
        if (ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fMSFBOType = kStandard_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_EXT_framebuffer_multisample") &&
                   ctxInfo.hasExtension("GL_EXT_framebuffer_blit")) {
            fMSFBOType = kStandard_MSFBOType;
        }
    } else if (GR_IS_GR_GL_ES(ctxInfo.standard())) {
        // We prefer multisampled-render-to-texture extensions over ES3 MSAA because we've observed
        // ES3 driver bugs on at least one device with a tiled GPU (N10).
        if (ctxInfo.hasExtension("GL_EXT_multisampled_render_to_texture")) {
            fMSFBOType = kES_EXT_MsToTexture_MSFBOType;
            fMSAAResolvesAutomatically = true;
        } else if (ctxInfo.hasExtension("GL_IMG_multisampled_render_to_texture")) {
            fMSFBOType = kES_IMG_MsToTexture_MSFBOType;
            fMSAAResolvesAutomatically = true;
        } else if (ctxInfo.version() >= GR_GL_VER(3,0)) {
            fMSFBOType = kStandard_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_multisample")) {
            fMSFBOType = kStandard_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_ANGLE_framebuffer_multisample")) {
            fMSFBOType = kStandard_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_APPLE_framebuffer_multisample")) {
            fMSFBOType = kES_Apple_MSFBOType;
        }
    } else if (GR_IS_GR_WEBGL(ctxInfo.standard())) {
        // No support in WebGL 1, but there is for 2.0
        if (ctxInfo.version() >= GR_GL_VER(2,0)) {
            fMSFBOType = kStandard_MSFBOType;
        } else {
            fMSFBOType = kNone_MSFBOType;
        }
    }
}

void GrGLCaps::initBlendEqationSupport(const GrGLContextInfo& ctxInfo) {
    GrShaderCaps* shaderCaps = static_cast<GrShaderCaps*>(fShaderCaps.get());

    bool layoutQualifierSupport = false;
    if ((GR_IS_GR_GL(fStandard) && shaderCaps->generation() >= k140_GrGLSLGeneration)  ||
        (GR_IS_GR_GL_ES(fStandard) && shaderCaps->generation() >= k330_GrGLSLGeneration)) {
        layoutQualifierSupport = true;
    } else if (GR_IS_GR_WEBGL(fStandard)) {
        return;
    }

    if (ctxInfo.hasExtension("GL_NV_blend_equation_advanced_coherent")) {
        fBlendEquationSupport = kAdvancedCoherent_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kAutomatic_AdvBlendEqInteraction;
    } else if (ctxInfo.hasExtension("GL_KHR_blend_equation_advanced_coherent") &&
               layoutQualifierSupport) {
        fBlendEquationSupport = kAdvancedCoherent_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kGeneralEnable_AdvBlendEqInteraction;
    } else if (ctxInfo.hasExtension("GL_NV_blend_equation_advanced")) {
        fBlendEquationSupport = kAdvanced_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kAutomatic_AdvBlendEqInteraction;
    } else if (ctxInfo.hasExtension("GL_KHR_blend_equation_advanced") && layoutQualifierSupport) {
        fBlendEquationSupport = kAdvanced_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kGeneralEnable_AdvBlendEqInteraction;
    }
}


void GrGLCaps::initStencilSupport(const GrGLContextInfo& ctxInfo) {

    // Build up list of legal stencil formats (though perhaps not supported on
    // the particular gpu/driver) from most preferred to least.

    // We push back stencil formats onto the fStencilFormats array in order of most preferred to
    // least preferred.

    if (GR_IS_GR_GL(ctxInfo.standard())) {
        bool supportsPackedDS =
            ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_EXT_packed_depth_stencil") ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object");

        // S1 thru S16 formats are in GL 3.0+, EXT_FBO, and ARB_FBO since we
        // require FBO support we can expect these are legal formats and don't
        // check.
        fStencilFormats.push_back() = GrGLFormat::kSTENCIL_INDEX8;
        fStencilFormats.push_back() = GrGLFormat::kSTENCIL_INDEX16;
        if (supportsPackedDS) {
            fStencilFormats.push_back() = GrGLFormat::kDEPTH24_STENCIL8;
        }
    } else if (GR_IS_GR_GL_ES(ctxInfo.standard())) {
        // ES2 has STENCIL_INDEX8 without extensions but requires extensions
        // for other formats.

        fStencilFormats.push_back() = GrGLFormat::kSTENCIL_INDEX8;
        if (ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_OES_packed_depth_stencil")) {
            fStencilFormats.push_back() = GrGLFormat::kDEPTH24_STENCIL8;
        }
    } else if (GR_IS_GR_WEBGL(ctxInfo.standard())) {
        fStencilFormats.push_back() = GrGLFormat::kSTENCIL_INDEX8;
        if (ctxInfo.version() >= GR_GL_VER(2,0)) {
            fStencilFormats.push_back() = GrGLFormat::kDEPTH24_STENCIL8;
        }
    }
}

#ifdef SK_ENABLE_DUMP_GPU
static const char* multi_draw_type_name(GrGLCaps::MultiDrawType multiDrawType) {
    switch (multiDrawType) {
        case GrGLCaps::MultiDrawType::kNone : return "kNone";
        case GrGLCaps::MultiDrawType::kMultiDrawIndirect : return "kMultiDrawIndirect";
        case GrGLCaps::MultiDrawType::kANGLEOrWebGL : return "kMultiDrawIndirect";
    }
    SkUNREACHABLE;
}

void GrGLCaps::onDumpJSON(SkJSONWriter* writer) const {

    // We are called by the base class, which has already called beginObject(). We choose to nest
    // all of our caps information in a named sub-object.
    writer->beginObject("GL caps");

    writer->beginArray("Stencil Formats");

    for (int i = 0; i < fStencilFormats.count(); ++i) {
        writer->beginObject(nullptr, false);
        writer->appendS32("stencil bits", GrGLFormatStencilBits(fStencilFormats[i]));
        writer->appendS32("total bytes", GrGLFormatBytesPerBlock(fStencilFormats[i]));
        writer->endObject();
    }

    writer->endArray();

    static const char* kMSFBOExtStr[] = {
        "None",
        "Standard",
        "Apple",
        "IMG MS To Texture",
        "EXT MS To Texture",
    };
    static_assert(0 == kNone_MSFBOType);
    static_assert(1 == kStandard_MSFBOType);
    static_assert(2 == kES_Apple_MSFBOType);
    static_assert(3 == kES_IMG_MsToTexture_MSFBOType);
    static_assert(4 == kES_EXT_MsToTexture_MSFBOType);
    static_assert(SK_ARRAY_COUNT(kMSFBOExtStr) == kLast_MSFBOType + 1);

    static const char* kInvalidateFBTypeStr[] = {
        "None",
        "Discard",
        "Invalidate",
    };
    static_assert(0 == kNone_InvalidateFBType);
    static_assert(1 == kDiscard_InvalidateFBType);
    static_assert(2 == kInvalidate_InvalidateFBType);
    static_assert(SK_ARRAY_COUNT(kInvalidateFBTypeStr) == kLast_InvalidateFBType + 1);

    static const char* kMapBufferTypeStr[] = {
        "None",
        "MapBuffer",
        "MapBufferRange",
        "Chromium",
    };
    static_assert(0 == kNone_MapBufferType);
    static_assert(1 == kMapBuffer_MapBufferType);
    static_assert(2 == kMapBufferRange_MapBufferType);
    static_assert(3 == kChromium_MapBufferType);
    static_assert(SK_ARRAY_COUNT(kMapBufferTypeStr) == kLast_MapBufferType + 1);

    writer->appendBool("Core Profile", fIsCoreProfile);
    writer->appendString("MSAA Type", kMSFBOExtStr[fMSFBOType]);
    writer->appendString("Invalidate FB Type", kInvalidateFBTypeStr[fInvalidateFBType]);
    writer->appendString("Map Buffer Type", kMapBufferTypeStr[fMapBufferType]);
    writer->appendString("Multi Draw Type", multi_draw_type_name(fMultiDrawType));
    writer->appendS32("Max FS Uniform Vectors", fMaxFragmentUniformVectors);
    writer->appendBool("Pack Flip Y support", fPackFlipYSupport);

    writer->appendBool("Texture Usage support", fTextureUsageSupport);
    writer->appendBool("GL_ARB_imaging support", fImagingSupport);
    writer->appendBool("Vertex array object support", fVertexArrayObjectSupport);
    writer->appendBool("Debug support", fDebugSupport);
    writer->appendBool("ES2 compatibility support", fES2CompatibilitySupport);
    writer->appendBool("drawRangeElements support", fDrawRangeElementsSupport);
    writer->appendBool("Base (vertex base) instance support", fBaseVertexBaseInstanceSupport);
    writer->appendBool("Bind uniform location support", fBindUniformLocationSupport);
    writer->appendBool("Rectangle texture support", fRectangleTextureSupport);
    writer->appendBool("Mipmap LOD control support", fMipmapLodControlSupport);
    writer->appendBool("Mipmap level control support", fMipmapLevelControlSupport);
    writer->appendBool("Use buffer data null hint", fUseBufferDataNullHint);
    writer->appendBool("Clear texture support", fClearTextureSupport);
    writer->appendBool("Program binary support", fProgramBinarySupport);
    writer->appendBool("Program parameters support", fProgramParameterSupport);
    writer->appendBool("Sampler object support", fSamplerObjectSupport);
    writer->appendBool("Using sampler objects", fUseSamplerObjects);
    writer->appendBool("Texture swizzle support", fTextureSwizzleSupport);
    writer->appendBool("Tiled rendering support", fTiledRenderingSupport);
    writer->appendBool("FB fetch requires enable per sample", fFBFetchRequiresEnablePerSample);
    writer->appendBool("sRGB Write Control", fSRGBWriteControl);

    writer->appendBool("Intermediate texture for partial updates of unorm textures ever bound to FBOs",
                       fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO);
    writer->appendBool("Intermediate texture for all updates of textures bound to FBOs",
                       fUseDrawInsteadOfAllRenderTargetWrites);
    writer->appendBool("Max instances per draw without crashing (or zero)",
                       fMaxInstancesPerDrawWithoutCrashing);

    writer->beginArray("formats");

    for (int i = 0; i < kGrGLColorFormatCount; ++i) {
        writer->beginObject(nullptr, false);
        writer->appendHexU32("flags", fFormatTable[i].fFlags);
        writer->appendHexU32("f_type", (uint32_t)fFormatTable[i].fFormatType);
        writer->appendHexU32("c_internal", fFormatTable[i].fCompressedInternalFormat);
        writer->appendHexU32("i_for_teximage", fFormatTable[i].fInternalFormatForTexImageOrStorage);
        writer->appendHexU32("i_for_renderbuffer", fFormatTable[i].fInternalFormatForRenderbuffer);
        writer->appendHexU32("default_ex_format", fFormatTable[i].fDefaultExternalFormat);
        writer->appendHexU32("default_ex_type", fFormatTable[i].fDefaultExternalType);
        writer->appendHexU32("default_color_type", (uint32_t)fFormatTable[i].fDefaultColorType);

        writer->beginArray("surface color types");
        for (int j = 0; j < fFormatTable[i].fColorTypeInfoCount; ++j) {
            const auto& ctInfo = fFormatTable[i].fColorTypeInfos[j];
            writer->beginObject(nullptr, false);
            writer->appendHexU32("colorType", (uint32_t)ctInfo.fColorType);
            writer->appendHexU32("flags", ctInfo.fFlags);

            writer->beginArray("data color types");
            for (int k = 0; k < ctInfo.fExternalIOFormatCount; ++k) {
                const auto& ioInfo = ctInfo.fExternalIOFormats[k];
                writer->beginObject(nullptr, false);
                writer->appendHexU32("colorType", (uint32_t)ioInfo.fColorType);
                writer->appendHexU32("ex_type", ioInfo.fExternalType);
                writer->appendHexU32("ex_teximage", ioInfo.fExternalTexImageFormat);
                writer->appendHexU32("ex_read", ioInfo.fExternalReadFormat);
                writer->endObject();
            }
            writer->endArray();
            writer->endObject();
        }
        writer->endArray();
        writer->endObject();
    }

    writer->endArray();
    writer->endObject();
}
#else
void GrGLCaps::onDumpJSON(SkJSONWriter* writer) const { }
#endif

void GrGLCaps::getTexImageExternalFormatAndType(GrGLFormat surfaceFormat, GrGLenum* externalFormat,
                                                GrGLenum* externalType) const {
    const auto& info = this->getFormatInfo(surfaceFormat);
    *externalType = info.fDefaultExternalType;
    *externalFormat = info.fDefaultExternalFormat;
}

void GrGLCaps::getTexSubImageDefaultFormatTypeAndColorType(GrGLFormat format,
                                                           GrGLenum* externalFormat,
                                                           GrGLenum* externalType,
                                                           GrColorType* colorType) const {
    const auto& info = this->getFormatInfo(format);
    *externalType = info.fDefaultExternalType;
    *externalFormat = info.fDefaultExternalFormat;
    *colorType = info.fDefaultColorType;
}

void GrGLCaps::getTexSubImageExternalFormatAndType(GrGLFormat surfaceFormat,
                                                   GrColorType surfaceColorType,
                                                   GrColorType memoryColorType,
                                                   GrGLenum* externalFormat,
                                                   GrGLenum* externalType) const {
    this->getExternalFormat(surfaceFormat, surfaceColorType, memoryColorType,
                            kTexImage_ExternalFormatUsage, externalFormat, externalType);
}

void GrGLCaps::getReadPixelsFormat(GrGLFormat surfaceFormat, GrColorType surfaceColorType,
                                   GrColorType memoryColorType, GrGLenum* externalFormat,
                                   GrGLenum* externalType) const {
    this->getExternalFormat(surfaceFormat, surfaceColorType, memoryColorType,
                            kReadPixels_ExternalFormatUsage, externalFormat, externalType);
}

void GrGLCaps::getExternalFormat(GrGLFormat surfaceFormat, GrColorType surfaceColorType,
                                 GrColorType memoryColorType, ExternalFormatUsage usage,
                                 GrGLenum* externalFormat, GrGLenum* externalType) const {
    SkASSERT(externalFormat && externalType);
    *externalFormat = this->getFormatInfo(surfaceFormat).externalFormat(
            surfaceColorType, memoryColorType, usage);
    *externalType = this->getFormatInfo(surfaceFormat).externalType(
            surfaceColorType, memoryColorType);
}

void GrGLCaps::setStencilFormatIndexForFormat(GrGLFormat format, int index) {
    SkASSERT(!this->hasStencilFormatBeenDeterminedForFormat(format));
    this->getFormatInfo(format).fStencilFormatIndex =
            index < 0 ? FormatInfo::kUnsupported_StencilFormatIndex : index;
}

void GrGLCaps::setColorTypeFormat(GrColorType colorType, GrGLFormat format) {
    int idx = static_cast<int>(colorType);
    SkASSERT(fColorTypeToFormatTable[idx] == GrGLFormat::kUnknown);
    fColorTypeToFormatTable[idx] = format;
}

void GrGLCaps::initFormatTable(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli,
                               const FormatWorkarounds& formatWorkarounds) {
    GrGLStandard standard = ctxInfo.standard();
    // standard can be unused (optimized away) if SK_ASSUME_GL_ES is set
    sk_ignore_unused_variable(standard);
    GrGLVersion version = ctxInfo.version();

    uint32_t nonMSAARenderFlags = FormatInfo::kFBOColorAttachment_Flag;
    uint32_t msaaRenderFlags = nonMSAARenderFlags;
    if (kNone_MSFBOType != fMSFBOType) {
        msaaRenderFlags |= FormatInfo::kFBOColorAttachmentWithMSAA_Flag;
    }

    bool texStorageSupported = false;
    if (GR_IS_GR_GL(standard)) {
        // The EXT version can apply to either GL or GLES.
        texStorageSupported = version >= GR_GL_VER(4,2) ||
                              ctxInfo.hasExtension("GL_ARB_texture_storage") ||
                              ctxInfo.hasExtension("GL_EXT_texture_storage");
    } else if (GR_IS_GR_GL_ES(standard)) {
        texStorageSupported = version >= GR_GL_VER(3,0) ||
                              ctxInfo.hasExtension("GL_EXT_texture_storage");
    } else if (GR_IS_GR_WEBGL(standard)) {
        texStorageSupported = version >= GR_GL_VER(2,0);
    }
    if (fDriverBugWorkarounds.disable_texture_storage) {
        texStorageSupported = false;
    }
#ifdef SK_BUILD_FOR_ANDROID
    // crbug.com/945506. Telemetry reported a memory usage regression for Android Go Chrome/WebView
    // when using glTexStorage2D. This appears to affect OOP-R (so not just over command buffer).
    if (!formatWorkarounds.fDontDisableTexStorageOnAndroid) {
        texStorageSupported = false;
    }
#endif

    // ES 2.0 requires that the internal/external formats match so we can't use sized internal
    // formats for glTexImage until ES 3.0. TODO: Support sized internal formats in WebGL2.
    bool texImageSupportsSizedInternalFormat =
            (GR_IS_GR_GL(standard) || (GR_IS_GR_GL_ES(standard) && version >= GR_GL_VER(3,0)));

    // for now we don't support floating point MSAA on ES
    uint32_t fpRenderFlags = (GR_IS_GR_GL(standard)) ? msaaRenderFlags : nonMSAARenderFlags;

    for (int i = 0; i < kGrColorTypeCnt; ++i) {
        fColorTypeToFormatTable[i] = GrGLFormat::kUnknown;
    }

    ///////////////////////////////////////////////////////////////////////////

    GrGLenum halfFloatType = GR_GL_HALF_FLOAT;
    if ((GR_IS_GR_GL_ES(standard) && version < GR_GL_VER(3, 0)) ||
        (GR_IS_GR_WEBGL(standard) && version < GR_GL_VER(2, 0))) {
        halfFloatType = GR_GL_HALF_FLOAT_OES;
    }

    // Format: RGBA8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRGBA8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_RGBA8;
        info.fDefaultExternalFormat = GR_GL_RGBA;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kRGBA_8888;
        info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
        if (GR_IS_GR_GL(standard)) {
            info.fFlags |= msaaRenderFlags;
        } else if (GR_IS_GR_GL_ES(standard)) {
            if (version >= GR_GL_VER(3,0) || ctxInfo.hasExtension("GL_OES_rgb8_rgba8") ||
                ctxInfo.hasExtension("GL_ARM_rgba8")) {
                info.fFlags |= msaaRenderFlags;
            }
        } else if (GR_IS_GR_WEBGL(standard)) {
            info.fFlags |= msaaRenderFlags;
        }

        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RGBA8;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RGBA8 : GR_GL_RGBA;
        }

        bool supportsBGRAColorType = GR_IS_GR_GL(standard) &&
                (version >= GR_GL_VER(1, 2) || ctxInfo.hasExtension("GL_EXT_bgra"));
        info.fColorTypeInfoCount = supportsBGRAColorType ? 3 : 2;
        info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RGBA8, Surface: kRGBA_8888
        {
            auto& ctInfo = info.fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGBA_8888;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            this->setColorTypeFormat(GrColorType::kRGBA_8888, GrGLFormat::kRGBA8);

            // External IO ColorTypes:
            ctInfo.fExternalIOFormatCount = 2;
            ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                    ctInfo.fExternalIOFormatCount);
            int ioIdx = 0;
            // Format: RGBA8, Surface: kRGBA_8888, Data: kRGBA_8888
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kRGBA_8888;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = GR_GL_RGBA;
                ioFormat.fExternalReadFormat = GR_GL_RGBA;
            }
            // Format: RGBA8, Surface: kRGBA_8888, Data: kBGRA_8888
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kBGRA_8888;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = 0;  // TODO: Enable this on non-ES GL
                ioFormat.fExternalReadFormat =
                        formatWorkarounds.fDisallowBGRA8ReadPixels ? 0 : GR_GL_BGRA;
                // Not guaranteed by ES/WebGL.
                ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
            }
        }

        // Format: RGBA8, Surface: kBGRA_8888
        if (supportsBGRAColorType) {
            auto& ctInfo = info.fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kBGRA_8888;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            this->setColorTypeFormat(GrColorType::kBGRA_8888, GrGLFormat::kRGBA8);

            // External IO ColorTypes:
            ctInfo.fExternalIOFormatCount = 2;
            ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                    ctInfo.fExternalIOFormatCount);
            int ioIdx = 0;
            // Format: RGBA8, Surface: kBGRA_8888, Data: kBGRA_8888
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kBGRA_8888;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = GR_GL_BGRA;
                ioFormat.fExternalReadFormat =
                        formatWorkarounds.fDisallowBGRA8ReadPixels ? 0 : GR_GL_BGRA;
                // Not guaranteed by ES/WebGL.
                ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
            }

            // Format: RGBA8, Surface: kBGRA_8888, Data: kRGBA_8888
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kRGBA_8888;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = 0;
                ioFormat.fExternalReadFormat = GR_GL_RGBA;
            }
        }

        // Format: RGBA8, Surface: kRGB_888x
        {
            auto& ctInfo = info.fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGB_888x;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            ctInfo.fReadSwizzle = GrSwizzle::RGB1();

            // External IO ColorTypes:
            ctInfo.fExternalIOFormatCount = 1;
            ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                    ctInfo.fExternalIOFormatCount);
            int ioIdx = 0;
            // Format: RGBA8, Surface: kRGB_888x, Data: kRGBA_888x
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kRGB_888x;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = GR_GL_RGBA;
                ioFormat.fExternalReadFormat = GR_GL_RGBA;
            }
        }
    }

    // Format: R8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kR8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_R8;
        info.fDefaultExternalFormat = GR_GL_RED;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kR_8;
        bool r8Support = false;
        if (GR_IS_GR_GL(standard)) {
            r8Support = version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_texture_rg");
        } else if (GR_IS_GR_GL_ES(standard)) {
            r8Support = version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_EXT_texture_rg");
        } else if (GR_IS_GR_WEBGL(standard)) {
            r8Support = ctxInfo.version() >= GR_GL_VER(2, 0);
        }
        if (formatWorkarounds.fDisallowR8ForPowerVRSGX54x) {
            r8Support = false;
        }

        if (r8Support) {
            info.fFlags |= FormatInfo::kTexturable_Flag
                        |  FormatInfo::kTransfers_Flag
                        |  msaaRenderFlags;
        }

        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_R8;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_R8 : GR_GL_RED;
        }

        if (r8Support) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: R8, Surface: kAlpha_8
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kAlpha_8;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("000r");
                ctInfo.fWriteSwizzle = GrSwizzle("a000");
                this->setColorTypeFormat(GrColorType::kAlpha_8, GrGLFormat::kR8);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: R8, Surface: kAlpha_8, Data: kAlpha_8
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kAlpha_8;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = GR_GL_RED;
                    ioFormat.fExternalReadFormat = GR_GL_RED;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: R8, Surface: kAlpha_8, Data: kAlpha_8xxx
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kAlpha_8xxx;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }

            // Format: R8, Surface: kGray_8
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kGray_8;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("rrr1");
                this->setColorTypeFormat(GrColorType::kGray_8, GrGLFormat::kR8);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: R8, Surface: kGray_8, Data: kGray_8
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kGray_8;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = GR_GL_RED;
                    ioFormat.fExternalReadFormat = GR_GL_RED;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: R8, Surface: kGray_8, Data: kGray_8xxx
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kGray_8xxx;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: ALPHA8
    {
        bool alpha8IsValidForGL = GR_IS_GR_GL(standard) &&
                (!fIsCoreProfile || version <= GR_GL_VER(3, 0));
        bool alpha8IsValidForGLES = GR_IS_GR_GL_ES(standard);
        bool alpha8IsValidForWebGL = GR_IS_GR_WEBGL(standard);

        FormatInfo& info = this->getFormatInfo(GrGLFormat::kALPHA8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        // GL_EXT_texture_storage adds GL_ALPHA8 for texture storage. However, ES3 has glTexStorage
        // but does not have GL_ALPHA8 (and requires a sized internal format for glTexStorage).
        // WebGL never has GL_ALPHA8.
        bool alpha8SizedEnumSupported =
                alpha8IsValidForGL ||
                (alpha8IsValidForGLES && ctxInfo.hasExtension("GL_EXT_texture_storage"));
        bool alpha8TexStorageSupported = alpha8SizedEnumSupported && texStorageSupported;

        bool alpha8IsRenderable = false;
        if (alpha8IsValidForGL) {
            // Core profile removes ALPHA8 support.
            // OpenGL 3.0+ (and GL_ARB_framebuffer_object) supports ALPHA8 as renderable.
            alpha8IsRenderable = ctxInfo.version() >= GR_GL_VER(3, 0) ||
                                 ctxInfo.hasExtension("GL_ARB_framebuffer_object");
        }
        info.fInternalFormatForRenderbuffer = GR_GL_ALPHA8;
        info.fDefaultExternalFormat = GR_GL_ALPHA;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kAlpha_8;
        if (alpha8IsValidForGL || alpha8IsValidForGLES || alpha8IsValidForWebGL) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
        }
        if (alpha8IsRenderable && alpha8IsValidForGL) {
            // We will use ALPHA8 to create MSAA renderbuffers.
            SkASSERT(alpha8SizedEnumSupported);
            info.fFlags |= msaaRenderFlags;
        }
        if (alpha8TexStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_ALPHA8;
        } else {
            // Even if GL_ALPHA8 is added to ES by GL_EXT_texture_storage it doesn't become legal
            // for glTexImage2D.
            if (!GR_IS_GR_GL_ES(standard) && texImageSupportsSizedInternalFormat &&
                alpha8SizedEnumSupported) {
                info.fInternalFormatForTexImageOrStorage = GR_GL_ALPHA8;
            } else {
                info.fInternalFormatForTexImageOrStorage = GR_GL_ALPHA;
            }
        }

        if (alpha8IsValidForGL || alpha8IsValidForGLES || alpha8IsValidForWebGL) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: ALPHA8, Surface: kAlpha_8
            {
                if (alpha8IsValidForGL || alpha8IsValidForGLES || alpha8IsValidForWebGL) {
                    auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                    ctInfo.fColorType = GrColorType::kAlpha_8;
                    ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag |
                                    ColorTypeInfo::kRenderable_Flag;
                    int idx = static_cast<int>(GrColorType::kAlpha_8);
                    if (fColorTypeToFormatTable[idx] == GrGLFormat::kUnknown) {
                        this->setColorTypeFormat(GrColorType::kAlpha_8, GrGLFormat::kALPHA8);
                    }

                    // External IO ColorTypes:
                    ctInfo.fExternalIOFormatCount = 2;
                    ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                            ctInfo.fExternalIOFormatCount);
                    int ioIdx = 0;
                    // Format: ALPHA8, Surface: kAlpha_8, Data: kAlpha_8
                    {
                        auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                        ioFormat.fColorType = GrColorType::kAlpha_8;
                        ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                        ioFormat.fExternalTexImageFormat = GR_GL_ALPHA;
                        ioFormat.fExternalReadFormat = GR_GL_ALPHA;
                        // Not guaranteed by ES/WebGL.
                        ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                    }

                    // Format: ALPHA8, Surface: kAlpha_8, Data: kRGBA_8888
                    {
                        auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                        ioFormat.fColorType = GrColorType::kRGBA_8888;
                        ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                        ioFormat.fExternalTexImageFormat = 0;
                        ioFormat.fExternalReadFormat = GR_GL_RGBA;
                    }
                }
            }
        }
    }

    // Format: LUMINANCE8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kLUMINANCE8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_LUMINANCE8;
        info.fDefaultExternalFormat = GR_GL_LUMINANCE;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kGray_8;
        bool lum8Supported = false;
        bool lum8SizedFormatSupported = false;
        if (GR_IS_GR_GL(standard) && !fIsCoreProfile) {
            lum8Supported = true;
            lum8SizedFormatSupported = true;
        } else if (GR_IS_GR_GL_ES(standard)) {
            lum8Supported = true;
            // Even on ES3 this extension is required to define LUMINANCE8.
            lum8SizedFormatSupported = ctxInfo.hasExtension("GL_EXT_texture_storage");
        } else if (GR_IS_GR_WEBGL(standard)) {
            lum8Supported = true;
        }
        if (lum8Supported) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
        }
        if (texStorageSupported && lum8SizedFormatSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE8;
        } else if (texImageSupportsSizedInternalFormat && lum8SizedFormatSupported) {
            info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE8;
        } else {
            info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE;
        }
        // We are not enabling attaching to an FBO for LUMINANCE8 mostly because of confusion in the
        // spec. For GLES it does not seem to ever support LUMINANCE8 being color-renderable. For GL
        // versions less than 3.0 it is provided by GL_ARB_framebuffer_object. However, the original
        // version of that extension did not add LUMINANCE8, but was added in a later revsion. So
        // even the presence of that extension does not guarantee support. GL 3.0 and higher (core
        // or compatibility) do not list LUMINANCE8 as color-renderable (which is strange since the
        // GL_ARB_framebuffer_object extension was meant to bring 3.0 functionality to lower
        // versions).

        if (lum8Supported) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: LUMINANCE8, Surface: kGray_8
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kGray_8;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                int idx = static_cast<int>(GrColorType::kGray_8);
                if (fColorTypeToFormatTable[idx] == GrGLFormat::kUnknown) {
                    this->setColorTypeFormat(GrColorType::kGray_8, GrGLFormat::kLUMINANCE8);
                }

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: LUMINANCE8, Surface: kGray_8, Data: kGray_8
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kGray_8;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = GR_GL_LUMINANCE;
                    ioFormat.fExternalReadFormat = 0;
                }

                // Format: LUMINANCE8, Surface: kGray_8, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: LUMINANCE8_ALPHA8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kLUMINANCE8_ALPHA8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_LUMINANCE8_ALPHA8;
        info.fDefaultExternalFormat = GR_GL_LUMINANCE_ALPHA;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kGrayAlpha_88;
        bool la8Supported = false;
        bool la8SizedFormatSupported = false;
        if (GR_IS_GR_GL(standard) && !fIsCoreProfile) {
            la8Supported = true;
            la8SizedFormatSupported = true;
        } else if (GR_IS_GR_GL_ES(standard)) {
            la8Supported = true;
            // Even on ES3 this extension is required to define LUMINANCE8_ALPHA8.
            la8SizedFormatSupported = ctxInfo.hasExtension("GL_EXT_texture_storage");
        } else if (GR_IS_GR_WEBGL(standard)) {
            la8Supported = true;
        }
        if (la8Supported) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
        }
        if (texStorageSupported && la8SizedFormatSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE8_ALPHA8;
        } else if (texImageSupportsSizedInternalFormat && la8SizedFormatSupported) {
            info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE8_ALPHA8;
        } else {
            info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE_ALPHA;
        }
        // See note in LUMINANCE8 section about not attaching to framebuffers.

        if (la8Supported) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: LUMINANCE8_ALPHA8, Surface: kGrayAlpha_88
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kGrayAlpha_88;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                int idx = static_cast<int>(GrColorType::kGrayAlpha_88);
                if (fColorTypeToFormatTable[idx] == GrGLFormat::kUnknown) {
                    this->setColorTypeFormat(GrColorType::kGrayAlpha_88,
                                             GrGLFormat::kLUMINANCE8_ALPHA8);
                }

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: LUMINANCE8, Surface: kGrayAlpha_88, Data: kGrayAlpha_88
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kGrayAlpha_88;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = GR_GL_LUMINANCE_ALPHA;
                    ioFormat.fExternalReadFormat = 0;
                }

                // Format: LUMINANCE8, Surface: kGrayAlpha_88, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }
    // Format: BGRA8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kBGRA8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;

        // We currently only use the renderbuffer format when allocating msaa renderbuffers, so we
        // are making decisions here based on that use case. The GL_EXT_texture_format_BGRA8888
        // extension adds BGRA color renderbuffer support for ES 2.0, but this does not guarantee
        // support for MSAA renderbuffers. Additionally, the renderable support was added in a later
        // revision of the extension. So it is possible for older drivers to support the extension
        // but only an early revision of it without renderable support. We have no way of
        // distinguishing between the two. The GL_APPLE_texture_format_BGRA8888 does not add support
        // for BGRA color renderbuffers at all. Ideally, for both cases we would use RGBA8 for our
        // format for the MSAA buffer. In the GL_EXT_texture_format_BGRA8888 case we can still
        // make the resolve BGRA and which will work for glBlitFramebuffer for resolving which just
        // requires the src and dst be bindable to FBOs. However, we can't do this in the current
        // world since some devices (e.g. chromium & angle) require the formats in glBlitFramebuffer
        // to match. We don't have a way to really check this during resolve since we only actually
        // have GrBackendFormat that is shared by the GrGLRenderTarget. We always set the
        // renderbuffer format to RGBA8 but disable MSAA unless we have the APPLE extension.
        // Once we break those up into different surface we can revisit doing this change.
        info.fInternalFormatForRenderbuffer = GR_GL_RGBA8;

        info.fDefaultExternalFormat = GR_GL_BGRA;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kBGRA_8888;

        GrGLenum bgraTexImageFormat;
        // If BGRA is supported as an internal format it must always be specified to glTex[Sub]Image
        // as a base format. Which base format depends on which extension is used.
        if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            // GL_APPLE_texture_format_BGRA8888:
            //     ES 2.0: the extension makes BGRA an external format but not an internal format.
            //     ES 3.0: the extension explicitly states GL_BGRA8 is not a valid internal format
            //             for glTexImage (just for glTexStorage).
            bgraTexImageFormat = GR_GL_RGBA;
        } else {
            // GL_EXT_texture_format_BGRA8888:
            //      This extension adds GL_BGRA as an unsized internal format. However, it is
            //      written against ES 2.0 and therefore doesn't define a GL_BGRA8 as ES 2.0 doesn't
            //      have sized internal formats. See later where we check for tex storage BGRA8
            //      support.
            bgraTexImageFormat = GR_GL_BGRA;
        }

        // TexStorage requires using a sized internal format and BGRA8 is only supported if we have
        // the GL_APPLE_texture_format_BGRA8888 extension or if we have GL_EXT_texture_storage and
        // GL_EXT_texture_format_BGRA8888.
        bool supportsBGRATexStorage = false;

        if (GR_IS_GR_GL_ES(standard)) {
            if (ctxInfo.hasExtension("GL_EXT_texture_format_BGRA8888")) {
                info.fFlags = FormatInfo::kTexturable_Flag
                            | FormatInfo::kTransfers_Flag
                            | nonMSAARenderFlags;
                // GL_EXT_texture storage has defined interactions with
                // GL_EXT_texture_format_BGRA8888. However, ES3 supports glTexStorage but
                // without GL_EXT_texture_storage it does not allow the BGRA8 sized internal format.
                if (ctxInfo.hasExtension("GL_EXT_texture_storage") &&
                    !formatWorkarounds.fDisableBGRATextureStorageForIntelWindowsES) {
                    supportsBGRATexStorage = true;
                }
            } else if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
                // This APPLE extension introduces complexity on ES2. It leaves the internal format
                // as RGBA, but allows BGRA as the external format. From testing, it appears that
                // the driver remembers the external format when the texture is created (with
                // TexImage). If you then try to upload data in the other swizzle (with
                // TexSubImage), it fails. We could work around this, but it adds even more state
                // tracking to code that is already too tricky. Instead, we opt not to support BGRA
                // on ES2 with this extension. This also side-steps some ambiguous interactions with
                // the texture storage extension.
                if (version >= GR_GL_VER(3,0)) {
                    // The APPLE extension doesn't explicitly make this renderable, but
                    // internally it appears to use RGBA8, which we'll patch up below.
                    info.fFlags = FormatInfo::kTexturable_Flag
                                | FormatInfo::kTransfers_Flag
                                | msaaRenderFlags;
                    supportsBGRATexStorage = true;
                }
            }
        }
        if (texStorageSupported && supportsBGRATexStorage) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_BGRA8;
        } else {
            info.fInternalFormatForTexImageOrStorage = bgraTexImageFormat;
        }

        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: BGRA8, Surface: kBGRA_8888
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kBGRA_8888;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kBGRA_8888, GrGLFormat::kBGRA8);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: BGRA8, Surface: kBGRA_8888, Data: kBGRA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kBGRA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = GR_GL_BGRA;
                    ioFormat.fExternalReadFormat = 0;
                    ioFormat.fExternalReadFormat =
                            formatWorkarounds.fDisallowBGRA8ReadPixels ? 0 : GR_GL_BGRA;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: BGRA8, Surface: kBGRA_8888, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: RGB565
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRGB565);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_RGB565;
        info.fDefaultExternalFormat = GR_GL_RGB;
        info.fDefaultExternalType = GR_GL_UNSIGNED_SHORT_5_6_5;
        info.fDefaultColorType = GrColorType::kBGR_565;
        if (GR_IS_GR_GL(standard)) {
            if (version >= GR_GL_VER(4, 2) || ctxInfo.hasExtension("GL_ARB_ES2_compatibility")) {
                info.fFlags = FormatInfo::kTexturable_Flag
                            | FormatInfo::kTransfers_Flag
                            | msaaRenderFlags;
            }
        } else if (GR_IS_GR_GL_ES(standard)) {
            info.fFlags = FormatInfo::kTexturable_Flag
                        | FormatInfo::kTransfers_Flag
                        | msaaRenderFlags;
        } else if (GR_IS_GR_WEBGL(standard)) {
            info.fFlags = FormatInfo::kTexturable_Flag
                        | FormatInfo::kTransfers_Flag
                        | msaaRenderFlags;
        }
        // 565 is not a sized internal format on desktop GL. So on desktop with
        // 565 we always use an unsized internal format to let the system pick
        // the best sized format to convert the 565 data to. Since TexStorage
        // only allows sized internal formats we disallow it.
        //
        // TODO: As of 4.2, regular GL supports 565. This logic is due for an
        // update.
        if (texStorageSupported && GR_IS_GR_GL_ES(standard)) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RGB565;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RGB565 : GR_GL_RGB;
        }

        if (SkToBool(info.fFlags &FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: RGB565, Surface: kBGR_565
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kBGR_565;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kBGR_565, GrGLFormat::kRGB565);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: RGB565, Surface: kBGR_565, Data: kBGR_565
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kBGR_565;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_SHORT_5_6_5;
                    ioFormat.fExternalTexImageFormat = GR_GL_RGB;
                    ioFormat.fExternalReadFormat = GR_GL_RGB;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: RGB565, Surface: kBGR_565, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: RGBA16F
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRGBA16F);
        info.fFormatType = FormatType::kFloat;
        info.fInternalFormatForRenderbuffer = GR_GL_RGBA16F;
        info.fDefaultExternalFormat = GR_GL_RGBA;
        info.fDefaultExternalType = halfFloatType;
        info.fDefaultColorType = GrColorType::kRGBA_F16;
        bool rgba16FTextureSupport = false;
        bool rgba16FRenderTargetSupport = false;

        if (GR_IS_GR_GL(standard)) {
            if (version >= GR_GL_VER(3, 0)) {
                rgba16FTextureSupport = true;
                rgba16FRenderTargetSupport = true;
            } else if (ctxInfo.hasExtension("GL_ARB_texture_float")) {
                rgba16FTextureSupport = true;
            }
        } else if (GR_IS_GR_GL_ES(standard)) {
            if (version >= GR_GL_VER(3, 0)) {
                rgba16FTextureSupport = true;
                rgba16FRenderTargetSupport =
                        version >= GR_GL_VER(3, 2) ||
                        ctxInfo.hasExtension("GL_EXT_color_buffer_half_float") ||
                        ctxInfo.hasExtension("GL_EXT_color_buffer_float");
            } else if (ctxInfo.hasExtension("GL_OES_texture_half_float") &&
                       ctxInfo.hasExtension("GL_OES_texture_half_float_linear")) {
                rgba16FTextureSupport = true;
                rgba16FRenderTargetSupport = ctxInfo.hasExtension("GL_EXT_color_buffer_half_float");
            }
        } else if (GR_IS_GR_WEBGL(standard)) {
            if (version >= GR_GL_VER(2, 0)) {
                rgba16FTextureSupport = true;
                rgba16FRenderTargetSupport =
                        ctxInfo.hasExtension("GL_EXT_color_buffer_half_float") ||
                        ctxInfo.hasExtension("EXT_color_buffer_half_float") ||
                        ctxInfo.hasExtension("GL_EXT_color_buffer_float") ||
                        ctxInfo.hasExtension("EXT_color_buffer_float");
            } else if ((ctxInfo.hasExtension("GL_OES_texture_half_float") ||
                        ctxInfo.hasExtension("OES_texture_half_float")) &&
                       (ctxInfo.hasExtension("GL_OES_texture_half_float_linear") ||
                        ctxInfo.hasExtension("OES_texture_half_float_linear"))) {
                rgba16FTextureSupport = true;
                // We don't check for EXT_color_buffer_float as it's only defined for WebGL 2.
                rgba16FRenderTargetSupport =
                        ctxInfo.hasExtension("GL_EXT_color_buffer_half_float") ||
                        ctxInfo.hasExtension("EXT_color_buffer_half_float");
            }
        }

        if (rgba16FTextureSupport) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
            if (rgba16FRenderTargetSupport) {
                info.fFlags |= fpRenderFlags;
            }
        }
        if (texStorageSupported && !formatWorkarounds.fDisableRGBA16FTexStorageForCrBug1008003) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RGBA16F;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RGBA16F : GR_GL_RGBA;
        }

        if (rgba16FTextureSupport) {
            uint32_t flags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;

            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: RGBA16F, Surface: kRGBA_F16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_F16;
                ctInfo.fFlags = flags;
                this->setColorTypeFormat(GrColorType::kRGBA_F16, GrGLFormat::kRGBA16F);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: RGBA16F, Surface: kRGBA_F16, Data: kRGBA_F16
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_F16;
                    ioFormat.fExternalType = halfFloatType;
                    ioFormat.fExternalTexImageFormat = GR_GL_RGBA;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: RGBA16F, Surface: kRGBA_F16, Data: kRGBA_F32
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_F32;
                    ioFormat.fExternalType = GR_GL_FLOAT;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }

            // Format: RGBA16F, Surface: kRGBA_F16_Clamped
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_F16_Clamped;
                ctInfo.fFlags = flags;
                this->setColorTypeFormat(GrColorType::kRGBA_F16_Clamped, GrGLFormat::kRGBA16F);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: RGBA16F, Surface: kRGBA_F16_Clamped, Data: kRGBA_F16_Clamped
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_F16_Clamped;
                    ioFormat.fExternalType = halfFloatType;
                    ioFormat.fExternalTexImageFormat = GR_GL_RGBA;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: RGBA16F, Surface: kRGBA_F16_Clamped, Data: kRGBA_F32
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_F32;
                    ioFormat.fExternalType = GR_GL_FLOAT;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: R16F
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kR16F);
        info.fFormatType = FormatType::kFloat;
        info.fInternalFormatForRenderbuffer = GR_GL_R16F;
        info.fDefaultExternalFormat = GR_GL_RED;
        info.fDefaultExternalType = halfFloatType;
        info.fDefaultColorType = GrColorType::kR_F16;
        bool r16FTextureSupport = false;
        bool r16FRenderTargetSupport = false;

        if (GR_IS_GR_GL(standard)) {
            if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_texture_rg")) {
                r16FTextureSupport = true;
                r16FRenderTargetSupport = true;
            }
        } else if (GR_IS_GR_GL_ES(standard)) {
            // It seems possible that a combination of GL_EXT_texture_rg and
            // GL_EXT_color_buffer_half_float might add this format to ES 2.0 but it is not entirely
            // clear. The latter mentions interaction but that may only be for renderbuffers as
            // neither adds the texture format explicitly.
            // GL_OES_texture_format_half_float makes no reference to RED formats.
            if (version >= GR_GL_VER(3, 0)) {
                r16FTextureSupport = true;
                r16FRenderTargetSupport = version >= GR_GL_VER(3, 2) ||
                                          ctxInfo.hasExtension("GL_EXT_color_buffer_float") ||
                                          ctxInfo.hasExtension("GL_EXT_color_buffer_half_float");
            }
        } else if (GR_IS_GR_WEBGL(standard)) {
            if (version >= GR_GL_VER(2, 0)) {
                r16FTextureSupport = true;
                r16FRenderTargetSupport = ctxInfo.hasExtension("GL_EXT_color_buffer_float") ||
                                          ctxInfo.hasExtension("EXT_color_buffer_float");
            }
        }

        if (r16FTextureSupport) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
            if (r16FRenderTargetSupport) {
                info.fFlags |= fpRenderFlags;
            }
        }
        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_R16F;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_R16F : GR_GL_RED;
        }

        if (r16FTextureSupport) {
            // Format: R16F, Surface: kAlpha_F16
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kAlpha_F16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("000r");
                ctInfo.fWriteSwizzle = GrSwizzle("a000");
                this->setColorTypeFormat(GrColorType::kAlpha_F16, GrGLFormat::kR16F);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: R16F, Surface: kAlpha_F16, Data: kAlpha_F16
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kAlpha_F16;
                    ioFormat.fExternalType = halfFloatType;
                    ioFormat.fExternalTexImageFormat = GR_GL_RED;
                    ioFormat.fExternalReadFormat = GR_GL_RED;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: R16F, Surface: kAlpha_F16, Data: kAlpha_F32xxx
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kAlpha_F32xxx;
                    ioFormat.fExternalType = GR_GL_FLOAT;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: LUMINANCE16F
    {
        // NOTE: We disallow lum16f on ES devices if linear filtering modes are not
        // supported. This is for simplicity, but a more granular approach is possible.
        bool lum16FSupported = false;
        bool lum16FSizedFormatSupported = false;
        if (GR_IS_GR_GL(standard)) {
            if (!fIsCoreProfile && ctxInfo.hasExtension("GL_ARB_texture_float")) {
                lum16FSupported = true;
                lum16FSizedFormatSupported = true;
            }
        } else if (GR_IS_GR_GL_ES(standard)) {
            if (ctxInfo.hasExtension("GL_OES_texture_half_float_linear") &&
                ctxInfo.hasExtension("GL_OES_texture_half_float")) {
                lum16FSupported = true;
                // Even on ES3 this extension is required to define LUMINANCE16F.
                lum16FSizedFormatSupported = ctxInfo.hasExtension("GL_EXT_texture_storage");
            }
        } // No WebGL support

        if (formatWorkarounds.fDisableLuminance16F) {
            lum16FSupported = false;
        }

        FormatInfo& info = this->getFormatInfo(GrGLFormat::kLUMINANCE16F);
        info.fFormatType = FormatType::kFloat;
        info.fInternalFormatForRenderbuffer = GR_GL_LUMINANCE16F;
        info.fDefaultExternalFormat = GR_GL_LUMINANCE;
        info.fDefaultExternalType = halfFloatType;
        info.fDefaultColorType = GrColorType::kGray_F16;

        if (lum16FSupported) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;

            if (texStorageSupported && lum16FSizedFormatSupported) {
                info.fFlags |= FormatInfo::kUseTexStorage_Flag;
                info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE16F;
            } else if (texImageSupportsSizedInternalFormat && lum16FSizedFormatSupported) {
                info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE16F;
            } else {
                info.fInternalFormatForTexImageOrStorage = GR_GL_LUMINANCE;
            }

            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: LUMINANCE16F, Surface: kAlpha_F16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kAlpha_F16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("000r");
                ctInfo.fWriteSwizzle = GrSwizzle("aaa0");

                int idx = static_cast<int>(GrColorType::kAlpha_F16);
                if (fColorTypeToFormatTable[idx] == GrGLFormat::kUnknown) {
                    this->setColorTypeFormat(GrColorType::kAlpha_F16, GrGLFormat::kLUMINANCE16F);
                }

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: LUMINANCE16F, Surface: kAlpha_F16, Data: kAlpha_F16
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kAlpha_F16;
                    ioFormat.fExternalType = halfFloatType;
                    ioFormat.fExternalTexImageFormat = GR_GL_LUMINANCE;
                    ioFormat.fExternalReadFormat = 0;
                }

                // Format: LUMINANCE16F, Surface: kAlpha_F16, Data: kRGBA_F32
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_F32;
                    ioFormat.fExternalType = GR_GL_FLOAT;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: RGB8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRGB8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_RGB8;
        info.fDefaultExternalFormat = GR_GL_RGB;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kRGB_888;
        info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
        if (GR_IS_GR_GL(standard)) {
            // Even in OpenGL 4.6 GL_RGB8 is required to be color renderable but not required to be
            // a supported render buffer format. Since we usually use render buffers for MSAA on
            // non-ES GL we don't support MSAA for GL_RGB8. On 4.2+ we could check using
            // glGetInternalFormativ(GL_RENDERBUFFER, GL_RGB8, GL_INTERNALFORMAT_SUPPORTED, ...) if
            // this becomes an issue.
            info.fFlags |= nonMSAARenderFlags;
        } else if (GR_IS_GR_GL_ES(standard)) {
            // 3.0 and the extension support this as a render buffer format.
            if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_OES_rgb8_rgba8")) {
                info.fFlags |= msaaRenderFlags;
            }
        } else if (GR_IS_GR_WEBGL(standard)) {
            // WebGL seems to support RBG8
            info.fFlags |= msaaRenderFlags;
        }
        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RGB8;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RGB8 : GR_GL_RGB;
        }

        info.fColorTypeInfoCount = 1;
        info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RGB8, Surface: kRGB_888x
        {
            auto& ctInfo = info.fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGB_888x;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            this->setColorTypeFormat(GrColorType::kRGB_888x, GrGLFormat::kRGB8);

            // External IO ColorTypes:
            ctInfo.fExternalIOFormatCount = 2;
            ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                    ctInfo.fExternalIOFormatCount);
            int ioIdx = 0;
            // Format: RGB8, Surface: kRGB_888x, Data: kRGB_888
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kRGB_888;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = GR_GL_RGB;
                ioFormat.fExternalReadFormat = 0;
            }

            // Format: RGB8, Surface: kRGB_888x, Data: kRGBA_8888
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kRGBA_8888;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = 0;
                ioFormat.fExternalReadFormat = GR_GL_RGBA;
            }
        }
    }

    // Format: RG8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRG8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_RG8;
        info.fDefaultExternalFormat = GR_GL_RG;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kRG_88;
        bool rg8Support = false;
        if (GR_IS_GR_GL(standard)) {
            rg8Support = version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_texture_rg");
        } else if (GR_IS_GR_GL_ES(standard)) {
            rg8Support = version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_EXT_texture_rg");
        } else if (GR_IS_GR_WEBGL(standard)) {
            rg8Support = version >= GR_GL_VER(2, 0);
        }
        if (rg8Support) {
            info.fFlags |= FormatInfo::kTexturable_Flag
                        |  FormatInfo::kTransfers_Flag
                        |  msaaRenderFlags;
            if (texStorageSupported) {
                info.fFlags |= FormatInfo::kUseTexStorage_Flag;
                info.fInternalFormatForTexImageOrStorage = GR_GL_RG8;
            }
        }
        if (!(info.fFlags & FormatInfo::kUseTexStorage_Flag)) {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RG8 : GR_GL_RG;
        }
        if (rg8Support) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: RG8, Surface: kRG_88
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRG_88;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kRG_88, GrGLFormat::kRG8);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: RG8, Surface: kRG_88, Data: kRG_88
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRG_88;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = GR_GL_RG;
                    ioFormat.fExternalReadFormat = 0;
                    if (GR_IS_GR_GL(standard) && !formatWorkarounds.fDisallowDirectRG8ReadPixels) {
                        ioFormat.fExternalReadFormat = GR_GL_RG;
                    }
                }

                // Format: RG8, Surface: kRG_88, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: RGB10_A2
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRGB10_A2);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_RGB10_A2;
        info.fDefaultExternalFormat = GR_GL_RGBA;
        info.fDefaultExternalType = GR_GL_UNSIGNED_INT_2_10_10_10_REV;
        info.fDefaultColorType = GrColorType::kRGBA_1010102;
        if (GR_IS_GR_GL(standard) ||
           (GR_IS_GR_GL_ES(standard) && version >= GR_GL_VER(3, 0))) {
            info.fFlags = FormatInfo::kTexturable_Flag
                        | FormatInfo::kTransfers_Flag
                        | msaaRenderFlags;
        } else if (GR_IS_GR_GL_ES(standard) &&
                   ctxInfo.hasExtension("GL_EXT_texture_type_2_10_10_10_REV")) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
        } // No WebGL support

        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RGB10_A2;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RGB10_A2 : GR_GL_RGBA;
        }

        if (SkToBool(info.fFlags & FormatInfo::kTexturable_Flag)) {
            bool supportsBGRAColorType = GR_IS_GR_GL(standard) &&
                    (version >= GR_GL_VER(1, 2) || ctxInfo.hasExtension("GL_EXT_bgra"));

            info.fColorTypeInfoCount = supportsBGRAColorType ? 2 : 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: RGB10_A2, Surface: kRGBA_1010102
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_1010102;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kRGBA_1010102, GrGLFormat::kRGB10_A2);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: RGB10_A2, Surface: kRGBA_1010102, Data: kRGBA_1010102
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_1010102;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_INT_2_10_10_10_REV;
                    ioFormat.fExternalTexImageFormat = GR_GL_RGBA;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: RGB10_A2, Surface: kRGBA_1010102, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
            //------------------------------------------------------------------
            // Format: RGB10_A2, Surface: kBGRA_1010102
            if (supportsBGRAColorType) {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kBGRA_1010102;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kBGRA_1010102, GrGLFormat::kRGB10_A2);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: RGB10_A2, Surface: kBGRA_1010102, Data: kBGRA_1010102
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kBGRA_1010102;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_INT_2_10_10_10_REV;
                    ioFormat.fExternalTexImageFormat = GR_GL_BGRA;
                    ioFormat.fExternalReadFormat =
                            formatWorkarounds.fDisallowBGRA8ReadPixels ? 0 : GR_GL_BGRA;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: RGB10_A2, Surface: kBGRA_1010102, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: RGBA4
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRGBA4);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_RGBA4;
        info.fDefaultExternalFormat = GR_GL_RGBA;
        info.fDefaultExternalType = GR_GL_UNSIGNED_SHORT_4_4_4_4;
        info.fDefaultColorType = GrColorType::kABGR_4444;
        info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
        if (GR_IS_GR_GL(standard)) {
            if (version >= GR_GL_VER(4, 2)) {
                info.fFlags |= msaaRenderFlags;
            }
        } else if (GR_IS_GR_GL_ES(standard)) {
            info.fFlags |= msaaRenderFlags;
        } else if (GR_IS_GR_WEBGL(standard)) {
            info.fFlags |= msaaRenderFlags;
        }
        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RGBA4;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RGBA4 : GR_GL_RGBA;
        }

        info.fColorTypeInfoCount = 1;
        info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RGBA4, Surface: kABGR_4444
        {
            auto& ctInfo = info.fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kABGR_4444;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            this->setColorTypeFormat(GrColorType::kABGR_4444, GrGLFormat::kRGBA4);

            // External IO ColorTypes:
            ctInfo.fExternalIOFormatCount = 2;
            ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                    ctInfo.fExternalIOFormatCount);
            int ioIdx = 0;
            // Format: RGBA4, Surface: kABGR_4444, Data: kABGR_4444
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kABGR_4444;
                ioFormat.fExternalType = GR_GL_UNSIGNED_SHORT_4_4_4_4;
                ioFormat.fExternalTexImageFormat = GR_GL_RGBA;
                ioFormat.fExternalReadFormat = GR_GL_RGBA;
                // Not guaranteed by ES/WebGL.
                ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
            }

            // Format: RGBA4, Surface: kABGR_4444, Data: kRGBA_8888
            {
                auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                ioFormat.fColorType = GrColorType::kRGBA_8888;
                ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                ioFormat.fExternalTexImageFormat = 0;
                ioFormat.fExternalReadFormat = GR_GL_RGBA;
            }
        }
    }

    // Format: SRGB8_ALPHA8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kSRGB8_ALPHA8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_SRGB8_ALPHA8;
        info.fDefaultExternalType = GR_GL_UNSIGNED_BYTE;
        info.fDefaultColorType = GrColorType::kRGBA_8888_SRGB;

        // We may modify the default external format below.
        info.fDefaultExternalFormat = GR_GL_RGBA;
        bool srgb8Alpha8TexStorageSupported = texStorageSupported;
        bool srgb8Alpha8TextureSupport = false;
        bool srgb8Alpha8RenderTargetSupport = false;
        if (GR_IS_GR_GL(standard)) {
            if (version >= GR_GL_VER(3, 0)) {
                srgb8Alpha8TextureSupport = true;
                srgb8Alpha8RenderTargetSupport = true;
            } else if (ctxInfo.hasExtension("GL_EXT_texture_sRGB")) {
                srgb8Alpha8TextureSupport = true;
                if (ctxInfo.hasExtension("GL_ARB_framebuffer_sRGB") ||
                    ctxInfo.hasExtension("GL_EXT_framebuffer_sRGB")) {
                    srgb8Alpha8RenderTargetSupport = true;
                }
            }
        } else if (GR_IS_GR_GL_ES(standard)) {
            if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_EXT_sRGB")) {
                srgb8Alpha8TextureSupport = true;
                srgb8Alpha8RenderTargetSupport = true;
            }
            if (version < GR_GL_VER(3, 0)) {
                // ES 2.0 requires that the external format matches the internal format.
                info.fDefaultExternalFormat = GR_GL_SRGB_ALPHA;
                // There is no defined interaction between GL_EXT_sRGB and GL_EXT_texture_storage.
                srgb8Alpha8TexStorageSupported = false;
            }
        } else if (GR_IS_GR_WEBGL(standard)) {
            // sRGB extension should be on most WebGL 1.0 contexts, although sometimes under 2
            // names.
            if (version >= GR_GL_VER(2, 0) || ctxInfo.hasExtension("GL_EXT_sRGB") ||
                ctxInfo.hasExtension("EXT_sRGB")) {
                srgb8Alpha8TextureSupport = true;
                srgb8Alpha8RenderTargetSupport = true;
            }
            if (version < GR_GL_VER(2, 0)) {
                // WebGL 1.0 requires that the external format matches the internal format.
                info.fDefaultExternalFormat = GR_GL_SRGB_ALPHA;
                // There is no extension to WebGL 1 that adds glTexStorage.
                SkASSERT(!srgb8Alpha8TexStorageSupported);
            }
        }

        if (srgb8Alpha8TextureSupport) {
            info.fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
            if (srgb8Alpha8RenderTargetSupport) {
                info.fFlags |= formatWorkarounds.fDisableSRGBRenderWithMSAAForMacAMD
                                       ? nonMSAARenderFlags
                                       : msaaRenderFlags;
            }
        }
        if (srgb8Alpha8TexStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_SRGB8_ALPHA8;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_SRGB8_ALPHA8 : GR_GL_SRGB_ALPHA;
        }

        if (srgb8Alpha8TextureSupport) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: SRGB8_ALPHA8, Surface: kRGBA_8888_SRGB
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_8888_SRGB;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kRGBA_8888_SRGB, GrGLFormat::kSRGB8_ALPHA8);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 1;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;

                // Format: SRGB8_ALPHA8, Surface: kRGBA_8888_SRGB, Data: kRGBA_8888_SRGB
                {
                    // GL does not do srgb<->rgb conversions when transferring between cpu and gpu.
                    // Thus, the external format is GL_RGBA. See below for note about ES2.0 and
                    // glTex[Sub]Image.
                    GrGLenum texImageExternalFormat = GR_GL_RGBA;

                    // OpenGL ES 2.0 + GL_EXT_sRGB allows GL_SRGB_ALPHA to be specified as the
                    // <format> param to Tex(Sub)Image. ES 2.0 requires the <internalFormat> and
                    // <format> params to match. Thus, on ES 2.0 we will use GL_SRGB_ALPHA as the
                    // <format> param. On OpenGL and ES 3.0+ GL_SRGB_ALPHA does not work for the
                    // <format> param to glTexImage.
                    if (GR_IS_GR_GL_ES(standard) && version == GR_GL_VER(2,0)) {
                        texImageExternalFormat = GR_GL_SRGB_ALPHA;
                    }
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888_SRGB;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = texImageExternalFormat;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: COMPRESSED_RGB8_BC1
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kCOMPRESSED_RGB8_BC1);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForTexImageOrStorage = GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        if (GR_IS_GR_GL(standard) || GR_IS_GR_GL_ES(standard)) {
            if (ctxInfo.hasExtension("GL_EXT_texture_compression_s3tc")) {
                info.fFlags = FormatInfo::kTexturable_Flag;
            }
        } // No WebGL support

        // There are no support GrColorTypes for this format
    }

    // Format: COMPRESSED_RGBA8_BC1
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kCOMPRESSED_RGBA8_BC1);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForTexImageOrStorage = GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        if (GR_IS_GR_GL(standard) || GR_IS_GR_GL_ES(standard)) {
            if (ctxInfo.hasExtension("GL_EXT_texture_compression_s3tc")) {
                info.fFlags = FormatInfo::kTexturable_Flag;
            }
        } // No WebGL support

          // There are no support GrColorTypes for this format
    }

    // Format: COMPRESSED_RGB8_ETC2
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kCOMPRESSED_RGB8_ETC2);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForTexImageOrStorage = GR_GL_COMPRESSED_RGB8_ETC2;
        if (!formatWorkarounds.fDisallowETC2Compression) {
            if (GR_IS_GR_GL(standard)) {
                if (version >= GR_GL_VER(4, 3) ||
                    ctxInfo.hasExtension("GL_ARB_ES3_compatibility")) {
                    info.fFlags = FormatInfo::kTexturable_Flag;
                }
            } else if (GR_IS_GR_GL_ES(standard)) {
                if (version >= GR_GL_VER(3, 0) ||
                    ctxInfo.hasExtension("GL_OES_compressed_ETC2_RGB8_texture")) {
                    info.fFlags = FormatInfo::kTexturable_Flag;
                }
            } // No WebGL support
        }

        // There are no support GrColorTypes for this format
    }

    // Format: COMPRESSED_ETC1_RGB8
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kCOMPRESSED_ETC1_RGB8);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForTexImageOrStorage = GR_GL_COMPRESSED_ETC1_RGB8;
        if (GR_IS_GR_GL_ES(standard)) {
            if (ctxInfo.hasExtension("GL_OES_compressed_ETC1_RGB8_texture")) {
                info.fFlags = FormatInfo::kTexturable_Flag;
            }
        } // No GL or WebGL support

        // There are no support GrColorTypes for this format
    }

    // Format: R16
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kR16);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForRenderbuffer = GR_GL_R16;
        info.fDefaultExternalFormat = GR_GL_RED;
        info.fDefaultExternalType = GR_GL_UNSIGNED_SHORT;
        info.fDefaultColorType = GrColorType::kR_16;
        bool r16Supported = false;
        if (!formatWorkarounds.fDisallowTextureUnorm16) {
            if (GR_IS_GR_GL(standard)) {
                r16Supported = version >= GR_GL_VER(3, 0) ||
                               ctxInfo.hasExtension("GL_ARB_texture_rg");
            } else if (GR_IS_GR_GL_ES(standard)) {
                r16Supported = ctxInfo.hasExtension("GL_EXT_texture_norm16");
            }  // No WebGL support
        }

        if (r16Supported) {
            info.fFlags = FormatInfo::kTexturable_Flag | msaaRenderFlags;
            if (!formatWorkarounds.fDisallowUnorm16Transfers) {
                info.fFlags |= FormatInfo::kTransfers_Flag;
            }
        }

        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_R16;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_R16 : GR_GL_RED;
        }

        if (r16Supported) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: R16, Surface: kAlpha_16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kAlpha_16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("000r");
                ctInfo.fWriteSwizzle = GrSwizzle("a000");
                this->setColorTypeFormat(GrColorType::kAlpha_16, GrGLFormat::kR16);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: R16, Surface: kAlpha_16, Data: kAlpha_16
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kAlpha_16;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_SHORT;
                    ioFormat.fExternalTexImageFormat = GR_GL_RED;
                    ioFormat.fExternalReadFormat = GR_GL_RED;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: R16, Surface: kAlpha_16, Data: kAlpha_8xxx
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kAlpha_8xxx;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: RG16
    {
        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRG16);
        info.fFormatType = FormatType::kNormalizedFixedPoint;
        info.fInternalFormatForTexImageOrStorage =
                texImageSupportsSizedInternalFormat ? GR_GL_RG16 : GR_GL_RG;
        info.fInternalFormatForRenderbuffer = GR_GL_RG16;
        info.fDefaultExternalFormat = GR_GL_RG;
        info.fDefaultExternalType = GR_GL_UNSIGNED_SHORT;
        info.fDefaultColorType = GrColorType::kRG_1616;
        bool rg16Supported = false;
        if (!formatWorkarounds.fDisallowTextureUnorm16) {
            if (GR_IS_GR_GL(standard)) {
                rg16Supported = version >= GR_GL_VER(3, 0) ||
                                ctxInfo.hasExtension("GL_ARB_texture_rg");
            } else if (GR_IS_GR_GL_ES(standard)) {
                rg16Supported = ctxInfo.hasExtension("GL_EXT_texture_norm16");
            }  // No WebGL support
        }

        if (rg16Supported) {
            info.fFlags = FormatInfo::kTexturable_Flag | msaaRenderFlags;
            if (!formatWorkarounds.fDisallowUnorm16Transfers) {
                info.fFlags |= FormatInfo::kTransfers_Flag;
            }
        }

        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RG16;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RG16 : GR_GL_RG;
        }

        if (rg16Supported) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: GR_GL_RG16, Surface: kRG_1616
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRG_1616;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kRG_1616, GrGLFormat::kRG16);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: GR_GL_RG16, Surface: kRG_1616, Data: kRG_1616
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRG_1616;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_SHORT;
                    ioFormat.fExternalTexImageFormat = GR_GL_RG;
                    ioFormat.fExternalReadFormat = GR_GL_RG;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: GR_GL_RG16, Surface: kRG_1616, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format: RGBA16
    {
        bool rgba16Support = false;
        if (!formatWorkarounds.fDisallowTextureUnorm16) {
            if (GR_IS_GR_GL(standard)) {
                rgba16Support = version >= GR_GL_VER(3, 0);
            } else if (GR_IS_GR_GL_ES(standard)) {
                rgba16Support = ctxInfo.hasExtension("GL_EXT_texture_norm16");
            }  // No WebGL support
        }

        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRGBA16);
        info.fFormatType = FormatType::kNormalizedFixedPoint;

        info.fInternalFormatForRenderbuffer = GR_GL_RGBA16;
        info.fDefaultExternalFormat = GR_GL_RGBA;
        info.fDefaultExternalType = GR_GL_UNSIGNED_SHORT;
        info.fDefaultColorType = GrColorType::kRGBA_16161616;
        if (rgba16Support) {
            info.fFlags = FormatInfo::kTexturable_Flag | msaaRenderFlags;
            if (!formatWorkarounds.fDisallowUnorm16Transfers) {
                info.fFlags |= FormatInfo::kTransfers_Flag;
            }
        }

        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RGBA16;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RGBA16 : GR_GL_RGBA;
        }

        if (rgba16Support) {
            // Format: GR_GL_RGBA16, Surface: kRGBA_16161616
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_16161616;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kRGBA_16161616, GrGLFormat::kRGBA16);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: GR_GL_RGBA16, Surface: kRGBA_16161616, Data: kRGBA_16161616
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_16161616;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_SHORT;
                    ioFormat.fExternalTexImageFormat = GR_GL_RGBA;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: GR_GL_RGBA16, Surface: kRGBA_16161616, Data: kRGBA_8888
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_8888;
                    ioFormat.fExternalType = GR_GL_UNSIGNED_BYTE;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    // Format:RG16F
    {
        bool rg16FTextureSupport = false;
        bool rg16FRenderTargetSupport = false;
        if (GR_IS_GR_GL(standard)) {
            if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_texture_float")) {
                rg16FTextureSupport = true;
                rg16FRenderTargetSupport = true;
            }
        } else if (GR_IS_GR_GL_ES(standard)) {
            // It seems possible that a combination of GL_EXT_texture_rg and
            // GL_EXT_color_buffer_half_float might add this format to ES 2.0 but it is not entirely
            // clear. The latter mentions interaction but that may only be for renderbuffers as
            // neither adds the texture format explicitly.
            // GL_OES_texture_format_half_float makes no reference to RG formats.
            if (version >= GR_GL_VER(3, 0)) {
                rg16FTextureSupport = true;
                rg16FRenderTargetSupport = version >= GR_GL_VER(3, 2) ||
                                           ctxInfo.hasExtension("GL_EXT_color_buffer_float") ||
                                           ctxInfo.hasExtension("GL_EXT_color_buffer_half_float");
            }
        } else if (GR_IS_GR_WEBGL(standard)) {
            if (version >= GR_GL_VER(2, 0)) {
                rg16FTextureSupport = true;
                rg16FRenderTargetSupport = ctxInfo.hasExtension("GL_EXT_color_buffer_half_float") ||
                                           ctxInfo.hasExtension("EXT_color_buffer_half_float") ||
                                           ctxInfo.hasExtension("GL_EXT_color_buffer_float") ||
                                           ctxInfo.hasExtension("EXT_color_buffer_float");
            }
        }

        FormatInfo& info = this->getFormatInfo(GrGLFormat::kRG16F);
        info.fFormatType = FormatType::kFloat;
        info.fInternalFormatForRenderbuffer = GR_GL_RG16F;
        info.fDefaultExternalFormat = GR_GL_RG;
        info.fDefaultExternalType = halfFloatType;
        info.fDefaultColorType = GrColorType::kRG_F16;
        if (rg16FTextureSupport) {
            info.fFlags |= FormatInfo::kTexturable_Flag | FormatInfo::kTransfers_Flag;
            if (rg16FRenderTargetSupport) {
                info.fFlags |= fpRenderFlags;
            }
        }

        if (texStorageSupported) {
            info.fFlags |= FormatInfo::kUseTexStorage_Flag;
            info.fInternalFormatForTexImageOrStorage = GR_GL_RG16F;
        } else {
            info.fInternalFormatForTexImageOrStorage =
                    texImageSupportsSizedInternalFormat ? GR_GL_RG16F : GR_GL_RG;
        }

        if (rg16FTextureSupport) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: GR_GL_RG16F, Surface: kRG_F16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRG_F16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                this->setColorTypeFormat(GrColorType::kRG_F16, GrGLFormat::kRG16F);

                // External IO ColorTypes:
                ctInfo.fExternalIOFormatCount = 2;
                ctInfo.fExternalIOFormats = std::make_unique<ColorTypeInfo::ExternalIOFormats[]>(
                        ctInfo.fExternalIOFormatCount);
                int ioIdx = 0;
                // Format: GR_GL_RG16F, Surface: kRG_F16, Data: kRG_F16
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRG_F16;
                    ioFormat.fExternalType = halfFloatType;
                    ioFormat.fExternalTexImageFormat = GR_GL_RG;
                    ioFormat.fExternalReadFormat = GR_GL_RG;
                    // Not guaranteed by ES/WebGL.
                    ioFormat.fRequiresImplementationReadQuery = !GR_IS_GR_GL(standard);
                }

                // Format: GR_GL_RG16F, Surface: kRG_F16, Data: kRGBA_F32
                {
                    auto& ioFormat = ctInfo.fExternalIOFormats[ioIdx++];
                    ioFormat.fColorType = GrColorType::kRGBA_F32;
                    ioFormat.fExternalType = GR_GL_FLOAT;
                    ioFormat.fExternalTexImageFormat = 0;
                    ioFormat.fExternalReadFormat = GR_GL_RGBA;
                }
            }
        }
    }

    this->setupSampleCounts(ctxInfo, gli);

#ifdef SK_DEBUG
    for (int i = 0; i < kGrGLColorFormatCount; ++i) {
        if (GrGLFormat::kUnknown == static_cast<GrGLFormat>(i)) {
            continue;
        }
        const auto& formatInfo = fFormatTable[i];
        // Make sure we didn't set fbo attachable with msaa and not fbo attachable.
        SkASSERT(!((formatInfo.fFlags & FormatInfo::kFBOColorAttachmentWithMSAA_Flag) &&
                  !(formatInfo.fFlags & FormatInfo::kFBOColorAttachment_Flag)));

        // Make sure we set all the formats' FormatType
        SkASSERT(formatInfo.fFormatType != FormatType::kUnknown);

        // Make sure if we added a ColorTypeInfo we filled it out
        for (int j = 0; j < formatInfo.fColorTypeInfoCount; ++j) {
            const auto& ctInfo = formatInfo.fColorTypeInfos[j];
            SkASSERT(ctInfo.fColorType != GrColorType::kUnknown);
            // Seems silly to add a color type if we don't support any flags on it.
            SkASSERT(ctInfo.fFlags);
            // Make sure if we added any ExternalIOFormats we filled it out
            for (int k = 0; k < ctInfo.fExternalIOFormatCount; ++k) {
                const auto& ioInfo = ctInfo.fExternalIOFormats[k];
                SkASSERT(ioInfo.fColorType != GrColorType::kUnknown);
            }
        }
    }
#endif
}

void GrGLCaps::setupSampleCounts(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli) {
    GrGLStandard standard = ctxInfo.standard();
    // standard can be unused (optimized away) if SK_ASSUME_GL_ES is set
    sk_ignore_unused_variable(standard);
    GrGLVersion version = ctxInfo.version();

    for (int i = 0; i < kGrGLColorFormatCount; ++i) {
        if (FormatInfo::kFBOColorAttachmentWithMSAA_Flag & fFormatTable[i].fFlags) {
            // We assume that MSAA rendering is supported only if we support non-MSAA rendering.
            SkASSERT(FormatInfo::kFBOColorAttachment_Flag & fFormatTable[i].fFlags);
            if ((GR_IS_GR_GL(standard) &&
                  (version >= GR_GL_VER(4,2) ||
                   ctxInfo.hasExtension("GL_ARB_internalformat_query"))) ||
                (GR_IS_GR_GL_ES(standard) && version >= GR_GL_VER(3,0))) {
                int count;
                GrGLFormat grGLFormat = static_cast<GrGLFormat>(i);
                GrGLenum glFormat = this->getRenderbufferInternalFormat(grGLFormat);
                GR_GL_GetInternalformativ(gli, GR_GL_RENDERBUFFER, glFormat,
                                          GR_GL_NUM_SAMPLE_COUNTS, 1, &count);
                if (count) {
                    std::unique_ptr<int[]> temp(new int[count]);
                    GR_GL_GetInternalformativ(gli, GR_GL_RENDERBUFFER, glFormat, GR_GL_SAMPLES,
                                              count, temp.get());
                    // GL has a concept of MSAA rasterization with a single sample but we do not.
                    if (count && temp[count - 1] == 1) {
                        --count;
                        SkASSERT(!count || temp[count -1] > 1);
                    }
                    fFormatTable[i].fColorSampleCounts.setCount(count+1);
                    // We initialize our supported values with 1 (no msaa) and reverse the order
                    // returned by GL so that the array is ascending.
                    fFormatTable[i].fColorSampleCounts[0] = 1;
                    for (int j = 0; j < count; ++j) {
#if defined(SK_BUILD_FOR_IOS) && TARGET_OS_SIMULATOR
                        // The iOS simulator is reporting incorrect values for sample counts,
                        // so force them to be a power of 2.
                        fFormatTable[i].fColorSampleCounts[j+1] = SkPrevPow2(temp[count - j - 1]);
#else
                        fFormatTable[i].fColorSampleCounts[j+1] = temp[count - j - 1];
#endif
                    }
                }
            } else {
                // Fake out the table using some semi-standard counts up to the max allowed sample
                // count.
                int maxSampleCnt = 1;
                if (GrGLCaps::kES_IMG_MsToTexture_MSFBOType == fMSFBOType) {
                    GR_GL_GetIntegerv(gli, GR_GL_MAX_SAMPLES_IMG, &maxSampleCnt);
                } else if (GrGLCaps::kNone_MSFBOType != fMSFBOType) {
                    GR_GL_GetIntegerv(gli, GR_GL_MAX_SAMPLES, &maxSampleCnt);
                }
                // Chrome has a mock GL implementation that returns 0.
                maxSampleCnt = std::max(1, maxSampleCnt);

                static constexpr int kDefaultSamples[] = {1, 2, 4, 8};
                int count = SK_ARRAY_COUNT(kDefaultSamples);
                for (; count > 0; --count) {
                    if (kDefaultSamples[count - 1] <= maxSampleCnt) {
                        break;
                    }
                }
                if (count > 0) {
                    fFormatTable[i].fColorSampleCounts.append(count, kDefaultSamples);
                }
            }
        } else if (FormatInfo::kFBOColorAttachment_Flag & fFormatTable[i].fFlags) {
            fFormatTable[i].fColorSampleCounts.setCount(1);
            fFormatTable[i].fColorSampleCounts[0] = 1;
        }
    }
}

bool GrGLCaps::canCopyTexSubImage(GrGLFormat dstFormat, bool dstHasMSAARenderBuffer,
                                  const GrTextureType* dstTypeIfTexture,
                                  GrGLFormat srcFormat, bool srcHasMSAARenderBuffer,
                                  const GrTextureType* srcTypeIfTexture) const {
    // When it comes to format types and component sizes the gl spec is fairly complex as
    // requirements differ depending on many properties (e.g. if the internalFormat was created with
    // a sized format or not). These affect the rules about which format types can be copied to
    // which other types. For now we are being more restrictive and requiring that the types must
    // match exactly.
    if (this->getFormatDefaultExternalType(dstFormat) !=
        this->getFormatDefaultExternalType(srcFormat)) {
        return false;
    }

    // Either both the src and dst formats need to be SRGB or both need to not be SRGB
    if (GrGLFormatIsSRGB(dstFormat) != GrGLFormatIsSRGB(srcFormat)) {
        return false;
    }

    if (GR_IS_GR_GL_ES(fStandard)) {
        // Table 3.9 of the ES2 spec indicates the supported formats with CopyTexSubImage
        // and BGRA isn't in the spec. There doesn't appear to be any extension that adds it.
        // ANGLE, for one, does not allow it. However, we've found it works on some drivers and
        // avoids bugs with using glBlitFramebuffer.
        if ((dstFormat == GrGLFormat::kBGRA8 || srcFormat == GrGLFormat::kBGRA8) &&
            !fAllowBGRA8CopyTexSubImage) {
            return false;
        }

        // Table 3.9 of the ES2 spec and 3.16 of ES3 spec indicates the supported internal base
        // formats with CopyTexSubImage. Each base format can be copied to itself or formats with
        // less channels.
        uint32_t dstChannels = GrGLFormatChannels(dstFormat);
        uint32_t srcChannels = GrGLFormatChannels(srcFormat);
        if (!dstChannels || !srcChannels) {
            // The formats don't represent color channels (i.e. may be depth stencil)
            return false;
        }
        // The dst channels have to be a subset of the srcChannels, except R, RG, or RGB, channels
        // can go to LUM. (See expansion of Table 3.9 in EXT_texture_rg).
        if ((dstChannels & srcChannels) != srcChannels) {
            if (dstChannels == kGray_SkColorChannelFlag ||
                dstChannels == kGrayAlpha_SkColorChannelFlags) {
                // The dst can't have gray if the src is alpha-only.
                if (srcChannels == kAlpha_SkColorChannelFlag) {
                    return false;
                }
            } else {
                return false;
            }
        }
    }

    // CopyTexSubImage is invalid or doesn't copy what we want when we have msaa render buffers.
    if (dstHasMSAARenderBuffer || srcHasMSAARenderBuffer) {
        return false;
    }

    // CopyTex(Sub)Image writes to a texture and we have no way of dynamically wrapping a RT in a
    // texture.
    if (!dstTypeIfTexture) {
        return false;
    }

    // Check that we could wrap the source in an FBO, that the dst is not TEXTURE_EXTERNAL, that no
    // mirroring is required
    return this->canFormatBeFBOColorAttachment(srcFormat) &&
           (!srcTypeIfTexture || *srcTypeIfTexture != GrTextureType::kExternal) &&
           *dstTypeIfTexture != GrTextureType::kExternal;
}

bool GrGLCaps::canCopyAsBlit(GrGLFormat dstFormat, int dstSampleCnt,
                             const GrTextureType* dstTypeIfTexture,
                             GrGLFormat srcFormat, int srcSampleCnt,
                             const GrTextureType* srcTypeIfTexture,
                             const SkRect& srcBounds, bool srcBoundsExact,
                             const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    auto blitFramebufferFlags = fBlitFramebufferFlags;
    if (!this->canFormatBeFBOColorAttachment(dstFormat) ||
        !this->canFormatBeFBOColorAttachment(srcFormat)) {
        return false;
    }

    if (dstTypeIfTexture && *dstTypeIfTexture == GrTextureType::kExternal) {
        return false;
    }
    if (srcTypeIfTexture && *srcTypeIfTexture == GrTextureType::kExternal) {
        return false;
    }

    if (GrGLCaps::kNoSupport_BlitFramebufferFlag & blitFramebufferFlags) {
        return false;
    }

    if (GrGLCaps::kResolveMustBeFull_BlitFrambufferFlag & blitFramebufferFlags) {
        if (srcSampleCnt > 1) {
            if (1 == dstSampleCnt) {
                return false;
            }
            if (SkRect::Make(srcRect) != srcBounds || !srcBoundsExact) {
                return false;
            }
        }
    }

    if (GrGLCaps::kNoMSAADst_BlitFramebufferFlag & blitFramebufferFlags) {
        if (dstSampleCnt > 1) {
            return false;
        }
    }

    if (GrGLCaps::kNoFormatConversion_BlitFramebufferFlag & blitFramebufferFlags) {
        if (srcFormat != dstFormat) {
            return false;
        }
    } else if (GrGLCaps::kNoFormatConversionForMSAASrc_BlitFramebufferFlag & blitFramebufferFlags) {
        if (srcSampleCnt > 1 && srcFormat != dstFormat) {
            return false;
        }
    }

    if (GrGLCaps::kRectsMustMatchForMSAASrc_BlitFramebufferFlag & blitFramebufferFlags) {
        if (srcSampleCnt > 1) {
            if (dstPoint.fX != srcRect.fLeft || dstPoint.fY != srcRect.fTop) {
                return false;
            }
        }
    }
    return true;
}

bool GrGLCaps::canCopyAsDraw(GrGLFormat dstFormat, bool srcIsTexturable) const {
    return this->isFormatRenderable(dstFormat, 1) && srcIsTexturable;
}

static bool has_msaa_render_buffer(const GrSurfaceProxy* surf, const GrGLCaps& glCaps) {
    const GrRenderTargetProxy* rt = surf->asRenderTargetProxy();
    if (!rt) {
        return false;
    }
    // A RT has a separate MSAA renderbuffer if:
    // 1) It's multisampled
    // 2) We're using an extension with separate MSAA renderbuffers
    // 3) It's not FBO 0, which is special and always auto-resolves
    return rt->numSamples() > 1 &&
           glCaps.usesMSAARenderBuffers() &&
           !rt->glRTFBOIDIs0();
}

bool GrGLCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    int dstSampleCnt = 0;
    int srcSampleCnt = 0;
    if (const GrRenderTargetProxy* rtProxy = dst->asRenderTargetProxy()) {
        dstSampleCnt = rtProxy->numSamples();
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        srcSampleCnt = rtProxy->numSamples();
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTargetProxy()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTargetProxy()));

    const GrTextureProxy* dstTex = dst->asTextureProxy();
    const GrTextureProxy* srcTex = src->asTextureProxy();

    GrTextureType dstTexType;
    GrTextureType* dstTexTypePtr = nullptr;
    GrTextureType srcTexType;
    GrTextureType* srcTexTypePtr = nullptr;
    if (dstTex) {
        dstTexType = dstTex->textureType();
        dstTexTypePtr = &dstTexType;
    }
    if (srcTex) {
        srcTexType = srcTex->textureType();
        srcTexTypePtr = &srcTexType;
    }

    auto dstFormat = dst->backendFormat().asGLFormat();
    auto srcFormat = src->backendFormat().asGLFormat();
    return this->canCopyTexSubImage(dstFormat, has_msaa_render_buffer(dst, *this), dstTexTypePtr,
                                    srcFormat, has_msaa_render_buffer(src, *this), srcTexTypePtr) ||
           this->canCopyAsBlit(dstFormat, dstSampleCnt, dstTexTypePtr, srcFormat, srcSampleCnt,
                               srcTexTypePtr, src->getBoundsRect(), src->priv().isExact(), srcRect,
                               dstPoint) ||
           this->canCopyAsDraw(dstFormat, SkToBool(srcTex));
}

GrCaps::DstCopyRestrictions GrGLCaps::getDstCopyRestrictions(const GrRenderTargetProxy* src,
                                                             GrColorType colorType) const {
    // If the src is a texture, we can implement the blit as a draw assuming the config is
    // renderable.
    if (src->asTextureProxy() && !this->isFormatAsColorTypeRenderable(colorType,
                                                                      src->backendFormat())) {
        return {};
    }

    if (const auto* texProxy = src->asTextureProxy()) {
        if (texProxy->textureType() == GrTextureType::kExternal) {
            // Not supported for FBO blit or CopyTexSubImage. Caller will have to fall back to a
            // draw (if the source is also a texture).
            return {};
        }
    }

    // We look for opportunities to use CopyTexSubImage, or fbo blit. If neither are
    // possible and we return false to fallback to creating a render target dst for render-to-
    // texture. This code prefers CopyTexSubImage to fbo blit and avoids triggering temporary fbo
    // creation. It isn't clear that avoiding temporary fbo creation is actually optimal.
    DstCopyRestrictions blitFramebufferRestrictions = {};
    if (src->numSamples() > 1 &&
        (fBlitFramebufferFlags & kResolveMustBeFull_BlitFrambufferFlag)) {
        blitFramebufferRestrictions.fRectsMustMatch = GrSurfaceProxy::RectsMustMatch::kYes;
        blitFramebufferRestrictions.fMustCopyWholeSrc = true;
        // Mirroring causes rects to mismatch later, don't allow it.
    } else if (src->numSamples() > 1 && (fBlitFramebufferFlags &
                                         kRectsMustMatchForMSAASrc_BlitFramebufferFlag)) {
        blitFramebufferRestrictions.fRectsMustMatch = GrSurfaceProxy::RectsMustMatch::kYes;
    }

    auto srcFormat = src->backendFormat().asGLFormat();
    // Check for format issues with glCopyTexSubImage2D
    if (srcFormat == GrGLFormat::kBGRA8) {
        // glCopyTexSubImage2D doesn't work with this config. If the bgra can be used with fbo blit
        // then we set up for that, otherwise fail.
        if (this->canFormatBeFBOColorAttachment(srcFormat)) {
            return blitFramebufferRestrictions;
        }
        // Caller will have to use a draw.
        return {};
    }

    {
        bool srcIsMSAARenderbuffer = src->numSamples() > 1 &&
                                     this->usesMSAARenderBuffers();
        if (srcIsMSAARenderbuffer) {
            // It's illegal to call CopyTexSubImage2D on a MSAA renderbuffer. Set up for FBO
            // blit or fail.
            if (this->canFormatBeFBOColorAttachment(srcFormat)) {
                return blitFramebufferRestrictions;
            }
            // Caller will have to use a draw.
            return {};
        }
    }

    // We'll do a CopyTexSubImage, no restrictions.
    return {};
}

void GrGLCaps::applyDriverCorrectnessWorkarounds(const GrGLContextInfo& ctxInfo,
                                                 const GrContextOptions& contextOptions,
                                                 const GrGLInterface* glInterface,
                                                 GrShaderCaps* shaderCaps,
                                                 FormatWorkarounds* formatWorkarounds) {
    // A driver bug on the nexus 6 causes incorrect dst copies when invalidate is called beforehand.
    // Thus we are disabling this extension for now on Adreno4xx devices.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno430       ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno4xx_other ||
        fDriverBugWorkarounds.disable_discard_framebuffer) {
        fInvalidateFBType = kNone_InvalidateFBType;
    }

    if (ctxInfo.renderer() == GrGLRenderer::kIntelCherryView) {
        // When running DMSAA_dst_read_with_existing_barrier with DMSAA disabled on linux Intel
        // HD405, the test fails when using texture barriers. Oddly the gpu doing the draw which
        // uses the barrier correctly. It is the next draw, which does not use or need a barrier,
        // that is blending with a dst as if the barrier draw didn't happen. Since this GPU is not
        // that important to us and this driver bug could probably manifest itself in the wild, we
        // are just disabling texture barrier support for the gpu.
        fTextureBarrierSupport = false;
    }

    // glClearTexImage seems to have a bug in NVIDIA drivers that was fixed sometime between
    // 340.96 and 367.57.
    if (GR_IS_GR_GL(ctxInfo.standard()) && ctxInfo.driver() == GrGLDriver::kNVIDIA &&
        ctxInfo.driverVersion() < GR_GL_DRIVER_VER(367, 57, 0)) {
        fClearTextureSupport = false;
    }

    // glBlitFramebuffer seems to produce incorrect results on QC, Mali400, and Tegra3 but
    // glCopyTexSubImage2D works (even though there is no extension that specifically allows it).
    if (ctxInfo.vendor()   == GrGLVendor::kQualcomm  ||
        ctxInfo.renderer() == GrGLRenderer::kMali4xx ||
        ctxInfo.renderer() == GrGLRenderer::kTegra_PreK1) {
        fAllowBGRA8CopyTexSubImage = true;
    }

    // http://anglebug.com/6030
    if (fMSFBOType == kES_EXT_MsToTexture_MSFBOType &&
        ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D11) {
        fDisallowDynamicMSAA = true;
    }

    // http://skbug.com/12081
    if (GR_IS_GR_WEBGL(ctxInfo.standard())) {
        fDisallowDynamicMSAA = true;
    }

#if defined(__has_feature)
#if defined(SK_BUILD_FOR_MAC) && __has_feature(thread_sanitizer)
    // See skbug.com/7058
    fMapBufferType = kNone_MapBufferType;
    fMapBufferFlags = kNone_MapFlags;
    fTransferFromBufferToTextureSupport = false;
    fTransferFromSurfaceToBufferSupport = false;
    fTransferBufferType = TransferBufferType::kNone;
#endif
#endif

    // We found that the Galaxy J5 with an Adreno 306 running 6.0.1 has a bug where
    // GL_INVALID_OPERATION thrown by glDrawArrays when using a buffer that was mapped. The same bug
    // did not reproduce on a Nexus7 2013 with a 320 running Android M with driver 127.0. It's
    // unclear whether this really affects a wide range of devices.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno3xx &&
        ctxInfo.driverVersion() > GR_GL_DRIVER_VER(127, 0, 0)) {
        fMapBufferType = kNone_MapBufferType;
        fMapBufferFlags = kNone_MapFlags;
        fTransferFromBufferToTextureSupport = false;
        fTransferFromSurfaceToBufferSupport = false;
        fTransferBufferType = TransferBufferType::kNone;
    }

    // The TransferPixelsToTexture test fails on ANGLE D3D9 and D3D11 if this is enabled.
    // https://anglebug.com/5542
    if (ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D9 ||
        ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D11) {
        fTransferPixelsToRowBytesSupport = false;
    }

    // Using MIPs on this GPU seems to be a source of trouble.
    if (ctxInfo.renderer() == GrGLRenderer::kPowerVR54x) {
        fMipmapSupport = false;
    }

#ifdef SK_BUILD_FOR_ANDROID
    if (ctxInfo.renderer() == GrGLRenderer::kPowerVR54x) {
        // Flutter found glTexSubImage2D for GL_RED is much slower than GL_ALPHA on the
        // "MC18 PERSONAL SHOPPER"
        formatWorkarounds->fDisallowR8ForPowerVRSGX54x = true;
    }
#endif

    if (ctxInfo.isOverCommandBuffer() && ctxInfo.version() >= GR_GL_VER(3,0)) {
        formatWorkarounds->fDisallowTextureUnorm16 = true;  // http://crbug.com/1224108
        formatWorkarounds->fDisallowETC2Compression = true;  // http://crbug.com/1224111
        fTransferFromSurfaceToBufferSupport = false;  // http://crbug.com/1224138

        // http://crbug.com/1224117
        fMapBufferFlags = kNone_MapFlags;
        fMapBufferType = kNone_MapBufferType;
    }

    // https://b.corp.google.com/issues/143074513
    // https://skbug.com/11152
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno615 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno620) {
        fMSFBOType = kNone_MSFBOType;
        fMSAAResolvesAutomatically = false;
    }

#ifndef SK_BUILD_FOR_IOS
    if (ctxInfo.renderer() == GrGLRenderer::kPowerVR54x   ||
        ctxInfo.renderer() == GrGLRenderer::kPowerVRRogue ||
        (ctxInfo.renderer() == GrGLRenderer::kAdreno3xx && !ctxInfo.isOverCommandBuffer())) {
        fPerformColorClearsAsDraws = true;
    }
#endif

    // A lot of GPUs have trouble with full screen clears (skbug.com/7195)
    if (ctxInfo.renderer() == GrGLRenderer::kAMDRadeonHD7xxx ||
        ctxInfo.renderer() == GrGLRenderer::kAMDRadeonR9M4xx) {
        fPerformColorClearsAsDraws = true;
    }

#ifdef SK_BUILD_FOR_MAC
    // crbug.com/768134 - On MacBook Pros, the Intel Iris Pro doesn't always perform
    // full screen clears
    // crbug.com/773107 - On MacBook Pros, a wide range of Intel GPUs don't always
    // perform full screen clears.
    // Update on 4/4/2018 - This appears to be fixed on driver 10.30.12 on a macOS 10.13.2 on a
    // Retina MBP Early 2015 with Iris 6100. It is possibly fixed on earlier drivers as well.
    // crbug.com/1039912 - Crash rate in glClear spiked after OS update, affecting mostly
    //   Broadwell on 10.13+
    if (ctxInfo.vendor() == GrGLVendor::kIntel &&
        (ctxInfo.driverVersion() < GR_GL_DRIVER_VER(10, 30, 12) ||
         ctxInfo.renderer() == GrGLRenderer::kIntelBroadwell)) {
        fPerformColorClearsAsDraws = true;
    }
    // crbug.com/969609 - NVIDIA on Mac sometimes segfaults during glClear in chrome. It seems
    // mostly concentrated in 10.13/14, GT 650Ms, driver 12+. But there are instances of older
    // drivers and GTX 775s, so we'll start with a broader workaround.
    if (ctxInfo.vendor() == GrGLVendor::kNVIDIA) {
        fPerformColorClearsAsDraws = true;
    }
#endif

    // See crbug.com/755871. This could probably be narrowed to just partial clears as the driver
    // bugs seems to involve clearing too much and not skipping the clear.
    // See crbug.com/768134. This is also needed for full clears and was seen on an nVidia K620
    // but only for D3D11 ANGLE.
    if (ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D11) {
        fPerformColorClearsAsDraws = true;
    }

    if (ctxInfo.renderer() == GrGLRenderer::kAdreno430 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno4xx_other) {
        // This is known to be fixed sometime between driver 145.0 and 219.0
        if (ctxInfo.driverVersion() <= GR_GL_DRIVER_VER(219, 0, 0)) {
            fPerformStencilClearsAsDraws = true;
        }
        // This is known to be fixed sometime between driver 129.0 and 145.0 on Nexus 6P.
        // On driver 129 on Android M it fails the unit tests called WritePixelsPendingIO without
        // the workaround. It passes on Android N with driver 145 without the workaround.
        // skbug.com/11834
        if (ctxInfo.driverVersion() < GR_GL_DRIVER_VER(145, 0, 0)) {
            fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO = true;
        }
    }

    if (fDriverBugWorkarounds.gl_clear_broken) {
        fPerformColorClearsAsDraws = true;
        fPerformStencilClearsAsDraws = true;
    }

    if (ctxInfo.vendor() == GrGLVendor::kQualcomm) {
        // It appears that all the Adreno GPUs have less than optimal performance when
        // drawing w/ large index buffers.
        fAvoidLargeIndexBufferDraws = true;
    }

    // This was reproduced on the following configurations:
    // - A Galaxy J5 (Adreno 306) running Android 6 with driver 140.0
    // - A Nexus 7 2013 (Adreno 320) running Android 5 with driver 104.0
    // - A Nexus 7 2013 (Adreno 320) running Android 6 with driver 127.0
    // - A Nexus 5 (Adreno 330) running Android 6 with driver 127.0
    // and not produced on:
    // - A Nexus 7 2013 (Adreno 320) running Android 4 with driver 53.0
    // The particular lines that get dropped from test images varies across different devices.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno3xx &&
        ctxInfo.driverVersion() > GR_GL_DRIVER_VER(53, 0, 0)) {
        fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines = true;
    }

    // TODO: Don't apply this on iOS?
    if (ctxInfo.renderer() == GrGLRenderer::kPowerVRRogue) {
        // Our Chromebook with GrGLRenderer::kPowerVRRogue crashes on large instanced draws. The
        // current minimum number of instances observed to crash is somewhere between 2^14 and 2^15.
        // Keep the number of instances below 1000, just to be safe.
        fMaxInstancesPerDrawWithoutCrashing = 999;
    } else if (fDriverBugWorkarounds.disallow_large_instanced_draw) {
        fMaxInstancesPerDrawWithoutCrashing = 0x4000000;
    }

#ifndef SK_BUILD_FOR_IOS
    if (ctxInfo.renderer() == GrGLRenderer::kPowerVRRogue) {
        // We saw this bug on a TecnoSpark 3 Pro with a PowerVR GE8300.
        // GL_VERSION: "OpenGL ES 3.2 build 1.10@51309121"
        // Possibly this could be more limited by driver version or HW generation.
        // When using samplers, we are seeing a bug where the gpu is sometimes not sampling the
        // correct mip level data. A workaround to this issue is that when binding a texture we also
        // set some texture state, and it seems like any inividual state works (e.g. min/mag filter,
        // base level, max level, etc.). Currently we just set the min filter level every time we
        // bind a texture as the workaround.
        fMustSetAnyTexParameterToEnableMipmapping = true;
        // ColorTypeBackendAllocationTest failed for kAlpha_8 and kGray_8 when using
        // GL_UNPACK_ROW_LENGTH. Perhaps this could be a more limited workaround by applying
        // only to single channel 8 bit unorm formats but we only have a monolithic query for this
        // support at present.
        fWritePixelsRowBytesSupport = false;
        // TransferPixelsToTextureTest fails for all color types on
        // TecnoSpark 3 Pro with a PowerVR GE8300, GL_VERSION: "OpenGL ES 3.2 build 1.10@51309121"
        // if GL_UNPACK_ROW_LENGTH is used.
        fTransferPixelsToRowBytesSupport = false;
    }
#endif

    // Texture uploads sometimes seem to be ignored to textures bound to FBOS on Tegra3.
    if (ctxInfo.renderer() == GrGLRenderer::kTegra_PreK1) {
        fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO = true;
        fUseDrawInsteadOfAllRenderTargetWrites = true;
    }

#ifdef SK_BUILD_FOR_MAC
    static constexpr bool isMAC = true;
#else
    static constexpr bool isMAC = false;
#endif

#ifdef SK_BUILD_FOR_ANDROID
    // Older versions of Android have problems with setting GL_TEXTURE_BASE_LEVEL or
    // GL_TEXTURE_MAX_LEVEL on GL_TEXTURE_EXTERTNAL_OES textures. We just leave them as is and hope
    // the client never changes them either.
    fDontSetBaseOrMaxLevelForExternalTextures = true;
    // PowerVR can crash setting the levels on Android up to Q for any texture?
    // https://crbug.com/1123874
    if (ctxInfo.vendor() == GrGLVendor::kImagination) {
        fMipmapLevelControlSupport =  false;
    }
#endif

    // We support manual mip-map generation (via iterative downsampling draw calls). This fixes
    // bugs on some cards/drivers that produce incorrect mip-maps for sRGB textures when using
    // glGenerateMipmap. Our implementation requires mip-level sampling control. Additionally,
    // it can be much slower (especially on mobile GPUs), so we opt-in only when necessary:
    if (fMipmapLevelControlSupport &&
        !ctxInfo.isOverCommandBuffer() &&  // http://crbug.com/1224110
        (contextOptions.fDoManualMipmapping                 ||
         ctxInfo.vendor()  == GrGLVendor::kIntel            ||
         (ctxInfo.driver() == GrGLDriver::kNVIDIA && isMAC) ||
         ctxInfo.vendor()  == GrGLVendor::kATI)) {
        fDoManualMipmapping = true;
    }

    // See http://crbug.com/710443
#ifdef SK_BUILD_FOR_MAC
    if (ctxInfo.renderer() == GrGLRenderer::kIntelBroadwell) {
        fClearToBoundaryValuesIsBroken = true;
    }
#endif
    if (ctxInfo.vendor() == GrGLVendor::kQualcomm) {
        fDrawArraysBaseVertexIsBroken = true;
    }

    // https://b.corp.google.com/issues/188410972
    if (ctxInfo.renderer() == GrGLRenderer::kVirgl) {
        fDrawInstancedSupport = false;
    }

    // http://anglebug.com/4538
    if (fBaseVertexBaseInstanceSupport && !fDrawInstancedSupport) {
        fBaseVertexBaseInstanceSupport = false;
        fNativeDrawIndirectSupport = false;
        fMultiDrawType = MultiDrawType::kNone;
    }

    // Currently the extension is advertised but fb fetch is broken on 500 series Adrenos like the
    // Galaxy S7.
    // TODO: Once this is fixed we can update the check here to look at a driver version number too.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno530 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno5xx_other) {
        shaderCaps->fFBFetchSupport = false;
    }

    // On the NexusS and GalaxyNexus, the use of 'any' causes the compilation error "Calls to any
    // function that may require a gradient calculation inside a conditional block may return
    // undefined results". This appears to be an issue with the 'any' call since even the simple
    // "result=black; if (any()) result=white;" code fails to compile.
    shaderCaps->fCanUseAnyFunctionInShader = (ctxInfo.vendor() != GrGLVendor::kImagination);

    if (ctxInfo.renderer() == GrGLRenderer::kTegra_PreK1) {
        // The Tegra3 compiler will sometimes never return if we have min(abs(x), 1.0),
        // so we must do the abs first in a separate expression.
        shaderCaps->fCanUseMinAndAbsTogether = false;

        // Tegra3 fract() seems to trigger undefined behavior for negative values, so we
        // must avoid this condition.
        shaderCaps->fCanUseFractForNegativeValues = false;

        // Seeing crashes on Tegra3 with inlined functions that have early returns. Looks like the
        // do { ... break; } while (false); construct is causing a crash in the driver.
        shaderCaps->fCanUseDoLoops = false;
    }

    // On Intel GPU there is an issue where it reads the second argument to atan "- %s.x" as an int
    // thus must us -1.0 * %s.x to work correctly
    if (ctxInfo.vendor() == GrGLVendor::kIntel) {
        shaderCaps->fMustForceNegatedAtanParamToFloat = true;
    }

#if defined(SK_BUILD_FOR_MAC)
    if (ctxInfo.vendor() == GrGLVendor::kATI) {
        // The Radeon GLSL compiler on Mac gets confused by ldexp(..., -x).
        // Convert to ldexp(..., x * -1).
        // http://skbug.com/12076
        shaderCaps->fMustForceNegatedLdexpParamToMultiply = true;
    }
#endif

    // On some Intel GPUs there is an issue where the driver outputs bogus values in the shader
    // when floor and abs are called on the same line. Thus we must execute an Op between them to
    // make sure the compiler doesn't re-inline them even if we break the calls apart.
    if (ctxInfo.vendor() == GrGLVendor::kIntel) {
        shaderCaps->fMustDoOpBetweenFloorAndAbs = true;
    }

    // On Adreno devices with framebuffer fetch support, there is a bug where they always return
    // the original dst color when reading the outColor even after being written to. By using a
    // local outColor we can work around this bug.
    if (shaderCaps->fFBFetchSupport && ctxInfo.vendor() == GrGLVendor::kQualcomm) {
        shaderCaps->fRequiresLocalOutputColorForFBFetch = true;
    }

    // Newer Mali GPUs do incorrect static analysis in specific situations: If there is uniform
    // color, and that uniform contains an opaque color, and the output of the shader is only based
    // on that uniform plus soemthing un-trackable (like a texture read), the compiler will deduce
    // that the shader always outputs opaque values. In that case, it appears to remove the shader
    // based blending code it normally injects, turning SrcOver into Src. To fix this, we always
    // insert an extra bit of math on the uniform that confuses the compiler just enough...
    if (ctxInfo.renderer() == GrGLRenderer::kMaliT) {
        shaderCaps->fMustObfuscateUniformColor = true;
    }

    // On Mali G series GPUs, applying transfer functions in the fragment shader with half-floats
    // produces answers that are much less accurate than expected/required. This forces full floats
    // for some intermediate values to get acceptable results.
    if (ctxInfo.renderer() == GrGLRenderer::kMaliG) {
        fShaderCaps->fColorSpaceMathNeedsFloat = true;
    }

    // On Mali 400 there is a bug using dFd* in the x direction. So we avoid using it when possible.
    if (ctxInfo.renderer() == GrGLRenderer::kMali4xx) {
        fShaderCaps->fAvoidDfDxForGradientsWhenPossible = true;
    }

#ifdef SK_BUILD_FOR_WIN
    // Check for ANGLE on Windows, so we can workaround a bug in D3D itself (anglebug.com/2098).
    //
    // Basically, if a shader has a construct like:
    //
    // float x = someCondition ? someValue : 0;
    // float2 result = (0 == x) ? float2(x, x)
    //                          : float2(2 * x / x, 0);
    //
    // ... the compiler will produce an error 'NaN and infinity literals not allowed', even though
    // we've explicitly guarded the division with a check against zero. This manifests in much
    // more complex ways in some of our shaders, so we use this caps bit to add an epsilon value
    // to the denominator of divisions, even when we've added checks that the denominator isn't 0.
    if (ctxInfo.angleBackend() != GrGLANGLEBackend::kUnknown || ctxInfo.isOverCommandBuffer()) {
        shaderCaps->fMustGuardDivisionEvenAfterExplicitZeroCheck = true;
    }
#endif

    if (ctxInfo.renderer() == GrGLRenderer::kAdreno615 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno620 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno630 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno640 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno6xx_other) {
        shaderCaps->fInBlendModesFailRandomlyForAllZeroVec = true;
    }

    // The Adreno 5xx and 6xx produce incorrect results when comparing a pair of matrices.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno530 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno5xx_other ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno615 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno620 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno630 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno640 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno6xx_other) {
        shaderCaps->fRewriteMatrixComparisons = true;
    }

    // We've seen Adreno 3xx devices produce incorrect (flipped) values for gl_FragCoord, in some
    // (rare) situations. It's sporadic, and mostly on older drivers. Additionally, old Adreno
    // compilers (see crbug.com/skia/4078) crash when accessing .zw of gl_FragCoord, so just bypass
    // using gl_FragCoord at all to get around it.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno3xx) {
        shaderCaps->fCanUseFragCoord = false;
    }

    // gl_FragCoord has an incorrect subpixel offset on legacy Tegra hardware.
    if (ctxInfo.renderer() == GrGLRenderer::kTegra_PreK1) {
        shaderCaps->fCanUseFragCoord = false;
    }

    // On Mali G71, mediump ints don't appear capable of representing every integer beyond +/-2048.
    // (Are they implemented with fp16?)
    if (ctxInfo.vendor() == GrGLVendor::kARM) {
        shaderCaps->fIncompleteShortIntPrecision = true;
    }

    if (fDriverBugWorkarounds.add_and_true_to_loop_condition) {
        shaderCaps->fAddAndTrueToLoopCondition = true;
    }

    if (fDriverBugWorkarounds.unfold_short_circuit_as_ternary_operation) {
        shaderCaps->fUnfoldShortCircuitAsTernary = true;
    }

    if (fDriverBugWorkarounds.emulate_abs_int_function) {
        shaderCaps->fEmulateAbsIntFunction = true;
    }

    if (fDriverBugWorkarounds.rewrite_do_while_loops) {
        shaderCaps->fRewriteDoWhileLoops = true;
    }

    if (fDriverBugWorkarounds.remove_pow_with_constant_exponent) {
        shaderCaps->fRemovePowWithConstantExponent = true;
    }

    if (fDriverBugWorkarounds.disable_dual_source_blending_support) {
        shaderCaps->fDualSourceBlendingSupport = false;
    }

    if (ctxInfo.renderer() == GrGLRenderer::kAdreno3xx ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno4xx_other) {
        shaderCaps->fMustWriteToFragColor = true;
    }

    // Disabling advanced blend on various platforms with major known issues. We also block Chrome
    // command buffer for now until its own denylists can be updated.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno430       ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno4xx_other ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno530       ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno5xx_other ||
        ctxInfo.driver()   == GrGLDriver::kIntel             ||
        ctxInfo.isOverCommandBuffer()                        ||
        ctxInfo.vendor()   == GrGLVendor::kARM /* http://skbug.com/11906 */) {
        fBlendEquationSupport = kBasic_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kNotSupported_AdvBlendEqInteraction;
    }

    // Non-coherent advanced blend has an issue on NVIDIA pre 337.00.
    if (ctxInfo.driver() == GrGLDriver::kNVIDIA &&
        ctxInfo.driverVersion() < GR_GL_DRIVER_VER(337, 00, 0) &&
        kAdvanced_BlendEquationSupport == fBlendEquationSupport) {
        fBlendEquationSupport = kBasic_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kNotSupported_AdvBlendEqInteraction;
    }

    if (fDriverBugWorkarounds.disable_blend_equation_advanced) {
        fBlendEquationSupport = kBasic_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kNotSupported_AdvBlendEqInteraction;
    }

    if (this->advancedBlendEquationSupport()) {
        if (ctxInfo.driver() == GrGLDriver::kNVIDIA &&
            ctxInfo.driverVersion() < GR_GL_DRIVER_VER(355, 00, 0)) {
            // Disable color-dodge and color-burn on pre-355.00 NVIDIA.
            fAdvBlendEqDisableFlags |= (1 << kColorDodge_GrBlendEquation) |
                                    (1 << kColorBurn_GrBlendEquation);
        }
        if (ctxInfo.vendor() == GrGLVendor::kARM) {
            // Disable color-burn on ARM until the fix is released.
            fAdvBlendEqDisableFlags |= (1 << kColorBurn_GrBlendEquation);
        }
    }

    // On Adreno 5xx devices, there is a bug where we first draw using dual source blending. Thus
    // the dst blend func references the dst. Then the next draw we disable blending. However, on
    // the second draw the driver has a bug where it tries to access the second color output again.
    // This is fixed by reseting the blend function to anything that does not reference src2 when we
    // disable blending.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno530 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno5xx_other) {
        fMustResetBlendFuncBetweenDualSourceAndDisable = true;
    }

    // Many ES3 drivers only advertise the ES2 image_external extension, but support the _essl3
    // extension, and require that it be enabled to work with ESSL3. Other devices require the ES2
    // extension to be enabled, even when using ESSL3. Enabling both extensions fixes both cases.
    // skbug.com/7713
    if (ctxInfo.hasExtension("GL_OES_EGL_image_external") &&
        ctxInfo.glslGeneration() >= k330_GrGLSLGeneration &&
        !shaderCaps->fExternalTextureSupport) { // i.e. Missing the _essl3 extension
        shaderCaps->fExternalTextureSupport = true;
        shaderCaps->fExternalTextureExtensionString = "GL_OES_EGL_image_external";
        shaderCaps->fSecondExternalTextureExtensionString = "GL_OES_EGL_image_external_essl3";
    }

#ifdef SK_BUILD_FOR_IOS
    // iOS drivers appear to implement TexSubImage by creating a staging buffer, and copying
    // UNPACK_ROW_LENGTH * height bytes. That's unsafe in several scenarios, and the simplest fix
    // is to just disable the feature.
    // https://github.com/flutter/flutter/issues/16718
    // https://bugreport.apple.com/web/?problemID=39948888
    fWritePixelsRowBytesSupport = false;
    // This affects all iOS devices for transfering from a PBO as well (presumably the issue is in
    // the GL->Metal layer).
    fTransferPixelsToRowBytesSupport = false;
#endif

    if (ctxInfo.vendor()   == GrGLVendor::kIntel       ||  // IntelIris640 drops draws completely.
        ctxInfo.renderer() == GrGLRenderer::kMaliT     ||  // Some curves appear flat on GalaxyS6.
        ctxInfo.renderer() == GrGLRenderer::kAdreno3xx ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno430 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno4xx_other ||  // We get garbage on Adreno405.
        ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D9) {  // D3D9 conic strokes fail.
        fDisableTessellationPathRenderer = true;
    }

    // http://skbug.com/9739
    bool isNVIDIAPascal =
            ctxInfo.driver() == GrGLDriver::kNVIDIA                              &&
            ctxInfo.hasExtension("GL_NV_conservative_raster_pre_snap_triangles") &&  // Pascal+.
            !ctxInfo.hasExtension("GL_NV_conservative_raster_underestimation");      // Volta+.
    if (isNVIDIAPascal && ctxInfo.driverVersion() < GR_GL_DRIVER_VER(440, 00, 0)) {
        if (GR_IS_GR_GL(ctxInfo.standard())) {
            // glMemoryBarrier wasn't around until version 4.2.
            if (ctxInfo.version() >= GR_GL_VER(4,2)) {
                fRequiresManualFBBarrierAfterTessellatedStencilDraw = true;
            } else {
                shaderCaps->fMaxTessellationSegments = 0;
            }
        } else {
            // glMemoryBarrier wasn't around until es version 3.1.
            if (ctxInfo.version() >= GR_GL_VER(3,1)) {
                fRequiresManualFBBarrierAfterTessellatedStencilDraw = true;
            } else {
                shaderCaps->fMaxTessellationSegments = 0;
            }
        }
    }

    if (ctxInfo.driver() == GrGLDriver::kQualcomm) {
        // Qualcomm fails to link programs with tessellation and does not give an error message.
        // http://skbug.com/9740
        shaderCaps->fMaxTessellationSegments = 0;
    }

#ifdef SK_BUILD_FOR_WIN
    // glDrawElementsIndirect fails GrMeshTest on every Win10 Intel bot.
    if (ctxInfo.driver() == GrGLDriver::kIntel ||
        (ctxInfo.angleVendor()  == GrGLVendor::kIntel &&
         ctxInfo.angleBackend() == GrGLANGLEBackend::kOpenGL)) {
        fNativeDrawIndexedIndirectIsBroken = true;
        fUseClientSideIndirectBuffers = true;
    }
#endif

    // PowerVRGX6250 drops every pixel if we modify the sample mask while color writes are disabled.
    if (ctxInfo.renderer() == GrGLRenderer::kPowerVRRogue) {
        fNeverDisableColorWrites = true;
        shaderCaps->fMustWriteToFragColor = true;
    }

    // It appears that Qualcomm drivers don't actually support
    // GL_NV_shader_noperspective_interpolation in ES 3.00 or 3.10 shaders, only 3.20.
    // https://crbug.com/986581
    if (ctxInfo.vendor() == GrGLVendor::kQualcomm &&
        k320es_GrGLSLGeneration != ctxInfo.glslGeneration()) {
        shaderCaps->fNoPerspectiveInterpolationSupport = false;
    }

    // We disable srgb write control for Adreno4xx devices.
    // see: https://bug.skia.org/5329
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno430 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno4xx_other) {
        fSRGBWriteControl = false;
    }

    // MacPro devices with AMD cards fail to create MSAA sRGB render buffers.
#if defined(SK_BUILD_FOR_MAC)
    if (ctxInfo.vendor() == GrGLVendor::kATI) {
        formatWorkarounds->fDisableSRGBRenderWithMSAAForMacAMD = true;
    }
#endif

    // Command buffer fails glTexSubImage2D with type == GL_HALF_FLOAT_OES if a GL_RGBA16F texture
    // is created with glTexStorage2D. See crbug.com/1008003.
    formatWorkarounds->fDisableRGBA16FTexStorageForCrBug1008003 =
            ctxInfo.isOverCommandBuffer() && ctxInfo.version() < GR_GL_VER(3, 0);

#if defined(SK_BUILD_FOR_WIN)
    // On Intel Windows ES contexts it seems that using texture storage with BGRA causes
    // problems with cross-context SkImages.
    formatWorkarounds->fDisableBGRATextureStorageForIntelWindowsES =
            ctxInfo.driver() == GrGLDriver::kIntel && GR_IS_GR_GL_ES(ctxInfo.standard());
#endif

    // On the Intel Iris 6100, interacting with LUM16F seems to confuse the driver. After
    // writing to/reading from a LUM16F texture reads from/writes to other formats behave
    // erratically.
    // All Adrenos claim to support LUM16F but don't appear to actually do so.
    // The failing devices/gpus were: Nexus5/Adreno330, Nexus5x/Adreno418, Pixel/Adreno530,
    // Pixel2XL/Adreno540 and Pixel3/Adreno630
    formatWorkarounds->fDisableLuminance16F = ctxInfo.renderer() == GrGLRenderer::kIntelBroadwell ||
                                              ctxInfo.vendor()   == GrGLVendor::kQualcomm;

#ifdef SK_BUILD_FOR_MAC
    // On a MacBookPro 11.5 running MacOS 10.13 with a Radeon M370X the TransferPixelsFrom test
    // fails when transferring out from a GL_RG8 texture using GL_RG/GL_UNSIGNED_BYTE.
    // The same error also occurs in MacOS 10.15 with a Radeon Pro 5300M.
    formatWorkarounds->fDisallowDirectRG8ReadPixels =
            ctxInfo.renderer() == GrGLRenderer::kAMDRadeonR9M3xx  ||
            ctxInfo.renderer() == GrGLRenderer::kAMDRadeonPro5xxx ||
            ctxInfo.renderer() == GrGLRenderer::kAMDRadeonProVegaxx;
#endif

#ifdef SK_BUILD_FOR_ANDROID
    // We don't usually use glTexStorage() on Android for performance reasons. (crbug.com/945506).
    // On a NVIDIA Shield TV running Android 7.0 creating a texture with glTexImage2D() with
    // internal format GL_LUMINANCE8 fails. However, it succeeds with glTexStorage2D().
    //
    // Additionally, on the Nexus 9 running Android 6.0.1 formats added by GL_EXT_texture_rg and
    // GL_EXT_texture_norm16 cause errors if they are created with glTexImage2D() with
    // an unsized internal format. We wouldn't normally do that but Chrome can limit us
    // artificially to ES2. (crbug.com/1003481)
    if (ctxInfo.vendor() == GrGLVendor::kNVIDIA) {
        formatWorkarounds->fDontDisableTexStorageOnAndroid = true;
    }
#endif

    // https://github.com/flutter/flutter/issues/38700
    if (ctxInfo.driver() == GrGLDriver::kAndroidEmulator) {
        shaderCaps->fNoDefaultPrecisionForExternalSamplers = true;
    }

    // http://skbug.com/9491: Nexus5 produces rendering artifacts when we use QCOM_tiled_rendering.
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno3xx) {
        fTiledRenderingSupport = false;
    }
    // https://github.com/flutter/flutter/issues/47164
    // https://github.com/flutter/flutter/issues/47804
    if (fTiledRenderingSupport && (!glInterface->fFunctions.fStartTiling ||
                                   !glInterface->fFunctions.fEndTiling)) {
        // Some devices expose the QCOM tiled memory extension string but don't actually provide the
        // start and end tiling functions (see above flutter bugs). To work around this, the funcs
        // are marked optional in the interface generator, but we turn off the tiled rendering cap
        // if they aren't provided. This disabling is in driver workarounds so that SKQP will still
        // fail on devices that advertise the extension w/o the functions.
        fTiledRenderingSupport = false;
    }

    if (ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D9) {
        formatWorkarounds->fDisallowBGRA8ReadPixels = true;
    }

    // We disable MSAA for all Intel GPUs. Before Gen9, performance was very bad. Even with Gen9,
    // we've seen driver crashes in the wild. We don't have data on Gen11 yet.
    // (crbug.com/527565, crbug.com/983926)
    if (ctxInfo.vendor() == GrGLVendor::kIntel ||
        ctxInfo.angleVendor() == GrGLVendor::kIntel) {
        fMSFBOType = kNone_MSFBOType;
    }

    // ANGLE doesn't support do-while loops.
    if (ctxInfo.angleBackend() != GrGLANGLEBackend::kUnknown) {
        shaderCaps->fCanUseDoLoops = false;
    }

    // ANGLE's D3D9 backend + AMD GPUs are flaky with program binary caching (skbug.com/10395)
    if (ctxInfo.angleBackend() == GrGLANGLEBackend::kD3D9 &&
        ctxInfo.angleVendor()  == GrGLVendor::kATI) {
        fProgramBinarySupport = false;
    }

    // Two Adreno 530 devices (LG G6 and OnePlus 3T) appear to have driver bugs that are corrupting
    // SkSL::Program memory. To get better/different crash reports, disable node-pooling, so that
    // program allocations aren't reused.  (crbug.com/1147008, crbug.com/1164271)
    if (ctxInfo.renderer() == GrGLRenderer::kAdreno530) {
        shaderCaps->fUseNodePools = false;
    }

    // skbug.com/11204. Avoid recursion issue in SurfaceContext::writePixels.
    if (fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO) {
        fReuseScratchTextures = false;
    }

    // skbug.com/11935. Don't reorder on these GPUs in GL on old drivers.
    if ((ctxInfo.renderer() == GrGLRenderer::kAdreno620 ||
        ctxInfo.renderer() == GrGLRenderer::kAdreno640) &&
        ctxInfo.driverVersion() < GR_GL_DRIVER_VER(571, 0, 0)) {
        fAvoidReorderingRenderTasks = true;
    }

    // http://skbug.com/11965
    if (ctxInfo.renderer() == GrGLRenderer::kGoogleSwiftShader) {
        fShaderCaps->fVertexIDSupport = false;
    }

    // http://crbug.com/1197152
    // http://b/187364475
    // We could limit this < 1.13 on ChromeOS but we don't really have a good way to detect
    // ChromeOS from here.
    if (ctxInfo.renderer()      == GrGLRenderer::kPowerVRRogue &&
        ctxInfo.driver()        == GrGLDriver::kImagination    &&
        ctxInfo.driverVersion() <  GR_GL_DRIVER_VER(1, 16, 0)) {
        fShaderCaps->fShaderDerivativeSupport = false;
    }

    if (ctxInfo.driver() == GrGLDriver::kFreedreno) {
        formatWorkarounds->fDisallowUnorm16Transfers = true;
    }

    // If we keep rebind the same texture to an FBO's color attachment but changing between MSAA and
    // non-MSAA we get corruption in the texture contents. Binding texture 0 and then rebinding the
    // original texture avoids this.
    // This was found on Nexus 5, Android 6.0.1, build M4B30Z
    // GL_VENDOR:   "Qualcomm"
    // GL_RENDERER: "Adreno (TM) 330"
    // GL_VERSION:  "OpenGL ES 3.0 V@127.0 AU@  (GIT@I96aee987eb)"
    //
    // We also so alpha blending issues on these GMs skbug_9819, p3_ovals, p3 on Mali-Gxx devices
    // The GM issues were observed on a Galaxy S9 running Android 10:
    // GL_VERSION:  "OpenGL ES 3.2 v1.r19p0-01rel0.###other-sha0123456789ABCDEF0###"
    // GL_RENDERER: "Mali-G72"
    // GL_VENDOR:   "ARM"
    // and a P30 running Android 9:
    // GL_VERSION:  "OpenGL ES 3.2 v1.r16p0-01rel0.4aee637066427cbcd25297324dba15f5"
    // GL_RENDERER: "Mali-G76"
    // GL_VENDOR:   "ARM"
    // but *not* a Galaxy S20 running Android 10:
    // GL_VERSION:  "OpenGL ES 3.2 v1.r20p0-01rel0.###other-sha0123456789ABCDEF0###"
    // GL_RENDERER: "Mali-G77"
    // GL_VENDOR:   "ARM"
    // It's unclear if the difference is driver version or Bifrost vs Valhall. The workaround is
    // fairly trivial so just applying to all Bifrost and Valhall.
    if ((ctxInfo.renderer() == GrGLRenderer::kAdreno3xx &&
         ctxInfo.driver()   == GrGLDriver::kQualcomm) ||
        (ctxInfo.renderer() == GrGLRenderer::kMaliG)) {
        fBindTexture0WhenChangingTextureFBOMultisampleCount = true;
    }
}

void GrGLCaps::onApplyOptionsOverrides(const GrContextOptions& options) {
    if (options.fDisableDriverCorrectnessWorkarounds) {
        SkASSERT(!fDoManualMipmapping);
        SkASSERT(!fClearToBoundaryValuesIsBroken);
        SkASSERT(0 == fMaxInstancesPerDrawWithoutCrashing);
        SkASSERT(!fDrawArraysBaseVertexIsBroken);
        SkASSERT(!fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO);
        SkASSERT(!fUseDrawInsteadOfAllRenderTargetWrites);
        SkASSERT(!fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines);
        SkASSERT(!fDontSetBaseOrMaxLevelForExternalTextures);
        SkASSERT(!fNeverDisableColorWrites);
    }
    if (options.fShaderCacheStrategy < GrContextOptions::ShaderCacheStrategy::kBackendBinary) {
        fProgramBinarySupport = false;
    }

    switch (options.fSkipGLErrorChecks) {
        case GrContextOptions::Enable::kNo:
            fSkipErrorChecks = false;
            break;
        case GrContextOptions::Enable::kYes:
            fSkipErrorChecks = true;
            break;
        case GrContextOptions::Enable::kDefault:
            break;
    }
}

bool GrGLCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    if (fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO) {
        if (auto tex = static_cast<const GrGLTexture*>(surface->asTexture())) {
            if (tex->hasBaseLevelBeenBoundToFBO()) {
                return false;
            }
        }
    }
    if (auto rt = surface->asRenderTarget()) {
        if (fUseDrawInsteadOfAllRenderTargetWrites) {
            return false;
        }
        if (rt->numSamples() > 1 && this->usesMSAARenderBuffers()) {
            return false;
        }
        return SkToBool(surface->asTexture());
    }
    return true;
}

GrCaps::SurfaceReadPixelsSupport GrGLCaps::surfaceSupportsReadPixels(
        const GrSurface* surface) const {
    if (auto tex = static_cast<const GrGLTexture*>(surface->asTexture())) {
        // We don't support reading pixels directly from EXTERNAL textures as it would require
        // binding the texture to a FBO. For now we also disallow reading back directly
        // from compressed textures.
        if (tex->target() == GR_GL_TEXTURE_EXTERNAL || GrGLFormatIsCompressed(tex->format())) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
    } else if (auto rt = static_cast<const GrGLRenderTarget*>(surface->asRenderTarget())) {
        // glReadPixels does not allow reading back from a MSAA framebuffer. If the underlying
        // GrSurface doesn't have a second FBO to resolve to then we must make a copy.
        if (rt->numSamples() > 1 && !rt->asTexture()) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
    }
    return SurfaceReadPixelsSupport::kSupported;
}

size_t offset_alignment_for_transfer_buffer(GrGLenum externalType) {
    // This switch is derived from a table titled "Pixel data type parameter values and the
    // corresponding GL data types" in the OpenGL spec (Table 8.2 in OpenGL 4.5).
    switch (externalType) {
        case GR_GL_UNSIGNED_BYTE:                   return sizeof(GrGLubyte);
        case GR_GL_BYTE:                            return sizeof(GrGLbyte);
        case GR_GL_UNSIGNED_SHORT:                  return sizeof(GrGLushort);
        case GR_GL_SHORT:                           return sizeof(GrGLshort);
        case GR_GL_UNSIGNED_INT:                    return sizeof(GrGLuint);
        case GR_GL_INT:                             return sizeof(GrGLint);
        case GR_GL_HALF_FLOAT:                      return sizeof(GrGLhalf);
        case GR_GL_HALF_FLOAT_OES:                  return sizeof(GrGLhalf);
        case GR_GL_FLOAT:                           return sizeof(GrGLfloat);
        case GR_GL_UNSIGNED_SHORT_5_6_5:            return sizeof(GrGLushort);
        case GR_GL_UNSIGNED_SHORT_4_4_4_4:          return sizeof(GrGLushort);
        case GR_GL_UNSIGNED_SHORT_5_5_5_1:          return sizeof(GrGLushort);
        case GR_GL_UNSIGNED_INT_2_10_10_10_REV:     return sizeof(GrGLuint);
#if 0  // GL types we currently don't use. Here for future reference.
        case GR_GL_UNSIGNED_BYTE_3_3_2:             return sizeof(GrGLubyte);
        case GR_GL_UNSIGNED_BYTE_2_3_3_REV:         return sizeof(GrGLubyte);
        case GR_GL_UNSIGNED_SHORT_5_6_5_REV:        return sizeof(GrGLushort);
        case GR_GL_UNSIGNED_SHORT_4_4_4_4_REV:      return sizeof(GrGLushort);
        case GR_GL_UNSIGNED_SHORT_1_5_5_5_REV:      return sizeof(GrGLushort);
        case GR_GL_UNSIGNED_INT_8_8_8_8:            return sizeof(GrGLuint);
        case GR_GL_UNSIGNED_INT_8_8_8_8_REV:        return sizeof(GrGLuint);
        case GR_GL_UNSIGNED_INT_10_10_10_2:         return sizeof(GrGLuint);
        case GR_GL_UNSIGNED_INT_24_8:               return sizeof(GrGLuint);
        case GR_GL_UNSIGNED_INT_10F_11F_11F_REV:    return sizeof(GrGLuint);
        case GR_GL_UNSIGNED_INT_5_9_9_9_REV:        return sizeof(GrGLuint);
        // This one is not corresponding to a GL data type and the spec just says it is 4.
        case GR_GL_FLOAT_32_UNSIGNED_INT_24_8_REV:  return 4;
#endif
        default:                                    return 0;
    }
}

GrCaps::SupportedRead GrGLCaps::onSupportedReadPixelsColorType(
        GrColorType srcColorType, const GrBackendFormat& srcBackendFormat,
        GrColorType dstColorType) const {

    SkImage::CompressionType compression = GrBackendFormatToCompressionType(srcBackendFormat);
    if (compression != SkImage::CompressionType::kNone) {
        return {SkCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                                                       : GrColorType::kRGBA_8888,
                0};
    }

    // We first try to find a supported read pixels GrColorType that matches the requested
    // dstColorType. If that doesn't exists we will use any valid read pixels GrColorType.
    GrCaps::SupportedRead fallbackRead = {GrColorType::kUnknown, 0};
    const auto& formatInfo = this->getFormatInfo(srcBackendFormat.asGLFormat());
    bool foundSrcCT = false;
    for (int i = 0; !foundSrcCT && i < formatInfo.fColorTypeInfoCount; ++i) {
        if (formatInfo.fColorTypeInfos[i].fColorType == srcColorType) {
            const ColorTypeInfo& ctInfo = formatInfo.fColorTypeInfos[i];
            foundSrcCT = true;
            for (int j = 0; j < ctInfo.fExternalIOFormatCount; ++j) {
                const auto& ioInfo = ctInfo.fExternalIOFormats[j];
                if (ioInfo.fExternalReadFormat != 0) {
                    if (formatInfo.fHaveQueriedImplementationReadSupport ||
                        !ioInfo.fRequiresImplementationReadQuery) {
                        GrGLenum transferOffsetAlignment = 0;
                        if (formatInfo.fFlags & FormatInfo::kTransfers_Flag) {
                            transferOffsetAlignment =
                                    offset_alignment_for_transfer_buffer(ioInfo.fExternalType);
                        }
                        if (ioInfo.fColorType == dstColorType) {
                            return {dstColorType, transferOffsetAlignment};
                        }
                        // Currently we just pick the first supported format that we find as our
                        // fallback.
                        if (fallbackRead.fColorType == GrColorType::kUnknown) {
                            fallbackRead = {ioInfo.fColorType, transferOffsetAlignment};
                        }
                    }
                }
            }
        }
    }
    return fallbackRead;
}

GrCaps::SupportedWrite GrGLCaps::supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                               const GrBackendFormat& surfaceFormat,
                                                               GrColorType srcColorType) const {
    // We first try to find a supported write pixels GrColorType that matches the data's
    // srcColorType. If that doesn't exists we will use any supported GrColorType.
    GrColorType fallbackCT = GrColorType::kUnknown;
    const auto& formatInfo = this->getFormatInfo(surfaceFormat.asGLFormat());
    bool foundSurfaceCT = false;
    size_t transferOffsetAlignment = 0;
    if (formatInfo.fFlags & FormatInfo::kTransfers_Flag) {
        transferOffsetAlignment = 1;
    }
    for (int i = 0; !foundSurfaceCT && i < formatInfo.fColorTypeInfoCount; ++i) {
        if (formatInfo.fColorTypeInfos[i].fColorType == surfaceColorType) {
            const ColorTypeInfo& ctInfo = formatInfo.fColorTypeInfos[i];
            foundSurfaceCT = true;
            for (int j = 0; j < ctInfo.fExternalIOFormatCount; ++j) {
                const auto& ioInfo = ctInfo.fExternalIOFormats[j];
                if (ioInfo.fExternalTexImageFormat != 0) {
                    if (ioInfo.fColorType == srcColorType) {
                        return {srcColorType, transferOffsetAlignment};
                    }
                    // Currently we just pick the first supported format that we find as our
                    // fallback.
                    if (fallbackCT == GrColorType::kUnknown) {
                        fallbackCT = ioInfo.fColorType;
                    }
                }
            }
        }
    }
    return {fallbackCT, transferOffsetAlignment};
}

bool GrGLCaps::onIsWindowRectanglesSupportedForRT(const GrBackendRenderTarget& backendRT) const {
    GrGLFramebufferInfo fbInfo;
    SkAssertResult(backendRT.getGLFramebufferInfo(&fbInfo));
    // Window Rectangles are not supported for FBO 0;
    return fbInfo.fFBOID != 0;
}

bool GrGLCaps::isFormatSRGB(const GrBackendFormat& format) const {
    return format.asGLFormat() == GrGLFormat::kSRGB8_ALPHA8;
}

bool GrGLCaps::isFormatTexturable(const GrBackendFormat& format, GrTextureType textureType) const {
    if (textureType == GrTextureType::kRectangle && !this->rectangleTextureSupport()) {
        return false;
    }
    return this->isFormatTexturable(format.asGLFormat());
}

bool GrGLCaps::isFormatTexturable(GrGLFormat format) const {
    const FormatInfo& info = this->getFormatInfo(format);
    return SkToBool(info.fFlags & FormatInfo::kTexturable_Flag);
}

bool GrGLCaps::isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                             int sampleCount) const {
    if (format.textureType() == GrTextureType::kRectangle && !this->rectangleTextureSupport()) {
        return false;
    }
    if (format.textureType() == GrTextureType::kExternal) {
        return false;
    }
    auto f = format.asGLFormat();
    const FormatInfo& info = this->getFormatInfo(f);
    if (!SkToBool(info.colorTypeFlags(ct) & ColorTypeInfo::kRenderable_Flag)) {
        return false;
    }

    return this->isFormatRenderable(f, sampleCount);
}

bool GrGLCaps::isFormatRenderable(const GrBackendFormat& format, int sampleCount) const {
    if (format.textureType() == GrTextureType::kRectangle && !this->rectangleTextureSupport()) {
        return false;
    }
    if (format.textureType() == GrTextureType::kExternal) {
        return false;
    }
    return this->isFormatRenderable(format.asGLFormat(), sampleCount);
}

int GrGLCaps::getRenderTargetSampleCount(int requestedCount, GrGLFormat format) const {
    const FormatInfo& info = this->getFormatInfo(format);

    int count = info.fColorSampleCounts.count();
    if (!count) {
        return 0;
    }

    requestedCount = std::max(1, requestedCount);
    if (1 == requestedCount) {
        return info.fColorSampleCounts[0] == 1 ? 1 : 0;
    }

    for (int sampleCount : info.fColorSampleCounts) {
        if (sampleCount >= requestedCount) {
            if (fDriverBugWorkarounds.max_msaa_sample_count_4) {
                sampleCount = std::min(sampleCount, 4);
            }
            return sampleCount;
        }
    }
    return 0;
}

int GrGLCaps::maxRenderTargetSampleCount(GrGLFormat format) const {
    const FormatInfo& info = this->getFormatInfo(format);
    const auto& table = info.fColorSampleCounts;
    if (!table.count()) {
        return 0;
    }
    int count = table[table.count() - 1];
    if (fDriverBugWorkarounds.max_msaa_sample_count_4) {
        count = std::min(count, 4);
    }
    return count;
}

bool GrGLCaps::canFormatBeFBOColorAttachment(GrGLFormat format) const {
    return SkToBool(this->getFormatInfo(format).fFlags & FormatInfo::kFBOColorAttachment_Flag);
}

bool GrGLCaps::isFormatCopyable(const GrBackendFormat& format) const {
    // In GL we have three ways to be able to copy. CopyTexImage, blit, and draw. CopyTexImage
    // requires the src to be an FBO attachment, blit requires both src and dst to be FBO
    // attachments, and draw requires the dst to be an FBO attachment. Thus to copy from and to
    // the same config, we need that config to be bindable to an FBO.
    return this->canFormatBeFBOColorAttachment(format.asGLFormat());
}

bool GrGLCaps::formatSupportsTexStorage(GrGLFormat format) const {
    return SkToBool(this->getFormatInfo(format).fFlags & FormatInfo::kUseTexStorage_Flag);
}

bool GrGLCaps::shouldQueryImplementationReadSupport(GrGLFormat format) const {
    const auto& formatInfo = const_cast<GrGLCaps*>(this)->getFormatInfo(format);
    if (!formatInfo.fHaveQueriedImplementationReadSupport) {
        // Check whether we will actually learn anything useful.
        bool needQuery = false;
        for (int i = 0; i < formatInfo.fColorTypeInfoCount && !needQuery; ++i) {
            const auto& surfCTInfo = formatInfo.fColorTypeInfos[i];
            for (int j = 0; j < surfCTInfo.fExternalIOFormatCount; ++j) {
                if (surfCTInfo.fExternalIOFormats[j].fRequiresImplementationReadQuery) {
                    needQuery = true;
                    break;
                }
            }
        }
        if (!needQuery) {
            // Pretend we already checked it.
            const_cast<FormatInfo&>(formatInfo).fHaveQueriedImplementationReadSupport = true;
        }
    }
    return !formatInfo.fHaveQueriedImplementationReadSupport;
}

void GrGLCaps::didQueryImplementationReadSupport(GrGLFormat format,
                                                 GrGLenum readFormat,
                                                 GrGLenum readType) const {
    auto& formatInfo = const_cast<GrGLCaps*>(this)->getFormatInfo(format);
    for (int i = 0; i < formatInfo.fColorTypeInfoCount; ++i) {
        auto& surfCTInfo = formatInfo.fColorTypeInfos[i];
        for (int j = 0; j < surfCTInfo.fExternalIOFormatCount; ++j) {
            auto& readCTInfo = surfCTInfo.fExternalIOFormats[j];
            if (readCTInfo.fRequiresImplementationReadQuery) {
                if (readCTInfo.fExternalReadFormat != readFormat ||
                    readCTInfo.fExternalType != readType) {
                    // Don't zero out fExternalType. It's also used for writing data to the texture!
                    readCTInfo.fExternalReadFormat = 0;
                }
            }
        }
    }
    formatInfo.fHaveQueriedImplementationReadSupport = true;
}

bool GrGLCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                 const GrBackendFormat& format) const {
    GrGLFormat glFormat = format.asGLFormat();
    const auto& info = this->getFormatInfo(glFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == ct) {
            return true;
        }
    }
    return false;
}

GrBackendFormat GrGLCaps::onGetDefaultBackendFormat(GrColorType ct) const {
    auto format = this->getFormatFromColorType(ct);
    if (format == GrGLFormat::kUnknown) {
        return {};
    }
    return GrBackendFormat::MakeGL(GrGLFormatToEnum(format), GR_GL_TEXTURE_2D);
}

GrBackendFormat GrGLCaps::getBackendFormatFromCompressionType(
        SkImage::CompressionType compressionType) const {
    switch (compressionType) {
        case SkImage::CompressionType::kNone:
            return {};
        case SkImage::CompressionType::kETC2_RGB8_UNORM:
            // if ETC2 is available default to that format
            if (this->isFormatTexturable(GrGLFormat::kCOMPRESSED_RGB8_ETC2)) {
                return GrBackendFormat::MakeGL(GR_GL_COMPRESSED_RGB8_ETC2, GR_GL_TEXTURE_2D);
            }
            if (this->isFormatTexturable(GrGLFormat::kCOMPRESSED_ETC1_RGB8)) {
                return GrBackendFormat::MakeGL(GR_GL_COMPRESSED_ETC1_RGB8, GR_GL_TEXTURE_2D);
            }
            return {};
        case SkImage::CompressionType::kBC1_RGB8_UNORM:
            if (this->isFormatTexturable(GrGLFormat::kCOMPRESSED_RGB8_BC1)) {
                return GrBackendFormat::MakeGL(GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                               GR_GL_TEXTURE_2D);
            }
            return {};
        case SkImage::CompressionType::kBC1_RGBA8_UNORM:
            if (this->isFormatTexturable(GrGLFormat::kCOMPRESSED_RGBA8_BC1)) {
                return GrBackendFormat::MakeGL(GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                                               GR_GL_TEXTURE_2D);
            }
            return {};
    }

    SkUNREACHABLE;
}

GrSwizzle GrGLCaps::onGetReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    GrGLFormat glFormat = format.asGLFormat();
    const auto& info = this->getFormatInfo(glFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fReadSwizzle;
        }
    }
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.",
                 (int)colorType, (int)glFormat);
    return {};
}

GrSwizzle GrGLCaps::getWriteSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    const auto& info = this->getFormatInfo(format.asGLFormat());
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fWriteSwizzle;
        }
    }
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.",
                 (int)colorType, (int)format.asGLFormat());
    return {};
}

GrDstSampleFlags GrGLCaps::onGetDstSampleFlagsForProxy(const GrRenderTargetProxy* rt) const {
    if (rt->asTextureProxy()) {
        return GrDstSampleFlags::kRequiresTextureBarrier;
    }
    return GrDstSampleFlags::kNone;
}

bool GrGLCaps::onSupportsDynamicMSAA(const GrRenderTargetProxy* rtProxy) const {
    return !fDisallowDynamicMSAA;
}

uint64_t GrGLCaps::computeFormatKey(const GrBackendFormat& format) const {
    auto glFormat = format.asGLFormat();
    return (uint64_t)(glFormat);
}

GrProgramDesc GrGLCaps::makeDesc(GrRenderTarget* /* rt */,
                                 const GrProgramInfo& programInfo,
                                 ProgramDescOverrideFlags overrideFlags) const {
    SkASSERT(overrideFlags == ProgramDescOverrideFlags::kNone);
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, programInfo, *this);
    return desc;
}

#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrGLCaps::getTestingCombinations() const {
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,
          GrBackendFormat::MakeGL(GR_GL_ALPHA8, GR_GL_TEXTURE_2D) },
        { GrColorType::kAlpha_8,
          GrBackendFormat::MakeGL(GR_GL_R8, GR_GL_TEXTURE_2D) },
        { GrColorType::kBGR_565,
          GrBackendFormat::MakeGL(GR_GL_RGB565, GR_GL_TEXTURE_2D) },
        { GrColorType::kABGR_4444,
          GrBackendFormat::MakeGL(GR_GL_RGBA4, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGBA_8888,
          GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGBA_8888_SRGB,
          GrBackendFormat::MakeGL(GR_GL_SRGB8_ALPHA8, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGB_888x,
          GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGB_888x,
          GrBackendFormat::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGB_888x,
          GrBackendFormat::MakeGL(GR_GL_COMPRESSED_RGB8_ETC2, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGB_888x,
          GrBackendFormat::MakeGL(GR_GL_COMPRESSED_ETC1_RGB8, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGB_888x,
          GrBackendFormat::MakeGL(GR_GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGBA_8888,
          GrBackendFormat::MakeGL(GR_GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GR_GL_TEXTURE_2D) },
        { GrColorType::kRG_88,
          GrBackendFormat::MakeGL(GR_GL_RG8, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGBA_1010102,
          GrBackendFormat::MakeGL(GR_GL_RGB10_A2, GR_GL_TEXTURE_2D) },
        { GrColorType::kGray_8,
          GrBackendFormat::MakeGL(GR_GL_LUMINANCE8, GR_GL_TEXTURE_2D) },
        { GrColorType::kGray_8,
          GrBackendFormat::MakeGL(GR_GL_R8, GR_GL_TEXTURE_2D) },
        { GrColorType::kGrayAlpha_88,
          GrBackendFormat::MakeGL(GR_GL_LUMINANCE8_ALPHA8, GR_GL_TEXTURE_2D) },
        { GrColorType::kAlpha_F16,
          GrBackendFormat::MakeGL(GR_GL_R16F, GR_GL_TEXTURE_2D) },
        { GrColorType::kAlpha_F16,
          GrBackendFormat::MakeGL(GR_GL_LUMINANCE16F, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGBA_F16,
          GrBackendFormat::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGBA_F16_Clamped,
          GrBackendFormat::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_2D) },
        { GrColorType::kAlpha_16,
          GrBackendFormat::MakeGL(GR_GL_R16, GR_GL_TEXTURE_2D) },
        { GrColorType::kRG_1616,
          GrBackendFormat::MakeGL(GR_GL_RG16, GR_GL_TEXTURE_2D) },
        { GrColorType::kRGBA_16161616,
          GrBackendFormat::MakeGL(GR_GL_RGBA16, GR_GL_TEXTURE_2D) },
        { GrColorType::kRG_F16,
          GrBackendFormat::MakeGL(GR_GL_RG16F, GR_GL_TEXTURE_2D) },
    };

    if (GR_IS_GR_GL(fStandard)) {
        combos.push_back({ GrColorType::kBGRA_8888,
                           GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D) });
        combos.push_back({ GrColorType::kBGRA_1010102,
                           GrBackendFormat::MakeGL(GR_GL_RGB10_A2, GR_GL_TEXTURE_2D) });
    } else {
        SkASSERT(GR_IS_GR_GL_ES(fStandard) || GR_IS_GR_WEBGL(fStandard));

        combos.push_back({ GrColorType::kBGRA_8888,
                           GrBackendFormat::MakeGL(GR_GL_BGRA8, GR_GL_TEXTURE_2D) });
    }
    if (this->rectangleTextureSupport()) {
        size_t count2D = combos.size();
        for (size_t i = 0; i < count2D; ++i) {
            auto combo2D = combos[i];
            GrGLenum formatEnum = GrGLFormatToEnum(combo2D.fFormat.asGLFormat());
            combos.push_back({combo2D.fColorType,
                              GrBackendFormat::MakeGL(formatEnum, GR_GL_TEXTURE_RECTANGLE)});
        }
    }
    return combos;
}
#endif
