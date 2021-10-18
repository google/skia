/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLGpu.h"

#include "include/core/SkPixmap.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkHalf.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrCpuBuffer.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/gl/GrGLAttachment.h"
#include "src/gpu/gl/GrGLBuffer.h"
#include "src/gpu/gl/GrGLOpsRenderPass.h"
#include "src/gpu/gl/GrGLSemaphore.h"
#include "src/gpu/gl/GrGLTextureRenderTarget.h"
#include "src/gpu/gl/builders/GrGLShaderStringBuilder.h"
#include "src/sksl/SkSLCompiler.h"

#include <cmath>
#include <memory>

#define GL_CALL(X) GR_GL_CALL(this->glInterface(), X)
#define GL_CALL_RET(RET, X) GR_GL_CALL_RET(this->glInterface(), RET, X)

#define GL_ALLOC_CALL(call)                                   \
    [&] {                                                     \
        if (this->glCaps().skipErrorChecks()) {               \
            GR_GL_CALL(this->glInterface(), call);            \
            return static_cast<GrGLenum>(GR_GL_NO_ERROR);     \
        } else {                                              \
            this->clearErrorsAndCheckForOOM();                \
            GR_GL_CALL_NOERRCHECK(this->glInterface(), call); \
            return this->getErrorAndCheckForOOM();            \
        }                                                     \
    }()

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
static_assert(0 == kAdd_GrBlendEquation);
static_assert(1 == kSubtract_GrBlendEquation);
static_assert(2 == kReverseSubtract_GrBlendEquation);
static_assert(3 == kScreen_GrBlendEquation);
static_assert(4 == kOverlay_GrBlendEquation);
static_assert(5 == kDarken_GrBlendEquation);
static_assert(6 == kLighten_GrBlendEquation);
static_assert(7 == kColorDodge_GrBlendEquation);
static_assert(8 == kColorBurn_GrBlendEquation);
static_assert(9 == kHardLight_GrBlendEquation);
static_assert(10 == kSoftLight_GrBlendEquation);
static_assert(11 == kDifference_GrBlendEquation);
static_assert(12 == kExclusion_GrBlendEquation);
static_assert(13 == kMultiply_GrBlendEquation);
static_assert(14 == kHSLHue_GrBlendEquation);
static_assert(15 == kHSLSaturation_GrBlendEquation);
static_assert(16 == kHSLColor_GrBlendEquation);
static_assert(17 == kHSLLuminosity_GrBlendEquation);
static_assert(SK_ARRAY_COUNT(gXfermodeEquation2Blend) == kGrBlendEquationCnt);

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

    // extended blend coeffs
    GR_GL_SRC1_COLOR,
    GR_GL_ONE_MINUS_SRC1_COLOR,
    GR_GL_SRC1_ALPHA,
    GR_GL_ONE_MINUS_SRC1_ALPHA,

    // Illegal... needs to map to something.
    GR_GL_ZERO,
};

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
        case GrSamplerState::Filter::kLinear:  return GR_GL_LINEAR;
    }
    SkUNREACHABLE;
}

static GrGLenum filter_to_gl_min_filter(GrSamplerState::Filter filter,
                                        GrSamplerState::MipmapMode mm) {
    switch (mm) {
        case GrSamplerState::MipmapMode::kNone:
            return filter_to_gl_mag_filter(filter);
        case GrSamplerState::MipmapMode::kNearest:
            switch (filter) {
                case GrSamplerState::Filter::kNearest: return GR_GL_NEAREST_MIPMAP_NEAREST;
                case GrSamplerState::Filter::kLinear:  return GR_GL_LINEAR_MIPMAP_NEAREST;
            }
            SkUNREACHABLE;
        case GrSamplerState::MipmapMode::kLinear:
            switch (filter) {
                case GrSamplerState::Filter::kNearest: return GR_GL_NEAREST_MIPMAP_LINEAR;
                case GrSamplerState::Filter::kLinear:  return GR_GL_LINEAR_MIPMAP_LINEAR;
            }
            SkUNREACHABLE;
    }
    SkUNREACHABLE;
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
    SkUNREACHABLE;
}

///////////////////////////////////////////////////////////////////////////////

