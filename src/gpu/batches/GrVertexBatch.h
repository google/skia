/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexBatch_DEFINED
#define GrVertexBatch_DEFINED

#include "GrDrawBatch.h"
#include "GrBatchTarget.h"

/**
 * Base class for vertex-based GrBatches.
 */
class GrVertexBatch : public GrDrawBatch {
public:
    GrVertexBatch();

    virtual void generateGeometry(GrBatchTarget*) = 0;

    // TODO this goes away when batches are everywhere
    void setNumberOfDraws(int numberOfDraws) { fNumberOfDraws = numberOfDraws; }
    int numberOfDraws() const { return fNumberOfDraws; }

protected:
    /** Helper for rendering instances using an instanced index index buffer. This class creates the
        space for the vertices and flushes the draws to the batch target. */
   class InstancedHelper {
   public:
        InstancedHelper() {}
        /** Returns the allocated storage for the vertices. The caller should populate the before
            vertices before calling issueDraws(). */
        void* init(GrBatchTarget* batchTarget, GrPrimitiveType, size_t vertexStride,
                   const GrIndexBuffer*, int verticesPerInstance, int indicesPerInstance,
                   int instancesToDraw);

        /** Call after init() to issue draws to the batch target.*/
        void issueDraw(GrBatchTarget* batchTarget) {
            SkASSERT(fVertices.instanceCount());
            batchTarget->draw(fVertices);
        }
    private:
        GrVertices  fVertices;
    };

    static const int kVerticesPerQuad = 4;
    static const int kIndicesPerQuad = 6;

    /** A specialization of InstanceHelper for quad rendering. */
    class QuadHelper : private InstancedHelper {
    public:
        QuadHelper() : INHERITED() {}
        /** Finds the cached quad index buffer and reserves vertex space. Returns NULL on failure
            and on sucess a pointer to the vertex data that the caller should populate before
            calling issueDraws(). */
        void* init(GrBatchTarget* batchTarget, size_t vertexStride, int quadsToDraw);

        using InstancedHelper::issueDraw;

    private:
        typedef InstancedHelper INHERITED;
    };

private:
    int                                 fNumberOfDraws;
    typedef GrDrawBatch INHERITED;
};

#endif
