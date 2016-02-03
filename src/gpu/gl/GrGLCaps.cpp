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
    fStencilFormats.reset();
    fMSFBOType = kNone_MSFBOType;
    fInvalidateFBType = kNone_InvalidateFBType;
    fMapBufferType = kNone_MapBufferType;
    fTransferBufferType = kNone_TransferBufferType;
    fMaxFragmentUniformVectors = 0;
    fMaxVertexAttributes = 0;
    fMaxFragmentTextureUnits = 0;
    fUnpackRowLengthSupport = false;
    fUnpackFlipYSupport = false;
    fPackRowLengthSupport = false;
    fPackFlipYSupport = false;
    fTextureUsageSupport = false;
    fTexStorageSupport = false;
    fTextureRedSupport = false;
    fImagingSupport = false;
    fVertexArrayObjectSupport = false;
    fDirectStateAccessSupport = false;
    fDebugSupport = false;
    fES2CompatibilitySupport = false;
    fMultisampleDisableSupport = false;
    fUseNonVBOVertexAndIndexDynamicData = false;
    fIsCoreProfile = false;
    fBindFragDataLocationSupport = false;
    fExternalTextureSupport = false;
    fRectangleTextureSupport = false;
    fTextureSwizzleSupport = false;
    fSRGBWriteControl = false;
    fRGBA8888PixelsOpsAreSlow = false;
    fPartialFBOReadIsSlow = false;

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

    if ((kGL_GrGLStandard == standard && version >= GR_GL_VER(3, 2)) ||
        ctxInfo.hasExtension("GL_ARB_texture_rectangle")) {
        fRectangleTextureSupport = true;
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
        fMixedSamplesSupport = ctxInfo.hasExtension("GL_NV_framebuffer_mixed_samples") ||
                ctxInfo.hasExtension("GL_CHROMIUM_framebuffer_mixed_samples");
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

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_pixel_buffer_object")) {
            fTransferBufferType = kPBO_TransferBufferType;
        } 
    } else {
        if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_NV_pixel_buffer_object")) {
            fTransferBufferType = kPBO_TransferBufferType;
        } else if (ctxInfo.hasExtension("GL_CHROMIUM_pixel_transfer_buffer_object")) {
            fTransferBufferType = kChromium_TransferBufferType;
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

    this->initShaderPrecisionTable(ctxInfo, gli, glslCaps);

    if (contextOptions.fUseShaderSwizzling) {
        fTextureSwizzleSupport = false;
    }

    // Requires fTextureRedSupport, fTextureSwizzleSupport, msaa support, ES compatibility have
    // already been detected.
    this->initConfigTable(ctxInfo, gli, glslCaps);

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
bool GrGLCaps::readPixelsSupported(const GrGLInterface* intf,
                                   GrPixelConfig readConfig,
                                   GrPixelConfig currFBOConfig) const {
    SkASSERT(this->isConfigRenderable(currFBOConfig, false));

    GrGLenum readFormat;
    GrGLenum readType;
    if (!this->getReadPixelsFormat(currFBOConfig, readConfig, &readFormat, &readType)) {
        return false;
    }

    if (kGL_GrGLStandard == intf->fStandard) {
        // All of our renderable configs can be converted to each other by glReadPixels in OpenGL.
        return true;
    }

    // See Section 16.1.2 in the ES 3.2 specification.

    if (kNormalizedFixedPoint_FormatType == fConfigTable[currFBOConfig].fFormatType) {
        if (GR_GL_RGBA == readFormat && GR_GL_UNSIGNED_BYTE == readType) {
            return true;
        }
    } else {
        SkASSERT(kFloat_FormatType == fConfigTable[currFBOConfig].fFormatType);
        if (GR_GL_RGBA == readFormat && GR_GL_FLOAT == readType) {
            return true;
        }
    }

    if (0 == fConfigTable[currFBOConfig].fSecondReadPixelsFormat.fFormat) {
        ReadPixelsFormat* rpFormat =
            const_cast<ReadPixelsFormat*>(&fConfigTable[currFBOConfig].fSecondReadPixelsFormat);
        GrGLint format = 0, type = 0;
        GR_GL_GetIntegerv(intf, GR_GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
        GR_GL_GetIntegerv(intf, GR_GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
        rpFormat->fFormat = format;
        rpFormat->fType = type;
    }

    return fConfigTable[currFBOConfig].fSecondReadPixelsFormat.fFormat == readFormat &&
           fConfigTable[currFBOConfig].fSecondReadPixelsFormat.fType == readType;
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
    r.appendf("Unpack Row length support: %s\n", (fUnpackRowLengthSupport ? "YES": "NO"));
    r.appendf("Unpack Flip Y support: %s\n", (fUnpackFlipYSupport ? "YES": "NO"));
    r.appendf("Pack Row length support: %s\n", (fPackRowLengthSupport ? "YES": "NO"));
    r.appendf("Pack Flip Y support: %s\n", (fPackFlipYSupport ? "YES": "NO"));

    r.appendf("Texture Usage support: %s\n", (fTextureUsageSupport ? "YES": "NO"));
    r.appendf("Texture Storage support: %s\n", (fTexStorageSupport ? "YES": "NO"));
    r.appendf("GL_R support: %s\n", (fTextureRedSupport ? "YES": "NO"));
    r.appendf("GL_ARB_imaging support: %s\n", (fImagingSupport ? "YES": "NO"));
    r.appendf("Vertex array object support: %s\n", (fVertexArrayObjectSupport ? "YES": "NO"));
    r.appendf("Direct state access support: %s\n", (fDirectStateAccessSupport ? "YES": "NO"));
    r.appendf("Debug support: %s\n", (fDebugSupport ? "YES": "NO"));
    r.appendf("Multisample disable support: %s\n", (fMultisampleDisableSupport ? "YES" : "NO"));
    r.appendf("Use non-VBO for dynamic data: %s\n",
             (fUseNonVBOVertexAndIndexDynamicData ? "YES" : "NO"));
    r.appendf("SRGB write contol: %s\n", (fSRGBWriteControl ? "YES" : "NO"));
    r.appendf("RGBA 8888 pixel ops are slow: %s\n", (fRGBA8888PixelsOpsAreSlow ? "YES" : "NO"));
    r.appendf("Partial FBO read is slow: %s\n", (fPartialFBOReadIsSlow ? "YES" : "NO"));
    r.appendf("Bind uniform location support: %s\n", (fBindUniformLocationSupport ? "YES" : "NO"));
    r.appendf("External texture support: %s\n", (fExternalTextureSupport ? "YES" : "NO"));
    r.appendf("Rectangle texture support: %s\n", (fRectangleTextureSupport? "YES" : "NO"));
    r.appendf("Texture swizzle support: %s\n", (fTextureSwizzleSupport ? "YES" : "NO"));

    r.append("Configs\n-------\n");
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        r.appendf("  cfg: %d flags: 0x%04x, b_internal: 0x%08x s_internal: 0x%08x, e_format: "
                  "0x%08x, e_format_teximage: 0x%08x, e_type: 0x%08x, i_for_teximage: 0x%08x, "
                  "i_for_renderbuffer: 0x%08x\n",
                  i,
                  fConfigTable[i].fFlags,
                  fConfigTable[i].fFormats.fBaseInternalFormat,
                  fConfigTable[i].fFormats.fSizedInternalFormat,
                  fConfigTable[i].fFormats.fExternalFormat[kOther_ExternalFormatUsage],
                  fConfigTable[i].fFormats.fExternalFormat[kTexImage_ExternalFormatUsage],
                  fConfigTable[i].fFormats.fExternalType,
                  fConfigTable[i].fFormats.fInternalFormatTexImage,
                  fConfigTable[i].fFormats.fInternalFormatRenderbuffer);
    }

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

bool GrGLCaps::getCompressedTexImageFormats(GrPixelConfig surfaceConfig,
                                            GrGLenum* internalFormat) const {
    if (!GrPixelConfigIsCompressed(surfaceConfig)) {
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
    if (GrPixelConfigIsCompressed(config)) {
        return false;
    }
    *internalFormat = fConfigTable[config].fFormats.fInternalFormatRenderbuffer;
    return true;
}

bool GrGLCaps::getExternalFormat(GrPixelConfig surfaceConfig, GrPixelConfig memoryConfig,
                                 ExternalFormatUsage usage, GrGLenum* externalFormat,
                                 GrGLenum* externalType) const {
    SkASSERT(externalFormat && externalType);
    if (GrPixelConfigIsCompressed(memoryConfig) || GrPixelConfigIsCompressed(memoryConfig)) {
        return false;
    }

    bool surfaceIsAlphaOnly = GrPixelConfigIsAlphaOnly(surfaceConfig);
    bool memoryIsAlphaOnly = GrPixelConfigIsAlphaOnly(memoryConfig);

    // We don't currently support moving RGBA data into and out of ALPHA surfaces. It could be
    // made to work in many cases using glPixelStore and what not but is not needed currently.
    if (surfaceIsAlphaOnly && !memoryIsAlphaOnly) {
        return false;
    }

    *externalFormat = fConfigTable[memoryConfig].fFormats.fExternalFormat[usage];
    *externalType = fConfigTable[memoryConfig].fFormats.fExternalType;

    return true;
}

void GrGLCaps::initConfigTable(const GrGLContextInfo& ctxInfo, const GrGLInterface* gli,
                               GrGLSLCaps* glslCaps) {
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
    uint32_t allRenderFlags = ConfigInfo::kRenderable_Flag;
    if (kNone_MSFBOType != fMSFBOType) {
        allRenderFlags |= ConfigInfo::kRenderableWithMSAA_Flag;
    }

    GrGLStandard standard = ctxInfo.standard();
    GrGLVersion version = ctxInfo.version();

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
    fConfigTable[kRGBA_8888_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_BGRA;
    fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fExternalType  = GR_GL_UNSIGNED_BYTE;
    fConfigTable[kBGRA_8888_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    if (kGL_GrGLStandard == standard) {
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGBA;
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA8;
        if (version >= GR_GL_VER(1, 2) || ctxInfo.hasExtension("GL_EXT_bgra")) {
            // Since the internal format is RGBA8, it is also renderable.
            fConfigTable[kBGRA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                            allRenderFlags;
        }
    } else {
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_BGRA;
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_BGRA8;
        if (ctxInfo.hasExtension("GL_APPLE_texture_format_BGRA8888")) {
            // The APPLE extension doesn't make this renderable.
            fConfigTable[kBGRA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
            if (version < GR_GL_VER(3,0) && !ctxInfo.hasExtension("GL_EXT_texture_storage")) {
                // On ES2 the internal format of a BGRA texture is RGBA with the APPLE extension.
                // Though, that seems to not be the case if the texture storage extension is
                // present. The specs don't exactly make that clear.
                fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGBA;
                fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA8;
            }
        } else if (ctxInfo.hasExtension("GL_EXT_texture_format_BGRA8888")) {
            fConfigTable[kBGRA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                            ConfigInfo::kRenderable_Flag;
            if (ctxInfo.hasExtension("GL_CHROMIUM_renderbuffer_format_BGRA8888") &&
                this->usesMSAARenderBuffers()) {
                fConfigTable[kBGRA_8888_GrPixelConfig].fFlags |=
                    ConfigInfo::kRenderableWithMSAA_Flag;
            }
        }
    }
    fConfigTable[kBGRA_8888_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

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
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_SRGB_ALPHA;
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_SRGB8_ALPHA8;
    // GL does not do srgb<->rgb conversions when transferring between cpu and gpu. Thus, the
    // external format is GL_RGBA. See below for note about ES2.0 and glTex[Sub]Image.
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_RGBA;
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    fConfigTable[kSRGBA_8888_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    if (srgbSupport) {
        fConfigTable[kSRGBA_8888_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag |
                                                         allRenderFlags;
    }
    fConfigTable[kSRGBA_8888_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

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
        if (version >= GR_GL_VER(4, 2) || ctxInfo.hasExtension("GL_ES2_compatibility")) {
            fConfigTable[kRGB_565_GrPixelConfig].fFlags |= allRenderFlags;
        }
    } else {
        fConfigTable[kRGB_565_GrPixelConfig].fFlags |= allRenderFlags;
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
    fConfigTable[kRGBA_4444_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    if (this->textureRedSupport()) {
        fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RED;
        fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_R8;
        fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
            GR_GL_RED;
        fConfigTable[kAlpha_8_GrPixelConfig].fSwizzle = GrSwizzle::RRRR();
    } else {
        fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_ALPHA;
        fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_ALPHA8;
        fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
            GR_GL_ALPHA;
        fConfigTable[kAlpha_8_GrPixelConfig].fSwizzle = GrSwizzle::AAAA();
    }
    fConfigTable[kAlpha_8_GrPixelConfig].fFormats.fExternalType = GR_GL_UNSIGNED_BYTE;
    fConfigTable[kAlpha_8_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    fConfigTable[kAlpha_8_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
    if (this->textureRedSupport() || kDesktop_ARB_MSFBOType == this->msFBOType()) {
        // desktop ARB extension/3.0+ supports ALPHA8 as renderable.
        // Core profile removes ALPHA8 support, but we should have chosen R8 in that case.
        fConfigTable[kAlpha_8_GrPixelConfig].fFlags |= allRenderFlags;
    }

    // Check for [half] floating point texture support
    // NOTE: We disallow floating point textures on ES devices if linear filtering modes are not
    // supported. This is for simplicity, but a more granular approach is possible. Coincidentally,
    // [half] floating point textures became part of the standard in ES3.1 / OGL 3.0.
    bool hasFPTextures = false;
    bool hasHalfFPTextures = false;
    // for now we don't support floating point MSAA on ES
    uint32_t fpRenderFlags = (kGL_GrGLStandard == standard) ?
                              allRenderFlags : (uint32_t)ConfigInfo::kRenderable_Flag;

    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(3, 0) || ctxInfo.hasExtension("GL_ARB_texture_float")) {
            hasFPTextures = true;
            hasHalfFPTextures = true;
        }
    } else {
        if (version >= GR_GL_VER(3, 1)) {
            hasFPTextures = true;
            hasHalfFPTextures = true;
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

    fConfigTable[kRGBA_float_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RGBA;
    fConfigTable[kRGBA_float_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_RGBA32F;
    fConfigTable[kRGBA_float_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        GR_GL_RGBA;
    fConfigTable[kRGBA_float_GrPixelConfig].fFormats.fExternalType = GR_GL_FLOAT;
    fConfigTable[kRGBA_float_GrPixelConfig].fFormatType = kFloat_FormatType;
    if (hasFPTextures) {
        fConfigTable[kRGBA_float_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
        // For now we only enable rendering to float on desktop, because on ES we'd have to solve
        // many precision issues and no clients actually want this yet.
        if (kGL_GrGLStandard == standard /* || version >= GR_GL_VER(3,2) ||
            ctxInfo.hasExtension("GL_EXT_color_buffer_float")*/) {
            fConfigTable[kRGBA_float_GrPixelConfig].fFlags |= fpRenderFlags;
        }
    }
    fConfigTable[kRGBA_float_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    if (this->textureRedSupport()) {
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_RED;
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_R16F;
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage]
            = GR_GL_RED;
        fConfigTable[kAlpha_half_GrPixelConfig].fSwizzle = GrSwizzle::RRRR();
    } else {
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_ALPHA;
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_ALPHA16F;
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage]
            = GR_GL_ALPHA;
        fConfigTable[kAlpha_half_GrPixelConfig].fSwizzle = GrSwizzle::AAAA();
    }
    if (kGL_GrGLStandard == ctxInfo.standard() || ctxInfo.version() >= GR_GL_VER(3, 0)) {
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fExternalType = GR_GL_HALF_FLOAT;
    } else {
        fConfigTable[kAlpha_half_GrPixelConfig].fFormats.fExternalType = GR_GL_HALF_FLOAT_OES;
    }
    fConfigTable[kAlpha_half_GrPixelConfig].fFormatType = kFloat_FormatType;
    if (hasHalfFPTextures) {
        fConfigTable[kAlpha_half_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
        // ES requires either 3.2 or the combination of EXT_color_buffer_half_float and support for
        // GL_RED internal format.
        if (kGL_GrGLStandard == standard || version >= GR_GL_VER(3,2) ||
            (this->textureRedSupport() &&
             ctxInfo.hasExtension("GL_EXT_color_buffer_half_float"))) {
            fConfigTable[kAlpha_half_GrPixelConfig].fFlags |= fpRenderFlags;
        }
    }

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
    fConfigTable[kRGBA_half_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    // Compressed texture support

    // glCompressedTexImage2D is available on all OpenGL ES devices. It is available on standard
    // OpenGL after version 1.3. We'll assume at least that level of OpenGL support.

    // TODO: Fix command buffer bindings and remove this.
    fCompressedTexSubImageSupport = SkToBool(gli->fFunctions.fCompressedTexSubImage2D);

    // No sized/unsized internal format distinction for compressed formats, no external format.
    // Below we set the external formats and types to 0.

    fConfigTable[kIndex_8_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_PALETTE8_RGBA8;
    fConfigTable[kIndex_8_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_PALETTE8_RGBA8;
    fConfigTable[kIndex_8_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] = 0;
    fConfigTable[kIndex_8_GrPixelConfig].fFormats.fExternalType = 0;
    fConfigTable[kIndex_8_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
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
                    fConfigTable[kIndex_8_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
                    break;
                }
            }
        }
    }
    fConfigTable[kIndex_8_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    // May change the internal format based on extensions.
    fConfigTable[kLATC_GrPixelConfig].fFormats.fBaseInternalFormat =
        GR_GL_COMPRESSED_LUMINANCE_LATC1;
    fConfigTable[kLATC_GrPixelConfig].fFormats.fSizedInternalFormat =
        GR_GL_COMPRESSED_LUMINANCE_LATC1;
    if (ctxInfo.hasExtension("GL_EXT_texture_compression_latc") ||
        ctxInfo.hasExtension("GL_NV_texture_compression_latc")) {
        fConfigTable[kLATC_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
    } else if ((kGL_GrGLStandard == standard && version >= GR_GL_VER(3, 0)) ||
               ctxInfo.hasExtension("GL_EXT_texture_compression_rgtc") ||
               ctxInfo.hasExtension("GL_ARB_texture_compression_rgtc")) {
        // RGTC is identical and available on OpenGL 3.0+ as well as with extensions
        fConfigTable[kLATC_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
        fConfigTable[kLATC_GrPixelConfig].fFormats.fBaseInternalFormat =
            GR_GL_COMPRESSED_RED_RGTC1;
        fConfigTable[kLATC_GrPixelConfig].fFormats.fSizedInternalFormat =
            GR_GL_COMPRESSED_RED_RGTC1;
    } else if (ctxInfo.hasExtension("GL_AMD_compressed_3DC_texture")) {
        fConfigTable[kLATC_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
        fConfigTable[kLATC_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_COMPRESSED_3DC_X;
        fConfigTable[kLATC_GrPixelConfig].fFormats.fSizedInternalFormat =
            GR_GL_COMPRESSED_3DC_X;

    }
    fConfigTable[kLATC_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] = 0;
    fConfigTable[kLATC_GrPixelConfig].fFormats.fExternalType = 0;
    fConfigTable[kLATC_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    fConfigTable[kLATC_GrPixelConfig].fSwizzle = GrSwizzle::RRRR();

    fConfigTable[kETC1_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_COMPRESSED_ETC1_RGB8;
    fConfigTable[kETC1_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_COMPRESSED_ETC1_RGB8;
    fConfigTable[kETC1_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] = 0;
    fConfigTable[kETC1_GrPixelConfig].fFormats.fExternalType = 0;
    fConfigTable[kETC1_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    if (kGL_GrGLStandard == standard) {
        if (version >= GR_GL_VER(4, 3) || ctxInfo.hasExtension("GL_ARB_ES3_compatibility")) {
            fConfigTable[kETC1_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
        }
    } else {
        if (version >= GR_GL_VER(3, 0) ||
            ctxInfo.hasExtension("GL_OES_compressed_ETC1_RGB8_texture") ||
            // ETC2 is a superset of ETC1, so we can just check for that, too.
            (ctxInfo.hasExtension("GL_OES_compressed_ETC2_RGB8_texture") &&
             ctxInfo.hasExtension("GL_OES_compressed_ETC2_RGBA8_texture"))) {
            fConfigTable[kETC1_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
        }
    }
    fConfigTable[kETC1_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

    fConfigTable[kR11_EAC_GrPixelConfig].fFormats.fBaseInternalFormat = GR_GL_COMPRESSED_R11_EAC;
    fConfigTable[kR11_EAC_GrPixelConfig].fFormats.fSizedInternalFormat = GR_GL_COMPRESSED_R11_EAC;
    fConfigTable[kR11_EAC_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] = 0;
    fConfigTable[kR11_EAC_GrPixelConfig].fFormats.fExternalType = 0;
    fConfigTable[kR11_EAC_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    // Check for R11_EAC. We don't support R11_EAC on desktop, as most cards default to
    // decompressing the textures in the driver, and is generally slower.
    if (kGLES_GrGLStandard == standard && version >= GR_GL_VER(3,0)) {
        fConfigTable[kR11_EAC_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
    }
    fConfigTable[kR11_EAC_GrPixelConfig].fSwizzle = GrSwizzle::RRRR();

    fConfigTable[kASTC_12x12_GrPixelConfig].fFormats.fBaseInternalFormat =
        GR_GL_COMPRESSED_RGBA_ASTC_12x12;
    fConfigTable[kASTC_12x12_GrPixelConfig].fFormats.fSizedInternalFormat =
        GR_GL_COMPRESSED_RGBA_ASTC_12x12;
    fConfigTable[kASTC_12x12_GrPixelConfig].fFormats.fExternalFormat[kOther_ExternalFormatUsage] =
        0;
    fConfigTable[kASTC_12x12_GrPixelConfig].fFormats.fExternalType = 0;
    fConfigTable[kASTC_12x12_GrPixelConfig].fFormatType = kNormalizedFixedPoint_FormatType;
    if (ctxInfo.hasExtension("GL_KHR_texture_compression_astc_hdr") ||
        ctxInfo.hasExtension("GL_KHR_texture_compression_astc_ldr") ||
        ctxInfo.hasExtension("GL_OES_texture_compression_astc")) {
        fConfigTable[kASTC_12x12_GrPixelConfig].fFlags = ConfigInfo::kTextureable_Flag;
    }
    fConfigTable[kASTC_12x12_GrPixelConfig].fSwizzle = GrSwizzle::RGBA();

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
    // OpenGL ES 2.0 + GL_EXT_sRGB allows GL_SRGB_ALPHA to be specified as the <format>
    // param to Tex(Sub)Image. ES 2.0 requires the <internalFormat> and <format> params to match.
    // Thus, on ES 2.0 we will use GL_SRGB_ALPHA as the <format> param.
    // On OpenGL and ES 3.0+ GL_SRGB_ALPHA does not work for the <format> param to glTexImage.
    if (ctxInfo.standard() == kGLES_GrGLStandard && ctxInfo.version() == GR_GL_VER(2,0)) {
        fConfigTable[kSRGBA_8888_GrPixelConfig].fFormats.fExternalFormat[kTexImage_ExternalFormatUsage] =
            GR_GL_SRGB_ALPHA;
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
    if (useSizedTexFormats && this->bgraIsInternalFormat())  {
        fConfigTable[kBGRA_8888_GrPixelConfig].fFormats.fInternalFormatTexImage = GR_GL_BGRA;
    }

    // If we don't have texture swizzle support then the shader generator must insert the
    // swizzle into shader code.
    if (!this->textureSwizzleSupport()) {
        for (int i = 0; i < kGrPixelConfigCnt; ++i) {
            glslCaps->fConfigTextureSwizzle[i] = fConfigTable[i].fSwizzle;
        }
    }

    // Shader output swizzles will default to RGBA. When we've use GL_RED instead of GL_ALPHA to
    // implement kAlpha_8_GrPixelConfig we need to swizzle the shader outputs so the alpha channel
    // gets written to the single component.
    if (this->textureRedSupport()) {
        for (int i = 0; i < kGrPixelConfigCnt; ++i) {
            GrPixelConfig config = static_cast<GrPixelConfig>(i);
            if (GrPixelConfigIsAlphaOnly(config) &&
                fConfigTable[i].fFormats.fBaseInternalFormat == GR_GL_RED) {
                glslCaps->fConfigOutputSwizzle[i] = GrSwizzle::AAAA();
            }
        }
    }

#ifdef SK_DEBUG
    // Make sure we initialized everything.
    ConfigInfo defaultEntry;
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
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

void GrGLCaps::onApplyOptionsOverrides(const GrContextOptions& options) {}
