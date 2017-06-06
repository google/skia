/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMesh_DEFINED
#define GrMesh_DEFINED

#include "GrBuffer.h"
#include "GrGpuResourceRef.h"

class GrPrimitiveProcessor;

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrOp to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrMesh {
public:
    GrMesh(GrPrimitiveType primitiveType)
        : fPrimitiveType(primitiveType)
        , fBaseVertex(0) {
        SkDEBUGCODE(fNonIndexNonInstanceData.fVertexCount = -1;)
    }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    bool isIndexed() const { return SkToBool(fIndexBuffer.get()); }
    bool isInstanced() const { return SkToBool(fInstanceBuffer.get()); }
    bool hasVertexData() const { return SkToBool(fVertexBuffer.get()); }

    void setNonIndexedNonInstanced(int vertexCount);

    void setIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                    uint16_t minIndexValue, uint16_t maxIndexValue);
    void setIndexedPatterned(const GrBuffer* indexBuffer, int indexCount, int vertexCount,
                             int patternRepeatCount, int maxPatternRepetitionsInIndexBuffer);

    void setInstanced(const GrBuffer* instanceBuffer, int instanceCount, int baseInstance,
                      int vertexCount);
    void setIndexedInstanced(const GrBuffer* indexBuffer, int indexCount,
                             const GrBuffer* instanceBuffer, int instanceCount, int baseInstance=0);

    void setVertexData(const GrBuffer* vertexBuffer, int baseVertex = 0);

    class SendToGpuImpl {
    public:
        virtual void sendMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                   const GrBuffer* vertexBuffer, int vertexCount,
                                   int baseVertex) = 0;

        virtual void sendIndexedMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                          const GrBuffer* indexBuffer, int indexCount,
                                          int baseIndex, uint16_t minIndexValue,
                                          uint16_t maxIndexValue, const GrBuffer* vertexBuffer,
                                          int baseVertex) = 0;

        virtual void sendInstancedMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                            const GrBuffer* vertexBuffer, int vertexCount,
                                            int baseVertex, const GrBuffer* instanceBuffer,
                                            int instanceCount, int baseInstance) = 0;

        virtual void sendIndexedInstancedMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                                   const GrBuffer* indexBuffer, int indexCount,
                                                   int baseIndex, const GrBuffer* vertexBuffer,
                                                   int baseVertex, const GrBuffer* instanceBuffer,
                                                   int instanceCount, int baseInstance) = 0;

        virtual ~SendToGpuImpl() {}
    };

    void sendToGpu(const GrPrimitiveProcessor&, SendToGpuImpl*) const;

    struct PatternBatch;

private:
    using PendingBuffer = GrPendingIOResource<const GrBuffer, kRead_GrIOType>;

    GrPrimitiveType   fPrimitiveType;
    PendingBuffer     fIndexBuffer;
    PendingBuffer     fInstanceBuffer;
    PendingBuffer     fVertexBuffer;
    int               fBaseVertex;

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
}

inline void GrMesh::setIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                               uint16_t minIndexValue, uint16_t maxIndexValue) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(baseIndex >= 0);
    SkASSERT(maxIndexValue >= minIndexValue);
    fIndexBuffer.reset(indexBuffer);
    fInstanceBuffer.reset(nullptr);
    fIndexData.fIndexCount = indexCount;
    fIndexData.fPatternRepeatCount = 0;
    fNonPatternIndexData.fBaseIndex = baseIndex;
    fNonPatternIndexData.fMinIndexValue = minIndexValue;
    fNonPatternIndexData.fMaxIndexValue = maxIndexValue;
}

inline void GrMesh::setIndexedPatterned(const GrBuffer* indexBuffer, int indexCount,
                                        int vertexCount, int patternRepeatCount,
                                        int maxPatternRepetitionsInIndexBuffer) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(vertexCount >= 1);
    SkASSERT(patternRepeatCount >= 1);
    SkASSERT(maxPatternRepetitionsInIndexBuffer >= 1);
    fIndexBuffer.reset(indexBuffer);
    fInstanceBuffer.reset(nullptr);
    fIndexData.fIndexCount = indexCount;
    fIndexData.fPatternRepeatCount = patternRepeatCount;
    fPatternData.fVertexCount = vertexCount;
    fPatternData.fMaxPatternRepetitionsInIndexBuffer = maxPatternRepetitionsInIndexBuffer;
}

