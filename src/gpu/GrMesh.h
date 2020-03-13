/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMesh_DEFINED
#define GrMesh_DEFINED

#include "src/gpu/GrBuffer.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrOpsRenderPass.h"

class GrPrimitiveProcessor;

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrOp to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrMesh {
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

inline void GrMesh::set(sk_sp<const GrBuffer> vertexBuffer, int vertexCount, int baseVertex) {
    SkASSERT(baseVertex >= 0);
    fIndexBuffer.reset();
    fVertexBuffer = std::move(vertexBuffer);
    fVertexCount = vertexCount;
    fBaseVertex = baseVertex;
    SkDEBUGCODE(fIsInitialized = true;)
}

inline void GrMesh::setIndexed(sk_sp<const GrBuffer> indexBuffer, int indexCount, int baseIndex,
                               uint16_t minIndexValue, uint16_t maxIndexValue,
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

inline void GrMesh::setIndexedPatterned(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                                        int patternRepeatCount,
                                        int maxPatternRepetitionsInIndexBuffer,
                                        sk_sp<const GrBuffer> vertexBuffer,
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

inline void GrMesh::draw(GrOpsRenderPass* opsRenderPass) const {
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
