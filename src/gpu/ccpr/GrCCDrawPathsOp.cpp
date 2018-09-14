/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCDrawPathsOp.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrMemoryPool.h"
#include "GrOpFlushState.h"
#include "ccpr/GrCCPathCache.h"
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

static int64_t area(const SkIRect& r) {
    return sk_64_mul(r.height(), r.width());
}

std::unique_ptr<GrCCDrawPathsOp> GrCCDrawPathsOp::Make(
        GrContext* context, const SkIRect& clipIBounds, const SkMatrix& m, const GrShape& shape,
        GrPaint&& paint) {
    SkRect conservativeDevBounds;
    m.mapRect(&conservativeDevBounds, shape.bounds());

    const SkStrokeRec& stroke = shape.style().strokeRec();
    float strokeDevWidth = 0;
    float conservativeInflationRadius = 0;
    if (!stroke.isFillStyle()) {
        strokeDevWidth = GrCoverageCountingPathRenderer::GetStrokeDevWidth(
                m, stroke, &conservativeInflationRadius);
        conservativeDevBounds.outset(conservativeInflationRadius, conservativeInflationRadius);
    }

    std::unique_ptr<GrCCDrawPathsOp> op;
    float conservativeSize = SkTMax(conservativeDevBounds.height(), conservativeDevBounds.width());
    if (conservativeSize > GrCoverageCountingPathRenderer::kPathCropThreshold) {
        // The path is too large. Crop it or analytic AA can run out of fp32 precision.
        SkPath croppedDevPath;
        shape.asPath(&croppedDevPath);
        croppedDevPath.transform(m, &croppedDevPath);

        SkIRect cropBox = clipIBounds;
        GrShape croppedDevShape;
        if (stroke.isFillStyle()) {
            GrCoverageCountingPathRenderer::CropPath(croppedDevPath, cropBox, &croppedDevPath);
            croppedDevShape = GrShape(croppedDevPath);
            conservativeDevBounds = croppedDevShape.bounds();
        } else {
            int r = SkScalarCeilToInt(conservativeInflationRadius);
            cropBox.outset(r, r);
            GrCoverageCountingPathRenderer::CropPath(croppedDevPath, cropBox, &croppedDevPath);
            SkStrokeRec devStroke = stroke;
            devStroke.setStrokeStyle(strokeDevWidth);
            croppedDevShape = GrShape(croppedDevPath, GrStyle(devStroke, nullptr));
            conservativeDevBounds = croppedDevPath.getBounds();
            conservativeDevBounds.outset(conservativeInflationRadius, conservativeInflationRadius);
        }

        // FIXME: This breaks local coords: http://skbug.com/8003
        return InternalMake(context, clipIBounds, SkMatrix::I(), croppedDevShape, strokeDevWidth,
                            conservativeDevBounds, std::move(paint));
    }

    return InternalMake(context, clipIBounds, m, shape, strokeDevWidth, conservativeDevBounds,
                        std::move(paint));
}

std::unique_ptr<GrCCDrawPathsOp> GrCCDrawPathsOp::InternalMake(
        GrContext* context, const SkIRect& clipIBounds, const SkMatrix& m, const GrShape& shape,
        float strokeDevWidth, const SkRect& conservativeDevBounds, GrPaint&& paint) {
    // The path itself should have been cropped if larger than kPathCropThreshold. If it had a
    // stroke, that would have further inflated its draw bounds.
    SkASSERT(SkTMax(conservativeDevBounds.height(), conservativeDevBounds.width()) <
             GrCoverageCountingPathRenderer::kPathCropThreshold +
             GrCoverageCountingPathRenderer::kMaxBoundsInflationFromStroke*2 + 1);

    SkIRect shapeConservativeIBounds;
    conservativeDevBounds.roundOut(&shapeConservativeIBounds);

    SkIRect maskDevIBounds;
    Visibility maskVisibility;
    if (clipIBounds.contains(shapeConservativeIBounds)) {
        maskDevIBounds = shapeConservativeIBounds;
        maskVisibility = Visibility::kComplete;
    } else {
        if (!maskDevIBounds.intersect(clipIBounds, shapeConservativeIBounds)) {
            return nullptr;
        }
        int64_t unclippedArea = area(shapeConservativeIBounds);
        int64_t clippedArea = area(maskDevIBounds);
        maskVisibility = (clippedArea >= unclippedArea/2 || unclippedArea < 100*100)
                ? Visibility::kMostlyComplete  // i.e., visible enough to justify rendering the
                                               // whole thing if we think we can cache it.
                : Visibility::kPartial;
    }

    GrOpMemoryPool* pool = context->contextPriv().opMemoryPool();

    return pool->allocate<GrCCDrawPathsOp>(m, shape, strokeDevWidth, shapeConservativeIBounds,
                                           maskDevIBounds, maskVisibility, conservativeDevBounds,
                                           std::move(paint));
}

