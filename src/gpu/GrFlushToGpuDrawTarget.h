/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFlushToGpuDrawTarget_DEFINED
#define GrFlushToGpuDrawTarget_DEFINED

#include "GrDrawTarget.h"

class GrIndexBufferAllocPool;
class GrVertexBufferAllocPool;
class GrGpu;

/**
 * Base class for draw targets that accumulate index and vertex data in buffers for deferred.
 * When draw target clients reserve geometry this subclass will place that geometry into
 * preallocated vertex/index buffers in the order the requests are made (assuming the requests fit
 * in the preallocated buffers).
 */
class GrFlushToGpuDrawTarget : public GrClipTarget {
public:
    GrFlushToGpuDrawTarget(GrGpu*, GrVertexBufferAllocPool*,GrIndexBufferAllocPool*);

    ~GrFlushToGpuDrawTarget() SK_OVERRIDE;

    /**
     * Empties the draw buffer of any queued up draws. This must not be called while inside an
     * unbalanced pushGeometrySource().
     */
    void reset();

    /**
     * This plays any queued up draws to its GrGpu target. It also resets this object (i.e. flushing
     * is destructive). This buffer must not have an active reserved vertex or index source. Any
     * reserved geometry on the target will be finalized because it's geometry source will be pushed
     * before flushing and popped afterwards.
     */
    void flush();

    bool geometryHints(size_t vertexStride, int* vertexCount, int* indexCount) const SK_OVERRIDE;

protected:
    GrGpu* getGpu() { return fGpu; }
    const GrGpu* getGpu() const{ return fGpu; }

    GrVertexBufferAllocPool* getVertexAllocPool() { return fVertexPool; }
    GrIndexBufferAllocPool* getIndexAllocPool() { return fIndexPool; }

    // TODO all of this goes away when batch is everywhere
    enum {
        kGeoPoolStatePreAllocCnt = 4,
    };

    struct GeometryPoolState {
        const GrVertexBuffer*   fPoolVertexBuffer;
        int                     fPoolStartVertex;
        const GrIndexBuffer*    fPoolIndexBuffer;
        int                     fPoolStartIndex;
        // caller may conservatively over reserve vertices / indices.
        // we release unused space back to allocator if possible
        // can only do this if there isn't an intervening pushGeometrySource()
        size_t                  fUsedPoolVertexBytes;
        size_t                  fUsedPoolIndexBytes;
    };

    typedef SkSTArray<kGeoPoolStatePreAllocCnt, GeometryPoolState> GeoPoolStateStack;
    const GeoPoolStateStack& getGeoPoolStateStack() const { return fGeoPoolStateStack; }

    void willReserveVertexAndIndexSpace(int vertexCount,
                                        size_t vertexStride,
                                        int indexCount) SK_OVERRIDE;

private:
    virtual void onReset() = 0;

    virtual void onFlush() = 0;

    void setDrawBuffers(DrawInfo*, size_t stride) SK_OVERRIDE;
    bool onReserveVertexSpace(size_t vertexSize, int vertexCount, void** vertices) SK_OVERRIDE;
    bool onReserveIndexSpace(int indexCount, void** indices) SK_OVERRIDE;
    void releaseReservedVertexSpace() SK_OVERRIDE;
    void releaseReservedIndexSpace() SK_OVERRIDE;
    void geometrySourceWillPush() SK_OVERRIDE;
    void geometrySourceWillPop(const GeometrySrcState& restoredState) SK_OVERRIDE;
    bool onCanCopySurface(const GrSurface* dst,
                          const GrSurface* src,
                          const SkIRect& srcRect,
                          const SkIPoint& dstPoint) SK_OVERRIDE;
    bool onInitCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) SK_OVERRIDE;

    GeoPoolStateStack                   fGeoPoolStateStack;
    SkAutoTUnref<GrGpu>                 fGpu;
    GrVertexBufferAllocPool*            fVertexPool;
    GrIndexBufferAllocPool*             fIndexPool;
    bool                                fFlushing;

    typedef GrClipTarget INHERITED;
};

#endif
