/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGpu.h"

#include <cmath>
#include "../private/GrGLSL.h"
#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrFixedClip.h"
#include "GrGLBuffer.h"
#include "GrGLGpuCommandBuffer.h"
#include "GrGLSemaphore.h"
#include "GrGLStencilAttachment.h"
#include "GrGLTextureRenderTarget.h"
#include "GrGpuResourcePriv.h"
#include "GrMesh.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrShaderCaps.h"
#include "GrSurfacePriv.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexturePriv.h"
#include "GrTypes.h"
#include "SkAutoMalloc.h"
#include "SkMakeUnique.h"
#include "SkMipMap.h"
#include "SkPixmap.h"
#include "SkSLCompiler.h"
#include "SkStrokeRec.h"
#include "SkTemplates.h"
#include "SkTraceEvent.h"
#include "SkTypes.h"
#include "builders/GrGLShaderStringBuilder.h"
#include "instanced/GLInstancedRendering.h"

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

//#define USE_NSIGHT

///////////////////////////////////////////////////////////////////////////////

using gr_instanced::InstancedRendering;
using gr_instanced::GLInstancedRendering;

using gr_instanced::OpAllocator;
using gr_instanced::GLOpAllocator;

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
    sk_sp<const GrGLInterface> glInterface(
        reinterpret_cast<const GrGLInterface*>(backendContext));
    if (!glInterface) {
        glInterface.reset(GrGLDefaultInterface());
    } else {
        glInterface->ref();
    }
    if (!glInterface) {
        return nullptr;
    }
#ifdef USE_NSIGHT
    const_cast<GrContextOptions&>(options).fSuppressPathRendering = true;
#endif
    GrGLContext* glContext = GrGLContext::Create(glInterface.get(), options);
    if (glContext) {
        return new GrGLGpu(glContext, context);
    }
    return nullptr;
}

static bool gPrintStartupSpew;

GrGLGpu::GrGLGpu(GrGLContext* ctx, GrContext* context)
    : GrGpu(context)
    , fGLContext(ctx)
    , fProgramCache(new ProgramCache(this))
    , fHWProgramID(0)
    , fTempSrcFBOID(0)
    , fTempDstFBOID(0)
    , fStencilClearFBOID(0)
    , fHWMaxUsedBufferTextureUnit(-1)
    , fHWMinSampleShading(0.0) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(fCopyPrograms); ++i) {
        fCopyPrograms[i].fProgram = 0;
    }
    for (size_t i = 0; i < SK_ARRAY_COUNT(fMipmapPrograms); ++i) {
        fMipmapPrograms[i].fProgram = 0;
    }
    fStencilClipClearProgram = 0;

    SkASSERT(ctx);
    fCaps.reset(SkRef(ctx->caps()));

    fHWBoundTextureUniqueIDs.reset(this->caps()->shaderCaps()->maxCombinedSamplers());
    fHWBoundImageStorages.reset(this->caps()->shaderCaps()->maxCombinedImageStorages());

    fHWBufferState[kVertex_GrBufferType].fGLTarget = GR_GL_ARRAY_BUFFER;
    fHWBufferState[kIndex_GrBufferType].fGLTarget = GR_GL_ELEMENT_ARRAY_BUFFER;
    fHWBufferState[kTexel_GrBufferType].fGLTarget = GR_GL_TEXTURE_BUFFER;
    fHWBufferState[kDrawIndirect_GrBufferType].fGLTarget = GR_GL_DRAW_INDIRECT_BUFFER;
    if (GrGLCaps::kChromium_TransferBufferType == this->glCaps().transferBufferType()) {
        fHWBufferState[kXferCpuToGpu_GrBufferType].fGLTarget =
            GR_GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM;
        fHWBufferState[kXferGpuToCpu_GrBufferType].fGLTarget =
            GR_GL_PIXEL_PACK_TRANSFER_BUFFER_CHROMIUM;
    } else {
        fHWBufferState[kXferCpuToGpu_GrBufferType].fGLTarget = GR_GL_PIXEL_UNPACK_BUFFER;
        fHWBufferState[kXferGpuToCpu_GrBufferType].fGLTarget = GR_GL_PIXEL_PACK_BUFFER;
    }
    GR_STATIC_ASSERT(6 == SK_ARRAY_COUNT(fHWBufferState));

    if (this->caps()->shaderCaps()->texelBufferSupport()) {
        fHWBufferTextures.reset(this->caps()->shaderCaps()->maxCombinedSamplers());
    }

    if (this->glCaps().shaderCaps()->pathRenderingSupport()) {
        fPathRendering.reset(new GrGLPathRendering(this));
    }

    GrGLClearErr(this->glInterface());
    if (gPrintStartupSpew) {
        const GrGLubyte* vendor;
        const GrGLubyte* renderer;
        const GrGLubyte* version;
        const GrGLubyte* glslVersion;
        GL_CALL_RET(vendor, GetString(GR_GL_VENDOR));
        GL_CALL_RET(renderer, GetString(GR_GL_RENDERER));
        GL_CALL_RET(version, GetString(GR_GL_VERSION));
        GL_CALL_RET(glslVersion, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
        SkDebugf("------------------------- create GrGLGpu %p --------------\n",
                 this);
        SkDebugf("------ VENDOR %s\n", vendor);
        SkDebugf("------ RENDERER %s\n", renderer);
        SkDebugf("------ VERSION %s\n",  version);
        SkDebugf("------ SHADING LANGUAGE VERSION %s\n", glslVersion);
        SkDebugf("------ EXTENSIONS\n");
        this->glContext().extensions().print();
        SkDebugf("\n");
        SkDebugf("%s", this->glCaps().dump().c_str());
    }
}

GrGLGpu::~GrGLGpu() {
    // Ensure any GrGpuResource objects get deleted first, since they may require a working GrGLGpu
    // to release the resources held by the objects themselves.
    fPathRendering.reset();
    fCopyProgramArrayBuffer.reset();
    fMipmapProgramArrayBuffer.reset();
    fStencilClipClearArrayBuffer.reset();

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

    for (size_t i = 0; i < SK_ARRAY_COUNT(fMipmapPrograms); ++i) {
        if (0 != fMipmapPrograms[i].fProgram) {
            GL_CALL(DeleteProgram(fMipmapPrograms[i].fProgram));
        }
    }

    if (0 != fStencilClipClearProgram) {
        GL_CALL(DeleteProgram(fStencilClipClearProgram));
    }

    delete fProgramCache;
}

void GrGLGpu::disconnect(DisconnectType type) {
    INHERITED::disconnect(type);
    if (DisconnectType::kCleanup == type) {
        if (fHWProgramID) {
            GL_CALL(UseProgram(0));
        }
        if (fTempSrcFBOID) {
            GL_CALL(DeleteFramebuffers(1, &fTempSrcFBOID));
        }
        if (fTempDstFBOID) {
            GL_CALL(DeleteFramebuffers(1, &fTempDstFBOID));
        }
        if (fStencilClearFBOID) {
            GL_CALL(DeleteFramebuffers(1, &fStencilClearFBOID));
        }
        for (size_t i = 0; i < SK_ARRAY_COUNT(fCopyPrograms); ++i) {
            if (fCopyPrograms[i].fProgram) {
                GL_CALL(DeleteProgram(fCopyPrograms[i].fProgram));
            }
        }
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMipmapPrograms); ++i) {
            if (fMipmapPrograms[i].fProgram) {
                GL_CALL(DeleteProgram(fMipmapPrograms[i].fProgram));
            }
        }
        if (fStencilClipClearProgram) {
            GL_CALL(DeleteProgram(fStencilClipClearProgram));
        }
    } else {
        if (fProgramCache) {
            fProgramCache->abandon();
        }
    }

    delete fProgramCache;
    fProgramCache = nullptr;

    fHWProgramID = 0;
    fTempSrcFBOID = 0;
    fTempDstFBOID = 0;
    fStencilClearFBOID = 0;
    fCopyProgramArrayBuffer.reset();
    for (size_t i = 0; i < SK_ARRAY_COUNT(fCopyPrograms); ++i) {
        fCopyPrograms[i].fProgram = 0;
    }
    fMipmapProgramArrayBuffer.reset();
    for (size_t i = 0; i < SK_ARRAY_COUNT(fMipmapPrograms); ++i) {
        fMipmapPrograms[i].fProgram = 0;
    }
    fStencilClipClearProgram = 0;
    fStencilClipClearArrayBuffer.reset();
    if (this->glCaps().shaderCaps()->pathRenderingSupport()) {
        this->glPathRendering()->disconnect(type);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrGLGpu::onResetContext(uint32_t resetBits) {
    if (resetBits & kMisc_GrGLBackendState) {
        // we don't use the zb at all
        GL_CALL(Disable(GR_GL_DEPTH_TEST));
        GL_CALL(DepthMask(GR_GL_FALSE));

        // We don't use face culling.
        GL_CALL(Disable(GR_GL_CULL_FACE));
        // We do use separate stencil. Our algorithms don't care which face is front vs. back so
        // just set this to the default for self-consistency.
        GL_CALL(FrontFace(GR_GL_CCW));

        fHWBufferState[kTexel_GrBufferType].invalidate();
        fHWBufferState[kDrawIndirect_GrBufferType].invalidate();
        fHWBufferState[kXferCpuToGpu_GrBufferType].invalidate();
        fHWBufferState[kXferGpuToCpu_GrBufferType].invalidate();

        if (kGL_GrGLStandard == this->glStandard()) {
#ifndef USE_NSIGHT
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

            if (this->caps()->wireframeMode()) {
                GL_CALL(PolygonMode(GR_GL_FRONT_AND_BACK, GR_GL_LINE));
            } else {
                GL_CALL(PolygonMode(GR_GL_FRONT_AND_BACK, GR_GL_FILL));
            }
#endif
            // Since ES doesn't support glPointSize at all we always use the VS to
            // set the point size
            GL_CALL(Enable(GR_GL_VERTEX_PROGRAM_POINT_SIZE));

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

        if (this->caps()->usesMixedSamples()) {
            if (0 != this->caps()->maxRasterSamples()) {
                fHWRasterMultisampleEnabled = kUnknown_TriState;
                fHWNumRasterSamples = 0;
            }

            // The skia blend modes all use premultiplied alpha and therefore expect RGBA coverage
            // modulation. This state has no effect when not rendering to a mixed sampled target.
            GL_CALL(CoverageModulation(GR_GL_RGBA));
        }
    }

    fHWActiveTextureUnitIdx = -1; // invalid
    fLastPrimitiveType = static_cast<GrPrimitiveType>(-1);

    if (resetBits & kTextureBinding_GrGLBackendState) {
        for (int s = 0; s < fHWBoundTextureUniqueIDs.count(); ++s) {
            fHWBoundTextureUniqueIDs[s].makeInvalid();
        }
        for (int b = 0; b < fHWBufferTextures.count(); ++b) {
            SkASSERT(this->caps()->shaderCaps()->texelBufferSupport());
            fHWBufferTextures[b].fKnownBound = false;
        }
        for (int i = 0; i < fHWBoundImageStorages.count(); ++i) {
            SkASSERT(this->caps()->shaderCaps()->imageLoadStoreSupport());
            fHWBoundImageStorages[i].fTextureUniqueID.makeInvalid();
        }
    }

    if (resetBits & kBlend_GrGLBackendState) {
        fHWBlendState.invalidate();
    }

    if (resetBits & kView_GrGLBackendState) {
        fHWScissorSettings.invalidate();
        fHWWindowRectsState.invalidate();
        fHWViewport.invalidate();
    }

    if (resetBits & kStencil_GrGLBackendState) {
        fHWStencilSettings.invalidate();
        fHWStencilTestEnabled = kUnknown_TriState;
    }

    // Vertex
    if (resetBits & kVertex_GrGLBackendState) {
        fHWVertexArrayState.invalidate();
        fHWBufferState[kVertex_GrBufferType].invalidate();
        fHWBufferState[kIndex_GrBufferType].invalidate();
    }

    if (resetBits & kRenderTarget_GrGLBackendState) {
        fHWBoundRenderTargetUniqueID.makeInvalid();
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

static bool check_backend_texture(const GrBackendTexture& backendTex, const GrGLCaps& caps,
                                  GrGLTexture::IDDesc* idDesc) {
    const GrGLTextureInfo* info = backendTex.getGLTextureInfo();
    if (!info || !info->fID) {
        return false;
    }

    idDesc->fInfo = *info;

    if (GR_GL_TEXTURE_EXTERNAL == idDesc->fInfo.fTarget) {
        if (!caps.shaderCaps()->externalTextureSupport()) {
            return false;
        }
    } else if (GR_GL_TEXTURE_RECTANGLE == idDesc->fInfo.fTarget) {
        if (!caps.rectangleTextureSupport()) {
            return false;
        }
    } else if (GR_GL_TEXTURE_2D != idDesc->fInfo.fTarget) {
        return false;
    }
    return true;
}

sk_sp<GrTexture> GrGLGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                               GrSurfaceOrigin origin,
                                               GrWrapOwnership ownership) {
    GrGLTexture::IDDesc idDesc;
    if (!check_backend_texture(backendTex, this->glCaps(), &idDesc)) {
        return nullptr;
    }
    if (kBorrow_GrWrapOwnership == ownership) {
        idDesc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    } else {
        idDesc.fOwnership = GrBackendObjectOwnership::kOwned;
    }

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kNone_GrSurfaceFlags;
    surfDesc.fWidth = backendTex.width();
    surfDesc.fHeight = backendTex.height();
    surfDesc.fConfig = backendTex.config();
    surfDesc.fSampleCnt = 0;
    // FIXME:  this should be calling resolve_origin(), but Chrome code is currently
    // assuming the old behaviour, which is that backend textures are always
    // BottomLeft, even for non-RT's.  Once Chrome is fixed, change this to:
    // glTexDesc.fOrigin = resolve_origin(desc.fOrigin, renderTarget);
    if (kDefault_GrSurfaceOrigin == origin) {
        surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    } else {
        surfDesc.fOrigin = origin;
    }
    return GrGLTexture::MakeWrapped(this, surfDesc, idDesc);
}

sk_sp<GrTexture> GrGLGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                         GrSurfaceOrigin origin,
                                                         int sampleCnt,
                                                         GrWrapOwnership ownership) {
    GrGLTexture::IDDesc idDesc;
    if (!check_backend_texture(backendTex, this->glCaps(), &idDesc)) {
        return nullptr;
    }

    // We don't support rendering to a EXTERNAL texture.
    if (GR_GL_TEXTURE_EXTERNAL == idDesc.fInfo.fTarget) {
        return nullptr;
    }

    if (kBorrow_GrWrapOwnership == ownership) {
        idDesc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    } else {
        idDesc.fOwnership = GrBackendObjectOwnership::kOwned;
    }

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    surfDesc.fWidth = backendTex.width();
    surfDesc.fHeight = backendTex.height();
    surfDesc.fConfig = backendTex.config();
    surfDesc.fSampleCnt = this->caps()->getSampleCount(sampleCnt, backendTex.config());
    // FIXME:  this should be calling resolve_origin(), but Chrome code is currently
    // assuming the old behaviour, which is that backend textures are always
    // BottomLeft, even for non-RT's.  Once Chrome is fixed, change this to:
    // glTexDesc.fOrigin = resolve_origin(desc.fOrigin, renderTarget);
    if (kDefault_GrSurfaceOrigin == origin) {
        surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    } else {
        surfDesc.fOrigin = origin;
    }

    GrGLRenderTarget::IDDesc rtIDDesc;
    if (!this->createRenderTargetObjects(surfDesc, idDesc.fInfo, &rtIDDesc)) {
        return nullptr;
    }
    sk_sp<GrGLTextureRenderTarget> texRT(
            GrGLTextureRenderTarget::MakeWrapped(this, surfDesc, idDesc, rtIDDesc));
    texRT->baseLevelWasBoundToFBO();
    return std::move(texRT);
}

sk_sp<GrRenderTarget> GrGLGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT,
                                                         GrSurfaceOrigin origin) {
    const GrGLFramebufferInfo* info = backendRT.getGLFramebufferInfo();
    if (!info) {
        return nullptr;
    }

    GrGLRenderTarget::IDDesc idDesc;
    idDesc.fRTFBOID = info->fFBOID;
    idDesc.fMSColorRenderbufferID = 0;
    idDesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    idDesc.fRTFBOOwnership = GrBackendObjectOwnership::kBorrowed;
    idDesc.fIsMixedSampled = false;

    GrSurfaceDesc desc;
    desc.fConfig = backendRT.config();
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = backendRT.width();
    desc.fHeight = backendRT.height();
    desc.fSampleCnt = this->caps()->getSampleCount(backendRT.sampleCnt(), backendRT.config());
    SkASSERT(kDefault_GrSurfaceOrigin != origin);
    desc.fOrigin = origin;

    return GrGLRenderTarget::MakeWrapped(this, desc, idDesc, backendRT.stencilBits());
}

sk_sp<GrRenderTarget> GrGLGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                  GrSurfaceOrigin origin,
                                                                  int sampleCnt) {
    const GrGLTextureInfo* info = tex.getGLTextureInfo();
    if (!info || !info->fID) {
        return nullptr;
    }

    GrGLTextureInfo texInfo;
    texInfo = *info;

    if (GR_GL_TEXTURE_RECTANGLE != texInfo.fTarget &&
        GR_GL_TEXTURE_2D != texInfo.fTarget) {
        // Only texture rectangle and texture 2d are supported. We do not check whether texture
        // rectangle is supported by Skia - if the caller provided us with a texture rectangle,
        // we assume the necessary support exists.
        return nullptr;
    }

    GrSurfaceDesc surfDesc;
    surfDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    surfDesc.fWidth = tex.width();
    surfDesc.fHeight = tex.height();
    surfDesc.fConfig = tex.config();
    surfDesc.fSampleCnt = this->caps()->getSampleCount(sampleCnt, tex.config());
    // FIXME:  this should be calling resolve_origin(), but Chrome code is currently
    // assuming the old behaviour, which is that backend textures are always
    // BottomLeft, even for non-RT's.  Once Chrome is fixed, change this to:
    // glTexDesc.fOrigin = resolve_origin(desc.fOrigin, renderTarget);
    if (kDefault_GrSurfaceOrigin == origin) {
        surfDesc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    } else {
        surfDesc.fOrigin = origin;
    }

    GrGLRenderTarget::IDDesc rtIDDesc;
    if (!this->createRenderTargetObjects(surfDesc, texInfo, &rtIDDesc)) {
        return nullptr;
    }
    return GrGLRenderTarget::MakeWrapped(this, surfDesc, rtIDDesc, 0);
}

