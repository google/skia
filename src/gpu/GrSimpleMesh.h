/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleMesh_DEFINED
#define GrSimpleMesh_DEFINED

#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrOpsRenderPass.h"

class GrPrimitiveProcessor;

/**
 * Used to communicate simple (non-instanced, direct) draws from GrOp to GrOpsRenderPass.
 * TODO: Consider migrating every Op to make the appropriate draw directly on GrOpsRenderPass.
 */
class GrSimpleMesh {
public:
    const GrBuffer* indexBuffer() const { return fIndexBuffer.get(); }
    const GrBuffer* vertexBuffer() const { return fVertexBuffer.get(); }

    void set(sk_sp<const GrBuffer> vertexBuffer, int vertexCount, int baseVertex);
    void setIndexed(sk_sp<const GrBuffer> indexBuffer, int indexCount, int baseIndex,
                    uint16_t minIndexValue, uint16_t maxIndexValue, GrPrimitiveRestart,
                    sk_sp<const GrBuffer> vertexBuffer, int baseVertex);
    void setIndexedPatterned(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                             int patternRepeatCount, int maxPatternRepetitionsInIndexBuffer,
                             sk_sp<const GrBuffer> vertexBuffer, int patternVertexCount,
                             int baseVertex);

    void draw(GrOpsRenderPass*) const;

private:
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

inline void GrSimpleMesh::draw(GrOpsRenderPass* opsRenderPass) const {
    SkASSERT(fIsInitialized);

    if (!fIndexBuffer) {
        opsRenderPass->bindBuffers(nullptr, nullptr, fVertexBuffer.get());
        opsRenderPass->draw(fVertexCount, fBaseVertex);
    } else {
        opsRenderPass->bindBuffers(fIndexBuffer.get(), nullptr, fVertexBuffer.get(),
                                   fPrimitiveRestart);
        if (0 == fPatternRepeatCount) {
            opsRenderPass->drawIndexed(fIndexCount, fBaseIndex, fMinIndexValue, fMaxIndexValue,
                                       fBaseVertex);
        } else {
            opsRenderPass->drawIndexPattern(fIndexCount, fPatternRepeatCount,
                                            fMaxPatternRepetitionsInIndexBuffer, fVertexCount,
                                            fBaseVertex);
        }
    }
}

#endif
