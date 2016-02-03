/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrGLGpu.h"
#include "GrGLGLSL.h"
#include "GrGLStencilAttachment.h"
#include "GrGLTextureRenderTarget.h"
#include "GrGpuResourcePriv.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrSurfacePriv.h"
#include "GrTexturePriv.h"
#include "GrTypes.h"
#include "GrVertices.h"
#include "builders/GrGLShaderStringBuilder.h"
#include "glsl/GrGLSL.h"
#include "glsl/GrGLSLCaps.h"
#include "SkStrokeRec.h"
#include "SkTemplates.h"

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->glInterface(), RET, X)

#define SKIP_CACHE_CHECK    true

#if GR_GL_CHECK_ALLOC_WITH_GET_ERROR
    #define CLEAR_ERROR_BEFORE_ALLOC(iface)   GrGLClearErr(iface)
    #define GL_ALLOC_CALL(iface, call)        GR_GL_CALL_NOERRCHECK(iface, call)
    #define CHECK_ALLOC_ERROR(iface)          GR_GL_GET_ERROR(iface)
#else
    #define CLEAR_ERROR_BEFORE_ALLOC(iface)
    #define GL_ALLOC_CALL(iface, call)        GR_GL_CALL(iface, call)
    #define CHECK_ALLOC_ERROR(iface)          GR_GL_NO_ERROR
#endif


///////////////////////////////////////////////////////////////////////////////


static const GrGLenum gXfermodeEquation2Blend[] = {
    // Basic OpenGL blend equations.
    GR_GL_FUNC_ADD,
    GR_GL_FUNC_SUBTRACT,
    GR_GL_FUNC_REVERSE_SUBTRACT,

    // GL_KHR_blend_equation_advanced.
    GR_GL_SCREEN,
    GR_GL_OVERLAY,
    GR_GL_DARKEN,
    GR_GL_LIGHTEN,
    GR_GL_COLORDODGE,
    GR_GL_COLORBURN,
    GR_GL_HARDLIGHT,
    GR_GL_SOFTLIGHT,
    GR_GL_DIFFERENCE,
    GR_GL_EXCLUSION,
    GR_GL_MULTIPLY,
    GR_GL_HSL_HUE,
    GR_GL_HSL_SATURATION,
    GR_GL_HSL_COLOR,
    GR_GL_HSL_LUMINOSITY
};
GR_STATIC_ASSERT(0 == kAdd_GrBlendEquation);
GR_STATIC_ASSERT(1 == kSubtract_GrBlendEquation);
GR_STATIC_ASSERT(2 == kReverseSubtract_GrBlendEquation);
GR_STATIC_ASSERT(3 == kScreen_GrBlendEquation);
GR_STATIC_ASSERT(4 == kOverlay_GrBlendEquation);
GR_STATIC_ASSERT(5 == kDarken_GrBlendEquation);
GR_STATIC_ASSERT(6 == kLighten_GrBlendEquation);
GR_STATIC_ASSERT(7 == kColorDodge_GrBlendEquation);
GR_STATIC_ASSERT(8 == kColorBurn_GrBlendEquation);
GR_STATIC_ASSERT(9 == kHardLight_GrBlendEquation);
GR_STATIC_ASSERT(10 == kSoftLight_GrBlendEquation);
GR_STATIC_ASSERT(11 == kDifference_GrBlendEquation);
GR_STATIC_ASSERT(12 == kExclusion_GrBlendEquation);
GR_STATIC_ASSERT(13 == kMultiply_GrBlendEquation);
GR_STATIC_ASSERT(14 == kHSLHue_GrBlendEquation);
GR_STATIC_ASSERT(15 == kHSLSaturation_GrBlendEquation);
GR_STATIC_ASSERT(16 == kHSLColor_GrBlendEquation);
GR_STATIC_ASSERT(17 == kHSLLuminosity_GrBlendEquation);
GR_STATIC_ASSERT(SK_ARRAY_COUNT(gXfermodeEquation2Blend) == kGrBlendEquationCnt);

static const GrGLenum gXfermodeCoeff2Blend[] = {
    GR_GL_ZERO,
    GR_GL_ONE,
    GR_GL_SRC_COLOR,
    GR_GL_ONE_MINUS_SRC_COLOR,
    GR_GL_DST_COLOR,
    GR_GL_ONE_MINUS_DST_COLOR,
    GR_GL_SRC_ALPHA,
    GR_GL_ONE_MINUS_SRC_ALPHA,
    GR_GL_DST_ALPHA,
    GR_GL_ONE_MINUS_DST_ALPHA,
    GR_GL_CONSTANT_COLOR,
    GR_GL_ONE_MINUS_CONSTANT_COLOR,
    GR_GL_CONSTANT_ALPHA,
    GR_GL_ONE_MINUS_CONSTANT_ALPHA,

    // extended blend coeffs
    GR_GL_SRC1_COLOR,
    GR_GL_ONE_MINUS_SRC1_COLOR,
    GR_GL_SRC1_ALPHA,
    GR_GL_ONE_MINUS_SRC1_ALPHA,
};

bool GrGLGpu::BlendCoeffReferencesConstant(GrBlendCoeff coeff) {
    static const bool gCoeffReferencesBlendConst[] = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        true,

        // extended blend coeffs
        false,
        false,
        false,
        false,
    };
    return gCoeffReferencesBlendConst[coeff];
    GR_STATIC_ASSERT(kGrBlendCoeffCnt == SK_ARRAY_COUNT(gCoeffReferencesBlendConst));

    GR_STATIC_ASSERT(0 == kZero_GrBlendCoeff);
    GR_STATIC_ASSERT(1 == kOne_GrBlendCoeff);
    GR_STATIC_ASSERT(2 == kSC_GrBlendCoeff);
    GR_STATIC_ASSERT(3 == kISC_GrBlendCoeff);
    GR_STATIC_ASSERT(4 == kDC_GrBlendCoeff);
    GR_STATIC_ASSERT(5 == kIDC_GrBlendCoeff);
    GR_STATIC_ASSERT(6 == kSA_GrBlendCoeff);
    GR_STATIC_ASSERT(7 == kISA_GrBlendCoeff);
    GR_STATIC_ASSERT(8 == kDA_GrBlendCoeff);
    GR_STATIC_ASSERT(9 == kIDA_GrBlendCoeff);
    GR_STATIC_ASSERT(10 == kConstC_GrBlendCoeff);
    GR_STATIC_ASSERT(11 == kIConstC_GrBlendCoeff);
    GR_STATIC_ASSERT(12 == kConstA_GrBlendCoeff);
    GR_STATIC_ASSERT(13 == kIConstA_GrBlendCoeff);

    GR_STATIC_ASSERT(14 == kS2C_GrBlendCoeff);
    GR_STATIC_ASSERT(15 == kIS2C_GrBlendCoeff);
    GR_STATIC_ASSERT(16 == kS2A_GrBlendCoeff);
    GR_STATIC_ASSERT(17 == kIS2A_GrBlendCoeff);

    // assertion for gXfermodeCoeff2Blend have to be in GrGpu scope
    GR_STATIC_ASSERT(kGrBlendCoeffCnt == SK_ARRAY_COUNT(gXfermodeCoeff2Blend));
}

///////////////////////////////////////////////////////////////////////////////


GrGpu* GrGLGpu::Create(GrBackendContext backendContext, const GrContextOptions& options,
                       GrContext* context) {
    SkAutoTUnref<const GrGLInterface> glInterface(
        reinterpret_cast<const GrGLInterface*>(backendContext));
    if (!glInterface) {
        glInterface.reset(GrGLDefaultInterface());
    } else {
        glInterface->ref();
    }
    if (!glInterface) {
        return nullptr;
    }
    GrGLContext* glContext = GrGLContext::Create(glInterface, options);
    if (glContext) {
        return new GrGLGpu(glContext, context);
    }
    return nullptr;
}

static bool gPrintStartupSpew;

GrGLGpu::GrGLGpu(GrGLContext* ctx, GrContext* context)
    : GrGpu(context)
    , fGLContext(ctx) {
    SkASSERT(ctx);
    fCaps.reset(SkRef(ctx->caps()));

    fHWBoundTextureUniqueIDs.reset(this->glCaps().maxFragmentTextureUnits());

    GrGLClearErr(this->glInterface());
    if (gPrintStartupSpew) {
        const GrGLubyte* vendor;
        const GrGLubyte* renderer;
        const GrGLubyte* version;
        GL_CALL_RET(vendor, GetString(GR_GL_VENDOR));
        GL_CALL_RET(renderer, GetString(GR_GL_RENDERER));
        GL_CALL_RET(version, GetString(GR_GL_VERSION));
        SkDebugf("------------------------- create GrGLGpu %p --------------\n",
                 this);
        SkDebugf("------ VENDOR %s\n", vendor);
        SkDebugf("------ RENDERER %s\n", renderer);
        SkDebugf("------ VERSION %s\n",  version);
        SkDebugf("------ EXTENSIONS\n");
        this->glContext().extensions().print();
        SkDebugf("\n");
        SkDebugf("%s", this->glCaps().dump().c_str());
    }

    fProgramCache = new ProgramCache(this);

    SkASSERT(this->glCaps().maxVertexAttributes() >= GrGeometryProcessor::kMaxVertexAttribs);

    fHWProgramID = 0;
    fTempSrcFBOID = 0;
    fTempDstFBOID = 0;
    fStencilClearFBOID = 0;

    if (this->glCaps().shaderCaps()->pathRenderingSupport()) {
        fPathRendering.reset(new GrGLPathRendering(this));
    }
    this->createCopyPrograms();
    fWireRectProgram.fProgram = 0;
    fWireRectArrayBuffer = 0;
}

GrGLGpu::~GrGLGpu() {
    // Delete the path rendering explicitly, since it will need working gpu object to release the
    // resources the object itself holds.
    fPathRendering.reset();

    if (0 != fHWProgramID) {
        // detach the current program so there is no confusion on OpenGL's part
        // that we want it to be deleted
        GL_CALL(UseProgram(0));
    }

    if (0 != fTempSrcFBOID) {
        GL_CALL(DeleteFramebuffers(1, &fTempSrcFBOID));
    }
    if (0 != fTempDstFBOID) {
        GL_CALL(DeleteFramebuffers(1, &fTempDstFBOID));
    }
    if (0 != fStencilClearFBOID) {
        GL_CALL(DeleteFramebuffers(1, &fStencilClearFBOID));
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(fCopyPrograms); ++i) {
        if (0 != fCopyPrograms[i].fProgram) {
            GL_CALL(DeleteProgram(fCopyPrograms[i].fProgram));
        }
    }

    if (0 != fCopyProgramArrayBuffer) {
        GL_CALL(DeleteBuffers(1, &fCopyProgramArrayBuffer));
    }

    if (0 != fWireRectProgram.fProgram) {
        GL_CALL(DeleteProgram(fWireRectProgram.fProgram));
    }

    if (0 != fWireRectArrayBuffer) {
        GL_CALL(DeleteBuffers(1, &fWireRectArrayBuffer));
    }

    delete fProgramCache;
}

void GrGLGpu::contextAbandoned() {
    INHERITED::contextAbandoned();
    fProgramCache->abandon();
    fHWProgramID = 0;
    fTempSrcFBOID = 0;
    fTempDstFBOID = 0;
    fStencilClearFBOID = 0;
    fCopyProgramArrayBuffer = 0;
    for (size_t i = 0; i < SK_ARRAY_COUNT(fCopyPrograms); ++i) {
        fCopyPrograms[i].fProgram = 0;
    }
    fWireRectProgram.fProgram = 0;
    fWireRectArrayBuffer = 0;
    if (this->glCaps().shaderCaps()->pathRenderingSupport()) {
        this->glPathRendering()->abandonGpuResources();
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrGLGpu::onResetContext(uint32_t resetBits) {
    // we don't use the zb at all
    if (resetBits & kMisc_GrGLBackendState) {
        GL_CALL(Disable(GR_GL_DEPTH_TEST));
        GL_CALL(DepthMask(GR_GL_FALSE));

        fHWDrawFace = GrPipelineBuilder::kInvalid_DrawFace;

        if (kGL_GrGLStandard == this->glStandard()) {
            // Desktop-only state that we never change
            if (!this->glCaps().isCoreProfile()) {
                GL_CALL(Disable(GR_GL_POINT_SMOOTH));
                GL_CALL(Disable(GR_GL_LINE_SMOOTH));
                GL_CALL(Disable(GR_GL_POLYGON_SMOOTH));
                GL_CALL(Disable(GR_GL_POLYGON_STIPPLE));
                GL_CALL(Disable(GR_GL_COLOR_LOGIC_OP));
                GL_CALL(Disable(GR_GL_INDEX_LOGIC_OP));
            }
            // The windows NVIDIA driver has GL_ARB_imaging in the extension string when using a
            // core profile. This seems like a bug since the core spec removes any mention of
            // GL_ARB_imaging.
            if (this->glCaps().imagingSupport() && !this->glCaps().isCoreProfile()) {
                GL_CALL(Disable(GR_GL_COLOR_TABLE));
            }
            GL_CALL(Disable(GR_GL_POLYGON_OFFSET_FILL));
            // Since ES doesn't support glPointSize at all we always use the VS to
            // set the point size
            GL_CALL(Enable(GR_GL_VERTEX_PROGRAM_POINT_SIZE));

            // We should set glPolygonMode(FRONT_AND_BACK,FILL) here, too. It isn't
            // currently part of our gl interface. There are probably others as
            // well.
        }

        if (kGLES_GrGLStandard == this->glStandard() &&
                this->hasExtension("GL_ARM_shader_framebuffer_fetch")) {
            // The arm extension requires specifically enabling MSAA fetching per sample.
            // On some devices this may have a perf hit.  Also multiple render targets are disabled
            GL_CALL(Enable(GR_GL_FETCH_PER_SAMPLE_ARM));
        }
        fHWWriteToColor = kUnknown_TriState;
        // we only ever use lines in hairline mode
        GL_CALL(LineWidth(1));
        GL_CALL(Disable(GR_GL_DITHER));
    }

    if (resetBits & kMSAAEnable_GrGLBackendState) {
        fMSAAEnabled = kUnknown_TriState;

        // In mixed samples mode coverage modulation allows the coverage to be converted to
        // "opacity", which can then be blended into the color buffer to accomplish antialiasing.
        // Enable coverage modulation suitable for premultiplied alpha colors.
        // This state has no effect when not rendering to a mixed sampled target.
        if (this->caps()->mixedSamplesSupport()) {
            GL_CALL(CoverageModulation(GR_GL_RGBA));
        }
    }

    fHWActiveTextureUnitIdx = -1; // invalid

    if (resetBits & kTextureBinding_GrGLBackendState) {
        for (int s = 0; s < fHWBoundTextureUniqueIDs.count(); ++s) {
            fHWBoundTextureUniqueIDs[s] = SK_InvalidUniqueID;
        }
    }

    if (resetBits & kBlend_GrGLBackendState) {
        fHWBlendState.invalidate();
    }

    if (resetBits & kView_GrGLBackendState) {
        fHWScissorSettings.invalidate();
        fHWViewport.invalidate();
    }

    if (resetBits & kStencil_GrGLBackendState) {
        fHWStencilSettings.invalidate();
        fHWStencilTestEnabled = kUnknown_TriState;
    }

    // Vertex
    if (resetBits & kVertex_GrGLBackendState) {
        fHWGeometryState.invalidate();
    }

    if (resetBits & kRenderTarget_GrGLBackendState) {
        fHWBoundRenderTargetUniqueID = SK_InvalidUniqueID;
        fHWSRGBFramebuffer = kUnknown_TriState;
    }

    if (resetBits & kPathRendering_GrGLBackendState) {
        if (this->caps()->shaderCaps()->pathRenderingSupport()) {
            this->glPathRendering()->resetContext();
        }
    }

    // we assume these values
    if (resetBits & kPixelStore_GrGLBackendState) {
        if (this->glCaps().unpackRowLengthSupport()) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
        }
        if (this->glCaps().packRowLengthSupport()) {
            GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
        }
        if (this->glCaps().unpackFlipYSupport()) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_FALSE));
        }
        if (this->glCaps().packFlipYSupport()) {
            GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, GR_GL_FALSE));
        }
    }

    if (resetBits & kProgram_GrGLBackendState) {
        fHWProgramID = 0;
    }
}

static GrSurfaceOrigin resolve_origin(GrSurfaceOrigin origin, bool renderTarget) {
    // By default, GrRenderTargets are GL's normal orientation so that they
    // can be drawn to by the outside world without the client having
    // to render upside down.
    if (kDefault_GrSurfaceOrigin == origin) {
        return renderTarget ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;
    } else {
        return origin;
    }
}

GrTexture* GrGLGpu::onWrapBackendTexture(const GrBackendTextureDesc& desc,
                                         GrWrapOwnership ownership) {
#ifdef SK_IGNORE_GL_TEXTURE_TARGET
    if (!desc.fTextureHandle) {
        return nullptr;
    }
#else
    const GrGLTextureInfo* info = reinterpret_cast<const GrGLTextureInfo*>(desc.fTextureHandle);
    if (!info || !info->fID) {
        return nullptr;
    }
#endif

    int maxSize = this->caps()->maxTextureSize();
    if (desc.fWidth > maxSize || desc.fHeight > maxSize) {
        return nullptr;
    }

    // next line relies on GrBackendTextureDesc's flags matching GrTexture's
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrBackendTextureFlag);

    GrGLTexture::IDDesc idDesc;
    GrSurfaceDesc surfDesc;

#ifdef SK_IGNORE_GL_TEXTURE_TARGET
    idDesc.fInfo.fID = static_cast<GrGLuint>(desc.fTextureHandle);
    // We only support GL_TEXTURE_2D at the moment.
    idDesc.fInfo.fTarget = GR_GL_TEXTURE_2D;
