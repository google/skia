/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLGpu.h"
#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrCpuBuffer.h"
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
#include "GrSurfaceProxyPriv.h"
#include "GrTexturePriv.h"
#include "GrTypes.h"
#include "SkAutoMalloc.h"
#include "SkConvertPixels.h"
#include "SkHalf.h"
#include "SkMakeUnique.h"
#include "SkMipMap.h"
#include "SkPixmap.h"
#include "SkSLCompiler.h"
#include "SkStrokeRec.h"
#include "SkTemplates.h"
#include "SkTo.h"
#include "SkTraceEvent.h"
#include "SkTypes.h"
#include "builders/GrGLShaderStringBuilder.h"

#include <cmath>

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
    return 0;
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
    return 0;
}

static GrGLenum filter_to_gl_min_filter(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest: return GR_GL_NEAREST;
        case GrSamplerState::Filter::kBilerp:  return GR_GL_LINEAR;
        case GrSamplerState::Filter::kMipMap:  return GR_GL_LINEAR_MIPMAP_LINEAR;
    }
    SK_ABORT("Unknown filter");
    return 0;
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
    return 0;
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
        GR_GL_CALL(fGpu->glInterface(), DeleteSamplers(kNumSamplers, fSamplers));
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

        if (this->caps()->usesMixedSamples()) {
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
        if (this->glCaps().unpackRowLengthSupport()) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
        }
        if (this->glCaps().packRowLengthSupport()) {
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
}

static bool check_backend_texture(const GrBackendTexture& backendTex, const GrGLCaps& caps,
                                  GrGLTexture::IDDesc* idDesc) {
    GrGLTextureInfo info;
    if (!backendTex.getGLTextureInfo(&info) || !info.fID) {
        return false;
    }

    idDesc->fInfo = info;

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
                                               GrWrapOwnership ownership, GrWrapCacheable cacheable,
                                               GrIOType ioType) {
    GrGLTexture::IDDesc idDesc;
    if (!check_backend_texture(backendTex, this->glCaps(), &idDesc)) {
        return nullptr;
    }
    if (!idDesc.fInfo.fFormat) {
        idDesc.fInfo.fFormat = this->glCaps().configSizedInternalFormat(backendTex.config());
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
    surfDesc.fSampleCnt = 1;

    GrMipMapsStatus mipMapsStatus = backendTex.hasMipMaps() ? GrMipMapsStatus::kValid
                                                            : GrMipMapsStatus::kNotAllocated;

    auto texture =
            GrGLTexture::MakeWrapped(this, surfDesc, mipMapsStatus, idDesc, cacheable, ioType);
    // We don't know what parameters are already set on wrapped textures.
    texture->textureParamsModified();
    return std::move(texture);
}

sk_sp<GrTexture> GrGLGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                         int sampleCnt,
                                                         GrWrapOwnership ownership,
                                                         GrWrapCacheable cacheable) {
    GrGLTexture::IDDesc idDesc;
    if (!check_backend_texture(backendTex, this->glCaps(), &idDesc)) {
        return nullptr;
    }
    if (!idDesc.fInfo.fFormat) {
        idDesc.fInfo.fFormat = this->glCaps().configSizedInternalFormat(backendTex.config());
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
    surfDesc.fSampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, backendTex.config());
    if (surfDesc.fSampleCnt < 1) {
        return nullptr;
    }

    GrGLRenderTarget::IDDesc rtIDDesc;
    if (!this->createRenderTargetObjects(surfDesc, idDesc.fInfo, &rtIDDesc)) {
        return nullptr;
    }

    GrMipMapsStatus mipMapsStatus = backendTex.hasMipMaps() ? GrMipMapsStatus::kDirty
                                                            : GrMipMapsStatus::kNotAllocated;

    sk_sp<GrGLTextureRenderTarget> texRT(GrGLTextureRenderTarget::MakeWrapped(
            this, surfDesc, idDesc, rtIDDesc, cacheable, mipMapsStatus));
    texRT->baseLevelWasBoundToFBO();
    // We don't know what parameters are already set on wrapped textures.
    texRT->textureParamsModified();
    return std::move(texRT);
}

sk_sp<GrRenderTarget> GrGLGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
    GrGLFramebufferInfo info;
    if (!backendRT.getGLFramebufferInfo(&info)) {
        return nullptr;
    }

    GrGLRenderTarget::IDDesc idDesc;
    idDesc.fRTFBOID = info.fFBOID;
    idDesc.fMSColorRenderbufferID = 0;
    idDesc.fTexFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    idDesc.fRTFBOOwnership = GrBackendObjectOwnership::kBorrowed;
    idDesc.fIsMixedSampled = false;

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = backendRT.width();
    desc.fHeight = backendRT.height();
    desc.fConfig = backendRT.config();
    desc.fSampleCnt =
            this->caps()->getRenderTargetSampleCount(backendRT.sampleCnt(), backendRT.config());

    return GrGLRenderTarget::MakeWrapped(this, desc, info.fFormat, idDesc, backendRT.stencilBits());
}

sk_sp<GrRenderTarget> GrGLGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                  int sampleCnt) {
    GrGLTextureInfo info;
    if (!tex.getGLTextureInfo(&info) || !info.fID) {
        return nullptr;
    }

    if (GR_GL_TEXTURE_RECTANGLE != info.fTarget &&
        GR_GL_TEXTURE_2D != info.fTarget) {
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
    surfDesc.fSampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.config());

    GrGLRenderTarget::IDDesc rtIDDesc;
    if (!this->createRenderTargetObjects(surfDesc, info, &rtIDDesc)) {
        return nullptr;
    }
    return GrGLRenderTarget::MakeWrapped(this, surfDesc, info.fFormat, rtIDDesc, 0);
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
                            GrColorType srcColorType, const GrMipLevel texels[],
                            int mipLevelCount) {
    auto glTex = static_cast<GrGLTexture*>(surface->asTexture());

    if (!check_write_and_transfer_input(glTex)) {
        return false;
    }

    this->bindTextureToScratchUnit(glTex->target(), glTex->textureID());

    // No sRGB transformation occurs in uploadTexData. We choose to make the src config match the
    // srgb-ness of the surface to avoid issues in ES2 where internal/external formats must match.
    // When we're on ES2 and the dst is GL_SRGB_ALPHA by making the config be kSRGB_8888 we know
    // that our caps will choose GL_SRGB_ALPHA as the external format, too. On ES3 or regular GL our
    // caps knows to make the external format be GL_RGBA.
    auto srgbEncoded = GrPixelConfigIsSRGBEncoded(surface->config());
    auto srcAsConfig = GrColorTypeToPixelConfig(srcColorType, srgbEncoded);

    SkASSERT(!GrPixelConfigIsCompressed(glTex->config()));
    return this->uploadTexData(glTex->config(), glTex->width(), glTex->height(), glTex->target(),
                               kWrite_UploadType, left, top, width, height, srcAsConfig, texels,
                               mipLevelCount);
}

// For GL_[UN]PACK_ALIGNMENT. TODO: This really wants to be GrColorType.
static inline GrGLint config_alignment(GrPixelConfig config) {
    SkASSERT(!GrPixelConfigIsCompressed(config));
    switch (config) {
        case kAlpha_8_GrPixelConfig:
        case kAlpha_8_as_Alpha_GrPixelConfig:
        case kAlpha_8_as_Red_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Lum_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
            return 1;
        case kRGB_565_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRG_88_GrPixelConfig:
        case kAlpha_half_GrPixelConfig:
        case kAlpha_half_as_Red_GrPixelConfig:
        case kRGBA_half_GrPixelConfig:
        case kRGBA_half_Clamped_GrPixelConfig:
            return 2;
        case kRGBA_8888_GrPixelConfig:
        case kRGB_888_GrPixelConfig:  // We're really talking about GrColorType::kRGB_888x here.
        case kRGB_888X_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kSRGBA_8888_GrPixelConfig:
        case kSBGRA_8888_GrPixelConfig:
        case kRGBA_1010102_GrPixelConfig:
        case kRGBA_float_GrPixelConfig:
        case kRG_float_GrPixelConfig:
            return 4;
        case kRGB_ETC1_GrPixelConfig:
        case kUnknown_GrPixelConfig:
            return 0;
    }
    SK_ABORT("Invalid pixel config");
    return 0;
}

