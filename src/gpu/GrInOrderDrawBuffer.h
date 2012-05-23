
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrInOrderDrawBuffer_DEFINED
#define GrInOrderDrawBuffer_DEFINED

#include "GrDrawTarget.h"
#include "GrAllocPool.h"
#include "GrAllocator.h"
#include "GrClip.h"

class GrGpu;
class GrIndexBufferAllocPool;
class GrVertexBufferAllocPool;

/**
 * GrInOrderDrawBuffer is an implementation of GrDrawTarget that queues up
 * draws for eventual playback into a GrGpu. In theory one draw buffer could
 * playback into another. When index or vertex buffers are used as geometry
 * sources it is the callers the draw buffer only holds references to the
 * buffers. It is the callers responsibility to ensure that the data is still
 * valid when the draw buffer is played back into a GrGpu. Similarly, it is the
 * caller's responsibility to ensure that all referenced textures, buffers,
 * and rendertargets are associated in the GrGpu object that the buffer is
 * played back into. The buffer requires VB and IB pools to store geometry.
 */

class GrInOrderDrawBuffer : public GrDrawTarget {
public:

    /**
     * Creates a GrInOrderDrawBuffer
     *
     * @param gpu        the gpu object where this will be played back
     *                   (possible indirectly). GrResources used with the draw
     *                   buffer are created by this gpu object.
     * @param vertexPool pool where vertices for queued draws will be saved when
     *                   the vertex source is either reserved or array.
     * @param indexPool  pool where indices for queued draws will be saved when
     *                   the index source is either reserved or array.
     */
    GrInOrderDrawBuffer(const GrGpu* gpu,
                        GrVertexBufferAllocPool* vertexPool,
                        GrIndexBufferAllocPool* indexPool);

    virtual ~GrInOrderDrawBuffer();

    /**
     * Provides the buffer with an index buffer that can be used for quad rendering.
     * The buffer may be able to batch consecutive drawRects if this is provided.
     * @param indexBuffer   index buffer with quad indices.
     */
    void setQuadIndexBuffer(const GrIndexBuffer* indexBuffer);

    /**
     * Empties the draw buffer of any queued up draws. This must not be called
     * while inside an unbalanced pushGeometrySource().
     */
    void reset();

    /**
     * plays the queued up draws to another target. Does not empty this buffer
     * so that it can be played back multiple times. This buffer must not have
     * an active reserved vertex or index source. Any reserved geometry on
     * the target will be finalized because it's geometry source will be pushed
     * before playback and popped afterwards.
     *
     * @param target    the target to receive the playback
     */
    void playback(GrDrawTarget* target);

    /**
     * A convenience method to do a playback followed by a reset. All the
     * constraints and side-effects or playback() and reset apply().
     */
    void flushTo(GrDrawTarget* target) {
        this->playback(target);
        this->reset();
    }

    /**
     * This function allows the draw buffer to automatically flush itself to
     * another target. This means the buffer may internally call 
     * this->flushTo(target) when it is safe to do so.
     * 
     * When the auto flush target is set to NULL (as it initially is) the draw
     * buffer will never automatically flush itself.
     */
    void setAutoFlushTarget(GrDrawTarget* target);

    // overrides from GrDrawTarget
    virtual void drawRect(const GrRect& rect, 
                          const GrMatrix* matrix = NULL,
                          StageMask stageEnableMask = 0,
                          const GrRect* srcRects[] = NULL,
                          const GrMatrix* srcMatrices[] = NULL) SK_OVERRIDE;

    virtual void drawIndexedInstances(GrPrimitiveType type,
                                      int instanceCount,
                                      int verticesPerInstance,
                                      int indicesPerInstance)
                                      SK_OVERRIDE;

    virtual bool geometryHints(GrVertexLayout vertexLayout,
                               int* vertexCount,
                               int* indexCount) const SK_OVERRIDE;

