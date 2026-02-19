/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrVertexChunkArray.h"

#include "src/gpu/ganesh/GrMeshDrawTarget.h"

#include <algorithm>
#include <limits>

GrVertexChunkBuilder::~GrVertexChunkBuilder() {
    if (!fChunks->empty()) {
        fTarget->putBackVertices(fCurrChunkVertexCapacity - fCurrChunkVertexCount, fStride);
        fChunks->back().fCount = fCurrChunkVertexCount;
    }
}

bool GrVertexChunkBuilder::allocChunk(int minCount) {
    if (!fChunks->empty()) {
        // No need to put back vertices; the buffer is full.
        fChunks->back().fCount = fCurrChunkVertexCount;
    }
    fCurrChunkVertexCount = 0;
    GrVertexChunk* chunk = &fChunks->push_back();
    int minAllocCount = std::max(minCount, fMinVerticesPerChunk);
    fCurrChunkVertexWriter = fTarget->makeVertexWriterAtLeast(fStride, minAllocCount,
                                                              minAllocCount, &chunk->fBuffer,
                                                              &chunk->fBase,
                                                              &fCurrChunkVertexCapacity);
    if (!fCurrChunkVertexWriter || !chunk->fBuffer || fCurrChunkVertexCapacity < minCount) SK_UNLIKELY {
        SkDebugf("WARNING: Failed to allocate vertex buffer for GrVertexChunk.\n");
        fChunks->pop_back();
        SkASSERT(fCurrChunkVertexCount == 0);
        fCurrChunkVertexCapacity = 0;
        return false;
    }

    int maxVerticesPerChunk = std::numeric_limits<int>::max() / fStride;
    if (maxVerticesPerChunk / 2 > fMinVerticesPerChunk) {
        fMinVerticesPerChunk *= 2;
    } else {
        fMinVerticesPerChunk = maxVerticesPerChunk;
    }
    return true;
}
