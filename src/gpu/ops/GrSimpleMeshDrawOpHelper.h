/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleMeshDrawOpHelper_DEFINED
#define GrSimpleMeshDrawOpHelper_DEFINED

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/ops/GrMeshDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include <new>

struct SkRect;

/**
 * This class can be used to help implement simple mesh draw ops. It reduces the amount of
 * boilerplate code to type and also provides a mechanism for optionally allocating space for a
 * GrProcessorSet based on a GrPaint. It is intended to be used by ops that construct a single
 * GrPipeline for a uniform primitive color and a GrPaint.
 */
class GrSimpleMeshDrawOpHelper {
public:
    /**
     * This can be used by a Op class to perform allocation and initialization such that a
     * GrProcessorSet (if required) is allocated as part of the the same allocation that as
     * the Op instance. It requires that Op implements a constructor of the form:
     *      Op(ProcessorSet*, GrColor, OpArgs...).
     */
    template <typename Op, typename... OpArgs>
    static GrOp::Owner FactoryHelper(GrRecordingContext*, GrPaint&&, OpArgs&&...);

    // Here we allow callers to specify a subset of the GrPipeline::InputFlags upon creation.
    enum class InputFlags : uint8_t {
        kNone = 0,
        kSnapVerticesToPixelCenters = (uint8_t)GrPipeline::InputFlags::kSnapVerticesToPixelCenters,
        kConservativeRaster = (uint8_t)GrPipeline::InputFlags::kConservativeRaster,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(InputFlags);

    GrSimpleMeshDrawOpHelper(GrProcessorSet*, GrAAType, InputFlags = InputFlags::kNone);
    ~GrSimpleMeshDrawOpHelper();

    GrSimpleMeshDrawOpHelper() = delete;
    GrSimpleMeshDrawOpHelper(const GrSimpleMeshDrawOpHelper&) = delete;
    GrSimpleMeshDrawOpHelper& operator=(const GrSimpleMeshDrawOpHelper&) = delete;

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const;

    // ignoreAAType should be set to true if the op already knows the AA settings are acceptible
    bool isCompatible(const GrSimpleMeshDrawOpHelper& that, const GrCaps&, const SkRect& thisBounds,
                      const SkRect& thatBounds, bool ignoreAAType = false) const;

    /**
     * Finalizes the processor set and determines whether the destination must be provided
     * to the fragment shader as a texture for blending.
     *
     * @param geometryCoverage Describes the coverage output of the op's geometry processor
     * @param geometryColor An in/out param. As input this informs processor analysis about the
     *                      color the op expects to output from its geometry processor. As output
     *                      this may be set to a known color in which case the op must output this
     *                      color from its geometry processor instead.
     */
    GrProcessorSet::Analysis finalizeProcessors(const GrCaps& caps, const GrAppliedClip* clip,
                                                GrClampType clampType,
                                                GrProcessorAnalysisCoverage geometryCoverage,
                                                GrProcessorAnalysisColor* geometryColor) {
        return this->finalizeProcessors(caps, clip, &GrUserStencilSettings::kUnused, clampType,
                                        geometryCoverage, geometryColor);
    }

    /**
     * Version of above that can be used by ops that have a constant color geometry processor
     * output. The op passes this color as 'geometryColor' and after return if 'geometryColor' has
     * changed the op must override its geometry processor color output with the new color.
     */
    GrProcessorSet::Analysis finalizeProcessors(const GrCaps&, const GrAppliedClip*, GrClampType,
                                                GrProcessorAnalysisCoverage geometryCoverage,
                                                SkPMColor4f* geometryColor, bool* wideColor);

    bool isTrivial() const {
      return fProcessors == nullptr;
    }

    bool usesLocalCoords() const {
        SkASSERT(fDidAnalysis);
        return fUsesLocalCoords;
    }

    bool compatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }

