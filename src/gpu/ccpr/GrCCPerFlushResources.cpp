/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCPerFlushResources.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ccpr/GrCCPathCache.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrFillRectOp.h"

#include <algorithm>

using CoverageType = GrCCAtlas::CoverageType;
using PathInstance = GrCCPathProcessor::Instance;

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

// Copies paths from a cached coverage count or msaa atlas into an 8-bit literal-coverage atlas.
class CopyAtlasOp : public AtlasOp {
public:
    DEFINE_OP_CLASS_ID

    static GrOp::Owner Make(
            GrRecordingContext* context, sk_sp<const GrCCPerFlushResources> resources,
            sk_sp<GrTextureProxy> copyProxy, int baseInstance, int endInstance,
            const SkISize& drawBounds) {
        return GrOp::Make<CopyAtlasOp>(
                context, std::move(resources), std::move(copyProxy), baseInstance,
                endInstance, drawBounds);
    }

    const char* name() const override { return "CopyAtlasOp (CCPR)"; }

    void visitProxies(const VisitProxyFunc& fn) const override {
        fn(fSrcProxy.get(), GrMipmapped::kNo);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        SkASSERT(fSrcProxy);
        SkASSERT(fSrcProxy->isInstantiated());

        GrColorType ct = GrCCAtlas::CoverageTypeToColorType(CoverageType::kA8_Multisample);
        GrSwizzle swizzle = flushState->caps().getReadSwizzle(fSrcProxy->backendFormat(), ct);
        GrCCPathProcessor pathProc(fSrcProxy->peekTexture(), swizzle, GrCCAtlas::kTextureOrigin);

        bool hasScissor = flushState->appliedClip() &&
                          flushState->appliedClip()->scissorState().enabled();
        GrPipeline pipeline(hasScissor ? GrScissorTest::kEnabled : GrScissorTest::kDisabled,
                            SkBlendMode::kSrc, flushState->drawOpArgs().writeView().swizzle());

        pathProc.drawPaths(flushState, pipeline, *fSrcProxy, *fResources, fBaseInstance,
                           fEndInstance, this->bounds());
    }

private:
    friend class ::GrOp; // for ctor

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

}  // namespace

static int inst_buffer_count(const GrCCPerFlushResourceSpecs& specs) {
    return specs.fNumCachedPaths +
           // Copies get two instances per draw: 1 copy + 1 draw.
           (specs.fNumCopiedPaths) * 2 +
           specs.fNumRenderedPaths;
           // No clips in instance buffers.
}

GrCCPerFlushResources::GrCCPerFlushResources(
        GrOnFlushResourceProvider* onFlushRP, CoverageType coverageType,
        const GrCCPerFlushResourceSpecs& specs)
        : fCopyAtlasStack(CoverageType::kA8_LiteralCoverage, specs.fCopyAtlasSpecs,
                          onFlushRP->caps())
        , fRenderedAtlasStack(coverageType, specs.fRenderedAtlasSpecs, onFlushRP->caps())
        , fIndexBuffer(GrCCPathProcessor::FindIndexBuffer(onFlushRP))
        , fVertexBuffer(GrCCPathProcessor::FindVertexBuffer(onFlushRP))
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
    fPathInstanceBuffer.resetAndMapBuffer(onFlushRP,
                                          inst_buffer_count(specs) * sizeof(PathInstance));
    if (!fPathInstanceBuffer.hasGpuBuffer()) {
        SkDebugf("WARNING: failed to allocate CCPR instance buffer. No paths will be drawn.\n");
        return;
    }

    SkDEBUGCODE(fEndCopyInstance = specs.fNumCopiedPaths);
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

