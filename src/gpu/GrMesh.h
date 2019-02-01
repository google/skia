/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMesh_DEFINED
#define GrMesh_DEFINED

#include "GrBuffer.h"
#include "GrPendingIOResource.h"

class GrPrimitiveProcessor;

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrOp to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrMesh {
public:
    GrMesh(GrPrimitiveType primitiveType = GrPrimitiveType::kTriangles)
            : fPrimitiveType(primitiveType), fBaseVertex(0) {
        SkDEBUGCODE(fNonIndexNonInstanceData.fVertexCount = -1;)
    }

    void setPrimitiveType(GrPrimitiveType type) { fPrimitiveType = type; }
    GrPrimitiveType primitiveType() const { return fPrimitiveType; }

    bool isIndexed() const { return SkToBool(fIndexBuffer.get()); }
    bool isInstanced() const { return SkToBool(fInstanceBuffer.get()); }
    bool hasVertexData() const { return SkToBool(fVertexBuffer.get()); }

    void setNonIndexedNonInstanced(int vertexCount);

    void setIndexed(sk_sp<const GrBuffer> indexBuffer, int indexCount, int baseIndex,
                    uint16_t minIndexValue, uint16_t maxIndexValue, GrPrimitiveRestart);
    void setIndexedPatterned(sk_sp<const GrBuffer> indexBuffer, int indexCount, int vertexCount,
                             int patternRepeatCount, int maxPatternRepetitionsInIndexBuffer);

    void setInstanced(sk_sp<const GrBuffer> instanceBuffer, int instanceCount, int baseInstance,
                      int vertexCount);
    void setIndexedInstanced(sk_sp<const GrBuffer>, int indexCount, sk_sp<const GrBuffer>,
                             int instanceCount, int baseInstance, GrPrimitiveRestart);

    void setVertexData(sk_sp<const GrBuffer> vertexBuffer, int baseVertex = 0);

    class SendToGpuImpl {
    public:
        virtual void sendMeshToGpu(GrPrimitiveType, const GrBuffer* vertexBuffer, int vertexCount,
                                   int baseVertex) = 0;

        virtual void sendIndexedMeshToGpu(GrPrimitiveType, const GrBuffer* indexBuffer,
                                          int indexCount, int baseIndex, uint16_t minIndexValue,
                                          uint16_t maxIndexValue, const GrBuffer* vertexBuffer,
                                          int baseVertex, GrPrimitiveRestart) = 0;

        virtual void sendInstancedMeshToGpu(GrPrimitiveType, const GrBuffer* vertexBuffer,
                                            int vertexCount, int baseVertex,
                                            const GrBuffer* instanceBuffer, int instanceCount,
                                            int baseInstance) = 0;

        virtual void sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrBuffer* indexBuffer,
                                                   int indexCount, int baseIndex,
                                                   const GrBuffer* vertexBuffer, int baseVertex,
                                                   const GrBuffer* instanceBuffer,
                                                   int instanceCount, int baseInstance,
                                                   GrPrimitiveRestart) = 0;

        virtual ~SendToGpuImpl() {}
    };

    void sendToGpu(SendToGpuImpl*) const;

private:
    GrPrimitiveType fPrimitiveType;
    sk_sp<const GrBuffer> fIndexBuffer;
    sk_sp<const GrBuffer> fInstanceBuffer;
    sk_sp<const GrBuffer> fVertexBuffer;
    int fBaseVertex;
    GrPrimitiveRestart fPrimitiveRestart;

    union {
        struct { // When fIndexBuffer == nullptr and fInstanceBuffer == nullptr.
            int   fVertexCount;
        } fNonIndexNonInstanceData;

        struct { // When fIndexBuffer != nullptr and fInstanceBuffer == nullptr.
            struct {
                int   fIndexCount;
                int   fPatternRepeatCount;
            } fIndexData;

            union {
                struct { // When fPatternRepeatCount == 0.
                    int        fBaseIndex;
                    uint16_t   fMinIndexValue;
                    uint16_t   fMaxIndexValue;
                } fNonPatternIndexData;

                struct { // When fPatternRepeatCount != 0.
                    int   fVertexCount;
                    int   fMaxPatternRepetitionsInIndexBuffer;
                } fPatternData;
            };
        };

        struct { // When fInstanceBuffer != nullptr.
            struct {
                int   fInstanceCount;
                int   fBaseInstance;
            } fInstanceData;

            union { // When fIndexBuffer == nullptr.
                struct {
                    int   fVertexCount;
                } fInstanceNonIndexData;

                struct { // When fIndexBuffer != nullptr.
                    int   fIndexCount;
                } fInstanceIndexData;
            };
        };
    };
};

inline void GrMesh::setNonIndexedNonInstanced(int vertexCount) {
    fIndexBuffer.reset(nullptr);
    fInstanceBuffer.reset(nullptr);
    fNonIndexNonInstanceData.fVertexCount = vertexCount;
    fPrimitiveRestart = GrPrimitiveRestart::kNo;
}

inline void GrMesh::setIndexed(sk_sp<const GrBuffer> indexBuffer, int indexCount, int baseIndex,
                               uint16_t minIndexValue, uint16_t maxIndexValue,
                               GrPrimitiveRestart primitiveRestart) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(baseIndex >= 0);
    SkASSERT(maxIndexValue >= minIndexValue);
    fIndexBuffer = std::move(indexBuffer);
    fInstanceBuffer.reset();
    fIndexData.fIndexCount = indexCount;
    fIndexData.fPatternRepeatCount = 0;
    fNonPatternIndexData.fBaseIndex = baseIndex;
    fNonPatternIndexData.fMinIndexValue = minIndexValue;
    fNonPatternIndexData.fMaxIndexValue = maxIndexValue;
    fPrimitiveRestart = primitiveRestart;
}