bool GrGLGpu::onTransferPixels(GrTexture* texture, int left, int top, int width, int height,
                               GrColorType bufferColorType, GrGpuBuffer* transferBuffer,
                               size_t offset, size_t rowBytes) {
    GrGLTexture* glTex = static_cast<GrGLTexture*>(texture);
    GrPixelConfig texConfig = glTex->config();
    SkASSERT(this->caps()->isConfigTexturable(texConfig));

    // Can't transfer compressed data
    SkASSERT(!GrPixelConfigIsCompressed(glTex->config()));

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

    int bpp = GrColorTypeBytesPerPixel(bufferColorType);
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
    auto bufferAsConfig = GrColorTypeToPixelConfig(bufferColorType, GrSRGBEncoded::kNo);
    if (!this->glCaps().getTexImageFormats(texConfig, bufferAsConfig, &internalFormat,
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
 * @param target         Which bound texture to target (GR_GL_TEXTURE_2D, e.g.)
 * @param internalFormat The data format used for the internal storage of the texture. May be sized.
 * @param internalFormatForTexStorage The data format used for the TexStorage API. Must be sized.
 * @param externalFormat The data format used for the external storage of the texture.
 * @param externalType   The type of the data used for the external storage of the texture.
 * @param texels         The texel data of the texture being created.
 * @param mipLevelCount  Number of mipmap levels
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
    // mipmaps.
    useTexStorage &= mipLevelCount > 1;

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
 * Creates storage space for the texture and fills it with texels.
 *
 * @param config         Compressed pixel config of the texture.
 * @param interface      The GL interface in use.
 * @param caps           The capabilities of the GL device.
 * @param target         Which bound texture to target (GR_GL_TEXTURE_2D, e.g.)
 * @param internalFormat The data format used for the internal storage of the texture.
 * @param texels         The texel data of the texture being created.
 * @param mipLevelCount  Number of mipmap levels
 * @param baseWidth      The width of the texture's base mipmap level
 * @param baseHeight     The height of the texture's base mipmap level
 */
static bool allocate_and_populate_compressed_texture(GrPixelConfig config,
                                                     const GrGLInterface& interface,
                                                     const GrGLCaps& caps,
                                                     GrGLenum target, GrGLenum internalFormat,
                                                     const GrMipLevel texels[], int mipLevelCount,
                                                     int baseWidth, int baseHeight) {
    CLEAR_ERROR_BEFORE_ALLOC(&interface);
    SkASSERT(GrPixelConfigIsCompressed(config));

    bool useTexStorage = caps.isConfigTexSupportEnabled(config);
    // We can only use TexStorage if we know we will not later change the storage requirements.
    // This means if we may later want to add mipmaps, we cannot use TexStorage.
    // Right now, we cannot know if we will later add mipmaps or not.
    // The only time we can use TexStorage is when we already have the
    // mipmaps.
    useTexStorage &= mipLevelCount > 1;

    if (useTexStorage) {
        // We never resize or change formats of textures.
        GL_ALLOC_CALL(&interface,
                      TexStorage2D(target,
                                   mipLevelCount,
                                   internalFormat,
                                   baseWidth, baseHeight));
        GrGLenum error = CHECK_ALLOC_ERROR(&interface);
        if (error != GR_GL_NO_ERROR) {
            return false;
        } else {
            for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
                const void* currentMipData = texels[currentMipLevel].fPixels;
                if (currentMipData == nullptr) {
                    // Compressed textures require data for every level
                    return false;
                }

                int twoToTheMipLevel = 1 << currentMipLevel;
                int currentWidth = SkTMax(1, baseWidth / twoToTheMipLevel);
                int currentHeight = SkTMax(1, baseHeight / twoToTheMipLevel);

                // Make sure that the width and height that we pass to OpenGL
                // is a multiple of the block size.
                size_t dataSize = GrCompressedFormatDataSize(config, currentWidth, currentHeight);
                GR_GL_CALL(&interface, CompressedTexSubImage2D(target,
                                                               currentMipLevel,
                                                               0, // left
                                                               0, // top
                                                               currentWidth,
                                                               currentHeight,
                                                               internalFormat,
                                                               SkToInt(dataSize),
                                                               currentMipData));
            }
        }
    } else {
        for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
            const void* currentMipData = texels[currentMipLevel].fPixels;
            if (currentMipData == nullptr) {
                // Compressed textures require data for every level
                return false;
            }

            int twoToTheMipLevel = 1 << currentMipLevel;
            int currentWidth = SkTMax(1, baseWidth / twoToTheMipLevel);
            int currentHeight = SkTMax(1, baseHeight / twoToTheMipLevel);

            // Make sure that the width and height that we pass to OpenGL
            // is a multiple of the block size.
            size_t dataSize = GrCompressedFormatDataSize(config, baseWidth, baseHeight);

            GL_ALLOC_CALL(&interface,
                          CompressedTexImage2D(target,
                                               currentMipLevel,
                                               internalFormat,
                                               currentWidth,
                                               currentHeight,
                                               0, // border
                                               SkToInt(dataSize),
                                               currentMipData));

            GrGLenum error = CHECK_ALLOC_ERROR(&interface);
            if (error != GR_GL_NO_ERROR) {
                return false;
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
        SkASSERT(caps.unpackRowLengthSupport());
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

// TODO: Make this take a GrColorType instead of dataConfig. This requires updating GrGLCaps to
// convert from GrColorType to externalFormat/externalType GLenum values.
bool GrGLGpu::uploadTexData(GrPixelConfig texConfig, int texWidth, int texHeight, GrGLenum target,
                            UploadType uploadType, int left, int top, int width, int height,
                            GrPixelConfig dataConfig, const GrMipLevel texels[], int mipLevelCount,
                            GrMipMapsStatus* mipMapsStatus) {
    // If we're uploading compressed data then we should be using uploadCompressedTexData
    SkASSERT(!GrPixelConfigIsCompressed(dataConfig));

    SkASSERT(this->caps()->isConfigTexturable(texConfig));
    SkDEBUGCODE(
        SkIRect subRect = SkIRect::MakeXYWH(left, top, width, height);
        SkIRect bounds = SkIRect::MakeWH(texWidth, texHeight);
        SkASSERT(bounds.contains(subRect));
    )
    SkASSERT(1 == mipLevelCount ||
             (0 == left && 0 == top && width == texWidth && height == texHeight));

    this->unbindCpuToGpuXferBuffer();

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

    // in case we need a temporary, trimmed copy of the src pixels
    SkAutoSMalloc<128 * 128> tempStorage;

    if (mipMapsStatus) {
        *mipMapsStatus = GrMipMapsStatus::kValid;
    }

    const bool usesMips = mipLevelCount > 1;

    // find the combined size of all the mip levels and the relative offset of
    // each into the collective buffer
    bool willNeedData = false;
    size_t combinedBufferSize = 0;
    SkTArray<size_t> individualMipOffsets(mipLevelCount);
    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (texelsShallowCopy[currentMipLevel].fPixels) {
            int twoToTheMipLevel = 1 << currentMipLevel;
            int currentWidth = SkTMax(1, width / twoToTheMipLevel);
            int currentHeight = SkTMax(1, height / twoToTheMipLevel);
            const size_t trimRowBytes = currentWidth * bpp;
            const size_t trimmedSize = trimRowBytes * currentHeight;

            const size_t rowBytes = texelsShallowCopy[currentMipLevel].fRowBytes
                    ? texelsShallowCopy[currentMipLevel].fRowBytes
                    : trimRowBytes;

            if (((!caps.unpackRowLengthSupport() || usesMips) && trimRowBytes != rowBytes)) {
                willNeedData = true;
            }

            individualMipOffsets.push_back(combinedBufferSize);
            combinedBufferSize += trimmedSize;
        } else {
            if (mipMapsStatus) {
                *mipMapsStatus = GrMipMapsStatus::kDirty;
            }
            individualMipOffsets.push_back(0);
        }
    }
    if (mipMapsStatus && mipLevelCount <= 1) {
        *mipMapsStatus = GrMipMapsStatus::kNotAllocated;
    }
    char* buffer = nullptr;
    if (willNeedData) {
        buffer = (char*)tempStorage.reset(combinedBufferSize);
    }

    for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
        if (!texelsShallowCopy[currentMipLevel].fPixels) {
            continue;
        }
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

        const size_t rowBytes = texelsShallowCopy[currentMipLevel].fRowBytes
                ? texelsShallowCopy[currentMipLevel].fRowBytes
                : trimRowBytes;

        // TODO: This optimization should be enabled with or without mips.
        // For use with mips, we must set GR_GL_UNPACK_ROW_LENGTH once per
        // mip level, before calling glTexImage2D.
        if (caps.unpackRowLengthSupport() && !usesMips) {
            // can't use this for flipping, only non-neg values allowed. :(
            if (rowBytes != trimRowBytes) {
                GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
                GR_GL_CALL(interface, PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
                restoreGLRowLength = true;
            }
        } else if (trimRowBytes != rowBytes) {
            // copy data into our new storage, skipping the trailing bytes
            const char* src = (const char*)texelsShallowCopy[currentMipLevel].fPixels;
            char* dst = buffer + individualMipOffsets[currentMipLevel];
            SkRectMemcpy(dst, trimRowBytes, src, rowBytes, trimRowBytes, currentHeight);
            // now point data to our copied version
            texelsShallowCopy[currentMipLevel].fPixels = buffer +
                individualMipOffsets[currentMipLevel];
            texelsShallowCopy[currentMipLevel].fRowBytes = trimRowBytes;
        }
    }

    if (mipLevelCount) {
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
        for (int currentMipLevel = 0; currentMipLevel < mipLevelCount; currentMipLevel++) {
            if (!texelsShallowCopy[currentMipLevel].fPixels) {
                continue;
            }
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

    restore_pixelstore_state(*interface, caps, restoreGLRowLength);

    return succeeded;
}

bool GrGLGpu::uploadCompressedTexData(GrPixelConfig texConfig, int texWidth, int texHeight,
                                      GrGLenum target, GrPixelConfig dataConfig,
                                      const GrMipLevel texels[], int mipLevelCount,
                                      GrMipMapsStatus* mipMapsStatus) {
    SkASSERT(this->caps()->isConfigTexturable(texConfig));

    const GrGLInterface* interface = this->glInterface();
    const GrGLCaps& caps = this->glCaps();

    // We only need the internal format for compressed 2D textures.
    GrGLenum internalFormat;
    if (!caps.getCompressedTexImageFormats(texConfig, &internalFormat)) {
        return false;
    }

    if (mipMapsStatus) {
        if (mipLevelCount <= 1) {
            *mipMapsStatus = GrMipMapsStatus::kNotAllocated;
        } else {
            *mipMapsStatus = GrMipMapsStatus::kValid;
        }
    }

    return allocate_and_populate_compressed_texture(texConfig, *interface, caps, target,
                                                    internalFormat, texels, mipLevelCount,
                                                    texWidth, texHeight);
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
            SK_ABORT("Shouldn't be here if we don't support multisampled renderbuffers.");
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
    idDesc->fIsMixedSampled = desc.fSampleCnt > 1 && this->caps()->usesMixedSamples();

    GrGLenum status;

    GrGLenum colorRenderbufferFormat = 0; // suppress warning

    if (desc.fSampleCnt > 1 && GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType()) {
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
    if (desc.fSampleCnt > 1 && this->glCaps().usesMSAARenderBuffers()) {
        GL_CALL(GenFramebuffers(1, &idDesc->fRTFBOID));
        GL_CALL(GenRenderbuffers(1, &idDesc->fMSColorRenderbufferID));
        if (!idDesc->fRTFBOID ||
            !idDesc->fMSColorRenderbufferID) {
            goto FAILED;
        }
        this->glCaps().getRenderbufferFormat(desc.fConfig, &colorRenderbufferFormat);
    } else {
        idDesc->fRTFBOID = idDesc->fTexFBOID;
    }

    // below here we may bind the FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();
    if (idDesc->fRTFBOID != idDesc->fTexFBOID) {
        SkASSERT(desc.fSampleCnt > 1);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, idDesc->fMSColorRenderbufferID));
        if (!renderbuffer_storage_msaa(*fGLContext,
                                       desc.fSampleCnt,
                                       colorRenderbufferFormat,
                                       desc.fWidth, desc.fHeight)) {
            goto FAILED;
        }
        this->bindFramebuffer(GR_GL_FRAMEBUFFER, idDesc->fRTFBOID);
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
    this->bindFramebuffer(GR_GL_FRAMEBUFFER, idDesc->fTexFBOID);

    if (this->glCaps().usesImplicitMSAAResolve() && desc.fSampleCnt > 1) {
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
        this->deleteFramebuffer(idDesc->fRTFBOID);
    }
    if (idDesc->fTexFBOID) {
        this->deleteFramebuffer(idDesc->fTexFBOID);
    }
    return false;
}

// good to set a break-point here to know when createTexture fails
static sk_sp<GrTexture> return_null_texture() {
//    SkDEBUGFAIL("null texture");
    return nullptr;
}

static GrGLTexture::SamplerParams set_initial_texture_params(const GrGLInterface* interface,
                                                             const GrGLTextureInfo& info) {
    // Some drivers like to know filter/wrap before seeing glTexImage2D. Some
    // drivers have a bug where an FBO won't be complete if it includes a
    // texture that is not mipmap complete (considering the filter in use).
    GrGLTexture::SamplerParams params;
    params.fMinFilter = GR_GL_NEAREST;
    params.fMagFilter = GR_GL_NEAREST;
    params.fWrapS = GR_GL_CLAMP_TO_EDGE;
    params.fWrapT = GR_GL_CLAMP_TO_EDGE;
    GR_GL_CALL(interface, TexParameteri(info.fTarget, GR_GL_TEXTURE_MAG_FILTER, params.fMagFilter));
    GR_GL_CALL(interface, TexParameteri(info.fTarget, GR_GL_TEXTURE_MIN_FILTER, params.fMinFilter));
    GR_GL_CALL(interface, TexParameteri(info.fTarget, GR_GL_TEXTURE_WRAP_S, params.fWrapS));
    GR_GL_CALL(interface, TexParameteri(info.fTarget, GR_GL_TEXTURE_WRAP_T, params.fWrapT));
    return params;
}

sk_sp<GrTexture> GrGLGpu::onCreateTexture(const GrSurfaceDesc& desc,
                                          SkBudgeted budgeted,
                                          const GrMipLevel texels[],
                                          int mipLevelCount) {
    // We fail if the MSAA was requested and is not available.
    if (GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType() && desc.fSampleCnt > 1) {
        //SkDebugf("MSAA RT requested but not supported on this platform.");
        return return_null_texture();
    }

    bool performClear = (desc.fFlags & kPerformInitialClear_GrSurfaceFlag) &&
                        !GrPixelConfigIsCompressed(desc.fConfig);

    GrMipLevel zeroLevel;
    std::unique_ptr<uint8_t[]> zeros;
    if (performClear && !this->glCaps().clearTextureSupport() &&
        !this->glCaps().canConfigBeFBOColorAttachment(desc.fConfig)) {
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
    GrMipMapsStatus mipMapsStatus;
    GrGLTexture::SamplerParams initialTexParams;
    if (!this->createTextureImpl(desc, &idDesc.fInfo, isRenderTarget, &initialTexParams, texels,
                                 mipLevelCount, &mipMapsStatus)) {
        return return_null_texture();
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
                                                  mipMapsStatus);
        tex->baseLevelWasBoundToFBO();
    } else {
        tex = sk_make_sp<GrGLTexture>(this, budgeted, desc, idDesc, mipMapsStatus);
    }

    tex->setCachedParams(&initialTexParams, tex->getCachedNonSamplerParams(),
                         this->getResetTimestamp());
#ifdef TRACE_TEXTURE_CREATION
    SkDebugf("--- new texture [%d] size=(%d %d) config=%d\n",
             idDesc.fInfo.fID, desc.fWidth, desc.fHeight, desc.fConfig);
#endif
    if (tex && performClear) {
        if (this->glCaps().clearTextureSupport()) {
            static constexpr uint32_t kZero = 0;
            GL_CALL(ClearTexImage(tex->textureID(), 0, GR_GL_RGBA, GR_GL_UNSIGNED_BYTE, &kZero));
        } else {
            GrGLIRect viewport;
            this->bindSurfaceFBOForPixelOps(tex.get(), GR_GL_FRAMEBUFFER, &viewport,
                                            kDst_TempFBOTarget);
            this->disableScissor();
            this->disableWindowRectangles();
            this->flushColorWrite(true);
            this->flushClearColor(0, 0, 0, 0);
            GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
            this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, tex.get());
            fHWBoundRenderTargetUniqueID.makeInvalid();
        }
    }
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

int GrGLGpu::getCompatibleStencilIndex(GrPixelConfig config) {
    static const int kSize = 16;
    SkASSERT(this->caps()->isConfigRenderable(config));
    if (!this->glCaps().hasStencilFormatBeenDeterminedForConfig(config)) {
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

        GrGLenum internalFormat;
        GrGLenum externalFormat;
        GrGLenum externalType;
        if (!this->glCaps().getTexImageFormats(config, config, &internalFormat, &externalFormat,
                                               &externalType)) {
            return false;
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
        fGLContext->caps()->setStencilFormatIndexForConfig(config, firstWorkingStencilFormatIndex);
    }
    return this->glCaps().getStencilFormatIndexForConfig(config);
}

bool GrGLGpu::createTextureImpl(const GrSurfaceDesc& desc, GrGLTextureInfo* info, bool renderTarget,
                                GrGLTexture::SamplerParams* initialTexParams,
                                const GrMipLevel texels[], int mipLevelCount,
                                GrMipMapsStatus* mipMapsStatus) {
    info->fID = 0;
    info->fTarget = GR_GL_TEXTURE_2D;
    GL_CALL(GenTextures(1, &(info->fID)));

    if (!info->fID) {
        return false;
    }

    this->bindTextureToScratchUnit(info->fTarget, info->fID);

    if (renderTarget && this->glCaps().textureUsageSupport()) {
        // provides a hint about how this texture will be used
        GL_CALL(TexParameteri(info->fTarget,
                              GR_GL_TEXTURE_USAGE,
                              GR_GL_FRAMEBUFFER_ATTACHMENT));
    }

    if (info) {
        *initialTexParams = set_initial_texture_params(this->glInterface(), *info);
    }

    bool success = false;
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        SkASSERT(!renderTarget);
        success = this->uploadCompressedTexData(desc.fConfig, desc.fWidth, desc.fHeight,
                                                info->fTarget, desc.fConfig,
                                                texels, mipLevelCount, mipMapsStatus);
    } else {
        success = this->uploadTexData(desc.fConfig, desc.fWidth, desc.fHeight, info->fTarget,
                                      kNewTexture_UploadType, 0, 0, desc.fWidth, desc.fHeight,
                                      desc.fConfig, texels, mipLevelCount, mipMapsStatus);
    }
    if (!success) {
        GL_CALL(DeleteTextures(1, &(info->fID)));
        return false;
    }
    info->fFormat = this->glCaps().configSizedInternalFormat(desc.fConfig);
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
    if (samples > 1) {
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

sk_sp<GrGpuBuffer> GrGLGpu::onCreateBuffer(size_t size, GrGpuBufferType intendedType,
                                           GrAccessPattern accessPattern, const void* data) {
    return GrGLBuffer::Make(this, size, intendedType, accessPattern, data);
}

void GrGLGpu::flushScissor(const GrScissorState& scissorState,
                           const GrGLIRect& rtViewport,
                           GrSurfaceOrigin rtOrigin) {
    if (scissorState.enabled()) {
        GrGLIRect scissor;
        scissor.setRelativeTo(rtViewport, scissorState.rect(), rtOrigin);
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
                                    const GrGLRenderTarget* rt, GrSurfaceOrigin origin) {
#ifndef USE_NSIGHT
    typedef GrWindowRectsState::Mode Mode;
    SkASSERT(!windowState.enabled() || rt->renderFBOID()); // Window rects can't be used on-screen.
    SkASSERT(windowState.numWindows() <= this->caps()->maxWindowRectangles());

    if (!this->caps()->maxWindowRectangles() ||
        fHWWindowRectsState.knownEqualTo(origin, rt->getViewport(), windowState)) {
        return;
    }

    // This is purely a workaround for a spurious warning generated by gcc. Otherwise the above
    // assert would be sufficient. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=5912
    int numWindows = SkTMin(windowState.numWindows(), int(GrWindowRectangles::kMaxWindows));
    SkASSERT(windowState.numWindows() == numWindows);

    GrGLIRect glwindows[GrWindowRectangles::kMaxWindows];
    const SkIRect* skwindows = windowState.windows().data();
    for (int i = 0; i < numWindows; ++i) {
        glwindows[i].setRelativeTo(rt->getViewport(), skwindows[i], origin);
    }

    GrGLenum glmode = (Mode::kExclusive == windowState.mode()) ? GR_GL_EXCLUSIVE : GR_GL_INCLUSIVE;
    GL_CALL(WindowRectangles(glmode, numWindows, glwindows->asInts()));

    fHWWindowRectsState.set(origin, rt->getViewport(), windowState);
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
        if (sampler.filter() == GrSamplerState::Filter::kMipMap &&
            tex->texturePriv().mipMapped() == GrMipMapped::kYes &&
            tex->texturePriv().mipMapsAreDirty()) {
            SkASSERT(this->caps()->mipMapSupport());
            this->regenerateMipMapLevels(static_cast<GrGLTexture*>(tex));
            SkASSERT(!tex->asRenderTarget() || !tex->asRenderTarget()->needsResolve());
        } else if (auto* rt = tex->asRenderTarget()) {
            if (rt->needsResolve()) {
                this->resolveRenderTarget(rt);
            }
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

    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);

    this->flushColorWrite(blendInfo.fWriteColor);

    this->flushProgram(std::move(program));

    // Swizzle the blend to match what the shader will output.
    const GrSwizzle& swizzle = this->caps()->shaderCaps()->configOutputSwizzle(
        renderTarget->config());
    this->flushBlend(blendInfo, swizzle);

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
    this->flushStencil(stencil);
    if (pipeline.isScissorEnabled()) {
        static constexpr SkIRect kBogusScissor{0, 0, 1, 1};
        GrScissorState state(fixedDynamicState ? fixedDynamicState->fScissorRect : kBogusScissor);
        this->flushScissor(state, glRT->getViewport(), origin);
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
    this->flushScissor(clip.scissorState(), glRT->getViewport(), origin);
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

    this->flushScissor(clip.scissorState(), glRT->getViewport(), origin);
    this->flushWindowRectangles(clip.windowRectsState(), glRT, origin);

    GL_CALL(StencilMask((uint32_t) clipStencilMask));
    GL_CALL(ClearStencil(value));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
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
        this->flushRenderTargetNoColorWrites(static_cast<GrGLRenderTarget*>(target));
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
        GrSurfaceDesc desc;
        desc.fConfig = rtConfig;
        desc.fWidth = desc.fHeight = 16;
        if (this->glCaps().isConfigRenderable(rtConfig)) {
            desc.fFlags = kRenderTarget_GrSurfaceFlag;
            temp = this->createTexture(desc, SkBudgeted::kNo);
            if (!temp) {
                return false;
            }
            GrGLRenderTarget* glrt = static_cast<GrGLRenderTarget*>(temp->asRenderTarget());
            this->flushRenderTargetNoColorWrites(glrt);
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

bool GrGLGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                           GrColorType dstColorType, void* buffer, size_t rowBytes) {
    SkASSERT(surface);

    GrGLRenderTarget* renderTarget = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    if (!renderTarget && !this->glCaps().canConfigBeFBOColorAttachment(surface->config())) {
        return false;
    }

    // TODO: Avoid this conversion by making GrGLCaps work with color types.
    auto dstAsConfig = GrColorTypeToPixelConfig(dstColorType, GrSRGBEncoded::kNo);

    if (!this->readPixelsSupported(surface, dstAsConfig)) {
        return false;
    }

    GrGLenum externalFormat;
    GrGLenum externalType;
    if (!this->glCaps().getReadPixelsFormat(surface->config(), dstAsConfig, &externalFormat,
                                            &externalType)) {
        return false;
    }

    GrGLIRect glvp;
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
        glvp = renderTarget->getViewport();
    } else {
        // Use a temporary FBO.
        this->bindSurfaceFBOForPixelOps(surface, GR_GL_FRAMEBUFFER, &glvp, kSrc_TempFBOTarget);
        fHWBoundRenderTargetUniqueID.makeInvalid();
    }

    // the read rect is viewport-relative
    GrGLIRect readRect;
    readRect.setRelativeTo(glvp, left, top, width, height, kTopLeft_GrSurfaceOrigin);

    int bytesPerPixel = GrBytesPerPixel(dstAsConfig);
    size_t tightRowBytes = bytesPerPixel * width;

    size_t readDstRowBytes = tightRowBytes;
    void* readDst = buffer;

    // determine if GL can read using the passed rowBytes or if we need a scratch buffer.
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
    GL_CALL(PixelStorei(GR_GL_PACK_ALIGNMENT, config_alignment(dstAsConfig)));

    bool reattachStencil = false;
    if (this->glCaps().detachStencilFromMSAABuffersBeforeReadPixels() &&
        renderTarget &&
        renderTarget->renderTargetPriv().getStencilAttachment() &&
        renderTarget->numColorSamples() > 1) {
        // Fix Adreno devices that won't read from MSAA framebuffers with stencil attached
        reattachStencil = true;
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, 0));
    }

    GL_CALL(ReadPixels(readRect.fLeft, readRect.fBottom,
                       readRect.fWidth, readRect.fHeight,
                       externalFormat, externalType, readDst));

    if (reattachStencil) {
        GrGLStencilAttachment* stencilAttachment = static_cast<GrGLStencilAttachment*>(
                renderTarget->renderTargetPriv().getStencilAttachment());
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, stencilAttachment->renderbufferID()));
    }

    if (readDstRowBytes != tightRowBytes) {
        SkASSERT(this->glCaps().packRowLengthSupport());
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    }

    if (readDst != buffer) {
        SkASSERT(readDst != buffer);
        SkASSERT(rowBytes != tightRowBytes);
        const char* src = reinterpret_cast<const char*>(readDst);
        char* dst = reinterpret_cast<char*>(buffer);
        SkRectMemcpy(dst, rowBytes, src, readDstRowBytes, tightRowBytes, height);
    }
    if (!renderTarget) {
        this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, surface);
    }
    return true;
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
        this->flushViewport(target->getViewport());
    }

    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(GrPixelConfigIsSRGB(target->config()));
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
                               glRT->getViewport(), origin);
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
    return GR_GL_TRIANGLES;
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
            const GrGLIRect& vp = rt->getViewport();
            const SkIRect dirtyRect = rt->getResolveRect();
            // The dirty rect tracked on the RT is always stored in the native coordinates of the
            // surface. Choose kTopLeft so no adjustments are made
            static constexpr auto kDirtyRectOrigin = kTopLeft_GrSurfaceOrigin;
            if (GrGLCaps::kES_Apple_MSFBOType == this->glCaps().msFBOType()) {
                // Apple's extension uses the scissor as the blit bounds.
                GrScissorState scissorState;
                scissorState.set(dirtyRect);
                this->flushScissor(scissorState, vp, kDirtyRectOrigin);
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
                    rect.setRelativeTo(vp, dirtyRect, kDirtyRectOrigin);
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

void GrGLGpu::flushHWAAState(GrRenderTarget* rt, bool useHWAA) {
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
}

void GrGLGpu::flushBlend(const GrXferProcessor::BlendInfo& blendInfo, const GrSwizzle& swizzle) {
    // Any optimization to disable blending should have already been applied and
    // tweaked the equation to "add" or "subtract", and the coeffs to (1, 0).

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
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
        SkPMColor4f blendConst = swizzle.applyTo(blendInfo.fBlendConstant);
        if (!fHWBlendState.fConstColorValid || fHWBlendState.fConstColor != blendConst) {
            GL_CALL(BlendColor(blendConst.fR, blendConst.fG, blendConst.fB, blendConst.fA));
            fHWBlendState.fConstColor = blendConst;
            fHWBlendState.fConstColorValid = true;
        }
    }
}

static void get_gl_swizzle_values(const GrSwizzle& swizzle, GrGLenum glValues[4]) {
    for (int i = 0; i < 4; ++i) {
        switch (swizzle[i]) {
            case 'r': glValues[i] = GR_GL_RED;   break;
            case 'g': glValues[i] = GR_GL_GREEN; break;
            case 'b': glValues[i] = GR_GL_BLUE;  break;
            case 'a': glValues[i] = GR_GL_ALPHA; break;
            case '1': glValues[i] = GR_GL_ONE;   break;
            default:  SK_ABORT("Unsupported component");
        }
    }
}

void GrGLGpu::bindTexture(int unitIdx, GrSamplerState samplerState, GrGLTexture* texture) {
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

    ResetTimestamp timestamp = texture->getCachedParamsTimestamp();
    bool setAll = timestamp < this->getResetTimestamp();

    const GrGLTexture::SamplerParams* samplerParamsToRecord = nullptr;
    GrGLTexture::SamplerParams newSamplerParams;
    if (fSamplerObjectCache) {
        fSamplerObjectCache->bindSampler(unitIdx, samplerState);
    } else {
        const GrGLTexture::SamplerParams& oldSamplerParams = texture->getCachedSamplerParams();
        samplerParamsToRecord = &newSamplerParams;

        newSamplerParams.fMinFilter = filter_to_gl_min_filter(samplerState.filter());
        newSamplerParams.fMagFilter = filter_to_gl_mag_filter(samplerState.filter());

        newSamplerParams.fWrapS = wrap_mode_to_gl_wrap(samplerState.wrapModeX(), this->glCaps());
        newSamplerParams.fWrapT = wrap_mode_to_gl_wrap(samplerState.wrapModeY(), this->glCaps());

        // These are the OpenGL default values.
        newSamplerParams.fMinLOD = -1000.f;
        newSamplerParams.fMaxLOD = 1000.f;

        if (setAll || newSamplerParams.fMagFilter != oldSamplerParams.fMagFilter) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAG_FILTER, newSamplerParams.fMagFilter));
        }
        if (setAll || newSamplerParams.fMinFilter != oldSamplerParams.fMinFilter) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MIN_FILTER, newSamplerParams.fMinFilter));
        }
        if (this->glCaps().mipMapLevelAndLodControlSupport()) {
            if (setAll || newSamplerParams.fMinLOD != oldSamplerParams.fMinLOD) {
                this->setTextureUnit(unitIdx);
                GL_CALL(TexParameterf(target, GR_GL_TEXTURE_MIN_LOD, newSamplerParams.fMinLOD));
            }
            if (setAll || newSamplerParams.fMaxLOD != oldSamplerParams.fMaxLOD) {
                this->setTextureUnit(unitIdx);
                GL_CALL(TexParameterf(target, GR_GL_TEXTURE_MAX_LOD, newSamplerParams.fMaxLOD));
            }
        }
        if (setAll || newSamplerParams.fWrapS != oldSamplerParams.fWrapS) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_WRAP_S, newSamplerParams.fWrapS));
        }
        if (setAll || newSamplerParams.fWrapT != oldSamplerParams.fWrapT) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_WRAP_T, newSamplerParams.fWrapT));
        }
        if (this->glCaps().clampToBorderSupport()) {
            // Make sure the border color is transparent black (the default)
            if (setAll || oldSamplerParams.fBorderColorInvalid) {
                this->setTextureUnit(unitIdx);
                static const GrGLfloat kTransparentBlack[4] = {0.f, 0.f, 0.f, 0.f};
                GL_CALL(TexParameterfv(target, GR_GL_TEXTURE_BORDER_COLOR, kTransparentBlack));
            }
        }
    }
    GrGLTexture::NonSamplerParams newNonSamplerParams;
    newNonSamplerParams.fBaseMipMapLevel = 0;
    newNonSamplerParams.fMaxMipMapLevel = texture->texturePriv().maxMipMapLevel();

    const GrGLTexture::NonSamplerParams& oldNonSamplerParams = texture->getCachedNonSamplerParams();
    if (this->glCaps().textureSwizzleSupport()) {
        auto swizzle = this->glCaps().configSwizzle(texture->config());
        newNonSamplerParams.fSwizzleKey = swizzle.asKey();
        if (setAll || swizzle.asKey() != oldNonSamplerParams.fSwizzleKey) {
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
        if (newNonSamplerParams.fBaseMipMapLevel != oldNonSamplerParams.fBaseMipMapLevel) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_BASE_LEVEL,
                                  newNonSamplerParams.fBaseMipMapLevel));
        }
        if (newNonSamplerParams.fMaxMipMapLevel != oldNonSamplerParams.fMaxMipMapLevel) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAX_LEVEL,
                                  newNonSamplerParams.fMaxMipMapLevel));
        }
    }
    texture->setCachedParams(samplerParamsToRecord, newNonSamplerParams, this->getResetTimestamp());
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
static inline bool can_blit_framebuffer_for_copy_surface(
                                                const GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                                const GrSurface* src, GrSurfaceOrigin srcOrigin,
                                                const SkIRect& srcRect,
                                                const SkIPoint& dstPoint,
                                                const GrGLCaps& caps) {
    int dstSampleCnt = 0;
    int srcSampleCnt = 0;
    if (const GrRenderTarget* rt = dst->asRenderTarget()) {
        dstSampleCnt = rt->numColorSamples();
    }
    if (const GrRenderTarget* rt = src->asRenderTarget()) {
        srcSampleCnt = rt->numColorSamples();
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTarget()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTarget()));

    const GrGLTexture* dstTex = static_cast<const GrGLTexture*>(dst->asTexture());
    const GrGLTexture* srcTex = static_cast<const GrGLTexture*>(src->asTexture());

    bool dstIsGLTexture2D = dstTex ? GR_GL_TEXTURE_2D == dstTex->target() : false;
    bool srcIsGLTexture2D = srcTex ? GR_GL_TEXTURE_2D == srcTex->target() : false;

    return caps.canCopyAsBlit(dst->config(), dstSampleCnt, SkToBool(dstTex), dstIsGLTexture2D,
                              dstOrigin, src->config(), srcSampleCnt, SkToBool(srcTex),
                              srcIsGLTexture2D, srcOrigin, src->getBoundsRect(), srcRect, dstPoint);
}

