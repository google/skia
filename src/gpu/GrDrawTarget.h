
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrDrawTarget_DEFINED
#define GrDrawTarget_DEFINED

#include "GrClip.h"
#include "GrColor.h"
#include "GrDrawState.h"
#include "GrMatrix.h"
#include "GrRefCnt.h"
#include "GrRenderTarget.h"
#include "GrSamplerState.h"
#include "GrStencil.h"
#include "GrTexture.h"

#include "SkXfermode.h"

class GrTexture;
class GrClipIterator;
class GrVertexBuffer;
class GrIndexBuffer;

class GrDrawTarget : public GrRefCnt {
public:
    /**
     * Represents the draw target capabilities.
     */
    struct Caps {
        Caps() { memset(this, 0, sizeof(Caps)); }
        Caps(const Caps& c) { *this = c; }
        Caps& operator= (const Caps& c) {
            memcpy(this, &c, sizeof(Caps));
            return *this;
        }
        void print() const;
        bool f8BitPaletteSupport        : 1;
        bool fNPOTTextureSupport        : 1;
        bool fNPOTTextureTileSupport    : 1;
        bool fNPOTRenderTargetSupport   : 1;
        bool fTwoSidedStencilSupport    : 1;
        bool fStencilWrapOpsSupport     : 1;
        bool fHWAALineSupport           : 1;
        bool fShaderSupport             : 1;
        bool fShaderDerivativeSupport   : 1;
        bool fGeometryShaderSupport     : 1;
        bool fFSAASupport               : 1;
        bool fDualSourceBlendingSupport : 1;
        bool fBufferLockSupport         : 1;
        bool fSupportPerVertexCoverage  : 1;
        int fMinRenderTargetWidth;
        int fMinRenderTargetHeight;
        int fMaxRenderTargetSize;
        int fMaxTextureSize;
    };

    /**
     *  Bitfield used to indicate which stages are in use.
     */
    typedef int StageBitfield;
    GR_STATIC_ASSERT(sizeof(StageBitfield)*8 >= GrDrawState::kNumStages);

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
        kEdgeAAConcave_StateBit =  0x10,
        // subclass may use additional bits internally
        kDummyStateBit,
        kLastPublicStateBit = kDummyStateBit-1
    };

    /**
     * Sets the stencil settings to use for the next draw.
     * Changing the clip has the side-effect of possibly zeroing
     * out the client settable stencil bits. So multipass algorithms
     * using stencil should not change the clip between passes.
     * @param settings  the stencil settings to use.
     */
    void setStencil(const GrStencilSettings& settings) {
        fCurrDrawState.fStencilSettings = settings;
    }

    /**
     * Shortcut to disable stencil testing and ops.
     */
    void disableStencil() {
        fCurrDrawState.fStencilSettings.setDisabled();
    }

