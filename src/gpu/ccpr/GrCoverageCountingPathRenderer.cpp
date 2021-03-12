/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include <memory>

#include "include/pathops/SkPathOps.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ccpr/GrCCClipProcessor.h"

bool GrCoverageCountingPathRenderer::IsSupported(const GrCaps& caps) {
    const GrShaderCaps& shaderCaps = *caps.shaderCaps();
    GrBackendFormat defaultA8Format = caps.getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                                   GrRenderable::kYes);
    if (caps.driverDisableMSAAClipAtlas() || !shaderCaps.integerSupport() ||
        !caps.drawInstancedSupport() || !shaderCaps.floatIs32Bits() ||
        !defaultA8Format.isValid() || // This checks both texturable and renderable
        !caps.halfFloatVertexAttributeSupport()) {
        return false;
    }

    if (caps.internalMultisampleCount(defaultA8Format) > 1 &&
        caps.sampleLocationsSupport() &&
        shaderCaps.sampleMaskSupport()) {
        return true;
    }

    return false;
}

std::unique_ptr<GrCoverageCountingPathRenderer> GrCoverageCountingPathRenderer::CreateIfSupported(
        const GrCaps& caps) {
    if (IsSupported(caps)) {
        return std::make_unique<GrCoverageCountingPathRenderer>();
    }
    return nullptr;
}

GrCCPerOpsTaskPaths* GrCoverageCountingPathRenderer::lookupPendingPaths(uint32_t opsTaskID) {
    auto it = fPendingPaths.find(opsTaskID);
    if (fPendingPaths.end() == it) {
        sk_sp<GrCCPerOpsTaskPaths> paths = sk_make_sp<GrCCPerOpsTaskPaths>();
        it = fPendingPaths.insert(std::make_pair(opsTaskID, std::move(paths))).first;
    }
    return it->second.get();
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
    key = (key << 1) | (uint32_t)GrFillRuleForSkPath(deviceSpacePath);
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
            clipPath.init(croppedPath, accessRect, caps);
        } else {
            clipPath.init(deviceSpacePath, accessRect, caps);
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
    SkASSERT(!fFlushing);
    SkASSERT(fFlushingPaths.empty());
    SkDEBUGCODE(fFlushing = true);

    if (fPendingPaths.empty()) {
        return;  // Nothing to draw.
    }

    GrCCAtlas::Specs specs;
    int maxPreferredRTSize = onFlushRP->caps()->maxPreferredRenderTargetSize();
    specs.fMaxPreferredTextureSize = maxPreferredRTSize;
    specs.fMinTextureSize = std::min(512, maxPreferredRTSize);

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

        for (const auto& clipsIter : fFlushingPaths.back()->fClipPaths) {
            clipsIter.second.accountForOwnPath(&specs);
        }
    }

    fPerFlushResources = std::make_unique<GrCCPerFlushResources>(onFlushRP, specs);

    // Layout the atlas(es) and render paths.
    for (const auto& flushingPaths : fFlushingPaths) {
        for (auto& clipsIter : flushingPaths->fClipPaths) {
            clipsIter.second.renderPathInAtlas(fPerFlushResources.get(), onFlushRP);
        }
    }

    // Allocate resources and then render the atlas(es).
    fPerFlushResources->finalize(onFlushRP);
}

void GrCoverageCountingPathRenderer::postFlush(GrDeferredUploadToken,
                                               SkSpan<const uint32_t> /* taskIDs */) {
    SkASSERT(fFlushing);

    fPerFlushResources.reset();

    if (!fFlushingPaths.empty()) {
        // We wait to erase these until after flush, once Ops and FPs are done accessing their data.
        fFlushingPaths.reset();
    }

    SkDEBUGCODE(fFlushing = false);
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
