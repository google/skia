/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCDrawPathsOp.h"

#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "ccpr/GrCCPerFlushResources.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"

static bool has_coord_transforms(const GrPaint& paint) {
    GrFragmentProcessor::Iter iter(paint);
    while (const GrFragmentProcessor* fp = iter.next()) {
        if (!fp->coordTransforms().empty()) {
            return true;
        }
    }
    return false;
}

std::unique_ptr<GrCCDrawPathsOp> GrCCDrawPathsOp::Make(GrContext*, const SkIRect& clipIBounds,
                                                       const SkMatrix& m, const GrShape& shape,
                                                       const SkRect& devBounds, GrPaint&& paint) {
    bool canStashPathMask = true;
    SkIRect looseClippedIBounds;
    devBounds.roundOut(&looseClippedIBounds);  // GrCCPathParser might find slightly tighter bounds.
    if (!clipIBounds.contains(looseClippedIBounds)) {
        canStashPathMask = false;
        if (!looseClippedIBounds.intersect(clipIBounds)) {
            return nullptr;
        }
    }
    return std::unique_ptr<GrCCDrawPathsOp>(new GrCCDrawPathsOp(looseClippedIBounds, m, shape,
                                                                canStashPathMask, devBounds,
                                                                std::move(paint)));
}

GrCCDrawPathsOp::GrCCDrawPathsOp(const SkIRect& looseClippedIBounds, const SkMatrix& m,
                                 const GrShape& shape, bool canStashPathMask,
                                 const SkRect& devBounds, GrPaint&& paint)
        : GrDrawOp(ClassID())
        , fViewMatrixIfUsingLocalCoords(has_coord_transforms(paint) ? m : SkMatrix::I())
        , fSRGBFlags(GrPipeline::SRGBFlagsFromPaint(paint))
        , fDraws({looseClippedIBounds, m, shape, paint.getColor(), nullptr, nullptr, {0, 0},
                  canStashPathMask, nullptr})
        , fProcessors(std::move(paint)) {  // Paint must be moved after fetching its color above.
    SkDEBUGCODE(fBaseInstance = -1);
    // FIXME: intersect with clip bounds to (hopefully) improve batching.
    // (This is nontrivial due to assumptions in generating the octagon cover geometry.)
    this->setBounds(devBounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);
}

GrCCDrawPathsOp::~GrCCDrawPathsOp() {
    if (fOwningPerOpListPaths) {
        // Remove CCPR's dangling pointer to this Op before deleting it.
        fOwningPerOpListPaths->fDrawOps.remove(this);
    }
}

GrCCDrawPathsOp::SingleDraw::~SingleDraw() {
    if (fCacheEntry) {
        // All currFlushAtlas references must be reset back to null before the flush is finished.
        fCacheEntry->setCurrFlushAtlas(nullptr);
    }
}

GrDrawOp::RequiresDstTexture GrCCDrawPathsOp::finalize(const GrCaps& caps,
                                                       const GrAppliedClip* clip,
                                                       GrPixelConfigIsClamped dstIsClamped) {
    SkASSERT(1 == fNumDraws);  // There should only be one single path draw in this Op right now.
    GrProcessorSet::Analysis analysis =
            fProcessors.finalize(fDraws.head().fColor, GrProcessorAnalysisCoverage::kSingleChannel,
                                 clip, false, caps, dstIsClamped, &fDraws.head().fColor);
    return RequiresDstTexture(analysis.requiresDstTexture());
}

bool GrCCDrawPathsOp::onCombineIfPossible(GrOp* op, const GrCaps&) {
    GrCCDrawPathsOp* that = op->cast<GrCCDrawPathsOp>();
    SkASSERT(fOwningPerOpListPaths);
    SkASSERT(fNumDraws);
    SkASSERT(!that->fOwningPerOpListPaths || that->fOwningPerOpListPaths == fOwningPerOpListPaths);
    SkASSERT(that->fNumDraws);

    if (fSRGBFlags != that->fSRGBFlags || fProcessors != that->fProcessors ||
        fViewMatrixIfUsingLocalCoords != that->fViewMatrixIfUsingLocalCoords) {
        return false;
    }

    fDraws.append(std::move(that->fDraws), &fOwningPerOpListPaths->fAllocator);
    this->joinBounds(*that);

    SkDEBUGCODE(fNumDraws += that->fNumDraws);
    SkDEBUGCODE(that->fNumDraws = 0);
    return true;
}

void GrCCDrawPathsOp::wasRecorded(GrCCPerOpListPaths* owningPerOpListPaths) {
    SkASSERT(1 == fNumDraws);
    SkASSERT(!fOwningPerOpListPaths);
    owningPerOpListPaths->fDrawOps.addToTail(this);
    fOwningPerOpListPaths = owningPerOpListPaths;
}