static bool rt_has_msaa_render_buffer(const GrGLRenderTarget* rt, const GrGLCaps& glCaps) {
    // A RT has a separate MSAA renderbuffer if:
    // 1) It's multisampled
    // 2) We're using an extension with separate MSAA renderbuffers
    // 3) It's not FBO 0, which is special and always auto-resolves
    return rt->numColorSamples() > 1 && glCaps.usesMSAARenderBuffers() && rt->renderFBOID() != 0;
}

static inline bool can_copy_texsubimage(const GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                        const GrSurface* src, GrSurfaceOrigin srcOrigin,
                                        const GrGLCaps& caps) {

    const GrGLRenderTarget* dstRT = static_cast<const GrGLRenderTarget*>(dst->asRenderTarget());
    const GrGLRenderTarget* srcRT = static_cast<const GrGLRenderTarget*>(src->asRenderTarget());
    const GrGLTexture* dstTex = static_cast<const GrGLTexture*>(dst->asTexture());
    const GrGLTexture* srcTex = static_cast<const GrGLTexture*>(src->asTexture());

    bool dstHasMSAARenderBuffer = dstRT ? rt_has_msaa_render_buffer(dstRT, caps) : false;
    bool srcHasMSAARenderBuffer = srcRT ? rt_has_msaa_render_buffer(srcRT, caps) : false;

    bool dstIsGLTexture2D = dstTex ? GR_GL_TEXTURE_2D == dstTex->target() : false;
    bool srcIsGLTexture2D = srcTex ? GR_GL_TEXTURE_2D == srcTex->target() : false;

    return caps.canCopyTexSubImage(dst->config(), dstHasMSAARenderBuffer, SkToBool(dstTex),
                                   dstIsGLTexture2D, dstOrigin, src->config(),
                                   srcHasMSAARenderBuffer, SkToBool(srcTex), srcIsGLTexture2D,
                                   srcOrigin);
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

        this->bindFramebuffer(fboTarget, *tempFBOID);
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
        this->bindFramebuffer(fboTarget, rt->renderFBOID());
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

bool GrGLGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                            GrSurface* src, GrSurfaceOrigin srcOrigin,
                            const SkIRect& srcRect, const SkIPoint& dstPoint,
                            bool canDiscardOutsideDstRect) {
    // None of our copy methods can handle a swizzle. TODO: Make copySurfaceAsDraw handle the
    // swizzle.
    if (this->caps()->shaderCaps()->configOutputSwizzle(src->config()) !=
        this->caps()->shaderCaps()->configOutputSwizzle(dst->config())) {
        return false;
    }
    // Don't prefer copying as a draw if the dst doesn't already have a FBO object.
    // This implicitly handles this->glCaps().useDrawInsteadOfAllRenderTargetWrites().
    bool preferCopy = SkToBool(dst->asRenderTarget());
    if (preferCopy && this->glCaps().canCopyAsDraw(dst->config(), SkToBool(src->asTexture()))) {
        if (this->copySurfaceAsDraw(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint)) {
            return true;
        }
    }

    if (can_copy_texsubimage(dst, dstOrigin, src, srcOrigin, this->glCaps())) {
        this->copySurfaceAsCopyTexSubImage(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint);
        return true;
    }

    if (can_blit_framebuffer_for_copy_surface(dst, dstOrigin, src, srcOrigin,
                                              srcRect, dstPoint, this->glCaps())) {
        return this->copySurfaceAsBlitFramebuffer(dst, dstOrigin, src, srcOrigin,
                                                  srcRect, dstPoint);
    }

    if (!preferCopy && this->glCaps().canCopyAsDraw(dst->config(), SkToBool(src->asTexture()))) {
        if (this->copySurfaceAsDraw(dst, dstOrigin, src, srcOrigin, srcRect, dstPoint)) {
            return true;
        }
    }

    return false;
}

