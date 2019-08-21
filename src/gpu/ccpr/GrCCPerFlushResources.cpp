/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCPerFlushResources.h"

#include "include/private/GrRecordingContext.h"
#include "src/core/SkMakeUnique.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/ccpr/GrCCPathCache.h"
#include "src/gpu/ccpr/GrGSCoverageProcessor.h"
#include "src/gpu/ccpr/GrSampleMaskProcessor.h"
#include "src/gpu/ccpr/GrVSCoverageProcessor.h"
#include "src/gpu/geometry/GrShape.h"

using CoverageType = GrCCAtlas::CoverageType;
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

// Copies paths from a cached coverage count or msaa atlas into an 8-bit literal-coverage atlas.
class CopyAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(
            GrRecordingContext* context, sk_sp<const GrCCPerFlushResources> resources,
            sk_sp<GrTextureProxy> copyProxy, int baseInstance, int endInstance,
            const SkISize& drawBounds) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<CopyAtlasOp>(std::move(resources), std::move(copyProxy), baseInstance,
                                           endInstance, drawBounds);
    }

    const char* name() const override { return "CopyAtlasOp (CCPR)"; }

    void visitProxies(const VisitProxyFunc& fn) const override {
        fn(fSrcProxy.get(), GrMipMapped::kNo);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        SkASSERT(fSrcProxy);
        auto srcProxy = fSrcProxy.get();
        SkASSERT(srcProxy->isInstantiated());

        auto coverageMode = GrCCPathProcessor::GetCoverageMode(
                fResources->renderedPathCoverageType());
        GrCCPathProcessor pathProc(coverageMode, srcProxy->peekTexture(),
                                   srcProxy->textureSwizzle(), srcProxy->origin());

        GrPipeline pipeline(GrScissorTest::kDisabled, SkBlendMode::kSrc,
                            flushState->drawOpArgs().fOutputSwizzle);
        GrPipeline::FixedDynamicState dynamicState;
        dynamicState.fPrimitiveProcessorTextures = &srcProxy;

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
template<typename ProcessorType> class RenderAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(
            GrRecordingContext* context, sk_sp<const GrCCPerFlushResources> resources,
            FillBatchID fillBatchID, StrokeBatchID strokeBatchID, const SkISize& drawBounds) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        return pool->allocate<RenderAtlasOp>(
                std::move(resources), fillBatchID, strokeBatchID, drawBounds);
    }

    // GrDrawOp interface.
    const char* name() const override { return "RenderAtlasOp (CCPR)"; }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        ProcessorType proc;
        GrPipeline pipeline(GrScissorTest::kEnabled, SkBlendMode::kPlus,
                            flushState->drawOpArgs().fOutputSwizzle);
        fResources->filler().drawFills(flushState, &proc, pipeline, fFillBatchID, fDrawBounds);
        fResources->stroker().drawStrokes(flushState, &proc, fStrokeBatchID, fDrawBounds);
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