void GrCCDrawPathsOp::accountForOwnPaths(GrCCPathCache* pathCache,
                                         GrOnFlushResourceProvider* onFlushRP,
                                         const GrUniqueKey& stashedAtlasKey,
                                         GrCCPerFlushResourceSpecs* specs) {
    using CreateIfAbsent = GrCCPathCache::CreateIfAbsent;
    using MaskTransform = GrCCPathCache::MaskTransform;

    for (SingleDraw& draw : fDraws) {
        SkASSERT(!draw.fCacheEntry);

        SkPath path;
        draw.fShape.asPath(&path);

        MaskTransform m(draw.fMatrix, &draw.fCachedMaskShift);
        draw.fCacheEntry = pathCache->find(draw.fShape, m, CreateIfAbsent(draw.fCanStashPathMask));
        if (auto cacheEntry = draw.fCacheEntry.get()) {
            SkASSERT(!cacheEntry->currFlushAtlas());  // Shouldn't be set until setupResources().
            if (cacheEntry->atlasKey().isValid()) {
                // Does the path already exist in a cached atlas?
                if (cacheEntry->hasCachedAtlas() &&
                    (draw.fCachedAtlasProxy = onFlushRP->findOrCreateProxyByUniqueKey(
                                                     cacheEntry->atlasKey(),
                                                     GrCCAtlas::kTextureOrigin))) {
                    ++specs->fNumCachedPaths;
                    continue;
                }

                // Does the path exist in the atlas that we stashed away from last flush? If so we
                // can copy it into a new 8-bit atlas and keep it in the resource cache.
                if (stashedAtlasKey.isValid() && stashedAtlasKey == cacheEntry->atlasKey()) {
                    SkASSERT(!cacheEntry->hasCachedAtlas());
                    ++specs->fNumCopiedPaths;
                    specs->fCopyPathStats.statPath(path);
                    specs->fCopyAtlasSpecs.accountForSpace(cacheEntry->width(),
                                                           cacheEntry->height());
                    continue;
                }

                // Whatever atlas the path used to reside in, it no longer exists.
                cacheEntry->resetAtlasKeyAndInfo();
            }

            if (!draw.fCanStashPathMask) {
                // No point in keeping this cache entry around anymore if we aren't going to try and
                // stash the the rendered path mask after flush.
                draw.fCacheEntry = nullptr;
                pathCache->evict(cacheEntry);
            }
        }

        ++specs->fNumRenderedPaths;
        specs->fRenderedPathStats.statPath(path);
        specs->fRenderedAtlasSpecs.accountForSpace(draw.fLooseClippedIBounds.width(),
                                                   draw.fLooseClippedIBounds.height());
    }
}

