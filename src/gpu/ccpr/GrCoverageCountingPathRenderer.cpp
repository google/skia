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

GrFPResult GrCoverageCountingPathRenderer::makeClipProcessor(
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

    if (deviceSpacePath.isEmpty() ||
        !SkIRect::Intersects(accessRect, deviceSpacePath.getBounds().roundOut())) {
        return GrFPFailure(nullptr);  // Totally clipped out.
    }

    uint32_t key = deviceSpacePath.getGenerationID();
    key = (key << 1) | (uint32_t)GrFillRuleForSkPath(deviceSpacePath);
    GrCCClipPath& clipPath =
            this->lookupPendingPaths(opsTaskID)->fClipPaths[key];
    if (!clipPath.isInitialized()) {
        // This ClipPath was just created during lookup. Initialize it.
        clipPath.init(deviceSpacePath, accessRect, caps);
    } else {
        clipPath.addAccess(accessRect);
    }

    auto mustCheckBounds = GrCCClipProcessor::MustCheckBounds(
            !clipPath.pathDevIBounds().contains(accessRect));
    return GrFPSuccess(std::make_unique<GrCCClipProcessor>(std::move(inputFP), caps, &clipPath,
                                                           mustCheckBounds));
}

namespace {

// Iterates all clip paths in an array of non-empty maps.
class ClipPathsPathsIter {
public:
    ClipPathsPathsIter(sk_sp<GrCCPerOpsTaskPaths>* opsTaskIter) : fOpsTaskIter(opsTaskIter) {}

    bool operator!=(const ClipPathsPathsIter& that) {
        if (fOpsTaskIter != that.fOpsTaskIter) {
            return true;
        }
        if (fMap != that.fMap) {  // fMap will be null when we are on the first element.
            return true;
        }
        return fMap && fMapIter != that.fMapIter;
    }

    void operator++() {
        if (!fMap) {
            fMap = &(*fOpsTaskIter)->fClipPaths;
            SkASSERT(!fMap->empty());  // We don't handle empty lists.
            fMapIter = fMap->begin();
        }
        if (++fMapIter == fMap->end()) {
            fMap = nullptr;
            ++fOpsTaskIter;
        }
    }

    GrCCClipPath* operator->() {
        auto it = (fMap) ? fMapIter : (*fOpsTaskIter)->fClipPaths.begin();
        return &(it->second);
    }

private:
    sk_sp<GrCCPerOpsTaskPaths>* fOpsTaskIter;
    std::map<uint32_t, GrCCClipPath>* fMap = nullptr;
    std::map<uint32_t, GrCCClipPath>::iterator fMapIter;
};

}  // namespace

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
    ClipPathsPathsIter it(fFlushingPaths.begin());
    ClipPathsPathsIter end(fFlushingPaths.end());
    auto startOfCurrentAtlas = it;
    for (; it != end; ++it) {
        if (const GrCCAtlas* retiredAtlas =
                it->renderPathInAtlas(fPerFlushResources.get(), onFlushRP)) {
            if (GrTexture* atlasTexture = retiredAtlas->textureProxy()->peekTexture()) {
                for (; startOfCurrentAtlas != it; ++startOfCurrentAtlas) {
                    startOfCurrentAtlas->assignAtlasTexture(sk_ref_sp(atlasTexture));
                }
            }
        }
    }

    // Allocate resources and then render the atlas(es).
    const GrCCAtlas* atlas = fPerFlushResources->finalize(onFlushRP);
    if (GrTexture* atlasTexture = atlas->textureProxy()->peekTexture()) {
        for (; startOfCurrentAtlas != it; ++startOfCurrentAtlas) {
            startOfCurrentAtlas->assignAtlasTexture(sk_ref_sp(atlasTexture));
        }
    }
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
