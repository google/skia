/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrFlushToGpuDrawTarget.h"
#include "GrContext.h"
#include "GrFontCache.h"
#include "GrGpu.h"
#include "GrBufferAllocPool.h"

GrFlushToGpuDrawTarget::GrFlushToGpuDrawTarget(GrGpu* gpu,
                                               GrVertexBufferAllocPool* vertexPool,
                                               GrIndexBufferAllocPool* indexPool)
    : INHERITED(gpu->getContext())
    , fGpu(SkRef(gpu))
    , fVertexPool(vertexPool)
    , fIndexPool(indexPool)
    , fFlushing(false) {

    fCaps.reset(SkRef(fGpu->caps()));

    SkASSERT(vertexPool);
    SkASSERT(indexPool);

    GeometryPoolState& poolState = fGeoPoolStateStack.push_back();
    poolState.fUsedPoolVertexBytes = 0;
    poolState.fUsedPoolIndexBytes = 0;
#ifdef SK_DEBUG
    poolState.fPoolVertexBuffer = (GrVertexBuffer*)~0;
    poolState.fPoolStartVertex = ~0;
    poolState.fPoolIndexBuffer = (GrIndexBuffer*)~0;
    poolState.fPoolStartIndex = ~0;
#endif
}

GrFlushToGpuDrawTarget::~GrFlushToGpuDrawTarget() {
    // This must be called by before the GrDrawTarget destructor
    this->releaseGeometry();
}

void GrFlushToGpuDrawTarget::setDrawBuffers(DrawInfo* info, size_t vertexStride) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    if (kBuffer_GeometrySrcType == this->getGeomSrc().fVertexSrc) {
        info->setVertexBuffer(this->getGeomSrc().fVertexBuffer);
    } else {
        // Update the bytes used since the last reserve-geom request.
        size_t bytes = (info->vertexCount() + info->startVertex()) * vertexStride;
        poolState.fUsedPoolVertexBytes = SkTMax(poolState.fUsedPoolVertexBytes, bytes);
        info->setVertexBuffer(poolState.fPoolVertexBuffer);
        info->adjustStartVertex(poolState.fPoolStartVertex);
    }

    if (info->isIndexed()) {
        if (kBuffer_GeometrySrcType == this->getGeomSrc().fIndexSrc) {
            info->setIndexBuffer(this->getGeomSrc().fIndexBuffer);
        } else {
            // Update the bytes used since the last reserve-geom request.
            size_t bytes = (info->indexCount() + info->startIndex()) * sizeof(uint16_t);
            poolState.fUsedPoolIndexBytes = SkTMax(poolState.fUsedPoolIndexBytes, bytes);
            info->setIndexBuffer(poolState.fPoolIndexBuffer);
            info->adjustStartIndex(poolState.fPoolStartIndex);
        }
    }
}

void GrFlushToGpuDrawTarget::reset() {
    SkASSERT(1 == fGeoPoolStateStack.count());
    this->resetVertexSource();
    this->resetIndexSource();

    fVertexPool->reset();
    fIndexPool->reset();

    this->onReset();
}

void GrFlushToGpuDrawTarget::flush() {
    SkASSERT(kReserved_GeometrySrcType != this->getGeomSrc().fVertexSrc);
    SkASSERT(kReserved_GeometrySrcType != this->getGeomSrc().fIndexSrc);

    if (fFlushing) {
        return;
    }
    fFlushing = true;

    fGpu->getContext()->getFontCache()->updateTextures();

    fGpu->saveActiveTraceMarkers();

    this->onFlush();

    fGpu->restoreActiveTraceMarkers();

    fFlushing = false;
    this->reset();
}

void GrFlushToGpuDrawTarget::willReserveVertexAndIndexSpace(int vertexCount,
                                                            size_t vertexStride,
                                                            int indexCount) {
    // We use geometryHints() to know whether to flush the draw buffer. We
    // can't flush if we are inside an unbalanced pushGeometrySource.
    // Moreover, flushing blows away vertex and index data that was
    // previously reserved. So if the vertex or index data is pulled from
    // reserved space and won't be released by this request then we can't
    // flush.
    bool insideGeoPush = fGeoPoolStateStack.count() > 1;

    bool unreleasedVertexSpace =
        !vertexCount &&
        kReserved_GeometrySrcType == this->getGeomSrc().fVertexSrc;

    bool unreleasedIndexSpace =
        !indexCount &&
        kReserved_GeometrySrcType == this->getGeomSrc().fIndexSrc;

    int vcount = vertexCount;
    int icount = indexCount;

    if (!insideGeoPush &&
        !unreleasedVertexSpace &&
        !unreleasedIndexSpace &&
        this->geometryHints(vertexStride, &vcount, &icount)) {
        this->flush();
    }
}

