/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipeline_DEFINED
#define GrPipeline_DEFINED

#include "GrColor.h"
#include "GrFragmentProcessor.h"
#include "GrNonAtomicRef.h"
#include "GrPendingProgramElement.h"
#include "GrProcessorSet.h"
#include "GrProgramDesc.h"
#include "GrScissorState.h"
#include "GrUserStencilSettings.h"
#include "GrWindowRectsState.h"
#include "SkMatrix.h"
#include "SkRefCnt.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrDisableColorXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

class GrAppliedClip;
class GrDeviceCoordTexture;
class GrOp;
class GrPipelineBuilder;
class GrRenderTargetContext;

/**
 * Class that holds an optimized version of a GrPipelineBuilder. It is meant to be an immutable
 * class, and contains all data needed to set the state for a gpu draw.
 */
class GrPipeline : public GrNonAtomicRef<GrPipeline> {
public:
    ///////////////////////////////////////////////////////////////////////////
    /// @name Creation

    enum Flags {
        /**
         * Perform HW anti-aliasing. This means either HW FSAA, if supported by the render target,
         * or smooth-line rendering if a line primitive is drawn and line smoothing is supported by
         * the 3D API.
         */
        kHWAntialias_Flag = 0x1,

        /**
         * Modifies the vertex shader so that vertices will be positioned at pixel centers.
         */
        kSnapVerticesToPixelCenters_Flag = 0x2,
    };

    struct InitArgs {
        uint32_t fFlags = 0;
        GrDrawFace fDrawFace = GrDrawFace::kBoth;
        const GrProcessorSet* fProcessors = nullptr;  // Must be finalized
        const GrUserStencilSettings* fUserStencil = &GrUserStencilSettings::kUnused;
        const GrAppliedClip* fAppliedClip = nullptr;
        GrRenderTarget* fRenderTarget = nullptr;
        const GrCaps* fCaps = nullptr;
        GrXferProcessor::DstTexture fDstTexture;
    };

    /**
     * A Default constructed pipeline is unusable until init() is called.
     **/
    GrPipeline() = default;

    /**
     * Creates a simple pipeline with default settings and no processors. The provided blend mode
     * must be "Porter Duff" (<= kLastCoeffMode). This pipeline is initialized without requiring
     * a call to init().
     **/
    GrPipeline(GrRenderTarget*, SkBlendMode);

    /** (Re)initializes a pipeline. After initialization the pipeline can be used. */
    void init(const InitArgs&);

    /** True if the pipeline has been initialized. */
    bool isInitialized() const { return SkToBool(fRenderTarget.get()); }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Comparisons

    /**
     * Returns true if these pipelines are equivalent.  Coord transforms may be applied either on
     * the GPU or the CPU. When we apply them on the CPU then the matrices need not agree in order
     * to combine draws. Therefore we take a param that indicates whether coord transforms should be
     * compared."
     */
    static bool AreEqual(const GrPipeline& a, const GrPipeline& b);

    /**
     * Allows a GrOp subclass to determine whether two GrOp instances can combine. This is a
     * stricter test than isEqual because it also considers blend barriers when the two ops'
     * bounds overlap
     */
    static bool CanCombine(const GrPipeline& a, const SkRect& aBounds,
                           const GrPipeline& b, const SkRect& bBounds,
                           const GrCaps& caps)  {
        if (!AreEqual(a, b)) {
            return false;
        }
        if (a.xferBarrierType(caps)) {
            return aBounds.fRight <= bBounds.fLeft ||
                   aBounds.fBottom <= bBounds.fTop ||
                   bBounds.fRight <= aBounds.fLeft ||
                   bBounds.fBottom <= aBounds.fTop;
        }
        return true;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name GrFragmentProcessors

    // Make the renderTarget's GrOpList (if it exists) be dependent on any
    // GrOpLists in this pipeline
    void addDependenciesTo(GrRenderTarget* rt) const;

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
    GrTexture* dstTexture(SkIPoint* offset = nullptr) const {
        if (offset) {
            *offset = fDstTextureOffset;
        }
        return fDstTexture.get();
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

    /**
     * Retrieves the currently set render-target.
     *
     * @return    The currently set render target.
     */
    GrRenderTarget* getRenderTarget() const { return fRenderTarget.get(); }

    const GrUserStencilSettings* getUserStencil() const { return fUserStencilSettings; }

    const GrScissorState& getScissorState() const { return fScissorState; }

    const GrWindowRectsState& getWindowRectsState() const { return fWindowRectsState; }

    bool isHWAntialiasState() const { return SkToBool(fFlags & kHWAntialias_Flag); }
    bool snapVerticesToPixelCenters() const {
        return SkToBool(fFlags & kSnapVerticesToPixelCenters_Flag);
    }
    bool getDisableOutputConversionToSRGB() const {
        return SkToBool(fFlags & kDisableOutputConversionToSRGB_Flag);
    }
    bool getAllowSRGBInputs() const {
        return SkToBool(fFlags & kAllowSRGBInputs_Flag);
    }
    bool usesDistanceVectorField() const {
        return SkToBool(fFlags & kUsesDistanceVectorField_Flag);
    }
    bool hasStencilClip() const {
        return SkToBool(fFlags & kHasStencilClip_Flag);
    }
    bool isStencilEnabled() const {
        return SkToBool(fFlags & kStencilEnabled_Flag);
    }

    GrXferBarrierType xferBarrierType(const GrCaps& caps) const {
        if (fDstTexture.get() && fDstTexture.get() == fRenderTarget.get()->asTexture()) {
            return kTexture_GrXferBarrierType;
        }
        return this->getXferProcessor().xferBarrierType(caps);
    }

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    GrDrawFace getDrawFace() const { return static_cast<GrDrawFace>(fDrawFace); }

private:
    /** This is a continuation of the public "Flags" enum. */
    enum PrivateFlags {
        kDisableOutputConversionToSRGB_Flag = 0x4,
        kAllowSRGBInputs_Flag = 0x8,
        kUsesDistanceVectorField_Flag = 0x10,
        kHasStencilClip_Flag = 0x20,
        kStencilEnabled_Flag = 0x40,
    };

    using RenderTarget = GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>;
    using DstTexture = GrPendingIOResource<GrTexture, kRead_GrIOType>;
    using PendingFragmentProcessor = GrPendingProgramElement<const GrFragmentProcessor>;
    using FragmentProcessorArray = SkAutoSTArray<8, PendingFragmentProcessor>;

    DstTexture fDstTexture;
    SkIPoint fDstTextureOffset;
    RenderTarget fRenderTarget;
    GrScissorState fScissorState;
    GrWindowRectsState fWindowRectsState;
    const GrUserStencilSettings* fUserStencilSettings;
    uint16_t fDrawFace;
    uint16_t fFlags;
    sk_sp<const GrXferProcessor> fXferProcessor;
    FragmentProcessorArray fFragmentProcessors;

    // This value is also the index in fFragmentProcessors where coverage processors begin.
    int fNumColorProcessors;

    typedef SkRefCnt INHERITED;
};

#endif