inline void GrMesh::setInstanced(const GrBuffer* instanceBuffer, int instanceCount,
                                 int baseInstance, int vertexCount) {
    SkASSERT(instanceBuffer);
    SkASSERT(instanceCount >= 1);
    SkASSERT(baseInstance >= 0);
    fIndexBuffer.reset(nullptr);
    fInstanceBuffer.reset(instanceBuffer);
    fInstanceData.fInstanceCount = instanceCount;
    fInstanceData.fBaseInstance = baseInstance;
    fInstanceNonIndexData.fVertexCount = vertexCount;
}

inline void GrMesh::setIndexedInstanced(const GrBuffer* indexBuffer, int indexCount,
                                        const GrBuffer* instanceBuffer, int instanceCount,
                                        int baseInstance) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(instanceBuffer);
    SkASSERT(instanceCount >= 1);
    SkASSERT(baseInstance >= 0);
    fIndexBuffer.reset(indexBuffer);
    fInstanceBuffer.reset(instanceBuffer);
    fInstanceData.fInstanceCount = instanceCount;
    fInstanceData.fBaseInstance = baseInstance;
    fInstanceIndexData.fIndexCount = indexCount;
}

inline void GrMesh::setVertexData(const GrBuffer* vertexBuffer, int baseVertex) {
    SkASSERT(baseVertex >= 0);
    fVertexBuffer.reset(vertexBuffer);
    fBaseVertex = baseVertex;
}

inline void GrMesh::sendToGpu(const GrPrimitiveProcessor& primProc, SendToGpuImpl* impl) const {
    if (this->isInstanced()) {
        if (!this->isIndexed()) {
            impl->sendInstancedMeshToGpu(primProc, fPrimitiveType, fVertexBuffer.get(),
                                         fInstanceNonIndexData.fVertexCount, fBaseVertex,
                                         fInstanceBuffer.get(), fInstanceData.fInstanceCount,
                                         fInstanceData.fBaseInstance);
        } else {
            impl->sendIndexedInstancedMeshToGpu(primProc, fPrimitiveType, fIndexBuffer.get(),
                                                fInstanceIndexData.fIndexCount, 0,
                                                fVertexBuffer.get(), fBaseVertex,
                                                fInstanceBuffer.get(), fInstanceData.fInstanceCount,
                                                fInstanceData.fBaseInstance);
        }
        return;
    }

    if (!this->isIndexed()) {
        SkASSERT(fNonIndexNonInstanceData.fVertexCount > 0);
        impl->sendMeshToGpu(primProc, fPrimitiveType, fVertexBuffer.get(),
                            fNonIndexNonInstanceData.fVertexCount, fBaseVertex);
        return;
    }

    if (0 == fIndexData.fPatternRepeatCount) {
        impl->sendIndexedMeshToGpu(primProc, fPrimitiveType, fIndexBuffer.get(),
                                   fIndexData.fIndexCount, fNonPatternIndexData.fBaseIndex,
                                   fNonPatternIndexData.fMinIndexValue,
                                   fNonPatternIndexData.fMaxIndexValue, fVertexBuffer.get(),
                                   fBaseVertex);
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
        impl->sendIndexedMeshToGpu(primProc, fPrimitiveType, fIndexBuffer.get(),
                                   fIndexData.fIndexCount * repeatCount, 0, minIndexValue,
                                   maxIndexValue, fVertexBuffer.get(),
                                   fBaseVertex + fPatternData.fVertexCount * baseRepetition);
        baseRepetition += repeatCount;
    } while (baseRepetition < fIndexData.fPatternRepeatCount);
}

#endif
