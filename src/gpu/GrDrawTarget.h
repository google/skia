
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrDrawTarget_DEFINED
#define GrDrawTarget_DEFINED

#include "GrClipData.h"
#include "GrDrawState.h"
#include "GrIndexBuffer.h"
#include "SkMatrix.h"
#include "GrRefCnt.h"

#include "SkClipStack.h"
#include "SkPath.h"
#include "SkTLazy.h"
#include "SkTArray.h"
#include "SkXfermode.h"

class GrClipData;
class GrPath;
class GrVertexBuffer;
class SkStrokeRec;

class GrDrawTarget : public GrRefCnt {
protected:
    /** This helper class allows GrDrawTarget subclasses to set the caps values without having to be
        made a friend of GrDrawTarget::Caps. */
    class CapsInternals {
    public:
        bool f8BitPaletteSupport        : 1;
        bool fNPOTTextureTileSupport    : 1;
        bool fTwoSidedStencilSupport    : 1;
        bool fStencilWrapOpsSupport     : 1;
        bool fHWAALineSupport           : 1;
        bool fShaderDerivativeSupport   : 1;
        bool fGeometryShaderSupport     : 1;
        bool fFSAASupport               : 1;
        bool fDualSourceBlendingSupport : 1;
        bool fBufferLockSupport         : 1;
        bool fPathStencilingSupport     : 1;
        int fMaxRenderTargetSize;
        int fMaxTextureSize;
    };

    class DrawInfo;

public:
    SK_DECLARE_INST_COUNT(GrDrawTarget)

    /**
     * Represents the draw target capabilities.
     */
    class Caps {
    public:
        Caps() { memset(this, 0, sizeof(Caps)); }
        Caps(const Caps& c) { *this = c; }
        Caps& operator= (const Caps& c) {
            memcpy(this, &c, sizeof(Caps));
            return *this;
        }
        void print() const;

        bool eightBitPaletteSupport() const { return fInternals.f8BitPaletteSupport; }
        bool npotTextureTileSupport() const { return fInternals.fNPOTTextureTileSupport; }
        bool twoSidedStencilSupport() const { return fInternals.fTwoSidedStencilSupport; }
        bool stencilWrapOpsSupport() const { return  fInternals.fStencilWrapOpsSupport; }
        bool hwAALineSupport() const { return fInternals.fHWAALineSupport; }
        bool shaderDerivativeSupport() const { return fInternals.fShaderDerivativeSupport; }
        bool geometryShaderSupport() const { return fInternals.fGeometryShaderSupport; }
        bool fsaaSupport() const { return fInternals.fFSAASupport; }
        bool dualSourceBlendingSupport() const { return fInternals.fDualSourceBlendingSupport; }
        bool bufferLockSupport() const { return fInternals.fBufferLockSupport; }
        bool pathStencilingSupport() const { return fInternals.fPathStencilingSupport; }

        int maxRenderTargetSize() const { return fInternals.fMaxRenderTargetSize; }
        int maxTextureSize() const { return fInternals.fMaxTextureSize; }
    private:
        CapsInternals fInternals;
        friend class GrDrawTarget; // to set values of fInternals
    };

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
    void setClip(const GrClipData* clip);

    /**
     * Gets the current clip.
     *
     * @return the clip.
     */
    const GrClipData* getClip() const;

    /**
     * Sets the draw state object for the draw target. Note that this does not
     * make a copy. The GrDrawTarget will take a reference to passed object.
     * Passing NULL will cause the GrDrawTarget to use its own internal draw
     * state object rather than an externally provided one.
     */
    void setDrawState(GrDrawState*  drawState);

    /**
     * Read-only access to the GrDrawTarget's current draw state.
     */
    const GrDrawState& getDrawState() const { return *fDrawState; }

    /**
     * Read-write access to the GrDrawTarget's current draw state. Note that
     * this doesn't ref.
     */
    GrDrawState* drawState() { return fDrawState; }