GrCCDrawPathsOp::GrCCDrawPathsOp(const SkMatrix& m, const GrShape& shape, float strokeDevWidth,
                                 const SkIRect& shapeConservativeIBounds,
                                 const SkIRect& maskDevIBounds, Visibility maskVisibility,
                                 const SkRect& conservativeDevBounds, GrPaint&& paint)
        : GrDrawOp(ClassID())
        , fViewMatrixIfUsingLocalCoords(has_coord_transforms(paint) ? m : SkMatrix::I())
        , fDraws(m, shape, strokeDevWidth, shapeConservativeIBounds, maskDevIBounds, maskVisibility,
                 paint.getColor())
        , fProcessors(std::move(paint)) {  // Paint must be moved after fetching its color above.
    SkDEBUGCODE(fBaseInstance = -1);
    // FIXME: intersect with clip bounds to (hopefully) improve batching.
    // (This is nontrivial due to assumptions in generating the octagon cover geometry.)
    this->setBounds(conservativeDevBounds, GrOp::HasAABloat::kYes, GrOp::IsZeroArea::kNo);
}

GrCCDrawPathsOp::~GrCCDrawPathsOp() {
    if (fOwningPerOpListPaths) {
        // Remove the list's dangling pointer to this Op before deleting it.
        fOwningPerOpListPaths->fDrawOps.remove(this);
    }
}

GrCCDrawPathsOp::SingleDraw::SingleDraw(const SkMatrix& m, const GrShape& shape,
                                        float strokeDevWidth,
                                        const SkIRect& shapeConservativeIBounds,
                                        const SkIRect& maskDevIBounds, Visibility maskVisibility,
                                        GrColor color)
        : fMatrix(m)
        , fShape(shape)
        , fStrokeDevWidth(strokeDevWidth)
        , fShapeConservativeIBounds(shapeConservativeIBounds)
        , fMaskDevIBounds(maskDevIBounds)
        , fMaskVisibility(maskVisibility)
        , fColor(color) {
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    if (fShape.hasUnstyledKey()) {
        // On AOSP we round view matrix translates to integer values for cachable paths. We do this
        // to match HWUI's cache hit ratio, which doesn't consider the matrix when caching paths.
        fMatrix.setTranslateX(SkScalarRoundToScalar(fMatrix.getTranslateX()));
        fMatrix.setTranslateY(SkScalarRoundToScalar(fMatrix.getTranslateY()));
    }
#endif
}

GrCCDrawPathsOp::SingleDraw::~SingleDraw() {
    if (fCacheEntry) {
        // All currFlushAtlas references must be reset back to null before the flush is finished.
        fCacheEntry->setCurrFlushAtlas(nullptr);
    }
}

GrDrawOp::RequiresDstTexture GrCCDrawPathsOp::finalize(const GrCaps& caps,
                                                       const GrAppliedClip* clip) {
    SkASSERT(1 == fNumDraws);  // There should only be one single path draw in this Op right now.
    SingleDraw* draw = &fDraws.head();

    const GrProcessorSet::Analysis& analysis = fProcessors.finalize(
            draw->fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip, false, caps,
            &draw->fColor);

    // Lines start looking jagged when they get thinner than 1px. For thin strokes it looks better
    // if we can convert them to hairline (i.e., inflate the stroke width to 1px), and instead
    // reduce the opacity to create the illusion of thin-ness. This strategy also helps reduce
    // artifacts from coverage dilation when there are self intersections.
    if (analysis.isCompatibleWithCoverageAsAlpha() &&
            !draw->fShape.style().strokeRec().isFillStyle() && draw->fStrokeDevWidth < 1) {
        // Modifying the shape affects its cache key. The draw can't have a cache entry yet or else
        // our next step would invalidate it.
        SkASSERT(!draw->fCacheEntry);
        SkASSERT(SkStrokeRec::kStroke_Style == draw->fShape.style().strokeRec().getStyle());

        SkPath path;
        draw->fShape.asPath(&path);

        // Create a hairline version of our stroke.
        SkStrokeRec hairlineStroke = draw->fShape.style().strokeRec();
        hairlineStroke.setStrokeStyle(0);

        // How transparent does a 1px stroke have to be in order to appear as thin as the real one?
        GrColor coverageAsAlpha = GrColorPackA4(SkScalarFloorToInt(draw->fStrokeDevWidth * 255));

        draw->fShape = GrShape(path, GrStyle(hairlineStroke, nullptr));
        draw->fStrokeDevWidth = 1;
        // fShapeConservativeIBounds already accounted for this possibility of inflating the stroke.
        draw->fColor = GrColorMul(draw->fColor, coverageAsAlpha);
    }

    return RequiresDstTexture(analysis.requiresDstTexture());
}