////////////////////////////////////////////////////////////////////////////////

bool GrGLGpu::onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                                   GrPixelConfig srcConfig,
                                   DrawPreference* drawPreference,
                                   WritePixelTempDrawInfo* tempDrawInfo) {
    if (SkToBool(dstSurface->asRenderTarget())) {
        if (this->glCaps().useDrawInsteadOfAllRenderTargetWrites()) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        }
    }

    GrGLTexture* texture = static_cast<GrGLTexture*>(dstSurface->asTexture());

    if (texture) {
        if (GR_GL_TEXTURE_EXTERNAL == texture->target()) {
             // We don't currently support writing pixels to EXTERNAL textures.
             return false;
        }
        if (GrPixelConfigIsUnorm(texture->config()) && texture->hasBaseLevelBeenBoundToFBO() &&
            this->glCaps().disallowTexSubImageForUnormConfigTexturesEverBoundToFBO() &&
            (width < dstSurface->width() || height < dstSurface->height())) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        }
    } else {
        // This subclass only allows writes to textures. If the dst is not a texture we have to draw
        // into it. We could use glDrawPixels on GLs that have it, but we don't today.
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    // If the dst is MSAA, we have to draw, or we'll just be writing to the resolve target.
    if (dstSurface->asRenderTarget() && dstSurface->asRenderTarget()->numColorSamples() > 0) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    if (GrPixelConfigIsSRGB(dstSurface->config()) != GrPixelConfigIsSRGB(srcConfig)) {
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
    }

    // Start off assuming no swizzling
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();
    tempDrawInfo->fWriteConfig = srcConfig;

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
            tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
            tempDrawInfo->fWriteConfig = dstSurface->config();
        } else if (this->glCaps().rgba8888PixelsOpsAreSlow() &&
                   kRGBA_8888_GrPixelConfig == srcConfig) {
            ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
            tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
            tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
            tempDrawInfo->fWriteConfig = dstSurface->config();
        } else if (kGLES_GrGLStandard == this->glStandard() &&
                   this->glCaps().bgraIsInternalFormat()) {
            // The internal format and external formats must match texture uploads so we can't
            // swizzle while uploading when BGRA is a distinct internal format.
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
            tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
            tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
            tempDrawInfo->fWriteConfig = dstSurface->config();
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
                            GrPixelConfig config,
                            const GrMipLevel texels[],
                            int mipLevelCount) {
    GrGLTexture* glTex = static_cast<GrGLTexture*>(surface->asTexture());

    if (!check_write_and_transfer_input(glTex, surface, config)) {
        return false;
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(glTex->target(), glTex->textureID()));

    return this->uploadTexData(glTex->config(), glTex->width(), glTex->height(),
                               glTex->origin(), glTex->target(), kWrite_UploadType,
                               left, top, width, height, config, texels, mipLevelCount);
}

// For GL_[UN]PACK_ALIGNMENT.
static inline GrGLint config_alignment(GrPixelConfig config) {
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kGray_8_GrPixelConfig:
            return 1;
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
            return 2;
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_8888_sint_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
            return 4;
        case kUnknown_GrPixelConfig:
            return 0;
    }
    SkFAIL("Invalid pixel config");
    return 0;
}

bool GrGLGpu::onTransferPixels(GrTexture* texture,
                               int left, int top, int width, int height,
                               GrPixelConfig config, GrBuffer* transferBuffer,
                               size_t offset, size_t rowBytes) {
    GrGLTexture* glTex = static_cast<GrGLTexture*>(texture);
    GrPixelConfig texConfig = glTex->config();
    SkASSERT(this->caps()->isConfigTexturable(texConfig));

    if (!check_write_and_transfer_input(glTex, texture, config)) {
        return false;
    }

    if (width <= 0 || width > SK_MaxS32 || height <= 0 || height > SK_MaxS32) {
        return false;
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(glTex->target(), glTex->textureID()));

    SkASSERT(!transferBuffer->isMapped());
    SkASSERT(!transferBuffer->isCPUBacked());
    const GrGLBuffer* glBuffer = static_cast<const GrGLBuffer*>(transferBuffer);
    this->bindBuffer(kXferCpuToGpu_GrBufferType, glBuffer);

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texture->width(), texture->height());
        SkASSERT(bounds.contains(subRect));
    )

    size_t bpp = GrBytesPerPixel(config);
    const size_t trimRowBytes = width * bpp;
    if (!rowBytes) {
        rowBytes = trimRowBytes;
    }
    const void* pixels = (void*)offset;
    if (width < 0 || height < 0) {
        return false;
    }

    bool restoreGLRowLength = false;
    if (trimRowBytes != rowBytes) {
        // we should have checked for this support already
        SkASSERT(this->glCaps().unpackRowLengthSupport());
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowBytes / bpp));
        restoreGLRowLength = true;
    }

    // Internal format comes from the texture desc.
    GrGLenum internalFormat;
    // External format and type come from the upload data.
    GrGLenum externalFormat;
    GrGLenum externalType;
    if (!this->glCaps().getTexImageFormats(texConfig, config, &internalFormat,
                                           &externalFormat, &externalType)) {
        return false;
    }

    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, config_alignment(texConfig)));
    GL_CALL(TexSubImage2D(glTex->target(),
                          0,
                          left, top,
                          width,
                          height,
                          externalFormat, externalType,
                          pixels));

    if (restoreGLRowLength) {
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }

    return true;
}

/**
 * Creates storage space for the texture and fills it with texels.
 *
 * @param config         Pixel config of the texture.
 * @param interface      The GL interface in use.
 * @param caps           The capabilities of the GL device.
 * @param internalFormat The data format used for the internal storage of the texture. May be sized.
 * @param internalFormatForTexStorage The data format used for the TexStorage API. Must be sized.
 * @param externalFormat The data format used for the external storage of the texture.
 * @param externalType   The type of the data used for the external storage of the texture.
 * @param texels         The texel data of the texture being created.
 * @param baseWidth      The width of the texture's base mipmap level
 * @param baseHeight     The height of the texture's base mipmap level
 */
static bool allocate_and_populate_texture(GrPixelConfig config,
                                          const GrGLInterface& interface,
                                          const GrGLCaps& caps,
                                          GrGLenum target,
                                          GrGLenum internalFormat,
                                          GrGLenum internalFormatForTexStorage,
                                          GrGLenum externalFormat,
                                          GrGLenum externalType,
                                          const GrMipLevel texels[], int mipLevelCount,
                                          int baseWidth, int baseHeight) {
    CLEAR_ERROR_BEFORE_ALLOC(&interface);

    bool useTexStorage = caps.isConfigTexSupportEnabled(config);
    // We can only use TexStorage if we know we will not later change the storage requirements.
    // This means if we may later want to add mipmaps, we cannot use TexStorage.
    // Right now, we cannot know if we will later add mipmaps or not.
    // The only time we can use TexStorage is when we already have the
    // mipmaps or are using a format incompatible with MIP maps.
    useTexStorage &= mipLevelCount > 1 || GrPixelConfigIsSint(config);

    if (useTexStorage) {
        // We never resize or change formats of textures.
        GL_ALLOC_CALL(&interface,
                      TexStorage2D(target, SkTMax(mipLevelCount, 1), internalFormatForTexStorage,
                                   baseWidth, baseHeight));
        GrGLenum error = CHECK_ALLOC_ERROR(&interface);
        if (error != GR_GL_NO_ERROR) {
            return  false;
        } else {
            for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
                const void* currentMipData = texels[currentMipLevel].fPixels;
                if (currentMipData == nullptr) {
                    continue;
                }
                int twoToTheMipLevel = 1 << currentMipLevel;
                int currentWidth = SkTMax(1, baseWidth / twoToTheMipLevel);
                int currentHeight = SkTMax(1, baseHeight / twoToTheMipLevel);

                GR_GL_CALL(&interface,
                           TexSubImage2D(target,
                                         currentMipLevel,
                                         0, // left
                                         0, // top
                                         currentWidth,
                                         currentHeight,
                                         externalFormat, externalType,
                                         currentMipData));
            }
            return true;
        }
    } else {
        if (!mipLevelCount) {
            GL_ALLOC_CALL(&interface,
                          TexImage2D(target,
                                     0,
                                     internalFormat,
                                     baseWidth,
                                     baseHeight,
                                     0, // border
                                     externalFormat, externalType,
                                     nullptr));
            GrGLenum error = CHECK_ALLOC_ERROR(&interface);
            if (error != GR_GL_NO_ERROR) {
                return false;
            }
        } else {
            for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
                int twoToTheMipLevel = 1 << currentMipLevel;
                int currentWidth = SkTMax(1, baseWidth / twoToTheMipLevel);
                int currentHeight = SkTMax(1, baseHeight / twoToTheMipLevel);
                const void* currentMipData = texels[currentMipLevel].fPixels;
                // Even if curremtMipData is nullptr, continue to call TexImage2D.
                // This will allocate texture memory which we can later populate.
                GL_ALLOC_CALL(&interface,
                              TexImage2D(target,
                                         currentMipLevel,
                                         internalFormat,
                                         currentWidth,
                                         currentHeight,
                                         0, // border
                                         externalFormat, externalType,
                                         currentMipData));
                GrGLenum error = CHECK_ALLOC_ERROR(&interface);
                if (error != GR_GL_NO_ERROR) {
                    return false;
                }
            }
        }
    }
    return true;
}

/**
 * After a texture is created, any state which was altered during its creation
 * needs to be restored.
 *
 * @param interface          The GL interface to use.
 * @param caps               The capabilities of the GL device.
 * @param restoreGLRowLength Should the row length unpacking be restored?
 * @param glFlipY            Did GL flip the texture vertically?
 */
static void restore_pixelstore_state(const GrGLInterface& interface, const GrGLCaps& caps,
                                     bool restoreGLRowLength, bool glFlipY) {
    if (restoreGLRowLength) {
        SkASSERT(caps.unpackRowLengthSupport());
        GR_GL_CALL(&interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
    if (glFlipY) {
        GR_GL_CALL(&interface, PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_FALSE));
    }
}

bool GrGLGpu::uploadTexData(GrPixelConfig texConfig, int texWidth, int texHeight,
                            GrSurfaceOrigin texOrigin, GrGLenum target, UploadType uploadType,
                            int left, int top, int width, int height, GrPixelConfig dataConfig,
                            const GrMipLevel texels[], int mipLevelCount) {
    SkASSERT(this->caps()->isConfigTexturable(texConfig));
    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texWidth, texHeight);
        SkASSERT(bounds.contains(subRect));
    )
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == texWidth && height == texHeight));

    // unbind any previous transfer buffer
    auto& xferBufferState = fHWBufferState[kXferCpuToGpu_GrBufferType];
    if (!xferBufferState.fBoundBufferUniqueID.isInvalid()) {
        GL_CALL(BindBuffer(xferBufferState.fGLTarget, 0));
        xferBufferState.invalidate();
    }

    // texels is const.
    // But we may need to flip the texture vertically to prepare it.
    // Rather than flip in place and alter the incoming data,
    // we allocate a new buffer to flip into.
    // This means we need to make a non-const shallow copy of texels.
    SkAutoTMalloc<GrMipLevel> texelsShallowCopy;

    if (mipLevelCount) {
        texelsShallowCopy.reset(mipLevelCount);
        memcpy(texelsShallowCopy.get(), texels, mipLevelCount*sizeof(GrMipLevel));
    }

    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        SkASSERT(texelsShallowCopy[currentMipLevel].fPixels);
    }

    const GrGLInterface* interface = this->glInterface();
    const GrGLCaps& caps = this->glCaps();

    size_t bpp = GrBytesPerPixel(dataConfig);

    if (width == 0 || height == 0) {
        return false;
    }

    // Internal format comes from the texture desc.
    GrGLenum internalFormat;
    // External format and type come from the upload data.
    GrGLenum externalFormat;
    GrGLenum externalType;
    if (!this->glCaps().getTexImageFormats(texConfig, dataConfig, &internalFormat, &externalFormat,
                                           &externalType)) {
        return false;
    }
    // TexStorage requires a sized format, and internalFormat may or may not be
    GrGLenum internalFormatForTexStorage = this->glCaps().configSizedInternalFormat(texConfig);

    /*
     *  Check whether to allocate a temporary buffer for flipping y or
     *  because our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL ES may not let us specify
     *  GL_UNPACK_ROW_LENGTH.
     */
    bool restoreGLRowLength = false;
    bool swFlipY = false;
    bool glFlipY = false;

    if (kBottomLeft_GrSurfaceOrigin == texOrigin && mipLevelCount) {
        if (caps.unpackFlipYSupport()) {
            glFlipY = true;
        } else {
            swFlipY = true;
        }
    }

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    // find the combined size of all the mip levels and the relative offset of
    // each into the collective buffer
    size_t combined_buffer_size = 0;
    SkTArray<size_t> individual_mip_offsets(mipLevelCount);
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        int twoToTheMipLevel = 1 << currentMipLevel;
        int currentWidth = SkTMax(1, width / twoToTheMipLevel);
        int currentHeight = SkTMax(1, height / twoToTheMipLevel);
        const size_t trimmedSize = currentWidth * bpp * currentHeight;
        individual_mip_offsets.push_back(combined_buffer_size);
        combined_buffer_size += trimmedSize;
    }
    char* buffer = (char*)tempStorage.reset(combined_buffer_size);

    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        int twoToTheMipLevel = 1 << currentMipLevel;
        int currentWidth = SkTMax(1, width / twoToTheMipLevel);
        int currentHeight = SkTMax(1, height / twoToTheMipLevel);
        const size_t trimRowBytes = currentWidth * bpp;

        /*
         *  check whether to allocate a temporary buffer for flipping y or
         *  because our srcData has extra bytes past each row. If so, we need
         *  to trim those off here, since GL ES may not let us specify
         *  GL_UNPACK_ROW_LENGTH.
         */
        restoreGLRowLength = false;

        const size_t rowBytes = texelsShallowCopy[currentMipLevel].fRowBytes ?
                                texelsShallowCopy[currentMipLevel].fRowBytes :
                                trimRowBytes;

        // TODO: This optimization should be enabled with or without mips.
        // For use with mips, we must set GR_GL_UNPACK_ROW_LENGTH once per
        // mip level, before calling glTexImage2D.
        const bool usesMips = mipLevelCount > 1;
        if (caps.unpackRowLengthSupport() && !swFlipY && !usesMips) {
            // can't use this for flipping, only non-neg values allowed. :(
            if (rowBytes != trimRowBytes) {
                GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
                GR_GL_CALL(interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
                restoreGLRowLength = true;
            }
        } else if (trimRowBytes != rowBytes || swFlipY) {
            // copy data into our new storage, skipping the trailing bytes
            const char* src = (const char*)texelsShallowCopy[currentMipLevel].fPixels;
            if (swFlipY && currentHeight >= 1) {
                src += (currentHeight - 1) * rowBytes;
            }
            char* dst = buffer + individual_mip_offsets[currentMipLevel];
            for (int y = 0; y < currentHeight; y++) {
                memcpy(dst, src, trimRowBytes);
                if (swFlipY) {
                    src -= rowBytes;
                } else {
                    src += rowBytes;
                }
                dst += trimRowBytes;
            }
            // now point data to our copied version
            texelsShallowCopy[currentMipLevel].fPixels = buffer +
                individual_mip_offsets[currentMipLevel];
            texelsShallowCopy[currentMipLevel].fRowBytes = trimRowBytes;
        }
    }

    if (mipLevelCount) {
        if (glFlipY) {
            GR_GL_CALL(interface, PixelStorei(GR_GL_UNPACK_FLIP_Y, GR_GL_TRUE));
        }
        GR_GL_CALL(interface, PixelStorei(GR_GL_UNPACK_ALIGNMENT, config_alignment(texConfig)));
    }

    bool succeeded = true;
    if (kNewTexture_UploadType == uploadType) {
        if (0 == left && 0 == top && texWidth == width && texHeight == height) {
            succeeded = allocate_and_populate_texture(
                    texConfig, *interface, caps, target, internalFormat,
                    internalFormatForTexStorage, externalFormat, externalType,
                    texelsShallowCopy, mipLevelCount, width, height);
        } else {
            succeeded = false;
        }
    } else {
        if (swFlipY || glFlipY) {
            top = texHeight - (top + height);
        }
        for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
            int twoToTheMipLevel = 1 << currentMipLevel;
            int currentWidth = SkTMax(1, width / twoToTheMipLevel);
            int currentHeight = SkTMax(1, height / twoToTheMipLevel);

            GL_CALL(TexSubImage2D(target,
                                  currentMipLevel,
                                  left, top,
                                  currentWidth,
                                  currentHeight,
                                  externalFormat, externalType,
                                  texelsShallowCopy[currentMipLevel].fPixels));
        }
    }

    restore_pixelstore_state(*interface, caps, restoreGLRowLength, glFlipY);

    return succeeded;
}