class GrGLGpu::SamplerObjectCache {
public:
    SamplerObjectCache(GrGLGpu* gpu) : fGpu(gpu) {
        fNumTextureUnits = fGpu->glCaps().shaderCaps()->maxFragmentSamplers();
        fTextureUnitStates = std::make_unique<UnitState[]>(fNumTextureUnits);
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

    void bindSampler(int unitIdx, GrSamplerState state) {
        int index = state.asIndex();
        if (!fSamplers[index]) {
            GrGLuint s;
            GR_GL_CALL(fGpu->glInterface(), GenSamplers(1, &s));
            if (!s) {
                return;
            }
            fSamplers[index] = s;
            GrGLenum minFilter = filter_to_gl_min_filter(state.filter(), state.mipmapMode());
            GrGLenum magFilter = filter_to_gl_mag_filter(state.filter());
            GrGLenum wrapX = wrap_mode_to_gl_wrap(state.wrapModeX(), fGpu->glCaps());
            GrGLenum wrapY = wrap_mode_to_gl_wrap(state.wrapModeY(), fGpu->glCaps());
            GR_GL_CALL(fGpu->glInterface(),
                       SamplerParameteri(s, GR_GL_TEXTURE_MIN_FILTER, minFilter));
            GR_GL_CALL(fGpu->glInterface(),
                       SamplerParameteri(s, GR_GL_TEXTURE_MAG_FILTER, magFilter));
            GR_GL_CALL(fGpu->glInterface(), SamplerParameteri(s, GR_GL_TEXTURE_WRAP_S, wrapX));
            GR_GL_CALL(fGpu->glInterface(), SamplerParameteri(s, GR_GL_TEXTURE_WRAP_T, wrapY));
        }
        if (!fTextureUnitStates[unitIdx].fKnown ||
            fTextureUnitStates[unitIdx].fSamplerIDIfKnown != fSamplers[index]) {
            GR_GL_CALL(fGpu->glInterface(), BindSampler(unitIdx, fSamplers[index]));
            fTextureUnitStates[unitIdx].fSamplerIDIfKnown = fSamplers[index];
            fTextureUnitStates[unitIdx].fKnown = true;
        }
    }

    void unbindSampler(int unitIdx) {
        if (!fTextureUnitStates[unitIdx].fKnown ||
            fTextureUnitStates[unitIdx].fSamplerIDIfKnown != 0) {
            GR_GL_CALL(fGpu->glInterface(), BindSampler(unitIdx, 0));
            fTextureUnitStates[unitIdx].fSamplerIDIfKnown = 0;
            fTextureUnitStates[unitIdx].fKnown = true;
        }
    }

    void invalidateBindings() {
        std::fill_n(fTextureUnitStates.get(), fNumTextureUnits, UnitState{});
    }

    void abandon() {
        fTextureUnitStates.reset();
        fNumTextureUnits = 0;
    }

    void release() {
        if (!fNumTextureUnits) {
            // We've already been abandoned.
            return;
        }
        GR_GL_CALL(fGpu->glInterface(), DeleteSamplers(kNumSamplers, fSamplers));
        std::fill_n(fSamplers, kNumSamplers, 0);
        // Deleting a bound sampler implicitly binds sampler 0. We just invalidate all of our
        // knowledge.
        std::fill_n(fTextureUnitStates.get(), fNumTextureUnits, UnitState{});
    }

private:
    static constexpr int kNumSamplers = GrSamplerState::kNumUniqueSamplers;
    struct UnitState {
        bool fKnown = false;
        GrGLuint fSamplerIDIfKnown = 0;
    };
    GrGLGpu* fGpu;
    std::unique_ptr<UnitState[]> fTextureUnitStates;
    GrGLuint fSamplers[kNumSamplers];
    int fNumTextureUnits;
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<GrGpu> GrGLGpu::Make(sk_sp<const GrGLInterface> interface, const GrContextOptions& options,
                           GrDirectContext* direct) {
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
    return sk_sp<GrGpu>(new GrGLGpu(std::move(glContext), direct));
}

GrGLGpu::GrGLGpu(std::unique_ptr<GrGLContext> ctx, GrDirectContext* dContext)
        : GrGpu(dContext)
        , fGLContext(std::move(ctx))
        , fProgramCache(new ProgramCache(dContext->priv().options().fRuntimeProgramCacheSize))
        , fHWProgramID(0)
        , fTempSrcFBOID(0)
        , fTempDstFBOID(0)
        , fStencilClearFBOID(0)
        , fFinishCallbacks(this) {
    SkASSERT(fGLContext);
    // Clear errors so we don't get confused whether we caused an error.
    this->clearErrorsAndCheckForOOM();
    // Toss out any pre-existing OOM that was hanging around before we got started.
    this->checkAndResetOOMed();

    this->initCapsAndCompiler(sk_ref_sp(fGLContext->caps()));

    fHWTextureUnitBindings.reset(this->numTextureUnits());

    this->hwBufferState(GrGpuBufferType::kVertex)->fGLTarget = GR_GL_ARRAY_BUFFER;
    this->hwBufferState(GrGpuBufferType::kIndex)->fGLTarget = GR_GL_ELEMENT_ARRAY_BUFFER;
    this->hwBufferState(GrGpuBufferType::kDrawIndirect)->fGLTarget = GR_GL_DRAW_INDIRECT_BUFFER;
    if (GrGLCaps::TransferBufferType::kChromium == this->glCaps().transferBufferType()) {
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
    static_assert(kGrGpuBufferTypeCount == SK_ARRAY_COUNT(fHWBufferState));

    if (this->glCaps().useSamplerObjects()) {
        fSamplerObjectCache = std::make_unique<SamplerObjectCache>(this);
    }
}

GrGLGpu::~GrGLGpu() {
    // Ensure any GrGpuResource objects get deleted first, since they may require a working GrGLGpu
    // to release the resources held by the objects themselves.
    fCopyProgramArrayBuffer.reset();
    fMipmapProgramArrayBuffer.reset();
    if (fProgramCache) {
        fProgramCache->reset();
    }

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

    fSamplerObjectCache.reset();

    fFinishCallbacks.callAll(true);
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
    fProgramCache->reset();
    fProgramCache.reset();

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

    fFinishCallbacks.callAll(/* doDelete */ DisconnectType::kCleanup == type);
}

GrThreadSafePipelineBuilder* GrGLGpu::pipelineBuilder() {
    return fProgramCache.get();
}

sk_sp<GrThreadSafePipelineBuilder> GrGLGpu::refPipelineBuilder() {
    return fProgramCache;
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

            fHWWireframeEnabled = kUnknown_TriState;
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
        if (this->glCaps().clientCanDisableMultisample()) {
            // Restore GL_MULTISAMPLE to its initial state. It being enabled has no effect on draws
            // to non-MSAA targets.
            GL_CALL(Enable(GR_GL_MULTISAMPLE));
        }
        fHWConservativeRasterEnabled = kUnknown_TriState;
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
        this->hwBufferState(GrGpuBufferType::kDrawIndirect)->invalidate();
        fHWPatchVertexCount = 0;
    }

    if (resetBits & kRenderTarget_GrGLBackendState) {
        fHWBoundRenderTargetUniqueID.makeInvalid();
        fHWSRGBFramebuffer = kUnknown_TriState;
        fBoundDrawFramebuffer = 0;
    }

    // we assume these values
    if (resetBits & kPixelStore_GrGLBackendState) {
        if (this->caps()->writePixelsRowBytesSupport() ||
            this->caps()->transferPixelsToRowBytesSupport()) {
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

static bool check_backend_texture(const GrBackendTexture& backendTex,
                                  const GrGLCaps& caps,
                                  GrGLTexture::Desc* desc,
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

    return true;
}

sk_sp<GrTexture> GrGLGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                               GrWrapOwnership ownership,
                                               GrWrapCacheable cacheable,
                                               GrIOType ioType) {
    GrGLTexture::Desc desc;
    if (!check_backend_texture(backendTex, this->glCaps(), &desc)) {
        return nullptr;
    }

    if (kBorrow_GrWrapOwnership == ownership) {
        desc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    } else {
        desc.fOwnership = GrBackendObjectOwnership::kOwned;
    }

    GrMipmapStatus mipmapStatus = backendTex.hasMipmaps() ? GrMipmapStatus::kValid
                                                          : GrMipmapStatus::kNotAllocated;

    auto texture = GrGLTexture::MakeWrapped(this, mipmapStatus, desc,
                                            backendTex.getGLTextureParams(), cacheable, ioType);
    if (this->glCaps().isFormatRenderable(backendTex.getBackendFormat(), 1)) {
        // Pessimistically assume this external texture may have been bound to a FBO.
        texture->baseLevelWasBoundToFBO();
    }
    return std::move(texture);
}

static bool check_compressed_backend_texture(const GrBackendTexture& backendTex,
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

    if (GR_GL_TEXTURE_2D != desc->fTarget) {
        return false;
    }
    if (backendTex.isProtected()) {
        // Not supported in GL backend at this time.
        return false;
    }

    return true;
}

sk_sp<GrTexture> GrGLGpu::onWrapCompressedBackendTexture(const GrBackendTexture& backendTex,
                                                         GrWrapOwnership ownership,
                                                         GrWrapCacheable cacheable) {
    GrGLTexture::Desc desc;
    if (!check_compressed_backend_texture(backendTex, this->glCaps(), &desc)) {
        return nullptr;
    }

    if (kBorrow_GrWrapOwnership == ownership) {
        desc.fOwnership = GrBackendObjectOwnership::kBorrowed;
    } else {
        desc.fOwnership = GrBackendObjectOwnership::kOwned;
    }

    GrMipmapStatus mipmapStatus = backendTex.hasMipmaps() ? GrMipmapStatus::kValid
                                                          : GrMipmapStatus::kNotAllocated;

    auto texture = GrGLTexture::MakeWrapped(this, mipmapStatus, desc,
                                            backendTex.getGLTextureParams(), cacheable,
                                            kRead_GrIOType);
    return std::move(texture);
}

sk_sp<GrTexture> GrGLGpu::onWrapRenderableBackendTexture(const GrBackendTexture& backendTex,
                                                         int sampleCnt,
                                                         GrWrapOwnership ownership,
                                                         GrWrapCacheable cacheable) {
    const GrGLCaps& caps = this->glCaps();

    GrGLTexture::Desc desc;
    if (!check_backend_texture(backendTex, this->glCaps(), &desc)) {
        return nullptr;
    }
    SkASSERT(caps.isFormatRenderable(desc.fFormat, sampleCnt));
    SkASSERT(caps.isFormatTexturable(desc.fFormat));

    // We don't support rendering to a EXTERNAL texture.
    if (GR_GL_TEXTURE_EXTERNAL == desc.fTarget) {
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

    GrMipmapStatus mipmapStatus = backendTex.hasMipmaps() ? GrMipmapStatus::kDirty
                                                          : GrMipmapStatus::kNotAllocated;

    sk_sp<GrGLTextureRenderTarget> texRT(GrGLTextureRenderTarget::MakeWrapped(
            this, sampleCnt, desc, backendTex.getGLTextureParams(), rtIDs, cacheable,
            mipmapStatus));
    texRT->baseLevelWasBoundToFBO();
    return std::move(texRT);
}

sk_sp<GrRenderTarget> GrGLGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& backendRT) {
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

    int sampleCount = this->glCaps().getRenderTargetSampleCount(backendRT.sampleCnt(), format);

    GrGLRenderTarget::IDs rtIDs;
    if (sampleCount <= 1) {
        rtIDs.fSingleSampleFBOID = info.fFBOID;
        rtIDs.fMultisampleFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    } else {
        rtIDs.fSingleSampleFBOID = GrGLRenderTarget::kUnresolvableFBOID;
        rtIDs.fMultisampleFBOID = info.fFBOID;
    }
    rtIDs.fMSColorRenderbufferID = 0;
    rtIDs.fRTFBOOwnership = GrBackendObjectOwnership::kBorrowed;
    rtIDs.fTotalMemorySamplesPerPixel = sampleCount;

    return GrGLRenderTarget::MakeWrapped(this, backendRT.dimensions(), format, sampleCount, rtIDs,
                                         backendRT.stencilBits());
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

bool GrGLGpu::onWritePixels(GrSurface* surface,
                            SkIRect rect,
                            GrColorType surfaceColorType,
                            GrColorType srcColorType,
                            const GrMipLevel texels[],
                            int mipLevelCount,
                            bool prepForTexSampling) {
    auto glTex = static_cast<GrGLTexture*>(surface->asTexture());

    if (!check_write_and_transfer_input(glTex)) {
        return false;
    }

    this->bindTextureToScratchUnit(glTex->target(), glTex->textureID());

    // If we have mips make sure the base/max levels cover the full range so that the uploads go to
    // the right levels. We've found some Radeons require this.
    if (mipLevelCount && this->glCaps().mipmapLevelControlSupport()) {
        auto params = glTex->parameters();
        GrGLTextureParameters::NonsamplerState nonsamplerState = params->nonsamplerState();
        int maxLevel = glTex->maxMipmapLevel();
        if (params->nonsamplerState().fBaseMipMapLevel != 0) {
            GL_CALL(TexParameteri(glTex->target(), GR_GL_TEXTURE_BASE_LEVEL, 0));
            nonsamplerState.fBaseMipMapLevel = 0;
        }
        if (params->nonsamplerState().fMaxMipmapLevel != maxLevel) {
            GL_CALL(TexParameteri(glTex->target(), GR_GL_TEXTURE_MAX_LEVEL, maxLevel));
            nonsamplerState.fBaseMipMapLevel = maxLevel;
        }
        params->set(nullptr, nonsamplerState, fResetTimestampForTextureParameters);
    }

    SkASSERT(!GrGLFormatIsCompressed(glTex->format()));
    return this->uploadColorTypeTexData(glTex->format(),
                                        surfaceColorType,
                                        glTex->dimensions(),
                                        glTex->target(),
                                        rect,
                                        srcColorType,
                                        texels,
                                        mipLevelCount);
}

bool GrGLGpu::onTransferPixelsTo(GrTexture* texture,
                                 SkIRect rect,
                                 GrColorType textureColorType,
                                 GrColorType bufferColorType,
                                 sk_sp<GrGpuBuffer> transferBuffer,
                                 size_t offset,
                                 size_t rowBytes) {
    GrGLTexture* glTex = static_cast<GrGLTexture*>(texture);

    // Can't transfer compressed data
    SkASSERT(!GrGLFormatIsCompressed(glTex->format()));

    if (!check_write_and_transfer_input(glTex)) {
        return false;
    }

    static_assert(sizeof(int) == sizeof(int32_t), "");

    this->bindTextureToScratchUnit(glTex->target(), glTex->textureID());

    SkASSERT(!transferBuffer->isMapped());
    SkASSERT(!transferBuffer->isCpuBuffer());
    const GrGLBuffer* glBuffer = static_cast<const GrGLBuffer*>(transferBuffer.get());
    this->bindBuffer(GrGpuBufferType::kXferCpuToGpu, glBuffer);

    SkASSERT(SkIRect::MakeSize(texture->dimensions()).contains(rect));

    size_t bpp = GrColorTypeBytesPerPixel(bufferColorType);
    const size_t trimRowBytes = rect.width() * bpp;
    const void* pixels = (void*)offset;

    bool restoreGLRowLength = false;
    if (trimRowBytes != rowBytes) {
        // we should have checked for this support already
        SkASSERT(this->glCaps().transferPixelsToRowBytesSupport());
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowBytes / bpp));
        restoreGLRowLength = true;
    }

    GrGLFormat textureFormat = glTex->format();
    // External format and type come from the upload data.
    GrGLenum externalFormat = 0;
    GrGLenum externalType = 0;
    this->glCaps().getTexSubImageExternalFormatAndType(
            textureFormat, textureColorType, bufferColorType, &externalFormat, &externalType);
    if (!externalFormat || !externalType) {
        return false;
    }

    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(TexSubImage2D(glTex->target(),
                          0,
                          rect.left(),
                          rect.top(),
                          rect.width(),
                          rect.height(),
                          externalFormat,
                          externalType,
                          pixels));

    if (restoreGLRowLength) {
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }

    return true;
}

bool GrGLGpu::onTransferPixelsFrom(GrSurface* surface,
                                   SkIRect rect,
                                   GrColorType surfaceColorType,
                                   GrColorType dstColorType,
                                   sk_sp<GrGpuBuffer> transferBuffer,
                                   size_t offset) {
    auto* glBuffer = static_cast<GrGLBuffer*>(transferBuffer.get());
    this->bindBuffer(GrGpuBufferType::kXferGpuToCpu, glBuffer);
    auto offsetAsPtr = reinterpret_cast<void*>(offset);
    return this->readOrTransferPixelsFrom(surface,
                                          rect,
                                          surfaceColorType,
                                          dstColorType,
                                          offsetAsPtr,
                                          rect.width());
}

void GrGLGpu::unbindXferBuffer(GrGpuBufferType type) {
    if (this->glCaps().transferBufferType() != GrGLCaps::TransferBufferType::kARB_PBO &&
        this->glCaps().transferBufferType() != GrGLCaps::TransferBufferType::kNV_PBO) {
        return;
    }
    SkASSERT(type == GrGpuBufferType::kXferCpuToGpu || type == GrGpuBufferType::kXferGpuToCpu);
    auto* xferBufferState = this->hwBufferState(type);
    if (!xferBufferState->fBufferZeroKnownBound) {
        GL_CALL(BindBuffer(xferBufferState->fGLTarget, 0));
        xferBufferState->fBoundBufferUniqueID.makeInvalid();
        xferBufferState->fBufferZeroKnownBound = true;
    }
}

bool GrGLGpu::uploadColorTypeTexData(GrGLFormat textureFormat,
                                     GrColorType textureColorType,
                                     SkISize texDims,
                                     GrGLenum target,
                                     SkIRect dstRect,
                                     GrColorType srcColorType,
                                     const GrMipLevel texels[],
                                     int mipLevelCount) {
    // If we're uploading compressed data then we should be using uploadCompressedTexData
    SkASSERT(!GrGLFormatIsCompressed(textureFormat));

    SkASSERT(this->glCaps().isFormatTexturable(textureFormat));

    size_t bpp = GrColorTypeBytesPerPixel(srcColorType);

    // External format and type come from the upload data.
    GrGLenum externalFormat;
    GrGLenum externalType;
    this->glCaps().getTexSubImageExternalFormatAndType(
            textureFormat, textureColorType, srcColorType, &externalFormat, &externalType);
    if (!externalFormat || !externalType) {
        return false;
    }
    this->uploadTexData(texDims, target, dstRect, externalFormat, externalType, bpp, texels,
                        mipLevelCount);
    return true;
}

bool GrGLGpu::uploadColorToTex(GrGLFormat textureFormat,
                               SkISize texDims,
                               GrGLenum target,
                               std::array<float, 4> color,
                               uint32_t levelMask) {
    GrColorType colorType;
    GrGLenum externalFormat, externalType;
    this->glCaps().getTexSubImageDefaultFormatTypeAndColorType(textureFormat, &externalFormat,
                                                               &externalType, &colorType);
    if (colorType == GrColorType::kUnknown) {
        return false;
    }

    std::unique_ptr<char[]> pixelStorage;
    size_t bpp = 0;
    int numLevels = SkMipmap::ComputeLevelCount(texDims) + 1;
    SkSTArray<16, GrMipLevel> levels;
    levels.resize(numLevels);
    SkISize levelDims = texDims;
    for (int i = 0; i < numLevels; ++i, levelDims = {std::max(levelDims.width()  >> 1, 1),
                                                     std::max(levelDims.height() >> 1, 1)}) {
        if (levelMask & (1 << i)) {
            if (!pixelStorage) {
                // Make one tight image at the first size and reuse it for smaller levels.
                GrImageInfo ii(colorType, kUnpremul_SkAlphaType, nullptr, levelDims);
                size_t rb = ii.minRowBytes();
                pixelStorage.reset(new char[rb * levelDims.height()]);
                if (!GrClearImage(ii, pixelStorage.get(), ii.minRowBytes(), color)) {
                    return false;
                }
                bpp = ii.bpp();
            }
            levels[i] = {pixelStorage.get(), levelDims.width()*bpp, nullptr};
        }
    }
    this->uploadTexData(texDims, target, SkIRect::MakeSize(texDims), externalFormat, externalType,
                        bpp, levels.begin(), levels.count());
    return true;
}

void GrGLGpu::uploadTexData(SkISize texDims,
                            GrGLenum target,
                            SkIRect dstRect,
                            GrGLenum externalFormat,
                            GrGLenum externalType,
                            size_t bpp,
                            const GrMipLevel texels[],
                            int mipLevelCount) {
    SkASSERT(!texDims.isEmpty());
    SkASSERT(!dstRect.isEmpty());
    SkASSERT(SkIRect::MakeSize(texDims).contains(dstRect));
    SkASSERT(mipLevelCount > 0 && mipLevelCount <= SkMipmap::ComputeLevelCount(texDims) + 1);
    SkASSERT(mipLevelCount == 1 || dstRect == SkIRect::MakeSize(texDims));

    const GrGLCaps& caps = this->glCaps();

    bool restoreGLRowLength = false;

    this->unbindXferBuffer(GrGpuBufferType::kXferCpuToGpu);
    GL_CALL(PixelStorei(GR_GL_UNPACK_ALIGNMENT, 1));

    SkISize dims = dstRect.size();
    for (int level = 0; level < mipLevelCount; ++level, dims = {std::max(dims.width()  >> 1, 1),
                                                                std::max(dims.height() >> 1, 1)}) {
        if (!texels[level].fPixels) {
            continue;
        }
        const size_t trimRowBytes = dims.width() * bpp;
        const size_t rowBytes = texels[level].fRowBytes;

        if (caps.writePixelsRowBytesSupport() && (rowBytes != trimRowBytes || restoreGLRowLength)) {
            GrGLint rowLength = static_cast<GrGLint>(rowBytes / bpp);
            GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, rowLength));
            restoreGLRowLength = true;
        } else {
            SkASSERT(rowBytes == trimRowBytes);
        }

        GL_CALL(TexSubImage2D(target, level, dstRect.x(), dstRect.y(), dims.width(), dims.height(),
                              externalFormat, externalType, texels[level].fPixels));
    }
    if (restoreGLRowLength) {
        SkASSERT(caps.writePixelsRowBytesSupport());
        GL_CALL(PixelStorei(GR_GL_UNPACK_ROW_LENGTH, 0));
    }
}

bool GrGLGpu::uploadCompressedTexData(SkImage::CompressionType compressionType,
                                      GrGLFormat format,
                                      SkISize dimensions,
                                      GrMipmapped mipMapped,
                                      GrGLenum target,
                                      const void* data, size_t dataSize) {
    SkASSERT(format != GrGLFormat::kUnknown);
    const GrGLCaps& caps = this->glCaps();

    // We only need the internal format for compressed 2D textures.
    GrGLenum internalFormat = caps.getTexImageOrStorageInternalFormat(format);
    if (!internalFormat) {
        return false;
    }

    SkASSERT(compressionType != SkImage::CompressionType::kNone);

    bool useTexStorage = caps.formatSupportsTexStorage(format);

    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height())+1;
    }

    // TODO: Make sure that the width and height that we pass to OpenGL
    // is a multiple of the block size.

    if (useTexStorage) {
        // We never resize or change formats of textures.
        GrGLenum error = GL_ALLOC_CALL(TexStorage2D(target, numMipLevels, internalFormat,
                                                    dimensions.width(), dimensions.height()));
        if (error != GR_GL_NO_ERROR) {
            return false;
        }

        size_t offset = 0;
        for (int level = 0; level < numMipLevels; ++level) {

            size_t levelDataSize = SkCompressedDataSize(compressionType, dimensions,
                                                        nullptr, false);

            error = GL_ALLOC_CALL(CompressedTexSubImage2D(target,
                                                          level,
                                                          0,  // left
                                                          0,  // top
                                                          dimensions.width(),
                                                          dimensions.height(),
                                                          internalFormat,
                                                          SkToInt(levelDataSize),
                                                          &((char*)data)[offset]));

            if (error != GR_GL_NO_ERROR) {
                return false;
            }

            offset += levelDataSize;
            dimensions = {std::max(1, dimensions.width()/2), std::max(1, dimensions.height()/2)};
        }
    } else {
        size_t offset = 0;

        for (int level = 0; level < numMipLevels; ++level) {
            size_t levelDataSize = SkCompressedDataSize(compressionType, dimensions,
                                                        nullptr, false);

            const char* rawLevelData = &((char*)data)[offset];
            GrGLenum error = GL_ALLOC_CALL(CompressedTexImage2D(target,
                                                                level,
                                                                internalFormat,
                                                                dimensions.width(),
                                                                dimensions.height(),
                                                                0,  // border
                                                                SkToInt(levelDataSize),
                                                                rawLevelData));

            if (error != GR_GL_NO_ERROR) {
                return false;
            }

            offset += levelDataSize;
            dimensions = {std::max(1, dimensions.width()/2), std::max(1, dimensions.height()/2)};
        }
    }
    return true;
}