#else
    idDesc.fInfo = *info;
#endif

    if (GR_GL_TEXTURE_EXTERNAL == idDesc.fInfo.fTarget) {
        if (renderTarget) {
            // This combination is not supported.
            return nullptr;
        }
        if (!this->glCaps().externalTextureSupport()) {
            return nullptr;
        }
    } else  if (GR_GL_TEXTURE_RECTANGLE == idDesc.fInfo.fTarget) {
        if (!this->glCaps().rectangleTextureSupport()) {
            return nullptr;
        }
    } else if (GR_GL_TEXTURE_2D != idDesc.fInfo.fTarget) {
        return nullptr;
    }

    // Sample count is interpreted to mean the number of samples that Gr code should allocate
    // for a render buffer that resolves to the texture. We don't support MSAA textures.
    if (desc.fSampleCnt && !renderTarget) {
        return nullptr;
    }

    switch (ownership) {
        case kAdopt_GrWrapOwnership:
            idDesc.fLifeCycle = GrGpuResource::kAdopted_LifeCycle;
            break;
        case kBorrow_GrWrapOwnership:
            idDesc.fLifeCycle = GrGpuResource::kBorrowed_LifeCycle;
            break;
    }

    surfDesc.fFlags = (GrSurfaceFlags) desc.fFlags;
    surfDesc.fWidth = desc.fWidth;
    surfDesc.fHeight = desc.fHeight;
    surfDesc.fConfig = desc.fConfig;
    surfDesc.fSampleCnt = SkTMin(desc.fSampleCnt, this->caps()->maxSampleCount());
    // FIXME:  this should be calling resolve_origin(), but Chrome code is currently
    // assuming the old behaviour, which is that backend textures are always
    // BottomLeft, even for non-RT's.  Once Chrome is fixed, change this to:
    // glTexDesc.fOrigin = resolve_origin(desc.fOrigin, renderTarget);
    if (kDefault_GrSurfaceOrigin == desc.fOrigin) {
        surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    } else {
        surfDesc.fOrigin = desc.fOrigin;
    }

    GrGLTexture* texture = nullptr;
    if (renderTarget) {
        GrGLRenderTarget::IDDesc rtIDDesc;
        if (!this->createRenderTargetObjects(surfDesc, GrGpuResource::kUncached_LifeCycle,
                                             idDesc.fInfo, &rtIDDesc)) {
            return nullptr;
        }
        texture = new GrGLTextureRenderTarget(this, surfDesc, idDesc, rtIDDesc);
    } else {
        texture = new GrGLTexture(this, surfDesc, idDesc);
    }
    if (nullptr == texture) {
        return nullptr;
    }

    return texture;
}

GrRenderTarget* GrGLGpu::onWrapBackendRenderTarget(const GrBackendRenderTargetDesc& wrapDesc,
                                                   GrWrapOwnership ownership) {
    GrGLRenderTarget::IDDesc idDesc;
    idDesc.fRTFBOID = static_cast<GrGLuint>(wrapDesc.fRenderTargetHandle);
    idDesc.fMSColorRenderbufferID = 0;
    idDesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    switch (ownership) {
        case kAdopt_GrWrapOwnership:
            idDesc.fLifeCycle = GrGpuResource::kAdopted_LifeCycle;
            break;
        case kBorrow_GrWrapOwnership:
            idDesc.fLifeCycle = GrGpuResource::kBorrowed_LifeCycle;
            break;
    }    
    idDesc.fSampleConfig = GrRenderTarget::kUnified_SampleConfig;

    GrSurfaceDesc desc;
    desc.fConfig = wrapDesc.fConfig;
    desc.fFlags = kCheckAllocation_GrSurfaceFlag | kRenderTarget_GrSurfaceFlag;
    desc.fWidth = wrapDesc.fWidth;
    desc.fHeight = wrapDesc.fHeight;
    desc.fSampleCnt = SkTMin(wrapDesc.fSampleCnt, this->caps()->maxSampleCount());
    desc.fOrigin = resolve_origin(wrapDesc.fOrigin, true);

    return GrGLRenderTarget::CreateWrapped(this, desc, idDesc, wrapDesc.fStencilBits);
}

////////////////////////////////////////////////////////////////////////////////
bool GrGLGpu::onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                                   size_t rowBytes, GrPixelConfig srcConfig,
                                   DrawPreference* drawPreference,
                                   WritePixelTempDrawInfo* tempDrawInfo) {
    if (kIndex_8_GrPixelConfig == srcConfig || GrPixelConfigIsCompressed(dstSurface->config())) {
        return false;
    }

    // This subclass only allows writes to textures. If the dst is not a texture we have to draw
    // into it. We could use glDrawPixels on GLs that have it, but we don't today.
    if (!dstSurface->asTexture()) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    } else {
        GrGLTexture* texture = static_cast<GrGLTexture*>(dstSurface->asTexture());
        if (GR_GL_TEXTURE_EXTERNAL == texture->target()) {
             // We don't currently support writing pixels to EXTERNAL textures.
             return false;
        }
    }

    if (GrPixelConfigIsSRGB(dstSurface->config()) != GrPixelConfigIsSRGB(srcConfig)) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    tempDrawInfo->fSwapRAndB = false;

    // These settings we will always want if a temp draw is performed. Initially set the config
    // to srcConfig, though that may be modified if we decide to do a R/G swap.
    tempDrawInfo->fTempSurfaceDesc.fFlags = kNone_GrSurfaceFlags;
    tempDrawInfo->fTempSurfaceDesc.fConfig = srcConfig;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 0;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // no CPU y-flip for TL.

    bool configsAreRBSwaps = GrPixelConfigSwapRAndB(srcConfig) == dstSurface->config();

    if (configsAreRBSwaps) {
        if (!this->caps()->isConfigTexturable(srcConfig)) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
            tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
            tempDrawInfo->fSwapRAndB = true;
        } else if (this->glCaps().rgba8888PixelsOpsAreSlow() &&
                   kRGBA_8888_GrPixelConfig == srcConfig) {
            ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
            tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
            tempDrawInfo->fSwapRAndB = true;
        } else if (kGLES_GrGLStandard == this->glStandard() &&
                   this->glCaps().bgraIsInternalFormat()) {
            // The internal format and external formats must match texture uploads so we can't
            // swizzle while uploading when BGRA is a distinct internal format.
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
            tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
            tempDrawInfo->fSwapRAndB = true;
        }
    }

    if (!this->glCaps().unpackFlipYSupport() &&
        kBottomLeft_GrSurfaceOrigin == dstSurface->origin()) {
        ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
    }

    return true;
}

static bool check_write_and_transfer_input(GrGLTexture* glTex, GrSurface* surface,
                                            GrPixelConfig config) {
    if (!glTex) {
        return false;
    }

    // OpenGL doesn't do sRGB <-> linear conversions when reading and writing pixels.
    if (GrPixelConfigIsSRGB(surface->config()) != GrPixelConfigIsSRGB(config)) {
        return false;
    }

    // Write or transfer of pixels is not implemented for TEXTURE_EXTERNAL textures
    if (GR_GL_TEXTURE_EXTERNAL == glTex->target()) {
        return false;
    }

    return true;
}

bool GrGLGpu::onWritePixels(GrSurface* surface,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes) {
    GrGLTexture* glTex = static_cast<GrGLTexture*>(surface->asTexture());

    if (!check_write_and_transfer_input(glTex, surface, config)) {
        return false;
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(glTex->target(), glTex->textureID()));

    bool success = false;
    if (GrPixelConfigIsCompressed(glTex->desc().fConfig)) {
        // We check that config == desc.fConfig in GrGLGpu::canWriteTexturePixels()
        SkASSERT(config == glTex->desc().fConfig);
        success = this->uploadCompressedTexData(glTex->desc(), glTex->target(), buffer, 
                                                kWrite_UploadType, left, top, width, height);
    } else {
        success = this->uploadTexData(glTex->desc(), glTex->target(), kWrite_UploadType,
                                      left, top, width, height, config, buffer, rowBytes);
    }

    if (success) {
        glTex->texturePriv().dirtyMipMaps(true);
        return true;
    }

    return false;
}

bool GrGLGpu::onTransferPixels(GrSurface* surface,
                               int left, int top, int width, int height,
                               GrPixelConfig config, GrTransferBuffer* buffer,
                               size_t offset, size_t rowBytes) {
    GrGLTexture* glTex = static_cast<GrGLTexture*>(surface->asTexture());

    if (!check_write_and_transfer_input(glTex, surface, config)) {
        return false;
    }

    // For the moment, can't transfer compressed data
    if (GrPixelConfigIsCompressed(glTex->desc().fConfig)) {
        return false;
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(glTex->target(), glTex->textureID()));

    SkASSERT(!buffer->isMapped());
    GrGLTransferBuffer* glBuffer = reinterpret_cast<GrGLTransferBuffer*>(buffer);
    // bind the transfer buffer
    SkASSERT(GR_GL_PIXEL_UNPACK_BUFFER == glBuffer->bufferType() ||
             GR_GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM == glBuffer->bufferType());
    GL_CALL(BindBuffer(glBuffer->bufferType(), glBuffer->bufferID()));

    bool success = false;
    success = this->uploadTexData(glTex->desc(), glTex->target(), kTransfer_UploadType,
                                  left, top, width, height, config, buffer, rowBytes);

    if (success) {
        glTex->texturePriv().dirtyMipMaps(true);
        return true;
    }

    return false;
}

// For GL_[UN]PACK_ALIGNMENT.
static inline GrGLint config_alignment(GrPixelConfig config) {
    SkASSERT(!GrPixelConfigIsCompressed(config));
    switch (config) {
        case kAlpha_8_GrPixelConfig:
            return 1;
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return 2;
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
            return 4;
        default:
            return 0;
    }
}

static inline GrGLenum check_alloc_error(const GrSurfaceDesc& desc,
                                         const GrGLInterface* interface) {
    if (SkToBool(desc.fFlags & kCheckAllocation_GrSurfaceFlag)) {
        return GR_GL_GET_ERROR(interface);
    } else {
        return CHECK_ALLOC_ERROR(interface);
    }
}

bool GrGLGpu::uploadTexData(const GrSurfaceDesc& desc,
                            GrGLenum target,
                            UploadType uploadType,
                            int left, int top, int width, int height,
                            GrPixelConfig dataConfig,
                            const void* dataOrOffset,
                            size_t rowBytes) {
    SkASSERT(dataOrOffset || kNewTexture_UploadType == uploadType || 
             kTransfer_UploadType == uploadType);

    // If we're uploading compressed data then we should be using uploadCompressedTexData
    SkASSERT(!GrPixelConfigIsCompressed(dataConfig));

    SkASSERT(this->caps()->isConfigTexturable(desc.fConfig));

    size_t bpp = GrBytesPerPixel(dataConfig);
    if (!GrSurfacePriv::AdjustWritePixelParams(desc.fWidth, desc.fHeight, bpp, &left, &top,
                                               &width, &height, &dataOrOffset, &rowBytes)) {
        return false;
    }
    size_t trimRowBytes = width * bpp;

    // in case we need a temporary, trimmed copy of the src pixels
#if defined(GOOGLE3)
    // Stack frame size is limited in GOOGLE3.
    SkAutoSMalloc<64 * 128> tempStorage;
#else
    SkAutoSMalloc<128 * 128> tempStorage;
#endif

    // Internal format comes from the texture desc.
    GrGLenum internalFormat;
    // External format and type come from the upload data.
    GrGLenum externalFormat;
    GrGLenum externalType;
    if (!this->glCaps().getTexImageFormats(desc.fConfig, dataConfig, &internalFormat,
                                           &externalFormat, &externalType)) {
        return false;
    }
    /*
     *  Check whether to allocate a temporary buffer for flipping y or
     *  because our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL ES may not let us specify
     *  GL_UNPACK_ROW_LENGTH.
     */
    bool restoreGLRowLength = false;
    bool swFlipY = false;
    bool glFlipY = false;
    if (dataOrOffset) {
        if (kBottomLeft_GrSurfaceOrigin == desc.fOrigin) {
            if (this->glCaps().unpackFlipYSupport()) {
                glFlipY = true;
            } else {
                swFlipY = true;
            }
        }
        if (this->glCaps().unpackRowLengthSupport() && !swFlipY) {
            // can't use this for flipping, only non-neg values allowed. :(
            if (rowBytes != trimRowBytes) {
                GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
                GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
                restoreGLRowLength = true;
            }
        } else if (kTransfer_UploadType != uploadType) {
            if (trimRowBytes != rowBytes || swFlipY) {
                // copy data into our new storage, skipping the trailing bytes
                size_t trimSize = height * trimRowBytes;
                const char* src = (const char*)dataOrOffset;
                if (swFlipY) {
                    src += (height - 1) * rowBytes;
                }
                char* dst = (char*)tempStorage.reset(trimSize);
                for (int y = 0; y < height; y++) {
                    memcpy(dst, src, trimRowBytes);
                    if (swFlipY) {
                        src -= rowBytes;
                    } else {
                        src += rowBytes;
                    }
                    dst += trimRowBytes;
                }
                // now point data to our copied version
                dataOrOffset = tempStorage.get();
            }
        } else {
            return false;
        }
        if (glFlipY) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_TRUE));
        }
        GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, config_alignment(dataConfig)));
    }
    bool succeeded = true;
    if (kNewTexture_UploadType == uploadType) {
        if (dataOrOffset && 
            !(0 == left && 0 == top && desc.fWidth == width && desc.fHeight == height)) {
            succeeded = false;
        } else {
            CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
            GL_ALLOC_CALL(this->glInterface(), TexImage2D(target, 0, internalFormat, desc.fWidth,
                                                          desc.fHeight, 0, externalFormat,
                                                          externalType, dataOrOffset));
            GrGLenum error = check_alloc_error(desc, this->glInterface());
            if (error != GR_GL_NO_ERROR) {
                succeeded = false;
            }
        }
    } else {
        if (swFlipY || glFlipY) {
            top = desc.fHeight - (top + height);
        }
        GL_CALL(TexSubImage2D(target,
                              0, // level
                              left, top,
                              width, height,
                              externalFormat, externalType, dataOrOffset));
    }

    if (restoreGLRowLength) {
        SkASSERT(this->glCaps().unpackRowLengthSupport());
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
    if (glFlipY) {
        GL_CALL(PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_FALSE));
    }
    return succeeded;
}

// TODO: This function is using a lot of wonky semantics like, if width == -1
// then set width = desc.fWdith ... blah. A better way to do it might be to
// create a CompressedTexData struct that takes a desc/ptr and figures out
// the proper upload semantics. Then users can construct this function how they
// see fit if they want to go against the "standard" way to do it.
bool GrGLGpu::uploadCompressedTexData(const GrSurfaceDesc& desc,
                                      GrGLenum target,
                                      const void* data,
                                      UploadType uploadType,
                                      int left, int top, int width, int height) {
    SkASSERT(this->caps()->isConfigTexturable(desc.fConfig));
    SkASSERT(kTransfer_UploadType != uploadType && 
             (data || kNewTexture_UploadType != uploadType));

    // No support for software flip y, yet...
    SkASSERT(kBottomLeft_GrSurfaceOrigin != desc.fOrigin);

    if (-1 == width) {
        width = desc.fWidth;
    }
#ifdef SK_DEBUG
    else {
        SkASSERT(width <= desc.fWidth);
    }
#endif

    if (-1 == height) {
        height = desc.fHeight;
    }
#ifdef SK_DEBUG
    else {
        SkASSERT(height <= desc.fHeight);
    }
#endif

    // Make sure that the width and height that we pass to OpenGL
    // is a multiple of the block size.
    size_t dataSize = GrCompressedFormatDataSize(desc.fConfig, width, height);

    // We only need the internal format for compressed 2D textures.
    GrGLenum internalFormat;
    if (!this->glCaps().getCompressedTexImageFormats(desc.fConfig, &internalFormat)) {
        return false;
    }

    if (kNewTexture_UploadType == uploadType) {
        CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
        GL_ALLOC_CALL(this->glInterface(),
                      CompressedTexImage2D(target,
                                           0, // level
                                           internalFormat,
                                           width, height,
                                           0, // border
                                           SkToInt(dataSize),
                                           data));
        GrGLenum error = check_alloc_error(desc, this->glInterface());
        if (error != GR_GL_NO_ERROR) {
            return false;
        }
    } else {
        // Paletted textures can't be updated.
        if (GR_GL_PALETTE8_RGBA8 == internalFormat) {
            return false;
        }
        GL_CALL(CompressedTexSubImage2D(target,
                                        0, // level
                                        left, top,
                                        width, height,
                                        internalFormat,
                                        SkToInt(dataSize),
                                        data));
    }

    return true;
}

static bool renderbuffer_storage_msaa(const GrGLContext& ctx,
                                      int sampleCount,
                                      GrGLenum format,
                                      int width, int height) {
    CLEAR_ERROR_BEFORE_ALLOC(ctx.interface());
    SkASSERT(GrGLCaps::kNone_MSFBOType != ctx.caps()->msFBOType());
    switch (ctx.caps()->msFBOType()) {
        case GrGLCaps::kDesktop_ARB_MSFBOType:
        case GrGLCaps::kDesktop_EXT_MSFBOType:
        case GrGLCaps::kMixedSamples_MSFBOType:
        case GrGLCaps::kES_3_0_MSFBOType:
            GL_ALLOC_CALL(ctx.interface(),
                            RenderbufferStorageMultisample(GR_GL_RENDERBUFFER,
                                                            sampleCount,
                                                            format,
                                                            width, height));
            break;
        case GrGLCaps::kES_Apple_MSFBOType:
            GL_ALLOC_CALL(ctx.interface(),
                            RenderbufferStorageMultisampleES2APPLE(GR_GL_RENDERBUFFER,
                                                                    sampleCount,
                                                                    format,
                                                                    width, height));
            break;
        case GrGLCaps::kES_EXT_MsToTexture_MSFBOType:
        case GrGLCaps::kES_IMG_MsToTexture_MSFBOType:
            GL_ALLOC_CALL(ctx.interface(),
                            RenderbufferStorageMultisampleES2EXT(GR_GL_RENDERBUFFER,
                                                                sampleCount,
                                                                format,
                                                                width, height));
            break;
        case GrGLCaps::kNone_MSFBOType:
            SkFAIL("Shouldn't be here if we don't support multisampled renderbuffers.");
            break;
    }
    return (GR_GL_NO_ERROR == CHECK_ALLOC_ERROR(ctx.interface()));
}

