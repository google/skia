/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshDrawOp_DEFINED
#define GrMeshDrawOp_DEFINED

#include "GrDrawOp.h"
#include "GrGeometryProcessor.h"
#include "GrMesh.h"
#include "GrPendingProgramElement.h"

#include "SkTLList.h"

class GrCaps;
class GrOpFlushState;

/**
 * Base class for mesh-drawing GrDrawOps.
 */
class GrMeshDrawOp : public GrDrawOp {
public:
    /** Abstract interface that represents a destination for a GrMeshDrawOp. */
    class Target;
    /** Provides GrOpFlushState with privileged access to GrMeshDrawOp. */
    class FlushStateAccess;

protected:
    GrMeshDrawOp(uint32_t classID);

    /** Helper for rendering repeating meshes using a patterned index buffer. This class creates the
        space for the vertices and flushes the draws to the GrMeshDrawOp::Target. */
    class PatternHelper {
    public:
        PatternHelper(GrPrimitiveType primitiveType) : fMesh(primitiveType) {}
        /** Returns the allocated storage for the vertices. The caller should populate the vertices
            before calling recordDraws(). */
        void* init(Target*, size_t vertexStride, const GrBuffer*, int verticesPerRepetition,
                   int indicesPerRepetition, int repeatCount);

        /** Call after init() to issue draws to the GrMeshDrawOp::Target.*/
        void recordDraw(Target*, const GrGeometryProcessor*, const GrPipeline*);

    private:
        GrMesh fMesh;
    };

    static const int kVerticesPerQuad = 4;
    static const int kIndicesPerQuad = 6;

    /** A specialization of InstanceHelper for quad rendering. */
    class QuadHelper : private PatternHelper {
    public:
        QuadHelper() : INHERITED(GrPrimitiveType::kTriangles) {}
        /** Finds the cached quad index buffer and reserves vertex space. Returns nullptr on failure
            and on success a pointer to the vertex data that the caller should populate before
            calling recordDraws(). */
        void* init(Target*, size_t vertexStride, int quadsToDraw);

        using PatternHelper::recordDraw;

    private:
        typedef PatternHelper INHERITED;
    };

private:
    void onPrepare(GrOpFlushState* state) final;
    void onExecute(GrOpFlushState* state) final;

    virtual void onPrepareDraws(Target*) = 0;

    // A set of contiguous draws that share a draw token and primitive processor. The draws all use
    // the op's pipeline. The meshes for the draw are stored in the fMeshes array and each
    // Queued draw uses fMeshCnt meshes from the fMeshes array. The reason for coallescing meshes
    // that share a primitive processor into a QueuedDraw is that it allows the Gpu object to setup
    // the shared state once and then issue draws for each mesh.
    struct QueuedDraw {
        int fMeshCnt = 0;
        GrPendingProgramElement<const GrGeometryProcessor> fGeometryProcessor;
        const GrPipeline* fPipeline;
    };

    // All draws in all the GrMeshDrawOps have implicit tokens based on the order they are enqueued
    // globally across all ops. This is the offset of the first entry in fQueuedDraws.
    // fQueuedDraws[i]'s token is fBaseDrawToken + i.
    GrDeferredUploadToken fBaseDrawToken;
    SkSTArray<4, GrMesh> fMeshes;
    SkSTArray<4, QueuedDraw, true> fQueuedDraws;

    typedef GrDrawOp INHERITED;
};

class GrMeshDrawOp::Target {
public:
    virtual ~Target() {}

    /** Adds a draw of a mesh. */
    virtual void draw(const GrGeometryProcessor*, const GrPipeline*, const GrMesh&) = 0;

    /**
     * Makes space for vertex data. The returned pointer is the location where vertex data
     * should be written. On return the buffer that will hold the data as well as an offset into
     * the buffer (in 'vertexSize' units) where the data will be placed.
     */
    virtual void* makeVertexSpace(size_t vertexSize, int vertexCount, const GrBuffer**,
                                  int* startVertex) = 0;

    /**
     * Makes space for index data. The returned pointer is the location where index data
     * should be written. On return the buffer that will hold the data as well as an offset into
     * the buffer (in uint16_t units) where the data will be placed.
     */
    virtual uint16_t* makeIndexSpace(int indexCount, const GrBuffer**, int* startIndex) = 0;

    /**
     * This is similar to makeVertexSpace. It allows the caller to use up to 'actualVertexCount'
     * vertices in the returned pointer, which may exceed 'minVertexCount'.
     * 'fallbackVertexCount' is the maximum number of vertices that should be allocated if a new
     * buffer is allocated on behalf of this request.
     */
    virtual void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount,
                                         int fallbackVertexCount, const GrBuffer**,
                                         int* startVertex, int* actualVertexCount) = 0;

    /**
     * This is similar to makeIndexSpace. It allows the caller to use up to 'actualIndexCount'
     * indices in the returned pointer, which may exceed 'minIndexCount'.
     * 'fallbackIndexCount' is the maximum number of indices that should be allocated if a new
     * buffer is allocated on behalf of this request.
     */
    virtual uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                            const GrBuffer**, int* startIndex,
                                            int* actualIndexCount) = 0;

    /** Helpers for ops which over-allocate and then return excess data to the pool. */
    virtual void putBackIndices(int indices) = 0;
    virtual void putBackVertices(int vertices, size_t vertexStride) = 0;

    /**
     * Allocate space for a pipeline. The target ensures this pipeline lifetime is at least
     * as long as any deferred execution of draws added via draw().
     * @tparam Args
     * @param args
     * @return
     */
    template <typename... Args>
    GrPipeline* allocPipeline(Args&&... args) {
        return this->pipelineArena()->make<GrPipeline>(std::forward<Args>(args)...);
    }

    /**
     * Helper that makes a pipeline targeting the op's render target that incorporates the op's
     * GrAppliedClip.
     * */
    GrPipeline* makePipeline(uint32_t pipelineFlags, GrProcessorSet&& processorSet,
                             GrAppliedClip&& clip) {
        GrPipeline::InitArgs pipelineArgs;
        pipelineArgs.fFlags = pipelineFlags;
        pipelineArgs.fProxy = this->proxy();
        pipelineArgs.fDstProxy = this->dstProxy();
        pipelineArgs.fCaps = &this->caps();
        pipelineArgs.fResourceProvider = this->resourceProvider();
        return this->allocPipeline(pipelineArgs, std::move(processorSet), std::move(clip));
    }

    virtual GrRenderTargetProxy* proxy() const = 0;

    virtual GrAppliedClip detachAppliedClip() = 0;

    virtual const GrXferProcessor::DstProxy& dstProxy() const = 0;

    virtual GrResourceProvider* resourceProvider() const = 0;

    virtual const GrCaps& caps() const = 0;

    virtual GrDeferredUploadTarget* deferredUploadTarget() = 0;

private:
    virtual SkArenaAlloc* pipelineArena() = 0;
};

#endif
