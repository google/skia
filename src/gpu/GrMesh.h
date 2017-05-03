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

    int               fSoftRepeatCount = 1;
    int               fMaxRepetitionsInIndexBuffer = 1;

    struct SoftRepetition;
    class SoftRepetitionsIterator;

    SkDEBUGCODE(void validate() const;)
};

struct GrMesh::SoftRepetition {
    int   fBaseVertex;
    int   fRepeatCount;
};

class GrMesh::SoftRepetitionsIterator {
public:
    SoftRepetitionsIterator(const GrMesh& mesh, int repetitionIdx)
        : fMesh(mesh)
        , fRepetitionIdx(repetitionIdx) {
        SkDEBUGCODE(mesh.validate());
    }

    bool operator!=(const SoftRepetitionsIterator& that) {
        SkASSERT(&fMesh == &that.fMesh);
        return fRepetitionIdx != that.fRepetitionIdx;
    }

    const SoftRepetition operator*() {
        SoftRepetition current;
        current.fBaseVertex = fMesh.fBaseVertex + fRepetitionIdx * fMesh.fVertexCount;
        current.fRepeatCount = SkTMin(fMesh.fSoftRepeatCount - fRepetitionIdx,
                                      fMesh.fMaxRepetitionsInIndexBuffer);
        return current;
    }

    void operator++() {
        fRepetitionIdx = SkTMin(fRepetitionIdx + fMesh.fMaxRepetitionsInIndexBuffer,
                                fMesh.fSoftRepeatCount);
    }

private:
    const GrMesh&    fMesh;
    int              fRepetitionIdx;
};

inline GrMesh::SoftRepetitionsIterator begin(const GrMesh& mesh) {
    return GrMesh::SoftRepetitionsIterator(mesh, 0);
}

inline GrMesh::SoftRepetitionsIterator end(const GrMesh& mesh) {
    return GrMesh::SoftRepetitionsIterator(mesh, mesh.fSoftRepeatCount);
}

#ifdef SK_DEBUG
inline void GrMesh::validate() const {
    SkASSERT(!fIndexBuffer || fIndexCount > 0);
    SkASSERT(fBaseIndex >= 0);

    SkASSERT(fVertexBuffer);
    SkASSERT(fVertexCount);
    SkASSERT(fBaseVertex >= 0);

    SkASSERT(fSoftRepeatCount > 0);
    SkASSERT(fMaxRepetitionsInIndexBuffer > 0);
}
#endif

#endif
