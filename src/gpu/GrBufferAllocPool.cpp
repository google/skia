
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrBufferAllocPool.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrIndexBuffer.h"
#include "GrTypes.h"
#include "GrVertexBuffer.h"

#include "SkTraceEvent.h"

#ifdef SK_DEBUG
    #define VALIDATE validate
#else
    static void VALIDATE(bool = false) {}
#endif

// page size
#define GrBufferAllocPool_MIN_BLOCK_SIZE ((size_t)1 << 12)

#define UNMAP_BUFFER(block)                                                               \
do {                                                                                      \
    TRACE_EVENT_INSTANT1(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),                           \
                         "GrBufferAllocPool Unmapping Buffer",                            \
                         TRACE_EVENT_SCOPE_THREAD,                                        \
                         "percent_unwritten",                                             \
                         (float)((block).fBytesFree) / (block).fBuffer->gpuMemorySize()); \
    (block).fBuffer->unmap();                                                             \
} while (false)

GrBufferAllocPool::GrBufferAllocPool(GrGpu* gpu,
                                     BufferType bufferType,
                                     bool frequentResetHint,
                                     size_t blockSize,
                                     int preallocBufferCnt) :
        fBlocks(SkTMax(8, 2*preallocBufferCnt)) {

    SkASSERT(NULL != gpu);
    fGpu = gpu;
    fGpu->ref();
    fGpuIsReffed = true;

    fBufferType = bufferType;
    fFrequentResetHint = frequentResetHint;
    fBufferPtr = NULL;
    fMinBlockSize = SkTMax(GrBufferAllocPool_MIN_BLOCK_SIZE, blockSize);

    fBytesInUse = 0;

    fPreallocBuffersInUse = 0;
    fPreallocBufferStartIdx = 0;
    for (int i = 0; i < preallocBufferCnt; ++i) {
        GrGeometryBuffer* buffer = this->createBuffer(fMinBlockSize);
        if (NULL != buffer) {
            *fPreallocBuffers.append() = buffer;
        }
    }
}

GrBufferAllocPool::~GrBufferAllocPool() {
    VALIDATE();
    if (fBlocks.count()) {
        GrGeometryBuffer* buffer = fBlocks.back().fBuffer;
        if (buffer->isMapped()) {
            UNMAP_BUFFER(fBlocks.back());
        }
    }
    while (!fBlocks.empty()) {
        destroyBlock();
    }
    fPreallocBuffers.unrefAll();
    releaseGpuRef();
}

void GrBufferAllocPool::releaseGpuRef() {
    if (fGpuIsReffed) {
        fGpu->unref();
        fGpuIsReffed = false;
    }
}

void GrBufferAllocPool::reset() {
    VALIDATE();
    fBytesInUse = 0;
    if (fBlocks.count()) {
        GrGeometryBuffer* buffer = fBlocks.back().fBuffer;
        if (buffer->isMapped()) {
            UNMAP_BUFFER(fBlocks.back());
        }
    }
    // fPreallocBuffersInUse will be decremented down to zero in the while loop
    int preallocBuffersInUse = fPreallocBuffersInUse;
    while (!fBlocks.empty()) {
        this->destroyBlock();
    }
    if (fPreallocBuffers.count()) {
        // must set this after above loop.
        fPreallocBufferStartIdx = (fPreallocBufferStartIdx +
                                   preallocBuffersInUse) %
                                  fPreallocBuffers.count();
    }
    // we may have created a large cpu mirror of a large VB. Reset the size
    // to match our pre-allocated VBs.
    fCpuData.reset(fMinBlockSize);
    SkASSERT(0 == fPreallocBuffersInUse);
    VALIDATE();
}

void GrBufferAllocPool::unmap() {
    VALIDATE();

    if (NULL != fBufferPtr) {
        BufferBlock& block = fBlocks.back();
        if (block.fBuffer->isMapped()) {
            UNMAP_BUFFER(block);
        } else {
            size_t flushSize = block.fBuffer->gpuMemorySize() - block.fBytesFree;
            this->flushCpuData(fBlocks.back(), flushSize);
        }
        fBufferPtr = NULL;
    }
    VALIDATE();
}