bool GrGLGpu::createRenderTargetObjects(const GrSurfaceDesc& desc,
                                        GrGpuResource::LifeCycle lifeCycle,
                                        const GrGLTextureInfo& texInfo,
                                        GrGLRenderTarget::IDDesc* idDesc) {
    idDesc->fMSColorRenderbufferID = 0;
    idDesc->fRTFBOID = 0;
    idDesc->fTexFBOID = 0;
    idDesc->fLifeCycle = lifeCycle;
    idDesc->fSampleConfig = (GrGLCaps::kMixedSamples_MSFBOType == this->glCaps().msFBOType() &&
                            desc.fSampleCnt > 0) ? GrRenderTarget::kStencil_SampleConfig :
                                                   GrRenderTarget::kUnified_SampleConfig;

    GrGLenum status;

    GrGLenum colorRenderbufferFormat = 0; // suppress warning

    if (desc.fSampleCnt > 0 && GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType()) {
        goto FAILED;
    }

    GL_CALL(GenFramebuffers(1, &idDesc->fTexFBOID));
    if (!idDesc->fTexFBOID) {
        goto FAILED;
    }

    // If we are using multisampling we will create two FBOS. We render to one and then resolve to
    // the texture bound to the other. The exception is the IMG multisample extension. With this
    // extension the texture is multisampled when rendered to and then auto-resolves it when it is
    // rendered from.
    if (desc.fSampleCnt > 0 && this->glCaps().usesMSAARenderBuffers()) {
        GL_CALL(GenFramebuffers(1, &idDesc->fRTFBOID));
        GL_CALL(GenRenderbuffers(1, &idDesc->fMSColorRenderbufferID));
        if (!idDesc->fRTFBOID ||
            !idDesc->fMSColorRenderbufferID) {
            goto FAILED;
        }
        if (!this->glCaps().getRenderbufferFormat(desc.fConfig, &colorRenderbufferFormat)) {
            return false;
        }
    } else {
        idDesc->fRTFBOID = idDesc->fTexFBOID;
    }

    // below here we may bind the FBO
    fHWBoundRenderTargetUniqueID = SK_InvalidUniqueID;
    if (idDesc->fRTFBOID != idDesc->fTexFBOID) {
        SkASSERT(desc.fSampleCnt > 0);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, idDesc->fMSColorRenderbufferID));
        if (!renderbuffer_storage_msaa(*fGLContext,
                                       desc.fSampleCnt,
                                       colorRenderbufferFormat,
                                       desc.fWidth, desc.fHeight)) {
            goto FAILED;
        }
        fStats.incRenderTargetBinds();
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, idDesc->fRTFBOID));
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                        GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_RENDERBUFFER,
                                        idDesc->fMSColorRenderbufferID));
        if ((desc.fFlags & kCheckAllocation_GrSurfaceFlag) ||
            !this->glCaps().isConfigVerifiedColorAttachment(desc.fConfig)) {
            GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                goto FAILED;
            }
            fGLContext->caps()->markConfigAsValidColorAttachment(desc.fConfig);
        }
    }
    fStats.incRenderTargetBinds();
    GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, idDesc->fTexFBOID));

    if (this->glCaps().usesImplicitMSAAResolve() && desc.fSampleCnt > 0) {
        GL_CALL(FramebufferTexture2DMultisample(GR_GL_FRAMEBUFFER,
                                                GR_GL_COLOR_ATTACHMENT0,
                                                texInfo.fTarget,
                                                texInfo.fID, 0, desc.fSampleCnt));
    } else {
        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                     GR_GL_COLOR_ATTACHMENT0,
                                     texInfo.fTarget,
                                     texInfo.fID, 0));
    }
    if ((desc.fFlags & kCheckAllocation_GrSurfaceFlag) ||
        !this->glCaps().isConfigVerifiedColorAttachment(desc.fConfig)) {
        GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            goto FAILED;
        }
        fGLContext->caps()->markConfigAsValidColorAttachment(desc.fConfig);
    }

    return true;

FAILED:
    if (idDesc->fMSColorRenderbufferID) {
        GL_CALL(DeleteRenderbuffers(1, &idDesc->fMSColorRenderbufferID));
    }
    if (idDesc->fRTFBOID != idDesc->fTexFBOID) {
        GL_CALL(DeleteFramebuffers(1, &idDesc->fRTFBOID));
    }
    if (idDesc->fTexFBOID) {
        GL_CALL(DeleteFramebuffers(1, &idDesc->fTexFBOID));
    }
    return false;
}

// good to set a break-point here to know when createTexture fails
static GrTexture* return_null_texture() {
//    SkDEBUGFAIL("null texture");
    return nullptr;
}

#if 0 && defined(SK_DEBUG)
static size_t as_size_t(int x) {
    return x;
}
#endif

GrTexture* GrGLGpu::onCreateTexture(const GrSurfaceDesc& desc,
                                    GrGpuResource::LifeCycle lifeCycle,
                                    const void* srcData, size_t rowBytes) {
    // We fail if the MSAA was requested and is not available.
    if (GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType() && desc.fSampleCnt) {
        //SkDebugf("MSAA RT requested but not supported on this platform.");
        return return_null_texture();
    }

    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    GrGLTexture::IDDesc idDesc;
    idDesc.fInfo.fID = 0;
    GL_CALL(GenTextures(1, &idDesc.fInfo.fID));
    idDesc.fLifeCycle = lifeCycle;
    // We only support GL_TEXTURE_2D at the moment.
    idDesc.fInfo.fTarget = GR_GL_TEXTURE_2D;

    if (!idDesc.fInfo.fID) {
        return return_null_texture();
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(idDesc.fInfo.fTarget, idDesc.fInfo.fID));

    if (renderTarget && this->glCaps().textureUsageSupport()) {
        // provides a hint about how this texture will be used
        GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                              GR_GL_TEXTURE_USAGE,
                              GR_GL_FRAMEBUFFER_ATTACHMENT));
    }

    // Some drivers like to know filter/wrap before seeing glTexImage2D. Some
    // drivers have a bug where an FBO won't be complete if it includes a
    // texture that is not mipmap complete (considering the filter in use).
    GrGLTexture::TexParams initialTexParams;
    // we only set a subset here so invalidate first
    initialTexParams.invalidate();
    initialTexParams.fMinFilter = GR_GL_NEAREST;
    initialTexParams.fMagFilter = GR_GL_NEAREST;
    initialTexParams.fWrapS = GR_GL_CLAMP_TO_EDGE;
    initialTexParams.fWrapT = GR_GL_CLAMP_TO_EDGE;
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_MAG_FILTER,
                          initialTexParams.fMagFilter));
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_MIN_FILTER,
                          initialTexParams.fMinFilter));
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_WRAP_S,
                          initialTexParams.fWrapS));
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_WRAP_T,
                          initialTexParams.fWrapT));
    if (!this->uploadTexData(desc, idDesc.fInfo.fTarget, kNewTexture_UploadType, 0, 0,
                             desc.fWidth, desc.fHeight,
                             desc.fConfig, srcData, rowBytes)) {
        GL_CALL(DeleteTextures(1, &idDesc.fInfo.fID));
        return return_null_texture();
    }

    GrGLTexture* tex;
    if (renderTarget) {
        // unbind the texture from the texture unit before binding it to the frame buffer
        GL_CALL(BindTexture(idDesc.fInfo.fTarget, 0));
        GrGLRenderTarget::IDDesc rtIDDesc;

        if (!this->createRenderTargetObjects(desc, lifeCycle, idDesc.fInfo, &rtIDDesc)) {
            GL_CALL(DeleteTextures(1, &idDesc.fInfo.fID));
            return return_null_texture();
        }
        tex = new GrGLTextureRenderTarget(this, desc, idDesc, rtIDDesc);
    } else {
        tex = new GrGLTexture(this, desc, idDesc);
    }
    tex->setCachedTexParams(initialTexParams, this->getResetTimestamp());
#ifdef TRACE_TEXTURE_CREATION
    SkDebugf("--- new texture [%d] size=(%d %d) config=%d\n",
             glTexDesc.fTextureID, desc.fWidth, desc.fHeight, desc.fConfig);
#endif
    return tex;
}

GrTexture* GrGLGpu::onCreateCompressedTexture(const GrSurfaceDesc& desc,
                                              GrGpuResource::LifeCycle lifeCycle,
                                              const void* srcData) {
    // Make sure that we're not flipping Y.
    if (kBottomLeft_GrSurfaceOrigin == desc.fOrigin) {
        return return_null_texture();
    }

    GrGLTexture::IDDesc idDesc;
    idDesc.fInfo.fID = 0;
    GL_CALL(GenTextures(1, &idDesc.fInfo.fID));
    idDesc.fLifeCycle = lifeCycle;
    // We only support GL_TEXTURE_2D at the moment.
    idDesc.fInfo.fTarget = GR_GL_TEXTURE_2D;

    if (!idDesc.fInfo.fID) {
        return return_null_texture();
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(idDesc.fInfo.fTarget, idDesc.fInfo.fID));

    // Some drivers like to know filter/wrap before seeing glTexImage2D. Some
    // drivers have a bug where an FBO won't be complete if it includes a
    // texture that is not mipmap complete (considering the filter in use).
    GrGLTexture::TexParams initialTexParams;
    // we only set a subset here so invalidate first
    initialTexParams.invalidate();
    initialTexParams.fMinFilter = GR_GL_NEAREST;
    initialTexParams.fMagFilter = GR_GL_NEAREST;
    initialTexParams.fWrapS = GR_GL_CLAMP_TO_EDGE;
    initialTexParams.fWrapT = GR_GL_CLAMP_TO_EDGE;
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_MAG_FILTER,
                          initialTexParams.fMagFilter));
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_MIN_FILTER,
                          initialTexParams.fMinFilter));
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_WRAP_S,
                          initialTexParams.fWrapS));
    GL_CALL(TexParameteri(idDesc.fInfo.fTarget,
                          GR_GL_TEXTURE_WRAP_T,
                          initialTexParams.fWrapT));

    if (!this->uploadCompressedTexData(desc, idDesc.fInfo.fTarget, srcData)) {
        GL_CALL(DeleteTextures(1, &idDesc.fInfo.fID));
        return return_null_texture();
    }

    GrGLTexture* tex;
    tex = new GrGLTexture(this, desc, idDesc);
    tex->setCachedTexParams(initialTexParams, this->getResetTimestamp());
#ifdef TRACE_TEXTURE_CREATION
    SkDebugf("--- new compressed texture [%d] size=(%d %d) config=%d\n",
             glTexDesc.fTextureID, desc.fWidth, desc.fHeight, desc.fConfig);
#endif
    return tex;
}

namespace {

const GrGLuint kUnknownBitCount = GrGLStencilAttachment::kUnknownBitCount;

void inline get_stencil_rb_sizes(const GrGLInterface* gl,
                                 GrGLStencilAttachment::Format* format) {

    // we shouldn't ever know one size and not the other
    SkASSERT((kUnknownBitCount == format->fStencilBits) ==
             (kUnknownBitCount == format->fTotalBits));
    if (kUnknownBitCount == format->fStencilBits) {
        GR_GL_GetRenderbufferParameteriv(gl, GR_GL_RENDERBUFFER,
                                         GR_GL_RENDERBUFFER_STENCIL_SIZE,
                                         (GrGLint*)&format->fStencilBits);
        if (format->fPacked) {
            GR_GL_GetRenderbufferParameteriv(gl, GR_GL_RENDERBUFFER,
                                             GR_GL_RENDERBUFFER_DEPTH_SIZE,
                                             (GrGLint*)&format->fTotalBits);
            format->fTotalBits += format->fStencilBits;
        } else {
            format->fTotalBits = format->fStencilBits;
        }
    }
}
}

int GrGLGpu::getCompatibleStencilIndex(GrPixelConfig config) {
    static const int kSize = 16;
    SkASSERT(this->caps()->isConfigRenderable(config, false));
    if (!this->glCaps().hasStencilFormatBeenDeterminedForConfig(config)) {
        // Default to unsupported, set this if we find a stencil format that works.
        int firstWorkingStencilFormatIndex = -1;
        // Create color texture
        GrGLuint colorID = 0;
        GL_CALL(GenTextures(1, &colorID));
        this->setScratchTextureUnit();
        GL_CALL(BindTexture(GR_GL_TEXTURE_2D, colorID));
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_MAG_FILTER,
                              GR_GL_NEAREST));
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_MIN_FILTER,
                              GR_GL_NEAREST));
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_WRAP_S,
                              GR_GL_CLAMP_TO_EDGE));
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D,
                              GR_GL_TEXTURE_WRAP_T,
                              GR_GL_CLAMP_TO_EDGE));

        GrGLenum internalFormat;
        GrGLenum externalFormat;
        GrGLenum externalType;
        if (!this->glCaps().getTexImageFormats(config, config, &internalFormat, &externalFormat,
                                               &externalType)) {
            return false;
        }
        CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
        GL_ALLOC_CALL(this->glInterface(), TexImage2D(GR_GL_TEXTURE_2D,
                                                      0,
                                                      internalFormat,
                                                      kSize,
                                                      kSize,
                                                      0,
                                                      externalFormat,
                                                      externalType,
                                                      NULL));
        if (GR_GL_NO_ERROR != CHECK_ALLOC_ERROR(this->glInterface())) {
            GL_CALL(DeleteTextures(1, &colorID));
            return -1;
        }

        // unbind the texture from the texture unit before binding it to the frame buffer
        GL_CALL(BindTexture(GR_GL_TEXTURE_2D, 0));

        // Create Framebuffer
        GrGLuint fb = 0;
        GL_CALL(GenFramebuffers(1, &fb));
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, fb));
        fHWBoundRenderTargetUniqueID = SK_InvalidUniqueID;
        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                     GR_GL_COLOR_ATTACHMENT0,
                                     GR_GL_TEXTURE_2D,
                                     colorID,
                                     0));
        GrGLuint sbRBID = 0;
        GL_CALL(GenRenderbuffers(1, &sbRBID));

        // look over formats till I find a compatible one
        int stencilFmtCnt = this->glCaps().stencilFormats().count();
        if (sbRBID) {
            GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, sbRBID));
            for (int i = 0; i < stencilFmtCnt && sbRBID; ++i) {
                const GrGLCaps::StencilFormat& sFmt = this->glCaps().stencilFormats()[i];
                CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
                GL_ALLOC_CALL(this->glInterface(), RenderbufferStorage(GR_GL_RENDERBUFFER,
                                                                       sFmt.fInternalFormat,
                                                                       kSize, kSize));
                if (GR_GL_NO_ERROR == CHECK_ALLOC_ERROR(this->glInterface())) {
                    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                    GR_GL_STENCIL_ATTACHMENT,
                                                    GR_GL_RENDERBUFFER, sbRBID));
                    if (sFmt.fPacked) {
                        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                        GR_GL_DEPTH_ATTACHMENT,
                                                        GR_GL_RENDERBUFFER, sbRBID));
                    } else {
                        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                        GR_GL_DEPTH_ATTACHMENT,
                                                        GR_GL_RENDERBUFFER, 0));
                    }
                    GrGLenum status;
                    GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
                    if (status == GR_GL_FRAMEBUFFER_COMPLETE) {
                        firstWorkingStencilFormatIndex = i;
                        break;
                    }
                    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                    GR_GL_STENCIL_ATTACHMENT,
                                                    GR_GL_RENDERBUFFER, 0));
                    if (sFmt.fPacked) {
                        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                        GR_GL_DEPTH_ATTACHMENT,
                                                        GR_GL_RENDERBUFFER, 0));
                    }
                }
            }
            GL_CALL(DeleteRenderbuffers(1, &sbRBID));
        }
        GL_CALL(DeleteTextures(1, &colorID));
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, 0));
        GL_CALL(DeleteFramebuffers(1, &fb));
        fGLContext->caps()->setStencilFormatIndexForConfig(config, firstWorkingStencilFormatIndex);
    }
    return this->glCaps().getStencilFormatIndexForConfig(config);
}

GrStencilAttachment* GrGLGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                     int width,
                                                                     int height) {
    // All internally created RTs are also textures. We don't create
    // SBs for a client's standalone RT (that is a RT that isn't also a texture).
    SkASSERT(rt->asTexture());
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    int samples = rt->numStencilSamples();
    GrGLStencilAttachment::IDDesc sbDesc;

    int sIdx = this->getCompatibleStencilIndex(rt->config());
    if (sIdx < 0) {
        return nullptr;
    }

    if (!sbDesc.fRenderbufferID) {
        GL_CALL(GenRenderbuffers(1, &sbDesc.fRenderbufferID));
    }
    if (!sbDesc.fRenderbufferID) {
        return nullptr;
    }
    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, sbDesc.fRenderbufferID));
    const GrGLCaps::StencilFormat& sFmt = this->glCaps().stencilFormats()[sIdx];
    CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
    // we do this "if" so that we don't call the multisample
    // version on a GL that doesn't have an MSAA extension.
    if (samples > 0) {
        SkAssertResult(renderbuffer_storage_msaa(*fGLContext,
                                                 samples,
                                                 sFmt.fInternalFormat,
                                                 width, height));
    } else {
        GL_ALLOC_CALL(this->glInterface(), RenderbufferStorage(GR_GL_RENDERBUFFER,
                                                               sFmt.fInternalFormat,
                                                               width, height));
        SkASSERT(GR_GL_NO_ERROR == check_alloc_error(rt->desc(), this->glInterface()));
    }
    fStats.incStencilAttachmentCreates();
    // After sized formats we attempt an unsized format and take
    // whatever sizes GL gives us. In that case we query for the size.
    GrGLStencilAttachment::Format format = sFmt;
    get_stencil_rb_sizes(this->glInterface(), &format);
    GrGLStencilAttachment* stencil = new GrGLStencilAttachment(this,
                                                               sbDesc,
                                                               width,
                                                               height,
                                                               samples,
                                                               format);
    return stencil;
}

