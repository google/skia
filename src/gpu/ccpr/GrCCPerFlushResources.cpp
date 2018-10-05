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
#include "GrShape.h"
#include "SkMakeUnique.h"
#include "ccpr/GrCCPathCache.h"

using FillBatchID = GrCCFiller::BatchID;
using StrokeBatchID = GrCCStroker::BatchID;
using PathInstance = GrCCPathProcessor::Instance;

static constexpr int kFillIdx = GrCCPerFlushResourceSpecs::kFillIdx;
static constexpr int kStrokeIdx = GrCCPerFlushResourceSpecs::kStrokeIdx;

namespace {

// Base class for an Op that renders a CCPR atlas.
class AtlasOp : public GrDrawOp {
public:
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    CombineResult onCombineIfPossible(GrOp* other, const GrCaps&) override {
        SK_ABORT("Only expected one Op per CCPR atlas.");
        return CombineResult::kMerged;
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
        GrPipeline::FixedDynamicState dynamicState;
        auto atlasProxy = fStashedAtlasProxy.get();
        dynamicState.fPrimitiveProcessorTextures = &atlasProxy;

        GrPipeline pipeline(flushState->proxy(), GrScissorTest::kDisabled, SkBlendMode::kSrc);
        GrCCPathProcessor pathProc(atlasProxy);
        pathProc.drawPaths(flushState, pipeline, &dynamicState, *fResources, fBaseInstance,
                           fEndInstance, this->bounds());
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
                                          FillBatchID fillBatchID, StrokeBatchID strokeBatchID,
                                          const SkISize& drawBounds) {
        GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

        return pool->allocate<RenderAtlasOp>(std::move(resources), fillBatchID, strokeBatchID,
                                             drawBounds);
    }

    // GrDrawOp interface.
    const char* name() const override { return "RenderAtlasOp (CCPR)"; }

    void onExecute(GrOpFlushState* flushState) override {
        fResources->filler().drawFills(flushState, fFillBatchID, fDrawBounds);
        fResources->stroker().drawStrokes(flushState, fStrokeBatchID, fDrawBounds);
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    RenderAtlasOp(sk_sp<const GrCCPerFlushResources> resources, FillBatchID fillBatchID,
                  StrokeBatchID strokeBatchID, const SkISize& drawBounds)
            : AtlasOp(ClassID(), std::move(resources), drawBounds)
            , fFillBatchID(fillBatchID)
            , fStrokeBatchID(strokeBatchID)
            , fDrawBounds(SkIRect::MakeWH(drawBounds.width(), drawBounds.height())) {
    }

    const FillBatchID fFillBatchID;
    const StrokeBatchID fStrokeBatchID;
    const SkIRect fDrawBounds;
};

}

static int inst_buffer_count(const GrCCPerFlushResourceSpecs& specs) {
    return specs.fNumCachedPaths +
           // Copies get two instances per draw: 1 copy + 1 draw.
           (specs.fNumCopiedPaths[kFillIdx] + specs.fNumCopiedPaths[kStrokeIdx]) * 2 +
           specs.fNumRenderedPaths[kFillIdx] + specs.fNumRenderedPaths[kStrokeIdx];
           // No clips in instance buffers.
}

GrCCPerFlushResources::GrCCPerFlushResources(GrOnFlushResourceProvider* onFlushRP,
                                             const GrCCPerFlushResourceSpecs& specs)
          // Overallocate by one point so we can call Sk4f::Store at the final SkPoint in the array.
          // (See transform_path_pts below.)
          // FIXME: instead use built-in instructions to write only the first two lanes of an Sk4f.
        : fLocalDevPtsBuffer(SkTMax(specs.fRenderedPathStats[kFillIdx].fMaxPointsPerPath,
                                    specs.fRenderedPathStats[kStrokeIdx].fMaxPointsPerPath) + 1)
        , fFiller(specs.fNumRenderedPaths[kFillIdx] + specs.fNumClipPaths,
                  specs.fRenderedPathStats[kFillIdx].fNumTotalSkPoints,
                  specs.fRenderedPathStats[kFillIdx].fNumTotalSkVerbs,
                  specs.fRenderedPathStats[kFillIdx].fNumTotalConicWeights)
        , fStroker(specs.fNumRenderedPaths[kStrokeIdx],
                   specs.fRenderedPathStats[kStrokeIdx].fNumTotalSkPoints,
                   specs.fRenderedPathStats[kStrokeIdx].fNumTotalSkVerbs)
        , fCopyAtlasStack(kAlpha_8_GrPixelConfig, specs.fCopyAtlasSpecs, onFlushRP->caps())
        , fRenderedAtlasStack(kAlpha_half_GrPixelConfig, specs.fRenderedAtlasSpecs,
                              onFlushRP->caps())
        , fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
        , fInstanceBuffer(onFlushRP->makeBuffer(kVertex_GrBufferType,
                                                inst_buffer_count(specs) * sizeof(PathInstance)))
        , fNextCopyInstanceIdx(0)
        , fNextPathInstanceIdx(specs.fNumCopiedPaths[kFillIdx] +
                               specs.fNumCopiedPaths[kStrokeIdx]) {
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
    SkDEBUGCODE(fEndCopyInstance =
                        specs.fNumCopiedPaths[kFillIdx] + specs.fNumCopiedPaths[kStrokeIdx]);
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
        retiredAtlas->setFillBatchID(fNextCopyInstanceIdx);
    }