#ifdef SK_DEBUG
void GrBufferAllocPool::validate(bool unusedBlockAllowed) const {
    if (NULL != fBufferPtr) {
        SkASSERT(!fBlocks.empty());
        if (fBlocks.back().fBuffer->isMapped()) {
            GrGeometryBuffer* buf = fBlocks.back().fBuffer;
            SkASSERT(buf->mapPtr() == fBufferPtr);
        } else {
            SkASSERT(fCpuData.get() == fBufferPtr);
        }
    } else {
        SkASSERT(fBlocks.empty() || !fBlocks.back().fBuffer->isMapped());
    }
    size_t bytesInUse = 0;
    for (int i = 0; i < fBlocks.count() - 1; ++i) {
        SkASSERT(!fBlocks[i].fBuffer->isMapped());
    }
    for (int i = 0; i < fBlocks.count(); ++i) {
        size_t bytes = fBlocks[i].fBuffer->gpuMemorySize() - fBlocks[i].fBytesFree;
        bytesInUse += bytes;
        SkASSERT(bytes || unusedBlockAllowed);
    }

    SkASSERT(bytesInUse == fBytesInUse);
    if (unusedBlockAllowed) {
        SkASSERT((fBytesInUse && !fBlocks.empty()) ||
                 (!fBytesInUse && (fBlocks.count() < 2)));
    } else {
        SkASSERT((0 == fBytesInUse) == fBlocks.empty());
    }
}
#endif

void* GrBufferAllocPool::makeSpace(size_t size,
                                   size_t alignment,
                                   const GrGeometryBuffer** buffer,
                                   size_t* offset) {
    VALIDATE();

    SkASSERT(NULL != buffer);
    SkASSERT(NULL != offset);

    if (NULL != fBufferPtr) {
        BufferBlock& back = fBlocks.back();
        size_t usedBytes = back.fBuffer->gpuMemorySize() - back.fBytesFree;
        size_t pad = GrSizeAlignUpPad(usedBytes,
                                      alignment);
        if ((size + pad) <= back.fBytesFree) {
            usedBytes += pad;
            *offset = usedBytes;
            *buffer = back.fBuffer;
            back.fBytesFree -= size + pad;
            fBytesInUse += size + pad;
            VALIDATE();
            return (void*)(reinterpret_cast<intptr_t>(fBufferPtr) + usedBytes);
        }
    }

    // We could honor the space request using by a partial update of the current
    // VB (if there is room). But we don't currently use draw calls to GL that
    // allow the driver to know that previously issued draws won't read from
    // the part of the buffer we update. Also, the GL buffer implementation
    // may be cheating on the actual buffer size by shrinking the buffer on
    // updateData() if the amount of data passed is less than the full buffer
    // size.

    if (!createBlock(size)) {
        return NULL;
    }
    SkASSERT(NULL != fBufferPtr);

    *offset = 0;
    BufferBlock& back = fBlocks.back();
    *buffer = back.fBuffer;
    back.fBytesFree -= size;
    fBytesInUse += size;
    VALIDATE();
    return fBufferPtr;
}

int GrBufferAllocPool::currentBufferItems(size_t itemSize) const {
    VALIDATE();
    if (NULL != fBufferPtr) {
        const BufferBlock& back = fBlocks.back();
        size_t usedBytes = back.fBuffer->gpuMemorySize() - back.fBytesFree;
        size_t pad = GrSizeAlignUpPad(usedBytes, itemSize);
        return static_cast<int>((back.fBytesFree - pad) / itemSize);
    } else if (fPreallocBuffersInUse < fPreallocBuffers.count()) {
        return static_cast<int>(fMinBlockSize / itemSize);
    }
    return 0;
}

int GrBufferAllocPool::preallocatedBuffersRemaining() const {
    return fPreallocBuffers.count() - fPreallocBuffersInUse;
}

int GrBufferAllocPool::preallocatedBufferCount() const {
    return fPreallocBuffers.count();
}

void GrBufferAllocPool::putBack(size_t bytes) {
    VALIDATE();

    // if the putBack unwinds all the preallocated buffers then we will
    // advance the starting index. As blocks are destroyed fPreallocBuffersInUse
    // will be decremented. I will reach zero if all blocks using preallocated
    // buffers are released.
    int preallocBuffersInUse = fPreallocBuffersInUse;

    while (bytes) {
        // caller shouldnt try to put back more than they've taken
        SkASSERT(!fBlocks.empty());
        BufferBlock& block = fBlocks.back();
        size_t bytesUsed = block.fBuffer->gpuMemorySize() - block.fBytesFree;
        if (bytes >= bytesUsed) {
            bytes -= bytesUsed;
            fBytesInUse -= bytesUsed;
            // if we locked a vb to satisfy the make space and we're releasing
            // beyond it, then unmap it.
            if (block.fBuffer->isMapped()) {
                UNMAP_BUFFER(block);
            }
            this->destroyBlock();
        } else {
            block.fBytesFree += bytes;
            fBytesInUse -= bytes;
            bytes = 0;
            break;
        }
    }
    if (!fPreallocBuffersInUse && fPreallocBuffers.count()) {
            fPreallocBufferStartIdx = (fPreallocBufferStartIdx +
                                       preallocBuffersInUse) %
                                      fPreallocBuffers.count();
    }
    VALIDATE();
}