////////////////////////////////////////////////////////////////////////////////

// GL_STREAM_DRAW triggers an optimization in Chromium's GPU process where a client's vertex buffer
// objects are implemented as client-side-arrays on tile-deferred architectures.
#define DYNAMIC_USAGE_PARAM GR_GL_STREAM_DRAW

GrVertexBuffer* GrGLGpu::onCreateVertexBuffer(size_t size, bool dynamic) {
    GrGLVertexBuffer::Desc desc;
    desc.fUsage = dynamic ? GrGLBufferImpl::kDynamicDraw_Usage : GrGLBufferImpl::kStaticDraw_Usage;
    desc.fSizeInBytes = size;

    if (this->glCaps().useNonVBOVertexAndIndexDynamicData() && dynamic) {
        desc.fID = 0;
        GrGLVertexBuffer* vertexBuffer = new GrGLVertexBuffer(this, desc);
        return vertexBuffer;
    } else {
        desc.fID = 0;
        GL_CALL(GenBuffers(1, &desc.fID));
        if (desc.fID) {
            fHWGeometryState.setVertexBufferID(this, desc.fID);
            CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
            // make sure driver can allocate memory for this buffer
            GL_ALLOC_CALL(this->glInterface(),
                          BufferData(GR_GL_ARRAY_BUFFER,
                                     (GrGLsizeiptr) desc.fSizeInBytes,
                                     nullptr,   // data ptr
                                     dynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW));
            if (CHECK_ALLOC_ERROR(this->glInterface()) != GR_GL_NO_ERROR) {
                GL_CALL(DeleteBuffers(1, &desc.fID));
                this->notifyVertexBufferDelete(desc.fID);
                return nullptr;
            }
            GrGLVertexBuffer* vertexBuffer = new GrGLVertexBuffer(this, desc);
            return vertexBuffer;
        }
        return nullptr;
    }
}

GrIndexBuffer* GrGLGpu::onCreateIndexBuffer(size_t size, bool dynamic) {
    GrGLIndexBuffer::Desc desc;
    desc.fUsage = dynamic ? GrGLBufferImpl::kDynamicDraw_Usage : GrGLBufferImpl::kStaticDraw_Usage;
    desc.fSizeInBytes = size;

    if (this->glCaps().useNonVBOVertexAndIndexDynamicData() && dynamic) {
        desc.fID = 0;
        GrIndexBuffer* indexBuffer = new GrGLIndexBuffer(this, desc);
        return indexBuffer;
    } else {
        desc.fID = 0;
        GL_CALL(GenBuffers(1, &desc.fID));
        if (desc.fID) {
            fHWGeometryState.setIndexBufferIDOnDefaultVertexArray(this, desc.fID);
            CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
            // make sure driver can allocate memory for this buffer
            GL_ALLOC_CALL(this->glInterface(),
                          BufferData(GR_GL_ELEMENT_ARRAY_BUFFER,
                                     (GrGLsizeiptr) desc.fSizeInBytes,
                                     nullptr,  // data ptr
                                     dynamic ? DYNAMIC_USAGE_PARAM : GR_GL_STATIC_DRAW));
            if (CHECK_ALLOC_ERROR(this->glInterface()) != GR_GL_NO_ERROR) {
                GL_CALL(DeleteBuffers(1, &desc.fID));
                this->notifyIndexBufferDelete(desc.fID);
                return nullptr;
            }
            GrIndexBuffer* indexBuffer = new GrGLIndexBuffer(this, desc);
            return indexBuffer;
        }
        return nullptr;
    }
}

GrTransferBuffer* GrGLGpu::onCreateTransferBuffer(size_t size, TransferType xferType) {
    GrGLCaps::TransferBufferType xferBufferType = this->ctxInfo().caps()->transferBufferType();
    if (GrGLCaps::kNone_TransferBufferType == xferBufferType) {
        return nullptr;
    }

    GrGLTransferBuffer::Desc desc;
    bool toGpu = (kCpuToGpu_TransferType == xferType);
    desc.fUsage = toGpu ? GrGLBufferImpl::kStreamDraw_Usage : GrGLBufferImpl::kStreamRead_Usage;

    desc.fSizeInBytes = size;
    desc.fID = 0;
    GL_CALL(GenBuffers(1, &desc.fID));
    if (desc.fID) {
        CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
        // make sure driver can allocate memory for this bmapuffer
        GrGLenum target;
        if (GrGLCaps::kChromium_TransferBufferType == xferBufferType) {
            target = toGpu ? GR_GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM
                         : GR_GL_PIXEL_PACK_TRANSFER_BUFFER_CHROMIUM;
        } else {
            SkASSERT(GrGLCaps::kPBO_TransferBufferType == xferBufferType);
            target = toGpu ? GR_GL_PIXEL_UNPACK_BUFFER : GR_GL_PIXEL_PACK_BUFFER;
        }
        GL_CALL(BindBuffer(target, desc.fID));
        GL_ALLOC_CALL(this->glInterface(), 
                      BufferData(target,
                                 (GrGLsizeiptr) desc.fSizeInBytes,
                                 nullptr,  // data ptr
                                 (toGpu ? GR_GL_STREAM_DRAW : GR_GL_STREAM_READ)));
        if (CHECK_ALLOC_ERROR(this->glInterface()) != GR_GL_NO_ERROR) {
            GL_CALL(DeleteBuffers(1, &desc.fID));
            return nullptr;
        }
        GrTransferBuffer* transferBuffer = new GrGLTransferBuffer(this, desc, target);
        return transferBuffer;
    }

    return nullptr;
}

void GrGLGpu::flushScissor(const GrScissorState& scissorState,
                           const GrGLIRect& rtViewport,
                           GrSurfaceOrigin rtOrigin) {
    if (scissorState.enabled()) {
        GrGLIRect scissor;
        scissor.setRelativeTo(rtViewport,
                              scissorState.rect().fLeft,
                              scissorState.rect().fTop,
                              scissorState.rect().width(),
                              scissorState.rect().height(),
                              rtOrigin);
        // if the scissor fully contains the viewport then we fall through and
        // disable the scissor test.
        if (!scissor.contains(rtViewport)) {
            if (fHWScissorSettings.fRect != scissor) {
                scissor.pushToGLScissor(this->glInterface());
                fHWScissorSettings.fRect = scissor;
            }
            if (kYes_TriState != fHWScissorSettings.fEnabled) {
                GL_CALL(Enable(GR_GL_SCISSOR_TEST));
                fHWScissorSettings.fEnabled = kYes_TriState;
            }
            return;
        }
    }

    // See fall through note above
    this->disableScissor();
}

bool GrGLGpu::flushGLState(const DrawArgs& args) {
    GrXferProcessor::BlendInfo blendInfo;
    const GrPipeline& pipeline = *args.fPipeline;
    args.fPipeline->getXferProcessor().getBlendInfo(&blendInfo);

    this->flushColorWrite(blendInfo.fWriteColor);
    this->flushDrawFace(pipeline.getDrawFace());

    SkAutoTUnref<GrGLProgram> program(fProgramCache->refProgram(args));
    if (!program) {
        GrCapsDebugf(this->caps(), "Failed to create program!\n");
        return false;
    }

    GrGLuint programID = program->programID();
    if (fHWProgramID != programID) {
        GL_CALL(UseProgram(programID));
        fHWProgramID = programID;
    }

    if (blendInfo.fWriteColor) {
        // Swizzle the blend to match what the shader will output.
        const GrSwizzle& swizzle = this->glCaps().glslCaps()->configOutputSwizzle(
            args.fPipeline->getRenderTarget()->config());
        this->flushBlend(blendInfo, swizzle);
    }

    SkSTArray<8, const GrTextureAccess*> textureAccesses;
    program->setData(*args.fPrimitiveProcessor, pipeline, &textureAccesses);

    int numTextureAccesses = textureAccesses.count();
    for (int i = 0; i < numTextureAccesses; i++) {
        this->bindTexture(i, textureAccesses[i]->getParams(),
                          static_cast<GrGLTexture*>(textureAccesses[i]->getTexture()));
    }

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(pipeline.getRenderTarget());
    this->flushStencil(pipeline.getStencil());
    this->flushScissor(pipeline.getScissorState(), glRT->getViewport(), glRT->origin());
    this->flushHWAAState(glRT, pipeline.isHWAntialiasState());

    // This must come after textures are flushed because a texture may need
    // to be msaa-resolved (which will modify bound FBO state).
    this->flushRenderTarget(glRT, nullptr);

    return true;
}

void GrGLGpu::setupGeometry(const GrPrimitiveProcessor& primProc,
                            const GrNonInstancedVertices& vertices,
                            size_t* indexOffsetInBytes) {
    GrGLVertexBuffer* vbuf;
    vbuf = (GrGLVertexBuffer*) vertices.vertexBuffer();

    SkASSERT(vbuf);
    SkASSERT(!vbuf->isMapped());

    GrGLIndexBuffer* ibuf = nullptr;
    if (vertices.isIndexed()) {
        SkASSERT(indexOffsetInBytes);

        *indexOffsetInBytes = 0;
        ibuf = (GrGLIndexBuffer*)vertices.indexBuffer();

        SkASSERT(ibuf);
        SkASSERT(!ibuf->isMapped());
        *indexOffsetInBytes += ibuf->baseOffset();
    }
    GrGLAttribArrayState* attribState =
        fHWGeometryState.bindArrayAndBuffersToDraw(this, vbuf, ibuf);

    int vaCount = primProc.numAttribs();
    if (vaCount > 0) {

        GrGLsizei stride = static_cast<GrGLsizei>(primProc.getVertexStride());

        size_t vertexOffsetInBytes = stride * vertices.startVertex();

        vertexOffsetInBytes += vbuf->baseOffset();

        uint32_t usedAttribArraysMask = 0;
        size_t offset = 0;

        for (int attribIndex = 0; attribIndex < vaCount; attribIndex++) {
            const GrGeometryProcessor::Attribute& attrib = primProc.getAttrib(attribIndex);
            usedAttribArraysMask |= (1 << attribIndex);
            GrVertexAttribType attribType = attrib.fType;
            attribState->set(this,
                             attribIndex,
                             vbuf->bufferID(),
                             GrGLAttribTypeToLayout(attribType).fCount,
                             GrGLAttribTypeToLayout(attribType).fType,
                             GrGLAttribTypeToLayout(attribType).fNormalized,
                             stride,
                             reinterpret_cast<GrGLvoid*>(vertexOffsetInBytes + offset));
            offset += attrib.fOffset;
        }
        attribState->disableUnusedArrays(this, usedAttribArraysMask);
    }
}

void GrGLGpu::buildProgramDesc(GrProgramDesc* desc,
                               const GrPrimitiveProcessor& primProc,
                               const GrPipeline& pipeline) const {
    if (!GrGLProgramDescBuilder::Build(desc, primProc, pipeline, *this->glCaps().glslCaps())) {
        SkDEBUGFAIL("Failed to generate GL program descriptor");
    }
}

void GrGLGpu::bindBuffer(GrGLuint id, GrGLenum type) {
    this->handleDirtyContext();
    if (GR_GL_ARRAY_BUFFER == type) {
        this->bindVertexBuffer(id);
    } else if (GR_GL_ELEMENT_ARRAY_BUFFER == type) {
        this->bindIndexBufferAndDefaultVertexArray(id);
    } else {
        GR_GL_CALL(this->glInterface(), BindBuffer(type, id));
    }
}

void GrGLGpu::releaseBuffer(GrGLuint id, GrGLenum type) {
    this->handleDirtyContext();
    GL_CALL(DeleteBuffers(1, &id));
    if (GR_GL_ARRAY_BUFFER == type) {
        this->notifyVertexBufferDelete(id);
    } else if (GR_GL_ELEMENT_ARRAY_BUFFER == type) {
        this->notifyIndexBufferDelete(id);
    }
}

static GrGLenum get_gl_usage(GrGLBufferImpl::Usage usage) {
    static const GrGLenum grToGL[] = {
        GR_GL_STATIC_DRAW,   // GrGLBufferImpl::kStaticDraw_Usage
        DYNAMIC_USAGE_PARAM, // GrGLBufferImpl::kDynamicDraw_Usage
        GR_GL_STREAM_DRAW,   // GrGLBufferImpl::kStreamDraw_Usage
        GR_GL_STREAM_READ,   // GrGLBufferImpl::kStreamRead_Usage
    };
    static_assert(SK_ARRAY_COUNT(grToGL) == GrGLBufferImpl::kUsageCount, "array_size_mismatch");

    return grToGL[usage];
}

void* GrGLGpu::mapBuffer(GrGLuint id, GrGLenum type, GrGLBufferImpl::Usage usage, 
                         size_t currentSize, size_t requestedSize) {
    void* mapPtr = nullptr;
    GrGLenum glUsage = get_gl_usage(usage);
    bool readOnly = (GrGLBufferImpl::kStreamRead_Usage == usage);

    // Handling dirty context is done in the bindBuffer call
    switch (this->glCaps().mapBufferType()) {
        case GrGLCaps::kNone_MapBufferType:
            break;
        case GrGLCaps::kMapBuffer_MapBufferType:
            this->bindBuffer(id, type);
            // Let driver know it can discard the old data
            if (GR_GL_USE_BUFFER_DATA_NULL_HINT || currentSize != requestedSize) {
                GL_CALL(BufferData(type, requestedSize, nullptr, glUsage));
            }
            GL_CALL_RET(mapPtr, MapBuffer(type, readOnly ? GR_GL_READ_ONLY : GR_GL_WRITE_ONLY));
            break;
        case GrGLCaps::kMapBufferRange_MapBufferType: {
            this->bindBuffer(id, type);
            // Make sure the GL buffer size agrees with fDesc before mapping.
            if (currentSize != requestedSize) {
                GL_CALL(BufferData(type, requestedSize, nullptr, glUsage));
            }
            GrGLbitfield writeAccess = GR_GL_MAP_WRITE_BIT;
            // TODO: allow the client to specify invalidation in the stream draw case
            if (GrGLBufferImpl::kStreamDraw_Usage != usage) {
                writeAccess |= GR_GL_MAP_INVALIDATE_BUFFER_BIT;
            }
            GL_CALL_RET(mapPtr, MapBufferRange(type, 0, requestedSize, readOnly ? 
                                                                       GR_GL_MAP_READ_BIT :                                                       
                                                                       writeAccess));
            break;
        }
        case GrGLCaps::kChromium_MapBufferType:
            this->bindBuffer(id, type);
            // Make sure the GL buffer size agrees with fDesc before mapping.
            if (currentSize != requestedSize) {
                GL_CALL(BufferData(type, requestedSize, nullptr, glUsage));
            }
            GL_CALL_RET(mapPtr, MapBufferSubData(type, 0, requestedSize, readOnly ? 
                                                                         GR_GL_READ_ONLY : 
                                                                         GR_GL_WRITE_ONLY));
            break;
    }
    return mapPtr;
}

void GrGLGpu::bufferData(GrGLuint id, GrGLenum type, GrGLBufferImpl::Usage usage, 
                         size_t currentSize, const void* src, size_t srcSizeInBytes) {
    SkASSERT(srcSizeInBytes <= currentSize);
    // bindbuffer handles dirty context
    this->bindBuffer(id, type);
    GrGLenum glUsage = get_gl_usage(usage);

#if GR_GL_USE_BUFFER_DATA_NULL_HINT
    if (currentSize == srcSizeInBytes) {
        GL_CALL(BufferData(type, (GrGLsizeiptr) srcSizeInBytes, src, glUsage));
    } else {
        // Before we call glBufferSubData we give the driver a hint using
        // glBufferData with nullptr. This makes the old buffer contents
        // inaccessible to future draws. The GPU may still be processing
        // draws that reference the old contents. With this hint it can
        // assign a different allocation for the new contents to avoid
        // flushing the gpu past draws consuming the old contents.
        // TODO I think we actually want to try calling bufferData here
        GL_CALL(BufferData(type, currentSize, nullptr, glUsage));
        GL_CALL(BufferSubData(type, 0, (GrGLsizeiptr) srcSizeInBytes, src));
    }
#else
    // Note that we're cheating on the size here. Currently no methods
    // allow a partial update that preserves contents of non-updated
    // portions of the buffer (map() does a glBufferData(..size, nullptr..))
    GL_CALL(BufferData(type, srcSizeInBytes, src, glUsage));
#endif
}

void GrGLGpu::unmapBuffer(GrGLuint id, GrGLenum type, void* mapPtr) {
    // bind buffer handles the dirty context
    switch (this->glCaps().mapBufferType()) {
        case GrGLCaps::kNone_MapBufferType:
            SkDEBUGFAIL("Shouldn't get here.");
            return;
        case GrGLCaps::kMapBuffer_MapBufferType: // fall through
        case GrGLCaps::kMapBufferRange_MapBufferType:
            this->bindBuffer(id, type);
            GL_CALL(UnmapBuffer(type));
            break;
        case GrGLCaps::kChromium_MapBufferType:
            this->bindBuffer(id, type);
            GL_CALL(UnmapBufferSubData(mapPtr));
            break;
    }
}