    fPathInstanceData[fNextCopyInstanceIdx++].set(entry, *newAtlasOffset, GrColor_WHITE, evenOdd);
    return &fCopyAtlasStack.current();
}

static bool transform_path_pts(const SkMatrix& m, const SkPath& path,
                               const SkAutoSTArray<32, SkPoint>& outDevPts, SkRect* devBounds,
                               SkRect* devBounds45) {
    const SkPoint* pts = SkPathPriv::PointData(path);
    int numPts = path.countPoints();
    SkASSERT(numPts + 1 <= outDevPts.count());
    SkASSERT(numPts);

    // m45 transforms path points into "45 degree" device space. A bounding box in this space gives
    // the circumscribing octagon's diagonals. We could use SK_ScalarRoot2Over2, but an orthonormal
    // transform is not necessary as long as the shader uses the correct inverse.
    SkMatrix m45;
    m45.setSinCos(1, 1);
    m45.preConcat(m);

    // X,Y,T are two parallel view matrices that accumulate two bounding boxes as they map points:
    // device-space bounds and "45 degree" device-space bounds (| 1 -1 | * devCoords).
    //                                                          | 1  1 |
    Sk4f X = Sk4f(m.getScaleX(), m.getSkewY(), m45.getScaleX(), m45.getSkewY());
    Sk4f Y = Sk4f(m.getSkewX(), m.getScaleY(), m45.getSkewX(), m45.getScaleY());
    Sk4f T = Sk4f(m.getTranslateX(), m.getTranslateY(), m45.getTranslateX(), m45.getTranslateY());

    // Map the path's points to device space and accumulate bounding boxes.
    Sk4f devPt = SkNx_fma(Y, Sk4f(pts[0].y()), T);
    devPt = SkNx_fma(X, Sk4f(pts[0].x()), devPt);
    Sk4f topLeft = devPt;
    Sk4f bottomRight = devPt;

    // Store all 4 values [dev.x, dev.y, dev45.x, dev45.y]. We are only interested in the first two,
    // and will overwrite [dev45.x, dev45.y] with the next point. This is why the dst buffer must
    // be at least one larger than the number of points.
    devPt.store(&outDevPts[0]);

    for (int i = 1; i < numPts; ++i) {
        devPt = SkNx_fma(Y, Sk4f(pts[i].y()), T);
        devPt = SkNx_fma(X, Sk4f(pts[i].x()), devPt);
        topLeft = Sk4f::Min(topLeft, devPt);
        bottomRight = Sk4f::Max(bottomRight, devPt);
        devPt.store(&outDevPts[i]);
    }

    if (!(Sk4f(0) == topLeft*0).allTrue() || !(Sk4f(0) == bottomRight*0).allTrue()) {
        // The bounds are infinite or NaN.
        return false;
    }

    SkPoint topLeftPts[2], bottomRightPts[2];
    topLeft.store(topLeftPts);
    bottomRight.store(bottomRightPts);
    devBounds->setLTRB(topLeftPts[0].x(), topLeftPts[0].y(), bottomRightPts[0].x(),
                       bottomRightPts[0].y());
    devBounds45->setLTRB(topLeftPts[1].x(), topLeftPts[1].y(), bottomRightPts[1].x(),
                         bottomRightPts[1].y());
    return true;
}

