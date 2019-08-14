/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPixmap.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkHalf.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrCpuBuffer.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/gl/GrGLBuffer.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLGpuCommandBuffer.h"
#include "src/gpu/gl/GrGLSemaphore.h"
#include "src/gpu/gl/GrGLStencilAttachment.h"
#include "src/gpu/gl/GrGLTextureRenderTarget.h"
#include "src/gpu/gl/builders/GrGLShaderStringBuilder.h"
#include "src/sksl/SkSLCompiler.h"

#include <cmath>

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->glInterface(), RET, X)

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
    GR_GL_HSL_LUMINOSITY,

    // Illegal... needs to map to something.
    GR_GL_FUNC_ADD,
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

    // Illegal... needs to map to something.
    GR_GL_ZERO,
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

        // Illegal.
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

//////////////////////////////////////////////////////////////////////////////

static int gl_target_to_binding_index(GrGLenum target) {
    switch (target) {
        case GR_GL_TEXTURE_2D:
            return 0;
        case GR_GL_TEXTURE_RECTANGLE:
            return 1;
        case GR_GL_TEXTURE_EXTERNAL:
            return 2;
    }
    SK_ABORT("Unexpected GL texture target.");
}

GrGpuResource::UniqueID GrGLGpu::TextureUnitBindings::boundID(GrGLenum target) const {
    return fTargetBindings[gl_target_to_binding_index(target)].fBoundResourceID;
}

bool GrGLGpu::TextureUnitBindings::hasBeenModified(GrGLenum target) const {
    return fTargetBindings[gl_target_to_binding_index(target)].fHasBeenModified;
}

void GrGLGpu::TextureUnitBindings::setBoundID(GrGLenum target, GrGpuResource::UniqueID resourceID) {
    int targetIndex = gl_target_to_binding_index(target);
    fTargetBindings[targetIndex].fBoundResourceID = resourceID;
    fTargetBindings[targetIndex].fHasBeenModified = true;
}

void GrGLGpu::TextureUnitBindings::invalidateForScratchUse(GrGLenum target) {
    this->setBoundID(target, GrGpuResource::UniqueID());
}

void GrGLGpu::TextureUnitBindings::invalidateAllTargets(bool markUnmodified) {
    for (auto& targetBinding : fTargetBindings) {
        targetBinding.fBoundResourceID.makeInvalid();
        if (markUnmodified) {
            targetBinding.fHasBeenModified = false;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

static GrGLenum filter_to_gl_mag_filter(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest: return GR_GL_NEAREST;
        case GrSamplerState::Filter::kBilerp:  return GR_GL_LINEAR;
        case GrSamplerState::Filter::kMipMap:  return GR_GL_LINEAR;
    }
    SK_ABORT("Unknown filter");
}

static GrGLenum filter_to_gl_min_filter(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest: return GR_GL_NEAREST;
        case GrSamplerState::Filter::kBilerp:  return GR_GL_LINEAR;
        case GrSamplerState::Filter::kMipMap:  return GR_GL_LINEAR_MIPMAP_LINEAR;
    }
    SK_ABORT("Unknown filter");
}

static inline GrGLenum wrap_mode_to_gl_wrap(GrSamplerState::WrapMode wrapMode,
                                            const GrCaps& caps) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:        return GR_GL_CLAMP_TO_EDGE;
        case GrSamplerState::WrapMode::kRepeat:       return GR_GL_REPEAT;
        case GrSamplerState::WrapMode::kMirrorRepeat: return GR_GL_MIRRORED_REPEAT;
        case GrSamplerState::WrapMode::kClampToBorder:
            // May not be supported but should have been caught earlier
            SkASSERT(caps.clampToBorderSupport());
            return GR_GL_CLAMP_TO_BORDER;
    }
    SK_ABORT("Unknown wrap mode");
}

///////////////////////////////////////////////////////////////////////////////

class GrGLGpu::SamplerObjectCache {
public:
    SamplerObjectCache(GrGLGpu* gpu) : fGpu(gpu) {
        fNumTextureUnits = fGpu->glCaps().shaderCaps()->maxFragmentSamplers();
        fHWBoundSamplers.reset(new GrGLuint[fNumTextureUnits]);
        std::fill_n(fHWBoundSamplers.get(), fNumTextureUnits, 0);
        std::fill_n(fSamplers, kNumSamplers, 0);
    }

    ~SamplerObjectCache() {
        if (!fNumTextureUnits) {
            // We've already been abandoned.
            return;
        }
        for (GrGLuint sampler : fSamplers) {
            // The spec states that "zero" values should be silently ignored, however they still
            // trigger GL errors on some NVIDIA platforms.
            if (sampler) {
                GR_GL_CALL(fGpu->glInterface(), DeleteSamplers(1, &sampler));
            }
        }
    }

    void bindSampler(int unitIdx, const GrSamplerState& state) {
        int index = StateToIndex(state);
        if (!fSamplers[index]) {
            GrGLuint s;
            GR_GL_CALL(fGpu->glInterface(), GenSamplers(1, &s));
            if (!s) {
                return;
            }
            fSamplers[index] = s;
            auto minFilter = filter_to_gl_min_filter(state.filter());
            auto magFilter = filter_to_gl_mag_filter(state.filter());
            auto wrapX = wrap_mode_to_gl_wrap(state.wrapModeX(), fGpu->glCaps());
            auto wrapY = wrap_mode_to_gl_wrap(state.wrapModeY(), fGpu->glCaps());
            GR_GL_CALL(fGpu->glInterface(),
                       SamplerParameteri(s, GR_GL_TEXTURE_MIN_FILTER, minFilter));
            GR_GL_CALL(fGpu->glInterface(),
                       SamplerParameteri(s, GR_GL_TEXTURE_MAG_FILTER, magFilter));
            GR_GL_CALL(fGpu->glInterface(), SamplerParameteri(s, GR_GL_TEXTURE_WRAP_S, wrapX));
            GR_GL_CALL(fGpu->glInterface(), SamplerParameteri(s, GR_GL_TEXTURE_WRAP_T, wrapY));
        }
        if (fHWBoundSamplers[unitIdx] != fSamplers[index]) {
            GR_GL_CALL(fGpu->glInterface(), BindSampler(unitIdx, fSamplers[index]));
            fHWBoundSamplers[unitIdx] = fSamplers[index];
        }
    }

    void invalidateBindings() {
        // When we have sampler support we always use samplers. So setting these to zero will cause
        // a rebind on next usage.
        std::fill_n(fHWBoundSamplers.get(), fNumTextureUnits, 0);
    }

    void abandon() {
        fHWBoundSamplers.reset();
        fNumTextureUnits = 0;
    }

    void release() {
        if (!fNumTextureUnits) {
            // We've already been abandoned.
            return;
        }
        GR_GL_CALL(fGpu->glInterface(), DeleteSamplers(kNumSamplers, fSamplers));
        std::fill_n(fSamplers, kNumSamplers, 0);
        // Deleting a bound sampler implicitly binds sampler 0.
        std::fill_n(fHWBoundSamplers.get(), fNumTextureUnits, 0);
    }

private:
    static int StateToIndex(const GrSamplerState& state) {
        int filter = static_cast<int>(state.filter());
        SkASSERT(filter >= 0 && filter < 3);
        int wrapX = static_cast<int>(state.wrapModeX());
        SkASSERT(wrapX >= 0 && wrapX < 4);
        int wrapY = static_cast<int>(state.wrapModeY());
        SkASSERT(wrapY >= 0 && wrapY < 4);
        int idx = 16 * filter + 4 * wrapX + wrapY;
        SkASSERT(idx < kNumSamplers);
        return idx;
    }

    GrGLGpu* fGpu;
    static constexpr int kNumSamplers = 48;
    std::unique_ptr<GrGLuint[]> fHWBoundSamplers;
    GrGLuint fSamplers[kNumSamplers];
    int fNumTextureUnits;
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<GrGpu> GrGLGpu::Make(sk_sp<const GrGLInterface> interface, const GrContextOptions& options,
                           GrContext* context) {
    if (!interface) {
        interface = GrGLMakeNativeInterface();
        // For clients that have written their own GrGLCreateNativeInterface and haven't yet updated
        // to GrGLMakeNativeInterface.
        if (!interface) {
            interface = sk_ref_sp(GrGLCreateNativeInterface());
        }
        if (!interface) {
            return nullptr;
        }
    }
#ifdef USE_NSIGHT
    const_cast<GrContextOptions&>(options).fSuppressPathRendering = true;
#endif
    auto glContext = GrGLContext::Make(std::move(interface), options);
    if (!glContext) {
        return nullptr;
    }
    return sk_sp<GrGpu>(new GrGLGpu(std::move(glContext), context));
}

GrGLGpu::GrGLGpu(std::unique_ptr<GrGLContext> ctx, GrContext* context)
        : GrGpu(context)
        , fGLContext(std::move(ctx))
        , fProgramCache(new ProgramCache(this))
        , fHWProgramID(0)
        , fTempSrcFBOID(0)
        , fTempDstFBOID(0)
        , fStencilClearFBOID(0) {
    SkASSERT(fGLContext);
    GrGLClearErr(this->glInterface());
    fCaps = sk_ref_sp(fGLContext->caps());

    fHWTextureUnitBindings.reset(this->numTextureUnits());

    this->hwBufferState(GrGpuBufferType::kVertex)->fGLTarget = GR_GL_ARRAY_BUFFER;
    this->hwBufferState(GrGpuBufferType::kIndex)->fGLTarget = GR_GL_ELEMENT_ARRAY_BUFFER;
    if (GrGLCaps::kChromium_TransferBufferType == this->glCaps().transferBufferType()) {
        this->hwBufferState(GrGpuBufferType::kXferCpuToGpu)->fGLTarget =
                GR_GL_PIXEL_UNPACK_TRANSFER_BUFFER_CHROMIUM;
        this->hwBufferState(GrGpuBufferType::kXferGpuToCpu)->fGLTarget =
                GR_GL_PIXEL_PACK_TRANSFER_BUFFER_CHROMIUM;
    } else {
        this->hwBufferState(GrGpuBufferType::kXferCpuToGpu)->fGLTarget = GR_GL_PIXEL_UNPACK_BUFFER;
        this->hwBufferState(GrGpuBufferType::kXferGpuToCpu)->fGLTarget = GR_GL_PIXEL_PACK_BUFFER;
    }
    for (int i = 0; i < kGrGpuBufferTypeCount; ++i) {
        fHWBufferState[i].invalidate();
    }
    GR_STATIC_ASSERT(4 == SK_ARRAY_COUNT(fHWBufferState));

    if (this->glCaps().shaderCaps()->pathRenderingSupport()) {
        fPathRendering.reset(new GrGLPathRendering(this));
    }

    if (this->glCaps().samplerObjectSupport()) {
        fSamplerObjectCache.reset(new SamplerObjectCache(this));
    }
}

GrGLGpu::~GrGLGpu() {
    // Ensure any GrGpuResource objects get deleted first, since they may require a working GrGLGpu
    // to release the resources held by the objects themselves.
    fPathRendering.reset();
    fCopyProgramArrayBuffer.reset();
    fMipmapProgramArrayBuffer.reset();

    fHWProgram.reset();
    if (fHWProgramID) {
        // detach the current program so there is no confusion on OpenGL's part
        // that we want it to be deleted
        GL_CALL(UseProgram(0));
    }

    if (fTempSrcFBOID) {
        this->deleteFramebuffer(fTempSrcFBOID);
    }
    if (fTempDstFBOID) {
        this->deleteFramebuffer(fTempDstFBOID);
    }
    if (fStencilClearFBOID) {
        this->deleteFramebuffer(fStencilClearFBOID);
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

    delete fProgramCache;
    fSamplerObjectCache.reset();
}

void GrGLGpu::disconnect(DisconnectType type) {
    INHERITED::disconnect(type);
    if (DisconnectType::kCleanup == type) {
        if (fHWProgramID) {
            GL_CALL(UseProgram(0));
        }
        if (fTempSrcFBOID) {
            this->deleteFramebuffer(fTempSrcFBOID);
        }
        if (fTempDstFBOID) {
            this->deleteFramebuffer(fTempDstFBOID);
        }
        if (fStencilClearFBOID) {
            this->deleteFramebuffer(fStencilClearFBOID);
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

        if (fSamplerObjectCache) {
            fSamplerObjectCache->release();
        }
    } else {
        if (fProgramCache) {
            fProgramCache->abandon();
        }
        if (fSamplerObjectCache) {
            fSamplerObjectCache->abandon();
        }
    }

    fHWProgram.reset();
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

        this->hwBufferState(GrGpuBufferType::kXferCpuToGpu)->invalidate();
        this->hwBufferState(GrGpuBufferType::kXferGpuToCpu)->invalidate();

        if (GR_IS_GR_GL(this->glStandard())) {
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

        if (GR_IS_GR_GL_ES(this->glStandard()) &&
            this->glCaps().fbFetchRequiresEnablePerSample()) {
            // The arm extension requires specifically enabling MSAA fetching per sample.
            // On some devices this may have a perf hit.  Also multiple render targets are disabled
            GL_CALL(Enable(GR_GL_FETCH_PER_SAMPLE));
        }
        fHWWriteToColor = kUnknown_TriState;
        // we only ever use lines in hairline mode
        GL_CALL(LineWidth(1));
        GL_CALL(Disable(GR_GL_DITHER));

        fHWClearColor[0] = fHWClearColor[1] = fHWClearColor[2] = fHWClearColor[3] = SK_FloatNaN;
    }

    if (resetBits & kMSAAEnable_GrGLBackendState) {
        fMSAAEnabled = kUnknown_TriState;

        if (this->caps()->mixedSamplesSupport()) {
            // The skia blend modes all use premultiplied alpha and therefore expect RGBA coverage
            // modulation. This state has no effect when not rendering to a mixed sampled target.
            GL_CALL(CoverageModulation(GR_GL_RGBA));
        }
    }

    fHWActiveTextureUnitIdx = -1; // invalid
    fLastPrimitiveType = static_cast<GrPrimitiveType>(-1);

    if (resetBits & kTextureBinding_GrGLBackendState) {
        for (int s = 0; s < this->numTextureUnits(); ++s) {
            fHWTextureUnitBindings[s].invalidateAllTargets(false);
        }
        if (fSamplerObjectCache) {
            fSamplerObjectCache->invalidateBindings();
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
        this->hwBufferState(GrGpuBufferType::kVertex)->invalidate();
        this->hwBufferState(GrGpuBufferType::kIndex)->invalidate();
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
        if (this->caps()->writePixelsRowBytesSupport()) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
        }
        if (this->glCaps().readPixelsRowBytesSupport()) {
            GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
        }
        if (this->glCaps().packFlipYSupport()) {
            GL_CALL(PixelStorei(GR_GL_PACK_REVERSE_ROW_ORDER, GR_GL_FALSE));
        }
    }

    if (resetBits & kProgram_GrGLBackendState) {
        fHWProgramID = 0;
        fHWProgram.reset();
    }
    ++fResetTimestampForTextureParameters;
}

static bool check_backend_texture(const GrBackendTexture& backendTex, const GrColorType colorType,
                                  const GrGLCaps& caps, GrGLTexture::Desc* desc,
                                  bool skipRectTexSupportCheck = false) {
    GrGLTextureInfo info;
    if (!backendTex.getGLTextureInfo(&info) || !info.fID || !info.fFormat) {
        return false;
    }

    desc->fSize = {backendTex.width(), backendTex.height()};
    desc->fTarget = info.fTarget;
    desc->fID = info.fID;
    desc->fFormat = GrGLFormatFromGLEnum(info.fFormat);

    if (desc->fFormat == GrGLFormat::kUnknown) {
        return false;
    }
    if (GR_GL_TEXTURE_EXTERNAL == desc->fTarget) {
        if (!caps.shaderCaps()->externalTextureSupport()) {
            return false;
        }
    } else if (GR_GL_TEXTURE_RECTANGLE == desc->fTarget) {
        if (!caps.rectangleTextureSupport() && !skipRectTexSupportCheck) {
            return false;
        }
    } else if (GR_GL_TEXTURE_2D != desc->fTarget) {
        return false;
    }
    if (backendTex.isProtected()) {
        // Not supported in GL backend at this time.
        return false;
    }

    desc->fConfig = caps.getConfigFromBackendFormat(backendTex.getBackendFormat(), colorType);
    SkASSERT(desc->fConfig != kUnknown_GrPixelConfig);

    return true;
}

sk_sp<GrTexture> GrGLGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                               GrColorType colorType, GrWrapOwnership ownership,
                                               GrWrapCacheable cacheable, GrIOType ioType) {
    GrGLTexture::Desc desc;
    if (!check_backend_texture(backendTex, colorType, this->glCaps(), &desc)) {
        return nullptr;
    }

    if (kBorrow_GrWrapOwnership == ownership) {
        desc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    } else {
        desc.fOwnership = GrBackendObjectOwnership::kOwned;
    }

    GrMipMapsStatus mipMapsStatus = backendTex.hasMipMaps() ? GrMipMapsStatus::kValid
                                                            : GrMipMapsStatus::kNotAllocated;

    auto texture = GrGLTexture::MakeWrapped(this, mipMapsStatus, desc,
                                            backendTex.getGLTextureParams(), cacheable, ioType);
    // We don't know what parameters are already set on wrapped textures.
    texture->textureParamsModified();
    return std::move(texture);
}

sk_sp<GrTexture> GrGLGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                         int sampleCnt,
                                                         GrColorType colorType,
                                                         GrWrapOwnership ownership,
                                                         GrWrapCacheable cacheable) {
    GrGLTexture::Desc desc;
    if (!check_backend_texture(backendTex, colorType, this->glCaps(), &desc)) {
        return nullptr;
    }

    // We don't support rendering to a EXTERNAL texture.
    if (GR_GL_TEXTURE_EXTERNAL == desc.fTarget) {
        return nullptr;
    }

    const GrGLCaps& caps = this->glCaps();

    if (!caps.isFormatRenderable(desc.fFormat, sampleCnt)) {
        return nullptr;
    }

    if (kBorrow_GrWrapOwnership == ownership) {
        desc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    } else {
        desc.fOwnership = GrBackendObjectOwnership::kOwned;
    }


    sampleCnt = caps.getRenderTargetSampleCount(sampleCnt, desc.fFormat);
    SkASSERT(sampleCnt);

    GrGLRenderTarget::IDs rtIDs;
    if (!this->createRenderTargetObjects(desc, sampleCnt, &rtIDs)) {
        return nullptr;
    }

    GrMipMapsStatus mipMapsStatus = backendTex.hasMipMaps() ? GrMipMapsStatus::kDirty
                                                            : GrMipMapsStatus::kNotAllocated;

    sk_sp<GrGLTextureRenderTarget> texRT(GrGLTextureRenderTarget::MakeWrapped(
            this, sampleCnt, desc, backendTex.getGLTextureParams(), rtIDs, cacheable,
            mipMapsStatus));
    texRT->baseLevelWasBoundToFBO();
    // We don't know what parameters are already set on wrapped textures.
    texRT->textureParamsModified();
    return std::move(texRT);
}

sk_sp<GrRenderTarget> GrGLGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT,
                                                         GrColorType grColorType) {
    GrGLFramebufferInfo info;
    if (!backendRT.getGLFramebufferInfo(&info)) {
        return nullptr;
    }

    if (backendRT.isProtected()) {
        // Not supported in GL at this time.
        return nullptr;
    }

    const auto format = backendRT.getBackendFormat().asGLFormat();
    if (!this->glCaps().isFormatRenderable(format, backendRT.sampleCnt())) {
        return nullptr;
    }

    GrGLRenderTarget::IDs rtIDs;
    rtIDs.fRTFBOID = info.fFBOID;
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    rtIDs.fRTFBOOwnership = GrBackendObjectOwnership::kBorrowed;

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendRT.getBackendFormat(),
                                                                    grColorType);
    SkASSERT(kUnknown_GrPixelConfig != config);

    const auto size = SkISize::Make(backendRT.width(), backendRT.height());
    int sampleCount = this->glCaps().getRenderTargetSampleCount(backendRT.sampleCnt(), format);

    return GrGLRenderTarget::MakeWrapped(this, size, format, config, sampleCount, rtIDs,
                                         backendRT.stencilBits());
}