void GrGLGpu::disableScissor() {
    if (kNo_TriState != fHWScissorSettings.fEnabled) {
        GL_CALL(Disable(GR_GL_SCISSOR_TEST));
        fHWScissorSettings.fEnabled = kNo_TriState;
        return;
    }
}

void GrGLGpu::onClear(GrRenderTarget* target, const SkIRect& rect, GrColor color) {
    // parent class should never let us get here with no RT
    SkASSERT(target);
    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);

    this->flushRenderTarget(glRT, &rect);
    GrScissorState scissorState;
    scissorState.set(rect);
    this->flushScissor(scissorState, glRT->getViewport(), glRT->origin());

    GrGLfloat r, g, b, a;
    static const GrGLfloat scale255 = 1.f / 255.f;
    a = GrColorUnpackA(color) * scale255;
    GrGLfloat scaleRGB = scale255;
    r = GrColorUnpackR(color) * scaleRGB;
    g = GrColorUnpackG(color) * scaleRGB;
    b = GrColorUnpackB(color) * scaleRGB;

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWWriteToColor = kYes_TriState;
    GL_CALL(ClearColor(r, g, b, a));
    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

void GrGLGpu::discard(GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    if (!this->caps()->discardRenderTargetSupport()) {
        return;
    }

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(renderTarget);
    if (renderTarget->getUniqueID() != fHWBoundRenderTargetUniqueID) {
        fHWBoundRenderTargetUniqueID = SK_InvalidUniqueID;
        fStats.incRenderTargetBinds();
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, glRT->renderFBOID()));
    }
    switch (this->glCaps().invalidateFBType()) {
        case GrGLCaps::kNone_InvalidateFBType:
            SkFAIL("Should never get here.");
            break;
        case GrGLCaps::kInvalidate_InvalidateFBType:
            if (0 == glRT->renderFBOID()) {
                //  When rendering to the default framebuffer the legal values for attachments
                //  are GL_COLOR, GL_DEPTH, GL_STENCIL, ... rather than the various FBO attachment
                //  types.
                static const GrGLenum attachments[] = { GR_GL_COLOR };
                GL_CALL(InvalidateFramebuffer(GR_GL_FRAMEBUFFER, SK_ARRAY_COUNT(attachments),
                        attachments));
            } else {
                static const GrGLenum attachments[] = { GR_GL_COLOR_ATTACHMENT0 };
                GL_CALL(InvalidateFramebuffer(GR_GL_FRAMEBUFFER, SK_ARRAY_COUNT(attachments),
                        attachments));
            }
            break;
        case GrGLCaps::kDiscard_InvalidateFBType: {
            if (0 == glRT->renderFBOID()) {
                //  When rendering to the default framebuffer the legal values for attachments
                //  are GL_COLOR, GL_DEPTH, GL_STENCIL, ... rather than the various FBO attachment
                //  types. See glDiscardFramebuffer() spec.
                static const GrGLenum attachments[] = { GR_GL_COLOR };
                GL_CALL(DiscardFramebuffer(GR_GL_FRAMEBUFFER, SK_ARRAY_COUNT(attachments),
                        attachments));
            } else {
                static const GrGLenum attachments[] = { GR_GL_COLOR_ATTACHMENT0 };
                GL_CALL(DiscardFramebuffer(GR_GL_FRAMEBUFFER, SK_ARRAY_COUNT(attachments),
                        attachments));
            }
            break;
        }
    }
    renderTarget->flagAsResolved();
}

void GrGLGpu::clearStencil(GrRenderTarget* target) {
    if (nullptr == target) {
        return;
    }
    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);
    this->flushRenderTarget(glRT, &SkIRect::EmptyIRect());

    this->disableScissor();

    GL_CALL(StencilMask(0xffffffff));
    GL_CALL(ClearStencil(0));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

void GrGLGpu::onClearStencilClip(GrRenderTarget* target, const SkIRect& rect, bool insideClip) {
    SkASSERT(target);

    GrStencilAttachment* sb = target->renderTargetPriv().getStencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);
    GrGLint stencilBitCount =  sb->bits();
#if 0
    SkASSERT(stencilBitCount > 0);
    GrGLint clipStencilMask  = (1 << (stencilBitCount - 1));
#else
    // we could just clear the clip bit but when we go through
    // ANGLE a partial stencil mask will cause clears to be
    // turned into draws. Our contract on GrDrawTarget says that
    // changing the clip between stencil passes may or may not
    // zero the client's clip bits. So we just clear the whole thing.
    static const GrGLint clipStencilMask  = ~0;
#endif
    GrGLint value;
    if (insideClip) {
        value = (1 << (stencilBitCount - 1));
    } else {
        value = 0;
    }
    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);
    this->flushRenderTarget(glRT, &SkIRect::EmptyIRect());

    GrScissorState scissorState;
    scissorState.set(rect);
    this->flushScissor(scissorState, glRT->getViewport(), glRT->origin());

    GL_CALL(StencilMask((uint32_t) clipStencilMask));
    GL_CALL(ClearStencil(value));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

static bool read_pixels_pays_for_y_flip(GrRenderTarget* renderTarget, const GrGLCaps& caps,
                                        int width, int height,  GrPixelConfig config,
                                        size_t rowBytes) {
    // If this render target is already TopLeft, we don't need to flip.
    if (kTopLeft_GrSurfaceOrigin == renderTarget->origin()) {
        return false;
    }

    // If the read is really small or smaller than the min texture size, don't force a draw.
    static const int kMinSize = 32;
    if (width < kMinSize || height < kMinSize) {
        return false;
    }

    // if GL can do the flip then we'll never pay for it.
    if (caps.packFlipYSupport()) {
        return false;
    }

    // If we have to do memcpy to handle non-trim rowBytes then we
    // get the flip for free. Otherwise it costs.
    // Note that we're assuming that 0 rowBytes has already been handled and that the width has been
    // clipped.
    return caps.packRowLengthSupport() || GrBytesPerPixel(config) * width == rowBytes;
}

bool GrGLGpu::onGetReadPixelsInfo(GrSurface* srcSurface, int width, int height, size_t rowBytes,
                                  GrPixelConfig readConfig, DrawPreference* drawPreference,
                                  ReadPixelTempDrawInfo* tempDrawInfo) {
    // This subclass can only read pixels from a render target. We could use glTexSubImage2D on
    // GL versions that support it but we don't today.
    if (!srcSurface->asRenderTarget()) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    if (GrPixelConfigIsSRGB(srcSurface->config()) != GrPixelConfigIsSRGB(readConfig)) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    tempDrawInfo->fSwapRAndB = false;

    // These settings we will always want if a temp draw is performed. The config is set below
    // depending on whether we want to do a R/B swap or not.
    tempDrawInfo->fTempSurfaceDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 0;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // no CPU y-flip for TL.
    tempDrawInfo->fUseExactScratch = this->glCaps().partialFBOReadIsSlow();

    // Start off assuming that any temp draw should be to the readConfig, then check if that will
    // be inefficient.
    GrPixelConfig srcConfig = srcSurface->config();
    tempDrawInfo->fTempSurfaceDesc.fConfig = readConfig;

    if (this->glCaps().rgba8888PixelsOpsAreSlow() && kRGBA_8888_GrPixelConfig == readConfig) {
        tempDrawInfo->fTempSurfaceDesc.fConfig = kBGRA_8888_GrPixelConfig;
        tempDrawInfo->fSwapRAndB = true;
        ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
    } else if (kMesa_GrGLDriver == this->glContext().driver() &&
               GrBytesPerPixel(readConfig) == 4 &&
               GrPixelConfigSwapRAndB(readConfig) == srcConfig) {
        // Mesa 3D takes a slow path on when reading back  BGRA from an RGBA surface and vice-versa.
        // Better to do a draw with a R/B swap and then read as the original config.
        tempDrawInfo->fTempSurfaceDesc.fConfig = srcConfig;
        tempDrawInfo->fSwapRAndB = true;
        ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
    } else if (readConfig == kBGRA_8888_GrPixelConfig &&
               !this->glCaps().readPixelsSupported(this->glInterface(), readConfig, srcConfig)) {
        tempDrawInfo->fTempSurfaceDesc.fConfig = kRGBA_8888_GrPixelConfig;
        tempDrawInfo->fSwapRAndB = true;
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    GrRenderTarget* srcAsRT = srcSurface->asRenderTarget();
    if (!srcAsRT) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    } else if (read_pixels_pays_for_y_flip(srcAsRT, this->glCaps(), width, height, readConfig,
                                           rowBytes)) {
        ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
    }

    return true;
}

bool GrGLGpu::onReadPixels(GrSurface* surface,
                           int left, int top,
                           int width, int height,
                           GrPixelConfig config,
                           void* buffer,
                           size_t rowBytes) {
    SkASSERT(surface);

    GrGLRenderTarget* tgt = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    if (!tgt) {
        return false;
    }

    // OpenGL doesn't do sRGB <-> linear conversions when reading and writing pixels.
    if (GrPixelConfigIsSRGB(surface->config()) != GrPixelConfigIsSRGB(config)) {
        return false;
    }

    GrGLenum externalFormat;
    GrGLenum externalType;
    if (!this->glCaps().getReadPixelsFormat(surface->config(), config, &externalFormat,
                                            &externalType)) {
        return false;
    }
    bool flipY = kBottomLeft_GrSurfaceOrigin == surface->origin();

    // resolve the render target if necessary
    switch (tgt->getResolveType()) {
        case GrGLRenderTarget::kCantResolve_ResolveType:
            return false;
        case GrGLRenderTarget::kAutoResolves_ResolveType:
            this->flushRenderTarget(tgt, &SkIRect::EmptyIRect());
            break;
        case GrGLRenderTarget::kCanResolve_ResolveType:
            this->onResolveRenderTarget(tgt);
            // we don't track the state of the READ FBO ID.
            fStats.incRenderTargetBinds();
            GL_CALL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER,
                                    tgt->textureFBOID()));
            break;
        default:
            SkFAIL("Unknown resolve type");
    }

    const GrGLIRect& glvp = tgt->getViewport();

    // the read rect is viewport-relative
    GrGLIRect readRect;
    readRect.setRelativeTo(glvp, left, top, width, height, tgt->origin());

    size_t tightRowBytes = GrBytesPerPixel(config) * width;

    size_t readDstRowBytes = tightRowBytes;
    void* readDst = buffer;

    // determine if GL can read using the passed rowBytes or if we need
    // a scratch buffer.
    SkAutoSMalloc<32 * sizeof(GrColor)> scratch;
    if (rowBytes != tightRowBytes) {
        if (this->glCaps().packRowLengthSupport()) {
            SkASSERT(!(rowBytes % sizeof(GrColor)));
            GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH,
                                static_cast<GrGLint>(rowBytes / sizeof(GrColor))));
            readDstRowBytes = rowBytes;
        } else {
            scratch.reset(tightRowBytes * height);
            readDst = scratch.get();
        }
    }
    if (flipY && this->glCaps().packFlipYSupport()) {
        GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, 1));
    }
    GL_CALL(PixelStorei(GR_GL_PACK_ALIGNMENT, config_alignment(config)));

    GL_CALL(ReadPixels(readRect.fLeft, readRect.fBottom,
                       readRect.fWidth, readRect.fHeight,
                       externalFormat, externalType, readDst));
    if (readDstRowBytes != tightRowBytes) {
        SkASSERT(this->glCaps().packRowLengthSupport());
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    }
    if (flipY && this->glCaps().packFlipYSupport()) {
        GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, 0));
        flipY = false;
    }

    // now reverse the order of the rows, since GL's are bottom-to-top, but our
    // API presents top-to-bottom. We must preserve the padding contents. Note
    // that the above readPixels did not overwrite the padding.
    if (readDst == buffer) {
        SkASSERT(rowBytes == readDstRowBytes);
        if (flipY) {
            scratch.reset(tightRowBytes);
            void* tmpRow = scratch.get();
            // flip y in-place by rows
            const int halfY = height >> 1;
            char* top = reinterpret_cast<char*>(buffer);
            char* bottom = top + (height - 1) * rowBytes;
            for (int y = 0; y < halfY; y++) {
                memcpy(tmpRow, top, tightRowBytes);
                memcpy(top, bottom, tightRowBytes);
                memcpy(bottom, tmpRow, tightRowBytes);
                top += rowBytes;
                bottom -= rowBytes;
            }
        }
    } else {
        SkASSERT(readDst != buffer);        SkASSERT(rowBytes != tightRowBytes);
        // copy from readDst to buffer while flipping y
        // const int halfY = height >> 1;
        const char* src = reinterpret_cast<const char*>(readDst);
        char* dst = reinterpret_cast<char*>(buffer);
        if (flipY) {
            dst += (height-1) * rowBytes;
        }
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, tightRowBytes);
            src += readDstRowBytes;
            if (!flipY) {
                dst += rowBytes;
            } else {
                dst -= rowBytes;
            }
        }
    }
    return true;
}

void GrGLGpu::flushRenderTarget(GrGLRenderTarget* target, const SkIRect* bound) {

    SkASSERT(target);

    uint32_t rtID = target->getUniqueID();
    if (fHWBoundRenderTargetUniqueID != rtID) {
        fStats.incRenderTargetBinds();
        GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, target->renderFBOID()));
#ifdef SK_DEBUG
        // don't do this check in Chromium -- this is causing
        // lots of repeated command buffer flushes when the compositor is
        // rendering with Ganesh, which is really slow; even too slow for
        // Debug mode.
        if (kChromium_GrGLDriver != this->glContext().driver()) {
            GrGLenum status;
            GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                SkDebugf("GrGLGpu::flushRenderTarget glCheckFramebufferStatus %x\n", status);
            }
        }
#endif
        fHWBoundRenderTargetUniqueID = rtID;
        const GrGLIRect& vp = target->getViewport();
        if (fHWViewport != vp) {
            vp.pushToGLViewport(this->glInterface());
            fHWViewport = vp;
        }
        if (this->glCaps().srgbWriteControl()) {
            bool enableSRGBWrite = GrPixelConfigIsSRGB(target->config());
            if (enableSRGBWrite && kYes_TriState != fHWSRGBFramebuffer) {
                GL_CALL(Enable(GR_GL_FRAMEBUFFER_SRGB));
                fHWSRGBFramebuffer = kYes_TriState;
            } else if (!enableSRGBWrite && kNo_TriState != fHWSRGBFramebuffer) {
                GL_CALL(Disable(GR_GL_FRAMEBUFFER_SRGB));
                fHWSRGBFramebuffer = kNo_TriState;
            }
        }
    }
    if (nullptr == bound || !bound->isEmpty()) {
        target->flagAsNeedingResolve(bound);
    }

    GrTexture *texture = target->asTexture();
    if (texture) {
        texture->texturePriv().dirtyMipMaps(true);
    }
}

GrGLenum gPrimitiveType2GLMode[] = {
    GR_GL_TRIANGLES,
    GR_GL_TRIANGLE_STRIP,
    GR_GL_TRIANGLE_FAN,
    GR_GL_POINTS,
    GR_GL_LINES,
    GR_GL_LINE_STRIP
};

#define SWAP_PER_DRAW 0

#if SWAP_PER_DRAW
    #if defined(SK_BUILD_FOR_MAC)
        #include <AGL/agl.h>
    #elif defined(SK_BUILD_FOR_WIN32)
        #include <gl/GL.h>
        void SwapBuf() {
            DWORD procID = GetCurrentProcessId();
            HWND hwnd = GetTopWindow(GetDesktopWindow());
            while(hwnd) {
                DWORD wndProcID = 0;
                GetWindowThreadProcessId(hwnd, &wndProcID);
                if(wndProcID == procID) {
                    SwapBuffers(GetDC(hwnd));
                }
                hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
            }
         }
    #endif
#endif

void GrGLGpu::onDraw(const DrawArgs& args, const GrNonInstancedVertices& vertices) {
    if (!this->flushGLState(args)) {
        return;
    }

    size_t indexOffsetInBytes = 0;
    this->setupGeometry(*args.fPrimitiveProcessor, vertices, &indexOffsetInBytes);

    SkASSERT((size_t)vertices.primitiveType() < SK_ARRAY_COUNT(gPrimitiveType2GLMode));

    if (vertices.isIndexed()) {
        GrGLvoid* indices =
            reinterpret_cast<GrGLvoid*>(indexOffsetInBytes + sizeof(uint16_t) *
                                        vertices.startIndex());
        // info.startVertex() was accounted for by setupGeometry.
        GL_CALL(DrawElements(gPrimitiveType2GLMode[vertices.primitiveType()],
                             vertices.indexCount(),
                             GR_GL_UNSIGNED_SHORT,
                             indices));
    } else {
        // Pass 0 for parameter first. We have to adjust glVertexAttribPointer() to account for
        // startVertex in the DrawElements case. So we always rely on setupGeometry to have
        // accounted for startVertex.
        GL_CALL(DrawArrays(gPrimitiveType2GLMode[vertices.primitiveType()], 0,
                           vertices.vertexCount()));
    }
#if SWAP_PER_DRAW
    glFlush();
    #if defined(SK_BUILD_FOR_MAC)
        aglSwapBuffers(aglGetCurrentContext());
        int set_a_break_pt_here = 9;
        aglSwapBuffers(aglGetCurrentContext());
    #elif defined(SK_BUILD_FOR_WIN32)
        SwapBuf();
        int set_a_break_pt_here = 9;
        SwapBuf();
    #endif
#endif
}