    /**
     * Color alpha and coverage are two inputs to the drawing pipeline. For some
     * blend modes it is safe to fold the coverage into constant or per-vertex
     * color alpha value. For other blend modes they must be handled separately.
     * Depending on features available in the underlying 3D API this may or may
     * not be possible.
     *
     * This function considers the current draw state and the draw target's
     * capabilities to determine whether coverage can be handled correctly. The
     * following assumptions are made:
     *    1. The caller intends to somehow specify coverage. This can be
     *       specified either by enabling a coverage stage on the GrDrawState or
     *       via the vertex layout.
     *    2. Other than enabling coverage stages, the current configuration of
     *       the target's GrDrawState is as it will be at draw time.
     *    3. If a vertex source has not yet been specified then all stages with
     *       non-NULL textures will be referenced by the vertex layout.
     */
    bool canApplyCoverage() const;

    /**
     * Determines whether incorporating partial pixel coverage into the constant
     * color specified by setColor or per-vertex colors will give the right
     * blending result. If a vertex source has not yet been specified then
     * the function assumes that all stages with non-NULL textures will be
     * referenced by the vertex layout.
     */
    bool canTweakAlphaForCoverage() const;

    /**
     * Given the current draw state and hw support, will HW AA lines be used
     * (if line primitive type is drawn)? If a vertex source has not yet been
     * specified then  the function assumes that all stages with non-NULL
     * textures will be referenced by the vertex layout.
     */
    bool willUseHWAALines() const;

    /**
     * There are three types of "sources" of geometry (vertices and indices) for
     * draw calls made on the target. When performing an indexed draw, the
     * indices and vertices can use different source types. Once a source is
     * specified it can be used for multiple draws. However, the time at which
     * the geometry data is no longer editable depends on the source type.
     *
     * Sometimes it is necessary to perform a draw while upstack code has
     * already specified geometry that it isn't finished with. So there are push
     * and pop methods. This allows the client to push the sources, draw
     * something using alternate sources, and then pop to restore the original
     * sources.
     *
     * Aside from pushes and pops, a source remains valid until another source
     * is set or resetVertexSource / resetIndexSource is called. Drawing from
     * a reset source is an error.
     *
     * The three types of sources are:
     *
     * 1. A cpu array (set*SourceToArray). This is useful when the caller
     *    already provided vertex data in a format compatible with a
     *    GrVertexLayout. The data in the array is consumed at the time that
     *    set*SourceToArray is called and subsequent edits to the array will not
     *    be reflected in draws.
     *
     * 2. Reserve. This is most useful when the caller has data it must
     *    transform before drawing and is not long-lived. The caller requests
     *    that the draw target make room for some amount of vertex and/or index
     *    data. The target provides ptrs to hold the vertex and/or index data.
     *
     *    The data is writable up until the next drawIndexed, drawNonIndexed,
     *    drawIndexedInstances, or pushGeometrySource. At this point the data is
     *    frozen and the ptrs are no longer valid.
     *
     *    Where the space is allocated and how it is uploaded to the GPU is
     *    subclass-dependent.
     *
     * 3. Vertex and Index Buffers. This is most useful for geometry that will
     *    is long-lived. When the data in the buffer is consumed depends on the
     *    GrDrawTarget subclass. For deferred subclasses the caller has to
     *    guarantee that the data is still available in the buffers at playback.
     *    (TODO: Make this more automatic as we have done for read/write pixels)
     */

