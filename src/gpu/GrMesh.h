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

    void setNonIndexed();
    void setIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex = 0);
    void setIndexedPatterned(const GrBuffer* indexBuffer, int indexCount,
                             int patternRepeatCount, int maxPatternRepetitionsInIndexBuffer);

    void setVertices(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex = 0);

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }

    bool isIndexed() const { return SkToBool(fIndexBuffer.get()); }
    const GrBuffer* indexBuffer() const { return fIndexBuffer.get(); }
    int indexCount() const { SkASSERT(this->isIndexed()); return fIndexCount; }
    int baseIndex() const { SkASSERT(this->isIndexed()); return fBaseIndex; }

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
    int               fPatternRepeatCount;
    int               fMaxPatternRepetitionsInIndexBuffer;

    PendingBuffer     fVertexBuffer;
    int               fVertexCount;
    int               fBaseVertex;

    class PatternIterator;
    friend GrMesh::PatternIterator begin(const GrMesh&);
    friend GrMesh::PatternIterator end(const GrMesh&);
};

inline void GrMesh::setNonIndexed() {
    fIndexBuffer.reset(nullptr);
}

inline void GrMesh::setIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex) {
    SkASSERT(indexBuffer);
    SkASSERT(indexCount >= 1);
    SkASSERT(baseIndex >= 0);
    fIndexBuffer.reset(indexBuffer);
    fIndexCount = indexCount;
    fBaseIndex = baseIndex;
    fPatternRepeatCount = fMaxPatternRepetitionsInIndexBuffer = 1;
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
}

inline void GrMesh::setVertices(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) {
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
        SkASSERT(fMesh.isIndexed());
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
