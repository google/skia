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
#include "ccpr/GrCCPathParser.h"

using PathInstance = GrCCPathProcessor::Instance;

// If a path spans more pixels than this, we need to crop it or else analytic AA can run out of fp32
// precision.
static constexpr float kPathCropThreshold = 1 << 16;

static void crop_path(const SkPath& path, const SkIRect& cropbox, SkPath* out) {
    SkPath cropboxPath;
    cropboxPath.addRect(SkRect::Make(cropbox));
    if (!Op(cropboxPath, path, kIntersect_SkPathOp, out)) {
        // This can fail if the PathOps encounter NaN or infinities.
        out->reset();
    }
    out->setIsVolatile(true);
}


GrCCPerOpListPaths::~GrCCPerOpListPaths() {
    // Ensure there are no surviving DrawPathsOps with a dangling pointer into this class.
    if (!fDrawOps.isEmpty()) {
        SK_ABORT("GrCCDrawPathsOp(s) not deleted during flush");
    }
    // Clip lazy proxies also reference this class from their callbacks, but those callbacks
    // are only invoked at flush time while we are still alive. (Unlike DrawPathsOps, that
    // unregister themselves upon destruction.) So it shouldn't matter if any clip proxies
    // are still around.
}

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    return shaderCaps.integerSupport() && shaderCaps.flatInterpolationSupport() &&
           caps.instanceAttribSupport() && GrCaps::kNone_MapFlags != caps.mapBufferFlags() &&
           caps.isConfigTexturable(kAlpha_half_GrPixelConfig) &&
           caps.isConfigRenderable(kAlpha_half_GrPixelConfig) &&
           !caps.blacklistCoverageCounting();
}

sk_sp<GrCoverageCountingPathRenderer> GrCoverageCountingPathRenderer::CreateIfSupported(
        const GrCaps& caps, bool drawCachablePaths) {
    auto ccpr = IsSupported(caps) ? new GrCoverageCountingPathRenderer(drawCachablePaths) : nullptr;
    return sk_sp<GrCoverageCountingPathRenderer>(ccpr);
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
    if (args.fShape->hasUnstyledKey() && !fDrawCachablePaths) {
        return CanDrawPath::kNo;
    }

    if (!args.fShape->style().isSimpleFill() || args.fShape->inverseFilled() ||
        args.fViewMatrix->hasPerspective() || GrAAType::kCoverage != args.fAAType) {
        return CanDrawPath::kNo;
    }

    SkPath path;
    args.fShape->asPath(&path);
    SkRect devBounds;
    SkIRect devIBounds;
    args.fViewMatrix->mapRect(&devBounds, path.getBounds());
    devBounds.roundOut(&devIBounds);
    if (!devIBounds.intersect(*args.fClipConservativeBounds)) {
        // Path is completely clipped away. Our code will eventually notice this before doing any
        // real work.
        return CanDrawPath::kYes;
    }

    if (devIBounds.height() * devIBounds.width() > 256 * 256) {
        // Large paths can blow up the atlas fast. And they are not ideal for a two-pass rendering
        // algorithm. Give the simpler direct renderers a chance before we commit to drawing it.
        return CanDrawPath::kAsBackup;
    }

    if (args.fShape->hasUnstyledKey() && path.countVerbs() > 50) {
        // Complex paths do better cached in an SDF, if the renderer will accept them.
        return CanDrawPath::kAsBackup;
    }

    return CanDrawPath::kYes;
}

bool GrCoverageCountingPathRenderer::onDrawPath(const DrawPathArgs& args) {
    SkASSERT(!fFlushing);

    SkIRect clipIBounds;
    GrRenderTargetContext* rtc = args.fRenderTargetContext;
    args.fClip->getConservativeBounds(rtc->width(), rtc->height(), &clipIBounds, nullptr);

    SkPath path;
    args.fShape->asPath(&path);

    SkRect devBounds;
    args.fViewMatrix->mapRect(&devBounds, path.getBounds());

    std::unique_ptr<GrCCDrawPathsOp> op;
    if (SkTMax(devBounds.height(), devBounds.width()) > kPathCropThreshold) {
        // The path is too large. Crop it or analytic AA can run out of fp32 precision.
        SkPath croppedPath;
        path.transform(*args.fViewMatrix, &croppedPath);
        crop_path(croppedPath, clipIBounds, &croppedPath);
        // FIXME: This breaks local coords: http://skbug.com/8003
        op = GrCCDrawPathsOp::Make(args.fContext, clipIBounds, SkMatrix::I(), croppedPath,
                                   croppedPath.getBounds(), std::move(args.fPaint));
    } else {
        op = GrCCDrawPathsOp::Make(args.fContext, clipIBounds, *args.fViewMatrix, path, devBounds,
                                   std::move(args.fPaint));
    }

    this->recordOp(std::move(op), args);
    return true;
}