bool GrGLGpu::createCopyProgram(GrTexture* srcTex) {
    TRACE_EVENT0("skia", TRACE_FUNC);

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

    const char* version = shaderCaps->versionDeclString();
    GrShaderVar aVertex("a_vertex", kHalf2_GrSLType, GrShaderVar::kIn_TypeModifier);
    GrShaderVar uTexCoordXform("u_texCoordXform", kHalf4_GrSLType,
                               GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uPosXform("u_posXform", kHalf4_GrSLType, GrShaderVar::kUniform_TypeModifier);
    GrShaderVar uTexture("u_texture", samplerType, GrShaderVar::kUniform_TypeModifier);
    GrShaderVar vTexCoord("v_texCoord", kHalf2_GrSLType, GrShaderVar::kOut_TypeModifier);
    GrShaderVar oFragColor("o_FragColor", kHalf4_GrSLType, GrShaderVar::kOut_TypeModifier);

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
        "  v_texCoord = half2(a_vertex.xy * u_texCoordXform.xy + u_texCoordXform.zw);"
        "  sk_Position.xy = a_vertex * u_posXform.xy + u_posXform.zw;"
        "  sk_Position.zw = half2(0, 1);"
        "}"
    );

    SkString fshaderTxt(version);
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
        "  sk_FragColor = texture(u_texture, v_texCoord);"
        "}"
    );

    const char* str;
    GrGLint length;

    str = vshaderTxt.c_str();
    length = SkToInt(vshaderTxt.size());
    SkSL::Program::Settings settings;
    settings.fCaps = shaderCaps;
    SkSL::String glsl;
    std::unique_ptr<SkSL::Program> program = GrSkSLtoGLSL(*fGLContext, GR_GL_VERTEX_SHADER,
                                                          &str, &length, 1, settings, &glsl);
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, glsl.c_str(), glsl.size(),
                                                  &fStats, settings);
    SkASSERT(program->fInputs.isEmpty());

    str = fshaderTxt.c_str();
    length = SkToInt(fshaderTxt.size());
    program = GrSkSLtoGLSL(*fGLContext, GR_GL_FRAGMENT_SHADER, &str, &length, 1, settings, &glsl);
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, glsl.c_str(), glsl.size(),
                                                  &fStats, settings);
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

    const char* version = shaderCaps->versionDeclString();
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

    SkString fshaderTxt(version);
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
    SkSL::String glsl;
    std::unique_ptr<SkSL::Program> program = GrSkSLtoGLSL(*fGLContext, GR_GL_VERTEX_SHADER,
                                                          &str, &length, 1, settings, &glsl);
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, glsl.c_str(), glsl.size(),
                                                  &fStats, settings);
    SkASSERT(program->fInputs.isEmpty());

    str = fshaderTxt.c_str();
    length = SkToInt(fshaderTxt.size());
    program = GrSkSLtoGLSL(*fGLContext, GR_GL_FRAGMENT_SHADER, &str, &length, 1, settings, &glsl);
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, glsl.c_str(), glsl.size(),
                                                  &fStats, settings);
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

