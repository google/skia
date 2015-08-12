/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipeline_DEFINED
#define GrPipeline_DEFINED

#include "GrColor.h"
#include "GrGpu.h"
#include "GrNonAtomicRef.h"
#include "GrPendingFragmentStage.h"
#include "GrPrimitiveProcessor.h"
#include "GrProgramDesc.h"
#include "GrStencil.h"
#include "GrTypesPriv.h"
#include "SkMatrix.h"
#include "SkRefCnt.h"

class GrBatch;
class GrDeviceCoordTexture;
class GrPipelineBuilder;

/**
 * Class that holds an optimized version of a GrPipelineBuilder. It is meant to be an immutable
 * class, and contains all data needed to set the state for a gpu draw.
 */
class GrPipeline : public GrNonAtomicRef {
public:
    ///////////////////////////////////////////////////////////////////////////
    /// @name Creation

    struct CreateArgs {
        const GrPipelineBuilder*    fPipelineBuilder;
        const GrCaps*               fCaps;
        GrProcOptInfo               fColorPOI;
        GrProcOptInfo               fCoveragePOI;
        const GrScissorState*       fScissor;
        GrXferProcessor::DstTexture fDstTexture;
    };

    /** Creates a pipeline into a pre-allocated buffer */
    static GrPipeline* CreateAt(void* memory, const CreateArgs&, GrPipelineOptimizations*);

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Comparisons

    /**
     * Returns true if these pipelines are equivalent.  Coord transforms may be applied either on
     * the GPU or the CPU. When we apply them on the CPU then the matrices need not agree in order
     * to combine draws. Therefore we take a param that indicates whether coord transforms should be
     * compared."
     */
    static bool AreEqual(const GrPipeline& a, const GrPipeline& b, bool ignoreCoordTransforms);

    /**
     * Allows a GrBatch subclass to determine whether two GrBatches can combine. This is a stricter
     * test than isEqual because it also considers blend barriers when the two batches' bounds
     * overlap
     */
    static bool CanCombine(const GrPipeline& a, const SkRect& aBounds,
                           const GrPipeline& b, const SkRect& bBounds,
                           const GrCaps& caps,
                           bool ignoreCoordTransforms = false)  {
        if (!AreEqual(a, b, ignoreCoordTransforms)) {
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


    int numColorFragmentStages() const { return fNumColorStages; }
    int numCoverageFragmentStages() const { return fFragmentStages.count() - fNumColorStages; }
    int numFragmentStages() const { return fFragmentStages.count(); }

    const GrXferProcessor* getXferProcessor() const { return fXferProcessor.get(); }

    const GrPendingFragmentStage& getColorStage(int idx) const {
        SkASSERT(idx < this->numColorFragmentStages());
        return fFragmentStages[idx];
    }
    const GrPendingFragmentStage& getCoverageStage(int idx) const {
        SkASSERT(idx < this->numCoverageFragmentStages());
        return fFragmentStages[fNumColorStages + idx];
    }
    const GrPendingFragmentStage& getFragmentStage(int idx) const {
        return fFragmentStages[idx];
    }

    /// @}

    /**
     * Retrieves the currently set render-target.
     *
     * @return    The currently set render target.
     */
    GrRenderTarget* getRenderTarget() const { return fRenderTarget.get(); }

    const GrStencilSettings& getStencil() const { return fStencilSettings; }

    const GrScissorState& getScissorState() const { return fScissorState; }

    bool isDitherState() const { return SkToBool(fFlags & kDither_Flag); }
    bool isHWAntialiasState() const { return SkToBool(fFlags & kHWAA_Flag); }
    bool snapVerticesToPixelCenters() const { return SkToBool(fFlags & kSnapVertices_Flag); }

    GrXferBarrierType xferBarrierType(const GrCaps& caps) const {
        return fXferProcessor->xferBarrierType(fRenderTarget.get(), caps);
    }

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    GrPipelineBuilder::DrawFace getDrawFace() const { return fDrawFace; }


    ///////////////////////////////////////////////////////////////////////////

    bool readsFragPosition() const { return fReadsFragPosition; }

    const SkTArray<const GrCoordTransform*, true>& coordTransforms() const {
        return fCoordTransforms;
    }

private:
    GrPipeline() { /** Initialized in factory function*/ }

    /**
     * Alter the program desc and inputs (attribs and processors) based on the blend optimization.
     */
    void adjustProgramFromOptimizations(const GrPipelineBuilder& ds,
                                        GrXferProcessor::OptFlags,
                                        const GrProcOptInfo& colorPOI,
                                        const GrProcOptInfo& coveragePOI,
                                        int* firstColorStageIdx,
                                        int* firstCoverageStageIdx);

    /**
     * Calculates the primary and secondary output types of the shader. For certain output types
     * the function may adjust the blend coefficients. After this function is called the src and dst
     * blend coeffs will represent those used by backend API.
     */
    void setOutputStateInfo(const GrPipelineBuilder& ds, GrXferProcessor::OptFlags,
                            const GrCaps&);

    enum Flags {
        kDither_Flag            = 0x1,
        kHWAA_Flag              = 0x2,
        kSnapVertices_Flag      = 0x4,
    };

    typedef GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> RenderTarget;
    typedef SkSTArray<8, GrPendingFragmentStage> FragmentStageArray;
    typedef GrPendingProgramElement<const GrXferProcessor> ProgramXferProcessor;
    RenderTarget                        fRenderTarget;
    GrScissorState                      fScissorState;
    GrStencilSettings                   fStencilSettings;
    GrPipelineBuilder::DrawFace         fDrawFace;
    uint32_t                            fFlags;
    ProgramXferProcessor                fXferProcessor;
    FragmentStageArray                  fFragmentStages;
    bool                                fReadsFragPosition;

    // This function is equivalent to the offset into fFragmentStages where coverage stages begin.
    int                                 fNumColorStages;

    SkSTArray<8, const GrCoordTransform*, true> fCoordTransforms;
    GrProgramDesc                       fDesc;

    typedef SkRefCnt INHERITED;
};

#endif
