/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrDeferredUpload.h"
#include "GrDrawOpAtlas.h"
#include "GrDrawingManager.h"
#include "GrMemoryPool.h"
#include "GrOnFlushResourceProvider.h"
#include "GrOpFlushState.h"
#include "GrRenderTargetContext.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureProxy.h"
#include "GrTypesPriv.h"
#include "GrXferProcessor.h"
#include "SkBitmap.h"
#include "SkColor.h"
#include "SkColorSpace.h"
#include "SkIPoint16.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPoint.h"
#include "SkRefCnt.h"
#include "Test.h"
#include "ops/GrDrawOp.h"
#include "text/GrAtlasManager.h"
#include "text/GrTextContext.h"
#include "text/GrTextUtils.h"

#include <memory>

class GrResourceProvider;

static const int kNumPlots = 2;
static const int kPlotSize = 32;
static const int kAtlasSize = kNumPlots * kPlotSize;

int GrDrawOpAtlas::numAllocated_TestingOnly() const {
    int count = 0;
    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        if (fProxies[i]->priv().isInstantiated()) {
            ++count;
        }
    }

    return count;
}

void GrAtlasManager::setMaxPages_TestingOnly(uint32_t maxPages) {
    for (int i = 0; i < kMaskFormatCount; i++) {
        if (fAtlases[i]) {
            fAtlases[i]->setMaxPages_TestingOnly(maxPages);
        }
    }
}

void GrDrawOpAtlas::setMaxPages_TestingOnly(uint32_t maxPages) {
    SkASSERT(!fNumActivePages);

    fMaxPages = maxPages;
}

void EvictionFunc(GrDrawOpAtlas::AtlasID atlasID, void*) {
    SkASSERT(0); // The unit test shouldn't exercise this code path
}

static void check(skiatest::Reporter* r, GrDrawOpAtlas* atlas,
                  uint32_t expectedActive, uint32_t expectedMax, int expectedAlloced) {
    REPORTER_ASSERT(r, expectedActive == atlas->numActivePages());
    REPORTER_ASSERT(r, expectedMax == atlas->maxPages());
    REPORTER_ASSERT(r, expectedAlloced == atlas->numAllocated_TestingOnly());
}

class TestingUploadTarget : public GrDeferredUploadTarget {
public:
    TestingUploadTarget() { }

    const GrTokenTracker* tokenTracker() final { return &fTokenTracker; }
    GrTokenTracker* writeableTokenTracker() { return &fTokenTracker; }

    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final {
        SkASSERT(0); // this test shouldn't invoke this code path
        return fTokenTracker.nextDrawToken();
    }

    virtual GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&& upload) final {
        return fTokenTracker.nextTokenToFlush();
    }

    void issueDrawToken() { fTokenTracker.issueDrawToken(); }
    void flushToken() { fTokenTracker.flushToken(); }

private:
    GrTokenTracker fTokenTracker;

    typedef GrDeferredUploadTarget INHERITED;
};

static bool fill_plot(GrDrawOpAtlas* atlas,
                      GrResourceProvider* resourceProvider,
                      GrDeferredUploadTarget* target,
                      GrDrawOpAtlas::AtlasID* atlasID,
                      int alpha) {
    SkImageInfo ii = SkImageInfo::MakeA8(kPlotSize, kPlotSize);

    SkBitmap data;
    data.allocPixels(ii);
    data.eraseARGB(alpha, 0, 0, 0);

    SkIPoint16 loc;
    GrDrawOpAtlas::ErrorCode code;
    code = atlas->addToAtlas(resourceProvider, atlasID, target, kPlotSize, kPlotSize,
                              data.getAddr(0, 0), &loc);
    return GrDrawOpAtlas::ErrorCode::kSucceeded == code;
}


