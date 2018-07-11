/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPerFlushResources.h"

#include "GrClip.h"
#include "GrMemoryPool.h"
#include "GrOnFlushResourceProvider.h"
#include "GrSurfaceContextPriv.h"
#include "GrRenderTargetContext.h"
#include "SkMakeUnique.h"
#include "ccpr/GrCCPathCache.h"

using CoverageCountBatchID = GrCCPathParser::CoverageCountBatchID;
using PathInstance = GrCCPathProcessor::Instance;

namespace {

// Base class for an Op that renders a CCPR atlas.
class AtlasOp : public GrDrawOp {
public:
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps&) override {
        SK_ABORT("Only expected one Op per CCPR atlas.");
        return true;
    }
    void onPrepare(GrOpFlushState*) override {}

protected:
    AtlasOp(uint32_t classID, sk_sp<const GrCCPerFlushResources> resources,
            const SkISize& drawBounds)
            : GrDrawOp(classID)
            , fResources(std::move(resources)) {
        this->setBounds(SkRect::MakeIWH(drawBounds.width(), drawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
    }

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
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<CopyAtlasOp>(std::move(resources), std::move(copyProxy),
                                           baseInstance, endInstance, drawBounds);
    }

    const char* name() const override { return "CopyAtlasOp (CCPR)"; }
    void visitProxies(const VisitProxyFunc& fn) const override { fn(fStashedAtlasProxy.get()); }

    void onExecute(GrOpFlushState* flushState) override {
        SkASSERT(fStashedAtlasProxy);
        GrPipeline pipeline(flushState->proxy(), GrPipeline::ScissorState::kDisabled,
                            SkBlendMode::kSrc);
        GrCCPathProcessor pathProc(flushState->resourceProvider(), std::move(fStashedAtlasProxy));
        pathProc.drawPaths(flushState, pipeline, nullptr, *fResources, fBaseInstance, fEndInstance,
                           this->bounds());
        // Ensure we released the stashed atlas proxy. This allows its underlying texture to be
        // reused as the current flush's mainline CCPR atlas if needed.
        SkASSERT(!fStashedAtlasProxy);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

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
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<RenderAtlasOp>(std::move(resources), batchID, drawBounds);
    }

    // GrDrawOp interface.
    const char* name() const override { return "RenderAtlasOp (CCPR)"; }

    void onExecute(GrOpFlushState* flushState) override {
        fResources->pathParser().drawCoverageCount(flushState, fBatchID, fDrawBounds);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

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

static int inst_buffer_count(const GrCCPerFlushResourceSpecs& specs) {
    return specs.fNumCachedPaths +
           specs.fNumCopiedPaths*2 +  // 1 copy + 1 draw.
           specs.fNumRenderedPaths;
}

GrCCPerFlushResources::GrCCPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                             const GrCCPerFlushResourceSpecs& specs)
        : fPathParser(specs.fNumRenderedPaths + specs.fNumClipPaths, specs.fRenderedPathStats)
        , fCopyAtlasStack(kAlpha_8_GrPixelConfig, specs.fCopyAtlasSpecs, onFlushRP->caps())
        , fRenderedAtlasStack(kAlpha_half_GrPixelConfig, specs.fRenderedAtlasSpecs,
                              onFlushRP->caps())
        , fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
        , fInstanceBuffer(onFlushRP->makeBuffer(kVertex_GrBufferType,
                                                inst_buffer_count(specs) * sizeof(PathInstance)))
        , fNextCopyInstanceIdx(0)
        , fNextPathInstanceIdx(specs.fNumCopiedPaths) {
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
    SkDEBUGCODE(fEndPathInstance = inst_buffer_count(specs));
}

GrCCAtlas* GrCCPerFlushResources::copyPathToCachedAtlas(const GrCCPathCacheEntry& entry,
                                                        GrCCPathProcessor::DoEvenOddFill evenOdd,
                                                        SkIVector* newAtlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextCopyInstanceIdx < fEndCopyInstance);
    SkASSERT(!entry.hasCachedAtlas());  // Unexpected, but not necessarily a problem.

    if (GrCCAtlas* retiredAtlas = fCopyAtlasStack.addRect(entry.devIBounds(), newAtlasOffset)) {
        // We did not fit in the previous copy atlas and it was retired. We will render the copies
        // up until fNextCopyInstanceIdx into the retired atlas during finalize().
        retiredAtlas->setUserBatchID(fNextCopyInstanceIdx);
    }

    fPathInstanceData[fNextCopyInstanceIdx++].set(entry, *newAtlasOffset, GrColor_WHITE, evenOdd);
    return &fCopyAtlasStack.current();
}

const GrCCAtlas* GrCCPerFlushResources::renderPathInAtlas(const SkIRect& clipIBounds,
                                                          const SkMatrix& m, const SkPath& path,
                                                          SkRect* devBounds, SkRect* devBounds45,
                                                          SkIRect* devIBounds,
                                                          SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx < fEndPathInstance);