bool GrGLGpu::copySurfaceAsDraw(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                GrSurface* src, GrSurfaceOrigin srcOrigin,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint) {
    GrGLTexture* srcTex = static_cast<GrGLTexture*>(src->asTexture());
    int progIdx = TextureToCopyProgramIdx(srcTex);

    if (!this->glCaps().canConfigBeFBOColorAttachment(dst->config())) {
        return false;
    }

    if (!fCopyPrograms[progIdx].fProgram) {
        if (!this->createCopyProgram(srcTex)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }

    int w = srcRect.width();
    int h = srcRect.height();

    this->bindTexture(0, GrSamplerState::ClampNearest(), srcTex);

    GrGLIRect dstVP;
    this->bindSurfaceFBOForPixelOps(dst, GR_GL_FRAMEBUFFER, &dstVP, kDst_TempFBOTarget);
    this->flushViewport(dstVP);
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
    if (kBottomLeft_GrSurfaceOrigin == dstOrigin) {
        dy0 = -dy0;
        dy1 = -dy1;
    }

    GrGLfloat sx0 = (GrGLfloat)srcRect.fLeft;
    GrGLfloat sx1 = (GrGLfloat)(srcRect.fLeft + w);
    GrGLfloat sy0 = (GrGLfloat)srcRect.fTop;
    GrGLfloat sy1 = (GrGLfloat)(srcRect.fTop + h);
    int sw = src->width();
    int sh = src->height();
    if (kBottomLeft_GrSurfaceOrigin == srcOrigin) {
        sy0 = sh - sy0;
        sy1 = sh - sy1;
    }
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

    GrXferProcessor::BlendInfo blendInfo;
    blendInfo.reset();
    this->flushBlend(blendInfo, GrSwizzle::RGBA());
    this->flushColorWrite(true);
    this->flushHWAAState(nullptr, false);
    this->disableScissor();
    this->disableWindowRectangles();
    this->disableStencil();
    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(true);
    }

    GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    this->unbindTextureFBOForPixelOps(GR_GL_FRAMEBUFFER, dst);
    this->didWriteToSurface(dst, dstOrigin, &dstRect);

    return true;
}

