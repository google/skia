/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDeferredUpload.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/ops/GrDrawOp.h"
#include "src/gpu/ops/GrOp.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrTextContext.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

#include <memory>
#include <utility>

class GrResourceProvider;

static const int kNumPlots = 2;
static const int kPlotSize = 32;
static const int kAtlasSize = kNumPlots * kPlotSize;

int GrDrawOpAtlas::numAllocated_TestingOnly() const {
    int count = 0;
    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        if (fProxies[i]->isInstantiated()) {
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
    auto proxyProvider = context->priv().proxyProvider();
    auto resourceProvider = context->priv().resourceProvider();
    auto drawingManager = context->priv().drawingManager();
    const GrCaps* caps = context->priv().caps();

    GrOnFlushResourceProvider onFlushResourceProvider(drawingManager);
    TestingUploadTarget uploadTarget;

    GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                           GrRenderable::kNo);

    std::unique_ptr<GrDrawOpAtlas> atlas = GrDrawOpAtlas::Make(
                                                proxyProvider,
                                                format,
                                                GrColorType::kAlpha_8,
                                                kAtlasSize, kAtlasSize,
                                                kAtlasSize/kNumPlots, kAtlasSize/kNumPlots,
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

    auto gpu = context->priv().getGpu();
    auto resourceProvider = context->priv().resourceProvider();
    auto drawingManager = context->priv().drawingManager();
    auto textContext = drawingManager->getTextContext();
    auto opMemoryPool = context->priv().opMemoryPool();

    auto rtc = context->priv().makeDeferredRenderTargetContext(SkBackingFit::kApprox, 32, 32,
                                                               GrColorType::kRGBA_8888, nullptr);

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    SkFont font;
    font.setEdging(SkFont::Edging::kAlias);

    const char* text = "a";

    std::unique_ptr<GrDrawOp> op = textContext->createOp_TestingOnly(
            context, textContext, rtc.get(), paint, font, SkMatrix::I(), text, 16, 16);
    bool hasMixedSampledCoverage = false;
    op->finalize(*context->priv().caps(), nullptr, hasMixedSampledCoverage, GrClampType::kAuto);

    TestingUploadTarget uploadTarget;

    GrOpFlushState flushState(gpu, resourceProvider, uploadTarget.writeableTokenTracker());
    GrOpFlushState::OpArgs opArgs = {
        op.get(),
        rtc->asRenderTargetProxy(),
        nullptr,
        rtc->asRenderTargetProxy()->outputSwizzle(),
        GrXferProcessor::DstProxy(nullptr, SkIPoint::Make(0, 0))
    };

    // Cripple the atlas manager so it can't allocate any pages. This will force a failure
    // in the preparation of the text op
    auto atlasManager = context->priv().getAtlasManager();
    unsigned int numProxies;
    atlasManager->getProxies(kA8_GrMaskFormat, &numProxies);
    atlasManager->setMaxPages_TestingOnly(0);

    flushState.setOpArgs(&opArgs);
    op->prepare(&flushState);
    flushState.setOpArgs(nullptr);
    opMemoryPool->release(std::move(op));
}

void test_atlas_config(skiatest::Reporter* reporter, int maxTextureSize, size_t maxBytes,
                       GrMaskFormat maskFormat, SkISize expectedDimensions,
                       SkISize expectedPlotDimensions) {
    GrDrawOpAtlasConfig config(maxTextureSize, maxBytes);
    REPORTER_ASSERT(reporter, config.atlasDimensions(maskFormat) == expectedDimensions);
    REPORTER_ASSERT(reporter, config.plotDimensions(maskFormat) == expectedPlotDimensions);
}

DEF_GPUTEST(GrDrawOpAtlasConfig_Basic, reporter, options) {
    // 1/4 MB
    test_atlas_config(reporter, 65536, 256 * 1024, kARGB_GrMaskFormat,
                      { 256, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 256 * 1024, kA8_GrMaskFormat,
                      { 512, 512 }, { 256, 256 });
    // 1/2 MB
    test_atlas_config(reporter, 65536, 512 * 1024, kARGB_GrMaskFormat,
                      { 512, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 512 * 1024, kA8_GrMaskFormat,
                      { 1024, 512 }, { 256, 256 });
    // 1 MB
    test_atlas_config(reporter, 65536, 1024 * 1024, kARGB_GrMaskFormat,
                      { 512, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 1024 * 1024, kA8_GrMaskFormat,
                      { 1024, 1024 }, { 256, 256 });
    // 2 MB
    test_atlas_config(reporter, 65536, 2 * 1024 * 1024, kARGB_GrMaskFormat,
                      { 1024, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 2 * 1024 * 1024, kA8_GrMaskFormat,
                      { 2048, 1024 }, { 512, 256 });
    // 4 MB
    test_atlas_config(reporter, 65536, 4 * 1024 * 1024, kARGB_GrMaskFormat,
                      { 1024, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 4 * 1024 * 1024, kA8_GrMaskFormat,
                      { 2048, 2048 }, { 512, 512 });
    // 8 MB
    test_atlas_config(reporter, 65536, 8 * 1024 * 1024, kARGB_GrMaskFormat,
                      { 2048, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 8 * 1024 * 1024, kA8_GrMaskFormat,
                      { 2048, 2048 }, { 512, 512 });
    // 16 MB (should be same as 8 MB)
    test_atlas_config(reporter, 65536, 16 * 1024 * 1024, kARGB_GrMaskFormat,
                      { 2048, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 16 * 1024 * 1024, kA8_GrMaskFormat,
                      { 2048, 2048 }, { 512, 512 });

    // 4MB, restricted texture size
    test_atlas_config(reporter, 1024, 8 * 1024 * 1024, kARGB_GrMaskFormat,
                      { 1024, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 1024, 8 * 1024 * 1024, kA8_GrMaskFormat,
                      { 1024, 1024 }, { 256, 256 });

    // 3 MB (should be same as 2 MB)
    test_atlas_config(reporter, 65536, 3 * 1024 * 1024, kARGB_GrMaskFormat,
                      { 1024, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 3 * 1024 * 1024, kA8_GrMaskFormat,
                      { 2048, 1024 }, { 512, 256 });

    // minimum size
    test_atlas_config(reporter, 65536, 0, kARGB_GrMaskFormat,
                      { 256, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 0, kA8_GrMaskFormat,
                      { 512, 512 }, { 256, 256 });
}
