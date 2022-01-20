/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrVertexChunkArray.h"

#include "src/gpu/GrMeshDrawTarget.h"

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