    fPathParser.parsePath(m, path, devBounds, devBounds45);
    devBounds->roundOut(devIBounds);

    if (!this->placeParsedPathInAtlas(clipIBounds, *devIBounds, devToAtlasOffset)) {
        SkDEBUGCODE(--fEndPathInstance);
        return nullptr;  // Path was degenerate or clipped away.
    }
    return &fRenderedAtlasStack.current();
}

const GrCCAtlas* GrCCPerFlushResources::renderDeviceSpacePathInAtlas(
        const SkIRect& clipIBounds, const SkPath& devPath, const SkIRect& devPathIBounds,
        SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());
    fPathParser.parseDeviceSpacePath(devPath);
    if (!this->placeParsedPathInAtlas(clipIBounds, devPathIBounds, devToAtlasOffset)) {
        return nullptr;
    }
    return &fRenderedAtlasStack.current();
}

bool GrCCPerFlushResources::placeParsedPathInAtlas(const SkIRect& clipIBounds,
                                                   const SkIRect& pathIBounds,
                                                   SkIVector* devToAtlasOffset) {
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

    if (GrCCAtlas* retiredAtlas =
                fRenderedAtlasStack.addRect(clippedPathIBounds, devToAtlasOffset)) {
        // We did not fit in the previous coverage count atlas and it was retired. Close the path
        // parser's current batch (which does not yet include the path we just parsed). We will
        // render this batch into the retired atlas during finalize().
        CoverageCountBatchID batchID = fPathParser.closeCurrentBatch();
        retiredAtlas->setUserBatchID(batchID);
    }
    fPathParser.saveParsedPath(scissorMode, clippedPathIBounds, *devToAtlasOffset);
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
        fCopyAtlasStack.current().setUserBatchID(fNextCopyInstanceIdx);
    }
    if (!fRenderedAtlasStack.empty()) {
        CoverageCountBatchID batchID = fPathParser.closeCurrentBatch();
        fRenderedAtlasStack.current().setUserBatchID(batchID);
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
        int endCopyInstance = atlas->getUserBatchID();
        if (endCopyInstance <= baseCopyInstance) {
            SkASSERT(endCopyInstance == baseCopyInstance);
            continue;
        }
        if (auto rtc = atlas->makeRenderTargetContext(onFlushRP)) {
            GrContext* ctx = rtc->surfPriv().getContext();
            auto op = CopyAtlasOp::Make(ctx, sk_ref_sp(this), stashedAtlasProxy, baseCopyInstance,
                                        endCopyInstance, atlas->drawBounds());
            rtc->addDrawOp(GrNoClip(), std::move(op));
            out->push_back(std::move(rtc));
        }
        baseCopyInstance = endCopyInstance;
    }

    // Render the coverage count atlas(es).
    for (GrCCAtlasStack::Iter atlas(fRenderedAtlasStack); atlas.next();) {
        // Copies will be finished by the time we get to this atlas. See if we can recycle the
        // stashed atlas texture instead of creating a new one.
        sk_sp<GrTexture> backingTexture;
        if (stashedAtlasProxy && atlas->currentWidth() == stashedAtlasProxy->width() &&
            atlas->currentHeight() == stashedAtlasProxy->height()) {
            backingTexture = sk_ref_sp(stashedAtlasProxy->priv().peekTexture());
            stashedAtlasProxy = nullptr;
        }

        if (auto rtc = atlas->makeRenderTargetContext(onFlushRP, std::move(backingTexture))) {
            auto op = RenderAtlasOp::Make(rtc->surfPriv().getContext(), sk_ref_sp(this),
                                          atlas->getUserBatchID(), atlas->drawBounds());
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
