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
        // "Intersect" draws that don't intersect the clip can be dropped.
        return deviceSpacePath.isInverseFillType() ? GrFPSuccess(nullptr) : GrFPFailure(nullptr);
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

// Iterates all GrCCClipPaths in an array of non-empty maps.
class ClipMapsIter {
public:
    ClipMapsIter(sk_sp<GrCCPerOpsTaskPaths>* mapsList) : fMapsList(mapsList) {}

    bool operator!=(const ClipMapsIter& that) {
        if (fMapsList != that.fMapsList) {
            return true;
        }
        // fPerOpsTaskClipPaths will be null when we are on the first element.
        if (fPerOpsTaskClipPaths != that.fPerOpsTaskClipPaths) {
            return true;
        }
        return fPerOpsTaskClipPaths && fClipPathsIter != that.fClipPathsIter;
    }

    void operator++() {
        // fPerOpsTaskClipPaths is null when we are on the first element.
        if (!fPerOpsTaskClipPaths) {
            fPerOpsTaskClipPaths = &(*fMapsList)->fClipPaths;
            SkASSERT(!fPerOpsTaskClipPaths->empty());  // We don't handle empty lists.
            fClipPathsIter = fPerOpsTaskClipPaths->begin();
        }
        if ((++fClipPathsIter) == fPerOpsTaskClipPaths->end()) {
            ++fMapsList;
            fPerOpsTaskClipPaths = nullptr;
        }
    }

    GrCCClipPath* operator->() {
        // fPerOpsTaskClipPaths is null when we are on the first element.
        const auto& it = (!fPerOpsTaskClipPaths) ? (*fMapsList)->fClipPaths.begin()
                                                 : fClipPathsIter;
        return &(it->second);
    }

private:
    sk_sp<GrCCPerOpsTaskPaths>* fMapsList;
    std::map<uint32_t, GrCCClipPath>* fPerOpsTaskClipPaths = nullptr;
    std::map<uint32_t, GrCCClipPath>::iterator fClipPathsIter;
};

}  // namespace

static void assign_atlas_textures(GrTexture* atlasTexture, ClipMapsIter nextPathToAssign,
                                  const ClipMapsIter& end) {
    if (!atlasTexture) {
        return;
    }
    for (; nextPathToAssign != end; ++nextPathToAssign) {
        nextPathToAssign->assignAtlasTexture(sk_ref_sp(atlasTexture));
    }
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
    ClipMapsIter it(fFlushingPaths.begin());
    ClipMapsIter end(fFlushingPaths.end());
    ClipMapsIter nextPathToAssign = it;  // The next GrCCClipPath to call assignAtlasTexture on.
    for (; it != end; ++it) {
        if (const GrCCAtlas* retiredAtlas =
                it->renderPathInAtlas(fPerFlushResources.get(), onFlushRP)) {
            assign_atlas_textures(retiredAtlas->textureProxy()->peekTexture(), nextPathToAssign,
                                  it);
            nextPathToAssign = it;
        }
    }

    // Allocate resources and then render the atlas(es).
    const GrCCAtlas* atlas = fPerFlushResources->finalize(onFlushRP);
    assign_atlas_textures(atlas->textureProxy()->peekTexture(), nextPathToAssign, end);
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
