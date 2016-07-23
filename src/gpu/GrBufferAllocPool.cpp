/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrBufferAllocPool.h"
#include "GrBuffer.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrTypes.h"

#include "SkTraceEvent.h"

#ifdef SK_DEBUG
    #define VALIDATE validate
#else
    static void VALIDATE(bool = false) {}
#endif

static const size_t MIN_VERTEX_BUFFER_SIZE = 1 << 15;
static const size_t MIN_INDEX_BUFFER_SIZE = 1 << 12;

// page size
#define GrBufferAllocPool_MIN_BLOCK_SIZE ((size_t)1 << 15)

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
                                     GrBufferType bufferType,
                                     size_t blockSize)
    : fBlocks(8) {

    fGpu = SkRef(gpu);
    fCpuData = nullptr;
    fBufferType = bufferType;
    fBufferPtr = nullptr;
    fMinBlockSize = SkTMax(GrBufferAllocPool_MIN_BLOCK_SIZE, blockSize);

    fBytesInUse = 0;

    fBufferMapThreshold = gpu->caps()->bufferMapThreshold();
}

void GrBufferAllocPool::deleteBlocks() {
    if (fBlocks.count()) {
        GrBuffer* buffer = fBlocks.back().fBuffer;
        if (buffer->isMapped()) {
            UNMAP_BUFFER(fBlocks.back());
        }
    }
    while (!fBlocks.empty()) {
        this->destroyBlock();
    }
    SkASSERT(!fBufferPtr);
}

GrBufferAllocPool::~GrBufferAllocPool() {
    VALIDATE();
    this->deleteBlocks();
    sk_free(fCpuData);
    fGpu->unref();
}

void GrBufferAllocPool::reset() {
    VALIDATE();
    fBytesInUse = 0;
    this->deleteBlocks();

    // we may have created a large cpu mirror of a large VB. Reset the size to match our minimum.
    this->resetCpuData(fMinBlockSize);

    VALIDATE();
}

void GrBufferAllocPool::unmap() {
    VALIDATE();

    if (fBufferPtr) {
        BufferBlock& block = fBlocks.back();
        if (block.fBuffer->isMapped()) {
            UNMAP_BUFFER(block);
        } else {
            size_t flushSize = block.fBuffer->gpuMemorySize() - block.fBytesFree;
            this->flushCpuData(fBlocks.back(), flushSize);
        }
        fBufferPtr = nullptr;
    }
    VALIDATE();
}

#ifdef SK_DEBUG
void GrBufferAllocPool::validate(bool unusedBlockAllowed) const {
    bool wasDestroyed = false;
    if (fBufferPtr) {
        SkASSERT(!fBlocks.empty());
        if (fBlocks.back().fBuffer->isMapped()) {
            GrBuffer* buf = fBlocks.back().fBuffer;
            SkASSERT(buf->mapPtr() == fBufferPtr);
        } else {
            SkASSERT(fCpuData == fBufferPtr);
        }
    } else {
        SkASSERT(fBlocks.empty() || !fBlocks.back().fBuffer->isMapped());
    }
    size_t bytesInUse = 0;
    for (int i = 0; i < fBlocks.count() - 1; ++i) {
        SkASSERT(!fBlocks[i].fBuffer->isMapped());
    }
    for (int i = 0; !wasDestroyed && i < fBlocks.count(); ++i) {
        if (fBlocks[i].fBuffer->wasDestroyed()) {
            wasDestroyed = true;
        } else {
            size_t bytes = fBlocks[i].fBuffer->gpuMemorySize() - fBlocks[i].fBytesFree;
            bytesInUse += bytes;
            SkASSERT(bytes || unusedBlockAllowed);
        }
    }

    if (!wasDestroyed) {
        SkASSERT(bytesInUse == fBytesInUse);
        if (unusedBlockAllowed) {
            SkASSERT((fBytesInUse && !fBlocks.empty()) ||
                     (!fBytesInUse && (fBlocks.count() < 2)));
        } else {
            SkASSERT((0 == fBytesInUse) == fBlocks.empty());
        }
    }
}
#endif

