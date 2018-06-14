/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPerFlushResources.h"

#include "GrClip.h"
#include "GrOnFlushResourceProvider.h"
#include "GrSurfaceContextPriv.h"
#include "GrRenderTargetContext.h"
#include "SkMakeUnique.h"
#include "ccpr/GrCCPathCache.h"

using CoverageCountBatchID = GrCCPathParser::CoverageCountBatchID;
using PathInstance = GrCCPathProcessor::Instance;

namespace {

// Bass class for an Op that renders a CCPR atlas.
class AtlasOp : public GrDrawOp {
public:
    AtlasOp(uint32_t classID, sk_sp<const GrCCPerFlushResources> resources,
            const SkISize& drawBounds)
            : GrDrawOp(classID)
            , fResources(std::move(resources)) {
        this->setBounds(SkRect::MakeIWH(drawBounds.width(), drawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                GrPixelConfigIsClamped) override { return RequiresDstTexture::kNo; }
    bool onCombineIfPossible(GrOp* other, const GrCaps&) override { return false; }
    void onPrepare(GrOpFlushState*) override {}

protected:
    const sk_sp<const GrCCPerFlushResources> fResources;
};

// Copies paths from a stashed coverage count atlas into an 8-bit literal-coverage atlas.
class CopyAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          sk_sp<const GrCCPerFlushResources> resources,
                                          sk_sp<GrTextureProxy> copyProxy, int baseInstance,
                                          int endInstance, const SkISize& drawBounds) {
        return std::unique_ptr<GrDrawOp>(new CopyAtlasOp(std::move(resources), std::move(copyProxy),
                                                         baseInstance, endInstance, drawBounds));
    }

    const char* name() const override { return "CopyAtlasOp (CCPR)"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fn(fStashedAtlasProxy.get()); }

    void onExecute(GrOpFlushState* flushState) override {
        SkASSERT(fStashedAtlasProxy);
        GrPipeline pipeline(flushState->proxy(), GrPipeline::ScissorState::kDisabled,
                            SkBlendMode::kSrc);
        GrCCPathProcessor pathProc(flushState->resourceProvider(), std::move(fStashedAtlasProxy));
        pathProc.drawPaths(flushState, pipeline, *fResources, fBaseInstance, fEndInstance,
                           this->bounds());
        // Ensure we released the stashed atlas proxy. This allows its underlying texture to be
        // reused as the current flush's mainline CCPR atlas if needed.
        SkASSERT(!fStashedAtlasProxy);
    }

private:
    CopyAtlasOp(sk_sp<const GrCCPerFlushResources> resources, sk_sp<GrTextureProxy> copyProxy,
                int baseInstance, int endInstance, const SkISize& drawBounds)
            : AtlasOp(ClassID(), std::move(resources), drawBounds)
            , fStashedAtlasProxy(copyProxy)
            , fBaseInstance(baseInstance)
            , fEndInstance(endInstance) {
    }

    sk_sp<GrTextureProxy> fStashedAtlasProxy;
    const int fBaseInstance;
    const int fEndInstance;
};

// Renders coverage counts to a CCPR atlas using the resources' pre-filled GrCCPathParser.
class RenderAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          sk_sp<const GrCCPerFlushResources> resources,
                                          CoverageCountBatchID batchID, const SkISize& drawBounds) {
        return std::unique_ptr<GrDrawOp>(new RenderAtlasOp(std::move(resources), batchID,
                                                           drawBounds));
    }

    // GrDrawOp interface.
    const char* name() const override { return "RenderAtlasOp (CCPR)"; }

    void onExecute(GrOpFlushState* flushState) override {
        fResources->pathParser().drawCoverageCount(flushState, fBatchID, fDrawBounds);
    }

private:
    friend class GrOpMemoryPool; // for ctor

    RenderAtlasOp(sk_sp<const GrCCPerFlushResources> resources, CoverageCountBatchID batchID,
                  const SkISize& drawBounds)
            : AtlasOp(ClassID(), std::move(resources), drawBounds)
            , fBatchID(batchID)
            , fDrawBounds(SkIRect::MakeWH(drawBounds.width(), drawBounds.height())) {
    }

    const CoverageCountBatchID fBatchID;
    const SkIRect fDrawBounds;
};

}

static int instance_buffer_count(const GrCCPerFlushResourceSpecs& specs) {
    return specs.fNumCachedPaths +
           specs.fNumCopiedPaths*2 +  // 1 copy + 1 draw.
           specs.fNumRenderedPaths;
}

