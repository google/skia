/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLCaps.h"

#include "GrContextOptions.h"
#include "GrGLContext.h"
#include "glsl/GrGLSLCaps.h"
#include "SkTSearch.h"
#include "SkTSort.h"

GrGLCaps::GrGLCaps(const GrContextOptions& contextOptions,
                   const GrGLContextInfo& ctxInfo,
                   const GrGLInterface* glInterface) : INHERITED(contextOptions) {
    fVerifiedColorConfigs.reset();
    fStencilFormats.reset();
    fMSFBOType = kNone_MSFBOType;
    fInvalidateFBType = kNone_InvalidateFBType;
    fLATCAlias = kLATC_LATCAlias;
    fMapBufferType = kNone_MapBufferType;
    fMaxFragmentUniformVectors = 0;
    fMaxVertexAttributes = 0;
    fMaxFragmentTextureUnits = 0;
    fRGBA8RenderbufferSupport = false;
    fBGRAIsInternalFormat = false;
    fUnpackRowLengthSupport = false;
    fUnpackFlipYSupport = false;
    fPackRowLengthSupport = false;
    fPackFlipYSupport = false;
    fTextureUsageSupport = false;
    fTexStorageSupport = false;
    fTextureRedSupport = false;
    fImagingSupport = false;
    fTwoFormatLimit = false;
    fVertexArrayObjectSupport = false;
    fInstancedDrawingSupport = false;
    fDirectStateAccessSupport = false;
    fDebugSupport = false;
    fES2CompatibilitySupport = false;
    fMultisampleDisableSupport = false;
    fUseNonVBOVertexAndIndexDynamicData = false;
    fIsCoreProfile = false;
    fBindFragDataLocationSupport = false;
    fExternalTextureSupport = false;
    fSRGBWriteControl = false;
    fRGBA8888PixelsOpsAreSlow = false;
    fPartialFBOReadIsSlow = false;

    fReadPixelsSupportedCache.reset();

    fShaderCaps.reset(new GrGLSLCaps(contextOptions));

    this->init(contextOptions, ctxInfo, glInterface);
}

void GrGLCaps::init(const GrContextOptions& contextOptions,
                    const GrGLContextInfo& ctxInfo,
                    const GrGLInterface* gli) {
    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

    /**************************************************************************
     * Caps specific to GrGLCaps
     **************************************************************************/

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
    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_IMAGE_UNITS, &fMaxFragmentTextureUnits);

    if (kGL_GrGLStandard == standard) {
        fRGBA8RenderbufferSupport = true;
    } else {
        fRGBA8RenderbufferSupport = version >= GR_GL_VER(3,0) ||
                                    ctxInfo.hasExtension("GL_OES_rgb8_rgba8") ||
                                    ctxInfo.hasExtension("GL_ARM_rgba8");
    }

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
        // The EXT version can apply to either GL or GLES.
        fTexStorageSupport = version >= GR_GL_VER(4,2) ||
                             ctxInfo.hasExtension("GL_ARB_texture_storage") ||
                             ctxInfo.hasExtension("GL_EXT_texture_storage");
    } else {
        // Qualcomm Adreno drivers appear to have issues with texture storage.
        fTexStorageSupport = (version >= GR_GL_VER(3,0) &&
                              kQualcomm_GrGLVendor != ctxInfo.vendor()) ||
                             ctxInfo.hasExtension("GL_EXT_texture_storage");
    }

    if (kGL_GrGLStandard == standard) {
        fTextureBarrierSupport = version >= GR_GL_VER(4,5) ||
                                 ctxInfo.hasExtension("GL_ARB_texture_barrier") ||
                                 ctxInfo.hasExtension("GL_NV_texture_barrier");
    } else {
        fTextureBarrierSupport = ctxInfo.hasExtension("GL_NV_texture_barrier");
    }

    // ARB_texture_rg is part of OpenGL 3.0, but mesa doesn't support GL_RED 
    // and GL_RG on FBO textures.
    if (kMesa_GrGLDriver != ctxInfo.driver()) {
        if (kGL_GrGLStandard == standard) {
            fTextureRedSupport = version >= GR_GL_VER(3,0) ||
                                 ctxInfo.hasExtension("GL_ARB_texture_rg");
        } else {
            fTextureRedSupport =  version >= GR_GL_VER(3,0) ||
                                  ctxInfo.hasExtension("GL_EXT_texture_rg");
        }
    }
    fImagingSupport = kGL_GrGLStandard == standard &&
                      ctxInfo.hasExtension("GL_ARB_imaging");

    // ES 2 only guarantees RGBA/uchar + one other format/type combo for
    // ReadPixels. The other format has to checked at run-time since it
    // can change based on which render target is bound
    fTwoFormatLimit = kGLES_GrGLStandard == standard;

    // We only enable srgb support if both textures and FBOs support srgb.
    bool srgbSupport = false;
    if (kGL_GrGLStandard == standard) {
        if (ctxInfo.version() >= GR_GL_VER(3,0)) {
            srgbSupport = true;
        } else if (ctxInfo.hasExtension("GL_EXT_texture_sRGB")) {
            if (ctxInfo.hasExtension("GL_ARB_framebuffer_sRGB") ||
                ctxInfo.hasExtension("GL_EXT_framebuffer_sRGB")) {
                srgbSupport = true;
            }
         }
        // All the above srgb extensions support toggling srgb writes
        fSRGBWriteControl = srgbSupport;
    } else {
        // See https://bug.skia.org/4148 for PowerVR issue.
        srgbSupport = kPowerVRRogue_GrGLRenderer != ctxInfo.renderer() &&
                      (ctxInfo.version() >= GR_GL_VER(3,0) || ctxInfo.hasExtension("GL_EXT_sRGB"));
        // ES through 3.1 requires EXT_srgb_write_control to support toggling
        // sRGB writing for destinations.
        fSRGBWriteControl = ctxInfo.hasExtension("GL_EXT_sRGB_write_control");
    }

    // SGX and Mali GPUs that are based on a tiled-deferred architecture that have trouble with
    // frequently changing VBOs. We've measured a performance increase using non-VBO vertex
    // data for dynamic content on these GPUs. Perhaps we should read the renderer string and
    // limit this decision to specific GPU families rather than basing it on the vendor alone.
    if (!GR_GL_MUST_USE_VBO &&
        (kARM_GrGLVendor == ctxInfo.vendor() ||
         kImagination_GrGLVendor == ctxInfo.vendor() ||
         kQualcomm_GrGLVendor == ctxInfo.vendor())) {
        fUseNonVBOVertexAndIndexDynamicData = true;
    }

    // A driver but on the nexus 6 causes incorrect dst copies when invalidate is called beforehand.
    // Thus we are blacklisting this extension for now on Adreno4xx devices.
    if (kAdreno4xx_GrGLRenderer != ctxInfo.renderer() &&
        ((kGL_GrGLStandard == standard && version >= GR_GL_VER(4,3)) ||
         (kGLES_GrGLStandard == standard && version >= GR_GL_VER(3,0)) ||
         ctxInfo.hasExtension("GL_ARB_invalidate_subdata"))) {
        fDiscardRenderTargetSupport = true;
        fInvalidateFBType = kInvalidate_InvalidateFBType;
    } else if (ctxInfo.hasExtension("GL_EXT_discard_framebuffer")) {
        fDiscardRenderTargetSupport = true;
        fInvalidateFBType = kDiscard_InvalidateFBType;
    }

    if (kARM_GrGLVendor == ctxInfo.vendor() || kImagination_GrGLVendor == ctxInfo.vendor()) {
        fFullClearIsFree = true;
    }

    if (kGL_GrGLStandard == standard) {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_ARB_vertex_array_object") ||
                                    ctxInfo.hasExtension("GL_APPLE_vertex_array_object");
    } else {
        fVertexArrayObjectSupport = version >= GR_GL_VER(3, 0) ||
                                    ctxInfo.hasExtension("GL_OES_vertex_array_object");
    }

    if ((kGL_GrGLStandard == standard && version >= GR_GL_VER(3,2)) ||
        (kGLES_GrGLStandard == standard && version >= GR_GL_VER(3,0))) {
        fInstancedDrawingSupport = true;
    } else {
        fInstancedDrawingSupport = (ctxInfo.hasExtension("GL_ARB_draw_instanced") ||
                                    ctxInfo.hasExtension("GL_EXT_draw_instanced")) &&
                                   (ctxInfo.hasExtension("GL_ARB_instanced_arrays") ||
                                    ctxInfo.hasExtension("GL_EXT_instanced_arrays"));
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
        if (version >= GR_GL_VER(3, 0)) {
            fBindFragDataLocationSupport = true;
        }
    } else {
        if (version >= GR_GL_VER(3, 0) && ctxInfo.hasExtension("GL_EXT_blend_func_extended")) {
            fBindFragDataLocationSupport = true;
        }
    }