public:
    ///////////////////////////////////////////////////////////////////////////

    GrDrawTarget();
    virtual ~GrDrawTarget();

    /**
     * Gets the capabilities of the draw target.
     */
    const Caps& getCaps() const { return fCaps; }

    /**
     * Sets the current clip to the region specified by clip. All draws will be
     * clipped against this clip if kClip_StateBit is enabled.
     *
     * Setting the clip may (or may not) zero out the client's stencil bits.
     *
     * @param description of the clipping region
     */
    void setClip(const GrClip& clip);

    /**
     * Gets the current clip.
     *
     * @return the clip.
     */
    const GrClip& getClip() const;

    /**
     * Sets the texture used at the next drawing call
     *
     * @param stage The texture stage for which the texture will be set
     *
     * @param texture The texture to set. Can be NULL though there is no advantage
     * to settings a NULL texture if doing non-textured drawing
     */
    void setTexture(int stage, GrTexture* texture);

    /**
     * Retrieves the currently set texture.
     *
     * @return    The currently set texture. The return value will be NULL if no
     *            texture has been set, NULL was most recently passed to
     *            setTexture, or the last setTexture was destroyed.
     */
    const GrTexture* getTexture(int stage) const;
    GrTexture* getTexture(int stage);

    /**
     * Sets the rendertarget used at the next drawing call
     *
     * @param target  The render target to set.
     */
    void setRenderTarget(GrRenderTarget* target);

    /**
     * Retrieves the currently set rendertarget.
     *
     * @return    The currently set render target.
     */
    const GrRenderTarget* getRenderTarget() const;
    GrRenderTarget* getRenderTarget();

    /**
     * Sets the sampler state for a stage used in subsequent draws.
     *
     * The sampler state determines how texture coordinates are
     * intepretted and used to sample the texture.
     *
     * @param stage           the stage of the sampler to set
     * @param samplerState    Specifies the sampler state.
     */
    void setSamplerState(int stage, const GrSamplerState& samplerState);

    /**
     * Concats the matrix of a stage's sampler.
     *
     * @param stage   the stage of the sampler to set
     * @param matrix  the matrix to concat
     */
    void preConcatSamplerMatrix(int stage, const GrMatrix& matrix)  {
        GrAssert(stage >= 0 && stage < GrDrawState::kNumStages);
        fCurrDrawState.fSamplerStates[stage].preConcatMatrix(matrix);
    }

    /**
     * Shortcut for preConcatSamplerMatrix on all stages in mask with same
     * matrix
     */
    void preConcatSamplerMatrices(int stageMask, const GrMatrix& matrix) {
        for (int i = 0; i < GrDrawState::kNumStages; ++i) {
            if ((1 << i) & stageMask) {
                this->preConcatSamplerMatrix(i, matrix);
            }
        }
    }

    /**
     * Shortcut for preConcatSamplerMatrix on all enabled stages in mask with
     * same matrix
     *
     * @param stage   the stage of the sampler to set
     * @param matrix  the matrix to concat
     */
    void preConcatEnabledSamplerMatrices(const GrMatrix& matrix) {
        StageBitfield stageMask = this->enabledStages();
        this->preConcatSamplerMatrices(stageMask, matrix);
    }

    /**
     * Gets the matrix of a stage's sampler
     *
     * @param stage     the stage to of sampler to get
     * @return the sampler state's matrix
     */
    const GrMatrix& getSamplerMatrix(int stage) const {
        return fCurrDrawState.fSamplerStates[stage].getMatrix();
    }

    /**
     * Sets the matrix of a stage's sampler
     *
     * @param stage     the stage of sampler set
     * @param matrix    the matrix to set
     */
    void setSamplerMatrix(int stage, const GrMatrix& matrix) {
        fCurrDrawState.fSamplerStates[stage].setMatrix(matrix);
    }

    /**
     * Sets the matrix applied to veretx positions.
     *
     * In the post-view-matrix space the rectangle [0,w]x[0,h]
     * fully covers the render target. (w and h are the width and height of the
     * the rendertarget.)
     *
     * @param m the matrix used to transform the vertex positions.
     */
    void setViewMatrix(const GrMatrix& m);

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
    void preConcatViewMatrix(const GrMatrix& m);

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
    void postConcatViewMatrix(const GrMatrix& m);

    /**
     * Retrieves the current view matrix
     * @return the current view matrix.
     */
    const GrMatrix& getViewMatrix() const;

    /**
     *  Retrieves the inverse of the current view matrix.
     *
     *  If the current view matrix is invertible, return true, and if matrix
     *  is non-null, copy the inverse into it. If the current view matrix is
     *  non-invertible, return false and ignore the matrix parameter.
     *
     * @param matrix if not null, will receive a copy of the current inverse.
     */
    bool getViewInverse(GrMatrix* matrix) const;

    /**
     *  Sets color for next draw to a premultiplied-alpha color.
     *
     *  @param the color to set.
     */
    void setColor(GrColor);

    /**
     * Gets the currently set color.
     * @return the current color.
     */
    GrColor getColor() const { return fCurrDrawState.fColor; }

    /**
     * Add a color filter that can be represented by a color and a mode.
     */
    void setColorFilter(GrColor, SkXfermode::Mode);

    /**
     *  Sets the color to be used for the next draw to be
     *  (r,g,b,a) = (alpha, alpha, alpha, alpha).
     *
     *  @param alpha The alpha value to set as the color.
     */
    void setAlpha(uint8_t alpha);

    /**
     * Controls whether clockwise, counterclockwise, or both faces are drawn.
     * @param face  the face(s) to draw.
     */
    void setDrawFace(GrDrawState::DrawFace face) {
        fCurrDrawState.fDrawFace = face;
    }

    /**
     * A common pattern is to compute a color with the initial stages and then
     * modulate that color by a coverage value in later stage(s) (AA, mask-
     * filters, glyph mask, etc). Color-filters, xfermodes, etc should be 
     * computed based on the pre-coverage-modulated color. The division of 
     * stages between color-computing and coverage-computing is specified by 
     * this method. Initially this is GrDrawState::kNumStages (all stages
     * are color-computing).
     */
    void setFirstCoverageStage(int firstCoverageStage) { 
        fCurrDrawState.fFirstCoverageStage = firstCoverageStage; 
    }

    /**
     * Gets the index of the first coverage-computing stage.
     */
    int getFirstCoverageStage() const { 
        return fCurrDrawState.fFirstCoverageStage; 
    }

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    GrDrawState::DrawFace getDrawFace() const {
        return fCurrDrawState.fDrawFace;
    }

    /**
     * Enable render state settings.
     *
     * @param flags   bitfield of StateBits specifing the states to enable
     */
    void enableState(uint32_t stateBits);

    /**
     * Disable render state settings.
     *
     * @param flags   bitfield of StateBits specifing the states to disable
     */
    void disableState(uint32_t stateBits);

    bool isDitherState() const {
        return 0 != (fCurrDrawState.fFlagBits & kDither_StateBit);
    }

    bool isHWAntialiasState() const {
        return 0 != (fCurrDrawState.fFlagBits & kHWAntialias_StateBit);
    }

    bool isClipState() const {
        return 0 != (fCurrDrawState.fFlagBits & kClip_StateBit);
    }

    bool isColorWriteDisabled() const {
        return 0 != (fCurrDrawState.fFlagBits & kNoColorWrites_StateBit);
    }

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
    void setBlendFunc(GrBlendCoeff srcCoeff, GrBlendCoeff dstCoeff);

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
    void setBlendConstant(GrColor constant) { fCurrDrawState.fBlendConstant = constant; }

    /**
     * Retrieves the last value set by setBlendConstant()
     * @return the blending constant value
     */
    GrColor getBlendConstant() const { return fCurrDrawState.fBlendConstant; }

    /**
     * Determines if blending will require a read of a dst given the current
     * state set on the draw target
     *
     * @return true if the dst surface will be read at each pixel hit by the
     *         a draw operation.
     */
    bool drawWillReadDst() const;

    /**
     * Color alpha and coverage are two inputs to the drawing pipeline. For some
     * blend modes it is safe to fold the coverage into constant or per-vertex
     * color alpha value. For other blend modes they must be handled separately.
     * Depending on features available in the underlying 3D API this may or may
     * not be possible.
     *
     * This function looks at the current blend on the draw target and the draw
     * target's capabilities to determine whether coverage can be handled
     * correctly.
     */
    bool canApplyCoverage() const;

    /**
     * Determines whether incorporating partial pixel coverage into the constant
     * color specified by setColor or per-vertex colors will give the right
     * blending result.
     */
    bool canTweakAlphaForCoverage() const;

     /**
      * Determines the interpretation per-vertex edge data when the
      * kEdge_VertexLayoutBit is set (see below). When per-vertex edges are not
      * specified the value of this setting has no effect.
      */
    void setVertexEdgeType(GrDrawState::VertexEdgeType type) {
        fCurrDrawState.fVertexEdgeType = type;
    }

    /**
     * Given the current draw state, vertex layout, and hw support, will HW AA
     * lines be used (if line primitive type is drawn)? (Note that lines are
     * always 1 pixel wide)
     */
    bool willUseHWAALines() const;

    /**
     * Sets the edge data required for edge antialiasing.
     *
     * @param edges       3 * 6 float values, representing the edge
     *                    equations in Ax + By + C form
     */
    void setEdgeAAData(const GrDrawState::Edge* edges, int numEdges);

    /**
     * Used to save and restore the GrGpu's drawing state
     */
    struct SavedDrawState {
    private:
        GrDrawState fState;
        friend class GrDrawTarget;
    };

    /**
     * Saves the current draw state. The state can be restored at a later time
     * with restoreDrawState.
     *
     * See also AutoStateRestore class.
     *
     * @param   state will hold the state after the function returns.
     */
    void saveCurrentDrawState(SavedDrawState* state) const;

    /**
     * Restores previously saved draw state. The client guarantees that state
     * was previously passed to saveCurrentDrawState and that the rendertarget
     * and texture set at save are still valid.
     *
     * See also AutoStateRestore class.
     *
     * @param   state the previously saved state to restore.
     */
    void restoreDrawState(const SavedDrawState& state);

    /**
     * Copies the draw state from another target to this target.
     *
     * @param srcTarget     draw target used as src of the draw state.
     */
    void copyDrawState(const GrDrawTarget& srcTarget);

    /**
     * The format of vertices is represented as a bitfield of flags.
     * Flags that indicate the layout of vertex data. Vertices always contain
     * positions and may also contain up to GrDrawState::kMaxTexCoords sets
     * of 2D texture * coordinates, per-vertex colors, and per-vertex coverage.
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
     * @param stage       the stage that will use texture coordinates.
     * @param texCoordIdx the index of the texture coordinates to use
     *
     * @return the bit to add to a GrVertexLayout bitfield.
     */
    static int StageTexCoordVertexLayoutBit(int stage, int texCoordIdx) {
        GrAssert(stage < GrDrawState::kNumStages);
        GrAssert(texCoordIdx < GrDrawState::kMaxTexCoords);
        return 1 << (stage + (texCoordIdx * GrDrawState::kNumStages));
    }