    /**
     * Reserves space for vertices and/or indices. Zero can be specifed as
     * either the vertex or index count if the caller desires to only reserve
     * space for only indices or only vertices. If zero is specifed for
     * vertexCount then the vertex source will be unmodified and likewise for
     * indexCount.
     *
     * If the function returns true then the reserve suceeded and the vertices
     * and indices pointers will point to the space created.
     *
     * If the target cannot make space for the request then this function will
     * return false. If vertexCount was non-zero then upon failure the vertex
     * source is reset and likewise for indexCount.
     *
     * The pointers to the space allocated for vertices and indices remain valid
     * until a drawIndexed, drawNonIndexed, drawIndexedInstances, or push/
     * popGeomtrySource is called. At that point logically a snapshot of the
     * data is made and the pointers are invalid.
     *
     * @param vertexLayout the format of vertices (ignored if vertexCount == 0).
     * @param vertexCount  the number of vertices to reserve space for. Can be
     *                     0.
     * @param indexCount   the number of indices to reserve space for. Can be 0.
     * @param vertices     will point to reserved vertex space if vertexCount is
     *                     non-zero. Illegal to pass NULL if vertexCount > 0.
     * @param indices      will point to reserved index space if indexCount is
     *                     non-zero. Illegal to pass NULL if indexCount > 0.
     */
     bool reserveVertexAndIndexSpace(GrVertexLayout vertexLayout,
                                     int vertexCount,
                                     int indexCount,
                                     void** vertices,
                                     void** indices);

    /**
     * Provides hints to caller about the number of vertices and indices
     * that can be allocated cheaply. This can be useful if caller is reserving
     * space but doesn't know exactly how much geometry is needed.
     *
     * Also may hint whether the draw target should be flushed first. This is
     * useful for deferred targets.
     *
     * @param vertexSize   size of vertices caller would like to reserve
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
    virtual bool geometryHints(size_t vertexSize,
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
     * in the buffer until drawIndexed, drawNonIndexed, or drawIndexedInstances.
     *
     * @param buffer        vertex buffer containing vertex data. Must be
     *                      unlocked before draw call.
     * @param vertexLayout  layout of the vertex data in the buffer.
     */
    void setVertexSourceToBuffer(GrVertexLayout vertexLayout,
                                 const GrVertexBuffer* buffer);

    /**
     * Sets source of index data for the next indexed draw. Data does not have
     * to be in the buffer until drawIndexed.
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
     * Query to find out if the vertex or index source is reserved.
     */
    bool hasReservedVerticesOrIndices() const {
        return kReserved_GeometrySrcType == this->getGeomSrc().fVertexSrc ||
        kReserved_GeometrySrcType == this->getGeomSrc().fIndexSrc;
    }

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
     * @param devBounds    optional bounds hint. This is a promise from the caller,
     *                     not a request for clipping.
     */
    void drawIndexed(GrPrimitiveType type,
                     int startVertex,
                     int startIndex,
                     int vertexCount,
                     int indexCount,
                     const SkRect* devBounds = NULL);

    /**
     * Draws non-indexed geometry using the current state and current vertex
     * sources.
     *
     * @param type         The type of primitives to draw.
     * @param startVertex  the vertex in the vertex array/buffer corresponding
     *                     to index 0
     * @param vertexCount  one greater than the max index.
     * @param devBounds    optional bounds hint. This is a promise from the caller,
     *                     not a request for clipping.
     */
    void drawNonIndexed(GrPrimitiveType type,
                        int startVertex,
                        int vertexCount,
                        const SkRect* devBounds = NULL);

    /**
     * Draws path into the stencil buffer. The fill must be either even/odd or
     * winding (not inverse or hairline). It will respect the HW antialias flag
     * on the draw state (if possible in the 3D API).
     */
    void stencilPath(const GrPath*, const SkStrokeRec& stroke, SkPath::FillType fill);

