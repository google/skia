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

class GrPrimitiveProcessor;

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrOp to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrMesh {
public:
    GrMesh() {
        SkDEBUGCODE(fNonIndexNonInstanceData.fVertexCount = -1;)
    }

    bool isIndexed() const { return SkToBool(fIndexBuffer.get()); }
    const GrBuffer* indexBuffer() const {
        SkASSERT(this->isIndexed());
        return fIndexBuffer.get();
    }
    GrPrimitiveRestart primitiveRestart() const {
        return GrPrimitiveRestart(fFlags & Flags::kUsePrimitiveRestart);
    }

    bool isInstanced() const { return fFlags & Flags::kIsInstanced; }
    const GrBuffer* instanceBuffer() const {
        SkASSERT(this->isInstanced() || !fInstanceBuffer);
        return fInstanceBuffer.get();
    }

    const GrBuffer* vertexBuffer() const { return fVertexBuffer.get(); }

    void setNonIndexedNonInstanced(int vertexCount);

    void setIndexed(sk_sp<const GrBuffer> indexBuffer, int indexCount, int baseIndex,
                    uint16_t minIndexValue, uint16_t maxIndexValue, GrPrimitiveRestart);
    void setIndexedPatterned(sk_sp<const GrBuffer> indexBuffer, int indexCount, int vertexCount,
                             int patternRepeatCount, int maxPatternRepetitionsInIndexBuffer);

    void setInstanced(sk_sp<const GrBuffer> instanceBuffer, int instanceCount, int baseInstance,
                      int vertexCount);
    void setIndexedInstanced(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                             sk_sp<const GrBuffer> instanceBuffer, int instanceCount,
                             int baseInstance, GrPrimitiveRestart);

    void setVertexData(sk_sp<const GrBuffer> vertexBuffer, int baseVertex = 0);

    class SendToGpuImpl {
    public:
        virtual void sendArrayMeshToGpu(GrPrimitiveType, const GrMesh&, int vertexCount,
                                        int baseVertex) = 0;
        virtual void sendIndexedMeshToGpu(GrPrimitiveType, const GrMesh&, int indexCount,
                                          int baseIndex, uint16_t minIndexValue,
                                          uint16_t maxIndexValue, int baseVertex) = 0;
        virtual void sendInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int vertexCount,
                                            int baseVertex, int instanceCount,
                                            int baseInstance) = 0;
        virtual void sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int indexCount,
                                                   int baseIndex, int baseVertex, int instanceCount,
                                                   int baseInstance) = 0;
        virtual ~SendToGpuImpl() {}
    };

    void sendToGpu(GrPrimitiveType, SendToGpuImpl*) const;

