/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCPerFlushResources.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ccpr/GrSampleMaskProcessor.h"
#include "src/gpu/geometry/GrStyledShape.h"

#include <algorithm>

using FillBatchID = GrCCFiller::BatchID;

namespace {

// Base class for an Op that renders a CCPR atlas.
class AtlasOp : public GrDrawOp {
public:
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*,
                                      bool hasMixedSampledCoverage, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    CombineResult onCombineIfPossible(GrOp* other, SkArenaAlloc*, const GrCaps&) override {
        // We will only make multiple copy ops if they have different source proxies.
        // TODO: make use of texture chaining.
        return CombineResult::kCannotCombine;
    }

protected:
    AtlasOp(uint32_t classID, sk_sp<const GrCCPerFlushResources> resources,
            const SkISize& drawBounds)
            : GrDrawOp(classID)
            , fResources(std::move(resources)) {
        this->setBounds(SkRect::MakeIWH(drawBounds.width(), drawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsHairline::kNo);
    }

    const sk_sp<const GrCCPerFlushResources> fResources;

private:
    void onPrePrepare(GrRecordingContext*,
                      const GrSurfaceProxyView& writeView,
                      GrAppliedClip*,
                      const GrXferProcessor::DstProxyView&,
                      GrXferBarrierFlags renderPassXferBarriers,
                      GrLoadOp colorLoadOp) final {}
    void onPrepare(GrOpFlushState*) final {}
};

// Renders coverage counts to a CCPR atlas using the resources' pre-filled GrCCPathParser.
template<typename ProcessorType> class RenderAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(
            GrRecordingContext* context, sk_sp<const GrCCPerFlushResources> resources,
            FillBatchID fillBatchID, const SkISize& drawBounds) {
        return GrOp::Make<RenderAtlasOp>(context,
                std::move(resources), fillBatchID, drawBounds);
    }

    // GrDrawOp interface.
    const char* name() const override { return "RenderAtlasOp (CCPR)"; }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        ProcessorType proc;
        GrPipeline pipeline(GrScissorTest::kEnabled, SkBlendMode::kPlus,
                            flushState->drawOpArgs().writeView().swizzle());
        fResources->filler().drawFills(flushState, &proc, pipeline, fFillBatchID, fDrawBounds);
    }

private:
    friend class ::GrOp; // for ctor

    RenderAtlasOp(sk_sp<const GrCCPerFlushResources> resources, FillBatchID fillBatchID,
                  const SkISize& drawBounds)
            : AtlasOp(ClassID(), std::move(resources), drawBounds)
            , fFillBatchID(fillBatchID)
            , fDrawBounds(SkIRect::MakeWH(drawBounds.width(), drawBounds.height())) {
    }

    const FillBatchID fFillBatchID;
    const SkIRect fDrawBounds;
};

}  // namespace

GrCCPerFlushResources::GrCCPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                             const GrCCPerFlushResourceSpecs& specs)
        : fFiller(GrCCFiller::Algorithm::kStencilWindingCount,
                  specs.fNumClipPaths,
                  specs.fRenderedPathStats.fNumTotalSkPoints,
                  specs.fRenderedPathStats.fNumTotalSkVerbs,
                  specs.fRenderedPathStats.fNumTotalConicWeights)
        , fRenderedAtlasStack(specs.fRenderedAtlasSpecs, onFlushRP->caps()) {
    int numRenderedPaths = specs.fNumClipPaths;
    fStencilResolveBuffer.resetAndMapBuffer(
            onFlushRP, numRenderedPaths * sizeof(GrStencilAtlasOp::ResolveRectInstance));
    if (!fStencilResolveBuffer.hasGpuBuffer()) {
        SkDebugf("WARNING: failed to allocate CCPR stencil resolve buffer. "
                 "No paths will be drawn.\n");
        return;
    }
    SkDEBUGCODE(fEndStencilResolveInstance = numRenderedPaths);
}

