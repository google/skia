/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchFlushState.h"

#include "GrBatchAtlas.h"
#include "GrPipeline.h"

GrBatchFlushState::GrBatchFlushState(GrGpu* gpu, GrResourceProvider* resourceProvider)
    : fGpu(gpu)
    , fResourceProvider(resourceProvider)
    , fVertexPool(gpu)
    , fIndexPool(gpu)
    , fLastIssuedToken(GrBatchDrawToken::AlreadyFlushedToken())
    , fLastFlushedToken(0) {}

void* GrBatchFlushState::makeVertexSpace(size_t vertexSize, int vertexCount,
                                         const GrBuffer** buffer, int* startVertex) {
    return fVertexPool.makeSpace(vertexSize, vertexCount, buffer, startVertex);
}

uint16_t* GrBatchFlushState::makeIndexSpace(int indexCount,
                                            const GrBuffer** buffer, int* startIndex) {
    return reinterpret_cast<uint16_t*>(fIndexPool.makeSpace(indexCount, buffer, startIndex));
}