GrOp::CombineResult GrCCDrawPathsOp::onCombineIfPossible(GrOp* op, const GrCaps&) {
    GrCCDrawPathsOp* that = op->cast<GrCCDrawPathsOp>();
    SkASSERT(fOwningPerOpListPaths);
    SkASSERT(fNumDraws);
    SkASSERT(!that->fOwningPerOpListPaths || that->fOwningPerOpListPaths == fOwningPerOpListPaths);
    SkASSERT(that->fNumDraws);

    if (fProcessors != that->fProcessors ||
        fViewMatrixIfUsingLocalCoords != that->fViewMatrixIfUsingLocalCoords) {
        return CombineResult::kCannotCombine;
    }

    fDraws.append(std::move(that->fDraws), &fOwningPerOpListPaths->fAllocator);
    this->joinBounds(*that);

    SkDEBUGCODE(fNumDraws += that->fNumDraws);
    SkDEBUGCODE(that->fNumDraws = 0);
    return CombineResult::kMerged;
}

void GrCCDrawPathsOp::wasRecorded(sk_sp<GrCCPerOpListPaths> owningPerOpListPaths) {
    SkASSERT(1 == fNumDraws);
    SkASSERT(!fOwningPerOpListPaths);
    fOwningPerOpListPaths = std::move(owningPerOpListPaths);
    fOwningPerOpListPaths->fDrawOps.addToTail(this);
}

