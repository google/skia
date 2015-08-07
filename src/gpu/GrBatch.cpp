/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatch.h"
#include "GrBatchTarget.h"
#include "GrResourceProvider.h"

#include "GrMemoryPool.h"
#include "SkSpinlock.h"

// TODO I noticed a small benefit to using a larger exclusive pool for batches.  Its very small,
// but seems to be mostly consistent.  There is a lot in flux right now, but we should really
// revisit this when batch is everywhere


// We use a global pool protected by a mutex(spinlock). Chrome may use the same GrContext on
// different threads. The GrContext is not used concurrently on different threads and there is a
// memory barrier between accesses of a context on different threads. Also, there may be multiple
// GrContexts and those contexts may be in use concurrently on different threads.
namespace {
SK_DECLARE_STATIC_SPINLOCK(gBatchSpinlock);
class MemoryPoolAccessor {
public:
    MemoryPoolAccessor() { gBatchSpinlock.acquire(); }

    ~MemoryPoolAccessor() { gBatchSpinlock.release(); }

    GrMemoryPool* pool() const {
        static GrMemoryPool gPool(16384, 16384);
        return &gPool;
    }
};
}

int32_t GrBatch::gCurrBatchClassID = GrBatch::kIllegalBatchID;

GrBATCH_SPEW(int32_t GrBatch::gCurrBatchUniqueID = GrBatch::kIllegalBatchID;)

void* GrBatch::operator new(size_t size) {
    return MemoryPoolAccessor().pool()->allocate(size);
}

void GrBatch::operator delete(void* target) {
    return MemoryPoolAccessor().pool()->release(target);
}

void* GrBatch::InstancedHelper::init(GrBatchTarget* batchTarget, GrPrimitiveType primType,
                                     size_t vertexStride, const GrIndexBuffer* indexBuffer,
                                     int verticesPerInstance, int indicesPerInstance,
                                     int instancesToDraw) {
    SkASSERT(batchTarget);
    if (!indexBuffer) {
        return NULL;
    }
    const GrVertexBuffer* vertexBuffer;
    int firstVertex;
    int vertexCount = verticesPerInstance * instancesToDraw;
    void* vertices = batchTarget->makeVertSpace(vertexStride, vertexCount,
                                                &vertexBuffer, &firstVertex);
    if (!vertices) {
        SkDebugf("Vertices could not be allocated for instanced rendering.");
        return NULL;
    }
    SkASSERT(vertexBuffer);
    size_t ibSize = indexBuffer->gpuMemorySize();
    int maxInstancesPerDraw = static_cast<int>(ibSize / (sizeof(uint16_t) * indicesPerInstance));

    fVertices.initInstanced(primType, vertexBuffer, indexBuffer,
        firstVertex, verticesPerInstance, indicesPerInstance, instancesToDraw,
        maxInstancesPerDraw);
    return vertices;
}

void* GrBatch::QuadHelper::init(GrBatchTarget* batchTarget, size_t vertexStride, int quadsToDraw) {
    SkAutoTUnref<const GrIndexBuffer> quadIndexBuffer(
        batchTarget->resourceProvider()->refQuadIndexBuffer());
    if (!quadIndexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return NULL;
    }
    return this->INHERITED::init(batchTarget, kTriangles_GrPrimitiveType, vertexStride,
                                 quadIndexBuffer, kVerticesPerQuad, kIndicesPerQuad, quadsToDraw);
}