const GrCCAtlas* GrCCPerFlushResources::renderShapeInAtlas(
        const SkIRect& clipIBounds, const SkMatrix& m, const GrShape& shape, float strokeDevWidth,
        SkRect* devBounds, SkRect* devBounds45, SkIRect* devIBounds, SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx < fEndPathInstance);

    SkPath path;
    shape.asPath(&path);
    if (path.isEmpty()) {
        SkDEBUGCODE(--fEndPathInstance);
        return nullptr;
    }
    if (!transform_path_pts(m, path, fLocalDevPtsBuffer, devBounds, devBounds45)) {
        // The transformed path had infinite or NaN bounds.
        SkDEBUGCODE(--fEndPathInstance);
        return nullptr;
    }

    const SkStrokeRec& stroke = shape.style().strokeRec();
    if (!stroke.isFillStyle()) {
        float r = SkStrokeRec::GetInflationRadius(stroke.getJoin(), stroke.getMiter(),
                                                  stroke.getCap(), strokeDevWidth);
        devBounds->outset(r, r);
        // devBounds45 is in (| 1 -1 | * devCoords) space.
        //                    | 1  1 |
        devBounds45->outset(r*SK_ScalarSqrt2, r*SK_ScalarSqrt2);
    }
    devBounds->roundOut(devIBounds);

    GrScissorTest scissorTest;
    SkIRect clippedPathIBounds;
    if (!this->placeRenderedPathInAtlas(clipIBounds, *devIBounds, &scissorTest, &clippedPathIBounds,
                                        devToAtlasOffset)) {
        SkDEBUGCODE(--fEndPathInstance);
        return nullptr;  // Path was degenerate or clipped away.
    }

    if (stroke.isFillStyle()) {
        SkASSERT(0 == strokeDevWidth);
        fFiller.parseDeviceSpaceFill(path, fLocalDevPtsBuffer.begin(), scissorTest,
                                     clippedPathIBounds, *devToAtlasOffset);
    } else {
        // Stroke-and-fill is not yet supported.
        SkASSERT(SkStrokeRec::kStroke_Style == stroke.getStyle() || stroke.isHairlineStyle());
        SkASSERT(!stroke.isHairlineStyle() || 1 == strokeDevWidth);
        fStroker.parseDeviceSpaceStroke(path, fLocalDevPtsBuffer.begin(), stroke, strokeDevWidth,
                                        scissorTest, clippedPathIBounds, *devToAtlasOffset);
    }
    return &fRenderedAtlasStack.current();
}

const GrCCAtlas* GrCCPerFlushResources::renderDeviceSpacePathInAtlas(
        const SkIRect& clipIBounds, const SkPath& devPath, const SkIRect& devPathIBounds,
        SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());

    if (devPath.isEmpty()) {
        return nullptr;
    }

    GrScissorTest scissorTest;
    SkIRect clippedPathIBounds;
    if (!this->placeRenderedPathInAtlas(clipIBounds, devPathIBounds, &scissorTest,
                                        &clippedPathIBounds, devToAtlasOffset)) {
        return nullptr;
    }

    fFiller.parseDeviceSpaceFill(devPath, SkPathPriv::PointData(devPath), scissorTest,
                                 clippedPathIBounds, *devToAtlasOffset);
    return &fRenderedAtlasStack.current();
}