bool GrBufferAllocPool::createBlock(size_t requestSize) {

    size_t size = SkTMax(requestSize, fMinBlockSize);
    SkASSERT(size >= GrBufferAllocPool_MIN_BLOCK_SIZE);

    VALIDATE();

    BufferBlock& block = fBlocks.push_back();

    if (size == fMinBlockSize &&
        fPreallocBuffersInUse < fPreallocBuffers.count()) {

        uint32_t nextBuffer = (fPreallocBuffersInUse +
                               fPreallocBufferStartIdx) %
                              fPreallocBuffers.count();
        block.fBuffer = fPreallocBuffers[nextBuffer];
        block.fBuffer->ref();
        ++fPreallocBuffersInUse;
    } else {
        block.fBuffer = this->createBuffer(size);
        if (NULL == block.fBuffer) {
            fBlocks.pop_back();
            return false;
        }
    }

    block.fBytesFree = size;
    if (NULL != fBufferPtr) {
        SkASSERT(fBlocks.count() > 1);
        BufferBlock& prev = fBlocks.fromBack(1);
        if (prev.fBuffer->isMapped()) {
            UNMAP_BUFFER(prev);
        } else {
            this->flushCpuData(prev, prev.fBuffer->gpuMemorySize() - prev.fBytesFree);
        }
        fBufferPtr = NULL;
    }

    SkASSERT(NULL == fBufferPtr);

    // If the buffer is CPU-backed we map it because it is free to do so and saves a copy.
    // Otherwise when buffer mapping is supported:
    //      a) If the frequently reset hint is set we only map when the requested size meets a
    //      threshold (since we don't expect it is likely that we will see more vertex data)
    //      b) If the hint is not set we map if the buffer size is greater than the threshold.
    bool attemptMap = block.fBuffer->isCPUBacked();
    if (!attemptMap && GrDrawTargetCaps::kNone_MapFlags != fGpu->caps()->mapBufferFlags()) {
        if (fFrequentResetHint) {
            attemptMap = requestSize > GR_GEOM_BUFFER_MAP_THRESHOLD;
        } else {
            attemptMap = size > GR_GEOM_BUFFER_MAP_THRESHOLD;
        }
    }

    if (attemptMap) {
        fBufferPtr = block.fBuffer->map();
    }

    if (NULL == fBufferPtr) {
        fBufferPtr = fCpuData.reset(size);
    }

    VALIDATE(true);

    return true;
}

void GrBufferAllocPool::destroyBlock() {
    SkASSERT(!fBlocks.empty());

    BufferBlock& block = fBlocks.back();
    if (fPreallocBuffersInUse > 0) {
        uint32_t prevPreallocBuffer = (fPreallocBuffersInUse +
                                       fPreallocBufferStartIdx +
                                       (fPreallocBuffers.count() - 1)) %
                                      fPreallocBuffers.count();
        if (block.fBuffer == fPreallocBuffers[prevPreallocBuffer]) {
            --fPreallocBuffersInUse;
        }
    }
    SkASSERT(!block.fBuffer->isMapped());
    block.fBuffer->unref();
    fBlocks.pop_back();
    fBufferPtr = NULL;
}

void GrBufferAllocPool::flushCpuData(const BufferBlock& block, size_t flushSize) {
    GrGeometryBuffer* buffer = block.fBuffer;
    SkASSERT(NULL != buffer);
    SkASSERT(!buffer->isMapped());
    SkASSERT(fCpuData.get() == fBufferPtr);
    SkASSERT(flushSize <= buffer->gpuMemorySize());
    VALIDATE(true);

    if (GrDrawTargetCaps::kNone_MapFlags != fGpu->caps()->mapBufferFlags() &&
        flushSize > GR_GEOM_BUFFER_MAP_THRESHOLD) {
        void* data = buffer->map();
        if (NULL != data) {
            memcpy(data, fBufferPtr, flushSize);
            UNMAP_BUFFER(block);
            return;
        }
    }
    buffer->updateData(fBufferPtr, flushSize);
    VALIDATE(true);
}

