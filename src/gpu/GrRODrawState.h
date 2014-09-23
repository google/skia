/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRODrawState_DEFINED
#define GrRODrawState_DEFINED

#include "GrProcessorStage.h"
#include "GrRenderTarget.h"
#include "GrStencil.h"
#include "SkMatrix.h"

class GrDrawState;
class GrDrawTargetCaps;
class GrPaint;
class GrTexture;

/**
 * Read-only base class for GrDrawState. This class contains all the necessary data to represent a
 * canonical DrawState. All methods in the class are const, thus once created the data in the class
 * cannot be changed.
 */
class GrRODrawState : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrRODrawState)

    GrRODrawState() {}

    GrRODrawState& operator= (const GrRODrawState& that);

    ///////////////////////////////////////////////////////////////////////////
    /// @name Vertex Attributes
    ////

    enum {
        kMaxVertexAttribCnt = kLast_GrVertexAttribBinding + 4,
    };

    const GrVertexAttrib* getVertexAttribs() const { return fVAPtr; }
    int getVertexAttribCount() const { return fVACount; }

    size_t getVertexStride() const { return fVAStride; }

    /**
     * Getters for index into getVertexAttribs() for particular bindings. -1 is returned if the
     * binding does not appear in the current attribs. These bindings should appear only once in
     * the attrib array.
     */

    int positionAttributeIndex() const {
        return fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding];
    }
    int localCoordAttributeIndex() const {
        return fFixedFunctionVertexAttribIndices[kLocalCoord_GrVertexAttribBinding];
    }
    int colorVertexAttributeIndex() const {
        return fFixedFunctionVertexAttribIndices[kColor_GrVertexAttribBinding];
    }
    int coverageVertexAttributeIndex() const {
        return fFixedFunctionVertexAttribIndices[kCoverage_GrVertexAttribBinding];
    }

    bool hasLocalCoordAttribute() const {
        return -1 != fFixedFunctionVertexAttribIndices[kLocalCoord_GrVertexAttribBinding];
    }
    bool hasColorVertexAttribute() const {
        return -1 != fFixedFunctionVertexAttribIndices[kColor_GrVertexAttribBinding];
    }
    bool hasCoverageVertexAttribute() const {
        return -1 != fFixedFunctionVertexAttribIndices[kCoverage_GrVertexAttribBinding];
    }

    const int* getFixedFunctionVertexAttribIndices() const {
        return fFixedFunctionVertexAttribIndices;
    }

    bool validateVertexAttribs() const;

    /// @}

    /**
     * Determines whether the output coverage is guaranteed to be one for all pixels hit by a draw.
     */
    bool hasSolidCoverage() const;

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

    int numColorStages() const { return fColorStages.count(); }
    int numCoverageStages() const { return fCoverageStages.count(); }
    int numTotalStages() const {
         return this->numColorStages() + this->numCoverageStages() +
                 (this->hasGeometryProcessor() ? 1 : 0);
    }

    bool hasGeometryProcessor() const { return SkToBool(fGeometryProcessor.get()); }
    const GrGeometryStage* getGeometryProcessor() const { return fGeometryProcessor.get(); }
    const GrFragmentStage& getColorStage(int stageIdx) const { return fColorStages[stageIdx]; }
    const GrFragmentStage& getCoverageStage(int stageIdx) const { return fCoverageStages[stageIdx]; }

    /**
     * Checks whether any of the effects will read the dst pixel color.
     */
    bool willEffectReadDstColor() const;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Blending
    ////

    GrBlendCoeff getSrcBlendCoeff() const { return fSrcBlend; }
    GrBlendCoeff getDstBlendCoeff() const { return fDstBlend; }

    void getDstBlendCoeff(GrBlendCoeff* srcBlendCoeff,
                          GrBlendCoeff* dstBlendCoeff) const {
        *srcBlendCoeff = fSrcBlend;
        *dstBlendCoeff = fDstBlend;
    }

    /**
     * Retrieves the last value set by setBlendConstant()
     * @return the blending constant value
     */
    GrColor getBlendConstant() const { return fBlendConstant; }

    /**
     * Determines whether multiplying the computed per-pixel color by the pixel's fractional
     * coverage before the blend will give the correct final destination color. In general it
     * will not as coverage is applied after blending.
     */
    bool canTweakAlphaForCoverage() const;

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
        // TODO: determine whether we really need to leave matrix unmodified
        // at call sites when inversion fails.
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
    GrRenderTarget* getRenderTarget() const {
        return static_cast<GrRenderTarget*>(fRenderTarget.getResource());
    }

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

    uint32_t getFlagBits() const { return fFlagBits; }

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
    /// @name Hints
    /// Hints that when provided can enable optimizations.
    ////

    enum Hints { kVertexColorsAreOpaque_Hint = 0x1, };

    bool vertexColorsAreOpaque() const { return kVertexColorsAreOpaque_Hint & fHints; }

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

protected:
    /**
     * Converts refs on GrGpuResources owned directly or indirectly by this GrRODrawState into
     * pending reads and writes. This should be called when a GrDrawState is recorded into
     * a GrDrawTarget for later execution. Subclasses of GrRODrawState may add setters. However,
     * once this call has been made the GrRODrawState is immutable. It is also no longer copyable.
     * In the future this conversion will automatically happen when converting a GrDrawState into
     * an optimized draw state.
     */
    void convertToPendingExec();

    friend class GrDrawTarget;

    explicit GrRODrawState(const GrRODrawState& drawState);

    bool isEqual(const GrRODrawState& that) const;

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
     * Determines what optimizations can be applied based on the blend. The coefficients may have
     * to be tweaked in order for the optimization to work. srcCoeff and dstCoeff are optional
     * params that receive the tweaked coefficients. Normally the function looks at the current
     * state to see if coverage is enabled. By setting forceCoverage the caller can speculatively
     * determine the blend optimizations that would be used if there was partial pixel coverage.
     *
     * Subclasses of GrDrawTarget that actually draw (as opposed to those that just buffer for
     * playback) must call this function and respect the flags that replace the output color.
     *
     * If the cached BlendOptFlags does not have the invalidate bit set, then getBlendOpts will
     * simply returned the cached flags and coefficients. Otherwise it will calculate the values. 
     */
    BlendOptFlags getBlendOpts(bool forceCoverage = false,
                               GrBlendCoeff* srcCoeff = NULL,
                               GrBlendCoeff* dstCoeff = NULL) const;

    typedef GrTGpuResourceRef<GrRenderTarget> ProgramRenderTarget;
    // These fields are roughly sorted by decreasing likelihood of being different in op==
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

    typedef SkSTArray<4, GrFragmentStage>   FragmentStageArray;
    SkAutoTDelete<GrGeometryStage>          fGeometryProcessor;
    FragmentStageArray                      fColorStages;
    FragmentStageArray                      fCoverageStages;

    uint32_t                            fHints;

    // This is simply a different representation of info in fVertexAttribs and thus does
    // not need to be compared in op==.
    int fFixedFunctionVertexAttribIndices[kGrFixedFunctionVertexAttribBindingCnt];

private:
    /**
     * Determines whether src alpha is guaranteed to be one for all src pixels
     */
    bool srcAlphaWillBeOne() const;

    typedef SkRefCnt INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrRODrawState::BlendOptFlags);

#endif
