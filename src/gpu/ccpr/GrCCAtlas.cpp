/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCAtlas.h"

#include "GrClip.h"
#include "GrOnFlushResourceProvider.h"
#include "GrRectanizer_skyline.h"
#include "GrRenderTargetContext.h"
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
    const std::unique_ptr<Node> fPrevious;
    const int fX, fY;
    GrRectanizerSkyline fRectanizer;
};

class GrCCAtlas::DrawCoverageCountOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    DrawCoverageCountOp(sk_sp<const GrCCPathParser> parser, CoverageCountBatchID batchID,
                        const SkISize& drawBounds)
            : INHERITED(ClassID())
            , fParser(std::move(parser))
            , fBatchID(batchID)
            , fDrawBounds(drawBounds) {
        this->setBounds(SkRect::MakeIWH(fDrawBounds.width(), fDrawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
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
    const sk_sp<const GrCCPathParser> fParser;
    const CoverageCountBatchID fBatchID;
    const SkISize fDrawBounds;

    typedef GrDrawOp INHERITED;
};

GrCCAtlas::GrCCAtlas(const GrCaps& caps, int minWidth, int minHeight)
        : fMaxAtlasSize(caps.maxRenderTargetSize()), fDrawBounds{0, 0} {
    SkASSERT(fMaxAtlasSize <= caps.maxTextureSize());
    SkASSERT(SkTMax(minWidth, minHeight) <= fMaxAtlasSize);
    int initialSize = GrNextPow2(SkTMax(minWidth, minHeight));
    initialSize = SkTMax(int(kMinSize), initialSize);
    initialSize = SkTMin(initialSize, fMaxAtlasSize);
    fHeight = fWidth = initialSize;
    fTopNode = skstd::make_unique<Node>(nullptr, 0, 0, initialSize, initialSize);
}

GrCCAtlas::~GrCCAtlas() {}

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

sk_sp<GrRenderTargetContext> GrCCAtlas::finalize(GrOnFlushResourceProvider* onFlushRP,
                                                 sk_sp<const GrCCPathParser> parser) {
    SkASSERT(fCoverageCountBatchID);
    SkASSERT(!fTextureProxy);

    GrSurfaceDesc desc;
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
    rtc->clear(&clearRect, 0, GrRenderTargetContext::CanClearFullscreen::kYes);

    auto op = skstd::make_unique<DrawCoverageCountOp>(std::move(parser), fCoverageCountBatchID,
                                                      fDrawBounds);
    rtc->addDrawOp(GrNoClip(), std::move(op));

    fTextureProxy = sk_ref_sp(rtc->asTextureProxy());
    return rtc;
}