    /**
     * Helper function for drawing rects. This does not use the current index
     * and vertex sources. After returning, the vertex and index sources may
     * have changed. They should be reestablished before the next drawIndexed
     * or drawNonIndexed. This cannot be called between reserving and releasing
     * geometry.
     *
     * A subclass may override this to perform more optimal rect rendering. Its
     * draws should be funneled through one of the public GrDrawTarget draw methods
     * (e.g. drawNonIndexed, drawIndexedInstances, ...). The base class draws a two
     * triangle fan using drawNonIndexed from reserved vertex space.
     *
     * @param rect      the rect to draw
     * @param matrix    optional matrix applied to rect (before viewMatrix)
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
                          const SkMatrix* matrix,
                          const GrRect* srcRects[],
                          const SkMatrix* srcMatrices[]);
    /**
     * Helper for drawRect when the caller doesn't need separate src rects or
     * matrices.
     */
    void drawSimpleRect(const GrRect& rect, const SkMatrix* matrix = NULL) {
        drawRect(rect, matrix, NULL, NULL);
    }
    void drawSimpleRect(const GrIRect& irect, const SkMatrix* matrix = NULL) {
        SkRect rect = SkRect::MakeFromIRect(irect);
        this->drawRect(rect, matrix, NULL, NULL);
    }

    /**
     * This call is used to draw multiple instances of some geometry with a
     * given number of vertices (V) and indices (I) per-instance. The indices in
     * the index source must have the form i[k+I] == i[k] + V. Also, all indices
     * i[kI] ... i[(k+1)I-1] must be elements of the range kV ... (k+1)V-1. As a
     * concrete example, the following index buffer for drawing a series of
     * quads each as two triangles each satisfies these conditions with V=4 and
     * I=6:
     *      (0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11, ...)
     *
     * The call assumes that the pattern of indices fills the entire index
     * source. The size of the index buffer limits the number of instances that
     * can be drawn by the GPU in a single draw. However, the caller may specify
     * any (positive) number for instanceCount and if necessary multiple GPU
     * draws will be issued. Moreover, when drawIndexedInstances is called
     * multiple times it may be possible for GrDrawTarget to group them into a
     * single GPU draw.
     *
     * @param type          the type of primitives to draw
     * @param instanceCount the number of instances to draw. Each instance
     *                      consists of verticesPerInstance vertices indexed by
     *                      indicesPerInstance indices drawn as the primitive
     *                      type specified by type.
     * @param verticesPerInstance   The number of vertices in each instance (V
     *                              in the above description).
     * @param indicesPerInstance    The number of indices in each instance (I
     *                              in the above description).
     * @param devBounds    optional bounds hint. This is a promise from the caller,
     *                     not a request for clipping.
     */
    void drawIndexedInstances(GrPrimitiveType type,
                              int instanceCount,
                              int verticesPerInstance,
                              int indicesPerInstance,
                              const SkRect* devBounds = NULL);

    /**
     * Clear the current render target if one isn't passed in. Ignores the
     * clip and all other draw state (blend mode, stages, etc). Clears the
     * whole thing if rect is NULL, otherwise just the rect.
     */
    virtual void clear(const GrIRect* rect,
                       GrColor color,
                       GrRenderTarget* renderTarget = NULL) = 0;

    /**
     * Release any resources that are cached but not currently in use. This
     * is intended to give an application some recourse when resources are low.
     */
    virtual void purgeResources() {};

    /**
     * For subclass internal use to invoke a call to onDraw(). See DrawInfo below.
     */
    void executeDraw(const DrawInfo& info) { this->onDraw(info); }

    ////////////////////////////////////////////////////////////////////////////

    /**
     * See AutoStateRestore below.
     */
    enum ASRInit {
        kPreserve_ASRInit,
        kReset_ASRInit
    };

    /**
     * Saves off the current state and restores it in the destructor. It will
     * install a new GrDrawState object on the target (setDrawState) and restore
     * the previous one in the destructor. The caller should call drawState() to
     * get the new draw state after the ASR is installed.
     *
     * GrDrawState* state = target->drawState();
     * AutoStateRestore asr(target, GrDrawTarget::kReset_ASRInit).
     * state->setRenderTarget(rt); // state refers to the GrDrawState set on
     *                             // target before asr was initialized.
     *                             // Therefore, rt is set on the GrDrawState
     *                             // that will be restored after asr's
     *                             // destructor rather than target's current
     *                             // GrDrawState.
     */
    class AutoStateRestore : ::GrNoncopyable {
    public:
        /**
         * Default ASR will have no effect unless set() is subsequently called.
         */
        AutoStateRestore();