private:
    static const int TEX_COORD_BIT_CNT = GrDrawState::kNumStages *
                                         GrDrawState::kMaxTexCoords;

public:
    /**
     * Generates a bit indicating that a texture stage uses the position
     * as its texture coordinate.
     *
     * @param stage       the stage that will use position as texture
     *                    coordinates.
     *
     * @return the bit to add to a GrVertexLayout bitfield.
     */
    static int StagePosAsTexCoordVertexLayoutBit(int stage) {
        GrAssert(stage < GrDrawState::kNumStages);
        return (1 << (TEX_COORD_BIT_CNT + stage));
    }

private:
    static const int STAGE_BIT_CNT = TEX_COORD_BIT_CNT +
        GrDrawState::kNumStages;

public:

    /**
     * Additional Bits that can be specified in GrVertexLayout.
     */
    enum VertexLayoutBits {
        /* vertices have colors (GrColor) */
        kColor_VertexLayoutBit              = 1 << (STAGE_BIT_CNT + 0),
        /* vertices have coverage (GrColor where all channels should have the 
         * same value)
         */
        kCoverage_VertexLayoutBit           = 1 << (STAGE_BIT_CNT + 1),
        /* Use text vertices. (Pos and tex coords may be a different type for
         * text [GrGpuTextVertex vs GrPoint].)
         */
        kTextFormat_VertexLayoutBit         = 1 << (STAGE_BIT_CNT + 2),

        /* Each vertex specificies an edge. Distance to the edge is used to
         * compute a coverage. See setVertexEdgeType().
         */
        kEdge_VertexLayoutBit               = 1 << (STAGE_BIT_CNT + 3),
        // for below assert
        kDummyVertexLayoutBit,
        kHighVertexLayoutBit = kDummyVertexLayoutBit - 1
    };
    // make sure we haven't exceeded the number of bits in GrVertexLayout.
    GR_STATIC_ASSERT(kHighVertexLayoutBit < ((uint64_t)1 << 8*sizeof(GrVertexLayout)));

    /**
     * There are three methods for specifying geometry (vertices and optionally
     * indices) to the draw target. When indexed drawing the indices and vertices
     * can use a different method. Once geometry is specified it can be used for
     * multiple drawIndexed and drawNonIndexed calls.
     *
     * Sometimes it is necessary to perform a draw while upstack code has
     * already specified geometry that it isn't finished with. There are push
     * pop methods
     *
     * 1. Provide a cpu array (set*SourceToArray). This is useful when the
     *    caller's client has already provided vertex data in a format
     *    the time compatible with a GrVertexLayout. The array must contain the
     *    data at set*SourceToArray is called. The source stays in effect for
     *    drawIndexed & drawNonIndexed calls until set*SourceToArray is called
     *    again or one of the other two paths is chosen.
     *
     * 2. Reserve. This is most useful when the caller has data it must
     *    transform before drawing and is not long-lived. The caller requests
     *    that the draw target make room for some amount of vertex and/or index
     *    data. The target provides ptrs to hold the vertex and/or index data.
     *
     *    The data is writable up until the next drawIndexed, drawNonIndexed, 
     *    or pushGeometrySource At this point the data is frozen and the ptrs
     *    are no longer valid.
     *
     * 3. Vertex and Index Buffers. This is most useful for geometry that will
     *    is long-lived. SetVertexSourceToBuffer and SetIndexSourceToBuffer are
     *    used to set the buffer and subsequent drawIndexed and drawNonIndexed 
     *    calls use this source until another source is set.
     */

    /**
     * Reserves space for vertices. Draw target will use reserved vertices at
     * at the next draw.
     *
     * If succeeds:
     *          if vertexCount > 0, *vertices will be the array
     *          of vertices to be filled by caller. The next draw will read
     *          these vertices.
     *
     * If a client does not already have a vertex buffer then this is the
     * preferred way to allocate vertex data. It allows the subclass of
     * GrDrawTarget to decide whether to put data in buffers, to group vertex
     * data that uses the same state (e.g. for deferred rendering), etc.
     *
     * After the next draw or pushGeometrySource the vertices ptr is no longer
     * valid and the geometry data cannot be further modified. The contents
     * that were put in the reserved space can be drawn by multiple draws,
     * however.
     *
     * @param vertexLayout the format of vertices (ignored if vertexCount == 0).
     * @param vertexCount  the number of vertices to reserve space for. Can be 0.
     * @param vertices     will point to reserved vertex space if vertexCount is
     *                     non-zero. Illegal to pass NULL if vertexCount > 0.
     *
     * @return  true if succeeded in allocating space for the vertices and false
     *               if not.
     */
    bool reserveVertexSpace(GrVertexLayout vertexLayout,
                            int vertexCount,
                            void** vertices);
    /**
     * Reserves space for indices. Draw target will use the reserved indices at
     * the next indexed draw.
     *
     * If succeeds:
     *          if indexCount > 0, *indices will be the array
     *          of indices to be filled by caller. The next draw will read
     *          these indices.
     *
     * If a client does not already have a index buffer then this is the
     * preferred way to allocate index data. It allows the subclass of
     * GrDrawTarget to decide whether to put data in buffers, to group index
     * data that uses the same state (e.g. for deferred rendering), etc.
     *
     * After the next indexed draw or pushGeometrySource the indices ptr is no
     * longer valid and the geometry data cannot be further modified. The
     * contents that were put in the reserved space can be drawn by multiple
     * draws, however.
     *
     * @param indexCount   the number of indices to reserve space for. Can be 0.
     * @param indices      will point to reserved index space if indexCount is
     *                     non-zero. Illegal to pass NULL if indexCount > 0.
     */

    bool reserveIndexSpace(int indexCount, void** indices);
    /**
     * Provides hints to caller about the number of vertices and indices
     * that can be allocated cheaply. This can be useful if caller is reserving
     * space but doesn't know exactly how much geometry is needed.
     *
     * Also may hint whether the draw target should be flushed first. This is
     * useful for deferred targets.
     *
     * @param vertexLayout layout of vertices caller would like to reserve
     * @param vertexCount  in: hint about how many vertices the caller would
     *                     like to allocate.
     *                     out: a hint about the number of vertices that can be
     *                     allocated cheaply. Negative means no hint.
     *                     Ignored if NULL.
     * @param indexCount   in: hint about how many indices the caller would
     *                     like to allocate.
     *                     out: a hint about the number of indices that can be
     *                     allocated cheaply. Negative means no hint.
     *                     Ignored if NULL.
     *
     * @return  true if target should be flushed based on the input values.
     */
    virtual bool geometryHints(GrVertexLayout vertexLayout,
                               int* vertexCount,
                               int* indexCount) const;

    /**
     * Sets source of vertex data for the next draw. Array must contain
     * the vertex data when this is called.
     *
     * @param array         cpu array containing vertex data.
     * @param size          size of the vertex data.
     * @param vertexCount   the number of vertices in the array.
     */
    void setVertexSourceToArray(GrVertexLayout vertexLayout,
                                const void* vertexArray,
                                int vertexCount);

    /**
     * Sets source of index data for the next indexed draw. Array must contain
     * the indices when this is called.
     *
     * @param array         cpu array containing index data.
     * @param indexCount    the number of indices in the array.
     */
    void setIndexSourceToArray(const void* indexArray, int indexCount);

    /**
     * Sets source of vertex data for the next draw. Data does not have to be
     * in the buffer until drawIndexed or drawNonIndexed.
     *
     * @param buffer        vertex buffer containing vertex data. Must be
     *                      unlocked before draw call.
     * @param vertexLayout  layout of the vertex data in the buffer.
     */
    void setVertexSourceToBuffer(GrVertexLayout vertexLayout,
                                 const GrVertexBuffer* buffer);

    /**
     * Sets source of index data for the next indexed draw. Data does not have
     * to be in the buffer until drawIndexed or drawNonIndexed.
     *
     * @param buffer index buffer containing indices. Must be unlocked
     *               before indexed draw call.
     */
    void setIndexSourceToBuffer(const GrIndexBuffer* buffer);
    
    /**
     * Resets vertex source. Drawing from reset vertices is illegal. Set vertex
     * source to reserved, array, or buffer before next draw. May be able to free
     * up temporary storage allocated by setVertexSourceToArray or
     * reserveVertexSpace.
     */
    void resetVertexSource();
    
    /**
     * Resets index source. Indexed Drawing from reset indices is illegal. Set
     * index source to reserved, array, or buffer before next indexed draw. May
     * be able to free up temporary storage allocated by setIndexSourceToArray
     * or reserveIndexSpace.
     */
    void resetIndexSource(); 

    /**
     * Pushes and resets the vertex/index sources. Any reserved vertex / index
     * data is finalized (i.e. cannot be updated after the matching pop but can
     * be drawn from). Must be balanced by a pop.
     */
    void pushGeometrySource();

    /**
     * Pops the vertex / index sources from the matching push.
     */
    void popGeometrySource();
    
    /**
     * Draws indexed geometry using the current state and current vertex / index
     * sources.
     *
     * @param type         The type of primitives to draw.
     * @param startVertex  the vertex in the vertex array/buffer corresponding
     *                     to index 0
     * @param startIndex   first index to read from index src.
     * @param vertexCount  one greater than the max index.
     * @param indexCount   the number of index elements to read. The index count
     *                     is effectively trimmed to the last completely
     *                     specified primitive.
     */
    void drawIndexed(GrPrimitiveType type,
                     int startVertex,
                     int startIndex,
                     int vertexCount,
                     int indexCount);

    /**
     * Draws non-indexed geometry using the current state and current vertex
     * sources.
     *
     * @param type         The type of primitives to draw.
     * @param startVertex  the vertex in the vertex array/buffer corresponding
     *                     to index 0
     * @param vertexCount  one greater than the max index.
     */
    void drawNonIndexed(GrPrimitiveType type,
                        int startVertex,
                        int vertexCount);

    /**
     * Helper function for drawing rects. This does not use the current index
     * and vertex sources. After returning, the vertex and index sources may
     * have changed. They should be reestablished before the next drawIndexed
     * or drawNonIndexed. This cannot be called between reserving and releasing
     * geometry. The GrDrawTarget subclass may be able to perform additional
     * optimizations if drawRect is used rather than drawIndexed or
     * drawNonIndexed.
     * @param rect      the rect to draw
     * @param matrix    optional matrix applied to rect (before viewMatrix)
     * @param stageEnableBitfield bitmask indicating which stages are enabled.
     *                            Bit i indicates whether stage i is enabled.
     * @param srcRects  specifies rects for stages enabled by stageEnableMask.
     *                  if stageEnableMask bit i is 1, srcRects is not NULL,
     *                  and srcRects[i] is not NULL, then srcRects[i] will be
     *                  used as coordinates for stage i. Otherwise, if stage i
     *                  is enabled then rect is used as the coordinates.
     * @param srcMatrices   optional matrices applied to srcRects. If
     *                      srcRect[i] is non-NULL and srcMatrices[i] is
     *                      non-NULL then srcRect[i] will be transformed by
     *                      srcMatrix[i]. srcMatrices can be NULL when no
     *                      srcMatrices are desired.
     */
    virtual void drawRect(const GrRect& rect,
                          const GrMatrix* matrix,
                          StageBitfield stageEnableBitfield,
                          const GrRect* srcRects[],
                          const GrMatrix* srcMatrices[]);

    /**
     * Helper for drawRect when the caller doesn't need separate src rects or
     * matrices.
     */
    void drawSimpleRect(const GrRect& rect,
                        const GrMatrix* matrix,
                        StageBitfield stageEnableBitfield) {
         drawRect(rect, matrix, stageEnableBitfield, NULL, NULL);
    }

    /**
     * Clear the render target. Ignores the clip and all other draw state
     * (blend mode, stages, etc). Clears the whole thing if rect is NULL,
     * otherwise just the rect.
     */
    virtual void clear(const GrIRect* rect, GrColor color) = 0;

    /**
     * Returns the maximum number of edges that may be specified in a single
     * draw call when performing edge antialiasing.  This is usually limited
     * by the number of fragment uniforms which may be uploaded.  Must be a
     * minimum of six, since a triangle's vertices each belong to two boundary
     * edges which may be distinct.
     */
    virtual int getMaxEdges() const { return 6; }

    ////////////////////////////////////////////////////////////////////////////

    class AutoStateRestore : ::GrNoncopyable {
    public:
        AutoStateRestore();
        AutoStateRestore(GrDrawTarget* target);
        ~AutoStateRestore();

        /**
         * if this object is already saving state for param target then
         * this does nothing. Otherise, it restores previously saved state on
         * previous target (if any) and saves current state on param target.
         */
        void set(GrDrawTarget* target);

    private:
        GrDrawTarget*       fDrawTarget;
        SavedDrawState      fDrawState;
    };

    ////////////////////////////////////////////////////////////////////////////

    class AutoViewMatrixRestore : ::GrNoncopyable {
    public:
        AutoViewMatrixRestore() {
            fDrawTarget = NULL;
        }

        AutoViewMatrixRestore(GrDrawTarget* target)
            : fDrawTarget(target), fMatrix(fDrawTarget->getViewMatrix()) {
            GrAssert(NULL != target);
        }

        void set(GrDrawTarget* target) {
            GrAssert(NULL != target);
            if (NULL != fDrawTarget) {
                fDrawTarget->setViewMatrix(fMatrix);
            }
            fDrawTarget = target;
            fMatrix = target->getViewMatrix();
        }

        ~AutoViewMatrixRestore() {
            if (NULL != fDrawTarget) {
                fDrawTarget->setViewMatrix(fMatrix);
            }
        }

    private:
        GrDrawTarget*       fDrawTarget;
        GrMatrix            fMatrix;
    };

    ////////////////////////////////////////////////////////////////////////////

    /** 
     * Sets the view matrix to I and preconcats all stage matrices enabled in
     * mask by the view inverse. Destructor undoes these changes.
     */
    class AutoDeviceCoordDraw : ::GrNoncopyable {
    public:
        AutoDeviceCoordDraw(GrDrawTarget* target, int stageMask);
        ~AutoDeviceCoordDraw();
    private:
        GrDrawTarget*       fDrawTarget;
        GrMatrix            fViewMatrix;
        GrMatrix            fSamplerMatrices[GrDrawState::kNumStages];
        int                 fStageMask;
    };

    ////////////////////////////////////////////////////////////////////////////

    class AutoReleaseGeometry : ::GrNoncopyable {
    public:
        AutoReleaseGeometry(GrDrawTarget*  target,
                            GrVertexLayout vertexLayout,
                            int            vertexCount,
                            int            indexCount);
        AutoReleaseGeometry();
        ~AutoReleaseGeometry();
        bool set(GrDrawTarget*  target,
                 GrVertexLayout vertexLayout,
                 int            vertexCount,
                 int            indexCount);
        bool succeeded() const { return NULL != fTarget; }
        void* vertices() const { GrAssert(this->succeeded()); return fVertices; }
        void* indices() const { GrAssert(this->succeeded()); return fIndices; }
        GrPoint* positions() const {
            return static_cast<GrPoint*>(this->vertices());
        }

    private:
        void reset();
        
        GrDrawTarget* fTarget;
        void*         fVertices;
        void*         fIndices;
    };

    ////////////////////////////////////////////////////////////////////////////

    class AutoClipRestore : ::GrNoncopyable {
    public:
        AutoClipRestore(GrDrawTarget* target) {
            fTarget = target;
            fClip = fTarget->getClip();
        }

        ~AutoClipRestore() {
            fTarget->setClip(fClip);
        }
    private:
        GrDrawTarget* fTarget;
        GrClip        fClip;
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    class AutoGeometryPush : ::GrNoncopyable {
    public:
        AutoGeometryPush(GrDrawTarget* target) {
            GrAssert(NULL != target);
            fTarget = target;
            target->pushGeometrySource();
        }
        ~AutoGeometryPush() {
            fTarget->popGeometrySource();
        }
    private:
        GrDrawTarget* fTarget;
    };

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
     * @param stage         the stage to query
     * @param vertexLayout  layout to query
     *
     * @return the texture coordinate index or -1 if the stage doesn't use
     *         separate (non-position) texture coordinates.
     */
    static int VertexTexCoordsForStage(int stage, GrVertexLayout vertexLayout);

    /**
     * Helper function to compute the offset of texture coordinates in a vertex
     * @return offset of texture coordinates in vertex layout or -1 if the
     *         layout has no texture coordinates. Will be 0 if positions are
     *         used as texture coordinates for the stage.
     */
    static int VertexStageCoordOffset(int stage, GrVertexLayout vertexLayout);

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
     * Helper function to determine if vertex layout contains either explicit or
     * implicit texture coordinates for a stage.
     *
     * @param stage         the stage to query
     * @param vertexLayout  layout to query
     *
     * @return true if vertex specifies texture coordinates for the stage,
     *              false otherwise.
     */
    static bool VertexUsesStage(int stage, GrVertexLayout vertexLayout);

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
                   int texCoordOffsetsByIdx[GrDrawState::kMaxTexCoords],
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
                   int texCoordOffsetsByStage[GrDrawState::kNumStages],
                   int* colorOffset,
                   int* coverageOffset,
                   int* edgeOffset);

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

