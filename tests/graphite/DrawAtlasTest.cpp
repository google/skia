/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/DrawAtlas.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

using namespace skgpu::graphite;
using MaskFormat = skgpu::MaskFormat;

namespace {
const int kNumPlots = 2;
const int kPlotSize = 32;
const int kAtlasSize = kNumPlots * kPlotSize;
uint32_t gEvictCount = 0;

class PlotEvictionCounter : public skgpu::PlotEvictionCallback {
public:
    void evict(skgpu::PlotLocator) override {
        ++gEvictCount;
    }
};

void check(skiatest::Reporter* r, DrawAtlas* atlas,
           uint32_t expectedActive, uint32_t expectedEvictCount) {
    REPORTER_ASSERT(r, atlas->numActivePages() == expectedActive);
    REPORTER_ASSERT(r, gEvictCount == expectedEvictCount);
    REPORTER_ASSERT(r, atlas->maxPages() == skgpu::PlotLocator::kMaxMultitexturePages);
}

bool fill_plot(DrawAtlas* atlas,
               Recorder* recorder,
               skgpu::AtlasLocator* atlasLocator,
               int alpha) {
    SkImageInfo ii = SkImageInfo::MakeA8(kPlotSize, kPlotSize);

    SkBitmap data;
    data.allocPixels(ii);
    data.eraseARGB(alpha, 0, 0, 0);

    DrawAtlas::ErrorCode code;
    code = atlas->addToAtlas(recorder, kPlotSize, kPlotSize,
                             data.getAddr(0, 0), atlasLocator);
    return DrawAtlas::ErrorCode::kSucceeded == code;
}
}  // anonymous namespace

