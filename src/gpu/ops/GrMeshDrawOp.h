/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMeshDrawOp_DEFINED
#define GrMeshDrawOp_DEFINED

#include "GrCaps.h"
#include "GrDrawOp.h"
#include "GrGeometryProcessor.h"
#include "GrMesh.h"
#include "GrPendingProgramElement.h"

#include "SkTLList.h"

class GrOpFlushState;

/**
 * Base class for mesh-drawing GrDrawOps.
 */
class GrMeshDrawOp : public GrDrawOp {
public:
    class Target;

    ~GrMeshDrawOp() override;

    /**
     * Get the inputs to pipeline analysis from GrMeshDrawOp.
     */
    void initPipelineAnalysis(GrPipelineAnalysis*) const;

    bool installPipeline(const GrPipeline::CreateArgs& args);

    /** Should these all assert that they are not called? */
    bool usesHWAAWhenAvailable() const override { return this->pipeline()->isHWAntialiasState(); }
    bool usesStencil() override { return !this->pipeline()->getUserStencil()->isUnused(); }
    bool willXPNeedDstTexture(const GrCaps& caps) const override {
        return (!caps.shaderCaps()->dstReadInShaderSupport() && this->pipeline()->getXferProcessor().willReadDstColor());
    }
    bool finalize(const GrAppliedClip&, const GrXferProcessor::DstTexture&) const override {
        return true;
    }

protected:
    GrMeshDrawOp(uint32_t classID);

    /** Helper for rendering instances using an instanced index index buffer. This class creates the
        space for the vertices and flushes the draws to the GrMeshDrawOp::Target. */
    class InstancedHelper {
    public:
        InstancedHelper() {}
        /** Returns the allocated storage for the vertices. The caller should populate the vertices
            before calling recordDraws(). */
        void* init(Target*, GrPrimitiveType, size_t vertexStride, const GrBuffer*,
                   int verticesPerInstance, int indicesPerInstance, int instancesToDraw);

        /** Call after init() to issue draws to the GrMeshDrawOp::Target.*/
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

    const GrPipeline* pipeline() const {
        SkASSERT(fPipelineInstalled);
        return reinterpret_cast<const GrPipeline*>(fPipelineStorage.get());
    }

private:
    /**
     * Provides information about the GrPrimitiveProccesor that will be used to issue draws by this
     * op to GrPipeline analysis.
     */
    virtual void getPipelineAnalysisInput(GrPipelineAnalysisDrawOpInput*) const = 0;

    /**
     * After GrPipeline analysis is complete this is called so that the op can use the analysis
     * results when constructing its GrPrimitiveProcessor.
     */
    virtual void applyPipelineOptimizations(const GrPipelineOptimizations&) = 0;

    void onPrepare(GrOpFlushState* state) final;
    void onExecute(GrOpFlushState* state, const SkRect& bounds, const GrAppliedClip*) final;

    virtual void onPrepareDraws(Target*) const = 0;

    // A set of contiguous draws that share a draw token and primitive processor. The draws all use
    // the op's pipeline. The meshes for the draw are stored in the fMeshes array and each
    // Queued draw uses fMeshCnt meshes from the fMeshes array. The reason for coallescing meshes
    // that share a primitive processor into a QueuedDraw is that it allows the Gpu object to setup
    // the shared state once and then issue draws for each mesh.
    struct QueuedDraw {
        int fMeshCnt = 0;
        GrPendingProgramElement<const GrGeometryProcessor> fGeometryProcessor;
    };

    // All draws in all the GrMeshDrawOps have implicit tokens based on the order they are enqueued
    // globally across all ops. This is the offset of the first entry in fQueuedDraws.
    // fQueuedDraws[i]'s token is fBaseDrawToken + i.
    GrDrawOpUploadToken fBaseDrawToken;
    SkAlignedSTStorage<1, GrPipeline> fPipelineStorage;
    bool fPipelineInstalled;
    SkSTArray<4, GrMesh> fMeshes;
    SkSTArray<4, QueuedDraw, true> fQueuedDraws;

    typedef GrDrawOp INHERITED;
};

#endif