#if 0 // Disabled due to https://bug.skia.org/4454
    fBindUniformLocationSupport = ctxInfo.hasExtension("GL_CHROMIUM_bind_uniform_location");
#else
    fBindUniformLocationSupport = false;
#endif

    if (ctxInfo.hasExtension("GL_OES_EGL_image_external")) {
        if (ctxInfo.glslGeneration() == k110_GrGLSLGeneration) {
            fExternalTextureSupport = true;
        } else if (ctxInfo.hasExtension("GL_OES_EGL_image_external_essl3") ||
                   ctxInfo.hasExtension("OES_EGL_image_external_essl3")) {
            // At least one driver has been found that has this extension without the "GL_" prefix.
            fExternalTextureSupport = true;
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

    /**************************************************************************
    * GrShaderCaps fields
    **************************************************************************/

    // This must be called after fCoreProfile is set on the GrGLCaps
    this->initGLSL(ctxInfo);
    GrGLSLCaps* glslCaps = static_cast<GrGLSLCaps*>(fShaderCaps.get());

    glslCaps->fPathRenderingSupport = this->hasPathRenderingSupport(ctxInfo, gli);

    // For now these two are equivalent but we could have dst read in shader via some other method.
    // Before setting this, initGLSL() must have been called.
    glslCaps->fDstReadInShaderSupport = glslCaps->fFBFetchSupport;

    // Enable supported shader-related caps
    if (kGL_GrGLStandard == standard) {
        glslCaps->fDualSourceBlendingSupport = (ctxInfo.version() >= GR_GL_VER(3, 3) ||
            ctxInfo.hasExtension("GL_ARB_blend_func_extended")) &&
            GrGLSLSupportsNamedFragmentShaderOutputs(ctxInfo.glslGeneration());
        glslCaps->fShaderDerivativeSupport = true;
        // we don't support GL_ARB_geometry_shader4, just GL 3.2+ GS
        glslCaps->fGeometryShaderSupport = ctxInfo.version() >= GR_GL_VER(3, 2) &&
            ctxInfo.glslGeneration() >= k150_GrGLSLGeneration;
    }
    else {
        glslCaps->fDualSourceBlendingSupport = ctxInfo.hasExtension("GL_EXT_blend_func_extended");

        glslCaps->fShaderDerivativeSupport = ctxInfo.version() >= GR_GL_VER(3, 0) ||
            ctxInfo.hasExtension("GL_OES_standard_derivatives");
    }

    /**************************************************************************
     * GrCaps fields
     **************************************************************************/

    // We need dual source blending and the ability to disable multisample in order to support mixed
    // samples in every corner case.
    if (fMultisampleDisableSupport && glslCaps->dualSourceBlendingSupport()) {
        fMixedSamplesSupport = ctxInfo.hasExtension("GL_NV_framebuffer_mixed_samples");
        // Workaround NVIDIA bug related to glInvalidateFramebuffer and mixed samples.
        if (fMixedSamplesSupport && kNVIDIA_GrGLDriver == ctxInfo.driver()) {
            fDiscardRenderTargetSupport = false;
            fInvalidateFBType = kNone_InvalidateFBType;
        }
    }

    // fPathRenderingSupport and fMixedSamplesSupport must be set before calling initFSAASupport.
    this->initFSAASupport(ctxInfo, gli);
    this->initBlendEqationSupport(ctxInfo);
    this->initStencilFormats(ctxInfo);

    if (kGL_GrGLStandard == standard) {
        // we could also look for GL_ATI_separate_stencil extension or
        // GL_EXT_stencil_two_side but they use different function signatures
        // than GL2.0+ (and than each other).
        fTwoSidedStencilSupport = (ctxInfo.version() >= GR_GL_VER(2,0));
        // supported on GL 1.4 and higher or by extension
        fStencilWrapOpsSupport = (ctxInfo.version() >= GR_GL_VER(1,4)) ||
                                  ctxInfo.hasExtension("GL_EXT_stencil_wrap");
    } else {
        // ES 2 has two sided stencil and stencil wrap
        fTwoSidedStencilSupport = true;
        fStencilWrapOpsSupport = true;
    }

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

    // On many GPUs, map memory is very expensive, so we effectively disable it here by setting the
    // threshold to the maximum unless the client gives us a hint that map memory is cheap.
    if (fGeometryBufferMapThreshold < 0) {
        // We think mapping on Chromium will be cheaper once we know ahead of time how much space
        // we will use for all GrBatchs. Right now we might wind up mapping a large buffer and using
        // a small subset.
#if 0
        fGeometryBufferMapThreshold = kChromium_GrGLDriver == ctxInfo.driver() ? 0 : SK_MaxS32;
#else
        fGeometryBufferMapThreshold = SK_MaxS32;
#endif
    }

    if (kGL_GrGLStandard == standard) {
        SkASSERT(ctxInfo.version() >= GR_GL_VER(2,0) ||
                 ctxInfo.hasExtension("GL_ARB_texture_non_power_of_two"));
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

    // Using MIPs on this GPU seems to be a source of trouble.
    if (kPowerVR54x_GrGLRenderer == ctxInfo.renderer()) {
        fMipMapSupport = false;
    }

    GR_GL_GetIntegerv(gli, GR_GL_MAX_TEXTURE_SIZE, &fMaxTextureSize);
    GR_GL_GetIntegerv(gli, GR_GL_MAX_RENDERBUFFER_SIZE, &fMaxRenderTargetSize);
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fMaxRenderTargetSize = SkTMin(fMaxTextureSize, fMaxRenderTargetSize);

    fGpuTracingSupport = ctxInfo.hasExtension("GL_EXT_debug_marker");

    // Disable scratch texture reuse on Mali and Adreno devices
    fReuseScratchTextures = kARM_GrGLVendor != ctxInfo.vendor() &&
                            kQualcomm_GrGLVendor != ctxInfo.vendor();

#if 0
    fReuseScratchBuffers = kARM_GrGLVendor != ctxInfo.vendor() &&
                           kQualcomm_GrGLVendor != ctxInfo.vendor();
#endif

    // initFSAASupport() must have been called before this point
    if (GrGLCaps::kES_IMG_MsToTexture_MSFBOType == fMSFBOType) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_SAMPLES_IMG, &fMaxSampleCount);
    } else if (GrGLCaps::kNone_MSFBOType != fMSFBOType) {
        GR_GL_GetIntegerv(gli, GR_GL_MAX_SAMPLES, &fMaxSampleCount);
    }

    if (kPowerVR54x_GrGLRenderer == ctxInfo.renderer() ||
        kPowerVRRogue_GrGLRenderer == ctxInfo.renderer() ||
        kAdreno3xx_GrGLRenderer == ctxInfo.renderer()) {
        fUseDrawInsteadOfClear = true;
    }

    if (kAdreno4xx_GrGLRenderer == ctxInfo.renderer()) {
        fUseDrawInsteadOfPartialRenderTargetWrite = true;
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
        // 3.1 has draw_instanced but not instanced_arrays, for the time being we only care about
        // instanced arrays, but we could make this more granular if we wanted
        fSupportsInstancedDraws =
                version >= GR_GL_VER(3, 2) ||
                (ctxInfo.hasExtension("GL_ARB_draw_instanced") &&
                 ctxInfo.hasExtension("GL_ARB_instanced_arrays"));
    } else {
        fSupportsInstancedDraws =
                version >= GR_GL_VER(3, 0) ||
                (ctxInfo.hasExtension("GL_EXT_draw_instanced") &&
                 ctxInfo.hasExtension("GL_EXT_instanced_arrays"));
    }

    this->initConfigTexturableTable(ctxInfo, gli, srgbSupport);
    this->initConfigRenderableTable(ctxInfo, srgbSupport);
    this->initShaderPrecisionTable(ctxInfo, gli, glslCaps);
    // Requires fTexutreSwizzleSupport and fTextureRedSupport to be set before this point.
    this->initConfigSwizzleTable(ctxInfo, glslCaps);

    this->applyOptionsOverrides(contextOptions);
    glslCaps->applyOptionsOverrides(contextOptions);
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
        case k310es_GrGLSLGeneration:
            SkASSERT(kGLES_GrGLStandard == standard);
            return "#version 310 es\n";
    }
    return "<no version>";
}