void GrGLGpu::copySurfaceAsCopyTexSubImage(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                           GrSurface* src, GrSurfaceOrigin srcOrigin,
                                           const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_copy_texsubimage(dst, dstOrigin, src, srcOrigin, this->glCaps()));
    GrGLIRect srcVP;
    this->bindSurfaceFBOForPixelOps(src, GR_GL_FRAMEBUFFER, &srcVP, kSrc_TempFBOTarget);
    GrGLTexture* dstTex = static_cast<GrGLTexture *>(dst->asTexture());
    SkASSERT(dstTex);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();
    GrGLIRect srcGLRect;
    srcGLRect.setRelativeTo(srcVP, srcRect, srcOrigin);

    this->bindTextureToScratchUnit(dstTex->target(), dstTex->textureID());
    GrGLint dstY;
    if (kBottomLeft_GrSurfaceOrigin == dstOrigin) {
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
    this->didWriteToSurface(dst, dstOrigin, &dstRect);
}

bool GrGLGpu::copySurfaceAsBlitFramebuffer(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                                           GrSurface* src, GrSurfaceOrigin srcOrigin,
                                           const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_blit_framebuffer_for_copy_surface(dst, dstOrigin, src, srcOrigin,
                                                   srcRect, dstPoint, this->glCaps()));
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
    srcGLRect.setRelativeTo(srcVP, srcRect, srcOrigin);
    dstGLRect.setRelativeTo(dstVP, dstRect, dstOrigin);

    // BlitFrameBuffer respects the scissor, so disable it.
    this->disableScissor();
    this->disableWindowRectangles();

    GrGLint srcY0;
    GrGLint srcY1;
    // Does the blit need to y-mirror or not?
    if (srcOrigin == dstOrigin) {
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
    this->didWriteToSurface(dst, dstOrigin, &dstRect);
    return true;
}