static bool renderbuffer_storage_msaa(const GrGLContext& ctx,
                                      int sampleCount,
                                      GrGLenum format,
                                      int width, int height) {
    CLEAR_ERROR_BEFORE_ALLOC(ctx.interface());
    SkASSERT(GrGLCaps::kNone_MSFBOType != ctx.caps()->msFBOType());
    switch (ctx.caps()->msFBOType()) {
        case GrGLCaps::kStandard_MSFBOType:
        case GrGLCaps::kMixedSamples_MSFBOType:
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
                                        const GrGLTextureInfo& texInfo,
                                        GrGLRenderTarget::IDDesc* idDesc) {
    idDesc->fMSColorRenderbufferID = 0;
    idDesc->fRTFBOID = 0;
    idDesc->fRTFBOOwnership = GrBackendObjectOwnership::kOwned;
    idDesc->fTexFBOID = 0;
    SkASSERT((GrGLCaps::kMixedSamples_MSFBOType == this->glCaps().msFBOType()) ==
             this->caps()->usesMixedSamples());
    idDesc->fIsMixedSampled = desc.fSampleCnt > 0 && this->caps()->usesMixedSamples();

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
    fHWBoundRenderTargetUniqueID.makeInvalid();
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
        if (!this->glCaps().isConfigVerifiedColorAttachment(desc.fConfig)) {
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
    if (!this->glCaps().isConfigVerifiedColorAttachment(desc.fConfig)) {
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
static sk_sp<GrTexture> return_null_texture() {
//    SkDEBUGFAIL("null texture");
    return nullptr;
}

#if 0 && defined(SK_DEBUG)
static size_t as_size_t(int x) {
    return x;
}
#endif

static void set_initial_texture_params(const GrGLInterface* interface,
                                       const GrGLTextureInfo& info,
                                       GrGLTexture::TexParams* initialTexParams) {
    // Some drivers like to know filter/wrap before seeing glTexImage2D. Some
    // drivers have a bug where an FBO won't be complete if it includes a
    // texture that is not mipmap complete (considering the filter in use).
    // we only set a subset here so invalidate first
    initialTexParams->invalidate();
    initialTexParams->fMinFilter = GR_GL_NEAREST;
    initialTexParams->fMagFilter = GR_GL_NEAREST;
    initialTexParams->fWrapS = GR_GL_CLAMP_TO_EDGE;
    initialTexParams->fWrapT = GR_GL_CLAMP_TO_EDGE;
    GR_GL_CALL(interface, TexParameteri(info.fTarget,
                                        GR_GL_TEXTURE_MAG_FILTER,
                                        initialTexParams->fMagFilter));
    GR_GL_CALL(interface, TexParameteri(info.fTarget,
                                        GR_GL_TEXTURE_MIN_FILTER,
                                        initialTexParams->fMinFilter));
    GR_GL_CALL(interface, TexParameteri(info.fTarget,
                                        GR_GL_TEXTURE_WRAP_S,
                                        initialTexParams->fWrapS));
    GR_GL_CALL(interface, TexParameteri(info.fTarget,
                                        GR_GL_TEXTURE_WRAP_T,
                                        initialTexParams->fWrapT));
}

sk_sp<GrTexture> GrGLGpu::onCreateTexture(const GrSurfaceDesc& desc,
                                          SkBudgeted budgeted,
                                          const GrMipLevel texels[],
                                          int mipLevelCount) {
    // We fail if the MSAA was requested and is not available.
    if (GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType() && desc.fSampleCnt) {
        //SkDebugf("MSAA RT requested but not supported on this platform.");
        return return_null_texture();
    }

    bool performClear = (desc.fFlags & kPerformInitialClear_GrSurfaceFlag);

    GrMipLevel zeroLevel;
    std::unique_ptr<uint8_t[]> zeros;
    // TODO: remove the GrPixelConfigIsSint test. This is here because we have yet to add support
    // for glClearBuffer* which must be used instead of glClearColor/glClear for integer FBO
    // attachments.
    if (performClear && !this->glCaps().clearTextureSupport() &&
        (!this->glCaps().canConfigBeFBOColorAttachment(desc.fConfig) ||
         GrPixelConfigIsSint(desc.fConfig))) {
        size_t rowSize = GrBytesPerPixel(desc.fConfig) * desc.fWidth;
        size_t size = rowSize * desc.fHeight;
        zeros.reset(new uint8_t[size]);
        memset(zeros.get(), 0, size);
        zeroLevel.fPixels = zeros.get();
        zeroLevel.fRowBytes = 0;
        texels = &zeroLevel;
        mipLevelCount = 1;
        performClear = false;
    }

    bool isRenderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    GrGLTexture::IDDesc idDesc;
    idDesc.fOwnership = GrBackendObjectOwnership::kOwned;
    GrGLTexture::TexParams initialTexParams;
    if (!this->createTextureImpl(desc, &idDesc.fInfo, isRenderTarget, &initialTexParams,
                                 texels, mipLevelCount)) {
        return return_null_texture();
    }

    bool wasMipMapDataProvided = false;
    if (mipLevelCount > 1) {
        wasMipMapDataProvided = true;
    }

    sk_sp<GrGLTexture> tex;
    if (isRenderTarget) {
        // unbind the texture from the texture unit before binding it to the frame buffer
        GL_CALL(BindTexture(idDesc.fInfo.fTarget, 0));
        GrGLRenderTarget::IDDesc rtIDDesc;

        if (!this->createRenderTargetObjects(desc, idDesc.fInfo, &rtIDDesc)) {
            GL_CALL(DeleteTextures(1, &idDesc.fInfo.fID));
            return return_null_texture();
        }
        tex = sk_make_sp<GrGLTextureRenderTarget>(this, budgeted, desc, idDesc, rtIDDesc,
                                                  wasMipMapDataProvided);
        tex->baseLevelWasBoundToFBO();
    } else {
        tex = sk_make_sp<GrGLTexture>(this, budgeted, desc, idDesc, wasMipMapDataProvided);
    }
    tex->setCachedTexParams(initialTexParams, this->getResetTimestamp());
#ifdef TRACE_TEXTURE_CREATION
    SkDebugf("--- new texture [%d] size=(%d %d) config=%d\n",
             idDesc.fInfo.fID, desc.fWidth, desc.fHeight, desc.fConfig);
#endif
    if (tex && performClear) {
        if (this->glCaps().clearTextureSupport()) {
            GrGLenum format = GrPixelConfigIsSint(tex->config()) ? GR_GL_RGBA_INTEGER : GR_GL_RGBA;
            static constexpr uint32_t kZero = 0;
            GL_CALL(ClearTexImage(tex->textureID(), 0, format, GR_GL_UNSIGNED_BYTE, &kZero));
        } else {
            SkASSERT(!GrPixelConfigIsSint(desc.fConfig));
            GrGLIRect viewport;
            this->bindSurfaceFBOForPixelOps(tex.get(), GR_GL_FRAMEBUFFER, &viewport,
                                            kDst_TempFBOTarget);
            this->disableScissor();
            this->disableWindowRectangles();
            GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
            fHWWriteToColor = kYes_TriState;
            GL_CALL(ClearColor(0, 0, 0, 0));
            GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
            this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, tex.get());
            fHWBoundRenderTargetUniqueID.makeInvalid();
        }
    }
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
        fHWBoundRenderTargetUniqueID.makeInvalid();
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

bool GrGLGpu::createTextureImpl(const GrSurfaceDesc& desc, GrGLTextureInfo* info,
                                bool renderTarget, GrGLTexture::TexParams* initialTexParams,
                                const GrMipLevel texels[], int mipLevelCount) {
    info->fID = 0;
    info->fTarget = GR_GL_TEXTURE_2D;
    GL_CALL(GenTextures(1, &(info->fID)));

    if (!info->fID) {
        return false;
    }

    this->setScratchTextureUnit();
    GL_CALL(BindTexture(info->fTarget, info->fID));

    if (renderTarget && this->glCaps().textureUsageSupport()) {
        // provides a hint about how this texture will be used
        GL_CALL(TexParameteri(info->fTarget,
                              GR_GL_TEXTURE_USAGE,
                              GR_GL_FRAMEBUFFER_ATTACHMENT));
    }

    if (info) {
        set_initial_texture_params(this->glInterface(), *info, initialTexParams);
    }
    if (!this->uploadTexData(desc.fConfig, desc.fWidth, desc.fHeight, desc.fOrigin, info->fTarget,
                             kNewTexture_UploadType, 0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                             texels, mipLevelCount)) {
        GL_CALL(DeleteTextures(1, &(info->fID)));
        return false;
    }
    return true;
}

GrStencilAttachment* GrGLGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                     int width, int height) {
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
        SkASSERT(GR_GL_NO_ERROR == CHECK_ALLOC_ERROR(this->glInterface()));
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

GrBuffer* GrGLGpu::onCreateBuffer(size_t size, GrBufferType intendedType,
                                  GrAccessPattern accessPattern, const void* data) {
    return GrGLBuffer::Create(this, size, intendedType, accessPattern, data);
}


std::unique_ptr<OpAllocator> GrGLGpu::onCreateInstancedRenderingAllocator() {
    return std::unique_ptr<OpAllocator>(new GLOpAllocator(this->caps()));
}

InstancedRendering* GrGLGpu::onCreateInstancedRendering() {
    return new GLInstancedRendering(this);
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

void GrGLGpu::flushWindowRectangles(const GrWindowRectsState& windowState,
                                    const GrGLRenderTarget* rt) {
#ifndef USE_NSIGHT
    typedef GrWindowRectsState::Mode Mode;
    SkASSERT(!windowState.enabled() || rt->renderFBOID()); // Window rects can't be used on-screen.
    SkASSERT(windowState.numWindows() <= this->caps()->maxWindowRectangles());

    if (!this->caps()->maxWindowRectangles() ||
        fHWWindowRectsState.knownEqualTo(rt->origin(), rt->getViewport(), windowState)) {
        return;
    }

    // This is purely a workaround for a spurious warning generated by gcc. Otherwise the above
    // assert would be sufficient. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=5912
    int numWindows = SkTMin(windowState.numWindows(), int(GrWindowRectangles::kMaxWindows));
    SkASSERT(windowState.numWindows() == numWindows);

    GrGLIRect glwindows[GrWindowRectangles::kMaxWindows];
    const SkIRect* skwindows = windowState.windows().data();
    for (int i = 0; i < numWindows; ++i) {
        glwindows[i].setRelativeTo(rt->getViewport(), skwindows[i], rt->origin());
    }

    GrGLenum glmode = (Mode::kExclusive == windowState.mode()) ? GR_GL_EXCLUSIVE : GR_GL_INCLUSIVE;
    GL_CALL(WindowRectangles(glmode, numWindows, glwindows->asInts()));

    fHWWindowRectsState.set(rt->origin(), rt->getViewport(), windowState);
#endif
}

void GrGLGpu::disableWindowRectangles() {
#ifndef USE_NSIGHT
    if (!this->caps()->maxWindowRectangles() || fHWWindowRectsState.knownDisabled()) {
        return;
    }
    GL_CALL(WindowRectangles(GR_GL_EXCLUSIVE, 0, nullptr));
    fHWWindowRectsState.setDisabled();
#endif
}

void GrGLGpu::flushMinSampleShading(float minSampleShading) {
    if (fHWMinSampleShading != minSampleShading) {
        if (minSampleShading > 0.0) {
            GL_CALL(Enable(GR_GL_SAMPLE_SHADING));
            GL_CALL(MinSampleShading(minSampleShading));
        }
        else {
            GL_CALL(Disable(GR_GL_SAMPLE_SHADING));
        }
        fHWMinSampleShading = minSampleShading;
    }
}

bool GrGLGpu::flushGLState(const GrPipeline& pipeline, const GrPrimitiveProcessor& primProc,
                           bool willDrawPoints) {
    sk_sp<GrGLProgram> program(fProgramCache->refProgram(this, pipeline, primProc, willDrawPoints));
    if (!program) {
        GrCapsDebugf(this->caps(), "Failed to create program!\n");
        return false;
    }

    program->generateMipmaps(primProc, pipeline);

    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);

    this->flushColorWrite(blendInfo.fWriteColor);
    this->flushMinSampleShading(primProc.getSampleShading());

    GrGLuint programID = program->programID();
    if (fHWProgramID != programID) {
        GL_CALL(UseProgram(programID));
        fHWProgramID = programID;
    }

    if (blendInfo.fWriteColor) {
        // Swizzle the blend to match what the shader will output.
        const GrSwizzle& swizzle = this->caps()->shaderCaps()->configOutputSwizzle(
            pipeline.getRenderTarget()->config());
        this->flushBlend(blendInfo, swizzle);
    }

    program->setData(primProc, pipeline);

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(pipeline.getRenderTarget());
    GrStencilSettings stencil;
    if (pipeline.isStencilEnabled()) {
        // TODO: attach stencil and create settings during render target flush.
        SkASSERT(glRT->renderTargetPriv().getStencilAttachment());
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                      glRT->renderTargetPriv().numStencilBits());
    }
    this->flushStencil(stencil);
    this->flushScissor(pipeline.getScissorState(), glRT->getViewport(), glRT->origin());
    this->flushWindowRectangles(pipeline.getWindowRectsState(), glRT);
    this->flushHWAAState(glRT, pipeline.isHWAntialiasState(), !stencil.isDisabled());

    // This must come after textures are flushed because a texture may need
    // to be msaa-resolved (which will modify bound FBO state).
    this->flushRenderTarget(glRT, nullptr, pipeline.getDisableOutputConversionToSRGB());

    return true;
}

void GrGLGpu::setupGeometry(const GrPrimitiveProcessor& primProc,
                            const GrBuffer* indexBuffer,
                            const GrBuffer* vertexBuffer,
                            int baseVertex,
                            const GrBuffer* instanceBuffer,
                            int baseInstance) {
    GrGLAttribArrayState* attribState;
    if (indexBuffer) {
        SkASSERT(indexBuffer && !indexBuffer->isMapped());
        attribState = fHWVertexArrayState.bindInternalVertexArray(this, indexBuffer);
    } else {
        attribState = fHWVertexArrayState.bindInternalVertexArray(this);
    }

    struct {
        const GrBuffer*   fBuffer;
        int               fStride;
        size_t            fBufferOffset;
    } bindings[2];

    if (int vertexStride = primProc.getVertexStride()) {
        SkASSERT(vertexBuffer && !vertexBuffer->isMapped());
        bindings[0].fBuffer = vertexBuffer;
        bindings[0].fStride = vertexStride;
        bindings[0].fBufferOffset = vertexBuffer->baseOffset() + baseVertex * vertexStride;
    }
    if (int instanceStride = primProc.getInstanceStride()) {
        SkASSERT(instanceBuffer && !instanceBuffer->isMapped());
        bindings[1].fBuffer = instanceBuffer;
        bindings[1].fStride = instanceStride;
        bindings[1].fBufferOffset = instanceBuffer->baseOffset() + baseInstance * instanceStride;
    }

    int numAttribs = primProc.numAttribs();
    attribState->enableVertexArrays(this, numAttribs);

    for (int i = 0; i < numAttribs; ++i) {
        using InputRate = GrPrimitiveProcessor::Attribute::InputRate;
        const GrGeometryProcessor::Attribute& attrib = primProc.getAttrib(i);
        const int divisor = InputRate::kPerInstance == attrib.fInputRate ? 1 : 0;
        const auto& binding = bindings[divisor];
        attribState->set(this, i, binding.fBuffer, attrib.fType, binding.fStride,
                         binding.fBufferOffset + attrib.fOffsetInRecord, divisor);
    }
}

GrGLenum GrGLGpu::bindBuffer(GrBufferType type, const GrBuffer* buffer) {
    this->handleDirtyContext();

    // Index buffer state is tied to the vertex array.
    if (kIndex_GrBufferType == type) {
        this->bindVertexArray(0);
    }

    SkASSERT(type >= 0 && type <= kLast_GrBufferType);
    auto& bufferState = fHWBufferState[type];

    if (buffer->uniqueID() != bufferState.fBoundBufferUniqueID) {
        if (buffer->isCPUBacked()) {
            if (!bufferState.fBufferZeroKnownBound) {
                GL_CALL(BindBuffer(bufferState.fGLTarget, 0));
            }
        } else {
            const GrGLBuffer* glBuffer = static_cast<const GrGLBuffer*>(buffer);
            GL_CALL(BindBuffer(bufferState.fGLTarget, glBuffer->bufferID()));
        }
        bufferState.fBufferZeroKnownBound = buffer->isCPUBacked();
        bufferState.fBoundBufferUniqueID = buffer->uniqueID();
    }

    return bufferState.fGLTarget;
}

void GrGLGpu::notifyBufferReleased(const GrGLBuffer* buffer) {
    if (buffer->hasAttachedToTexture()) {
        // Detach this buffer from any textures to ensure the underlying memory is freed.
        GrGpuResource::UniqueID uniqueID = buffer->uniqueID();
        for (int i = fHWMaxUsedBufferTextureUnit; i >= 0; --i) {
            auto& buffTex = fHWBufferTextures[i];
            if (uniqueID != buffTex.fAttachedBufferUniqueID) {
                continue;
            }
            if (i == fHWMaxUsedBufferTextureUnit) {
                --fHWMaxUsedBufferTextureUnit;
            }

            this->setTextureUnit(i);
            if (!buffTex.fKnownBound) {
                SkASSERT(buffTex.fTextureID);
                GL_CALL(BindTexture(GR_GL_TEXTURE_BUFFER, buffTex.fTextureID));
                buffTex.fKnownBound = true;
            }
            GL_CALL(TexBuffer(GR_GL_TEXTURE_BUFFER,
                              this->glCaps().configSizedInternalFormat(buffTex.fTexelConfig), 0));
        }
    }
}

void GrGLGpu::disableScissor() {
    if (kNo_TriState != fHWScissorSettings.fEnabled) {
        GL_CALL(Disable(GR_GL_SCISSOR_TEST));
        fHWScissorSettings.fEnabled = kNo_TriState;
        return;
    }
}

void GrGLGpu::clear(const GrFixedClip& clip, GrColor color, GrRenderTarget* target) {
    this->handleDirtyContext();

    // parent class should never let us get here with no RT
    SkASSERT(target);
    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);

    this->flushRenderTarget(glRT, clip.scissorEnabled() ? &clip.scissorRect() : nullptr);
    this->flushScissor(clip.scissorState(), glRT->getViewport(), glRT->origin());
    this->flushWindowRectangles(clip.windowRectsState(), glRT);

    GrGLfloat r, g, b, a;
    static const GrGLfloat scale255 = 1.f / 255.f;
    a = GrColorUnpackA(color) * scale255;
    GrGLfloat scaleRGB = scale255;
    r = GrColorUnpackR(color) * scaleRGB;
    g = GrColorUnpackG(color) * scaleRGB;
    b = GrColorUnpackB(color) * scaleRGB;

    GL_CALL(ColorMask(GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE, GR_GL_TRUE));
    fHWWriteToColor = kYes_TriState;

    if (this->glCaps().clearToBoundaryValuesIsBroken() &&
        (1 == r || 0 == r) && (1 == g || 0 == g) && (1 == b || 0 == b) && (1 == a || 0 == a)) {
        static const GrGLfloat safeAlpha1 = nextafter(1.f, 2.f);
        static const GrGLfloat safeAlpha0 = nextafter(0.f, -1.f);
        a = (1 == a) ? safeAlpha1 : safeAlpha0;
    }
    GL_CALL(ClearColor(r, g, b, a));
    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

void GrGLGpu::clearStencil(GrRenderTarget* target) {
    if (nullptr == target) {
        return;
    }
    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);
    this->flushRenderTarget(glRT, &SkIRect::EmptyIRect());

    this->disableScissor();
    this->disableWindowRectangles();

    GL_CALL(StencilMask(0xffffffff));
    GL_CALL(ClearStencil(0));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

void GrGLGpu::clearStencilClip(const GrFixedClip& clip,
                               bool insideStencilMask,
                               GrRenderTarget* target) {
    SkASSERT(target);
    this->handleDirtyContext();

    if (this->glCaps().useDrawToClearStencilClip()) {
        this->clearStencilClipAsDraw(clip, insideStencilMask, target);
        return;
    }

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
    // turned into draws. Our contract on GrOpList says that
    // changing the clip between stencil passes may or may not
    // zero the client's clip bits. So we just clear the whole thing.
    static const GrGLint clipStencilMask  = ~0;
#endif
    GrGLint value;
    if (insideStencilMask) {
        value = (1 << (stencilBitCount - 1));
    } else {
        value = 0;
    }
    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);
    this->flushRenderTarget(glRT, &SkIRect::EmptyIRect());

    this->flushScissor(clip.scissorState(), glRT->getViewport(), glRT->origin());
    this->flushWindowRectangles(clip.windowRectsState(), glRT);

    GL_CALL(StencilMask((uint32_t) clipStencilMask));
    GL_CALL(ClearStencil(value));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

static bool read_pixels_pays_for_y_flip(GrSurfaceOrigin origin, const GrGLCaps& caps,
                                        int width, int height,  GrPixelConfig config,
                                        size_t rowBytes) {
    // If the surface is already TopLeft, we don't need to flip.
    if (kTopLeft_GrSurfaceOrigin == origin) {
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

bool GrGLGpu::readPixelsSupported(GrRenderTarget* target, GrPixelConfig readConfig) {
#ifdef SK_BUILD_FOR_MAC
    // Chromium may ask us to read back from locked IOSurfaces. Calling the command buffer's
    // glGetIntegerv() with GL_IMPLEMENTATION_COLOR_READ_FORMAT/_TYPE causes the command buffer
    // to make a call to check the framebuffer status which can hang the driver. So in Mac Chromium
    // we always use a temporary surface to test for read pixels support.
    // https://www.crbug.com/662802
    if (this->glContext().driver() == kChromium_GrGLDriver) {
        return this->readPixelsSupported(target->config(), readConfig);
    }
#endif
    auto bindRenderTarget = [this, target]() -> bool {
        this->flushRenderTarget(static_cast<GrGLRenderTarget*>(target), &SkIRect::EmptyIRect());
        return true;
    };
    auto unbindRenderTarget = []{};
    auto getIntegerv = [this](GrGLenum query, GrGLint* value) {
        GR_GL_GetIntegerv(this->glInterface(), query, value);
    };
    GrPixelConfig rtConfig = target->config();
    return this->glCaps().readPixelsSupported(rtConfig, readConfig, getIntegerv, bindRenderTarget,
                                              unbindRenderTarget);
}

bool GrGLGpu::readPixelsSupported(GrPixelConfig rtConfig, GrPixelConfig readConfig) {
    sk_sp<GrTexture> temp;
    auto bindRenderTarget = [this, rtConfig, &temp]() -> bool {
        GrTextureDesc desc;
        desc.fConfig = rtConfig;
        desc.fWidth = desc.fHeight = 16;
        if (this->glCaps().isConfigRenderable(rtConfig, false)) {
            desc.fFlags = kRenderTarget_GrSurfaceFlag;
            temp = this->createTexture(desc, SkBudgeted::kNo);
            if (!temp) {
                return false;
            }
            GrGLRenderTarget* glrt = static_cast<GrGLRenderTarget*>(temp->asRenderTarget());
            this->flushRenderTarget(glrt, &SkIRect::EmptyIRect());
            return true;
        } else if (this->glCaps().canConfigBeFBOColorAttachment(rtConfig)) {
            temp = this->createTexture(desc, SkBudgeted::kNo);
            if (!temp) {
                return false;
            }
            GrGLIRect vp;
            this->bindSurfaceFBOForPixelOps(temp.get(), GR_GL_FRAMEBUFFER, &vp, kDst_TempFBOTarget);
            fHWBoundRenderTargetUniqueID.makeInvalid();
            return true;
        }
        return false;
    };
    auto unbindRenderTarget = [this, &temp]() {
        this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, temp.get());
    };
    auto getIntegerv = [this](GrGLenum query, GrGLint* value) {
        GR_GL_GetIntegerv(this->glInterface(), query, value);
    };
    return this->glCaps().readPixelsSupported(rtConfig, readConfig, getIntegerv, bindRenderTarget,
                                              unbindRenderTarget);
}

bool GrGLGpu::readPixelsSupported(GrSurface* surfaceForConfig, GrPixelConfig readConfig) {
    if (GrRenderTarget* rt = surfaceForConfig->asRenderTarget()) {
        return this->readPixelsSupported(rt, readConfig);
    } else {
        GrPixelConfig config = surfaceForConfig->config();
        return this->readPixelsSupported(config, readConfig);
    }
}

static bool requires_srgb_conversion(GrPixelConfig a, GrPixelConfig b) {
    if (GrPixelConfigIsSRGB(a)) {
        return !GrPixelConfigIsSRGB(b) && !GrPixelConfigIsAlphaOnly(b);
    } else if (GrPixelConfigIsSRGB(b)) {
        return !GrPixelConfigIsSRGB(a) && !GrPixelConfigIsAlphaOnly(a);
    }
    return false;
}

bool GrGLGpu::onGetReadPixelsInfo(GrSurface* srcSurface, int width, int height, size_t rowBytes,
                                  GrPixelConfig readConfig, DrawPreference* drawPreference,
                                  ReadPixelTempDrawInfo* tempDrawInfo) {
    GrPixelConfig srcConfig = srcSurface->config();

    // These settings we will always want if a temp draw is performed.
    tempDrawInfo->fTempSurfaceDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 0;
    tempDrawInfo->fTempSurfaceDesc.fOrigin = kTopLeft_GrSurfaceOrigin; // no CPU y-flip for TL.
    tempDrawInfo->fTempSurfaceFit = this->glCaps().partialFBOReadIsSlow() ? SkBackingFit::kExact
                                                                          : SkBackingFit::kApprox;
    // For now assume no swizzling, we may change that below.
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();

    // Depends on why we need/want a temp draw. Start off assuming no change, the surface we read
    // from will be srcConfig and we will read readConfig pixels from it.
    // Note that if we require a draw and return a non-renderable format for the temp surface the
    // base class will fail for us.
    tempDrawInfo->fTempSurfaceDesc.fConfig = srcConfig;
    tempDrawInfo->fReadConfig = readConfig;

    if (requires_srgb_conversion(srcConfig, readConfig)) {
        if (!this->readPixelsSupported(readConfig, readConfig)) {
            return false;
        }
        // Draw to do srgb to linear conversion or vice versa.
        ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        tempDrawInfo->fTempSurfaceDesc.fConfig = readConfig;
        tempDrawInfo->fReadConfig = readConfig;
        return true;
    }

    if (this->glCaps().rgba8888PixelsOpsAreSlow() && kRGBA_8888_GrPixelConfig == readConfig &&
        this->readPixelsSupported(kBGRA_8888_GrPixelConfig, kBGRA_8888_GrPixelConfig)) {
        tempDrawInfo->fTempSurfaceDesc.fConfig = kBGRA_8888_GrPixelConfig;
        tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
        tempDrawInfo->fReadConfig = kBGRA_8888_GrPixelConfig;
        ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
    } else if (this->glCaps().rgbaToBgraReadbackConversionsAreSlow() &&
               GrBytesPerPixel(readConfig) == 4 &&
               GrPixelConfigSwapRAndB(readConfig) == srcConfig &&
               this->readPixelsSupported(srcSurface, srcConfig)) {
        // Mesa 3D takes a slow path on when reading back BGRA from an RGBA surface and vice-versa.
        // Better to do a draw with a R/B swap and then read as the original config.
        tempDrawInfo->fTempSurfaceDesc.fConfig = srcConfig;
        tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
        tempDrawInfo->fReadConfig = srcConfig;
        ElevateDrawPreference(drawPreference, kGpuPrefersDraw_DrawPreference);
    } else if (!this->readPixelsSupported(srcSurface, readConfig)) {
        if (readConfig == kBGRA_8888_GrPixelConfig &&
            this->glCaps().canConfigBeFBOColorAttachment(kRGBA_8888_GrPixelConfig) &&
            this->readPixelsSupported(kRGBA_8888_GrPixelConfig, kRGBA_8888_GrPixelConfig)) {
            // We're trying to read BGRA but it's not supported. If RGBA is renderable and
            // we can read it back, then do a swizzling draw to a RGBA and read it back (which
            // will effectively be BGRA).
            tempDrawInfo->fTempSurfaceDesc.fConfig = kRGBA_8888_GrPixelConfig;
            tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
            tempDrawInfo->fReadConfig = kRGBA_8888_GrPixelConfig;
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        } else if (readConfig == kSBGRA_8888_GrPixelConfig &&
            this->glCaps().canConfigBeFBOColorAttachment(kSRGBA_8888_GrPixelConfig) &&
            this->readPixelsSupported(kSRGBA_8888_GrPixelConfig, kSRGBA_8888_GrPixelConfig)) {
            // We're trying to read sBGRA but it's not supported. If sRGBA is renderable and
            // we can read it back, then do a swizzling draw to a sRGBA and read it back (which
            // will effectively be sBGRA).
            tempDrawInfo->fTempSurfaceDesc.fConfig = kSRGBA_8888_GrPixelConfig;
            tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
            tempDrawInfo->fReadConfig = kSRGBA_8888_GrPixelConfig;
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        } else if (readConfig == kAlpha_8_GrPixelConfig) {
            // onReadPixels implements a fallback for cases where we are want to read kAlpha_8,
            // it's unsupported, but 32bit RGBA reads are supported.
            // Don't attempt to do any srgb conversions since we only care about alpha.
            GrPixelConfig cpuTempConfig = kRGBA_8888_GrPixelConfig;
            if (GrPixelConfigIsSRGB(srcSurface->config())) {
                cpuTempConfig = kSRGBA_8888_GrPixelConfig;
            }
            if (!this->readPixelsSupported(srcSurface, cpuTempConfig)) {
                // If we can't read RGBA from the src try to draw to a kRGBA_8888 (or kSRGBA_8888)
                // first and then onReadPixels will read that to a 32bit temporary buffer.
                if (this->glCaps().canConfigBeFBOColorAttachment(cpuTempConfig)) {
                    ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
                    tempDrawInfo->fTempSurfaceDesc.fConfig = cpuTempConfig;
                    tempDrawInfo->fReadConfig = kAlpha_8_GrPixelConfig;
                } else {
                    return false;
                }
            } else {
                SkASSERT(tempDrawInfo->fTempSurfaceDesc.fConfig == srcConfig);
                SkASSERT(tempDrawInfo->fReadConfig == kAlpha_8_GrPixelConfig);
            }
        } else if (this->glCaps().canConfigBeFBOColorAttachment(readConfig) &&
                   this->readPixelsSupported(readConfig, readConfig)) {
            // Do a draw to convert from the src config to the read config.
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
            tempDrawInfo->fTempSurfaceDesc.fConfig = readConfig;
            tempDrawInfo->fReadConfig = readConfig;
        } else {
            return false;
        }
    }

    if ((srcSurface->asRenderTarget() || this->glCaps().canConfigBeFBOColorAttachment(srcConfig)) &&
        read_pixels_pays_for_y_flip(srcSurface->origin(), this->glCaps(), width, height, readConfig,
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

    GrGLRenderTarget* renderTarget = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    if (!renderTarget && !this->glCaps().canConfigBeFBOColorAttachment(surface->config())) {
        return false;
    }

    // OpenGL doesn't do sRGB <-> linear conversions when reading and writing pixels.
    if (requires_srgb_conversion(surface->config(), config)) {
        return false;
    }

    // We have a special case fallback for reading eight bit alpha. We will read back all four 8
    // bit channels as RGBA and then extract A.
    if (!this->readPixelsSupported(surface, config)) {
        // Don't attempt to do any srgb conversions since we only care about alpha.
        GrPixelConfig tempConfig = kRGBA_8888_GrPixelConfig;
        if (GrPixelConfigIsSRGB(surface->config())) {
            tempConfig = kSRGBA_8888_GrPixelConfig;
        }
        if (kAlpha_8_GrPixelConfig == config &&
            this->readPixelsSupported(surface, tempConfig)) {
            std::unique_ptr<uint32_t[]> temp(new uint32_t[width * height * 4]);
            if (this->onReadPixels(surface, left, top, width, height, tempConfig, temp.get(),
                                   width*4)) {
                uint8_t* dst = reinterpret_cast<uint8_t*>(buffer);
                for (int j = 0; j < height; ++j) {
                    for (int i = 0; i < width; ++i) {
                        dst[j*rowBytes + i] = (0xFF000000U & temp[j*width+i]) >> 24;
                    }
                }
                return true;
            }
        }
        return false;
    }

    GrGLenum externalFormat;
    GrGLenum externalType;
    if (!this->glCaps().getReadPixelsFormat(surface->config(), config, &externalFormat,
                                            &externalType)) {
        return false;
    }
    bool flipY = kBottomLeft_GrSurfaceOrigin == surface->origin();

    GrGLIRect glvp;
    if (renderTarget) {
        // resolve the render target if necessary
        switch (renderTarget->getResolveType()) {
            case GrGLRenderTarget::kCantResolve_ResolveType:
                return false;
            case GrGLRenderTarget::kAutoResolves_ResolveType:
                this->flushRenderTarget(renderTarget, &SkIRect::EmptyIRect());
                break;
            case GrGLRenderTarget::kCanResolve_ResolveType:
                this->onResolveRenderTarget(renderTarget);
                // we don't track the state of the READ FBO ID.
                fStats.incRenderTargetBinds();
                GL_CALL(BindFramebuffer(GR_GL_READ_FRAMEBUFFER, renderTarget->textureFBOID()));
                break;
            default:
                SkFAIL("Unknown resolve type");
        }
        glvp = renderTarget->getViewport();
    } else {
        // Use a temporary FBO.
        this->bindSurfaceFBOForPixelOps(surface, GR_GL_FRAMEBUFFER, &glvp, kSrc_TempFBOTarget);
        fHWBoundRenderTargetUniqueID.makeInvalid();
    }

    // the read rect is viewport-relative
    GrGLIRect readRect;
    readRect.setRelativeTo(glvp, left, top, width, height, surface->origin());

    size_t bytesPerPixel = GrBytesPerPixel(config);
    size_t tightRowBytes = bytesPerPixel * width;

    size_t readDstRowBytes = tightRowBytes;
    void* readDst = buffer;

    // determine if GL can read using the passed rowBytes or if we need
    // a scratch buffer.
    SkAutoSMalloc<32 * sizeof(GrColor)> scratch;
    if (rowBytes != tightRowBytes) {
        if (this->glCaps().packRowLengthSupport() && !(rowBytes % bytesPerPixel)) {
            GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH,
                                static_cast<GrGLint>(rowBytes / bytesPerPixel)));
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
        SkASSERT(readDst != buffer);
        SkASSERT(rowBytes != tightRowBytes);
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
    if (!renderTarget) {
        this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, surface);
    }
    return true;
}

GrGpuCommandBuffer* GrGLGpu::createCommandBuffer(
        const GrGpuCommandBuffer::LoadAndStoreInfo& colorInfo,
        const GrGpuCommandBuffer::LoadAndStoreInfo& stencilInfo) {
    return new GrGLGpuCommandBuffer(this);
}

void GrGLGpu::flushRenderTarget(GrGLRenderTarget* target, const SkIRect* bounds, bool disableSRGB) {
    SkASSERT(target);

    GrGpuResource::UniqueID rtID = target->uniqueID();
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
        this->flushViewport(target->getViewport());
    }

    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(GrPixelConfigIsSRGB(target->config()) && !disableSRGB);
    }

    this->didWriteToSurface(target, bounds);
}

void GrGLGpu::flushFramebufferSRGB(bool enable) {
    if (enable && kYes_TriState != fHWSRGBFramebuffer) {
        GL_CALL(Enable(GR_GL_FRAMEBUFFER_SRGB));
        fHWSRGBFramebuffer = kYes_TriState;
    } else if (!enable && kNo_TriState != fHWSRGBFramebuffer) {
        GL_CALL(Disable(GR_GL_FRAMEBUFFER_SRGB));
        fHWSRGBFramebuffer = kNo_TriState;
    }
}

void GrGLGpu::flushViewport(const GrGLIRect& viewport) {
    if (fHWViewport != viewport) {
        viewport.pushToGLViewport(this->glInterface());
        fHWViewport = viewport;
    }
}

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

void GrGLGpu::draw(const GrPipeline& pipeline,
                   const GrPrimitiveProcessor& primProc,
                   const GrMesh meshes[],
                   const GrPipeline::DynamicState dynamicStates[],
                   int meshCount) {
    this->handleDirtyContext();

    bool hasPoints = false;
    for (int i = 0; i < meshCount; ++i) {
        if (meshes[i].primitiveType() == GrPrimitiveType::kPoints) {
            hasPoints = true;
            break;
        }
    }
    if (!this->flushGLState(pipeline, primProc, hasPoints)) {
        return;
    }

    for (int i = 0; i < meshCount; ++i) {
        if (GrXferBarrierType barrierType = pipeline.xferBarrierType(*this->caps())) {
            this->xferBarrier(pipeline.getRenderTarget(), barrierType);
        }

        if (dynamicStates) {
            if (pipeline.getScissorState().enabled()) {
                GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(pipeline.getRenderTarget());
                this->flushScissor(dynamicStates[i].fScissorRect,
                                   glRT->getViewport(), glRT->origin());
            }
        }
        if (this->glCaps().requiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines() &&
            GrIsPrimTypeLines(meshes[i].primitiveType()) &&
            !GrIsPrimTypeLines(fLastPrimitiveType)) {
            GL_CALL(Enable(GR_GL_CULL_FACE));
            GL_CALL(Disable(GR_GL_CULL_FACE));
        }
        meshes[i].sendToGpu(primProc, this);
        fLastPrimitiveType = meshes[i].primitiveType();
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

static GrGLenum gr_primitive_type_to_gl_mode(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
            return GR_GL_TRIANGLES;
        case GrPrimitiveType::kTriangleStrip:
            return GR_GL_TRIANGLE_STRIP;
        case GrPrimitiveType::kTriangleFan:
            return GR_GL_TRIANGLE_FAN;
        case GrPrimitiveType::kPoints:
            return GR_GL_POINTS;
        case GrPrimitiveType::kLines:
            return GR_GL_LINES;
        case GrPrimitiveType::kLineStrip:
            return GR_GL_LINE_STRIP;
        case GrPrimitiveType::kLinesAdjacency:
            return GR_GL_LINES_ADJACENCY;
    }
    SkFAIL("invalid GrPrimitiveType");
    return GR_GL_TRIANGLES;
}

void GrGLGpu::sendMeshToGpu(const GrPrimitiveProcessor& primProc, GrPrimitiveType primitiveType,
                            const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) {
    const GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    if (this->glCaps().drawArraysBaseVertexIsBroken()) {
        this->setupGeometry(primProc, nullptr, vertexBuffer, baseVertex, nullptr, 0);
        GL_CALL(DrawArrays(glPrimType, 0, vertexCount));
    } else {
        this->setupGeometry(primProc, nullptr, vertexBuffer, 0, nullptr, 0);
        GL_CALL(DrawArrays(glPrimType, baseVertex, vertexCount));
    }
    fStats.incNumDraws();
}

void GrGLGpu::sendIndexedMeshToGpu(const GrPrimitiveProcessor& primProc,
                                   GrPrimitiveType primitiveType, const GrBuffer* indexBuffer,
                                   int indexCount, int baseIndex, uint16_t minIndexValue,
                                   uint16_t maxIndexValue, const GrBuffer* vertexBuffer,
                                   int baseVertex) {
    const GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    GrGLvoid* const indices = reinterpret_cast<void*>(indexBuffer->baseOffset() +
                                                      sizeof(uint16_t) * baseIndex);

    this->setupGeometry(primProc, indexBuffer, vertexBuffer, baseVertex, nullptr, 0);

    if (this->glCaps().drawRangeElementsSupport()) {
        GL_CALL(DrawRangeElements(glPrimType, minIndexValue, maxIndexValue, indexCount,
                                  GR_GL_UNSIGNED_SHORT, indices));
    } else {
        GL_CALL(DrawElements(glPrimType, indexCount, GR_GL_UNSIGNED_SHORT, indices));
    }
    fStats.incNumDraws();
}

void GrGLGpu::sendInstancedMeshToGpu(const GrPrimitiveProcessor& primProc, GrPrimitiveType
                                     primitiveType, const GrBuffer* vertexBuffer,
                                     int vertexCount, int baseVertex,
                                     const GrBuffer* instanceBuffer, int instanceCount,
                                     int baseInstance) {
    const GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    this->setupGeometry(primProc, nullptr, vertexBuffer, 0, instanceBuffer, baseInstance);
    GL_CALL(DrawArraysInstanced(glPrimType, baseVertex, vertexCount, instanceCount));
    fStats.incNumDraws();
}

void GrGLGpu::sendIndexedInstancedMeshToGpu(const GrPrimitiveProcessor& primProc,
                                            GrPrimitiveType primitiveType,
                                            const GrBuffer* indexBuffer, int indexCount,
                                            int baseIndex, const GrBuffer* vertexBuffer,
                                            int baseVertex, const GrBuffer* instanceBuffer,
                                            int instanceCount, int baseInstance) {
    const GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    GrGLvoid* indices = reinterpret_cast<void*>(indexBuffer->baseOffset() +
                                                sizeof(uint16_t) * baseIndex);
    this->setupGeometry(primProc, indexBuffer, vertexBuffer, baseVertex,
                        instanceBuffer, baseInstance);
    GL_CALL(DrawElementsInstanced(glPrimType, indexCount, GR_GL_UNSIGNED_SHORT, indices,
                                  instanceCount));
    fStats.incNumDraws();
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
            fHWBoundRenderTargetUniqueID.makeInvalid();
            const GrGLIRect& vp = rt->getViewport();
            const SkIRect dirtyRect = rt->getResolveRect();

            if (GrGLCaps::kES_Apple_MSFBOType == this->glCaps().msFBOType()) {
                // Apple's extension uses the scissor as the blit bounds.
                GrScissorState scissorState;
                scissorState.set(dirtyRect);
                this->flushScissor(scissorState, vp, rt->origin());
                this->disableWindowRectangles();
                GL_CALL(ResolveMultisampleFramebuffer());
            } else {
                int l, b, r, t;
                if (GrGLCaps::kResolveMustBeFull_BlitFrambufferFlag &
                    this->glCaps().blitFramebufferSupportFlags()) {
                    l = 0;
                    b = 0;
                    r = target->width();
                    t = target->height();
                } else {
                    GrGLIRect rect;
                    rect.setRelativeTo(vp, dirtyRect.fLeft, dirtyRect.fTop,
                                       dirtyRect.width(), dirtyRect.height(), target->origin());
                    l = rect.fLeft;
                    b = rect.fBottom;
                    r = rect.fLeft + rect.fWidth;
                    t = rect.fBottom + rect.fHeight;
                }

                // BlitFrameBuffer respects the scissor, so disable it.
                this->disableScissor();
                this->disableWindowRectangles();
                GL_CALL(BlitFramebuffer(l, b, r, t, l, b, r, t,
                                        GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
            }
        }
        rt->flagAsResolved();
    }
}

namespace {


GrGLenum gr_to_gl_stencil_op(GrStencilOp op) {
    static const GrGLenum gTable[kGrStencilOpCount] = {
        GR_GL_KEEP,        // kKeep
        GR_GL_ZERO,        // kZero
        GR_GL_REPLACE,     // kReplace
        GR_GL_INVERT,      // kInvert
        GR_GL_INCR_WRAP,   // kIncWrap
        GR_GL_DECR_WRAP,   // kDecWrap
        GR_GL_INCR,        // kIncClamp
        GR_GL_DECR,        // kDecClamp
    };
    GR_STATIC_ASSERT(0 == (int)GrStencilOp::kKeep);
    GR_STATIC_ASSERT(1 == (int)GrStencilOp::kZero);
    GR_STATIC_ASSERT(2 == (int)GrStencilOp::kReplace);
    GR_STATIC_ASSERT(3 == (int)GrStencilOp::kInvert);
    GR_STATIC_ASSERT(4 == (int)GrStencilOp::kIncWrap);
    GR_STATIC_ASSERT(5 == (int)GrStencilOp::kDecWrap);
    GR_STATIC_ASSERT(6 == (int)GrStencilOp::kIncClamp);
    GR_STATIC_ASSERT(7 == (int)GrStencilOp::kDecClamp);
    SkASSERT(op < (GrStencilOp)kGrStencilOpCount);
    return gTable[(int)op];
}

void set_gl_stencil(const GrGLInterface* gl,
                    const GrStencilSettings::Face& face,
                    GrGLenum glFace) {
    GrGLenum glFunc = GrToGLStencilFunc(face.fTest);
    GrGLenum glFailOp = gr_to_gl_stencil_op(face.fFailOp);
    GrGLenum glPassOp = gr_to_gl_stencil_op(face.fPassOp);

    GrGLint ref = face.fRef;
    GrGLint mask = face.fTestMask;
    GrGLint writeMask = face.fWriteMask;

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
    if (stencilSettings.isDisabled()) {
        this->disableStencil();
    } else if (fHWStencilSettings != stencilSettings) {
        if (kYes_TriState != fHWStencilTestEnabled) {
            GL_CALL(Enable(GR_GL_STENCIL_TEST));

            fHWStencilTestEnabled = kYes_TriState;
        }
        if (stencilSettings.isTwoSided()) {
            set_gl_stencil(this->glInterface(),
                           stencilSettings.front(),
                           GR_GL_FRONT);
            set_gl_stencil(this->glInterface(),
                           stencilSettings.back(),
                           GR_GL_BACK);
        } else {
            set_gl_stencil(this->glInterface(),
                           stencilSettings.front(),
                           GR_GL_FRONT_AND_BACK);
        }
        fHWStencilSettings = stencilSettings;
    }
}

void GrGLGpu::disableStencil() {
    if (kNo_TriState != fHWStencilTestEnabled) {
        GL_CALL(Disable(GR_GL_STENCIL_TEST));

        fHWStencilTestEnabled = kNo_TriState;
        fHWStencilSettings.invalidate();
    }
}

void GrGLGpu::flushHWAAState(GrRenderTarget* rt, bool useHWAA, bool stencilEnabled) {
    // rt is only optional if useHWAA is false.
    SkASSERT(rt || !useHWAA);
    SkASSERT(!useHWAA || rt->isStencilBufferMultisampled());

    if (this->caps()->multisampleDisableSupport()) {
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

    if (0 != this->caps()->maxRasterSamples()) {
        if (useHWAA && GrFSAAType::kMixedSamples == rt->fsaaType() && !stencilEnabled) {
            // Since stencil is disabled and we want more samples than are in the color buffer, we
            // need to tell the rasterizer explicitly how many to run.
            if (kYes_TriState != fHWRasterMultisampleEnabled) {
                GL_CALL(Enable(GR_GL_RASTER_MULTISAMPLE));
                fHWRasterMultisampleEnabled = kYes_TriState;
            }
            if (rt->numStencilSamples() != fHWNumRasterSamples) {
                SkASSERT(rt->numStencilSamples() <= this->caps()->maxRasterSamples());
                GL_CALL(RasterSamples(rt->numStencilSamples(), GR_GL_TRUE));
                fHWNumRasterSamples = rt->numStencilSamples();
            }
        } else {
            if (kNo_TriState != fHWRasterMultisampleEnabled) {
                GL_CALL(Disable(GR_GL_RASTER_MULTISAMPLE));
                fHWRasterMultisampleEnabled = kNo_TriState;
            }
        }
    } else {
        SkASSERT(!useHWAA || GrFSAAType::kMixedSamples != rt->fsaaType() || stencilEnabled);
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

    if (fHWBlendState.fSrcCoeff != srcCoeff || fHWBlendState.fDstCoeff != dstCoeff) {
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

void GrGLGpu::bindTexture(int unitIdx, const GrSamplerParams& params, bool allowSRGBInputs,
                          GrGLTexture* texture) {
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

    GrGpuResource::UniqueID textureID = texture->uniqueID();
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
    GrSamplerParams::FilterMode filterMode = params.filterMode();

    if (GrSamplerParams::kMipMap_FilterMode == filterMode) {
        if (!this->caps()->mipMapSupport()) {
            filterMode = GrSamplerParams::kBilerp_FilterMode;
        }
    }

    newTexParams.fMinFilter = glMinFilterModes[filterMode];
    newTexParams.fMagFilter = glMagFilterModes[filterMode];

    if (this->glCaps().srgbDecodeDisableSupport() && GrPixelConfigIsSRGB(texture->config())) {
        newTexParams.fSRGBDecode = allowSRGBInputs ? GR_GL_DECODE_EXT : GR_GL_SKIP_DECODE_EXT;
        if (setAll || newTexParams.fSRGBDecode != oldTexParams.fSRGBDecode) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SRGB_DECODE_EXT, newTexParams.fSRGBDecode));
        }
    }

#ifdef SK_DEBUG
    // We were supposed to ensure MipMaps were up-to-date and built correctly before getting here.
    if (GrSamplerParams::kMipMap_FilterMode == filterMode) {
        SkASSERT(!texture->texturePriv().mipMapsAreDirty());
        if (GrPixelConfigIsSRGB(texture->config())) {
            SkDestinationSurfaceColorMode colorMode = allowSRGBInputs
                ? SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware
                : SkDestinationSurfaceColorMode::kLegacy;
            SkASSERT(texture->texturePriv().mipColorMode() == colorMode);
        }
    }
#endif

    newTexParams.fMaxMipMapLevel = texture->texturePriv().maxMipMapLevel();

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
    if (setAll || newTexParams.fMaxMipMapLevel != oldTexParams.fMaxMipMapLevel) {
        // These are not supported in ES2 contexts
        if (this->glCaps().mipMapLevelAndLodControlSupport()) {
            if (newTexParams.fMaxMipMapLevel != 0) {
                this->setTextureUnit(unitIdx);
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MIN_LOD, 0));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_BASE_LEVEL, 0));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAX_LOD,
                                      newTexParams.fMaxMipMapLevel));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAX_LEVEL,
                                      newTexParams.fMaxMipMapLevel));
            }
        }
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
        this->setTextureSwizzle(unitIdx, target, newTexParams.fSwizzleRGBA);
    }
    texture->setCachedTexParams(newTexParams, this->getResetTimestamp());
}