GrCCPerFlushResources::GrCCPerFlushResources(
        GrOnFlushResourceProvider* onFlushRP, CoverageType coverageType,
        const GrCCPerFlushResourceSpecs& specs)
        // Overallocate by one point so we can call Sk4f::Store at the final SkPoint in the array.
        // (See transform_path_pts below.)
        // FIXME: instead use built-in instructions to write only the first two lanes of an Sk4f.
        : fLocalDevPtsBuffer(SkTMax(specs.fRenderedPathStats[kFillIdx].fMaxPointsPerPath,
                                    specs.fRenderedPathStats[kStrokeIdx].fMaxPointsPerPath) + 1)
        , fFiller((CoverageType::kFP16_CoverageCount == coverageType)
                          ? GrCCFiller::Algorithm::kCoverageCount
                          : GrCCFiller::Algorithm::kStencilWindingCount,
                  specs.fNumRenderedPaths[kFillIdx] + specs.fNumClipPaths,
                  specs.fRenderedPathStats[kFillIdx].fNumTotalSkPoints,
                  specs.fRenderedPathStats[kFillIdx].fNumTotalSkVerbs,
                  specs.fRenderedPathStats[kFillIdx].fNumTotalConicWeights)
        , fStroker(specs.fNumRenderedPaths[kStrokeIdx],
                   specs.fRenderedPathStats[kStrokeIdx].fNumTotalSkPoints,
                   specs.fRenderedPathStats[kStrokeIdx].fNumTotalSkVerbs)
        , fCopyAtlasStack(CoverageType::kA8_LiteralCoverage, specs.fCopyAtlasSpecs,
                          onFlushRP->caps())
        , fRenderedAtlasStack(coverageType, specs.fRenderedAtlasSpecs, onFlushRP->caps())
        , fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
        , fInstanceBuffer(onFlushRP->makeBuffer(GrGpuBufferType::kVertex,
                                                inst_buffer_count(specs) * sizeof(PathInstance)))
        , fNextCopyInstanceIdx(0)
        , fNextPathInstanceIdx(
                specs.fNumCopiedPaths[kFillIdx] + specs.fNumCopiedPaths[kStrokeIdx]) {
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

    if (CoverageType::kA8_Multisample == coverageType) {
        int numRenderedPaths =
                specs.fNumRenderedPaths[kFillIdx] + specs.fNumRenderedPaths[kStrokeIdx] +
                specs.fNumClipPaths;
        fStencilResolveBuffer = onFlushRP->makeBuffer(
                GrGpuBufferType::kVertex,
                numRenderedPaths * sizeof(GrStencilAtlasOp::ResolveRectInstance));
        fStencilResolveInstanceData = static_cast<GrStencilAtlasOp::ResolveRectInstance*>(
                fStencilResolveBuffer->map());
        SkASSERT(fStencilResolveInstanceData);
        SkDEBUGCODE(fEndStencilResolveInstance = numRenderedPaths);
    }

    SkDEBUGCODE(fEndCopyInstance =
                        specs.fNumCopiedPaths[kFillIdx] + specs.fNumCopiedPaths[kStrokeIdx]);
    SkDEBUGCODE(fEndPathInstance = inst_buffer_count(specs));
}