inline void GrMesh::setIndexedPatterned(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                                        int vertexCount, int patternRepeatCount,
                                        int maxPatternRepetitionsInIndexBuffer) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(vertexCount >= 1);
    SkASSERT(patternRepeatCount >= 1);
    SkASSERT(maxPatternRepetitionsInIndexBuffer >= 1);
    fIndexBuffer = std::move(indexBuffer);
    fInstanceBuffer.reset();
    fIndexData.fIndexCount = indexCount;
    fIndexData.fPatternRepeatCount = patternRepeatCount;
    fPatternData.fVertexCount = vertexCount;
    fPatternData.fMaxPatternRepetitionsInIndexBuffer = maxPatternRepetitionsInIndexBuffer;
    fPrimitiveRestart = GrPrimitiveRestart::kNo;
}

inline void GrMesh::setInstanced(sk_sp<const GrBuffer> instanceBuffer, int instanceCount,
                                 int baseInstance, int vertexCount) {
    SkASSERT(instanceBuffer);
    SkASSERT(instanceCount >= 1);
    SkASSERT(baseInstance >= 0);
    fIndexBuffer.reset();
    fInstanceBuffer = std::move(instanceBuffer);
    fInstanceData.fInstanceCount = instanceCount;
    fInstanceData.fBaseInstance = baseInstance;
    fInstanceNonIndexData.fVertexCount = vertexCount;
    fPrimitiveRestart = GrPrimitiveRestart::kNo;
}

inline void GrMesh::setIndexedInstanced(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                                        sk_sp<const GrBuffer> instanceBuffer, int instanceCount,
                                        int baseInstance, GrPrimitiveRestart primitiveRestart) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(instanceBuffer);
    SkASSERT(instanceCount >= 1);
    SkASSERT(baseInstance >= 0);
    fIndexBuffer = std::move(indexBuffer);
    fInstanceBuffer = std::move(instanceBuffer);
    fInstanceData.fInstanceCount = instanceCount;
    fInstanceData.fBaseInstance = baseInstance;
    fInstanceIndexData.fIndexCount = indexCount;
    fPrimitiveRestart = primitiveRestart;
}

inline void GrMesh::setVertexData(sk_sp<const GrBuffer> vertexBuffer, int baseVertex) {
    SkASSERT(baseVertex >= 0);
    fVertexBuffer = std::move(vertexBuffer);
    fBaseVertex = baseVertex;
}

inline void GrMesh::sendToGpu(SendToGpuImpl* impl) const {
    if (this->isInstanced()) {
        if (!this->isIndexed()) {
            impl->sendInstancedMeshToGpu(fPrimitiveType, fVertexBuffer.get(),
                                         fInstanceNonIndexData.fVertexCount, fBaseVertex,
                                         fInstanceBuffer.get(), fInstanceData.fInstanceCount,
                                         fInstanceData.fBaseInstance);
        } else {
            impl->sendIndexedInstancedMeshToGpu(
                    fPrimitiveType, fIndexBuffer.get(), fInstanceIndexData.fIndexCount, 0,
                    fVertexBuffer.get(), fBaseVertex, fInstanceBuffer.get(),
                    fInstanceData.fInstanceCount, fInstanceData.fBaseInstance, fPrimitiveRestart);
        }
        return;
    }

    if (!this->isIndexed()) {
        SkASSERT(fNonIndexNonInstanceData.fVertexCount > 0);
        impl->sendMeshToGpu(fPrimitiveType, fVertexBuffer.get(),
                            fNonIndexNonInstanceData.fVertexCount, fBaseVertex);
        return;
    }

    if (0 == fIndexData.fPatternRepeatCount) {
        impl->sendIndexedMeshToGpu(fPrimitiveType, fIndexBuffer.get(), fIndexData.fIndexCount,
                                   fNonPatternIndexData.fBaseIndex,
                                   fNonPatternIndexData.fMinIndexValue,
                                   fNonPatternIndexData.fMaxIndexValue, fVertexBuffer.get(),
                                   fBaseVertex, fPrimitiveRestart);
        return;
    }

    SkASSERT(fIndexData.fPatternRepeatCount > 0);
    int baseRepetition = 0;
    do {
        int repeatCount = SkTMin(fPatternData.fMaxPatternRepetitionsInIndexBuffer,
                                 fIndexData.fPatternRepeatCount - baseRepetition);
        // A patterned index buffer must contain indices in the range [0..vertexCount].
        int minIndexValue = 0;
        int maxIndexValue = fPatternData.fVertexCount * repeatCount - 1;
        SkASSERT(fPrimitiveRestart == GrPrimitiveRestart::kNo);
        impl->sendIndexedMeshToGpu(
                fPrimitiveType, fIndexBuffer.get(), fIndexData.fIndexCount * repeatCount, 0,
                minIndexValue, maxIndexValue, fVertexBuffer.get(),
                fBaseVertex + fPatternData.fVertexCount * baseRepetition, GrPrimitiveRestart::kNo);
        baseRepetition += repeatCount;
    } while (baseRepetition < fIndexData.fPatternRepeatCount);
}

#endif