void GrGLGpu::bindTexelBuffer(int unitIdx, GrPixelConfig texelConfig, GrGLBuffer* buffer) {
    SkASSERT(this->glCaps().canUseConfigWithTexelBuffer(texelConfig));
    SkASSERT(unitIdx >= 0 && unitIdx < fHWBufferTextures.count());

    BufferTexture& buffTex = fHWBufferTextures[unitIdx];

    if (!buffTex.fKnownBound) {
        if (!buffTex.fTextureID) {
            GL_CALL(GenTextures(1, &buffTex.fTextureID));
            if (!buffTex.fTextureID) {
                return;
            }
        }

        this->setTextureUnit(unitIdx);
        GL_CALL(BindTexture(GR_GL_TEXTURE_BUFFER, buffTex.fTextureID));

        buffTex.fKnownBound = true;
    }

    if (buffer->uniqueID() != buffTex.fAttachedBufferUniqueID ||
        buffTex.fTexelConfig != texelConfig) {

        this->setTextureUnit(unitIdx);
        GL_CALL(TexBuffer(GR_GL_TEXTURE_BUFFER,
                          this->glCaps().configSizedInternalFormat(texelConfig),
                          buffer->bufferID()));

        buffTex.fTexelConfig = texelConfig;
        buffTex.fAttachedBufferUniqueID = buffer->uniqueID();

        if (this->glCaps().textureSwizzleSupport() &&
            this->glCaps().configSwizzle(texelConfig) != buffTex.fSwizzle) {
            GrGLenum glSwizzle[4];
            get_tex_param_swizzle(texelConfig, this->glCaps(), glSwizzle);
            this->setTextureSwizzle(unitIdx, GR_GL_TEXTURE_BUFFER, glSwizzle);
            buffTex.fSwizzle = this->glCaps().configSwizzle(texelConfig);
        }

        buffer->setHasAttachedToTexture();
        fHWMaxUsedBufferTextureUnit = SkTMax(unitIdx, fHWMaxUsedBufferTextureUnit);
    }
}