        /**
         * Saves the state on target. The state will be restored when the ASR
         * is destroyed. If this constructor is used do not call set().
         *
         * @param init  Should the newly installed GrDrawState be a copy of the
         *              previous state or a default-initialized GrDrawState.
         */
        AutoStateRestore(GrDrawTarget* target, ASRInit init);

        ~AutoStateRestore();

        /**
         * Saves the state on target. The state will be restored when the ASR
         * is destroyed. This should only be called once per ASR object and only
         * when the default constructor was used. For nested saves use multiple
         * ASR objects.
         *
         * @param init  Should the newly installed GrDrawState be a copy of the
         *              previous state or a default-initialized GrDrawState.
         */
        void set(GrDrawTarget* target, ASRInit init);

    private:
        GrDrawTarget*        fDrawTarget;
        SkTLazy<GrDrawState> fTempState;
        GrDrawState*         fSavedState;
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

        AutoClipRestore(GrDrawTarget* target, const SkIRect& newClip);

        ~AutoClipRestore() {
            fTarget->setClip(fClip);
        }
    private:
        GrDrawTarget*           fTarget;
        const GrClipData*       fClip;
        SkTLazy<SkClipStack>    fStack;
        GrClipData              fReplacementClip;
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

    /**
     * Determines what optimizations can be applied based on the blend. The coefficients may have
     * to be tweaked in order for the optimization to work. srcCoeff and dstCoeff are optional
     * params that receive the tweaked coefficients. Normally the function looks at the current
     * state to see if coverage is enabled. By setting forceCoverage the caller can speculatively
     * determine the blend optimizations that would be used if there was partial pixel coverage.
     */
    BlendOptFlags getBlendOpts(bool forceCoverage = false,
                               GrBlendCoeff* srcCoeff = NULL,
                               GrBlendCoeff* dstCoeff = NULL) const;

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

    int indexCountInCurrentSource() const {
        const GeometrySrcState& src = this->getGeomSrc();
        switch (src.fIndexSrc) {
            case kNone_GeometrySrcType:
                return 0;
            case kReserved_GeometrySrcType:
            case kArray_GeometrySrcType:
                return src.fIndexCount;
            case kBuffer_GeometrySrcType:
                return src.fIndexBuffer->sizeInBytes() / sizeof(uint16_t);
            default:
                GrCrash("Unexpected Index Source.");
                return 0;
        }
    }

    // allows derived class to set the caps
    CapsInternals* capsInternals() { return &fCaps.fInternals; }

    // A subclass may override this function if it wishes to be notified when the clip is changed.
    // The override should call INHERITED::clipWillBeSet().
    virtual void clipWillBeSet(const GrClipData* clipData);

    // subclasses must call this in their destructors to ensure all vertex
    // and index sources have been released (including those held by
    // pushGeometrySource())
    void releaseGeometry();

    // accessors for derived classes
    const GeometrySrcState& getGeomSrc() const { return fGeoSrcStateStack.back(); }
    // it is preferable to call this rather than getGeomSrc()->fVertexLayout because of the assert.
    GrVertexLayout getVertexLayout() const {
        // the vertex layout is only valid if a vertex source has been specified.
        GrAssert(this->getGeomSrc().fVertexSrc != kNone_GeometrySrcType);
        return this->getGeomSrc().fVertexLayout;
    }

    Caps fCaps;

    /**
     * Used to communicate draws to subclass's onDraw function.
     */
    class DrawInfo {
    public:
        DrawInfo(const DrawInfo& di) { (*this) = di; }
        DrawInfo& operator =(const DrawInfo& di);

