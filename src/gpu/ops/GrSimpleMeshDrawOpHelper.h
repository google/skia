/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleMeshDrawOpHelper_DEFINED
#define GrSimpleMeshDrawOpHelper_DEFINED

#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrPipeline.h"

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
    static std::unique_ptr<GrDrawOp> FactoryHelper(GrPaint&& paint, OpArgs... opArgs);

    enum class Flags : uint32_t {
        kNone = 0x0,
        kSnapVerticesToPixelCenters = 0x1,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    GrSimpleMeshDrawOpHelper(const MakeArgs&, GrAAType, Flags = Flags::kNone);
    ~GrSimpleMeshDrawOpHelper();

    GrSimpleMeshDrawOpHelper() = delete;
    GrSimpleMeshDrawOpHelper(const GrSimpleMeshDrawOpHelper&) = delete;
    GrSimpleMeshDrawOpHelper& operator=(const GrSimpleMeshDrawOpHelper&) = delete;

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const;

    bool isCompatible(const GrSimpleMeshDrawOpHelper& that, const GrCaps&, const SkRect& thisBounds,
                      const SkRect& thatBounds) const;

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
    GrDrawOp::RequiresDstTexture xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip,
                                                      GrProcessorAnalysisCoverage geometryCoverage,
                                                      GrProcessorAnalysisColor* geometryColor);

    /**
     * Version of above that can be used by ops that have a constant color geometry processor
     * output. The op passes this color as 'geometryColor' and after return if 'geometryColor' has
     * changed the op must override its geometry processor color output with the new color.
     */
    GrDrawOp::RequiresDstTexture xpRequiresDstTexture(const GrCaps&, const GrAppliedClip*,
                                                      GrProcessorAnalysisCoverage geometryCoverage,
                                                      GrColor* geometryColor);

    bool usesLocalCoords() const {
        SkASSERT(fDidAnalysis);
        return fUsesLocalCoords;
    }

    bool compatibleWithAlphaAsCoverage() const { return fCompatibleWithAlphaAsCoveage; }

    GrPipeline* makePipeline(GrMeshDrawOp::Target* target) const {
        return target->allocPipeline(this->pipelineInitArgs(target));
    }

    struct MakeArgs {
    private:
        MakeArgs() = default;

        GrProcessorSet* fProcessorSet;
        uint32_t fSRGBFlags;

        friend class GrSimpleMeshDrawOpHelper;
    };

    SkString dumpInfo() const;

protected:
    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
    uint32_t pipelineFlags() const { return fPipelineFlags; }
    const GrProcessorSet& processors() const {
        return fProcessors ? *fProcessors : GrProcessorSet::EmptySet();
    }

    GrPipeline::InitArgs pipelineInitArgs(GrMeshDrawOp::Target* target) const;

private:
    GrProcessorSet* fProcessors;
    unsigned fPipelineFlags : 8;
    unsigned fAAType : 2;
    unsigned fRequiresDstTexture : 1;
    unsigned fUsesLocalCoords : 1;
    unsigned fCompatibleWithAlphaAsCoveage : 1;
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
    using Flags = GrSimpleMeshDrawOpHelper::Flags;

    // using declarations can't be templated, so this is a pass through function instead.
    template <typename Op, typename... OpArgs>
    static std::unique_ptr<GrDrawOp> FactoryHelper(GrPaint&& paint, OpArgs... opArgs) {
        return GrSimpleMeshDrawOpHelper::FactoryHelper<Op, OpArgs...>(
                std::move(paint), std::forward<OpArgs>(opArgs)...);
    }

    GrSimpleMeshDrawOpHelperWithStencil(const MakeArgs&, GrAAType, const GrUserStencilSettings*,
                                        Flags = Flags::kNone);

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const;

    using GrSimpleMeshDrawOpHelper::xpRequiresDstTexture;
    using GrSimpleMeshDrawOpHelper::usesLocalCoords;
    using GrSimpleMeshDrawOpHelper::compatibleWithAlphaAsCoverage;

    bool isCompatible(const GrSimpleMeshDrawOpHelperWithStencil& that, const GrCaps&,
                      const SkRect& thisBounds, const SkRect& thatBounds) const;

    const GrPipeline* makePipeline(GrMeshDrawOp::Target*) const;

    SkString dumpInfo() const;

private:
    const GrUserStencilSettings* fStencilSettings;
    typedef GrSimpleMeshDrawOpHelper INHERITED;
};

template <typename Op, typename... OpArgs>
std::unique_ptr<GrDrawOp> GrSimpleMeshDrawOpHelper::FactoryHelper(GrPaint&& paint,
                                                                  OpArgs... opArgs) {
    MakeArgs makeArgs;
    makeArgs.fSRGBFlags = GrPipeline::SRGBFlagsFromPaint(paint);
    GrColor color = paint.getColor();
    if (paint.isTrivial()) {
        makeArgs.fProcessorSet = nullptr;
        return std::unique_ptr<GrDrawOp>(new Op(makeArgs, color, std::forward<OpArgs>(opArgs)...));
    } else {
        char* mem = (char*)GrOp::operator new(sizeof(Op) + sizeof(GrProcessorSet));
        char* setMem = mem + sizeof(Op);
        makeArgs.fProcessorSet = new (setMem) GrProcessorSet(std::move(paint));
        return std::unique_ptr<GrDrawOp>(
                new (mem) Op(makeArgs, color, std::forward<OpArgs>(opArgs)...));
    }
}

GR_MAKE_BITFIELD_CLASS_OPS(GrSimpleMeshDrawOpHelper::Flags)

#endif
