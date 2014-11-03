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
#include "GrProcessorStage.h"
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
    /**
     * Returns a snapshot of the current optimized state. If the current drawState has a valid
     * cached optimiezed state it will simply return a pointer to it otherwise it will create a new
     * GrOptDrawState. In all cases the GrOptDrawState is reffed and ownership is given to the
     * caller.
     */
    static GrOptDrawState* Create(const GrDrawState& drawState, GrGpu*,
                                  const GrDeviceCoordTexture* dstCopy, GrGpu::DrawType drawType);

    bool operator== (const GrOptDrawState& that) const;

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
    ///
    /// See the documentation of kCoverageDrawing_StateBit for information about disabling the
    /// the color / coverage distinction.
    ////

    int numColorStages() const { return fNumColorStages; }
    int numCoverageStages() const { return fFragmentStages.count() - fNumColorStages; }
    int numFragmentStages() const { return fFragmentStages.count(); }
    int numTotalStages() const {
         return this->numFragmentStages() + (this->hasGeometryProcessor() ? 1 : 0);
    }

    bool hasGeometryProcessor() const { return SkToBool(fGeometryProcessor.get()); }
    const GrGeometryProcessor* getGeometryProcessor() const { return fGeometryProcessor.get(); }
    const GrFragmentStage& getColorStage(int idx) const {
        SkASSERT(idx < this->numColorStages());
        return fFragmentStages[idx];
    }
    const GrFragmentStage& getCoverageStage(int idx) const {
        SkASSERT(idx < this->numCoverageStages());
        return fFragmentStages[fNumColorStages + idx];
    }
    const GrFragmentStage& getFragmentStage(int idx) const { return fFragmentStages[idx]; }

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

    /**
     *  Retrieves the inverse of the current view matrix.
     *
     *  If the current view matrix is invertible, return true, and if matrix
     *  is non-null, copy the inverse into it. If the current view matrix is
     *  non-invertible, return false and ignore the matrix parameter.
     *
     * @param matrix if not null, will receive a copy of the current inverse.
     */
    bool getViewInverse(SkMatrix* matrix) const {
        SkMatrix inverse;
        if (fViewMatrix.invert(&inverse)) {
            if (matrix) {
                *matrix = inverse;
            }
            return true;
        }
        return false;
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
    /// @name State Flags
    ////

    /**
     *  Flags that affect rendering. Controlled using enable/disableState(). All
     *  default to disabled.
     */
    enum StateBits {
        /**
         * Perform dithering. TODO: Re-evaluate whether we need this bit
         */
        kDither_StateBit        = 0x01,
        /**
         * Perform HW anti-aliasing. This means either HW FSAA, if supported by the render target,
         * or smooth-line rendering if a line primitive is drawn and line smoothing is supported by
         * the 3D API.
         */
        kHWAntialias_StateBit   = 0x02,
        /**
         * Draws will respect the clip, otherwise the clip is ignored.
         */
        kClip_StateBit          = 0x04,
        /**
         * Disables writing to the color buffer. Useful when performing stencil
         * operations.
         */
        kNoColorWrites_StateBit = 0x08,

        /**
         * Usually coverage is applied after color blending. The color is blended using the coeffs
         * specified by setBlendFunc(). The blended color is then combined with dst using coeffs
         * of src_coverage, 1-src_coverage. Sometimes we are explicitly drawing a coverage mask. In
         * this case there is no distinction between coverage and color and the caller needs direct
         * control over the blend coeffs. When set, there will be a single blend step controlled by
         * setBlendFunc() which will use coverage*color as the src color.
         */
         kCoverageDrawing_StateBit = 0x10,

        // Users of the class may add additional bits to the vector
        kDummyStateBit,
        kLastPublicStateBit = kDummyStateBit-1,
    };

    bool isStateFlagEnabled(uint32_t stateBit) const { return 0 != (stateBit & fFlagBits); }

    bool isDitherState() const { return 0 != (fFlagBits & kDither_StateBit); }
    bool isHWAntialiasState() const { return 0 != (fFlagBits & kHWAntialias_StateBit); }
    bool isClipState() const { return 0 != (fFlagBits & kClip_StateBit); }
    bool isColorWriteDisabled() const { return 0 != (fFlagBits & kNoColorWrites_StateBit); }
    bool isCoverageDrawing() const { return 0 != (fFlagBits & kCoverageDrawing_StateBit); }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Face Culling
    ////

    enum DrawFace {
        kInvalid_DrawFace = -1,

        kBoth_DrawFace,
        kCCW_DrawFace,
        kCW_DrawFace,
    };

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    DrawFace getDrawFace() const { return fDrawFace; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    /** Return type for CombineIfPossible. */
    enum CombinedState {
        /** The GrDrawStates cannot be combined. */
        kIncompatible_CombinedState,
        /** Either draw state can be used in place of the other. */
        kAOrB_CombinedState,
        /** Use the first draw state. */
        kA_CombinedState,
        /** Use the second draw state. */
        kB_CombinedState,
    };

    /// @}

    const GrProgramDesc& programDesc() const { return fDesc; }

private:
    /**
     * Optimizations for blending / coverage to that can be applied based on the current state.
     */
    enum BlendOptFlags {
        /**
         * No optimization
         */
        kNone_BlendOpt                  = 0,
        /**
         * Don't draw at all
         */
        kSkipDraw_BlendOptFlag          = 0x1,
        /**
         * The coverage value does not have to be computed separately from alpha, the the output
         * color can be the modulation of the two.
         */
        kCoverageAsAlpha_BlendOptFlag   = 0x2,
        /**
         * Instead of emitting a src color, emit coverage in the alpha channel and r,g,b are
         * "don't cares".
         */
        kEmitCoverage_BlendOptFlag      = 0x4,
        /**
         * Emit transparent black instead of the src color, no need to compute coverage.
         */
        kEmitTransBlack_BlendOptFlag    = 0x8,
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(BlendOptFlags);

    /**
     * Constructs and optimized drawState out of a GrRODrawState.
     */
    GrOptDrawState(const GrDrawState& drawState, BlendOptFlags blendOptFlags,
                   GrBlendCoeff optSrcCoeff, GrBlendCoeff optDstCoeff,
                   GrGpu*, const GrDeviceCoordTexture* dstCopy, GrGpu::DrawType);

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
     * Alter the OptDrawState (adjusting stages, vertex attribs, flags, etc.) based on the
     * BlendOptFlags.
     */
    void adjustFromBlendOpts(const GrDrawState& ds, GrProgramDesc::DescInfo*,
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
    void setOutputStateInfo(const GrDrawState& ds, const GrDrawTargetCaps&,
                            int firstCoverageStageIdx, GrProgramDesc::DescInfo*,
                            bool* separateCoverageFromColor);

    bool isEqual(const GrOptDrawState& that) const;

    // These fields are roughly sorted by decreasing likelihood of being different in op==
    typedef GrTGpuResourceRef<GrRenderTarget> ProgramRenderTarget;
    ProgramRenderTarget                 fRenderTarget;
    GrColor                             fColor;
    SkMatrix                            fViewMatrix;
    GrColor                             fBlendConstant;
    uint32_t                            fFlagBits;
    const GrVertexAttrib*               fVAPtr;
    int                                 fVACount;
    size_t                              fVAStride;
    GrStencilSettings                   fStencilSettings;
    uint8_t                             fCoverage;
    DrawFace                            fDrawFace;
    GrBlendCoeff                        fSrcBlend;
    GrBlendCoeff                        fDstBlend;

    typedef SkSTArray<8, GrFragmentStage> FragmentStageArray;
    typedef GrProgramElementRef<const GrGeometryProcessor> ProgramGeometryProcessor;
    ProgramGeometryProcessor            fGeometryProcessor;
    FragmentStageArray                  fFragmentStages;

    // This function is equivalent to the offset into fFragmentStages where coverage stages begin.
    int                                 fNumColorStages;

    SkAutoSTArray<4, GrVertexAttrib> fOptVA;

    BlendOptFlags   fBlendOptFlags;

    GrProgramDesc fDesc;

    typedef SkRefCnt INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrOptDrawState::BlendOptFlags);

#endif