void GrCCDrawPathsOp::accountForOwnPaths(GrCCPathCache* pathCache,
                                         GrOnFlushResourceProvider* onFlushRP,
                                         const GrUniqueKey& stashedAtlasKey,
                                         GrCCPerFlushResourceSpecs* specs) {
    using CreateIfAbsent = GrCCPathCache::CreateIfAbsent;
    using MaskTransform = GrCCPathCache::MaskTransform;

    for (SingleDraw& draw : fDraws) {
        SkPath path;
        draw.fShape.asPath(&path);

        SkASSERT(!draw.fCacheEntry);

        if (pathCache) {
            MaskTransform m(draw.fMatrix, &draw.fCachedMaskShift);
            bool canStashPathMask = draw.fMaskVisibility >= Visibility::kMostlyComplete;
            draw.fCacheEntry = pathCache->find(draw.fShape, m, CreateIfAbsent(canStashPathMask));
        }

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
                    int idx = (draw.fShape.style().strokeRec().isFillStyle())
                            ? GrCCPerFlushResourceSpecs::kFillIdx
                            : GrCCPerFlushResourceSpecs::kStrokeIdx;
                    ++specs->fNumCopiedPaths[idx];
                    specs->fCopyPathStats[idx].statPath(path);
                    specs->fCopyAtlasSpecs.accountForSpace(cacheEntry->width(),
                                                           cacheEntry->height());
                    continue;
                }

                // Whatever atlas the path used to reside in, it no longer exists.
                cacheEntry->resetAtlasKeyAndInfo();
            }

            if (Visibility::kMostlyComplete == draw.fMaskVisibility && cacheEntry->hitCount() > 1) {
                int shapeSize = SkTMax(draw.fShapeConservativeIBounds.height(),
                                       draw.fShapeConservativeIBounds.width());
                if (shapeSize <= onFlushRP->caps()->maxRenderTargetSize()) {
                    // We've seen this path before with a compatible matrix, and it's mostly
                    // visible. Just render the whole mask so we can try to cache it.
                    draw.fMaskDevIBounds = draw.fShapeConservativeIBounds;
                    draw.fMaskVisibility = Visibility::kComplete;
                }
            }
        }

        int idx = (draw.fShape.style().strokeRec().isFillStyle())
                ? GrCCPerFlushResourceSpecs::kFillIdx
                : GrCCPerFlushResourceSpecs::kStrokeIdx;
        ++specs->fNumRenderedPaths[idx];
        specs->fRenderedPathStats[idx].statPath(path);
        specs->fRenderedAtlasSpecs.accountForSpace(draw.fMaskDevIBounds.width(),
                                                   draw.fMaskDevIBounds.height());
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

        auto doEvenOddFill = DoEvenOddFill(draw.fShape.style().strokeRec().isFillStyle() &&
                                           SkPath::kEvenOdd_FillType == path.getFillType());
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
                                                onFlushRP->contextUniqueID(), newOffset,
                                                atlas->refOrMakeCachedAtlasInfo());
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
        if (auto atlas = resources->renderShapeInAtlas(
                    draw.fMaskDevIBounds, draw.fMatrix, draw.fShape, draw.fStrokeDevWidth,
                    &devBounds, &devBounds45, &devIBounds, &devToAtlasOffset)) {
            this->recordInstance(atlas->textureProxy(), resources->nextPathInstanceIdx());
            resources->appendDrawPathInstance().set(devBounds, devBounds45, devToAtlasOffset,
                                                    draw.fColor, doEvenOddFill);

            // If we have a spot in the path cache, try to make a note of where this mask is so we
            // can reuse it in the future.
            if (auto cacheEntry = draw.fCacheEntry.get()) {
                SkASSERT(!cacheEntry->hasCachedAtlas());

                if (Visibility::kComplete != draw.fMaskVisibility || cacheEntry->hitCount() <= 1) {
                    // Don't cache a path mask unless it's completely visible with a hit count > 1.
                    //
                    // NOTE: mostly-visible paths with a hit count > 1 should have been promoted to
                    // fully visible during accountForOwnPaths().
                    continue;
                }

                if (resources->nextAtlasToStash() != atlas) {
                    // This mask does not belong to the atlas that will be stashed for next flush.
                    continue;
                }

                const GrUniqueKey& atlasKey =
                        resources->nextAtlasToStash()->getOrAssignUniqueKey(onFlushRP);
                cacheEntry->initAsStashedAtlas(atlasKey, onFlushRP->contextUniqueID(),
                                               devToAtlasOffset, devBounds, devBounds45, devIBounds,
                                               draw.fCachedMaskShift);
                // Remember this atlas in case we encounter the path again during the same flush.
                cacheEntry->setCurrFlushAtlas(atlas);
            }
            continue;
        }
    }

    if (!fInstanceRanges.empty()) {
        fInstanceRanges.back().fEndInstanceIdx = resources->nextPathInstanceIdx();
    }
}

inline void GrCCDrawPathsOp::recordInstance(GrTextureProxy* atlasProxy, int instanceIdx) {
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
    initArgs.fProxy = flushState->drawOpArgs().fProxy;
    initArgs.fCaps = &flushState->caps();
    initArgs.fResourceProvider = flushState->resourceProvider();
    initArgs.fDstProxy = flushState->drawOpArgs().fDstProxy;
    auto clip = flushState->detachAppliedClip();
    GrPipeline::FixedDynamicState fixedDynamicState(clip.scissorState().rect());
    GrPipeline pipeline(initArgs, std::move(fProcessors), std::move(clip));

    int baseInstance = fBaseInstance;
    SkASSERT(baseInstance >= 0);  // Make sure setupResources() has been called.

    for (const InstanceRange& range : fInstanceRanges) {
        SkASSERT(range.fEndInstanceIdx > baseInstance);

        GrCCPathProcessor pathProc(range.fAtlasProxy, fViewMatrixIfUsingLocalCoords);
        GrTextureProxy* atlasProxy = range.fAtlasProxy;
        fixedDynamicState.fPrimitiveProcessorTextures = &atlasProxy;
        pathProc.drawPaths(flushState, pipeline, &fixedDynamicState, *resources, baseInstance,
                           range.fEndInstanceIdx, this->bounds());

        baseInstance = range.fEndInstanceIdx;
    }
}
