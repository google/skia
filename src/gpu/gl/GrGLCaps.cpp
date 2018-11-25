/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLCaps.h"
#include "GrContextOptions.h"
#include "GrGLContext.h"
#include "GrGLRenderTarget.h"
#include "GrGLTexture.h"
#include "GrShaderCaps.h"
#include "GrSurfaceProxyPriv.h"
#include "SkJSONWriter.h"
#include "SkTSearch.h"
#include "SkTSort.h"

GrGLCaps::GrGLCaps(const GrContextOptions& contextOptions,
                   const GrGLContextInfo& ctxInfo,
                   const GrGLInterface* glInterface) : INHERITED(contextOptions) {
    fStandard = ctxInfo.standard();

    fStencilFormats.reset();
    fMSFBOType = kNone_MSFBOType;
    fInvalidateFBType = kNone_InvalidateFBType;
    fMapBufferType = kNone_MapBufferType;
    fTransferBufferType = kNone_TransferBufferType;
    fMaxFragmentUniformVectors = 0;
    fUnpackRowLengthSupport = false;
    fUnpackFlipYSupport = false;
    fPackRowLengthSupport = false;
    fPackFlipYSupport = false;
    fTextureUsageSupport = false;
    fAlpha8IsRenderable = false;
    fImagingSupport = false;
    fVertexArrayObjectSupport = false;
    fDirectStateAccessSupport = false;
    fDebugSupport = false;
    fES2CompatibilitySupport = false;
    fDrawIndirectSupport = false;
    fMultiDrawIndirectSupport = false;
    fBaseInstanceSupport = false;
    fIsCoreProfile = false;
    fBindFragDataLocationSupport = false;
    fRectangleTextureSupport = false;
    fTextureSwizzleSupport = false;
    fRGBA8888PixelsOpsAreSlow = false;
    fPartialFBOReadIsSlow = false;
    fMipMapLevelAndLodControlSupport = false;
    fRGBAToBGRAReadbackConversionsAreSlow = false;
    fDoManualMipmapping = false;
    fSRGBDecodeDisableAffectsMipmaps = false;
    fClearToBoundaryValuesIsBroken = false;
    fClearTextureSupport = false;
    fDrawArraysBaseVertexIsBroken = false;
    fUseDrawToClearColor = false;
    fUseDrawToClearStencilClip = false;
    fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO = false;
    fUseDrawInsteadOfAllRenderTargetWrites = false;
    fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines = false;
    fProgramBinarySupport = false;

    fBlitFramebufferFlags = kNoSupport_BlitFramebufferFlag;
    fMaxInstancesPerDrawArraysWithoutCrashing = 0;

    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    this->init(contextOptions, ctxInfo, glInterface);
}

void GrGLCaps::init(const GrContextOptions& contextOptions,
                    const GrGLContextInfo& ctxInfo,
                    const GrGLInterface* gli) {
    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

    if (kGLES_GrGLStandard == standard) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_VECTORS,
                          &fMaxFragmentUniformVectors);
    } else {
        SkASSERT(kGL_GrGLStandard == standard);
        GrGLint max;
        GR_GL_GetIntegerv(gli, GR_GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max);
        fMaxFragmentUniformVectors = max / 4;
        if (version >= GR_GL_VER(3, 2)) {
            GrGLint profileMask;
            GR_GL_GetIntegerv(gli, GR_GL_CONTEXT_PROFILE_MASK, &profileMask);
            fIsCoreProfile = SkToBool(profileMask & GR_GL_CONTEXT_CORE_PROFILE_BIT);
        }
    }
    GR_GL_GetIntegerv(gli, GR_GL_MAX_VERTEX_ATTRIBS, &fMaxVertexAttributes);

    if (kGL_GrGLStandard == standard) {
        fUnpackRowLengthSupport = true;
        fUnpackFlipYSupport = false;
        fPackRowLengthSupport = true;
        fPackFlipYSupport = false;
    } else {
        fUnpackRowLengthSupport = version >= GR_GL_VER(3,0) ||
                                  ctxInfo.hasExtension("GL_EXT_unpack_subimage");
        fUnpackFlipYSupport = ctxInfo.hasExtension("GL_CHROMIUM_flipy");
        fPackRowLengthSupport = version >= GR_GL_VER(3,0) ||
                                ctxInfo.hasExtension("GL_NV_pack_subimage");
        fPackFlipYSupport =
            ctxInfo.hasExtension("GL_ANGLE_pack_reverse_row_order");
    }

    fTextureUsageSupport = (kGLES_GrGLStandard == standard) &&
                            ctxInfo.hasExtension("GL_ANGLE_texture_usage");

    if (kGL_GrGLStandard == standard) {
        fTextureBarrierSupport = version >= GR_GL_VER(4,5) ||
                                 ctxInfo.hasExtension("GL_ARB_texture_barrier") ||
                                 ctxInfo.hasExtension("GL_NV_texture_barrier");
    } else {
        fTextureBarrierSupport = ctxInfo.hasExtension("GL_NV_texture_barrier");
    }

    if (kGL_GrGLStandard == standard) {
        fSampleLocationsSupport = version >= GR_GL_VER(3,2) ||
                                  ctxInfo.hasExtension("GL_ARB_texture_multisample");
    } else {
        fSampleLocationsSupport = version >= GR_GL_VER(3,1);
    }

    fImagingSupport = kGL_GrGLStandard == standard &&
                      ctxInfo.hasExtension("GL_ARB_imaging");

    if (((kGL_GrGLStandard == standard && version >= GR_GL_VER(4,3)) ||
         (kGLES_GrGLStandard == standard && version >= GR_GL_VER(3,0)) ||
         ctxInfo.hasExtension("GL_ARB_invalidate_subdata"))) {
        fDiscardRenderTargetSupport = true;
        fInvalidateFBType = kInvalidate_InvalidateFBType;
    } else if (ctxInfo.hasExtension("GL_EXT_discard_framebuffer")) {
        fDiscardRenderTargetSupport = true;
        fInvalidateFBType = kDiscard_InvalidateFBType;
    }

    // For future reference on Desktop GL, GL_PRIMITIVE_RESTART_FIXED_INDEX appears in 4.3, and
    // GL_PRIMITIVE_RESTART (where the client must call glPrimitiveRestartIndex) appears in 3.1.
    if (kGLES_GrGLStandard == standard) {
        // Primitive restart can cause a 3x slowdown on Adreno. Enable conservatively.
        // TODO: Evaluate on PowerVR.
        // FIXME: Primitive restart would likely be a win on iOS if we had an enum value for it.
        if (kARM_GrGLVendor == ctxInfo.vendor()) {
            fUsePrimitiveRestart = version >= GR_GL_VER(3,0);
        }
    }

    if (kARM_GrGLVendor == ctxInfo.vendor() ||
        kImagination_GrGLVendor == ctxInfo.vendor() ||
        kQualcomm_GrGLVendor == ctxInfo.vendor() ) {
        fPreferFullscreenClears = true;
    }

    if (kGL_GrGLStandard == standard) {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_ARB_vertex_array_object") ||
                                    ctxInfo.hasExtension("GL_APPLE_vertex_array_object");
    } else {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_OES_vertex_array_object");
    }

    if (kGL_GrGLStandard == standard) {
        fDirectStateAccessSupport = ctxInfo.hasExtension("GL_EXT_direct_state_access");
    } else {
        fDirectStateAccessSupport = false;
    }

    if (kGL_GrGLStandard == standard && version >= GR_GL_VER(4,3)) {
        fDebugSupport = true;
    } else {
        fDebugSupport = ctxInfo.hasExtension("GL_KHR_debug");
    }

    if (kGL_GrGLStandard == standard) {
        fES2CompatibilitySupport = ctxInfo.hasExtension("GL_ARB_ES2_compatibility");
    }
    else {
        fES2CompatibilitySupport = true;
    }

    if (kGL_GrGLStandard == standard) {
        fMultisampleDisableSupport = true;
    } else {
        fMultisampleDisableSupport = ctxInfo.hasExtension("GL_EXT_multisample_compatibility");
    }

    if (kGL_GrGLStandard == standard) {
        // 3.1 has draw_instanced but not instanced_arrays, for the time being we only care about
        // instanced arrays, but we could make this more granular if we wanted
        fInstanceAttribSupport =
                version >= GR_GL_VER(3, 2) ||
                (ctxInfo.hasExtension("GL_ARB_draw_instanced") &&
                 ctxInfo.hasExtension("GL_ARB_instanced_arrays"));
    } else {
        fInstanceAttribSupport =
                version >= GR_GL_VER(3, 0) ||
                (ctxInfo.hasExtension("GL_EXT_draw_instanced") &&
                 ctxInfo.hasExtension("GL_EXT_instanced_arrays"));
    }

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3, 0)) {
            fBindFragDataLocationSupport = true;
        }
    } else {
        if (version >= GR_GL_VER(3, 0) && ctxInfo.hasExtension("GL_EXT_blend_func_extended")) {
            fBindFragDataLocationSupport = true;
        }
    }

    fBindUniformLocationSupport = ctxInfo.hasExtension("GL_CHROMIUM_bind_uniform_location");

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3, 1) || ctxInfo.hasExtension("GL_ARB_texture_rectangle")) {
            // We also require textureSize() support for rectangle 2D samplers which was added in
            // GLSL 1.40.
            if (ctxInfo.glslGeneration() >= k140_GrGLSLGeneration) {
                fRectangleTextureSupport = true;
            }
        }
    } else {
        // Command buffer exposes this in GL ES context for Chromium reasons,
        // but it should not be used. Also, at the time of writing command buffer
        // lacks TexImage2D support and ANGLE lacks GL ES 3.0 support.
    }

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3,3) || ctxInfo.hasExtension("GL_ARB_texture_swizzle")) {
            fTextureSwizzleSupport = true;
        }
    } else {
        if (version >= GR_GL_VER(3,0)) {
            fTextureSwizzleSupport = true;
        }
    }

    if (kGL_GrGLStandard == standard) {
        fMipMapLevelAndLodControlSupport = true;
    } else if (kGLES_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3,0)) {
            fMipMapLevelAndLodControlSupport = true;
        }
    }

#ifdef SK_BUILD_FOR_WIN
    // We're assuming that on Windows Chromium we're using ANGLE.
    bool isANGLE = kANGLE_GrGLDriver == ctxInfo.driver() ||
                   kChromium_GrGLDriver == ctxInfo.driver();
    // Angle has slow read/write pixel paths for 32bit RGBA (but fast for BGRA).
    fRGBA8888PixelsOpsAreSlow = isANGLE;
    // On DX9 ANGLE reading a partial FBO is slow. TODO: Check whether this is still true and
    // check DX11 ANGLE.
    fPartialFBOReadIsSlow = isANGLE;
#endif

    bool isMESA = kMesa_GrGLDriver == ctxInfo.driver();
    bool isMAC = false;
#ifdef SK_BUILD_FOR_MAC
    isMAC = true;
#endif

    // Both mesa and mac have reduced performance if reading back an RGBA framebuffer as BGRA or
    // vis-versa.
    fRGBAToBGRAReadbackConversionsAreSlow = isMESA || isMAC;

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(4,4) || ctxInfo.hasExtension("GL_ARB_clear_texture")) {
            fClearTextureSupport = true;
        }
    } else if (ctxInfo.hasExtension("GL_EXT_clear_texture")) {
        fClearTextureSupport = true;
    }

    /**************************************************************************
    * GrShaderCaps fields
    **************************************************************************/

    // This must be called after fCoreProfile is set on the GrGLCaps
    this->initGLSL(ctxInfo, gli);
    GrShaderCaps* shaderCaps = fShaderCaps.get();

    shaderCaps->fPathRenderingSupport = this->hasPathRenderingSupport(ctxInfo, gli);
#if GR_TEST_UTILS
    if (contextOptions.fSuppressPathRendering) {
        shaderCaps->fPathRenderingSupport = false;
    }