void GrCCPerFlushResources::recordCopyPathInstance(
        const GrCCPathCacheEntry& entry, const SkIVector& newAtlasOffset, GrFillRule fillRule,
        sk_sp<GrTextureProxy> srcProxy) {
    SkASSERT(fNextCopyInstanceIdx < fEndCopyInstance);

    // Write the instance at the back of the array.
    int currentInstanceIdx = fNextCopyInstanceIdx++;
    fPathInstanceBuffer[currentInstanceIdx].set(entry, newAtlasOffset, SK_PMColor4fWHITE, fillRule);

    // Percolate the instance forward until it's contiguous with other instances that share the same
    // proxy.
    for (int i = fCopyPathRanges.count() - 1; i >= fCurrCopyAtlasRangesIdx; --i) {
        if (fCopyPathRanges[i].fSrcProxy == srcProxy) {
            ++fCopyPathRanges[i].fCount;
            return;
        }
        int rangeFirstInstanceIdx = currentInstanceIdx - fCopyPathRanges[i].fCount;
        std::swap(fPathInstanceBuffer[rangeFirstInstanceIdx],
                  fPathInstanceBuffer[currentInstanceIdx]);
        currentInstanceIdx = rangeFirstInstanceIdx;
    }

    // An instance with this particular proxy did not yet exist in the array. Add a range for it,
    // first moving any later ranges back to make space for it at fCurrCopyAtlasRangesIdx.
    fCopyPathRanges.push_back();
    std::move_backward(fCopyPathRanges.begin() + fCurrCopyAtlasRangesIdx,
                       fCopyPathRanges.end() - 1,
                       fCopyPathRanges.end());
    fCopyPathRanges[fCurrCopyAtlasRangesIdx] = {std::move(srcProxy), 1};
}

static bool calc_octo_bounds(const SkMatrix& m, const SkPath& path, GrOctoBounds* octoBounds) {
    const SkPoint* pts = SkPathPriv::PointData(path);
    int numPts = path.countPoints();
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

    for (int i = 1; i < numPts; ++i) {
        devPt = SkNx_fma(Y, Sk4f(pts[i].y()), T);
        devPt = SkNx_fma(X, Sk4f(pts[i].x()), devPt);
        topLeft = Sk4f::Min(topLeft, devPt);
        bottomRight = Sk4f::Max(bottomRight, devPt);
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
        GrOnFlushResourceProvider* onFlushRP, const SkIRect& clipIBounds, const SkMatrix& m,
        const GrStyledShape& shape, float strokeDevWidth, GrOctoBounds* octoBounds,
        SkIRect* devIBounds, SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextPathInstanceIdx < fEndPathInstance);
    SkASSERT(shape.style().strokeRec().isFillStyle());

    SkPath path;
    shape.asPath(&path);
    if (path.isEmpty()) {
        return nullptr;
    }
    if (!calc_octo_bounds(m, path, octoBounds)) {
        // The transformed path had infinite or NaN bounds.
        return nullptr;
    }

    GrScissorTest enableScissorInAtlas;
    if (clipIBounds.contains(octoBounds->bounds())) {
        enableScissorInAtlas = GrScissorTest::kDisabled;
    } else if (octoBounds->clip(clipIBounds)) {
        enableScissorInAtlas = GrScissorTest::kEnabled;
    } else {
        // The clip and octo bounds do not intersect. Draw nothing.
        return nullptr;
    }
    octoBounds->roundOut(devIBounds);
    SkASSERT(clipIBounds.contains(*devIBounds));

    this->placeRenderedPathInAtlas(onFlushRP, *devIBounds, enableScissorInAtlas, devToAtlasOffset);

    SkMatrix atlasMatrix = m;
    atlasMatrix.postTranslate(devToAtlasOffset->fX, devToAtlasOffset->fY);
    this->enqueueRenderedPath(path, GrFillRuleForSkPath(path), *devIBounds, atlasMatrix,
                              enableScissorInAtlas, *devToAtlasOffset);

    return &fRenderedAtlasStack.current();
}

const GrCCAtlas* GrCCPerFlushResources::renderDeviceSpacePathInAtlas(
        GrOnFlushResourceProvider* onFlushRP, const SkIRect& clipIBounds, const SkPath& devPath,
        const SkIRect& devPathIBounds, GrFillRule fillRule, SkIVector* devToAtlasOffset) {
    SkASSERT(this->isMapped());

    if (devPath.isEmpty()) {
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
        return nullptr;
    }

    this->placeRenderedPathInAtlas(onFlushRP, clippedPathIBounds, enableScissorInAtlas,
                                   devToAtlasOffset);

    SkMatrix atlasMatrix = SkMatrix::Translate(devToAtlasOffset->fX, devToAtlasOffset->fY);
    this->enqueueRenderedPath(devPath, fillRule, clippedPathIBounds, atlasMatrix,
                              enableScissorInAtlas, *devToAtlasOffset);

    return &fRenderedAtlasStack.current();
}