void GrGLGpu::bindImageStorage(int unitIdx, GrIOType ioType, GrGLTexture *texture) {
    SkASSERT(texture);
    if (texture->uniqueID() != fHWBoundImageStorages[unitIdx].fTextureUniqueID ||
        ioType != fHWBoundImageStorages[unitIdx].fIOType) {
        GrGLenum access = GR_GL_READ_ONLY;
        switch (ioType) {
            case kRead_GrIOType:
                access = GR_GL_READ_ONLY;
                break;
            case kWrite_GrIOType:
                access = GR_GL_WRITE_ONLY;
                break;
            case kRW_GrIOType:
                access = GR_GL_READ_WRITE;
                break;
        }
        GrGLenum format = this->glCaps().getImageFormat(texture->config());
        GL_CALL(BindImageTexture(unitIdx, texture->textureID(), 0, GR_GL_FALSE, 0, access, format));
    }
}

void GrGLGpu::generateMipmaps(const GrSamplerParams& params, bool allowSRGBInputs,
                              GrGLTexture* texture) {
    SkASSERT(texture);

    // First, figure out if we need mips for this texture at all:
    GrSamplerParams::FilterMode filterMode = params.filterMode();

    if (GrSamplerParams::kMipMap_FilterMode == filterMode) {
        if (!this->caps()->mipMapSupport()) {
            filterMode = GrSamplerParams::kBilerp_FilterMode;
        }
    }

    if (GrSamplerParams::kMipMap_FilterMode != filterMode) {
        return;
    }

    // If this is an sRGB texture and the mips were previously built the "other" way
    // (gamma-correct vs. not), then we need to rebuild them. We don't need to check for
    // srgbSupport - we'll *never* get an sRGB pixel config if we don't support it.
    SkDestinationSurfaceColorMode colorMode = allowSRGBInputs
        ? SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware
        : SkDestinationSurfaceColorMode::kLegacy;
    if (GrPixelConfigIsSRGB(texture->config()) &&
        colorMode != texture->texturePriv().mipColorMode()) {
        texture->texturePriv().dirtyMipMaps(true);
    }

    // If the mips aren't dirty, we're done:
    if (!texture->texturePriv().mipMapsAreDirty()) {
        return;
    }

    // If we created a rt/tex and rendered to it without using a texture and now we're texturing
    // from the rt it will still be the last bound texture, but it needs resolving.
    GrGLRenderTarget* texRT = static_cast<GrGLRenderTarget*>(texture->asRenderTarget());
    if (texRT) {
        this->onResolveRenderTarget(texRT);
    }

    GrGLenum target = texture->target();
    this->setScratchTextureUnit();
    GL_CALL(BindTexture(target, texture->textureID()));

    // Configure sRGB decode, if necessary. This state is the only thing needed for the driver
    // call (glGenerateMipmap) to work correctly. Our manual method dirties other state, too.
    if (this->glCaps().srgbDecodeDisableSupport() && GrPixelConfigIsSRGB(texture->config())) {
        GrGLenum srgbDecode = allowSRGBInputs ? GR_GL_DECODE_EXT : GR_GL_SKIP_DECODE_EXT;
        // Command buffer's sRGB decode extension doesn't influence mipmap generation correctly.
        // If we set this to skip_decode, it appears to suppress sRGB -> Linear for each downsample,
        // but not the Linear -> sRGB when writing the next level. The result is that mip-chains
        // get progressively brighter as you go down. Forcing this to 'decode' gives predictable
        // (and only slightly incorrect) results. See crbug.com/655247 (~comment 28)
        if (!this->glCaps().srgbDecodeDisableAffectsMipmaps()) {
            srgbDecode = GR_GL_DECODE_EXT;
        }
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SRGB_DECODE_EXT, srgbDecode));
    }

    // Either do manual mipmap generation or (if that fails), just rely on the driver:
    if (!this->generateMipmap(texture, allowSRGBInputs)) {
        GL_CALL(GenerateMipmap(target));
    }

    texture->texturePriv().dirtyMipMaps(false);
    texture->texturePriv().setMaxMipMapLevel(SkMipMap::ComputeLevelCount(
        texture->width(), texture->height()));
    texture->texturePriv().setMipColorMode(colorMode);

    // We have potentially set lots of state on the texture. Easiest to dirty it all:
    texture->textureParamsModified();
}