// This is a basic DrawOpAtlas test. It simply verifies that multitexture atlases correctly
// add and remove pages. Note that this is simulating flush-time behavior.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BasicDrawOpAtlas, reporter, ctxInfo) {
    auto context = ctxInfo.grContext();
    auto proxyProvider = context->contextPriv().proxyProvider();
    auto resourceProvider = context->contextPriv().resourceProvider();
    auto drawingManager = context->contextPriv().drawingManager();

    GrOnFlushResourceProvider onFlushResourceProvider(drawingManager);
    TestingUploadTarget uploadTarget;

    std::unique_ptr<GrDrawOpAtlas> atlas = GrDrawOpAtlas::Make(
                                                proxyProvider,
                                                kAlpha_8_GrPixelConfig,
                                                kAtlasSize, kAtlasSize,
                                                kNumPlots, kNumPlots,
                                                GrDrawOpAtlas::AllowMultitexturing::kYes,
                                                EvictionFunc, nullptr);
    check(reporter, atlas.get(), 0, 4, 0);

    // Fill up the first level
    GrDrawOpAtlas::AtlasID atlasIDs[kNumPlots * kNumPlots];
    for (int i = 0; i < kNumPlots * kNumPlots; ++i) {
        bool result = fill_plot(atlas.get(), resourceProvider, &uploadTarget, &atlasIDs[i], i*32);
        REPORTER_ASSERT(reporter, result);
        check(reporter, atlas.get(), 1, 4, 1);
    }

    atlas->instantiate(&onFlushResourceProvider);
    check(reporter, atlas.get(), 1, 4, 1);

    // Force allocation of a second level
    GrDrawOpAtlas::AtlasID atlasID;
    bool result = fill_plot(atlas.get(), resourceProvider, &uploadTarget, &atlasID, 4*32);
    REPORTER_ASSERT(reporter, result);
    check(reporter, atlas.get(), 2, 4, 2);

    // Simulate a lot of draws using only the first plot. The last texture should be compacted.
    for (int i = 0; i < 512; ++i) {
        atlas->setLastUseToken(atlasIDs[0], uploadTarget.tokenTracker()->nextDrawToken());
        uploadTarget.issueDrawToken();
        uploadTarget.flushToken();
        atlas->compact(uploadTarget.tokenTracker()->nextTokenToFlush());
    }

    check(reporter, atlas.get(), 1, 4, 1);
}

// This test verifies that the GrAtlasTextOp::onPrepare method correctly handles a failure
// when allocating an atlas page.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrAtlasTextOpPreparation, reporter, ctxInfo) {

    auto context = ctxInfo.grContext();

    auto gpu = context->contextPriv().getGpu();
    auto resourceProvider = context->contextPriv().resourceProvider();
    auto drawingManager = context->contextPriv().drawingManager();
    auto textContext = drawingManager->getTextContext();
    auto opMemoryPool = context->contextPriv().opMemoryPool();

    auto rtc =  context->contextPriv().makeDeferredRenderTargetContext(SkBackingFit::kApprox,
                                                                       32, 32,
                                                                       kRGBA_8888_GrPixelConfig,
                                                                       nullptr);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setLCDRenderText(false);
    paint.setAntiAlias(false);
    paint.setSubpixelText(false);
    GrTextUtils::Paint utilsPaint(&paint, &rtc->colorSpaceInfo());

    const char* text = "a";

    std::unique_ptr<GrDrawOp> op = textContext->createOp_TestingOnly(context, textContext,
                                                                     rtc.get(), paint,
                                                                     SkMatrix::I(), text,
                                                                     16, 16);
    op->finalize(*context->contextPriv().caps(), nullptr);

    TestingUploadTarget uploadTarget;

    GrOpFlushState flushState(gpu, resourceProvider, uploadTarget.writeableTokenTracker());
    GrOpFlushState::OpArgs opArgs = {
        op.get(),
        rtc->asRenderTargetProxy(),
        nullptr,
        GrXferProcessor::DstProxy(nullptr, SkIPoint::Make(0, 0))
    };

    // Cripple the atlas manager so it can't allocate any pages. This will force a failure
    // in the preparation of the text op
    auto atlasManager = context->contextPriv().getAtlasManager();
    unsigned int numProxies;
    atlasManager->getProxies(kA8_GrMaskFormat, &numProxies);
    atlasManager->setMaxPages_TestingOnly(0);

    flushState.setOpArgs(&opArgs);
    op->prepare(&flushState);
    flushState.setOpArgs(nullptr);
    opMemoryPool->release(std::move(op));
}