private:
    enum class Flags : uint8_t {
        kNone = 0,
        kUsePrimitiveRestart = 1 << 0,
        kIsInstanced = 1 << 1,
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);
    static_assert(Flags(GrPrimitiveRestart::kNo) == Flags::kNone);
    static_assert(Flags(GrPrimitiveRestart::kYes) == Flags::kUsePrimitiveRestart);

    sk_sp<const GrBuffer> fIndexBuffer;
    sk_sp<const GrBuffer> fInstanceBuffer;
    sk_sp<const GrBuffer> fVertexBuffer;
    int fBaseVertex = 0;
    Flags fFlags = Flags::kNone;

    union {
        struct { // When fIndexBuffer == nullptr and isInstanced() == false.
            int   fVertexCount;
        } fNonIndexNonInstanceData;

        struct { // When fIndexBuffer != nullptr and isInstanced() == false.
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

        struct { // When isInstanced() != false.
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

GR_MAKE_BITFIELD_CLASS_OPS(GrMesh::Flags);

inline void GrMesh::setNonIndexedNonInstanced(int vertexCount) {
    fIndexBuffer.reset();
    fInstanceBuffer.reset();
    fNonIndexNonInstanceData.fVertexCount = vertexCount;
    fFlags = Flags::kNone;
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
    fFlags = Flags(primitiveRestart);
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
    fFlags = Flags::kNone;
}

inline void GrMesh::setInstanced(sk_sp<const GrBuffer> instanceBuffer, int instanceCount,
                                 int baseInstance, int vertexCount) {
    SkASSERT(instanceCount >= 1);
    SkASSERT(baseInstance >= 0);
    fIndexBuffer.reset();
    fInstanceBuffer = std::move(instanceBuffer);
    fInstanceData.fInstanceCount = instanceCount;
    fInstanceData.fBaseInstance = baseInstance;
    fInstanceNonIndexData.fVertexCount = vertexCount;
    fFlags = Flags::kIsInstanced;
}

inline void GrMesh::setIndexedInstanced(sk_sp<const GrBuffer> indexBuffer, int indexCount,
                                        sk_sp<const GrBuffer> instanceBuffer, int instanceCount,
                                        int baseInstance, GrPrimitiveRestart primitiveRestart) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(instanceCount >= 1);
    SkASSERT(baseInstance >= 0);
    fIndexBuffer = std::move(indexBuffer);
    fInstanceBuffer = std::move(instanceBuffer);
    fInstanceData.fInstanceCount = instanceCount;
    fInstanceData.fBaseInstance = baseInstance;
    fInstanceIndexData.fIndexCount = indexCount;
    fFlags = Flags::kIsInstanced | Flags(primitiveRestart);
}

inline void GrMesh::setVertexData(sk_sp<const GrBuffer> vertexBuffer, int baseVertex) {
    SkASSERT(baseVertex >= 0);
    fVertexBuffer = std::move(vertexBuffer);
    fBaseVertex = baseVertex;
}

inline void GrMesh::sendToGpu(GrPrimitiveType primitiveType, SendToGpuImpl* impl) const {
    if (this->isInstanced()) {
        if (!this->isIndexed()) {
            impl->sendInstancedMeshToGpu(primitiveType, *this, fInstanceNonIndexData.fVertexCount,
                                         fBaseVertex, fInstanceData.fInstanceCount,
                                         fInstanceData.fBaseInstance);
        } else {
            impl->sendIndexedInstancedMeshToGpu(
                    primitiveType, *this, fInstanceIndexData.fIndexCount, 0, fBaseVertex,
                    fInstanceData.fInstanceCount, fInstanceData.fBaseInstance);
        }
        return;
    }

    if (!this->isIndexed()) {
        SkASSERT(fNonIndexNonInstanceData.fVertexCount > 0);
        impl->sendArrayMeshToGpu(primitiveType, *this, fNonIndexNonInstanceData.fVertexCount,
                                 fBaseVertex);
        return;
    }

    if (0 == fIndexData.fPatternRepeatCount) {
        impl->sendIndexedMeshToGpu(primitiveType, *this, fIndexData.fIndexCount,
                                   fNonPatternIndexData.fBaseIndex,
                                   fNonPatternIndexData.fMinIndexValue,
                                   fNonPatternIndexData.fMaxIndexValue, fBaseVertex);
        return;
    }

    SkASSERT(fIndexData.fPatternRepeatCount > 0);
    int baseRepetition = 0;
    do {
        int repeatCount = std::min(fPatternData.fMaxPatternRepetitionsInIndexBuffer,
                                 fIndexData.fPatternRepeatCount - baseRepetition);
        int indexCount = fIndexData.fIndexCount * repeatCount;
        // A patterned index buffer must contain indices in the range [0..vertexCount].
        int minIndexValue = 0;
        int maxIndexValue = fPatternData.fVertexCount * repeatCount - 1;
        SkASSERT(!(fFlags & Flags::kUsePrimitiveRestart));
        impl->sendIndexedMeshToGpu(primitiveType, *this, indexCount, 0, minIndexValue,
                                   maxIndexValue,
                                   fBaseVertex + fPatternData.fVertexCount * baseRepetition);
        baseRepetition += repeatCount;
    } while (baseRepetition < fIndexData.fPatternRepeatCount);
}

#endif
