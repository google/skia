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

/**
 * Used to communicate index and vertex buffers, counts, and offsets for a draw from GrOp to
 * GrGpu. It also holds the primitive type for the draw. TODO: Consider moving ownership of this
 * and draw-issuing responsibility to GrPrimitiveProcessor. The rest of the vertex info lives there
 * already (stride, attribute mappings).
 */
class GrMesh {
public:
    GrMesh(GrPrimitiveType primitiveType)
        : fPrimitiveType(primitiveType) {
        SkDEBUGCODE(fVertexCount = 0;)
        SkDEBUGCODE(fBaseVertex = -1;)
    }

    void setNonIndexedNonInstanced();

    void setIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex=0);
    void setIndexedPatterned(const GrBuffer* indexBuffer, int indexCount,
                             int patternRepeatCount, int maxPatternRepetitionsInIndexBuffer);

    void setInstanced(const GrBuffer* instanceBuffer, int instanceCount, int baseInstance=0);
    void setIndexedInstanced(const GrBuffer* indexBuffer, int indexCount,
                             const GrBuffer* instanceBuffer, int instanceCount, int baseInstance=0);

    void setVertices(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex=0);

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }

    bool isIndexed() const { return SkToBool(fIndexBuffer.get()); }
    const GrBuffer* indexBuffer() const { return fIndexBuffer.get(); }
    int indexCount() const { SkASSERT(this->isIndexed()); return fIndexCount; }
    int baseIndex() const { SkASSERT(this->isIndexed()); return fBaseIndex; }

    bool isInstanced() const { return SkToBool(fInstanceBuffer.get()); }
    const GrBuffer* instanceBuffer() const { return fInstanceBuffer.get(); }
    int instanceCount() const { SkASSERT(this->isInstanced()); return fInstanceCount; }
    int baseInstance() const { SkASSERT(this->isInstanced()); return fBaseInstance; }

    const GrBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
    int vertexCount() const { SkASSERT(fVertexCount >= 1); return fVertexCount; }
    int baseVertex() const { SkASSERT(fBaseVertex >= 0); return fBaseVertex; }

    struct PatternBatch;

private:
    using PendingBuffer = GrPendingIOResource<const GrBuffer, kRead_GrIOType>;

    GrPrimitiveType   fPrimitiveType;

    PendingBuffer     fIndexBuffer;
    int               fIndexCount;
    int               fBaseIndex;

    PendingBuffer     fInstanceBuffer;
    union {
        struct { // If fInstanceBuffer is non-null.
            int       fInstanceCount;
            int       fBaseInstance;
        };
        struct { // If fInstanceBuffer is null (and fIndexBuffer is non-null).
            int       fPatternRepeatCount;
            int       fMaxPatternRepetitionsInIndexBuffer;
        };
    };

    PendingBuffer     fVertexBuffer;
    int               fVertexCount;
    int               fBaseVertex;

    class PatternIterator;
    friend GrMesh::PatternIterator begin(const GrMesh&);
    friend GrMesh::PatternIterator end(const GrMesh&);
};

inline void GrMesh::setNonIndexedNonInstanced() {
    fIndexBuffer.reset(nullptr);
    fInstanceBuffer.reset(nullptr);
}

inline void GrMesh::setIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(baseIndex >= 0);
    fIndexBuffer.reset(indexBuffer);
    fIndexCount = indexCount;
    fBaseIndex = baseIndex;
    fPatternRepeatCount = fMaxPatternRepetitionsInIndexBuffer = 1;
    fInstanceBuffer.reset(nullptr);
}

inline void GrMesh::setIndexedPatterned(const GrBuffer* indexBuffer, int indexCount,
                                        int patternRepeatCount,
                                        int maxPatternRepetitionsInIndexBuffer) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(patternRepeatCount >= 1);
    SkASSERT(maxPatternRepetitionsInIndexBuffer >= 1);
    fIndexBuffer.reset(indexBuffer);
    fIndexCount = indexCount;
    fBaseIndex = 0;
    fPatternRepeatCount = patternRepeatCount;
    fMaxPatternRepetitionsInIndexBuffer = maxPatternRepetitionsInIndexBuffer;
    fInstanceBuffer.reset(nullptr);
}

inline void GrMesh::setInstanced(const GrBuffer* instanceBuffer, int instanceCount,
                                 int baseInstance) {
    SkASSERT(instanceBuffer);
    SkASSERT(instanceCount >= 1);
    SkASSERT(baseInstance >= 0);
    fIndexBuffer.reset(nullptr);
    fInstanceBuffer.reset(instanceBuffer);
    fInstanceCount = instanceCount;
    fBaseInstance = baseInstance;
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
    fIndexCount = indexCount;
    fBaseIndex = 0;
    fInstanceBuffer.reset(instanceBuffer);
    fInstanceCount = instanceCount;
    fBaseInstance = baseInstance;
}

inline void GrMesh::setVertices(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) {
    SkASSERT(vertexBuffer || 0 == baseVertex);
    SkASSERT(vertexCount >= 1);
    SkASSERT(baseVertex >= 0);
    fVertexBuffer.reset(vertexBuffer);
    fVertexCount = vertexCount;
    fBaseVertex = baseVertex;
}

struct GrMesh::PatternBatch {
    int   fBaseVertex;
    int   fRepeatCount;
};

class GrMesh::PatternIterator {
public:
    PatternIterator(const GrMesh& mesh, int repetitionIdx)
        : fMesh(mesh)
        , fRepetitionIdx(repetitionIdx) {
        SkASSERT(!fMesh.isInstanced());
    }

    bool operator!=(const PatternIterator& that) {
        SkASSERT(&fMesh == &that.fMesh);
        return fRepetitionIdx != that.fRepetitionIdx;
    }

    const PatternBatch operator*() {
        PatternBatch batch;
        batch.fBaseVertex = fMesh.fBaseVertex + fRepetitionIdx * fMesh.fVertexCount;
        batch.fRepeatCount = SkTMin(fMesh.fPatternRepeatCount - fRepetitionIdx,
                                    fMesh.fMaxPatternRepetitionsInIndexBuffer);
        return batch;
    }

    void operator++() {
        fRepetitionIdx = SkTMin(fRepetitionIdx + fMesh.fMaxPatternRepetitionsInIndexBuffer,
                                fMesh.fPatternRepeatCount);
    }

private:
    const GrMesh&    fMesh;
    int              fRepetitionIdx;
};

inline GrMesh::PatternIterator begin(const GrMesh& mesh) {
    return GrMesh::PatternIterator(mesh, 0);
}

inline GrMesh::PatternIterator end(const GrMesh& mesh) {
    return GrMesh::PatternIterator(mesh, mesh.fPatternRepeatCount);
}

#endif
