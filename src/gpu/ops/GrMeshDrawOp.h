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
    class Target;

    /**
     * Performs analysis of the fragment processors in GrProcessorSet and GrAppliedClip using the
     * initial color and coverage from this op's geometry processor.
     */
    void analyzeProcessors(GrProcessorSet::FragmentProcessorAnalysis* analysis,
                           const GrProcessorSet& processors,
                           const GrAppliedClip* appliedClip,
                           const GrCaps& caps) const {
        GrPipelineAnalysisColor inputColor;
        GrPipelineAnalysisCoverage inputCoverage;
        this->getFragmentProcessorAnalysisInputs(&inputColor, &inputCoverage);
        analysis->init(inputColor, inputCoverage, processors, appliedClip, caps);
    }

    void initPipeline(const GrPipeline::InitArgs& args) {
        this->applyPipelineOptimizations(fPipeline.init(args));
    }

    /**
     * Mesh draw ops use a legacy system in GrRenderTargetContext where the pipeline is created when
     * the op is recorded. These methods are unnecessary as this information is in the pipeline.
     */
    FixedFunctionFlags fixedFunctionFlags() const override {
        SkFAIL("This should never be called for mesh draw ops.");
        return FixedFunctionFlags::kNone;
    }
    bool xpRequiresDstTexture(const GrCaps&, const GrAppliedClip*) override {
        SkFAIL("Should never be called for mesh draw ops.");
        return false;
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
        SkASSERT(fPipeline.isInitialized());
        return &fPipeline;
    }

private:
    /**
     * Provides information about the GrPrimitiveProccesor color and coverage outputs which become
     * inputs to the first color and coverage fragment processors.
     */
    virtual void getFragmentProcessorAnalysisInputs(GrPipelineAnalysisColor*,
                                                    GrPipelineAnalysisCoverage*) const = 0;

    /**
     * After GrPipeline analysis is complete this is called so that the op can use the analysis
     * results when constructing its GrPrimitiveProcessor.
     */
    virtual void applyPipelineOptimizations(const GrPipelineOptimizations&) = 0;

    void onPrepare(GrOpFlushState* state) final;
    void onExecute(GrOpFlushState* state) final;

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
    GrPipeline fPipeline;
    SkSTArray<4, GrMesh> fMeshes;
    SkSTArray<4, QueuedDraw, true> fQueuedDraws;

    typedef GrDrawOp INHERITED;
};

#endif