#endif

    // Enable supported shader-related caps
    if (kGL_GrGLStandard == standard) {
        shaderCaps->fDualSourceBlendingSupport = (ctxInfo.version() >= GR_GL_VER(3, 3) ||
            ctxInfo.hasExtension("GL_ARB_blend_func_extended")) &&
            GrGLSLSupportsNamedFragmentShaderOutputs(ctxInfo.glslGeneration());

        shaderCaps->fShaderDerivativeSupport = true;

        // we don't support GL_ARB_geometry_shader4, just GL 3.2+ GS
        shaderCaps->fGeometryShaderSupport = ctxInfo.version() >= GR_GL_VER(3, 2) &&
            ctxInfo.glslGeneration() >= k150_GrGLSLGeneration;
        if (shaderCaps->fGeometryShaderSupport) {
            if (ctxInfo.glslGeneration() >= k400_GrGLSLGeneration) {
                shaderCaps->fGSInvocationsSupport = true;
            } else if (ctxInfo.hasExtension("GL_ARB_gpu_shader5")) {
                shaderCaps->fGSInvocationsSupport = true;
                shaderCaps->fGSInvocationsExtensionString = "GL_ARB_gpu_shader5";
            }
        }

        shaderCaps->fIntegerSupport = ctxInfo.version() >= GR_GL_VER(3, 0) &&
            ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
    } else {
        shaderCaps->fDualSourceBlendingSupport = ctxInfo.hasExtension("GL_EXT_blend_func_extended");

        shaderCaps->fShaderDerivativeSupport = ctxInfo.version() >= GR_GL_VER(3, 0) ||
            ctxInfo.hasExtension("GL_OES_standard_derivatives");

        // Mali has support for geometry shaders, but in practice with ccpr they are slower than the
        // backup impl that only uses vertex shaders.
        if (kARM_GrGLVendor != ctxInfo.vendor()) {
            if (ctxInfo.version() >= GR_GL_VER(3,2)) {
                shaderCaps->fGeometryShaderSupport = true;
            } else if (ctxInfo.hasExtension("GL_EXT_geometry_shader")) {
                shaderCaps->fGeometryShaderSupport = true;
                shaderCaps->fGeometryShaderExtensionString = "GL_EXT_geometry_shader";
            }
            shaderCaps->fGSInvocationsSupport = shaderCaps->fGeometryShaderSupport;
        }

        shaderCaps->fIntegerSupport = ctxInfo.version() >= GR_GL_VER(3, 0) &&
            ctxInfo.glslGeneration() >= k330_GrGLSLGeneration; // We use this value for GLSL ES 3.0.
    }

    // Protect ourselves against tracking huge amounts of texture state.
    static const uint8_t kMaxSaneSamplers = 32;
    GrGLint maxSamplers;
    GR_GL_GetIntegerv(gli, GR_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxSamplers);
    shaderCaps->fMaxVertexSamplers = SkTMin<GrGLint>(kMaxSaneSamplers, maxSamplers);
    if (shaderCaps->fGeometryShaderSupport) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &maxSamplers);
        shaderCaps->fMaxGeometrySamplers = SkTMin<GrGLint>(kMaxSaneSamplers, maxSamplers);
    }
    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_IMAGE_UNITS, &maxSamplers);
    shaderCaps->fMaxFragmentSamplers = SkTMin<GrGLint>(kMaxSaneSamplers, maxSamplers);
    GR_GL_GetIntegerv(gli, GR_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxSamplers);
    shaderCaps->fMaxCombinedSamplers = SkTMin<GrGLint>(kMaxSaneSamplers, maxSamplers);

    // This is all *very* approximate.
    switch (ctxInfo.vendor()) {
        case kNVIDIA_GrGLVendor:
            // We've seen a range from 100 x 100 (TegraK1, GTX660) up to 300 x 300 (GTX 1070)
            // but it doesn't clearly align with Pascal vs Maxwell vs Kepler.
            fShaderCaps->fDisableImageMultitexturingDstRectAreaThreshold = 150 * 150;
            break;
        case kImagination_GrGLVendor:
            // Two PowerVR Rogues, Nexus Player and Chromebook Cb5-312T (PowerVR GX6250), show that
            // it is always a win to use multitexturing.
            if (kPowerVRRogue_GrGLRenderer == ctxInfo.renderer()) {
                fShaderCaps->fDisableImageMultitexturingDstRectAreaThreshold =
                        std::numeric_limits<size_t>::max();
            }
            break;
        case kATI_GrGLVendor:
            // So far no AMD GPU shows a performance difference. A tie goes to disabling
            // multitexturing for simplicity's sake.
            fShaderCaps->fDisableImageMultitexturingDstRectAreaThreshold = 0;
            break;
        default:
            break;
    }

    // SGX and Mali GPUs that are based on a tiled-deferred architecture that have trouble with
    // frequently changing VBOs. We've measured a performance increase using non-VBO vertex
    // data for dynamic content on these GPUs. Perhaps we should read the renderer string and
    // limit this decision to specific GPU families rather than basing it on the vendor alone.
    if (!GR_GL_MUST_USE_VBO &&
        !fIsCoreProfile &&
        (kARM_GrGLVendor == ctxInfo.vendor() ||
         kImagination_GrGLVendor == ctxInfo.vendor() ||
         kQualcomm_GrGLVendor == ctxInfo.vendor())) {
        fPreferClientSideDynamicBuffers = true;
    }

    if (!contextOptions.fAvoidStencilBuffers) {
        // To reduce surface area, if we avoid stencil buffers, we also disable MSAA.
        this->initFSAASupport(contextOptions, ctxInfo, gli);
        this->initStencilSupport(ctxInfo);
    }

    // Setup blit framebuffer
    if (kGL_GrGLStandard != ctxInfo.standard()) {
        if (ctxInfo.version() >= GR_GL_VER(3, 0)) {
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
    } else {
        if (fUsesMixedSamples ||
            ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object") ||
            ctxInfo.hasExtension("GL_EXT_framebuffer_blit")) {
            fBlitFramebufferFlags = 0;
        }
    }

    this->initBlendEqationSupport(ctxInfo);

    if (kGL_GrGLStandard == standard) {
        fMapBufferFlags = kCanMap_MapFlag; // we require VBO support and the desktop VBO
                                            // extension includes glMapBuffer.
        if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_map_buffer_range")) {
            fMapBufferFlags |= kSubset_MapFlag;
            fMapBufferType = kMapBufferRange_MapBufferType;
        } else {
            fMapBufferType = kMapBuffer_MapBufferType;
        }
    } else {
        // Unextended GLES2 doesn't have any buffer mapping.
        fMapBufferFlags = kNone_MapBufferType;
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
    }

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_pixel_buffer_object")) {
            fTransferBufferType = kPBO_TransferBufferType;
        }
    } else {
        if (version >= GR_GL_VER(3, 0) ||
            (ctxInfo.hasExtension("GL_NV_pixel_buffer_object") &&
             // GL_EXT_unpack_subimage needed to support subtexture rectangles
             ctxInfo.hasExtension("GL_EXT_unpack_subimage"))) {
            fTransferBufferType = kPBO_TransferBufferType;
// TODO: get transfer buffers working in Chrome
//        } else if (ctxInfo.hasExtension("GL_CHROMIUM_pixel_transfer_buffer_object")) {
//            fTransferBufferType = kChromium_TransferBufferType;
        }
    }

    // On many GPUs, map memory is very expensive, so we effectively disable it here by setting the
    // threshold to the maximum unless the client gives us a hint that map memory is cheap.
    if (fBufferMapThreshold < 0) {
#if 0
        // We think mapping on Chromium will be cheaper once we know ahead of time how much space
        // we will use for all GrMeshDrawOps. Right now we might wind up mapping a large buffer and
        // using a small subset.
        fBufferMapThreshold = kChromium_GrGLDriver == ctxInfo.driver() ? 0 : SK_MaxS32;
#else
        fBufferMapThreshold = SK_MaxS32;
#endif
    }

    if (kGL_GrGLStandard == standard) {
        fNPOTTextureTileSupport = true;
        fMipMapSupport = true;
    } else {
        // Unextended ES2 supports NPOT textures with clamp_to_edge and non-mip filters only
        // ES3 has no limitations.
        fNPOTTextureTileSupport = ctxInfo.version() >= GR_GL_VER(3,0) ||
                                  ctxInfo.hasExtension("GL_OES_texture_npot");
        // ES2 supports MIP mapping for POT textures but our caps don't allow for limited MIP
        // support. The OES extension or ES 3.0 allow for MIPS on NPOT textures. So, apparently,
        // does the undocumented GL_IMG_texture_npot extension. This extension does not seem to
        // to alllow arbitrary wrap modes, however.
        fMipMapSupport = fNPOTTextureTileSupport || ctxInfo.hasExtension("GL_IMG_texture_npot");
    }

    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_SIZE, &fMaxTextureSize);
    GR_GL_GetIntegerv(gli, GR_GL_MAX_RENDERBUFFER_SIZE, &fMaxRenderTargetSize);
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fMaxRenderTargetSize = SkTMin(fMaxTextureSize, fMaxRenderTargetSize);

    fGpuTracingSupport = ctxInfo.hasExtension("GL_EXT_debug_marker");

    // Disable scratch texture reuse on Mali and Adreno devices
    fReuseScratchTextures = kARM_GrGLVendor != ctxInfo.vendor();

#if 0
    fReuseScratchBuffers = kARM_GrGLVendor != ctxInfo.vendor() &&
                           kQualcomm_GrGLVendor != ctxInfo.vendor();
#endif

    if (ctxInfo.hasExtension("GL_EXT_window_rectangles")) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_WINDOW_RECTANGLES, &fMaxWindowRectangles);
    }

#ifdef SK_BUILD_FOR_WIN
    // On ANGLE deferring flushes can lead to GPU starvation
    fPreferVRAMUseOverFlushes = !isANGLE;
#endif

    if (kChromium_GrGLDriver == ctxInfo.driver()) {
        fMustClearUploadedBufferData = true;
    }

    if (kGL_GrGLStandard == standard) {
        // ARB allows mixed size FBO attachments, EXT does not.
        if (ctxInfo.version() >= GR_GL_VER(3, 0) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fOversizedStencilSupport = true;
        } else {
            SkASSERT(ctxInfo.hasExtension("GL_EXT_framebuffer_object"));
        }
    } else {
        // ES 3.0 supports mixed size FBO attachments, 2.0 does not.
        fOversizedStencilSupport = ctxInfo.version() >= GR_GL_VER(3, 0);
    }

    if (kGL_GrGLStandard == standard) {
        fDrawIndirectSupport = version >= GR_GL_VER(4,0) ||
                               ctxInfo.hasExtension("GL_ARB_draw_indirect");
        fBaseInstanceSupport = version >= GR_GL_VER(4,2);
        fMultiDrawIndirectSupport = version >= GR_GL_VER(4,3) ||
                                    (fDrawIndirectSupport &&
                                     !fBaseInstanceSupport && // The ARB extension has no base inst.
                                     ctxInfo.hasExtension("GL_ARB_multi_draw_indirect"));
        fDrawRangeElementsSupport = version >= GR_GL_VER(2,0);
    } else {
        fDrawIndirectSupport = version >= GR_GL_VER(3,1);
        fMultiDrawIndirectSupport = fDrawIndirectSupport &&
                                    ctxInfo.hasExtension("GL_EXT_multi_draw_indirect");
        fBaseInstanceSupport = fDrawIndirectSupport &&
                               ctxInfo.hasExtension("GL_EXT_base_instance");
        fDrawRangeElementsSupport = version >= GR_GL_VER(3,0);
    }

    if (kGL_GrGLStandard == standard) {
        if ((version >= GR_GL_VER(4, 0) || ctxInfo.hasExtension("GL_ARB_sample_shading"))) {
            fSampleShadingSupport = true;
        }
    } else if (ctxInfo.hasExtension("GL_OES_sample_shading")) {
        fSampleShadingSupport = true;
    }

    // TODO: support CHROMIUM_sync_point and maybe KHR_fence_sync
    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3, 2) || ctxInfo.hasExtension("GL_ARB_sync")) {
            fFenceSyncSupport = true;
        }
    } else if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_APPLE_sync")) {
        fFenceSyncSupport = true;
    }

    // Safely moving textures between contexts requires fences.
    fCrossContextTextureSupport = fFenceSyncSupport;

    fSRGBDecodeDisableSupport = ctxInfo.hasExtension("GL_EXT_texture_sRGB_decode");

    fSRGBDecodeDisableAffectsMipmaps = fSRGBDecodeDisableSupport &&
        kChromium_GrGLDriver != ctxInfo.driver();

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(4, 1)) {
            fProgramBinarySupport = true;
        }
    } else if (version >= GR_GL_VER(3, 0)) {
        fProgramBinarySupport = true;
    }

    // Requires fTextureRedSupport, fTextureSwizzleSupport, msaa support, ES compatibility have
    // already been detected.
    this->initConfigTable(contextOptions, ctxInfo, gli, shaderCaps);

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(ctxInfo, contextOptions, shaderCaps);
    }

    this->applyOptionsOverrides(contextOptions);
    shaderCaps->applyOptionsOverrides(contextOptions);

    // For now these two are equivalent but we could have dst read in shader via some other method.
    shaderCaps->fDstReadInShaderSupport = shaderCaps->fFBFetchSupport;
}

const char* get_glsl_version_decl_string(GrGLStandard standard, GrGLSLGeneration generation,
                                         bool isCoreProfile) {
    switch (generation) {
        case k110_GrGLSLGeneration:
            if (kGLES_GrGLStandard == standard) {
                // ES2s shader language is based on version 1.20 but is version
                // 1.00 of the ES language.
                return "#version 100\n";
            } else {
                SkASSERT(kGL_GrGLStandard == standard);
                return "#version 110\n";
            }
        case k130_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            return "#version 130\n";
        case k140_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            return "#version 140\n";
        case k150_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            if (isCoreProfile) {
                return "#version 150\n";
            } else {
                return "#version 150 compatibility\n";
            }
        case k330_GrGLSLGeneration:
            if (kGLES_GrGLStandard == standard) {
                return "#version 300 es\n";
            } else {
                SkASSERT(kGL_GrGLStandard == standard);
                if (isCoreProfile) {
                    return "#version 330\n";
                } else {
                    return "#version 330 compatibility\n";
                }
            }
        case k400_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            if (isCoreProfile) {
                return "#version 400\n";
            } else {
                return "#version 400 compatibility\n";
            }
        case k420_GrGLSLGeneration:
            SkASSERT(kGL_GrGLStandard == standard);
            if (isCoreProfile) {
                return "#version 420\n";
            }
            else {
                return "#version 420 compatibility\n";
            }
        case k310es_GrGLSLGeneration:
            SkASSERT(kGLES_GrGLStandard == standard);
            return "#version 310 es\n";
        case k320es_GrGLSLGeneration:
            SkASSERT(kGLES_GrGLStandard == standard);
            return "#version 320 es\n";
    }
    return "<no version>";
}

