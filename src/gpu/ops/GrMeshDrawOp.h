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
#include "GrPipelineBuilder.h"

#include "SkTLList.h"

class GrCaps;
class GrOpFlushState;

/**
 * Base class for mesh-drawing GrDrawOps.
 */
class GrMeshDrawOp : public GrDrawOp {
public:
    class Target;

protected:
    GrMeshDrawOp(uint32_t classID);

    /** Helper for rendering instances using an instanced index buffer. This class creates the space
        for the vertices and flushes the draws to the GrMeshDrawOp::Target. */
    class InstancedHelper {
    public:
        InstancedHelper() {}
        /** Returns the allocated storage for the vertices. The caller should populate the vertices
            before calling recordDraws(). */
        void* init(Target*, GrPrimitiveType, size_t vertexStride, const GrBuffer*,
                   int verticesPerInstance, int indicesPerInstance, int instancesToDraw);

        /** Call after init() to issue draws to the GrMeshDrawOp::Target.*/
        void recordDraw(Target*, const GrGeometryProcessor*, const GrPipeline*);

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
        const GrPipeline* fPipeline;
    };

    // All draws in all the GrMeshDrawOps have implicit tokens based on the order they are enqueued
    // globally across all ops. This is the offset of the first entry in fQueuedDraws.
    // fQueuedDraws[i]'s token is fBaseDrawToken + i.
    GrDrawOpUploadToken fBaseDrawToken;
    SkSTArray<4, GrMesh> fMeshes;
    SkSTArray<4, QueuedDraw, true> fQueuedDraws;

    typedef GrDrawOp INHERITED;
};

/**
 * Many of our ops derive from this class which initializes a GrPipeline just before being recorded.
 * We are migrating away from use of this class.
 */
class GrLegacyMeshDrawOp : public GrMeshDrawOp {
public:
    /**
     * Performs analysis of the fragment processors in GrProcessorSet and GrAppliedClip using the
     * initial color and coverage from this op's geometry processor.
     */
    GrProcessorSet::Analysis analyzeUpdateAndRecordProcessors(GrPipelineBuilder* pipelineBuilder,
                                                              const GrAppliedClip* appliedClip,
                                                              bool isMixedSamples,
                                                              const GrCaps& caps,
                                                              GrColor* overrideColor) const {
        GrProcessorAnalysisColor inputColor;
        GrProcessorAnalysisCoverage inputCoverage;
        this->getProcessorAnalysisInputs(&inputColor, &inputCoverage);
        return pipelineBuilder->finalizeProcessors(inputColor, inputCoverage, appliedClip,
                                                   isMixedSamples, caps, overrideColor);
    }

    void initPipeline(const GrPipeline::InitArgs& args, const GrProcessorSet::Analysis& analysis,
                      GrColor overrideColor) {
        fPipeline.init(args);
        this->applyPipelineOptimizations(PipelineOptimizations(analysis, overrideColor));
    }

    /**
     * Mesh draw ops use a legacy system in GrRenderTargetContext where the pipeline is created when
     * the op is recorded. These methods are unnecessary as this information is in the pipeline.
     */
    FixedFunctionFlags fixedFunctionFlags() const override {
        SkFAIL("This should never be called for legacy mesh draw ops.");
        return FixedFunctionFlags::kNone;
    }
    bool xpRequiresDstTexture(const GrCaps&, const GrAppliedClip*) override {
        SkFAIL("Should never be called for legacy mesh draw ops.");
        return false;
    }

protected:
    GrLegacyMeshDrawOp(uint32_t classID) : INHERITED(classID) {}
    /**
     * This is a legacy class only used by GrLegacyMeshDrawOp and will be removed. It presents some
     * aspects of GrProcessorSet::Analysis to GrLegacyMeshDrawOp subclasses.
     */
    class PipelineOptimizations {
    public:
        PipelineOptimizations(const GrProcessorSet::Analysis& analysis, GrColor overrideColor) {
            fFlags = 0;
            if (analysis.inputColorIsOverridden()) {
                fFlags |= kUseOverrideColor_Flag;
                fOverrideColor = overrideColor;
            }
            if (analysis.usesLocalCoords()) {
                fFlags |= kReadsLocalCoords_Flag;
            }
            if (analysis.isCompatibleWithCoverageAsAlpha()) {
                fFlags |= kCanTweakAlphaForCoverage_Flag;
            }
        }

        /** Does the pipeline require access to (implicit or explicit) local coordinates? */
        bool readsLocalCoords() const { return SkToBool(kReadsLocalCoords_Flag & fFlags); }

        /** Does the pipeline allow the GrPrimitiveProcessor to combine color and coverage into one
            color output ? */
        bool canTweakAlphaForCoverage() const {
            return SkToBool(kCanTweakAlphaForCoverage_Flag & fFlags);
        }

        /** Does the pipeline require the GrPrimitiveProcessor to specify a specific color (and if
            so get the color)? */
        bool getOverrideColorIfSet(GrColor* overrideColor) const {
            if (SkToBool(kUseOverrideColor_Flag & fFlags)) {
                if (overrideColor) {
                    *overrideColor = fOverrideColor;
                }
                return true;
            }
            return false;
        }

    private:
        enum {
            // If this is not set the primitive processor need not produce local coordinates
            kReadsLocalCoords_Flag = 0x1,
            // If this flag is set then the primitive processor may produce color*coverage as
            // its color output (and not output a separate coverage).
            kCanTweakAlphaForCoverage_Flag = 0x2,
            // If this flag is set the GrPrimitiveProcessor must produce fOverrideColor as its
            // output color. If not set fOverrideColor is to be ignored.
            kUseOverrideColor_Flag = 0x4,
        };

        uint32_t fFlags;
        GrColor fOverrideColor;
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
    virtual void getProcessorAnalysisInputs(GrProcessorAnalysisColor*,
                                            GrProcessorAnalysisCoverage*) const = 0;

    /**
     * After processor analysis is complete this is called so that the op can use the analysis
     * results when constructing its GrPrimitiveProcessor.
     */
    virtual void applyPipelineOptimizations(const PipelineOptimizations&) = 0;

    GrPipeline fPipeline;

    typedef GrMeshDrawOp INHERITED;
};

#endif