protected:

    /**
     * Optimizations for blending / coverage to be applied based on the current
     * state.
     * Subclasses that actually draw (as opposed to those that just buffer for
     * playback) must implement the flags that replace the output color.
     */
    enum BlendOptFlags {
        /**
         * No optimization
         */
        kNone_BlendOpt = 0,
        /**
         * Don't draw at all
         */
        kSkipDraw_BlendOptFlag = 0x2,
        /**
         * Emit the src color, disable HW blending (replace dst with src)
         */
        kDisableBlend_BlendOptFlag = 0x4,
        /**
         * The coverage value does not have to be computed separately from
         * alpha, the the output color can be the modulation of the two.
         */
        kCoverageAsAlpha_BlendOptFlag = 0x1,
        /**
         * Instead of emitting a src color, emit coverage in the alpha channel
         * and r,g,b are "don't cares".
         */
        kEmitCoverage_BlendOptFlag = 0x10,
        /**
         * Emit transparent black instead of the src color, no need to compute
         * coverage.
         */
        kEmitTransBlack_BlendOptFlag = 0x8,
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(BlendOptFlags);

    // Determines what optimizations can be applied based on the blend.
    // The coeffecients may have to be tweaked in order for the optimization
    // to work. srcCoeff and dstCoeff are optional params that receive the
    // tweaked coeffecients.
    // Normally the function looks at the current state to see if coverage
    // is enabled. By setting forceCoverage the caller can speculatively
    // determine the blend optimizations that would be used if there was
    // partial pixel coverage
    BlendOptFlags getBlendOpts(bool forceCoverage = false,
                               GrBlendCoeff* srcCoeff = NULL,
                               GrBlendCoeff* dstCoeff = NULL) const;

    // determine if src alpha is guaranteed to be one for all src pixels
    bool srcAlphaWillBeOne() const;

    enum GeometrySrcType {
        kNone_GeometrySrcType,     //<! src has not been specified
        kReserved_GeometrySrcType, //<! src was set using reserve*Space
        kArray_GeometrySrcType,    //<! src was set using set*SourceToArray
        kBuffer_GeometrySrcType    //<! src was set using set*SourceToBuffer
    };
    
    struct GeometrySrcState {
        GeometrySrcType         fVertexSrc;
        union {
            // valid if src type is buffer
            const GrVertexBuffer*   fVertexBuffer;
            // valid if src type is reserved or array
            int                     fVertexCount;
        };
        
        GeometrySrcType         fIndexSrc;
        union {
            // valid if src type is buffer
            const GrIndexBuffer*    fIndexBuffer;
            // valid if src type is reserved or array
            int                     fIndexCount;
        };
        
        GrVertexLayout          fVertexLayout;
    };
    
    // given a vertex layout and a draw state, will a stage be used?
    static bool StageWillBeUsed(int stage, GrVertexLayout layout, 
                                const GrDrawState& state) {
        return NULL != state.fTextures[stage] && VertexUsesStage(stage, layout);
    }

    bool isStageEnabled(int stage) const {
        return StageWillBeUsed(stage, this->getGeomSrc().fVertexLayout, 
                               fCurrDrawState);
    }

    StageBitfield enabledStages() const {
        StageBitfield mask = 0;
        for (int s = 0; s < GrDrawState::kNumStages; ++s) {
            mask |= this->isStageEnabled(s) ? 1 : 0;
        }
        return mask;
    }

    // Helpers for GrDrawTarget subclasses that won't have private access to
    // SavedDrawState but need to peek at the state values.
    static GrDrawState& accessSavedDrawState(SavedDrawState& sds)
                                                        { return sds.fState; }
    static const GrDrawState& accessSavedDrawState(const SavedDrawState& sds)
                                                        { return sds.fState; }

    // implemented by subclass to allocate space for reserved geom
    virtual bool onReserveVertexSpace(GrVertexLayout vertexLayout,
                                      int vertexCount,
                                      void** vertices) = 0;
    virtual bool onReserveIndexSpace(int indexCount, void** indices) = 0;
    // implemented by subclass to handle release of reserved geom space
    virtual void releaseReservedVertexSpace() = 0;
    virtual void releaseReservedIndexSpace() = 0;
    // subclass must consume array contents when set
    virtual void onSetVertexSourceToArray(const void* vertexArray,
                                          int vertexCount) = 0;
    virtual void onSetIndexSourceToArray(const void* indexArray,
                                         int indexCount) = 0;
    // subclass is notified that geom source will be set away from an array
    virtual void releaseVertexArray() = 0;
    virtual void releaseIndexArray() = 0;
    // subclass overrides to be notified just before geo src state
    // is pushed/popped.
    virtual void geometrySourceWillPush() = 0;
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState) = 0;
    // subclass called to perform drawing
    virtual void onDrawIndexed(GrPrimitiveType type,
                               int startVertex,
                               int startIndex,
                               int vertexCount,
                               int indexCount) = 0;
    virtual void onDrawNonIndexed(GrPrimitiveType type,
                                  int startVertex,
                                  int vertexCount) = 0;
    // subclass overrides to be notified when clip is set. Must call
    // INHERITED::clipwillBeSet
    virtual void clipWillBeSet(const GrClip& clip);

    // Helpers for drawRect, protected so subclasses that override drawRect
    // can use them.
    static GrVertexLayout GetRectVertexLayout(StageBitfield stageEnableBitfield,
                                              const GrRect* srcRects[]);

    static void SetRectVertices(const GrRect& rect,
                                const GrMatrix* matrix,
                                const GrRect* srcRects[],
                                const GrMatrix* srcMatrices[],
                                GrVertexLayout layout,
                                void* vertices);

    // accessor for derived classes
    const GeometrySrcState& getGeomSrc() const {
        return fGeoSrcStateStack.back();
    }

    GrClip fClip;

    GrDrawState fCurrDrawState;

    Caps fCaps;

private:
    // called when setting a new vert/idx source to unref prev vb/ib
    void releasePreviousVertexSource();
    void releasePreviousIndexSource();
    
    enum {
        kPreallocGeoSrcStateStackCnt = 4,
    };
    SkSTArray<kPreallocGeoSrcStateStackCnt, 
              GeometrySrcState, true>           fGeoSrcStateStack;
    
};

GR_MAKE_BITFIELD_OPS(GrDrawTarget::BlendOptFlags);

#endif
