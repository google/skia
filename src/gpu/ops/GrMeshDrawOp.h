/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshDrawOp_DEFINED
#define GrMeshDrawOp_DEFINED

#include "GrAppliedClip.h"
#include "GrDrawOp.h"
#include "GrGeometryProcessor.h"
#include "GrMesh.h"
#include "GrPendingProgramElement.h"

class GrAtlasManager;
class GrCaps;
class GrGlyphCache;
class GrOpFlushState;

/**
 * Base class for mesh-drawing GrDrawOps.
 */
class GrMeshDrawOp : public GrDrawOp {
public:
    /** Abstract interface that represents a destination for a GrMeshDrawOp. */
    class Target;

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
        void recordDraw(Target*, const GrGeometryProcessor*, const GrPipeline*,
                        const GrPipeline::FixedDynamicState*);

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
    typedef GrDrawOp INHERITED;
};

class GrMeshDrawOp::Target {
public:
    virtual ~Target() {}

    /** Adds a draw of a mesh. */
    virtual void draw(const GrGeometryProcessor*, const GrPipeline*,
                      const GrPipeline::FixedDynamicState*, const GrMesh&) = 0;

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

    template <typename... Args>
    GrPipeline::FixedDynamicState* allocFixedDynamicState(Args&... args) {
        return this->pipelineArena()->make<GrPipeline::FixedDynamicState>(
                std::forward<Args>(args)...);
    }

    // Once we have C++17 structured bindings make this just be a tuple because then we can do:
    //      auto [pipeline, fixedDynamicState] = target->makePipeline(...);
    // in addition to:
    //      std::tie(flushInfo.fPipeline, flushInfo.fFixedState) = target->makePipeline(...);
    struct PipelineAndFixedDynamicState {
        const GrPipeline* fPipeline;
        const GrPipeline::FixedDynamicState* fFixedDynamicState;
    };

    /**
     * Helper that makes a pipeline targeting the op's render target that incorporates the op's
     * GrAppliedClip and uses a fixed dynamic state.
     */
    PipelineAndFixedDynamicState makePipeline(uint32_t pipelineFlags, GrProcessorSet&& processorSet,
                                              GrAppliedClip&& clip) {
        GrPipeline::InitArgs pipelineArgs;
        pipelineArgs.fFlags = pipelineFlags;
        pipelineArgs.fProxy = this->proxy();
        pipelineArgs.fDstProxy = this->dstProxy();
        pipelineArgs.fCaps = &this->caps();
        pipelineArgs.fResourceProvider = this->resourceProvider();
        const auto* state = this->allocFixedDynamicState(clip.scissorState().rect());
        return {this->allocPipeline(pipelineArgs, std::move(processorSet), std::move(clip)), state};
    }

    virtual GrRenderTargetProxy* proxy() const = 0;

    virtual GrAppliedClip detachAppliedClip() = 0;

    virtual const GrXferProcessor::DstProxy& dstProxy() const = 0;

    virtual GrResourceProvider* resourceProvider() const = 0;
    uint32_t contextUniqueID() const { return this->resourceProvider()->contextUniqueID(); }

    virtual GrGlyphCache* glyphCache() const = 0;
    virtual GrAtlasManager* atlasManager() const = 0;

    virtual const GrCaps& caps() const = 0;

    virtual GrDeferredUploadTarget* deferredUploadTarget() = 0;

private:
    virtual SkArenaAlloc* pipelineArena() = 0;
};

#endif