sk_sp<GrRenderTarget> GrGLGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                  int sampleCnt,
                                                                  GrColorType colorType) {
    GrGLTexture::Desc desc;
    // We do not check whether texture rectangle is supported by Skia - if the caller provided us
    // with a texture rectangle,we assume the necessary support exists.
    if (!check_backend_texture(tex, colorType, this->glCaps(), &desc, true)) {
        return nullptr;
    }

    if (!this->glCaps().isFormatRenderable(desc.fFormat, sampleCnt)) {
        return nullptr;
    }

    const int sampleCount = this->glCaps().getRenderTargetSampleCount(sampleCnt, desc.fFormat);
    GrGLRenderTarget::IDs rtIDs;
    if (!this->createRenderTargetObjects(desc, sampleCount, &rtIDs)) {
        return nullptr;
    }
    return GrGLRenderTarget::MakeWrapped(this, desc.fSize, desc.fFormat, desc.fConfig, sampleCount,
                                         rtIDs, 0);
}

static bool check_write_and_transfer_input(GrGLTexture* glTex) {
    if (!glTex) {
        return false;
    }

    // Write or transfer of pixels is not implemented for TEXTURE_EXTERNAL textures
    if (GR_GL_TEXTURE_EXTERNAL == glTex->target()) {
        return false;
    }

    return true;
}

bool GrGLGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                            GrColorType surfaceColorType, GrColorType srcColorType,
                            const GrMipLevel texels[], int mipLevelCount) {
    auto glTex = static_cast<GrGLTexture*>(surface->asTexture());

    if (!check_write_and_transfer_input(glTex)) {
        return false;
    }

    this->bindTextureToScratchUnit(glTex->target(), glTex->textureID());

    SkASSERT(!GrGLFormatIsCompressed(glTex->format()));
    return this->uploadTexData(glTex->format(), surfaceColorType, glTex->width(), glTex->height(),
                               glTex->target(), kWrite_UploadType, left, top,width,
                               height, srcColorType, texels, mipLevelCount);
}

bool GrGLGpu::onTransferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                                 GrColorType textureColorType, GrColorType bufferColorType,
                                 GrGpuBuffer* transferBuffer, size_t offset, size_t rowBytes) {
    GrGLTexture* glTex = static_cast<GrGLTexture*>(texture);

    // Can't transfer compressed data
    SkASSERT(!GrGLFormatIsCompressed(glTex->format()));

    if (!check_write_and_transfer_input(glTex)) {
        return false;
    }

    static_assert(sizeof(int) == sizeof(int32_t), "");
    if (width <= 0 || height <= 0) {
        return false;
    }

    this->bindTextureToScratchUnit(glTex->target(), glTex->textureID());

    SkASSERT(!transferBuffer->isMapped());
    SkASSERT(!transferBuffer->isCpuBuffer());
    const GrGLBuffer* glBuffer = static_cast<const GrGLBuffer*>(transferBuffer);
    this->bindBuffer(GrGpuBufferType::kXferCpuToGpu, glBuffer);

    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texture->width(), texture->height());
        SkASSERT(bounds.contains(subRect));
    )

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    const size_t trimRowBytes = width * bpp;
    const void* pixels = (void*)offset;
    if (width < 0 || height < 0) {
        return false;
    }

    bool restoreGLRowLength = false;
    if (trimRowBytes != rowBytes) {
        // we should have checked for this support already
        SkASSERT(this->glCaps().writePixelsRowBytesSupport());
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowBytes / bpp));
        restoreGLRowLength = true;
    }

    GrGLFormat textureFormat = glTex->format();
    // Internal format comes from the texture desc.
    GrGLenum internalFormat;
    // External format and type come from the upload data.
    GrGLenum externalFormat = 0;
    GrGLenum externalType = 0;
    this->glCaps().getTexImageFormats(textureFormat, textureColorType, bufferColorType,
                                      &internalFormat, &externalFormat, &externalType);
    if (!externalFormat || !externalType) {
        return false;
    }

    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, 1));
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

bool GrGLGpu::onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                   GrColorType surfaceColorType, GrColorType dstColorType,
                                   GrGpuBuffer* transferBuffer, size_t offset) {
    auto* glBuffer = static_cast<GrGLBuffer*>(transferBuffer);
    this->bindBuffer(GrGpuBufferType::kXferGpuToCpu, glBuffer);
    auto offsetAsPtr = reinterpret_cast<void*>(offset);
    return this->readOrTransferPixelsFrom(surface, left, top, width, height, surfaceColorType,
                                          dstColorType, offsetAsPtr, width);
}

/**
 * Creates storage space for the texture and fills it with texels.
 *
 * @param format         The format of the texture.
 * @param interface      The GL interface in use.
 * @param caps           The capabilities of the GL device.
 * @param target         Which bound texture to target (GR_GL_TEXTURE_2D, e.g.)
 * @param internalFormat The data format used for the internal storage of the texture. May be sized.
 * @param internalFormatForTexStorage The data format used for the TexStorage API. Must be sized.
 * @param externalFormat The data format used for the external storage of the texture.
 * @param externalType   The type of the data used for the external storage of the texture.
 * @param dataBpp        The bytes per pixel of the data in texels.
 * @param texels         The texel data of the texture being created.
 * @param mipLevelCount  Number of mipmap levels
 * @param baseWidth      The width of the texture's base mipmap level
 * @param baseHeight     The height of the texture's base mipmap level
 */
static bool allocate_and_populate_texture(GrGLFormat format,
                                          const GrGLInterface& interface,
                                          const GrGLCaps& caps,
                                          GrGLenum target,
                                          GrGLenum internalFormat,
                                          GrGLenum internalFormatForTexStorage,
                                          GrGLenum externalFormat,
                                          GrGLenum externalType,
                                          size_t dataBpp,
                                          const GrMipLevel texels[],
                                          int mipLevelCount,
                                          int baseWidth,
                                          int baseHeight,
                                          bool* changedUnpackRowLength,
                                          GrMipMapsStatus* mipMapsStatus) {
    CLEAR_ERROR_BEFORE_ALLOC(&interface);

    if (caps.formatSupportsTexStorage(format)) {
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
                    if (mipMapsStatus) {
                        *mipMapsStatus = GrMipMapsStatus::kDirty;
                    }
                    continue;
                }
                int twoToTheMipLevel = 1 << currentMipLevel;
                const int currentWidth = SkTMax(1, baseWidth / twoToTheMipLevel);
                const int currentHeight = SkTMax(1, baseHeight / twoToTheMipLevel);

                if (texels[currentMipLevel].fPixels) {
                    const size_t trimRowBytes = currentWidth * dataBpp;
                    const size_t rowBytes = texels[currentMipLevel].fRowBytes;
                    if (rowBytes != trimRowBytes) {
                        SkASSERT(caps.writePixelsRowBytesSupport());
                        GrGLint rowLength = static_cast<GrGLint>(rowBytes / dataBpp);
                        GR_GL_CALL(&interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
                        *changedUnpackRowLength = true;
                    } else if (*changedUnpackRowLength) {
                        SkASSERT(caps.writePixelsRowBytesSupport());
                        GR_GL_CALL(&interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
                        *changedUnpackRowLength = false;
                    }
                }

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
                const int currentWidth = SkTMax(1, baseWidth / twoToTheMipLevel);
                const int currentHeight = SkTMax(1, baseHeight / twoToTheMipLevel);

                const void* currentMipData = texels[currentMipLevel].fPixels;
                if (currentMipData) {
                    const size_t trimRowBytes = currentWidth * dataBpp;
                    const size_t rowBytes = texels[currentMipLevel].fRowBytes;
                    if (rowBytes != trimRowBytes) {
                        SkASSERT(caps.writePixelsRowBytesSupport());
                        GrGLint rowLength = static_cast<GrGLint>(rowBytes / dataBpp);
                        GR_GL_CALL(&interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
                        *changedUnpackRowLength = true;
                    } else if (*changedUnpackRowLength) {
                        SkASSERT(caps.writePixelsRowBytesSupport());
                        GR_GL_CALL(&interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
                        *changedUnpackRowLength = false;
                    }
                } else if (mipMapsStatus) {
                    *mipMapsStatus = GrMipMapsStatus::kDirty;
                }

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
                                     bool restoreGLRowLength) {
    if (restoreGLRowLength) {
        SkASSERT(caps.writePixelsRowBytesSupport());
        GR_GL_CALL(&interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
}

void GrGLGpu::unbindCpuToGpuXferBuffer() {
    auto* xferBufferState = this->hwBufferState(GrGpuBufferType::kXferCpuToGpu);
    if (!xferBufferState->fBoundBufferUniqueID.isInvalid()) {
        GL_CALL(BindBuffer(xferBufferState->fGLTarget, 0));
        xferBufferState->invalidate();
    }
}

bool GrGLGpu::uploadTexData(GrGLFormat textureFormat, GrColorType textureColorType,
                            int texWidth, int texHeight, GrGLenum target, UploadType uploadType,
                            int left, int top, int width, int height, GrColorType srcColorType,
                            const GrMipLevel texels[],int mipLevelCount,
                            GrMipMapsStatus* mipMapsStatus) {
    // If we're uploading compressed data then we should be using uploadCompressedTexData
    SkASSERT(!GrGLFormatIsCompressed(textureFormat));

    SkASSERT(this->glCaps().isFormatTexturable(textureFormat));
    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texWidth, texHeight);
        SkASSERT(bounds.contains(subRect));
    )
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == texWidth && height == texHeight));

    this->unbindCpuToGpuXferBuffer();

    const GrGLInterface* interface = this->glInterface();
    const GrGLCaps& caps = this->glCaps();

    size_t bpp = GrColorTypeBytesPerPixel(srcColorType);

    if (width == 0 || height == 0) {
        return false;
    }

    // Internal format comes from the texture desc.
    GrGLenum internalFormat;
    // External format and type come from the upload data.
    GrGLenum externalFormat;
    GrGLenum externalType;
    this->glCaps().getTexImageFormats(textureFormat, textureColorType, srcColorType,
                                      &internalFormat, &externalFormat, &externalType);
    if (!externalFormat || !externalType) {
        return false;
    }

    GrGLenum internalFormatForTexStorage = this->glCaps().getSizedInternalFormat(textureFormat);

    /*
     *  Check whether to allocate a temporary buffer for flipping y or
     *  because our srcData has extra bytes past each row. If so, we need
     *  to trim those off here, since GL ES may not let us specify
     *  GL_UNPACK_ROW_LENGTH.
     */
    bool restoreGLRowLength = false;

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    if (mipMapsStatus) {
        *mipMapsStatus = (mipLevelCount > 1) ?
                GrMipMapsStatus::kValid : GrMipMapsStatus::kNotAllocated;
    }

    if (mipLevelCount) {
        GR_GL_CALL(interface, PixelStorei(GR_GL_UNPACK_ALIGNMENT, 1));
    }

    bool succeeded = true;
    if (kNewTexture_UploadType == uploadType) {
        if (0 == left && 0 == top && texWidth == width && texHeight == height) {
            succeeded = allocate_and_populate_texture(
                    textureFormat, *interface, caps, target, internalFormat,
                    internalFormatForTexStorage, externalFormat, externalType, bpp, texels,
                    mipLevelCount, width, height, &restoreGLRowLength, mipMapsStatus);
        } else {
            succeeded = false;
        }
    } else {
        for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
            if (!texels[currentMipLevel].fPixels) {
                if (mipMapsStatus) {
                    *mipMapsStatus = GrMipMapsStatus::kDirty;
                }
                continue;
            }
            int twoToTheMipLevel = 1 << currentMipLevel;
            const int currentWidth = SkTMax(1, width / twoToTheMipLevel);
            const int currentHeight = SkTMax(1, height / twoToTheMipLevel);
            const size_t trimRowBytes = currentWidth * bpp;
            const size_t rowBytes = texels[currentMipLevel].fRowBytes;

            if (caps.writePixelsRowBytesSupport() && rowBytes != trimRowBytes) {
                GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
                GR_GL_CALL(interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
                restoreGLRowLength = true;
            }

            GL_CALL(TexSubImage2D(target,
                                  currentMipLevel,
                                  left, top,
                                  currentWidth,
                                  currentHeight,
                                  externalFormat, externalType,
                                  texels[currentMipLevel].fPixels));
        }
    }

    restore_pixelstore_state(*interface, caps, restoreGLRowLength);

    return succeeded;
}

bool GrGLGpu::uploadCompressedTexData(GrGLFormat format,
                                      SkImage::CompressionType compressionType,
                                      const SkISize& size,
                                      GrGLenum target,
                                      const void* data) {
    SkASSERT(format != GrGLFormat::kUnknown);
    const GrGLCaps& caps = this->glCaps();

    // We only need the internal format for compressed 2D textures.
    GrGLenum internalFormat = caps.getTexImageInternalFormat(format);
    if (!internalFormat) {
        return 0;
    }

    bool useTexStorage = caps.formatSupportsTexStorage(format);

    static constexpr int kMipLevelCount = 1;

    // Make sure that the width and height that we pass to OpenGL
    // is a multiple of the block size.
    size_t dataSize = GrCompressedDataSize(compressionType, size.width(), size.height());

    if (useTexStorage) {
        // We never resize or change formats of textures.
        GL_ALLOC_CALL(
                this->glInterface(),
                TexStorage2D(target, kMipLevelCount, internalFormat, size.width(), size.height()));
        GrGLenum error = CHECK_ALLOC_ERROR(this->glInterface());
        if (error != GR_GL_NO_ERROR) {
            return false;
        }
        GL_CALL(CompressedTexSubImage2D(target,
                                        0,  // level
                                        0,  // left
                                        0,  // top
                                        size.width(),
                                        size.height(),
                                        internalFormat,
                                        SkToInt(dataSize),
                                        data));
    } else {
        GL_ALLOC_CALL(this->glInterface(), CompressedTexImage2D(target,
                                                                0,  // level
                                                                internalFormat,
                                                                size.width(),
                                                                size.height(),
                                                                0,  // border
                                                                SkToInt(dataSize),
                                                                data));

        GrGLenum error = CHECK_ALLOC_ERROR(this->glInterface());
        if (error != GR_GL_NO_ERROR) {
            return false;
        }
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
        case GrGLCaps::kStandard_MSFBOType:
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
            SK_ABORT("Shouldn't be here if we don't support multisampled renderbuffers.");
            break;
    }
    return (GR_GL_NO_ERROR == CHECK_ALLOC_ERROR(ctx.interface()));
}

bool GrGLGpu::createRenderTargetObjects(const GrGLTexture::Desc& desc,
                                        int sampleCount,
                                        GrGLRenderTarget::IDs* rtIDs) {
    rtIDs->fMSColorRenderbufferID = 0;
    rtIDs->fRTFBOID = 0;
    rtIDs->fRTFBOOwnership = GrBackendObjectOwnership::kOwned;
    rtIDs->fTexFBOID = 0;

    GrGLenum status;

    GrGLenum colorRenderbufferFormat = 0; // suppress warning

    if (desc.fFormat == GrGLFormat::kUnknown) {
        goto FAILED;
    }

    if (sampleCount > 1 && GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType()) {
        goto FAILED;
    }

    GL_CALL(GenFramebuffers(1, &rtIDs->fTexFBOID));
    if (!rtIDs->fTexFBOID) {
        goto FAILED;
    }

    // If we are using multisampling we will create two FBOS. We render to one and then resolve to
    // the texture bound to the other. The exception is the IMG multisample extension. With this
    // extension the texture is multisampled when rendered to and then auto-resolves it when it is
    // rendered from.
    if (sampleCount > 1 && this->glCaps().usesMSAARenderBuffers()) {
        GL_CALL(GenFramebuffers(1, &rtIDs->fRTFBOID));
        GL_CALL(GenRenderbuffers(1, &rtIDs->fMSColorRenderbufferID));
        if (!rtIDs->fRTFBOID || !rtIDs->fMSColorRenderbufferID) {
            goto FAILED;
        }
        colorRenderbufferFormat = this->glCaps().getRenderbufferInternalFormat(desc.fFormat);
    } else {
        rtIDs->fRTFBOID = rtIDs->fTexFBOID;
    }

    // below here we may bind the FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();
    if (rtIDs->fRTFBOID != rtIDs->fTexFBOID) {
        SkASSERT(sampleCount > 1);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, rtIDs->fMSColorRenderbufferID));
        if (!renderbuffer_storage_msaa(*fGLContext, sampleCount, colorRenderbufferFormat,
                                       desc.fSize.width(), desc.fSize.height())) {
            goto FAILED;
        }
        this->bindFramebuffer(GR_GL_FRAMEBUFFER, rtIDs->fRTFBOID);
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                        GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_RENDERBUFFER,
                                        rtIDs->fMSColorRenderbufferID));
        if (!this->glCaps().isFormatVerifiedColorAttachment(desc.fFormat)) {
            GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                goto FAILED;
            }
            fGLContext->caps()->markFormatAsValidColorAttachment(desc.fFormat);
        }
    }
    this->bindFramebuffer(GR_GL_FRAMEBUFFER, rtIDs->fTexFBOID);

    if (this->glCaps().usesImplicitMSAAResolve() && sampleCount > 1) {
        GL_CALL(FramebufferTexture2DMultisample(GR_GL_FRAMEBUFFER,
                                                GR_GL_COLOR_ATTACHMENT0,
                                                desc.fTarget,
                                                desc.fID,
                                                0,
                                                sampleCount));
    } else {
        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                     GR_GL_COLOR_ATTACHMENT0,
                                     desc.fTarget,
                                     desc.fID,
                                     0));
    }
    if (!this->glCaps().isFormatVerifiedColorAttachment(desc.fFormat)) {
        GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
        if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
            goto FAILED;
        }
        fGLContext->caps()->markFormatAsValidColorAttachment(desc.fFormat);
    }

    return true;

