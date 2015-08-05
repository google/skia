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
    GrPipeline(const GrPipelineBuilder&,
               const GrProcOptInfo& colorPOI,
               const GrProcOptInfo& coveragePOI,
               const GrCaps&,
               const GrScissorState&,
               const GrXferProcessor::DstTexture*);

    /*
     * Returns true if these pipelines are equivalent.  Coord transforms may be applied either on
     * the GPU or the CPU. When we apply them on the CPU then the matrices need not agree in order
     * to combine draws. Therefore we take a param that indicates whether coord transforms should be
     * compared."
     */
    bool isEqual(const GrPipeline& that, bool ignoreCoordTransforms = false) const;

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
    // Skip any draws that refer to this pipeline (they should be a no-op).
    bool mustSkip() const { return NULL == this->getRenderTarget(); }

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    GrPipelineBuilder::DrawFace getDrawFace() const { return fDrawFace; }


    ///////////////////////////////////////////////////////////////////////////

    bool readsFragPosition() const { return fReadsFragPosition; }

    const GrPipelineInfo& infoForPrimitiveProcessor() const {
        return fInfoForPrimitiveProcessor;
    }

    const SkTArray<const GrCoordTransform*, true>& coordTransforms() const {
        return fCoordTransforms;
    }

private:
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
    GrPipelineInfo                      fInfoForPrimitiveProcessor;

    // This function is equivalent to the offset into fFragmentStages where coverage stages begin.
    int                                 fNumColorStages;

    SkSTArray<8, const GrCoordTransform*, true> fCoordTransforms;
    int                                 fNumCoordTransforms;
    GrProgramDesc                       fDesc;

    typedef SkRefCnt INHERITED;
};

#endif