void GrCCPerFlushResources::upgradeEntryToLiteralCoverageAtlas(
        GrCCPathCache* pathCache, GrOnFlushResourceProvider* onFlushRP, GrCCPathCacheEntry* entry,
        GrFillRule fillRule) {
    using ReleaseAtlasResult = GrCCPathCacheEntry::ReleaseAtlasResult;
    SkASSERT(this->isMapped());
    SkASSERT(fNextCopyInstanceIdx < fEndCopyInstance);

    const GrCCCachedAtlas* cachedAtlas = entry->cachedAtlas();
    SkASSERT(cachedAtlas);
    SkASSERT(cachedAtlas->getOnFlushProxy());

    if (CoverageType::kA8_LiteralCoverage == cachedAtlas->coverageType()) {
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

    this->recordCopyPathInstance(
            *entry, newAtlasOffset, fillRule, sk_ref_sp(cachedAtlas->getOnFlushProxy()));

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

void GrCCPerFlushResources::recordCopyPathInstance(
        const GrCCPathCacheEntry& entry, const SkIVector& newAtlasOffset, GrFillRule fillRule,
        sk_sp<GrTextureProxy> srcProxy) {
    SkASSERT(fNextCopyInstanceIdx < fEndCopyInstance);

    // Write the instance at the back of the array.
    int currentInstanceIdx = fNextCopyInstanceIdx++;
    constexpr uint64_t kWhite = (((uint64_t) SK_Half1) <<  0) |
                                (((uint64_t) SK_Half1) << 16) |
                                (((uint64_t) SK_Half1) << 32) |
                                (((uint64_t) SK_Half1) << 48);
    fPathInstanceData[currentInstanceIdx].set(entry, newAtlasOffset, kWhite, fillRule);

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

static bool transform_path_pts(
        const SkMatrix& m, const SkPath& path, const SkAutoSTArray<32, SkPoint>& outDevPts,
        GrOctoBounds* octoBounds) {
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

    const SkRect& devBounds = SkRect::MakeLTRB(
            topLeftPts[0].x(), topLeftPts[0].y(), bottomRightPts[0].x(), bottomRightPts[0].y());
    const SkRect& devBounds45 = SkRect::MakeLTRB(
            topLeftPts[1].x(), topLeftPts[1].y(), bottomRightPts[1].x(), bottomRightPts[1].y());

    octoBounds->set(devBounds, devBounds45);
    return true;
}

GrCCAtlas* GrCCPerFlushResources::renderShapeInAtlas(
        const SkIRect& clipIBounds, const SkMatrix& m, const GrShape& shape, float strokeDevWidth,
        GrOctoBounds* octoBounds, SkIRect* devIBounds, SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx < fEndPathInstance);

    SkPath path;
    shape.asPath(&path);
    if (path.isEmpty()) {
        SkDEBUGCODE(--fEndPathInstance);
        SkDEBUGCODE(--fEndStencilResolveInstance);
        return nullptr;
    }
    if (!transform_path_pts(m, path, fLocalDevPtsBuffer, octoBounds)) {
        // The transformed path had infinite or NaN bounds.
        SkDEBUGCODE(--fEndPathInstance);
        SkDEBUGCODE(--fEndStencilResolveInstance);
        return nullptr;
    }

    const SkStrokeRec& stroke = shape.style().strokeRec();
    if (!stroke.isFillStyle()) {
        float r = SkStrokeRec::GetInflationRadius(
                stroke.getJoin(), stroke.getMiter(), stroke.getCap(), strokeDevWidth);
        octoBounds->outset(r);
    }

    GrScissorTest enableScissorInAtlas;
    if (clipIBounds.contains(octoBounds->bounds())) {
        enableScissorInAtlas = GrScissorTest::kDisabled;
    } else if (octoBounds->clip(clipIBounds)) {
        enableScissorInAtlas = GrScissorTest::kEnabled;
    } else {
        // The clip and octo bounds do not intersect. Draw nothing.
        SkDEBUGCODE(--fEndPathInstance);
        SkDEBUGCODE(--fEndStencilResolveInstance);
        return nullptr;
    }
    octoBounds->roundOut(devIBounds);
    SkASSERT(clipIBounds.contains(*devIBounds));

    this->placeRenderedPathInAtlas(*devIBounds, enableScissorInAtlas, devToAtlasOffset);

    GrFillRule fillRule;
    if (stroke.isFillStyle()) {
        SkASSERT(0 == strokeDevWidth);
        fFiller.parseDeviceSpaceFill(path, fLocalDevPtsBuffer.begin(), enableScissorInAtlas,
                                     *devIBounds, *devToAtlasOffset);
        fillRule = GrFillRuleForSkPath(path);
    } else {
        // Stroke-and-fill is not yet supported.
        SkASSERT(SkStrokeRec::kStroke_Style == stroke.getStyle() || stroke.isHairlineStyle());
        SkASSERT(!stroke.isHairlineStyle() || 1 == strokeDevWidth);
        fStroker.parseDeviceSpaceStroke(
                path, fLocalDevPtsBuffer.begin(), stroke, strokeDevWidth, enableScissorInAtlas,
                *devIBounds, *devToAtlasOffset);
        fillRule = GrFillRule::kNonzero;
    }

    if (GrCCAtlas::CoverageType::kA8_Multisample == this->renderedPathCoverageType()) {
        this->recordStencilResolveInstance(*devIBounds, *devToAtlasOffset, fillRule);
    }

    return &fRenderedAtlasStack.current();
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
    if (GrCCAtlas::CoverageType::kA8_Multisample == this->renderedPathCoverageType()) {
        this->recordStencilResolveInstance(clippedPathIBounds, *devToAtlasOffset, fillRule);
    }

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
        retiredAtlas->setStrokeBatchID(fStroker.closeCurrentBatch());
        retiredAtlas->setEndStencilResolveInstance(fNextStencilResolveInstanceIdx);
    }
}

void GrCCPerFlushResources::recordStencilResolveInstance(
        const SkIRect& clippedPathIBounds, const SkIVector& devToAtlasOffset, GrFillRule fillRule) {
    SkASSERT(GrCCAtlas::CoverageType::kA8_Multisample == this->renderedPathCoverageType());
    SkASSERT(fNextStencilResolveInstanceIdx < fEndStencilResolveInstance);

    SkIRect atlasIBounds = clippedPathIBounds.makeOffset(
            devToAtlasOffset.x(), devToAtlasOffset.y());
    if (GrFillRule::kEvenOdd == fillRule) {
        // Make even/odd fills counterclockwise. The resolve draw uses two-sided stencil, with
        // "nonzero" settings in front and "even/odd" settings in back.
        std::swap(atlasIBounds.fLeft, atlasIBounds.fRight);
    }
    fStencilResolveInstanceData[fNextStencilResolveInstanceIdx++] = {
            (int16_t)atlasIBounds.left(), (int16_t)atlasIBounds.top(),
            (int16_t)atlasIBounds.right(), (int16_t)atlasIBounds.bottom()};
}

bool GrCCPerFlushResources::finalize(GrOnFlushResourceProvider* onFlushRP,
                                     SkTArray<std::unique_ptr<GrRenderTargetContext>>* out) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx == fEndPathInstance);
    SkASSERT(fNextCopyInstanceIdx == fEndCopyInstance);
    SkASSERT(GrCCAtlas::CoverageType::kA8_Multisample != this->renderedPathCoverageType() ||
             fNextStencilResolveInstanceIdx == fEndStencilResolveInstance);

    fInstanceBuffer->unmap();
    fPathInstanceData = nullptr;

    if (fStencilResolveBuffer) {
        fStencilResolveBuffer->unmap();
        fStencilResolveInstanceData = nullptr;
    }

    if (!fCopyAtlasStack.empty()) {
        fCopyAtlasStack.current().setFillBatchID(fCopyPathRanges.count());
        fCurrCopyAtlasRangesIdx = fCopyPathRanges.count();
    }
    if (!fRenderedAtlasStack.empty()) {
        fRenderedAtlasStack.current().setFillBatchID(fFiller.closeCurrentBatch());
        fRenderedAtlasStack.current().setStrokeBatchID(fStroker.closeCurrentBatch());
        fRenderedAtlasStack.current().setEndStencilResolveInstance(fNextStencilResolveInstanceIdx);
    }

    // Build the GPU buffers to render path coverage counts. (This must not happen until after the
    // final calls to fFiller/fStroker.closeCurrentBatch().)
    if (!fFiller.prepareToDraw(onFlushRP)) {
        return false;
    }
    if (!fStroker.prepareToDraw(onFlushRP)) {
        return false;
    }

    // Draw the copies from coverage count or msaa atlas(es) into 8-bit cached atlas(es).
    int copyRangeIdx = 0;
    int baseCopyInstance = 0;
    for (GrCCAtlasStack::Iter atlas(fCopyAtlasStack); atlas.next();) {
        int endCopyRange = atlas->getFillBatchID();
        SkASSERT(endCopyRange > copyRangeIdx);

        auto rtc = atlas->makeRenderTargetContext(onFlushRP);
        for (; copyRangeIdx < endCopyRange; ++copyRangeIdx) {
            const CopyPathRange& copyRange = fCopyPathRanges[copyRangeIdx];
            int endCopyInstance = baseCopyInstance + copyRange.fCount;
            if (rtc) {
                auto op = CopyAtlasOp::Make(
                        rtc->surfPriv().getContext(), sk_ref_sp(this), copyRange.fSrcProxy,
                        baseCopyInstance, endCopyInstance, atlas->drawBounds());
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
    int baseStencilResolveInstance = 0;
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
            std::unique_ptr<GrDrawOp> op;
            if (CoverageType::kA8_Multisample == fRenderedAtlasStack.coverageType()) {
                op = GrStencilAtlasOp::Make(
                        rtc->surfPriv().getContext(), sk_ref_sp(this), atlas->getFillBatchID(),
                        atlas->getStrokeBatchID(), baseStencilResolveInstance,
                        atlas->getEndStencilResolveInstance(), atlas->drawBounds());
            } else if (onFlushRP->caps()->shaderCaps()->geometryShaderSupport()) {
                op = RenderAtlasOp<GrGSCoverageProcessor>::Make(
                        rtc->surfPriv().getContext(), sk_ref_sp(this), atlas->getFillBatchID(),
                        atlas->getStrokeBatchID(), atlas->drawBounds());
            } else {
                op = RenderAtlasOp<GrVSCoverageProcessor>::Make(
                        rtc->surfPriv().getContext(), sk_ref_sp(this), atlas->getFillBatchID(),
                        atlas->getStrokeBatchID(), atlas->drawBounds());
            }
            rtc->addDrawOp(GrNoClip(), std::move(op));
            out->push_back(std::move(rtc));
        }

        SkASSERT(atlas->getEndStencilResolveInstance() >= baseStencilResolveInstance);
        baseStencilResolveInstance = atlas->getEndStencilResolveInstance();
    }
    SkASSERT(GrCCAtlas::CoverageType::kA8_Multisample != this->renderedPathCoverageType() ||
             baseStencilResolveInstance == fEndStencilResolveInstance);

    return true;
}

void GrCCPerFlushResourceSpecs::cancelCopies() {
    // Convert copies to cached draws.
    fNumCachedPaths += fNumCopiedPaths[kFillIdx] + fNumCopiedPaths[kStrokeIdx];
    fNumCopiedPaths[kFillIdx] = fNumCopiedPaths[kStrokeIdx] = 0;
    fCopyPathStats[kFillIdx] = fCopyPathStats[kStrokeIdx] = GrCCRenderedPathStats();
    fCopyAtlasSpecs = GrCCAtlas::Specs();
}