    virtual void clear(const GrIRect* rect, GrColor color) SK_OVERRIDE;

protected:
    virtual void willReserveVertexAndIndexSpace(GrVertexLayout vertexLayout,
                                                int vertexCount,
                                                int indexCount) SK_OVERRIDE;
private:
    struct Draw {
        GrPrimitiveType         fPrimitiveType;
        int                     fStartVertex;
        int                     fStartIndex;
        int                     fVertexCount;
        int                     fIndexCount;
        bool                    fStateChanged;
        bool                    fClipChanged;
        GrVertexLayout          fVertexLayout;
        const GrVertexBuffer*   fVertexBuffer;
        const GrIndexBuffer*    fIndexBuffer;
    };

    struct Clear {
        int fBeforeDrawIdx;
        GrIRect fRect;
        GrColor fColor;
    };

    // overrides from GrDrawTarget
    virtual void onDrawIndexed(GrPrimitiveType primitiveType,
                               int startVertex,
                               int startIndex,
                               int vertexCount,
                               int indexCount);
    virtual void onDrawNonIndexed(GrPrimitiveType primitiveType,
                                  int startVertex,
                                  int vertexCount);
    virtual bool onReserveVertexSpace(GrVertexLayout layout, 
                                      int vertexCount,
                                      void** vertices);
    virtual bool onReserveIndexSpace(int indexCount, void** indices);
    virtual void releaseReservedVertexSpace();
    virtual void releaseReservedIndexSpace();
    virtual void onSetVertexSourceToArray(const void* vertexArray,
                                          int vertexCount);
    virtual void onSetIndexSourceToArray(const void* indexArray,
                                         int indexCount);
    virtual void releaseVertexArray();
    virtual void releaseIndexArray();
    virtual void geometrySourceWillPush();
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState);
    virtual void clipWillBeSet(const GrClip& newClip);

    bool needsNewState() const;
    bool needsNewClip() const;

    void pushState();
    void storeClip();

    // call this to invalidate the tracking data that is used to concatenate 
    // multiple draws into a single draw.
    void resetDrawTracking();

    enum {
        kDrawPreallocCnt         = 8,
        kStatePreallocCnt        = 8,
        kClipPreallocCnt         = 8,
        kClearPreallocCnt        = 4,
        kGeoPoolStatePreAllocCnt = 4,
    };

    GrSTAllocator<kDrawPreallocCnt, Draw>               fDraws;
    GrSTAllocator<kStatePreallocCnt, GrDrawState>       fStates;
    GrSTAllocator<kClearPreallocCnt, Clear>             fClears;
    GrSTAllocator<kClipPreallocCnt, GrClip>             fClips;

    GrDrawTarget*                   fAutoFlushTarget;

    bool                            fClipSet;

    GrVertexBufferAllocPool&        fVertexPool;

    GrIndexBufferAllocPool&         fIndexPool;

    // these are used to attempt to concatenate drawRect calls
    GrVertexLayout                  fLastRectVertexLayout;
    const GrIndexBuffer*            fQuadIndexBuffer;
    int                             fMaxQuads;
    int                             fCurrQuad;

    // bookkeeping to attempt to concantenate drawIndexedInstances calls
    struct {
        int            fVerticesPerInstance;
        int            fIndicesPerInstance;
        void reset() {
            fVerticesPerInstance = 0;
            fIndicesPerInstance = 0;
        }
    } fInstancedDrawTracker;

    struct GeometryPoolState {
        const GrVertexBuffer*           fPoolVertexBuffer;
        int                             fPoolStartVertex;
        const GrIndexBuffer*            fPoolIndexBuffer;
        int                             fPoolStartIndex;
        // caller may conservatively over reserve vertices / indices.
        // we release unused space back to allocator if possible
        // can only do this if there isn't an intervening pushGeometrySource()
        size_t                          fUsedPoolVertexBytes;
        size_t                          fUsedPoolIndexBytes;
    };
    SkSTArray<kGeoPoolStatePreAllocCnt, GeometryPoolState> fGeoPoolStateStack;

    typedef GrDrawTarget INHERITED;
};

#endif
