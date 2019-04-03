/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleMeshDrawOpHelper_DEFINED
#define GrSimpleMeshDrawOpHelper_DEFINED

#include "GrMemoryPool.h" // only here bc of the templated FactoryHelper
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrPipeline.h"
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
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
    struct MakeArgs;

    /**
     * This can be used by a Op class to perform allocation and initialization such that a
     * GrProcessorSet (if required) is allocated as part of the the same allocation that as
     * the Op instance. It requires that Op implements a constructor of the form:
     *      Op(MakeArgs, GrColor, OpArgs...)
     * which is public or made accessible via 'friend'.
     */
    template <typename Op, typename... OpArgs>
    static std::unique_ptr<GrDrawOp> FactoryHelper(GrRecordingContext*, GrPaint&&, OpArgs...);

    // Here we allow callers to specify a subset of the GrPipeline::InputFlags upon creation.
    enum class InputFlags : uint8_t {
        kNone = 0,
        kSnapVerticesToPixelCenters = (uint8_t)GrPipeline::InputFlags::kSnapVerticesToPixelCenters,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(InputFlags);

    GrSimpleMeshDrawOpHelper(const MakeArgs&, GrAAType, InputFlags = InputFlags::kNone);
    ~GrSimpleMeshDrawOpHelper();

    GrSimpleMeshDrawOpHelper() = delete;
    GrSimpleMeshDrawOpHelper(const GrSimpleMeshDrawOpHelper&) = delete;
    GrSimpleMeshDrawOpHelper& operator=(const GrSimpleMeshDrawOpHelper&) = delete;

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const;

    // noneAACompatibleWithCoverage should be set to true if the op can properly render a non-AA
    // primitive merged into a coverage-based op.
    bool isCompatible(const GrSimpleMeshDrawOpHelper& that, const GrCaps&, const SkRect& thisBounds,
                      const SkRect& thatBounds, bool noneAACompatibleWithCoverage = false) const;

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
    GrProcessorSet::Analysis finalizeProcessors(
            const GrCaps& caps, const GrAppliedClip* clip, GrFSAAType fsaaType,
            GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
            GrProcessorAnalysisColor* geometryColor) {
        return this->finalizeProcessors(caps, clip, &GrUserStencilSettings::kUnused, fsaaType,
                                        clampType, geometryCoverage, geometryColor);
    }

    /**
     * Version of above that can be used by ops that have a constant color geometry processor
     * output. The op passes this color as 'geometryColor' and after return if 'geometryColor' has
     * changed the op must override its geometry processor color output with the new color.
     */
    GrProcessorSet::Analysis finalizeProcessors(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType,
            GrProcessorAnalysisCoverage geometryCoverage, SkPMColor4f* geometryColor,
            bool* wideColor);

    bool isTrivial() const {
      return fProcessors == nullptr;
    }

    bool usesLocalCoords() const {
        SkASSERT(fDidAnalysis);
        return fUsesLocalCoords;
    }

    bool compatibleWithCoverageAsAlpha() const { return fCompatibleWithCoverageAsAlpha; }

    struct MakeArgs {
    private:
        MakeArgs() = default;

        GrProcessorSet* fProcessorSet;

        friend class GrSimpleMeshDrawOpHelper;
    };

    void visitProxies(const std::function<void(GrSurfaceProxy*)>& func) const {
        if (fProcessors) {
            fProcessors->visitProxies(func);
        }
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const;
#endif
    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }

    void setAAType(GrAAType aaType) {
      fAAType = static_cast<unsigned>(aaType);
    }

    void executeDrawsAndUploads(const GrOp*, GrOpFlushState*, const SkRect& chainBounds);

protected:
    GrPipeline::InputFlags pipelineFlags() const { return fPipelineFlags; }

    GrProcessorSet::Analysis finalizeProcessors(
            const GrCaps& caps, const GrAppliedClip*, const GrUserStencilSettings*, GrFSAAType,
            GrClampType, GrProcessorAnalysisCoverage geometryCoverage,
            GrProcessorAnalysisColor* geometryColor);

    GrProcessorSet* fProcessors;
    GrPipeline::InputFlags fPipelineFlags;
    unsigned fAAType : 2;
    unsigned fUsesLocalCoords : 1;
    unsigned fCompatibleWithCoverageAsAlpha : 1;
    SkDEBUGCODE(unsigned fMadePipeline : 1;)
    SkDEBUGCODE(unsigned fDidAnalysis : 1;)
};

