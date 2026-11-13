/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrVertexChunkArray.h"

#include "include/private/SkTo.h"
#include "src/gpu/ganesh/GrBufferAllocPool.h"
#include "src/gpu/ganesh/GrMeshDrawTarget.h"

#include <algorithm>

GrVertexChunkBuilder::~GrVertexChunkBuilder() {
    if (!fChunks->empty()) {
        fTarget->putBackVertices(fCurrChunkVertexCapacity - fCurrChunkVertexCount, fStride);
        fChunks->back().fCount = fCurrChunkVertexCount;
    }
}

bool GrVertexChunkBuilder::allocChunk(int minCount) {
    // Cap the initial heuristic preallocation to 512 KiB (16 * kDefaultBufferSize) so inflated
    // path verb counts cannot trigger multi-gigabyte allocations up front, while allowing
    // subsequent chunks to double up to 16 MiB (512 * kDefaultBufferSize) as vertices are actually
    // written to avoid excessive draw calls on large paths.
    static constexpr size_t kMaxInitialPreallocBytes = 16 * GrBufferAllocPool::kDefaultBufferSize;
    static constexpr size_t kMaxChunkGrowthBytes = 512 * GrBufferAllocPool::kDefaultBufferSize;
    if (!fChunks->empty()) {
        // No need to put back vertices; the buffer is full.
        fChunks->back().fCount = fCurrChunkVertexCount;
    } else {
        int maxInitialVertices = std::max(1, SkToInt(kMaxInitialPreallocBytes / fStride));
        fMinVerticesPerChunk = std::min(fMinVerticesPerChunk, maxInitialVertices);
    }
    fCurrChunkVertexCount = 0;
    GrVertexChunk* chunk = &fChunks->push_back();
    int minAllocCount = std::max(minCount, fMinVerticesPerChunk);
    fCurrChunkVertexWriter = fTarget->makeVertexWriterAtLeast(fStride, minAllocCount,
                                                              minAllocCount, &chunk->fBuffer,
                                                              &chunk->fBase,
                                                              &fCurrChunkVertexCapacity);
    if (!fCurrChunkVertexWriter && minAllocCount > minCount) SK_UNLIKELY {
        // If fStride > kDefaultBufferSize, kDefaultBufferSize / fStride is 0, so std::max
        // guarantees we still request at least minCount vertices.
        fMinVerticesPerChunk = std::max(minCount,
                                        SkToInt(GrBufferAllocPool::kDefaultBufferSize / fStride));
        fCurrChunkVertexWriter = fTarget->makeVertexWriterAtLeast(fStride, minCount,
                                                                  fMinVerticesPerChunk,
                                                                  &chunk->fBuffer,
                                                                  &chunk->fBase,
                                                                  &fCurrChunkVertexCapacity);
    }
    if (!fCurrChunkVertexWriter || !chunk->fBuffer ||
        fCurrChunkVertexCapacity < minCount) SK_UNLIKELY {
        SkDebugf("WARNING: Failed to allocate vertex buffer for GrVertexChunk.\n");
        fChunks->pop_back();
        SkASSERT(fCurrChunkVertexCount == 0);
        fCurrChunkVertexCapacity = 0;
        return false;
    }

    int maxVerticesPerChunk = std::max(1, SkToInt(kMaxChunkGrowthBytes / fStride));
    if (maxVerticesPerChunk / 2 > fMinVerticesPerChunk) {
        fMinVerticesPerChunk *= 2;
    } else {
        fMinVerticesPerChunk = maxVerticesPerChunk;
    }
    return true;
}
