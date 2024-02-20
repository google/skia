/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkMacros.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrBufferAllocPool.h"

#include <memory>
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrCpuBuffer.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrResourceProvider.h"

sk_sp<GrBufferAllocPool::CpuBufferCache> GrBufferAllocPool::CpuBufferCache::Make(
        int maxBuffersToCache) {
    return sk_sp<CpuBufferCache>(new CpuBufferCache(maxBuffersToCache));
}

GrBufferAllocPool::CpuBufferCache::CpuBufferCache(int maxBuffersToCache)
        : fMaxBuffersToCache(maxBuffersToCache) {
    if (fMaxBuffersToCache) {
        fBuffers = std::make_unique<Buffer[]>(fMaxBuffersToCache);
    }
}

sk_sp<GrCpuBuffer> GrBufferAllocPool::CpuBufferCache::makeBuffer(size_t size,
                                                                 bool mustBeInitialized) {
    SkASSERT(size > 0);
    Buffer* result = nullptr;
    if (size == kDefaultBufferSize) {
        int i = 0;
        for (; i < fMaxBuffersToCache && fBuffers[i].fBuffer; ++i) {
            SkASSERT(fBuffers[i].fBuffer->size() == kDefaultBufferSize);
            if (fBuffers[i].fBuffer->unique()) {
                result = &fBuffers[i];
            }
        }
        if (!result && i < fMaxBuffersToCache) {
            fBuffers[i].fBuffer = GrCpuBuffer::Make(size);
            result = &fBuffers[i];
        }
    }
    Buffer tempResult;
    if (!result) {
        tempResult.fBuffer = GrCpuBuffer::Make(size);
        result = &tempResult;
    }
    if (mustBeInitialized && !result->fCleared) {
        result->fCleared = true;
        memset(result->fBuffer->data(), 0, result->fBuffer->size());
    }
    return result->fBuffer;
}

void GrBufferAllocPool::CpuBufferCache::releaseAll() {
    for (int i = 0; i < fMaxBuffersToCache && fBuffers[i].fBuffer; ++i) {
        fBuffers[i].fBuffer.reset();
        fBuffers[i].fCleared = false;
    }
}

//////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
    #define VALIDATE validate
#else
    static void VALIDATE(bool = false) {}
#endif

#define UNMAP_BUFFER(block)                                                          \
    do {                                                                             \
        TRACE_EVENT_INSTANT1("skia.gpu", "GrBufferAllocPool Unmapping Buffer",       \
                             TRACE_EVENT_SCOPE_THREAD, "percent_unwritten",          \
                             (float)((block).fBytesFree) / (block).fBuffer->size()); \
        SkASSERT(!block.fBuffer->isCpuBuffer());                                     \
        static_cast<GrGpuBuffer*>(block.fBuffer.get())->unmap();                     \
    } while (false)

GrBufferAllocPool::GrBufferAllocPool(GrGpu* gpu, GrGpuBufferType bufferType,
                                     sk_sp<CpuBufferCache> cpuBufferCache)
        : fBlocks(8)
        , fCpuBufferCache(std::move(cpuBufferCache))
        , fGpu(gpu)
        , fBufferType(bufferType) {}

