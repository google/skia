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
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrResourceProvider.h"
#include "GrTypes.h"
#include "SkMacros.h"
#include "SkSafeMath.h"
#include "SkTraceEvent.h"

#ifdef SK_DEBUG
    #define VALIDATE validate
#else
    static void VALIDATE(bool = false) {}
#endif

#define UNMAP_BUFFER(block)                                                               \
do {                                                                                      \
    TRACE_EVENT_INSTANT1("skia.gpu",                                                      \
                         "GrBufferAllocPool Unmapping Buffer",                            \
                         TRACE_EVENT_SCOPE_THREAD,                                        \
                         "percent_unwritten",                                             \
                         (float)((block).fBytesFree) / (block).fBuffer->gpuMemorySize()); \
    (block).fBuffer->unmap();                                                             \
} while (false)

constexpr size_t GrBufferAllocPool::kDefaultBufferSize;

GrBufferAllocPool::GrBufferAllocPool(GrGpu* gpu, GrBufferType bufferType, void* initialBuffer)
        : fBlocks(8), fGpu(gpu), fBufferType(bufferType), fInitialCpuData(initialBuffer) {
    if (fInitialCpuData) {
        fCpuDataSize = kDefaultBufferSize;
        fCpuData = fInitialCpuData;
    }
}

