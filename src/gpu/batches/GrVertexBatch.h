/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVertexBatch_DEFINED
#define GrVertexBatch_DEFINED

#include "GrDrawBatch.h"
#include "GrPrimitiveProcessor.h"
#include "GrPendingProgramElement.h"
#include "GrVertices.h"

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
        /** Returns the allocated storage for the vertices. The caller should populate the before
            vertices before calling issueDraws(). */
        void* init(Target*, GrPrimitiveType, size_t vertexStride,
                   const GrIndexBuffer*, int verticesPerInstance, int indicesPerInstance,
                   int instancesToDraw);

        /** Call after init() to issue draws to the batch target.*/
        void recordDraw(Target* target);
    private:
        GrVertices  fVertices;
    };

    static const int kVerticesPerQuad = 4;
    static const int kIndicesPerQuad = 6;

    /** A specialization of InstanceHelper for quad rendering. */
    class QuadHelper : private InstancedHelper {
    public:
        QuadHelper() : INHERITED() {}
        /** Finds the cached quad index buffer and reserves vertex space. Returns nullptr on failure
            and on sucess a pointer to the vertex data that the caller should populate before
            calling issueDraws(). */
        void* init(Target* batchTarget, size_t vertexStride, int quadsToDraw);

        using InstancedHelper::recordDraw;
    private:
        typedef InstancedHelper INHERITED;
    };

private:
    void onPrepare(GrBatchFlushState* state) final;
    void onDraw(GrBatchFlushState* state) final;

    virtual void onPrepareDraws(Target*) const = 0;

    // A set of contiguous draws with no inline uploads between them that all use the same
    // primitive processor. All the draws in a DrawArray share a primitive processor and use the
    // the batch's GrPipeline.
    struct DrawArray {
        SkSTArray<1, GrVertices, true>                      fDraws;
        GrPendingProgramElement<const GrPrimitiveProcessor> fPrimitiveProcessor;
    };

    // Array of DrawArray. There may be inline uploads between each DrawArray and each DrawArray
    // may use a different primitive processor.
    typedef SkTLList<DrawArray, 4> DrawArrayList;
    DrawArrayList fDrawArrays;

    typedef GrDrawBatch INHERITED;
};

#endif