void GrGLGpu::onResolveRenderTarget(GrRenderTarget* target) {
    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(target);
    if (rt->needsResolve()) {
        // Some extensions automatically resolves the texture when it is read.
        if (this->glCaps().usesMSAARenderBuffers()) {
            SkASSERT(rt->textureFBOID() != rt->renderFBOID());
            fStats.incRenderTargetBinds();
            fStats.incRenderTargetBinds();
            GL_CALL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, rt->renderFBOID()));
            GL_CALL(BindFramebuffer(GR_GL_DRAW_FRAMEBUFFER, rt->textureFBOID()));
            // make sure we go through flushRenderTarget() since we've modified
            // the bound DRAW FBO ID.
            fHWBoundRenderTargetUniqueID = SK_InvalidUniqueID;
            const GrGLIRect& vp = rt->getViewport();
            const SkIRect dirtyRect = rt->getResolveRect();

            if (GrGLCaps::kES_Apple_MSFBOType == this->glCaps().msFBOType()) {
                // Apple's extension uses the scissor as the blit bounds.
                GrScissorState scissorState;
                scissorState.set(dirtyRect);
                this->flushScissor(scissorState, vp, rt->origin());
                GL_CALL(ResolveMultisampleFramebuffer());
            } else {
                GrGLIRect r;
                r.setRelativeTo(vp, dirtyRect.fLeft, dirtyRect.fTop,
                                dirtyRect.width(), dirtyRect.height(), target->origin());

                int right = r.fLeft + r.fWidth;
                int top = r.fBottom + r.fHeight;

                // BlitFrameBuffer respects the scissor, so disable it.
                this->disableScissor();
                GL_CALL(BlitFramebuffer(r.fLeft, r.fBottom, right, top,
                                        r.fLeft, r.fBottom, right, top,
                                        GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
            }
        }
        rt->flagAsResolved();
    }
}

namespace {


GrGLenum gr_to_gl_stencil_op(GrStencilOp op) {
    static const GrGLenum gTable[] = {
        GR_GL_KEEP,        // kKeep_StencilOp
        GR_GL_REPLACE,     // kReplace_StencilOp
        GR_GL_INCR_WRAP,   // kIncWrap_StencilOp
        GR_GL_INCR,        // kIncClamp_StencilOp
        GR_GL_DECR_WRAP,   // kDecWrap_StencilOp
        GR_GL_DECR,        // kDecClamp_StencilOp
        GR_GL_ZERO,        // kZero_StencilOp
        GR_GL_INVERT,      // kInvert_StencilOp
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kStencilOpCount);
    GR_STATIC_ASSERT(0 == kKeep_StencilOp);
    GR_STATIC_ASSERT(1 == kReplace_StencilOp);
    GR_STATIC_ASSERT(2 == kIncWrap_StencilOp);
    GR_STATIC_ASSERT(3 == kIncClamp_StencilOp);
    GR_STATIC_ASSERT(4 == kDecWrap_StencilOp);
    GR_STATIC_ASSERT(5 == kDecClamp_StencilOp);
    GR_STATIC_ASSERT(6 == kZero_StencilOp);
    GR_STATIC_ASSERT(7 == kInvert_StencilOp);
    SkASSERT((unsigned) op < kStencilOpCount);
    return gTable[op];
}

void set_gl_stencil(const GrGLInterface* gl,
                    const GrStencilSettings& settings,
                    GrGLenum glFace,
                    GrStencilSettings::Face grFace) {
    GrGLenum glFunc = GrToGLStencilFunc(settings.func(grFace));
    GrGLenum glFailOp = gr_to_gl_stencil_op(settings.failOp(grFace));
    GrGLenum glPassOp = gr_to_gl_stencil_op(settings.passOp(grFace));

    GrGLint ref = settings.funcRef(grFace);
    GrGLint mask = settings.funcMask(grFace);
    GrGLint writeMask = settings.writeMask(grFace);

    if (GR_GL_FRONT_AND_BACK == glFace) {
        // we call the combined func just in case separate stencil is not
        // supported.
        GR_GL_CALL(gl, StencilFunc(glFunc, ref, mask));
        GR_GL_CALL(gl, StencilMask(writeMask));
        GR_GL_CALL(gl, StencilOp(glFailOp, GR_GL_KEEP, glPassOp));
    } else {
        GR_GL_CALL(gl, StencilFuncSeparate(glFace, glFunc, ref, mask));
        GR_GL_CALL(gl, StencilMaskSeparate(glFace, writeMask));
        GR_GL_CALL(gl, StencilOpSeparate(glFace, glFailOp, GR_GL_KEEP, glPassOp));
    }
}
}

void GrGLGpu::flushStencil(const GrStencilSettings& stencilSettings) {
    if (fHWStencilSettings != stencilSettings) {
        if (stencilSettings.isDisabled()) {
            if (kNo_TriState != fHWStencilTestEnabled) {
                GL_CALL(Disable(GR_GL_STENCIL_TEST));
                fHWStencilTestEnabled = kNo_TriState;
            }
        } else {
            if (kYes_TriState != fHWStencilTestEnabled) {
                GL_CALL(Enable(GR_GL_STENCIL_TEST));
                fHWStencilTestEnabled = kYes_TriState;
            }
        }
        if (!stencilSettings.isDisabled()) {
            if (this->caps()->twoSidedStencilSupport()) {
                set_gl_stencil(this->glInterface(),
                               stencilSettings,
                               GR_GL_FRONT,
                               GrStencilSettings::kFront_Face);
                set_gl_stencil(this->glInterface(),
                               stencilSettings,
                               GR_GL_BACK,
                               GrStencilSettings::kBack_Face);
            } else {
                set_gl_stencil(this->glInterface(),
                               stencilSettings,
                               GR_GL_FRONT_AND_BACK,
                               GrStencilSettings::kFront_Face);
            }
        }
        fHWStencilSettings = stencilSettings;
    }
}

void GrGLGpu::flushHWAAState(GrRenderTarget* rt, bool useHWAA) {
    SkASSERT(!useHWAA || rt->isStencilBufferMultisampled());

    if (this->glCaps().multisampleDisableSupport()) {
        if (useHWAA) {
            if (kYes_TriState != fMSAAEnabled) {
                GL_CALL(Enable(GR_GL_MULTISAMPLE));
                fMSAAEnabled = kYes_TriState;
            }
        } else {
            if (kNo_TriState != fMSAAEnabled) {
                GL_CALL(Disable(GR_GL_MULTISAMPLE));
                fMSAAEnabled = kNo_TriState;
            }
        }
    }
}

void GrGLGpu::flushBlend(const GrXferProcessor::BlendInfo& blendInfo, const GrSwizzle& swizzle) {
    // Any optimization to disable blending should have already been applied and
    // tweaked the equation to "add" or "subtract", and the coeffs to (1, 0).

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = (kAdd_GrBlendEquation == equation || kSubtract_GrBlendEquation == equation) &&
                    kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff;
    if (blendOff) {
        if (kNo_TriState != fHWBlendState.fEnabled) {
            GL_CALL(Disable(GR_GL_BLEND));

            // Workaround for the ARM KHR_blend_equation_advanced blacklist issue
            // https://code.google.com/p/skia/issues/detail?id=3943
            if (kARM_GrGLVendor == this->ctxInfo().vendor() &&
                GrBlendEquationIsAdvanced(fHWBlendState.fEquation)) {
                SkASSERT(this->caps()->advancedBlendEquationSupport());
                // Set to any basic blending equation.
                GrBlendEquation blend_equation = kAdd_GrBlendEquation;
                GL_CALL(BlendEquation(gXfermodeEquation2Blend[blend_equation]));
                fHWBlendState.fEquation = blend_equation;
            }

            fHWBlendState.fEnabled = kNo_TriState;
        }
        return;
    }

    if (kYes_TriState != fHWBlendState.fEnabled) {
        GL_CALL(Enable(GR_GL_BLEND));
        fHWBlendState.fEnabled = kYes_TriState;
    }

    if (fHWBlendState.fEquation != equation) {
        GL_CALL(BlendEquation(gXfermodeEquation2Blend[equation]));
        fHWBlendState.fEquation = equation;
    }

    if (GrBlendEquationIsAdvanced(equation)) {
        SkASSERT(this->caps()->advancedBlendEquationSupport());
        // Advanced equations have no other blend state.
        return;
    }

    if (fHWBlendState.fSrcCoeff != srcCoeff ||
        fHWBlendState.fDstCoeff != dstCoeff) {
        GL_CALL(BlendFunc(gXfermodeCoeff2Blend[srcCoeff],
                          gXfermodeCoeff2Blend[dstCoeff]));
        fHWBlendState.fSrcCoeff = srcCoeff;
        fHWBlendState.fDstCoeff = dstCoeff;
    }

    if ((BlendCoeffReferencesConstant(srcCoeff) || BlendCoeffReferencesConstant(dstCoeff))) {
        GrColor blendConst = blendInfo.fBlendConstant;
        blendConst = swizzle.applyTo(blendConst);
        if (!fHWBlendState.fConstColorValid || fHWBlendState.fConstColor != blendConst) {
            GrGLfloat c[4];
            GrColorToRGBAFloat(blendConst, c);
            GL_CALL(BlendColor(c[0], c[1], c[2], c[3]));
            fHWBlendState.fConstColor = blendConst;
            fHWBlendState.fConstColorValid = true;
        }
    }
}

static inline GrGLenum tile_to_gl_wrap(SkShader::TileMode tm) {
    static const GrGLenum gWrapModes[] = {
        GR_GL_CLAMP_TO_EDGE,
        GR_GL_REPEAT,
        GR_GL_MIRRORED_REPEAT
    };
    GR_STATIC_ASSERT(SkShader::kTileModeCount == SK_ARRAY_COUNT(gWrapModes));
    GR_STATIC_ASSERT(0 == SkShader::kClamp_TileMode);
    GR_STATIC_ASSERT(1 == SkShader::kRepeat_TileMode);
    GR_STATIC_ASSERT(2 == SkShader::kMirror_TileMode);
    return gWrapModes[tm];
}

static GrGLenum get_component_enum_from_char(char component) {
    switch (component) {
        case 'r':
           return GR_GL_RED;
        case 'g':
           return GR_GL_GREEN;
        case 'b':
           return GR_GL_BLUE;
        case 'a':
           return GR_GL_ALPHA;
        default:
            SkFAIL("Unsupported component");
            return 0;
    }
}

/** If texture swizzling is available using tex parameters then it is preferred over mangling
  the generated shader code. This potentially allows greater reuse of cached shaders. */
static void get_tex_param_swizzle(GrPixelConfig config,
                                  const GrGLCaps& caps,
                                  GrGLenum* glSwizzle) {
    const GrSwizzle& swizzle = caps.configSwizzle(config);
    for (int i = 0; i < 4; ++i) {
        glSwizzle[i] = get_component_enum_from_char(swizzle.c_str()[i]);
    }
}

void GrGLGpu::bindTexture(int unitIdx, const GrTextureParams& params, GrGLTexture* texture) {
    SkASSERT(texture);

#ifdef SK_DEBUG
    if (!this->caps()->npotTextureTileSupport()) {
        const bool tileX = SkShader::kClamp_TileMode != params.getTileModeX();
        const bool tileY = SkShader::kClamp_TileMode != params.getTileModeY();
        if (tileX || tileY) {
            const int w = texture->width();
            const int h = texture->height();
            SkASSERT(SkIsPow2(w) && SkIsPow2(h));
        }
    }
#endif

    // If we created a rt/tex and rendered to it without using a texture and now we're texturing
    // from the rt it will still be the last bound texture, but it needs resolving. So keep this
    // out of the "last != next" check.
    GrGLRenderTarget* texRT = static_cast<GrGLRenderTarget*>(texture->asRenderTarget());
    if (texRT) {
        this->onResolveRenderTarget(texRT);
    }

    uint32_t textureID = texture->getUniqueID();
    GrGLenum target = texture->target();
    if (fHWBoundTextureUniqueIDs[unitIdx] != textureID) {
        this->setTextureUnit(unitIdx);
        GL_CALL(BindTexture(target, texture->textureID()));
        fHWBoundTextureUniqueIDs[unitIdx] = textureID;
    }

    ResetTimestamp timestamp;
    const GrGLTexture::TexParams& oldTexParams = texture->getCachedTexParams(&timestamp);
    bool setAll = timestamp < this->getResetTimestamp();
    GrGLTexture::TexParams newTexParams;

    static GrGLenum glMinFilterModes[] = {
        GR_GL_NEAREST,
        GR_GL_LINEAR,
        GR_GL_LINEAR_MIPMAP_LINEAR
    };
    static GrGLenum glMagFilterModes[] = {
        GR_GL_NEAREST,
        GR_GL_LINEAR,
        GR_GL_LINEAR
    };
    GrTextureParams::FilterMode filterMode = params.filterMode();

    if (GrTextureParams::kMipMap_FilterMode == filterMode) {
        if (!this->caps()->mipMapSupport() || GrPixelConfigIsCompressed(texture->config())) {
            filterMode = GrTextureParams::kBilerp_FilterMode;
        }
    }

    newTexParams.fMinFilter = glMinFilterModes[filterMode];
    newTexParams.fMagFilter = glMagFilterModes[filterMode];

    if (GrTextureParams::kMipMap_FilterMode == filterMode &&
        texture->texturePriv().mipMapsAreDirty()) {
        GL_CALL(GenerateMipmap(target));
        texture->texturePriv().dirtyMipMaps(false);
    }

    newTexParams.fWrapS = tile_to_gl_wrap(params.getTileModeX());
    newTexParams.fWrapT = tile_to_gl_wrap(params.getTileModeY());
    get_tex_param_swizzle(texture->config(), this->glCaps(), newTexParams.fSwizzleRGBA);
    if (setAll || newTexParams.fMagFilter != oldTexParams.fMagFilter) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAG_FILTER, newTexParams.fMagFilter));
    }
    if (setAll || newTexParams.fMinFilter != oldTexParams.fMinFilter) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MIN_FILTER, newTexParams.fMinFilter));
    }
    if (setAll || newTexParams.fWrapS != oldTexParams.fWrapS) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_WRAP_S, newTexParams.fWrapS));
    }
    if (setAll || newTexParams.fWrapT != oldTexParams.fWrapT) {
        this->setTextureUnit(unitIdx);
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_WRAP_T, newTexParams.fWrapT));
    }
    if (this->glCaps().textureSwizzleSupport() &&
        (setAll || memcmp(newTexParams.fSwizzleRGBA,
                          oldTexParams.fSwizzleRGBA,
                          sizeof(newTexParams.fSwizzleRGBA)))) {
        this->setTextureUnit(unitIdx);
        if (this->glStandard() == kGLES_GrGLStandard) {
            // ES3 added swizzle support but not GL_TEXTURE_SWIZZLE_RGBA.
            const GrGLenum* swizzle = newTexParams.fSwizzleRGBA;
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_R, swizzle[0]));
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_G, swizzle[1]));
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_B, swizzle[2]));
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_A, swizzle[3]));
        } else {
            GR_STATIC_ASSERT(sizeof(newTexParams.fSwizzleRGBA[0]) == sizeof(GrGLint));
            const GrGLint* swizzle = reinterpret_cast<const GrGLint*>(newTexParams.fSwizzleRGBA);
            GL_CALL(TexParameteriv(target, GR_GL_TEXTURE_SWIZZLE_RGBA, swizzle));
        }
    }
    texture->setCachedTexParams(newTexParams, this->getResetTimestamp());
}

void GrGLGpu::flushColorWrite(bool writeColor) {
    if (!writeColor) {
        if (kNo_TriState != fHWWriteToColor) {
            GL_CALL(ColorMask(GR_GL_FALSE, GR_GL_FALSE,
                              GR_GL_FALSE, GR_GL_FALSE));
            fHWWriteToColor = kNo_TriState;
        }
    } else {
        if (kYes_TriState != fHWWriteToColor) {
            GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
            fHWWriteToColor = kYes_TriState;
        }
    }
}

void GrGLGpu::flushDrawFace(GrPipelineBuilder::DrawFace face) {
    if (fHWDrawFace != face) {
        switch (face) {
            case GrPipelineBuilder::kCCW_DrawFace:
                GL_CALL(Enable(GR_GL_CULL_FACE));
                GL_CALL(CullFace(GR_GL_BACK));
                break;
            case GrPipelineBuilder::kCW_DrawFace:
                GL_CALL(Enable(GR_GL_CULL_FACE));
                GL_CALL(CullFace(GR_GL_FRONT));
                break;
            case GrPipelineBuilder::kBoth_DrawFace:
                GL_CALL(Disable(GR_GL_CULL_FACE));
                break;
            default:
                SkFAIL("Unknown draw face.");
        }
        fHWDrawFace = face;
    }
}

void GrGLGpu::setTextureUnit(int unit) {
    SkASSERT(unit >= 0 && unit < fHWBoundTextureUniqueIDs.count());
    if (unit != fHWActiveTextureUnitIdx) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + unit));
        fHWActiveTextureUnitIdx = unit;
    }
}

void GrGLGpu::setScratchTextureUnit() {
    // Bind the last texture unit since it is the least likely to be used by GrGLProgram.
    int lastUnitIdx = fHWBoundTextureUniqueIDs.count() - 1;
    if (lastUnitIdx != fHWActiveTextureUnitIdx) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + lastUnitIdx));
        fHWActiveTextureUnitIdx = lastUnitIdx;
    }
    // clear out the this field so that if a program does use this unit it will rebind the correct
    // texture.
    fHWBoundTextureUniqueIDs[lastUnitIdx] = SK_InvalidUniqueID;
}