void GrGLCaps::initGLSL(const GrGLContextInfo& ctxInfo) {
    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

    /**************************************************************************
    * Caps specific to GrGLSLCaps
    **************************************************************************/

    GrGLSLCaps* glslCaps = static_cast<GrGLSLCaps*>(fShaderCaps.get());
    glslCaps->fGLSLGeneration = ctxInfo.glslGeneration();

    if (kGLES_GrGLStandard == standard) {
        if (ctxInfo.hasExtension("GL_EXT_shader_framebuffer_fetch")) {
            glslCaps->fFBFetchNeedsCustomOutput = (version >= GR_GL_VER(3, 0));
            glslCaps->fFBFetchSupport = true;
            glslCaps->fFBFetchColorName = "gl_LastFragData[0]";
            glslCaps->fFBFetchExtensionString = "GL_EXT_shader_framebuffer_fetch";
        }
        else if (ctxInfo.hasExtension("GL_NV_shader_framebuffer_fetch")) {
            // Actually, we haven't seen an ES3.0 device with this extension yet, so we don't know
            glslCaps->fFBFetchNeedsCustomOutput = false;
            glslCaps->fFBFetchSupport = true;
            glslCaps->fFBFetchColorName = "gl_LastFragData[0]";
            glslCaps->fFBFetchExtensionString = "GL_NV_shader_framebuffer_fetch";
        }
        else if (ctxInfo.hasExtension("GL_ARM_shader_framebuffer_fetch")) {
            // The arm extension also requires an additional flag which we will set onResetContext
            glslCaps->fFBFetchNeedsCustomOutput = false;
            glslCaps->fFBFetchSupport = true;
            glslCaps->fFBFetchColorName = "gl_LastFragColorARM";
            glslCaps->fFBFetchExtensionString = "GL_ARM_shader_framebuffer_fetch";
        }
        glslCaps->fUsesPrecisionModifiers = true;
    }

    glslCaps->fBindlessTextureSupport = ctxInfo.hasExtension("GL_NV_bindless_texture");

    // Adreno GPUs have a tendency to drop tiles when there is a divide-by-zero in a shader
    glslCaps->fDropsTileOnZeroDivide = kQualcomm_GrGLVendor == ctxInfo.vendor();

    // On the NexusS and GalaxyNexus, the use of 'any' causes the compilation error "Calls to any
    // function that may require a gradient calculation inside a conditional block may return
    // undefined results". This appears to be an issue with the 'any' call since even the simple
    // "result=black; if (any()) result=white;" code fails to compile. This issue comes into play
    // from our GrTextureDomain processor.
    glslCaps->fCanUseAnyFunctionInShader = kImagination_GrGLVendor != ctxInfo.vendor();

    glslCaps->fForceHighPrecisionNDSTransform = kARM_GrGLVendor == ctxInfo.vendor() ||
                                                kPowerVR54x_GrGLRenderer == ctxInfo.renderer();

    glslCaps->fVersionDeclString = get_glsl_version_decl_string(standard, glslCaps->fGLSLGeneration,
                                                                fIsCoreProfile);

    if (kGLES_GrGLStandard == standard && k110_GrGLSLGeneration == glslCaps->fGLSLGeneration) {
        glslCaps->fShaderDerivativeExtensionString = "GL_OES_standard_derivatives";
    }

    // Frag Coords Convention support is not part of ES
    // Known issue on at least some Intel platforms:
    // http://code.google.com/p/skia/issues/detail?id=946
    if (kIntel_GrGLVendor != ctxInfo.vendor() &&
        kGLES_GrGLStandard != standard &&
        (ctxInfo.glslGeneration() >= k150_GrGLSLGeneration ||
         ctxInfo.hasExtension("GL_ARB_fragment_coord_conventions"))) {
        glslCaps->fFragCoordConventionsExtensionString = "GL_ARB_fragment_coord_conventions";
    }

    if (kGLES_GrGLStandard == standard) {
        glslCaps->fSecondaryOutputExtensionString = "GL_EXT_blend_func_extended";
    }

    if (fExternalTextureSupport) {
        if (ctxInfo.glslGeneration() == k110_GrGLSLGeneration) {
            glslCaps->fExternalTextureExtensionString = "GL_OES_EGL_image_external";
        } else {
            glslCaps->fExternalTextureExtensionString = "GL_OES_EGL_image_external_essl3";
        }
    }

    // The Tegra3 compiler will sometimes never return if we have min(abs(x), 1.0), so we must do
    // the abs first in a separate expression.
    if (kTegra3_GrGLRenderer == ctxInfo.renderer()) {
        glslCaps->fCanUseMinAndAbsTogether = false;
    }

    // On Intel GPU there is an issue where it reads the second argument to atan "- %s.x" as an int
    // thus must us -1.0 * %s.x to work correctly
    if (kIntel_GrGLVendor == ctxInfo.vendor()) {
        glslCaps->fMustForceNegatedAtanParamToFloat = true;
    }
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
    if (nullptr == gli->fFunctions.fStencilThenCoverFillPath ||
        nullptr == gli->fFunctions.fStencilThenCoverStrokePath ||
        nullptr == gli->fFunctions.fStencilThenCoverFillPathInstanced ||
        nullptr == gli->fFunctions.fStencilThenCoverStrokePathInstanced ||
        nullptr == gli->fFunctions.fProgramPathFragmentInputGen) {
        return false;
    }
    return true;
}

