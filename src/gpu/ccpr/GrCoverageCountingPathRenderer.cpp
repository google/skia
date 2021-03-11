/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"

#include "include/pathops/SkPathOps.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ccpr/GrCCClipProcessor.h"
#include "src/gpu/ccpr/GrCCDrawPathsOp.h"
#include "src/gpu/ccpr/GrCCPathCache.h"

using PathInstance = GrCCPathProcessor::Instance;

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps, CoverageType* coverageType) {
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    GrBackendFormat defaultA8Format = caps.getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                                   GrRenderable::kYes);
    if (caps.driverDisableCCPR() || !shaderCaps.integerSupport() ||
        !caps.drawInstancedSupport() || !shaderCaps.floatIs32Bits() ||
        !defaultA8Format.isValid() || // This checks both texturable and renderable
        !caps.halfFloatVertexAttributeSupport()) {
        return false;
    }

    if (!caps.driverDisableMSAACCPR() &&
        caps.internalMultisampleCount(defaultA8Format) > 1 &&
        caps.sampleLocationsSupport() &&
        shaderCaps.sampleMaskSupport()) {
        if (coverageType) {
            *coverageType = CoverageType::kA8_Multisample;
        }
        return true;
    }

    return false;
}

sk_sp<GrCoverageCountingPathRenderer> GrCoverageCountingPathRenderer::CreateIfSupported(
        const GrCaps& caps, AllowCaching allowCaching, uint32_t contextUniqueID) {
    CoverageType coverageType;
    if (IsSupported(caps, &coverageType)) {
        return sk_sp<GrCoverageCountingPathRenderer>(new GrCoverageCountingPathRenderer(
                coverageType, allowCaching, contextUniqueID));
    }
    return nullptr;
}

GrCoverageCountingPathRenderer::GrCoverageCountingPathRenderer(
        CoverageType coverageType, AllowCaching allowCaching, uint32_t contextUniqueID)
        : fCoverageType(coverageType) {
    if (AllowCaching::kYes == allowCaching) {
        fPathCache = std::make_unique<GrCCPathCache>(contextUniqueID);
    }
}

GrCCPerOpsTaskPaths* GrCoverageCountingPathRenderer::lookupPendingPaths(uint32_t opsTaskID) {
    auto it = fPendingPaths.find(opsTaskID);
    if (fPendingPaths.end() == it) {
        sk_sp<GrCCPerOpsTaskPaths> paths = sk_make_sp<GrCCPerOpsTaskPaths>();
        it = fPendingPaths.insert(std::make_pair(opsTaskID, std::move(paths))).first;
    }
    return it->second.get();
}

GrPathRenderer::CanDrawPath GrCoverageCountingPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
#if 1
    // The atlas takes up too much memory. We should focus on other path renderers instead.
    return CanDrawPath::kNo;
#else
    const GrStyledShape& shape = *args.fShape;
    // We use "kCoverage", or analytic AA, no mater what the coverage type of our atlas: Even if the
    // atlas is multisampled, that resolves into analytic coverage before we draw the path to the
    // main canvas.
    if (GrAAType::kCoverage != args.fAAType || shape.style().hasPathEffect() ||
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
        case SkStrokeRec::kHairline_Style:
        case SkStrokeRec::kStrokeAndFill_Style:
            return CanDrawPath::kNo;
    }

    SK_ABORT("Invalid stroke style.");
#endif
}

bool GrCoverageCountingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    SkASSERT(!fFlushing);

    auto op = GrCCDrawPathsOp::Make(args.fContext, *args.fClipConservativeBounds, *args.fViewMatrix,
                                    *args.fShape, std::move(args.fPaint));
    this->recordOp(std::move(op), args);
    return true;
}

void GrCoverageCountingPathRenderer::recordOp(GrOp::Owner op,
                                              const DrawPathArgs& args) {
    if (op) {
        auto addToOwningPerOpsTaskPaths = [this](GrOp* op, uint32_t opsTaskID) {
            op->cast<GrCCDrawPathsOp>()->addToOwningPerOpsTaskPaths(
                    sk_ref_sp(this->lookupPendingPaths(opsTaskID)));
        };
        args.fRenderTargetContext->addDrawOp(args.fClip, std::move(op),
                                             addToOwningPerOpsTaskPaths);
    }
}

