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
#include "GrRefCnt.h"
#include "GrRenderTarget.h"
#include "GrStencil.h"
#include "GrTemplates.h"
#include "GrTexture.h"
#include "effects/GrSimpleTextureEffect.h"

#include "SkMatrix.h"
#include "SkXfermode.h"

class GrPaint;

class GrDrawState : public GrRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawState)

    /**
     * Total number of effect stages. Each stage can host a GrEffect. A stage is enabled if it has a
     * GrEffect. The effect produces an output color in the fragment shader. It's inputs are the
     * output from the previous enabled stage and a position. The position is either derived from
     * the interpolated vertex positions or explicit per-vertex coords, depending upon the
     * GrVertexLayout used to draw.
     *
     * The stages are divided into two sets, color-computing and coverage-computing. The final color
     * stage produces the final pixel color. The coverage-computing stages function exactly as the
     * color-computing but the output of the final coverage stage is treated as a fractional pixel
     * coverage rather than as input to the src/dst color blend step.
     *
     * The input color to the first enabled color-stage is either the constant color or interpolated
     * per-vertex colors, depending upon GrVertexLayout. The input to the first coverage stage is
     * either a constant coverage (usually full-coverage), interpolated per-vertex coverage, or
     * edge-AA computed coverage. (This latter is going away as soon as it can be rewritten as a
     * GrEffect).
     *
     * See the documentation of kCoverageDrawing_StateBit for information about disabling the
     * the color / coverage distinction.
     *
     * Stages 0 through GrPaint::kTotalStages-1 are reserved for stages copied from the client's
     * GrPaint. Stages GrPaint::kTotalStages through kNumStages-2 are earmarked for use by
     * GrTextContext and GrPathRenderer-derived classes. kNumStages-1 is earmarked for clipping
     * by GrClipMaskManager.
     */
    enum {
        kNumStages = 5,
        kMaxTexCoords = kNumStages
    };

    GrDrawState() {
#if GR_DEBUG
        VertexLayoutUnitTest();
#endif
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

        fCommon.fColor = 0xffffffff;
        fCommon.fViewMatrix.reset();
        fCommon.fSrcBlend = kOne_GrBlendCoeff;
        fCommon.fDstBlend = kZero_GrBlendCoeff;
        fCommon.fBlendConstant = 0x0;
        fCommon.fFlagBits = 0x0;
        fCommon.fVertexEdgeType = kHairLine_EdgeType;
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
    /// @name Vertex Format
    ////

    /**
     * The format of vertices is represented as a bitfield of flags.
     * Flags that indicate the layout of vertex data. Vertices always contain
     * positions and may also contain up to GrDrawState::kMaxTexCoords sets
     * of 2D texture coordinates, per-vertex colors, and per-vertex coverage.
     * Each stage can
     * use any of the texture coordinates as its input texture coordinates or it
     * may use the positions as texture coordinates.
     *
     * If no texture coordinates are specified for a stage then the stage is
     * disabled.
     *
     * Only one type of texture coord can be specified per stage. For
     * example StageTexCoordVertexLayoutBit(0, 2) and
     * StagePosAsTexCoordVertexLayoutBit(0) cannot both be specified.
     *
     * The order in memory is always (position, texture coord 0, ..., color,
     * coverage) with any unused fields omitted. Note that this means that if
     * only texture coordinates 1 is referenced then there is no texture
     * coordinates 0 and the order would be (position, texture coordinate 1
     * [, color][, coverage]).
     */

    /**
     * Generates a bit indicating that a texture stage uses texture coordinates
     *
     * @param stageIdx    the stage that will use texture coordinates.
     * @param texCoordIdx the index of the texture coordinates to use
     *
     * @return the bit to add to a GrVertexLayout bitfield.
     */
    static int StageTexCoordVertexLayoutBit(int stageIdx, int texCoordIdx) {
        GrAssert(stageIdx < kNumStages);
        GrAssert(texCoordIdx < kMaxTexCoords);
        return 1 << (stageIdx + (texCoordIdx * kNumStages));
    }

    static bool StageUsesTexCoords(GrVertexLayout layout, int stageIdx);

private:
    // non-stage bits start at this index.
    static const int STAGE_BIT_CNT = kNumStages * kMaxTexCoords;
public:

    /**
     * Additional Bits that can be specified in GrVertexLayout.
     */
    enum VertexLayoutBits {
        /* vertices have colors (GrColor) */
        kColor_VertexLayoutBit              = 1 << (STAGE_BIT_CNT + 0),
        /* vertices have coverage (GrColor)
         */
        kCoverage_VertexLayoutBit           = 1 << (STAGE_BIT_CNT + 1),
        /* Use text vertices. (Pos and tex coords may be a different type for
         * text [GrGpuTextVertex vs GrPoint].)
         */
        kTextFormat_VertexLayoutBit         = 1 << (STAGE_BIT_CNT + 2),

        /* Each vertex specificies an edge. Distance to the edge is used to
         * compute a coverage. See GrDrawState::setVertexEdgeType().
         */
        kEdge_VertexLayoutBit               = 1 << (STAGE_BIT_CNT + 3),
        // for below assert
        kDummyVertexLayoutBit,
        kHighVertexLayoutBit = kDummyVertexLayoutBit - 1
    };
    // make sure we haven't exceeded the number of bits in GrVertexLayout.
    GR_STATIC_ASSERT(kHighVertexLayoutBit < ((uint64_t)1 << 8*sizeof(GrVertexLayout)));

    ////////////////////////////////////////////////////////////////////////////
    // Helpers for picking apart vertex layouts

    /**
     * Helper function to compute the size of a vertex from a vertex layout
     * @return size of a single vertex.
     */
    static size_t VertexSize(GrVertexLayout vertexLayout);

    /**
     * Helper function for determining the index of texture coordinates that
     * is input for a texture stage. Note that a stage may instead use positions
     * as texture coordinates, in which case the result of the function is
     * indistinguishable from the case when the stage is disabled.
     *
     * @param stageIdx      the stage to query
     * @param vertexLayout  layout to query
     *
     * @return the texture coordinate index or -1 if the stage doesn't use
     *         separate (non-position) texture coordinates.
     */
    static int VertexTexCoordsForStage(int stageIdx, GrVertexLayout vertexLayout);

    /**
     * Helper function to compute the offset of texture coordinates in a vertex
     * @return offset of texture coordinates in vertex layout or -1 if the
     *         layout has no texture coordinates. Will be 0 if positions are
     *         used as texture coordinates for the stage.
     */
    static int VertexStageCoordOffset(int stageIdx, GrVertexLayout vertexLayout);

    /**
     * Helper function to compute the offset of the color in a vertex
     * @return offset of color in vertex layout or -1 if the
     *         layout has no color.
     */
    static int VertexColorOffset(GrVertexLayout vertexLayout);

    /**
     * Helper function to compute the offset of the coverage in a vertex
     * @return offset of coverage in vertex layout or -1 if the
     *         layout has no coverage.
     */
    static int VertexCoverageOffset(GrVertexLayout vertexLayout);

     /**
      * Helper function to compute the offset of the edge pts in a vertex
      * @return offset of edge in vertex layout or -1 if the
      *         layout has no edge.
      */
     static int VertexEdgeOffset(GrVertexLayout vertexLayout);

    /**
     * Helper function to determine if vertex layout contains explicit texture
     * coordinates of some index.
     *
     * @param coordIndex    the tex coord index to query
     * @param vertexLayout  layout to query
     *
     * @return true if vertex specifies texture coordinates for the index,
     *              false otherwise.
     */
    static bool VertexUsesTexCoordIdx(int coordIndex,
                                      GrVertexLayout vertexLayout);

    /**
     * Helper function to compute the size of each vertex and the offsets of
     * texture coordinates and color. Determines tex coord offsets by tex coord
     * index rather than by stage. (Each stage can be mapped to any t.c. index
     * by StageTexCoordVertexLayoutBit.)
     *
     * @param vertexLayout          the layout to query
     * @param texCoordOffsetsByIdx  after return it is the offset of each
     *                              tex coord index in the vertex or -1 if
     *                              index isn't used. (optional)
     * @param colorOffset           after return it is the offset of the
     *                              color field in each vertex, or -1 if
     *                              there aren't per-vertex colors. (optional)
     * @param coverageOffset        after return it is the offset of the
     *                              coverage field in each vertex, or -1 if
     *                              there aren't per-vertex coeverages.
     *                              (optional)
     * @param edgeOffset            after return it is the offset of the
     *                              edge eq field in each vertex, or -1 if
     *                              there aren't per-vertex edge equations.
     *                              (optional)
     * @return size of a single vertex
     */
    static int VertexSizeAndOffsetsByIdx(GrVertexLayout vertexLayout,
                   int texCoordOffsetsByIdx[kMaxTexCoords],
                   int *colorOffset,
                   int *coverageOffset,
                   int* edgeOffset);

    /**
     * Helper function to compute the size of each vertex and the offsets of
     * texture coordinates and color. Determines tex coord offsets by stage
     * rather than by index. (Each stage can be mapped to any t.c. index
     * by StageTexCoordVertexLayoutBit.) If a stage uses positions for
     * tex coords then that stage's offset will be 0 (positions are always at 0).
     *
     * @param vertexLayout              the layout to query
     * @param texCoordOffsetsByStage    after return it is the offset of each
     *                                  tex coord index in the vertex or -1 if
     *                                  index isn't used. (optional)
     * @param colorOffset               after return it is the offset of the
     *                                  color field in each vertex, or -1 if
     *                                  there aren't per-vertex colors.
     *                                  (optional)
     * @param coverageOffset            after return it is the offset of the
     *                                  coverage field in each vertex, or -1 if
     *                                  there aren't per-vertex coeverages.
     *                                  (optional)
     * @param edgeOffset                after return it is the offset of the
     *                                  edge eq field in each vertex, or -1 if
     *                                  there aren't per-vertex edge equations.
     *                                  (optional)
     * @return size of a single vertex
     */
    static int VertexSizeAndOffsetsByStage(GrVertexLayout vertexLayout,
                   int texCoordOffsetsByStage[kNumStages],
                   int* colorOffset,
                   int* coverageOffset,
                   int* edgeOffset);

    /**
     * Determines whether src alpha is guaranteed to be one for all src pixels
     */
    bool srcAlphaWillBeOne(GrVertexLayout) const;

    /**
     * Determines whether the output coverage is guaranteed to be one for all pixels hit by a draw.
     */
    bool hasSolidCoverage(GrVertexLayout) const;

    /**
     * Accessing positions, texture coords, or colors, of a vertex within an
     * array is a hassle involving casts and simple math. These helpers exist
     * to keep GrDrawTarget clients' code a bit nicer looking.
     */

    /**
     * Gets a pointer to a GrPoint of a vertex's position or texture
     * coordinate.
     * @param vertices      the vetex array
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

    static void VertexLayoutUnitTest();

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
     * after color-computing texture stages.
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
        AutoColorRestore() : fDrawState(NULL) {}

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

    /**
     * Creates a GrSimpleTextureEffect.
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

    void disableStage(int stageIdx) { this->setEffect(stageIdx, NULL); }

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
     * Called when the source coord system is changing. preConcat gives the transformation from the
     * old coord system to the new coord system.
     */
    void preConcatStageMatrices(const SkMatrix& preConcat) {
        this->preConcatStageMatrices(~0U, preConcat);
    }
    /**
     * Version of above that applies the update matrix selectively to stages via a mask.
     */
    void preConcatStageMatrices(uint32_t stageMask, const SkMatrix& preConcat) {
        for (int i = 0; i < kNumStages; ++i) {
            if (((1 << i) & stageMask) && this->isStageEnabled(i)) {
                fStages[i].preConcatCoordChange(preConcat);
            }
        }
    }

    /**
     * Called when the source coord system is changing. preConcatInverse is the inverse of the
     * transformation from the old coord system to the new coord system. Returns false if the matrix
     * cannot be inverted.
     */
    bool preConcatStageMatricesWithInverse(const SkMatrix& preConcatInverse) {
        SkMatrix inv;
        bool computed = false;
        for (int i = 0; i < kNumStages; ++i) {
            if (this->isStageEnabled(i)) {
                if (!computed && !preConcatInverse.invert(&inv)) {
                    return false;
                } else {
                    computed = true;
                }
                fStages[i].preConcatCoordChange(preConcatInverse);
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

        AutoViewMatrixRestore(GrDrawState* ds,
                              const SkMatrix& preconcatMatrix,
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
                 const SkMatrix& preconcatMatrix,
                 uint32_t explicitCoordStageMask = 0);

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
    // @name Edge AA
    // Edge equations can be specified to perform anti-aliasing. Because the
    // edges are specified as per-vertex data, vertices that are shared by
    // multiple edges must be split.
    //
    ////

    /**
     * When specifying edges as vertex data this enum specifies what type of
     * edges are in use. The edges are always 4 SkScalars in memory, even when
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
        /* Axis-aligned ellipse specified as center_x, center_y, x_radius, x_radius/y_radius
           all in window space (y-down). */
        kEllipse_EdgeType,

        kVertexEdgeTypeCnt
    };

    /**
     * Determines the interpretation per-vertex edge data when the
     * kEdge_VertexLayoutBit is set (see GrDrawTarget). When per-vertex edges
     * are not specified the value of this setting has no effect.
     */
    void setVertexEdgeType(VertexEdgeType type) {
        GrAssert(type >=0 && type < kVertexEdgeTypeCnt);
        fCommon.fVertexEdgeType = type;
    }

    VertexEdgeType getVertexEdgeType() const { return fCommon.fVertexEdgeType; }

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

    // Most stages are usually not used, so conditionals here
    // reduce the expected number of bytes touched by 50%.
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
        GrColor                         fColor;
        SkMatrix                        fViewMatrix;
        GrBlendCoeff                    fSrcBlend;
        GrBlendCoeff                    fDstBlend;
        GrColor                         fBlendConstant;
        uint32_t                        fFlagBits;
        VertexEdgeType                  fVertexEdgeType;
        GrStencilSettings               fStencilSettings;
        int                             fFirstCoverageStage;
        GrColor                         fCoverage;
        SkXfermode::Mode                fColorFilterMode;
        GrColor                         fColorFilterColor;
        DrawFace                        fDrawFace;
        bool operator== (const CommonState& other) const {
            return fColor == other.fColor &&
                   fViewMatrix.cheapEqualTo(other.fViewMatrix) &&
                   fSrcBlend == other.fSrcBlend &&
                   fDstBlend == other.fDstBlend &&
                   fBlendConstant == other.fBlendConstant &&
                   fFlagBits == other.fFlagBits &&
                   fVertexEdgeType == other.fVertexEdgeType &&
                   fStencilSettings == other.fStencilSettings &&
                   fFirstCoverageStage == other.fFirstCoverageStage &&
                   fCoverage == other.fCoverage &&
                   fColorFilterMode == other.fColorFilterMode &&
                   fColorFilterColor == other.fColorFilterColor &&
                   fDrawFace == other.fDrawFace;
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
        GrRenderTarget*                 fRenderTarget;
        CommonState                     fCommon;
        GrEffectStage::DeferredStage    fStages[kNumStages];

        GR_DEBUGCODE(bool fInitialized;)
    };

private:
    SkAutoTUnref<GrRenderTarget>    fRenderTarget;
    CommonState                     fCommon;
    GrEffectStage                   fStages[kNumStages];

    typedef GrRefCnt INHERITED;
};

#endif