void GrGLCaps::initConfigRenderableTable(const GrGLContextInfo& ctxInfo, bool srgbSupport) {
    // OpenGL < 3.0
    //  no support for render targets unless the GL_ARB_framebuffer_object
    //  extension is supported (in which case we get ALPHA, RED, RG, RGB,
    //  RGBA (ALPHA8, RGBA4, RGBA8) for OpenGL > 1.1). Note that we
    //  probably don't get R8 in this case.

    // OpenGL 3.0
    //  base color renderable: ALPHA, RED, RG, RGB, and RGBA
    //  sized derivatives: ALPHA8, R8, RGBA4, RGBA8

    // >= OpenGL 3.1
    //  base color renderable: RED, RG, RGB, and RGBA
    //  sized derivatives: R8, RGBA4, RGBA8
    //  if the GL_ARB_compatibility extension is supported then we get back
    //  support for GL_ALPHA and ALPHA8

    // GL_EXT_bgra adds BGRA render targets to any version

    // ES 2.0
    //  color renderable: RGBA4, RGB5_A1, RGB565
    //  GL_EXT_texture_rg adds support for R8 as a color render target
    //  GL_OES_rgb8_rgba8 and/or GL_ARM_rgba8 adds support for RGBA8
    //  GL_EXT_texture_format_BGRA8888 and/or GL_APPLE_texture_format_BGRA8888 added BGRA support

    // ES 3.0
    // Same as ES 2.0 except R8 and RGBA8 are supported without extensions (the functions called
    // below already account for this).

    GrGLStandard standard = ctxInfo.standard();

    enum {
        kNo_MSAA = 0,
        kYes_MSAA = 1,
    };

    if (kGL_GrGLStandard == standard) {
        // Post 3.0 we will get R8
        // Prior to 3.0 we will get ALPHA8 (with GL_ARB_framebuffer_object)
        if (ctxInfo.version() >= GR_GL_VER(3,0) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fConfigRenderSupport[kAlpha_8_GrPixelConfig][kNo_MSAA] = true;
            fConfigRenderSupport[kAlpha_8_GrPixelConfig][kYes_MSAA] = true;
        }
    } else {
        // On ES we can only hope for R8
        fConfigRenderSupport[kAlpha_8_GrPixelConfig][kNo_MSAA] = fTextureRedSupport;
        fConfigRenderSupport[kAlpha_8_GrPixelConfig][kYes_MSAA] = fTextureRedSupport;
    }

    if (kGL_GrGLStandard != standard) {
        // only available in ES
        fConfigRenderSupport[kRGB_565_GrPixelConfig][kNo_MSAA] = true;
        fConfigRenderSupport[kRGB_565_GrPixelConfig][kYes_MSAA] = true;
    }

    // we no longer support 444 as a render target
    fConfigRenderSupport[kRGBA_4444_GrPixelConfig][kNo_MSAA]  = false;
    fConfigRenderSupport[kRGBA_4444_GrPixelConfig][kYes_MSAA]  = false;

    if (this->fRGBA8RenderbufferSupport) {
        fConfigRenderSupport[kRGBA_8888_GrPixelConfig][kNo_MSAA]  = true;
        fConfigRenderSupport[kRGBA_8888_GrPixelConfig][kYes_MSAA]  = true;
    }

    if (this->isConfigTexturable(kBGRA_8888_GrPixelConfig)) {
        fConfigRenderSupport[kBGRA_8888_GrPixelConfig][kNo_MSAA]  = true;
        // The GL_EXT_texture_format_BGRA8888 extension does not add BGRA to the list of
        // configs that are color-renderable and can be passed to glRenderBufferStorageMultisample.
        // Chromium may have an extension to allow BGRA renderbuffers to work on desktop platforms.
        if (ctxInfo.hasExtension("GL_CHROMIUM_renderbuffer_format_BGRA8888")) {
            fConfigRenderSupport[kBGRA_8888_GrPixelConfig][kYes_MSAA] = true;
        } else {
            fConfigRenderSupport[kBGRA_8888_GrPixelConfig][kYes_MSAA] =
                !fBGRAIsInternalFormat || !this->usesMSAARenderBuffers();
        }
    }

    if (this->fRGBA8RenderbufferSupport && srgbSupport) {
        fConfigRenderSupport[kSRGBA_8888_GrPixelConfig][kNo_MSAA] = true;
        fConfigRenderSupport[kSRGBA_8888_GrPixelConfig][kYes_MSAA] = true;
    }
    
    if (this->isConfigTexturable(kRGBA_float_GrPixelConfig)) {
        if (kGL_GrGLStandard == standard) {
            fConfigRenderSupport[kRGBA_float_GrPixelConfig][kNo_MSAA] = true;
            fConfigRenderSupport[kRGBA_float_GrPixelConfig][kYes_MSAA] = true;
        } else {
            // for now we only enable this on desktop, because on ES we'd have to solve many
            // precision issues and no clients actually want this yet
            /*
            if (ctxInfo.hasExtension("GL_EXT_color_buffer_float")) {
                fConfigRenderSupport[kRGBA_float_GrPixelConfig][kNo_MSAA] = true;
            } else {
                fConfigRenderSupport[kRGBA_float_GrPixelConfig][kNo_MSAA] = false;
            }
            // for now we don't support floating point MSAA on ES
            fConfigRenderSupport[kRGBA_float_GrPixelConfig][kYes_MSAA] = false;*/
            fConfigRenderSupport[kRGBA_float_GrPixelConfig][kNo_MSAA] = false;
            fConfigRenderSupport[kRGBA_float_GrPixelConfig][kYes_MSAA] = false;
        }
    }

    if (this->isConfigTexturable(kAlpha_half_GrPixelConfig)) {
        if (kGL_GrGLStandard == standard) {
            fConfigRenderSupport[kAlpha_half_GrPixelConfig][kNo_MSAA] = true;
            fConfigRenderSupport[kAlpha_half_GrPixelConfig][kYes_MSAA] = true;
        } else if (ctxInfo.version() >= GR_GL_VER(3,0)) {
            fConfigRenderSupport[kAlpha_half_GrPixelConfig][kNo_MSAA] = true;
            // for now we don't support floating point MSAA on ES
            fConfigRenderSupport[kAlpha_half_GrPixelConfig][kYes_MSAA] = false;
        } else {
            if (ctxInfo.hasExtension("GL_EXT_color_buffer_half_float") && fTextureRedSupport) {
                fConfigRenderSupport[kAlpha_half_GrPixelConfig][kNo_MSAA] = true;
            } else {
                fConfigRenderSupport[kAlpha_half_GrPixelConfig][kNo_MSAA] = false;
            }
            // for now we don't support floating point MSAA on ES
            fConfigRenderSupport[kAlpha_half_GrPixelConfig][kYes_MSAA] = false;
        }
    }
    
    if (this->isConfigTexturable(kRGBA_half_GrPixelConfig)) {
        if (kGL_GrGLStandard == standard) {
            fConfigRenderSupport[kRGBA_half_GrPixelConfig][kNo_MSAA] = true;
            fConfigRenderSupport[kRGBA_half_GrPixelConfig][kYes_MSAA] = true;
        } else if (ctxInfo.version() >= GR_GL_VER(3, 0)) {
            fConfigRenderSupport[kRGBA_half_GrPixelConfig][kNo_MSAA] = true;
            // for now we don't support floating point MSAA on ES
            fConfigRenderSupport[kRGBA_half_GrPixelConfig][kYes_MSAA] = false;
        } else {
            if (ctxInfo.hasExtension("GL_EXT_color_buffer_half_float")) {
                fConfigRenderSupport[kRGBA_half_GrPixelConfig][kNo_MSAA] = true;
            } else {
                fConfigRenderSupport[kRGBA_half_GrPixelConfig][kNo_MSAA] = false;
            }
            // for now we don't support floating point MSAA on ES
            fConfigRenderSupport[kRGBA_half_GrPixelConfig][kYes_MSAA] = false;
        }
    }

    // If we don't support MSAA then undo any places above where we set a config as renderable with
    // msaa.
    if (kNone_MSFBOType == fMSFBOType) {
        for (int i = 0; i < kGrPixelConfigCnt; ++i) {
            fConfigRenderSupport[i][kYes_MSAA] = false;
        }
    }
}