// Determines whether glBlitFramebuffer could be used between src and dst.
static inline bool can_blit_framebuffer(const GrSurface* dst,
                                        const GrSurface* src,
                                        const GrGLGpu* gpu) {
    if (gpu->glCaps().isConfigRenderable(dst->config(), dst->desc().fSampleCnt > 0) &&
        gpu->glCaps().isConfigRenderable(src->config(), src->desc().fSampleCnt > 0) &&
        gpu->glCaps().usesMSAARenderBuffers()) {
        // ES3 doesn't allow framebuffer blits when the src has MSAA and the configs don't match
        // or the rects are not the same (not just the same size but have the same edges).
        if (GrGLCaps::kES_3_0_MSFBOType == gpu->glCaps().msFBOType() &&
            (src->desc().fSampleCnt > 0 || src->config() != dst->config())) {
           return false;
        }
        const GrGLTexture* dstTex = static_cast<const GrGLTexture*>(dst->asTexture());
        if (dstTex && dstTex->target() != GR_GL_TEXTURE_2D) {
            return false;
        }
        const GrGLTexture* srcTex = static_cast<const GrGLTexture*>(dst->asTexture());
        if (srcTex && srcTex->target() != GR_GL_TEXTURE_2D) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

static inline bool can_copy_texsubimage(const GrSurface* dst,
                                        const GrSurface* src,
                                        const GrGLGpu* gpu) {
    // Table 3.9 of the ES2 spec indicates the supported formats with CopyTexSubImage
    // and BGRA isn't in the spec. There doesn't appear to be any extension that adds it. Perhaps
    // many drivers would allow it to work, but ANGLE does not.
    if (kGLES_GrGLStandard == gpu->glStandard() && gpu->glCaps().bgraIsInternalFormat() &&
        (kBGRA_8888_GrPixelConfig == dst->config() || kBGRA_8888_GrPixelConfig == src->config())) {
        return false;
    }
    const GrGLRenderTarget* dstRT = static_cast<const GrGLRenderTarget*>(dst->asRenderTarget());
    // If dst is multisampled (and uses an extension where there is a separate MSAA renderbuffer)
    // then we don't want to copy to the texture but to the MSAA buffer.
    if (dstRT && dstRT->renderFBOID() != dstRT->textureFBOID()) {
        return false;
    }
    const GrGLRenderTarget* srcRT = static_cast<const GrGLRenderTarget*>(src->asRenderTarget());
    // If the src is multisampled (and uses an extension where there is a separate MSAA
    // renderbuffer) then it is an invalid operation to call CopyTexSubImage
    if (srcRT && srcRT->renderFBOID() != srcRT->textureFBOID()) {
        return false;
    }

    const GrGLTexture* dstTex = static_cast<const GrGLTexture*>(dst->asTexture());
    // CopyTex(Sub)Image writes to a texture and we have no way of dynamically wrapping a RT in a
    // texture.
    if (!dstTex) {
        return false;
    }

    const GrGLTexture* srcTex = static_cast<const GrGLTexture*>(src->asTexture());
    
    // Check that we could wrap the source in an FBO, that the dst is TEXTURE_2D, that no mirroring
    // is required.
    if (gpu->glCaps().isConfigRenderable(src->config(), src->desc().fSampleCnt > 0) &&
        !GrPixelConfigIsCompressed(src->config()) &&
        (!srcTex || srcTex->target() == GR_GL_TEXTURE_2D) &&
        dstTex->target() == GR_GL_TEXTURE_2D &&
        dst->origin() == src->origin()) {
        return true;
    } else {
        return false;
    }
}

// If a temporary FBO was created, its non-zero ID is returned. The viewport that the copy rect is
// relative to is output.
void GrGLGpu::bindSurfaceFBOForCopy(GrSurface* surface, GrGLenum fboTarget, GrGLIRect* viewport,
                                    TempFBOTarget tempFBOTarget) {
    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    if (nullptr == rt) {
        SkASSERT(surface->asTexture());
        GrGLuint texID = static_cast<GrGLTexture*>(surface->asTexture())->textureID();
        GrGLenum target = static_cast<GrGLTexture*>(surface->asTexture())->target();
        GrGLuint* tempFBOID;
        tempFBOID = kSrc_TempFBOTarget == tempFBOTarget ? &fTempSrcFBOID : &fTempDstFBOID;

        if (0 == *tempFBOID) {
            GR_GL_CALL(this->glInterface(), GenFramebuffers(1, tempFBOID));
        }

        fStats.incRenderTargetBinds();
        GR_GL_CALL(this->glInterface(), BindFramebuffer(fboTarget, *tempFBOID));
        GR_GL_CALL(this->glInterface(), FramebufferTexture2D(fboTarget,
                                                             GR_GL_COLOR_ATTACHMENT0,
                                                             target,
                                                             texID,
                                                             0));
        viewport->fLeft = 0;
        viewport->fBottom = 0;
        viewport->fWidth = surface->width();
        viewport->fHeight = surface->height();
    } else {
        fStats.incRenderTargetBinds();
        GR_GL_CALL(this->glInterface(), BindFramebuffer(fboTarget, rt->renderFBOID()));
        *viewport = rt->getViewport();
    }
}

void GrGLGpu::unbindTextureFBOForCopy(GrGLenum fboTarget, GrSurface* surface) {
    // bindSurfaceFBOForCopy temporarily binds textures that are not render targets to 
    if (!surface->asRenderTarget()) {
        SkASSERT(surface->asTexture());
        GrGLenum textureTarget = static_cast<GrGLTexture*>(surface->asTexture())->target();
        GR_GL_CALL(this->glInterface(), FramebufferTexture2D(fboTarget,
                                                             GR_GL_COLOR_ATTACHMENT0,
                                                             textureTarget,
                                                             0,
                                                             0));
    }
}

bool GrGLGpu::initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) const {
    // If the src is a texture, we can implement the blit as a draw assuming the config is
    // renderable.
    if (src->asTexture() && this->caps()->isConfigRenderable(src->config(), false)) {
        desc->fOrigin = kDefault_GrSurfaceOrigin;
        desc->fFlags = kRenderTarget_GrSurfaceFlag;
        desc->fConfig = src->config();
        return true;
    }

    const GrGLTexture* srcTexture = static_cast<const GrGLTexture*>(src->asTexture());
    if (srcTexture && srcTexture->target() != GR_GL_TEXTURE_2D) {
        // Not supported for FBO blit or CopyTexSubImage
        return false;
    }

    // We look for opportunities to use CopyTexSubImage, or fbo blit. If neither are
    // possible and we return false to fallback to creating a render target dst for render-to-
    // texture. This code prefers CopyTexSubImage to fbo blit and avoids triggering temporary fbo
    // creation. It isn't clear that avoiding temporary fbo creation is actually optimal.

    // Check for format issues with glCopyTexSubImage2D
    if (kGLES_GrGLStandard == this->glStandard() && this->glCaps().bgraIsInternalFormat() &&
        kBGRA_8888_GrPixelConfig == src->config()) {
        // glCopyTexSubImage2D doesn't work with this config. If the bgra can be used with fbo blit
        // then we set up for that, otherwise fail.
        if (this->caps()->isConfigRenderable(kBGRA_8888_GrPixelConfig, false)) {
            desc->fOrigin = kDefault_GrSurfaceOrigin;
            desc->fFlags = kRenderTarget_GrSurfaceFlag;
            desc->fConfig = kBGRA_8888_GrPixelConfig;
            return true;
        }
        return false;
    } else if (nullptr == src->asRenderTarget()) {
        // CopyTexSubImage2D or fbo blit would require creating a temp fbo for the src.
        return false;
    }

    const GrGLRenderTarget* srcRT = static_cast<const GrGLRenderTarget*>(src->asRenderTarget());
    if (srcRT && srcRT->renderFBOID() != srcRT->textureFBOID()) {
        // It's illegal to call CopyTexSubImage2D on a MSAA renderbuffer. Set up for FBO blit or
        // fail.
        if (this->caps()->isConfigRenderable(src->config(), false)) {
            desc->fOrigin = kDefault_GrSurfaceOrigin;
            desc->fFlags = kRenderTarget_GrSurfaceFlag;
            desc->fConfig = src->config();
            return true;
        }
        return false;
    }

    // We'll do a CopyTexSubImage. Make the dst a plain old texture.
    desc->fConfig = src->config();
    desc->fOrigin = src->origin();
    desc->fFlags = kNone_GrSurfaceFlags;
    return true;
}

bool GrGLGpu::onCopySurface(GrSurface* dst,
                            GrSurface* src,
                            const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
    // None of our copy methods can handle a swizzle. TODO: Make copySurfaceAsDraw handle the
    // swizzle.
    if (this->glCaps().glslCaps()->configOutputSwizzle(src->config()) !=
        this->glCaps().glslCaps()->configOutputSwizzle(dst->config())) {
        return false;
    }
    if (src->asTexture() && dst->asRenderTarget()) {
        this->copySurfaceAsDraw(dst, src, srcRect, dstPoint);
        return true;
    }
    
    if (can_copy_texsubimage(dst, src, this)) {
        this->copySurfaceAsCopyTexSubImage(dst, src, srcRect, dstPoint);
        return true;
    }

    if (can_blit_framebuffer(dst, src, this)) {
        return this->copySurfaceAsBlitFramebuffer(dst, src, srcRect, dstPoint);
    }

    return false;
}

void GrGLGpu::createCopyPrograms() {
    for (size_t i = 0; i < SK_ARRAY_COUNT(fCopyPrograms); ++i) {
        fCopyPrograms[i].fProgram = 0;
    }
    const char* version = this->glCaps().glslCaps()->versionDeclString();
    static const GrSLType kSamplerTypes[3] = { kSampler2D_GrSLType, kSamplerExternal_GrSLType,
                                               kSampler2DRect_GrSLType };
    SkASSERT(3 == SK_ARRAY_COUNT(fCopyPrograms));
    for (int i = 0; i < 3; ++i) {
        if (kSamplerExternal_GrSLType == kSamplerTypes[i] &&
            !this->glCaps().externalTextureSupport()) {
            continue;
        }
        if (kSampler2DRect_GrSLType == kSamplerTypes[i] &&
            !this->glCaps().rectangleTextureSupport()) {
            continue;
        }
        GrGLSLShaderVar aVertex("a_vertex", kVec2f_GrSLType, GrShaderVar::kAttribute_TypeModifier);
        GrGLSLShaderVar uTexCoordXform("u_texCoordXform", kVec4f_GrSLType,
                                     GrShaderVar::kUniform_TypeModifier);
        GrGLSLShaderVar uPosXform("u_posXform", kVec4f_GrSLType,
                                  GrShaderVar::kUniform_TypeModifier);
        GrGLSLShaderVar uTexture("u_texture", kSamplerTypes[i],
                                 GrShaderVar::kUniform_TypeModifier);
        GrGLSLShaderVar vTexCoord("v_texCoord", kVec2f_GrSLType,
                                  GrShaderVar::kVaryingOut_TypeModifier);
        GrGLSLShaderVar oFragColor("o_FragColor", kVec4f_GrSLType,
                                   GrShaderVar::kOut_TypeModifier);

        SkString vshaderTxt(version);
        aVertex.appendDecl(this->glCaps().glslCaps(), &vshaderTxt);
        vshaderTxt.append(";");
        uTexCoordXform.appendDecl(this->glCaps().glslCaps(), &vshaderTxt);
        vshaderTxt.append(";");
        uPosXform.appendDecl(this->glCaps().glslCaps(), &vshaderTxt);
        vshaderTxt.append(";");
        vTexCoord.appendDecl(this->glCaps().glslCaps(), &vshaderTxt);
        vshaderTxt.append(";");
    
        vshaderTxt.append(
            "// Copy Program VS\n"
            "void main() {"
            "  v_texCoord = a_vertex.xy * u_texCoordXform.xy + u_texCoordXform.zw;"
            "  gl_Position.xy = a_vertex * u_posXform.xy + u_posXform.zw;"
            "  gl_Position.zw = vec2(0, 1);"
            "}"
        );

        SkString fshaderTxt(version);
        if (kSamplerTypes[i] == kSamplerExternal_GrSLType) {
            fshaderTxt.appendf("#extension %s : require\n",
                               this->glCaps().glslCaps()->externalTextureExtensionString());
        }
        GrGLSLAppendDefaultFloatPrecisionDeclaration(kDefault_GrSLPrecision,
                                                     *this->glCaps().glslCaps(),
                                                     &fshaderTxt);
        vTexCoord.setTypeModifier(GrShaderVar::kVaryingIn_TypeModifier);
        vTexCoord.appendDecl(this->glCaps().glslCaps(), &fshaderTxt);
        fshaderTxt.append(";");
        uTexture.appendDecl(this->glCaps().glslCaps(), &fshaderTxt);
        fshaderTxt.append(";");
        const char* fsOutName;
        if (this->glCaps().glslCaps()->mustDeclareFragmentShaderOutput()) {
            oFragColor.appendDecl(this->glCaps().glslCaps(), &fshaderTxt);
            fshaderTxt.append(";");
            fsOutName = oFragColor.c_str();
        } else {
            fsOutName = "gl_FragColor";
        }
        fshaderTxt.appendf(
            "// Copy Program FS\n"
            "void main() {"
            "  %s = %s(u_texture, v_texCoord);"
            "}",
            fsOutName,
            GrGLSLTexture2DFunctionName(kVec2f_GrSLType, kSamplerTypes[i], this->glslGeneration())
        );
    
        GL_CALL_RET(fCopyPrograms[i].fProgram, CreateProgram());
        const char* str;
        GrGLint length;

        str = vshaderTxt.c_str();
        length = SkToInt(vshaderTxt.size());
        GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[i].fProgram,
                                                      GR_GL_VERTEX_SHADER, &str, &length, 1,
                                                      &fStats);

        str = fshaderTxt.c_str();
        length = SkToInt(fshaderTxt.size());
        GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[i].fProgram,
                                                      GR_GL_FRAGMENT_SHADER, &str, &length, 1,
                                                      &fStats);

        GL_CALL(LinkProgram(fCopyPrograms[i].fProgram));

        GL_CALL_RET(fCopyPrograms[i].fTextureUniform,
                    GetUniformLocation(fCopyPrograms[i].fProgram, "u_texture"));
        GL_CALL_RET(fCopyPrograms[i].fPosXformUniform,
                    GetUniformLocation(fCopyPrograms[i].fProgram, "u_posXform"));
        GL_CALL_RET(fCopyPrograms[i].fTexCoordXformUniform,
                    GetUniformLocation(fCopyPrograms[i].fProgram, "u_texCoordXform"));

        GL_CALL(BindAttribLocation(fCopyPrograms[i].fProgram, 0, "a_vertex"));

        GL_CALL(DeleteShader(vshader));
        GL_CALL(DeleteShader(fshader));
    }
    fCopyProgramArrayBuffer = 0;
    GL_CALL(GenBuffers(1, &fCopyProgramArrayBuffer));
    fHWGeometryState.setVertexBufferID(this, fCopyProgramArrayBuffer);
    static const GrGLfloat vdata[] = {
        0, 0,
        0, 1,
        1, 0,
        1, 1
    };
    GL_ALLOC_CALL(this->glInterface(),
                  BufferData(GR_GL_ARRAY_BUFFER,
                             (GrGLsizeiptr) sizeof(vdata),
                             vdata,  // data ptr
                             GR_GL_STATIC_DRAW));
}

void GrGLGpu::createWireRectProgram() {
    SkASSERT(!fWireRectProgram.fProgram);
    GrGLSLShaderVar uColor("u_color", kVec4f_GrSLType, GrShaderVar::kUniform_TypeModifier);
    GrGLSLShaderVar uRect("u_rect", kVec4f_GrSLType, GrShaderVar::kUniform_TypeModifier);
    GrGLSLShaderVar aVertex("a_vertex", kVec2f_GrSLType, GrShaderVar::kAttribute_TypeModifier);
    const char* version = this->glCaps().glslCaps()->versionDeclString();

    // The rect uniform specifies the rectangle in NDC space as a vec4 (left,top,right,bottom). The
    // program is used with a vbo containing the unit square. Vertices are computed from the rect
    // uniform using the 4 vbo vertices.
    SkString vshaderTxt(version);
    aVertex.appendDecl(this->glCaps().glslCaps(), &vshaderTxt);
    vshaderTxt.append(";");
    uRect.appendDecl(this->glCaps().glslCaps(), &vshaderTxt);
    vshaderTxt.append(";");
    vshaderTxt.append(
        "// Wire Rect Program VS\n"
        "void main() {"
        "  gl_Position.x = u_rect.x + a_vertex.x * (u_rect.z - u_rect.x);"
        "  gl_Position.y = u_rect.y + a_vertex.y * (u_rect.w - u_rect.y);"
        "  gl_Position.zw = vec2(0, 1);"
        "}"
    );

    GrGLSLShaderVar oFragColor("o_FragColor", kVec4f_GrSLType, GrShaderVar::kOut_TypeModifier);

    SkString fshaderTxt(version);
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kDefault_GrSLPrecision,
                                                 *this->glCaps().glslCaps(),
                                                 &fshaderTxt);
    uColor.appendDecl(this->glCaps().glslCaps(), &fshaderTxt);
    fshaderTxt.append(";");
    const char* fsOutName;
    if (this->glCaps().glslCaps()->mustDeclareFragmentShaderOutput()) {
        oFragColor.appendDecl(this->glCaps().glslCaps(), &fshaderTxt);
        fshaderTxt.append(";");
        fsOutName = oFragColor.c_str();
    } else {
        fsOutName = "gl_FragColor";
    }
    fshaderTxt.appendf(
        "// Write Rect Program FS\n"
        "void main() {"
        "  %s = %s;"
        "}",
        fsOutName,
        uColor.c_str()
    );

    GL_CALL_RET(fWireRectProgram.fProgram, CreateProgram());
    const char* str;
    GrGLint length;

    str = vshaderTxt.c_str();
    length = SkToInt(vshaderTxt.size());
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fWireRectProgram.fProgram,
                                                  GR_GL_VERTEX_SHADER, &str, &length, 1,
                                                  &fStats);

    str = fshaderTxt.c_str();
    length = SkToInt(fshaderTxt.size());
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fWireRectProgram.fProgram,
                                                  GR_GL_FRAGMENT_SHADER, &str, &length, 1,
                                                  &fStats);

    GL_CALL(LinkProgram(fWireRectProgram.fProgram));

    GL_CALL_RET(fWireRectProgram.fColorUniform,
                GetUniformLocation(fWireRectProgram.fProgram, "u_color"));
    GL_CALL_RET(fWireRectProgram.fRectUniform,
                GetUniformLocation(fWireRectProgram.fProgram, "u_rect"));
    GL_CALL(BindAttribLocation(fWireRectProgram.fProgram, 0, "a_vertex"));

    GL_CALL(DeleteShader(vshader));
    GL_CALL(DeleteShader(fshader));
    GL_CALL(GenBuffers(1, &fWireRectArrayBuffer));
    fHWGeometryState.setVertexBufferID(this, fWireRectArrayBuffer);
    static const GrGLfloat vdata[] = {
        0, 0,
        0, 1,
        1, 1,
        1, 0,
    };
    GL_ALLOC_CALL(this->glInterface(),
                  BufferData(GR_GL_ARRAY_BUFFER,
                             (GrGLsizeiptr) sizeof(vdata),
                             vdata,  // data ptr
                             GR_GL_STATIC_DRAW));
}