void GrCCPerFlushResources::placeRenderedPathInAtlas(
        GrOnFlushResourceProvider* onFlushRP, const SkIRect& clippedPathIBounds,
        GrScissorTest scissorTest, SkIVector* devToAtlasOffset) {
    if (GrCCAtlas* retiredAtlas =
                fRenderedAtlasStack.addRect(clippedPathIBounds, devToAtlasOffset)) {
        // We did not fit in the previous coverage count atlas and it was retired. Render the
        // retired atlas.
        this->flushRenderPaths(onFlushRP, retiredAtlas);
    }
}

void GrCCPerFlushResources::enqueueRenderedPath(const SkPath& path, GrFillRule fillRule,
                                                const SkIRect& clippedDevIBounds,
                                                const SkMatrix& pathToAtlasMatrix,
                                                GrScissorTest enableScissorInAtlas,
                                                SkIVector devToAtlasOffset) {
    SkPath* atlasPath;
    if (enableScissorInAtlas == GrScissorTest::kDisabled) {
        atlasPath = &fAtlasPaths[(int)fillRule].fUberPath;
    } else {
        auto& [scissoredPath, scissor] = fAtlasPaths[(int)fillRule].fScissoredPaths.push_back();
        scissor = clippedDevIBounds.makeOffset(devToAtlasOffset);
        atlasPath = &scissoredPath;
    }
    auto origin = clippedDevIBounds.topLeft() + devToAtlasOffset;
    atlasPath->moveTo(origin.fX, origin.fY);  // Implicit moveTo(0,0).
    atlasPath->addPath(path, pathToAtlasMatrix);
}

static void draw_stencil_to_coverage(GrOnFlushResourceProvider* onFlushRP,
                                     GrSurfaceDrawContext* surfaceDrawContext, SkRect&& rect) {
    auto aaType = GrAAType::kMSAA;
    auto fillRectFlags = GrSimpleMeshDrawOpHelper::InputFlags::kNone;

    // This will be the final op in the surfaceDrawContext. So if Ganesh is planning to discard the
    // stencil values anyway, then we might not actually need to reset the stencil values back to 0.
    bool mustResetStencil = !onFlushRP->caps()->discardStencilValuesAfterRenderPass();

    if (surfaceDrawContext->numSamples() == 1) {
        // We are mixed sampled. We need to either enable conservative raster (preferred) or disable
        // MSAA in order to avoid double blend artifacts. (Even if we disable MSAA for the cover
        // geometry, the stencil test is still multisampled and will still produce smooth results.)
        if (onFlushRP->caps()->conservativeRasterSupport()) {
            fillRectFlags |= GrSimpleMeshDrawOpHelper::InputFlags::kConservativeRaster;
        } else {
            aaType = GrAAType::kNone;
        }
        mustResetStencil = true;
    }

    const GrUserStencilSettings* stencil;
    if (mustResetStencil) {
        constexpr static GrUserStencilSettings kTestAndResetStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>());

        // Outset the cover rect in case there are T-junctions in the path bounds.
        rect.outset(1, 1);
        stencil = &kTestAndResetStencil;
    } else {
        constexpr static GrUserStencilSettings kTestStencil(
            GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kKeep,
                GrUserStencilOp::kKeep,
                0xffff>());

        stencil = &kTestStencil;
    }

    GrPaint paint;
    paint.setColor4f(SK_PMColor4fWHITE);
    GrQuad coverQuad(rect);
    DrawQuad drawQuad{coverQuad, coverQuad, GrQuadAAFlags::kAll};
    auto coverOp = GrFillRectOp::Make(surfaceDrawContext->recordingContext(), std::move(paint),
                                      aaType, &drawQuad, stencil, fillRectFlags);
    surfaceDrawContext->addDrawOp(nullptr, std::move(coverOp));
}