void GrGLCaps::initConfigTexturableTable(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli,
                                         bool srgbSupport) {
    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

    // Base texture support
    fConfigTextureSupport[kAlpha_8_GrPixelConfig] = true;
    fConfigTextureSupport[kRGB_565_GrPixelConfig] = true;
    fConfigTextureSupport[kRGBA_4444_GrPixelConfig] = true;
    fConfigTextureSupport[kRGBA_8888_GrPixelConfig] = true;

    // Disable this for now, while we investigate https://bug.skia.org/4333
    if (false) {
        // Check for 8-bit palette..
        GrGLint numFormats;
        GR_GL_GetIntegerv(gli, GR_GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFormats);
        if (numFormats) {
            SkAutoSTMalloc<10, GrGLint> formats(numFormats);
            GR_GL_GetIntegerv(gli, GR_GL_COMPRESSED_TEXTURE_FORMATS, formats);
            for (int i = 0; i < numFormats; ++i) {
                if (GR_GL_PALETTE8_RGBA8 == formats[i]) {
                    fConfigTextureSupport[kIndex_8_GrPixelConfig] = true;
                    break;
                }
            }
        }
    }

    // Check for BGRA
    if (kGL_GrGLStandard == standard) {
        fConfigTextureSupport[kBGRA_8888_GrPixelConfig] =
            version >= GR_GL_VER(1,2) || ctxInfo.hasExtension("GL_EXT_bgra");
    } else {
        if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            fConfigTextureSupport[kBGRA_8888_GrPixelConfig] = true;
        } else if (ctxInfo.hasExtension("GL_EXT_texture_format_BGRA8888")) {
            fConfigTextureSupport[kBGRA_8888_GrPixelConfig] = true;
            fBGRAIsInternalFormat = true;
        }
        SkASSERT(fConfigTextureSupport[kBGRA_8888_GrPixelConfig] ||
                 kSkia8888_GrPixelConfig != kBGRA_8888_GrPixelConfig);
    }

    fConfigTextureSupport[kSRGBA_8888_GrPixelConfig] = srgbSupport;
    
    // Compressed texture support

    // glCompressedTexImage2D is available on all OpenGL ES devices...
    // however, it is only available on standard OpenGL after version 1.3
    bool hasCompressTex2D = (kGL_GrGLStandard != standard || version >= GR_GL_VER(1, 3));

    fCompressedTexSubImageSupport =
        hasCompressTex2D && (gli->fFunctions.fCompressedTexSubImage2D);

    // Check for ETC1
    bool hasETC1 = false;

    // First check version for support
    if (kGL_GrGLStandard == standard) {
        hasETC1 = hasCompressTex2D &&
            (version >= GR_GL_VER(4, 3) ||
             ctxInfo.hasExtension("GL_ARB_ES3_compatibility"));
    } else {
        hasETC1 = hasCompressTex2D &&
            (version >= GR_GL_VER(3, 0) ||
             ctxInfo.hasExtension("GL_OES_compressed_ETC1_RGB8_texture") ||
             // ETC2 is a superset of ETC1, so we can just check for that, too.
             (ctxInfo.hasExtension("GL_OES_compressed_ETC2_RGB8_texture") &&
              ctxInfo.hasExtension("GL_OES_compressed_ETC2_RGBA8_texture")));
    }
    fConfigTextureSupport[kETC1_GrPixelConfig] = hasETC1;

    // Check for LATC under its various forms
    LATCAlias alias = kLATC_LATCAlias;
    bool hasLATC = hasCompressTex2D &&
        (ctxInfo.hasExtension("GL_EXT_texture_compression_latc") ||
         ctxInfo.hasExtension("GL_NV_texture_compression_latc"));

    // Check for RGTC
    if (!hasLATC) {
        // If we're using OpenGL 3.0 or later, then we have RGTC, an identical compression format.
        if (kGL_GrGLStandard == standard) {
            hasLATC = version >= GR_GL_VER(3, 0);
        }

        if (!hasLATC) {
            hasLATC =
                ctxInfo.hasExtension("GL_EXT_texture_compression_rgtc") ||
                ctxInfo.hasExtension("GL_ARB_texture_compression_rgtc");
        }

        if (hasLATC) {
            alias = kRGTC_LATCAlias;
        }
    }

    // Check for 3DC
    if (!hasLATC) {
        hasLATC = ctxInfo.hasExtension("GL_AMD_compressed_3DC_texture");
        if (hasLATC) {
            alias = k3DC_LATCAlias;
        }
    }

    fConfigTextureSupport[kLATC_GrPixelConfig] = hasLATC;
    fLATCAlias = alias;

    // Check for R11_EAC ... We don't support R11_EAC on desktop, as most
    // cards default to decompressing the textures in the driver, and is
    // generally slower.
    if (kGL_GrGLStandard != standard) {
        fConfigTextureSupport[kR11_EAC_GrPixelConfig] = version >= GR_GL_VER(3, 0);
    }

    // Check for ASTC
    fConfigTextureSupport[kASTC_12x12_GrPixelConfig] =
        ctxInfo.hasExtension("GL_KHR_texture_compression_astc_hdr") ||
        ctxInfo.hasExtension("GL_KHR_texture_compression_astc_ldr") ||
        ctxInfo.hasExtension("GL_OES_texture_compression_astc");

    // Check for floating point texture support
    // NOTE: We disallow floating point textures on ES devices if linear
    // filtering modes are not supported.  This is for simplicity, but a more
    // granular approach is possible.  Coincidentally, floating point textures became part of
    // the standard in ES3.1 / OGL 3.1, hence the shorthand
    bool hasFPTextures = version >= GR_GL_VER(3, 1);
    if (!hasFPTextures) {
        hasFPTextures = ctxInfo.hasExtension("GL_ARB_texture_float") ||
                        (ctxInfo.hasExtension("GL_OES_texture_float_linear") &&
                         ctxInfo.hasExtension("GL_OES_texture_float"));
    }
    fConfigTextureSupport[kRGBA_float_GrPixelConfig] = hasFPTextures;

    // Check for fp16 texture support
    // NOTE: We disallow floating point textures on ES devices if linear
    // filtering modes are not supported.  This is for simplicity, but a more
    // granular approach is possible.  Coincidentally, 16-bit floating point textures became part of
    // the standard in ES3.1 / OGL 3.1, hence the shorthand
    bool hasHalfFPTextures = version >= GR_GL_VER(3, 1);
    if (!hasHalfFPTextures) {
        hasHalfFPTextures = ctxInfo.hasExtension("GL_ARB_texture_float") ||
                            (ctxInfo.hasExtension("GL_OES_texture_half_float_linear") &&
                             ctxInfo.hasExtension("GL_OES_texture_half_float"));
    }
    fConfigTextureSupport[kAlpha_half_GrPixelConfig] = hasHalfFPTextures;
    fConfigTextureSupport[kRGBA_half_GrPixelConfig] = hasHalfFPTextures;
}