void GrGLGpu::setTextureSwizzle(int unitIdx, GrGLenum target, const GrGLenum swizzle[]) {
    this->setTextureUnit(unitIdx);
    if (this->glStandard() == kGLES_GrGLStandard) {
        // ES3 added swizzle support but not GL_TEXTURE_SWIZZLE_RGBA.
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_R, swizzle[0]));
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_G, swizzle[1]));
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_B, swizzle[2]));
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_A, swizzle[3]));
    } else {
        GR_STATIC_ASSERT(sizeof(swizzle[0]) == sizeof(GrGLint));
        GL_CALL(TexParameteriv(target, GR_GL_TEXTURE_SWIZZLE_RGBA,
                               reinterpret_cast<const GrGLint*>(swizzle)));
    }
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
    fHWBoundTextureUniqueIDs[lastUnitIdx].makeInvalid();
}

// Determines whether glBlitFramebuffer could be used between src and dst by onCopySurface.
static inline bool can_blit_framebuffer_for_copy_surface(const GrSurface* dst,
                                                         const GrSurface* src,
                                                         const SkIRect& srcRect,
                                                         const SkIPoint& dstPoint,
                                                         const GrGLGpu* gpu) {
    auto blitFramebufferFlags = gpu->glCaps().blitFramebufferSupportFlags();
    if (!gpu->glCaps().canConfigBeFBOColorAttachment(dst->config()) ||
        !gpu->glCaps().canConfigBeFBOColorAttachment(src->config())) {
        return false;
    }
    // Blits are not allowed between int color buffers and float/fixed color buffers. GrGpu should
    // have filtered such cases out.
    SkASSERT(GrPixelConfigIsSint(dst->config()) == GrPixelConfigIsSint(src->config()));
    const GrGLTexture* dstTex = static_cast<const GrGLTexture*>(dst->asTexture());
    const GrGLTexture* srcTex = static_cast<const GrGLTexture*>(dst->asTexture());
    const GrRenderTarget* dstRT = dst->asRenderTarget();
    const GrRenderTarget* srcRT = src->asRenderTarget();
    if (dstTex && dstTex->target() != GR_GL_TEXTURE_2D) {
        return false;
    }
    if (srcTex && srcTex->target() != GR_GL_TEXTURE_2D) {
        return false;
    }
    if (GrGLCaps::kNoSupport_BlitFramebufferFlag & blitFramebufferFlags) {
        return false;
    }
    if (GrGLCaps::kNoScalingOrMirroring_BlitFramebufferFlag & blitFramebufferFlags) {
        // We would mirror to compensate for origin changes. Note that copySurface is
        // specified such that the src and dst rects are the same.
        if (dst->origin() != src->origin()) {
            return false;
        }
    }
    if (GrGLCaps::kResolveMustBeFull_BlitFrambufferFlag & blitFramebufferFlags) {
        if (srcRT && srcRT->numColorSamples()) {
            if (dstRT && !dstRT->numColorSamples()) {
                return false;
            }
            if (SkRect::Make(srcRect) != srcRT->getBoundsRect()) {
                return false;
            }
        }
    }
    if (GrGLCaps::kNoMSAADst_BlitFramebufferFlag & blitFramebufferFlags) {
        if (dstRT && dstRT->numColorSamples() > 0) {
            return false;
        }
    }    
    if (GrGLCaps::kNoFormatConversion_BlitFramebufferFlag & blitFramebufferFlags) {
        if (dst->config() != src->config()) {
            return false;
        }
    } else if (GrGLCaps::kNoFormatConversionForMSAASrc_BlitFramebufferFlag & blitFramebufferFlags) {
        const GrRenderTarget* srcRT = src->asRenderTarget();
        if (srcRT && srcRT->numColorSamples() && dst->config() != src->config()) {
            return false;
        }
    }
    if (GrGLCaps::kRectsMustMatchForMSAASrc_BlitFramebufferFlag & blitFramebufferFlags) {
        if (srcRT && srcRT->numColorSamples()) {
            if (dstPoint.fX != srcRect.fLeft || dstPoint.fY != srcRect.fTop) {
                return false;
            }
            if (dst->origin() != src->origin()) {
                return false;
            }
        }
    }
    return true;
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
    if (gpu->glCaps().canConfigBeFBOColorAttachment(src->config()) &&
        (!srcTex || srcTex->target() == GR_GL_TEXTURE_2D) && dstTex->target() == GR_GL_TEXTURE_2D &&
        dst->origin() == src->origin()) {
        return true;
    } else {
        return false;
    }
}

