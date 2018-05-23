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

GrCCRTPendingPaths* GrCoverageCountingPathRenderer::lookupRTPendingPaths(uint32_t opListID) {
    auto it = fRTPendingPathsMap.find(opListID);
    if (fRTPendingPathsMap.end() == it) {
        it = fRTPendingPathsMap.insert(
                     std::make_pair(opListID, skstd::make_unique<GrCCRTPendingPaths>())).first;
    }
    return it->second.get();
}

void GrCoverageCountingPathRenderer::adoptAndRecordOp(GrCCDrawPathsOp* op,
                                                      const DrawPathArgs& args) {
    GrRenderTargetContext* rtc = args.fRenderTargetContext;
    if (uint32_t opListID = rtc->addDrawOp(*args.fClip, std::unique_ptr<GrDrawOp>(op))) {
        op->wasRecorded(this->lookupRTPendingPaths(opListID));
    }
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

    if (SkTMax(devBounds.height(), devBounds.width()) > kPathCropThreshold) {
        // The path is too large. Crop it or analytic AA can run out of fp32 precision.
        SkPath croppedPath;
        path.transform(*args.fViewMatrix, &croppedPath);
        crop_path(croppedPath, clipIBounds, &croppedPath);
        this->adoptAndRecordOp(new GrCCDrawPathsOp(std::move(args.fPaint), clipIBounds,
                                                   SkMatrix::I(), croppedPath,
                                                   croppedPath.getBounds()), args);
        return true;
    }

    this->adoptAndRecordOp(new GrCCDrawPathsOp(std::move(args.fPaint), clipIBounds,
                                               *args.fViewMatrix, path, devBounds), args);
    return true;
}

std::unique_ptr<GrFragmentProcessor> GrCoverageCountingPathRenderer::makeClipProcessor(
        GrProxyProvider* proxyProvider,
        uint32_t opListID, const SkPath& deviceSpacePath, const SkIRect& accessRect,
        int rtWidth, int rtHeight) {
    using MustCheckBounds = GrCCClipProcessor::MustCheckBounds;

    SkASSERT(!fFlushing);

    GrCCClipPath& clipPath =
            this->lookupRTPendingPaths(opListID)->fClipPaths[deviceSpacePath.getGenerationID()];
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
                                              SkTArray<sk_sp<GrRenderTargetContext>>* atlasDraws) {
    SkASSERT(!fFlushing);
    SkASSERT(fFlushingRTPaths.empty());
    SkDEBUGCODE(fFlushing = true);

    if (fRTPendingPathsMap.empty()) {
        return;  // Nothing to draw.
    }

    // Move the RT paths from fRTPendingPathsMap to fFlushingRTPaths, and count up the paths about
    // to be flushed so we can preallocate buffers.
    int numPathDraws = 0;
    int numClipPaths = 0;
    GrCCPathParser::PathStats flushingPathStats;
    fFlushingRTPaths.reserve(numOpListIDs);
    for (int i = 0; i < numOpListIDs; ++i) {
        auto iter = fRTPendingPathsMap.find(opListIDs[i]);
        if (fRTPendingPathsMap.end() == iter) {
            continue;  // No paths on this RT.
        }

        fFlushingRTPaths.push_back(std::move(iter->second)).get();
        fRTPendingPathsMap.erase(iter);

        for (const GrCCDrawPathsOp* op : fFlushingRTPaths.back()->fDrawOps) {
            numPathDraws += op->countPaths(&flushingPathStats);
        }
        for (const auto& clipsIter : fFlushingRTPaths.back()->fClipPaths) {
            flushingPathStats.statPath(clipsIter.second.deviceSpacePath());
        }
        numClipPaths += fFlushingRTPaths.back()->fClipPaths.size();
    }

    if (0 == numPathDraws + numClipPaths) {
        return;  // Nothing to draw.
    }

    auto resources = sk_make_sp<GrCCPerFlushResources>(onFlushRP, numPathDraws, numClipPaths,
                                                       flushingPathStats);
    if (!resources->isMapped()) {
        return;  // Some allocation failed.
    }

    // Layout atlas(es) and parse paths.
    SkDEBUGCODE(int numSkippedPaths = 0);
    for (const auto& flushingPaths : fFlushingRTPaths) {
        for (GrCCDrawPathsOp* op : flushingPaths->fDrawOps) {
            op->setupResources(resources.get(), onFlushRP);
            SkDEBUGCODE(numSkippedPaths += op->numSkippedInstances_debugOnly());
        }
        for (auto& clipsIter : flushingPaths->fClipPaths) {
            clipsIter.second.renderPathInAtlas(resources.get(), onFlushRP);
        }
    }
    SkASSERT(resources->nextPathInstanceIdx() == numPathDraws - numSkippedPaths);

    // Allocate the atlases and create instance buffers to draw them.
    if (!resources->finalize(onFlushRP, atlasDraws)) {
        return;
    }

    // Commit flushing paths to the resources once they are successfully completed.
    for (auto& flushingPaths : fFlushingRTPaths) {
        flushingPaths->fPerFlushResources = resources;
    }
}

void GrCoverageCountingPathRenderer::postFlush(GrDeferredUploadToken, const uint32_t* opListIDs,
                                               int numOpListIDs) {
    SkASSERT(fFlushing);
    // We wait to erase these until after flush, once Ops and FPs are done accessing their data.
    fFlushingRTPaths.reset();
    SkDEBUGCODE(fFlushing = false);
}
