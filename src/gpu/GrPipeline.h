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
#include "GrPendingFragmentStage.h"
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
class GrPipeline {
public:
    SK_DECLARE_INST_COUNT(GrPipeline)

    // TODO get rid of this version of the constructor when we use batch everywhere
    GrPipeline(const GrPipelineBuilder& pipelineBuilder, const GrPrimitiveProcessor*,
               const GrDrawTargetCaps&, const GrScissorState&,
               const GrDeviceCoordTexture* dstCopy);

    GrPipeline(GrBatch*, const GrPipelineBuilder&, const GrDrawTargetCaps&,
               const GrScissorState&, const GrDeviceCoordTexture* dstCopy);

    /*
     * Returns true if it is possible to combine the two GrPipelines and it will update 'this'
     * to subsume 'that''s draw.
     */
    bool isEqual(const GrPipeline& that) const;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Effect Stages
    /// Each stage hosts a GrProcessor. The effect produces an output color or coverage in the
    /// fragment shader. Its inputs are the output from the previous stage as well as some variables
    /// available to it in the fragment and vertex shader (e.g. the vertex position, the dst color,
    /// the fragment position, local coordinates).
    ///
    /// The stages are divided into two sets, color-computing and coverage-computing. The final
    /// color stage produces the final pixel color. The coverage-computing stages function exactly
    /// as the color-computing but the output of the final coverage stage is treated as a fractional
    /// pixel coverage rather than as input to the src/dst color blend step.
    ///
    /// The input color to the first color-stage is either the constant color or interpolated
    /// per-vertex colors. The input to the first coverage stage is either a constant coverage
    /// (usually full-coverage) or interpolated per-vertex coverage.
    ////

    int numColorStages() const { return fNumColorStages; }
    int numCoverageStages() const { return fFragmentStages.count() - fNumColorStages; }
    int numFragmentStages() const { return fFragmentStages.count(); }

    const GrXferProcessor* getXferProcessor() const { return fXferProcessor.get(); }

    const GrPendingFragmentStage& getColorStage(int idx) const {
        SkASSERT(idx < this->numColorStages());
        return fFragmentStages[idx];
    }
    const GrPendingFragmentStage& getCoverageStage(int idx) const {
        SkASSERT(idx < this->numCoverageStages());
        return fFragmentStages[fNumColorStages + idx];
    }
    const GrPendingFragmentStage& getFragmentStage(int idx) const {
        return fFragmentStages[idx];
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Render Target
    ////

    /**
     * Retrieves the currently set render-target.
     *
     * @return    The currently set render target.
     */
    GrRenderTarget* getRenderTarget() const { return fRenderTarget.get(); }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Stencil
    ////

    const GrStencilSettings& getStencil() const { return fStencilSettings; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name ScissorState
    ////

    const GrScissorState& getScissorState() const { return fScissorState; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Boolean Queries
    ////

    bool isDitherState() const { return SkToBool(fFlags & kDither_Flag); }
    bool isHWAntialiasState() const { return SkToBool(fFlags & kHWAA_Flag); }
    bool mustSkip() const { return NULL == this->getRenderTarget(); }

    /// @}

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    GrPipelineBuilder::DrawFace getDrawFace() const { return fDrawFace; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    const GrDeviceCoordTexture* getDstCopy() const { return fDstCopy.texture() ? &fDstCopy : NULL; }

    const GrProgramDesc::DescInfo& descInfo() const { return fDescInfo; }

    const GrPipelineInfo& getInitBatchTracker() const { return fInitBT; }

private:
    // TODO we can have one constructor once GrBatch is complete
    void internalConstructor(const GrPipelineBuilder&,
                             const GrProcOptInfo& colorPOI,
                             const GrProcOptInfo& coveragePOI,
                             const GrDrawTargetCaps&,
                             const GrScissorState&,
                             const GrDeviceCoordTexture* dstCopy);

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
                            const GrDrawTargetCaps&);

    enum Flags {
        kDither_Flag            = 0x1,
        kHWAA_Flag              = 0x2,
    };

    typedef GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> RenderTarget;
    typedef SkSTArray<8, GrPendingFragmentStage> FragmentStageArray;
    typedef GrPendingProgramElement<const GrXferProcessor> ProgramXferProcessor;
    RenderTarget                        fRenderTarget;
    GrScissorState                      fScissorState;
    GrStencilSettings                   fStencilSettings;
    GrPipelineBuilder::DrawFace         fDrawFace;
    GrDeviceCoordTexture                fDstCopy;
    uint32_t                            fFlags;
    ProgramXferProcessor                fXferProcessor;
    FragmentStageArray                  fFragmentStages;
    GrProgramDesc::DescInfo             fDescInfo;
    GrPipelineInfo                      fInitBT;

    // This function is equivalent to the offset into fFragmentStages where coverage stages begin.
    int                                 fNumColorStages;

    GrProgramDesc fDesc;

    typedef SkRefCnt INHERITED;
};

#endif