void GrCCDrawPathsOp::setupResources(GrOnFlushResourceProvider* onFlushRP,
                                     GrCCPerFlushResources* resources, DoCopiesToCache doCopies) {
    using DoEvenOddFill = GrCCPathProcessor::DoEvenOddFill;
    SkASSERT(fNumDraws > 0);
    SkASSERT(-1 == fBaseInstance);
    fBaseInstance = resources->nextPathInstanceIdx();

    for (SingleDraw& draw : fDraws) {
        SkPath path;
        draw.fShape.asPath(&path);

        auto doEvenOddFill = DoEvenOddFill(SkPath::kEvenOdd_FillType == path.getFillType());
        SkASSERT(SkPath::kEvenOdd_FillType == path.getFillType() ||
                 SkPath::kWinding_FillType == path.getFillType());

        if (auto cacheEntry = draw.fCacheEntry.get()) {
            // Does the path already exist in a cached atlas texture?
            if (auto proxy = draw.fCachedAtlasProxy.get()) {
                SkASSERT(!cacheEntry->currFlushAtlas());
                this->recordInstance(proxy, resources->nextPathInstanceIdx());
                resources->appendDrawPathInstance().set(*cacheEntry, draw.fCachedMaskShift,
                                                        draw.fColor);
                continue;
            }

            // Have we already encountered this path during the flush? (i.e. was the same SkPath
            // drawn more than once during the same flush, with a compatible matrix?)
            if (auto atlas = cacheEntry->currFlushAtlas()) {
                this->recordInstance(atlas->textureProxy(), resources->nextPathInstanceIdx());
                resources->appendDrawPathInstance().set(
                        *cacheEntry, draw.fCachedMaskShift, draw.fColor,
                        cacheEntry->hasCachedAtlas() ? DoEvenOddFill::kNo : doEvenOddFill);
                continue;
            }

            // If the cache entry still has a valid atlas key at this point, it means the path
            // exists in the atlas that we stashed away from last flush. Copy it into a permanent
            // 8-bit atlas in the resource cache.
            if (DoCopiesToCache::kYes == doCopies && cacheEntry->atlasKey().isValid()) {
                SkIVector newOffset;
                GrCCAtlas* atlas =
                        resources->copyPathToCachedAtlas(*cacheEntry, doEvenOddFill, &newOffset);
                cacheEntry->updateToCachedAtlas(atlas->getOrAssignUniqueKey(onFlushRP),
                                                newOffset, atlas->refOrMakeCachedAtlasInfo());
                this->recordInstance(atlas->textureProxy(), resources->nextPathInstanceIdx());
                resources->appendDrawPathInstance().set(*cacheEntry, draw.fCachedMaskShift,
                                                        draw.fColor);
                // Remember this atlas in case we encounter the path again during the same flush.
                cacheEntry->setCurrFlushAtlas(atlas);
                continue;
            }
        }

        // Render the raw path into a coverage count atlas. renderPathInAtlas() gives us two tight
        // bounding boxes: One in device space, as well as a second one rotated an additional 45
        // degrees. The path vertex shader uses these two bounding boxes to generate an octagon that
        // circumscribes the path.
        SkASSERT(!draw.fCachedAtlasProxy);
        SkRect devBounds, devBounds45;
        SkIRect devIBounds;
        SkIVector devToAtlasOffset;
        if (auto atlas = resources->renderPathInAtlas(draw.fLooseClippedIBounds, draw.fMatrix, path,
                                                      &devBounds, &devBounds45, &devIBounds,
                                                      &devToAtlasOffset)) {
            this->recordInstance(atlas->textureProxy(), resources->nextPathInstanceIdx());
            resources->appendDrawPathInstance().set(devBounds, devBounds45, devToAtlasOffset,
                                                    draw.fColor, doEvenOddFill);
            if (draw.fCacheEntry && draw.fCanStashPathMask &&
                resources->nextAtlasToStash() == atlas) {
                const GrUniqueKey& atlasKey =
                        resources->nextAtlasToStash()->getOrAssignUniqueKey(onFlushRP);
                draw.fCacheEntry->initAsStashedAtlas(atlasKey, devToAtlasOffset, devBounds,
                                                     devBounds45, devIBounds,
                                                     draw.fCachedMaskShift);
                // Remember this atlas in case we encounter the path again during the same flush.
                draw.fCacheEntry->setCurrFlushAtlas(atlas);
            }
            continue;
        }
    }

    if (!fInstanceRanges.empty()) {
        fInstanceRanges.back().fEndInstanceIdx = resources->nextPathInstanceIdx();
    }
}

inline void GrCCDrawPathsOp::recordInstance(const GrTextureProxy* atlasProxy, int instanceIdx) {
    if (fInstanceRanges.empty()) {
        fInstanceRanges.push_back({atlasProxy, instanceIdx});
        return;
    }
    if (fInstanceRanges.back().fAtlasProxy != atlasProxy) {
        fInstanceRanges.back().fEndInstanceIdx = instanceIdx;
        fInstanceRanges.push_back({atlasProxy, instanceIdx});
        return;
    }
}

void GrCCDrawPathsOp::onExecute(GrOpFlushState* flushState) {
    SkASSERT(fOwningPerOpListPaths);

    const GrCCPerFlushResources* resources = fOwningPerOpListPaths->fFlushResources.get();
    if (!resources) {
        return;  // Setup failed.
    }

    GrPipeline::InitArgs initArgs;
    initArgs.fFlags = fSRGBFlags;
    initArgs.fProxy = flushState->drawOpArgs().fProxy;
    initArgs.fCaps = &flushState->caps();
    initArgs.fResourceProvider = flushState->resourceProvider();
    initArgs.fDstProxy = flushState->drawOpArgs().fDstProxy;
    GrPipeline pipeline(initArgs, std::move(fProcessors), flushState->detachAppliedClip());

    int baseInstance = fBaseInstance;
    SkASSERT(baseInstance >= 0);  // Make sure setupResources() has been called.

    for (const InstanceRange& range : fInstanceRanges) {
        SkASSERT(range.fEndInstanceIdx > baseInstance);

        GrCCPathProcessor pathProc(flushState->resourceProvider(), sk_ref_sp(range.fAtlasProxy),
                                   fViewMatrixIfUsingLocalCoords);
        pathProc.drawPaths(flushState, pipeline, *resources, baseInstance, range.fEndInstanceIdx,
                           this->bounds());

        baseInstance = range.fEndInstanceIdx;
    }
}