bool is_float_fp32(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli, GrGLenum precision) {
    if (kGLES_GrGLStandard != ctxInfo.standard() &&
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
    if (kGLES_GrGLStandard == standard) {
        if (ctxInfo.hasExtension("GL_EXT_shader_framebuffer_fetch")) {
            shaderCaps->fFBFetchNeedsCustomOutput = (version >= GR_GL_VER(3, 0));
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "gl_LastFragData[0]";
            shaderCaps->fFBFetchExtensionString = "GL_EXT_shader_framebuffer_fetch";
        }
        else if (ctxInfo.hasExtension("GL_NV_shader_framebuffer_fetch")) {
            // Actually, we haven't seen an ES3.0 device with this extension yet, so we don't know
            shaderCaps->fFBFetchNeedsCustomOutput = false;
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "gl_LastFragData[0]";
            shaderCaps->fFBFetchExtensionString = "GL_NV_shader_framebuffer_fetch";
        }
        else if (ctxInfo.hasExtension("GL_ARM_shader_framebuffer_fetch")) {
            // The arm extension also requires an additional flag which we will set onResetContext
            shaderCaps->fFBFetchNeedsCustomOutput = false;
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "gl_LastFragColorARM";
            shaderCaps->fFBFetchExtensionString = "GL_ARM_shader_framebuffer_fetch";
        }
        shaderCaps->fUsesPrecisionModifiers = true;
    }

    shaderCaps->fBindlessTextureSupport = ctxInfo.hasExtension("GL_NV_bindless_texture");

    if (kGL_GrGLStandard == standard) {
        shaderCaps->fFlatInterpolationSupport = ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
    } else {
        shaderCaps->fFlatInterpolationSupport =
            ctxInfo.glslGeneration() >= k330_GrGLSLGeneration; // This is the value for GLSL ES 3.0.
    }
    // Flat interpolation appears to be slow on Qualcomm GPUs (tested Adreno 405 and 530).
    shaderCaps->fPreferFlatInterpolation = shaderCaps->fFlatInterpolationSupport &&
                                           kQualcomm_GrGLVendor != ctxInfo.vendor();
    if (kGL_GrGLStandard == standard) {
        shaderCaps->fNoPerspectiveInterpolationSupport =
            ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
    } else {
        if (ctxInfo.hasExtension("GL_NV_shader_noperspective_interpolation")) {
            shaderCaps->fNoPerspectiveInterpolationSupport = true;
            shaderCaps->fNoPerspectiveInterpolationExtensionString =
                "GL_NV_shader_noperspective_interpolation";
        }
    }

    if (kGL_GrGLStandard == standard) {
        shaderCaps->fMultisampleInterpolationSupport =
                ctxInfo.glslGeneration() >= k400_GrGLSLGeneration;
    } else {
        if (ctxInfo.glslGeneration() >= k320es_GrGLSLGeneration) {
            shaderCaps->fMultisampleInterpolationSupport = true;
        } else if (ctxInfo.hasExtension("GL_OES_shader_multisample_interpolation")) {
            shaderCaps->fMultisampleInterpolationSupport = true;
            shaderCaps->fMultisampleInterpolationExtensionString =
                "GL_OES_shader_multisample_interpolation";
        }
    }

    if (kGL_GrGLStandard == standard) {
        shaderCaps->fSampleVariablesSupport = ctxInfo.glslGeneration() >= k400_GrGLSLGeneration;
    } else {
        if (ctxInfo.glslGeneration() >= k320es_GrGLSLGeneration) {
            shaderCaps->fSampleVariablesSupport = true;
        } else if (ctxInfo.hasExtension("GL_OES_sample_variables")) {
            shaderCaps->fSampleVariablesSupport = true;
            shaderCaps->fSampleVariablesExtensionString = "GL_OES_sample_variables";
        }
    }

    if (shaderCaps->fSampleVariablesSupport &&
        ctxInfo.hasExtension("GL_NV_sample_mask_override_coverage")) {
        shaderCaps->fSampleMaskOverrideCoverageSupport = true;
    }

    shaderCaps->fVersionDeclString = get_glsl_version_decl_string(standard,
                                                                  shaderCaps->fGLSLGeneration,
                                                                  fIsCoreProfile);

    if (kGLES_GrGLStandard == standard && k110_GrGLSLGeneration == shaderCaps->fGLSLGeneration) {
        shaderCaps->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
    }

    // Frag Coords Convention support is not part of ES
    if (kGLES_GrGLStandard != standard &&
        (ctxInfo.glslGeneration() >= k150_GrGLSLGeneration ||
         ctxInfo.hasExtension("GL_ARB_fragment_coord_conventions"))) {
        shaderCaps->fFragCoordConventionsExtensionString = "GL_ARB_fragment_coord_conventions";
    }

    if (kGLES_GrGLStandard == standard) {
        shaderCaps->fSecondaryOutputExtensionString = "GL_EXT_blend_func_extended";
    }

    if (ctxInfo.hasExtension("GL_OES_EGL_image_external")) {
        if (ctxInfo.glslGeneration() == k110_GrGLSLGeneration) {
            shaderCaps->fExternalTextureSupport = true;
        } else if (ctxInfo.hasExtension("GL_OES_EGL_image_external_essl3") ||
                   ctxInfo.hasExtension("OES_EGL_image_external_essl3")) {
            // At least one driver has been found that has this extension without the "GL_" prefix.
            shaderCaps->fExternalTextureSupport = true;
        }
    }

    if (shaderCaps->fExternalTextureSupport) {
        if (ctxInfo.glslGeneration() == k110_GrGLSLGeneration) {
            shaderCaps->fExternalTextureExtensionString = "GL_OES_EGL_image_external";
        } else {
            shaderCaps->fExternalTextureExtensionString = "GL_OES_EGL_image_external_essl3";
        }
    }

    if (kGL_GrGLStandard == standard) {
        shaderCaps->fTexelFetchSupport = ctxInfo.glslGeneration() >= k130_GrGLSLGeneration;
    } else {
        shaderCaps->fTexelFetchSupport =
            ctxInfo.glslGeneration() >= k330_GrGLSLGeneration; // We use this value for GLSL ES 3.0.
    }

    if (shaderCaps->fTexelFetchSupport) {
        if (kGL_GrGLStandard == standard) {
            shaderCaps->fTexelBufferSupport = ctxInfo.version() >= GR_GL_VER(3, 1) &&
                                            ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
        } else {
            if (ctxInfo.version() >= GR_GL_VER(3, 2) &&
                ctxInfo.glslGeneration() >= k320es_GrGLSLGeneration) {
                shaderCaps->fTexelBufferSupport = true;
            } else if (ctxInfo.hasExtension("GL_OES_texture_buffer")) {
                shaderCaps->fTexelBufferSupport = true;
                shaderCaps->fTexelBufferExtensionString = "GL_OES_texture_buffer";
            } else if (ctxInfo.hasExtension("GL_EXT_texture_buffer")) {
                shaderCaps->fTexelBufferSupport = true;
                shaderCaps->fTexelBufferExtensionString = "GL_EXT_texture_buffer";
            }
        }
    }

    if (kGL_GrGLStandard == standard) {
        shaderCaps->fVertexIDSupport = true;
    } else {
        // Desktop GLSL 3.30 == ES GLSL 3.00.
        shaderCaps->fVertexIDSupport = ctxInfo.glslGeneration() >= k330_GrGLSLGeneration;
    }

    shaderCaps->fFloatIs32Bits = is_float_fp32(ctxInfo, gli, GR_GL_HIGH_FLOAT);
    shaderCaps->fHalfIs32Bits = is_float_fp32(ctxInfo, gli, GR_GL_MEDIUM_FLOAT);
}

bool GrGLCaps::hasPathRenderingSupport(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli) {
    bool hasChromiumPathRendering = ctxInfo.hasExtension("GL_CHROMIUM_path_rendering");

    if (!(ctxInfo.hasExtension("GL_NV_path_rendering") || hasChromiumPathRendering)) {
        return false;
    }

    if (kGL_GrGLStandard == ctxInfo.standard()) {
        if (ctxInfo.version() < GR_GL_VER(4, 3) &&
            !ctxInfo.hasExtension("GL_ARB_program_interface_query")) {
            return false;
        }
    } else {
        if (!hasChromiumPathRendering &&
            ctxInfo.version() < GR_GL_VER(3, 1)) {
            return false;
        }
    }
    // We only support v1.3+ of GL_NV_path_rendering which allows us to
    // set individual fragment inputs with ProgramPathFragmentInputGen. The API
    // additions are detected by checking the existence of the function.
    // We also use *Then* functions that not all drivers might have. Check
    // them for consistency.
    if (!gli->fFunctions.fStencilThenCoverFillPath ||
        !gli->fFunctions.fStencilThenCoverStrokePath ||
        !gli->fFunctions.fStencilThenCoverFillPathInstanced ||
        !gli->fFunctions.fStencilThenCoverStrokePathInstanced ||
        !gli->fFunctions.fProgramPathFragmentInputGen) {
        return false;
    }
    return true;
}

bool GrGLCaps::readPixelsSupported(GrPixelConfig surfaceConfig,
                                   GrPixelConfig readConfig,
                                   std::function<void (GrGLenum, GrGLint*)> getIntegerv,
                                   std::function<bool ()> bindRenderTarget,
                                   std::function<void ()> unbindRenderTarget) const {
    // If it's not possible to even have a color attachment of surfaceConfig then read pixels is
    // not supported regardless of readConfig.
    if (!this->canConfigBeFBOColorAttachment(surfaceConfig)) {
        return false;
    }

    if (GrPixelConfigIsSint(surfaceConfig) != GrPixelConfigIsSint(readConfig)) {
        return false;
    }

    GrGLenum readFormat;
    GrGLenum readType;
    if (!this->getReadPixelsFormat(surfaceConfig, readConfig, &readFormat, &readType)) {
        return false;
    }

    if (kGL_GrGLStandard == fStandard) {
        // Some OpenGL implementations allow GL_ALPHA as a format to glReadPixels. However,
        // the manual (https://www.opengl.org/sdk/docs/man/) says only these formats are allowed:
        // GL_STENCIL_INDEX, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL, GL_RED, GL_GREEN, GL_BLUE,
        // GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA. We check for the subset that we would use.
        // The manual does not seem to fully match the spec as the spec allows integer formats
        // when the bound color buffer is an integer buffer. It doesn't specify which integer
        // formats are allowed, so perhaps all of them are. We only use GL_RGBA_INTEGER currently.
        if (readFormat != GR_GL_RED && readFormat != GR_GL_RG && readFormat != GR_GL_RGB &&
            readFormat != GR_GL_RGBA && readFormat != GR_GL_BGRA &&
            readFormat != GR_GL_RGBA_INTEGER) {
            return false;
        }
        // There is also a set of allowed types, but all the types we use are in the set:
        // GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT,
        // GL_HALF_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV,
        // GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4,
        // GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV,
        // GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV,GL_UNSIGNED_INT_10_10_10_2,
        // GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_24_8, GL_UNSIGNED_INT_10F_11F_11F_REV,
        // GL_UNSIGNED_INT_5_9_9_9_REV, or GL_FLOAT_32_UNSIGNED_INT_24_8_REV.
        return true;
    }

    // See Section 16.1.2 in the ES 3.2 specification.
    switch (fConfigTable[surfaceConfig].fFormatType) {
        case kNormalizedFixedPoint_FormatType:
            if (GR_GL_RGBA == readFormat && GR_GL_UNSIGNED_BYTE == readType) {
                return true;
            }
            break;
        case kInteger_FormatType:
            if (GR_GL_RGBA_INTEGER == readFormat && GR_GL_INT == readType) {
                return true;
            }
            break;
        case kFloat_FormatType:
            if (GR_GL_RGBA == readFormat && GR_GL_FLOAT == readType) {
                return true;
            }
            break;
    }

    if (0 == fConfigTable[surfaceConfig].fSecondReadPixelsFormat.fFormat) {
        ReadPixelsFormat* rpFormat =
            const_cast<ReadPixelsFormat*>(&fConfigTable[surfaceConfig].fSecondReadPixelsFormat);
        GrGLint format = 0, type = 0;
        if (!bindRenderTarget()) {
            return false;
        }
        getIntegerv(GR_GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
        getIntegerv(GR_GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
        rpFormat->fFormat = format;
        rpFormat->fType = type;
        unbindRenderTarget();
    }

    return fConfigTable[surfaceConfig].fSecondReadPixelsFormat.fFormat == readFormat &&
           fConfigTable[surfaceConfig].fSecondReadPixelsFormat.fType == readType;
}

void GrGLCaps::initFSAASupport(const GrContextOptions& contextOptions, const GrGLContextInfo& ctxInfo,
                               const GrGLInterface* gli) {
    // We need dual source blending and the ability to disable multisample in order to support mixed
    // samples in every corner case. We only use mixed samples if the stencil-and-cover path
    // renderer is available and enabled; no other path renderers support this feature.
    if (fMultisampleDisableSupport &&
        this->shaderCaps()->dualSourceBlendingSupport() &&
        this->shaderCaps()->pathRenderingSupport()
#if GR_TEST_UTILS
        && (contextOptions.fGpuPathRenderers & GpuPathRenderers::kStencilAndCover)
#endif
        ) {
        fUsesMixedSamples = ctxInfo.hasExtension("GL_NV_framebuffer_mixed_samples") ||
                            ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_mixed_samples");
    }

    if (kGL_GrGLStandard != ctxInfo.standard()) {
        if (ctxInfo.version() >= GR_GL_VER(3,0) &&
            ctxInfo.renderer() != kGalliumLLVM_GrGLRenderer) {
            // The gallium llvmpipe renderer for es3.0 does not have textureRed support even though
            // it is part of the spec. Thus alpha8 will not be renderable for those devices.
            fAlpha8IsRenderable = true;
        }
        // We prefer the EXT/IMG extension over ES3 MSAA because we've observed
        // ES3 driver bugs on at least one device with a tiled GPU (N10).
        if (ctxInfo.hasExtension("GL_EXT_multisampled_render_to_texture")) {
            fMSFBOType = kES_EXT_MsToTexture_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_IMG_multisampled_render_to_texture")) {
            fMSFBOType = kES_IMG_MsToTexture_MSFBOType;
        } else if (fUsesMixedSamples) {
            fMSFBOType = kMixedSamples_MSFBOType;
        } else if (ctxInfo.version() >= GR_GL_VER(3,0)) {
            fMSFBOType = kStandard_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_multisample")) {
            fMSFBOType = kStandard_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_ANGLE_framebuffer_multisample")) {
            fMSFBOType = kStandard_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_APPLE_framebuffer_multisample")) {
            fMSFBOType = kES_Apple_MSFBOType;
        }
    } else {
        if (fUsesMixedSamples) {
            fMSFBOType = kMixedSamples_MSFBOType;
        } else if (ctxInfo.version() >= GR_GL_VER(3,0) ||
                   ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {

            fMSFBOType = kStandard_MSFBOType;
            if (!fIsCoreProfile && ctxInfo.renderer() != kOSMesa_GrGLRenderer) {
                // Core profile removes ALPHA8 support.
                // OpenGL 3.0+ (and GL_ARB_framebuffer_object) supports ALPHA8 as renderable.
                // However, osmesa fails if it is used even when GL_ARB_framebuffer_object is
                // present.
                fAlpha8IsRenderable = true;
            }
        } else if (ctxInfo.hasExtension("GL_EXT_framebuffer_multisample") &&
                   ctxInfo.hasExtension("GL_EXT_framebuffer_blit")) {
            fMSFBOType = kStandard_MSFBOType;
        }
    }

    // We disable MSAA across the board for Intel GPUs for performance reasons.
    if (kIntel_GrGLVendor == ctxInfo.vendor()) {
        fMSFBOType = kNone_MSFBOType;
    }

    // We only have a use for raster multisample if there is coverage modulation from mixed samples.
    if (fUsesMixedSamples && ctxInfo.hasExtension("GL_EXT_raster_multisample")) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_RASTER_SAMPLES, &fMaxRasterSamples);
    }
}

void GrGLCaps::initBlendEqationSupport(const GrGLContextInfo& ctxInfo) {
    GrShaderCaps* shaderCaps = static_cast<GrShaderCaps*>(fShaderCaps.get());

    bool layoutQualifierSupport = false;
    if ((kGL_GrGLStandard == fStandard && shaderCaps->generation() >= k140_GrGLSLGeneration)  ||
        (kGLES_GrGLStandard == fStandard && shaderCaps->generation() >= k330_GrGLSLGeneration)) {
        layoutQualifierSupport = true;
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
        // TODO: Use kSpecificEnables_AdvBlendEqInteraction if "blend_support_all_equations" is
        // slow on a particular platform.
    }
}

namespace {
const GrGLuint kUnknownBitCount = GrGLStencilAttachment::kUnknownBitCount;
}

void GrGLCaps::initStencilSupport(const GrGLContextInfo& ctxInfo) {

    // Build up list of legal stencil formats (though perhaps not supported on
    // the particular gpu/driver) from most preferred to least.

    // these consts are in order of most preferred to least preferred
    // we don't bother with GL_STENCIL_INDEX1 or GL_DEPTH32F_STENCIL8

    static const StencilFormat
                  // internal Format      stencil bits      total bits        packed?
        gS8    = {GR_GL_STENCIL_INDEX8,   8,                8,                false},
        gS16   = {GR_GL_STENCIL_INDEX16,  16,               16,               false},
        gD24S8 = {GR_GL_DEPTH24_STENCIL8, 8,                32,               true },
        gS4    = {GR_GL_STENCIL_INDEX4,   4,                4,                false},
    //  gS     = {GR_GL_STENCIL_INDEX,    kUnknownBitCount, kUnknownBitCount, false},
        gDS    = {GR_GL_DEPTH_STENCIL,    kUnknownBitCount, kUnknownBitCount, true };

    if (kGL_GrGLStandard == ctxInfo.standard()) {
        bool supportsPackedDS =
            ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_EXT_packed_depth_stencil") ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object");

        // S1 thru S16 formats are in GL 3.0+, EXT_FBO, and ARB_FBO since we
        // require FBO support we can expect these are legal formats and don't
        // check. These also all support the unsized GL_STENCIL_INDEX.
        fStencilFormats.push_back() = gS8;
        fStencilFormats.push_back() = gS16;
        if (supportsPackedDS) {
            fStencilFormats.push_back() = gD24S8;
        }
        fStencilFormats.push_back() = gS4;
        if (supportsPackedDS) {
            fStencilFormats.push_back() = gDS;
        }
    } else {
        // ES2 has STENCIL_INDEX8 without extensions but requires extensions
        // for other formats.
        // ES doesn't support using the unsized format.

        fStencilFormats.push_back() = gS8;
        //fStencilFormats.push_back() = gS16;
        if (ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_OES_packed_depth_stencil")) {
            fStencilFormats.push_back() = gD24S8;
        }
        if (ctxInfo.hasExtension("GL_OES_stencil4")) {
            fStencilFormats.push_back() = gS4;
        }
    }
}

void GrGLCaps::onDumpJSON(SkJSONWriter* writer) const {

    // We are called by the base class, which has already called beginObject(). We choose to nest
    // all of our caps information in a named sub-object.
    writer->beginObject("GL caps");

    writer->beginArray("Stencil Formats");

    for (int i = 0; i < fStencilFormats.count(); ++i) {
        writer->beginObject(nullptr, false);
        writer->appendS32("stencil bits", fStencilFormats[i].fStencilBits);
        writer->appendS32("total bits", fStencilFormats[i].fTotalBits);
        writer->endObject();
    }

    writer->endArray();

    static const char* kMSFBOExtStr[] = {
        "None",
        "Standard",
        "Apple",
        "IMG MS To Texture",
        "EXT MS To Texture",
        "MixedSamples",
    };
    GR_STATIC_ASSERT(0 == kNone_MSFBOType);
    GR_STATIC_ASSERT(1 == kStandard_MSFBOType);
    GR_STATIC_ASSERT(2 == kES_Apple_MSFBOType);
    GR_STATIC_ASSERT(3 == kES_IMG_MsToTexture_MSFBOType);
    GR_STATIC_ASSERT(4 == kES_EXT_MsToTexture_MSFBOType);
    GR_STATIC_ASSERT(5 == kMixedSamples_MSFBOType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kMSFBOExtStr) == kLast_MSFBOType + 1);

    static const char* kInvalidateFBTypeStr[] = {
        "None",
        "Discard",
        "Invalidate",
    };
    GR_STATIC_ASSERT(0 == kNone_InvalidateFBType);
    GR_STATIC_ASSERT(1 == kDiscard_InvalidateFBType);
    GR_STATIC_ASSERT(2 == kInvalidate_InvalidateFBType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kInvalidateFBTypeStr) == kLast_InvalidateFBType + 1);

    static const char* kMapBufferTypeStr[] = {
        "None",
        "MapBuffer",
        "MapBufferRange",
        "Chromium",
    };
    GR_STATIC_ASSERT(0 == kNone_MapBufferType);
    GR_STATIC_ASSERT(1 == kMapBuffer_MapBufferType);
    GR_STATIC_ASSERT(2 == kMapBufferRange_MapBufferType);
    GR_STATIC_ASSERT(3 == kChromium_MapBufferType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kMapBufferTypeStr) == kLast_MapBufferType + 1);

    writer->appendBool("Core Profile", fIsCoreProfile);
    writer->appendString("MSAA Type", kMSFBOExtStr[fMSFBOType]);
    writer->appendString("Invalidate FB Type", kInvalidateFBTypeStr[fInvalidateFBType]);
    writer->appendString("Map Buffer Type", kMapBufferTypeStr[fMapBufferType]);
    writer->appendS32("Max FS Uniform Vectors", fMaxFragmentUniformVectors);
    writer->appendBool("Unpack Row length support", fUnpackRowLengthSupport);
    writer->appendBool("Unpack Flip Y support", fUnpackFlipYSupport);
    writer->appendBool("Pack Row length support", fPackRowLengthSupport);
    writer->appendBool("Pack Flip Y support", fPackFlipYSupport);

    writer->appendBool("Texture Usage support", fTextureUsageSupport);
    writer->appendBool("Alpha8 is renderable", fAlpha8IsRenderable);
    writer->appendBool("GL_ARB_imaging support", fImagingSupport);
    writer->appendBool("Vertex array object support", fVertexArrayObjectSupport);
    writer->appendBool("Direct state access support", fDirectStateAccessSupport);
    writer->appendBool("Debug support", fDebugSupport);
    writer->appendBool("Draw indirect support", fDrawIndirectSupport);
    writer->appendBool("Multi draw indirect support", fMultiDrawIndirectSupport);
    writer->appendBool("Base instance support", fBaseInstanceSupport);
    writer->appendBool("RGBA 8888 pixel ops are slow", fRGBA8888PixelsOpsAreSlow);
    writer->appendBool("Partial FBO read is slow", fPartialFBOReadIsSlow);
    writer->appendBool("Bind uniform location support", fBindUniformLocationSupport);
    writer->appendBool("Rectangle texture support", fRectangleTextureSupport);
    writer->appendBool("Texture swizzle support", fTextureSwizzleSupport);
    writer->appendBool("BGRA to RGBA readback conversions are slow",
                       fRGBAToBGRAReadbackConversionsAreSlow);
    writer->appendBool("Draw To clear color", fUseDrawToClearColor);
    writer->appendBool("Draw To clear stencil clip", fUseDrawToClearStencilClip);
    writer->appendBool("Intermediate texture for partial updates of unorm textures ever bound to FBOs",
                       fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO);
    writer->appendBool("Intermediate texture for all updates of textures bound to FBOs",
                       fUseDrawInsteadOfAllRenderTargetWrites);
    writer->appendBool("Max instances per glDrawArraysInstanced without crashing (or zero)",
                       fMaxInstancesPerDrawArraysWithoutCrashing);

    writer->beginArray("configs");

    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        writer->beginObject(nullptr, false);
        writer->appendHexU32("flags", fConfigTable[i].fFlags);
        writer->appendHexU32("b_internal", fConfigTable[i].fFormats.fBaseInternalFormat);
        writer->appendHexU32("s_internal", fConfigTable[i].fFormats.fSizedInternalFormat);
        writer->appendHexU32("e_format",
                             fConfigTable[i].fFormats.fExternalFormat[kOther_ExternalFormatUsage]);
        writer->appendHexU32(
                "e_format_teximage",
                fConfigTable[i].fFormats.fExternalFormat[kTexImage_ExternalFormatUsage]);
        writer->appendHexU32("e_type", fConfigTable[i].fFormats.fExternalType);
        writer->appendHexU32("i_for_teximage", fConfigTable[i].fFormats.fInternalFormatTexImage);
        writer->appendHexU32("i_for_renderbuffer",
                             fConfigTable[i].fFormats.fInternalFormatRenderbuffer);
        writer->endObject();
    }

    writer->endArray();
    writer->endObject();
}

