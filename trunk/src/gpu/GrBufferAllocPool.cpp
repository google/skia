
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrBufferAllocPool.h"
#include "GrTypes.h"
#include "GrVertexBuffer.h"
#include "GrIndexBuffer.h"
#include "GrGpu.h"

#if GR_DEBUG
    #define VALIDATE validate
#else
    static void VALIDATE(bool x = false) {}
#endif

// page size
#define GrBufferAllocPool_MIN_BLOCK_SIZE ((size_t)1 << 12)

GrBufferAllocPool::GrBufferAllocPool(GrGpu* gpu,
                                     BufferType bufferType,
                                     bool frequentResetHint,
                                     size_t blockSize,
                                     int preallocBufferCnt) :
        fBlocks(GrMax(8, 2*preallocBufferCnt)) {

    GrAssert(NULL != gpu);
    fGpu = gpu;
    fGpu->ref();
    fGpuIsReffed = true;

    fBufferType = bufferType;
    fFrequentResetHint = frequentResetHint;
    fBufferPtr = NULL;
    fMinBlockSize = GrMax(GrBufferAllocPool_MIN_BLOCK_SIZE, blockSize);

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
        if (buffer->isLocked()) {
            buffer->unlock();
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
        if (buffer->isLocked()) {
            buffer->unlock();
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
    GrAssert(0 == fPreallocBuffersInUse);
    VALIDATE();
}

void GrBufferAllocPool::unlock() {
    VALIDATE();

    if (NULL != fBufferPtr) {
        BufferBlock& block = fBlocks.back();
        if (block.fBuffer->isLocked()) {
            block.fBuffer->unlock();
        } else {
            size_t flushSize = block.fBuffer->sizeInBytes() - block.fBytesFree;
            flushCpuData(fBlocks.back().fBuffer, flushSize);
        }
        fBufferPtr = NULL;
    }
    VALIDATE();
}

#if GR_DEBUG
void GrBufferAllocPool::validate(bool unusedBlockAllowed) const {
    if (NULL != fBufferPtr) {
        GrAssert(!fBlocks.empty());
        if (fBlocks.back().fBuffer->isLocked()) {
            GrGeometryBuffer* buf = fBlocks.back().fBuffer;
            GrAssert(buf->lockPtr() == fBufferPtr);
        } else {
            GrAssert(fCpuData.get() == fBufferPtr);
        }
    } else {
        GrAssert(fBlocks.empty() || !fBlocks.back().fBuffer->isLocked());
    }
    size_t bytesInUse = 0;
    for (int i = 0; i < fBlocks.count() - 1; ++i) {
        GrAssert(!fBlocks[i].fBuffer->isLocked());
    }
    for (int i = 0; i < fBlocks.count(); ++i) {
        size_t bytes = fBlocks[i].fBuffer->sizeInBytes() - fBlocks[i].fBytesFree;
        bytesInUse += bytes;
        GrAssert(bytes || unusedBlockAllowed);
    }

    GrAssert(bytesInUse == fBytesInUse);
    if (unusedBlockAllowed) {
        GrAssert((fBytesInUse && !fBlocks.empty()) ||
                 (!fBytesInUse && (fBlocks.count() < 2)));
    } else {
        GrAssert((0 == fBytesInUse) == fBlocks.empty());
    }
}
#endif

void* GrBufferAllocPool::makeSpace(size_t size,
                                   size_t alignment,
                                   const GrGeometryBuffer** buffer,
                                   size_t* offset) {
    VALIDATE();

    GrAssert(NULL != buffer);
    GrAssert(NULL != offset);

    if (NULL != fBufferPtr) {
        BufferBlock& back = fBlocks.back();
        size_t usedBytes = back.fBuffer->sizeInBytes() - back.fBytesFree;
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
    GrAssert(NULL != fBufferPtr);

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
        size_t usedBytes = back.fBuffer->sizeInBytes() - back.fBytesFree;
        size_t pad = GrSizeAlignUpPad(usedBytes, itemSize);
        return (back.fBytesFree - pad) / itemSize;
    } else if (fPreallocBuffersInUse < fPreallocBuffers.count()) {
        return fMinBlockSize / itemSize;
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
        GrAssert(!fBlocks.empty());
        BufferBlock& block = fBlocks.back();
        size_t bytesUsed = block.fBuffer->sizeInBytes() - block.fBytesFree;
        if (bytes >= bytesUsed) {
            bytes -= bytesUsed;
            fBytesInUse -= bytesUsed;
            // if we locked a vb to satisfy the make space and we're releasing
            // beyond it, then unlock it.
            if (block.fBuffer->isLocked()) {
                block.fBuffer->unlock();
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

    size_t size = GrMax(requestSize, fMinBlockSize);
    GrAssert(size >= GrBufferAllocPool_MIN_BLOCK_SIZE);

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
        GrAssert(fBlocks.count() > 1);
        BufferBlock& prev = fBlocks.fromBack(1);
        if (prev.fBuffer->isLocked()) {
            prev.fBuffer->unlock();
        } else {
            flushCpuData(prev.fBuffer,
                         prev.fBuffer->sizeInBytes() - prev.fBytesFree);
        }
        fBufferPtr = NULL;
    }

    GrAssert(NULL == fBufferPtr);

    if (fGpu->getCaps().bufferLockSupport() &&
        size > GR_GEOM_BUFFER_LOCK_THRESHOLD &&
        (!fFrequentResetHint || requestSize > GR_GEOM_BUFFER_LOCK_THRESHOLD)) {
        fBufferPtr = block.fBuffer->lock();
    }

    if (NULL == fBufferPtr) {
        fBufferPtr = fCpuData.reset(size);
    }

    VALIDATE(true);

    return true;
}

void GrBufferAllocPool::destroyBlock() {
    GrAssert(!fBlocks.empty());

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
    GrAssert(!block.fBuffer->isLocked());
    block.fBuffer->unref();
    fBlocks.pop_back();
    fBufferPtr = NULL;
}

void GrBufferAllocPool::flushCpuData(GrGeometryBuffer* buffer,
                                     size_t flushSize) {
    GrAssert(NULL != buffer);
    GrAssert(!buffer->isLocked());
    GrAssert(fCpuData.get() == fBufferPtr);
    GrAssert(flushSize <= buffer->sizeInBytes());
    VALIDATE(true);

    if (fGpu->getCaps().bufferLockSupport() &&
        flushSize > GR_GEOM_BUFFER_LOCK_THRESHOLD) {
        void* data = buffer->lock();
        if (NULL != data) {
            memcpy(data, fBufferPtr, flushSize);
            buffer->unlock();
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
        GrAssert(kVertex_BufferType == fBufferType);
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

void* GrVertexBufferAllocPool::makeSpace(GrVertexLayout layout,
                                         int vertexCount,
                                         const GrVertexBuffer** buffer,
                                         int* startVertex) {

    GrAssert(vertexCount >= 0);
    GrAssert(NULL != buffer);
    GrAssert(NULL != startVertex);

    size_t vSize = GrDrawTarget::VertexSize(layout);
    size_t offset = 0; // assign to suppress warning
    const GrGeometryBuffer* geomBuffer = NULL; // assign to suppress warning
    void* ptr = INHERITED::makeSpace(vSize * vertexCount,
                                     vSize,
                                     &geomBuffer,
                                     &offset);

    *buffer = (const GrVertexBuffer*) geomBuffer;
    GrAssert(0 == offset % vSize);
    *startVertex = offset / vSize;
    return ptr;
}

bool GrVertexBufferAllocPool::appendVertices(GrVertexLayout layout,
                                             int vertexCount,
                                             const void* vertices,
                                             const GrVertexBuffer** buffer,
                                             int* startVertex) {
    void* space = makeSpace(layout, vertexCount, buffer, startVertex);
    if (NULL != space) {
        memcpy(space,
               vertices,
               GrDrawTarget::VertexSize(layout) * vertexCount);
        return true;
    } else {
        return false;
    }
}

int GrVertexBufferAllocPool::preallocatedBufferVertices(GrVertexLayout layout) const {
    return INHERITED::preallocatedBufferSize() /
            GrDrawTarget::VertexSize(layout);
}

int GrVertexBufferAllocPool::currentBufferVertices(GrVertexLayout layout) const {
    return currentBufferItems(GrDrawTarget::VertexSize(layout));
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

    GrAssert(indexCount >= 0);
    GrAssert(NULL != buffer);
    GrAssert(NULL != startIndex);

    size_t offset = 0; // assign to suppress warning
    const GrGeometryBuffer* geomBuffer = NULL; // assign to suppress warning
    void* ptr = INHERITED::makeSpace(indexCount * sizeof(uint16_t),
                                     sizeof(uint16_t),
                                     &geomBuffer,
                                     &offset);

    *buffer = (const GrIndexBuffer*) geomBuffer;
    GrAssert(0 == offset % sizeof(uint16_t));
    *startIndex = offset / sizeof(uint16_t);
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
    return INHERITED::preallocatedBufferSize() / sizeof(uint16_t);
}

int GrIndexBufferAllocPool::currentBufferIndices() const {
    return currentBufferItems(sizeof(uint16_t));
}