void GrBufferAllocPool::deleteBlocks() {
    if (fBlocks.count()) {
        GrBuffer* buffer = fBlocks.back().fBuffer.get();
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
    if (fCpuData != fInitialCpuData) {
        sk_free(fCpuData);
    }
}

void GrBufferAllocPool::reset() {
    VALIDATE();
    fBytesInUse = 0;
    this->deleteBlocks();
    this->resetCpuData(0);
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
        if (!fBlocks.back().fBuffer->isMapped()) {
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
                                   sk_sp<const GrBuffer>* buffer,
                                   size_t* offset) {
    VALIDATE();

    SkASSERT(buffer);
    SkASSERT(offset);

    if (fBufferPtr) {
        BufferBlock& back = fBlocks.back();
        size_t usedBytes = back.fBuffer->gpuMemorySize() - back.fBytesFree;
        size_t pad = GrSizeAlignUpPad(usedBytes, alignment);
        SkSafeMath safeMath;
        size_t alignedSize = safeMath.add(pad, size);
        if (!safeMath.ok()) {
            return nullptr;
        }
        if (alignedSize <= back.fBytesFree) {
            memset((void*)(reinterpret_cast<intptr_t>(fBufferPtr) + usedBytes), 0, pad);
            usedBytes += pad;
            *offset = usedBytes;
            *buffer = back.fBuffer;
            back.fBytesFree -= alignedSize;
            fBytesInUse += alignedSize;
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

void* GrBufferAllocPool::makeSpaceAtLeast(size_t minSize,
                                          size_t fallbackSize,
                                          size_t alignment,
                                          sk_sp<const GrBuffer>* buffer,
                                          size_t* offset,
                                          size_t* actualSize) {
    VALIDATE();

    SkASSERT(buffer);
    SkASSERT(offset);
    SkASSERT(actualSize);

    if (fBufferPtr) {
        BufferBlock& back = fBlocks.back();
        size_t usedBytes = back.fBuffer->gpuMemorySize() - back.fBytesFree;
        size_t pad = GrSizeAlignUpPad(usedBytes, alignment);
        if ((minSize + pad) <= back.fBytesFree) {
            // Consume padding first, to make subsequent alignment math easier
            memset((void*)(reinterpret_cast<intptr_t>(fBufferPtr) + usedBytes), 0, pad);
            usedBytes += pad;
            back.fBytesFree -= pad;
            fBytesInUse += pad;

            // Give caller all remaining space in this block up to fallbackSize (but aligned
            // correctly)
            size_t size;
            if (back.fBytesFree >= fallbackSize) {
                SkASSERT(GrSizeAlignDown(fallbackSize, alignment) == fallbackSize);
                size = fallbackSize;
            } else {
                size = GrSizeAlignDown(back.fBytesFree, alignment);
            }
            *offset = usedBytes;
            *buffer = back.fBuffer;
            *actualSize = size;
            back.fBytesFree -= size;
            fBytesInUse += size;
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

    if (!this->createBlock(fallbackSize)) {
        return nullptr;
    }
    SkASSERT(fBufferPtr);

    *offset = 0;
    BufferBlock& back = fBlocks.back();
    *buffer = back.fBuffer;
    *actualSize = fallbackSize;
    back.fBytesFree -= fallbackSize;
    fBytesInUse += fallbackSize;
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
    size_t size = SkTMax(requestSize, kDefaultBufferSize);

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
        attemptMap = size > fGpu->caps()->bufferMapThreshold();
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
    SkASSERT(!fBlocks.back().fBuffer->isMapped());
    fBlocks.pop_back();
    fBufferPtr = nullptr;
}

void* GrBufferAllocPool::resetCpuData(size_t newSize) {
    if (newSize <= fCpuDataSize) {
        SkASSERT(!newSize || fCpuData);
        return fCpuData;
    }
    if (fCpuData != fInitialCpuData) {
        sk_free(fCpuData);
    }
    if (fGpu->caps()->mustClearUploadedBufferData()) {
        fCpuData = sk_calloc_throw(newSize);
    } else {
        fCpuData = sk_malloc_throw(newSize);
    }
    fCpuDataSize = newSize;
    return fCpuData;
}


void GrBufferAllocPool::flushCpuData(const BufferBlock& block, size_t flushSize) {
    GrBuffer* buffer = block.fBuffer.get();
    SkASSERT(buffer);
    SkASSERT(!buffer->isMapped());
    SkASSERT(fCpuData == fBufferPtr);
    SkASSERT(flushSize <= buffer->gpuMemorySize());
    VALIDATE(true);

    if (GrCaps::kNone_MapFlags != fGpu->caps()->mapBufferFlags() &&
        flushSize > fGpu->caps()->bufferMapThreshold()) {
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

sk_sp<GrBuffer> GrBufferAllocPool::getBuffer(size_t size) {
    auto resourceProvider = fGpu->getContext()->contextPriv().resourceProvider();

    return resourceProvider->createBuffer(size, fBufferType, kDynamic_GrAccessPattern,
            GrResourceProvider::Flags::kNone);
}

////////////////////////////////////////////////////////////////////////////////

GrVertexBufferAllocPool::GrVertexBufferAllocPool(GrGpu* gpu, void* initialCpuBuffer)
        : GrBufferAllocPool(gpu, kVertex_GrBufferType, initialCpuBuffer) {}

void* GrVertexBufferAllocPool::makeSpace(size_t vertexSize,
                                         int vertexCount,
                                         sk_sp<const GrBuffer>* buffer,
                                         int* startVertex) {
    SkASSERT(vertexCount >= 0);
    SkASSERT(buffer);
    SkASSERT(startVertex);

    size_t offset SK_INIT_TO_AVOID_WARNING;
    void* ptr = INHERITED::makeSpace(SkSafeMath::Mul(vertexSize, vertexCount),
                                     vertexSize,
                                     buffer,
                                     &offset);

    SkASSERT(0 == offset % vertexSize);
    *startVertex = static_cast<int>(offset / vertexSize);
    return ptr;
}

void* GrVertexBufferAllocPool::makeSpaceAtLeast(size_t vertexSize, int minVertexCount,
                                                int fallbackVertexCount,
                                                sk_sp<const GrBuffer>* buffer, int* startVertex,
                                                int* actualVertexCount) {
    SkASSERT(minVertexCount >= 0);
    SkASSERT(fallbackVertexCount >= minVertexCount);
    SkASSERT(buffer);
    SkASSERT(startVertex);
    SkASSERT(actualVertexCount);

    size_t offset SK_INIT_TO_AVOID_WARNING;
    size_t actualSize SK_INIT_TO_AVOID_WARNING;
    void* ptr = INHERITED::makeSpaceAtLeast(SkSafeMath::Mul(vertexSize, minVertexCount),
                                            SkSafeMath::Mul(vertexSize, fallbackVertexCount),
                                            vertexSize,
                                            buffer,
                                            &offset,
                                            &actualSize);

    SkASSERT(0 == offset % vertexSize);
    *startVertex = static_cast<int>(offset / vertexSize);

    SkASSERT(0 == actualSize % vertexSize);
    SkASSERT(actualSize >= vertexSize * minVertexCount);
    *actualVertexCount = static_cast<int>(actualSize / vertexSize);

    return ptr;
}

////////////////////////////////////////////////////////////////////////////////

GrIndexBufferAllocPool::GrIndexBufferAllocPool(GrGpu* gpu, void* initialCpuBuffer)
        : GrBufferAllocPool(gpu, kIndex_GrBufferType, initialCpuBuffer) {}

void* GrIndexBufferAllocPool::makeSpace(int indexCount, sk_sp<const GrBuffer>* buffer,
                                        int* startIndex) {
    SkASSERT(indexCount >= 0);
    SkASSERT(buffer);
    SkASSERT(startIndex);

    size_t offset SK_INIT_TO_AVOID_WARNING;
    void* ptr = INHERITED::makeSpace(SkSafeMath::Mul(indexCount, sizeof(uint16_t)),
                                     sizeof(uint16_t),
                                     buffer,
                                     &offset);

    SkASSERT(0 == offset % sizeof(uint16_t));
    *startIndex = static_cast<int>(offset / sizeof(uint16_t));
    return ptr;
}

void* GrIndexBufferAllocPool::makeSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                               sk_sp<const GrBuffer>* buffer, int* startIndex,
                                               int* actualIndexCount) {
    SkASSERT(minIndexCount >= 0);
    SkASSERT(fallbackIndexCount >= minIndexCount);
    SkASSERT(buffer);
    SkASSERT(startIndex);
    SkASSERT(actualIndexCount);

    size_t offset SK_INIT_TO_AVOID_WARNING;
    size_t actualSize SK_INIT_TO_AVOID_WARNING;
    void* ptr = INHERITED::makeSpaceAtLeast(SkSafeMath::Mul(minIndexCount, sizeof(uint16_t)),
                                            SkSafeMath::Mul(fallbackIndexCount, sizeof(uint16_t)),
                                            sizeof(uint16_t),
                                            buffer,
                                            &offset,
                                            &actualSize);

    SkASSERT(0 == offset % sizeof(uint16_t));
    *startIndex = static_cast<int>(offset / sizeof(uint16_t));

    SkASSERT(0 == actualSize % sizeof(uint16_t));
    SkASSERT(actualSize >= minIndexCount * sizeof(uint16_t));
    *actualIndexCount = static_cast<int>(actualSize / sizeof(uint16_t));
    return ptr;
}