GrCCPerFlushResources::GrCCPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                             const GrCCPerFlushResourceSpecs& specs)
        : fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
        , fInstanceBuffer(onFlushRP->makeBuffer(kVertex_GrBufferType,
                                                instance_buffer_count(specs) *
                                                sizeof(PathInstance)))
        , fNextCopyInstanceIdx(0)
        , fNextPathInstanceIdx(specs.fNumCopiedPaths)
        , fPathParser(specs.fNumRenderedPaths + specs.fNumClipPaths, specs.fRenderedPathStats)
        , fCopyAtlasStack(specs.fCopyAtlasSpecs)
        , fRenderedAtlasStack(specs.fRenderedAtlasSpecs) {
    if (!fIndexBuffer) {
        SkDebugf("WARNING: failed to allocate CCPR index buffer. No paths will be drawn.\n");
        return;
    }
    if (!fVertexBuffer) {
        SkDebugf("WARNING: failed to allocate CCPR vertex buffer. No paths will be drawn.\n");
        return;
    }
    if (!fInstanceBuffer) {
        SkDebugf("WARNING: failed to allocate CCPR instance buffer. No paths will be drawn.\n");
        return;
    }
    fPathInstanceData = static_cast<PathInstance*>(fInstanceBuffer->map());
    SkASSERT(fPathInstanceData);
    SkDEBUGCODE(fEndCopyInstance = specs.fNumCopiedPaths);
    SkDEBUGCODE(fEndPathInstance = instance_buffer_count(specs));
}

GrCCAtlas* GrCCPerFlushResources::copyPathToCachedAtlas(const GrCCPathCacheEntry& entry,
                                                        GrCCPathProcessor::DoEvenOddFill evenOdd,
                                                        SkIVector* newAtlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextCopyInstanceIdx < fEndCopyInstance);
    SkASSERT(!entry.hasCachedAtlas());  // Not a problem if we ever actually want to do this.

    if (GrCCAtlas* retiredAtlas = fCopyAtlasStack.addRect(entry.devIBounds(), newAtlasOffset)) {
        // We did not fit in the previous copy atlas, so it was retired. It will render the copy
        // instances up until fNextCopyInstanceIdx.
        retiredAtlas->setRenderToken(fNextCopyInstanceIdx);
    }

    fPathInstanceData[fNextCopyInstanceIdx++].set(entry, *newAtlasOffset, GrColorPackA4(255),
                                                  evenOdd);
    return &fCopyAtlasStack.current();
}

const GrCCAtlas* GrCCPerFlushResources::renderPathInAtlas(const SkIRect& clipIBounds,
                                                          const SkMatrix& m, const SkPath& path,
                                                          SkRect* devBounds, SkRect* devBounds45,
                                                          SkIRect* devIBounds,
                                                          SkIVector* atlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx < fEndPathInstance);

    fPathParser.parsePath(m, path, devBounds, devBounds45);
    devBounds->roundOut(devIBounds);

    if (!this->placeParsedPathInAtlas(clipIBounds, *devIBounds, atlasOffset)) {
        SkDEBUGCODE(--fEndPathInstance);
        return nullptr;  // Path was degenerate or clipped away.
    }
    return &fRenderedAtlasStack.current();
}

const GrCCAtlas* GrCCPerFlushResources::renderDeviceSpacePathInAtlas(
        const SkIRect& clipIBounds, const SkPath& devPath, const SkIRect& devPathIBounds,
        SkIVector* atlasOffset) {
    SkASSERT(this->isMapped());
    fPathParser.parseDeviceSpacePath(devPath);
    if (!this->placeParsedPathInAtlas(clipIBounds, devPathIBounds, atlasOffset)) {
        return nullptr;
    }
    return &fRenderedAtlasStack.current();
}

bool GrCCPerFlushResources::placeParsedPathInAtlas(const SkIRect& clipIBounds,
                                                   const SkIRect& pathIBounds,
                                                   SkIVector* atlasOffset) {
    using ScissorMode = GrCCPathParser::ScissorMode;
    ScissorMode scissorMode;
    SkIRect clippedPathIBounds;
    if (clipIBounds.contains(pathIBounds)) {
        clippedPathIBounds = pathIBounds;
        scissorMode = ScissorMode::kNonScissored;
    } else if (clippedPathIBounds.intersect(clipIBounds, pathIBounds)) {
        scissorMode = ScissorMode::kScissored;
    } else {
        fPathParser.discardParsedPath();
        return false;
    }

    if (GrCCAtlas* retiredAtlas = fRenderedAtlasStack.addRect(clippedPathIBounds, atlasOffset)) {
        // We did not fit in the previous coverage count atlas, so it was retired. Close off of the
        // path parser's current batch and the retired atlas will render it.
        retiredAtlas->setRenderToken(fPathParser.closeCurrentBatch());
    }
    fPathParser.saveParsedPath(scissorMode, clippedPathIBounds, *atlasOffset);
    return true;
}