FAILED:
    if (rtIDs->fMSColorRenderbufferID) {
        GL_CALL(DeleteRenderbuffers(1, &rtIDs->fMSColorRenderbufferID));
    }
    if (rtIDs->fRTFBOID != rtIDs->fTexFBOID) {
        this->deleteFramebuffer(rtIDs->fRTFBOID);
    }
    if (rtIDs->fTexFBOID) {
        this->deleteFramebuffer(rtIDs->fTexFBOID);
    }
    return false;
}

// good to set a break-point here to know when createTexture fails
static sk_sp<GrTexture> return_null_texture() {
//    SkDEBUGFAIL("null texture");
    return nullptr;
}

static GrGLTextureParameters::SamplerOverriddenState set_initial_texture_params(
        const GrGLInterface* interface, GrGLenum target) {
    // Some drivers like to know filter/wrap before seeing glTexImage2D. Some
    // drivers have a bug where an FBO won't be complete if it includes a
    // texture that is not mipmap complete (considering the filter in use).
    GrGLTextureParameters::SamplerOverriddenState state;
    state.fMinFilter = GR_GL_NEAREST;
    state.fMagFilter = GR_GL_NEAREST;
    state.fWrapS = GR_GL_CLAMP_TO_EDGE;
    state.fWrapT = GR_GL_CLAMP_TO_EDGE;
    GR_GL_CALL(interface, TexParameteri(target, GR_GL_TEXTURE_MAG_FILTER, state.fMagFilter));
    GR_GL_CALL(interface, TexParameteri(target, GR_GL_TEXTURE_MIN_FILTER, state.fMinFilter));
    GR_GL_CALL(interface, TexParameteri(target, GR_GL_TEXTURE_WRAP_S, state.fWrapS));
    GR_GL_CALL(interface, TexParameteri(target, GR_GL_TEXTURE_WRAP_T, state.fWrapT));
    return state;
}

sk_sp<GrTexture> GrGLGpu::onCreateTexture(const GrSurfaceDesc& desc,
                                          const GrBackendFormat& format,
                                          GrRenderable renderable,
                                          int renderTargetSampleCnt,
                                          SkBudgeted budgeted,
                                          GrProtected isProtected,
                                          const GrMipLevel texels[],
                                          int mipLevelCount) {
    // We don't support protected textures in GL.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }
    SkASSERT(GrGLCaps::kNone_MSFBOType != this->glCaps().msFBOType() || renderTargetSampleCnt == 1);


    GrMipMapsStatus mipMapsStatus;
    GrGLTextureParameters::SamplerOverriddenState initialState;
    GrGLTexture::Desc texDesc;
    texDesc.fSize = {desc.fWidth, desc.fHeight};
    texDesc.fTarget = GR_GL_TEXTURE_2D;
    texDesc.fFormat = format.asGLFormat();
    texDesc.fConfig = desc.fConfig;
    texDesc.fOwnership = GrBackendObjectOwnership::kOwned;
    SkASSERT(texDesc.fFormat != GrGLFormat::kUnknown);
    SkASSERT(!GrGLFormatIsCompressed(texDesc.fFormat));

    // TODO: Take these as parameters.
    auto textureColorType = GrPixelConfigToColorType(desc.fConfig);
    auto srcColorType = GrPixelConfigToColorType(desc.fConfig);
    texDesc.fID = this->createTexture2D({desc.fWidth, desc.fHeight},
                                        texDesc.fFormat,
                                        renderable,
                                        &initialState,
                                        textureColorType,
                                        srcColorType,
                                        texels,
                                        mipLevelCount,
                                        &mipMapsStatus);

    if (!texDesc.fID) {
        return return_null_texture();
    }

    sk_sp<GrGLTexture> tex;
    if (renderable == GrRenderable::kYes) {
        // unbind the texture from the texture unit before binding it to the frame buffer
        GL_CALL(BindTexture(texDesc.fTarget, 0));
        GrGLRenderTarget::IDs rtIDDesc;

        if (!this->createRenderTargetObjects(texDesc, renderTargetSampleCnt, &rtIDDesc)) {
            GL_CALL(DeleteTextures(1, &texDesc.fID));
            return return_null_texture();
        }
        tex = sk_make_sp<GrGLTextureRenderTarget>(
                this, budgeted, renderTargetSampleCnt, texDesc, rtIDDesc, mipMapsStatus);
        tex->baseLevelWasBoundToFBO();
    } else {
        tex = sk_make_sp<GrGLTexture>(this, budgeted, texDesc, mipMapsStatus);
    }
    // The non-sampler params are still at their default values.
    tex->parameters()->set(&initialState, GrGLTextureParameters::NonsamplerState(),
                           fResetTimestampForTextureParameters);
    bool clearLevelsWithoutData =
            this->caps()->shouldInitializeTextures() && this->glCaps().clearTextureSupport();

    if (clearLevelsWithoutData) {
        static constexpr uint32_t kZero = 0;
        int levelCnt = SkTMax(1, tex->texturePriv().maxMipMapLevel());
        for (int i = 0; i < levelCnt; ++i) {
            if (i >= mipLevelCount || !texels[i].fPixels) {
                GL_CALL(ClearTexImage(tex->textureID(), i, GR_GL_RGBA, GR_GL_UNSIGNED_BYTE,
                                      &kZero));
            }
        }
    }
    return std::move(tex);
}

sk_sp<GrTexture> GrGLGpu::onCreateCompressedTexture(int width, int height,
                                                    const GrBackendFormat& format,
                                                    SkImage::CompressionType compression,
                                                    SkBudgeted budgeted, const void* data) {
    GrGLTextureParameters::SamplerOverriddenState initialState;
    GrGLTexture::Desc desc;
    desc.fSize = {width, height};
    desc.fTarget = GR_GL_TEXTURE_2D;
    desc.fConfig = GrCompressionTypePixelConfig(compression);
    desc.fOwnership = GrBackendObjectOwnership::kOwned;
    desc.fFormat = format.asGLFormat();
    desc.fID = this->createCompressedTexture2D(desc.fSize, desc.fFormat, compression, &initialState,
                                               data);
    if (!desc.fID) {
        return nullptr;
    }
    auto tex = sk_make_sp<GrGLTexture>(this, budgeted, desc, GrMipMapsStatus::kNotAllocated);
    // The non-sampler params are still at their default values.
    tex->parameters()->set(&initialState, GrGLTextureParameters::NonsamplerState(),
                           fResetTimestampForTextureParameters);
    return std::move(tex);
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

int GrGLGpu::getCompatibleStencilIndex(GrGLFormat format) {
    static const int kSize = 16;
    SkASSERT(this->glCaps().canFormatBeFBOColorAttachment(format));

    if (!this->glCaps().hasStencilFormatBeenDeterminedForFormat(format)) {
        // Default to unsupported, set this if we find a stencil format that works.
        int firstWorkingStencilFormatIndex = -1;

        // Create color texture
        GrGLuint colorID = 0;
        GL_CALL(GenTextures(1, &colorID));
        this->bindTextureToScratchUnit(GR_GL_TEXTURE_2D, colorID);
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

        GrGLenum internalFormat = this->glCaps().getTexImageInternalFormat(format);
        GrGLenum externalFormat = this->glCaps().getBaseInternalFormat(format);
        GrGLenum externalType   = this->glCaps().getFormatDefaultExternalType(format);
        if (!internalFormat || !externalFormat || !externalType) {
            return -1;
        }

        this->unbindCpuToGpuXferBuffer();
        CLEAR_ERROR_BEFORE_ALLOC(this->glInterface());
        GL_ALLOC_CALL(this->glInterface(), TexImage2D(GR_GL_TEXTURE_2D,
                                                      0,
                                                      internalFormat,
                                                      kSize,
                                                      kSize,
                                                      0,
                                                      externalFormat,
                                                      externalType,
                                                      nullptr));
        if (GR_GL_NO_ERROR != CHECK_ALLOC_ERROR(this->glInterface())) {
            GL_CALL(DeleteTextures(1, &colorID));
            return -1;
        }

        // unbind the texture from the texture unit before binding it to the frame buffer
        GL_CALL(BindTexture(GR_GL_TEXTURE_2D, 0));

        // Create Framebuffer
        GrGLuint fb = 0;
        GL_CALL(GenFramebuffers(1, &fb));
        this->bindFramebuffer(GR_GL_FRAMEBUFFER, fb);
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
        this->bindFramebuffer(GR_GL_FRAMEBUFFER, 0);
        this->deleteFramebuffer(fb);
        fGLContext->caps()->setStencilFormatIndexForFormat(format, firstWorkingStencilFormatIndex);
    }
    return this->glCaps().getStencilFormatIndexForFormat(format);
}

GrGLuint GrGLGpu::createCompressedTexture2D(
        const SkISize& size,
        GrGLFormat format,
        SkImage::CompressionType compression,
        GrGLTextureParameters::SamplerOverriddenState* initialState,
        const void* data) {
    if (format == GrGLFormat::kUnknown) {
        return 0;
    }
    GrGLuint id = 0;
    GL_CALL(GenTextures(1, &id));
    if (!id) {
        return 0;
    }

    this->bindTextureToScratchUnit(GR_GL_TEXTURE_2D, id);

    *initialState = set_initial_texture_params(this->glInterface(), GR_GL_TEXTURE_2D);

    if (!this->uploadCompressedTexData(format, compression, size, GR_GL_TEXTURE_2D, data)) {
        GL_CALL(DeleteTextures(1, &id));
        return 0;
    }
    return id;
}

GrGLuint GrGLGpu::createTexture2D(const SkISize& size,
                                  GrGLFormat format,
                                  GrRenderable renderable,
                                  GrGLTextureParameters::SamplerOverriddenState* initialState,
                                  GrColorType textureColorType,
                                  GrColorType srcColorType,
                                  const GrMipLevel texels[],
                                  int mipLevelCount,
                                  GrMipMapsStatus* mipMapsStatus) {
    SkASSERT(format != GrGLFormat::kUnknown);
    SkASSERT(!GrGLFormatIsCompressed(format));

    GrGLuint id = 0;
    GL_CALL(GenTextures(1, &id));

    if (!id) {
        return 0;
    }

    this->bindTextureToScratchUnit(GR_GL_TEXTURE_2D, id);

    if (GrRenderable::kYes == renderable && this->glCaps().textureUsageSupport()) {
        // provides a hint about how this texture will be used
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_USAGE, GR_GL_FRAMEBUFFER_ATTACHMENT));
    }

    *initialState = set_initial_texture_params(this->glInterface(), GR_GL_TEXTURE_2D);

    if (!this->uploadTexData(format,
                             textureColorType,
                             size.width(), size.height(),
                             GR_GL_TEXTURE_2D,
                             kNewTexture_UploadType,
                             0,
                             0,
                             size.width(),
                             size.height(),
                             srcColorType,
                             texels,
                             mipLevelCount,
                             mipMapsStatus)) {
        GL_CALL(DeleteTextures(1, &id));
        return 0;
    }
    return id;
}