// If a temporary FBO was created, its non-zero ID is returned. The viewport that the copy rect is
// relative to is output.
void GrGLGpu::bindSurfaceFBOForPixelOps(GrSurface* surface, GrGLenum fboTarget, GrGLIRect* viewport,
                                        TempFBOTarget tempFBOTarget) {
    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    if (!rt) {
        SkASSERT(surface->asTexture());
        GrGLTexture* texture = static_cast<GrGLTexture*>(surface->asTexture());
        GrGLuint texID = texture->textureID();
        GrGLenum target = texture->target();
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
        texture->baseLevelWasBoundToFBO();
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

void GrGLGpu::unbindTextureFBOForPixelOps(GrGLenum fboTarget, GrSurface* surface) {
    // bindSurfaceFBOForPixelOps temporarily binds textures that are not render targets to
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

bool GrGLGpu::onCopySurface(GrSurface* dst,
                            GrSurface* src,
                            const SkIRect& srcRect,
                            const SkIPoint& dstPoint) {
    // None of our copy methods can handle a swizzle. TODO: Make copySurfaceAsDraw handle the
    // swizzle.
    if (this->caps()->shaderCaps()->configOutputSwizzle(src->config()) !=
        this->caps()->shaderCaps()->configOutputSwizzle(dst->config())) {
        return false;
    }
    // Don't prefer copying as a draw if the dst doesn't already have a FBO object.
    bool preferCopy = SkToBool(dst->asRenderTarget());
    if (preferCopy && src->asTexture()) {
        if (this->copySurfaceAsDraw(dst, src, srcRect, dstPoint)) {
            return true;
        }
    }

    if (can_copy_texsubimage(dst, src, this)) {
        this->copySurfaceAsCopyTexSubImage(dst, src, srcRect, dstPoint);
        return true;
    }

    if (can_blit_framebuffer_for_copy_surface(dst, src, srcRect, dstPoint, this)) {
        return this->copySurfaceAsBlitFramebuffer(dst, src, srcRect, dstPoint);
    }

    if (!preferCopy && src->asTexture()) {
        if (this->copySurfaceAsDraw(dst, src, srcRect, dstPoint)) {
            return true;
        }
    }

    return false;
}

bool GrGLGpu::createCopyProgram(GrTexture* srcTex) {
    TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("skia"), "GrGLGpu::createCopyProgram()");

    int progIdx = TextureToCopyProgramIdx(srcTex);
    const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();
    GrSLType samplerType = srcTex->texturePriv().samplerType();

    if (!fCopyProgramArrayBuffer) {
        static const GrGLfloat vdata[] = {
            0, 0,
            0, 1,
            1, 0,
            1, 1
        };
        fCopyProgramArrayBuffer.reset(GrGLBuffer::Create(this, sizeof(vdata), kVertex_GrBufferType,
                                                         kStatic_GrAccessPattern, vdata));
    }
    if (!fCopyProgramArrayBuffer) {
        return false;
    }

    SkASSERT(!fCopyPrograms[progIdx].fProgram);
    GL_CALL_RET(fCopyPrograms[progIdx].fProgram, CreateProgram());
    if (!fCopyPrograms[progIdx].fProgram) {
        return false;
    }

    const char* version = shaderCaps->versionDeclString();
    GrShaderVar aVertex("a_vertex", kVec2f_GrSLType, GrShaderVar::kIn_TypeModifier);
    GrShaderVar uTexCoordXform("u_texCoordXform", kVec4f_GrSLType,
                               GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uPosXform("u_posXform", kVec4f_GrSLType, GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uTexture("u_texture", samplerType, GrShaderVar::kUniform_TypeModifier);
    GrShaderVar vTexCoord("v_texCoord", kVec2f_GrSLType, GrShaderVar::kOut_TypeModifier);
    GrShaderVar oFragColor("o_FragColor", kVec4f_GrSLType, GrShaderVar::kOut_TypeModifier);

    SkString vshaderTxt(version);
    if (shaderCaps->noperspectiveInterpolationSupport()) {
        if (const char* extension = shaderCaps->noperspectiveInterpolationExtensionString()) {
            vshaderTxt.appendf("#extension %s : require\n", extension);
        }
        vTexCoord.addModifier("noperspective");
    }

    aVertex.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";");
    uTexCoordXform.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";");
    uPosXform.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";");
    vTexCoord.appendDecl(shaderCaps, &vshaderTxt);
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
    if (shaderCaps->noperspectiveInterpolationSupport()) {
        if (const char* extension = shaderCaps->noperspectiveInterpolationExtensionString()) {
            fshaderTxt.appendf("#extension %s : require\n", extension);
        }
    }
    if (samplerType == kTextureExternalSampler_GrSLType) {
        fshaderTxt.appendf("#extension %s : require\n",
                           shaderCaps->externalTextureExtensionString());
    }
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kMedium_GrSLPrecision, *shaderCaps,
                                                 &fshaderTxt);
    vTexCoord.setTypeModifier(GrShaderVar::kIn_TypeModifier);
    vTexCoord.appendDecl(shaderCaps, &fshaderTxt);
    fshaderTxt.append(";");
    uTexture.appendDecl(shaderCaps, &fshaderTxt);
    fshaderTxt.append(";");
    fshaderTxt.appendf(
        "// Copy Program FS\n"
        "void main() {"
        "  sk_FragColor = texture(u_texture, v_texCoord);"
        "}"
    );

    const char* str;
    GrGLint length;

    str = vshaderTxt.c_str();
    length = SkToInt(vshaderTxt.size());
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    SkSL::Program::Inputs inputs;
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, &str, &length, 1,
                                                  &fStats, settings, &inputs);
    SkASSERT(inputs.isEmpty());

    str = fshaderTxt.c_str();
    length = SkToInt(fshaderTxt.size());
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, &str, &length, 1,
                                                  &fStats, settings, &inputs);
    SkASSERT(inputs.isEmpty());

    GL_CALL(LinkProgram(fCopyPrograms[progIdx].fProgram));

    GL_CALL_RET(fCopyPrograms[progIdx].fTextureUniform,
                GetUniformLocation(fCopyPrograms[progIdx].fProgram, "u_texture"));
    GL_CALL_RET(fCopyPrograms[progIdx].fPosXformUniform,
                GetUniformLocation(fCopyPrograms[progIdx].fProgram, "u_posXform"));
    GL_CALL_RET(fCopyPrograms[progIdx].fTexCoordXformUniform,
                GetUniformLocation(fCopyPrograms[progIdx].fProgram, "u_texCoordXform"));

    GL_CALL(BindAttribLocation(fCopyPrograms[progIdx].fProgram, 0, "a_vertex"));

    GL_CALL(DeleteShader(vshader));
    GL_CALL(DeleteShader(fshader));

    return true;
}

bool GrGLGpu::createMipmapProgram(int progIdx) {
    const bool oddWidth = SkToBool(progIdx & 0x2);
    const bool oddHeight = SkToBool(progIdx & 0x1);
    const int numTaps = (oddWidth ? 2 : 1) * (oddHeight ? 2 : 1);

    const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();

    SkASSERT(!fMipmapPrograms[progIdx].fProgram);
    GL_CALL_RET(fMipmapPrograms[progIdx].fProgram, CreateProgram());
    if (!fMipmapPrograms[progIdx].fProgram) {
        return false;
    }

    const char* version = shaderCaps->versionDeclString();
    GrShaderVar aVertex("a_vertex", kVec2f_GrSLType, GrShaderVar::kIn_TypeModifier);
    GrShaderVar uTexCoordXform("u_texCoordXform", kVec4f_GrSLType,
                               GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uTexture("u_texture", kTexture2DSampler_GrSLType,
                         GrShaderVar::kUniform_TypeModifier);
    // We need 1, 2, or 4 texture coordinates (depending on parity of each dimension):
    GrShaderVar vTexCoords[] = {
        GrShaderVar("v_texCoord0", kVec2f_GrSLType, GrShaderVar::kOut_TypeModifier),
        GrShaderVar("v_texCoord1", kVec2f_GrSLType, GrShaderVar::kOut_TypeModifier),
        GrShaderVar("v_texCoord2", kVec2f_GrSLType, GrShaderVar::kOut_TypeModifier),
        GrShaderVar("v_texCoord3", kVec2f_GrSLType, GrShaderVar::kOut_TypeModifier),
    };
    GrShaderVar oFragColor("o_FragColor", kVec4f_GrSLType,GrShaderVar::kOut_TypeModifier);

    SkString vshaderTxt(version);
    if (shaderCaps->noperspectiveInterpolationSupport()) {
        if (const char* extension = shaderCaps->noperspectiveInterpolationExtensionString()) {
            vshaderTxt.appendf("#extension %s : require\n", extension);
        }
        vTexCoords[0].addModifier("noperspective");
        vTexCoords[1].addModifier("noperspective");
        vTexCoords[2].addModifier("noperspective");
        vTexCoords[3].addModifier("noperspective");
    }

    aVertex.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";");
    uTexCoordXform.appendDecl(shaderCaps, &vshaderTxt);
    vshaderTxt.append(";");
    for (int i = 0; i < numTaps; ++i) {
        vTexCoords[i].appendDecl(shaderCaps, &vshaderTxt);
        vshaderTxt.append(";");
    }

    vshaderTxt.append(
        "// Mipmap Program VS\n"
        "void main() {"
        "  gl_Position.xy = a_vertex * vec2(2, 2) - vec2(1, 1);"
        "  gl_Position.zw = vec2(0, 1);"
    );

    // Insert texture coordinate computation:
    if (oddWidth && oddHeight) {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy * u_texCoordXform.yw;"
            "  v_texCoord1 = a_vertex.xy * u_texCoordXform.yw + vec2(u_texCoordXform.x, 0);"
            "  v_texCoord2 = a_vertex.xy * u_texCoordXform.yw + vec2(0, u_texCoordXform.z);"
            "  v_texCoord3 = a_vertex.xy * u_texCoordXform.yw + u_texCoordXform.xz;"
        );
    } else if (oddWidth) {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy * vec2(u_texCoordXform.y, 1);"
            "  v_texCoord1 = a_vertex.xy * vec2(u_texCoordXform.y, 1) + vec2(u_texCoordXform.x, 0);"
        );
    } else if (oddHeight) {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy * vec2(1, u_texCoordXform.w);"
            "  v_texCoord1 = a_vertex.xy * vec2(1, u_texCoordXform.w) + vec2(0, u_texCoordXform.z);"
        );
    } else {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy;"
        );
    }

    vshaderTxt.append("}");

    SkString fshaderTxt(version);
    if (shaderCaps->noperspectiveInterpolationSupport()) {
        if (const char* extension = shaderCaps->noperspectiveInterpolationExtensionString()) {
            fshaderTxt.appendf("#extension %s : require\n", extension);
        }
    }
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kMedium_GrSLPrecision, *shaderCaps,
                                                 &fshaderTxt);
    for (int i = 0; i < numTaps; ++i) {
        vTexCoords[i].setTypeModifier(GrShaderVar::kIn_TypeModifier);
        vTexCoords[i].appendDecl(shaderCaps, &fshaderTxt);
        fshaderTxt.append(";");
    }
    uTexture.appendDecl(shaderCaps, &fshaderTxt);
    fshaderTxt.append(";");
    fshaderTxt.append(
        "// Mipmap Program FS\n"
        "void main() {"
    );

    if (oddWidth && oddHeight) {
        fshaderTxt.append(
            "  sk_FragColor = (texture(u_texture, v_texCoord0) + "
            "                  texture(u_texture, v_texCoord1) + "
            "                  texture(u_texture, v_texCoord2) + "
            "                  texture(u_texture, v_texCoord3)) * 0.25;"
        );
    } else if (oddWidth || oddHeight) {
        fshaderTxt.append(
            "  sk_FragColor = (texture(u_texture, v_texCoord0) + "
            "                  texture(u_texture, v_texCoord1)) * 0.5;"
        );
    } else {
        fshaderTxt.append(
            "  sk_FragColor = texture(u_texture, v_texCoord0);"
        );
    }

    fshaderTxt.append("}");

    const char* str;
    GrGLint length;

    str = vshaderTxt.c_str();
    length = SkToInt(vshaderTxt.size());
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    SkSL::Program::Inputs inputs;
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, &str, &length, 1,
                                                  &fStats, settings, &inputs);
    SkASSERT(inputs.isEmpty());

    str = fshaderTxt.c_str();
    length = SkToInt(fshaderTxt.size());
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, &str, &length, 1,
                                                  &fStats, settings, &inputs);
    SkASSERT(inputs.isEmpty());

    GL_CALL(LinkProgram(fMipmapPrograms[progIdx].fProgram));

    GL_CALL_RET(fMipmapPrograms[progIdx].fTextureUniform,
                GetUniformLocation(fMipmapPrograms[progIdx].fProgram, "u_texture"));
    GL_CALL_RET(fMipmapPrograms[progIdx].fTexCoordXformUniform,
                GetUniformLocation(fMipmapPrograms[progIdx].fProgram, "u_texCoordXform"));

    GL_CALL(BindAttribLocation(fMipmapPrograms[progIdx].fProgram, 0, "a_vertex"));

    GL_CALL(DeleteShader(vshader));
    GL_CALL(DeleteShader(fshader));

    return true;
}

bool GrGLGpu::createStencilClipClearProgram() {
    TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("skia"), "GrGLGpu::createStencilClipClearProgram()");

    if (!fStencilClipClearArrayBuffer) {
        static const GrGLfloat vdata[] = {-1, -1, 1, -1, -1, 1, 1, 1};
        fStencilClipClearArrayBuffer.reset(GrGLBuffer::Create(
                this, sizeof(vdata), kVertex_GrBufferType, kStatic_GrAccessPattern, vdata));
        if (!fStencilClipClearArrayBuffer) {
            return false;
        }
    }

    SkASSERT(!fStencilClipClearProgram);
    GL_CALL_RET(fStencilClipClearProgram, CreateProgram());
    if (!fStencilClipClearProgram) {
        return false;
    }

    GrShaderVar aVertex("a_vertex", kVec2f_GrSLType, GrShaderVar::kIn_TypeModifier);
    const char* version = this->caps()->shaderCaps()->versionDeclString();

    SkString vshaderTxt(version);
    aVertex.appendDecl(this->caps()->shaderCaps(), &vshaderTxt);
    vshaderTxt.append(";");
    vshaderTxt.append(
            "// Stencil Clip Clear Program VS\n"
            "void main() {"
            "  gl_Position = vec4(a_vertex.x, a_vertex.y, 0, 1);"
            "}");

    SkString fshaderTxt(version);
    GrGLSLAppendDefaultFloatPrecisionDeclaration(kMedium_GrSLPrecision,
                                                 *this->caps()->shaderCaps(),
                                                 &fshaderTxt);
    fshaderTxt.appendf(
            "// Stencil Clip Clear Program FS\n"
            "void main() {"
            "  sk_FragColor = vec4(0);"
            "}");

    const char* str;
    GrGLint length;

    str = vshaderTxt.c_str();
    length = SkToInt(vshaderTxt.size());
    SkSL::Program::Settings settings;
    settings.fCaps = this->caps()->shaderCaps();
    SkSL::Program::Inputs inputs;
    GrGLuint vshader =
            GrGLCompileAndAttachShader(*fGLContext, fStencilClipClearProgram, GR_GL_VERTEX_SHADER,
                                       &str, &length, 1, &fStats, settings, &inputs);
    SkASSERT(inputs.isEmpty());

    str = fshaderTxt.c_str();
    length = SkToInt(fshaderTxt.size());
    GrGLuint fshader =
            GrGLCompileAndAttachShader(*fGLContext, fStencilClipClearProgram, GR_GL_FRAGMENT_SHADER,
                                       &str, &length, 1, &fStats, settings, &inputs);
    SkASSERT(inputs.isEmpty());

    GL_CALL(LinkProgram(fStencilClipClearProgram));

    GL_CALL(BindAttribLocation(fStencilClipClearProgram, 0, "a_vertex"));

    GL_CALL(DeleteShader(vshader));
    GL_CALL(DeleteShader(fshader));

    return true;
}

void GrGLGpu::clearStencilClipAsDraw(const GrFixedClip& clip, bool insideStencilMask,
                                     GrRenderTarget* rt) {
    // TODO: This should swizzle the output to match dst's config, though it is a debugging
    // visualization.

    this->handleDirtyContext();
    if (!fStencilClipClearProgram) {
        if (!this->createStencilClipClearProgram()) {
            SkDebugf("Failed to create stencil clip clear program.\n");
            return;
        }
    }

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(rt->asRenderTarget());
    this->flushRenderTarget(glRT, nullptr);

    GL_CALL(UseProgram(fStencilClipClearProgram));
    fHWProgramID = fStencilClipClearProgram;

    fHWVertexArrayState.setVertexArrayID(this, 0);

    GrGLAttribArrayState* attribs = fHWVertexArrayState.bindInternalVertexArray(this);
    attribs->enableVertexArrays(this, 1);
    attribs->set(this, 0, fStencilClipClearArrayBuffer.get(), kVec2f_GrVertexAttribType,
                 2 * sizeof(GrGLfloat), 0);

    GrXferProcessor::BlendInfo blendInfo;
    blendInfo.reset();
    this->flushBlend(blendInfo, GrSwizzle::RGBA());
    this->flushColorWrite(false);
    this->flushHWAAState(glRT, false, false);
    this->flushScissor(clip.scissorState(), glRT->getViewport(), glRT->origin());
    this->flushWindowRectangles(clip.windowRectsState(), glRT);
    GrStencilAttachment* sb = rt->renderTargetPriv().getStencilAttachment();
    // This should only be called internally when we know we have a stencil buffer.
    SkASSERT(sb);
    GrStencilSettings settings = GrStencilSettings(
            *GrStencilSettings::SetClipBitSettings(insideStencilMask), false, sb->bits());
    this->flushStencil(settings);
    GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
}


bool GrGLGpu::copySurfaceAsDraw(GrSurface* dst,
                                GrSurface* src,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint) {
    GrGLTexture* srcTex = static_cast<GrGLTexture*>(src->asTexture());
    int progIdx = TextureToCopyProgramIdx(srcTex);

    if (!fCopyPrograms[progIdx].fProgram) {
        if (!this->createCopyProgram(srcTex)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }

    int w = srcRect.width();
    int h = srcRect.height();

    GrSamplerParams params(SkShader::kClamp_TileMode, GrSamplerParams::kNone_FilterMode);
    this->bindTexture(0, params, true, srcTex);

    GrGLIRect dstVP;
    this->bindSurfaceFBOForPixelOps(dst, GR_GL_FRAMEBUFFER, &dstVP, kDst_TempFBOTarget);
    this->flushViewport(dstVP);
    fHWBoundRenderTargetUniqueID.makeInvalid();

    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY, w, h);

    GL_CALL(UseProgram(fCopyPrograms[progIdx].fProgram));
    fHWProgramID = fCopyPrograms[progIdx].fProgram;

    fHWVertexArrayState.setVertexArrayID(this, 0);

    GrGLAttribArrayState* attribs = fHWVertexArrayState.bindInternalVertexArray(this);
    attribs->enableVertexArrays(this, 1);
    attribs->set(this, 0, fCopyProgramArrayBuffer.get(), kVec2f_GrVertexAttribType,
                 2 * sizeof(GrGLfloat), 0);

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
    int sw = src->width();
    int sh = src->height();
    if (kBottomLeft_GrSurfaceOrigin == src->origin()) {
        sy0 = sh - sy0;
        sy1 = sh - sy1;
    }
    // src rect edges in normalized texture space (0 to 1)
    sx0 /= sw;
    sx1 /= sw;
    sy0 /= sh;
    sy1 /= sh;

    GL_CALL(Uniform4f(fCopyPrograms[progIdx].fPosXformUniform, dx1 - dx0, dy1 - dy0, dx0, dy0));
    GL_CALL(Uniform4f(fCopyPrograms[progIdx].fTexCoordXformUniform,
                      sx1 - sx0, sy1 - sy0, sx0, sy0));
    GL_CALL(Uniform1i(fCopyPrograms[progIdx].fTextureUniform, 0));

    GrXferProcessor::BlendInfo blendInfo;
    blendInfo.reset();
    this->flushBlend(blendInfo, GrSwizzle::RGBA());
    this->flushColorWrite(true);
    this->flushHWAAState(nullptr, false, false);
    this->disableScissor();
    this->disableWindowRectangles();
    this->disableStencil();
    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(true);
    }

    GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, dst);
    this->didWriteToSurface(dst, &dstRect);

    return true;
}

