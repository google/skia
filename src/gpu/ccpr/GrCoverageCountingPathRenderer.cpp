/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCoverageCountingPathRenderer.h"

#include "GrCaps.h"
#include "GrClip.h"
#include "GrProxyProvider.h"
#include "SkMakeUnique.h"
#include "SkPathOps.h"
#include "ccpr/GrCCClipProcessor.h"
#include "ccpr/GrCCDrawPathsOp.h"
#include "ccpr/GrCCPathCache.h"

using PathInstance = GrCCPathProcessor::Instance;

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    return caps.instanceAttribSupport() && shaderCaps.integerSupport() &&
           shaderCaps.floatIs32Bits() && GrCaps::kNone_MapFlags != caps.mapBufferFlags() &&
           caps.isConfigTexturable(kAlpha_half_GrPixelConfig) &&
           caps.isConfigRenderable(kAlpha_half_GrPixelConfig) &&
           caps.isConfigTexturable(kAlpha_8_GrPixelConfig) &&
           caps.isConfigRenderable(kAlpha_8_GrPixelConfig) &&
           caps.halfFloatVertexAttributeSupport() &&
           !caps.blacklistCoverageCounting();
}

sk_sp<GrCoverageCountingPathRenderer> GrCoverageCountingPathRenderer::CreateIfSupported(
        const GrCaps& caps, AllowCaching allowCaching, uint32_t contextUniqueID) {
    return sk_sp<GrCoverageCountingPathRenderer>((IsSupported(caps))
            ? new GrCoverageCountingPathRenderer(allowCaching, contextUniqueID)
            : nullptr);
}

GrCoverageCountingPathRenderer::GrCoverageCountingPathRenderer(AllowCaching allowCaching,
                                                               uint32_t contextUniqueID) {
    if (AllowCaching::kYes == allowCaching) {
        fPathCache = skstd::make_unique<GrCCPathCache>(contextUniqueID);
    }
}

GrCCPerOpListPaths* GrCoverageCountingPathRenderer::lookupPendingPaths(uint32_t opListID) {
    auto it = fPendingPaths.find(opListID);
    if (fPendingPaths.end() == it) {
        sk_sp<GrCCPerOpListPaths> paths = sk_make_sp<GrCCPerOpListPaths>();
        it = fPendingPaths.insert(std::make_pair(opListID, std::move(paths))).first;
    }
    return it->second.get();
}

GrPathRenderer::CanDrawPath GrCoverageCountingPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    const GrShape& shape = *args.fShape;
    if (!(AATypeFlags::kCoverage & args.fAATypeFlags) || shape.style().hasPathEffect() ||
        args.fViewMatrix->hasPerspective() || shape.inverseFilled()) {
        return CanDrawPath::kNo;
    }

    SkPath path;
    shape.asPath(&path);

    const SkStrokeRec& stroke = shape.style().strokeRec();
    switch (stroke.getStyle()) {
        case SkStrokeRec::kFill_Style: {
            SkRect devBounds;
            args.fViewMatrix->mapRect(&devBounds, path.getBounds());

            SkIRect clippedIBounds;
            devBounds.roundOut(&clippedIBounds);
            if (!clippedIBounds.intersect(*args.fClipConservativeBounds)) {
                // The path is completely clipped away. Our code will eventually notice this before
                // doing any real work.
                return CanDrawPath::kYes;
            }

            int64_t numPixels = sk_64_mul(clippedIBounds.height(), clippedIBounds.width());
            if (path.countVerbs() > 1000 && path.countPoints() > numPixels) {
                // This is a complicated path that has more vertices than pixels! Let's let the SW
                // renderer have this one: It will probably be faster and a bitmap will require less
                // total memory on the GPU than CCPR instance buffers would for the raw path data.
                return CanDrawPath::kNo;
            }

            if (numPixels > 256 * 256) {
                // Large paths can blow up the atlas fast. And they are not ideal for a two-pass
                // rendering algorithm. Give the simpler direct renderers a chance before we commit
                // to drawing it.
                return CanDrawPath::kAsBackup;
            }

            if (args.fShape->hasUnstyledKey() && path.countVerbs() > 50) {
                // Complex paths do better cached in an SDF, if the renderer will accept them.
                return CanDrawPath::kAsBackup;
            }

            return CanDrawPath::kYes;
        }

        case SkStrokeRec::kStroke_Style:
            if (!args.fViewMatrix->isSimilarity()) {
                // The stroker currently only supports rigid-body transfoms for the stroke lines
                // themselves. This limitation doesn't affect hairlines since their stroke lines are
                // defined relative to device space.
                return CanDrawPath::kNo;
            }
            // fallthru
        case SkStrokeRec::kHairline_Style: {
            float inflationRadius;
            GetStrokeDevWidth(*args.fViewMatrix, stroke, &inflationRadius);
            if (!(inflationRadius <= kMaxBoundsInflationFromStroke)) {
                // Let extremely wide strokes be converted to fill paths and drawn by the CCPR
                // filler instead. (Cast the logic negatively in order to also catch r=NaN.)
                return CanDrawPath::kNo;
            }
            SkASSERT(!SkScalarIsNaN(inflationRadius));
            if (SkPathPriv::ConicWeightCnt(path)) {
                // The stroker does not support conics yet.
                return CanDrawPath::kNo;
            }
            return CanDrawPath::kYes;
        }

        case SkStrokeRec::kStrokeAndFill_Style:
            return CanDrawPath::kNo;
    }

    SK_ABORT("Invalid stroke style.");
    return CanDrawPath::kNo;
}