void GrCoverageCountingPathRenderer::recordOp(std::unique_ptr<GrCCDrawPathsOp> opHolder,
                                              const DrawPathArgs& args) {
    if (GrCCDrawPathsOp* op = opHolder.get()) {
        GrRenderTargetContext* rtc = args.fRenderTargetContext;
        if (uint32_t opListID = rtc->addDrawOp(*args.fClip, std::move(opHolder))) {
            op->wasRecorded(this->lookupPendingPaths(opListID));
        }
    }
}

std::unique_ptr<GrFragmentProcessor> GrCoverageCountingPathRenderer::makeClipProcessor(
        GrProxyProvider* proxyProvider,
        uint32_t opListID, const SkPath& deviceSpacePath, const SkIRect& accessRect,
        int rtWidth, int rtHeight) {
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
            int maxRTSize = proxyProvider->caps()->maxRenderTargetSize();
            crop_path(deviceSpacePath, SkIRect::MakeWH(maxRTSize, maxRTSize), &croppedPath);
            clipPath.init(proxyProvider, croppedPath, accessRect, rtWidth, rtHeight);
        } else {
            clipPath.init(proxyProvider, deviceSpacePath, accessRect, rtWidth, rtHeight);
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
    SkASSERT(!fFlushing);
    SkASSERT(fFlushingPaths.empty());
    SkDEBUGCODE(fFlushing = true);

    if (fPendingPaths.empty()) {
        return;  // Nothing to draw.
    }

    GrCCPerFlushResourceSpecs resourceSpecs;
    int maxPreferredRTSize = onFlushRP->caps()->maxPreferredRenderTargetSize();
    resourceSpecs.fAtlasSpecs.fMaxPreferredTextureSize = maxPreferredRTSize;
    resourceSpecs.fAtlasSpecs.fMinTextureSize = SkTMin(1024, maxPreferredRTSize);

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

        for (const GrCCDrawPathsOp* op : fFlushingPaths.back()->fDrawOps) {
            op->accountForOwnPaths(&resourceSpecs);
        }
        for (const auto& clipsIter : fFlushingPaths.back()->fClipPaths) {
            clipsIter.second.accountForOwnPath(&resourceSpecs);
        }
    }

    if (resourceSpecs.isEmpty()) {
        return;  // Nothing to draw.
    }

    auto resources = sk_make_sp<GrCCPerFlushResources>(onFlushRP, resourceSpecs);
    if (!resources->isMapped()) {
        return;  // Some allocation failed.
    }

    // Layout atlas(es) and parse paths.
    SkDEBUGCODE(int numSkippedPaths = 0);
    for (const auto& flushingPaths : fFlushingPaths) {
        for (GrCCDrawPathsOp* op : flushingPaths->fDrawOps) {
            op->setupResources(resources.get(), onFlushRP);
            SkDEBUGCODE(numSkippedPaths += op->numSkippedInstances_debugOnly());
        }
        for (auto& clipsIter : flushingPaths->fClipPaths) {
            clipsIter.second.renderPathInAtlas(resources.get(), onFlushRP);
        }
    }
    SkASSERT(resources->nextPathInstanceIdx() == resourceSpecs.fNumRenderedPaths - numSkippedPaths);

    // Allocate the atlases and create instance buffers to draw them.
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

    // In DDL mode these aren't guaranteed to be deleted so we must clear out the perFlush
    // resources manually.
    for (auto& flushingPaths : fFlushingPaths) {
        flushingPaths->fFlushResources = nullptr;
    }

    // We wait to erase these until after flush, once Ops and FPs are done accessing their data.
    fFlushingPaths.reset();
    SkDEBUGCODE(fFlushing = false);
}