GrStencilAttachment* GrGLGpu::createStencilAttachmentForRenderTarget(
        const GrRenderTarget* rt, int width, int height, int numStencilSamples) {
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    GrGLStencilAttachment::IDDesc sbDesc;

    int sIdx = this->getCompatibleStencilIndex(rt->backendFormat().asGLFormat());
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
    if (numStencilSamples > 1) {
        SkAssertResult(renderbuffer_storage_msaa(*fGLContext,
                                                 numStencilSamples,
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
                                                               numStencilSamples,
                                                               format);
    return stencil;
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<GrGpuBuffer> GrGLGpu::onCreateBuffer(size_t size, GrGpuBufferType intendedType,
                                           GrAccessPattern accessPattern, const void* data) {
    return GrGLBuffer::Make(this, size, intendedType, accessPattern, data);
}

void GrGLGpu::flushScissor(const GrScissorState& scissorState, int rtWidth, int rtHeight,
                           GrSurfaceOrigin rtOrigin) {
    if (scissorState.enabled()) {
        GrGLIRect scissor;
        scissor.setRelativeTo(rtHeight, scissorState.rect(), rtOrigin);
        // if the scissor fully contains the viewport then we fall through and
        // disable the scissor test.
        if (!scissor.contains(rtWidth, rtHeight)) {
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
                                    const GrGLRenderTarget* rt, GrSurfaceOrigin origin) {
#ifndef USE_NSIGHT
    typedef GrWindowRectsState::Mode Mode;
    SkASSERT(!windowState.enabled() || rt->renderFBOID()); // Window rects can't be used on-screen.
    SkASSERT(windowState.numWindows() <= this->caps()->maxWindowRectangles());

    if (!this->caps()->maxWindowRectangles() ||
        fHWWindowRectsState.knownEqualTo(origin, rt->width(), rt->height(), windowState)) {
        return;
    }

    // This is purely a workaround for a spurious warning generated by gcc. Otherwise the above
    // assert would be sufficient. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=5912
    int numWindows = SkTMin(windowState.numWindows(), int(GrWindowRectangles::kMaxWindows));
    SkASSERT(windowState.numWindows() == numWindows);

    GrGLIRect glwindows[GrWindowRectangles::kMaxWindows];
    const SkIRect* skwindows = windowState.windows().data();
    for (int i = 0; i < numWindows; ++i) {
        glwindows[i].setRelativeTo(rt->height(), skwindows[i], origin);
    }

    GrGLenum glmode = (Mode::kExclusive == windowState.mode()) ? GR_GL_EXCLUSIVE : GR_GL_INCLUSIVE;
    GL_CALL(WindowRectangles(glmode, numWindows, glwindows->asInts()));

    fHWWindowRectsState.set(origin, rt->width(), rt->height(), windowState);
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

void GrGLGpu::resolveAndGenerateMipMapsForProcessorTextures(
        const GrPrimitiveProcessor& primProc,
        const GrPipeline& pipeline,
        const GrTextureProxy* const primProcTextures[],
        int numPrimitiveProcessorTextureSets) {
    auto genLevelsIfNeeded = [this](GrTexture* tex, const GrSamplerState& sampler) {
        SkASSERT(tex);
        auto* rt = tex->asRenderTarget();
        if (rt && rt->needsResolve()) {
            this->resolveRenderTarget(rt);
            // TEMPORARY: MSAA resolve will have dirtied mipmaps. This goes away once we switch
            // to resolving MSAA from the opList as well.
            if (GrSamplerState::Filter::kMipMap == sampler.filter() &&
                (tex->width() != 1 || tex->height() != 1)) {
                SkASSERT(tex->texturePriv().mipMapped() == GrMipMapped::kYes);
                SkASSERT(tex->texturePriv().mipMapsAreDirty());
                this->regenerateMipMapLevels(tex);
            }
        }
        // Ensure mipmaps were all resolved ahead of time by the opList.
        if (GrSamplerState::Filter::kMipMap == sampler.filter() &&
            (tex->width() != 1 || tex->height() != 1)) {
            // There are some cases where we might be given a non-mipmapped texture with a mipmap
            // filter. See skbug.com/7094.
            SkASSERT(tex->texturePriv().mipMapped() != GrMipMapped::kYes ||
                     !tex->texturePriv().mipMapsAreDirty());
        }
    };

    for (int set = 0, tex = 0; set < numPrimitiveProcessorTextureSets; ++set) {
        for (int sampler = 0; sampler < primProc.numTextureSamplers(); ++sampler, ++tex) {
            GrTexture* texture = primProcTextures[tex]->peekTexture();
            genLevelsIfNeeded(texture, primProc.textureSampler(sampler).samplerState());
        }
    }

    GrFragmentProcessor::Iter iter(pipeline);
    while (const GrFragmentProcessor* fp = iter.next()) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const auto& textureSampler = fp->textureSampler(i);
            genLevelsIfNeeded(textureSampler.peekTexture(), textureSampler.samplerState());
        }
    }
}

bool GrGLGpu::flushGLState(GrRenderTarget* renderTarget,
                           GrSurfaceOrigin origin,
                           const GrPrimitiveProcessor& primProc,
                           const GrPipeline& pipeline,
                           const GrPipeline::FixedDynamicState* fixedDynamicState,
                           const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                           int dynamicStateArraysLength,
                           bool willDrawPoints) {
    const GrTextureProxy* const* primProcProxiesForMipRegen = nullptr;
    const GrTextureProxy* const* primProcProxiesToBind = nullptr;
    int numPrimProcTextureSets = 1;  // number of texture per prim proc sampler.
    if (dynamicStateArrays && dynamicStateArrays->fPrimitiveProcessorTextures) {
        primProcProxiesForMipRegen = dynamicStateArrays->fPrimitiveProcessorTextures;
        numPrimProcTextureSets = dynamicStateArraysLength;
    } else if (fixedDynamicState && fixedDynamicState->fPrimitiveProcessorTextures) {
        primProcProxiesForMipRegen = fixedDynamicState->fPrimitiveProcessorTextures;
        primProcProxiesToBind = fixedDynamicState->fPrimitiveProcessorTextures;
    }

    SkASSERT(SkToBool(primProcProxiesForMipRegen) == SkToBool(primProc.numTextureSamplers()));

    sk_sp<GrGLProgram> program(fProgramCache->refProgram(this, renderTarget, origin, primProc,
                                                         primProcProxiesForMipRegen,
                                                         pipeline, willDrawPoints));
    if (!program) {
        GrCapsDebugf(this->caps(), "Failed to create program!\n");
        return false;
    }
    this->resolveAndGenerateMipMapsForProcessorTextures(
            primProc, pipeline, primProcProxiesForMipRegen, numPrimProcTextureSets);

    this->flushProgram(std::move(program));

    // Swizzle the blend to match what the shader will output.
    this->flushBlendAndColorWrite(
            pipeline.getXferProcessor().getBlendInfo(), pipeline.outputSwizzle());

    fHWProgram->updateUniformsAndTextureBindings(renderTarget, origin,
                                                 primProc, pipeline, primProcProxiesToBind);

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(renderTarget);
    GrStencilSettings stencil;
    if (pipeline.isStencilEnabled()) {
        // TODO: attach stencil and create settings during render target flush.
        SkASSERT(glRT->renderTargetPriv().getStencilAttachment());
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                      glRT->renderTargetPriv().numStencilBits());
    }
    this->flushStencil(stencil, origin);
    if (pipeline.isScissorEnabled()) {
        static constexpr SkIRect kBogusScissor{0, 0, 1, 1};
        GrScissorState state(fixedDynamicState ? fixedDynamicState->fScissorRect : kBogusScissor);
        this->flushScissor(state, glRT->width(), glRT->height(), origin);
    } else {
        this->disableScissor();
    }
    this->flushWindowRectangles(pipeline.getWindowRectsState(), glRT, origin);
    this->flushHWAAState(glRT, pipeline.isHWAntialiasState());

    // This must come after textures are flushed because a texture may need
    // to be msaa-resolved (which will modify bound FBO state).
    this->flushRenderTarget(glRT);

    return true;
}

void GrGLGpu::flushProgram(sk_sp<GrGLProgram> program) {
    if (!program) {
        fHWProgram.reset();
        fHWProgramID = 0;
        return;
    }
    SkASSERT((program == fHWProgram) == (fHWProgramID == program->programID()));
    if (program == fHWProgram) {
        return;
    }
    auto id = program->programID();
    SkASSERT(id);
    GL_CALL(UseProgram(id));
    fHWProgram = std::move(program);
    fHWProgramID = id;
}

void GrGLGpu::flushProgram(GrGLuint id) {
    SkASSERT(id);
    if (fHWProgramID == id) {
        SkASSERT(!fHWProgram);
        return;
    }
    fHWProgram.reset();
    GL_CALL(UseProgram(id));
    fHWProgramID = id;
}

void GrGLGpu::setupGeometry(const GrBuffer* indexBuffer,
                            const GrBuffer* vertexBuffer,
                            int baseVertex,
                            const GrBuffer* instanceBuffer,
                            int baseInstance,
                            GrPrimitiveRestart enablePrimitiveRestart) {
    SkASSERT((enablePrimitiveRestart == GrPrimitiveRestart::kNo) || indexBuffer);

    GrGLAttribArrayState* attribState;
    if (indexBuffer) {
        SkASSERT(indexBuffer->isCpuBuffer() ||
                 !static_cast<const GrGpuBuffer*>(indexBuffer)->isMapped());
        attribState = fHWVertexArrayState.bindInternalVertexArray(this, indexBuffer);
    } else {
        attribState = fHWVertexArrayState.bindInternalVertexArray(this);
    }

    int numAttribs = fHWProgram->numVertexAttributes() + fHWProgram->numInstanceAttributes();
    attribState->enableVertexArrays(this, numAttribs, enablePrimitiveRestart);

    if (int vertexStride = fHWProgram->vertexStride()) {
        SkASSERT(vertexBuffer);
        SkASSERT(vertexBuffer->isCpuBuffer() ||
                 !static_cast<const GrGpuBuffer*>(vertexBuffer)->isMapped());
        size_t bufferOffset = baseVertex * static_cast<size_t>(vertexStride);
        for (int i = 0; i < fHWProgram->numVertexAttributes(); ++i) {
            const auto& attrib = fHWProgram->vertexAttribute(i);
            static constexpr int kDivisor = 0;
            attribState->set(this, attrib.fLocation, vertexBuffer, attrib.fCPUType, attrib.fGPUType,
                             vertexStride, bufferOffset + attrib.fOffset, kDivisor);
        }
    }
    if (int instanceStride = fHWProgram->instanceStride()) {
        SkASSERT(instanceBuffer);
        SkASSERT(instanceBuffer->isCpuBuffer() ||
                 !static_cast<const GrGpuBuffer*>(instanceBuffer)->isMapped());
        size_t bufferOffset = baseInstance * static_cast<size_t>(instanceStride);
        int attribIdx = fHWProgram->numVertexAttributes();
        for (int i = 0; i < fHWProgram->numInstanceAttributes(); ++i, ++attribIdx) {
            const auto& attrib = fHWProgram->instanceAttribute(i);
            static constexpr int kDivisor = 1;
            attribState->set(this, attrib.fLocation, instanceBuffer, attrib.fCPUType,
                             attrib.fGPUType, instanceStride, bufferOffset + attrib.fOffset,
                             kDivisor);
        }
    }
}

GrGLenum GrGLGpu::bindBuffer(GrGpuBufferType type, const GrBuffer* buffer) {
    this->handleDirtyContext();

    // Index buffer state is tied to the vertex array.
    if (GrGpuBufferType::kIndex == type) {
        this->bindVertexArray(0);
    }

    auto* bufferState = this->hwBufferState(type);
    if (buffer->isCpuBuffer()) {
        if (!bufferState->fBufferZeroKnownBound) {
            GL_CALL(BindBuffer(bufferState->fGLTarget, 0));
            bufferState->fBufferZeroKnownBound = true;
            bufferState->fBoundBufferUniqueID.makeInvalid();
        }
    } else if (static_cast<const GrGpuBuffer*>(buffer)->uniqueID() !=
               bufferState->fBoundBufferUniqueID) {
        const GrGLBuffer* glBuffer = static_cast<const GrGLBuffer*>(buffer);
        GL_CALL(BindBuffer(bufferState->fGLTarget, glBuffer->bufferID()));
        bufferState->fBufferZeroKnownBound = false;
        bufferState->fBoundBufferUniqueID = glBuffer->uniqueID();
    }

    return bufferState->fGLTarget;
}
void GrGLGpu::disableScissor() {
    if (kNo_TriState != fHWScissorSettings.fEnabled) {
        GL_CALL(Disable(GR_GL_SCISSOR_TEST));
        fHWScissorSettings.fEnabled = kNo_TriState;
        return;
    }
}

void GrGLGpu::clear(const GrFixedClip& clip, const SkPMColor4f& color,
                    GrRenderTarget* target, GrSurfaceOrigin origin) {
    // parent class should never let us get here with no RT
    SkASSERT(target);
    SkASSERT(!this->caps()->performColorClearsAsDraws());
    SkASSERT(!clip.scissorEnabled() || !this->caps()->performPartialClearsAsDraws());

    this->handleDirtyContext();

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);

    if (clip.scissorEnabled()) {
        this->flushRenderTarget(glRT, origin, clip.scissorRect());
    } else {
        this->flushRenderTarget(glRT);
    }
    this->flushScissor(clip.scissorState(), glRT->width(), glRT->height(), origin);
    this->flushWindowRectangles(clip.windowRectsState(), glRT, origin);
    this->flushColorWrite(true);

    GrGLfloat r = color.fR, g = color.fG, b = color.fB, a = color.fA;
    if (this->glCaps().clearToBoundaryValuesIsBroken() &&
        (1 == r || 0 == r) && (1 == g || 0 == g) && (1 == b || 0 == b) && (1 == a || 0 == a)) {
        static const GrGLfloat safeAlpha1 = nextafter(1.f, 2.f);
        static const GrGLfloat safeAlpha0 = nextafter(0.f, -1.f);
        a = (1 == a) ? safeAlpha1 : safeAlpha0;
    }
    this->flushClearColor(r, g, b, a);

    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

void GrGLGpu::clearStencil(GrRenderTarget* target, int clearValue) {
    SkASSERT(!this->caps()->performStencilClearsAsDraws());

    if (!target) {
        return;
    }

    GrStencilAttachment* sb = target->renderTargetPriv().getStencilAttachment();
    // this should only be called internally when we know we have a
    // stencil buffer.
    SkASSERT(sb);

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);
    this->flushRenderTargetNoColorWrites(glRT);

    this->disableScissor();
    this->disableWindowRectangles();

    GL_CALL(StencilMask(0xffffffff));
    GL_CALL(ClearStencil(clearValue));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
    if (!clearValue) {
        sb->cleared();
    }
}

void GrGLGpu::clearStencilClip(const GrFixedClip& clip,
                               bool insideStencilMask,
                               GrRenderTarget* target, GrSurfaceOrigin origin) {
    SkASSERT(target);
    SkASSERT(!this->caps()->performStencilClearsAsDraws());
    this->handleDirtyContext();

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
    this->flushRenderTargetNoColorWrites(glRT);

    this->flushScissor(clip.scissorState(), glRT->width(), glRT->height(), origin);
    this->flushWindowRectangles(clip.windowRectsState(), glRT, origin);

    GL_CALL(StencilMask((uint32_t) clipStencilMask));
    GL_CALL(ClearStencil(value));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

bool GrGLGpu::readOrTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                       GrColorType surfaceColorType, GrColorType dstColorType,
                                       void* offsetOrPtr, int rowWidthInPixels) {
    SkASSERT(surface);

    auto format = surface->backendFormat().asGLFormat();
    GrGLRenderTarget* renderTarget = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    if (!renderTarget && !this->glCaps().isFormatRenderable(format, 1)) {
        return false;
    }
    GrGLenum externalFormat = 0;
    GrGLenum externalType = 0;
    this->glCaps().getReadPixelsFormat(surface->backendFormat().asGLFormat(),
                                       surfaceColorType,
                                       dstColorType,
                                       &externalFormat,
                                       &externalType);
    if (!externalFormat || !externalType) {
        return false;
    }

    if (renderTarget) {
        // resolve the render target if necessary
        switch (renderTarget->getResolveType()) {
            case GrGLRenderTarget::kCantResolve_ResolveType:
                return false;
            case GrGLRenderTarget::kAutoResolves_ResolveType:
                this->flushRenderTargetNoColorWrites(renderTarget);
                break;
            case GrGLRenderTarget::kCanResolve_ResolveType:
                this->onResolveRenderTarget(renderTarget);
                // we don't track the state of the READ FBO ID.
                this->bindFramebuffer(GR_GL_READ_FRAMEBUFFER, renderTarget->textureFBOID());
                break;
            default:
                SK_ABORT("Unknown resolve type");
        }
    } else {
        // Use a temporary FBO.
        this->bindSurfaceFBOForPixelOps(surface, GR_GL_FRAMEBUFFER, kSrc_TempFBOTarget);
        fHWBoundRenderTargetUniqueID.makeInvalid();
    }

    // the read rect is viewport-relative
    GrGLIRect readRect;
    readRect.setRelativeTo(surface->height(), left, top, width, height, kTopLeft_GrSurfaceOrigin);

    // determine if GL can read using the passed rowBytes or if we need a scratch buffer.
    if (rowWidthInPixels != width) {
        SkASSERT(this->glCaps().readPixelsRowBytesSupport());
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, rowWidthInPixels));
    }
    GL_CALL(PixelStorei(GR_GL_PACK_ALIGNMENT, 1));

    bool reattachStencil = false;
    if (this->glCaps().detachStencilFromMSAABuffersBeforeReadPixels() &&
        renderTarget &&
        renderTarget->renderTargetPriv().getStencilAttachment() &&
        renderTarget->numSamples() > 1) {
        // Fix Adreno devices that won't read from MSAA framebuffers with stencil attached
        reattachStencil = true;
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, 0));
    }

    GL_CALL(ReadPixels(readRect.fLeft, readRect.fBottom, readRect.fWidth, readRect.fHeight,
                       externalFormat, externalType, offsetOrPtr));

    if (reattachStencil) {
        GrGLStencilAttachment* stencilAttachment = static_cast<GrGLStencilAttachment*>(
                renderTarget->renderTargetPriv().getStencilAttachment());
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, stencilAttachment->renderbufferID()));
    }

    if (rowWidthInPixels != width) {
        SkASSERT(this->glCaps().readPixelsRowBytesSupport());
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    }

    if (!renderTarget) {
        this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, surface);
    }
    return true;
}

bool GrGLGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                           GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                           size_t rowBytes) {
    SkASSERT(surface);

    size_t bytesPerPixel = GrColorTypeBytesPerPixel(dstColorType);

    // GL_PACK_ROW_LENGTH is in terms of pixels not bytes.
    int rowPixelWidth;

    if (rowBytes == SkToSizeT(width * bytesPerPixel)) {
        rowPixelWidth = width;
    } else {
        SkASSERT(!(rowBytes % bytesPerPixel));
        rowPixelWidth = rowBytes / bytesPerPixel;
    }
    return this->readOrTransferPixelsFrom(surface, left, top, width, height, surfaceColorType,
                                          dstColorType, buffer, rowPixelWidth);
}

