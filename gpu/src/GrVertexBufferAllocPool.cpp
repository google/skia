/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrVertexBufferAllocPool.h"
#include "GrVertexBuffer.h"
#include "GrGpu.h"

#define GrVertexBufferAllocPool_MIN_BLOCK_SIZE      ((size_t)1 << 10)

GrVertexBufferAllocPool::GrVertexBufferAllocPool(GrGpu* gpu,
                                                 size_t blockSize,
                                                 int preallocBufferCnt) :
        fBlocks(GrMax(8, 2*preallocBufferCnt)) {
    GrAssert(NULL != gpu);
    fGpu = gpu;
    fGpu->ref();
    fBufferPtr = NULL;
    fMinBlockSize = GrMax(GrVertexBufferAllocPool_MIN_BLOCK_SIZE, blockSize);

    fPreallocBuffersInUse = 0;
    fFirstPreallocBuffer = 0;
    for (int i = 0; i < preallocBufferCnt; ++i) {
        GrVertexBuffer* buffer = gpu->createVertexBuffer(fMinBlockSize, true);
        if (NULL != buffer) {
            *fPreallocBuffers.append() = buffer;
            buffer->ref();
        }
    }
}

GrVertexBufferAllocPool::~GrVertexBufferAllocPool() {
    fPreallocBuffers.unrefAll();
    while (!fBlocks.empty()) {
        destroyBlock();
    }
    fGpu->unref();
}

void GrVertexBufferAllocPool::reset() {
    while (!fBlocks.empty()) {
        destroyBlock();
    }
    if (fPreallocBuffers.count()) {
        // must set this after above loop.
        fFirstPreallocBuffer = (fFirstPreallocBuffer + fPreallocBuffersInUse) %
                               fPreallocBuffers.count();
    }
    GrAssert(0 == fPreallocBuffersInUse);
}

void GrVertexBufferAllocPool::unlock() {
    GrAssert((NULL == fBufferPtr) ? (!fBlocks.empty() ||
                                     !fBlocks.back().fVertexBuffer->isLocked()) :
                                    (!fBlocks.empty() &&
                                     fBlocks.back().fVertexBuffer->isLocked()));
    if (NULL != fBufferPtr) {
        GrAssert(!fBlocks.empty());
        GrAssert(fBlocks.back().fVertexBuffer->isLocked());
        fBufferPtr = NULL;
        fBlocks.back().fVertexBuffer->unlock();
    }
#if GR_DEBUG
    for (uint32_t i = 0; i < fBlocks.count(); ++i) {
        GrAssert(!fBlocks[i].fVertexBuffer->isLocked());
    }
#endif
}

void* GrVertexBufferAllocPool::alloc(GrVertexLayout layout,
                                     uint32_t vertexCount,
                                     GrVertexBuffer** buffer,
                                     uint32_t* startVertex) {
    GrAssert(NULL != buffer);
    size_t vSize = GrDrawTarget::VertexSize(layout);
    size_t bytes = vSize * vertexCount;

    if (NULL != fBufferPtr) {
        GrAssert(!fBlocks.empty());
        GrAssert(fBlocks.back().fVertexBuffer->isLocked());
        BufferBlock& back = fBlocks.back();
        uint32_t usedBytes = back.fVertexBuffer->size() - back.fBytesFree;
        uint32_t pad = GrUIAlignUpPad(usedBytes, layout);
        if ((bytes + pad) <= back.fBytesFree) {
            usedBytes += pad;
            *startVertex = usedBytes / vSize;
            *buffer = back.fVertexBuffer;
            back.fBytesFree -= bytes + pad;
            return (void*)((intptr_t)fBufferPtr + usedBytes);
        }
    }

    if (!createBlock(GrMax(bytes, fMinBlockSize))) {
        return NULL;
    }
    *startVertex = 0;
    GrAssert(NULL != fBufferPtr);
    BufferBlock& back = fBlocks.back();
    *buffer = back.fVertexBuffer;
    back.fBytesFree -= bytes;
    return fBufferPtr;
}

