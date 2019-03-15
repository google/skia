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
#include "GrRecordingContext.h"
#include "GrRecordingContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrShape.h"
#include "GrSurfaceContextPriv.h"
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
    GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, GrFSAAType, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }
    CombineResult onCombineIfPossible(GrOp* other, const GrCaps&) override {
        // We will only make multiple copy ops if they have different source proxies.
        // TODO: make use of texture chaining.
        return CombineResult::kCannotCombine;
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

// Copies paths from a cached coverage count atlas into an 8-bit literal-coverage atlas.
class CopyAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          sk_sp<const GrCCPerFlushResources> resources,
                                          sk_sp<GrTextureProxy> copyProxy, int baseInstance,
                                          int endInstance, const SkISize& drawBounds) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<CopyAtlasOp>(std::move(resources), std::move(copyProxy),
                                           baseInstance, endInstance, drawBounds);
    }

    const char* name() const override { return "CopyAtlasOp (CCPR)"; }
    void visitProxies(const VisitProxyFunc& fn, VisitorType) const override { fn(fSrcProxy.get()); }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        SkASSERT(fSrcProxy);
        GrPipeline::FixedDynamicState dynamicState;
        auto srcProxy = fSrcProxy.get();
        dynamicState.fPrimitiveProcessorTextures = &srcProxy;

        GrPipeline pipeline(GrScissorTest::kDisabled, SkBlendMode::kSrc);
        GrCCPathProcessor pathProc(srcProxy);
        pathProc.drawPaths(flushState, pipeline, &dynamicState, *fResources, fBaseInstance,
                           fEndInstance, this->bounds());
    }

private:
    friend class ::GrOpMemoryPool; // for ctor

    CopyAtlasOp(sk_sp<const GrCCPerFlushResources> resources, sk_sp<GrTextureProxy> srcProxy,
                int baseInstance, int endInstance, const SkISize& drawBounds)
            : AtlasOp(ClassID(), std::move(resources), drawBounds)
            , fSrcProxy(srcProxy)
            , fBaseInstance(baseInstance)
            , fEndInstance(endInstance) {
    }
    sk_sp<GrTextureProxy> fSrcProxy;
    const int fBaseInstance;
    const int fEndInstance;
};