GrGpuRTCommandBuffer* GrGLGpu::getCommandBuffer(
        GrRenderTarget* rt, GrSurfaceOrigin origin, const SkRect& bounds,
        const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
        const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    if (!fCachedRTCommandBuffer) {
        fCachedRTCommandBuffer.reset(new GrGLGpuRTCommandBuffer(this));
    }

    fCachedRTCommandBuffer->set(rt, origin, colorInfo, stencilInfo);
    return fCachedRTCommandBuffer.get();
}

GrGpuTextureCommandBuffer* GrGLGpu::getCommandBuffer(GrTexture* texture, GrSurfaceOrigin origin) {
    if (!fCachedTexCommandBuffer) {
        fCachedTexCommandBuffer.reset(new GrGLGpuTextureCommandBuffer(this));
    }

    fCachedTexCommandBuffer->set(texture, origin);
    return fCachedTexCommandBuffer.get();
}

void GrGLGpu::flushRenderTarget(GrGLRenderTarget* target, GrSurfaceOrigin origin,
                                const SkIRect& bounds) {
    this->flushRenderTargetNoColorWrites(target);
    this->didWriteToSurface(target, origin, &bounds);
}

void GrGLGpu::flushRenderTarget(GrGLRenderTarget* target) {
    this->flushRenderTargetNoColorWrites(target);
    this->didWriteToSurface(target, kTopLeft_GrSurfaceOrigin, nullptr);
}

void GrGLGpu::flushRenderTargetNoColorWrites(GrGLRenderTarget* target) {
    SkASSERT(target);
    GrGpuResource::UniqueID rtID = target->uniqueID();
    if (fHWBoundRenderTargetUniqueID != rtID) {
        this->bindFramebuffer(GR_GL_FRAMEBUFFER, target->renderFBOID());
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
        this->flushViewport(target->width(), target->height());
    }

    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(this->caps()->isFormatSRGB(target->backendFormat()));
    }
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

void GrGLGpu::flushViewport(int width, int height) {
    GrGLIRect viewport = {0, 0, width, height};
    if (fHWViewport != viewport) {
        viewport.pushToGLViewport(this->glInterface());
        fHWViewport = viewport;
    }
}

#define SWAP_PER_DRAW 0

#if SWAP_PER_DRAW
    #if defined(SK_BUILD_FOR_MAC)
        #include <AGL/agl.h>
    #elif defined(SK_BUILD_FOR_WIN)
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

void GrGLGpu::draw(GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
                   const GrPrimitiveProcessor& primProc,
                   const GrPipeline& pipeline,
                   const GrPipeline::FixedDynamicState* fixedDynamicState,
                   const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                   const GrMesh meshes[],
                   int meshCount) {
    this->handleDirtyContext();

    bool hasPoints = false;
    for (int i = 0; i < meshCount; ++i) {
        if (meshes[i].primitiveType() == GrPrimitiveType::kPoints) {
            hasPoints = true;
            break;
        }
    }
    if (!this->flushGLState(renderTarget, origin, primProc, pipeline, fixedDynamicState,
                            dynamicStateArrays, meshCount, hasPoints)) {
        return;
    }

    bool dynamicScissor = false;
    bool dynamicPrimProcTextures = false;
    if (dynamicStateArrays) {
        dynamicScissor = pipeline.isScissorEnabled() && dynamicStateArrays->fScissorRects;
        dynamicPrimProcTextures = dynamicStateArrays->fPrimitiveProcessorTextures;
    }
    for (int m = 0; m < meshCount; ++m) {
        if (GrXferBarrierType barrierType = pipeline.xferBarrierType(renderTarget->asTexture(),
                                                                     *this->caps())) {
            this->xferBarrier(renderTarget, barrierType);
        }

        if (dynamicScissor) {
            GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(renderTarget);
            this->flushScissor(GrScissorState(dynamicStateArrays->fScissorRects[m]),
                               glRT->width(), glRT->height(), origin);
        }
        if (dynamicPrimProcTextures) {
            auto texProxyArray = dynamicStateArrays->fPrimitiveProcessorTextures +
                                 m * primProc.numTextureSamplers();
            fHWProgram->updatePrimitiveProcessorTextureBindings(primProc, texProxyArray);
        }
        if (this->glCaps().requiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines() &&
            GrIsPrimTypeLines(meshes[m].primitiveType()) &&
            !GrIsPrimTypeLines(fLastPrimitiveType)) {
            GL_CALL(Enable(GR_GL_CULL_FACE));
            GL_CALL(Disable(GR_GL_CULL_FACE));
        }
        meshes[m].sendToGpu(this);
        fLastPrimitiveType = meshes[m].primitiveType();
    }

#if SWAP_PER_DRAW
    glFlush();
    #if defined(SK_BUILD_FOR_MAC)
        aglSwapBuffers(aglGetCurrentContext());
        int set_a_break_pt_here = 9;
        aglSwapBuffers(aglGetCurrentContext());
    #elif defined(SK_BUILD_FOR_WIN)
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
        case GrPrimitiveType::kPoints:
            return GR_GL_POINTS;
        case GrPrimitiveType::kLines:
            return GR_GL_LINES;
        case GrPrimitiveType::kLineStrip:
            return GR_GL_LINE_STRIP;
        case GrPrimitiveType::kLinesAdjacency:
            return GR_GL_LINES_ADJACENCY;
    }
    SK_ABORT("invalid GrPrimitiveType");
}

void GrGLGpu::sendMeshToGpu(GrPrimitiveType primitiveType, const GrBuffer* vertexBuffer,
                            int vertexCount, int baseVertex) {
    const GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    if (this->glCaps().drawArraysBaseVertexIsBroken()) {
        this->setupGeometry(nullptr, vertexBuffer, baseVertex, nullptr, 0, GrPrimitiveRestart::kNo);
        GL_CALL(DrawArrays(glPrimType, 0, vertexCount));
    } else {
        this->setupGeometry(nullptr, vertexBuffer, 0, nullptr, 0, GrPrimitiveRestart::kNo);
        GL_CALL(DrawArrays(glPrimType, baseVertex, vertexCount));
    }
    fStats.incNumDraws();
}

static const GrGLvoid* element_ptr(const GrBuffer* indexBuffer, int baseIndex) {
    size_t baseOffset = baseIndex * sizeof(uint16_t);
    if (indexBuffer->isCpuBuffer()) {
        return static_cast<const GrCpuBuffer*>(indexBuffer)->data() + baseOffset;
    } else {
        return reinterpret_cast<const GrGLvoid*>(baseOffset);
    }
}

void GrGLGpu::sendIndexedMeshToGpu(GrPrimitiveType primitiveType, const GrBuffer* indexBuffer,
                                   int indexCount, int baseIndex, uint16_t minIndexValue,
                                   uint16_t maxIndexValue, const GrBuffer* vertexBuffer,
                                   int baseVertex, GrPrimitiveRestart enablePrimitiveRestart) {
    const GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    const GrGLvoid* elementPtr = element_ptr(indexBuffer, baseIndex);

    this->setupGeometry(indexBuffer, vertexBuffer, baseVertex, nullptr, 0, enablePrimitiveRestart);

    if (this->glCaps().drawRangeElementsSupport()) {
        GL_CALL(DrawRangeElements(glPrimType, minIndexValue, maxIndexValue, indexCount,
                                  GR_GL_UNSIGNED_SHORT, elementPtr));
    } else {
        GL_CALL(DrawElements(glPrimType, indexCount, GR_GL_UNSIGNED_SHORT, elementPtr));
    }
    fStats.incNumDraws();
}

void GrGLGpu::sendInstancedMeshToGpu(GrPrimitiveType primitiveType, const GrBuffer* vertexBuffer,
                                     int vertexCount, int baseVertex,
                                     const GrBuffer* instanceBuffer, int instanceCount,
                                     int baseInstance) {
    GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    int maxInstances = this->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        this->setupGeometry(nullptr, vertexBuffer, 0, instanceBuffer, baseInstance + i,
                            GrPrimitiveRestart::kNo);
        GL_CALL(DrawArraysInstanced(glPrimType, baseVertex, vertexCount,
                                    SkTMin(instanceCount - i, maxInstances)));
        fStats.incNumDraws();
    }
}

void GrGLGpu::sendIndexedInstancedMeshToGpu(GrPrimitiveType primitiveType,
                                            const GrBuffer* indexBuffer, int indexCount,
                                            int baseIndex, const GrBuffer* vertexBuffer,
                                            int baseVertex, const GrBuffer* instanceBuffer,
                                            int instanceCount, int baseInstance,
                                            GrPrimitiveRestart enablePrimitiveRestart) {
    const GrGLenum glPrimType = gr_primitive_type_to_gl_mode(primitiveType);
    const GrGLvoid* elementPtr = element_ptr(indexBuffer, baseIndex);
    int maxInstances = this->glCaps().maxInstancesPerDrawWithoutCrashing(instanceCount);
    for (int i = 0; i < instanceCount; i += maxInstances) {
        this->setupGeometry(indexBuffer, vertexBuffer, baseVertex, instanceBuffer, baseInstance + i,
                            enablePrimitiveRestart);
        GL_CALL(DrawElementsInstanced(glPrimType, indexCount, GR_GL_UNSIGNED_SHORT, elementPtr,
                                      SkTMin(instanceCount - i, maxInstances)));
        fStats.incNumDraws();
    }
}

void GrGLGpu::onResolveRenderTarget(GrRenderTarget* target) {
    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(target);
    if (rt->needsResolve()) {
        // Some extensions automatically resolves the texture when it is read.
        if (this->glCaps().usesMSAARenderBuffers()) {
            SkASSERT(rt->textureFBOID() != rt->renderFBOID());
            SkASSERT(rt->textureFBOID() != 0 && rt->renderFBOID() != 0);
            this->bindFramebuffer(GR_GL_READ_FRAMEBUFFER, rt->renderFBOID());
            this->bindFramebuffer(GR_GL_DRAW_FRAMEBUFFER, rt->textureFBOID());

            // make sure we go through flushRenderTarget() since we've modified
            // the bound DRAW FBO ID.
            fHWBoundRenderTargetUniqueID.makeInvalid();
            const SkIRect dirtyRect = rt->getResolveRect();
            // The dirty rect tracked on the RT is always stored in the native coordinates of the
            // surface. Choose kTopLeft so no adjustments are made
            static constexpr auto kDirtyRectOrigin = kTopLeft_GrSurfaceOrigin;
            if (GrGLCaps::kES_Apple_MSFBOType == this->glCaps().msFBOType()) {
                // Apple's extension uses the scissor as the blit bounds.
                GrScissorState scissorState;
                scissorState.set(dirtyRect);
                this->flushScissor(scissorState, rt->width(), rt->height(), kDirtyRectOrigin);
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
                    rect.setRelativeTo(rt->height(), dirtyRect, kDirtyRectOrigin);
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

void GrGLGpu::flushStencil(const GrStencilSettings& stencilSettings, GrSurfaceOrigin origin) {
    if (stencilSettings.isDisabled()) {
        this->disableStencil();
    } else if (fHWStencilSettings != stencilSettings ||
               (stencilSettings.isTwoSided() && fHWStencilOrigin != origin)) {
        if (kYes_TriState != fHWStencilTestEnabled) {
            GL_CALL(Enable(GR_GL_STENCIL_TEST));

            fHWStencilTestEnabled = kYes_TriState;
        }
        if (stencilSettings.isTwoSided()) {
            set_gl_stencil(this->glInterface(), stencilSettings.front(origin), GR_GL_FRONT);
            set_gl_stencil(this->glInterface(), stencilSettings.back(origin), GR_GL_BACK);
        } else {
            set_gl_stencil(
                    this->glInterface(), stencilSettings.frontAndBack(), GR_GL_FRONT_AND_BACK);
        }
        fHWStencilSettings = stencilSettings;
        fHWStencilOrigin = origin;
    }
}

void GrGLGpu::disableStencil() {
    if (kNo_TriState != fHWStencilTestEnabled) {
        GL_CALL(Disable(GR_GL_STENCIL_TEST));

        fHWStencilTestEnabled = kNo_TriState;
        fHWStencilSettings.invalidate();
    }
}

void GrGLGpu::flushHWAAState(GrRenderTarget* rt, bool useHWAA) {
    // rt is only optional if useHWAA is false.
    SkASSERT(rt || !useHWAA);
#ifdef SK_DEBUG
    if (useHWAA && rt->numSamples() <= 1) {
        SkASSERT(this->caps()->mixedSamplesSupport());
        SkASSERT(0 != static_cast<GrGLRenderTarget*>(rt)->renderFBOID());
        SkASSERT(rt->renderTargetPriv().getStencilAttachment());
    }
#endif

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
}

void GrGLGpu::flushBlendAndColorWrite(
        const GrXferProcessor::BlendInfo& blendInfo, const GrSwizzle& swizzle) {
    if (this->glCaps().neverDisableColorWrites() && !blendInfo.fWriteColor) {
        // We need to work around a driver bug by using a blend state that preserves the dst color,
        // rather than disabling color writes.
        GrXferProcessor::BlendInfo preserveDstBlend;
        preserveDstBlend.fSrcBlend = kZero_GrBlendCoeff;
        preserveDstBlend.fDstBlend = kOne_GrBlendCoeff;
        this->flushBlendAndColorWrite(preserveDstBlend, swizzle);
        return;
    }

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;

    // Any optimization to disable blending should have already been applied and
    // tweaked the equation to "add" or "subtract", and the coeffs to (1, 0).
    bool blendOff =
        ((kAdd_GrBlendEquation == equation || kSubtract_GrBlendEquation == equation) &&
        kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff) ||
        !blendInfo.fWriteColor;

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
    } else {
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
            SkPMColor4f blendConst = swizzle.applyTo(blendInfo.fBlendConstant);
            if (!fHWBlendState.fConstColorValid || fHWBlendState.fConstColor != blendConst) {
                GL_CALL(BlendColor(blendConst.fR, blendConst.fG, blendConst.fB, blendConst.fA));
                fHWBlendState.fConstColor = blendConst;
                fHWBlendState.fConstColorValid = true;
            }
        }
    }

    this->flushColorWrite(blendInfo.fWriteColor);
}

static void get_gl_swizzle_values(const GrSwizzle& swizzle, GrGLenum glValues[4]) {
    for (int i = 0; i < 4; ++i) {
        switch (swizzle[i]) {
            case 'r': glValues[i] = GR_GL_RED;   break;
            case 'g': glValues[i] = GR_GL_GREEN; break;
            case 'b': glValues[i] = GR_GL_BLUE;  break;
            case 'a': glValues[i] = GR_GL_ALPHA; break;
            case '0': glValues[i] = GR_GL_ZERO;  break;
            case '1': glValues[i] = GR_GL_ONE;   break;
            default:  SK_ABORT("Unsupported component");
        }
    }
}

void GrGLGpu::bindTexture(int unitIdx, GrSamplerState samplerState, const GrSwizzle& swizzle,
                          GrGLTexture* texture) {
    SkASSERT(texture);

#ifdef SK_DEBUG
    if (!this->caps()->npotTextureTileSupport()) {
        if (samplerState.isRepeated()) {
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
    if (fHWTextureUnitBindings[unitIdx].boundID(target) != textureID) {
        this->setTextureUnit(unitIdx);
        GL_CALL(BindTexture(target, texture->textureID()));
        fHWTextureUnitBindings[unitIdx].setBoundID(target, textureID);
    }

    if (samplerState.filter() == GrSamplerState::Filter::kMipMap) {
        if (!this->caps()->mipMapSupport() ||
            texture->texturePriv().mipMapped() == GrMipMapped::kNo) {
            samplerState.setFilterMode(GrSamplerState::Filter::kBilerp);
        }
    }

#ifdef SK_DEBUG
    // We were supposed to ensure MipMaps were up-to-date before getting here.
    if (samplerState.filter() == GrSamplerState::Filter::kMipMap) {
        SkASSERT(!texture->texturePriv().mipMapsAreDirty());
    }
#endif

    auto timestamp = texture->parameters()->resetTimestamp();
    bool setAll = timestamp < fResetTimestampForTextureParameters;

    const GrGLTextureParameters::SamplerOverriddenState* samplerStateToRecord = nullptr;
    GrGLTextureParameters::SamplerOverriddenState newSamplerState;
    if (fSamplerObjectCache) {
        fSamplerObjectCache->bindSampler(unitIdx, samplerState);
    } else {
        const GrGLTextureParameters::SamplerOverriddenState& oldSamplerState =
                texture->parameters()->samplerOverriddenState();
        samplerStateToRecord = &newSamplerState;

        newSamplerState.fMinFilter = filter_to_gl_min_filter(samplerState.filter());
        newSamplerState.fMagFilter = filter_to_gl_mag_filter(samplerState.filter());

        newSamplerState.fWrapS = wrap_mode_to_gl_wrap(samplerState.wrapModeX(), this->glCaps());
        newSamplerState.fWrapT = wrap_mode_to_gl_wrap(samplerState.wrapModeY(), this->glCaps());

        // These are the OpenGL default values.
        newSamplerState.fMinLOD = -1000.f;
        newSamplerState.fMaxLOD = 1000.f;

        if (setAll || newSamplerState.fMagFilter != oldSamplerState.fMagFilter) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAG_FILTER, newSamplerState.fMagFilter));
        }
        if (setAll || newSamplerState.fMinFilter != oldSamplerState.fMinFilter) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MIN_FILTER, newSamplerState.fMinFilter));
        }
        if (this->glCaps().mipMapLevelAndLodControlSupport()) {
            if (setAll || newSamplerState.fMinLOD != oldSamplerState.fMinLOD) {
                this->setTextureUnit(unitIdx);
                GL_CALL(TexParameterf(target, GR_GL_TEXTURE_MIN_LOD, newSamplerState.fMinLOD));
            }
            if (setAll || newSamplerState.fMaxLOD != oldSamplerState.fMaxLOD) {
                this->setTextureUnit(unitIdx);
                GL_CALL(TexParameterf(target, GR_GL_TEXTURE_MAX_LOD, newSamplerState.fMaxLOD));
            }
        }
        if (setAll || newSamplerState.fWrapS != oldSamplerState.fWrapS) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_WRAP_S, newSamplerState.fWrapS));
        }
        if (setAll || newSamplerState.fWrapT != oldSamplerState.fWrapT) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_WRAP_T, newSamplerState.fWrapT));
        }
        if (this->glCaps().clampToBorderSupport()) {
            // Make sure the border color is transparent black (the default)
            if (setAll || oldSamplerState.fBorderColorInvalid) {
                this->setTextureUnit(unitIdx);
                static const GrGLfloat kTransparentBlack[4] = {0.f, 0.f, 0.f, 0.f};
                GL_CALL(TexParameterfv(target, GR_GL_TEXTURE_BORDER_COLOR, kTransparentBlack));
            }
        }
    }
    GrGLTextureParameters::NonsamplerState newNonsamplerState;
    newNonsamplerState.fBaseMipMapLevel = 0;
    newNonsamplerState.fMaxMipMapLevel = texture->texturePriv().maxMipMapLevel();

    const GrGLTextureParameters::NonsamplerState& oldNonsamplerState =
            texture->parameters()->nonsamplerState();
    if (!this->caps()->shaderCaps()->textureSwizzleAppliedInShader()) {
        newNonsamplerState.fSwizzleKey = swizzle.asKey();
        if (setAll || swizzle.asKey() != oldNonsamplerState.fSwizzleKey) {
            GrGLenum glValues[4];
            get_gl_swizzle_values(swizzle, glValues);
            this->setTextureUnit(unitIdx);
            if (GR_IS_GR_GL(this->glStandard())) {
                GR_STATIC_ASSERT(sizeof(glValues[0]) == sizeof(GrGLint));
                GL_CALL(TexParameteriv(target, GR_GL_TEXTURE_SWIZZLE_RGBA,
                                       reinterpret_cast<const GrGLint*>(glValues)));
            } else if (GR_IS_GR_GL_ES(this->glStandard())) {
                // ES3 added swizzle support but not GL_TEXTURE_SWIZZLE_RGBA.
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_R, glValues[0]));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_G, glValues[1]));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_B, glValues[2]));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_A, glValues[3]));
            }
        }
    }
    // These are not supported in ES2 contexts
    if (this->glCaps().mipMapLevelAndLodControlSupport() &&
        (texture->texturePriv().textureType() != GrTextureType::kExternal ||
         !this->glCaps().dontSetBaseOrMaxLevelForExternalTextures())) {
        if (newNonsamplerState.fBaseMipMapLevel != oldNonsamplerState.fBaseMipMapLevel) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_BASE_LEVEL,
                                  newNonsamplerState.fBaseMipMapLevel));
        }
        if (newNonsamplerState.fMaxMipMapLevel != oldNonsamplerState.fMaxMipMapLevel) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAX_LEVEL,
                                  newNonsamplerState.fMaxMipMapLevel));
        }
    }
    texture->parameters()->set(samplerStateToRecord, newNonsamplerState,
                               fResetTimestampForTextureParameters);
}

