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
#include "GrSamplerState.h"
#include "GrStencil.h"

#include "SkXfermode.h"

class GrRenderTarget;
class GrTexture;

struct GrDrawState {

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
     */
    enum {
        kNumStages = 3,
        kMaxTexCoords = kNumStages
    };

    /**
     *  Bitfield used to indicate a set of stages.
     */
    typedef uint32_t StageMask;
    GR_STATIC_ASSERT(sizeof(StageMask)*8 >= GrDrawState::kNumStages);

    GrDrawState() {
        // make sure any pad is zero for memcmp
        // all GrDrawState members should default to something
        // valid by the memset
        memset(this, 0, sizeof(GrDrawState));
            
        // memset exceptions
        fColorFilterMode = SkXfermode::kDstIn_Mode;
        fFirstCoverageStage = kNumStages;

        // pedantic assertion that our ptrs will
        // be NULL (0 ptr is mem addr 0)
        GrAssert((intptr_t)(void*)NULL == 0LL);

        GrAssert(fStencilSettings.isDisabled());
        fFirstCoverageStage = kNumStages;
    }

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

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Textures
    ////

    /**
     * Sets the texture used at the next drawing call
     *
     * @param stage The texture stage for which the texture will be set
     *
     * @param texture The texture to set. Can be NULL though there is no
     * advantage to settings a NULL texture if doing non-textured drawing
     */
    void setTexture(int stage, GrTexture* texture) {
        GrAssert((unsigned)stage < kNumStages);
        fTextures[stage] = texture;
    }