bool GrGLGpu::renderbufferStorageMSAA(const GrGLContext& ctx, int sampleCount, GrGLenum format,
                                      int width, int height) {
    SkASSERT(GrGLCaps::kNone_MSFBOType != ctx.caps()->msFBOType());
    GrGLenum error;
    switch (ctx.caps()->msFBOType()) {
        case GrGLCaps::kStandard_MSFBOType:
            error = GL_ALLOC_CALL(RenderbufferStorageMultisample(GR_GL_RENDERBUFFER, sampleCount,
                                                                 format, width, height));
            break;
        case GrGLCaps::kES_Apple_MSFBOType:
            error = GL_ALLOC_CALL(RenderbufferStorageMultisampleES2APPLE(
                    GR_GL_RENDERBUFFER, sampleCount, format, width, height));
            break;
        case GrGLCaps::kES_EXT_MsToTexture_MSFBOType:
        case GrGLCaps::kES_IMG_MsToTexture_MSFBOType:
            error = GL_ALLOC_CALL(RenderbufferStorageMultisampleES2EXT(
                    GR_GL_RENDERBUFFER, sampleCount, format, width, height));
            break;
        case GrGLCaps::kNone_MSFBOType:
            SkUNREACHABLE;
            break;
    }
    return error == GR_GL_NO_ERROR;
}

bool GrGLGpu::createRenderTargetObjects(const GrGLTexture::Desc& desc,
                                        int sampleCount,
                                        GrGLRenderTarget::IDs* rtIDs) {
    rtIDs->fMSColorRenderbufferID = 0;
    rtIDs->fMultisampleFBOID = 0;
    rtIDs->fRTFBOOwnership = GrBackendObjectOwnership::kOwned;
    rtIDs->fSingleSampleFBOID = 0;
    rtIDs->fTotalMemorySamplesPerPixel = 0;

    GrGLenum colorRenderbufferFormat = 0; // suppress warning

    if (desc.fFormat == GrGLFormat::kUnknown) {
        goto FAILED;
    }

    if (sampleCount > 1 && GrGLCaps::kNone_MSFBOType == this->glCaps().msFBOType()) {
        goto FAILED;
    }

    GL_CALL(GenFramebuffers(1, &rtIDs->fSingleSampleFBOID));
    if (!rtIDs->fSingleSampleFBOID) {
        goto FAILED;
    }

    // If we are using multisampling we will create two FBOS. We render to one and then resolve to
    // the texture bound to the other. The exception is the IMG multisample extension. With this
    // extension the texture is multisampled when rendered to and then auto-resolves it when it is
    // rendered from.
    if (sampleCount <= 1) {
        rtIDs->fMultisampleFBOID = GrGLRenderTarget::kUnresolvableFBOID;
    } else if (this->glCaps().usesImplicitMSAAResolve()) {
        // GrGLRenderTarget target will configure the FBO as multisample or not base on need.
        rtIDs->fMultisampleFBOID = rtIDs->fSingleSampleFBOID;
    } else {
        GL_CALL(GenFramebuffers(1, &rtIDs->fMultisampleFBOID));
        if (!rtIDs->fMultisampleFBOID) {
            goto FAILED;
        }
        GL_CALL(GenRenderbuffers(1, &rtIDs->fMSColorRenderbufferID));
        if (!rtIDs->fMSColorRenderbufferID) {
            goto FAILED;
        }
        colorRenderbufferFormat = this->glCaps().getRenderbufferInternalFormat(desc.fFormat);
    }

    // below here we may bind the FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();
    if (rtIDs->fMSColorRenderbufferID) {
        SkASSERT(sampleCount > 1);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, rtIDs->fMSColorRenderbufferID));
        if (!this->renderbufferStorageMSAA(*fGLContext, sampleCount, colorRenderbufferFormat,
                                           desc.fSize.width(), desc.fSize.height())) {
            goto FAILED;
        }
        this->bindFramebuffer(GR_GL_FRAMEBUFFER, rtIDs->fMultisampleFBOID);
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                        GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_RENDERBUFFER,
                                        rtIDs->fMSColorRenderbufferID));
        rtIDs->fTotalMemorySamplesPerPixel += sampleCount;
    }

    this->bindFramebuffer(GR_GL_FRAMEBUFFER, rtIDs->fSingleSampleFBOID);
    GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER,
                                 GR_GL_COLOR_ATTACHMENT0,
                                 desc.fTarget,
                                 desc.fID,
                                 0));
    ++rtIDs->fTotalMemorySamplesPerPixel;

    return true;

FAILED:
    if (rtIDs->fMSColorRenderbufferID) {
        GL_CALL(DeleteRenderbuffers(1, &rtIDs->fMSColorRenderbufferID));
    }
    if (rtIDs->fMultisampleFBOID != rtIDs->fSingleSampleFBOID) {
        this->deleteFramebuffer(rtIDs->fMultisampleFBOID);
    }
    if (rtIDs->fSingleSampleFBOID) {
        this->deleteFramebuffer(rtIDs->fSingleSampleFBOID);
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

sk_sp<GrTexture> GrGLGpu::onCreateTexture(SkISize dimensions,
                                          const GrBackendFormat& format,
                                          GrRenderable renderable,
                                          int renderTargetSampleCnt,
                                          SkBudgeted budgeted,
                                          GrProtected isProtected,
                                          int mipLevelCount,
                                          uint32_t levelClearMask) {
    // We don't support protected textures in GL.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }
    SkASSERT(GrGLCaps::kNone_MSFBOType != this->glCaps().msFBOType() || renderTargetSampleCnt == 1);

    SkASSERT(mipLevelCount > 0);
    GrMipmapStatus mipmapStatus =
            mipLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;
    GrGLTextureParameters::SamplerOverriddenState initialState;
    GrGLTexture::Desc texDesc;
    texDesc.fSize = dimensions;
    switch (format.textureType()) {
        case GrTextureType::kExternal:
        case GrTextureType::kNone:
            return nullptr;
        case GrTextureType::k2D:
            texDesc.fTarget = GR_GL_TEXTURE_2D;
            break;
        case GrTextureType::kRectangle:
            if (mipLevelCount > 1 || !this->glCaps().rectangleTextureSupport()) {
                return nullptr;
            }
            texDesc.fTarget = GR_GL_TEXTURE_RECTANGLE;
            break;
    }
    texDesc.fFormat = format.asGLFormat();
    texDesc.fOwnership = GrBackendObjectOwnership::kOwned;
    SkASSERT(texDesc.fFormat != GrGLFormat::kUnknown);
    SkASSERT(!GrGLFormatIsCompressed(texDesc.fFormat));

    texDesc.fID = this->createTexture(dimensions, texDesc.fFormat, texDesc.fTarget, renderable,
                                      &initialState, mipLevelCount);

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
                this, budgeted, renderTargetSampleCnt, texDesc, rtIDDesc, mipmapStatus);
        tex->baseLevelWasBoundToFBO();
    } else {
        tex = sk_make_sp<GrGLTexture>(this, budgeted, texDesc, mipmapStatus);
    }
    // The non-sampler params are still at their default values.
    tex->parameters()->set(&initialState, GrGLTextureParameters::NonsamplerState(),
                           fResetTimestampForTextureParameters);
    if (levelClearMask) {
        if (this->glCaps().clearTextureSupport()) {
            GrGLenum externalFormat, externalType;
            GrColorType colorType;
            this->glCaps().getTexSubImageDefaultFormatTypeAndColorType(
                    texDesc.fFormat, &externalFormat, &externalType, &colorType);
            for (int i = 0; i < mipLevelCount; ++i) {
                if (levelClearMask & (1U << i)) {
                    GL_CALL(ClearTexImage(tex->textureID(), i, externalFormat, externalType,
                                          nullptr));
                }
            }
        } else if (this->glCaps().canFormatBeFBOColorAttachment(format.asGLFormat()) &&
                   !this->glCaps().performColorClearsAsDraws()) {
            this->flushScissorTest(GrScissorTest::kDisabled);
            this->disableWindowRectangles();
            this->flushColorWrite(true);
            this->flushClearColor({0, 0, 0, 0});
            for (int i = 0; i < mipLevelCount; ++i) {
                if (levelClearMask & (1U << i)) {
                    this->bindSurfaceFBOForPixelOps(tex.get(), i, GR_GL_FRAMEBUFFER,
                                                    kDst_TempFBOTarget);
                    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
                    this->unbindSurfaceFBOForPixelOps(tex.get(), i, GR_GL_FRAMEBUFFER);
                }
            }
            fHWBoundRenderTargetUniqueID.makeInvalid();
        } else {
            this->bindTextureToScratchUnit(texDesc.fTarget, tex->textureID());
            std::array<float, 4> zeros = {};
            this->uploadColorToTex(texDesc.fFormat,
                                   texDesc.fSize,
                                   texDesc.fTarget,
                                   zeros,
                                   levelClearMask);
        }
    }
    return std::move(tex);
}

sk_sp<GrTexture> GrGLGpu::onCreateCompressedTexture(SkISize dimensions,
                                                    const GrBackendFormat& format,
                                                    SkBudgeted budgeted,
                                                    GrMipmapped mipMapped,
                                                    GrProtected isProtected,
                                                    const void* data, size_t dataSize) {
    // We don't support protected textures in GL.
    if (isProtected == GrProtected::kYes) {
        return nullptr;
    }
    SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);

    GrGLTextureParameters::SamplerOverriddenState initialState;
    GrGLTexture::Desc desc;
    desc.fSize = dimensions;
    desc.fTarget = GR_GL_TEXTURE_2D;
    desc.fOwnership = GrBackendObjectOwnership::kOwned;
    desc.fFormat = format.asGLFormat();
    desc.fID = this->createCompressedTexture2D(desc.fSize, compression, desc.fFormat,
                                               mipMapped, &initialState);
    if (!desc.fID) {
        return nullptr;
    }

    if (data) {
        if (!this->uploadCompressedTexData(compression, desc.fFormat, dimensions, mipMapped,
                                           GR_GL_TEXTURE_2D, data, dataSize)) {
            GL_CALL(DeleteTextures(1, &desc.fID));
            return nullptr;
        }
    }

    // Unbind this texture from the scratch texture unit.
    this->bindTextureToScratchUnit(GR_GL_TEXTURE_2D, 0);

    GrMipmapStatus mipmapStatus = mipMapped == GrMipmapped::kYes
                                                            ? GrMipmapStatus::kValid
                                                            : GrMipmapStatus::kNotAllocated;

    auto tex = sk_make_sp<GrGLTexture>(this, budgeted, desc, mipmapStatus);
    // The non-sampler params are still at their default values.
    tex->parameters()->set(&initialState, GrGLTextureParameters::NonsamplerState(),
                           fResetTimestampForTextureParameters);
    return std::move(tex);
}

GrBackendTexture GrGLGpu::onCreateCompressedBackendTexture(
        SkISize dimensions, const GrBackendFormat& format, GrMipmapped mipMapped,
        GrProtected isProtected) {
    // We don't support protected textures in GL.
    if (isProtected == GrProtected::kYes) {
        return {};
    }

    this->handleDirtyContext();

    GrGLFormat glFormat = format.asGLFormat();
    if (glFormat == GrGLFormat::kUnknown) {
        return {};
    }

    SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);

    GrGLTextureInfo info;
    GrGLTextureParameters::SamplerOverriddenState initialState;

    info.fTarget = GR_GL_TEXTURE_2D;
    info.fFormat = GrGLFormatToEnum(glFormat);
    info.fID = this->createCompressedTexture2D(dimensions, compression, glFormat,
                                               mipMapped, &initialState);
    if (!info.fID) {
        return {};
    }

    // Unbind this texture from the scratch texture unit.
    this->bindTextureToScratchUnit(GR_GL_TEXTURE_2D, 0);

    auto parameters = sk_make_sp<GrGLTextureParameters>();
    // The non-sampler params are still at their default values.
    parameters->set(&initialState, GrGLTextureParameters::NonsamplerState(),
                    fResetTimestampForTextureParameters);

    return GrBackendTexture(dimensions.width(), dimensions.height(), mipMapped, info,
                            std::move(parameters));
}

bool GrGLGpu::onUpdateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                               sk_sp<GrRefCntedCallback> finishedCallback,
                                               const void* data,
                                               size_t length) {
    GrGLTextureInfo info;
    SkAssertResult(backendTexture.getGLTextureInfo(&info));

    GrBackendFormat format = backendTexture.getBackendFormat();
    GrGLFormat glFormat = format.asGLFormat();
    if (glFormat == GrGLFormat::kUnknown) {
        return false;
    }
    SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);

    GrMipmapped mipMapped = backendTexture.hasMipmaps() ? GrMipmapped::kYes : GrMipmapped::kNo;

    this->bindTextureToScratchUnit(info.fTarget, info.fID);

    // If we have mips make sure the base level is set to 0 and the max level set to numMipLevels-1
    // so that the uploads go to the right levels.
    if (backendTexture.hasMipMaps() && this->glCaps().mipmapLevelControlSupport()) {
        auto params = backendTexture.getGLTextureParams();
        GrGLTextureParameters::NonsamplerState nonsamplerState = params->nonsamplerState();
        if (params->nonsamplerState().fBaseMipMapLevel != 0) {
            GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_BASE_LEVEL, 0));
            nonsamplerState.fBaseMipMapLevel = 0;
        }
        int numMipLevels =
                SkMipmap::ComputeLevelCount(backendTexture.width(), backendTexture.height()) + 1;
        if (params->nonsamplerState().fMaxMipmapLevel != (numMipLevels - 1)) {
            GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_MAX_LEVEL, numMipLevels - 1));
            nonsamplerState.fBaseMipMapLevel = numMipLevels - 1;
        }
        params->set(nullptr, nonsamplerState, fResetTimestampForTextureParameters);
    }

    bool result = this->uploadCompressedTexData(compression,
                                                glFormat,
                                                backendTexture.dimensions(),
                                                mipMapped,
                                                GR_GL_TEXTURE_2D,
                                                data,
                                                length);

    // Unbind this texture from the scratch texture unit.
    this->bindTextureToScratchUnit(info.fTarget, 0);

    return result;
}

