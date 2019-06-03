/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipeline_DEFINED
#define GrPipeline_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/private/GrColor.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrNonAtomicRef.h"
#include "src/gpu/GrPendingIOResource.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrScissorState.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/GrWindowRectsState.h"
#include "src/gpu/effects/GrCoverageSetOpXP.h"
#include "src/gpu/effects/GrDisableColorXP.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/effects/generated/GrSimpleTextureEffect.h"
#include "src/gpu/geometry/GrRect.h"

class GrAppliedClip;
class GrOp;
class GrRenderTargetContext;

/**
 * This immutable object contains information needed to set build a shader program and set API
 * state for a draw. It is used along with a GrPrimitiveProcessor and a source of geometric
 * data (GrMesh or GrPath) to draw.
 */
class GrPipeline {
public:
    ///////////////////////////////////////////////////////////////////////////
    /// @name Creation

    // Pipeline options that the caller may enable.
    // NOTE: This enum is extended later by GrPipeline::Flags.
    enum class InputFlags : uint8_t {
        kNone = 0,
        /**
         * Perform HW anti-aliasing. This means either HW FSAA, if supported by the render target,
         * or smooth-line rendering if a line primitive is drawn and line smoothing is supported by
         * the 3D API.
         */
        kHWAntialias = (1 << 0),
        /**
         * Modifies the vertex shader so that vertices will be positioned at pixel centers.
         */
        kSnapVerticesToPixelCenters = (1 << 1),  // This value must be last. (See kLastInputFlag.)
    };

    struct InitArgs {
        InputFlags fInputFlags = InputFlags::kNone;
        const GrUserStencilSettings* fUserStencil = &GrUserStencilSettings::kUnused;
        const GrCaps* fCaps = nullptr;
        GrResourceProvider* fResourceProvider = nullptr;
        GrXferProcessor::DstProxy fDstProxy;
    };

    /**
     * Some state can be changed between GrMeshes without changing GrPipelines. This is generally
     * less expensive then using multiple pipelines. Such state is called "dynamic state". It can
     * be specified in two ways:
     * 1) FixedDynamicState - use this to specify state that does not vary between GrMeshes.
     * 2) DynamicStateArrays - use this to specify per mesh values for dynamic state.
     **/
    struct FixedDynamicState {
        explicit FixedDynamicState(const SkIRect& scissorRect) : fScissorRect(scissorRect) {}
        FixedDynamicState() = default;
        SkIRect fScissorRect = SkIRect::EmptyIRect();
        // Must have GrPrimitiveProcessor::numTextureSamplers() entries. Can be null if no samplers
        // or textures are passed using DynamicStateArrays.
        GrTextureProxy** fPrimitiveProcessorTextures = nullptr;
    };

    /**
     * Any non-null array overrides the FixedDynamicState on a mesh-by-mesh basis. Arrays must
     * have one entry for each GrMesh.
     */
    struct DynamicStateArrays {
        const SkIRect* fScissorRects = nullptr;
        // Must have GrPrimitiveProcessor::numTextureSamplers() * num_meshes entries.
        // Can be null if no samplers or to use the same textures for all meshes via'
        // FixedDynamicState.
        GrTextureProxy** fPrimitiveProcessorTextures = nullptr;
    };

    /**
     * Creates a simple pipeline with default settings and no processors. The provided blend mode
     * must be "Porter Duff" (<= kLastCoeffMode). If using GrScissorTest::kEnabled, the caller must
     * specify a scissor rectangle through the DynamicState struct.
     **/
    GrPipeline(GrScissorTest, SkBlendMode, InputFlags = InputFlags::kNone,
               const GrUserStencilSettings* = &GrUserStencilSettings::kUnused);

    GrPipeline(const InitArgs&, GrProcessorSet&&, GrAppliedClip&&);

    GrPipeline(const GrPipeline&) = delete;
    GrPipeline& operator=(const GrPipeline&) = delete;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name GrFragmentProcessors

    // Make the renderTargetContext's GrOpList be dependent on any GrOpLists in this pipeline
    void addDependenciesTo(GrOpList* recipient, const GrCaps&) const;

    int numColorFragmentProcessors() const { return fNumColorProcessors; }
    int numCoverageFragmentProcessors() const {
        return fFragmentProcessors.count() - fNumColorProcessors;
    }
    int numFragmentProcessors() const { return fFragmentProcessors.count(); }