std::unique_ptr<GrFragmentProcessor> GrCoverageCountingPathRenderer::makeClipProcessor(
        std::unique_ptr<GrFragmentProcessor> inputFP, uint32_t opsTaskID,
        const SkPath& deviceSpacePath, const SkIRect& accessRect, const GrCaps& caps) {
#ifdef SK_DEBUG
    SkASSERT(!fFlushing);
    SkIRect pathIBounds;
    deviceSpacePath.getBounds().roundOut(&pathIBounds);
    SkIRect maskBounds;
    if (maskBounds.intersect(accessRect, pathIBounds)) {
        SkASSERT(maskBounds.height64() * maskBounds.width64() <= kMaxClipPathArea);
    }
#endif

    uint32_t key = deviceSpacePath.getGenerationID();
    if (CoverageType::kA8_Multisample == fCoverageType) {
        // We only need to consider fill rule in MSAA mode. In coverage count mode Even/Odd and
        // Nonzero both reference the same coverage count mask.
        key = (key << 1) | (uint32_t)GrFillRuleForSkPath(deviceSpacePath);
    }
    GrCCClipPath& clipPath =
            this->lookupPendingPaths(opsTaskID)->fClipPaths[key];
    if (!clipPath.isInitialized()) {
        // This ClipPath was just created during lookup. Initialize it.
        const SkRect& pathDevBounds = deviceSpacePath.getBounds();
        if (std::max(pathDevBounds.height(), pathDevBounds.width()) > kPathCropThreshold) {
            // The path is too large. Crop it or analytic AA can run out of fp32 precision.
            SkPath croppedPath;
            int maxRTSize = caps.maxRenderTargetSize();
            CropPath(deviceSpacePath, SkIRect::MakeWH(maxRTSize, maxRTSize), &croppedPath);
            clipPath.init(croppedPath, accessRect, fCoverageType, caps);
        } else {
            clipPath.init(deviceSpacePath, accessRect, fCoverageType, caps);
        }
    } else {
        clipPath.addAccess(accessRect);
    }

    auto mustCheckBounds = GrCCClipProcessor::MustCheckBounds(
            !clipPath.pathDevIBounds().contains(accessRect));
    return std::make_unique<GrCCClipProcessor>(std::move(inputFP), caps, &clipPath,
                                               mustCheckBounds);
}

void GrCoverageCountingPathRenderer::preFlush(
        GrOnFlushResourceProvider* onFlushRP, SkSpan<const uint32_t> taskIDs) {
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
    specs.fCopyAtlasSpecs.fMaxPreferredTextureSize = std::min(2048, maxPreferredRTSize);
    SkASSERT(0 == specs.fCopyAtlasSpecs.fMinTextureSize);
    specs.fRenderedAtlasSpecs.fMaxPreferredTextureSize = maxPreferredRTSize;
    specs.fRenderedAtlasSpecs.fMinTextureSize = std::min(512, maxPreferredRTSize);

    // Move the per-opsTask paths that are about to be flushed from fPendingPaths to fFlushingPaths,
    // and count them up so we can preallocate buffers.
    fFlushingPaths.reserve_back(taskIDs.count());
    for (uint32_t taskID : taskIDs) {
        auto iter = fPendingPaths.find(taskID);
        if (fPendingPaths.end() == iter) {
            continue;  // No paths on this opsTask.
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
    int numCopies = specs.fNumCopiedPaths;
    auto doCopies = DoCopiesToA8Coverage(numCopies > kDoCopiesThreshold ||
                                         specs.fCopyAtlasSpecs.fApproxNumPixels > 256 * 256);
    if (numCopies && DoCopiesToA8Coverage::kNo == doCopies) {
        specs.cancelCopies();
    }

    auto resources = sk_make_sp<GrCCPerFlushResources>(onFlushRP, fCoverageType, specs);
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
    if (!resources->finalize(onFlushRP)) {
        return;
    }

    // Commit flushing paths to the resources once they are successfully completed.
    for (auto& flushingPaths : fFlushingPaths) {
        SkASSERT(!flushingPaths->fFlushResources);
        flushingPaths->fFlushResources = resources;
    }
}

void GrCoverageCountingPathRenderer::postFlush(GrDeferredUploadToken,
                                               SkSpan<const uint32_t> /* taskIDs */) {
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
                stroke.getJoin(), stroke.getMiter(), stroke.getCap(), std::max(strokeDevWidth, 1.f));
    }
    return strokeDevWidth;
}