bool GrGLGpu::onRegenerateMipMapLevels(GrTexture* texture) {
    auto glTex = static_cast<GrGLTexture*>(texture);
    // Mipmaps are only supported on 2D textures:
    if (GR_GL_TEXTURE_2D != glTex->target()) {
        return false;
    }

    // Manual implementation of mipmap generation, to work around driver bugs w/sRGB.
    // Uses draw calls to do a series of downsample operations to successive mips.

    // The manual approach requires the ability to limit which level we're sampling and that the
    // destination can be bound to a FBO:
    if (!this->glCaps().doManualMipmapping() ||
        !this->glCaps().canConfigBeFBOColorAttachment(texture->config())) {
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
    this->bindTexture(0, GrSamplerState::ClampBilerp(), glTex);

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
    GrXferProcessor::BlendInfo blendInfo;
    blendInfo.reset();
    this->flushBlend(blendInfo, GrSwizzle::RGBA());
    this->flushColorWrite(true);
    this->flushHWAAState(nullptr, false);
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
        viewport.fWidth = width;
        viewport.fHeight = height;
        this->flushViewport(viewport);

        GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    }

    // Unbind:
    GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                 GR_GL_TEXTURE_2D, 0, 0));

    // We modified the base level param.
    GrGLTexture::NonSamplerParams params = glTex->getCachedNonSamplerParams();
    params.fBaseMipMapLevel = levelCount - 2; // we drew the 2nd to last level into the last level.
    glTex->setCachedParams(nullptr, params, this->getResetTimestamp());

    return true;
}