bool GrCCPerFlushResources::finalize(GrOnFlushResourceProvider* onFlushRP,
                                     sk_sp<GrTextureProxy> stashedAtlasProxy,
                                     SkTArray<sk_sp<GrRenderTargetContext>>* out) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx == fEndPathInstance);
    // No assert for fEndCopyInstance because the caller may have detected and skipped duplicates.

    fInstanceBuffer->unmap();
    fPathInstanceData = nullptr;

    if (!fCopyAtlasStack.empty()) {
        fCopyAtlasStack.current().setRenderToken(fNextCopyInstanceIdx);
    }
    if (!fRenderedAtlasStack.empty()) {
        fRenderedAtlasStack.current().setRenderToken(fPathParser.closeCurrentBatch());
    }

    // Build the GPU buffers to render path coverage counts. (This must not happen until after the
    // final call to fPathParser.closeCurrentBatch().)
    if (!fPathParser.finalize(onFlushRP)) {
        SkDebugf("WARNING: failed to allocate GPU buffers for CCPR. No paths will be drawn.\n");
        return false;
    }

    // Draw the copies from the stashed atlas into 8-bit cached atlas(es).
    int baseCopyInstance = 0;
    for (GrCCAtlasStack::Iter atlas(fCopyAtlasStack); atlas.next();) {
        int endCopyInstance = atlas->renderToken();
        if (endCopyInstance <= baseCopyInstance) {
            SkASSERT(endCopyInstance == baseCopyInstance);
            continue;
        }
        if (auto rtc = atlas->makeClearedTextureProxy(onFlushRP, kAlpha_8_GrPixelConfig)) {
            GrContext* ctx = rtc->surfPriv().getContext();
            auto op = CopyAtlasOp::Make(ctx, sk_ref_sp(this), stashedAtlasProxy, baseCopyInstance,
                                        endCopyInstance, atlas->drawBounds());
            rtc->addDrawOp(GrNoClip(), std::move(op));
            out->push_back(std::move(rtc));
        }
        baseCopyInstance = endCopyInstance;
    }

    // Release the stashed atlas before creating new one(s). This allows us to recycle the same
    // underlying texture with the upcoming rendered atlases.
    stashedAtlasProxy = nullptr;

    // Render the coverage count atlas(es).
    for (GrCCAtlasStack::Iter atlas(fRenderedAtlasStack); atlas.next();) {
        if (auto rtc = atlas->makeClearedTextureProxy(onFlushRP, kAlpha_half_GrPixelConfig)) {
            auto op = RenderAtlasOp::Make(rtc->surfPriv().getContext(), sk_ref_sp(this),
                                          atlas->renderToken(), atlas->drawBounds());
            rtc->addDrawOp(GrNoClip(), std::move(op));
            out->push_back(std::move(rtc));
        }
    }

    return true;
}

void GrCCPerFlushResourceSpecs::convertCopiesToRenders() {
    fNumRenderedPaths += fNumCopiedPaths;
    fNumCopiedPaths = 0;

    fRenderedAtlasSpecs.fApproxNumPixels += fCopyAtlasSpecs.fApproxNumPixels;
    fRenderedAtlasSpecs.fMinWidth =
            SkTMax(fRenderedAtlasSpecs.fMinWidth, fCopyAtlasSpecs.fMinWidth);
    fRenderedAtlasSpecs.fMinHeight =
            SkTMax(fRenderedAtlasSpecs.fMinHeight, fCopyAtlasSpecs.fMinHeight);
    fCopyAtlasSpecs = GrCCAtlas::Specs();

    fRenderedPathStats.fMaxPointsPerPath =
            SkTMax(fRenderedPathStats.fMaxPointsPerPath, fCopyPathStats.fMaxPointsPerPath);
    fRenderedPathStats.fNumTotalSkPoints += fCopyPathStats.fNumTotalSkPoints;
    fRenderedPathStats.fNumTotalSkVerbs += fCopyPathStats.fNumTotalSkVerbs;
    fRenderedPathStats.fNumTotalConicWeights += fCopyPathStats.fNumTotalConicWeights;
    fCopyPathStats = GrCCPathParser::PathStats();
}
