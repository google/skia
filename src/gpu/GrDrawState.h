/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawState_DEFINED
#define GrDrawState_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrColor.h"
#include "GrEffectStage.h"
#include "GrPaint.h"
#include "GrRefCnt.h"
#include "GrRenderTarget.h"
#include "GrStencil.h"
#include "GrTemplates.h"
#include "GrTexture.h"
#include "GrTypesPriv.h"
#include "effects/GrSimpleTextureEffect.h"

#include "SkMatrix.h"
#include "SkXfermode.h"

class GrDrawState : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawState)

    /**
     * Total number of effect stages. Each stage can host a GrEffect. A stage is enabled if it has a
     * GrEffect. The effect produces an output color in the fragment shader. It's inputs are the
     * output from the previous enabled stage and a position. The position is either derived from
     * the interpolated vertex positions or explicit per-vertex coords, depending upon the
     * GrAttribBindings used to draw.
     *
     * The stages are divided into two sets, color-computing and coverage-computing. The final color
     * stage produces the final pixel color. The coverage-computing stages function exactly as the
     * color-computing but the output of the final coverage stage is treated as a fractional pixel
     * coverage rather than as input to the src/dst color blend step.
     *
     * The input color to the first enabled color-stage is either the constant color or interpolated
     * per-vertex colors. The input to the first coverage stage is either a constant coverage
     * (usually full-coverage) or interpolated per-vertex coverage.
     *
     * See the documentation of kCoverageDrawing_StateBit for information about disabling the
     * the color / coverage distinction.
     *
     * Stages 0 through GrPaint::kTotalStages-1 are reserved for stages copied from the client's
     * GrPaint. Stage GrPaint::kTotalStages is earmarked for use by GrTextContext, GrPathRenderer-
     * derived classes, and the rect/oval helper classes. GrPaint::kTotalStages+1 is earmarked for
     * clipping by GrClipMaskManager. TODO: replace fixed size array of stages with variable size
     * arrays of color and coverage stages.
     */
    enum {
        kNumStages = GrPaint::kTotalStages + 2,
    };

    GrDrawState() {
        this->reset();
    }

    GrDrawState(const GrDrawState& state) {
        *this = state;
    }

    virtual ~GrDrawState() {
        this->disableStages();
    }

    /**
     * Resets to the default state.
     * GrEffects will be removed from all stages.
     */
    void reset() {

        this->disableStages();

        fRenderTarget.reset(NULL);

        this->setDefaultVertexAttribs();

        fCommon.fColor = 0xffffffff;
        fCommon.fViewMatrix.reset();
        fCommon.fSrcBlend = kOne_GrBlendCoeff;
        fCommon.fDstBlend = kZero_GrBlendCoeff;
        fCommon.fBlendConstant = 0x0;
        fCommon.fFlagBits = 0x0;
        fCommon.fStencilSettings.setDisabled();
        fCommon.fFirstCoverageStage = kNumStages;
        fCommon.fCoverage = 0xffffffff;
        fCommon.fColorFilterMode = SkXfermode::kDst_Mode;
        fCommon.fColorFilterColor = 0x0;
        fCommon.fDrawFace = kBoth_DrawFace;
    }

    /**
     * Initializes the GrDrawState based on a GrPaint. Note that GrDrawState
     * encompasses more than GrPaint. Aspects of GrDrawState that have no
     * GrPaint equivalents are not modified. GrPaint has fewer stages than
     * GrDrawState. The extra GrDrawState stages are disabled.
     */
    void setFromPaint(const GrPaint& paint);

    ///////////////////////////////////////////////////////////////////////////
    /// @name Vertex Attributes
    ////

    enum {
        kMaxVertexAttribCnt = kLast_GrVertexAttribBinding + 4,
    };

   /**
     * The format of vertices is represented as an array of GrVertexAttribs, with each representing
     * the type of the attribute, its offset, and semantic binding (see GrVertexAttrib in
     * GrTypesPriv.h).
     *
     * The mapping of attributes with kEffect bindings to GrEffect inputs is specified when
     * setEffect is called.
     */

    /**
     *  Sets vertex attributes for next draw.
     *
     *  @param attribs    the array of vertex attributes to set.
     *  @param count      the number of attributes being set, limited to kMaxVertexAttribCnt.
     */
    void setVertexAttribs(const GrVertexAttrib attribs[], int count);

    const GrVertexAttrib* getVertexAttribs() const { return fCommon.fVertexAttribs.begin(); }
    int getVertexAttribCount() const { return fCommon.fVertexAttribs.count(); }

    size_t getVertexSize() const;

    /**
     *  Sets default vertex attributes for next draw. The default is a single attribute:
     *  {kVec2f_GrVertexAttribType, 0, kPosition_GrVertexAttribType}
     */
    void setDefaultVertexAttribs();

    /**
     * Getters for index into getVertexAttribs() for particular bindings. -1 is returned if the
     * binding does not appear in the current attribs. These bindings should appear only once in
     * the attrib array.
     */

    int positionAttributeIndex() const {
        return fCommon.fFixedFunctionVertexAttribIndices[kPosition_GrVertexAttribBinding];
    }
    int localCoordAttributeIndex() const {
        return fCommon.fFixedFunctionVertexAttribIndices[kLocalCoord_GrVertexAttribBinding];
    }
    int colorVertexAttributeIndex() const {
        return fCommon.fFixedFunctionVertexAttribIndices[kColor_GrVertexAttribBinding];
    }
    int coverageVertexAttributeIndex() const {
        return fCommon.fFixedFunctionVertexAttribIndices[kCoverage_GrVertexAttribBinding];
    }

    bool hasLocalCoordAttribute() const {
        return -1 != fCommon.fFixedFunctionVertexAttribIndices[kLocalCoord_GrVertexAttribBinding];
    }
    bool hasColorVertexAttribute() const {
        return -1 != fCommon.fFixedFunctionVertexAttribIndices[kColor_GrVertexAttribBinding];
    }
    bool hasCoverageVertexAttribute() const {
        return -1 != fCommon.fFixedFunctionVertexAttribIndices[kCoverage_GrVertexAttribBinding];
    }

    bool validateVertexAttribs() const;

    /**
     * Accessing positions, local coords, or colors, of a vertex within an array is a hassle
     * involving casts and simple math. These helpers exist to keep GrDrawTarget clients' code a bit
     * nicer looking.
     */

    /**
     * Gets a pointer to a GrPoint of a vertex's position or texture
     * coordinate.
     * @param vertices      the vertex array
     * @param vertexIndex   the index of the vertex in the array
     * @param vertexSize    the size of each vertex in the array
     * @param offset        the offset in bytes of the vertex component.
     *                      Defaults to zero (corresponding to vertex position)
     * @return pointer to the vertex component as a GrPoint
     */
    static GrPoint* GetVertexPoint(void* vertices,
                                   int vertexIndex,
                                   int vertexSize,
                                   int offset = 0) {
        intptr_t start = GrTCast<intptr_t>(vertices);
        return GrTCast<GrPoint*>(start + offset +
                                 vertexIndex * vertexSize);
    }
    static const GrPoint* GetVertexPoint(const void* vertices,
                                         int vertexIndex,
                                         int vertexSize,
                                         int offset = 0) {
        intptr_t start = GrTCast<intptr_t>(vertices);
        return GrTCast<const GrPoint*>(start + offset +
                                       vertexIndex * vertexSize);
    }

    /**
     * Gets a pointer to a GrColor inside a vertex within a vertex array.
     * @param vertices      the vetex array
     * @param vertexIndex   the index of the vertex in the array
     * @param vertexSize    the size of each vertex in the array
     * @param offset        the offset in bytes of the vertex color
     * @return pointer to the vertex component as a GrColor
     */
    static GrColor* GetVertexColor(void* vertices,
                                   int vertexIndex,
                                   int vertexSize,
                                   int offset) {
        intptr_t start = GrTCast<intptr_t>(vertices);
        return GrTCast<GrColor*>(start + offset +
                                 vertexIndex * vertexSize);
    }
    static const GrColor* GetVertexColor(const void* vertices,
                                         int vertexIndex,
                                         int vertexSize,
                                         int offset) {
        const intptr_t start = GrTCast<intptr_t>(vertices);
        return GrTCast<const GrColor*>(start + offset +
                                       vertexIndex * vertexSize);
    }

    /// @}

    /**
     * Determines whether src alpha is guaranteed to be one for all src pixels
     */
    bool srcAlphaWillBeOne() const;

    /**
     * Determines whether the output coverage is guaranteed to be one for all pixels hit by a draw.
     */
    bool hasSolidCoverage() const;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Color
    ////

    /**
     *  Sets color for next draw to a premultiplied-alpha color.
     *
     *  @param color    the color to set.
     */
    void setColor(GrColor color) { fCommon.fColor = color; }

    GrColor getColor() const { return fCommon.fColor; }

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
     * after color-computing effect stages.
     */
    void setColorFilter(GrColor c, SkXfermode::Mode mode) {
        fCommon.fColorFilterColor = c;
        fCommon.fColorFilterMode = mode;
    }

    GrColor getColorFilterColor() const { return fCommon.fColorFilterColor; }
    SkXfermode::Mode getColorFilterMode() const { return fCommon.fColorFilterMode; }

    /**
     * Constructor sets the color to be 'color' which is undone by the destructor.
     */
    class AutoColorRestore : public ::GrNoncopyable {
    public:
        AutoColorRestore() : fDrawState(NULL), fOldColor(0) {}

        AutoColorRestore(GrDrawState* drawState, GrColor color) {
            fDrawState = NULL;
            this->set(drawState, color);
        }

        void reset() {
            if (NULL != fDrawState) {
                fDrawState->setColor(fOldColor);
                fDrawState = NULL;
            }
        }

        void set(GrDrawState* drawState, GrColor color) {
            this->reset();
            fDrawState = drawState;
            fOldColor = fDrawState->getColor();
            fDrawState->setColor(color);
        }

        ~AutoColorRestore() { this->reset(); }
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
        fCommon.fCoverage = GrColorPackRGBA(coverage, coverage, coverage, coverage);
    }

    /**
     * Version of above that specifies 4 channel per-vertex color. The value
     * should be premultiplied.
     */
    void setCoverage4(GrColor coverage) {
        fCommon.fCoverage = coverage;
    }

    GrColor getCoverage() const {
        return fCommon.fCoverage;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Effect Stages
    ////

    const GrEffectRef* setEffect(int stageIdx, const GrEffectRef* effect) {
        fStages[stageIdx].setEffect(effect);
        return effect;
    }

    const GrEffectRef* setEffect(int stageIdx, const GrEffectRef* effect,
                                 int attr0, int attr1 = -1) {
        fStages[stageIdx].setEffect(effect, attr0, attr1);
        return effect;
    }

    /**
     * Creates a GrSimpleTextureEffect that uses local coords as texture coordinates.
     */
    void createTextureEffect(int stageIdx, GrTexture* texture, const SkMatrix& matrix) {
        GrAssert(!this->getStage(stageIdx).getEffect());
        GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix);
        this->setEffect(stageIdx, effect)->unref();
    }
    void createTextureEffect(int stageIdx,
                             GrTexture* texture,
                             const SkMatrix& matrix,
                             const GrTextureParams& params) {
        GrAssert(!this->getStage(stageIdx).getEffect());
        GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix, params);
        this->setEffect(stageIdx, effect)->unref();
    }

    bool stagesDisabled() {
        for (int i = 0; i < kNumStages; ++i) {
            if (NULL != fStages[i].getEffect()) {
                return false;
            }
        }
        return true;
    }

    void disableStage(int stageIdx) {
        this->setEffect(stageIdx, NULL);
    }

    /**
     * Release all the GrEffects referred to by this draw state.
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

    /**
     * Returns the current stage by index.
     */
    const GrEffectStage& getStage(int stageIdx) const {
        GrAssert((unsigned)stageIdx < kNumStages);
        return fStages[stageIdx];
    }

    /**
     * Called when the source coord system is changing. This ensures that effects will see the
     * correct local coordinates. oldToNew gives the transformation from the old coord system in
     * which the geometry was specified to the new coordinate system from which it will be rendered.
     */
    void localCoordChange(const SkMatrix& oldToNew) {
        for (int i = 0; i < kNumStages; ++i) {
            if (this->isStageEnabled(i)) {
                fStages[i].localCoordChange(oldToNew);
            }
        }
    }

    /**
     * Checks whether any of the effects will read the dst pixel color.
     */
    bool willEffectReadDst() const {
        for (int s = 0; s < kNumStages; ++s) {
            if (this->isStageEnabled(s) && (*this->getStage(s).getEffect())->willReadDst()) {
                return true;
            }
        }
        return false;
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
        fCommon.fFirstCoverageStage = firstCoverageStage;
    }

    /**
     * Gets the index of the first coverage-computing stage.
     */
    int getFirstCoverageStage() const {
        return fCommon.fFirstCoverageStage;
    }

    ///@}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Blending
    ////

    /**
     * Sets the blending function coefficients.
     *
     * The blend function will be:
     *    D' = sat(S*srcCoef + D*dstCoef)
     *
     *   where D is the existing destination color, S is the incoming source
     *   color, and D' is the new destination color that will be written. sat()
     *   is the saturation function.
     *
     * @param srcCoef coefficient applied to the src color.
     * @param dstCoef coefficient applied to the dst color.
     */
    void setBlendFunc(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff) {
        fCommon.fSrcBlend = srcCoeff;
        fCommon.fDstBlend = dstCoeff;
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

    GrBlendCoeff getSrcBlendCoeff() const { return fCommon.fSrcBlend; }
    GrBlendCoeff getDstBlendCoeff() const { return fCommon.fDstBlend; }

    void getDstBlendCoeff(GrBlendCoeff* srcBlendCoeff,
                          GrBlendCoeff* dstBlendCoeff) const {
        *srcBlendCoeff = fCommon.fSrcBlend;
        *dstBlendCoeff = fCommon.fDstBlend;
    }

    /**
     * Sets the blending function constant referenced by the following blending
     * coefficients:
     *      kConstC_GrBlendCoeff
     *      kIConstC_GrBlendCoeff
     *      kConstA_GrBlendCoeff
     *      kIConstA_GrBlendCoeff
     *
     * @param constant the constant to set
     */
    void setBlendConstant(GrColor constant) { fCommon.fBlendConstant = constant; }

    /**
     * Retrieves the last value set by setBlendConstant()
     * @return the blending constant value
     */
    GrColor getBlendConstant() const { return fCommon.fBlendConstant; }

    /**
     * Determines whether multiplying the computed per-pixel color by the pixel's fractional
     * coverage before the blend will give the correct final destination color. In general it
     * will not as coverage is applied after blending.
     */
    bool canTweakAlphaForCoverage() const;

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
         * Emit the src color, disable HW blending (replace dst with src)
         */
        kDisableBlend_BlendOptFlag      = 0x2,
        /**
         * The coverage value does not have to be computed separately from alpha, the the output
         * color can be the modulation of the two.
         */
        kCoverageAsAlpha_BlendOptFlag   = 0x4,
        /**
         * Instead of emitting a src color, emit coverage in the alpha channel and r,g,b are
         * "don't cares".
         */
        kEmitCoverage_BlendOptFlag      = 0x8,
        /**
         * Emit transparent black instead of the src color, no need to compute coverage.
         */
        kEmitTransBlack_BlendOptFlag    = 0x10,
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
     */
    BlendOptFlags getBlendOpts(bool forceCoverage = false,
                               GrBlendCoeff* srcCoeff = NULL,
                               GrBlendCoeff* dstCoeff = NULL) const;

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name View Matrix
    ////

    /**
     * Sets the matrix applied to vertex positions.
     *
     * In the post-view-matrix space the rectangle [0,w]x[0,h]
     * fully covers the render target. (w and h are the width and height of the
     * the render-target.)
     */
    void setViewMatrix(const SkMatrix& m) { fCommon.fViewMatrix = m; }

    /**
     * Gets a writable pointer to the view matrix.
     */
    SkMatrix* viewMatrix() { return &fCommon.fViewMatrix; }

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
    void preConcatViewMatrix(const SkMatrix& m) { fCommon.fViewMatrix.preConcat(m); }

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
    void postConcatViewMatrix(const SkMatrix& m) { fCommon.fViewMatrix.postConcat(m); }

    /**
     * Retrieves the current view matrix
     * @return the current view matrix.
     */
    const SkMatrix& getViewMatrix() const { return fCommon.fViewMatrix; }

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
        if (fCommon.fViewMatrix.invert(&inverse)) {
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
     * Effect matrices are automatically adjusted to compensate.
     */
    class AutoViewMatrixRestore : public ::GrNoncopyable {
    public:
        AutoViewMatrixRestore() : fDrawState(NULL) {}

        AutoViewMatrixRestore(GrDrawState* ds, const SkMatrix& preconcatMatrix) {
            fDrawState = NULL;
            this->set(ds, preconcatMatrix);
        }

        ~AutoViewMatrixRestore() { this->restore(); }

        /**
         * Can be called prior to destructor to restore the original matrix.
         */
        void restore();

        void set(GrDrawState* drawState, const SkMatrix& preconcatMatrix);

        bool isSet() const { return NULL != fDrawState; }

    private:
        GrDrawState*                        fDrawState;
        SkMatrix                            fViewMatrix;
        GrEffectStage::SavedCoordChange     fSavedCoordChanges[GrDrawState::kNumStages];
        uint32_t                            fRestoreMask;
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
        AutoDeviceCoordDraw(GrDrawState* drawState) {
            fDrawState = NULL;
            this->set(drawState);
        }

        ~AutoDeviceCoordDraw() { this->restore(); }

        bool set(GrDrawState* drawState);

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
        const SkMatrix& getOriginalMatrix() const {
            GrAssert(this->succeeded());
            return fViewMatrix;
        }

        /**
         * Can be called prior to destructor to restore the original matrix.
         */
        void restore();

    private:
        GrDrawState*                        fDrawState;
        SkMatrix                            fViewMatrix;
        GrEffectStage::SavedCoordChange     fSavedCoordChanges[GrDrawState::kNumStages];
        uint32_t                            fRestoreMask;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Render Target
    ////

    /**
     * Sets the render-target used at the next drawing call
     *
     * @param target  The render target to set.
     */
    void setRenderTarget(GrRenderTarget* target) {
        fRenderTarget.reset(SkSafeRef(target));
    }

    /**
     * Retrieves the currently set render-target.
     *
     * @return    The currently set render target.
     */
    const GrRenderTarget* getRenderTarget() const { return fRenderTarget.get(); }
    GrRenderTarget* getRenderTarget() { return fRenderTarget.get(); }

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
        fCommon.fStencilSettings = settings;
    }

    /**
     * Shortcut to disable stencil testing and ops.
     */
    void disableStencil() {
        fCommon.fStencilSettings.setDisabled();
    }

    const GrStencilSettings& getStencil() const { return fCommon.fStencilSettings; }

    GrStencilSettings* stencil() { return &fCommon.fStencilSettings; }

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

    void resetStateFlags() {
        fCommon.fFlagBits = 0;
    }

    /**
     * Enable render state settings.
     *
     * @param stateBits bitfield of StateBits specifying the states to enable
     */
    void enableState(uint32_t stateBits) {
        fCommon.fFlagBits |= stateBits;
    }

    /**
     * Disable render state settings.
     *
     * @param stateBits bitfield of StateBits specifying the states to disable
     */
    void disableState(uint32_t stateBits) {
        fCommon.fFlagBits &= ~(stateBits);
    }

    /**
     * Enable or disable stateBits based on a boolean.
     *
     * @param stateBits bitfield of StateBits to enable or disable
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
        return 0 != (fCommon.fFlagBits & kDither_StateBit);
    }

    bool isHWAntialiasState() const {
        return 0 != (fCommon.fFlagBits & kHWAntialias_StateBit);
    }

    bool isClipState() const {
        return 0 != (fCommon.fFlagBits & kClip_StateBit);
    }

    bool isColorWriteDisabled() const {
        return 0 != (fCommon.fFlagBits & kNoColorWrites_StateBit);
    }

    bool isCoverageDrawing() const {
        return 0 != (fCommon.fFlagBits & kCoverageDrawing_StateBit);
    }

    bool isStateFlagEnabled(uint32_t stateBit) const {
        return 0 != (stateBit & fCommon.fFlagBits);
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
        fCommon.fDrawFace = face;
    }

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    DrawFace getDrawFace() const { return fCommon.fDrawFace; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    bool isStageEnabled(int s) const {
        GrAssert((unsigned)s < kNumStages);
        return (NULL != fStages[s].getEffect());
    }

    bool operator ==(const GrDrawState& s) const {
        if (fRenderTarget.get() != s.fRenderTarget.get() || fCommon != s.fCommon) {
            return false;
        }
        for (int i = 0; i < kNumStages; i++) {
            bool enabled = this->isStageEnabled(i);
            if (enabled != s.isStageEnabled(i)) {
                return false;
            }
            if (enabled && this->fStages[i] != s.fStages[i]) {
                return false;
            }
        }
        return true;
    }
    bool operator !=(const GrDrawState& s) const { return !(*this == s); }

    GrDrawState& operator= (const GrDrawState& s) {
        this->setRenderTarget(s.fRenderTarget.get());
        fCommon = s.fCommon;
        for (int i = 0; i < kNumStages; i++) {
            if (s.isStageEnabled(i)) {
                this->fStages[i] = s.fStages[i];
            }
        }
        return *this;
    }

private:

    /** Fields that are identical in GrDrawState and GrDrawState::DeferredState. */
    struct CommonState {
        // These fields are roughly sorted by decreasing likelihood of being different in op==
        GrColor                                  fColor;
        SkMatrix                                 fViewMatrix;
        GrBlendCoeff                             fSrcBlend;
        GrBlendCoeff                             fDstBlend;
        GrColor                                  fBlendConstant;
        uint32_t                                 fFlagBits;
        GrVertexAttribArray<kMaxVertexAttribCnt> fVertexAttribs;
        GrStencilSettings                        fStencilSettings;
        int                                      fFirstCoverageStage;
        GrColor                                  fCoverage;
        SkXfermode::Mode                         fColorFilterMode;
        GrColor                                  fColorFilterColor;
        DrawFace                                 fDrawFace;

        // This is simply a different representation of info in fVertexAttribs and thus does
        // not need to be compared in op==.
        int fFixedFunctionVertexAttribIndices[kGrFixedFunctionVertexAttribBindingCnt];

        bool operator== (const CommonState& other) const {
            bool result = fColor == other.fColor &&
                          fViewMatrix.cheapEqualTo(other.fViewMatrix) &&
                          fSrcBlend == other.fSrcBlend &&
                          fDstBlend == other.fDstBlend &&
                          fBlendConstant == other.fBlendConstant &&
                          fFlagBits == other.fFlagBits &&
                          fVertexAttribs == other.fVertexAttribs &&
                          fStencilSettings == other.fStencilSettings &&
                          fFirstCoverageStage == other.fFirstCoverageStage &&
                          fCoverage == other.fCoverage &&
                          fColorFilterMode == other.fColorFilterMode &&
                          fColorFilterColor == other.fColorFilterColor &&
                          fDrawFace == other.fDrawFace;
            GrAssert(!result || 0 == memcmp(fFixedFunctionVertexAttribIndices,
                                            other.fFixedFunctionVertexAttribIndices,
                                            sizeof(fFixedFunctionVertexAttribIndices)));
            return result;
        }
        bool operator!= (const CommonState& other) const { return !(*this == other); }
    };

    /** GrDrawState uses GrEffectStages to hold stage state which holds a ref on GrEffectRef.
        DeferredState must directly reference GrEffects, however. */
    struct SavedEffectStage {
        SavedEffectStage() : fEffect(NULL) {}
        const GrEffect*                    fEffect;
        GrEffectStage::SavedCoordChange    fCoordChange;
    };

public:
    /**
     * DeferredState contains all of the data of a GrDrawState but does not hold refs on GrResource
     * objects. Resources are allowed to hit zero ref count while in DeferredStates. Their internal
     * dispose mechanism returns them to the cache. This allows recycling resources through the
     * the cache while they are in a deferred draw queue.
     */
    class DeferredState {
    public:
        DeferredState() : fRenderTarget(NULL) {
            GR_DEBUGCODE(fInitialized = false;)
        }
        // TODO: Remove this when DeferredState no longer holds a ref to the RT
        ~DeferredState() { SkSafeUnref(fRenderTarget); }

        void saveFrom(const GrDrawState& drawState) {
            fCommon = drawState.fCommon;
            // TODO: Here we will copy the GrRenderTarget pointer without taking a ref.
            fRenderTarget = drawState.fRenderTarget.get();
            SkSafeRef(fRenderTarget);
            // Here we ref the effects directly rather than the effect-refs. TODO: When the effect-
            // ref gets fully unref'ed it will cause the underlying effect to unref its resources
            // and recycle them to the cache (if no one else is holding a ref to the resources).
            for (int i = 0; i < kNumStages; ++i) {
                fStages[i].saveFrom(drawState.fStages[i]);
            }
            GR_DEBUGCODE(fInitialized = true;)
        }

        void restoreTo(GrDrawState* drawState) {
            GrAssert(fInitialized);
            drawState->fCommon = fCommon;
            drawState->setRenderTarget(fRenderTarget);
            for (int i = 0; i < kNumStages; ++i) {
                fStages[i].restoreTo(&drawState->fStages[i]);
            }
        }

        bool isEqual(const GrDrawState& state) const {
            if (fRenderTarget != state.fRenderTarget.get() || fCommon != state.fCommon) {
                return false;
            }
            for (int i = 0; i < kNumStages; ++i) {
                if (!fStages[i].isEqual(state.fStages[i])) {
                    return false;
                }
            }
            return true;
        }

    private:
        GrRenderTarget*                       fRenderTarget;
        CommonState                           fCommon;
        GrEffectStage::DeferredStage          fStages[kNumStages];

        GR_DEBUGCODE(bool fInitialized;)
    };

private:

    SkAutoTUnref<GrRenderTarget>           fRenderTarget;
    CommonState                            fCommon;
    GrEffectStage                          fStages[kNumStages];

    typedef GrRefCnt INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrDrawState::BlendOptFlags);

#endif