    const GrXferProcessor& getXferProcessor() const {
        if (fXferProcessor) {
            return *fXferProcessor.get();
        } else {
            // A null xp member means the common src-over case. GrXferProcessor's ref'ing
            // mechanism is not thread safe so we do not hold a ref on this global.
            return GrPorterDuffXPFactory::SimpleSrcOverXP();
        }
    }

    /**
     * If the GrXferProcessor uses a texture to access the dst color, then this returns that
     * texture and the offset to the dst contents within that texture.
     */
    GrTextureProxy* dstTextureProxy(SkIPoint* offset = nullptr) const {
        if (offset) {
            *offset = fDstTextureOffset;
        }
        return fDstTextureProxy.get();
    }

    GrTexture* peekDstTexture(SkIPoint* offset = nullptr) const {
        if (GrTextureProxy* dstProxy = this->dstTextureProxy(offset)) {
            return dstProxy->peekTexture();
        }

        return nullptr;
    }

    const GrFragmentProcessor& getColorFragmentProcessor(int idx) const {
        SkASSERT(idx < this->numColorFragmentProcessors());
        return *fFragmentProcessors[idx].get();
    }

    const GrFragmentProcessor& getCoverageFragmentProcessor(int idx) const {
        SkASSERT(idx < this->numCoverageFragmentProcessors());
        return *fFragmentProcessors[fNumColorProcessors + idx].get();
    }

    const GrFragmentProcessor& getFragmentProcessor(int idx) const {
        return *fFragmentProcessors[idx].get();
    }

    /// @}

    const GrUserStencilSettings* getUserStencil() const { return fUserStencilSettings; }

    bool isScissorEnabled() const {
        return SkToBool(fFlags & Flags::kScissorEnabled);
    }

    const GrWindowRectsState& getWindowRectsState() const { return fWindowRectsState; }

    bool isHWAntialiasState() const { return SkToBool(fFlags & InputFlags::kHWAntialias); }
    bool snapVerticesToPixelCenters() const {
        return SkToBool(fFlags & InputFlags::kSnapVerticesToPixelCenters);
    }
    bool hasStencilClip() const {
        return SkToBool(fFlags & Flags::kHasStencilClip);
    }
    bool isStencilEnabled() const {
        return SkToBool(fFlags & Flags::kStencilEnabled);
    }
    bool isBad() const { return SkToBool(fFlags & Flags::kIsBad); }

    GrXferBarrierType xferBarrierType(GrTexture*, const GrCaps&) const;

    // Used by Vulkan and Metal to cache their respective pipeline objects
    uint32_t getBlendInfoKey() const;

private:
    void markAsBad() { fFlags |= Flags::kIsBad; }

    static constexpr uint8_t kLastInputFlag = (uint8_t)InputFlags::kSnapVerticesToPixelCenters;

    /** This is a continuation of the public "InputFlags" enum. */
    enum class Flags : uint8_t {
        kHasStencilClip = (kLastInputFlag << 1),
        kStencilEnabled = (kLastInputFlag << 2),
        kScissorEnabled = (kLastInputFlag << 3),
        kIsBad = (kLastInputFlag << 4),
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    friend bool operator&(Flags, InputFlags);

    using DstTextureProxy = GrPendingIOResource<GrTextureProxy, kRead_GrIOType>;
    using FragmentProcessorArray = SkAutoSTArray<8, std::unique_ptr<const GrFragmentProcessor>>;

    DstTextureProxy fDstTextureProxy;
    SkIPoint fDstTextureOffset;
    GrWindowRectsState fWindowRectsState;
    const GrUserStencilSettings* fUserStencilSettings;
    Flags fFlags;
    sk_sp<const GrXferProcessor> fXferProcessor;
    FragmentProcessorArray fFragmentProcessors;

    // This value is also the index in fFragmentProcessors where coverage processors begin.
    int fNumColorProcessors;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrPipeline::InputFlags);
GR_MAKE_BITFIELD_CLASS_OPS(GrPipeline::Flags);

inline bool operator&(GrPipeline::Flags flags, GrPipeline::InputFlags inputFlag) {
    return (flags & (GrPipeline::Flags)inputFlag);
}

#endif