void GrGLGpu::drawDebugWireRect(GrRenderTarget* rt, const SkIRect& rect, GrColor color) {
    // TODO: This should swizzle the output to match dst's config, though it is a debugging
    // visualization.

    this->handleDirtyContext();
    if (!fWireRectProgram.fProgram) {
        this->createWireRectProgram();
    }

    int w = rt->width();
    int h = rt->height();

    // Compute the edges of the rectangle (top,left,right,bottom) in NDC space. Must consider
    // whether the render target is flipped or not.
    GrGLfloat edges[4];
    edges[0] = SkIntToScalar(rect.fLeft) + 0.5f;
    edges[2] = SkIntToScalar(rect.fRight) - 0.5f;
    if (kBottomLeft_GrSurfaceOrigin == rt->origin()) {
        edges[1] = h - (SkIntToScalar(rect.fTop) + 0.5f);
        edges[3] = h - (SkIntToScalar(rect.fBottom) - 0.5f);
    } else {
        edges[1] = SkIntToScalar(rect.fTop) + 0.5f;
        edges[3] = SkIntToScalar(rect.fBottom) - 0.5f;
    }
    edges[0] = 2 * edges[0] / w - 1.0f;
    edges[1] = 2 * edges[1] / h - 1.0f;
    edges[2] = 2 * edges[2] / w - 1.0f;
    edges[3] = 2 * edges[3] / h - 1.0f;

    GrGLfloat channels[4];
    static const GrGLfloat scale255 = 1.f / 255.f;
    channels[0] = GrColorUnpackR(color) * scale255;
    channels[1] = GrColorUnpackG(color) * scale255;
    channels[2] = GrColorUnpackB(color) * scale255;
    channels[3] = GrColorUnpackA(color) * scale255;

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(rt->asRenderTarget());
    this->flushRenderTarget(glRT, &rect);

    GL_CALL(UseProgram(fWireRectProgram.fProgram));
    fHWProgramID = fWireRectProgram.fProgram;

    fHWGeometryState.setVertexArrayID(this, 0);

    GrGLAttribArrayState* attribs =
        fHWGeometryState.bindArrayAndBufferToDraw(this, fWireRectArrayBuffer);
    attribs->set(this, 0, fWireRectArrayBuffer, 2, GR_GL_FLOAT, false, 2 * sizeof(GrGLfloat), 0);
    attribs->disableUnusedArrays(this, 0x1);

    GL_CALL(Uniform4fv(fWireRectProgram.fRectUniform, 1, edges));
    GL_CALL(Uniform4fv(fWireRectProgram.fColorUniform, 1, channels));

    GrXferProcessor::BlendInfo blendInfo;
    blendInfo.reset();
    this->flushBlend(blendInfo, GrSwizzle::RGBA());
    this->flushColorWrite(true);
    this->flushDrawFace(GrPipelineBuilder::kBoth_DrawFace);
    this->flushHWAAState(glRT, false);
    this->disableScissor();
    GrStencilSettings stencil;
    stencil.setDisabled();
    this->flushStencil(stencil);

    GL_CALL(DrawArrays(GR_GL_LINE_LOOP, 0, 4));
}


void GrGLGpu::copySurfaceAsDraw(GrSurface* dst,
                                GrSurface* src,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint) {
    int w = srcRect.width();
    int h = srcRect.height();

    GrGLTexture* srcTex = static_cast<GrGLTexture*>(src->asTexture());
    GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
    this->bindTexture(0, params, srcTex);

    GrGLRenderTarget* dstRT = static_cast<GrGLRenderTarget*>(dst->asRenderTarget());
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY, w, h);
    this->flushRenderTarget(dstRT, &dstRect);

    int progIdx = TextureTargetToCopyProgramIdx(srcTex->target());

    GL_CALL(UseProgram(fCopyPrograms[progIdx].fProgram));
    fHWProgramID = fCopyPrograms[progIdx].fProgram;

    fHWGeometryState.setVertexArrayID(this, 0);

    GrGLAttribArrayState* attribs =
        fHWGeometryState.bindArrayAndBufferToDraw(this, fCopyProgramArrayBuffer);
    attribs->set(this, 0, fCopyProgramArrayBuffer, 2, GR_GL_FLOAT, false, 2 * sizeof(GrGLfloat), 0);
    attribs->disableUnusedArrays(this, 0x1);

    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    GrGLfloat dx0 = 2.f * dstPoint.fX / dw - 1.f;
    GrGLfloat dx1 = 2.f * (dstPoint.fX + w) / dw - 1.f;
    GrGLfloat dy0 = 2.f * dstPoint.fY / dh - 1.f;
    GrGLfloat dy1 = 2.f * (dstPoint.fY + h) / dh - 1.f;
    if (kBottomLeft_GrSurfaceOrigin == dst->origin()) {
        dy0 = -dy0;
        dy1 = -dy1;
    }

    GrGLfloat sx0 = (GrGLfloat)srcRect.fLeft;
    GrGLfloat sx1 = (GrGLfloat)(srcRect.fLeft + w);
    GrGLfloat sy0 = (GrGLfloat)srcRect.fTop;
    GrGLfloat sy1 = (GrGLfloat)(srcRect.fTop + h);
    int sh = src->height();
    if (kBottomLeft_GrSurfaceOrigin == src->origin()) {
        sy0 = sh - sy0;
        sy1 = sh - sy1;
    }
    // src rect edges in normalized texture space (0 to 1) unless we're using a RECTANGLE texture.
    GrGLenum srcTarget = srcTex->target();
    if (GR_GL_TEXTURE_RECTANGLE != srcTarget) {
        int sw = src->width();
        sx0 /= sw;
        sx1 /= sw;
        sy0 /= sh;
        sy1 /= sh;
    }

    GL_CALL(Uniform4f(fCopyPrograms[progIdx].fPosXformUniform, dx1 - dx0, dy1 - dy0, dx0, dy0));
    GL_CALL(Uniform4f(fCopyPrograms[progIdx].fTexCoordXformUniform,
                      sx1 - sx0, sy1 - sy0, sx0, sy0));
    GL_CALL(Uniform1i(fCopyPrograms[progIdx].fTextureUniform, 0));

    GrXferProcessor::BlendInfo blendInfo;
    blendInfo.reset();
    this->flushBlend(blendInfo, GrSwizzle::RGBA());
    this->flushColorWrite(true);
    this->flushDrawFace(GrPipelineBuilder::kBoth_DrawFace);
    this->flushHWAAState(dstRT, false);
    this->disableScissor();
    GrStencilSettings stencil;
    stencil.setDisabled();
    this->flushStencil(stencil);

    GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
}

void GrGLGpu::copySurfaceAsCopyTexSubImage(GrSurface* dst,
                                           GrSurface* src,
                                           const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_copy_texsubimage(dst, src, this));
    GrGLIRect srcVP;
    this->bindSurfaceFBOForCopy(src, GR_GL_FRAMEBUFFER, &srcVP, kSrc_TempFBOTarget);
    GrGLTexture* dstTex = static_cast<GrGLTexture*>(dst->asTexture());
    SkASSERT(dstTex);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID = SK_InvalidUniqueID;
    GrGLIRect srcGLRect;
    srcGLRect.setRelativeTo(srcVP,
                            srcRect.fLeft,
                            srcRect.fTop,
                            srcRect.width(),
                            srcRect.height(),
                            src->origin());

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(dstTex->target(), dstTex->textureID()));
    GrGLint dstY;
    if (kBottomLeft_GrSurfaceOrigin == dst->origin()) {
        dstY = dst->height() - (dstPoint.fY + srcGLRect.fHeight);
    } else {
        dstY = dstPoint.fY;
    }
    GL_CALL(CopyTexSubImage2D(dstTex->target(), 0,
                                dstPoint.fX, dstY,
                                srcGLRect.fLeft, srcGLRect.fBottom,
                                srcGLRect.fWidth, srcGLRect.fHeight));
    this->unbindTextureFBOForCopy(GR_GL_FRAMEBUFFER, src);
}

bool GrGLGpu::copySurfaceAsBlitFramebuffer(GrSurface* dst,
                                           GrSurface* src,
                                           const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_blit_framebuffer(dst, src, this));
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    if (dst == src) {
        if (SkIRect::IntersectsNoEmptyCheck(dstRect, srcRect)) {
            return false;
        }
    }

    GrGLIRect dstVP;
    GrGLIRect srcVP;
    this->bindSurfaceFBOForCopy(dst, GR_GL_DRAW_FRAMEBUFFER, &dstVP, kDst_TempFBOTarget);
    this->bindSurfaceFBOForCopy(src, GR_GL_READ_FRAMEBUFFER, &srcVP, kSrc_TempFBOTarget);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID = SK_InvalidUniqueID;
    GrGLIRect srcGLRect;
    GrGLIRect dstGLRect;
    srcGLRect.setRelativeTo(srcVP,
                            srcRect.fLeft,
                            srcRect.fTop,
                            srcRect.width(),
                            srcRect.height(),
                            src->origin());
    dstGLRect.setRelativeTo(dstVP,
                            dstRect.fLeft,
                            dstRect.fTop,
                            dstRect.width(),
                            dstRect.height(),
                            dst->origin());

    // BlitFrameBuffer respects the scissor, so disable it.
    this->disableScissor();

    GrGLint srcY0;
    GrGLint srcY1;
    // Does the blit need to y-mirror or not?
    if (src->origin() == dst->origin()) {
        srcY0 = srcGLRect.fBottom;
        srcY1 = srcGLRect.fBottom + srcGLRect.fHeight;
    } else {
        srcY0 = srcGLRect.fBottom + srcGLRect.fHeight;
        srcY1 = srcGLRect.fBottom;
    }
    GL_CALL(BlitFramebuffer(srcGLRect.fLeft,
                            srcY0,
                            srcGLRect.fLeft + srcGLRect.fWidth,
                            srcY1,
                            dstGLRect.fLeft,
                            dstGLRect.fBottom,
                            dstGLRect.fLeft + dstGLRect.fWidth,
                            dstGLRect.fBottom + dstGLRect.fHeight,
                            GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
    this->unbindTextureFBOForCopy(GR_GL_DRAW_FRAMEBUFFER, dst);
    this->unbindTextureFBOForCopy(GR_GL_READ_FRAMEBUFFER, src);
    return true;
}

void GrGLGpu::xferBarrier(GrRenderTarget* rt, GrXferBarrierType type) {
    SkASSERT(type);
    switch (type) {
        case kTexture_GrXferBarrierType: {
            GrGLRenderTarget* glrt = static_cast<GrGLRenderTarget*>(rt);
            if (glrt->textureFBOID() != glrt->renderFBOID()) {
                // The render target uses separate storage so no need for glTextureBarrier.
                // FIXME: The render target will resolve automatically when its texture is bound,
                // but we could resolve only the bounds that will be read if we do it here instead.
                return;
            }
            SkASSERT(this->caps()->textureBarrierSupport());
            GL_CALL(TextureBarrier());
            return;
        }
        case kBlend_GrXferBarrierType:
            SkASSERT(GrCaps::kAdvanced_BlendEquationSupport ==
                     this->caps()->blendEquationSupport());
            GL_CALL(BlendBarrier());
            return;
        default: break; // placate compiler warnings that kNone not handled
    }
}

GrBackendObject GrGLGpu::createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                         GrPixelConfig config) const {
    if (!this->caps()->isConfigTexturable(config)) {
        return false;
    }
    GrGLTextureInfo* info = new GrGLTextureInfo;
    info->fTarget = GR_GL_TEXTURE_2D;
    info->fID = 0;
    GL_CALL(GenTextures(1, &info->fID));
    GL_CALL(ActiveTexture(GR_GL_TEXTURE0));
    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(BindTexture(info->fTarget, info->fID));
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_WRAP_S, GR_GL_CLAMP_TO_EDGE));
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_WRAP_T, GR_GL_CLAMP_TO_EDGE));

    GrGLenum internalFormat;
    GrGLenum externalFormat;
    GrGLenum externalType;

    if (!this->glCaps().getTexImageFormats(config, config, &internalFormat, &externalFormat,
                                           &externalType)) {
        delete info;
#ifdef SK_IGNORE_GL_TEXTURE_TARGET
        return 0;
#else
        return reinterpret_cast<GrBackendObject>(nullptr);
#endif
    }

    GL_CALL(TexImage2D(info->fTarget, 0, internalFormat, w, h, 0, externalFormat,
                       externalType, pixels));

#ifdef SK_IGNORE_GL_TEXTURE_TARGET
    GrGLuint id = info->fID;
    delete info;
    return id;
#else
    return reinterpret_cast<GrBackendObject>(info);
#endif
}

bool GrGLGpu::isTestingOnlyBackendTexture(GrBackendObject id) const {
#ifdef SK_IGNORE_GL_TEXTURE_TARGET
    GrGLuint texID = (GrGLuint)id;
#else
    GrGLuint texID = reinterpret_cast<const GrGLTextureInfo*>(id)->fID;
#endif

    GrGLboolean result;
    GL_CALL_RET(result, IsTexture(texID));

    return (GR_GL_TRUE == result);
}

void GrGLGpu::deleteTestingOnlyBackendTexture(GrBackendObject id, bool abandonTexture) const {
#ifdef SK_IGNORE_GL_TEXTURE_TARGET
    GrGLuint texID = (GrGLuint)id;
#else
    const GrGLTextureInfo* info = reinterpret_cast<const GrGLTextureInfo*>(id);
    GrGLuint texID = info->fID;
#endif

    if (!abandonTexture) {
        GL_CALL(DeleteTextures(1, &texID));
    }

#ifndef SK_IGNORE_GL_TEXTURE_TARGET
    delete info;
#endif
}

void GrGLGpu::resetShaderCacheForTesting() const {
    fProgramCache->abandon();
}

///////////////////////////////////////////////////////////////////////////////
GrGLAttribArrayState* GrGLGpu::HWGeometryState::bindArrayAndBuffersToDraw(
                                                GrGLGpu* gpu,
                                                const GrGLVertexBuffer* vbuffer,
                                                const GrGLIndexBuffer* ibuffer) {
    SkASSERT(vbuffer);
    GrGLuint vbufferID = vbuffer->bufferID();
    GrGLuint* ibufferIDPtr = nullptr;
    GrGLuint ibufferID;
    if (ibuffer) {
        ibufferID = ibuffer->bufferID();
        ibufferIDPtr = &ibufferID;
    }
    return this->internalBind(gpu, vbufferID, ibufferIDPtr);
}

GrGLAttribArrayState* GrGLGpu::HWGeometryState::bindArrayAndBufferToDraw(GrGLGpu* gpu,
                                                                         GrGLuint vbufferID) {
    return this->internalBind(gpu, vbufferID, nullptr);
}

GrGLAttribArrayState* GrGLGpu::HWGeometryState::bindArrayAndBuffersToDraw(GrGLGpu* gpu,
                                                                          GrGLuint vbufferID,
                                                                          GrGLuint ibufferID) {
    return this->internalBind(gpu, vbufferID, &ibufferID);
}

GrGLAttribArrayState* GrGLGpu::HWGeometryState::internalBind(GrGLGpu* gpu,
                                                             GrGLuint vbufferID,
                                                             GrGLuint* ibufferID) {
    GrGLAttribArrayState* attribState;

    if (gpu->glCaps().isCoreProfile() && 0 != vbufferID) {
        if (!fVBOVertexArray) {
            GrGLuint arrayID;
            GR_GL_CALL(gpu->glInterface(), GenVertexArrays(1, &arrayID));
            int attrCount = gpu->glCaps().maxVertexAttributes();
            fVBOVertexArray = new GrGLVertexArray(arrayID, attrCount);
        }
        if (ibufferID) {
            attribState = fVBOVertexArray->bindWithIndexBuffer(gpu, *ibufferID);
        } else {
            attribState = fVBOVertexArray->bind(gpu);
        }
    } else {
        if (ibufferID) {
            this->setIndexBufferIDOnDefaultVertexArray(gpu, *ibufferID);
        } else {
            this->setVertexArrayID(gpu, 0);
        }
        int attrCount = gpu->glCaps().maxVertexAttributes();
        if (fDefaultVertexArrayAttribState.count() != attrCount) {
            fDefaultVertexArrayAttribState.resize(attrCount);
        }
        attribState = &fDefaultVertexArrayAttribState;
    }
    return attribState;
}