void GrGLGpu::onResetTextureBindings() {
    static constexpr GrGLenum kTargets[] = {GR_GL_TEXTURE_2D, GR_GL_TEXTURE_RECTANGLE,
                                            GR_GL_TEXTURE_EXTERNAL};
    for (int i = 0; i < this->numTextureUnits(); ++i) {
        this->setTextureUnit(i);
        for (auto target : kTargets) {
            if (fHWTextureUnitBindings[i].hasBeenModified(target)) {
                GL_CALL(BindTexture(target, 0));
            }
        }
        fHWTextureUnitBindings[i].invalidateAllTargets(true);
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

void GrGLGpu::flushClearColor(GrGLfloat r, GrGLfloat g, GrGLfloat b, GrGLfloat a) {
    if (r != fHWClearColor[0] || g != fHWClearColor[1] ||
        b != fHWClearColor[2] || a != fHWClearColor[3]) {
        GL_CALL(ClearColor(r, g, b, a));
        fHWClearColor[0] = r;
        fHWClearColor[1] = g;
        fHWClearColor[2] = b;
        fHWClearColor[3] = a;
    }
}

void GrGLGpu::setTextureUnit(int unit) {
    SkASSERT(unit >= 0 && unit < this->numTextureUnits());
    if (unit != fHWActiveTextureUnitIdx) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + unit));
        fHWActiveTextureUnitIdx = unit;
    }
}

void GrGLGpu::bindTextureToScratchUnit(GrGLenum target, GrGLint textureID) {
    // Bind the last texture unit since it is the least likely to be used by GrGLProgram.
    int lastUnitIdx = this->numTextureUnits() - 1;
    if (lastUnitIdx != fHWActiveTextureUnitIdx) {
        GL_CALL(ActiveTexture(GR_GL_TEXTURE0 + lastUnitIdx));
        fHWActiveTextureUnitIdx = lastUnitIdx;
    }
    // Clear out the this field so that if a GrGLProgram does use this unit it will rebind the
    // correct texture.
    fHWTextureUnitBindings[lastUnitIdx].invalidateForScratchUse(target);
    GL_CALL(BindTexture(target, textureID));
}

// Determines whether glBlitFramebuffer could be used between src and dst by onCopySurface.
static inline bool can_blit_framebuffer_for_copy_surface(const GrSurface* dst,
                                                         const GrSurface* src,
                                                         const SkIRect& srcRect,
                                                         const SkIPoint& dstPoint,
                                                         const GrGLCaps& caps) {
    int dstSampleCnt = 0;
    int srcSampleCnt = 0;
    if (const GrRenderTarget* rt = dst->asRenderTarget()) {
        dstSampleCnt = rt->numSamples();
    }
    if (const GrRenderTarget* rt = src->asRenderTarget()) {
        srcSampleCnt = rt->numSamples();
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTarget()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTarget()));

    GrGLFormat dstFormat = dst->backendFormat().asGLFormat();
    GrGLFormat srcFormat = src->backendFormat().asGLFormat();

    const GrGLTexture* dstTex = static_cast<const GrGLTexture*>(dst->asTexture());
    const GrGLTexture* srcTex = static_cast<const GrGLTexture*>(src->asTexture());

    GrTextureType dstTexType;
    GrTextureType* dstTexTypePtr = nullptr;
    GrTextureType srcTexType;
    GrTextureType* srcTexTypePtr = nullptr;
    if (dstTex) {
        dstTexType = dstTex->texturePriv().textureType();
        dstTexTypePtr = &dstTexType;
    }
    if (srcTex) {
        srcTexType = srcTex->texturePriv().textureType();
        srcTexTypePtr = &srcTexType;
    }

    return caps.canCopyAsBlit(dstFormat, dstSampleCnt, dstTexTypePtr,
                              srcFormat, srcSampleCnt, srcTexTypePtr,
                              src->getBoundsRect(), true, srcRect, dstPoint);
}

static bool rt_has_msaa_render_buffer(const GrGLRenderTarget* rt, const GrGLCaps& glCaps) {
    // A RT has a separate MSAA renderbuffer if:
    // 1) It's multisampled
    // 2) We're using an extension with separate MSAA renderbuffers
    // 3) It's not FBO 0, which is special and always auto-resolves
    return rt->numSamples() > 1 && glCaps.usesMSAARenderBuffers() && rt->renderFBOID() != 0;
}

static inline bool can_copy_texsubimage(const GrSurface* dst, const GrSurface* src,
                                        const GrGLCaps& caps) {

    const GrGLRenderTarget* dstRT = static_cast<const GrGLRenderTarget*>(dst->asRenderTarget());
    const GrGLRenderTarget* srcRT = static_cast<const GrGLRenderTarget*>(src->asRenderTarget());
    const GrGLTexture* dstTex = static_cast<const GrGLTexture*>(dst->asTexture());
    const GrGLTexture* srcTex = static_cast<const GrGLTexture*>(src->asTexture());

    bool dstHasMSAARenderBuffer = dstRT ? rt_has_msaa_render_buffer(dstRT, caps) : false;
    bool srcHasMSAARenderBuffer = srcRT ? rt_has_msaa_render_buffer(srcRT, caps) : false;

    GrGLFormat dstFormat = dst->backendFormat().asGLFormat();
    GrGLFormat srcFormat = src->backendFormat().asGLFormat();

    GrTextureType dstTexType;
    GrTextureType* dstTexTypePtr = nullptr;
    GrTextureType srcTexType;
    GrTextureType* srcTexTypePtr = nullptr;
    if (dstTex) {
        dstTexType = dstTex->texturePriv().textureType();
        dstTexTypePtr = &dstTexType;
    }
    if (srcTex) {
        srcTexType = srcTex->texturePriv().textureType();
        srcTexTypePtr = &srcTexType;
    }

    return caps.canCopyTexSubImage(dstFormat, dstHasMSAARenderBuffer, dstTexTypePtr,
                                   srcFormat, srcHasMSAARenderBuffer, srcTexTypePtr);
}

// If a temporary FBO was created, its non-zero ID is returned.
void GrGLGpu::bindSurfaceFBOForPixelOps(GrSurface* surface, GrGLenum fboTarget,
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

        this->bindFramebuffer(fboTarget, *tempFBOID);
        GR_GL_CALL(this->glInterface(), FramebufferTexture2D(fboTarget,
                                                             GR_GL_COLOR_ATTACHMENT0,
                                                             target,
                                                             texID,
                                                             0));
        texture->baseLevelWasBoundToFBO();
    } else {
        this->bindFramebuffer(fboTarget, rt->renderFBOID());
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

void GrGLGpu::onFBOChanged() {
    if (this->caps()->workarounds().flush_on_framebuffer_change ||
        this->caps()->workarounds().restore_scissor_on_fbo_change) {
        GL_CALL(Flush());
    }
}

void GrGLGpu::bindFramebuffer(GrGLenum target, GrGLuint fboid) {
    fStats.incRenderTargetBinds();
    GL_CALL(BindFramebuffer(target, fboid));
    if (target == GR_GL_FRAMEBUFFER || target == GR_GL_DRAW_FRAMEBUFFER) {
        fBoundDrawFramebuffer = fboid;
    }

    if (this->caps()->workarounds().restore_scissor_on_fbo_change) {
        // The driver forgets the correct scissor when modifying the FBO binding.
        if (!fHWScissorSettings.fRect.isInvalid()) {
            fHWScissorSettings.fRect.pushToGLScissor(this->glInterface());
        }
    }

    this->onFBOChanged();
}

void GrGLGpu::deleteFramebuffer(GrGLuint fboid) {
    if (fboid == fBoundDrawFramebuffer &&
        this->caps()->workarounds().unbind_attachments_on_bound_render_fbo_delete) {
        // This workaround only applies to deleting currently bound framebuffers
        // on Adreno 420.  Because this is a somewhat rare case, instead of
        // tracking all the attachments of every framebuffer instead just always
        // unbind all attachments.
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_RENDERBUFFER, 0));
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, 0));
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_DEPTH_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, 0));
    }

    GL_CALL(DeleteFramebuffers(1, &fboid));

    // Deleting the currently bound framebuffer rebinds to 0.
    if (fboid == fBoundDrawFramebuffer) {
        this->onFBOChanged();
    }
}

bool GrGLGpu::onCopySurface(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                            const SkIPoint& dstPoint, bool canDiscardOutsideDstRect) {
    // Don't prefer copying as a draw if the dst doesn't already have a FBO object.
    // This implicitly handles this->glCaps().useDrawInsteadOfAllRenderTargetWrites().
    bool preferCopy = SkToBool(dst->asRenderTarget());
    auto dstFormat = dst->backendFormat().asGLFormat();
    if (preferCopy && this->glCaps().canCopyAsDraw(dstFormat, SkToBool(src->asTexture()))) {
        if (this->copySurfaceAsDraw(dst, src, srcRect, dstPoint)) {
            return true;
        }
    }

    if (can_copy_texsubimage(dst, src, this->glCaps())) {
        this->copySurfaceAsCopyTexSubImage(dst, src, srcRect, dstPoint);
        return true;
    }

    if (can_blit_framebuffer_for_copy_surface(dst, src, srcRect, dstPoint, this->glCaps())) {
        return this->copySurfaceAsBlitFramebuffer(dst, src, srcRect, dstPoint);
    }

    if (!preferCopy && this->glCaps().canCopyAsDraw(dstFormat, SkToBool(src->asTexture()))) {
        if (this->copySurfaceAsDraw(dst, src, srcRect, dstPoint)) {
            return true;
        }
    }

    return false;
}

