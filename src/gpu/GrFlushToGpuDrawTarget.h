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

protected:
    GrGpu* getGpu() { return fGpu; }
    const GrGpu* getGpu() const{ return fGpu; }

    GrVertexBufferAllocPool* getVertexAllocPool() { return fVertexPool; }
    GrIndexBufferAllocPool* getIndexAllocPool() { return fIndexPool; }

private:
    virtual void onReset() = 0;

    virtual void onFlush() = 0;

    bool onCanCopySurface(const GrSurface* dst,
                          const GrSurface* src,
                          const SkIRect& srcRect,
                          const SkIPoint& dstPoint) override;
    bool onInitCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) override;

    SkAutoTUnref<GrGpu>                 fGpu;
    GrVertexBufferAllocPool*            fVertexPool;
    GrIndexBufferAllocPool*             fIndexPool;
    bool                                fFlushing;

    typedef GrClipTarget INHERITED;
};

#endif