        GrPrimitiveType primitiveType() const { return fPrimitiveType; }
        int startVertex() const { return fStartVertex; }
        int startIndex() const { return fStartIndex; }
        int vertexCount() const { return fVertexCount; }
        int indexCount() const { return fIndexCount; }
        int verticesPerInstance() const { return fVerticesPerInstance; }
        int indicesPerInstance() const { return fIndicesPerInstance; }
        int instanceCount() const { return fInstanceCount; }

        bool isIndexed() const { return fIndexCount > 0; }
#if GR_DEBUG
        bool isInstanced() const; // this version is longer because of asserts
#else
        bool isInstanced() const { return fInstanceCount > 0; }
#endif

        // adds or remove instances
        void adjustInstanceCount(int instanceOffset);
        // shifts the start vertex
        void adjustStartVertex(int vertexOffset);
        // shifts the start index
        void adjustStartIndex(int indexOffset);

        void setDevBounds(const SkRect& bounds) {
            fDevBoundsStorage = bounds;
            fDevBounds = &fDevBoundsStorage;
        }
        const SkRect* getDevBounds() const { return fDevBounds; }

    private:
        DrawInfo() { fDevBounds = NULL; }

        friend class GrDrawTarget;

        GrPrimitiveType fPrimitiveType;

        int             fStartVertex;
        int             fStartIndex;
        int             fVertexCount;
        int             fIndexCount;

        int             fInstanceCount;
        int             fVerticesPerInstance;
        int             fIndicesPerInstance;

        SkRect          fDevBoundsStorage;
        SkRect*         fDevBounds;
    };

private:
    // A subclass can optionally overload this function to be notified before
    // vertex and index space is reserved.
    virtual void willReserveVertexAndIndexSpace(size_t vertexSize, int vertexCount, int indexCount) {}

    // implemented by subclass to allocate space for reserved geom
    virtual bool onReserveVertexSpace(size_t vertexSize, int vertexCount, void** vertices) = 0;
    virtual bool onReserveIndexSpace(int indexCount, void** indices) = 0;
    // implemented by subclass to handle release of reserved geom space
    virtual void releaseReservedVertexSpace() = 0;
    virtual void releaseReservedIndexSpace() = 0;
    // subclass must consume array contents when set
    virtual void onSetVertexSourceToArray(const void* vertexArray, int vertexCount) = 0;
    virtual void onSetIndexSourceToArray(const void* indexArray, int indexCount) = 0;
    // subclass is notified that geom source will be set away from an array
    virtual void releaseVertexArray() = 0;
    virtual void releaseIndexArray() = 0;
    // subclass overrides to be notified just before geo src state is pushed/popped.
    virtual void geometrySourceWillPush() = 0;
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState) = 0;
    // subclass called to perform drawing
    virtual void onDraw(const DrawInfo&) = 0;
    virtual void onStencilPath(const GrPath*, const SkStrokeRec& stroke, SkPath::FillType fill) = 0;

    // helpers for reserving vertex and index space.
    bool reserveVertexSpace(GrVertexLayout vertexLayout,
                            int vertexCount,
                            void** vertices);
    bool reserveIndexSpace(int indexCount, void** indices);

    // called by drawIndexed and drawNonIndexed. Use a negative indexCount to
    // indicate non-indexed drawing.
    bool checkDraw(GrPrimitiveType type, int startVertex,
                   int startIndex, int vertexCount,
                   int indexCount) const;
    // called when setting a new vert/idx source to unref prev vb/ib
    void releasePreviousVertexSource();
    void releasePreviousIndexSource();

    enum {
        kPreallocGeoSrcStateStackCnt = 4,
    };
    SkSTArray<kPreallocGeoSrcStateStackCnt, GeometrySrcState, true> fGeoSrcStateStack;
    const GrClipData*                                               fClip;
    GrDrawState*                                                    fDrawState;
    GrDrawState                                                     fDefaultDrawState;

    typedef GrRefCnt INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrDrawTarget::BlendOptFlags);

#endif