bool GrGLCaps::bgraIsInternalFormat() const {
    return fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fBaseInternalFormat == GR_GL_BGRA;
}

bool GrGLCaps::getTexImageFormats(GrPixelConfig surfaceConfig, GrPixelConfig externalConfig,
                                  GrGLenum* internalFormat, GrGLenum* externalFormat,
                                  GrGLenum* externalType) const {
    if (!this->getExternalFormat(surfaceConfig, externalConfig, kTexImage_ExternalFormatUsage,
                                 externalFormat, externalType)) {
        return false;
    }
    *internalFormat = fConfigTable[surfaceConfig].fFormats.fInternalFormatTexImage;
    return true;
}

bool GrGLCaps::getReadPixelsFormat(GrPixelConfig surfaceConfig, GrPixelConfig externalConfig,
                                   GrGLenum* externalFormat, GrGLenum* externalType) const {
    if (!this->getExternalFormat(surfaceConfig, externalConfig, kOther_ExternalFormatUsage,
                                 externalFormat, externalType)) {
        return false;
    }
    return true;
}

bool GrGLCaps::getRenderbufferFormat(GrPixelConfig config, GrGLenum* internalFormat) const {
    *internalFormat = fConfigTable[config].fFormats.fInternalFormatRenderbuffer;
    return true;
}

bool GrGLCaps::getExternalFormat(GrPixelConfig surfaceConfig, GrPixelConfig memoryConfig,
                                 ExternalFormatUsage usage, GrGLenum* externalFormat,
                                 GrGLenum* externalType) const {
    SkASSERT(externalFormat && externalType);

    bool surfaceIsAlphaOnly = GrPixelConfigIsAlphaOnly(surfaceConfig);
    bool memoryIsAlphaOnly = GrPixelConfigIsAlphaOnly(memoryConfig);

    // We don't currently support moving RGBA data into and out of ALPHA surfaces. It could be
    // made to work in many cases using glPixelStore and what not but is not needed currently.
    if (surfaceIsAlphaOnly && !memoryIsAlphaOnly) {
        return false;
    }

    *externalFormat = fConfigTable[memoryConfig].fFormats.fExternalFormat[usage];
    *externalType = fConfigTable[memoryConfig].fFormats.fExternalType;

    // When GL_RED is supported as a texture format, our alpha-only textures are stored using
    // GL_RED and we swizzle in order to map all components to 'r'. However, in this case the
    // surface is not alpha-only and we want alpha to really mean the alpha component of the
    // texture, not the red component.
    if (memoryIsAlphaOnly && !surfaceIsAlphaOnly) {
        if (GR_GL_RED == *externalFormat) {
            *externalFormat = GR_GL_ALPHA;
        }
    }

    return true;
}

