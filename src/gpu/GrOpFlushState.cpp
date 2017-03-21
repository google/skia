/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrOpFlushState.h"

#include "GrDrawOpAtlas.h"
#include "GrPipeline.h"

GrOpFlushState::GrOpFlushState(GrGpu* gpu, GrResourceProvider* resourceProvider)
        : fGpu(gpu)
        , fResourceProvider(resourceProvider)
        , fCommandBuffer(nullptr)
        , fVertexPool(gpu)
        , fIndexPool(gpu)
        , fLastIssuedToken(GrDrawOpUploadToken::AlreadyFlushedToken())
        , fLastFlushedToken(0)
        , fOpArgs(nullptr) {}

void* GrOpFlushState::makeVertexSpace(size_t vertexSize, int vertexCount,
                                         const GrBuffer** buffer, int* startVertex) {
    return fVertexPool.makeSpace(vertexSize, vertexCount, buffer, startVertex);
}

uint16_t* GrOpFlushState::makeIndexSpace(int indexCount,
                                            const GrBuffer** buffer, int* startIndex) {
    return reinterpret_cast<uint16_t*>(fIndexPool.makeSpace(indexCount, buffer, startIndex));
}
