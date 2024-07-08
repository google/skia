/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleMesh_DEFINED
#define GrSimpleMesh_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrBuffer.h"

#include <cstdint>
#include <utility>

/**
 * Used to communicate simple (non-instanced, direct) draws from GrOp to GrOpsRenderPass.
 * TODO: Consider migrating every Op to make the appropriate draw directly on GrOpsRenderPass.
 */
struct GrSimpleMesh {
    void set(sk_sp<const GrBuffer> vertexBuffer, int vertexCount, int baseVertex);
    void setIndexed(sk_sp<const GrBuffer> indexBuffer, int indexCount, int baseIndex,
                    uint16_t minIndexValue, uint16_t maxIndexValue, GrPrimitiveRestart,
                    sk_sp<const GrBuffer> vertexBuffer, int baseVertex);
    void setIndexedPatterned(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                             int patternRepeatCount, int maxPatternRepetitionsInIndexBuffer,
                             sk_sp<const GrBuffer> vertexBuffer, int patternVertexCount,
                             int baseVertex);

    sk_sp<const GrBuffer> fIndexBuffer;
    int fIndexCount;
    int fPatternRepeatCount;
    int fMaxPatternRepetitionsInIndexBuffer;
    int fBaseIndex;
    uint16_t fMinIndexValue;
    uint16_t fMaxIndexValue;
    GrPrimitiveRestart fPrimitiveRestart = GrPrimitiveRestart::kNo;

    sk_sp<const GrBuffer> fVertexBuffer;
    int fVertexCount;
    int fBaseVertex = 0;

    SkDEBUGCODE(bool fIsInitialized = false;)
};

inline void GrSimpleMesh::set(sk_sp<const GrBuffer> vertexBuffer, int vertexCount, int baseVertex) {
    SkASSERT(baseVertex >= 0);
    fIndexBuffer.reset();
    fVertexBuffer = std::move(vertexBuffer);
    fVertexCount = vertexCount;
    fBaseVertex = baseVertex;
    SkDEBUGCODE(fIsInitialized = true;)
}

inline void GrSimpleMesh::setIndexed(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                                     int baseIndex, uint16_t minIndexValue, uint16_t maxIndexValue,
                                     GrPrimitiveRestart primitiveRestart,
                                     sk_sp<const GrBuffer> vertexBuffer, int baseVertex) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(baseIndex >= 0);
    SkASSERT(maxIndexValue >= minIndexValue);
    SkASSERT(baseVertex >= 0);
    fIndexBuffer = std::move(indexBuffer);
    fIndexCount = indexCount;
    fPatternRepeatCount = 0;
    fBaseIndex = baseIndex;
    fMinIndexValue = minIndexValue;
    fMaxIndexValue = maxIndexValue;
    fPrimitiveRestart = primitiveRestart;
    fVertexBuffer = std::move(vertexBuffer);
    fBaseVertex = baseVertex;
    SkDEBUGCODE(fIsInitialized = true;)
}

inline void GrSimpleMesh::setIndexedPatterned(
        sk_sp<const GrBuffer> indexBuffer, int indexCount, int patternRepeatCount,
        int maxPatternRepetitionsInIndexBuffer, sk_sp<const GrBuffer> vertexBuffer,
        int patternVertexCount, int baseVertex) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(patternVertexCount >= 1);
    SkASSERT(patternRepeatCount >= 1);
    SkASSERT(maxPatternRepetitionsInIndexBuffer >= 1);
    SkASSERT(baseVertex >= 0);
    fIndexBuffer = std::move(indexBuffer);
    fIndexCount = indexCount;
    fPatternRepeatCount = patternRepeatCount;
    fVertexCount = patternVertexCount;
    fMaxPatternRepetitionsInIndexBuffer = maxPatternRepetitionsInIndexBuffer;
    fPrimitiveRestart = GrPrimitiveRestart::kNo;
    fVertexBuffer = std::move(vertexBuffer);
    fBaseVertex = baseVertex;
    SkDEBUGCODE(fIsInitialized = true;)
}

#endif