bool GrGLGpu::createCopyProgram(GrTexture* srcTex) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    int progIdx = TextureToCopyProgramIdx(srcTex);
    const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();
    GrSLType samplerType =
            GrSLCombinedSamplerTypeForTextureType(srcTex->texturePriv().textureType());

    if (!fCopyProgramArrayBuffer) {
        static const GrGLfloat vdata[] = {
            0, 0,
            0, 1,
            1, 0,
            1, 1
        };
        fCopyProgramArrayBuffer = GrGLBuffer::Make(this, sizeof(vdata), GrGpuBufferType::kVertex,
                                                   kStatic_GrAccessPattern, vdata);
    }
    if (!fCopyProgramArrayBuffer) {
        return false;
    }

    SkASSERT(!fCopyPrograms[progIdx].fProgram);
    GL_CALL_RET(fCopyPrograms[progIdx].fProgram, CreateProgram());
    if (!fCopyPrograms[progIdx].fProgram) {
        return false;
    }

    GrShaderVar aVertex("a_vertex", kHalf2_GrSLType, GrShaderVar::kIn_TypeModifier);
    GrShaderVar uTexCoordXform("u_texCoordXform", kHalf4_GrSLType,
                               GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uPosXform("u_posXform", kHalf4_GrSLType, GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uTexture("u_texture", samplerType, GrShaderVar::kUniform_TypeModifier);
    GrShaderVar vTexCoord("v_texCoord", kHalf2_GrSLType, GrShaderVar::kOut_TypeModifier);
    GrShaderVar oFragColor("o_FragColor", kHalf4_GrSLType, GrShaderVar::kOut_TypeModifier);

    SkString vshaderTxt;
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
        "  v_texCoord = half2(a_vertex.xy * u_texCoordXform.xy + u_texCoordXform.zw);"
        "  sk_Position.xy = a_vertex * u_posXform.xy + u_posXform.zw;"
        "  sk_Position.zw = half2(0, 1);"
        "}"
    );

    SkString fshaderTxt;
    if (shaderCaps->noperspectiveInterpolationSupport()) {
        if (const char* extension = shaderCaps->noperspectiveInterpolationExtensionString()) {
            fshaderTxt.appendf("#extension %s : require\n", extension);
        }
    }
    vTexCoord.setTypeModifier(GrShaderVar::kIn_TypeModifier);
    vTexCoord.appendDecl(shaderCaps, &fshaderTxt);
    fshaderTxt.append(";");
    uTexture.appendDecl(shaderCaps, &fshaderTxt);
    fshaderTxt.append(";");
    fshaderTxt.appendf(
        "// Copy Program FS\n"
        "void main() {"
        "  sk_FragColor = sample(u_texture, v_texCoord);"
        "}"
    );

    auto errorHandler = this->getContext()->priv().getShaderErrorHandler();
    SkSL::String sksl(vshaderTxt.c_str(), vshaderTxt.size());
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    SkSL::String glsl;
    std::unique_ptr<SkSL::Program> program = GrSkSLtoGLSL(*fGLContext, SkSL::Program::kVertex_Kind,
                                                          sksl, settings, &glsl, errorHandler);
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, glsl, &fStats, errorHandler);
    SkASSERT(program->fInputs.isEmpty());

    sksl.assign(fshaderTxt.c_str(), fshaderTxt.size());
    program = GrSkSLtoGLSL(*fGLContext, SkSL::Program::kFragment_Kind, sksl, settings, &glsl,
                           errorHandler);
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, glsl, &fStats,
                                                  errorHandler);
    SkASSERT(program->fInputs.isEmpty());

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

    GrShaderVar aVertex("a_vertex", kHalf2_GrSLType, GrShaderVar::kIn_TypeModifier);
    GrShaderVar uTexCoordXform("u_texCoordXform", kHalf4_GrSLType,
                               GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uTexture("u_texture", kTexture2DSampler_GrSLType,
                         GrShaderVar::kUniform_TypeModifier);
    // We need 1, 2, or 4 texture coordinates (depending on parity of each dimension):
    GrShaderVar vTexCoords[] = {
        GrShaderVar("v_texCoord0", kHalf2_GrSLType, GrShaderVar::kOut_TypeModifier),
        GrShaderVar("v_texCoord1", kHalf2_GrSLType, GrShaderVar::kOut_TypeModifier),
        GrShaderVar("v_texCoord2", kHalf2_GrSLType, GrShaderVar::kOut_TypeModifier),
        GrShaderVar("v_texCoord3", kHalf2_GrSLType, GrShaderVar::kOut_TypeModifier),
    };
    GrShaderVar oFragColor("o_FragColor", kHalf4_GrSLType,GrShaderVar::kOut_TypeModifier);

    SkString vshaderTxt;
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
        "  sk_Position.xy = a_vertex * half2(2, 2) - half2(1, 1);"
        "  sk_Position.zw = half2(0, 1);"
    );

    // Insert texture coordinate computation:
    if (oddWidth && oddHeight) {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy * u_texCoordXform.yw;"
            "  v_texCoord1 = a_vertex.xy * u_texCoordXform.yw + half2(u_texCoordXform.x, 0);"
            "  v_texCoord2 = a_vertex.xy * u_texCoordXform.yw + half2(0, u_texCoordXform.z);"
            "  v_texCoord3 = a_vertex.xy * u_texCoordXform.yw + u_texCoordXform.xz;"
        );
    } else if (oddWidth) {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy * half2(u_texCoordXform.y, 1);"
            "  v_texCoord1 = a_vertex.xy * half2(u_texCoordXform.y, 1) + half2(u_texCoordXform.x, 0);"
        );
    } else if (oddHeight) {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy * half2(1, u_texCoordXform.w);"
            "  v_texCoord1 = a_vertex.xy * half2(1, u_texCoordXform.w) + half2(0, u_texCoordXform.z);"
        );
    } else {
        vshaderTxt.append(
            "  v_texCoord0 = a_vertex.xy;"
        );
    }

    vshaderTxt.append("}");

    SkString fshaderTxt;
    if (shaderCaps->noperspectiveInterpolationSupport()) {
        if (const char* extension = shaderCaps->noperspectiveInterpolationExtensionString()) {
            fshaderTxt.appendf("#extension %s : require\n", extension);
        }
    }
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
            "  sk_FragColor = (sample(u_texture, v_texCoord0) + "
            "                  sample(u_texture, v_texCoord1) + "
            "                  sample(u_texture, v_texCoord2) + "
            "                  sample(u_texture, v_texCoord3)) * 0.25;"
        );
    } else if (oddWidth || oddHeight) {
        fshaderTxt.append(
            "  sk_FragColor = (sample(u_texture, v_texCoord0) + "
            "                  sample(u_texture, v_texCoord1)) * 0.5;"
        );
    } else {
        fshaderTxt.append(
            "  sk_FragColor = sample(u_texture, v_texCoord0);"
        );
    }

    fshaderTxt.append("}");

    auto errorHandler = this->getContext()->priv().getShaderErrorHandler();
    SkSL::String sksl(vshaderTxt.c_str(), vshaderTxt.size());
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    SkSL::String glsl;
    std::unique_ptr<SkSL::Program> program = GrSkSLtoGLSL(*fGLContext, SkSL::Program::kVertex_Kind,
                                                          sksl, settings, &glsl, errorHandler);
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, glsl, &fStats, errorHandler);
    SkASSERT(program->fInputs.isEmpty());

    sksl.assign(fshaderTxt.c_str(), fshaderTxt.size());
    program = GrSkSLtoGLSL(*fGLContext, SkSL::Program::kFragment_Kind, sksl, settings, &glsl,
                           errorHandler);
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, glsl, &fStats,
                                                  errorHandler);
    SkASSERT(program->fInputs.isEmpty());

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

bool GrGLGpu::copySurfaceAsDraw(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                const SkIPoint& dstPoint) {
    auto* srcTex = static_cast<GrGLTexture*>(src->asTexture());
    auto* dstTex = static_cast<GrGLTexture*>(src->asTexture());
    auto* dstRT  = static_cast<GrGLRenderTarget*>(src->asRenderTarget());
    if (!srcTex) {
        return false;
    }
    int progIdx = TextureToCopyProgramIdx(srcTex);
    if (!dstRT) {
        SkASSERT(dstTex);
        if (!this->glCaps().isFormatRenderable(dstTex->format(), 1)) {
            return false;
        }
    }
    if (!fCopyPrograms[progIdx].fProgram) {
        if (!this->createCopyProgram(srcTex)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }
    int w = srcRect.width();
    int h = srcRect.height();
    // We don't swizzle at all in our copies.
    this->bindTexture(0, GrSamplerState::ClampNearest(), GrSwizzle::RGBA(), srcTex);
    this->bindSurfaceFBOForPixelOps(dst, GR_GL_FRAMEBUFFER, kDst_TempFBOTarget);
    this->flushViewport(dst->width(), dst->height());
    fHWBoundRenderTargetUniqueID.makeInvalid();
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY, w, h);
    this->flushProgram(fCopyPrograms[progIdx].fProgram);
    fHWVertexArrayState.setVertexArrayID(this, 0);
    GrGLAttribArrayState* attribs = fHWVertexArrayState.bindInternalVertexArray(this);
    attribs->enableVertexArrays(this, 1);
    attribs->set(this, 0, fCopyProgramArrayBuffer.get(), kFloat2_GrVertexAttribType,
                 kFloat2_GrSLType, 2 * sizeof(GrGLfloat), 0);
    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    GrGLfloat dx0 = 2.f * dstPoint.fX / dw - 1.f;
    GrGLfloat dx1 = 2.f * (dstPoint.fX + w) / dw - 1.f;
    GrGLfloat dy0 = 2.f * dstPoint.fY / dh - 1.f;
    GrGLfloat dy1 = 2.f * (dstPoint.fY + h) / dh - 1.f;
    GrGLfloat sx0 = (GrGLfloat)srcRect.fLeft;
    GrGLfloat sx1 = (GrGLfloat)(srcRect.fLeft + w);
    GrGLfloat sy0 = (GrGLfloat)srcRect.fTop;
    GrGLfloat sy1 = (GrGLfloat)(srcRect.fTop + h);
    int sw = src->width();
    int sh = src->height();
    if (srcTex->texturePriv().textureType() != GrTextureType::kRectangle) {
        // src rect edges in normalized texture space (0 to 1)
        sx0 /= sw;
        sx1 /= sw;
        sy0 /= sh;
        sy1 /= sh;
    }
    GL_CALL(Uniform4f(fCopyPrograms[progIdx].fPosXformUniform, dx1 - dx0, dy1 - dy0, dx0, dy0));
    GL_CALL(Uniform4f(fCopyPrograms[progIdx].fTexCoordXformUniform,
                      sx1 - sx0, sy1 - sy0, sx0, sy0));
    GL_CALL(Uniform1i(fCopyPrograms[progIdx].fTextureUniform, 0));
    this->flushBlendAndColorWrite(GrXferProcessor::BlendInfo(), GrSwizzle::RGBA());
    this->flushHWAAState(nullptr, false);
    this->disableScissor();
    this->disableWindowRectangles();
    this->disableStencil();
    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(true);
    }
    GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, dst);
    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
    return true;
}

void GrGLGpu::copySurfaceAsCopyTexSubImage(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_copy_texsubimage(dst, src, this->glCaps()));
    this->bindSurfaceFBOForPixelOps(src, GR_GL_FRAMEBUFFER, kSrc_TempFBOTarget);
    GrGLTexture* dstTex = static_cast<GrGLTexture *>(dst->asTexture());
    SkASSERT(dstTex);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();

    this->bindTextureToScratchUnit(dstTex->target(), dstTex->textureID());
    GL_CALL(CopyTexSubImage2D(dstTex->target(), 0,
                              dstPoint.fX, dstPoint.fY,
                              srcRect.fLeft, srcRect.fTop,
                              srcRect.width(), srcRect.height()));
    this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, src);
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
}

bool GrGLGpu::copySurfaceAsBlitFramebuffer(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_blit_framebuffer_for_copy_surface(dst, src, srcRect, dstPoint, this->glCaps()));
    SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.fX, dstPoint.fY,
                                        srcRect.width(), srcRect.height());
    if (dst == src) {
        if (SkIRect::IntersectsNoEmptyCheck(dstRect, srcRect)) {
            return false;
        }
    }

    this->bindSurfaceFBOForPixelOps(dst, GR_GL_DRAW_FRAMEBUFFER, kDst_TempFBOTarget);
    this->bindSurfaceFBOForPixelOps(src, GR_GL_READ_FRAMEBUFFER, kSrc_TempFBOTarget);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();

    // BlitFrameBuffer respects the scissor, so disable it.
    this->disableScissor();
    this->disableWindowRectangles();

    GL_CALL(BlitFramebuffer(srcRect.fLeft,
                            srcRect.fTop,
                            srcRect.fRight,
                            srcRect.fBottom,
                            dstRect.fLeft,
                            dstRect.fTop,
                            dstRect.fRight,
                            dstRect.fBottom,
                            GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
    this->unbindTextureFBOForPixelOps(GR_GL_DRAW_FRAMEBUFFER, dst);
    this->unbindTextureFBOForPixelOps(GR_GL_READ_FRAMEBUFFER, src);

    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
    return true;
}

bool GrGLGpu::onRegenerateMipMapLevels(GrTexture* texture) {
    auto glTex = static_cast<GrGLTexture*>(texture);
    // Mipmaps are only supported on 2D textures:
    if (GR_GL_TEXTURE_2D != glTex->target()) {
        return false;
    }
    GrGLFormat format = glTex->format();
    // Manual implementation of mipmap generation, to work around driver bugs w/sRGB.
    // Uses draw calls to do a series of downsample operations to successive mips.

    // The manual approach requires the ability to limit which level we're sampling and that the
    // destination can be bound to a FBO:
    if (!this->glCaps().doManualMipmapping() || !this->glCaps().isFormatRenderable(format, 1)) {
        GrGLenum target = glTex->target();
        this->bindTextureToScratchUnit(target, glTex->textureID());
        GL_CALL(GenerateMipmap(glTex->target()));
        return true;
    }

    int width = texture->width();
    int height = texture->height();
    int levelCount = SkMipMap::ComputeLevelCount(width, height) + 1;
    SkASSERT(levelCount == texture->texturePriv().maxMipMapLevel() + 1);

    // Create (if necessary), then bind temporary FBO:
    if (0 == fTempDstFBOID) {
        GL_CALL(GenFramebuffers(1, &fTempDstFBOID));
    }
    this->bindFramebuffer(GR_GL_FRAMEBUFFER, fTempDstFBOID);
    fHWBoundRenderTargetUniqueID.makeInvalid();

    // Bind the texture, to get things configured for filtering.
    // We'll be changing our base level further below:
    this->setTextureUnit(0);
    // The mipmap program does not do any swizzling.
    this->bindTexture(0, GrSamplerState::ClampBilerp(), GrSwizzle::RGBA(), glTex);

    // Vertex data:
    if (!fMipmapProgramArrayBuffer) {
        static const GrGLfloat vdata[] = {
            0, 0,
            0, 1,
            1, 0,
            1, 1
        };
        fMipmapProgramArrayBuffer = GrGLBuffer::Make(this, sizeof(vdata), GrGpuBufferType::kVertex,
                                                     kStatic_GrAccessPattern, vdata);
    }
    if (!fMipmapProgramArrayBuffer) {
        return false;
    }

    fHWVertexArrayState.setVertexArrayID(this, 0);

    GrGLAttribArrayState* attribs = fHWVertexArrayState.bindInternalVertexArray(this);
    attribs->enableVertexArrays(this, 1);
    attribs->set(this, 0, fMipmapProgramArrayBuffer.get(), kFloat2_GrVertexAttribType,
                 kFloat2_GrSLType, 2 * sizeof(GrGLfloat), 0);

    // Set "simple" state once:
    this->flushBlendAndColorWrite(GrXferProcessor::BlendInfo(), GrSwizzle::RGBA());
    this->flushHWAAState(nullptr, false);
    this->disableScissor();
    this->disableWindowRectangles();
    this->disableStencil();

    // Do all the blits:
    width = texture->width();
    height = texture->height();

    for (GrGLint level = 1; level < levelCount; ++level) {
        // Get and bind the program for this particular downsample (filter shape can vary):
        int progIdx = TextureSizeToMipmapProgramIdx(width, height);
        if (!fMipmapPrograms[progIdx].fProgram) {
            if (!this->createMipmapProgram(progIdx)) {
                SkDebugf("Failed to create mipmap program.\n");
                // Invalidate all params to cover base level change in a previous iteration.
                glTex->textureParamsModified();
                return false;
            }
        }
        this->flushProgram(fMipmapPrograms[progIdx].fProgram);

        // Texcoord uniform is expected to contain (1/w, (w-1)/w, 1/h, (h-1)/h)
        const float invWidth = 1.0f / width;
        const float invHeight = 1.0f / height;
        GL_CALL(Uniform4f(fMipmapPrograms[progIdx].fTexCoordXformUniform,
                          invWidth, (width - 1) * invWidth, invHeight, (height - 1) * invHeight));
        GL_CALL(Uniform1i(fMipmapPrograms[progIdx].fTextureUniform, 0));

        // Only sample from previous mip
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_BASE_LEVEL, level - 1));

        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0, GR_GL_TEXTURE_2D,
                                     glTex->textureID(), level));

        width = SkTMax(1, width / 2);
        height = SkTMax(1, height / 2);
        this->flushViewport(width, height);

        GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    }

    // Unbind:
    GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                 GR_GL_TEXTURE_2D, 0, 0));

    // We modified the base level param.
    GrGLTextureParameters::NonsamplerState nonsamplerState = glTex->parameters()->nonsamplerState();
    // We drew the 2nd to last level into the last level.
    nonsamplerState.fBaseMipMapLevel = levelCount - 2;
    glTex->parameters()->set(nullptr, nonsamplerState, fResetTimestampForTextureParameters);

    return true;
}

void GrGLGpu::querySampleLocations(
        GrRenderTarget* renderTarget, SkTArray<SkPoint>* sampleLocations) {
    this->flushRenderTargetNoColorWrites(static_cast<GrGLRenderTarget*>(renderTarget));

    int effectiveSampleCnt;
    GR_GL_GetIntegerv(this->glInterface(), GR_GL_SAMPLES, &effectiveSampleCnt);
    SkASSERT(effectiveSampleCnt >= renderTarget->numSamples());

    sampleLocations->reset(effectiveSampleCnt);
    for (int i = 0; i < effectiveSampleCnt; ++i) {
        GL_CALL(GetMultisamplefv(GR_GL_SAMPLE_POSITION, i, &(*sampleLocations)[i].fX));
    }
}