bool GrFlushToGpuDrawTarget::geometryHints(size_t vertexStride,
                                           int* vertexCount,
                                           int* indexCount) const {
    // we will recommend a flush if the data could fit in a single
    // preallocated buffer but none are left and it can't fit
    // in the current buffer (which may not be prealloced).
    bool flush = false;
    if (indexCount) {
        int32_t currIndices = fIndexPool->currentBufferIndices();
        if (*indexCount > currIndices &&
            (!fIndexPool->preallocatedBuffersRemaining() &&
             *indexCount <= fIndexPool->preallocatedBufferIndices())) {

            flush = true;
        }
        *indexCount = currIndices;
    }
    if (vertexCount) {
        int32_t currVertices = fVertexPool->currentBufferVertices(vertexStride);
        if (*vertexCount > currVertices &&
            (!fVertexPool->preallocatedBuffersRemaining() &&
             *vertexCount <= fVertexPool->preallocatedBufferVertices(vertexStride))) {

            flush = true;
        }
        *vertexCount = currVertices;
    }
    return flush;
}

bool GrFlushToGpuDrawTarget::onReserveVertexSpace(size_t vertexSize,
                                                  int vertexCount,
                                                  void** vertices) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    SkASSERT(vertexCount > 0);
    SkASSERT(vertices);
    SkASSERT(0 == poolState.fUsedPoolVertexBytes);

    *vertices = fVertexPool->makeSpace(vertexSize,
                                       vertexCount,
                                       &poolState.fPoolVertexBuffer,
                                       &poolState.fPoolStartVertex);
    return SkToBool(*vertices);
}

bool GrFlushToGpuDrawTarget::onReserveIndexSpace(int indexCount, void** indices) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    SkASSERT(indexCount > 0);
    SkASSERT(indices);
    SkASSERT(0 == poolState.fUsedPoolIndexBytes);

    *indices = fIndexPool->makeSpace(indexCount,
                                     &poolState.fPoolIndexBuffer,
                                     &poolState.fPoolStartIndex);
    return SkToBool(*indices);
}

void GrFlushToGpuDrawTarget::releaseReservedVertexSpace() {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GeometrySrcState& geoSrc = this->getGeomSrc();

    // If we get a release vertex space call then our current source should either be reserved
    // or array (which we copied into reserved space).
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fVertexSrc);

    // When the caller reserved vertex buffer space we gave it back a pointer
    // provided by the vertex buffer pool. At each draw we tracked the largest
    // offset into the pool's pointer that was referenced. Now we return to the
    // pool any portion at the tail of the allocation that no draw referenced.
    size_t reservedVertexBytes = geoSrc.fVertexSize * geoSrc.fVertexCount;
    fVertexPool->putBack(reservedVertexBytes - poolState.fUsedPoolVertexBytes);
    poolState.fUsedPoolVertexBytes = 0;
    poolState.fPoolVertexBuffer = NULL;
    poolState.fPoolStartVertex = 0;
}

void GrFlushToGpuDrawTarget::releaseReservedIndexSpace() {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GeometrySrcState& geoSrc = this->getGeomSrc();

    // If we get a release index space call then our current source should either be reserved
    // or array (which we copied into reserved space).
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fIndexSrc);

    // Similar to releaseReservedVertexSpace we return any unused portion at
    // the tail
    size_t reservedIndexBytes = sizeof(uint16_t) * geoSrc.fIndexCount;
    fIndexPool->putBack(reservedIndexBytes - poolState.fUsedPoolIndexBytes);
    poolState.fUsedPoolIndexBytes = 0;
    poolState.fPoolIndexBuffer = NULL;
    poolState.fPoolStartIndex = 0;
}

void GrFlushToGpuDrawTarget::geometrySourceWillPush() {
    GeometryPoolState& poolState = fGeoPoolStateStack.push_back();
    poolState.fUsedPoolVertexBytes = 0;
    poolState.fUsedPoolIndexBytes = 0;
#ifdef SK_DEBUG
    poolState.fPoolVertexBuffer = (GrVertexBuffer*)~0;
    poolState.fPoolStartVertex = ~0;
    poolState.fPoolIndexBuffer = (GrIndexBuffer*)~0;
    poolState.fPoolStartIndex = ~0;
#endif
}

void GrFlushToGpuDrawTarget::geometrySourceWillPop(const GeometrySrcState& restoredState) {
    SkASSERT(fGeoPoolStateStack.count() > 1);
    fGeoPoolStateStack.pop_back();
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    // we have to assume that any slack we had in our vertex/index data
    // is now unreleasable because data may have been appended later in the
    // pool.
    if (kReserved_GeometrySrcType == restoredState.fVertexSrc) {
        poolState.fUsedPoolVertexBytes = restoredState.fVertexSize * restoredState.fVertexCount;
    }
    if (kReserved_GeometrySrcType == restoredState.fIndexSrc) {
        poolState.fUsedPoolIndexBytes = sizeof(uint16_t) * restoredState.fIndexCount;
    }
}

bool GrFlushToGpuDrawTarget::onCanCopySurface(const GrSurface* dst,
                                              const GrSurface* src,
                                              const SkIRect& srcRect,
                                              const SkIPoint& dstPoint) {
    return getGpu()->canCopySurface(dst, src, srcRect, dstPoint);
}

bool GrFlushToGpuDrawTarget::onInitCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) {
    return getGpu()->initCopySurfaceDstDesc(src, desc);
}