    void visitProxies(const GrVisitProxyFunc& func) const {
        if (fProcessors) {
            fProcessors->visitProxies(func);
        }
    }

#if GR_TEST_UTILS
    SkString dumpInfo() const;
#endif
    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }

    void setAAType(GrAAType aaType) {
        fAAType = static_cast<unsigned>(aaType);
    }

    static const GrPipeline* CreatePipeline(
                                const GrCaps*,
                                SkArenaAlloc*,
                                GrSwizzle writeViewSwizzle,
                                GrAppliedClip&&,
                                const GrDstProxyView&,
                                GrProcessorSet&&,
                                GrPipeline::InputFlags pipelineFlags);
    static const GrPipeline* CreatePipeline(
                                GrOpFlushState*,
                                GrProcessorSet&&,
                                GrPipeline::InputFlags pipelineFlags);

    const GrPipeline* createPipeline(GrOpFlushState* flushState);

    const GrPipeline* createPipeline(const GrCaps*,
                                     SkArenaAlloc*,
                                     GrSwizzle writeViewSwizzle,
                                     GrAppliedClip&&,
                                     const GrDstProxyView&);

    static GrProgramInfo* CreateProgramInfo(SkArenaAlloc*,
                                            const GrPipeline*,
                                            const GrSurfaceProxyView& writeView,
                                            GrGeometryProcessor*,
                                            GrPrimitiveType,
                                            GrXferBarrierFlags renderPassXferBarriers,
                                            GrLoadOp colorLoadOp,
                                            const GrUserStencilSettings*
                                                                = &GrUserStencilSettings::kUnused);

    // Create a programInfo with the following properties:
    //     its primitive processor uses no textures
    //     it has no dynamic state besides the scissor clip
    static GrProgramInfo* CreateProgramInfo(const GrCaps*,
                                            SkArenaAlloc*,
                                            const GrSurfaceProxyView& writeView,
                                            GrAppliedClip&&,
                                            const GrDstProxyView&,
                                            GrGeometryProcessor*,
                                            GrProcessorSet&&,
                                            GrPrimitiveType,
                                            GrXferBarrierFlags renderPassXferBarriers,
                                            GrLoadOp colorLoadOp,
                                            GrPipeline::InputFlags pipelineFlags
                                                                = GrPipeline::InputFlags::kNone,
                                            const GrUserStencilSettings*
                                                                = &GrUserStencilSettings::kUnused);

    GrProgramInfo* createProgramInfo(const GrCaps*,
                                     SkArenaAlloc*,
                                     const GrSurfaceProxyView& writeView,
                                     GrAppliedClip&&,
                                     const GrDstProxyView&,
                                     GrGeometryProcessor*,
                                     GrPrimitiveType,
                                     GrXferBarrierFlags renderPassXferBarriers,
                                     GrLoadOp colorLoadOp);

    GrProcessorSet detachProcessorSet() {
        return fProcessors ? std::move(*fProcessors) : GrProcessorSet::MakeEmptySet();
    }

    GrPipeline::InputFlags pipelineFlags() const { return fPipelineFlags; }

protected:
    GrProcessorSet::Analysis finalizeProcessors(const GrCaps& caps, const GrAppliedClip*,
                                                const GrUserStencilSettings*, GrClampType,
                                                GrProcessorAnalysisCoverage geometryCoverage,
                                                GrProcessorAnalysisColor* geometryColor);

    GrProcessorSet* fProcessors;
    GrPipeline::InputFlags fPipelineFlags;
    unsigned fAAType : 2;
    unsigned fUsesLocalCoords : 1;
    unsigned fCompatibleWithCoverageAsAlpha : 1;
    SkDEBUGCODE(unsigned fMadePipeline : 1;)
    SkDEBUGCODE(unsigned fDidAnalysis : 1;)
};

template<typename Op, typename... Args>
GrOp::Owner GrOp::MakeWithProcessorSet(
        GrRecordingContext* context, const SkPMColor4f& color,
        GrPaint&& paint, Args&&... args) {
    char* bytes = (char*)::operator new(sizeof(Op) + sizeof(GrProcessorSet));
    char* setMem = bytes + sizeof(Op);
    GrProcessorSet* processorSet = new (setMem)  GrProcessorSet{std::move(paint)};
    return Owner{new (bytes) Op(processorSet, color, std::forward<Args>(args)...)};
}

template <typename Op, typename... OpArgs>
GrOp::Owner GrSimpleMeshDrawOpHelper::FactoryHelper(GrRecordingContext* context,
                                                    GrPaint&& paint,
                                                    OpArgs&& ... opArgs) {
    auto color = paint.getColor4f();
    if (paint.isTrivial()) {
        return GrOp::Make<Op>(context, nullptr, color, std::forward<OpArgs>(opArgs)...);
    } else {
        return GrOp::MakeWithProcessorSet<Op>(
                context, color, std::move(paint), std::forward<OpArgs>(opArgs)...);
    }
}

GR_MAKE_BITFIELD_CLASS_OPS(GrSimpleMeshDrawOpHelper::InputFlags)

#endif