bool GrGLCaps::doReadPixelsSupported(const GrGLInterface* intf,
                                     GrGLenum format,
                                     GrGLenum type) const {
    if (GR_GL_RGBA == format && GR_GL_UNSIGNED_BYTE == type) {
        // ES 2 guarantees this format is supported
        return true;
    }

    if (!fTwoFormatLimit) {
        // not limited by ES 2's constraints
        return true;
    }

    GrGLint otherFormat = GR_GL_RGBA;
    GrGLint otherType = GR_GL_UNSIGNED_BYTE;

    // The other supported format/type combo supported for ReadPixels
    // can change based on which render target is bound
    GR_GL_GetIntegerv(intf,
                      GR_GL_IMPLEMENTATION_COLOR_READ_FORMAT,
                      &otherFormat);

    GR_GL_GetIntegerv(intf,
                      GR_GL_IMPLEMENTATION_COLOR_READ_TYPE,
                      &otherType);

    return (GrGLenum)otherFormat == format && (GrGLenum)otherType == type;
}

bool GrGLCaps::readPixelsSupported(const GrGLInterface* intf,
                                   GrGLenum format,
                                   GrGLenum type,
                                   GrGLenum currFboFormat) const {
    ReadPixelsSupportedFormat key = {format, type, currFboFormat};
    if (const bool* supported = fReadPixelsSupportedCache.find(key)) {
        return *supported;
    }
    bool supported = this->doReadPixelsSupported(intf, format, type);
    fReadPixelsSupportedCache.set(key, supported);
    return supported;
}

void GrGLCaps::initFSAASupport(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli) {

    fMSFBOType = kNone_MSFBOType;
    if (kGL_GrGLStandard != ctxInfo.standard()) {
        // We prefer the EXT/IMG extension over ES3 MSAA because we've observed
        // ES3 driver bugs on at least one device with a tiled GPU (N10).
        if (ctxInfo.hasExtension("GL_EXT_multisampled_render_to_texture")) {
            fMSFBOType = kES_EXT_MsToTexture_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_IMG_multisampled_render_to_texture")) {
            fMSFBOType = kES_IMG_MsToTexture_MSFBOType;
        } else if (fMixedSamplesSupport && fShaderCaps->pathRenderingSupport()) {
            fMSFBOType = kMixedSamples_MSFBOType;
        } else if (ctxInfo.version() >= GR_GL_VER(3,0)) {
            fMSFBOType = GrGLCaps::kES_3_0_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_multisample")) {
            // chrome's extension is equivalent to the EXT msaa
            // and fbo_blit extensions.
            fMSFBOType = kDesktop_EXT_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_APPLE_framebuffer_multisample")) {
            fMSFBOType = kES_Apple_MSFBOType;
        }
    } else {
        if (fMixedSamplesSupport && fShaderCaps->pathRenderingSupport()) {
            fMSFBOType = kMixedSamples_MSFBOType;
        } else if ((ctxInfo.version() >= GR_GL_VER(3,0)) ||
            ctxInfo.hasExtension("GL_ARB_framebuffer_object")) {
            fMSFBOType = GrGLCaps::kDesktop_ARB_MSFBOType;
        } else if (ctxInfo.hasExtension("GL_EXT_framebuffer_multisample") &&
                   ctxInfo.hasExtension("GL_EXT_framebuffer_blit")) {
            fMSFBOType = GrGLCaps::kDesktop_EXT_MSFBOType;
        }
    }
}

