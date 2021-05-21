/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexChunkArray_DEFINED
#define GrVertexChunkArray_DEFINED

#include "include/private/SkNoncopyable.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

// Represents a chunk of vertex data. Use with GrVertexChunkArray and GrVertexChunkBuilder. We write
// the data out in chunks when we don't start out knowing exactly how many vertices (or instances)
// we will end up writing.
struct GrVertexChunk {
    sk_sp<const GrBuffer> fBuffer;
    int fCount = 0;
    int fBase;  // baseVertex or baseInstance, depending on the use case.
};

// Represents an array of GrVertexChunks.
//
// We only preallocate 1 chunk because if the array needs to grow, then we're also allocating a
// brand new GPU buffer anyway.
using GrVertexChunkArray = SkSTArray<1, GrVertexChunk>;

// Builds a GrVertexChunkArray. The provided Target must not be used externally throughout the
// entire lifetime of this object.
class GrVertexChunkBuilder : SkNoncopyable {
public:
    GrVertexChunkBuilder(GrMeshDrawOp::Target* target, GrVertexChunkArray* chunks, size_t stride,
                         int minVerticesPerChunk)
            : fTarget(target)
            , fChunks(chunks)
            , fStride(stride)
            , fMinVerticesPerChunk(minVerticesPerChunk) {
        SkASSERT(fMinVerticesPerChunk > 0);
    }

    ~GrVertexChunkBuilder() {
        if (!fChunks->empty()) {
            fTarget->putBackVertices(fCurrChunkVertexCapacity - fCurrChunkVertexCount, fStride);
            fChunks->back().fCount = fCurrChunkVertexCount;
        }
    }

    // Appends 'count' contiguous vertices. These vertices are not guaranteed to be contiguous with
    // previous or future calls to appendVertices.
    SK_ALWAYS_INLINE GrVertexWriter appendVertices(int count) {
        SkASSERT(count > 0);
        if (fCurrChunkVertexCount + count > fCurrChunkVertexCapacity && !this->allocChunk(count)) {
            SkDEBUGCODE(fLastAppendAmount = 0;)
            return {nullptr};
        }
        SkASSERT(fCurrChunkVertexCount + count <= fCurrChunkVertexCapacity);
        fCurrChunkVertexCount += count;
        SkDEBUGCODE(fLastAppendAmount = count;)
        return std::exchange(fCurrChunkVertexWriter,
                             fCurrChunkVertexWriter.makeOffset(fStride * count));
    }

    SK_ALWAYS_INLINE GrVertexWriter appendVertex() { return this->appendVertices(1); }

    // Pops the most recent 'count' contiguous vertices. Since there is no guarantee of contiguity
    // between appends, 'count' may be no larger than the most recent call to appendVertices().
    void popVertices(int count) {
        SkASSERT(count <= fLastAppendAmount);
        SkASSERT(fLastAppendAmount <= fCurrChunkVertexCount);
        SkASSERT(count >= 0);
        fCurrChunkVertexCount -= count;
        fCurrChunkVertexWriter = fCurrChunkVertexWriter.makeOffset(fStride * -count);
        SkDEBUGCODE(fLastAppendAmount -= count;)
    }

private:
    bool allocChunk(int minCount) {
        if (!fChunks->empty()) {
            // No need to put back vertices; the buffer is full.
            fChunks->back().fCount = fCurrChunkVertexCount;
        }
        fCurrChunkVertexCount = 0;
        GrVertexChunk* chunk = &fChunks->push_back();
        int minAllocCount = std::max(minCount, fMinVerticesPerChunk);
        fCurrChunkVertexWriter = {fTarget->makeVertexSpaceAtLeast(fStride, minAllocCount,
                                                                  minAllocCount, &chunk->fBuffer,
                                                                  &chunk->fBase,
                                                                  &fCurrChunkVertexCapacity)};
        if (!fCurrChunkVertexWriter || !chunk->fBuffer || fCurrChunkVertexCapacity < minCount) {
            SkDebugf("WARNING: Failed to allocate vertex buffer for GrVertexChunk.\n");
            fChunks->pop_back();
            SkASSERT(fCurrChunkVertexCount == 0);
            fCurrChunkVertexCapacity = 0;
            return false;
        }
        fMinVerticesPerChunk *= 2;
        return true;
    }

    GrMeshDrawOp::Target* const fTarget;
    GrVertexChunkArray* const fChunks;
    const size_t fStride;
    int fMinVerticesPerChunk;

    GrVertexWriter fCurrChunkVertexWriter;
    int fCurrChunkVertexCount = 0;
    int fCurrChunkVertexCapacity = 0;

    SkDEBUGCODE(int fLastAppendAmount = 0;)
};

#endif