GrGeometryBuffer* GrBufferAllocPool::createBuffer(size_t size) {
    if (kIndex_BufferType == fBufferType) {
        return fGpu->createIndexBuffer(size, true);
    } else {
        SkASSERT(kVertex_BufferType == fBufferType);
        return fGpu->createVertexBuffer(size, true);
    }
}

////////////////////////////////////////////////////////////////////////////////

GrVertexBufferAllocPool::GrVertexBufferAllocPool(GrGpu* gpu,
                                                 bool frequentResetHint,
                                                 size_t bufferSize,
                                                 int preallocBufferCnt)
: GrBufferAllocPool(gpu,
                    kVertex_BufferType,
                    frequentResetHint,
                    bufferSize,
                    preallocBufferCnt) {
}

void* GrVertexBufferAllocPool::makeSpace(size_t vertexSize,
                                         int vertexCount,
                                         const GrVertexBuffer** buffer,
                                         int* startVertex) {

    SkASSERT(vertexCount >= 0);
    SkASSERT(NULL != buffer);
    SkASSERT(NULL != startVertex);

    size_t offset = 0; // assign to suppress warning
    const GrGeometryBuffer* geomBuffer = NULL; // assign to suppress warning
    void* ptr = INHERITED::makeSpace(vertexSize * vertexCount,
                                     vertexSize,
                                     &geomBuffer,
                                     &offset);

    *buffer = (const GrVertexBuffer*) geomBuffer;
    SkASSERT(0 == offset % vertexSize);
    *startVertex = static_cast<int>(offset / vertexSize);
    return ptr;
}

bool GrVertexBufferAllocPool::appendVertices(size_t vertexSize,
                                             int vertexCount,
                                             const void* vertices,
                                             const GrVertexBuffer** buffer,
                                             int* startVertex) {
    void* space = makeSpace(vertexSize, vertexCount, buffer, startVertex);
    if (NULL != space) {
        memcpy(space,
               vertices,
               vertexSize * vertexCount);
        return true;
    } else {
        return false;
    }
}

int GrVertexBufferAllocPool::preallocatedBufferVertices(size_t vertexSize) const {
    return static_cast<int>(INHERITED::preallocatedBufferSize() / vertexSize);
}

int GrVertexBufferAllocPool::currentBufferVertices(size_t vertexSize) const {
    return currentBufferItems(vertexSize);
}

////////////////////////////////////////////////////////////////////////////////

GrIndexBufferAllocPool::GrIndexBufferAllocPool(GrGpu* gpu,
                                               bool frequentResetHint,
                                               size_t bufferSize,
                                               int preallocBufferCnt)
: GrBufferAllocPool(gpu,
                    kIndex_BufferType,
                    frequentResetHint,
                    bufferSize,
                    preallocBufferCnt) {
}

void* GrIndexBufferAllocPool::makeSpace(int indexCount,
                                        const GrIndexBuffer** buffer,
                                        int* startIndex) {

    SkASSERT(indexCount >= 0);
    SkASSERT(NULL != buffer);
    SkASSERT(NULL != startIndex);

    size_t offset = 0; // assign to suppress warning
    const GrGeometryBuffer* geomBuffer = NULL; // assign to suppress warning
    void* ptr = INHERITED::makeSpace(indexCount * sizeof(uint16_t),
                                     sizeof(uint16_t),
                                     &geomBuffer,
                                     &offset);

    *buffer = (const GrIndexBuffer*) geomBuffer;
    SkASSERT(0 == offset % sizeof(uint16_t));
    *startIndex = static_cast<int>(offset / sizeof(uint16_t));
    return ptr;
}

bool GrIndexBufferAllocPool::appendIndices(int indexCount,
                                           const void* indices,
                                           const GrIndexBuffer** buffer,
                                           int* startIndex) {
    void* space = makeSpace(indexCount, buffer, startIndex);
    if (NULL != space) {
        memcpy(space, indices, sizeof(uint16_t) * indexCount);
        return true;
    } else {
        return false;
    }
}

int GrIndexBufferAllocPool::preallocatedBufferIndices() const {
    return static_cast<int>(INHERITED::preallocatedBufferSize() / sizeof(uint16_t));
}

int GrIndexBufferAllocPool::currentBufferIndices() const {
    return currentBufferItems(sizeof(uint16_t));
}
