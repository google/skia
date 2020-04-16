/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDynamicAtlas.h"

#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRectanizerSkyline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetContext.h"

// Each Node covers a sub-rectangle of the final atlas. When a GrDynamicAtlas runs out of room, we
// create a new Node the same size as all combined nodes in the atlas as-is, and then place the new
// Node immediately below or beside the others (thereby doubling the size of the GyDynamicAtlas).
class GrDynamicAtlas::Node {
public:
    Node(std::unique_ptr<Node> previous, int l, int t, int r, int b)
            : fPrevious(std::move(previous)), fX(l), fY(t), fRectanizer(r - l, b - t) {}

    Node* previous() const { return fPrevious.get(); }

    bool addRect(int w, int h, SkIPoint16* loc) {
        // Pad all paths except those that are expected to take up an entire physical texture.
        if (w < fRectanizer.width()) {
            w = std::min(w + kPadding, fRectanizer.width());
        }
        if (h < fRectanizer.height()) {
            h = std::min(h + kPadding, fRectanizer.height());
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

sk_sp<GrTextureProxy> GrDynamicAtlas::MakeLazyAtlasProxy(
        const LazyInstantiateAtlasCallback& callback, GrColorType colorType,
        InternalMultisample internalMultisample, const GrCaps& caps,
        GrSurfaceProxy::UseAllocator useAllocator) {
    GrBackendFormat format = caps.getDefaultBackendFormat(colorType, GrRenderable::kYes);

    int sampleCount = 1;
    if (!caps.mixedSamplesSupport() && InternalMultisample::kYes == internalMultisample) {
        sampleCount = caps.internalMultisampleCount(format);
    }

    auto instantiate = [cb = std::move(callback), format, sampleCount](GrResourceProvider* rp) {
        return cb(rp, format, sampleCount);
    };

    sk_sp<GrTextureProxy> proxy =
            GrProxyProvider::MakeFullyLazyProxy(std::move(instantiate), format, GrRenderable::kYes,
                                                sampleCount, GrProtected::kNo, caps, useAllocator);

    return proxy;
}

GrDynamicAtlas::GrDynamicAtlas(GrColorType colorType, InternalMultisample internalMultisample,
                               SkISize initialSize, int maxAtlasSize, const GrCaps& caps)
        : fColorType(colorType)
        , fInternalMultisample(internalMultisample)
        , fMaxAtlasSize(maxAtlasSize) {
    SkASSERT(fMaxAtlasSize <= caps.maxTextureSize());
    this->reset(initialSize, caps);
}

GrDynamicAtlas::~GrDynamicAtlas() {
}

void GrDynamicAtlas::reset(SkISize initialSize, const GrCaps& caps) {
    fWidth = std::min(SkNextPow2(initialSize.width()), fMaxAtlasSize);
    fHeight = std::min(SkNextPow2(initialSize.height()), fMaxAtlasSize);
    fTopNode = nullptr;
    fDrawBounds.setEmpty();
    fTextureProxy = MakeLazyAtlasProxy(
            [this](GrResourceProvider* resourceProvider, const GrBackendFormat& format,
                   int sampleCount) {
                if (!fBackingTexture) {
                    fBackingTexture = resourceProvider->createTexture(
                            {fWidth, fHeight}, format, GrRenderable::kYes, sampleCount,
                            GrMipMapped::kNo, SkBudgeted::kYes, GrProtected::kNo);
                }
                return GrSurfaceProxy::LazyCallbackResult(fBackingTexture);
            },
            fColorType, fInternalMultisample, caps, GrSurfaceProxy::UseAllocator::kNo);
    fBackingTexture = nullptr;
}

bool GrDynamicAtlas::addRect(const SkIRect& devIBounds, SkIVector* offset) {
    // This can't be called anymore once instantiate() has been called.
    SkASSERT(!this->isInstantiated());

    SkIPoint16 location;
    if (!this->internalPlaceRect(devIBounds.width(), devIBounds.height(), &location)) {
        return false;
    }
    offset->set(location.x() - devIBounds.left(), location.y() - devIBounds.top());

    fDrawBounds.fWidth = std::max(fDrawBounds.width(), location.x() + devIBounds.width());
    fDrawBounds.fHeight = std::max(fDrawBounds.height(), location.y() + devIBounds.height());
    return true;
}

bool GrDynamicAtlas::internalPlaceRect(int w, int h, SkIPoint16* loc) {
    if (std::max(h, w) > fMaxAtlasSize) {
        return false;
    }
    if (std::min(h, w) <= 0) {
        loc->set(0, 0);
        return true;
    }

    if (!fTopNode) {
        if (w > fWidth) {
            fWidth = std::min(SkNextPow2(w), fMaxAtlasSize);
        }
        if (h > fHeight) {
            fHeight = std::min(SkNextPow2(h), fMaxAtlasSize);
        }
        fTopNode = std::make_unique<Node>(nullptr, 0, 0, fWidth, fHeight);
    }

    for (Node* node = fTopNode.get(); node; node = node->previous()) {
        if (node->addRect(w, h, loc)) {
            return true;
        }
    }

    // The rect didn't fit. Grow the atlas and try again.
    do {
        if (fWidth >= fMaxAtlasSize && fHeight >= fMaxAtlasSize) {
            return false;
        }
        if (fHeight <= fWidth) {
            int top = fHeight;
            fHeight = std::min(fHeight * 2, fMaxAtlasSize);
            fTopNode = std::make_unique<Node>(std::move(fTopNode), 0, top, fWidth, fHeight);
        } else {
            int left = fWidth;
            fWidth = std::min(fWidth * 2, fMaxAtlasSize);
            fTopNode = std::make_unique<Node>(std::move(fTopNode), left, 0, fWidth, fHeight);
        }
    } while (!fTopNode->addRect(w, h, loc));

    return true;
}

std::unique_ptr<GrRenderTargetContext> GrDynamicAtlas::instantiate(
        GrOnFlushResourceProvider* onFlushRP, sk_sp<GrTexture> backingTexture) {
    SkASSERT(!this->isInstantiated());  // This method should only be called once.
    // Caller should have cropped any paths to the destination render target instead of asking for
    // an atlas larger than maxRenderTargetSize.
    SkASSERT(std::max(fHeight, fWidth) <= fMaxAtlasSize);
    SkASSERT(fMaxAtlasSize <= onFlushRP->caps()->maxRenderTargetSize());

    // Finalize the content size of our proxy. The GPU can potentially make optimizations if it
    // knows we only intend to write out a smaller sub-rectangle of the backing texture.
    fTextureProxy->priv().setLazyDimensions(fDrawBounds);

    if (backingTexture) {
#ifdef SK_DEBUG
        auto backingRT = backingTexture->asRenderTarget();
        SkASSERT(backingRT);
        SkASSERT(backingRT->backendFormat() == fTextureProxy->backendFormat());
        SkASSERT(backingRT->numSamples() == fTextureProxy->asRenderTargetProxy()->numSamples());
        SkASSERT(backingRT->width() == fWidth);
        SkASSERT(backingRT->height() == fHeight);
#endif
        fBackingTexture = std::move(backingTexture);
    }
    auto rtc = onFlushRP->makeRenderTargetContext(fTextureProxy, kTextureOrigin, fColorType,
                                                  nullptr, nullptr);
    if (!rtc) {
        onFlushRP->printWarningMessage(SkStringPrintf(
                "WARNING: failed to allocate a %ix%i atlas. Some masks will not be drawn.\n",
                fWidth, fHeight).c_str());
        return nullptr;
    }

    SkIRect clearRect = SkIRect::MakeSize(fDrawBounds);
    rtc->clear(&clearRect, SK_PMColor4fTRANSPARENT,
               GrRenderTargetContext::CanClearFullscreen::kYes);
    return rtc;
}
