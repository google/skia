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
                 paint.getColor4f())
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
                                        const SkPMColor4f& color)
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

GrDrawOp::RequiresDstTexture GrCCDrawPathsOp::finalize(const GrCaps& caps,
                                                       const GrAppliedClip* clip) {
    SkASSERT(1 == fNumDraws);  // There should only be one single path draw in this Op right now.
    return fDraws.head().finalize(caps, clip, &fProcessors);
}

GrDrawOp::RequiresDstTexture GrCCDrawPathsOp::SingleDraw::finalize(
        const GrCaps& caps, const GrAppliedClip* clip, GrProcessorSet* processors) {
    const GrProcessorSet::Analysis& analysis = processors->finalize(
            fColor, GrProcessorAnalysisCoverage::kSingleChannel, clip, false, caps,
            &fColor);

    // Lines start looking jagged when they get thinner than 1px. For thin strokes it looks better
    // if we can convert them to hairline (i.e., inflate the stroke width to 1px), and instead
    // reduce the opacity to create the illusion of thin-ness. This strategy also helps reduce
    // artifacts from coverage dilation when there are self intersections.
    if (analysis.isCompatibleWithCoverageAsAlpha() &&
            !fShape.style().strokeRec().isFillStyle() && fStrokeDevWidth < 1) {
        // Modifying the shape affects its cache key. The draw can't have a cache entry yet or else
        // our next step would invalidate it.
        SkASSERT(!fCacheEntry);
        SkASSERT(SkStrokeRec::kStroke_Style == fShape.style().strokeRec().getStyle());

        SkPath path;
        fShape.asPath(&path);

        // Create a hairline version of our stroke.
        SkStrokeRec hairlineStroke = fShape.style().strokeRec();
        hairlineStroke.setStrokeStyle(0);

        // How transparent does a 1px stroke have to be in order to appear as thin as the real one?
        float coverage = fStrokeDevWidth;

        fShape = GrShape(path, GrStyle(hairlineStroke, nullptr));
        fStrokeDevWidth = 1;

        // TODO4F: Preserve float colors
        // fShapeConservativeIBounds already accounted for this possibility of inflating the stroke.
        fColor = fColor * coverage;
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

    SkDEBUGCODE(fNumDraws += that->fNumDraws);
    SkDEBUGCODE(that->fNumDraws = 0);
    return CombineResult::kMerged;
}

void GrCCDrawPathsOp::addToOwningPerOpListPaths(sk_sp<GrCCPerOpListPaths> owningPerOpListPaths) {
    SkASSERT(1 == fNumDraws);
    SkASSERT(!fOwningPerOpListPaths);
    fOwningPerOpListPaths = std::move(owningPerOpListPaths);
    fOwningPerOpListPaths->fDrawOps.addToTail(this);
}

void GrCCDrawPathsOp::accountForOwnPaths(GrCCPathCache* pathCache,
                                         GrOnFlushResourceProvider* onFlushRP,
                                         GrCCPerFlushResourceSpecs* specs) {
    for (SingleDraw& draw : fDraws) {
        draw.accountForOwnPath(pathCache, onFlushRP, specs);
    }
}

void GrCCDrawPathsOp::SingleDraw::accountForOwnPath(
        GrCCPathCache* pathCache, GrOnFlushResourceProvider* onFlushRP,
        GrCCPerFlushResourceSpecs* specs) {
    using CreateIfAbsent = GrCCPathCache::CreateIfAbsent;
    using MaskTransform = GrCCPathCache::MaskTransform;
    using CoverageType = GrCCAtlas::CoverageType;

    SkPath path;
    fShape.asPath(&path);

    SkASSERT(!fCacheEntry);

    if (pathCache) {
        MaskTransform m(fMatrix, &fCachedMaskShift);
        bool canStashPathMask = fMaskVisibility >= Visibility::kMostlyComplete;
        fCacheEntry = pathCache->find(onFlushRP, fShape, m, CreateIfAbsent(canStashPathMask));
    }

    if (fCacheEntry) {
        if (const GrCCCachedAtlas* cachedAtlas = fCacheEntry->cachedAtlas()) {
            SkASSERT(cachedAtlas->getOnFlushProxy());
            if (CoverageType::kA8_LiteralCoverage == cachedAtlas->coverageType()) {
                ++specs->fNumCachedPaths;
            } else {
                // Suggest that this path be copied to a literal coverage atlas, to save memory.
                // (The client may decline this copy via DoCopiesToA8Coverage::kNo.)
                int idx = (fShape.style().strokeRec().isFillStyle())
                        ? GrCCPerFlushResourceSpecs::kFillIdx
                        : GrCCPerFlushResourceSpecs::kStrokeIdx;
                ++specs->fNumCopiedPaths[idx];
                specs->fCopyPathStats[idx].statPath(path);
                specs->fCopyAtlasSpecs.accountForSpace(fCacheEntry->width(), fCacheEntry->height());
                fDoCopyToA8Coverage = true;
            }
            return;
        }

        if (Visibility::kMostlyComplete == fMaskVisibility && fCacheEntry->hitCount() > 1) {
            int shapeSize = SkTMax(fShapeConservativeIBounds.height(),
                                   fShapeConservativeIBounds.width());
            if (shapeSize <= onFlushRP->caps()->maxRenderTargetSize()) {
                // We've seen this path before with a compatible matrix, and it's mostly
                // visible. Just render the whole mask so we can try to cache it.
                fMaskDevIBounds = fShapeConservativeIBounds;
                fMaskVisibility = Visibility::kComplete;
            }
        }
    }

    int idx = (fShape.style().strokeRec().isFillStyle())
            ? GrCCPerFlushResourceSpecs::kFillIdx
            : GrCCPerFlushResourceSpecs::kStrokeIdx;
    ++specs->fNumRenderedPaths[idx];
    specs->fRenderedPathStats[idx].statPath(path);
    specs->fRenderedAtlasSpecs.accountForSpace(fMaskDevIBounds.width(),
                                               fMaskDevIBounds.height());
}

void GrCCDrawPathsOp::setupResources(
        GrCCPathCache* pathCache, GrOnFlushResourceProvider* onFlushRP,
        GrCCPerFlushResources* resources, DoCopiesToA8Coverage doCopies) {
    SkASSERT(fNumDraws > 0);
    SkASSERT(-1 == fBaseInstance);
    fBaseInstance = resources->nextPathInstanceIdx();

    for (SingleDraw& draw : fDraws) {
        draw.setupResources(pathCache, onFlushRP, resources, doCopies, this);
    }

    if (!fInstanceRanges.empty()) {
        fInstanceRanges.back().fEndInstanceIdx = resources->nextPathInstanceIdx();
    }
}

void GrCCDrawPathsOp::SingleDraw::setupResources(
        GrCCPathCache* pathCache, GrOnFlushResourceProvider* onFlushRP,
        GrCCPerFlushResources* resources, DoCopiesToA8Coverage doCopies, GrCCDrawPathsOp* op) {
    using DoEvenOddFill = GrCCPathProcessor::DoEvenOddFill;

    SkPath path;
    fShape.asPath(&path);

    auto doEvenOddFill = DoEvenOddFill(fShape.style().strokeRec().isFillStyle() &&
                                       SkPath::kEvenOdd_FillType == path.getFillType());
    SkASSERT(SkPath::kEvenOdd_FillType == path.getFillType() ||
             SkPath::kWinding_FillType == path.getFillType());

    if (fCacheEntry) {
        // Does the path already exist in a cached atlas texture?
        if (fCacheEntry->cachedAtlas()) {
            SkASSERT(fCacheEntry->cachedAtlas()->getOnFlushProxy());
            if (DoCopiesToA8Coverage::kYes == doCopies && fDoCopyToA8Coverage) {
                resources->upgradeEntryToLiteralCoverageAtlas(pathCache, onFlushRP,
                                                              fCacheEntry.get(), doEvenOddFill);
                SkASSERT(fCacheEntry->cachedAtlas());
                SkASSERT(GrCCAtlas::CoverageType::kA8_LiteralCoverage
                                 == fCacheEntry->cachedAtlas()->coverageType());
                SkASSERT(fCacheEntry->cachedAtlas()->getOnFlushProxy());
            }
            op->recordInstance(fCacheEntry->cachedAtlas()->getOnFlushProxy(),
                               resources->nextPathInstanceIdx());
            // TODO4F: Preserve float colors
            resources->appendDrawPathInstance().set(*fCacheEntry, fCachedMaskShift,
                                                    fColor.toBytes_RGBA());
            return;
        }
    }

    // Render the raw path into a coverage count atlas. renderShapeInAtlas() gives us two tight
    // bounding boxes: One in device space, as well as a second one rotated an additional 45
    // degrees. The path vertex shader uses these two bounding boxes to generate an octagon that
    // circumscribes the path.
    SkRect devBounds, devBounds45;
    SkIRect devIBounds;
    SkIVector devToAtlasOffset;
    if (auto atlas = resources->renderShapeInAtlas(
                fMaskDevIBounds, fMatrix, fShape, fStrokeDevWidth, &devBounds, &devBounds45,
                &devIBounds, &devToAtlasOffset)) {
        op->recordInstance(atlas->textureProxy(), resources->nextPathInstanceIdx());
        // TODO4F: Preserve float colors
        resources->appendDrawPathInstance().set(devBounds, devBounds45, devToAtlasOffset,
                                                fColor.toBytes_RGBA(), doEvenOddFill);

        // If we have a spot in the path cache, try to make a note of where this mask is so we
        // can reuse it in the future.
        if (fCacheEntry) {
            SkASSERT(!fCacheEntry->cachedAtlas());

            if (Visibility::kComplete != fMaskVisibility || fCacheEntry->hitCount() <= 1) {
                // Don't cache a path mask unless it's completely visible with a hit count > 1.
                //
                // NOTE: mostly-visible paths with a hit count > 1 should have been promoted to
                // fully visible during accountForOwnPaths().
                return;
            }

            fCacheEntry->setCoverageCountAtlas(onFlushRP, atlas, devToAtlasOffset, devBounds,
                                               devBounds45, devIBounds, fCachedMaskShift);
        }
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

void GrCCDrawPathsOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
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
