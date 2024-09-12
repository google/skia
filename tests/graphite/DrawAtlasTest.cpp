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
                                         CtsEnforcement::kNextRelease) {
    auto recorder = context->makeRecorder();

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
    check(reporter, atlas.get(), 0, 0);

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
        check(reporter, atlas.get(), 1, 0);
    }

    // Force creation of a second page.
    bool result = fill_plot(atlas.get(), recorder.get(), &atlasLocator, 4 * 32);
    REPORTER_ASSERT(reporter, result);
    check(reporter, atlas.get(), 2, 0);

    // Simulate a lot of draws using only the first plot. The last texture should be compacted.
    for (int i = 0; i < 512; ++i) {
        atlas->setLastUseToken(testAtlasLocator, recorder->priv().tokenTracker()->nextFlushToken());
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken(), /*forceCompact=*/false);
    }
    check(reporter, atlas.get(), 1, 0);

    // Simulate a lot of non-atlas draws. We should end up with no textures.
    for (int i = 0; i < 512; ++i) {
        recorder->priv().issueFlushToken();
        atlas->compact(recorder->priv().tokenTracker()->nextFlushToken(), /*forceCompact=*/false);
    }
    check(reporter, atlas.get(), 0, 1);

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
        check(reporter, atlas.get(), p, 0);
    }
    // Try one more, it should fail.
    result = fill_plot(atlas.get(), recorder.get(), &atlasLocator, 0xff);
    REPORTER_ASSERT(reporter, !result);
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

DEF_GRAPHITE_TEST(DrawAtlasConfig_Basic, reporter, CtsEnforcement::kNextRelease) {
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
