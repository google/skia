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
struct GrMesh {
    using PendingBuffer = GrPendingIOResource<const GrBuffer, kRead_GrIOType>;

    GrPrimitiveType   fPrimitiveType;

    PendingBuffer     fIndexBuffer;
    int               fIndexCount = 0;
    int               fBaseIndex = 0;

    PendingBuffer     fVertexBuffer;
    int               fVertexCount = 0;
    int               fBaseVertex = 0;

    int               fPatternRepeatCount = 1;
    int               fMaxPatternRepetitionsInIndexBuffer = 1;

    struct PatternBatch;
    class PatternIterator;

    SkDEBUGCODE(void validate() const;)
};

struct GrMesh::PatternBatch {
    int   fBaseVertex;
    int   fRepeatCount;
};

class GrMesh::PatternIterator {
public:
    PatternIterator(const GrMesh& mesh, int repetitionIdx)
        : fMesh(mesh)
        , fRepetitionIdx(repetitionIdx) {
        SkDEBUGCODE(mesh.validate());
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

#ifdef SK_DEBUG
inline void GrMesh::validate() const {
    SkASSERT(!fIndexBuffer || fIndexCount > 0);
    SkASSERT(fBaseIndex >= 0);

    SkASSERT(fVertexBuffer);
    SkASSERT(fVertexCount);
    SkASSERT(fBaseVertex >= 0);

    SkASSERT(fPatternRepeatCount > 0);
    SkASSERT(fMaxPatternRepetitionsInIndexBuffer > 0);
}
#endif

#endif