void GrGLGpu::querySampleLocations(
        GrRenderTarget* renderTarget, const GrStencilSettings& stencilSettings,
        SkTArray<SkPoint>* sampleLocations) {
    this->flushStencil(stencilSettings);
    this->flushHWAAState(renderTarget, true);
    this->flushRenderTarget(static_cast<GrGLRenderTarget*>(renderTarget));

    int effectiveSampleCnt;
    GR_GL_GetIntegerv(this->glInterface(), GR_GL_SAMPLES, &effectiveSampleCnt);
    SkASSERT(effectiveSampleCnt >= renderTarget->numStencilSamples());

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

#if GR_TEST_UTILS
GrBackendTexture GrGLGpu::createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                          GrColorType colorType, bool /*isRT*/,
                                                          GrMipMapped mipMapped,
                                                          size_t rowBytes) {
    this->handleDirtyContext();

    GrPixelConfig config = GrColorTypeToPixelConfig(colorType, GrSRGBEncoded::kNo);
    if (!this->caps()->isConfigTexturable(config)) {
        return GrBackendTexture();  // invalid
    }

    if (w > this->caps()->maxTextureSize() || h > this->caps()->maxTextureSize()) {
        return GrBackendTexture();  // invalid
    }

    // Currently we don't support uploading pixel data when mipped.
    if (pixels && GrMipMapped::kYes == mipMapped) {
        return GrBackendTexture();  // invalid
    }

    int bpp = GrColorTypeBytesPerPixel(colorType);
    const size_t trimRowBytes = w * bpp;
    if (!rowBytes) {
        rowBytes = trimRowBytes;
    }

    GrGLTextureInfo info;
    info.fTarget = GR_GL_TEXTURE_2D;
    info.fID = 0;
    GL_CALL(GenTextures(1, &info.fID));
    this->bindTextureToScratchUnit(info.fTarget, info.fID);
    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
    GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
    GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_WRAP_S, GR_GL_CLAMP_TO_EDGE));
    GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_WRAP_T, GR_GL_CLAMP_TO_EDGE));

    // we have to do something special for compressed textures
    if (GrPixelConfigIsCompressed(config)) {
        GrGLenum internalFormat;
        const GrGLInterface* interface = this->glInterface();
        const GrGLCaps& caps = this->glCaps();
        if (!caps.getCompressedTexImageFormats(config, &internalFormat)) {
            return GrBackendTexture();
        }

        GrMipLevel mipLevel = { pixels, rowBytes };
        if (!allocate_and_populate_compressed_texture(config, *interface, caps, info.fTarget,
                                                      internalFormat, &mipLevel, 1,
                                                      w, h)) {
            return GrBackendTexture();
        }
    } else {
        bool restoreGLRowLength = false;
        if (trimRowBytes != rowBytes && this->glCaps().unpackRowLengthSupport()) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowBytes / bpp));
            restoreGLRowLength = true;
        }

        GrGLenum internalFormat;
        GrGLenum externalFormat;
        GrGLenum externalType;

        if (!this->glCaps().getTexImageFormats(config, config, &internalFormat, &externalFormat,
                                               &externalType)) {
            return GrBackendTexture();  // invalid
        }

        info.fFormat = this->glCaps().configSizedInternalFormat(config);

        this->unbindCpuToGpuXferBuffer();

        // Figure out the number of mip levels.
        int mipLevels = 1;
        if (GrMipMapped::kYes == mipMapped) {
            mipLevels = SkMipMap::ComputeLevelCount(w, h) + 1;
        }

        size_t baseLayerSize = bpp * w * h;
        SkAutoMalloc defaultStorage(baseLayerSize);
        if (!pixels) {
            // Fill in the texture with all zeros so we don't have random garbage
            pixels = defaultStorage.get();
            memset(defaultStorage.get(), 0, baseLayerSize);
        } else if (trimRowBytes != rowBytes && !restoreGLRowLength) {
            // We weren't able to use GR_GL_UNPACK_ROW_LENGTH so make a copy
            char* copy = (char*)defaultStorage.get();
            for (int y = 0; y < h; ++y) {
                memcpy(&copy[y*trimRowBytes], &((const char*)pixels)[y*rowBytes], trimRowBytes);
            }
            pixels = copy;
        }

        int width = w;
        int height = h;
        for (int i = 0; i < mipLevels; ++i) {
            GL_CALL(TexImage2D(info.fTarget, i, internalFormat, width, height, 0, externalFormat,
                               externalType, pixels));
            width = SkTMax(1, width / 2);
            height = SkTMax(1, height / 2);
        }
        if (restoreGLRowLength) {
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
        }
    }

    // unbind the texture from the texture unit to avoid asserts
    GL_CALL(BindTexture(info.fTarget, 0));

    GrBackendTexture beTex = GrBackendTexture(w, h, mipMapped, info);
    // Lots of tests don't go through Skia's public interface which will set the config so for
    // testing we make sure we set a config here.
    beTex.setPixelConfig(config);
    return beTex;
}

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

void GrGLGpu::deleteTestingOnlyBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kOpenGL == tex.backend());

    GrGLTextureInfo info;
    if (tex.getGLTextureInfo(&info)) {
        GL_CALL(DeleteTextures(1, &info.fID));
    }
}

GrBackendRenderTarget GrGLGpu::createTestingOnlyBackendRenderTarget(int w, int h,
                                                                    GrColorType colorType) {
    if (w > this->caps()->maxRenderTargetSize() || h > this->caps()->maxRenderTargetSize()) {
        return GrBackendRenderTarget();  // invalid
    }
    this->handleDirtyContext();
    auto config = GrColorTypeToPixelConfig(colorType, GrSRGBEncoded::kNo);
    if (!this->glCaps().isConfigRenderable(config)) {
        return {};
    }
    bool useTexture = false;
    GrGLenum colorBufferFormat;
    GrGLenum externalFormat = 0, externalType = 0;
    if (config == kBGRA_8888_GrPixelConfig && this->glCaps().bgraIsInternalFormat()) {
        // BGRA render buffers are not supported.
        this->glCaps().getTexImageFormats(config, config, &colorBufferFormat, &externalFormat,
                                          &externalType);
        useTexture = true;
    } else {
        this->glCaps().getRenderbufferFormat(config, &colorBufferFormat);
    }
    int sFormatIdx = this->getCompatibleStencilIndex(config);
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
    this->glCaps().getSizedInternalFormat(config, &info.fFormat);
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
    // Lots of tests don't go through Skia's public interface which will set the config so for
    // testing we make sure we set a config here.
    beRT.setPixelConfig(config);
#ifdef SK_DEBUG
    SkColorType skColorType = GrColorTypeToSkColorType(colorType);
    if (skColorType != kUnknown_SkColorType) {
        SkASSERT(this->caps()->validateBackendRenderTarget(
                         beRT, GrColorTypeToSkColorType(colorType)) != kUnknown_GrPixelConfig);
    }
#endif
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

void GrGLGpu::onFinishFlush(GrSurfaceProxy*, SkSurface::BackendSurfaceAccess access,
                            SkSurface::FlushFlags flags, bool insertedSemaphore) {
    // If we inserted semaphores during the flush, we need to call GLFlush.
    if (insertedSemaphore) {
        GL_CALL(Flush());
    }
    if (flags & SkSurface::kSyncCpu_FlushFlag) {
        GL_CALL(Finish());
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
                                                 GrResourceProvider::SemaphoreWrapType wrapType,
                                                 GrWrapOwnership ownership) {
    SkASSERT(this->caps()->fenceSyncSupport());
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
            return 0;
    }
}

#ifdef SK_ENABLE_DUMP_GPU
#include "SkJSONWriter.h"
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
