/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexBatch_DEFINED
#define GrVertexBatch_DEFINED

#include "GrDrawBatch.h"
#include "GrGeometryProcessor.h"
#include "GrMesh.h"
#include "GrPendingProgramElement.h"

#include "SkTLList.h"

class GrBatchFlushState;

/**
 * Base class for vertex-based GrBatches.
 */
class GrVertexBatch : public GrDrawBatch {
public:
    class Target;

    GrVertexBatch(uint32_t classID);

protected:
    /** Helper for rendering instances using an instanced index index buffer. This class creates the
        space for the vertices and flushes the draws to the batch target. */
   class InstancedHelper {
   public:
        InstancedHelper() {}
        /** Returns the allocated storage for the vertices. The caller should populate the vertices
            before calling recordDraws(). */
        void* init(Target*, GrPrimitiveType, size_t vertexStride,
                   const GrBuffer*, int verticesPerInstance, int indicesPerInstance,
                   int instancesToDraw);

        /** Call after init() to issue draws to the batch target.*/
        void recordDraw(Target*, const GrGeometryProcessor*);
    private:
        GrMesh fMesh;
    };

    static const int kVerticesPerQuad = 4;
    static const int kIndicesPerQuad = 6;

    /** A specialization of InstanceHelper for quad rendering. */
    class QuadHelper : private InstancedHelper {
    public:
        QuadHelper() : INHERITED() {}
        /** Finds the cached quad index buffer and reserves vertex space. Returns nullptr on failure
            and on success a pointer to the vertex data that the caller should populate before
            calling recordDraws(). */
        void* init(Target*, size_t vertexStride, int quadsToDraw);

        using InstancedHelper::recordDraw;
    private:
        typedef InstancedHelper INHERITED;
    };

private:
    void onPrepare(GrBatchFlushState* state) final;
    void onDraw(GrBatchFlushState* state) final;

    virtual void onPrepareDraws(Target*) const = 0;

    // A set of contiguous draws that share a draw token and primitive processor. The draws all use
    // the batch's pipeline. The meshes for the draw are stored in the fMeshes array and each
    // Queued draw uses fMeshCnt meshes from the fMeshes array. The reason for coallescing meshes
    // that share a primitive processor into a QueuedDraw is that it allows the Gpu object to setup
    // the shared state once and then issue draws for each mesh.
    struct QueuedDraw {
        int fMeshCnt = 0;
        GrPendingProgramElement<const GrGeometryProcessor> fGeometryProcessor;
    };

    // All draws in all the vertex batches have implicit tokens based on the order they are
    // enqueued globally across all batches. This is the offset of the first entry in fQueuedDraws.
    // fQueuedDraws[i]'s token is fBaseDrawToken + i.
    GrBatchDrawToken fBaseDrawToken;

    SkSTArray<4, GrMesh>           fMeshes;
    SkSTArray<4, QueuedDraw, true> fQueuedDraws;

    typedef GrDrawBatch INHERITED;
};

#endif
