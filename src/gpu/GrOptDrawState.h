/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOptDrawState_DEFINED
#define GrOptDrawState_DEFINED

#include "GrColor.h"
#include "GrGpu.h"
#include "GrPendingFragmentStage.h"
#include "GrProgramDesc.h"
#include "GrStencil.h"
#include "GrTypesPriv.h"
#include "SkMatrix.h"
#include "SkRefCnt.h"

class GrDeviceCoordTexture;
class GrDrawState;

/**
 * Class that holds an optimized version of a GrDrawState. It is meant to be an immutable class,
 * and contains all data needed to set the state for a gpu draw.
 */
class GrOptDrawState : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrOptDrawState)

    typedef GrClipMaskManager::ScissorState ScissorState;

    GrOptDrawState(const GrDrawState& drawState, GrGpu*, const ScissorState&,
                   const GrDeviceCoordTexture* dstCopy, GrGpu::DrawType);

    bool operator== (const GrOptDrawState& that) const;
    bool operator!= (const GrOptDrawState& that) const { return !(*this == that); }

    ///////////////////////////////////////////////////////////////////////////
    /// @name Vertex Attributes
    ////

    enum {
        kMaxVertexAttribCnt = kLast_GrVertexAttribBinding + 4,
    };

    const GrVertexAttrib* getVertexAttribs() const { return fVAPtr; }
    int getVertexAttribCount() const { return fVACount; }

    size_t getVertexStride() const { return fVAStride; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Color
    ////

    GrColor getColor() const { return fColor; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Coverage
    ////

    uint8_t getCoverage() const { return fCoverage; }

    GrColor getCoverageColor() const {
        return GrColorPackRGBA(fCoverage, fCoverage, fCoverage, fCoverage);
    }

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
    int numTotalStages() const {
         return this->numFragmentStages() + (this->hasGeometryProcessor() ? 1 : 0);
    }

    bool hasGeometryProcessor() const { return SkToBool(fGeometryProcessor.get()); }
    const GrGeometryProcessor* getGeometryProcessor() const { return fGeometryProcessor.get(); }
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
    /// @name Blending
    ////

    GrBlendCoeff getSrcBlendCoeff() const { return fSrcBlend; }
    GrBlendCoeff getDstBlendCoeff() const { return fDstBlend; }

    /**
     * Retrieves the last value set by setBlendConstant()
     * @return the blending constant value
     */
    GrColor getBlendConstant() const { return fBlendConstant; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name View Matrix
    ////

    /**
     * Retrieves the current view matrix
     * @return the current view matrix.
     */
    const SkMatrix& getViewMatrix() const { return fViewMatrix; }

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

    const ScissorState& getScissorState() const { return fScissorState; }

    /// @}


    ///////////////////////////////////////////////////////////////////////////
    /// @name Boolean Queries
    ////

    bool isDitherState() const { return SkToBool(fFlags & kDither_Flag); }
    bool isHWAntialiasState() const { return SkToBool(fFlags & kHWAA_Flag); }
    bool isColorWriteDisabled() const { return SkToBool(fFlags & kDisableColorWrite_Flag); }
    bool mustSkip() const { return NULL == this->getRenderTarget(); }

    /// @}

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    GrDrawState::DrawFace getDrawFace() const { return fDrawFace; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    const GrDeviceCoordTexture* getDstCopy() const { return fDstCopy.texture() ? &fDstCopy : NULL; }

    const GrProgramDesc& programDesc() const { return fDesc; }

private:
    /**
     * Loops through all the color stage effects to check if the stage will ignore color input or
     * always output a constant color. In the ignore color input case we can ignore all previous
     * stages. In the constant color case, we can ignore all previous stages and
     * the current one and set the state color to the constant color.
     */
    void computeEffectiveColorStages(const GrDrawState& ds, GrProgramDesc::DescInfo*,
                                     int* firstColorStageIdx, uint8_t* fixFunctionVAToRemove);

    /**
     * Loops through all the coverage stage effects to check if the stage will ignore color input.
     * If a coverage stage will ignore input, then we can ignore all coverage stages before it. We
     * loop to determine the first effective coverage stage.
     */
    void computeEffectiveCoverageStages(const GrDrawState& ds, GrProgramDesc::DescInfo* descInfo,
                                        int* firstCoverageStageIdx);

    /**
     * This function takes in a flag and removes the corresponding fixed function vertex attributes.
     * The flags are in the same order as GrVertexAttribBinding array. If bit i of removeVAFlags is
     * set, then vertex attributes with binding (GrVertexAttribute)i will be removed.
     */
    void removeFixedFunctionVertexAttribs(uint8_t removeVAFlags, GrProgramDesc::DescInfo*);

    /**
     * Alter the program desc and inputs (attribs and processors) based on the blend optimization.
     */
    void adjustProgramForBlendOpt(const GrDrawState& ds, GrDrawState::BlendOpt,
                                  GrProgramDesc::DescInfo*,
                                  int* firstColorStageIdx, int* firstCoverageStageIdx,
                                  uint8_t* fixedFunctionVAToRemove);

    /**
     * Loop over the effect stages to determine various info like what data they will read and what
     * shaders they require.
     */
    void getStageStats(const GrDrawState& ds, int firstColorStageIdx, int firstCoverageStageIdx,
                       GrProgramDesc::DescInfo*);

    /**
     * Calculates the primary and secondary output types of the shader. For certain output types
     * the function may adjust the blend coefficients. After this function is called the src and dst
     * blend coeffs will represent those used by backend API.
     */
    void setOutputStateInfo(const GrDrawState& ds, GrDrawState::BlendOpt, const GrDrawTargetCaps&,
                            GrProgramDesc::DescInfo*);

    enum Flags {
        kDither_Flag            = 0x1,
        kHWAA_Flag              = 0x2,
        kDisableColorWrite_Flag = 0x4,
    };

    typedef GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> RenderTarget;
    typedef SkSTArray<8, GrPendingFragmentStage> FragmentStageArray;
    typedef GrPendingProgramElement<const GrGeometryProcessor> ProgramGeometryProcessor;
    RenderTarget                        fRenderTarget;
    ScissorState                        fScissorState;
    GrColor                             fColor;
    SkMatrix                            fViewMatrix;
    GrColor                             fBlendConstant;
    const GrVertexAttrib*               fVAPtr;
    int                                 fVACount;
    size_t                              fVAStride;
    GrStencilSettings                   fStencilSettings;
    uint8_t                             fCoverage;
    GrDrawState::DrawFace               fDrawFace;
    GrDeviceCoordTexture                fDstCopy;
    GrBlendCoeff                        fSrcBlend;
    GrBlendCoeff                        fDstBlend;
    uint32_t                            fFlags;
    ProgramGeometryProcessor            fGeometryProcessor;
    FragmentStageArray                  fFragmentStages;

    // This function is equivalent to the offset into fFragmentStages where coverage stages begin.
    int                                 fNumColorStages;

    SkAutoSTArray<4, GrVertexAttrib> fOptVA;

    GrProgramDesc fDesc;

    typedef SkRefCnt INHERITED;
};

#endif