void GrGLGpu::copySurfaceAsCopyTexSubImage(GrSurface* dst,
                                           GrSurface* src,
                                           const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_copy_texsubimage(dst, src, this));
    GrGLIRect srcVP;
    this->bindSurfaceFBOForPixelOps(src, GR_GL_FRAMEBUFFER, &srcVP, kSrc_TempFBOTarget);
    GrGLTexture* dstTex = static_cast<GrGLTexture *>(dst->asTexture());
    SkASSERT(dstTex);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();
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
    this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, src);
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    this->didWriteToSurface(dst, &dstRect);
}

bool GrGLGpu::copySurfaceAsBlitFramebuffer(GrSurface* dst,
                                           GrSurface* src,
                                           const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_blit_framebuffer_for_copy_surface(dst, src, srcRect, dstPoint, this));
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    if (dst == src) {
        if (SkIRect::IntersectsNoEmptyCheck(dstRect, srcRect)) {
            return false;
        }
    }

    GrGLIRect dstVP;
    GrGLIRect srcVP;
    this->bindSurfaceFBOForPixelOps(dst, GR_GL_DRAW_FRAMEBUFFER, &dstVP, kDst_TempFBOTarget);
    this->bindSurfaceFBOForPixelOps(src, GR_GL_READ_FRAMEBUFFER, &srcVP, kSrc_TempFBOTarget);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();
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
    this->disableWindowRectangles();

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
    this->unbindTextureFBOForPixelOps(GR_GL_DRAW_FRAMEBUFFER, dst);
    this->unbindTextureFBOForPixelOps(GR_GL_READ_FRAMEBUFFER, src);
    this->didWriteToSurface(dst, &dstRect);
    return true;
}

// Manual implementation of mipmap generation, to work around driver bugs w/sRGB.
// Uses draw calls to do a series of downsample operations to successive mips.
// If this returns false, then the calling code falls back to using glGenerateMipmap.
bool GrGLGpu::generateMipmap(GrGLTexture* texture, bool gammaCorrect) {
    SkASSERT(!GrPixelConfigIsSint(texture->config()));
    // Our iterative downsample requires the ability to limit which level we're sampling:
    if (!this->glCaps().doManualMipmapping()) {
        return false;
    }

    // Mipmaps are only supported on 2D textures:
    if (GR_GL_TEXTURE_2D != texture->target()) {
        return false;
    }

    // We need to be able to render to the texture for this to work:
    if (!this->glCaps().canConfigBeFBOColorAttachment(texture->config())) {
        return false;
    }

    // If we're mipping an sRGB texture, we need to ensure FB sRGB is correct:
    if (GrPixelConfigIsSRGB(texture->config())) {
        // If we have write-control, just set the state that we want:
        if (this->glCaps().srgbWriteControl()) {
            this->flushFramebufferSRGB(gammaCorrect);
        } else if (!gammaCorrect) {
            // If we don't have write-control we can't do non-gamma-correct mipmapping:
            return false;
        }
    }

    int width = texture->width();
    int height = texture->height();
    int levelCount = SkMipMap::ComputeLevelCount(width, height) + 1;

    // Define all mips, if we haven't previously done so:
    if (0 == texture->texturePriv().maxMipMapLevel()) {
        GrGLenum internalFormat;
        GrGLenum externalFormat;
        GrGLenum externalType;
        if (!this->glCaps().getTexImageFormats(texture->config(), texture->config(),
                                               &internalFormat, &externalFormat, &externalType)) {
            return false;
        }

        for (GrGLint level = 1; level < levelCount; ++level) {
            // Define the next mip:
            width = SkTMax(1, width / 2);
            height = SkTMax(1, height / 2);
            GL_ALLOC_CALL(this->glInterface(), TexImage2D(GR_GL_TEXTURE_2D, level, internalFormat,
                                                          width, height, 0,
                                                          externalFormat, externalType, nullptr));
        }
    }

    // Create (if necessary), then bind temporary FBO:
    if (0 == fTempDstFBOID) {
        GL_CALL(GenFramebuffers(1, &fTempDstFBOID));
    }
    GL_CALL(BindFramebuffer(GR_GL_FRAMEBUFFER, fTempDstFBOID));
    fHWBoundRenderTargetUniqueID.makeInvalid();

    // Bind the texture, to get things configured for filtering.
    // We'll be changing our base level further below:
    this->setTextureUnit(0);
    GrSamplerParams params(SkShader::kClamp_TileMode, GrSamplerParams::kBilerp_FilterMode);
    this->bindTexture(0, params, gammaCorrect, texture);

    // Vertex data:
    if (!fMipmapProgramArrayBuffer) {
        static const GrGLfloat vdata[] = {
            0, 0,
            0, 1,
            1, 0,
            1, 1
        };
        fMipmapProgramArrayBuffer.reset(GrGLBuffer::Create(this, sizeof(vdata),
                                                           kVertex_GrBufferType,
                                                           kStatic_GrAccessPattern, vdata));
    }
    if (!fMipmapProgramArrayBuffer) {
        return false;
    }

    fHWVertexArrayState.setVertexArrayID(this, 0);

    GrGLAttribArrayState* attribs = fHWVertexArrayState.bindInternalVertexArray(this);
    attribs->enableVertexArrays(this, 1);
    attribs->set(this, 0, fMipmapProgramArrayBuffer.get(), kVec2f_GrVertexAttribType,
                 2 * sizeof(GrGLfloat), 0);

    // Set "simple" state once:
    GrXferProcessor::BlendInfo blendInfo;
    blendInfo.reset();
    this->flushBlend(blendInfo, GrSwizzle::RGBA());
    this->flushColorWrite(true);
    this->flushHWAAState(nullptr, false, false);
    this->disableScissor();
    this->disableWindowRectangles();
    this->disableStencil();

    // Do all the blits:
    width = texture->width();
    height = texture->height();
    GrGLIRect viewport;
    viewport.fLeft = 0;
    viewport.fBottom = 0;
    for (GrGLint level = 1; level < levelCount; ++level) {
        // Get and bind the program for this particular downsample (filter shape can vary):
        int progIdx = TextureSizeToMipmapProgramIdx(width, height);
        if (!fMipmapPrograms[progIdx].fProgram) {
            if (!this->createMipmapProgram(progIdx)) {
                SkDebugf("Failed to create mipmap program.\n");
                return false;
            }
        }
        GL_CALL(UseProgram(fMipmapPrograms[progIdx].fProgram));
        fHWProgramID = fMipmapPrograms[progIdx].fProgram;

        // Texcoord uniform is expected to contain (1/w, (w-1)/w, 1/h, (h-1)/h)
        const float invWidth = 1.0f / width;
        const float invHeight = 1.0f / height;
        GL_CALL(Uniform4f(fMipmapPrograms[progIdx].fTexCoordXformUniform,
                          invWidth, (width - 1) * invWidth, invHeight, (height - 1) * invHeight));
        GL_CALL(Uniform1i(fMipmapPrograms[progIdx].fTextureUniform, 0));

        // Only sample from previous mip
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_BASE_LEVEL, level - 1));

        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                     GR_GL_TEXTURE_2D, texture->textureID(), level));

        width = SkTMax(1, width / 2);
        height = SkTMax(1, height / 2);
        viewport.fWidth = width;
        viewport.fHeight = height;
        this->flushViewport(viewport);

        GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    }

    // Unbind:
    GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                 GR_GL_TEXTURE_2D, 0, 0));

    return true;
}

void GrGLGpu::onQueryMultisampleSpecs(GrRenderTarget* rt, const GrStencilSettings& stencil,
                                      int* effectiveSampleCnt, SamplePattern* samplePattern) {
    SkASSERT(GrFSAAType::kMixedSamples != rt->fsaaType() ||
             rt->renderTargetPriv().getStencilAttachment() || stencil.isDisabled());

    this->flushStencil(stencil);
    this->flushHWAAState(rt, true, !stencil.isDisabled());
    this->flushRenderTarget(static_cast<GrGLRenderTarget*>(rt), &SkIRect::EmptyIRect());

    if (0 != this->caps()->maxRasterSamples()) {
        GR_GL_GetIntegerv(this->glInterface(), GR_GL_EFFECTIVE_RASTER_SAMPLES, effectiveSampleCnt);
    } else {
        GR_GL_GetIntegerv(this->glInterface(), GR_GL_SAMPLES, effectiveSampleCnt);
    }

    SkASSERT(*effectiveSampleCnt >= rt->numStencilSamples());

    if (this->caps()->sampleLocationsSupport()) {
        samplePattern->reset(*effectiveSampleCnt);
        for (int i = 0; i < *effectiveSampleCnt; ++i) {
            GrGLfloat pos[2];
            GL_CALL(GetMultisamplefv(GR_GL_SAMPLE_POSITION, i, pos));
            if (kTopLeft_GrSurfaceOrigin == rt->origin()) {
                (*samplePattern)[i].set(pos[0], pos[1]);
            } else {
                (*samplePattern)[i].set(pos[0], 1 - pos[1]);
            }
        }
    }
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
                                                         GrPixelConfig config, bool /*isRT*/) {
    if (!this->caps()->isConfigTexturable(config)) {
        return false;
    }
    std::unique_ptr<GrGLTextureInfo> info = skstd::make_unique<GrGLTextureInfo>();
    info->fTarget = GR_GL_TEXTURE_2D;
    info->fID = 0;
    GL_CALL(GenTextures(1, &info->fID));
    GL_CALL(ActiveTexture(GR_GL_TEXTURE0));
    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(BindTexture(info->fTarget, info->fID));
    fHWBoundTextureUniqueIDs[0].makeInvalid();
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_WRAP_S, GR_GL_CLAMP_TO_EDGE));
    GL_CALL(TexParameteri(info->fTarget, GR_GL_TEXTURE_WRAP_T, GR_GL_CLAMP_TO_EDGE));

    GrGLenum internalFormat;
    GrGLenum externalFormat;
    GrGLenum externalType;

    if (!this->glCaps().getTexImageFormats(config, config, &internalFormat, &externalFormat,
                                           &externalType)) {
        return reinterpret_cast<GrBackendObject>(nullptr);
    }

    GL_CALL(TexImage2D(info->fTarget, 0, internalFormat, w, h, 0, externalFormat,
                       externalType, pixels));

    return reinterpret_cast<GrBackendObject>(info.release());
}

bool GrGLGpu::isTestingOnlyBackendTexture(GrBackendObject id) const {
    GrGLuint texID = reinterpret_cast<const GrGLTextureInfo*>(id)->fID;

    GrGLboolean result;
    GL_CALL_RET(result, IsTexture(texID));

    return (GR_GL_TRUE == result);
}

void GrGLGpu::deleteTestingOnlyBackendTexture(GrBackendObject id, bool abandonTexture) {
    std::unique_ptr<const GrGLTextureInfo> info(reinterpret_cast<const GrGLTextureInfo*>(id));
    GrGLuint texID = info->fID;

    if (!abandonTexture) {
        GL_CALL(DeleteTextures(1, &texID));
    }
}

void GrGLGpu::resetShaderCacheForTesting() const {
    fProgramCache->abandon();
}

///////////////////////////////////////////////////////////////////////////////

GrGLAttribArrayState* GrGLGpu::HWVertexArrayState::bindInternalVertexArray(GrGLGpu* gpu,
                                                                           const GrBuffer* ibuf) {
    GrGLAttribArrayState* attribState;

    if (gpu->glCaps().isCoreProfile()) {
        if (!fCoreProfileVertexArray) {
            GrGLuint arrayID;
            GR_GL_CALL(gpu->glInterface(), GenVertexArrays(1, &arrayID));
            int attrCount = gpu->glCaps().maxVertexAttributes();
            fCoreProfileVertexArray = new GrGLVertexArray(arrayID, attrCount);
        }
        if (ibuf) {
            attribState = fCoreProfileVertexArray->bindWithIndexBuffer(gpu, ibuf);
        } else {
            attribState = fCoreProfileVertexArray->bind(gpu);
        }
    } else {
        if (ibuf) {
            // bindBuffer implicitly binds VAO 0 when binding an index buffer.
            gpu->bindBuffer(kIndex_GrBufferType, ibuf);
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

bool GrGLGpu::onIsACopyNeededForTextureParams(GrTextureProxy* proxy,
                                              const GrSamplerParams& textureParams,
                                              GrTextureProducer::CopyParams* copyParams,
                                              SkScalar scaleAdjust[2]) const {
    const GrTexture* texture = proxy->priv().peekTexture();
    if (!texture) {
        // The only way to get and EXTERNAL or RECTANGLE texture in Ganesh is to wrap them.
        // In that case the proxy should already be instantiated.
        return false;
    }

    if (textureParams.isTiled() ||
        GrSamplerParams::kMipMap_FilterMode == textureParams.filterMode()) {
        const GrGLTexture* glTexture = static_cast<const GrGLTexture*>(texture);
        if (GR_GL_TEXTURE_EXTERNAL == glTexture->target() ||
            GR_GL_TEXTURE_RECTANGLE == glTexture->target()) {
            copyParams->fFilter = GrSamplerParams::kNone_FilterMode;
            copyParams->fWidth = texture->width();
            copyParams->fHeight = texture->height();
            return true;
        }
    }
    return false;
}

GrFence SK_WARN_UNUSED_RESULT GrGLGpu::insertFence() {
    SkASSERT(this->caps()->fenceSyncSupport());
    GrGLsync sync;
    GL_CALL_RET(sync, FenceSync(GR_GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    GR_STATIC_ASSERT(sizeof(GrFence) >= sizeof(GrGLsync));
    return (GrFence)sync;
}

bool GrGLGpu::waitFence(GrFence fence, uint64_t timeout) {
    GrGLenum result;
    GL_CALL_RET(result, ClientWaitSync((GrGLsync)fence, GR_GL_SYNC_FLUSH_COMMANDS_BIT, timeout));
    return (GR_GL_CONDITION_SATISFIED == result);
}

void GrGLGpu::deleteFence(GrFence fence) const {
    this->deleteSync((GrGLsync)fence);
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrGLGpu::makeSemaphore(bool isOwned) {
    SkASSERT(this->caps()->fenceSyncSupport());
    return GrGLSemaphore::Make(this, isOwned);
}

sk_sp<GrSemaphore> GrGLGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                 GrWrapOwnership ownership) {
    SkASSERT(this->caps()->fenceSyncSupport());
    return GrGLSemaphore::MakeWrapped(this, semaphore.glSync(), ownership);
}


void GrGLGpu::insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) {
    GrGLSemaphore* glSem = static_cast<GrGLSemaphore*>(semaphore.get());

    GrGLsync sync;
    GL_CALL_RET(sync, FenceSync(GR_GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    glSem->setSync(sync);

    if (flush) {
        GL_CALL(Flush());
    }
}

void GrGLGpu::waitSemaphore(sk_sp<GrSemaphore> semaphore) {
    GrGLSemaphore* glSem = static_cast<GrGLSemaphore*>(semaphore.get());

    GL_CALL(WaitSync(glSem->sync(), 0, GR_GL_TIMEOUT_IGNORED));
}

void GrGLGpu::deleteSync(GrGLsync sync) const {
    GL_CALL(DeleteSync(sync));
}

sk_sp<GrSemaphore> GrGLGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    // Set up a semaphore to be signaled once the data is ready, and flush GL
    sk_sp<GrSemaphore> semaphore = this->makeSemaphore(true);
    this->insertSemaphore(semaphore, true);

    return semaphore;
}

int GrGLGpu::TextureToCopyProgramIdx(GrTexture* texture) {
    switch (texture->texturePriv().samplerType()) {
        case kTexture2DSampler_GrSLType:
            return 0;
        case kITexture2DSampler_GrSLType:
            return 1;
        case kTexture2DRectSampler_GrSLType:
            return 2;
        case kTextureExternalSampler_GrSLType:
            return 3;
        default:
            SkFAIL("Unexpected samper type");
            return 0;
    }
}