int GrVertexBufferAllocPool::currentBufferVertices(GrVertexLayout layout) const {
    if (NULL != fBufferPtr) {
        GrAssert(!fBlocks.empty());
        const BufferBlock& back = fBlocks.back();
        GrAssert(back.fVertexBuffer->isLocked());
        return back.fBytesFree / GrDrawTarget::VertexSize(layout);
    } else if (fPreallocBuffersInUse < fPreallocBuffers.count()) {
        return fMinBlockSize / GrDrawTarget::VertexSize(layout);
    }
    return 0;
}

int GrVertexBufferAllocPool::preallocatedBuffersRemaining() const {
    return fPreallocBuffers.count() - fPreallocBuffersInUse;
}

int GrVertexBufferAllocPool::preallocatedBufferVertices(GrVertexLayout layout) const {
    return fPreallocBuffers.count() ?
                        (fMinBlockSize / GrDrawTarget::VertexSize(layout)) :
                        0;
}

int GrVertexBufferAllocPool::preallocatedBufferCount() const {
    return fPreallocBuffers.count();
}

void GrVertexBufferAllocPool::release(size_t bytes) {
    if (NULL != fBufferPtr) {
        GrAssert(!fBlocks.empty());
        BufferBlock& back = fBlocks.back();
        GrAssert(back.fVertexBuffer->isLocked());
        size_t bytesUsed = back.fVertexBuffer->size() - back.fBytesFree;
        if (bytes >= bytesUsed) {
            destroyBlock();
            bytes -= bytesUsed;
        } else {
            back.fBytesFree += bytes;
            return;
        }
    }
    GrAssert(NULL == fBufferPtr);
    GrAssert(fBlocks.empty() ||
             !fBlocks.back().fVertexBuffer->isLocked());
    // we don't honor release if it is within an already unlocked VB
    // Our VB semantics say locking a VB discards its previous content
    while (!fBlocks.empty() &&
           bytes >= fBlocks.back().fVertexBuffer->size()) {
        bytes -= fBlocks.back().fVertexBuffer->size();
        destroyBlock();
    }
}

bool GrVertexBufferAllocPool::createBlock(size_t size) {
    GrAssert(size >= GrVertexBufferAllocPool_MIN_BLOCK_SIZE);

    BufferBlock& block = fBlocks.push_back();

    if (size == fMinBlockSize &&
        fPreallocBuffersInUse < fPreallocBuffers.count()) {

        uint32_t nextBuffer = (fPreallocBuffersInUse + fFirstPreallocBuffer) %
                              fPreallocBuffers.count();
        block.fVertexBuffer = fPreallocBuffers[nextBuffer];
        block.fVertexBuffer->ref();
        ++fPreallocBuffersInUse;
    } else {
        block.fVertexBuffer = fGpu->createVertexBuffer(size, true);
        if (NULL == block.fVertexBuffer) {
            fBlocks.pop_back();
            return false;
        }
    }

    block.fBytesFree = size;
    if (NULL != fBufferPtr) {
        GrAssert(fBlocks.count() > 1);
        BufferBlock& prev = fBlocks.fromBack(1);
        GrAssert(prev.fVertexBuffer->isLocked());
        fBufferPtr = NULL;
        prev.fVertexBuffer->unlock();
    }
    fBufferPtr = block.fVertexBuffer->lock();
    return true;
}

void GrVertexBufferAllocPool::destroyBlock() {
    GrAssert(!fBlocks.empty());

    BufferBlock& block = fBlocks.back();
    if (fPreallocBuffersInUse > 0) {
        uint32_t prevPreallocBuffer = (fPreallocBuffersInUse +
                                       fFirstPreallocBuffer +
                                       (fPreallocBuffers.count() - 1)) %
                                      fPreallocBuffers.count();
        if (block.fVertexBuffer == fPreallocBuffers[prevPreallocBuffer]) {
            --fPreallocBuffersInUse;
        }
    }
    block.fVertexBuffer->unref();
    fBlocks.pop_back();
    fBufferPtr = NULL;
}