/**
 * This class extends GrSimpleMeshDrawOpHelper to support an optional GrUserStencilSettings. This
 * uses private inheritance because it non-virtually overrides methods in the base class and should
 * never be used with a GrSimpleMeshDrawOpHelper pointer or reference.
 */
class GrSimpleMeshDrawOpHelperWithStencil : private GrSimpleMeshDrawOpHelper {
public:
    using MakeArgs = GrSimpleMeshDrawOpHelper::MakeArgs;
    using InputFlags = GrSimpleMeshDrawOpHelper::InputFlags;

    using GrSimpleMeshDrawOpHelper::visitProxies;

    // using declarations can't be templated, so this is a pass through function instead.
    template <typename Op, typename... OpArgs>
    static std::unique_ptr<GrDrawOp> FactoryHelper(GrRecordingContext* context, GrPaint&& paint,
                                                   OpArgs... opArgs) {
        return GrSimpleMeshDrawOpHelper::FactoryHelper<Op, OpArgs...>(
                context, std::move(paint), std::forward<OpArgs>(opArgs)...);
    }

    GrSimpleMeshDrawOpHelperWithStencil(const MakeArgs&, GrAAType, const GrUserStencilSettings*,
                                        InputFlags = InputFlags::kNone);

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const;

    GrProcessorSet::Analysis finalizeProcessors(
            const GrCaps& caps, const GrAppliedClip* clip, GrFSAAType fsaaType,
            GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
            GrProcessorAnalysisColor* geometryColor) {
        return this->INHERITED::finalizeProcessors(
                caps, clip, fStencilSettings, fsaaType, clampType, geometryCoverage, geometryColor);
    }

    GrProcessorSet::Analysis finalizeProcessors(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType,
            GrProcessorAnalysisCoverage geometryCoverage, SkPMColor4f* geometryColor,
            bool* wideColor);

    using GrSimpleMeshDrawOpHelper::aaType;
    using GrSimpleMeshDrawOpHelper::setAAType;
    using GrSimpleMeshDrawOpHelper::isTrivial;
    using GrSimpleMeshDrawOpHelper::usesLocalCoords;
    using GrSimpleMeshDrawOpHelper::compatibleWithCoverageAsAlpha;

    bool isCompatible(const GrSimpleMeshDrawOpHelperWithStencil& that, const GrCaps&,
                      const SkRect& thisBounds, const SkRect& thatBounds,
                      bool noneAACompatibleWithCoverage = false) const;

    void executeDrawsAndUploads(const GrOp*, GrOpFlushState*, const SkRect& chainBounds);

#ifdef SK_DEBUG
    SkString dumpInfo() const;
#endif

private:
    const GrUserStencilSettings* fStencilSettings;
    typedef GrSimpleMeshDrawOpHelper INHERITED;
};

template <typename Op, typename... OpArgs>
std::unique_ptr<GrDrawOp> GrSimpleMeshDrawOpHelper::FactoryHelper(GrRecordingContext* context,
                                                                  GrPaint&& paint,
                                                                  OpArgs... opArgs) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    MakeArgs makeArgs;

    if (paint.isTrivial()) {
        makeArgs.fProcessorSet = nullptr;
        return pool->allocate<Op>(makeArgs, paint.getColor4f(), std::forward<OpArgs>(opArgs)...);
    } else {
        char* mem = (char*) pool->allocate(sizeof(Op) + sizeof(GrProcessorSet));
        char* setMem = mem + sizeof(Op);
        auto color = paint.getColor4f();
        makeArgs.fProcessorSet = new (setMem) GrProcessorSet(std::move(paint));
        return std::unique_ptr<GrDrawOp>(new (mem) Op(makeArgs, color,
                                                      std::forward<OpArgs>(opArgs)...));
    }
}

GR_MAKE_BITFIELD_CLASS_OPS(GrSimpleMeshDrawOpHelper::InputFlags)

#endif