int GrGLGpu::getCompatibleStencilIndex(GrGLFormat format) {
    static const int kSize = 16;
    SkASSERT(this->glCaps().canFormatBeFBOColorAttachment(format));

    if (!this->glCaps().hasStencilFormatBeenDeterminedForFormat(format)) {
        // Default to unsupported, set this if we find a stencil format that works.
        int firstWorkingStencilFormatIndex = -1;

        GrGLuint colorID = this->createTexture({kSize, kSize}, format, GR_GL_TEXTURE_2D,
                                               GrRenderable::kYes, nullptr, 1);
        if (!colorID) {
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
                GrGLFormat sFmt = this->glCaps().stencilFormats()[i];
                GrGLenum error = GL_ALLOC_CALL(RenderbufferStorage(
                        GR_GL_RENDERBUFFER, GrGLFormatToEnum(sFmt), kSize, kSize));
                if (error == GR_GL_NO_ERROR) {
                    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER,
                                                    GR_GL_STENCIL_ATTACHMENT,
                                                    GR_GL_RENDERBUFFER, sbRBID));
                    if (GrGLFormatIsPackedDepthStencil(sFmt)) {
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
                    if (GrGLFormatIsPackedDepthStencil(sFmt)) {
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

static void set_khr_debug_label(GrGLGpu* gpu, const GrGLuint id) {
    if (gpu->glCaps().debugSupport()) {
        const char* label = "Skia";
        GR_GL_CALL(gpu->glInterface(), ObjectLabel(GR_GL_TEXTURE, id, -1, label));
    }
}

GrGLuint GrGLGpu::createCompressedTexture2D(
        SkISize dimensions,
        SkImage::CompressionType compression,
        GrGLFormat format,
        GrMipmapped mipMapped,
        GrGLTextureParameters::SamplerOverriddenState* initialState) {
    if (format == GrGLFormat::kUnknown) {
        return 0;
    }
    GrGLuint id = 0;
    GL_CALL(GenTextures(1, &id));
    if (!id) {
        return 0;
    }

    this->bindTextureToScratchUnit(GR_GL_TEXTURE_2D, id);

    set_khr_debug_label(this, id);

    *initialState = set_initial_texture_params(this->glInterface(), GR_GL_TEXTURE_2D);

    return id;
}

GrGLuint GrGLGpu::createTexture(SkISize dimensions,
                                GrGLFormat format,
                                GrGLenum target,
                                GrRenderable renderable,
                                GrGLTextureParameters::SamplerOverriddenState* initialState,
                                int mipLevelCount) {
    SkASSERT(format != GrGLFormat::kUnknown);
    SkASSERT(!GrGLFormatIsCompressed(format));

    GrGLuint id = 0;
    GL_CALL(GenTextures(1, &id));

    if (!id) {
        return 0;
    }

    this->bindTextureToScratchUnit(target, id);

    set_khr_debug_label(this, id);

    if (GrRenderable::kYes == renderable && this->glCaps().textureUsageSupport()) {
        // provides a hint about how this texture will be used
        GL_CALL(TexParameteri(target, GR_GL_TEXTURE_USAGE, GR_GL_FRAMEBUFFER_ATTACHMENT));
    }

    if (initialState) {
        *initialState = set_initial_texture_params(this->glInterface(), target);
    } else {
        set_initial_texture_params(this->glInterface(), target);
    }

    GrGLenum internalFormat = this->glCaps().getTexImageOrStorageInternalFormat(format);

    bool success = false;
    if (internalFormat) {
        if (this->glCaps().formatSupportsTexStorage(format)) {
            auto levelCount = std::max(mipLevelCount, 1);
            GrGLenum error = GL_ALLOC_CALL(TexStorage2D(target, levelCount, internalFormat,
                                                        dimensions.width(), dimensions.height()));
            success = (error == GR_GL_NO_ERROR);
        } else {
            GrGLenum externalFormat, externalType;
            this->glCaps().getTexImageExternalFormatAndType(format, &externalFormat, &externalType);
            GrGLenum error = GR_GL_NO_ERROR;
            if (externalFormat && externalType) {
                for (int level = 0; level < mipLevelCount && error == GR_GL_NO_ERROR; level++) {
                    const int twoToTheMipLevel = 1 << level;
                    const int currentWidth = std::max(1, dimensions.width() / twoToTheMipLevel);
                    const int currentHeight = std::max(1, dimensions.height() / twoToTheMipLevel);
                    error = GL_ALLOC_CALL(TexImage2D(target, level, internalFormat, currentWidth,
                                                     currentHeight, 0, externalFormat, externalType,
                                                     nullptr));
                }
                success = (error == GR_GL_NO_ERROR);
            }
        }
    }
    if (success) {
        return id;
    }
    GL_CALL(DeleteTextures(1, &id));
    return 0;
}

sk_sp<GrAttachment> GrGLGpu::makeStencilAttachment(const GrBackendFormat& colorFormat,
                                                   SkISize dimensions, int numStencilSamples) {
    int sIdx = this->getCompatibleStencilIndex(colorFormat.asGLFormat());
    if (sIdx < 0) {
        return nullptr;
    }
    GrGLFormat sFmt = this->glCaps().stencilFormats()[sIdx];

    auto stencil = GrGLAttachment::MakeStencil(this, dimensions, numStencilSamples, sFmt);
    if (stencil) {
        fStats.incStencilAttachmentCreates();
    }
    return std::move(stencil);
}

sk_sp<GrAttachment> GrGLGpu::makeMSAAAttachment(SkISize dimensions, const GrBackendFormat& format,
                                                int numSamples, GrProtected isProtected,
                                                GrMemoryless isMemoryless) {
    SkASSERT(isMemoryless == GrMemoryless::kNo);
    return GrGLAttachment::MakeMSAA(this, dimensions, numSamples, format.asGLFormat());
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<GrGpuBuffer> GrGLGpu::onCreateBuffer(size_t size, GrGpuBufferType intendedType,
                                           GrAccessPattern accessPattern, const void* data) {
    return GrGLBuffer::Make(this, size, intendedType, accessPattern, data);
}

void GrGLGpu::flushScissorTest(GrScissorTest scissorTest) {
    if (GrScissorTest::kEnabled == scissorTest) {
        if (kYes_TriState != fHWScissorSettings.fEnabled) {
            GL_CALL(Enable(GR_GL_SCISSOR_TEST));
            fHWScissorSettings.fEnabled = kYes_TriState;
        }
    } else {
        if (kNo_TriState != fHWScissorSettings.fEnabled) {
            GL_CALL(Disable(GR_GL_SCISSOR_TEST));
            fHWScissorSettings.fEnabled = kNo_TriState;
        }
    }
}

void GrGLGpu::flushScissorRect(const SkIRect& scissor, int rtHeight, GrSurfaceOrigin rtOrigin) {
    SkASSERT(fHWScissorSettings.fEnabled == TriState::kYes_TriState);
    auto nativeScissor = GrNativeRect::MakeRelativeTo(rtOrigin, rtHeight, scissor);
    if (fHWScissorSettings.fRect != nativeScissor) {
        GL_CALL(Scissor(nativeScissor.fX, nativeScissor.fY, nativeScissor.fWidth,
                        nativeScissor.fHeight));
        fHWScissorSettings.fRect = nativeScissor;
    }
}

void GrGLGpu::flushViewport(const SkIRect& viewport, int rtHeight, GrSurfaceOrigin rtOrigin) {
    auto nativeViewport = GrNativeRect::MakeRelativeTo(rtOrigin, rtHeight, viewport);
    if (fHWViewport != nativeViewport) {
        GL_CALL(Viewport(nativeViewport.fX, nativeViewport.fY,
                         nativeViewport.fWidth, nativeViewport.fHeight));
        fHWViewport = nativeViewport;
    }
}

void GrGLGpu::flushWindowRectangles(const GrWindowRectsState& windowState,
                                    const GrGLRenderTarget* rt, GrSurfaceOrigin origin) {
#ifndef USE_NSIGHT
    typedef GrWindowRectsState::Mode Mode;
    // Window rects can't be used on-screen.
    SkASSERT(!windowState.enabled() || !rt->glRTFBOIDis0());
    SkASSERT(windowState.numWindows() <= this->caps()->maxWindowRectangles());

    if (!this->caps()->maxWindowRectangles() ||
        fHWWindowRectsState.knownEqualTo(origin, rt->width(), rt->height(), windowState)) {
        return;
    }

    // This is purely a workaround for a spurious warning generated by gcc. Otherwise the above
    // assert would be sufficient. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=5912
    int numWindows = std::min(windowState.numWindows(), int(GrWindowRectangles::kMaxWindows));
    SkASSERT(windowState.numWindows() == numWindows);

    GrNativeRect glwindows[GrWindowRectangles::kMaxWindows];
    const SkIRect* skwindows = windowState.windows().data();
    for (int i = 0; i < numWindows; ++i) {
        glwindows[i].setRelativeTo(origin, rt->height(), skwindows[i]);
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

bool GrGLGpu::flushGLState(GrRenderTarget* renderTarget, bool useMultisampleFBO,
                           const GrProgramInfo& programInfo) {
    this->handleDirtyContext();

    sk_sp<GrGLProgram> program = fProgramCache->findOrCreateProgram(this->getContext(),
                                                                    programInfo);
    if (!program) {
        GrCapsDebugf(this->caps(), "Failed to create program!\n");
        return false;
    }

    this->flushProgram(std::move(program));

    if (GrPrimitiveType::kPatches == programInfo.primitiveType()) {
        this->flushPatchVertexCount(programInfo.tessellationPatchVertexCount());
    }

    // Swizzle the blend to match what the shader will output.
    this->flushBlendAndColorWrite(programInfo.pipeline().getXferProcessor().getBlendInfo(),
                                  programInfo.pipeline().writeSwizzle());

    fHWProgram->updateUniforms(renderTarget, programInfo);

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(renderTarget);
    GrStencilSettings stencil;
    if (programInfo.isStencilEnabled()) {
        SkASSERT(glRT->getStencilAttachment(useMultisampleFBO));
        stencil.reset(*programInfo.userStencilSettings(),
                      programInfo.pipeline().hasStencilClip(),
                      glRT->numStencilBits(useMultisampleFBO));
    }
    this->flushStencil(stencil, programInfo.origin());
    this->flushScissorTest(GrScissorTest(programInfo.pipeline().isScissorTestEnabled()));
    this->flushWindowRectangles(programInfo.pipeline().getWindowRectsState(),
                                glRT, programInfo.origin());
    this->flushConservativeRasterState(programInfo.pipeline().usesConservativeRaster());
    this->flushWireframeState(programInfo.pipeline().isWireframe());

    // This must come after textures are flushed because a texture may need
    // to be msaa-resolved (which will modify bound FBO state).
    this->flushRenderTarget(glRT, useMultisampleFBO);

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

void GrGLGpu::clear(const GrScissorState& scissor,
                    std::array<float, 4> color,
                    GrRenderTarget* target,
                    bool useMultisampleFBO,
                    GrSurfaceOrigin origin) {
    // parent class should never let us get here with no RT
    SkASSERT(target);
    SkASSERT(!this->caps()->performColorClearsAsDraws());
    SkASSERT(!scissor.enabled() || !this->caps()->performPartialClearsAsDraws());

    this->handleDirtyContext();

    GrGLRenderTarget* glRT = static_cast<GrGLRenderTarget*>(target);

    if (scissor.enabled()) {
        this->flushRenderTarget(glRT, useMultisampleFBO, origin, scissor.rect());
    } else {
        this->flushRenderTarget(glRT, useMultisampleFBO);
    }
    this->flushScissor(scissor, glRT->height(), origin);
    this->disableWindowRectangles();
    this->flushColorWrite(true);
    this->flushClearColor(color);
    GL_CALL(Clear(GR_GL_COLOR_BUFFER_BIT));
}

static bool use_tiled_rendering(const GrGLCaps& glCaps,
                                const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilLoadStore) {
    // Only use the tiled rendering extension if we can explicitly clear and discard the stencil.
    // Otherwise it's faster to just not use it.
    return glCaps.tiledRenderingSupport() && GrLoadOp::kClear == stencilLoadStore.fLoadOp &&
           GrStoreOp::kDiscard == stencilLoadStore.fStoreOp;
}

void GrGLGpu::beginCommandBuffer(GrGLRenderTarget* rt, bool useMultisampleFBO,
                                 const SkIRect& bounds, GrSurfaceOrigin origin,
                                 const GrOpsRenderPass::LoadAndStoreInfo& colorLoadStore,
                                 const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilLoadStore) {
    SkASSERT(!fIsExecutingCommandBuffer_DebugOnly);

    this->handleDirtyContext();

    this->flushRenderTarget(rt, useMultisampleFBO);
    SkDEBUGCODE(fIsExecutingCommandBuffer_DebugOnly = true);

    if (use_tiled_rendering(this->glCaps(), stencilLoadStore)) {
        auto nativeBounds = GrNativeRect::MakeRelativeTo(origin, rt->height(), bounds);
        GrGLbitfield preserveMask = (GrLoadOp::kLoad == colorLoadStore.fLoadOp)
                ? GR_GL_COLOR_BUFFER_BIT0 : GR_GL_NONE;
        SkASSERT(GrLoadOp::kLoad != stencilLoadStore.fLoadOp);  // Handled by use_tiled_rendering().
        GL_CALL(StartTiling(nativeBounds.fX, nativeBounds.fY, nativeBounds.fWidth,
                            nativeBounds.fHeight, preserveMask));
    }

    GrGLbitfield clearMask = 0;
    if (GrLoadOp::kClear == colorLoadStore.fLoadOp) {
        SkASSERT(!this->caps()->performColorClearsAsDraws());
        this->flushClearColor(colorLoadStore.fClearColor);
        this->flushColorWrite(true);
        clearMask |= GR_GL_COLOR_BUFFER_BIT;
    }
    if (GrLoadOp::kClear == stencilLoadStore.fLoadOp) {
        SkASSERT(!this->caps()->performStencilClearsAsDraws());
        GL_CALL(StencilMask(0xffffffff));
        GL_CALL(ClearStencil(0));
        clearMask |= GR_GL_STENCIL_BUFFER_BIT;
    }
    if (clearMask) {
        this->flushScissorTest(GrScissorTest::kDisabled);
        this->disableWindowRectangles();
        GL_CALL(Clear(clearMask));
    }
}

void GrGLGpu::endCommandBuffer(GrGLRenderTarget* rt, bool useMultisampleFBO,
                               const GrOpsRenderPass::LoadAndStoreInfo& colorLoadStore,
                               const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilLoadStore) {
    SkASSERT(fIsExecutingCommandBuffer_DebugOnly);

    this->handleDirtyContext();

    if (rt->uniqueID() != fHWBoundRenderTargetUniqueID ||
        useMultisampleFBO != fHWBoundFramebufferIsMSAA) {
        // The framebuffer binding changed in the middle of a command buffer. We should have already
        // printed a warning during onFBOChanged.
        return;
    }

    if (GrGLCaps::kNone_InvalidateFBType != this->glCaps().invalidateFBType()) {
        SkSTArray<2, GrGLenum> discardAttachments;
        if (GrStoreOp::kDiscard == colorLoadStore.fStoreOp) {
            discardAttachments.push_back(
                    rt->isFBO0(useMultisampleFBO) ? GR_GL_COLOR : GR_GL_COLOR_ATTACHMENT0);
        }
        if (GrStoreOp::kDiscard == stencilLoadStore.fStoreOp) {
            discardAttachments.push_back(
                    rt->isFBO0(useMultisampleFBO) ? GR_GL_STENCIL : GR_GL_STENCIL_ATTACHMENT);

        }

        if (!discardAttachments.empty()) {
            if (GrGLCaps::kInvalidate_InvalidateFBType == this->glCaps().invalidateFBType()) {
                GL_CALL(InvalidateFramebuffer(GR_GL_FRAMEBUFFER, discardAttachments.count(),
                                              discardAttachments.begin()));
            } else {
                SkASSERT(GrGLCaps::kDiscard_InvalidateFBType == this->glCaps().invalidateFBType());
                GL_CALL(DiscardFramebuffer(GR_GL_FRAMEBUFFER, discardAttachments.count(),
                                           discardAttachments.begin()));
            }
        }
    }

    if (use_tiled_rendering(this->glCaps(), stencilLoadStore)) {
        GrGLbitfield preserveMask = (GrStoreOp::kStore == colorLoadStore.fStoreOp)
                ? GR_GL_COLOR_BUFFER_BIT0 : GR_GL_NONE;
        // Handled by use_tiled_rendering().
        SkASSERT(GrStoreOp::kStore != stencilLoadStore.fStoreOp);
        GL_CALL(EndTiling(preserveMask));
    }

    SkDEBUGCODE(fIsExecutingCommandBuffer_DebugOnly = false);
}

void GrGLGpu::clearStencilClip(const GrScissorState& scissor, bool insideStencilMask,
                               GrRenderTarget* target, bool useMultisampleFBO,
                               GrSurfaceOrigin origin) {
    SkASSERT(target);
    SkASSERT(!this->caps()->performStencilClearsAsDraws());
    SkASSERT(!scissor.enabled() || !this->caps()->performPartialClearsAsDraws());
    this->handleDirtyContext();

    GrAttachment* sb = target->getStencilAttachment(useMultisampleFBO);
    if (!sb) {
        // We should only get here if we marked a proxy as requiring a SB. However,
        // the SB creation could later fail. Likely clipping is going to go awry now.
        return;
    }

    GrGLint stencilBitCount =  GrBackendFormatStencilBits(sb->backendFormat());
#if 0
    SkASSERT(stencilBitCount > 0);
    GrGLint clipStencilMask  = (1 << (stencilBitCount - 1));
#else
    // we could just clear the clip bit but when we go through
    // ANGLE a partial stencil mask will cause clears to be
    // turned into draws. Our contract on OpsTask says that
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
    this->flushRenderTargetNoColorWrites(glRT, useMultisampleFBO);

    this->flushScissor(scissor, glRT->height(), origin);
    this->disableWindowRectangles();

    GL_CALL(StencilMask((uint32_t) clipStencilMask));
    GL_CALL(ClearStencil(value));
    GL_CALL(Clear(GR_GL_STENCIL_BUFFER_BIT));
    fHWStencilSettings.invalidate();
}

bool GrGLGpu::readOrTransferPixelsFrom(GrSurface* surface,
                                       SkIRect rect,
                                       GrColorType surfaceColorType,
                                       GrColorType dstColorType,
                                       void* offsetOrPtr,
                                       int rowWidthInPixels) {
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
        // Always bind the single sample FBO since we can't read pixels from an MSAA framebuffer.
        constexpr bool useMultisampleFBO = false;
        if (renderTarget->numSamples() > 1 && renderTarget->isFBO0(useMultisampleFBO)) {
            return false;
        }
        this->flushRenderTargetNoColorWrites(renderTarget, useMultisampleFBO);
    } else {
        // Use a temporary FBO.
        this->bindSurfaceFBOForPixelOps(surface, 0, GR_GL_FRAMEBUFFER, kSrc_TempFBOTarget);
        fHWBoundRenderTargetUniqueID.makeInvalid();
    }

    // determine if GL can read using the passed rowBytes or if we need a scratch buffer.
    if (rowWidthInPixels != rect.width()) {
        SkASSERT(this->glCaps().readPixelsRowBytesSupport());
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, rowWidthInPixels));
    }
    GL_CALL(PixelStorei(GR_GL_PACK_ALIGNMENT, 1));

    GL_CALL(ReadPixels(rect.left(),
                       rect.top(),
                       rect.width(),
                       rect.height(),
                       externalFormat,
                       externalType,
                       offsetOrPtr));

    if (rowWidthInPixels != rect.width()) {
        SkASSERT(this->glCaps().readPixelsRowBytesSupport());
        GL_CALL(PixelStorei(GR_GL_PACK_ROW_LENGTH, 0));
    }

    if (!renderTarget) {
        this->unbindSurfaceFBOForPixelOps(surface, 0, GR_GL_FRAMEBUFFER);
    }
    return true;
}

bool GrGLGpu::onReadPixels(GrSurface* surface,
                           SkIRect rect,
                           GrColorType surfaceColorType,
                           GrColorType dstColorType,
                           void* buffer,
                           size_t rowBytes) {
    SkASSERT(surface);

    size_t bytesPerPixel = GrColorTypeBytesPerPixel(dstColorType);

    // GL_PACK_ROW_LENGTH is in terms of pixels not bytes.
    int rowPixelWidth;

    if (rowBytes == SkToSizeT(rect.width()*bytesPerPixel)) {
        rowPixelWidth = rect.width();
    } else {
        SkASSERT(!(rowBytes % bytesPerPixel));
        rowPixelWidth = rowBytes / bytesPerPixel;
    }
    this->unbindXferBuffer(GrGpuBufferType::kXferGpuToCpu);
    return this->readOrTransferPixelsFrom(surface,
                                          rect,
                                          surfaceColorType,
                                          dstColorType,
                                          buffer,
                                          rowPixelWidth);
}

GrOpsRenderPass* GrGLGpu::onGetOpsRenderPass(
        GrRenderTarget* rt,
        bool useMultisampleFBO,
        GrAttachment*,
        GrSurfaceOrigin origin,
        const SkIRect& bounds,
        const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
        const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
        const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
        GrXferBarrierFlags renderPassXferBarriers) {
    if (!fCachedOpsRenderPass) {
        fCachedOpsRenderPass = std::make_unique<GrGLOpsRenderPass>(this);
    }
    if (useMultisampleFBO && rt->numSamples() == 1) {
        // We will be using dynamic msaa. Ensure there is an attachment.
        auto glRT = static_cast<GrGLRenderTarget*>(rt);
        if (!glRT->ensureDynamicMSAAAttachment()) {
            SkDebugf("WARNING: Failed to make dmsaa attachment. Render pass will be dropped.");
            return nullptr;
        }
    }
    fCachedOpsRenderPass->set(rt, useMultisampleFBO, bounds, origin, colorInfo, stencilInfo);
    return fCachedOpsRenderPass.get();
}

void GrGLGpu::flushRenderTarget(GrGLRenderTarget* target, bool useMultisampleFBO,
                                GrSurfaceOrigin origin, const SkIRect& bounds) {
    this->flushRenderTargetNoColorWrites(target, useMultisampleFBO);
    this->didWriteToSurface(target, origin, &bounds);
}

void GrGLGpu::flushRenderTarget(GrGLRenderTarget* target, bool useMultisampleFBO) {
    this->flushRenderTargetNoColorWrites(target, useMultisampleFBO);
    this->didWriteToSurface(target, kTopLeft_GrSurfaceOrigin, nullptr);
}

void GrGLGpu::flushRenderTargetNoColorWrites(GrGLRenderTarget* target, bool useMultisampleFBO) {
    SkASSERT(target);
    GrGpuResource::UniqueID rtID = target->uniqueID();
    if (fHWBoundRenderTargetUniqueID != rtID || fHWBoundFramebufferIsMSAA != useMultisampleFBO) {
        target->bind(useMultisampleFBO);
#ifdef SK_DEBUG
        // don't do this check in Chromium -- this is causing
        // lots of repeated command buffer flushes when the compositor is
        // rendering with Ganesh, which is really slow; even too slow for
        // Debug mode.
        if (!this->glCaps().skipErrorChecks()) {
            GrGLenum status;
            GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
            if (status != GR_GL_FRAMEBUFFER_COMPLETE) {
                SkDebugf("GrGLGpu::flushRenderTargetNoColorWrites glCheckFramebufferStatus %x\n",
                         status);
            }
        }
#endif
        fHWBoundRenderTargetUniqueID = rtID;
        fHWBoundFramebufferIsMSAA = useMultisampleFBO;
        this->flushViewport(SkIRect::MakeSize(target->dimensions()),
                            target->height(),
                            kTopLeft_GrSurfaceOrigin); // the origin is irrelevant in this case
    }
    if (this->caps()->workarounds().force_update_scissor_state_when_binding_fbo0) {
        // The driver forgets the correct scissor state when using FBO 0.
        if (!fHWScissorSettings.fRect.isInvalid()) {
            const GrNativeRect& r = fHWScissorSettings.fRect;
            GL_CALL(Scissor(r.fX, r.fY, r.fWidth, r.fHeight));
        }
        if (fHWScissorSettings.fEnabled == kYes_TriState) {
            GL_CALL(Disable(GR_GL_SCISSOR_TEST));
            GL_CALL(Enable(GR_GL_SCISSOR_TEST));
        } else if (fHWScissorSettings.fEnabled == kNo_TriState) {
            GL_CALL(Enable(GR_GL_SCISSOR_TEST));
            GL_CALL(Disable(GR_GL_SCISSOR_TEST));
        }
    }

    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(this->caps()->isFormatSRGB(target->backendFormat()));
    }

    if (this->glCaps().shouldQueryImplementationReadSupport(target->format())) {
        GrGLint format;
        GrGLint type;
        GR_GL_GetIntegerv(this->glInterface(), GR_GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
        GR_GL_GetIntegerv(this->glInterface(), GR_GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
        this->glCaps().didQueryImplementationReadSupport(target->format(), format, type);
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

GrGLenum GrGLGpu::prepareToDraw(GrPrimitiveType primitiveType) {
    fStats.incNumDraws();

    if (this->glCaps().requiresCullFaceEnableDisableWhenDrawingLinesAfterNonLines() &&
        GrIsPrimTypeLines(primitiveType) && !GrIsPrimTypeLines(fLastPrimitiveType)) {
        GL_CALL(Enable(GR_GL_CULL_FACE));
        GL_CALL(Disable(GR_GL_CULL_FACE));
    }
    fLastPrimitiveType = primitiveType;

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
        case GrPrimitiveType::kPatches:
            return GR_GL_PATCHES;
        case GrPrimitiveType::kPath:
            SK_ABORT("non-mesh-based GrPrimitiveType");
            return 0;
    }
    SK_ABORT("invalid GrPrimitiveType");
}

void GrGLGpu::onResolveRenderTarget(GrRenderTarget* target, const SkIRect& resolveRect) {
    auto glRT = static_cast<GrGLRenderTarget*>(target);
    if (this->glCaps().framebufferResolvesMustBeFullSize()) {
        this->resolveRenderFBOs(glRT, SkIRect::MakeSize(glRT->dimensions()),
                                ResolveDirection::kMSAAToSingle);
    } else {
        this->resolveRenderFBOs(glRT, resolveRect, ResolveDirection::kMSAAToSingle);
    }
}

void GrGLGpu::resolveRenderFBOs(GrGLRenderTarget* rt, const SkIRect& resolveRect,
                                ResolveDirection resolveDirection,
                                bool invalidateReadBufferAfterBlit) {
    this->handleDirtyContext();
    rt->bindForResolve(resolveDirection);

    const GrGLCaps& caps = this->glCaps();

    // make sure we go through flushRenderTarget() since we've modified
    // the bound DRAW FBO ID.
    fHWBoundRenderTargetUniqueID.makeInvalid();
    if (GrGLCaps::kES_Apple_MSFBOType == caps.msFBOType()) {
        // The Apple extension doesn't support blitting from single to multisample.
        SkASSERT(resolveDirection != ResolveDirection::kSingleToMSAA);
        SkASSERT(resolveRect == SkIRect::MakeSize(rt->dimensions()));
        // Apple's extension uses the scissor as the blit bounds.
        // Passing in kTopLeft_GrSurfaceOrigin will make sure no transformation of the rect
        // happens inside flushScissor since resolveRect is already in native device coordinates.
        GrScissorState scissor(rt->dimensions());
        SkAssertResult(scissor.set(resolveRect));
        this->flushScissor(scissor, rt->height(), kTopLeft_GrSurfaceOrigin);
        this->disableWindowRectangles();
        GL_CALL(ResolveMultisampleFramebuffer());
    } else {
        SkASSERT(!caps.framebufferResolvesMustBeFullSize() ||
                 resolveRect == SkIRect::MakeSize(rt->dimensions()));
        int l = resolveRect.x();
        int b = resolveRect.y();
        int r = resolveRect.x() + resolveRect.width();
        int t = resolveRect.y() + resolveRect.height();

        // BlitFrameBuffer respects the scissor, so disable it.
        this->flushScissorTest(GrScissorTest::kDisabled);
        this->disableWindowRectangles();
        GL_CALL(BlitFramebuffer(l, b, r, t, l, b, r, t, GR_GL_COLOR_BUFFER_BIT, GR_GL_NEAREST));
    }

    if (caps.invalidateFBType() != GrGLCaps::kNone_InvalidateFBType &&
        invalidateReadBufferAfterBlit) {
        // Invalidate the read FBO attachment after the blit, in hopes that this allows the driver
        // to perform tiling optimizations.
        bool readBufferIsMSAA = resolveDirection == ResolveDirection::kMSAAToSingle;
        GrGLenum colorDiscardAttachment = rt->isFBO0(readBufferIsMSAA) ? GR_GL_COLOR
                                                                       : GR_GL_COLOR_ATTACHMENT0;
        if (caps.invalidateFBType() == GrGLCaps::kInvalidate_InvalidateFBType) {
            GL_CALL(InvalidateFramebuffer(GR_GL_READ_FRAMEBUFFER, 1, &colorDiscardAttachment));
        } else {
            SkASSERT(caps.invalidateFBType() == GrGLCaps::kDiscard_InvalidateFBType);
            // glDiscardFramebuffer only accepts GL_FRAMEBUFFER.
            rt->bind(readBufferIsMSAA);
            GL_CALL(DiscardFramebuffer(GR_GL_FRAMEBUFFER, 1, &colorDiscardAttachment));
        }
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
    static_assert(0 == (int)GrStencilOp::kKeep);
    static_assert(1 == (int)GrStencilOp::kZero);
    static_assert(2 == (int)GrStencilOp::kReplace);
    static_assert(3 == (int)GrStencilOp::kInvert);
    static_assert(4 == (int)GrStencilOp::kIncWrap);
    static_assert(5 == (int)GrStencilOp::kDecWrap);
    static_assert(6 == (int)GrStencilOp::kIncClamp);
    static_assert(7 == (int)GrStencilOp::kDecClamp);
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
}  // namespace

void GrGLGpu::flushStencil(const GrStencilSettings& stencilSettings, GrSurfaceOrigin origin) {
    if (stencilSettings.isDisabled()) {
        this->disableStencil();
    } else if (fHWStencilSettings != stencilSettings ||
               (stencilSettings.isTwoSided() && fHWStencilOrigin != origin)) {
        if (kYes_TriState != fHWStencilTestEnabled) {
            GL_CALL(Enable(GR_GL_STENCIL_TEST));

            fHWStencilTestEnabled = kYes_TriState;
        }
        if (!stencilSettings.isTwoSided()) {
            set_gl_stencil(this->glInterface(), stencilSettings.singleSidedFace(),
                           GR_GL_FRONT_AND_BACK);
        } else {
            set_gl_stencil(this->glInterface(), stencilSettings.postOriginCWFace(origin),
                           GR_GL_FRONT);
            set_gl_stencil(this->glInterface(), stencilSettings.postOriginCCWFace(origin),
                           GR_GL_BACK);
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

void GrGLGpu::flushConservativeRasterState(bool enabled) {
    if (this->caps()->conservativeRasterSupport()) {
        if (enabled) {
            if (kYes_TriState != fHWConservativeRasterEnabled) {
                GL_CALL(Enable(GR_GL_CONSERVATIVE_RASTERIZATION));
                fHWConservativeRasterEnabled = kYes_TriState;
            }
        } else {
            if (kNo_TriState != fHWConservativeRasterEnabled) {
                GL_CALL(Disable(GR_GL_CONSERVATIVE_RASTERIZATION));
                fHWConservativeRasterEnabled = kNo_TriState;
            }
        }
    }
}

void GrGLGpu::flushWireframeState(bool enabled) {
    if (this->caps()->wireframeSupport()) {
        if (this->caps()->wireframeMode() || enabled) {
            if (kYes_TriState != fHWWireframeEnabled) {
                GL_CALL(PolygonMode(GR_GL_FRONT_AND_BACK, GR_GL_LINE));
                fHWWireframeEnabled = kYes_TriState;
            }
        } else {
            if (kNo_TriState != fHWWireframeEnabled) {
                GL_CALL(PolygonMode(GR_GL_FRONT_AND_BACK, GR_GL_FILL));
                fHWWireframeEnabled = kNo_TriState;
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
    bool blendOff = GrBlendShouldDisable(equation, srcCoeff, dstCoeff) ||
                    !blendInfo.fWriteColor;

    if (blendOff) {
        if (kNo_TriState != fHWBlendState.fEnabled) {
            GL_CALL(Disable(GR_GL_BLEND));

            // Workaround for the ARM KHR_blend_equation_advanced disable flags issue
            // https://code.google.com/p/skia/issues/detail?id=3943
            if (this->ctxInfo().vendor() == GrGLVendor::kARM &&
                GrBlendEquationIsAdvanced(fHWBlendState.fEquation)) {
                SkASSERT(this->caps()->advancedBlendEquationSupport());
                // Set to any basic blending equation.
                GrBlendEquation blend_equation = kAdd_GrBlendEquation;
                GL_CALL(BlendEquation(gXfermodeEquation2Blend[blend_equation]));
                fHWBlendState.fEquation = blend_equation;
            }

            // Workaround for Adreno 5xx BlendFunc bug. See crbug.com/1241134.
            // We must also check to see if the blend coeffs are invalid because the client may have
            // reset our gl state and thus we will have forgotten if the previous use was a coeff
            // that referenced src2.
            if (this->glCaps().mustResetBlendFuncBetweenDualSourceAndDisable() &&
                (GrBlendCoeffRefsSrc2(fHWBlendState.fSrcCoeff) ||
                 GrBlendCoeffRefsSrc2(fHWBlendState.fDstCoeff) ||
                 fHWBlendState.fSrcCoeff == kIllegal_GrBlendCoeff ||
                 fHWBlendState.fDstCoeff == kIllegal_GrBlendCoeff)) {
                // We just reset the blend func to anything that doesn't reference src2
                GL_CALL(BlendFunc(GR_GL_ONE, GR_GL_ZERO));
                fHWBlendState.fSrcCoeff = kOne_GrBlendCoeff;
                fHWBlendState.fDstCoeff = kZero_GrBlendCoeff;
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

        if (GrBlendCoeffRefsConstant(srcCoeff) || GrBlendCoeffRefsConstant(dstCoeff)) {
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

void GrGLGpu::bindTexture(int unitIdx, GrSamplerState samplerState, const GrSwizzle& swizzle,
                          GrGLTexture* texture) {
    SkASSERT(texture);

#ifdef SK_DEBUG
    if (!this->caps()->npotTextureTileSupport()) {
        if (samplerState.isRepeatedX()) {
            const int w = texture->width();
            SkASSERT(SkIsPow2(w));
        }
        if (samplerState.isRepeatedY()) {
            const int h = texture->height();
            SkASSERT(SkIsPow2(h));
        }
    }
#endif

    GrGpuResource::UniqueID textureID = texture->uniqueID();
    GrGLenum target = texture->target();
    if (fHWTextureUnitBindings[unitIdx].boundID(target) != textureID) {
        this->setTextureUnit(unitIdx);
        GL_CALL(BindTexture(target, texture->textureID()));
        fHWTextureUnitBindings[unitIdx].setBoundID(target, textureID);
    }

    if (samplerState.mipmapped() == GrMipmapped::kYes) {
        if (!this->caps()->mipmapSupport() || texture->mipmapped() == GrMipmapped::kNo) {
            samplerState.setMipmapMode(GrSamplerState::MipmapMode::kNone);
        } else {
            SkASSERT(!texture->mipmapsAreDirty());
        }
    }

    auto timestamp = texture->parameters()->resetTimestamp();
    bool setAll = timestamp < fResetTimestampForTextureParameters;
    const GrGLTextureParameters::SamplerOverriddenState* samplerStateToRecord = nullptr;
    GrGLTextureParameters::SamplerOverriddenState newSamplerState;
    if (this->glCaps().useSamplerObjects()) {
        fSamplerObjectCache->bindSampler(unitIdx, samplerState);
        if (this->glCaps().mustSetAnyTexParameterToEnableMipmapping()) {
            if (samplerState.mipmapped() == GrMipmapped::kYes) {
                GrGLenum minFilter = filter_to_gl_min_filter(samplerState.filter(),
                                                             samplerState.mipmapMode());
                const GrGLTextureParameters::SamplerOverriddenState& oldSamplerState =
                        texture->parameters()->samplerOverriddenState();
                this->setTextureUnit(unitIdx);
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MIN_FILTER, minFilter));
                newSamplerState = oldSamplerState;
                newSamplerState.fMinFilter = minFilter;
                samplerStateToRecord = &newSamplerState;
            }
        }
    } else {
        if (fSamplerObjectCache) {
            fSamplerObjectCache->unbindSampler(unitIdx);
        }
        const GrGLTextureParameters::SamplerOverriddenState& oldSamplerState =
                texture->parameters()->samplerOverriddenState();
        samplerStateToRecord = &newSamplerState;

        newSamplerState.fMinFilter = filter_to_gl_min_filter(samplerState.filter(),
                                                             samplerState.mipmapMode());
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
        if (this->glCaps().mipmapLodControlSupport()) {
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
    newNonsamplerState.fMaxMipmapLevel = texture->maxMipmapLevel();
    newNonsamplerState.fSwizzleIsRGBA = true;

    const GrGLTextureParameters::NonsamplerState& oldNonsamplerState =
            texture->parameters()->nonsamplerState();
    if (this->glCaps().textureSwizzleSupport()) {
        if (setAll || !oldNonsamplerState.fSwizzleIsRGBA) {
            static constexpr GrGLenum kRGBA[4] {
                GR_GL_RED,
                GR_GL_GREEN,
                GR_GL_BLUE,
                GR_GL_ALPHA
            };
            this->setTextureUnit(unitIdx);
            if (GR_IS_GR_GL(this->glStandard())) {
                static_assert(sizeof(kRGBA[0]) == sizeof(GrGLint));
                GL_CALL(TexParameteriv(target, GR_GL_TEXTURE_SWIZZLE_RGBA,
                                       reinterpret_cast<const GrGLint*>(kRGBA)));
            } else if (GR_IS_GR_GL_ES(this->glStandard())) {
                // ES3 added swizzle support but not GL_TEXTURE_SWIZZLE_RGBA.
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_R, kRGBA[0]));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_G, kRGBA[1]));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_B, kRGBA[2]));
                GL_CALL(TexParameteri(target, GR_GL_TEXTURE_SWIZZLE_A, kRGBA[3]));
            }
        }
    }
    // These are not supported in ES2 contexts
    if (this->glCaps().mipmapLevelControlSupport() &&
        (texture->textureType() != GrTextureType::kExternal ||
         !this->glCaps().dontSetBaseOrMaxLevelForExternalTextures())) {
        if (newNonsamplerState.fBaseMipMapLevel != oldNonsamplerState.fBaseMipMapLevel) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_BASE_LEVEL,
                                  newNonsamplerState.fBaseMipMapLevel));
        }
        if (newNonsamplerState.fMaxMipmapLevel != oldNonsamplerState.fMaxMipmapLevel) {
            this->setTextureUnit(unitIdx);
            GL_CALL(TexParameteri(target, GR_GL_TEXTURE_MAX_LEVEL,
                                  newNonsamplerState.fMaxMipmapLevel));
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

void GrGLGpu::flushPatchVertexCount(uint8_t count) {
    SkASSERT(this->caps()->shaderCaps()->tessellationSupport());
    if (fHWPatchVertexCount != count) {
        GL_CALL(PatchParameteri(GR_GL_PATCH_VERTICES, count));
        fHWPatchVertexCount = count;
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

void GrGLGpu::flushClearColor(std::array<float, 4> color) {
    GrGLfloat r = color[0], g = color[1], b = color[2], a = color[3];
    if (this->glCaps().clearToBoundaryValuesIsBroken() &&
        (1 == r || 0 == r) && (1 == g || 0 == g) && (1 == b || 0 == b) && (1 == a || 0 == a)) {
        static const GrGLfloat safeAlpha1 = nextafter(1.f, 2.f);
        static const GrGLfloat safeAlpha0 = nextafter(0.f, -1.f);
        a = (1 == a) ? safeAlpha1 : safeAlpha0;
    }
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
        dstTexType = dstTex->textureType();
        dstTexTypePtr = &dstTexType;
    }
    if (srcTex) {
        srcTexType = srcTex->textureType();
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
    return rt->numSamples() > 1 && glCaps.usesMSAARenderBuffers() && !rt->isFBO0(true/*msaa*/);
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
        dstTexType = dstTex->textureType();
        dstTexTypePtr = &dstTexType;
    }
    if (srcTex) {
        srcTexType = srcTex->textureType();
        srcTexTypePtr = &srcTexType;
    }

    return caps.canCopyTexSubImage(dstFormat, dstHasMSAARenderBuffer, dstTexTypePtr,
                                   srcFormat, srcHasMSAARenderBuffer, srcTexTypePtr);
}

void GrGLGpu::bindSurfaceFBOForPixelOps(GrSurface* surface, int mipLevel, GrGLenum fboTarget,
                                        TempFBOTarget tempFBOTarget) {
    GrGLRenderTarget* rt = static_cast<GrGLRenderTarget*>(surface->asRenderTarget());
    if (!rt || mipLevel > 0) {
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
        GR_GL_CALL(
                this->glInterface(),
                FramebufferTexture2D(fboTarget, GR_GL_COLOR_ATTACHMENT0, target, texID, mipLevel));
        if (mipLevel == 0) {
            texture->baseLevelWasBoundToFBO();
        }
    } else {
        rt->bindForPixelOps(fboTarget);
    }
}

void GrGLGpu::unbindSurfaceFBOForPixelOps(GrSurface* surface, int mipLevel, GrGLenum fboTarget) {
    // bindSurfaceFBOForPixelOps temporarily binds textures that are not render targets to
    if (mipLevel > 0 || !surface->asRenderTarget()) {
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
    if (this->caps()->workarounds().flush_on_framebuffer_change) {
        this->flush(FlushType::kForce);
    }
#ifdef SK_DEBUG
    if (fIsExecutingCommandBuffer_DebugOnly) {
        SkDebugf("WARNING: GL FBO binding changed while executing a command buffer. "
                 "This will severely hurt performance.\n");
    }
#endif
}

void GrGLGpu::bindFramebuffer(GrGLenum target, GrGLuint fboid) {
    GL_CALL(BindFramebuffer(target, fboid));
    if (target == GR_GL_FRAMEBUFFER || target == GR_GL_DRAW_FRAMEBUFFER) {
        fBoundDrawFramebuffer = fboid;
    }
    this->onFBOChanged();
}

void GrGLGpu::deleteFramebuffer(GrGLuint fboid) {
    // We're relying on the GL state shadowing being correct in the workaround code below so we
    // need to handle a dirty context.
    this->handleDirtyContext();
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
                            const SkIPoint& dstPoint) {
    // Don't prefer copying as a draw if the dst doesn't already have a FBO object.
    // This implicitly handles this->glCaps().useDrawInsteadOfAllRenderTargetWrites().
    bool preferCopy = SkToBool(dst->asRenderTarget());
    auto dstFormat = dst->backendFormat().asGLFormat();
    if (preferCopy && this->glCaps().canCopyAsDraw(dstFormat, SkToBool(src->asTexture()))) {
        GrRenderTarget* dstRT = dst->asRenderTarget();
        bool drawToMultisampleFBO = dstRT && dstRT->numSamples() > 1;
        if (this->copySurfaceAsDraw(dst, drawToMultisampleFBO, src, srcRect, dstPoint)) {
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
        GrRenderTarget* dstRT = dst->asRenderTarget();
        bool drawToMultisampleFBO = dstRT && dstRT->numSamples() > 1;
        if (this->copySurfaceAsDraw(dst, drawToMultisampleFBO, src, srcRect, dstPoint)) {
            return true;
        }
    }

    return false;
}

bool GrGLGpu::createCopyProgram(GrTexture* srcTex) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    int progIdx = TextureToCopyProgramIdx(srcTex);
    const GrShaderCaps* shaderCaps = this->caps()->shaderCaps();
    GrSLType samplerType = GrSLCombinedSamplerTypeForTextureType(srcTex->textureType());

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

    GrShaderVar aVertex("a_vertex", kHalf2_GrSLType, GrShaderVar::TypeModifier::In);
    GrShaderVar uTexCoordXform("u_texCoordXform", kHalf4_GrSLType,
                               GrShaderVar::TypeModifier::Uniform);
    GrShaderVar uPosXform("u_posXform", kHalf4_GrSLType, GrShaderVar::TypeModifier::Uniform);
    GrShaderVar uTexture("u_texture", samplerType, GrShaderVar::TypeModifier::Uniform);
    GrShaderVar vTexCoord("v_texCoord", kHalf2_GrSLType, GrShaderVar::TypeModifier::Out);
    GrShaderVar oFragColor("o_FragColor", kHalf4_GrSLType, GrShaderVar::TypeModifier::Out);

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
    vTexCoord.setTypeModifier(GrShaderVar::TypeModifier::In);
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
    SkSL::String glsl;
    std::unique_ptr<SkSL::Program> program = GrSkSLtoGLSL(this, SkSL::ProgramKind::kVertex,
                                                          sksl, settings, &glsl, errorHandler);
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, glsl, fProgramCache->stats(),
                                                  errorHandler);
    SkASSERT(program->fInputs == SkSL::Program::Inputs());

    sksl.assign(fshaderTxt.c_str(), fshaderTxt.size());
    program = GrSkSLtoGLSL(this, SkSL::ProgramKind::kFragment, sksl, settings, &glsl,
                           errorHandler);
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fCopyPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, glsl,
                                                  fProgramCache->stats(), errorHandler);
    SkASSERT(program->fInputs == SkSL::Program::Inputs());

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

    GrShaderVar aVertex("a_vertex", kHalf2_GrSLType, GrShaderVar::TypeModifier::In);
    GrShaderVar uTexCoordXform("u_texCoordXform", kHalf4_GrSLType,
                               GrShaderVar::TypeModifier::Uniform);
    GrShaderVar uTexture("u_texture", kTexture2DSampler_GrSLType,
                         GrShaderVar::TypeModifier::Uniform);
    // We need 1, 2, or 4 texture coordinates (depending on parity of each dimension):
    GrShaderVar vTexCoords[] = {
        GrShaderVar("v_texCoord0", kHalf2_GrSLType, GrShaderVar::TypeModifier::Out),
        GrShaderVar("v_texCoord1", kHalf2_GrSLType, GrShaderVar::TypeModifier::Out),
        GrShaderVar("v_texCoord2", kHalf2_GrSLType, GrShaderVar::TypeModifier::Out),
        GrShaderVar("v_texCoord3", kHalf2_GrSLType, GrShaderVar::TypeModifier::Out),
    };
    GrShaderVar oFragColor("o_FragColor", kHalf4_GrSLType,GrShaderVar::TypeModifier::Out);

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
        "  sk_Position.xy = a_vertex * half2(2) - half2(1);"
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
        vTexCoords[i].setTypeModifier(GrShaderVar::TypeModifier::In);
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
    SkSL::String glsl;
    std::unique_ptr<SkSL::Program> program = GrSkSLtoGLSL(this, SkSL::ProgramKind::kVertex,
                                                          sksl, settings, &glsl, errorHandler);
    GrGLuint vshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_VERTEX_SHADER, glsl,
                                                  fProgramCache->stats(), errorHandler);
    SkASSERT(program->fInputs == SkSL::Program::Inputs());

    sksl.assign(fshaderTxt.c_str(), fshaderTxt.size());
    program = GrSkSLtoGLSL(this, SkSL::ProgramKind::kFragment, sksl, settings, &glsl,
                           errorHandler);
    GrGLuint fshader = GrGLCompileAndAttachShader(*fGLContext, fMipmapPrograms[progIdx].fProgram,
                                                  GR_GL_FRAGMENT_SHADER, glsl,
                                                  fProgramCache->stats(), errorHandler);
    SkASSERT(program->fInputs == SkSL::Program::Inputs());

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

bool GrGLGpu::copySurfaceAsDraw(GrSurface* dst, bool drawToMultisampleFBO, GrSurface* src,
                                const SkIRect& srcRect, const SkIPoint& dstPoint) {
    auto* srcTex = static_cast<GrGLTexture*>(src->asTexture());
    if (!srcTex) {
        return false;
    }
    // We don't swizzle at all in our copies.
    this->bindTexture(0, GrSamplerState::Filter::kNearest, GrSwizzle::RGBA(), srcTex);
    if (auto* dstRT = static_cast<GrGLRenderTarget*>(dst->asRenderTarget())) {
        this->flushRenderTargetNoColorWrites(dstRT, drawToMultisampleFBO);
    } else {
        auto* dstTex = static_cast<GrGLTexture*>(src->asTexture());
        SkASSERT(dstTex);
        SkASSERT(!drawToMultisampleFBO);
        if (!this->glCaps().isFormatRenderable(dstTex->format(), 1)) {
            return false;
        }
        this->bindSurfaceFBOForPixelOps(dst, 0, GR_GL_FRAMEBUFFER, kDst_TempFBOTarget);
        fHWBoundRenderTargetUniqueID.makeInvalid();
    }
    int progIdx = TextureToCopyProgramIdx(srcTex);
    if (!fCopyPrograms[progIdx].fProgram) {
        if (!this->createCopyProgram(srcTex)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }
    this->flushViewport(SkIRect::MakeSize(dst->dimensions()),
                        dst->height(),
                        kTopLeft_GrSurfaceOrigin); // the origin is irrelevant in this case
    int w = srcRect.width();
    int h = srcRect.height();
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
    if (srcTex->textureType() != GrTextureType::kRectangle) {
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
    this->flushConservativeRasterState(false);
    this->flushWireframeState(false);
    this->flushScissorTest(GrScissorTest::kDisabled);
    this->disableWindowRectangles();
    this->disableStencil();
    if (this->glCaps().srgbWriteControl()) {
        this->flushFramebufferSRGB(true);
    }
    GL_CALL(DrawArrays(GR_GL_TRIANGLE_STRIP, 0, 4));
    this->unbindSurfaceFBOForPixelOps(dst, 0, GR_GL_FRAMEBUFFER);
    // The rect is already in device space so we pass in kTopLeft so no flip is done.
    this->didWriteToSurface(dst, kTopLeft_GrSurfaceOrigin, &dstRect);
    return true;
}

void GrGLGpu::copySurfaceAsCopyTexSubImage(GrSurface* dst, GrSurface* src, const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    SkASSERT(can_copy_texsubimage(dst, src, this->glCaps()));
    this->bindSurfaceFBOForPixelOps(src, 0, GR_GL_FRAMEBUFFER, kSrc_TempFBOTarget);
    GrGLTexture* dstTex = static_cast<GrGLTexture *>(dst->asTexture());
    SkASSERT(dstTex);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();

    this->bindTextureToScratchUnit(dstTex->target(), dstTex->textureID());
    GL_CALL(CopyTexSubImage2D(dstTex->target(), 0,
                              dstPoint.fX, dstPoint.fY,
                              srcRect.fLeft, srcRect.fTop,
                              srcRect.width(), srcRect.height()));
    this->unbindSurfaceFBOForPixelOps(src, 0, GR_GL_FRAMEBUFFER);
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
        if (SkIRect::Intersects(dstRect, srcRect)) {
            return false;
        }
    }

    this->bindSurfaceFBOForPixelOps(dst, 0, GR_GL_DRAW_FRAMEBUFFER, kDst_TempFBOTarget);
    this->bindSurfaceFBOForPixelOps(src, 0, GR_GL_READ_FRAMEBUFFER, kSrc_TempFBOTarget);
    // We modified the bound FBO
    fHWBoundRenderTargetUniqueID.makeInvalid();

    // BlitFrameBuffer respects the scissor, so disable it.
    this->flushScissorTest(GrScissorTest::kDisabled);
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
    this->unbindSurfaceFBOForPixelOps(dst, 0, GR_GL_DRAW_FRAMEBUFFER);
    this->unbindSurfaceFBOForPixelOps(src, 0, GR_GL_READ_FRAMEBUFFER);

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
    int levelCount = SkMipmap::ComputeLevelCount(width, height) + 1;
    SkASSERT(levelCount == texture->maxMipmapLevel() + 1);

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
    this->bindTexture(0, GrSamplerState::Filter::kLinear, GrSwizzle::RGBA(), glTex);

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
    this->flushScissorTest(GrScissorTest::kDisabled);
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
        SkASSERT(this->glCaps().mipmapLevelControlSupport());
        GL_CALL(TexParameteri(GR_GL_TEXTURE_2D, GR_GL_TEXTURE_BASE_LEVEL, level - 1));

        GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0, GR_GL_TEXTURE_2D,
                                     glTex->textureID(), level));

        width = std::max(1, width / 2);
        height = std::max(1, height / 2);
        this->flushViewport(SkIRect::MakeWH(width, height), height, kTopLeft_GrSurfaceOrigin);

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

void GrGLGpu::xferBarrier(GrRenderTarget* rt, GrXferBarrierType type) {
    SkASSERT(type);
    switch (type) {
        case kTexture_GrXferBarrierType: {
            GrGLRenderTarget* glrt = static_cast<GrGLRenderTarget*>(rt);
            SkASSERT(glrt->asTexture());
            SkASSERT(!glrt->isFBO0(false/*multisample*/));
            if (glrt->requiresManualMSAAResolve()) {
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

void GrGLGpu::insertManualFramebufferBarrier() {
    SkASSERT(this->caps()->requiresManualFBBarrierAfterTessellatedStencilDraw());
    GL_CALL(MemoryBarrier(GR_GL_FRAMEBUFFER_BARRIER_BIT));
}

GrBackendTexture GrGLGpu::onCreateBackendTexture(SkISize dimensions,
                                                 const GrBackendFormat& format,
                                                 GrRenderable renderable,
                                                 GrMipmapped mipMapped,
                                                 GrProtected isProtected) {
    // We don't support protected textures in GL.
    if (isProtected == GrProtected::kYes) {
        return {};
    }

    this->handleDirtyContext();

    GrGLFormat glFormat = format.asGLFormat();
    if (glFormat == GrGLFormat::kUnknown) {
        return {};
    }

    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    // Compressed formats go through onCreateCompressedBackendTexture
    SkASSERT(!GrGLFormatIsCompressed(glFormat));

    GrGLTextureInfo info;
    GrGLTextureParameters::SamplerOverriddenState initialState;

    if (glFormat == GrGLFormat::kUnknown) {
        return {};
    }
    switch (format.textureType()) {
        case GrTextureType::kNone:
        case GrTextureType::kExternal:
            return {};
        case GrTextureType::k2D:
            info.fTarget = GR_GL_TEXTURE_2D;
            break;
        case GrTextureType::kRectangle:
            if (!this->glCaps().rectangleTextureSupport() || mipMapped == GrMipmapped::kYes) {
                return {};
            }
            info.fTarget = GR_GL_TEXTURE_RECTANGLE;
            break;
    }
    info.fFormat = GrGLFormatToEnum(glFormat);
    info.fID = this->createTexture(dimensions, glFormat, info.fTarget, renderable, &initialState,
                                   numMipLevels);
    if (!info.fID) {
        return {};
    }

    // Unbind this texture from the scratch texture unit.
    this->bindTextureToScratchUnit(info.fTarget, 0);

    auto parameters = sk_make_sp<GrGLTextureParameters>();
    // The non-sampler params are still at their default values.
    parameters->set(&initialState, GrGLTextureParameters::NonsamplerState(),
                    fResetTimestampForTextureParameters);

    return GrBackendTexture(dimensions.width(), dimensions.height(), mipMapped, info,
                            std::move(parameters));
}

bool GrGLGpu::onClearBackendTexture(const GrBackendTexture& backendTexture,
                                    sk_sp<GrRefCntedCallback> finishedCallback,
                                    std::array<float, 4> color) {
    this->handleDirtyContext();

    GrGLTextureInfo info;
    SkAssertResult(backendTexture.getGLTextureInfo(&info));

    int numMipLevels = 1;
    if (backendTexture.hasMipmaps()) {
        numMipLevels =
                SkMipmap::ComputeLevelCount(backendTexture.width(), backendTexture.height()) + 1;
    }

    GrGLFormat glFormat = GrGLFormatFromGLEnum(info.fFormat);

    this->bindTextureToScratchUnit(info.fTarget, info.fID);

    // If we have mips make sure the base level is set to 0 and the max level set to numMipLevels-1
    // so that the uploads go to the right levels.
    if (numMipLevels && this->glCaps().mipmapLevelControlSupport()) {
        auto params = backendTexture.getGLTextureParams();
        GrGLTextureParameters::NonsamplerState nonsamplerState = params->nonsamplerState();
        if (params->nonsamplerState().fBaseMipMapLevel != 0) {
            GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_BASE_LEVEL, 0));
            nonsamplerState.fBaseMipMapLevel = 0;
        }
        if (params->nonsamplerState().fMaxMipmapLevel != (numMipLevels - 1)) {
            GL_CALL(TexParameteri(info.fTarget, GR_GL_TEXTURE_MAX_LEVEL, numMipLevels - 1));
            nonsamplerState.fBaseMipMapLevel = numMipLevels - 1;
        }
        params->set(nullptr, nonsamplerState, fResetTimestampForTextureParameters);
    }

    uint32_t levelMask = (1 << numMipLevels) - 1;
    bool result = this->uploadColorToTex(glFormat,
                                         backendTexture.dimensions(),
                                         info.fTarget,
                                         color,
                                         levelMask);

    // Unbind this texture from the scratch texture unit.
    this->bindTextureToScratchUnit(info.fTarget, 0);
    return result;
}

void GrGLGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kOpenGL == tex.backend());

    GrGLTextureInfo info;
    if (tex.getGLTextureInfo(&info)) {
        GL_CALL(DeleteTextures(1, &info.fID));
    }
}

bool GrGLGpu::compile(const GrProgramDesc& desc, const GrProgramInfo& programInfo) {
    GrThreadSafePipelineBuilder::Stats::ProgramCacheResult stat;

    sk_sp<GrGLProgram> tmp = fProgramCache->findOrCreateProgram(this->getContext(),
                                                                desc, programInfo, &stat);
    if (!tmp) {
        return false;
    }

    return stat != GrThreadSafePipelineBuilder::Stats::ProgramCacheResult::kHit;
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

GrBackendRenderTarget GrGLGpu::createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                                    GrColorType colorType,
                                                                    int sampleCnt,
                                                                    GrProtected isProtected) {
    if (dimensions.width()  > this->caps()->maxRenderTargetSize() ||
        dimensions.height() > this->caps()->maxRenderTargetSize()) {
        return {};
    }
    if (isProtected == GrProtected::kYes) {
        return {};
    }

    this->handleDirtyContext();
    auto format = this->glCaps().getFormatFromColorType(colorType);
    sampleCnt = this->glCaps().getRenderTargetSampleCount(sampleCnt, format);
    if (!sampleCnt) {
        return {};
    }
    // We make a texture instead of a render target if we're using a
    // "multisampled_render_to_texture" style extension or have a BGRA format that
    // is allowed for textures but not render buffer internal formats.
    bool useTexture = false;
    if (sampleCnt > 1 && !this->glCaps().usesMSAARenderBuffers()) {
        useTexture = true;
    } else if (format == GrGLFormat::kBGRA8 &&
        this->glCaps().getRenderbufferInternalFormat(GrGLFormat::kBGRA8) != GR_GL_BGRA8) {
        // We have a BGRA extension that doesn't support BGRA render buffers. We can use a texture
        // unless we've been asked for MSAA. Note we already checked above for render-to-
        // multisampled-texture style extensions.
        if (sampleCnt > 1) {
            return {};
        }
        useTexture = true;
    }
    int sFormatIdx = this->getCompatibleStencilIndex(format);
    if (sFormatIdx < 0) {
        return {};
    }
    GrGLuint colorID = 0;
    GrGLuint stencilID = 0;
    GrGLFramebufferInfo info;
    info.fFBOID = 0;
    info.fFormat = GrGLFormatToEnum(format);

    auto deleteIDs = [&](bool saveFBO = false) {
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
        if (!saveFBO && info.fFBOID) {
            this->deleteFramebuffer(info.fFBOID);
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

    GL_CALL(GenFramebuffers(1, &info.fFBOID));
    if (!info.fFBOID) {
        deleteIDs();
        return {};
    }

    this->invalidateBoundRenderTarget();

    this->bindFramebuffer(GR_GL_FRAMEBUFFER, info.fFBOID);
    if (useTexture) {
        GrGLTextureParameters::SamplerOverriddenState initialState;
        colorID = this->createTexture(dimensions, format, GR_GL_TEXTURE_2D, GrRenderable::kYes,
                                      &initialState, 1);
        if (!colorID) {
            deleteIDs();
            return {};
        }
        if (sampleCnt == 1) {
            GL_CALL(FramebufferTexture2D(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                         GR_GL_TEXTURE_2D, colorID, 0));
        } else {
            GL_CALL(FramebufferTexture2DMultisample(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                                    GR_GL_TEXTURE_2D, colorID, 0, sampleCnt));
        }
    } else {
        GrGLenum renderBufferFormat = this->glCaps().getRenderbufferInternalFormat(format);
        GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, colorID));
        if (sampleCnt == 1) {
            GL_CALL(RenderbufferStorage(GR_GL_RENDERBUFFER, renderBufferFormat, dimensions.width(),
                                        dimensions.height()));
        } else {
            if (!this->renderbufferStorageMSAA(this->glContext(), sampleCnt, renderBufferFormat,
                                               dimensions.width(), dimensions.height())) {
                deleteIDs();
                return {};
            }
        }
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_COLOR_ATTACHMENT0,
                                        GR_GL_RENDERBUFFER, colorID));
    }
    GL_CALL(BindRenderbuffer(GR_GL_RENDERBUFFER, stencilID));
    auto stencilBufferFormat = this->glCaps().stencilFormats()[sFormatIdx];
    if (sampleCnt == 1) {
        GL_CALL(RenderbufferStorage(GR_GL_RENDERBUFFER, GrGLFormatToEnum(stencilBufferFormat),
                                    dimensions.width(), dimensions.height()));
    } else {
        if (!this->renderbufferStorageMSAA(this->glContext(), sampleCnt,
                                           GrGLFormatToEnum(stencilBufferFormat),
                                           dimensions.width(), dimensions.height())) {
            deleteIDs();
            return {};
        }
    }
    GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_STENCIL_ATTACHMENT, GR_GL_RENDERBUFFER,
                                    stencilID));
    if (GrGLFormatIsPackedDepthStencil(this->glCaps().stencilFormats()[sFormatIdx])) {
        GL_CALL(FramebufferRenderbuffer(GR_GL_FRAMEBUFFER, GR_GL_DEPTH_ATTACHMENT,
                                        GR_GL_RENDERBUFFER, stencilID));
    }

    // We don't want to have to recover the renderbuffer/texture IDs later to delete them. OpenGL
    // has this rule that if a renderbuffer/texture is deleted and a FBO other than the current FBO
    // has the RB attached then deletion is delayed. So we unbind the FBO here and delete the
    // renderbuffers/texture.
    this->bindFramebuffer(GR_GL_FRAMEBUFFER, 0);
    deleteIDs(/* saveFBO = */ true);

    this->bindFramebuffer(GR_GL_FRAMEBUFFER, info.fFBOID);
    GrGLenum status;
    GL_CALL_RET(status, CheckFramebufferStatus(GR_GL_FRAMEBUFFER));
    if (GR_GL_FRAMEBUFFER_COMPLETE != status) {
        this->deleteFramebuffer(info.fFBOID);
        return {};
    }

    auto stencilBits = SkToInt(GrGLFormatStencilBits(this->glCaps().stencilFormats()[sFormatIdx]));

    GrBackendRenderTarget beRT = GrBackendRenderTarget(dimensions.width(), dimensions.height(),
                                                       sampleCnt, stencilBits, info);
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
#endif

///////////////////////////////////////////////////////////////////////////////

GrGLAttribArrayState* GrGLGpu::HWVertexArrayState::bindInternalVertexArray(GrGLGpu* gpu,
                                                                           const GrBuffer* ibuf) {
    SkASSERT(!ibuf || ibuf->isCpuBuffer() || !static_cast<const GrGpuBuffer*>(ibuf)->isMapped());
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

void GrGLGpu::addFinishedProc(GrGpuFinishedProc finishedProc,
                              GrGpuFinishedContext finishedContext) {
    fFinishCallbacks.add(finishedProc, finishedContext);
}

void GrGLGpu::flush(FlushType flushType) {
    if (fNeedsGLFlush || flushType == FlushType::kForce) {
        GL_CALL(Flush());
        fNeedsGLFlush = false;
    }
}

bool GrGLGpu::onSubmitToGpu(bool syncCpu) {
    if (syncCpu || (!fFinishCallbacks.empty() && !this->caps()->fenceSyncSupport())) {
        this->finishOutstandingGpuWork();
        fFinishCallbacks.callAll(true);
    } else {
        this->flush();
        // See if any previously inserted finish procs are good to go.
        fFinishCallbacks.check();
    }
    if (!this->glCaps().skipErrorChecks()) {
        this->clearErrorsAndCheckForOOM();
    }
    return true;
}

void GrGLGpu::submit(GrOpsRenderPass* renderPass) {
    // The GrGLOpsRenderPass doesn't buffer ops so there is nothing to do here
    SkASSERT(fCachedOpsRenderPass.get() == renderPass);
    fCachedOpsRenderPass->reset();
}

GrFence SK_WARN_UNUSED_RESULT GrGLGpu::insertFence() {
    if (!this->caps()->fenceSyncSupport()) {
        return 0;
    }
    GrGLsync sync;
    if (this->glCaps().fenceType() == GrGLCaps::FenceType::kNVFence) {
        static_assert(sizeof(GrGLsync) >= sizeof(GrGLuint));
        GrGLuint fence = 0;
        GL_CALL(GenFences(1, &fence));
        GL_CALL(SetFence(fence, GR_GL_ALL_COMPLETED));
        sync = reinterpret_cast<GrGLsync>(static_cast<intptr_t>(fence));
    } else {
        GL_CALL_RET(sync, FenceSync(GR_GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    }
    this->setNeedsFlush();
    static_assert(sizeof(GrFence) >= sizeof(GrGLsync));
    return (GrFence)sync;
}

bool GrGLGpu::waitSync(GrGLsync sync, uint64_t timeout, bool flush) {
    if (this->glCaps().fenceType() == GrGLCaps::FenceType::kNVFence) {
        GrGLuint nvFence = static_cast<GrGLuint>(reinterpret_cast<intptr_t>(sync));
        if (!timeout) {
            if (flush) {
                this->flush(FlushType::kForce);
            }
            GrGLboolean result;
            GL_CALL_RET(result, TestFence(nvFence));
            return result == GR_GL_TRUE;
        }
        // Ignore non-zero timeouts. GL_NV_fence has no timeout functionality.
        // If this really becomes necessary we could poll TestFence().
        // FinishFence always flushes so no need to check flush param.
        GL_CALL(FinishFence(nvFence));
        return true;
    } else {
        GrGLbitfield flags = flush ? GR_GL_SYNC_FLUSH_COMMANDS_BIT : 0;
        GrGLenum result;
        GL_CALL_RET(result, ClientWaitSync(sync, flags, timeout));
        return (GR_GL_CONDITION_SATISFIED == result || GR_GL_ALREADY_SIGNALED == result);
    }
}

bool GrGLGpu::waitFence(GrFence fence) {
    if (!this->caps()->fenceSyncSupport()) {
        return true;
    }
    return this->waitSync(reinterpret_cast<GrGLsync>(fence), 0, false);
}

void GrGLGpu::deleteFence(GrFence fence) const {
    if (this->caps()->fenceSyncSupport()) {
        this->deleteSync(reinterpret_cast<GrGLsync>(fence));
    }
}

std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT GrGLGpu::makeSemaphore(bool isOwned) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrGLSemaphore::Make(this, isOwned);
}

std::unique_ptr<GrSemaphore> GrGLGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                           GrSemaphoreWrapType /* wrapType */,
                                                           GrWrapOwnership ownership) {
    SkASSERT(this->caps()->semaphoreSupport());
    return GrGLSemaphore::MakeWrapped(this, semaphore.glSync(), ownership);
}

void GrGLGpu::insertSemaphore(GrSemaphore* semaphore) {
    SkASSERT(semaphore);
    GrGLSemaphore* glSem = static_cast<GrGLSemaphore*>(semaphore);

    GrGLsync sync;
    GL_CALL_RET(sync, FenceSync(GR_GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    glSem->setSync(sync);
    this->setNeedsFlush();
}

void GrGLGpu::waitSemaphore(GrSemaphore* semaphore) {
    SkASSERT(semaphore);
    GrGLSemaphore* glSem = static_cast<GrGLSemaphore*>(semaphore);

    GL_CALL(WaitSync(glSem->sync(), 0, GR_GL_TIMEOUT_IGNORED));
}

void GrGLGpu::checkFinishProcs() {
    fFinishCallbacks.check();
}

void GrGLGpu::finishOutstandingGpuWork() {
    GL_CALL(Finish());
}

void GrGLGpu::clearErrorsAndCheckForOOM() {
    while (this->getErrorAndCheckForOOM() != GR_GL_NO_ERROR) {}
}

GrGLenum GrGLGpu::getErrorAndCheckForOOM() {
#if GR_GL_CHECK_ERROR
    if (this->glInterface()->checkAndResetOOMed()) {
        this->setOOMed();
    }
#endif
    GrGLenum error = this->fGLContext->glInterface()->fFunctions.fGetError();
    if (error == GR_GL_OUT_OF_MEMORY) {
        this->setOOMed();
    }
    return error;
}

void GrGLGpu::deleteSync(GrGLsync sync) const {
    if (this->glCaps().fenceType() == GrGLCaps::FenceType::kNVFence) {
        GrGLuint nvFence = SkToUInt(reinterpret_cast<intptr_t>(sync));
        GL_CALL(DeleteFences(1, &nvFence));
    } else {
        GL_CALL(DeleteSync(sync));
    }
}

std::unique_ptr<GrSemaphore> GrGLGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    // Set up a semaphore to be signaled once the data is ready, and flush GL
    std::unique_ptr<GrSemaphore> semaphore = this->makeSemaphore(true);
    SkASSERT(semaphore);
    this->insertSemaphore(semaphore.get());
    // We must call flush here to make sure the GrGLSync object gets created and sent to the gpu.
    this->flush(FlushType::kForce);

    return semaphore;
}

int GrGLGpu::TextureToCopyProgramIdx(GrTexture* texture) {
    switch (GrSLCombinedSamplerTypeForTextureType(texture->textureType())) {
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