// Renders coverage counts to a CCPR atlas using the resources' pre-filled GrCCPathParser.
class RenderAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          sk_sp<const GrCCPerFlushResources> resources,
                                          FillBatchID fillBatchID, StrokeBatchID strokeBatchID,
                                          const SkISize& drawBounds) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<RenderAtlasOp>(std::move(resources), fillBatchID, strokeBatchID,
                                             drawBounds);
    }

    // GrDrawOp interface.
    const char* name() const override { return "RenderAtlasOp (CCPR)"; }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
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
        , fCopyAtlasStack(GrCCAtlas::CoverageType::kA8_LiteralCoverage, specs.fCopyAtlasSpecs,
                          onFlushRP->caps())
        , fRenderedAtlasStack(GrCCAtlas::CoverageType::kFP16_CoverageCount,
                              specs.fRenderedAtlasSpecs, onFlushRP->caps())
        , fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
        , fInstanceBuffer(onFlushRP->makeBuffer(GrGpuBufferType::kVertex,
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

void GrCCPerFlushResources::upgradeEntryToLiteralCoverageAtlas(
        GrCCPathCache* pathCache, GrOnFlushResourceProvider* onFlushRP, GrCCPathCacheEntry* entry,
        GrCCPathProcessor::DoEvenOddFill evenOdd) {
    using ReleaseAtlasResult = GrCCPathCacheEntry::ReleaseAtlasResult;
    SkASSERT(this->isMapped());
    SkASSERT(fNextCopyInstanceIdx < fEndCopyInstance);

    const GrCCCachedAtlas* cachedAtlas = entry->cachedAtlas();
    SkASSERT(cachedAtlas);
    SkASSERT(cachedAtlas->getOnFlushProxy());

    if (GrCCAtlas::CoverageType::kA8_LiteralCoverage == cachedAtlas->coverageType()) {
        // This entry has already been upgraded to literal coverage. The path must have been drawn
        // multiple times during the flush.
        SkDEBUGCODE(--fEndCopyInstance);
        return;
    }

    SkIVector newAtlasOffset;
    if (GrCCAtlas* retiredAtlas = fCopyAtlasStack.addRect(entry->devIBounds(), &newAtlasOffset)) {
        // We did not fit in the previous copy atlas and it was retired. We will render the ranges
        // up until fCopyPathRanges.count() into the retired atlas during finalize().
        retiredAtlas->setFillBatchID(fCopyPathRanges.count());
        fCurrCopyAtlasRangesIdx = fCopyPathRanges.count();
    }

    this->recordCopyPathInstance(*entry, newAtlasOffset, evenOdd,
                                 sk_ref_sp(cachedAtlas->getOnFlushProxy()));

    sk_sp<GrTexture> previousAtlasTexture =
            sk_ref_sp(cachedAtlas->getOnFlushProxy()->peekTexture());
    GrCCAtlas* newAtlas = &fCopyAtlasStack.current();
    if (ReleaseAtlasResult::kDidInvalidateFromCache ==
            entry->upgradeToLiteralCoverageAtlas(pathCache, onFlushRP, newAtlas, newAtlasOffset)) {
        // This texture just got booted out of the cache. Keep it around, in case we might be able
        // to recycle it for a new atlas. We can recycle it because copying happens before rendering
        // new paths, and every path from the atlas that we're planning to use this flush will be
        // copied to a new atlas. We'll never copy some and leave others.
        fRecyclableAtlasTextures.push_back(std::move(previousAtlasTexture));
    }
}

template<typename T, typename... Args>
static void emplace_at_memcpy(SkTArray<T>* array, int idx, Args&&... args) {
    if (int moveCount = array->count() - idx) {
        array->push_back();
        T* location = array->begin() + idx;
        memcpy(location+1, location, moveCount * sizeof(T));
        new (location) T(std::forward<Args>(args)...);
    } else {
        array->emplace_back(std::forward<Args>(args)...);
    }
}

void GrCCPerFlushResources::recordCopyPathInstance(const GrCCPathCacheEntry& entry,
                                                   const SkIVector& newAtlasOffset,
                                                   GrCCPathProcessor::DoEvenOddFill evenOdd,
                                                   sk_sp<GrTextureProxy> srcProxy) {
    SkASSERT(fNextCopyInstanceIdx < fEndCopyInstance);

    // Write the instance at the back of the array.
    int currentInstanceIdx = fNextCopyInstanceIdx++;
    constexpr uint64_t kWhite = (((uint64_t) SK_Half1) <<  0) |
                                (((uint64_t) SK_Half1) << 16) |
                                (((uint64_t) SK_Half1) << 32) |
                                (((uint64_t) SK_Half1) << 48);
    fPathInstanceData[currentInstanceIdx].set(entry, newAtlasOffset, kWhite, evenOdd);

    // Percolate the instance forward until it's contiguous with other instances that share the same
    // proxy.
    for (int i = fCopyPathRanges.count() - 1; i >= fCurrCopyAtlasRangesIdx; --i) {
        if (fCopyPathRanges[i].fSrcProxy == srcProxy) {
            ++fCopyPathRanges[i].fCount;
            return;
        }
        int rangeFirstInstanceIdx = currentInstanceIdx - fCopyPathRanges[i].fCount;
        std::swap(fPathInstanceData[rangeFirstInstanceIdx], fPathInstanceData[currentInstanceIdx]);
        currentInstanceIdx = rangeFirstInstanceIdx;
    }

    // An instance with this particular proxy did not yet exist in the array. Add a range for it.
    emplace_at_memcpy(&fCopyPathRanges, fCurrCopyAtlasRangesIdx, std::move(srcProxy), 1);
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

GrCCAtlas* GrCCPerFlushResources::renderShapeInAtlas(
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
                                     SkTArray<sk_sp<GrRenderTargetContext>>* out) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx == fEndPathInstance);
    SkASSERT(fNextCopyInstanceIdx == fEndCopyInstance);

    fInstanceBuffer->unmap();
    fPathInstanceData = nullptr;

    if (!fCopyAtlasStack.empty()) {
        fCopyAtlasStack.current().setFillBatchID(fCopyPathRanges.count());
        fCurrCopyAtlasRangesIdx = fCopyPathRanges.count();
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

    // Draw the copies from 16-bit literal coverage atlas(es) into 8-bit cached atlas(es).
    int copyRangeIdx = 0;
    int baseCopyInstance = 0;
    for (GrCCAtlasStack::Iter atlas(fCopyAtlasStack); atlas.next();) {
        int endCopyRange = atlas->getFillBatchID();
        SkASSERT(endCopyRange > copyRangeIdx);

        sk_sp<GrRenderTargetContext> rtc = atlas->makeRenderTargetContext(onFlushRP);
        for (; copyRangeIdx < endCopyRange; ++copyRangeIdx) {
            const CopyPathRange& copyRange = fCopyPathRanges[copyRangeIdx];
            int endCopyInstance = baseCopyInstance + copyRange.fCount;
            if (rtc) {
                auto op = CopyAtlasOp::Make(rtc->surfPriv().getContext(), sk_ref_sp(this),
                                            copyRange.fSrcProxy, baseCopyInstance, endCopyInstance,
                                            atlas->drawBounds());
                rtc->addDrawOp(GrNoClip(), std::move(op));
            }
            baseCopyInstance = endCopyInstance;
        }
        out->push_back(std::move(rtc));
    }
    SkASSERT(fCopyPathRanges.count() == copyRangeIdx);
    SkASSERT(fNextCopyInstanceIdx == baseCopyInstance);
    SkASSERT(baseCopyInstance == fEndCopyInstance);

    // Render the coverage count atlas(es).
    for (GrCCAtlasStack::Iter atlas(fRenderedAtlasStack); atlas.next();) {
        // Copies will be finished by the time we get to rendering new atlases. See if we can
        // recycle any previous invalidated atlas textures instead of creating new ones.
        sk_sp<GrTexture> backingTexture;
        for (sk_sp<GrTexture>& texture : fRecyclableAtlasTextures) {
            if (texture && atlas->currentHeight() == texture->height() &&
                    atlas->currentWidth() == texture->width()) {
                backingTexture = skstd::exchange(texture, nullptr);
                break;
            }
        }

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

void GrCCPerFlushResourceSpecs::cancelCopies() {
    // Convert copies to cached draws.
    fNumCachedPaths += fNumCopiedPaths[kFillIdx] + fNumCopiedPaths[kStrokeIdx];
    fNumCopiedPaths[kFillIdx] = fNumCopiedPaths[kStrokeIdx] = 0;
    fCopyPathStats[kFillIdx] = fCopyPathStats[kStrokeIdx] = GrCCRenderedPathStats();
    fCopyAtlasSpecs = GrCCAtlas::Specs();
}