const GrCCAtlas* GrCCPerFlushResources::renderDeviceSpacePathInAtlas(
        const SkIRect& clipIBounds, const SkPath& devPath, const SkIRect& devPathIBounds,
        GrFillRule fillRule, SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());

    if (devPath.isEmpty()) {
        SkDEBUGCODE(--fEndStencilResolveInstance);
        return nullptr;
    }

    GrScissorTest enableScissorInAtlas;
    SkIRect clippedPathIBounds;
    if (clipIBounds.contains(devPathIBounds)) {
        clippedPathIBounds = devPathIBounds;
        enableScissorInAtlas = GrScissorTest::kDisabled;
    } else if (clippedPathIBounds.intersect(clipIBounds, devPathIBounds)) {
        enableScissorInAtlas = GrScissorTest::kEnabled;
    } else {
        // The clip and path bounds do not intersect. Draw nothing.
        SkDEBUGCODE(--fEndStencilResolveInstance);
        return nullptr;
    }

    this->placeRenderedPathInAtlas(clippedPathIBounds, enableScissorInAtlas, devToAtlasOffset);
    fFiller.parseDeviceSpaceFill(devPath, SkPathPriv::PointData(devPath), enableScissorInAtlas,
                                 clippedPathIBounds, *devToAtlasOffset);

    // In MSAA mode we also record an internal draw instance that will be used to resolve stencil
    // winding values to coverage when the atlas is generated.
    this->recordStencilResolveInstance(clippedPathIBounds, *devToAtlasOffset, fillRule);

    return &fRenderedAtlasStack.current();
}

void GrCCPerFlushResources::placeRenderedPathInAtlas(
        const SkIRect& clippedPathIBounds, GrScissorTest scissorTest, SkIVector* devToAtlasOffset) {
    if (GrCCAtlas* retiredAtlas =
                fRenderedAtlasStack.addRect(clippedPathIBounds, devToAtlasOffset)) {
        // We did not fit in the previous coverage count atlas and it was retired. Close the path
        // parser's current batch (which does not yet include the path we just parsed). We will
        // render this batch into the retired atlas during finalize().
        retiredAtlas->setFillBatchID(fFiller.closeCurrentBatch());
        retiredAtlas->setEndStencilResolveInstance(fNextStencilResolveInstanceIdx);
    }
}

void GrCCPerFlushResources::recordStencilResolveInstance(
        const SkIRect& clippedPathIBounds, const SkIVector& devToAtlasOffset, GrFillRule fillRule) {
    SkASSERT(fNextStencilResolveInstanceIdx < fEndStencilResolveInstance);

    SkIRect atlasIBounds = clippedPathIBounds.makeOffset(devToAtlasOffset);
    if (GrFillRule::kEvenOdd == fillRule) {
        // Make even/odd fills counterclockwise. The resolve draw uses two-sided stencil, with
        // "nonzero" settings in front and "even/odd" settings in back.
        std::swap(atlasIBounds.fLeft, atlasIBounds.fRight);
    }
    fStencilResolveBuffer[fNextStencilResolveInstanceIdx++] = {
            (int16_t)atlasIBounds.left(), (int16_t)atlasIBounds.top(),
            (int16_t)atlasIBounds.right(), (int16_t)atlasIBounds.bottom()};
}

bool GrCCPerFlushResources::finalize(GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextStencilResolveInstanceIdx == fEndStencilResolveInstance);

    if (fStencilResolveBuffer.hasGpuBuffer()) {
        fStencilResolveBuffer.unmapBuffer();
    }

    if (!fRenderedAtlasStack.empty()) {
        fRenderedAtlasStack.current().setFillBatchID(fFiller.closeCurrentBatch());
        fRenderedAtlasStack.current().setEndStencilResolveInstance(fNextStencilResolveInstanceIdx);
    }

    // Build the GPU buffers to render path coverage counts. (This must not happen until after the
    // final calls to fFiller/fStroker.closeCurrentBatch().)
    if (!fFiller.prepareToDraw(onFlushRP)) {
        return false;
    }

    // Render the coverage count atlas(es).
    int baseStencilResolveInstance = 0;
    for (GrCCAtlas& atlas : fRenderedAtlasStack.atlases()) {
        if (auto rtc = atlas.instantiate(onFlushRP)) {
            GrOp::Owner op;
            op = GrStencilAtlasOp::Make(
                    rtc->recordingContext(), sk_ref_sp(this), atlas.getFillBatchID(),
                    baseStencilResolveInstance,
                    atlas.getEndStencilResolveInstance(), atlas.drawBounds());
            rtc->addDrawOp(nullptr, std::move(op));
            if (rtc->asSurfaceProxy()->requiresManualMSAAResolve()) {
                onFlushRP->addTextureResolveTask(sk_ref_sp(rtc->asTextureProxy()),
                                                 GrSurfaceProxy::ResolveFlags::kMSAA);
            }
        }

        SkASSERT(atlas.getEndStencilResolveInstance() >= baseStencilResolveInstance);
        baseStencilResolveInstance = atlas.getEndStencilResolveInstance();
    }
    SkASSERT(baseStencilResolveInstance == fEndStencilResolveInstance);

    return true;
}