// This is a basic DrawOpAtlas test. It simply verifies that multitexture atlases correctly
// add and remove pages. Note that this is simulating flush-time behavior.
DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(BasicDrawAtlas,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();

    gEvictCount = 0;
    SkColorType atlasColorType = kAlpha_8_SkColorType;
    PlotEvictionCounter evictor;
    skgpu::AtlasGenerationCounter counter;
    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(atlasColorType,
                                                       SkColorTypeBytesPerPixel(atlasColorType),
                                                       kAtlasSize, kAtlasSize,
                                                       kAtlasSize/kNumPlots, kAtlasSize/kNumPlots,
                                                       &counter,
                                                       DrawAtlas::AllowMultitexturing::kYes,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"BasicDrawAtlasTest");
    check(reporter, atlas.get(), /*expectedActive=*/0, /*expectedEvictCount=*/0);

    // Fill up the first page
    skgpu::AtlasLocator atlasLocator;
    skgpu::AtlasLocator testAtlasLocator;
    for (int i = 0; i < kNumPlots * kNumPlots; ++i) {
        bool result = fill_plot(atlas.get(), recorder.get(), &atlasLocator, i * 32);
        REPORTER_ASSERT(reporter, result);
        // We will check drawing with the first plot.
        if (i == 0) {
            testAtlasLocator = atlasLocator;
        }
        check(reporter, atlas.get(), /*expectedActive=*/1, /*expectedEvictCount=*/0);
    }
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 4);

    // Force creation of a second page.
    bool result = fill_plot(atlas.get(), recorder.get(), &atlasLocator, 4 * 32);
    REPORTER_ASSERT(reporter, result);
    check(reporter, atlas.get(), /*expectedActive=*/2, /*expectedEvictCount=*/0);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 5);

    // Simulate a lot of draws using only the first plot. The last texture should be compacted.
    for (int i = 0; i < 512; ++i) {
        atlas->setLastUseToken(testAtlasLocator, recorder->priv().tokenTracker()->nextFlushToken());
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    }
    check(reporter, atlas.get(), /*expectedActive=*/1, /*expectedEvictCount=*/1);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 4);

    // Simulate a lot of non-atlas draws. We should end up with no textures.
    for (int i = 0; i < 512; ++i) {
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    }
    check(reporter, atlas.get(), /*expectedActive=*/0, /*expectedEvictCount=*/5);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);

    // Fill the atlas all the way up.
    gEvictCount = 0;
    for (int p = 1; p <= 4; ++p) {
        for (int i = 0; i < kNumPlots * kNumPlots; ++i) {
            result = fill_plot(atlas.get(), recorder.get(), &atlasLocator, p * i * 16);
            atlas->setLastUseToken(atlasLocator, recorder->priv().tokenTracker()->nextFlushToken());
            // We will check drawing with plot index 2 in the 3rd page
            if (p == 3 && i == 2) {
                testAtlasLocator = atlasLocator;
            }
            REPORTER_ASSERT(reporter, result);
        }
        check(reporter, atlas.get(), /*expectedActive=*/p, /*expectedEvictCount=*/0);
    }
    // Try one more, it should fail.
    result = fill_plot(atlas.get(), recorder.get(), &atlasLocator, 0xff);
    REPORTER_ASSERT(reporter, !result);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 16);

    // Try to clear everything out. Should fail because there are pending "draws."
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    check(reporter, atlas.get(), /*expectedActive=*/4, /*expectedEvictCount=*/0);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 16);

    // Flush those draws
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());

    // Simulate a draw using only a plot in the 3rd page
    atlas->setLastUseToken(testAtlasLocator, recorder->priv().tokenTracker()->nextFlushToken());

    // FreeGpuResources should only remove the 4th page
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    check(reporter, atlas.get(), /*expectedActive=*/3, /*expectedEvictCount=*/4);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 12);

    // Now flush
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());

    // FreeGpuResources should clear everything out
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    check(reporter, atlas.get(), /*expectedActive=*/0, /*expectedEvictCount=*/16);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ThrashDrawAtlasCache,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    auto recorder = context->makeRecorder();
    PlotEvictionCounter evictor;
    skgpu::AtlasGenerationCounter counter;

    // Use a 4-page atlas with 4 plots per page (16 total plots)
    constexpr int numPlots = 2;
    constexpr int plotSize = 32;
    constexpr int atlasSize = numPlots * plotSize;
    constexpr int maxPages = skgpu::PlotLocator::kMaxMultitexturePages;
    constexpr int totalPlots = numPlots * numPlots * maxPages;

    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(kAlpha_8_SkColorType,
                                                       1,
                                                       atlasSize,
                                                       atlasSize,
                                                       plotSize,
                                                       plotSize,
                                                       &counter,
                                                       DrawAtlas::AllowMultitexturing::kYes,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"ThrashDrawAtlasTest");
    std::vector<skgpu::AtlasLocator> locators(totalPlots);
    SkASSERT(totalPlots == 16);

    // Test kTryAgain failure and recovery
    // Fill the entire atlas and mark all plots as in-use for the current flush
    gEvictCount = 0;
    for (int i = 0; i < totalPlots; ++i) {
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &locators[i], i));
        atlas->setLastUseToken(locators[i], recorder->priv().tokenTracker()->nextFlushToken());
    }
    check(reporter, atlas.get(), /*expectedActive=*/maxPages, /*expectedEvictCount=*/0);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == totalPlots);

    // Try to add one more plot. All plots are "in-flight", so this must fail.
    REPORTER_ASSERT(reporter, !fill_plot(atlas.get(), recorder.get(), &locators[0], 0xff));

    // Now, simulate a flush. This makes the plots evictable.
    recorder->priv().issueFlushToken();
    atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());

    // Try adding again. It should succeed by evicting the least recently used plot.
    REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &locators[0], 0xff));
    check(reporter, atlas.get(), /*expectedActive=*/maxPages, /*expectedEvictCount=*/1);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == totalPlots);

    // Reset atlas state
    gEvictCount = 0;
    atlas->evictAllPlots();
    REPORTER_ASSERT(reporter, gEvictCount == totalPlots);

    // After evicting all plots, repeatedly compact until no pages are active.
    recorder->priv().issueFlushToken();
    while (atlas->numActivePages() > 0) {
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
        recorder->priv().issueFlushToken();
    }
    check(reporter, atlas.get(), /*expectedActive=*/0, /*expectedEvictCount=*/totalPlots);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);

    // Test partial page compaction
    gEvictCount = 0;
    // Refill the atlas
    for (int i = 0; i < totalPlots; ++i) {
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &locators[i], i));
        atlas->setLastUseToken(locators[i], recorder->priv().tokenTracker()->nextFlushToken());
    }
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == totalPlots);
    recorder->priv().issueFlushToken();
    atlas->compact(
            recorder->priv().tokenTracker()->nextFlushToken());  // One compact to settle things

    // Use only one plot on the last page repeatedly, making others on that page stale.
    // The locator for the first plot on the last page is at index (totalPlots - numPlots*numPlots).
    skgpu::AtlasLocator& lastPagePlot = locators[totalPlots - (numPlots * numPlots)];

    // After many flushes, the other 3 plots on the last page should be evicted.
    // We loop one more than kPlotRecentlyUsedCount (32) times to ensure eviction.
    for (int i = 0; i < 33; ++i) {
        atlas->setLastUseToken(lastPagePlot, recorder->priv().tokenTracker()->nextFlushToken());
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    }

    // Expect 3 evictions from the last page. The page itself should remain active. Not all
    // evictions clear the data on a plot, so check for nonEmptyPlots here rather than
    // numAllocatedPlots.
    constexpr int plotsOnPage = numPlots * numPlots;
    check(reporter,
          atlas.get(),
          /*expectedActive=*/maxPages,
          /*expectedEvictCount=*/plotsOnPage - 1);
    REPORTER_ASSERT(reporter, atlas->numNonEmptyPlots() == totalPlots - (plotsOnPage - 1));

    // FreeGpuResources
    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    check(reporter, atlas.get(), /*expectedActive=*/0, /*expectedEvictCount=*/16);
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
}

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(DrawAtlasProxyLifetime,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNever) {
    gEvictCount = 0;
    auto recorder = context->makeRecorder();
    PlotEvictionCounter evictor;
    skgpu::AtlasGenerationCounter counter;
    constexpr int plotsOnPage = kNumPlots * kNumPlots;
    std::unique_ptr<DrawAtlas> atlas = DrawAtlas::Make(kAlpha_8_SkColorType,
                                                       1,
                                                       kAtlasSize,
                                                       kAtlasSize,
                                                       kPlotSize,
                                                       kPlotSize,
                                                       &counter,
                                                       DrawAtlas::AllowMultitexturing::kYes,
                                                       DrawAtlas::UseStorageTextures::kNo,
                                                       &evictor,
                                                       /*label=*/"DrawAtlasProxyTest");

    // Fill three pages and collect a shared pointer to each page's TextureProxy.
    std::vector<sk_sp<TextureProxy>> proxies;
    std::vector<skgpu::AtlasLocator> locators(plotsOnPage * 3);

    for (int i = 0; i < plotsOnPage * 3; ++i) {
        REPORTER_ASSERT(reporter, fill_plot(atlas.get(), recorder.get(), &locators[i], i));
        if (locators[i].pageIndex() >= proxies.size()) {
            proxies.push_back(atlas->getProxies()[locators[i].pageIndex()]);
        }
    }

    REPORTER_ASSERT(reporter, atlas->numActivePages() == 3);
    REPORTER_ASSERT(reporter, proxies.size() == 3);

    // All proxies should have a ref count > 1 (one from our vector, one from the atlas).
    for (const auto& proxy : proxies) {
        REPORTER_ASSERT(reporter, proxy);
        REPORTER_ASSERT(reporter, !proxy->unique());
    }

    // Simulate many frames where only a plot on the first page (page 0) is ever used. This will
    // make pages 1 and 2 stale and eligible for compaction. Use the first locator we created, which
    // is guaranteed to be on page 0.
    skgpu::AtlasLocator& firstPageLocator = locators[0];

    for (int i = 0; i < 50; ++i) {
        atlas->setLastUseToken(firstPageLocator, recorder->priv().tokenTracker()->nextFlushToken());
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken());
    }

    // The atlas should have compacted away the idle pages (2 and 1).
    REPORTER_ASSERT(reporter, atlas->numActivePages() == 1);
    // The proxy for page 0 should still be shared with the atlas.
    REPORTER_ASSERT(reporter, !proxies[0]->unique());
    // The proxies for pages 1 and 2 should now be unique to our vector
    REPORTER_ASSERT(reporter, proxies[1]->unique());
    REPORTER_ASSERT(reporter, proxies[2]->unique());

    atlas->freeGpuResources(recorder->priv().tokenTracker()->nextFlushToken());
    REPORTER_ASSERT(reporter, atlas->numAllocatedPlots() == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////

namespace {
void test_draw_atlas_config(skiatest::Reporter* reporter, int maxTextureSize, size_t maxBytes,
                            MaskFormat maskFormat, SkISize expectedDimensions,
                            SkISize expectedPlotDimensions) {
    DrawAtlasConfig config(maxTextureSize, maxBytes);
    REPORTER_ASSERT(reporter, config.atlasDimensions(maskFormat) == expectedDimensions);
    REPORTER_ASSERT(reporter, config.plotDimensions(maskFormat) == expectedPlotDimensions);
}
}  // anonymous namespace

DEF_GRAPHITE_TEST(DrawAtlasConfig_Basic, reporter, CtsEnforcement::kNever) {
    // 1/4 MB
    test_draw_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kARGB,
                           { 256, 256 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kA8,
                           { 512, 512 }, { 256, 256 });
    // 1/2 MB
    test_draw_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kARGB,
                           { 512, 256 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kA8,
                           { 1024, 512 }, { 256, 256 });
    // 1 MB
    test_draw_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kARGB,
                           { 512, 512 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kA8,
                           { 1024, 1024 }, { 256, 256 });
    // 2 MB
    test_draw_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 512 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 1024 }, { 512, 256 });
    // 4 MB
    test_draw_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 2048 }, { 512, 512 });
    // 8 MB
    test_draw_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kARGB,
                           { 2048, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 2048 }, { 512, 512 });
    // 16 MB (should be same as 8 MB)
    test_draw_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kARGB,
                           { 2048, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 2048 }, { 512, 512 });

    // 4MB, restricted texture size
    test_draw_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 1024 }, { 256, 256 });
    test_draw_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kA8,
                           { 1024, 1024 }, { 256, 256 });

    // 3 MB (should be same as 2 MB)
    test_draw_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kARGB,
                           { 1024, 512 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kA8,
                           { 2048, 1024 }, { 512, 256 });

    // minimum size
    test_draw_atlas_config(reporter, 65536, 0, MaskFormat::kARGB,
                           { 256, 256 }, { 256, 256 });
    test_draw_atlas_config(reporter, 65536, 0, MaskFormat::kA8,
                           { 512, 512 }, { 256, 256 });
}
