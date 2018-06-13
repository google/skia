/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCAtlas.h"

#include "GrClip.h"
#include "GrMemoryPool.h"
#include "GrOnFlushResourceProvider.h"
#include "GrSurfaceContextPriv.h"
#include "GrRectanizer_skyline.h"
#include "GrRenderTargetContext.h"
#include "GrSurfaceContextPriv.h"
#include "GrTextureProxy.h"
#include "SkMakeUnique.h"
#include "SkMathPriv.h"
#include "ccpr/GrCCCoverageProcessor.h"
#include "ccpr/GrCCPathParser.h"
#include "ops/GrDrawOp.h"

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

class GrCCAtlas::DrawCoverageCountOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrContext* context,
                                          sk_sp<const GrCCPathParser> parser,
                                          CoverageCountBatchID batchID,
                                          const SkISize& drawBounds) {
        return std::unique_ptr<GrDrawOp>(new DrawCoverageCountOp(std::move(parser),
                                                                 batchID, drawBounds));
    }

    // GrDrawOp interface.
    const char* name() const override { return "GrCCAtlas::DrawCoverageCountOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                GrPixelConfigIsClamped) override { return RequiresDstTexture::kNo; }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState* flushState) override {
        fParser->drawCoverageCount(flushState, fBatchID,
                                   SkIRect::MakeWH(fDrawBounds.width(), fDrawBounds.height()));
    }

private:
    friend class GrOpMemoryPool; // for ctor

    DrawCoverageCountOp(sk_sp<const GrCCPathParser> parser, CoverageCountBatchID batchID,
                        const SkISize& drawBounds)
            : INHERITED(ClassID())
            , fParser(std::move(parser))
            , fBatchID(batchID)
            , fDrawBounds(drawBounds) {
        this->setBounds(SkRect::MakeIWH(fDrawBounds.width(), fDrawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
    }

    const sk_sp<const GrCCPathParser> fParser;
    const CoverageCountBatchID fBatchID;
    const SkISize fDrawBounds;

    typedef GrDrawOp INHERITED;
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

bool GrCCAtlas::addRect(int w, int h, SkIPoint16* loc) {
    // This can't be called anymore once setCoverageCountBatchID() has been called.
    SkASSERT(!fCoverageCountBatchID);
    SkASSERT(!fTextureProxy);

    if (!this->internalPlaceRect(w, h, loc)) {
        return false;
    }

    fDrawBounds.fWidth = SkTMax(fDrawBounds.width(), loc->x() + w);
    fDrawBounds.fHeight = SkTMax(fDrawBounds.height(), loc->y() + h);
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

sk_sp<GrRenderTargetContext> GrCCAtlas::finalize(GrOnFlushResourceProvider* onFlushRP,
                                                 sk_sp<const GrCCPathParser> parser) {
    SkASSERT(fCoverageCountBatchID);
    SkASSERT(!fTextureProxy);
    // Caller should have cropped any paths to the destination render target instead of asking for
    // an atlas larger than maxRenderTargetSize.
    SkASSERT(fMaxTextureSize <= onFlushRP->caps()->maxRenderTargetSize());

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = fWidth;
    desc.fHeight = fHeight;
    desc.fConfig = kAlpha_half_GrPixelConfig;
    sk_sp<GrRenderTargetContext> rtc =
            onFlushRP->makeRenderTargetContext(desc, kTopLeft_GrSurfaceOrigin, nullptr, nullptr);
    if (!rtc) {
        SkDebugf("WARNING: failed to allocate a %ix%i atlas. Some paths will not be drawn.\n",
                 fWidth, fHeight);
        return nullptr;
    }

    SkIRect clearRect = SkIRect::MakeSize(fDrawBounds);
    rtc->clear(&clearRect, 0, GrRenderTargetContext::CanClearFullscreen::kYes);

    GrContext* context = rtc->surfPriv().getContext();

    std::unique_ptr<GrDrawOp> op = DrawCoverageCountOp::Make(context,
                                                             std::move(parser),
                                                             fCoverageCountBatchID,
                                                             fDrawBounds);
    rtc->addDrawOp(GrNoClip(), std::move(op));

    fTextureProxy = sk_ref_sp(rtc->asTextureProxy());
    return rtc;
}
