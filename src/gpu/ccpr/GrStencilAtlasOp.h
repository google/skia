/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilAtlasOp_DEFINED
#define GrStencilAtlasOp_DEFINED

#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/ccpr/GrCCFiller.h"
#include "src/gpu/ccpr/GrCCStroker.h"
#include "src/gpu/ops/GrDrawOp.h"

class GrCCPerFlushResources;

// Renders literal A8 coverage to a CCPR atlas using an intermediate MSAA stencil buffer.
class GrStencilAtlasOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    using FillBatchID = GrCCFiller::BatchID;
    using StrokeBatchID = GrCCStroker::BatchID;

    // Once all the paths in an atlas have been drawn to the stencil buffer, we make a final pass
    // where we draw "resolve" rects over each path whose purpose is to convert winding counts to A8
    // coverage.
    struct ResolveRectInstance {
        int16_t l, t, r, b;
    };

    // GrDrawOp interface.
    const char* name() const override { return "StencilAtlasOp (CCPR)"; }
    FixedFunctionFlags fixedFunctionFlags() const override {
        return FixedFunctionFlags::kUsesHWAA | FixedFunctionFlags::kUsesStencil;
    }

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    CombineResult onCombineIfPossible(GrOp* other, const GrCaps&) override {
        // We will only make multiple copy ops if they have different source proxies.
        // TODO: make use of texture chaining.
        return CombineResult::kCannotCombine;
    }
    void onPrepare(GrOpFlushState*) override {}

    static std::unique_ptr<GrDrawOp> Make(
            GrRecordingContext*, sk_sp<const GrCCPerFlushResources>, FillBatchID, StrokeBatchID,
            int baseStencilResolveInstance, int endStencilResolveInstance,
            const SkISize& drawBounds);

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override;

private:
    friend class ::GrOpMemoryPool; // for ctor

    GrStencilAtlasOp(sk_sp<const GrCCPerFlushResources> resources, FillBatchID fillBatchID,
                     StrokeBatchID strokeBatchID, int baseStencilResolveInstance,
                     int endStencilResolveInstance, const SkISize& drawBounds)
            : GrDrawOp(ClassID())
            , fResources(std::move(resources))
            , fFillBatchID(fillBatchID)
            , fStrokeBatchID(strokeBatchID)
            , fBaseStencilResolveInstance(baseStencilResolveInstance)
            , fEndStencilResolveInstance(endStencilResolveInstance)
            , fDrawBounds(drawBounds) {
        this->setBounds(SkRect::MakeIWH(fDrawBounds.width(), fDrawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsHairline::kNo);
    }

    const sk_sp<const GrCCPerFlushResources> fResources;
    const FillBatchID fFillBatchID;
    const StrokeBatchID fStrokeBatchID;
    const int fBaseStencilResolveInstance;
    const int fEndStencilResolveInstance;
    const SkISize fDrawBounds;
    int fResolveBaseVertex;
};


#endif
