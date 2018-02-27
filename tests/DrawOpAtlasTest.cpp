/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextPriv.h"
#include "Test.h"
#include "text/GrGlyphCache.h"

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

    const GrTokenTracker* tokenTracker() final {
        return &fTokenTracker;
    }

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
    bool result = atlas->addToAtlas(resourceProvider, atlasID, target, kPlotSize, kPlotSize,
                                    data.getAddr(0, 0), &loc);
    return result;
}


DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DrawOpAtlas, reporter, ctxInfo) {
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

#endif
