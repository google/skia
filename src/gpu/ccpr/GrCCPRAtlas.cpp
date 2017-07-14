/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRAtlas.h"

#include "GrOnFlushResourceProvider.h"
#include "GrClip.h"
#include "GrRectanizer_skyline.h"
#include "GrTextureProxy.h"
#include "GrRenderTargetContext.h"
#include "SkMakeUnique.h"
#include "SkMathPriv.h"
#include "ccpr/GrCCPRCoverageProcessor.h"
#include "ops/GrDrawOp.h"

class GrCCPRAtlas::Node {
public:
    Node(std::unique_ptr<Node> previous, int l, int t, int r, int b)
            : fPrevious(std::move(previous))
            , fX(l), fY(t)
            , fRectanizer(r - l, b - t) {}

    Node* previous() const { return fPrevious.get(); }

    bool addRect(int w, int h, SkIPoint16* loc) {
        static constexpr int kPad = 1;

        if (!fRectanizer.addRect(w + kPad, h + kPad, loc)) {
            return false;
        }
        loc->fX += fX;
        loc->fY += fY;
        return true;
    }

private:
    const std::unique_ptr<Node>   fPrevious;
    const int                     fX, fY;
    GrRectanizerSkyline           fRectanizer;
};

GrCCPRAtlas::GrCCPRAtlas(const GrCaps& caps, int minWidth, int minHeight)
        : fMaxAtlasSize(caps.maxRenderTargetSize())
        , fDrawBounds{0, 0} {
    SkASSERT(fMaxAtlasSize <= caps.maxTextureSize());
    SkASSERT(SkTMax(minWidth, minHeight) <= fMaxAtlasSize);
    int initialSize = GrNextPow2(SkTMax(minWidth, minHeight));
    initialSize = SkTMax(int(kMinSize), initialSize);
    initialSize = SkTMin(initialSize, fMaxAtlasSize);
    fHeight = fWidth = initialSize;
    fTopNode = skstd::make_unique<Node>(nullptr, 0, 0, initialSize, initialSize);
}

GrCCPRAtlas::~GrCCPRAtlas() {
}

bool GrCCPRAtlas::addRect(int w, int h, SkIPoint16* loc) {
    // This can't be called anymore once finalize() has been called.
    SkASSERT(!fTextureProxy);

    if (!this->internalPlaceRect(w, h, loc)) {
        return false;
    }

    fDrawBounds.fWidth = SkTMax(fDrawBounds.width(), loc->x() + w);
    fDrawBounds.fHeight = SkTMax(fDrawBounds.height(), loc->y() + h);
    return true;
}

bool GrCCPRAtlas::internalPlaceRect(int w, int h, SkIPoint16* loc) {
    SkASSERT(SkTMax(w, h) < fMaxAtlasSize);

    for (Node* node = fTopNode.get(); node; node = node->previous()) {
        if (node->addRect(w, h, loc)) {
            return true;
        }
    }

    // The rect didn't fit. Grow the atlas and try again.
    do {
        SkASSERT(SkTMax(fWidth, fHeight) <= fMaxAtlasSize);
        if (fWidth == fMaxAtlasSize && fHeight == fMaxAtlasSize) {
            return false;
        }
        if (fHeight <= fWidth) {
            int top = fHeight;
            fHeight = SkTMin(fHeight * 2, fMaxAtlasSize);
            fTopNode = skstd::make_unique<Node>(std::move(fTopNode), 0, top, fWidth, fHeight);
        } else {
            int left = fWidth;
            fWidth = SkTMin(fWidth * 2, fMaxAtlasSize);
            fTopNode = skstd::make_unique<Node>(std::move(fTopNode), left, 0, fWidth, fHeight);
        }
    } while (!fTopNode->addRect(w, h, loc));

    return true;
}

sk_sp<GrRenderTargetContext> GrCCPRAtlas::finalize(GrOnFlushResourceProvider* onFlushRP,
                                                     std::unique_ptr<GrDrawOp> atlasOp) {
    SkASSERT(!fTextureProxy);

    GrSurfaceDesc desc;
    desc.fOrigin = GrCCPRCoverageProcessor::kAtlasOrigin;
    desc.fWidth = fWidth;
    desc.fHeight = fHeight;
    desc.fConfig = kAlpha_half_GrPixelConfig;
    sk_sp<GrRenderTargetContext> rtc = onFlushRP->makeRenderTargetContext(desc, nullptr, nullptr);
    if (!rtc) {
        SkDebugf("WARNING: failed to allocate a %ix%i atlas. Some paths will not be drawn.\n",
                 fWidth, fHeight);
        return nullptr;
    }

    SkIRect clearRect = SkIRect::MakeSize(fDrawBounds);
    rtc->clear(&clearRect, 0, true);
    rtc->addDrawOp(GrNoClip(), std::move(atlasOp));

    fTextureProxy = sk_ref_sp(rtc->asTextureProxy());
    return rtc;
}
