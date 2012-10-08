/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawState_DEFINED
#define GrDrawState_DEFINED

#include "GrColor.h"
#include "GrMatrix.h"
#include "GrNoncopyable.h"
#include "GrRefCnt.h"
#include "GrSamplerState.h"
#include "GrStencil.h"
#include "GrTexture.h"
#include "GrRenderTarget.h"
#include "effects/GrSingleTextureEffect.h"

#include "SkXfermode.h"

class GrPaint;

class GrDrawState : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawState)

    /**
     * Number of texture stages. Each stage takes as input a color and
     * 2D texture coordinates. The color input to the first enabled stage is the
     * per-vertex color or the constant color (setColor/setAlpha) if there are
     * no per-vertex colors. For subsequent stages the input color is the output
     * color from the previous enabled stage. The output color of each stage is
     * the input color modulated with the result of a texture lookup. Texture
     * lookups are specified by a texture a sampler (setSamplerState). Texture
     * coordinates for each stage come from the vertices based on a
     * GrVertexLayout bitfield. The output fragment color is the output color of
     * the last enabled stage. The presence or absence of texture coordinates
     * for each stage in the vertex layout indicates whether a stage is enabled
     * or not.
     *
     * Stages 0 through GrPaint::kTotalStages-1 are reserved for setting up
     * the draw (i.e., textures and filter masks). Stages GrPaint::kTotalStages
     * through kNumStages-1 are earmarked for use by GrTextContext and
     * GrPathRenderer-derived classes.
     */
    enum {
        kNumStages = 5,
        kMaxTexCoords = kNumStages
    };

    GrDrawState()
        : fRenderTarget(NULL) {

        this->reset();
    }

    GrDrawState(const GrDrawState& state)
        : fRenderTarget(NULL) {

        *this = state;
    }

    virtual ~GrDrawState() {
        this->disableStages();
        GrSafeSetNull(fRenderTarget);
    }

    /**
     * Resets to the default state.
     * Sampler states *will* be modified: textures or CustomStage objects
     * will be released.
     */
    void reset() {

        this->disableStages();

        fColor = 0xffffffff;
        fViewMatrix.reset();
        GrSafeSetNull(fRenderTarget);
        fSrcBlend = kOne_GrBlendCoeff;
        fDstBlend = kZero_GrBlendCoeff;
        fBlendConstant = 0x0;
        fFlagBits = 0x0;
        fVertexEdgeType = kHairLine_EdgeType;
        fStencilSettings.setDisabled();
        fFirstCoverageStage = kNumStages;
        fCoverage = 0xffffffff;
        fColorFilterMode = SkXfermode::kDst_Mode;
        fColorFilterColor = 0x0;
        fDrawFace = kBoth_DrawFace;
    }

    /**
     * Initializes the GrDrawState based on a GrPaint. Note that GrDrawState
     * encompases more than GrPaint. Aspects of GrDrawState that have no
     * GrPaint equivalents are not modified. GrPaint has fewer stages than
     * GrDrawState. The extra GrDrawState stages are disabled.
     */
    void setFromPaint(const GrPaint& paint);

    ///////////////////////////////////////////////////////////////////////////
    /// @name Color
    ////

    /**
     *  Sets color for next draw to a premultiplied-alpha color.
     *
     *  @param color    the color to set.
     */
    void setColor(GrColor color) { fColor = color; }

    GrColor getColor() const { return fColor; }

    /**
     *  Sets the color to be used for the next draw to be
     *  (r,g,b,a) = (alpha, alpha, alpha, alpha).
     *
     *  @param alpha The alpha value to set as the color.
     */
    void setAlpha(uint8_t a) {
        this->setColor((a << 24) | (a << 16) | (a << 8) | a);
    }

    /**
     * Add a color filter that can be represented by a color and a mode. Applied
     * after color-computing texture stages.
     */
    void setColorFilter(GrColor c, SkXfermode::Mode mode) {
        fColorFilterColor = c;
        fColorFilterMode = mode;
    }

    GrColor getColorFilterColor() const { return fColorFilterColor; }
    SkXfermode::Mode getColorFilterMode() const { return fColorFilterMode; }

    /**
     * Constructor sets the color to be 'color' which is undone by the destructor.
     */
    class AutoColorRestore : public ::GrNoncopyable {
    public:
        AutoColorRestore(GrDrawState* drawState, GrColor color) {
            fDrawState = drawState;
            fOldColor = fDrawState->getColor();
            fDrawState->setColor(color);
        }
        ~AutoColorRestore() {
            fDrawState->setColor(fOldColor);
        }
    private:
        GrDrawState*    fDrawState;
        GrColor         fOldColor;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Coverage
    ////

    /**
     * Sets a constant fractional coverage to be applied to the draw. The
     * initial value (after construction or reset()) is 0xff. The constant
     * coverage is ignored when per-vertex coverage is provided.
     */
    void setCoverage(uint8_t coverage) {
        fCoverage = GrColorPackRGBA(coverage, coverage, coverage, coverage);
    }

    /**
     * Version of above that specifies 4 channel per-vertex color. The value
     * should be premultiplied.
     */
    void setCoverage4(GrColor coverage) {
        fCoverage = coverage;
    }

    GrColor getCoverage() const {
        return fCoverage;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Textures
    ////

    /**
     * Creates a GrSingleTextureEffect.
     */
    void createTextureEffect(int stage, GrTexture* texture) {
        GrAssert(!this->getSampler(stage).getCustomStage());
        this->sampler(stage)->setCustomStage(
            SkNEW_ARGS(GrSingleTextureEffect, (texture)))->unref();
    }
    void createTextureEffect(int stage, GrTexture* texture, const GrTextureParams& params) {
        GrAssert(!this->getSampler(stage).getCustomStage());
        this->sampler(stage)->setCustomStage(
            SkNEW_ARGS(GrSingleTextureEffect, (texture, params)))->unref();
    }


    bool stagesDisabled() {
        for (int i = 0; i < kNumStages; ++i) {
            if (NULL != fSamplerStates[i].getCustomStage()) {
                return false;
            }
        }
        return true;
    }

    void disableStage(int index) {
        fSamplerStates[index].setCustomStage(NULL);
    }

    /**
     * Release all the textures and custom stages referred to by this
     * draw state.
     */
    void disableStages() {
        for (int i = 0; i < kNumStages; ++i) {
            this->disableStage(i);
        }
    }

    class AutoStageDisable : public ::GrNoncopyable {
    public:
        AutoStageDisable(GrDrawState* ds) : fDrawState(ds) {}
        ~AutoStageDisable() {
            if (NULL != fDrawState) {
                fDrawState->disableStages();
            }
        }
    private:
        GrDrawState* fDrawState;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Samplers
    ////

    /**
     * Returns the current sampler for a stage.
     */
    const GrSamplerState& getSampler(int stage) const {
        GrAssert((unsigned)stage < kNumStages);
        return fSamplerStates[stage];
    }

    /**
     * Writable pointer to a stage's sampler.
     */
    GrSamplerState* sampler(int stage) {
        GrAssert((unsigned)stage < kNumStages);
        return fSamplerStates + stage;
    }

    /**
     * Preconcats the matrix of all samplers of enabled stages with a matrix.
     */
    void preConcatSamplerMatrices(const GrMatrix& matrix) {
        for (int i = 0; i < kNumStages; ++i) {
            if (this->isStageEnabled(i)) {
                fSamplerStates[i].preConcatMatrix(matrix);
            }
        }
    }

    /**
     * Preconcats the matrix of all samplers in the mask with the inverse of a
     * matrix. If the matrix inverse cannot be computed (and there is at least
     * one enabled stage) then false is returned.
     */
    bool preConcatSamplerMatricesWithInverse(const GrMatrix& matrix) {
        GrMatrix inv;
        bool computed = false;
        for (int i = 0; i < kNumStages; ++i) {
            if (this->isStageEnabled(i)) {
                if (!computed && !matrix.invert(&inv)) {
                    return false;
                } else {
                    computed = true;
                }
                fSamplerStates[i].preConcatMatrix(inv);
            }
        }
        return true;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Coverage / Color Stages
    ////

    /**
     * A common pattern is to compute a color with the initial stages and then
     * modulate that color by a coverage value in later stage(s) (AA, mask-
     * filters, glyph mask, etc). Color-filters, xfermodes, etc should be
     * computed based on the pre-coverage-modulated color. The division of
     * stages between color-computing and coverage-computing is specified by
     * this method. Initially this is kNumStages (all stages
     * are color-computing).
     */
    void setFirstCoverageStage(int firstCoverageStage) {
        GrAssert((unsigned)firstCoverageStage <= kNumStages);
        fFirstCoverageStage = firstCoverageStage;
    }

    /**
     * Gets the index of the first coverage-computing stage.
     */
    int getFirstCoverageStage() const {
        return fFirstCoverageStage;
    }

    ///@}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Blending
    ////

    /**
     * Sets the blending function coeffecients.
     *
     * The blend function will be:
     *    D' = sat(S*srcCoef + D*dstCoef)
     *
     *   where D is the existing destination color, S is the incoming source
     *   color, and D' is the new destination color that will be written. sat()
     *   is the saturation function.
     *
     * @param srcCoef coeffecient applied to the src color.
     * @param dstCoef coeffecient applied to the dst color.
     */
    void setBlendFunc(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
        fSrcBlend = srcCoeff;
        fDstBlend = dstCoeff;
    #if GR_DEBUG
        switch (dstCoeff) {
        case kDC_GrBlendCoeff:
        case kIDC_GrBlendCoeff:
        case kDA_GrBlendCoeff:
        case kIDA_GrBlendCoeff:
            GrPrintf("Unexpected dst blend coeff. Won't work correctly with"
                     "coverage stages.\n");
            break;
        default:
            break;
        }
        switch (srcCoeff) {
        case kSC_GrBlendCoeff:
        case kISC_GrBlendCoeff:
        case kSA_GrBlendCoeff:
        case kISA_GrBlendCoeff:
            GrPrintf("Unexpected src blend coeff. Won't work correctly with"
                     "coverage stages.\n");
            break;
        default:
            break;
        }
    #endif
    }

    GrBlendCoeff getSrcBlendCoeff() const { return fSrcBlend; }
    GrBlendCoeff getDstBlendCoeff() const { return fDstBlend; }

    void getDstBlendCoeff(GrBlendCoeff* srcBlendCoeff,
                          GrBlendCoeff* dstBlendCoeff) const {
        *srcBlendCoeff = fSrcBlend;
        *dstBlendCoeff = fDstBlend;
    }

    /**
     * Sets the blending function constant referenced by the following blending
     * coeffecients:
     *      kConstC_GrBlendCoeff
     *      kIConstC_GrBlendCoeff
     *      kConstA_GrBlendCoeff
     *      kIConstA_GrBlendCoeff
     *
     * @param constant the constant to set
     */
    void setBlendConstant(GrColor constant) { fBlendConstant = constant; }

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
     * Sets the matrix applied to vertex positions.
     *
     * In the post-view-matrix space the rectangle [0,w]x[0,h]
     * fully covers the render target. (w and h are the width and height of the
     * the rendertarget.)
     */
    void setViewMatrix(const GrMatrix& m) { fViewMatrix = m; }

    /**
     * Gets a writable pointer to the view matrix.
     */
    GrMatrix* viewMatrix() { return &fViewMatrix; }

    /**
     *  Multiplies the current view matrix by a matrix
     *
     *  After this call V' = V*m where V is the old view matrix,
     *  m is the parameter to this function, and V' is the new view matrix.
     *  (We consider positions to be column vectors so position vector p is
     *  transformed by matrix X as p' = X*p.)
     *
     *  @param m the matrix used to modify the view matrix.
     */
    void preConcatViewMatrix(const GrMatrix& m) { fViewMatrix.preConcat(m); }

    /**
     *  Multiplies the current view matrix by a matrix
     *
     *  After this call V' = m*V where V is the old view matrix,
     *  m is the parameter to this function, and V' is the new view matrix.
     *  (We consider positions to be column vectors so position vector p is
     *  transformed by matrix X as p' = X*p.)
     *
     *  @param m the matrix used to modify the view matrix.
     */
    void postConcatViewMatrix(const GrMatrix& m) { fViewMatrix.postConcat(m); }

    /**
     * Retrieves the current view matrix
     * @return the current view matrix.
     */
    const GrMatrix& getViewMatrix() const { return fViewMatrix; }

    /**
     *  Retrieves the inverse of the current view matrix.
     *
     *  If the current view matrix is invertible, return true, and if matrix
     *  is non-null, copy the inverse into it. If the current view matrix is
     *  non-invertible, return false and ignore the matrix parameter.
     *
     * @param matrix if not null, will receive a copy of the current inverse.
     */
    bool getViewInverse(GrMatrix* matrix) const {
        // TODO: determine whether we really need to leave matrix unmodified
        // at call sites when inversion fails.
        GrMatrix inverse;
        if (fViewMatrix.invert(&inverse)) {
            if (matrix) {
                *matrix = inverse;
            }
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////

    /**
     * Preconcats the current view matrix and restores the previous view matrix in the destructor.
     * Stage matrices are automatically adjusted to compensate.
     */
    class AutoViewMatrixRestore : public ::GrNoncopyable {
    public:
        AutoViewMatrixRestore() : fDrawState(NULL) {}

        AutoViewMatrixRestore(GrDrawState* ds,
                              const GrMatrix& preconcatMatrix,
                              uint32_t explicitCoordStageMask = 0) {
            fDrawState = NULL;
            this->set(ds, preconcatMatrix, explicitCoordStageMask);
        }

        ~AutoViewMatrixRestore() { this->restore(); }

        /**
         * Can be called prior to destructor to restore the original matrix.
         */
        void restore();
        
        void set(GrDrawState* drawState,
                 const GrMatrix& preconcatMatrix,
                 uint32_t explicitCoordStageMask = 0);

        bool isSet() const { return NULL != fDrawState; }

    private:
        GrDrawState*       fDrawState;
        GrMatrix           fViewMatrix;
        GrMatrix           fSamplerMatrices[GrDrawState::kNumStages];
        uint32_t           fRestoreMask;
    };

    ////////////////////////////////////////////////////////////////////////////

    /**
     * This sets the view matrix to identity and adjusts stage matrices to compensate. The
     * destructor undoes the changes, restoring the view matrix that was set before the
     * constructor. It is similar to passing the inverse of the current view matrix to
     * AutoViewMatrixRestore, but lazily computes the inverse only if necessary.
     */
    class AutoDeviceCoordDraw : ::GrNoncopyable {
    public:
        AutoDeviceCoordDraw() : fDrawState(NULL) {}
        /**
         * If a stage's texture matrix is applied to explicit per-vertex coords, rather than to
         * positions, then we don't want to modify its matrix. The explicitCoordStageMask is used
         * to specify such stages.
         */
        AutoDeviceCoordDraw(GrDrawState* drawState,
                            uint32_t explicitCoordStageMask = 0) {
            fDrawState = NULL;
            this->set(drawState, explicitCoordStageMask);
        }

        ~AutoDeviceCoordDraw() { this->restore(); }

        bool set(GrDrawState* drawState, uint32_t explicitCoordStageMask = 0);

        /**
         * Returns true if this object was successfully initialized on to a GrDrawState. It may
         * return false because a non-default constructor or set() were never called or because
         * the view matrix was not invertible.
         */
        bool succeeded() const { return NULL != fDrawState; }

        /**
         * Returns the matrix that was set previously set on the drawState. This is only valid
         * if succeeded returns true.
         */
        const GrMatrix& getOriginalMatrix() const {
            GrAssert(this->succeeded());
            return fViewMatrix;
        }

        /**
         * Can be called prior to destructor to restore the original matrix.
         */
        void restore();

    private:
        GrDrawState*       fDrawState;
        GrMatrix           fViewMatrix;
        GrMatrix           fSamplerMatrices[GrDrawState::kNumStages];
        uint32_t           fRestoreMask;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Render Target
    ////

    /**
     * Sets the rendertarget used at the next drawing call
     *
     * @param target  The render target to set.
     */
    void setRenderTarget(GrRenderTarget* target) {
        GrSafeAssign(fRenderTarget, target);
    }

    /**
     * Retrieves the currently set rendertarget.
     *
     * @return    The currently set render target.
     */
    const GrRenderTarget* getRenderTarget() const { return fRenderTarget; }
    GrRenderTarget* getRenderTarget() { return fRenderTarget; }

    class AutoRenderTargetRestore : public ::GrNoncopyable {
    public:
        AutoRenderTargetRestore() : fDrawState(NULL), fSavedTarget(NULL) {}
        AutoRenderTargetRestore(GrDrawState* ds, GrRenderTarget* newTarget) {
            fDrawState = NULL;
            fSavedTarget = NULL;
            this->set(ds, newTarget);
        }
        ~AutoRenderTargetRestore() { this->restore(); }

        void restore() {
            if (NULL != fDrawState) {
                fDrawState->setRenderTarget(fSavedTarget);
                fDrawState = NULL;
            }
            GrSafeSetNull(fSavedTarget);
        }

        void set(GrDrawState* ds, GrRenderTarget* newTarget) {
            this->restore();

            if (NULL != ds) {
                GrAssert(NULL == fSavedTarget);
                fSavedTarget = ds->getRenderTarget();
                SkSafeRef(fSavedTarget);
                ds->setRenderTarget(newTarget);
                fDrawState = ds;
            }
        }
    private:
        GrDrawState* fDrawState;
        GrRenderTarget* fSavedTarget;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Stencil
    ////

    /**
     * Sets the stencil settings to use for the next draw.
     * Changing the clip has the side-effect of possibly zeroing
     * out the client settable stencil bits. So multipass algorithms
     * using stencil should not change the clip between passes.
     * @param settings  the stencil settings to use.
     */
    void setStencil(const GrStencilSettings& settings) {
        fStencilSettings = settings;
    }

    /**
     * Shortcut to disable stencil testing and ops.
     */
    void disableStencil() {
        fStencilSettings.setDisabled();
    }

    const GrStencilSettings& getStencil() const { return fStencilSettings; }

    GrStencilSettings* stencil() { return &fStencilSettings; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Color Matrix
    ////

    /**
     * Sets the color matrix to use for the next draw.
     * @param matrix  the 5x4 matrix to apply to the incoming color
     */
    void setColorMatrix(const float matrix[20]) {
        memcpy(fColorMatrix, matrix, sizeof(fColorMatrix));
    }

    const float* getColorMatrix() const { return fColorMatrix; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    // @name Edge AA
    // Edge equations can be specified to perform antialiasing. Because the
    // edges are specified as per-vertex data, vertices that are shared by
    // multiple edges must be split.
    //
    ////

    /**
     * When specifying edges as vertex data this enum specifies what type of
     * edges are in use. The edges are always 4 GrScalars in memory, even when
     * the edge type requires fewer than 4.
     *
     * TODO: Fix the fact that HairLine and Circle edge types use y-down coords.
     *       (either adjust in VS or use origin_upper_left in GLSL)
     */
    enum VertexEdgeType {
        /* 1-pixel wide line
           2D implicit line eq (a*x + b*y +c = 0). 4th component unused */
        kHairLine_EdgeType,
        /* Quadratic specified by u^2-v canonical coords (only 2
           components used). Coverage based on signed distance with negative
           being inside, positive outside. Edge specified in window space
           (y-down) */
        kQuad_EdgeType,
        /* Same as above but for hairline quadratics. Uses unsigned distance.
           Coverage is min(0, 1-distance). */
        kHairQuad_EdgeType,
        /* Circle specified as center_x, center_y, outer_radius, inner_radius
           all in window space (y-down). */
        kCircle_EdgeType,

        kVertexEdgeTypeCnt
    };

    /**
     * Determines the interpretation per-vertex edge data when the
     * kEdge_VertexLayoutBit is set (see GrDrawTarget). When per-vertex edges
     * are not specified the value of this setting has no effect.
     */
    void setVertexEdgeType(VertexEdgeType type) {
        GrAssert(type >=0 && type < kVertexEdgeTypeCnt);
        fVertexEdgeType = type;
    }

    VertexEdgeType getVertexEdgeType() const { return fVertexEdgeType; }

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
         * Perform HW anti-aliasing. This means either HW FSAA, if supported
         * by the render target, or smooth-line rendering if a line primitive
         * is drawn and line smoothing is supported by the 3D API.
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
         * Draws will apply the color matrix, otherwise the color matrix is
         * ignored.
         */
        kColorMatrix_StateBit   = 0x10,

        // Users of the class may add additional bits to the vector
        kDummyStateBit,
        kLastPublicStateBit = kDummyStateBit-1,
    };

    void resetStateFlags() {
        fFlagBits = 0;
    }

    /**
     * Enable render state settings.
     *
     * @param stateBits bitfield of StateBits specifing the states to enable
     */
    void enableState(uint32_t stateBits) {
        fFlagBits |= stateBits;
    }

    /**
     * Disable render state settings.
     *
     * @param stateBits bitfield of StateBits specifing the states to disable
     */
    void disableState(uint32_t stateBits) {
        fFlagBits &= ~(stateBits);
    }

    /**
     * Enable or disable stateBits based on a boolean.
     *
     * @param stateBits bitfield of StateBits to enable or disablt
     * @param enable    if true enable stateBits, otherwise disable
     */
    void setState(uint32_t stateBits, bool enable) {
        if (enable) {
            this->enableState(stateBits);
        } else {
            this->disableState(stateBits);
        }
    }

    bool isDitherState() const {
        return 0 != (fFlagBits & kDither_StateBit);
    }

    bool isHWAntialiasState() const {
        return 0 != (fFlagBits & kHWAntialias_StateBit);
    }

    bool isClipState() const {
        return 0 != (fFlagBits & kClip_StateBit);
    }

    bool isColorWriteDisabled() const {
        return 0 != (fFlagBits & kNoColorWrites_StateBit);
    }

    bool isStateFlagEnabled(uint32_t stateBit) const {
        return 0 != (stateBit & fFlagBits);
    }

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
     * Controls whether clockwise, counterclockwise, or both faces are drawn.
     * @param face  the face(s) to draw.
     */
    void setDrawFace(DrawFace face) {
        GrAssert(kInvalid_DrawFace != face);
        fDrawFace = face;
    }

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    DrawFace getDrawFace() const { return fDrawFace; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    bool isStageEnabled(int s) const {
        GrAssert((unsigned)s < kNumStages);
        return (NULL != fSamplerStates[s].getCustomStage());
    }

    // Most stages are usually not used, so conditionals here
    // reduce the expected number of bytes touched by 50%.
    bool operator ==(const GrDrawState& s) const {
        if (fColor != s.fColor ||
            !s.fViewMatrix.cheapEqualTo(fViewMatrix) ||
            fRenderTarget != s.fRenderTarget ||
            fSrcBlend != s.fSrcBlend ||
            fDstBlend != s.fDstBlend ||
            fBlendConstant != s.fBlendConstant ||
            fFlagBits != s.fFlagBits ||
            fVertexEdgeType != s.fVertexEdgeType ||
            fStencilSettings != s.fStencilSettings ||
            fFirstCoverageStage != s.fFirstCoverageStage ||
            fCoverage != s.fCoverage ||
            fColorFilterMode != s.fColorFilterMode ||
            fColorFilterColor != s.fColorFilterColor ||
            fDrawFace != s.fDrawFace) {
            return false;
        }

        for (int i = 0; i < kNumStages; i++) {
            bool enabled = this->isStageEnabled(i);
            if (enabled != s.isStageEnabled(i)) {
                return false;
            }
            if (enabled && this->fSamplerStates[i] != s.fSamplerStates[i]) {
                return false;
            }
        }
        if (kColorMatrix_StateBit & s.fFlagBits) {
            if (memcmp(fColorMatrix,
                        s.fColorMatrix,
                        sizeof(fColorMatrix))) {
                return false;
            }
        }

        return true;
    }
    bool operator !=(const GrDrawState& s) const { return !(*this == s); }

    // Most stages are usually not used, so conditionals here
    // reduce the expected number of bytes touched by 50%.
    GrDrawState& operator =(const GrDrawState& s) {
        fColor = s.fColor;
        fViewMatrix = s.fViewMatrix;
        SkRefCnt_SafeAssign(fRenderTarget, s.fRenderTarget);
        fSrcBlend = s.fSrcBlend;
        fDstBlend = s.fDstBlend;
        fBlendConstant = s.fBlendConstant;
        fFlagBits = s.fFlagBits;
        fVertexEdgeType = s.fVertexEdgeType;
        fStencilSettings = s.fStencilSettings;
        fFirstCoverageStage = s.fFirstCoverageStage;
        fCoverage = s.fCoverage;
        fColorFilterMode = s.fColorFilterMode;
        fColorFilterColor = s.fColorFilterColor;
        fDrawFace = s.fDrawFace;

        for (int i = 0; i < kNumStages; i++) {
            if (s.isStageEnabled(i)) {
                this->fSamplerStates[i] = s.fSamplerStates[i];
            }
        }

        if (kColorMatrix_StateBit & s.fFlagBits) {
            memcpy(this->fColorMatrix, s.fColorMatrix, sizeof(fColorMatrix));
        }

        return *this;
    }

private:

    // These fields are roughly sorted by decreasing liklihood of being different in op==
    GrColor             fColor;
    GrMatrix            fViewMatrix;
    GrRenderTarget*     fRenderTarget;
    GrBlendCoeff        fSrcBlend;
    GrBlendCoeff        fDstBlend;
    GrColor             fBlendConstant;
    uint32_t            fFlagBits;
    VertexEdgeType      fVertexEdgeType;
    GrStencilSettings   fStencilSettings;
    int                 fFirstCoverageStage;
    GrColor             fCoverage;
    SkXfermode::Mode    fColorFilterMode;
    GrColor             fColorFilterColor;
    DrawFace            fDrawFace;

    // This field must be last; it will not be copied or compared
    // if the corresponding fTexture[] is NULL.
    GrSamplerState      fSamplerStates[kNumStages];
    // only compared if the color matrix enable flag is set
    float               fColorMatrix[20];       // 5 x 4 matrix

    typedef GrRefCnt INHERITED;
};

#endif