void GrCCPerFlushResources::flushRenderPaths(GrOnFlushResourceProvider* onFlushRP,
                                             GrCCAtlas* atlas) {
    auto surfaceDrawContext = atlas->instantiate(onFlushRP);

    for (int i = 0; i < (int)SK_ARRAY_COUNT(fAtlasPaths); ++i) {
        SkPathFillType fillType = (i == (int)GrFillRule::kNonzero) ? SkPathFillType::kWinding
                                                                   : SkPathFillType::kEvenOdd;
        SkPath& uberPath = fAtlasPaths[i].fUberPath;
        if (!uberPath.isEmpty()) {
            uberPath.setIsVolatile(true);
            uberPath.setFillType(fillType);
            surfaceDrawContext->stencilPath(nullptr, GrAA::kYes, SkMatrix::I(), uberPath);
            uberPath.reset();
        }
        for (auto& [scissoredPath, scissor] : fAtlasPaths[i].fScissoredPaths) {
            GrFixedClip fixedClip(
                    surfaceDrawContext->asRenderTargetProxy()->backingStoreDimensions(), scissor);
            scissoredPath.setIsVolatile(true);
            scissoredPath.setFillType(fillType);
            surfaceDrawContext->stencilPath(&fixedClip, GrAA::kYes, SkMatrix::I(), scissoredPath);
        }
        fAtlasPaths[i].fScissoredPaths.reset();
    }

    draw_stencil_to_coverage(onFlushRP, surfaceDrawContext.get(),
                             SkRect::MakeSize(SkSize::Make(atlas->drawBounds())));

    if (surfaceDrawContext->asSurfaceProxy()->requiresManualMSAAResolve()) {
        onFlushRP->addTextureResolveTask(sk_ref_sp(surfaceDrawContext->asTextureProxy()),
                                         GrSurfaceProxy::ResolveFlags::kMSAA);
    }
}

bool GrCCPerFlushResources::finalize(GrOnFlushResourceProvider* onFlushRP) {
    SkASSERT(this->isMapped());
    SkASSERT(fNextCopyInstanceIdx == fEndCopyInstance);

    fPathInstanceBuffer.unmapBuffer();

    if (!fCopyAtlasStack.empty()) {
        fCopyAtlasStack.current().setFillBatchID(fCopyPathRanges.count());
        fCurrCopyAtlasRangesIdx = fCopyPathRanges.count();
    }
    if (!fRenderedAtlasStack.empty()) {
        this->flushRenderPaths(onFlushRP, &fRenderedAtlasStack.current());
    }
#ifdef SK_DEBUG
    // These paths should have been rendered and reset to empty by this point.
    for (const auto& [uberPath, scissoredPaths] : fAtlasPaths) {
        SkASSERT(uberPath.isEmpty());
        SkASSERT(scissoredPaths.empty());
    }
#endif

    // Draw the copies from coverage count or msaa atlas(es) into 8-bit cached atlas(es).
    int copyRangeIdx = 0;
    int baseCopyInstance = 0;
    for (GrCCAtlas& atlas : fCopyAtlasStack.atlases()) {
        int endCopyRange = atlas.getFillBatchID();
        SkASSERT(endCopyRange > copyRangeIdx);

        auto rtc = atlas.instantiate(onFlushRP);
        for (; copyRangeIdx < endCopyRange; ++copyRangeIdx) {
            const CopyPathRange& copyRange = fCopyPathRanges[copyRangeIdx];
            int endCopyInstance = baseCopyInstance + copyRange.fCount;
            if (rtc) {
                auto op = CopyAtlasOp::Make(rtc->recordingContext(), sk_ref_sp(this),
                                            copyRange.fSrcProxy, baseCopyInstance, endCopyInstance,
                                            atlas.drawBounds());
                rtc->addDrawOp(nullptr, std::move(op));
            }
            baseCopyInstance = endCopyInstance;
        }
    }
    SkASSERT(fCopyPathRanges.count() == copyRangeIdx);
    SkASSERT(fNextCopyInstanceIdx == baseCopyInstance);
    SkASSERT(baseCopyInstance == fEndCopyInstance);

    return true;
}

void GrCCPerFlushResourceSpecs::cancelCopies() {
    // Convert copies to cached draws.
    fNumCachedPaths += fNumCopiedPaths;
    fNumCopiedPaths = 0;
    fCopyPathStats = GrCCRenderedPathStats();
    fCopyAtlasSpecs = GrCCAtlas::Specs();
}
