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
#include "ccpr/GrCCPathParser.h"
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

class GrCCPRAtlas::DrawOp : public GrDrawOp {
public:
    DEFINE_OP_CLASS_ID

    DrawOp(GrCCPRAtlas* atlas, sk_sp<const GrCCPathParser> parser)
        : INHERITED(ClassID())
        , fAtlas(atlas)
        , fParser(std::move(parser)) {
        this->setBounds(SkRect::MakeIWH(fAtlas->fDrawBounds.width(), fAtlas->fDrawBounds.height()),
                        GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
    }

    // GrDrawOp interface.
    const char* name() const override { return "CCAtlasOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*,
                                GrPixelConfigIsClamped) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState*) override;

private:
    GrCCPRAtlas* const fAtlas;
    const sk_sp<const GrCCPathParser> fParser;

    friend class GrCCPRCoverageOpsBuilder;

    typedef GrDrawOp INHERITED;
};

GrCCPRAtlas::GrCCPRAtlas(const GrCaps& caps, int minWidth, int minHeight,
                         const PrimitiveTallies instanceStartIndices[kNumScissorModes])
        : fMaxAtlasSize(caps.maxRenderTargetSize())
        , fDrawBounds{0, 0}
        , fInstanceStartIndices{instanceStartIndices[0], instanceStartIndices[1]} {
    SkASSERT(fMaxAtlasSize <= caps.maxTextureSize());
    SkASSERT(SkTMax(minWidth, minHeight) <= fMaxAtlasSize);
    int initialSize = GrNextPow2(SkTMax(minWidth, minHeight));
    initialSize = SkTMax(int(kMinSize), initialSize);
    initialSize = SkTMin(initialSize, fMaxAtlasSize);
    fHeight = fWidth = initialSize;
    fTopNode = skstd::make_unique<Node>(nullptr, 0, 0, initialSize, initialSize);
}

GrCCPRAtlas::~GrCCPRAtlas() {}

bool GrCCPRAtlas::placeParsedPath(ScissorMode scissorMode, const SkIRect& clippedPathIBounds,
                                  int16_t* atlasOffsetX, int16_t* atlasOffsetY, 
                                  GrCCPathParser* parser) {
    // This can't be called anymore once finalize() have been called.
    SkASSERT(!fTextureProxy);

    SkIPoint16 location;
    if (!this->internalPlaceRect(clippedPathIBounds.width(), clippedPathIBounds.height(),
                                 &location)) {
        return false;
    }

    *atlasOffsetX = location.x() - static_cast<int16_t>(clippedPathIBounds.left());
    *atlasOffsetY = location.y() - static_cast<int16_t>(clippedPathIBounds.top());

    const PrimitiveTallies& instanceCounts = 
            parser->saveParsedPath(scissorMode, clippedPathIBounds, *atlasOffsetX, *atlasOffsetY);
    if (GrCCPathParser::ScissorMode::kNonScissored == scissorMode) {
        fUnscissoredInstanceCounts += instanceCounts;
    } else {
        fScissorBatches.emplace_back(instanceCounts,
                                     clippedPathIBounds.makeOffset(*atlasOffsetX, *atlasOffsetY));
    }

    fDrawBounds.fWidth = SkTMax(fDrawBounds.width(), location.x() + clippedPathIBounds.width());
    fDrawBounds.fHeight = SkTMax(fDrawBounds.height(), location.y() + clippedPathIBounds.height());
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
                                                   sk_sp<const GrCCPathParser> parser) {
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
    rtc->addDrawOp(GrNoClip(), skstd::make_unique<DrawOp>(this, std::move(parser)));

    fTextureProxy = sk_ref_sp(rtc->asTextureProxy());
    return rtc;
}

void GrCCPRAtlas::DrawOp::onExecute(GrOpFlushState* flushState) {
    GrPipeline pipeline(flushState->drawOpArgs().fProxy, GrPipeline::ScissorState::kEnabled,
                        SkBlendMode::kPlus);
    fParser->drawCoverageCount(flushState, SkIRect::MakeWH(fAtlas->fDrawBounds.width(), fAtlas->fDrawBounds.height()), pipeline,
                               fAtlas->fInstanceStartIndices, fAtlas->fUnscissoredInstanceCounts,
                               fAtlas->fScissorBatches);
}