bool GrCCPerFlushResources::placeRenderedPathInAtlas(const SkIRect& clipIBounds,
                                                     const SkIRect& pathIBounds,
                                                     GrScissorTest* scissorTest,
                                                     SkIRect* clippedPathIBounds,
                                                     SkIVector* devToAtlasOffset) {
    if (clipIBounds.contains(pathIBounds)) {
        *clippedPathIBounds = pathIBounds;
        *scissorTest = GrScissorTest::kDisabled;
    } else if (clippedPathIBounds->intersect(clipIBounds, pathIBounds)) {
        *scissorTest = GrScissorTest::kEnabled;
    } else {
        return false;
    }

    if (GrCCAtlas* retiredAtlas =
                fRenderedAtlasStack.addRect(*clippedPathIBounds, devToAtlasOffset)) {
        // We did not fit in the previous coverage count atlas and it was retired. Close the path
        // parser's current batch (which does not yet include the path we just parsed). We will
        // render this batch into the retired atlas during finalize().
        retiredAtlas->setFillBatchID(fFiller.closeCurrentBatch());
        retiredAtlas->setStrokeBatchID(fStroker.closeCurrentBatch());
    }
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
        fCopyAtlasStack.current().setFillBatchID(fNextCopyInstanceIdx);
    }
    if (!fRenderedAtlasStack.empty()) {
        fRenderedAtlasStack.current().setFillBatchID(fFiller.closeCurrentBatch());
        fRenderedAtlasStack.current().setStrokeBatchID(fStroker.closeCurrentBatch());
    }

    // Build the GPU buffers to render path coverage counts. (This must not happen until after the
    // final calls to fFiller/fStroker.closeCurrentBatch().)
    if (!fFiller.prepareToDraw(onFlushRP)) {
        return false;
    }
    if (!fStroker.prepareToDraw(onFlushRP)) {
        return false;
    }

    // Draw the copies from the stashed atlas into 8-bit cached atlas(es).
    int baseCopyInstance = 0;
    for (GrCCAtlasStack::Iter atlas(fCopyAtlasStack); atlas.next();) {
        int endCopyInstance = atlas->getFillBatchID();
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
            backingTexture = sk_ref_sp(stashedAtlasProxy->peekTexture());
        }

        // Delete the stashed proxy here. That way, if we can't recycle the stashed atlas texture,
        // we free this memory prior to allocating a new backing texture.
        stashedAtlasProxy = nullptr;

        if (auto rtc = atlas->makeRenderTargetContext(onFlushRP, std::move(backingTexture))) {
            auto op = RenderAtlasOp::Make(rtc->surfPriv().getContext(), sk_ref_sp(this),
                                          atlas->getFillBatchID(), atlas->getStrokeBatchID(),
                                          atlas->drawBounds());
            rtc->addDrawOp(GrNoClip(), std::move(op));
            out->push_back(std::move(rtc));
        }
    }

    return true;
}

void GrCCPerFlushResourceSpecs::convertCopiesToRenders() {
    for (int i = 0; i < 2; ++i) {
        fNumRenderedPaths[i] += fNumCopiedPaths[i];
        fNumCopiedPaths[i] = 0;

        fRenderedPathStats[i].fMaxPointsPerPath =
               SkTMax(fRenderedPathStats[i].fMaxPointsPerPath, fCopyPathStats[i].fMaxPointsPerPath);
        fRenderedPathStats[i].fNumTotalSkPoints += fCopyPathStats[i].fNumTotalSkPoints;
        fRenderedPathStats[i].fNumTotalSkVerbs += fCopyPathStats[i].fNumTotalSkVerbs;
        fRenderedPathStats[i].fNumTotalConicWeights += fCopyPathStats[i].fNumTotalConicWeights;
        fCopyPathStats[i] = GrCCRenderedPathStats();
    }

    fRenderedAtlasSpecs.fApproxNumPixels += fCopyAtlasSpecs.fApproxNumPixels;
    fRenderedAtlasSpecs.fMinWidth =
            SkTMax(fRenderedAtlasSpecs.fMinWidth, fCopyAtlasSpecs.fMinWidth);
    fRenderedAtlasSpecs.fMinHeight =
            SkTMax(fRenderedAtlasSpecs.fMinHeight, fCopyAtlasSpecs.fMinHeight);
    fCopyAtlasSpecs = GrCCAtlas::Specs();
}