bool GrCoverageCountingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    SkASSERT(!fFlushing);

    SkIRect clipIBounds;
    GrRenderTargetContext* rtc = args.fRenderTargetContext;
    args.fClip->getConservativeBounds(rtc->width(), rtc->height(), &clipIBounds, nullptr);

    auto op = GrCCDrawPathsOp::Make(args.fContext, clipIBounds, *args.fViewMatrix, *args.fShape,
                                    std::move(args.fPaint));
    this->recordOp(std::move(op), args);
    return true;
}

void GrCoverageCountingPathRenderer::recordOp(std::unique_ptr<GrCCDrawPathsOp> op,
                                              const DrawPathArgs& args) {
    if (op) {
        auto addToOwningPerOpListPaths = [this](GrOp* op, uint32_t opListID) {
            op->cast<GrCCDrawPathsOp>()->addToOwningPerOpListPaths(
                    sk_ref_sp(this->lookupPendingPaths(opListID)));
        };
        args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op), addToOwningPerOpListPaths);
    }
}

std::unique_ptr<GrFragmentProcessor> GrCoverageCountingPathRenderer::makeClipProcessor(
        uint32_t opListID, const SkPath& deviceSpacePath, const SkIRect& accessRect, int rtWidth,
        int rtHeight, const GrCaps& caps) {
    using MustCheckBounds = GrCCClipProcessor::MustCheckBounds;

    SkASSERT(!fFlushing);

    GrCCClipPath& clipPath =
            this->lookupPendingPaths(opListID)->fClipPaths[deviceSpacePath.getGenerationID()];
    if (!clipPath.isInitialized()) {
        // This ClipPath was just created during lookup. Initialize it.
        const SkRect& pathDevBounds = deviceSpacePath.getBounds();
        if (SkTMax(pathDevBounds.height(), pathDevBounds.width()) > kPathCropThreshold) {
            // The path is too large. Crop it or analytic AA can run out of fp32 precision.
            SkPath croppedPath;
            int maxRTSize = caps.maxRenderTargetSize();
            CropPath(deviceSpacePath, SkIRect::MakeWH(maxRTSize, maxRTSize), &croppedPath);
            clipPath.init(croppedPath, accessRect, rtWidth, rtHeight, caps);
        } else {
            clipPath.init(deviceSpacePath, accessRect, rtWidth, rtHeight, caps);
        }
    } else {
        clipPath.addAccess(accessRect);
    }

    bool mustCheckBounds = !clipPath.pathDevIBounds().contains(accessRect);
    return skstd::make_unique<GrCCClipProcessor>(&clipPath, MustCheckBounds(mustCheckBounds),
                                                 deviceSpacePath.getFillType());
}