void GrGLCaps::initBlendEqationSupport(const GrGLContextInfo& ctxInfo) {
    GrGLSLCaps* glslCaps = static_cast<GrGLSLCaps*>(fShaderCaps.get());

    // Disabling advanced blend on various platforms with major known issues. We also block Chrome
    // for now until its own blacklists can be updated.
    if (kAdreno4xx_GrGLRenderer == ctxInfo.renderer() ||
        kIntel_GrGLDriver == ctxInfo.driver() ||
        kChromium_GrGLDriver == ctxInfo.driver()) {
        return;
    }

    if (ctxInfo.hasExtension("GL_NV_blend_equation_advanced_coherent")) {
        fBlendEquationSupport = kAdvancedCoherent_BlendEquationSupport;
        glslCaps->fAdvBlendEqInteraction = GrGLSLCaps::kAutomatic_AdvBlendEqInteraction;
    } else if (ctxInfo.hasExtension("GL_KHR_blend_equation_advanced_coherent")) {
        fBlendEquationSupport = kAdvancedCoherent_BlendEquationSupport;
        glslCaps->fAdvBlendEqInteraction = GrGLSLCaps::kGeneralEnable_AdvBlendEqInteraction;
    } else if (kNVIDIA_GrGLDriver == ctxInfo.driver() &&
               ctxInfo.driverVersion() < GR_GL_DRIVER_VER(337,00)) {
        // Non-coherent advanced blend has an issue on NVIDIA pre 337.00.
        return;
    } else if (ctxInfo.hasExtension("GL_NV_blend_equation_advanced")) {
        fBlendEquationSupport = kAdvanced_BlendEquationSupport;
        glslCaps->fAdvBlendEqInteraction = GrGLSLCaps::kAutomatic_AdvBlendEqInteraction;
    } else if (ctxInfo.hasExtension("GL_KHR_blend_equation_advanced")) {
        fBlendEquationSupport = kAdvanced_BlendEquationSupport;
        glslCaps->fAdvBlendEqInteraction = GrGLSLCaps::kGeneralEnable_AdvBlendEqInteraction;
        // TODO: Use kSpecificEnables_AdvBlendEqInteraction if "blend_support_all_equations" is
        // slow on a particular platform.
    } else {
        return; // No advanced blend support.
    }

    SkASSERT(this->advancedBlendEquationSupport());

    if (kNVIDIA_GrGLDriver == ctxInfo.driver()) {
        // Blacklist color-dodge and color-burn on NVIDIA until the fix is released.
        fAdvBlendEqBlacklist |= (1 << kColorDodge_GrBlendEquation) |
                                (1 << kColorBurn_GrBlendEquation);
    }
    if (kARM_GrGLVendor == ctxInfo.vendor()) {
        // Blacklist color-burn on ARM until the fix is released.
        fAdvBlendEqBlacklist |= (1 << kColorBurn_GrBlendEquation);
    }
}

namespace {
const GrGLuint kUnknownBitCount = GrGLStencilAttachment::kUnknownBitCount;
}