    /**
     * Retrieves the currently set texture.
     *
     * @return    The currently set texture. The return value will be NULL if no
     *            texture has been set, NULL was most recently passed to
     *            setTexture, or the last setTexture was destroyed.
     */
    const GrTexture* getTexture(int stage) const {
        GrAssert((unsigned)stage < kNumStages);
        return fTextures[stage];
    }
    GrTexture* getTexture(int stage) {
        GrAssert((unsigned)stage < kNumStages);
        return fTextures[stage];
    }

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
     * Preconcats the matrix of all samplers in the mask with the same matrix.
     */
    void preConcatSamplerMatrices(StageMask stageMask, const GrMatrix& matrix) {
        GrAssert(!(stageMask & kIllegalStageMaskBits));
        for (int i = 0; i < kNumStages; ++i) {
            if ((1 << i) & stageMask) {
                fSamplerStates[i].preConcatMatrix(matrix);
            }
        }
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
        case kDC_BlendCoeff:
        case kIDC_BlendCoeff:
        case kDA_BlendCoeff:
        case kIDA_BlendCoeff:
            GrPrintf("Unexpected dst blend coeff. Won't work correctly with"
                     "coverage stages.\n");
            break;
        default:
            break;
        }
        switch (srcCoeff) {
        case kSC_BlendCoeff:
        case kISC_BlendCoeff:
        case kSA_BlendCoeff:
        case kISA_BlendCoeff:
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
     *      kConstC_BlendCoeff
     *      kIConstC_BlendCoeff
     *      kConstA_BlendCoeff
     *      kIConstA_BlendCoeff
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
     * Sets the matrix applied to veretx positions.
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

    class AutoViewMatrixRestore : public ::GrNoncopyable {
    public:
        AutoViewMatrixRestore() : fDrawState(NULL) {}
        AutoViewMatrixRestore(GrDrawState* ds, const GrMatrix& newMatrix) {
            fDrawState = NULL;
            this->set(ds, newMatrix);
        }
        AutoViewMatrixRestore(GrDrawState* ds) {
            fDrawState = NULL;
            this->set(ds);
        }
        ~AutoViewMatrixRestore() {
            this->set(NULL, GrMatrix::I());
        }
        void set(GrDrawState* ds, const GrMatrix& newMatrix) {
            if (NULL != fDrawState) {
                fDrawState->setViewMatrix(fSavedMatrix);
            }
            if (NULL != ds) {
                fSavedMatrix = ds->getViewMatrix();
                ds->setViewMatrix(newMatrix);
            }
            fDrawState = ds;
        }
        void set(GrDrawState* ds) {
            if (NULL != fDrawState) {
                fDrawState->setViewMatrix(fSavedMatrix);
            }
            if (NULL != ds) {
                fSavedMatrix = ds->getViewMatrix();
            }
            fDrawState = ds;
        }
    private:
        GrDrawState* fDrawState;
        GrMatrix fSavedMatrix;
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
    void setRenderTarget(GrRenderTarget* target) { fRenderTarget = target; }

    /**
     * Retrieves the currently set rendertarget.
     *
     * @return    The currently set render target.
     */
    const GrRenderTarget* getRenderTarget() const { return fRenderTarget; }
    GrRenderTarget* getRenderTarget() { return fRenderTarget; }

    class AutoRenderTargetRestore : public ::GrNoncopyable {
    public:
        AutoRenderTargetRestore() : fDrawState(NULL) {}
        AutoRenderTargetRestore(GrDrawState* ds, GrRenderTarget* newTarget) {
            fDrawState = NULL;
            this->set(ds, newTarget);
        }
        ~AutoRenderTargetRestore() { this->set(NULL, NULL); }
        void set(GrDrawState* ds, GrRenderTarget* newTarget) {
            if (NULL != fDrawState) {
                fDrawState->setRenderTarget(fSavedTarget);
            }
            if (NULL != ds) {
                fSavedTarget = ds->getRenderTarget();
                ds->setRenderTarget(newTarget);
            }
            fDrawState = ds;
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
    // There are two ways to perform antialiasing using edge equations. One
    // is to specify an (linear or quadratic) edge eq per-vertex. This requires
    // splitting vertices shared by primitives.
    //
    // The other is via setEdgeAAData which sets a set of edges and each
    // is tested against all the edges.
    ////

    /**
     * When specifying edges as vertex data this enum specifies what type of
     * edges are in use. The edges are always 4 GrScalars in memory, even when
     * the edge type requires fewer than 4.
     */
    enum VertexEdgeType {
        /* 1-pixel wide line
           2D implicit line eq (a*x + b*y +c = 0). 4th component unused */
        kHairLine_EdgeType,
        /* 1-pixel wide quadratic
           u^2-v canonical coords (only 2 components used) */
        kHairQuad_EdgeType
    };

    /**
     * Determines the interpretation per-vertex edge data when the 
     * kEdge_VertexLayoutBit is set (see GrDrawTarget). When per-vertex edges
     * are not specified the value of this setting has no effect.
     */
    void setVertexEdgeType(VertexEdgeType type) {
        fVertexEdgeType = type;
    }

    VertexEdgeType getVertexEdgeType() const {
        return fVertexEdgeType;
    }

    /**
     * The absolute maximum number of edges that may be specified for
     * a single draw call when performing edge antialiasing.  This is used for
     * the size of several static buffers, so implementations of getMaxEdges()
     * (below) should clamp to this value.
     */
    enum {
        // TODO: this should be 32 when GrTesselatedPathRenderer is used
        // Visual Studio 2010 does not permit a member array of size 0.
        kMaxEdges = 1
    };

    class Edge {
      public:
        Edge() {}
        Edge(float x, float y, float z) : fX(x), fY(y), fZ(z) {}
        GrPoint intersect(const Edge& other) {
            return GrPoint::Make(
                SkFloatToScalar((fY * other.fZ - other.fY * fZ) /
                                (fX * other.fY - other.fX * fY)),
                SkFloatToScalar((fX * other.fZ - other.fX * fZ) /
                                (other.fX * fY - fX * other.fY)));
        }
        float fX, fY, fZ;
    };

    /**
     * Sets the edge data required for edge antialiasing.
     *
     * @param edges       3 * numEdges float values, representing the edge
     *                    equations in Ax + By + C form
     */
    void setEdgeAAData(const Edge* edges, int numEdges) {
        GrAssert(numEdges <= GrDrawState::kMaxEdges);
        memcpy(fEdgeAAEdges, edges, numEdges * sizeof(GrDrawState::Edge));
        fEdgeAANumEdges = numEdges;
    }

    int getNumAAEdges() const { return fEdgeAANumEdges; }

    const Edge* getAAEdges() const { return fEdgeAAEdges; }

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
         * Modifies the behavior of edge AA specified by setEdgeAA. If set, 
         * will test edge pairs for convexity when rasterizing. Set this if the 
         * source polygon is non-convex.
         */
        kEdgeAAConcave_StateBit = 0x10,
        /**
         * Draws will apply the color matrix, otherwise the color matrix is
         * ignored.
         */
        kColorMatrix_StateBit   = 0x20,

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
     * @param flags   bitfield of StateBits specifing the states to enable
     */
    void enableState(uint32_t stateBits) {
        fFlagBits |= stateBits;
    }

    /**
     * Disable render state settings.
     *
     * @param flags   bitfield of StateBits specifing the states to disable
     */
    void disableState(uint32_t stateBits) {
        fFlagBits &= ~(stateBits);
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

    bool isConcaveEdgeAAState() const {
        return 0 != (fFlagBits & kEdgeAAConcave_StateBit);
    }

    bool isStateFlagEnabled(uint32_t stateBit) const {
        return 0 != (stateBit & fFlagBits);
    }

    void copyStateFlags(const GrDrawState& ds) {
        fFlagBits = ds.fFlagBits;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Face Culling
    ////

    enum DrawFace {
        kBoth_DrawFace,
        kCCW_DrawFace,
        kCW_DrawFace,
    };

    /**
     * Controls whether clockwise, counterclockwise, or both faces are drawn.
     * @param face  the face(s) to draw.
     */
    void setDrawFace(DrawFace face) {
        fDrawFace = face;
    }

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    DrawFace getDrawFace() const {
        return fDrawFace;
    }
    
    /// @}

    ///////////////////////////////////////////////////////////////////////////

    // Most stages are usually not used, so conditionals here
    // reduce the expected number of bytes touched by 50%.
    bool operator ==(const GrDrawState& s) const {
        if (memcmp(this, &s, this->leadingBytes())) return false;

        for (int i = 0; i < kNumStages; i++) {
            if (fTextures[i] &&
                memcmp(&this->fSamplerStates[i], &s.fSamplerStates[i],
                       sizeof(GrSamplerState))) {
                return false;
            }
        }

        return true;
    }
    bool operator !=(const GrDrawState& s) const { return !(*this == s); }

    // Most stages are usually not used, so conditionals here 
    // reduce the expected number of bytes touched by 50%.
    GrDrawState& operator =(const GrDrawState& s) {
        memcpy(this, &s, this->leadingBytes());

        for (int i = 0; i < kNumStages; i++) {
            if (s.fTextures[i]) {
                memcpy(&this->fSamplerStates[i], &s.fSamplerStates[i],
                       sizeof(GrSamplerState));
            }
        }

        return *this;
    }

private:
    static const StageMask kIllegalStageMaskBits = ~((1 << kNumStages)-1);
    uint8_t                 fFlagBits;
    GrBlendCoeff            fSrcBlend : 8;
    GrBlendCoeff            fDstBlend : 8;
    DrawFace                fDrawFace : 8;
    uint8_t                 fFirstCoverageStage;
    SkXfermode::Mode        fColorFilterMode : 8;
    GrColor                 fBlendConstant;
    GrTexture*              fTextures[kNumStages];
    GrRenderTarget*         fRenderTarget;
    GrColor                 fColor;
    GrColor                 fColorFilterColor;
    float                   fColorMatrix[20];
    GrStencilSettings       fStencilSettings;
    GrMatrix                fViewMatrix;
    // @{ Data for GrTesselatedPathRenderer
    // TODO: currently ignored in copying & comparison for performance.
    // Must be considered if GrTesselatedPathRenderer is being used.

    VertexEdgeType          fVertexEdgeType;
    int                     fEdgeAANumEdges;
    Edge                    fEdgeAAEdges[kMaxEdges];

    // @}
    // This field must be last; it will not be copied or compared
    // if the corresponding fTexture[] is NULL.
    GrSamplerState          fSamplerStates[kNumStages];

    size_t leadingBytes() const {
        // Can't use offsetof() with non-POD types, so stuck with pointer math.
        // TODO: ignores GrTesselatedPathRenderer data structures. We don't
        // have a compile-time flag that lets us know if it's being used, and
        // checking at runtime seems to cost 5% performance.
        return (size_t) ((unsigned char*)&fEdgeAANumEdges -
                         (unsigned char*)&fFlagBits);
    }

};

#endif