void GrGLCaps::initConfigTable(const GrContextOptions& contextOptions,
                               const GrGLContextInfo& ctxInfo, const GrGLInterface* gli,
                               GrShaderCaps* shaderCaps) {
    /*
        Comments on renderability of configs on various GL versions.
          OpenGL < 3.0:
            no built in support for render targets.
            GL_EXT_framebuffer_object adds possible support for any sized format with base internal
              format RGB, RGBA and NV float formats we don't use.
              This is the following:
                R3_G3_B2, RGB4, RGB5, RGB8, RGB10, RGB12, RGB16, RGBA2, RGBA4, RGB5_A1, RGBA8
                RGB10_A2, RGBA12,RGBA16
              Though, it is hard to believe the more obscure formats such as RGBA12 would work
              since they aren't required by later standards and the driver can simply return
              FRAMEBUFFER_UNSUPPORTED for anything it doesn't allow.
            GL_ARB_framebuffer_object adds everything added by the EXT extension and additionally
              any sized internal format with a base internal format of ALPHA, LUMINANCE,
              LUMINANCE_ALPHA, INTENSITY, RED, and RG.
              This adds a lot of additional renderable sized formats, including ALPHA8.
              The GL_ARB_texture_rg brings in the RED and RG formats (8, 8I, 8UI, 16, 16I, 16UI,
              16F, 32I, 32UI, and 32F variants).
              Again, the driver has an escape hatch via FRAMEBUFFER_UNSUPPORTED.

            For both the above extensions we limit ourselves to those that are also required by
            OpenGL 3.0.

          OpenGL 3.0:
            Any format with base internal format ALPHA, RED, RG, RGB or RGBA is "color-renderable"
            but are not required to be supported as renderable textures/renderbuffer.
            Required renderable color formats:
                - RGBA32F, RGBA32I, RGBA32UI, RGBA16, RGBA16F, RGBA16I,
                  RGBA16UI, RGBA8, RGBA8I, RGBA8UI, SRGB8_ALPHA8, and
                  RGB10_A2.
                - R11F_G11F_B10F.
                - RG32F, RG32I, RG32UI, RG16, RG16F, RG16I, RG16UI, RG8, RG8I,
                  and RG8UI.
                - R32F, R32I, R32UI, R16F, R16I, R16UI, R16, R8, R8I, and R8UI.
                - ALPHA8

          OpenGL 3.1, 3.2, 3.3
            Same as 3.0 except ALPHA8 requires GL_ARB_compatibility/compatibility profile.
          OpengGL 3.3, 4.0, 4.1
            Adds RGB10_A2UI.
          OpengGL 4.2
            Adds
                - RGB5_A1, RGBA4
                - RGB565
          OpenGL 4.4
            Does away with the separate list and adds a column to the sized internal color format
            table. However, no new formats become required color renderable.

          ES 2.0
            color renderable: RGBA4, RGB5_A1, RGB565
            GL_EXT_texture_rg adds support for R8, RG5 as a color render target
            GL_OES_rgb8_rgba8 adds support for RGB8 and RGBA8
            GL_ARM_rgba8 adds support for RGBA8 (but not RGB8)
            GL_EXT_texture_format_BGRA8888 does not add renderbuffer support
            GL_CHROMIUM_renderbuffer_format_BGRA8888 adds BGRA8 as color-renderable
            GL_APPLE_texture_format_BGRA8888 does not add renderbuffer support

          ES 3.0
                - RGBA32I, RGBA32UI, RGBA16I, RGBA16UI, RGBA8, RGBA8I,
                  RGBA8UI, SRGB8_ALPHA8, RGB10_A2, RGB10_A2UI, RGBA4, and
                  RGB5_A1.
                - RGB8 and RGB565.
                - RG32I, RG32UI, RG16I, RG16UI, RG8, RG8I, and RG8UI.
                - R32I, R32UI, R16I, R16UI, R8, R8I, and R8UI
          ES 3.1
            Adds RGB10_A2, RGB10_A2UI,
          ES 3.2
            Adds R16F, RG16F, RGBA16F, R32F, RG32F, RGBA32F, R11F_G11F_B10F.
    */

    // Correctness workarounds.
    bool disableTextureRedForMesa = false;
    bool disableSRGBForX86PowerVR = false;
    bool disableSRGBWriteControlForAdreno4xx = false;
    bool disableR8TexStorageForANGLEGL = false;
    bool disableSRGBRenderWithMSAAForMacAMD = false;

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        // ARB_texture_rg is part of OpenGL 3.0, but osmesa doesn't support GL_RED
        // and GL_RG on FBO textures.
        disableTextureRedForMesa = kOSMesa_GrGLRenderer == ctxInfo.renderer();

        bool isX86PowerVR = false;
#if defined(SK_CPU_X86)
        if (kPowerVRRogue_GrGLRenderer == ctxInfo.renderer()) {
            isX86PowerVR = true;
        }
#endif
        // NexusPlayer has strange bugs with sRGB (skbug.com/4148). This is a targeted fix to
        // blacklist that device (and any others that might be sharing the same driver).
        disableSRGBForX86PowerVR = isX86PowerVR;
        disableSRGBWriteControlForAdreno4xx = kAdreno4xx_GrGLRenderer == ctxInfo.renderer();

        // Angle with es2->GL has a bug where it will hang trying to call TexSubImage on GL_R8
        // formats on miplevels > 0. We already disable texturing on gles > 2.0 so just need to
        // check that we are not going to OpenGL.
        disableR8TexStorageForANGLEGL = GrGLANGLEBackend::kOpenGL == ctxInfo.angleBackend();

        // MacPro devices with AMD cards fail to create MSAA sRGB render buffers.
#if defined(SK_BUILD_FOR_MAC)
        disableSRGBRenderWithMSAAForMacAMD = kATI_GrGLVendor == ctxInfo.vendor();
#endif
    }

    uint32_t nonMSAARenderFlags = ConfigInfo::kRenderable_Flag |
                                  ConfigInfo::kFBOColorAttachment_Flag;
    uint32_t allRenderFlags = nonMSAARenderFlags;
    if (kNone_MSFBOType != fMSFBOType) {
        allRenderFlags |= ConfigInfo::kRenderableWithMSAA_Flag;
    }
    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

    bool texStorageSupported = false;
    if (kGL_GrGLStandard == standard) {
        // The EXT version can apply to either GL or GLES.
        texStorageSupported = version >= GR_GL_VER(4,2) ||
                              ctxInfo.hasExtension("GL_ARB_texture_storage") ||
                              ctxInfo.hasExtension("GL_EXT_texture_storage");
    } else {
        texStorageSupported = version >= GR_GL_VER(3,0) ||
                              ctxInfo.hasExtension("GL_EXT_texture_storage");
    }

    bool texelBufferSupport = this->shaderCaps()->texelBufferSupport();

    bool textureRedSupport = false;

    if (!disableTextureRedForMesa) {
        if (kGL_GrGLStandard == standard) {
            textureRedSupport =
                    version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_texture_rg");
        } else {
            textureRedSupport =
                    version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_EXT_texture_rg");
        }
    }

    fConfigTable[kUnknown_GrPixelConfig].fFormats.fBaseInternalFormat = 0;
    fConfigTable[kUnknown_GrPixelConfig].fFormats.fSizedInternalFormat = 0;
    fConfigTable[kUnknown_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] = 0;
    fConfigTable[kUnknown_GrPixelConfig].fFormats.fExternalType = 0;
    fConfigTable[kUnknown_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    fConfigTable[kUnknown_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    fConfigTable[kRGBA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGBA;
    fConfigTable[kRGBA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA8;
    fConfigTable[kRGBA_8888_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_RGBA;
    fConfigTable[kRGBA_8888_GrPixelConfig].fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    fConfigTable[kRGBA_8888_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    fConfigTable[kRGBA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
    if (kGL_GrGLStandard == standard) {
        // We require some form of FBO support and all GLs with FBO support can render to RGBA8
        fConfigTable[kRGBA_8888_GrPixelConfig].fFlags |= allRenderFlags;
    } else {
        if (version >= GR_GL_VER(3,0) || ctxInfo.hasExtension("GL_OES_rgb8_rgba8") ||
            ctxInfo.hasExtension("GL_ARM_rgba8")) {
            fConfigTable[kRGBA_8888_GrPixelConfig].fFlags |= allRenderFlags;
        }
    }
    if (texStorageSupported) {
        fConfigTable[kRGBA_8888_GrPixelConfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }
    if (texelBufferSupport) {
        fConfigTable[kRGBA_8888_GrPixelConfig].fFlags |= ConfigInfo::kCanUseWithTexelBuffer_Flag;
    }
    fConfigTable[kRGBA_8888_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_BGRA;
    fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fExternalType  = GR_GL_UNSIGNED_BYTE;
    fConfigTable[kBGRA_8888_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;

   // TexStorage requires using a sized internal format and BGRA8 is only supported if we have the
   // GL_APPLE_texture_format_BGRA8888 extension or if we have GL_EXT_texutre_storage and
   // GL_EXT_texture_format_BGRA8888.
    bool supportsBGRATexStorage = false;

    if (kGL_GrGLStandard == standard) {
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGBA;
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA8;
        if (version >= GR_GL_VER(1, 2) || ctxInfo.hasExtension("GL_EXT_bgra")) {
            // Since the internal format is RGBA8, it is also renderable.
            fConfigTable[kBGRA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                            allRenderFlags;
        }
        // Since we are using RGBA8 we can use tex storage.
        supportsBGRATexStorage = true;
    } else {
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_BGRA;
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_BGRA8;
        if (ctxInfo.hasExtension("GL_EXT_texture_format_BGRA8888")) {
            fConfigTable[kBGRA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                            nonMSAARenderFlags;

            if (ctxInfo.hasExtension("GL_EXT_texture_storage")) {
                supportsBGRATexStorage = true;
            }
            if (ctxInfo.hasExtension("GL_CHROMIUM_renderbuffer_format_BGRA8888") &&
                (this->usesMSAARenderBuffers() || this->fMSFBOType == kMixedSamples_MSFBOType)) {
                fConfigTable[kBGRA_8888_GrPixelConfig].fFlags |=
                    ConfigInfo::kRenderableWithMSAA_Flag;
            }
        } else if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            // This APPLE extension introduces complexity on ES2. It leaves the internal format
            // as RGBA, but allows BGRA as the external format. From testing, it appears that the
            // driver remembers the external format when the texture is created (with TexImage).
            // If you then try to upload data in the other swizzle (with TexSubImage), it fails.
            // We could work around this, but it adds even more state tracking to code that is
            // already too tricky. Instead, we opt not to support BGRA on ES2 with this extension.
            // This also side-steps some ambiguous interactions with the texture storage extension.
            if (version >= GR_GL_VER(3,0)) {
                // The APPLE extension doesn't make this renderable.
                fConfigTable[kBGRA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
                supportsBGRATexStorage = true;
            }
        }
    }

    if (texStorageSupported && supportsBGRATexStorage) {
        fConfigTable[kBGRA_8888_GrPixelConfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }
    fConfigTable[kBGRA_8888_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    // We only enable srgb support if both textures and FBOs support srgb,
    // *and* we can disable sRGB decode-on-read, to support "legacy" mode.
    if (kGL_GrGLStandard == standard) {
        if (ctxInfo.version() >= GR_GL_VER(3,0)) {
            fSRGBSupport = true;
        } else if (ctxInfo.hasExtension("GL_EXT_texture_sRGB")) {
            if (ctxInfo.hasExtension("GL_ARB_framebuffer_sRGB") ||
                ctxInfo.hasExtension("GL_EXT_framebuffer_sRGB")) {
                fSRGBSupport = true;
            }
        }
        // All the above srgb extensions support toggling srgb writes
        if (fSRGBSupport) {
            fSRGBWriteControl = true;
        }
    } else {
        fSRGBSupport = ctxInfo.version() >= GR_GL_VER(3,0) || ctxInfo.hasExtension("GL_EXT_sRGB");
        if (disableSRGBForX86PowerVR) {
            fSRGBSupport = false;
        }
        // ES through 3.1 requires EXT_srgb_write_control to support toggling
        // sRGB writing for destinations.
        // See https://bug.skia.org/5329 for Adreno4xx issue.
        fSRGBWriteControl = !disableSRGBWriteControlForAdreno4xx &&
            ctxInfo.hasExtension("GL_EXT_sRGB_write_control");
    }
    if (contextOptions.fRequireDecodeDisableForSRGB && !fSRGBDecodeDisableSupport) {
        // To support "legacy" L32 mode, we require the ability to turn off sRGB decode. Clients
        // can opt-out of that requirement, if they intend to always do linear blending.
        fSRGBSupport = false;
    }

    // This is very conservative, if we're on a platform where N32 is BGRA, and using ES, disable
    // all sRGB support. Too much code relies on creating surfaces with N32 + sRGB colorspace,
    // and sBGRA is basically impossible to support on any version of ES (with our current code).
    // In particular, ES2 doesn't support sBGRA at all, and even in ES3, there is no valid pair
    // of formats that can be used for TexImage calls to upload BGRA data to sRGBA (which is what
    // we *have* to use as the internal format, because sBGRA doesn't exist). This primarily
    // affects Windows.
    if (kSkia8888_GrPixelConfig == kBGRA_8888_GrPixelConfig && kGLES_GrGLStandard == standard) {
        fSRGBSupport = false;
    }

    // ES2 Command Buffer has several TexStorage restrictions. It appears to fail for any format
    // not explicitly allowed by GL_EXT_texture_storage, particularly those from other extensions.
    bool isCommandBufferES2 = kChromium_GrGLDriver == ctxInfo.driver() && version < GR_GL_VER(3, 0);

    uint32_t srgbRenderFlags = allRenderFlags;
    if (disableSRGBRenderWithMSAAForMacAMD) {
        srgbRenderFlags &= ~ConfigInfo::kRenderableWithMSAA_Flag;
    }

    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_SRGB_ALPHA;
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_SRGB8_ALPHA8;
    // GL does not do srgb<->rgb conversions when transferring between cpu and gpu. Thus, the
    // external format is GL_RGBA. See below for note about ES2.0 and glTex[Sub]Image.
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_RGBA;
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    if (fSRGBSupport) {
        fConfigTable[kSRGBA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                         srgbRenderFlags;
    }
    // ES2 Command Buffer does not allow TexStorage with SRGB8_ALPHA8_EXT
    if (texStorageSupported && !isCommandBufferES2) {
        fConfigTable[kSRGBA_8888_GrPixelConfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }
    fConfigTable[kSRGBA_8888_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();
    // sBGRA is not a "real" thing in OpenGL, but GPUs support it, and on platforms where
    // kN32 == BGRA, we need some way to work with it. (The default framebuffer on Windows
    // is in this format, for example).
    fConfigTable[kSBGRA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_SRGB_ALPHA;
    fConfigTable[kSBGRA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_SRGB8_ALPHA8;
    // GL does not do srgb<->rgb conversions when transferring between cpu and gpu. Thus, the
    // external format is GL_BGRA.
    fConfigTable[kSBGRA_8888_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_BGRA;
    fConfigTable[kSBGRA_8888_GrPixelConfig].fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    fConfigTable[kSBGRA_8888_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    if (fSRGBSupport && kGL_GrGLStandard == standard) {
        fConfigTable[kSBGRA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                         srgbRenderFlags;
    }

    if (texStorageSupported) {
        fConfigTable[kSBGRA_8888_GrPixelConfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }
    fConfigTable[kSBGRA_8888_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    bool hasIntegerTextures;
    if (standard == kGL_GrGLStandard) {
        hasIntegerTextures = version >= GR_GL_VER(3, 0) ||
                             ctxInfo.hasExtension("GL_EXT_texture_integer");
    } else {
        hasIntegerTextures = (version >= GR_GL_VER(3, 0));
    }
    // We may have limited GLSL to an earlier version that doesn't have integer sampler types.
    if (ctxInfo.glslGeneration() == k110_GrGLSLGeneration) {
        hasIntegerTextures = false;
    }
    fConfigTable[kRGBA_8888_sint_GrPixelConfig].fFormats.fBaseInternalFormat  = GR_GL_RGBA_INTEGER;
    fConfigTable[kRGBA_8888_sint_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA8I;
    fConfigTable[kRGBA_8888_sint_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] = GR_GL_RGBA_INTEGER;
    fConfigTable[kRGBA_8888_sint_GrPixelConfig].fFormats.fExternalType = GR_GL_BYTE;
    fConfigTable[kRGBA_8888_sint_GrPixelConfig].fFormatType = kInteger_FormatType;
    // We currently only support using integer textures as srcs, not for rendering (even though GL
    // allows it).
    if (hasIntegerTextures) {
        fConfigTable[kRGBA_8888_sint_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                             ConfigInfo::kFBOColorAttachment_Flag;
        if (texStorageSupported) {
            fConfigTable[kRGBA_8888_sint_GrPixelConfig].fFlags |=
                ConfigInfo::kCanUseTexStorage_Flag;
        }
    }

    fConfigTable[kRGB_565_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGB;
    if (this->ES2CompatibilitySupport()) {
        fConfigTable[kRGB_565_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGB565;
    } else {
        fConfigTable[kRGB_565_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGB5;
    }
    fConfigTable[kRGB_565_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_RGB;
    fConfigTable[kRGB_565_GrPixelConfig].fFormats.fExternalType = GR_GL_UNSIGNED_SHORT_5_6_5;
    fConfigTable[kRGB_565_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    fConfigTable[kRGB_565_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(4, 2) || ctxInfo.hasExtension("GL_ARB_ES2_compatibility")) {
            fConfigTable[kRGB_565_GrPixelConfig].fFlags |= allRenderFlags;
        }
    } else {
        fConfigTable[kRGB_565_GrPixelConfig].fFlags |= allRenderFlags;
    }
    // 565 is not a sized internal format on desktop GL. So on desktop with
    // 565 we always use an unsized internal format to let the system pick
    // the best sized format to convert the 565 data to. Since TexStorage
    // only allows sized internal formats we disallow it.
    //
    // TODO: As of 4.2, regular GL supports 565. This logic is due for an
    // update.
    if (texStorageSupported && kGL_GrGLStandard != standard) {
        fConfigTable[kRGB_565_GrPixelConfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }
    fConfigTable[kRGB_565_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    fConfigTable[kRGBA_4444_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGBA;
    fConfigTable[kRGBA_4444_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA4;
    fConfigTable[kRGBA_4444_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_RGBA;
    fConfigTable[kRGBA_4444_GrPixelConfig].fFormats.fExternalType = GR_GL_UNSIGNED_SHORT_4_4_4_4;
    fConfigTable[kRGBA_4444_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    fConfigTable[kRGBA_4444_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(4, 2)) {
            fConfigTable[kRGBA_4444_GrPixelConfig].fFlags |= allRenderFlags;
        }
    } else {
        fConfigTable[kRGBA_4444_GrPixelConfig].fFlags |= allRenderFlags;
    }
    if (texStorageSupported) {
        fConfigTable[kRGBA_4444_GrPixelConfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }
    fConfigTable[kRGBA_4444_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    bool alpha8IsValidForGL = kGL_GrGLStandard == standard &&
            (!fIsCoreProfile || version <= GR_GL_VER(3, 0));

    ConfigInfo& alphaInfo = fConfigTable[kAlpha_8_as_Alpha_GrPixelConfig];
    alphaInfo.fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    alphaInfo.fFormatType = kNormalizedFixedPoint_FormatType;
    if (alpha8IsValidForGL || (kGL_GrGLStandard != standard && version < GR_GL_VER(3, 0))) {
        alphaInfo.fFlags = ConfigInfo::kTextureable_Flag;
    }
    alphaInfo.fFormats.fBaseInternalFormat = GR_GL_ALPHA;
    alphaInfo.fFormats.fSizedInternalFormat = GR_GL_ALPHA8;
    alphaInfo.fFormats.fExternalFormat[kOther_ExternalFormatUsage] = GR_GL_ALPHA;
    alphaInfo.fSwizzle = GrSwizzle::AAAA();
    if (fAlpha8IsRenderable && alpha8IsValidForGL) {
        alphaInfo.fFlags |= allRenderFlags;
    }

    ConfigInfo& redInfo = fConfigTable[kAlpha_8_as_Red_GrPixelConfig];
    redInfo.fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    redInfo.fFormatType = kNormalizedFixedPoint_FormatType;
    redInfo.fFormats.fBaseInternalFormat = GR_GL_RED;
    redInfo.fFormats.fSizedInternalFormat = GR_GL_R8;
    redInfo.fFormats.fExternalFormat[kOther_ExternalFormatUsage] = GR_GL_RED;
    redInfo.fSwizzle = GrSwizzle::RRRR();

    // ES2 Command Buffer does not allow TexStorage with R8_EXT (so Alpha_8 and Gray_8)
    if (texStorageSupported && !isCommandBufferES2) {
        if (!disableR8TexStorageForANGLEGL) {
            alphaInfo.fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
        }
        redInfo.fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }

    if (textureRedSupport) {
        redInfo.fFlags |= ConfigInfo::kTextureable_Flag | allRenderFlags;
        if (texelBufferSupport) {
            redInfo.fFlags |= ConfigInfo::kCanUseWithTexelBuffer_Flag;
        }

        fConfigTable[kAlpha_8_GrPixelConfig] = redInfo;
    } else {
        redInfo.fFlags = 0;

        fConfigTable[kAlpha_8_GrPixelConfig] = alphaInfo;
    }

    ConfigInfo& grayLumInfo = fConfigTable[kGray_8_as_Lum_GrPixelConfig];
    grayLumInfo.fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    grayLumInfo.fFormatType = kNormalizedFixedPoint_FormatType;
    grayLumInfo.fFormats.fBaseInternalFormat = GR_GL_LUMINANCE;
    grayLumInfo.fFormats.fSizedInternalFormat = GR_GL_LUMINANCE8;
    grayLumInfo.fFormats.fExternalFormat[kOther_ExternalFormatUsage] = GR_GL_LUMINANCE;
    grayLumInfo.fSwizzle = GrSwizzle::RGBA();
    if ((standard == kGL_GrGLStandard && version <= GR_GL_VER(3, 0)) ||
        (standard == kGLES_GrGLStandard && version < GR_GL_VER(3, 0))) {
        grayLumInfo.fFlags = ConfigInfo::kTextureable_Flag;
    }

    ConfigInfo& grayRedInfo = fConfigTable[kGray_8_as_Red_GrPixelConfig];
    grayRedInfo.fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    grayRedInfo.fFormatType = kNormalizedFixedPoint_FormatType;
    grayRedInfo.fFormats.fBaseInternalFormat = GR_GL_RED;
    grayRedInfo.fFormats.fSizedInternalFormat = GR_GL_R8;
    grayRedInfo.fFormats.fExternalFormat[kOther_ExternalFormatUsage] = GR_GL_RED;
    grayRedInfo.fSwizzle = GrSwizzle::RRRA();
    grayRedInfo.fFlags = ConfigInfo::kTextureable_Flag;

#if 0 // Leaving Gray8 as non-renderable, to keep things simple and match raster. Needs to be
      // updated to support Gray8_as_Lum and Gray8_as_red if this is ever enabled.
    if (this->textureRedSupport() ||
        (kDesktop_ARB_MSFBOType == this->msFBOType() &&
         ctxInfo.renderer() != kOSMesa_GrGLRenderer)) {
        // desktop ARB extension/3.0+ supports LUMINANCE8 as renderable.
        // However, osmesa fails if it used even when GL_ARB_framebuffer_object is present.
        // Core profile removes LUMINANCE8 support, but we should have chosen R8 in that case.
        fConfigTable[kGray_8_GrPixelConfig].fFlags |= allRenderFlags;
    }
#endif
    if (texStorageSupported && !isCommandBufferES2) {
        if (!disableR8TexStorageForANGLEGL) {
            grayLumInfo.fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
        }
        grayRedInfo.fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }

    if (textureRedSupport) {
        if (texelBufferSupport) {
            grayRedInfo.fFlags |= ConfigInfo::kCanUseWithTexelBuffer_Flag;
        }
        fConfigTable[kGray_8_GrPixelConfig] = grayRedInfo;
    } else {
        grayRedInfo.fFlags = 0;
        fConfigTable[kGray_8_GrPixelConfig] = grayLumInfo;
    }

    // Check for [half] floating point texture support
    // NOTE: We disallow floating point textures on ES devices if linear filtering modes are not
    // supported. This is for simplicity, but a more granular approach is possible. Coincidentally,
    // [half] floating point textures became part of the standard in ES3.1 / OGL 3.0.
    bool hasFPTextures = false;
    bool hasHalfFPTextures = false;
    bool rgIsTexturable = false;
    // for now we don't support floating point MSAA on ES
    uint32_t fpRenderFlags = (kGL_GrGLStandard == standard) ? allRenderFlags : nonMSAARenderFlags;

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3, 0)) {
            hasFPTextures = true;
            hasHalfFPTextures = true;
            rgIsTexturable = true;
        }
    } else {
        if (version >= GR_GL_VER(3, 0)) {
            hasFPTextures = true;
            hasHalfFPTextures = true;
            rgIsTexturable = true;
        } else {
            if (ctxInfo.hasExtension("GL_OES_texture_float_linear") &&
                ctxInfo.hasExtension("GL_OES_texture_float")) {
                hasFPTextures = true;
            }
            if (ctxInfo.hasExtension("GL_OES_texture_half_float_linear") &&
                ctxInfo.hasExtension("GL_OES_texture_half_float")) {
                hasHalfFPTextures = true;
            }
        }
    }

    for (auto fpconfig : {kRGBA_float_GrPixelConfig, kRG_float_GrPixelConfig}) {
        const GrGLenum format = kRGBA_float_GrPixelConfig == fpconfig ? GR_GL_RGBA : GR_GL_RG;
        fConfigTable[fpconfig].fFormats.fBaseInternalFormat = format;
        fConfigTable[fpconfig].fFormats.fSizedInternalFormat =
            kRGBA_float_GrPixelConfig == fpconfig ? GR_GL_RGBA32F : GR_GL_RG32F;
        fConfigTable[fpconfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] = format;
        fConfigTable[fpconfig].fFormats.fExternalType = GR_GL_FLOAT;
        fConfigTable[fpconfig].fFormatType = kFloat_FormatType;
        if (hasFPTextures) {
            fConfigTable[fpconfig].fFlags = rgIsTexturable ? ConfigInfo::kTextureable_Flag : 0;
            // For now we only enable rendering to float on desktop, because on ES we'd have to
            // solve many precision issues and no clients actually want this yet.
            if (kGL_GrGLStandard == standard /* || version >= GR_GL_VER(3,2) ||
                ctxInfo.hasExtension("GL_EXT_color_buffer_float")*/) {
                fConfigTable[fpconfig].fFlags |= fpRenderFlags;
            }
        }
        if (texStorageSupported) {
            fConfigTable[fpconfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
        }
        if (texelBufferSupport) {
            fConfigTable[fpconfig].fFlags |= ConfigInfo::kCanUseWithTexelBuffer_Flag;
        }
        fConfigTable[fpconfig].fSwizzle = GrSwizzle::RGBA();
    }

    GrGLenum redHalfExternalType;
    if (kGL_GrGLStandard == ctxInfo.standard() || ctxInfo.version() >= GR_GL_VER(3, 0)) {
        redHalfExternalType = GR_GL_HALF_FLOAT;
    } else {
        redHalfExternalType = GR_GL_HALF_FLOAT_OES;
    }
    ConfigInfo& redHalf = fConfigTable[kAlpha_half_as_Red_GrPixelConfig];
    redHalf.fFormats.fExternalType = redHalfExternalType;
    redHalf.fFormatType = kFloat_FormatType;
    redHalf.fFormats.fBaseInternalFormat = GR_GL_RED;
    redHalf.fFormats.fSizedInternalFormat = GR_GL_R16F;
    redHalf.fFormats.fExternalFormat[kOther_ExternalFormatUsage] = GR_GL_RED;
    redHalf.fSwizzle = GrSwizzle::RRRR();
    if (textureRedSupport && hasHalfFPTextures) {
        redHalf.fFlags = ConfigInfo::kTextureable_Flag;

        if (kGL_GrGLStandard == standard || version >= GR_GL_VER(3, 2) ||
            (textureRedSupport && ctxInfo.hasExtension("GL_EXT_color_buffer_half_float"))) {
            redHalf.fFlags |= fpRenderFlags;
        }

        if (texStorageSupported && !isCommandBufferES2) {
            redHalf.fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
        }

        if (texelBufferSupport) {
            redHalf.fFlags |= ConfigInfo::kCanUseWithTexelBuffer_Flag;
        }
    }
    fConfigTable[kAlpha_half_GrPixelConfig] = redHalf;

    fConfigTable[kRGBA_half_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGBA;
    fConfigTable[kRGBA_half_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA16F;
    fConfigTable[kRGBA_half_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_RGBA;
    if (kGL_GrGLStandard == ctxInfo.standard() || ctxInfo.version() >= GR_GL_VER(3, 0)) {
        fConfigTable[kRGBA_half_GrPixelConfig].fFormats.fExternalType = GR_GL_HALF_FLOAT;
    } else {
        fConfigTable[kRGBA_half_GrPixelConfig].fFormats.fExternalType = GR_GL_HALF_FLOAT_OES;
    }
    fConfigTable[kRGBA_half_GrPixelConfig].fFormatType = kFloat_FormatType;
    if (hasHalfFPTextures) {
        fConfigTable[kRGBA_half_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
        // ES requires 3.2 or EXT_color_buffer_half_float.
        if (kGL_GrGLStandard == standard || version >= GR_GL_VER(3,2) ||
             ctxInfo.hasExtension("GL_EXT_color_buffer_half_float")) {
            fConfigTable[kRGBA_half_GrPixelConfig].fFlags |= fpRenderFlags;
        }
    }
    if (texStorageSupported) {
        fConfigTable[kRGBA_half_GrPixelConfig].fFlags |= ConfigInfo::kCanUseTexStorage_Flag;
    }
    if (texelBufferSupport) {
        fConfigTable[kRGBA_half_GrPixelConfig].fFlags |= ConfigInfo::kCanUseWithTexelBuffer_Flag;
    }
    fConfigTable[kRGBA_half_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    // Bulk populate the texture internal/external formats here and then deal with exceptions below.

    // ES 2.0 requires that the internal/external formats match.
    bool useSizedTexFormats = (kGL_GrGLStandard == ctxInfo.standard() ||
                               ctxInfo.version() >= GR_GL_VER(3,0));
    // All ES versions (thus far) require sized internal formats for render buffers.
    // TODO: Always use sized internal format?
    bool useSizedRbFormats = kGLES_GrGLStandard == ctxInfo.standard();

    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        // Almost always we want to pass fExternalFormat[kOther_ExternalFormatUsage] as the <format>
        // param to glTex[Sub]Image.
        fConfigTable[i].fFormats.fExternalFormat[kTexImage_ExternalFormatUsage] =
            fConfigTable[i].fFormats.fExternalFormat[kOther_ExternalFormatUsage];
        fConfigTable[i].fFormats.fInternalFormatTexImage = useSizedTexFormats ?
            fConfigTable[i].fFormats.fSizedInternalFormat :
            fConfigTable[i].fFormats.fBaseInternalFormat;
        fConfigTable[i].fFormats.fInternalFormatRenderbuffer = useSizedRbFormats ?
            fConfigTable[i].fFormats.fSizedInternalFormat :
            fConfigTable[i].fFormats.fBaseInternalFormat;
    }
    // If we're on ES 3.0+ but because of a driver workaround selected GL_ALPHA to implement the
    // kAlpha_8_GrPixelConfig then we actually have to use a base internal format rather than a
    // sized internal format. This is because there is no valid 8 bit alpha sized internal format
    // in ES.
    if (useSizedTexFormats && kGLES_GrGLStandard == ctxInfo.standard() && !textureRedSupport) {
        SkASSERT(fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fBaseInternalFormat == GR_GL_ALPHA8);
        SkASSERT(fConfigTable[kAlpha_8_as_Alpha_GrPixelConfig].fFormats.fBaseInternalFormat ==
                     GR_GL_ALPHA8);
        fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fInternalFormatTexImage =
            fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fBaseInternalFormat;
        fConfigTable[kAlpha_8_as_Alpha_GrPixelConfig].fFormats.fInternalFormatTexImage =
            fConfigTable[kAlpha_8_as_Alpha_GrPixelConfig].fFormats.fBaseInternalFormat;
    }

    // OpenGL ES 2.0 + GL_EXT_sRGB allows GL_SRGB_ALPHA to be specified as the <format>
    // param to Tex(Sub)Image. ES 2.0 requires the <internalFormat> and <format> params to match.
    // Thus, on ES 2.0 we will use GL_SRGB_ALPHA as the <format> param.
    // On OpenGL and ES 3.0+ GL_SRGB_ALPHA does not work for the <format> param to glTexImage.
    if (ctxInfo.standard() == kGLES_GrGLStandard && ctxInfo.version() == GR_GL_VER(2,0)) {
        fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fExternalFormat[kTexImage_ExternalFormatUsage] =
            GR_GL_SRGB_ALPHA;

        // Additionally, because we had to "invent" sBGRA, there is no way to make it work
        // in ES 2.0, because there is no <internalFormat> we can use. So just make that format
        // unsupported. (If we have no sRGB support at all, this will get overwritten below).
        fConfigTable[kSBGRA_8888_GrPixelConfig].fFlags = 0;
    }

    // If BGRA is supported as an internal format it must always be specified to glTex[Sub]Image
    // as a base format.
    // GL_EXT_texture_format_BGRA8888:
    //      This extension GL_BGRA as an unsized internal format. However, it is written against ES
    //      2.0 and therefore doesn't define a value for GL_BGRA8 as ES 2.0 uses unsized internal
    //      formats.
    // GL_APPLE_texture_format_BGRA8888:
    //     ES 2.0: the extension makes BGRA an external format but not an internal format.
    //     ES 3.0: the extension explicitly states GL_BGRA8 is not a valid internal format for
    //             glTexImage (just for glTexStorage).
    if (useSizedTexFormats && this->bgraIsInternalFormat()) {
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fInternalFormatTexImage = GR_GL_BGRA;
    }

    // If we don't have texture swizzle support then the shader generator must insert the
    // swizzle into shader code.
    if (!this->textureSwizzleSupport()) {
        for (int i = 0; i < kGrPixelConfigCnt; ++i) {
            shaderCaps->fConfigTextureSwizzle[i] = fConfigTable[i].fSwizzle;
        }
    }

    // Shader output swizzles will default to RGBA. When we've use GL_RED instead of GL_ALPHA to
    // implement kAlpha_8_GrPixelConfig we need to swizzle the shader outputs so the alpha channel
    // gets written to the single component.
    if (textureRedSupport) {
        for (int i = 0; i < kGrPixelConfigCnt; ++i) {
            GrPixelConfig config = static_cast<GrPixelConfig>(i);
            if (GrPixelConfigIsAlphaOnly(config) &&
                fConfigTable[i].fFormats.fBaseInternalFormat == GR_GL_RED) {
                shaderCaps->fConfigOutputSwizzle[i] = GrSwizzle::AAAA();
            }
        }
    }

    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        if (ConfigInfo::kRenderableWithMSAA_Flag & fConfigTable[i].fFlags) {
            // We assume that MSAA rendering is supported only if we support non-MSAA rendering.
            SkASSERT(ConfigInfo::kRenderable_Flag & fConfigTable[i].fFlags);
            if ((kGL_GrGLStandard == ctxInfo.standard() &&
                 (ctxInfo.version() >= GR_GL_VER(4,2) ||
                  ctxInfo.hasExtension("GL_ARB_internalformat_query"))) ||
                (kGLES_GrGLStandard == ctxInfo.standard() && ctxInfo.version() >= GR_GL_VER(3,0))) {
                int count;
                GrGLenum format = fConfigTable[i].fFormats.fInternalFormatRenderbuffer;
                GR_GL_GetInternalformativ(gli, GR_GL_RENDERBUFFER, format, GR_GL_NUM_SAMPLE_COUNTS,
                                          1, &count);
                if (count) {
                    int* temp = new int[count];
                    GR_GL_GetInternalformativ(gli, GR_GL_RENDERBUFFER, format, GR_GL_SAMPLES, count,
                                              temp);
                    // GL has a concept of MSAA rasterization with a single sample but we do not.
                    if (count && temp[count - 1] == 1) {
                        --count;
                        SkASSERT(!count || temp[count -1] > 1);
                    }
                    fConfigTable[i].fColorSampleCounts.setCount(count+1);
                    // We initialize our supported values with 1 (no msaa) and reverse the order
                    // returned by GL so that the array is ascending.
                    fConfigTable[i].fColorSampleCounts[0] = 1;
                    for (int j = 0; j < count; ++j) {
                        fConfigTable[i].fColorSampleCounts[j+1] = temp[count - j - 1];
                    }
                    delete[] temp;
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
                maxSampleCnt = SkTMax(1, maxSampleCnt);

                static constexpr int kDefaultSamples[] = {1, 2, 4, 8};
                int count = SK_ARRAY_COUNT(kDefaultSamples);
                for (; count > 0; --count) {
                    if (kDefaultSamples[count - 1] <= maxSampleCnt) {
                        break;
                    }
                }
                if (count > 0) {
                    fConfigTable[i].fColorSampleCounts.append(count, kDefaultSamples);
                }
            }
        } else if (ConfigInfo::kRenderable_Flag & fConfigTable[i].fFlags) {
            fConfigTable[i].fColorSampleCounts.setCount(1);
            fConfigTable[i].fColorSampleCounts[0] = 1;
        }
    }

#ifdef SK_DEBUG
    // Make sure we initialized everything.
    ConfigInfo defaultEntry;
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        // Make sure we didn't set renderable and not blittable or renderable with msaa and not
        // renderable.
        SkASSERT(!((ConfigInfo::kRenderable_Flag) && !(ConfigInfo::kFBOColorAttachment_Flag)));
        SkASSERT(!((ConfigInfo::kRenderableWithMSAA_Flag) && !(ConfigInfo::kRenderable_Flag)));
        SkASSERT(defaultEntry.fFormats.fBaseInternalFormat !=
                 fConfigTable[i].fFormats.fBaseInternalFormat);
        SkASSERT(defaultEntry.fFormats.fSizedInternalFormat !=
                 fConfigTable[i].fFormats.fSizedInternalFormat);
        for (int j = 0; j < kExternalFormatUsageCnt; ++j) {
            SkASSERT(defaultEntry.fFormats.fExternalFormat[j] !=
                     fConfigTable[i].fFormats.fExternalFormat[j]);
        }
        SkASSERT(defaultEntry.fFormats.fExternalType != fConfigTable[i].fFormats.fExternalType);
    }
#endif
}

bool GrGLCaps::initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                                  bool* rectsMustMatch, bool* disallowSubrect) const {
    // By default, we don't require rects to match.
    *rectsMustMatch = false;

    // By default, we allow subrects.
    *disallowSubrect = false;

    // If the src is a texture, we can implement the blit as a draw assuming the config is
    // renderable.
    if (src->asTextureProxy() && this->isConfigRenderable(src->config(), false)) {
        desc->fOrigin = kBottomLeft_GrSurfaceOrigin;
        desc->fFlags = kRenderTarget_GrSurfaceFlag;
        desc->fConfig = src->config();
        return true;
    }

    {
        // The only way we could see a non-GR_GL_TEXTURE_2D texture would be if it were
        // wrapped. In that case the proxy would already be instantiated.
        const GrTexture* srcTexture = src->priv().peekTexture();
        const GrGLTexture* glSrcTexture = static_cast<const GrGLTexture*>(srcTexture);
        if (glSrcTexture && glSrcTexture->target() != GR_GL_TEXTURE_2D) {
            // Not supported for FBO blit or CopyTexSubImage
            return false;
        }
    }

    // We look for opportunities to use CopyTexSubImage, or fbo blit. If neither are
    // possible and we return false to fallback to creating a render target dst for render-to-
    // texture. This code prefers CopyTexSubImage to fbo blit and avoids triggering temporary fbo
    // creation. It isn't clear that avoiding temporary fbo creation is actually optimal.
    GrSurfaceOrigin originForBlitFramebuffer = kTopLeft_GrSurfaceOrigin;
    bool rectsMustMatchForBlitFramebuffer = false;
    bool disallowSubrectForBlitFramebuffer = false;
    if (src->numColorSamples() &&
        (this->blitFramebufferSupportFlags() & kResolveMustBeFull_BlitFrambufferFlag)) {
        rectsMustMatchForBlitFramebuffer = true;
        disallowSubrectForBlitFramebuffer = true;
        // Mirroring causes rects to mismatch later, don't allow it.
        originForBlitFramebuffer = src->origin();
    } else if (src->numColorSamples() && (this->blitFramebufferSupportFlags() &
                                          kRectsMustMatchForMSAASrc_BlitFramebufferFlag)) {
        rectsMustMatchForBlitFramebuffer = true;
        // Mirroring causes rects to mismatch later, don't allow it.
        originForBlitFramebuffer = src->origin();
    } else if (this->blitFramebufferSupportFlags() & kNoScalingOrMirroring_BlitFramebufferFlag) {
        originForBlitFramebuffer = src->origin();
    }

    // Check for format issues with glCopyTexSubImage2D
    if (this->bgraIsInternalFormat() && kBGRA_8888_GrPixelConfig == src->config()) {
        // glCopyTexSubImage2D doesn't work with this config. If the bgra can be used with fbo blit
        // then we set up for that, otherwise fail.
        if (this->canConfigBeFBOColorAttachment(kBGRA_8888_GrPixelConfig)) {
            desc->fOrigin = originForBlitFramebuffer;
            desc->fConfig = kBGRA_8888_GrPixelConfig;
            *rectsMustMatch = rectsMustMatchForBlitFramebuffer;
            *disallowSubrect = disallowSubrectForBlitFramebuffer;
            return true;
        }
        return false;
    }

    {
        bool srcIsMSAARenderbuffer = GrFSAAType::kUnifiedMSAA == src->fsaaType() &&
                                     this->usesMSAARenderBuffers();
        if (srcIsMSAARenderbuffer) {
            // It's illegal to call CopyTexSubImage2D on a MSAA renderbuffer. Set up for FBO
            // blit or fail.
            if (this->canConfigBeFBOColorAttachment(src->config())) {
                desc->fOrigin = originForBlitFramebuffer;
                desc->fConfig = src->config();
                *rectsMustMatch = rectsMustMatchForBlitFramebuffer;
                *disallowSubrect = disallowSubrectForBlitFramebuffer;
                return true;
            }
            return false;
        }
    }

    // We'll do a CopyTexSubImage. Make the dst a plain old texture.
    desc->fConfig = src->config();
    desc->fOrigin = src->origin();
    desc->fFlags = kNone_GrSurfaceFlags;
    return true;
}

void GrGLCaps::applyDriverCorrectnessWorkarounds(const GrGLContextInfo& ctxInfo,
                                                 const GrContextOptions& contextOptions,
                                                 GrShaderCaps* shaderCaps) {
    // A driver but on the nexus 6 causes incorrect dst copies when invalidate is called beforehand.
    // Thus we are blacklisting this extension for now on Adreno4xx devices.
    if (kAdreno4xx_GrGLRenderer == ctxInfo.renderer()) {
        fDiscardRenderTargetSupport = false;
        fInvalidateFBType = kNone_InvalidateFBType;
    }

    // glClearTexImage seems to have a bug in NVIDIA drivers that was fixed sometime between
    // 340.96 and 367.57.
    if (kGL_GrGLStandard == ctxInfo.standard() &&
        ctxInfo.driver() == kNVIDIA_GrGLDriver &&
        ctxInfo.driverVersion() < GR_GL_DRIVER_VER(367, 57)) {
        fClearTextureSupport = false;
    }

    // Calling glClearTexImage crashes on the NexusPlayer.
    if (kPowerVRRogue_GrGLRenderer == ctxInfo.renderer()) {
        fClearTextureSupport = false;
    }

    // On at least some MacBooks, GLSL 4.0 geometry shaders break if we use invocations.
#ifdef SK_BUILD_FOR_MAC
    if (shaderCaps->fGeometryShaderSupport) {
        shaderCaps->fGSInvocationsSupport = false;
    }
#endif

    // Qualcomm driver @103.0 has been observed to crash compiling ccpr geometry
    // shaders. @127.0 is the earliest verified driver to not crash.
    if (kQualcomm_GrGLDriver == ctxInfo.driver() &&
        ctxInfo.driverVersion() < GR_GL_DRIVER_VER(127,0)) {
        shaderCaps->fGeometryShaderSupport = false;
    }

#if defined(__has_feature)
#if defined(SK_BUILD_FOR_MAC) && __has_feature(thread_sanitizer)
    // See skbug.com/7058
    fMapBufferType = kNone_MapBufferType;
    fMapBufferFlags = kNone_MapFlags;
#endif
#endif

    // We found that the Galaxy J5 with an Adreno 306 running 6.0.1 has a bug where
    // GL_INVALID_OPERATION thrown by glDrawArrays when using a buffer that was mapped. The same bug
    // did not reproduce on a Nexus7 2013 with a 320 running Android M with driver 127.0. It's
    // unclear whether this really affects a wide range of devices.
    if (ctxInfo.renderer() == kAdreno3xx_GrGLRenderer &&
        ctxInfo.driverVersion() > GR_GL_DRIVER_VER(127, 0)) {
        fMapBufferType = kNone_MapBufferType;
        fMapBufferFlags = kNone_MapFlags;
    }

    // TODO: re-enable for ANGLE
    if (kANGLE_GrGLDriver == ctxInfo.driver()) {
        fTransferBufferType = kNone_TransferBufferType;
    }

    // Using MIPs on this GPU seems to be a source of trouble.
    if (kPowerVR54x_GrGLRenderer == ctxInfo.renderer()) {
        fMipMapSupport = false;
    }

    if (kPowerVRRogue_GrGLRenderer == ctxInfo.renderer()) {
        // Temporarily disabling clip analytic fragments processors on Nexus player while we work
        // around a driver bug related to gl_FragCoord.
        // https://bugs.chromium.org/p/skia/issues/detail?id=7286
        fMaxClipAnalyticFPs = 0;
    }

#ifndef SK_BUILD_FOR_IOS
    if (kPowerVR54x_GrGLRenderer == ctxInfo.renderer() ||
        kPowerVRRogue_GrGLRenderer == ctxInfo.renderer() ||
        (kAdreno3xx_GrGLRenderer == ctxInfo.renderer() &&
         ctxInfo.driver() != kChromium_GrGLDriver)) {
        fUseDrawToClearColor = true;
    }
#endif

    // A lot of GPUs have trouble with full screen clears (skbug.com/7195)
    if (kAMDRadeonHD7xxx_GrGLRenderer == ctxInfo.renderer() ||
        kAMDRadeonR9M4xx_GrGLRenderer == ctxInfo.renderer()) {
        fUseDrawToClearColor = true;
    }

#ifdef SK_BUILD_FOR_MAC
    // crbug.com/768134 - On MacBook Pros, the Intel Iris Pro doesn't always perform
    // full screen clears
    // crbug.com/773107 - On MacBook Pros, a wide range of Intel GPUs don't always
    // perform full screen clears.
    if (kIntel_GrGLVendor == ctxInfo.vendor()) {
        fUseDrawToClearColor = true;
    }
#endif

    // See crbug.com/755871. This could probably be narrowed to just partial clears as the driver
    // bugs seems to involve clearing too much and not skipping the clear.
    // See crbug.com/768134. This is also needed for full clears and was seen on an nVidia K620
    // but only for D3D11 ANGLE.
    if (GrGLANGLEBackend::kD3D11 == ctxInfo.angleBackend()) {
        fUseDrawToClearColor = true;
    }

    if (kAdreno4xx_GrGLRenderer == ctxInfo.renderer()) {
        // This is known to be fixed sometime between driver 145.0 and 219.0
        if (ctxInfo.driverVersion() <= GR_GL_DRIVER_VER(219, 0)) {
            fUseDrawToClearStencilClip = true;
        }
        fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO = true;
    }

    // This was reproduced on the following configurations:
    // - A Galaxy J5 (Adreno 306) running Android 6 with driver 140.0
    // - A Nexus 7 2013 (Adreno 320) running Android 5 with driver 104.0
    // - A Nexus 7 2013 (Adreno 320) running Android 6 with driver 127.0
    // - A Nexus 5 (Adreno 330) running Android 6 with driver 127.0
    // and not produced on:
    // - A Nexus 7 2013 (Adreno 320) running Android 4 with driver 53.0
    // The particular lines that get dropped from test images varies across different devices.
    if (kAdreno3xx_GrGLRenderer == ctxInfo.renderer() &&
        ctxInfo.driverVersion() > GR_GL_DRIVER_VER(53, 0)) {
        fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines = true;
    }

    // Our Chromebook with kPowerVRRogue_GrGLRenderer seems to crash when glDrawArraysInstanced is
    // given 1 << 15 or more instances.
    if (kPowerVRRogue_GrGLRenderer == ctxInfo.renderer()) {
        fMaxInstancesPerDrawArraysWithoutCrashing = 0x7fff;
    }

    // Texture uploads sometimes seem to be ignored to textures bound to FBOS on Tegra3.
    if (kTegra3_GrGLRenderer == ctxInfo.renderer()) {
        fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO = true;
        fUseDrawInsteadOfAllRenderTargetWrites = true;
    }

    if (kGL_GrGLStandard == ctxInfo.standard() && kIntel_GrGLVendor == ctxInfo.vendor() ) {
        fSampleShadingSupport = false;
    }

#ifdef SK_BUILD_FOR_MAC
    static constexpr bool isMAC = true;
#else
    static constexpr bool isMAC = false;
#endif

    // We support manual mip-map generation (via iterative downsampling draw calls). This fixes
    // bugs on some cards/drivers that produce incorrect mip-maps for sRGB textures when using
    // glGenerateMipmap. Our implementation requires mip-level sampling control. Additionally,
    // it can be much slower (especially on mobile GPUs), so we opt-in only when necessary:
    if (fMipMapLevelAndLodControlSupport &&
        (contextOptions.fDoManualMipmapping ||
         (kIntel_GrGLVendor == ctxInfo.vendor()) ||
         (kNVIDIA_GrGLDriver == ctxInfo.driver() && isMAC) ||
         (kATI_GrGLVendor == ctxInfo.vendor()))) {
        fDoManualMipmapping = true;
    }

    // See http://crbug.com/710443
#ifdef SK_BUILD_FOR_MAC
    if (kIntel6xxx_GrGLRenderer == ctxInfo.renderer()) {
        fClearToBoundaryValuesIsBroken = true;
    }
#endif
    if (kQualcomm_GrGLVendor == ctxInfo.vendor()) {
        fDrawArraysBaseVertexIsBroken = true;
    }

    // The ccpr vertex-shader implementation does not work on this platform. Only allow CCPR with
    // GS.

    if (kANGLE_GrGLRenderer == ctxInfo.renderer() &&
        GrGLANGLERenderer::kSkylake == ctxInfo.angleRenderer()) {
        bool gsSupport = fShaderCaps->geometryShaderSupport();
#if GR_TEST_UTILS
        gsSupport &= !contextOptions.fSuppressGeometryShaders;
#endif
        fBlacklistCoverageCounting = !gsSupport;
    }
    // Currently the extension is advertised but fb fetch is broken on 500 series Adrenos like the
    // Galaxy S7.
    // TODO: Once this is fixed we can update the check here to look at a driver version number too.
    if (kAdreno5xx_GrGLRenderer == ctxInfo.renderer()) {
        shaderCaps->fFBFetchSupport = false;
    }

    // Pre-361 NVIDIA has a bug with NV_sample_mask_override_coverage.
    if (kNVIDIA_GrGLDriver == ctxInfo.driver() &&
        ctxInfo.driverVersion() < GR_GL_DRIVER_VER(361,00)) {
        shaderCaps->fSampleMaskOverrideCoverageSupport = false;
    }

    // Adreno GPUs have a tendency to drop tiles when there is a divide-by-zero in a shader
    shaderCaps->fDropsTileOnZeroDivide = kQualcomm_GrGLVendor == ctxInfo.vendor();

    // On the NexusS and GalaxyNexus, the use of 'any' causes the compilation error "Calls to any
    // function that may require a gradient calculation inside a conditional block may return
    // undefined results". This appears to be an issue with the 'any' call since even the simple
    // "result=black; if (any()) result=white;" code fails to compile. This issue comes into play
    // from our GrTextureDomain processor.
    shaderCaps->fCanUseAnyFunctionInShader = kImagination_GrGLVendor != ctxInfo.vendor();

    // Known issue on at least some Intel platforms:
    // http://code.google.com/p/skia/issues/detail?id=946
    if (kIntel_GrGLVendor == ctxInfo.vendor()) {
        shaderCaps->fFragCoordConventionsExtensionString = nullptr;
    }

    if (kTegra3_GrGLRenderer == ctxInfo.renderer()) {
        // The Tegra3 compiler will sometimes never return if we have min(abs(x), 1.0),
        // so we must do the abs first in a separate expression.
        shaderCaps->fCanUseMinAndAbsTogether = false;

        // Tegra3 fract() seems to trigger undefined behavior for negative values, so we
        // must avoid this condition.
        shaderCaps->fCanUseFractForNegativeValues = false;
    }

    // On Intel GPU there is an issue where it reads the second argument to atan "- %s.x" as an int
    // thus must us -1.0 * %s.x to work correctly
    if (kIntel_GrGLVendor == ctxInfo.vendor()) {
        shaderCaps->fMustForceNegatedAtanParamToFloat = true;
    }

    // On some Intel GPUs there is an issue where the driver outputs bogus values in the shader
    // when floor and abs are called on the same line. Thus we must execute an Op between them to
    // make sure the compiler doesn't re-inline them even if we break the calls apart.
    if (kIntel_GrGLVendor == ctxInfo.vendor()) {
        shaderCaps->fMustDoOpBetweenFloorAndAbs = true;
    }

    // On Adreno devices with framebuffer fetch support, there is a bug where they always return
    // the original dst color when reading the outColor even after being written to. By using a
    // local outColor we can work around this bug.
    if (shaderCaps->fFBFetchSupport && kQualcomm_GrGLVendor == ctxInfo.vendor()) {
        shaderCaps->fRequiresLocalOutputColorForFBFetch = true;
    }

    // Newer Mali GPUs do incorrect static analysis in specific situations: If there is uniform
    // color, and that uniform contains an opaque color, and the output of the shader is only based
    // on that uniform plus soemthing un-trackable (like a texture read), the compiler will deduce
    // that the shader always outputs opaque values. In that case, it appears to remove the shader
    // based blending code it normally injects, turning SrcOver into Src. To fix this, we always
    // insert an extra bit of math on the uniform that confuses the compiler just enough...
    if (kMaliT_GrGLRenderer == ctxInfo.renderer()) {
        shaderCaps->fMustObfuscateUniformColor = true;
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
    if (kANGLE_GrGLDriver == ctxInfo.driver() || kChromium_GrGLDriver == ctxInfo.driver()) {
        shaderCaps->fMustGuardDivisionEvenAfterExplicitZeroCheck = true;
    }
#endif

    // We've seen Adreno 3xx devices produce incorrect (flipped) values for gl_FragCoord, in some
    // (rare) situations. It's sporadic, and mostly on older drivers. It also seems to be the case
    // that the interpolation of vertex shader outputs is quite inaccurate.
    if (kAdreno3xx_GrGLRenderer == ctxInfo.renderer()) {
        shaderCaps->fCanUseFragCoord = false;
        shaderCaps->fInterpolantsAreInaccurate = true;
    }

    // Disabling advanced blend on various platforms with major known issues. We also block Chrome
    // for now until its own blacklists can be updated.
    if (kAdreno4xx_GrGLRenderer == ctxInfo.renderer() ||
        kAdreno5xx_GrGLRenderer == ctxInfo.renderer() ||
        kIntel_GrGLDriver == ctxInfo.driver() ||
        kChromium_GrGLDriver == ctxInfo.driver()) {
        fBlendEquationSupport = kBasic_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kNotSupported_AdvBlendEqInteraction;
    }

    // Non-coherent advanced blend has an issue on NVIDIA pre 337.00.
    if (kNVIDIA_GrGLDriver == ctxInfo.driver() &&
        ctxInfo.driverVersion() < GR_GL_DRIVER_VER(337,00) &&
        kAdvanced_BlendEquationSupport == fBlendEquationSupport) {
        fBlendEquationSupport = kBasic_BlendEquationSupport;
        shaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kNotSupported_AdvBlendEqInteraction;
    }

    if (this->advancedBlendEquationSupport()) {
        if (kNVIDIA_GrGLDriver == ctxInfo.driver() &&
            ctxInfo.driverVersion() < GR_GL_DRIVER_VER(355,00)) {
            // Blacklist color-dodge and color-burn on pre-355.00 NVIDIA.
            fAdvBlendEqBlacklist |= (1 << kColorDodge_GrBlendEquation) |
                                    (1 << kColorBurn_GrBlendEquation);
        }
        if (kARM_GrGLVendor == ctxInfo.vendor()) {
            // Blacklist color-burn on ARM until the fix is released.
            fAdvBlendEqBlacklist |= (1 << kColorBurn_GrBlendEquation);
        }
    }

    // Workaround NVIDIA bug related to glInvalidateFramebuffer and mixed samples.
    if (fMultisampleDisableSupport &&
        this->shaderCaps()->dualSourceBlendingSupport() &&
        this->shaderCaps()->pathRenderingSupport() &&
        fUsesMixedSamples &&
#if GR_TEST_UTILS
        (contextOptions.fGpuPathRenderers & GpuPathRenderers::kStencilAndCover) &&
#endif
        (kNVIDIA_GrGLDriver == ctxInfo.driver() ||
         kChromium_GrGLDriver == ctxInfo.driver())) {
            fDiscardRenderTargetSupport = false;
            fInvalidateFBType = kNone_InvalidateFBType;
    }
}

void GrGLCaps::onApplyOptionsOverrides(const GrContextOptions& options) {
    if (options.fDisableDriverCorrectnessWorkarounds) {
        SkASSERT(!fDoManualMipmapping);
        SkASSERT(!fClearToBoundaryValuesIsBroken);
        SkASSERT(0 == fMaxInstancesPerDrawArraysWithoutCrashing);
        SkASSERT(!fDrawArraysBaseVertexIsBroken);
        SkASSERT(!fUseDrawToClearColor);
        SkASSERT(!fUseDrawToClearStencilClip);
        SkASSERT(!fDisallowTexSubImageForUnormConfigTexturesEverBoundToFBO);
        SkASSERT(!fUseDrawInsteadOfAllRenderTargetWrites);
        SkASSERT(!fRequiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines);
    }
    if (options.fUseDrawInsteadOfPartialRenderTargetWrite) {
        fUseDrawInsteadOfAllRenderTargetWrites = true;
    }
    if (GrContextOptions::Enable::kNo == options.fUseDrawInsteadOfGLClear) {
        fUseDrawToClearColor = false;
    } else if (GrContextOptions::Enable::kYes == options.fUseDrawInsteadOfGLClear) {
        fUseDrawToClearColor = true;
    }
    if (options.fDoManualMipmapping) {
        fDoManualMipmapping = true;
    }
}

int GrGLCaps::getSampleCount(int requestedCount, GrPixelConfig config) const {
    requestedCount = SkTMax(1, requestedCount);
    int count = fConfigTable[config].fColorSampleCounts.count();
    if (!count || !this->isConfigRenderable(config, requestedCount > 1)) {
        return 0;
    }

    if (1 == requestedCount) {
        return fConfigTable[config].fColorSampleCounts[0] == 1 ? 1 : 0;
    }

    for (int i = 0; i < count; ++i) {
        if (fConfigTable[config].fColorSampleCounts[i] >= requestedCount) {
            return fConfigTable[config].fColorSampleCounts[i];
        }
    }
    return 0;
}

bool validate_sized_format(GrGLenum format, SkColorType ct, GrPixelConfig* config,
                           GrGLStandard standard) {
    *config = kUnknown_GrPixelConfig;

    switch (ct) {
        case kUnknown_SkColorType:
            return false;
        case kAlpha_8_SkColorType:
            if (GR_GL_ALPHA8 == format) {
                *config = kAlpha_8_as_Alpha_GrPixelConfig;
            } else if (GR_GL_R8 == format) {
                *config = kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGB_565_SkColorType:
            if (GR_GL_RGB565 == format) {
                *config = kRGB_565_GrPixelConfig;
            }
            break;
        case kARGB_4444_SkColorType:
            if (GR_GL_RGBA4 == format) {
                *config = kRGBA_4444_GrPixelConfig;
            }
            break;
        case kRGBA_8888_SkColorType:
            if (GR_GL_RGBA8 == format) {
                *config = kRGBA_8888_GrPixelConfig;
            } else if (GR_GL_SRGB8_ALPHA8 == format) {
                *config = kSRGBA_8888_GrPixelConfig;
            }
            break;
        case kRGB_888x_SkColorType:
            return false;
        case kBGRA_8888_SkColorType:
            if (GR_GL_RGBA8 == format) {
                if (kGL_GrGLStandard == standard) {
                    *config = kBGRA_8888_GrPixelConfig;
                }
            } else if (GR_GL_BGRA8 == format) {
                if (kGLES_GrGLStandard == standard) {
                    *config = kBGRA_8888_GrPixelConfig;
                }
            } else if (GR_GL_SRGB8_ALPHA8 == format) {
                *config = kSBGRA_8888_GrPixelConfig;
            }
            break;
        case kRGBA_1010102_SkColorType:
            return false;
        case kRGB_101010x_SkColorType:
            return false;
        case kGray_8_SkColorType:
            if (GR_GL_LUMINANCE8 == format) {
                *config = kGray_8_as_Lum_GrPixelConfig;
            } else if (GR_GL_R8 == format) {
                *config = kGray_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGBA_F16_SkColorType:
            if (GR_GL_RGBA16F == format) {
                *config = kRGBA_half_GrPixelConfig;
            }
            break;
    }

    return kUnknown_GrPixelConfig != *config;
}

bool GrGLCaps::validateBackendTexture(const GrBackendTexture& tex, SkColorType ct,
                                      GrPixelConfig* config) const {
    const GrGLTextureInfo* texInfo = tex.getGLTextureInfo();
    if (!texInfo) {
        return false;
    }
    return validate_sized_format(texInfo->fFormat, ct, config, fStandard);
}

bool GrGLCaps::validateBackendRenderTarget(const GrBackendRenderTarget& rt, SkColorType ct,
                                           GrPixelConfig* config) const {
    const GrGLFramebufferInfo* fbInfo = rt.getGLFramebufferInfo();
    if (!fbInfo) {
        return false;
    }
    return validate_sized_format(fbInfo->fFormat, ct, config, fStandard);
}

