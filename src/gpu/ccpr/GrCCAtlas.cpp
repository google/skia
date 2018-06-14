/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCAtlas.h"

#include "GrCaps.h"
#include "GrOnFlushResourceProvider.h"
#include "GrRectanizer_skyline.h"
#include "GrRenderTargetContext.h"
#include "GrTextureProxy.h"
#include "SkMakeUnique.h"
#include "SkMathPriv.h"

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

GrCCAtlas::GrCCAtlas(const Specs& specs)
        : fMaxTextureSize(SkTMax(SkTMax(specs.fMinHeight, specs.fMinWidth),
                                 specs.fMaxPreferredTextureSize)) {
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
}

GrCCAtlas::~GrCCAtlas() {
}

bool GrCCAtlas::addRect(const SkIRect& devIBounds, SkIVector* offset) {
    // This can't be called anymore once makeClearedTextureProxy() has been called.
    SkASSERT(!fTextureProxy);

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

sk_sp<GrRenderTargetContext> GrCCAtlas::initInternalTextureProxy(
        GrOnFlushResourceProvider* onFlushRP, GrPixelConfig config) {
    SkASSERT(!fTextureProxy);
    // Caller should have cropped any paths to the destination render target instead of asking for
    // an atlas larger than maxRenderTargetSize.
    SkASSERT(SkTMax(fHeight, fWidth) <= fMaxTextureSize);
    SkASSERT(fMaxTextureSize <= onFlushRP->caps()->maxRenderTargetSize());

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = fWidth;
    desc.fHeight = fHeight;
    desc.fConfig = config;
    sk_sp<GrRenderTargetContext> rtc =
            onFlushRP->makeRenderTargetContext(desc, kTextureOrigin, nullptr, nullptr);
    if (!rtc) {
        SkDebugf("WARNING: failed to allocate a %ix%i atlas. Some paths will not be drawn.\n",
                 fWidth, fHeight);
        return nullptr;
    }

    SkIRect clearRect = SkIRect::MakeSize(fDrawBounds);
    rtc->clear(&clearRect, 0, GrRenderTargetContext::CanClearFullscreen::kYes);

    fTextureProxy = sk_ref_sp(rtc->asTextureProxy());
    return rtc;
}

GrCCAtlas* GrCCAtlasStack::addRect(const SkIRect& devIBounds, SkIVector* offset) {
    GrCCAtlas* retiredAtlas = nullptr;
    if (fAtlases.empty() || !fAtlases.back().addRect(devIBounds, offset)) {
        // The retired atlas is out of room and can't grow any bigger.
        retiredAtlas = !fAtlases.empty() ? &fAtlases.back() : nullptr;
        fAtlases.emplace_back(fSpecs);
        SkASSERT(devIBounds.width() <= fSpecs.fMinWidth);
        SkASSERT(devIBounds.height() <= fSpecs.fMinHeight);
        SkAssertResult(fAtlases.back().addRect(devIBounds, offset));
    }
    return retiredAtlas;
}