void GrCoverageCountingPathRenderer::preFlush(GrOnFlushResourceProvider* onFlushRP,
                                              const uint32_t* opListIDs, int numOpListIDs,
                                              SkTArray<sk_sp<GrRenderTargetContext>>* out) {
    using DoCopiesToA8Coverage = GrCCDrawPathsOp::DoCopiesToA8Coverage;
    SkASSERT(!fFlushing);
    SkASSERT(fFlushingPaths.empty());
    SkDEBUGCODE(fFlushing = true);

    if (fPathCache) {
        fPathCache->doPreFlushProcessing();
    }

    if (fPendingPaths.empty()) {
        return;  // Nothing to draw.
    }

    GrCCPerFlushResourceSpecs specs;
    int maxPreferredRTSize = onFlushRP->caps()->maxPreferredRenderTargetSize();
    specs.fCopyAtlasSpecs.fMaxPreferredTextureSize = SkTMin(2048, maxPreferredRTSize);
    SkASSERT(0 == specs.fCopyAtlasSpecs.fMinTextureSize);
    specs.fRenderedAtlasSpecs.fMaxPreferredTextureSize = maxPreferredRTSize;
    specs.fRenderedAtlasSpecs.fMinTextureSize = SkTMin(512, maxPreferredRTSize);

    // Move the per-opList paths that are about to be flushed from fPendingPaths to fFlushingPaths,
    // and count them up so we can preallocate buffers.
    fFlushingPaths.reserve(numOpListIDs);
    for (int i = 0; i < numOpListIDs; ++i) {
        auto iter = fPendingPaths.find(opListIDs[i]);
        if (fPendingPaths.end() == iter) {
            continue;  // No paths on this opList.
        }

        fFlushingPaths.push_back(std::move(iter->second));
        fPendingPaths.erase(iter);

        for (GrCCDrawPathsOp* op : fFlushingPaths.back()->fDrawOps) {
            op->accountForOwnPaths(fPathCache.get(), onFlushRP, &specs);
        }
        for (const auto& clipsIter : fFlushingPaths.back()->fClipPaths) {
            clipsIter.second.accountForOwnPath(&specs);
        }
    }

    if (specs.isEmpty()) {
        return;  // Nothing to draw.
    }

    // Determine if there are enough reusable paths from last flush for it to be worth our time to
    // copy them to cached atlas(es).
    int numCopies = specs.fNumCopiedPaths[GrCCPerFlushResourceSpecs::kFillIdx] +
                    specs.fNumCopiedPaths[GrCCPerFlushResourceSpecs::kStrokeIdx];
    auto doCopies = DoCopiesToA8Coverage(numCopies > 100 ||
                                         specs.fCopyAtlasSpecs.fApproxNumPixels > 256 * 256);
    if (numCopies && DoCopiesToA8Coverage::kNo == doCopies) {
        specs.cancelCopies();
    }

    auto resources = sk_make_sp<GrCCPerFlushResources>(onFlushRP, specs);
    if (!resources->isMapped()) {
        return;  // Some allocation failed.
    }

    // Layout the atlas(es) and parse paths.
    for (const auto& flushingPaths : fFlushingPaths) {
        for (GrCCDrawPathsOp* op : flushingPaths->fDrawOps) {
            op->setupResources(fPathCache.get(), onFlushRP, resources.get(), doCopies);
        }
        for (auto& clipsIter : flushingPaths->fClipPaths) {
            clipsIter.second.renderPathInAtlas(resources.get(), onFlushRP);
        }
    }

    if (fPathCache) {
        // Purge invalidated textures from previous atlases *before* calling finalize(). That way,
        // the underlying textures objects can be freed up and reused for the next atlases.
        fPathCache->purgeInvalidatedAtlasTextures(onFlushRP);
    }

    // Allocate resources and then render the atlas(es).
    if (!resources->finalize(onFlushRP, out)) {
        return;
    }

    // Commit flushing paths to the resources once they are successfully completed.
    for (auto& flushingPaths : fFlushingPaths) {
        SkASSERT(!flushingPaths->fFlushResources);
        flushingPaths->fFlushResources = resources;
    }
}

void GrCoverageCountingPathRenderer::postFlush(GrDeferredUploadToken, const uint32_t* opListIDs,
                                               int numOpListIDs) {
    SkASSERT(fFlushing);

    if (!fFlushingPaths.empty()) {
        // In DDL mode these aren't guaranteed to be deleted so we must clear out the perFlush
        // resources manually.
        for (auto& flushingPaths : fFlushingPaths) {
            flushingPaths->fFlushResources = nullptr;
        }

        // We wait to erase these until after flush, once Ops and FPs are done accessing their data.
        fFlushingPaths.reset();
    }

    SkDEBUGCODE(fFlushing = false);
}

void GrCoverageCountingPathRenderer::purgeCacheEntriesOlderThan(
        GrProxyProvider* proxyProvider, const GrStdSteadyClock::time_point& purgeTime) {
    if (fPathCache) {
        fPathCache->purgeEntriesOlderThan(proxyProvider, purgeTime);
    }
}

void GrCoverageCountingPathRenderer::CropPath(const SkPath& path, const SkIRect& cropbox,
                                              SkPath* out) {
    SkPath cropboxPath;
    cropboxPath.addRect(SkRect::Make(cropbox));
    if (!Op(cropboxPath, path, kIntersect_SkPathOp, out)) {
        // This can fail if the PathOps encounter NaN or infinities.
        out->reset();
    }
    out->setIsVolatile(true);
}

float GrCoverageCountingPathRenderer::GetStrokeDevWidth(const SkMatrix& m,
                                                        const SkStrokeRec& stroke,
                                                        float* inflationRadius) {
    float strokeDevWidth;
    if (stroke.isHairlineStyle()) {
        strokeDevWidth = 1;
    } else {
        SkASSERT(SkStrokeRec::kStroke_Style == stroke.getStyle());
        SkASSERT(m.isSimilarity());  // Otherwise matrixScaleFactor = m.getMaxScale().
        float matrixScaleFactor = SkVector::Length(m.getScaleX(), m.getSkewY());
        strokeDevWidth = stroke.getWidth() * matrixScaleFactor;
    }
    if (inflationRadius) {
        // Inflate for a minimum stroke width of 1. In some cases when the stroke is less than 1px
        // wide, we may inflate it to 1px and instead reduce the opacity.
        *inflationRadius = SkStrokeRec::GetInflationRadius(
                stroke.getJoin(), stroke.getMiter(), stroke.getCap(), SkTMax(strokeDevWidth, 1.f));
    }
    return strokeDevWidth;
}
