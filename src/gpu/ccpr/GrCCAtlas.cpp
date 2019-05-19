/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCAtlas.h"

#include "include/gpu/GrTexture.h"
#include "include/private/GrTextureProxy.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRectanizer_skyline.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/ccpr/GrCCPathCache.h"
#include <atomic>

class GrCCAtlas::Node {
public:
    Node(std::unique_ptr<Node> previous, int l, int t, int r, int b)
            : fPrevious(std::move(previous)), fX(l), fY(t), fRectanizer(r - l, b - t) {}

    Node* previous() const { return fPrevious.get(); }

    bool addRect(int w, int h, SkIPoint16* loc, int maxAtlasSize) {
        // Pad all paths except those that are expected to take up an entire physical texture.
        if (w < maxAtlasSize) {
            w = SkTMin(w + kPadding, maxAtlasSize);
        }
        if (h < maxAtlasSize) {
            h = SkTMin(h + kPadding, maxAtlasSize);
        }
        if (!fRectanizer.addRect(w, h, loc)) {
            return false;
        }
        loc->fX += fX;
        loc->fY += fY;
        return true;
    }

private:
    const std::unique_ptr<Node> fPrevious;
    const int fX, fY;
    GrRectanizerSkyline fRectanizer;
};

GrCCAtlas::GrCCAtlas(CoverageType coverageType, const Specs& specs, const GrCaps& caps)
        : fCoverageType(coverageType)
        , fMaxTextureSize(SkTMax(SkTMax(specs.fMinHeight, specs.fMinWidth),
                                 specs.fMaxPreferredTextureSize)) {
    // Caller should have cropped any paths to the destination render target instead of asking for
    // an atlas larger than maxRenderTargetSize.
    SkASSERT(fMaxTextureSize <= caps.maxTextureSize());
    SkASSERT(specs.fMaxPreferredTextureSize > 0);

    // Begin with the first pow2 dimensions whose area is theoretically large enough to contain the
    // pending paths, favoring height over width if necessary.
    int log2area = SkNextLog2(SkTMax(specs.fApproxNumPixels, 1));
    fHeight = 1 << ((log2area + 1) / 2);
    fWidth = 1 << (log2area / 2);

    fWidth = SkTClamp(fWidth, specs.fMinTextureSize, specs.fMaxPreferredTextureSize);
    fHeight = SkTClamp(fHeight, specs.fMinTextureSize, specs.fMaxPreferredTextureSize);

    if (fWidth < specs.fMinWidth || fHeight < specs.fMinHeight) {
        // They want to stuff a particularly large path into the atlas. Just punt and go with their
        // min width and height. The atlas will grow as needed.
        fWidth = SkTMin(specs.fMinWidth + kPadding, fMaxTextureSize);
        fHeight = SkTMin(specs.fMinHeight + kPadding, fMaxTextureSize);
    }

    fTopNode = skstd::make_unique<Node>(nullptr, 0, 0, fWidth, fHeight);

    GrColorType colorType = (CoverageType::kFP16_CoverageCount == fCoverageType)
            ? GrColorType::kAlpha_F16 : GrColorType::kAlpha_8;
    const GrBackendFormat format =
            caps.getBackendFormatFromGrColorType(colorType, GrSRGBEncoded::kNo);
    GrPixelConfig pixelConfig = (CoverageType::kFP16_CoverageCount == fCoverageType)
            ? kAlpha_half_GrPixelConfig : kAlpha_8_GrPixelConfig;

    fTextureProxy = GrProxyProvider::MakeFullyLazyProxy(
            [this, pixelConfig](GrResourceProvider* resourceProvider) {
                    if (!fBackingTexture) {
                        GrSurfaceDesc desc;
                        desc.fFlags = kRenderTarget_GrSurfaceFlag;
                        desc.fWidth = fWidth;
                        desc.fHeight = fHeight;
                        desc.fConfig = pixelConfig;
                        fBackingTexture = resourceProvider->createTexture(
                            desc, SkBudgeted::kYes, GrResourceProvider::Flags::kNoPendingIO);
                    }
                    return GrSurfaceProxy::LazyInstantiationResult(fBackingTexture);
            },
            format, GrProxyProvider::Renderable::kYes, kTextureOrigin, pixelConfig, caps);

    fTextureProxy->priv().setIgnoredByResourceAllocator();
}

GrCCAtlas::~GrCCAtlas() {
}

bool GrCCAtlas::addRect(const SkIRect& devIBounds, SkIVector* offset) {
    // This can't be called anymore once makeRenderTargetContext() has been called.
    SkASSERT(!fTextureProxy->isInstantiated());

    SkIPoint16 location;
    if (!this->internalPlaceRect(devIBounds.width(), devIBounds.height(), &location)) {
        return false;
    }
    offset->set(location.x() - devIBounds.left(), location.y() - devIBounds.top());

    fDrawBounds.fWidth = SkTMax(fDrawBounds.width(), location.x() + devIBounds.width());
    fDrawBounds.fHeight = SkTMax(fDrawBounds.height(), location.y() + devIBounds.height());
    return true;
}