void GrGLGpu::xferBarrier(GrRenderTarget* rt, GrXferBarrierType type) {
    SkASSERT(type);
    switch (type) {
        case kTexture_GrXferBarrierType: {
            GrGLRenderTarget* glrt = static_cast<GrGLRenderTarget*>(rt);
            SkASSERT(glrt->textureFBOID() != 0 && glrt->renderFBOID() != 0);
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

static GrPixelConfig gl_format_to_pixel_config(GrGLFormat format) {
    switch (format) {
        case GrGLFormat::kRGBA8:                return kRGBA_8888_GrPixelConfig;
        case GrGLFormat::kRGB8:                 return kRGB_888_GrPixelConfig;
        case GrGLFormat::kRG8:                  return kRG_88_GrPixelConfig;
        case GrGLFormat::kBGRA8:                return kBGRA_8888_GrPixelConfig;
        case GrGLFormat::kLUMINANCE8:           return kGray_8_GrPixelConfig;
        case GrGLFormat::kSRGB8_ALPHA8:         return kSRGBA_8888_GrPixelConfig;
        case GrGLFormat::kRGB10_A2:             return kRGBA_1010102_GrPixelConfig;
        case GrGLFormat::kRGB565:               return kRGB_565_GrPixelConfig;
        case GrGLFormat::kRGBA4:                return kRGBA_4444_GrPixelConfig;
        case GrGLFormat::kRGBA32F:              return kRGBA_float_GrPixelConfig;
        case GrGLFormat::kRGBA16F:              return kRGBA_half_GrPixelConfig;
        case GrGLFormat::kR16:                  return kR_16_GrPixelConfig;
        case GrGLFormat::kRG16:                 return kRG_1616_GrPixelConfig;
        case GrGLFormat::kRGBA16:               return kRGBA_16161616_GrPixelConfig;
        case GrGLFormat::kRG16F:                return kRG_half_GrPixelConfig;
        case GrGLFormat::kUnknown:              return kUnknown_GrPixelConfig;

        // Configs with multiple equivalent formats.

        case GrGLFormat::kR16F:                 return kAlpha_half_GrPixelConfig;
        case GrGLFormat::kLUMINANCE16F:         return kAlpha_half_GrPixelConfig;

        case GrGLFormat::kALPHA8:               return kAlpha_8_GrPixelConfig;
        case GrGLFormat::kR8:                   return kAlpha_8_GrPixelConfig;

        case GrGLFormat::kCOMPRESSED_RGB8_ETC2: return kRGB_ETC1_GrPixelConfig;
        case GrGLFormat::kCOMPRESSED_ETC1_RGB8: return kRGB_ETC1_GrPixelConfig;
    }
    SkUNREACHABLE;
}

GrBackendTexture GrGLGpu::createBackendTexture(int w, int h,
                                               const GrBackendFormat& format,
                                               GrMipMapped mipMapped,
                                               GrRenderable renderable,
                                               const void* srcPixels, size_t rowBytes,
                                               const SkColor4f* color,
                                               GrProtected isProtected) {
    this->handleDirtyContext();

    GrGLFormat glFormat = format.asGLFormat();
    if (glFormat == GrGLFormat::kUnknown) {
        return GrBackendTexture();  // invalid
    }

    GrPixelConfig config = gl_format_to_pixel_config(glFormat);

    if (config == kUnknown_GrPixelConfig) {
        return GrBackendTexture();  // invalid
    }

    auto textureColorType = GrPixelConfigToColorType(config);

    if (!this->caps()->isFormatTexturableAndUploadable(textureColorType, format)) {
        return GrBackendTexture();  // invalid
    }

    if (w < 1 || w > this->caps()->maxTextureSize() ||
        h < 1 || h > this->caps()->maxTextureSize()) {
        return GrBackendTexture();  // invalid
    }

    // Currently we don't support uploading pixel data when mipped.
    if (srcPixels && GrMipMapped::kYes == mipMapped) {
        return GrBackendTexture();  // invalid
    }

    if (mipMapped == GrMipMapped::kYes && !this->caps()->mipMapSupport()) {
        return GrBackendTexture();  // invalid
    }

    GrGLTextureInfo info;
    GrGLTextureParameters::SamplerOverriddenState initialState;

    int mipLevelCount = 0;
    SkAutoTMalloc<GrMipLevel> texels;
    SkAutoMalloc pixelStorage;
    SkImage::CompressionType compressionType;
    if (GrGLFormatToCompressionType(glFormat, &compressionType)) {
        // Compressed textures currently must be non-MIP mapped and have initial data.
        if (mipMapped == GrMipMapped::kYes) {
            return GrBackendTexture();
        }
        if (!srcPixels) {
            if (!color) {
                return GrBackendTexture();
            }
            SkASSERT(0 == rowBytes);
            size_t size = GrCompressedDataSize(compressionType, w, h);
            srcPixels = pixelStorage.reset(size);
            GrFillInCompressedData(compressionType, w, h, (char*)srcPixels, *color);
        }
        info.fID = this->createCompressedTexture2D(
                {w, h}, glFormat, compressionType, &initialState, srcPixels);
        if (!info.fID) {
            return GrBackendTexture();
        }
        info.fFormat = GrGLFormatToEnum(glFormat);
        info.fTarget = GR_GL_TEXTURE_2D;
    } else {
        if (srcPixels) {
            mipLevelCount = 1;
            texels.reset(mipLevelCount);
            texels.get()[0] = {srcPixels, rowBytes};
        } else if (color) {
            mipLevelCount = 1;
            if (GrMipMapped::kYes == mipMapped) {
                mipLevelCount = SkMipMap::ComputeLevelCount(w, h) + 1;
            }

            texels.reset(mipLevelCount);
            SkTArray<size_t> individualMipOffsets(mipLevelCount);

            size_t bytesPerPixel = GrBytesPerPixel(config);

            size_t totalSize = GrComputeTightCombinedBufferSize(
                    bytesPerPixel, w, h, &individualMipOffsets, mipLevelCount);

            char* tmpPixels = (char*)pixelStorage.reset(totalSize);

            GrFillInData(config, w, h, individualMipOffsets, tmpPixels, *color);
            for (int i = 0; i < mipLevelCount; ++i) {
                size_t offset = individualMipOffsets[i];

                int twoToTheMipLevel = 1 << i;
                int currentWidth = SkTMax(1, w / twoToTheMipLevel);

                texels.get()[i] = {&(tmpPixels[offset]), currentWidth * bytesPerPixel};
            }
        }
        GrSurfaceDesc desc;
        desc.fWidth = w;
        desc.fHeight = h;
        desc.fConfig = config;

        info.fTarget = GR_GL_TEXTURE_2D;
        info.fFormat = GrGLFormatToEnum(glFormat);
        // TODO: Take these as parameters.
        auto srcColorType = GrPixelConfigToColorType(desc.fConfig);
        info.fID = this->createTexture2D({desc.fWidth, desc.fHeight},
                                         glFormat,
                                         renderable,
                                         &initialState,
                                         textureColorType,
                                         srcColorType,
                                         texels,
                                         mipLevelCount,
                                         nullptr);
        if (!info.fID) {
            return GrBackendTexture();  // invalid
        }
    }

    // unbind the texture from the texture unit to avoid asserts
    GL_CALL(BindTexture(info.fTarget, 0));

    auto parameters = sk_make_sp<GrGLTextureParameters>();
    parameters->set(&initialState, GrGLTextureParameters::NonsamplerState(),
                    fResetTimestampForTextureParameters);

    return GrBackendTexture(w, h, mipMapped, info, std::move(parameters));
}

void GrGLGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kOpenGL == tex.backend());

    GrGLTextureInfo info;
    if (tex.getGLTextureInfo(&info)) {
        GL_CALL(DeleteTextures(1, &info.fID));
    }
}

#if GR_TEST_UTILS

bool GrGLGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kOpenGL == tex.backend());

    GrGLTextureInfo info;
    if (!tex.getGLTextureInfo(&info)) {
        return false;
    }

    GrGLboolean result;
    GL_CALL_RET(result, IsTexture(info.fID));

    return (GR_GL_TRUE == result);
}

GrBackendRenderTarget GrGLGpu::createTestingOnlyBackendRenderTarget(int w, int h,
                                                                    GrColorType colorType) {
    if (w > this->caps()->maxRenderTargetSize() || h > this->caps()->maxRenderTargetSize()) {
        return GrBackendRenderTarget();  // invalid
    }
    this->handleDirtyContext();
    auto format = this->glCaps().getFormatFromColorType(colorType);
    if (!this->glCaps().isFormatRenderable(format, 1)) {
        return {};
    }
    bool useTexture = false;
    GrGLenum colorBufferFormat;
    GrGLenum externalFormat = 0, externalType = 0;
    if (format == GrGLFormat::kBGRA8) {
        // BGRA render buffers are not supported.
        this->glCaps().getTexImageFormats(format, colorType, colorType, &colorBufferFormat,
                                          &externalFormat, &externalType);
        useTexture = true;
    } else {
        colorBufferFormat = this->glCaps().getRenderbufferInternalFormat(format);
    }
    int sFormatIdx = this->getCompatibleStencilIndex(format);
    if (sFormatIdx < 0) {
        return {};
    }
    GrGLuint colorID = 0;
    GrGLuint stencilID = 0;
    auto deleteIDs = [&] {
        if (colorID) {
            if (useTexture) {
                GL_CALL(DeleteTextures(1, &colorID));
            } else {
                GL_CALL(DeleteRenderbuffers(1, &colorID));
            }
        }
        if (stencilID) {
            GL_CALL(DeleteRenderbuffers(1, &stencilID));
        }
    };

    if (useTexture) {
        GL_CALL(GenTextures(1, &colorID));
    } else {
        GL_CALL(GenRenderbuffers(1, &colorID));
    }
    GL_CALL(GenRenderbuffers(1, &stencilID));
    if (!stencilID || !colorID) {
        deleteIDs();
        return {};
    }

    GrGLFramebufferInfo info;
    info.fFBOID = 0;
    info.fFormat = this->glCaps().formatSizedInternalFormat(format);
    GL_CALL(GenFramebuffers(1, &info.fFBOID));
    if (!info.fFBOID) {
        deleteIDs();
        return {};
    }

    this->invalidateBoundRenderTarget();

    this->bindFramebuffer(GR_GL_FRAMEBUFFER, info.fFBOID);
    if (useTexture) {
        this->bindTextureToScratchUnit(GR_GL_TEXTURE_2D, colorID);
        GL_CALL(TexImage2D(GR_GL_TEXTURE_2D, 0, colorBufferFormat, w, h, 0, externalFormat,
                           externalType, nullptr));
        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0, GR_GL_TEXTURE_2D,
                                     colorID, 0));
    } else {
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, colorID));
        GL_ALLOC_CALL(this->glInterface(),
                      RenderbufferStorage(GR_GL_RENDERBUFFER, colorBufferFormat, w, h));
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_RENDERBUFFER, colorID));
    }
    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, stencilID));
    auto stencilBufferFormat = this->glCaps().stencilFormats()[sFormatIdx].fInternalFormat;
    GL_ALLOC_CALL(this->glInterface(),
                  RenderbufferStorage(GR_GL_RENDERBUFFER, stencilBufferFormat, w, h));
    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT, GR_GL_RENDERBUFFER,
                                    stencilID));
    if (this->glCaps().stencilFormats()[sFormatIdx].fPacked) {
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_DEPTH_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, stencilID));
    }

    // We don't want to have to recover the renderbuffer/texture IDs later to delete them. OpenGL
    // has this rule that if a renderbuffer/texture is deleted and a FBO other than the current FBO
    // has the RB attached then deletion is delayed. So we unbind the FBO here and delete the
    // renderbuffers/texture.
    this->bindFramebuffer(GR_GL_FRAMEBUFFER, 0);
    deleteIDs();

    this->bindFramebuffer(GR_GL_FRAMEBUFFER, info.fFBOID);
    GrGLenum status;
    GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    if (GR_GL_FRAMEBUFFER_COMPLETE != status) {
        this->deleteFramebuffer(info.fFBOID);
        return {};
    }
    auto stencilBits = SkToInt(this->glCaps().stencilFormats()[sFormatIdx].fStencilBits);

    GrBackendRenderTarget beRT = GrBackendRenderTarget(w, h, 1, stencilBits, info);
    SkASSERT(this->caps()->areColorTypeAndFormatCompatible(colorType, beRT.getBackendFormat()));
    return beRT;
}

void GrGLGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    SkASSERT(GrBackendApi::kOpenGL == backendRT.backend());
    GrGLFramebufferInfo info;
    if (backendRT.getGLFramebufferInfo(&info)) {
        if (info.fFBOID) {
            this->deleteFramebuffer(info.fFBOID);
        }
    }
}

void GrGLGpu::testingOnly_flushGpuAndSync() {
    GL_CALL(Finish());
}
#endif

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
            gpu->bindBuffer(GrGpuBufferType::kIndex, ibuf);
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

void GrGLGpu::onFinishFlush(GrSurfaceProxy*[], int, SkSurface::BackendSurfaceAccess access,
                            const GrFlushInfo& info, const GrPrepareForExternalIORequests&) {
    // If we inserted semaphores during the flush, we need to call GLFlush.
    bool insertedSemaphore = info.fNumSemaphores > 0 && this->caps()->semaphoreSupport();
    // We call finish if the client told us to sync or if we have a finished proc but don't support
    // GLsync objects.
    bool finish = (info.fFlags & kSyncCpu_GrFlushFlag) ||
                  (info.fFinishedProc && !this->caps()->fenceSyncSupport());
    if (finish) {
        GL_CALL(Finish());
        // After a finish everything previously sent to GL is done.
        for (const auto& cb : fFinishCallbacks) {
            cb.fCallback(cb.fContext);
            this->deleteSync(cb.fSync);
        }
        fFinishCallbacks.clear();
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
    } else {
        if (info.fFinishedProc) {
            FinishCallback callback;
            callback.fCallback = info.fFinishedProc;
            callback.fContext = info.fFinishedContext;
            callback.fSync = (GrGLsync)this->insertFence();
            fFinishCallbacks.push_back(callback);
            GL_CALL(Flush());
        } else if (insertedSemaphore) {
            // Must call flush after semaphores in case they are waited on another GL context.
            GL_CALL(Flush());
        }
        // See if any previously inserted finish procs are good to go.
        this->checkFinishProcs();
    }
}

void GrGLGpu::submit(GrGpuCommandBuffer* buffer) {
    if (buffer->asRTCommandBuffer()) {
        SkASSERT(fCachedRTCommandBuffer.get() == buffer);
        fCachedRTCommandBuffer->reset();
    } else {
        SkASSERT(fCachedTexCommandBuffer.get() == buffer);
        fCachedTexCommandBuffer->reset();
    }
}

GrFence SK_WARN_UNUSED_RESULT GrGLGpu::insertFence() {
    SkASSERT(this->caps()->fenceSyncSupport());
    GrGLsync sync;
    GL_CALL_RET(sync, FenceSync(GR_GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    GR_STATIC_ASSERT(sizeof(GrFence) >= sizeof(GrGLsync));
    return (GrFence)sync;
}

bool GrGLGpu::waitSync(GrGLsync sync, uint64_t timeout, bool flush) {
    GrGLbitfield flags = flush ? GR_GL_SYNC_FLUSH_COMMANDS_BIT : 0;
    GrGLenum result;
    GL_CALL_RET(result, ClientWaitSync(sync, flags, timeout));
    return (GR_GL_CONDITION_SATISFIED == result || GR_GL_ALREADY_SIGNALED == result);
}

bool GrGLGpu::waitFence(GrFence fence, uint64_t timeout) {
    return this->waitSync((GrGLsync)fence, timeout, /* flush = */ true);
}

void GrGLGpu::deleteFence(GrFence fence) const {
    this->deleteSync((GrGLsync)fence);
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrGLGpu::makeSemaphore(bool isOwned) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrGLSemaphore::Make(this, isOwned);
}

sk_sp<GrSemaphore> GrGLGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                 GrResourceProvider::SemaphoreWrapType wrapType,
                                                 GrWrapOwnership ownership) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrGLSemaphore::MakeWrapped(this, semaphore.glSync(), ownership);
}

void GrGLGpu::insertSemaphore(sk_sp<GrSemaphore> semaphore) {
    GrGLSemaphore* glSem = static_cast<GrGLSemaphore*>(semaphore.get());

    GrGLsync sync;
    GL_CALL_RET(sync, FenceSync(GR_GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    glSem->setSync(sync);
}

void GrGLGpu::waitSemaphore(sk_sp<GrSemaphore> semaphore) {
    GrGLSemaphore* glSem = static_cast<GrGLSemaphore*>(semaphore.get());

    GL_CALL(WaitSync(glSem->sync(), 0, GR_GL_TIMEOUT_IGNORED));
}

void GrGLGpu::checkFinishProcs() {
    // Bail after the first unfinished sync since we expect they signal in the order inserted.
    while (!fFinishCallbacks.empty() && this->waitSync(fFinishCallbacks.front().fSync,
                                                       /* timeout = */ 0, /* flush  = */ false)) {
        fFinishCallbacks.front().fCallback(fFinishCallbacks.front().fContext);
        this->deleteSync(fFinishCallbacks.front().fSync);
        fFinishCallbacks.pop_front();
    }
}

void GrGLGpu::deleteSync(GrGLsync sync) const {
    GL_CALL(DeleteSync(sync));
}

void GrGLGpu::insertEventMarker(const char* msg) {
    GL_CALL(InsertEventMarker(strlen(msg), msg));
}

sk_sp<GrSemaphore> GrGLGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    // Set up a semaphore to be signaled once the data is ready, and flush GL
    sk_sp<GrSemaphore> semaphore = this->makeSemaphore(true);
    this->insertSemaphore(semaphore);
    // We must call flush here to make sure the GrGLSync object gets created and sent to the gpu.
    GL_CALL(Flush());

    return semaphore;
}

int GrGLGpu::TextureToCopyProgramIdx(GrTexture* texture) {
    switch (GrSLCombinedSamplerTypeForTextureType(texture->texturePriv().textureType())) {
        case kTexture2DSampler_GrSLType:
            return 0;
        case kTexture2DRectSampler_GrSLType:
            return 1;
        case kTextureExternalSampler_GrSLType:
            return 2;
        default:
            SK_ABORT("Unexpected samper type");
    }
}

#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"
void GrGLGpu::onDumpJSON(SkJSONWriter* writer) const {
    // We are called by the base class, which has already called beginObject(). We choose to nest
    // all of our caps information in a named sub-object.
    writer->beginObject("GL GPU");

    const GrGLubyte* str;
    GL_CALL_RET(str, GetString(GR_GL_VERSION));
    writer->appendString("GL_VERSION", (const char*)(str));
    GL_CALL_RET(str, GetString(GR_GL_RENDERER));
    writer->appendString("GL_RENDERER", (const char*)(str));
    GL_CALL_RET(str, GetString(GR_GL_VENDOR));
    writer->appendString("GL_VENDOR", (const char*)(str));
    GL_CALL_RET(str, GetString(GR_GL_SHADING_LANGUAGE_VERSION));
    writer->appendString("GL_SHADING_LANGUAGE_VERSION", (const char*)(str));

    writer->appendName("extensions");
    glInterface()->fExtensions.dumpJSON(writer);

    writer->endObject();
}
#endif