void GrBufferAllocPool::deleteBlocks() {
    if (!fBlocks.empty()) {
        GrBuffer* buffer = fBlocks.back().fBuffer.get();
        if (!buffer->isCpuBuffer() && static_cast<GrGpuBuffer*>(buffer)->isMapped()) {
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
        GrBuffer* buffer = block.fBuffer.get();
        if (!buffer->isCpuBuffer()) {
            if (static_cast<GrGpuBuffer*>(buffer)->isMapped()) {
                UNMAP_BUFFER(block);
            } else {
                size_t flushSize = block.fBuffer->size() - block.fBytesFree;
                this->flushCpuData(fBlocks.back(), flushSize);
            }
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
        const GrBuffer* buffer = fBlocks.back().fBuffer.get();
        if (!buffer->isCpuBuffer() && !static_cast<const GrGpuBuffer*>(buffer)->isMapped()) {
            SkASSERT(fCpuStagingBuffer && fCpuStagingBuffer->data() == fBufferPtr);
        }
    } else if (!fBlocks.empty()) {
        const GrBuffer* buffer = fBlocks.back().fBuffer.get();
        SkASSERT(buffer->isCpuBuffer() || !static_cast<const GrGpuBuffer*>(buffer)->isMapped());
    }
    size_t bytesInUse = 0;
    for (int i = 0; i < fBlocks.size() - 1; ++i) {
        const GrBuffer* buffer = fBlocks[i].fBuffer.get();
        SkASSERT(buffer->isCpuBuffer() || !static_cast<const GrGpuBuffer*>(buffer)->isMapped());
    }
    for (int i = 0; !wasDestroyed && i < fBlocks.size(); ++i) {
        GrBuffer* buffer = fBlocks[i].fBuffer.get();
        if (!buffer->isCpuBuffer() && static_cast<GrGpuBuffer*>(buffer)->wasDestroyed()) {
            wasDestroyed = true;
        } else {
            size_t bytes = fBlocks[i].fBuffer->size() - fBlocks[i].fBytesFree;
            bytesInUse += bytes;
            SkASSERT(bytes || unusedBlockAllowed);
        }
    }

    if (!wasDestroyed) {
        SkASSERT(bytesInUse == fBytesInUse);
        if (unusedBlockAllowed) {
            SkASSERT((fBytesInUse && !fBlocks.empty()) ||
                     (!fBytesInUse && (fBlocks.size() < 2)));
        } else {
            SkASSERT((0 == fBytesInUse) == fBlocks.empty());
        }
    }
}
#endif

static inline size_t align_up_pad(size_t x, size_t alignment) {
    return (alignment - x % alignment) % alignment;
}

static inline size_t align_down(size_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}

void* GrBufferAllocPool::makeSpace(size_t size,
                                   size_t alignment,
                                   sk_sp<const GrBuffer>* buffer,
                                   size_t* offset) {
    VALIDATE();

    SkASSERT(buffer);
    SkASSERT(offset);

    if (fBufferPtr) {
        BufferBlock& back = fBlocks.back();
        size_t usedBytes = back.fBuffer->size() - back.fBytesFree;
        size_t pad = align_up_pad(usedBytes, alignment);
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
    // the part of the buffer we update. Also, when this was written the GL
    // buffer implementation was cheating on the actual buffer size by shrinking
    // the buffer in updateData() if the amount of data passed was less than
    // the full buffer size. This is old code and both concerns may be obsolete.

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

    size_t usedBytes = (fBlocks.empty()) ? 0 : fBlocks.back().fBuffer->size() -
                                               fBlocks.back().fBytesFree;
    size_t pad = align_up_pad(usedBytes, alignment);
    if (!fBufferPtr || fBlocks.empty() || (minSize + pad) > fBlocks.back().fBytesFree) {
        // We either don't have a block yet or the current block doesn't have enough free space.
        // Create a new one.
        if (!this->createBlock(fallbackSize)) {
            return nullptr;
        }
        usedBytes = 0;
        pad = 0;
    }
    SkASSERT(fBufferPtr);

    // Consume padding first, to make subsequent alignment math easier
    memset(static_cast<char*>(fBufferPtr) + usedBytes, 0, pad);
    usedBytes += pad;
    fBlocks.back().fBytesFree -= pad;
    fBytesInUse += pad;

    // Give caller all remaining space in this block (but aligned correctly)
    size_t size = align_down(fBlocks.back().fBytesFree, alignment);
    *offset = usedBytes;
    *buffer = fBlocks.back().fBuffer;
    *actualSize = size;
    fBlocks.back().fBytesFree -= size;
    fBytesInUse += size;
    VALIDATE();
    return static_cast<char*>(fBufferPtr) + usedBytes;
}

void GrBufferAllocPool::putBack(size_t bytes) {
    VALIDATE();
    if (!bytes) {
        return;
    }
    SkASSERT(!fBlocks.empty());
    BufferBlock& block = fBlocks.back();
    // Caller shouldn't try to put back more than they've taken and all those bytes should fit into
    // one block. All the uses of this call are sequential with a single makeSpaceAtLeast call. So
    // we should not have a case where someone is putting back bytes that are greater than the
    // current block.
    // It is possible the caller returns all their allocated bytes thus the <= and not just <.
    SkASSERT(bytes <= (block.fBuffer->size() - block.fBytesFree));
    block.fBytesFree += bytes;
    fBytesInUse -= bytes;

    // We don't allow blocks without any used bytes. So if we end up in that case after putting
    // back the bytes then destroy the block. This scenario shouldn't occur often, but even if we
    // end up allocating a new block immediately after destroying this one, the GPU and CPU buffers
    // will usually be cached so the new block shouldn't be too expensive to make.
    // TODO: This was true in older versions and uses of this class but is it still needed to
    // have this restriction?
    if (block.fBytesFree == block.fBuffer->size()) {
        GrBuffer* buffer = block.fBuffer.get();
        if (!buffer->isCpuBuffer() && static_cast<GrGpuBuffer*>(buffer)->isMapped()) {
            UNMAP_BUFFER(block);
        }
        this->destroyBlock();
    }

    VALIDATE();
}

bool GrBufferAllocPool::createBlock(size_t requestSize) {
    size_t size = std::max(requestSize, kDefaultBufferSize);

    VALIDATE();

    BufferBlock& block = fBlocks.push_back();

    block.fBuffer = this->getBuffer(size);
    if (!block.fBuffer) {
        fBlocks.pop_back();
        return false;
    }

    block.fBytesFree = block.fBuffer->size();
    if (fBufferPtr) {
        SkASSERT(fBlocks.size() > 1);
        BufferBlock& prev = fBlocks.fromBack(1);
        GrBuffer* buffer = prev.fBuffer.get();
        if (!buffer->isCpuBuffer()) {
            if (static_cast<GrGpuBuffer*>(buffer)->isMapped()) {
                UNMAP_BUFFER(prev);
            } else {
                this->flushCpuData(prev, prev.fBuffer->size() - prev.fBytesFree);
            }
        }
        fBufferPtr = nullptr;
    }

    SkASSERT(!fBufferPtr);

    // If the buffer is CPU-backed we "map" it because it is free to do so and saves a copy.
    // Otherwise when buffer mapping is supported we map if the buffer size is greater than the
    // threshold.
    if (block.fBuffer->isCpuBuffer()) {
        fBufferPtr = static_cast<GrCpuBuffer*>(block.fBuffer.get())->data();
        SkASSERT(fBufferPtr);
    } else {
        if (GrCaps::kNone_MapFlags != fGpu->caps()->mapBufferFlags() &&
            size > fGpu->caps()->bufferMapThreshold()) {
            fBufferPtr = static_cast<GrGpuBuffer*>(block.fBuffer.get())->map();
        }
    }
    if (!fBufferPtr) {
        this->resetCpuData(block.fBytesFree);
        fBufferPtr = fCpuStagingBuffer->data();
    }

    VALIDATE(true);

    return true;
}

void GrBufferAllocPool::destroyBlock() {
    SkASSERT(!fBlocks.empty());
    SkASSERT(fBlocks.back().fBuffer->isCpuBuffer() ||
             !static_cast<GrGpuBuffer*>(fBlocks.back().fBuffer.get())->isMapped());
    fBlocks.pop_back();
    fBufferPtr = nullptr;
}

void GrBufferAllocPool::resetCpuData(size_t newSize) {
    SkASSERT(newSize >= kDefaultBufferSize || !newSize);
    if (!newSize) {
        fCpuStagingBuffer.reset();
        return;
    }
    if (fCpuStagingBuffer && newSize <= fCpuStagingBuffer->size()) {
        return;
    }
    bool mustInitialize = fGpu->caps()->mustClearUploadedBufferData();
    fCpuStagingBuffer = fCpuBufferCache ? fCpuBufferCache->makeBuffer(newSize, mustInitialize)
                                        : GrCpuBuffer::Make(newSize);
}

void GrBufferAllocPool::flushCpuData(const BufferBlock& block, size_t flushSize) {
    SkASSERT(block.fBuffer.get());
    SkASSERT(!block.fBuffer.get()->isCpuBuffer());
    GrGpuBuffer* buffer = static_cast<GrGpuBuffer*>(block.fBuffer.get());
    SkASSERT(!buffer->isMapped());
    SkASSERT(fCpuStagingBuffer && fCpuStagingBuffer->data() == fBufferPtr);
    SkASSERT(flushSize <= buffer->size());
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
    buffer->updateData(fBufferPtr, /*offset=*/0, flushSize, /*preserve=*/false);
    VALIDATE(true);
}

sk_sp<GrBuffer> GrBufferAllocPool::getBuffer(size_t size) {
    const GrCaps& caps = *fGpu->caps();
    auto resourceProvider = fGpu->getContext()->priv().resourceProvider();
    if (caps.preferClientSideDynamicBuffers() ||
        (fBufferType == GrGpuBufferType::kDrawIndirect && caps.useClientSideIndirectBuffers())) {
        // Create a CPU buffer.
        bool mustInitialize = caps.mustClearUploadedBufferData();
        return fCpuBufferCache ? fCpuBufferCache->makeBuffer(size, mustInitialize)
                               : GrCpuBuffer::Make(size);
    }
    return resourceProvider->createBuffer(size,
                                          fBufferType,
                                          kDynamic_GrAccessPattern,
                                          GrResourceProvider::ZeroInit::kNo);
}

////////////////////////////////////////////////////////////////////////////////

GrVertexBufferAllocPool::GrVertexBufferAllocPool(GrGpu* gpu, sk_sp<CpuBufferCache> cpuBufferCache)
        : GrBufferAllocPool(gpu, GrGpuBufferType::kVertex, std::move(cpuBufferCache)) {}

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

GrIndexBufferAllocPool::GrIndexBufferAllocPool(GrGpu* gpu, sk_sp<CpuBufferCache> cpuBufferCache)
        : GrBufferAllocPool(gpu, GrGpuBufferType::kIndex, std::move(cpuBufferCache)) {}

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