bool GrCCAtlas::internalPlaceRect(int w, int h, SkIPoint16* loc) {
    for (Node* node = fTopNode.get(); node; node = node->previous()) {
        if (node->addRect(w, h, loc, fMaxTextureSize)) {
            return true;
        }
    }

    // The rect didn't fit. Grow the atlas and try again.
    do {
        if (fWidth == fMaxTextureSize && fHeight == fMaxTextureSize) {
            return false;
        }
        if (fHeight <= fWidth) {
            int top = fHeight;
            fHeight = SkTMin(fHeight * 2, fMaxTextureSize);
            fTopNode = skstd::make_unique<Node>(std::move(fTopNode), 0, top, fWidth, fHeight);
        } else {
            int left = fWidth;
            fWidth = SkTMin(fWidth * 2, fMaxTextureSize);
            fTopNode = skstd::make_unique<Node>(std::move(fTopNode), left, 0, fWidth, fHeight);
        }
    } while (!fTopNode->addRect(w, h, loc, fMaxTextureSize));

    return true;
}

void GrCCAtlas::setFillBatchID(int id) {
    // This can't be called anymore once makeRenderTargetContext() has been called.
    SkASSERT(!fTextureProxy->isInstantiated());
    fFillBatchID = id;
}

void GrCCAtlas::setStrokeBatchID(int id) {
    // This can't be called anymore once makeRenderTargetContext() has been called.
    SkASSERT(!fTextureProxy->isInstantiated());
    fStrokeBatchID = id;
}

static uint32_t next_atlas_unique_id() {
    static std::atomic<uint32_t> nextID;
    return nextID++;
}

sk_sp<GrCCCachedAtlas> GrCCAtlas::refOrMakeCachedAtlas(GrOnFlushResourceProvider* onFlushRP) {
    if (!fCachedAtlas) {
        static const GrUniqueKey::Domain kAtlasDomain = GrUniqueKey::GenerateDomain();

        GrUniqueKey atlasUniqueKey;
        GrUniqueKey::Builder builder(&atlasUniqueKey, kAtlasDomain, 1, "CCPR Atlas");
        builder[0] = next_atlas_unique_id();
        builder.finish();

        onFlushRP->assignUniqueKeyToProxy(atlasUniqueKey, fTextureProxy.get());

        fCachedAtlas = sk_make_sp<GrCCCachedAtlas>(fCoverageType, atlasUniqueKey, fTextureProxy);
    }

    SkASSERT(fCachedAtlas->coverageType() == fCoverageType);
    SkASSERT(fCachedAtlas->getOnFlushProxy() == fTextureProxy.get());
    return fCachedAtlas;
}

sk_sp<GrRenderTargetContext> GrCCAtlas::makeRenderTargetContext(
        GrOnFlushResourceProvider* onFlushRP, sk_sp<GrTexture> backingTexture) {
    SkASSERT(!fTextureProxy->isInstantiated());  // This method should only be called once.
    // Caller should have cropped any paths to the destination render target instead of asking for
    // an atlas larger than maxRenderTargetSize.
    SkASSERT(SkTMax(fHeight, fWidth) <= fMaxTextureSize);
    SkASSERT(fMaxTextureSize <= onFlushRP->caps()->maxRenderTargetSize());

    // Finalize the content size of our proxy. The GPU can potentially make optimizations if it
    // knows we only intend to write out a smaller sub-rectangle of the backing texture.
    fTextureProxy->priv().setLazySize(fDrawBounds.width(), fDrawBounds.height());

    if (backingTexture) {
        SkASSERT(backingTexture->config() == kAlpha_half_GrPixelConfig);
        SkASSERT(backingTexture->width() == fWidth);
        SkASSERT(backingTexture->height() == fHeight);
        fBackingTexture = std::move(backingTexture);
    }

    sk_sp<GrRenderTargetContext> rtc =
            onFlushRP->makeRenderTargetContext(fTextureProxy, nullptr, nullptr);
    if (!rtc) {
        SkDebugf("WARNING: failed to allocate a %ix%i atlas. Some paths will not be drawn.\n",
                 fWidth, fHeight);
        return nullptr;
    }

    SkIRect clearRect = SkIRect::MakeSize(fDrawBounds);
    rtc->clear(&clearRect, SK_PMColor4fTRANSPARENT,
               GrRenderTargetContext::CanClearFullscreen::kYes);
    return rtc;
}

GrCCAtlas* GrCCAtlasStack::addRect(const SkIRect& devIBounds, SkIVector* devToAtlasOffset) {
    GrCCAtlas* retiredAtlas = nullptr;
    if (fAtlases.empty() || !fAtlases.back().addRect(devIBounds, devToAtlasOffset)) {
        // The retired atlas is out of room and can't grow any bigger.
        retiredAtlas = !fAtlases.empty() ? &fAtlases.back() : nullptr;
        fAtlases.emplace_back(fCoverageType, fSpecs, *fCaps);
        SkASSERT(devIBounds.width() <= fSpecs.fMinWidth);
        SkASSERT(devIBounds.height() <= fSpecs.fMinHeight);
        SkAssertResult(fAtlases.back().addRect(devIBounds, devToAtlasOffset));
    }
    return retiredAtlas;
}