void GrGLCaps::initStencilFormats(const GrGLContextInfo& ctxInfo) {

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

SkString GrGLCaps::dump() const {

    SkString r = INHERITED::dump();

    r.appendf("--- GL-Specific ---\n");
    for (int i = 0; i < fStencilFormats.count(); ++i) {
        r.appendf("Stencil Format %d, stencil bits: %02d, total bits: %02d\n",
                 i,
                 fStencilFormats[i].fStencilBits,
                 fStencilFormats[i].fTotalBits);
    }

    static const char* kMSFBOExtStr[] = {
        "None",
        "ARB",
        "EXT",
        "ES 3.0",
        "Apple",
        "IMG MS To Texture",
        "EXT MS To Texture",
        "MixedSamples",
    };
    GR_STATIC_ASSERT(0 == kNone_MSFBOType);
    GR_STATIC_ASSERT(1 == kDesktop_ARB_MSFBOType);
    GR_STATIC_ASSERT(2 == kDesktop_EXT_MSFBOType);
    GR_STATIC_ASSERT(3 == kES_3_0_MSFBOType);
    GR_STATIC_ASSERT(4 == kES_Apple_MSFBOType);
    GR_STATIC_ASSERT(5 == kES_IMG_MsToTexture_MSFBOType);
    GR_STATIC_ASSERT(6 == kES_EXT_MsToTexture_MSFBOType);
    GR_STATIC_ASSERT(7 == kMixedSamples_MSFBOType);
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

    r.appendf("Core Profile: %s\n", (fIsCoreProfile ? "YES" : "NO"));
    r.appendf("MSAA Type: %s\n", kMSFBOExtStr[fMSFBOType]);
    r.appendf("Invalidate FB Type: %s\n", kInvalidateFBTypeStr[fInvalidateFBType]);
    r.appendf("Map Buffer Type: %s\n", kMapBufferTypeStr[fMapBufferType]);
    r.appendf("Max FS Uniform Vectors: %d\n", fMaxFragmentUniformVectors);
    r.appendf("Max FS Texture Units: %d\n", fMaxFragmentTextureUnits);
    r.appendf("Max Vertex Attributes: %d\n", fMaxVertexAttributes);
    r.appendf("Support RGBA8 Render Buffer: %s\n", (fRGBA8RenderbufferSupport ? "YES": "NO"));
    r.appendf("BGRA is an internal format: %s\n", (fBGRAIsInternalFormat ? "YES": "NO"));
    r.appendf("Unpack Row length support: %s\n", (fUnpackRowLengthSupport ? "YES": "NO"));
    r.appendf("Unpack Flip Y support: %s\n", (fUnpackFlipYSupport ? "YES": "NO"));
    r.appendf("Pack Row length support: %s\n", (fPackRowLengthSupport ? "YES": "NO"));
    r.appendf("Pack Flip Y support: %s\n", (fPackFlipYSupport ? "YES": "NO"));

    r.appendf("Texture Usage support: %s\n", (fTextureUsageSupport ? "YES": "NO"));
    r.appendf("Texture Storage support: %s\n", (fTexStorageSupport ? "YES": "NO"));
    r.appendf("GL_R support: %s\n", (fTextureRedSupport ? "YES": "NO"));
    r.appendf("GL_ARB_imaging support: %s\n", (fImagingSupport ? "YES": "NO"));
    r.appendf("Two Format Limit: %s\n", (fTwoFormatLimit ? "YES": "NO"));
    r.appendf("Vertex array object support: %s\n", (fVertexArrayObjectSupport ? "YES": "NO"));
    r.appendf("Instanced drawing support: %s\n", (fInstancedDrawingSupport ? "YES": "NO"));
    r.appendf("Direct state access support: %s\n", (fDirectStateAccessSupport ? "YES": "NO"));
    r.appendf("Debug support: %s\n", (fDebugSupport ? "YES": "NO"));
    r.appendf("Multisample disable support: %s\n", (fMultisampleDisableSupport ? "YES" : "NO"));
    r.appendf("Use non-VBO for dynamic data: %s\n",
             (fUseNonVBOVertexAndIndexDynamicData ? "YES" : "NO"));
    r.appendf("SRGB write contol: %s\n", (fSRGBWriteControl ? "YES" : "NO"));
    r.appendf("RGBA 8888 pixel ops are slow: %s\n", (fRGBA8888PixelsOpsAreSlow ? "YES" : "NO"));
    r.appendf("Partial FBO read is slow: %s\n", (fPartialFBOReadIsSlow ? "YES" : "NO"));
    r.appendf("Bind uniform location support: %s\n", (fBindUniformLocationSupport ? "YES" : "NO"));
    return r;
}

static GrGLenum precision_to_gl_float_type(GrSLPrecision p) {
    switch (p) {
    case kLow_GrSLPrecision:
        return GR_GL_LOW_FLOAT;
    case kMedium_GrSLPrecision:
        return GR_GL_MEDIUM_FLOAT;
    case kHigh_GrSLPrecision:
        return GR_GL_HIGH_FLOAT;
    }
    SkFAIL("Unknown precision.");
    return -1;
}

static GrGLenum shader_type_to_gl_shader(GrShaderType type) {
    switch (type) {
    case kVertex_GrShaderType:
        return GR_GL_VERTEX_SHADER;
    case kGeometry_GrShaderType:
        return GR_GL_GEOMETRY_SHADER;
    case kFragment_GrShaderType:
        return GR_GL_FRAGMENT_SHADER;
    }
    SkFAIL("Unknown shader type.");
    return -1;
}

void GrGLCaps::initShaderPrecisionTable(const GrGLContextInfo& ctxInfo,
                                        const GrGLInterface* intf, 
                                        GrGLSLCaps* glslCaps) {
    if (kGLES_GrGLStandard == ctxInfo.standard() || ctxInfo.version() >= GR_GL_VER(4, 1) ||
        ctxInfo.hasExtension("GL_ARB_ES2_compatibility")) {
        for (int s = 0; s < kGrShaderTypeCount; ++s) {
            if (kGeometry_GrShaderType != s) {
                GrShaderType shaderType = static_cast<GrShaderType>(s);
                GrGLenum glShader = shader_type_to_gl_shader(shaderType);
                GrShaderCaps::PrecisionInfo* first = nullptr;
                glslCaps->fShaderPrecisionVaries = false;
                for (int p = 0; p < kGrSLPrecisionCount; ++p) {
                    GrSLPrecision precision = static_cast<GrSLPrecision>(p);
                    GrGLenum glPrecision = precision_to_gl_float_type(precision);
                    GrGLint range[2];
                    GrGLint bits;
                    GR_GL_GetShaderPrecisionFormat(intf, glShader, glPrecision, range, &bits);
                    if (bits) {
                        glslCaps->fFloatPrecisions[s][p].fLogRangeLow = range[0];
                        glslCaps->fFloatPrecisions[s][p].fLogRangeHigh = range[1];
                        glslCaps->fFloatPrecisions[s][p].fBits = bits;
                        if (!first) {
                            first = &glslCaps->fFloatPrecisions[s][p];
                        }
                        else if (!glslCaps->fShaderPrecisionVaries) {
                            glslCaps->fShaderPrecisionVaries = 
                                                     (*first != glslCaps->fFloatPrecisions[s][p]);
                        }
                    }
                }
            }
        }
    }
    else {
        // We're on a desktop GL that doesn't have precision info. Assume they're all 32bit float.
        glslCaps->fShaderPrecisionVaries = false;
        for (int s = 0; s < kGrShaderTypeCount; ++s) {
            if (kGeometry_GrShaderType != s) {
                for (int p = 0; p < kGrSLPrecisionCount; ++p) {
                    glslCaps->fFloatPrecisions[s][p].fLogRangeLow = 127;
                    glslCaps->fFloatPrecisions[s][p].fLogRangeHigh = 127;
                    glslCaps->fFloatPrecisions[s][p].fBits = 23;
                }
            }
        }
    }
    // GetShaderPrecisionFormat doesn't accept GL_GEOMETRY_SHADER as a shader type. Assume they're
    // the same as the vertex shader. Only fragment shaders were ever allowed to omit support for
    // highp. GS was added after GetShaderPrecisionFormat was added to the list of features that
    // are recommended against.
    if (glslCaps->fGeometryShaderSupport) {
        for (int p = 0; p < kGrSLPrecisionCount; ++p) {
            glslCaps->fFloatPrecisions[kGeometry_GrShaderType][p] = 
                                               glslCaps->fFloatPrecisions[kVertex_GrShaderType][p];
        }
    }
}

void GrGLCaps::initConfigSwizzleTable(const GrGLContextInfo& ctxInfo, GrGLSLCaps* glslCaps) {
    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

    glslCaps->fMustSwizzleInShader = true;
    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3,3) || ctxInfo.hasExtension("GL_ARB_texture_swizzle")) {
            glslCaps->fMustSwizzleInShader = false;
        }
    } else {
        if (version >= GR_GL_VER(3,0)) {
            glslCaps->fMustSwizzleInShader = false;
        }
    }

    glslCaps->fConfigSwizzle[kUnknown_GrPixelConfig] = nullptr;
    if (fTextureRedSupport) {
        glslCaps->fConfigSwizzle[kAlpha_8_GrPixelConfig] = "rrrr";
        glslCaps->fConfigSwizzle[kAlpha_half_GrPixelConfig] = "rrrr";
    } else {
        glslCaps->fConfigSwizzle[kAlpha_8_GrPixelConfig] = "aaaa";
        glslCaps->fConfigSwizzle[kAlpha_half_GrPixelConfig] = "aaaa";
    }
    glslCaps->fConfigSwizzle[kIndex_8_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kRGB_565_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kRGBA_4444_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kRGBA_8888_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kBGRA_8888_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kSRGBA_8888_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kETC1_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kLATC_GrPixelConfig] = "rrrr";
    glslCaps->fConfigSwizzle[kR11_EAC_GrPixelConfig] = "rrrr";
    glslCaps->fConfigSwizzle[kASTC_12x12_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kRGBA_float_GrPixelConfig] = "rgba";
    glslCaps->fConfigSwizzle[kRGBA_half_GrPixelConfig] = "rgba";

}

void GrGLCaps::onApplyOptionsOverrides(const GrContextOptions& options) {}


