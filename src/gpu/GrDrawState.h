/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawState_DEFINED
#define GrDrawState_DEFINED

#include "GrBackendEffectFactory.h"
#include "GrBlend.h"
#include "GrColor.h"
#include "GrEffectStage.h"
#include "GrPaint.h"
#include "GrPoint.h"
#include "GrRenderTarget.h"
#include "GrStencil.h"
#include "GrTemplates.h"
#include "GrTexture.h"
#include "GrTypesPriv.h"
#include "effects/GrSimpleTextureEffect.h"

#include "SkMatrix.h"
#include "SkTypes.h"
#include "SkXfermode.h"

class GrDrawState : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawState)

    GrDrawState() {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        this->reset();
    }

    GrDrawState(const SkMatrix& initialViewMatrix) {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        this->reset(initialViewMatrix);
    }

    /**
     * Copies another draw state.
     **/
    GrDrawState(const GrDrawState& state) : INHERITED() {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        *this = state;
    }

    /**
     * Copies another draw state with a preconcat to the view matrix.
     **/
    GrDrawState(const GrDrawState& state, const SkMatrix& preConcatMatrix) {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        *this = state;
        if (!preConcatMatrix.isIdentity()) {
            for (int i = 0; i < fColorStages.count(); ++i) {
                fColorStages[i].localCoordChange(preConcatMatrix);
            }
            for (int i = 0; i < fCoverageStages.count(); ++i) {
                fCoverageStages[i].localCoordChange(preConcatMatrix);
            }
        }
    }

    virtual ~GrDrawState() { SkASSERT(0 == fBlockEffectRemovalCnt); }

    /**
     * Resets to the default state. GrEffects will be removed from all stages.
     */
    void reset() { this->onReset(NULL); }

    void reset(const SkMatrix& initialViewMatrix) { this->onReset(&initialViewMatrix); }

    /**
     * Initializes the GrDrawState based on a GrPaint, view matrix and render target. Note that
     * GrDrawState encompasses more than GrPaint. Aspects of GrDrawState that have no GrPaint
     * equivalents are set to default values. Clipping will be enabled.
     */
    void setFromPaint(const GrPaint& , const SkMatrix& viewMatrix, GrRenderTarget*);

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
     *  Sets vertex attributes for next draw. The object driving the templatization
     *  should be a global GrVertexAttrib array that is never changed.
     */
    template <const GrVertexAttrib A[]> void setVertexAttribs(int count) {
        this->setVertexAttribs(A, count);
    }

    const GrVertexAttrib* getVertexAttribs() const { return fCommon.fVAPtr; }
    int getVertexAttribCount() const { return fCommon.fVACount; }

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
     * Helper to save/restore vertex attribs
     */
     class AutoVertexAttribRestore {
     public:
         AutoVertexAttribRestore(GrDrawState* drawState) {
             SkASSERT(NULL != drawState);
             fDrawState = drawState;
             fVAPtr = drawState->fCommon.fVAPtr;
             fVACount = drawState->fCommon.fVACount;
             fDrawState->setDefaultVertexAttribs();
         }

         ~AutoVertexAttribRestore(){
             fDrawState->setVertexAttribs(fVAPtr, fVACount);
         }

     private:
         GrDrawState*          fDrawState;
         const GrVertexAttrib* fVAPtr;
         int                   fVACount;
     };

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
     * Constructor sets the color to be 'color' which is undone by the destructor.
     */
    class AutoColorRestore : public ::SkNoncopyable {
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

    uint8_t getCoverage() const {
        return GrColorUnpackR(fCommon.fCoverage);
    }

    GrColor getCoverageColor() const {
        return fCommon.fCoverage;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Effect Stages
    /// Each stage hosts a GrEffect. The effect produces an output color or coverage in the fragment
    /// shader. Its inputs are the output from the previous stage as well as some variables
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

    const GrEffectRef* addColorEffect(const GrEffectRef* effect, int attr0 = -1, int attr1 = -1) {
        SkASSERT(NULL != effect);
        SkNEW_APPEND_TO_TARRAY(&fColorStages, GrEffectStage, (effect, attr0, attr1));
        return effect;
    }

    const GrEffectRef* addCoverageEffect(const GrEffectRef* effect, int attr0 = -1, int attr1 = -1) {
        SkASSERT(NULL != effect);
        SkNEW_APPEND_TO_TARRAY(&fCoverageStages, GrEffectStage, (effect, attr0, attr1));
        return effect;
    }

    /**
     * Creates a GrSimpleTextureEffect that uses local coords as texture coordinates.
     */
    void addColorTextureEffect(GrTexture* texture, const SkMatrix& matrix) {
        GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix);
        this->addColorEffect(effect)->unref();
    }

    void addCoverageTextureEffect(GrTexture* texture, const SkMatrix& matrix) {
        GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix);
        this->addCoverageEffect(effect)->unref();
    }

    void addColorTextureEffect(GrTexture* texture,
                               const SkMatrix& matrix,
                               const GrTextureParams& params) {
        GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix, params);
        this->addColorEffect(effect)->unref();
    }

    void addCoverageTextureEffect(GrTexture* texture,
                                  const SkMatrix& matrix,
                                  const GrTextureParams& params) {
        GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix, params);
        this->addCoverageEffect(effect)->unref();
    }

    /**
     * When this object is destroyed it will remove any effects from the draw state that were added
     * after its constructor.
     */
    class AutoRestoreEffects : public ::SkNoncopyable {
    public:
        AutoRestoreEffects() : fDrawState(NULL), fColorEffectCnt(0), fCoverageEffectCnt(0) {}

        AutoRestoreEffects(GrDrawState* ds) : fDrawState(NULL), fColorEffectCnt(0), fCoverageEffectCnt(0) {
            this->set(ds);
        }

        ~AutoRestoreEffects() { this->set(NULL); }

        void set(GrDrawState* ds) {
            if (NULL != fDrawState) {
                int n = fDrawState->fColorStages.count() - fColorEffectCnt;
                SkASSERT(n >= 0);
                fDrawState->fColorStages.pop_back_n(n);
                n = fDrawState->fCoverageStages.count() - fCoverageEffectCnt;
                SkASSERT(n >= 0);
                fDrawState->fCoverageStages.pop_back_n(n);
                SkDEBUGCODE(--fDrawState->fBlockEffectRemovalCnt;)
            }
            fDrawState = ds;
            if (NULL != ds) {
                fColorEffectCnt = ds->fColorStages.count();
                fCoverageEffectCnt = ds->fCoverageStages.count();
                SkDEBUGCODE(++ds->fBlockEffectRemovalCnt;)
            }
        }

    private:
        GrDrawState* fDrawState;
        int fColorEffectCnt;
        int fCoverageEffectCnt;
    };

    int numColorStages() const { return fColorStages.count(); }
    int numCoverageStages() const { return fCoverageStages.count(); }
    int numTotalStages() const { return this->numColorStages() + this->numCoverageStages(); }

    const GrEffectStage& getColorStage(int stageIdx) const { return fColorStages[stageIdx]; }
    const GrEffectStage& getCoverageStage(int stageIdx) const { return fCoverageStages[stageIdx]; }

    /**
     * Checks whether any of the effects will read the dst pixel color.
     */
    bool willEffectReadDstColor() const;

    /// @}

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
    #ifdef SK_DEBUG
        if (GrBlendCoeffRefsDst(dstCoeff)) {
            GrPrintf("Unexpected dst blend coeff. Won't work correctly with coverage stages.\n");
        }
        if (GrBlendCoeffRefsSrc(srcCoeff)) {
            GrPrintf("Unexpected src blend coeff. Won't work correctly with coverage stages.\n");
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
     * Sets the view matrix to identity and updates any installed effects to compensate for the
     * coord system change.
     */
    bool setIdentityViewMatrix();

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
     * Effect matrices are automatically adjusted to compensate and adjusted back in the destructor.
     */
    class AutoViewMatrixRestore : public ::SkNoncopyable {
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

        /** Sets the draw state's matrix to identity. This can fail because the current view matrix
            is not invertible. */
        bool setIdentity(GrDrawState* drawState);

    private:
        void doEffectCoordChanges(const SkMatrix& coordChangeMatrix);

        GrDrawState*                                        fDrawState;
        SkMatrix                                            fViewMatrix;
        int                                                 fNumColorStages;
        SkAutoSTArray<8, GrEffectStage::SavedCoordChange>   fSavedCoordChanges;
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

    class AutoRenderTargetRestore : public ::SkNoncopyable {
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
            SkSafeSetNull(fSavedTarget);
        }

        void set(GrDrawState* ds, GrRenderTarget* newTarget) {
            this->restore();

            if (NULL != ds) {
                SkASSERT(NULL == fSavedTarget);
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
        SkASSERT(kInvalid_DrawFace != face);
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

    bool operator ==(const GrDrawState& s) const {
        if (fRenderTarget.get() != s.fRenderTarget.get() ||
            fColorStages.count() != s.fColorStages.count() ||
            fCoverageStages.count() != s.fCoverageStages.count() ||
            fCommon != s.fCommon) {
            return false;
        }
        for (int i = 0; i < fColorStages.count(); i++) {
            if (fColorStages[i] != s.fColorStages[i]) {
                return false;
            }
        }
        for (int i = 0; i < fCoverageStages.count(); i++) {
            if (fCoverageStages[i] != s.fCoverageStages[i]) {
                return false;
            }
        }
        return true;
    }
    bool operator !=(const GrDrawState& s) const { return !(*this == s); }

    GrDrawState& operator= (const GrDrawState& s) {
        SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
        this->setRenderTarget(s.fRenderTarget.get());
        fCommon = s.fCommon;
        fColorStages = s.fColorStages;
        fCoverageStages = s.fCoverageStages;
        return *this;
    }

private:

    void onReset(const SkMatrix* initialViewMatrix) {
        SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numTotalStages());
        fColorStages.reset();
        fCoverageStages.reset();

        fRenderTarget.reset(NULL);

        this->setDefaultVertexAttribs();

        fCommon.fColor = 0xffffffff;
        if (NULL == initialViewMatrix) {
            fCommon.fViewMatrix.reset();
        } else {
            fCommon.fViewMatrix = *initialViewMatrix;
        }
        fCommon.fSrcBlend = kOne_GrBlendCoeff;
        fCommon.fDstBlend = kZero_GrBlendCoeff;
        fCommon.fBlendConstant = 0x0;
        fCommon.fFlagBits = 0x0;
        fCommon.fStencilSettings.setDisabled();
        fCommon.fCoverage = 0xffffffff;
        fCommon.fDrawFace = kBoth_DrawFace;
    }

    /** Fields that are identical in GrDrawState and GrDrawState::DeferredState. */
    struct CommonState {
        // These fields are roughly sorted by decreasing likelihood of being different in op==
        GrColor               fColor;
        SkMatrix              fViewMatrix;
        GrBlendCoeff          fSrcBlend;
        GrBlendCoeff          fDstBlend;
        GrColor               fBlendConstant;
        uint32_t              fFlagBits;
        const GrVertexAttrib* fVAPtr;
        int                   fVACount;
        GrStencilSettings     fStencilSettings;
        GrColor               fCoverage;
        DrawFace              fDrawFace;

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
                          fVACount == other.fVACount &&
                          !memcmp(fVAPtr, other.fVAPtr, fVACount * sizeof(GrVertexAttrib)) &&
                          fStencilSettings == other.fStencilSettings &&
                          fCoverage == other.fCoverage &&
                          fDrawFace == other.fDrawFace;
            SkASSERT(!result || 0 == memcmp(fFixedFunctionVertexAttribIndices,
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
            SkDEBUGCODE(fInitialized = false;)
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
            fStages.reset(drawState.fColorStages.count() + drawState.fCoverageStages.count());
            fColorStageCnt = drawState.fColorStages.count();
            for (int i = 0; i < fColorStageCnt; ++i) {
                fStages[i].saveFrom(drawState.fColorStages[i]);
            }
            for (int i = 0; i < drawState.fCoverageStages.count(); ++i) {
                fStages[i + fColorStageCnt].saveFrom(drawState.fCoverageStages[i]);
            }
            SkDEBUGCODE(fInitialized = true;)
        }

        void restoreTo(GrDrawState* drawState) {
            SkASSERT(fInitialized);
            drawState->fCommon = fCommon;
            drawState->setRenderTarget(fRenderTarget);
            // reinflate color/cov stage arrays.
            drawState->fColorStages.reset();
            for (int i = 0; i < fColorStageCnt; ++i) {
                SkNEW_APPEND_TO_TARRAY(&drawState->fColorStages, GrEffectStage, (fStages[i]));
            }
            int coverageStageCnt = fStages.count() - fColorStageCnt;
            drawState->fCoverageStages.reset();
            for (int i = 0; i < coverageStageCnt; ++i) {
                SkNEW_APPEND_TO_TARRAY(&drawState->fCoverageStages,
                                        GrEffectStage, (fStages[i + fColorStageCnt]));
            }
        }

        bool isEqual(const GrDrawState& state) const {
            int numCoverageStages = fStages.count() - fColorStageCnt;
            if (fRenderTarget != state.fRenderTarget.get() ||
                fColorStageCnt != state.fColorStages.count() ||
                numCoverageStages != state.fCoverageStages.count() ||
                fCommon != state.fCommon) {
                return false;
            }
            bool explicitLocalCoords = state.hasLocalCoordAttribute();
            for (int i = 0; i < fColorStageCnt; ++i) {
                if (!fStages[i].isEqual(state.fColorStages[i], explicitLocalCoords)) {
                    return false;
                }
            }
            for (int i = 0; i < numCoverageStages; ++i) {
                int s = fColorStageCnt + i;
                if (!fStages[s].isEqual(state.fCoverageStages[i], explicitLocalCoords)) {
                    return false;
                }
            }
            return true;
        }

    private:
        typedef SkAutoSTArray<8, GrEffectStage::DeferredStage> DeferredStageArray;

        GrRenderTarget*                       fRenderTarget;
        CommonState                           fCommon;
        int                                   fColorStageCnt;
        DeferredStageArray                    fStages;

        SkDEBUGCODE(bool fInitialized;)
    };

private:

    SkAutoTUnref<GrRenderTarget>        fRenderTarget;
    CommonState                         fCommon;

    typedef SkSTArray<4, GrEffectStage> EffectStageArray;
    EffectStageArray                    fColorStages;
    EffectStageArray                    fCoverageStages;

    // Some of the auto restore objects assume that no effects are removed during their lifetime.
    // This is used to assert that this condition holds.
    SkDEBUGCODE(int fBlockEffectRemovalCnt;)

    /**
     *  Sets vertex attributes for next draw.
     *
     *  @param attribs    the array of vertex attributes to set.
     *  @param count      the number of attributes being set, limited to kMaxVertexAttribCnt.
     */
    void setVertexAttribs(const GrVertexAttrib attribs[], int count);

    typedef SkRefCnt INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrDrawState::BlendOptFlags);

#endif