void* GrBufferAllocPool::makeSpace(size_t size,
                                   size_t alignment,
                                   const GrBuffer** buffer,
                                   size_t* offset) {
    VALIDATE();

    SkASSERT(buffer);
    SkASSERT(offset);

    if (fBufferPtr) {
        BufferBlock& back = fBlocks.back();
        size_t usedBytes = back.fBuffer->gpuMemorySize() - back.fBytesFree;
        size_t pad = GrSizeAlignUpPad(usedBytes, alignment);
        if ((size + pad) <= back.fBytesFree) {
            memset((void*)(reinterpret_cast<intptr_t>(fBufferPtr) + usedBytes), 0, pad);
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

    if (!this->createBlock(size)) {
        return nullptr;
    }
    SkASSERT(fBufferPtr);

    *offset = 0;
    BufferBlock& back = fBlocks.back();
    *buffer = back.fBuffer;
    back.fBytesFree -= size;
    fBytesInUse += size;
    VALIDATE();
    return fBufferPtr;
}

void GrBufferAllocPool::putBack(size_t bytes) {
    VALIDATE();

    while (bytes) {
        // caller shouldn't try to put back more than they've taken
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

    VALIDATE();
}

bool GrBufferAllocPool::createBlock(size_t requestSize) {

    size_t size = SkTMax(requestSize, fMinBlockSize);
    SkASSERT(size >= GrBufferAllocPool_MIN_BLOCK_SIZE);

    VALIDATE();

    BufferBlock& block = fBlocks.push_back();

    block.fBuffer = this->getBuffer(size);
    if (!block.fBuffer) {
        fBlocks.pop_back();
        return false;
    }

    block.fBytesFree = block.fBuffer->gpuMemorySize();
    if (fBufferPtr) {
        SkASSERT(fBlocks.count() > 1);
        BufferBlock& prev = fBlocks.fromBack(1);
        if (prev.fBuffer->isMapped()) {
            UNMAP_BUFFER(prev);
        } else {
            this->flushCpuData(prev, prev.fBuffer->gpuMemorySize() - prev.fBytesFree);
        }
        fBufferPtr = nullptr;
    }

    SkASSERT(!fBufferPtr);

    // If the buffer is CPU-backed we map it because it is free to do so and saves a copy.
    // Otherwise when buffer mapping is supported we map if the buffer size is greater than the
    // threshold.
    bool attemptMap = block.fBuffer->isCPUBacked();
    if (!attemptMap && GrCaps::kNone_MapFlags != fGpu->caps()->mapBufferFlags()) {
        attemptMap = size > fBufferMapThreshold;
    }

    if (attemptMap) {
        fBufferPtr = block.fBuffer->map();
    }

    if (!fBufferPtr) {
        fBufferPtr = this->resetCpuData(block.fBytesFree);
    }

    VALIDATE(true);

    return true;
}

void GrBufferAllocPool::destroyBlock() {
    SkASSERT(!fBlocks.empty());

    BufferBlock& block = fBlocks.back();

    SkASSERT(!block.fBuffer->isMapped());
    block.fBuffer->unref();
    fBlocks.pop_back();
    fBufferPtr = nullptr;
}

void* GrBufferAllocPool::resetCpuData(size_t newSize) {
    sk_free(fCpuData);
    if (newSize) {
        if (fGpu->caps()->mustClearUploadedBufferData()) {
            fCpuData = sk_calloc(newSize);
        } else {
            fCpuData = sk_malloc_throw(newSize);
        }
    } else {
        fCpuData = nullptr;
    }
    return fCpuData;
}


void GrBufferAllocPool::flushCpuData(const BufferBlock& block, size_t flushSize) {
    GrBuffer* buffer = block.fBuffer;
    SkASSERT(buffer);
    SkASSERT(!buffer->isMapped());
    SkASSERT(fCpuData == fBufferPtr);
    SkASSERT(flushSize <= buffer->gpuMemorySize());
    VALIDATE(true);

    if (GrCaps::kNone_MapFlags != fGpu->caps()->mapBufferFlags() &&
        flushSize > fBufferMapThreshold) {
        void* data = buffer->map();
        if (data) {
            memcpy(data, fBufferPtr, flushSize);
            UNMAP_BUFFER(block);
            return;
        }
    }
    buffer->updateData(fBufferPtr, flushSize);
    VALIDATE(true);
}

GrBuffer* GrBufferAllocPool::getBuffer(size_t size) {

    GrResourceProvider* rp = fGpu->getContext()->resourceProvider();

    // Shouldn't have to use this flag (https://bug.skia.org/4156)
    static const uint32_t kFlags = GrResourceProvider::kNoPendingIO_Flag;
    return rp->createBuffer(size, fBufferType, kDynamic_GrAccessPattern, kFlags);
}

////////////////////////////////////////////////////////////////////////////////

GrVertexBufferAllocPool::GrVertexBufferAllocPool(GrGpu* gpu)
    : GrBufferAllocPool(gpu, kVertex_GrBufferType, MIN_VERTEX_BUFFER_SIZE) {
}

void* GrVertexBufferAllocPool::makeSpace(size_t vertexSize,
                                         int vertexCount,
                                         const GrBuffer** buffer,
                                         int* startVertex) {

    SkASSERT(vertexCount >= 0);
    SkASSERT(buffer);
    SkASSERT(startVertex);

    size_t offset = 0; // assign to suppress warning
    void* ptr = INHERITED::makeSpace(vertexSize * vertexCount,
                                     vertexSize,
                                     buffer,
                                     &offset);

    SkASSERT(0 == offset % vertexSize);
    *startVertex = static_cast<int>(offset / vertexSize);
    return ptr;
}

////////////////////////////////////////////////////////////////////////////////

GrIndexBufferAllocPool::GrIndexBufferAllocPool(GrGpu* gpu)
    : GrBufferAllocPool(gpu, kIndex_GrBufferType, MIN_INDEX_BUFFER_SIZE) {
}

void* GrIndexBufferAllocPool::makeSpace(int indexCount,
                                        const GrBuffer** buffer,
                                        int* startIndex) {

    SkASSERT(indexCount >= 0);
    SkASSERT(buffer);
    SkASSERT(startIndex);

    size_t offset = 0; // assign to suppress warning
    void* ptr = INHERITED::makeSpace(indexCount * sizeof(uint16_t),
                                     sizeof(uint16_t),
                                     buffer,
                                     &offset);

    SkASSERT(0 == offset % sizeof(uint16_t));
    *startIndex = static_cast<int>(offset / sizeof(uint16_t));
    return ptr;
}
