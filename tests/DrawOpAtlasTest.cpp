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
#include "include/gpu/GrDirectContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrMemoryPool.h"
#include "src/gpu/ganesh/GrOnFlushResourceProvider.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#include "src/gpu/ganesh/ops/GrDrawOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "tests/Test.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"

#include <functional>
#include <memory>
#include <utility>
#include "include/core/SkTextBlob.h"

#include "src/core/SkStrikeCache.h"
#include "src/text/gpu/Glyph.h"

using Glyph = sktext::gpu::Glyph;

using MaskFormat = skgpu::MaskFormat;

class GrResourceProvider;

DEFINE_bool(verboseSkyline, false, "Skyline will be very verbose.");

static const int kNumPlots = 2;
static const int kPlotSize = 32;
static const int kAtlasSize = kNumPlots * kPlotSize;

int GrDrawOpAtlas::numAllocated_TestingOnly() const {
    int count = 0;
    for (uint32_t i = 0; i < this->maxPages(); ++i) {
        if (fViews[i].proxy()->isInstantiated()) {
            ++count;
        }
    }

    return count;
}

void GrAtlasManager::setMaxPages_TestingOnly(uint32_t maxPages) {
    for (int i = 0; i < skgpu::kMaskFormatCount; i++) {
        if (fAtlases[i]) {
            fAtlases[i]->setMaxPages_TestingOnly(maxPages);
        }
    }
}

void GrDrawOpAtlas::setMaxPages_TestingOnly(uint32_t maxPages) {
    SkASSERT(!fNumActivePages);

    fMaxPages = maxPages;
}

class AssertOnEvict : public skgpu::PlotEvictionCallback {
public:
    void evict(skgpu::PlotLocator) override {
        SkASSERT(0); // The unit test shouldn't exercise this code path
    }
};

static void check(skiatest::Reporter* r, GrDrawOpAtlas* atlas,
                  uint32_t expectedActive, uint32_t expectedMax, int expectedAlloced) {
    REPORTER_ASSERT(r, expectedActive == atlas->numActivePages());
    REPORTER_ASSERT(r, expectedMax == atlas->maxPages());
    REPORTER_ASSERT(r, expectedAlloced == atlas->numAllocated_TestingOnly());
}

class TestingUploadTarget : public GrDeferredUploadTarget {
public:
    TestingUploadTarget() { }

    const skgpu::TokenTracker* tokenTracker() final { return &fTokenTracker; }
    skgpu::TokenTracker* writeableTokenTracker() { return &fTokenTracker; }

    skgpu::DrawToken addInlineUpload(GrDeferredTextureUploadFn&&) final {
        SkASSERT(0); // this test shouldn't invoke this code path
        return fTokenTracker.nextDrawToken();
    }

    skgpu::DrawToken addASAPUpload(GrDeferredTextureUploadFn&& upload) final {
        return fTokenTracker.nextTokenToFlush();
    }

    void issueDrawToken() { fTokenTracker.issueDrawToken(); }
    void issueFlushToken() { fTokenTracker.issueFlushToken(); }

private:
    skgpu::TokenTracker fTokenTracker;

    using INHERITED = GrDeferredUploadTarget;
};

static bool fill_plot(GrDrawOpAtlas* atlas,
                      GrResourceProvider* resourceProvider,
                      GrDeferredUploadTarget* target,
                      skgpu::AtlasLocator* atlasLocator,
                      int alpha,
                      int plotSize = kPlotSize,
                      void** plot = nullptr) {
    SkImageInfo ii = SkImageInfo::MakeA8(plotSize, plotSize);

    SkBitmap data;
    data.allocPixels(ii);
    data.eraseARGB(alpha, 0, 0, 0);
    if (plot != nullptr) {
        *plot = data.getAddr(0, 0);
    }

    GrDrawOpAtlas::ErrorCode code;
    code = atlas->addToAtlas(resourceProvider, target, plotSize, plotSize,
                             data.getAddr(0, 0), atlasLocator);
    return GrDrawOpAtlas::ErrorCode::kSucceeded == code;
}


// This is a basic DrawOpAtlas test. It simply verifies that multitexture atlases correctly
// add and remove pages. Note that this is simulating flush-time behavior.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(BasicDrawOpAtlas, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    auto proxyProvider = context->priv().proxyProvider();
    auto resourceProvider = context->priv().resourceProvider();
    auto drawingManager = context->priv().drawingManager();
    const GrCaps* caps = context->priv().caps();

    GrOnFlushResourceProvider onFlushResourceProvider(drawingManager);
    TestingUploadTarget uploadTarget;

    GrColorType atlasColorType = GrColorType::kAlpha_8;
    GrBackendFormat format = caps->getDefaultBackendFormat(atlasColorType,
                                                           GrRenderable::kNo);

    AssertOnEvict evictor;
    skgpu::AtlasGenerationCounter counter;

    std::unique_ptr<GrDrawOpAtlas> atlas = GrDrawOpAtlas::Make(
                                                proxyProvider,
                                                format,
                                                GrColorTypeToSkColorType(atlasColorType),
                                                GrColorTypeBytesPerPixel(atlasColorType),
                                                kAtlasSize, kAtlasSize,
                                                kAtlasSize/kNumPlots, kAtlasSize/kNumPlots,
                                                &counter,
                                                GrDrawOpAtlas::AllowMultitexturing::kYes,
                                                &evictor,
                                                /*label=*/"BasicDrawOpAtlasTest",
                                                skgpu::PadAllGlyphs::kNo);
    check(reporter, atlas.get(), 0, 4, 0);

    // Fill up the first level
    skgpu::AtlasLocator atlasLocators[kNumPlots * kNumPlots];
    for (int i = 0; i < kNumPlots * kNumPlots; ++i) {
        bool result = fill_plot(
                atlas.get(), resourceProvider, &uploadTarget, &atlasLocators[i], i * 32);
        REPORTER_ASSERT(reporter, result);
        check(reporter, atlas.get(), 1, 4, 1);
    }

    atlas->instantiate(&onFlushResourceProvider);
    check(reporter, atlas.get(), 1, 4, 1);

    // Force allocation of a second level
    skgpu::AtlasLocator atlasLocator;
    bool result = fill_plot(atlas.get(), resourceProvider, &uploadTarget, &atlasLocator, 4 * 32);
    REPORTER_ASSERT(reporter, result);
    check(reporter, atlas.get(), 2, 4, 2);

    // Simulate a lot of draws using only the first plot. The last texture should be compacted.
    for (int i = 0; i < 512; ++i) {
        atlas->setLastUseToken(atlasLocators[0], uploadTarget.tokenTracker()->nextDrawToken());
        uploadTarget.issueDrawToken();
        uploadTarget.issueFlushToken();
        atlas->compact(uploadTarget.tokenTracker()->nextTokenToFlush());
    }

    check(reporter, atlas.get(), 1, 4, 1);
}

#if SK_GPU_V1
#include "src/gpu/ganesh/v1/SurfaceDrawContext_v1.h"

// This test verifies that the AtlasTextOp::onPrepare method correctly handles a failure
// when allocating an atlas page.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrAtlasTextOpPreparation, reporter, ctxInfo) {

    auto dContext = ctxInfo.directContext();

    auto gpu = dContext->priv().getGpu();
    auto resourceProvider = dContext->priv().resourceProvider();

    auto sdc = skgpu::v1::SurfaceDrawContext::Make(dContext, GrColorType::kRGBA_8888, nullptr,
                                                   SkBackingFit::kApprox, {32, 32},
                                                   SkSurfaceProps(),
                                                   /*label=*/"AtlasTextOpPreparation");

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    SkFont font;
    font.setEdging(SkFont::Edging::kAlias);

    const char* text = "a";
    SkMatrixProvider matrixProvider(SkMatrix::I());

    GrOp::Owner op = skgpu::v1::AtlasTextOp::CreateOpTestingOnly(sdc.get(), paint,
                                                                 font, matrixProvider,
                                                                 text, 16, 16);
    if (!op) {
        return;
    }

    auto atlasTextOp = (skgpu::v1::AtlasTextOp*)op.get();
    atlasTextOp->finalize(*dContext->priv().caps(), nullptr, GrClampType::kAuto);

    TestingUploadTarget uploadTarget;

    GrOpFlushState flushState(gpu, resourceProvider, uploadTarget.writeableTokenTracker());

    GrSurfaceProxyView surfaceView = sdc->writeSurfaceView();
    GrOpFlushState::OpArgs opArgs(op.get(),
                                  surfaceView,
                                  false /*usesMSAASurface*/,
                                  nullptr,
                                  GrDstProxyView(),
                                  GrXferBarrierFlags::kNone,
                                  GrLoadOp::kLoad);

    // Modify the atlas manager so it can't allocate any pages. This will force a failure
    // in the preparation of the text op
    auto atlasManager = dContext->priv().getAtlasManager();
    unsigned int numProxies;
    atlasManager->getViews(MaskFormat::kA8, &numProxies);
    atlasManager->setMaxPages_TestingOnly(0);

    flushState.setOpArgs(&opArgs);
    op->prepare(&flushState);
    flushState.setOpArgs(nullptr);
}
#endif // SK_GPU_V1

void test_atlas_config(skiatest::Reporter* reporter, int maxTextureSize, size_t maxBytes,
                       MaskFormat maskFormat, SkISize expectedDimensions,
                       SkISize expectedPlotDimensions) {
    GrDrawOpAtlasConfig config(maxTextureSize, maxBytes);
    REPORTER_ASSERT(reporter, config.atlasDimensions(maskFormat) == expectedDimensions);
    REPORTER_ASSERT(reporter, config.plotDimensions(maskFormat) == expectedPlotDimensions);
}

DEF_GPUTEST(GrDrawOpAtlasConfig_Basic, reporter, options) {
    // 1/4 MB
    test_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kARGB,
                      { 256, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 256 * 1024, MaskFormat::kA8,
                      { 512, 512 }, { 256, 256 });
    // 1/2 MB
    test_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kARGB,
                      { 512, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 512 * 1024, MaskFormat::kA8,
                      { 1024, 512 }, { 256, 256 });
    // 1 MB
    test_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kARGB,
                      { 512, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 1024 * 1024, MaskFormat::kA8,
                      { 1024, 1024 }, { 256, 256 });
    // 2 MB
    test_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 2 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 1024 }, { 512, 256 });
    // 4 MB
    test_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 4 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 2048 }, { 512, 512 });
    // 8 MB
    test_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kARGB,
                      { 2048, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 8 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 2048 }, { 512, 512 });
    // 16 MB (should be same as 8 MB)
    test_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kARGB,
                      { 2048, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 16 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 2048 }, { 512, 512 });

    // 4MB, restricted texture size
    test_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 1024 }, { 256, 256 });
    test_atlas_config(reporter, 1024, 8 * 1024 * 1024, MaskFormat::kA8,
                      { 1024, 1024 }, { 256, 256 });

    // 3 MB (should be same as 2 MB)
    test_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kARGB,
                      { 1024, 512 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 3 * 1024 * 1024, MaskFormat::kA8,
                      { 2048, 1024 }, { 512, 256 });

    // minimum size
    test_atlas_config(reporter, 65536, 0, MaskFormat::kARGB,
                      { 256, 256 }, { 256, 256 });
    test_atlas_config(reporter, 65536, 0, MaskFormat::kA8,
                      { 512, 512 }, { 256, 256 });
}

static void supportBilerpFromGlyphAtlas(GrContextOptions* options) {
    options->fSupportBilerpFromGlyphAtlas = true;
}
static void doNotSupportBilerpFromGlyphAtlas(GrContextOptions* options) {
    options->fSupportBilerpFromGlyphAtlas = false;
}

namespace skgpu {
class RectanizerSkylineTestingPeer {
public:
    static int allocatedBytes(skgpu::Rectanizer* rectanizer) { return rectanizer->fAreaSoFar; }

    static int allBytes(skgpu::Rectanizer* rectanizer) {
        return rectanizer->fWidth * rectanizer->fHeight;
    }
};

class PlotTestingPeer {
public:
    static int allocatedBytes(skgpu::Plot* plot) {
        return RectanizerSkylineTestingPeer::allocatedBytes(plot->fRectanizer.get());
    }

    static int allBytes(skgpu::Plot* plot) {
        return RectanizerSkylineTestingPeer::allBytes(plot->fRectanizer.get());
    }

    static SkIRect getRect(skgpu::Plot* plot) {
        return SkIRect::MakeXYWH(plot->fOffset.fX, plot->fOffset.fY, plot->fWidth, plot->fHeight);
    }

    static unsigned char* data(skgpu::Plot* plot) { return plot->fData; }
};
}

class Data : public skgpu::PlotEvictionCallback
           , public skgpu::RectanizerSkylineTestingPeer
           , public skgpu::PlotTestingPeer {
public:
    SkArenaAlloc alloc;
    TestingUploadTarget uploadTarget;
    GrResourceProvider* resourceProvider;
    std::vector<SkGlyph> skGlyphs;
    std::vector<Glyph> grGlyphs;
    std::vector<int> srcPaddings;
    std::vector<bool> checked;      // To avoid double checked glyphs from evicted plots
    GrAtlasManager* atlasManager;
    skiatest::Reporter* reporter;
    sk_sp<SkTypeface> typeface;
    SkString text;

    int allocatedBytes = 0;
    int optimizedBytes = 0;
    int allBytes = 0;
    int evicted = 0;
    int evictedGlyphs = 0;
    int evictedArea = 0;

    Data(const sk_gpu_test::ContextInfo& ctxInfo,
         skiatest::Reporter* reporter,
         const SkString& text)
            : alloc(1 << 12)
            , reporter(reporter)
            , text(text) {
        auto dContext = ctxInfo.directContext();
        resourceProvider = dContext->priv().resourceProvider();
        atlasManager = dContext->priv().getAtlasManager();
        atlasManager->setAtlasDimensionsToMinimum_ForTesting();
        atlasManager->freeAll();
        unsigned int numProxies;
        atlasManager->getViews(MaskFormat::kA8, &numProxies);
        atlasManager->setMaxPages_TestingOnly(1);
        GrDrawOpAtlas* atlas = atlasManager->getAtlas_TestingOnly(MaskFormat::kA8);
        atlas->checkEvictedPlot_testingOnly(this);

        typeface = SkTypeface::MakeFromName("Segoe UI", SkFontStyle());
        this->reset();

    }
    void evict(skgpu::PlotLocator plotLocator) override {
        ++evicted;
        auto genID = plotLocator.genID();
        bool once = true;
        auto glyphs = 0;
        for (auto i = 0ul; i < skGlyphs.size(); ++i) {
            auto& grGlyph = grGlyphs[i];
            auto pageIndex = grGlyph.fAtlasLocator.pageIndex();
            auto plotIndex = grGlyph.fAtlasLocator.plotIndex();
            skgpu::Plot* plot = atlasManager->getAtlas_TestingOnly(MaskFormat::kA8)->
                                            getPlot_testingOnly(pageIndex, plotIndex);
            if (genID == plot->genID()) {
                if (checkPadding(i)) {
                    ++glyphs;
                }
                if (once) {
                    once = false;
                    int allocatedBytes1 = PlotTestingPeer::allocatedBytes(plot);
                    int allBytes1 = PlotTestingPeer::allBytes(plot);
                    allBytes += allBytes1;
                    allocatedBytes += allocatedBytes1;
                    evictedArea += allocatedBytes1;
                    if (FLAGS_verboseSkyline) {
                        SkDebugf(
                                "Plot #%u: allocated=%d (%.2f)",
                                plotIndex,
                                allocatedBytes1,
                                allocatedBytes1 * 100.0f / allBytes1);
                    }
                }
            }
        }
        if (FLAGS_verboseSkyline) {
            SkDebugf(", %d glyphs\n", glyphs);
        }
        evictedGlyphs += glyphs;
    }

    void reset() {
        skGlyphs.clear();
        grGlyphs.clear();
        srcPaddings.clear();
        checked.clear();
    }

    void add(SkGlyph skGlyph, Glyph grGlyph, int srcPadding) {
        skGlyphs.emplace_back(skGlyph);
        grGlyphs.emplace_back(grGlyph);
        srcPaddings.emplace_back(srcPadding);
        checked.emplace_back(false);
    }

    bool checkPadding(int i) {
        if (checked[i]) {
            return false;
        }
        auto& skGlyph = skGlyphs[i];
        auto& grGlyph = grGlyphs[i];
        // Check sizes
        // Check if grGlyph is surrounded with zero padding
        auto pageIndex = grGlyph.fAtlasLocator.pageIndex();
        auto plotIndex = grGlyph.fAtlasLocator.plotIndex();
        skgpu::Plot* plot = atlasManager->getAtlas_TestingOnly(MaskFormat::kA8)->
                                        getPlot_testingOnly(pageIndex, plotIndex);
        auto plotRect = PlotTestingPeer::getRect(plot);
        auto loc = grGlyph.fAtlasLocator.getUVs();
        SkRect glyphRect = SkRect::MakeLTRB(loc[0], loc[1], loc[2], loc[3]);
        if (srcPaddings[i] == 1) {
            REPORTER_ASSERT(reporter, skGlyph.width() == glyphRect.width());
            REPORTER_ASSERT(reporter, skGlyph.height() == glyphRect.height());
            glyphRect.offset(-plotRect.fLeft, -plotRect.fTop);
            auto data = PlotTestingPeer::data(plot);
            // Check if there is a zero padding around each glyph
            REPORTER_ASSERT(reporter, glyphRect.fTop > 0);
            REPORTER_ASSERT(reporter, glyphRect.fBottom + 1 <= plotRect.height());
            int y0 = glyphRect.fTop - 1;
            int y1 = glyphRect.fBottom;
            for (int x = glyphRect.fLeft; x < glyphRect.fRight; ++x) {
                auto byte0 = data + y0 * plotRect.width() + x;
                REPORTER_ASSERT(reporter, *byte0 == 0);
                auto byte1 = data + y1 * plotRect.width() + x;
                REPORTER_ASSERT(reporter, *byte1 == 0);
            }
            for (int x = glyphRect.fLeft; x < glyphRect.fRight; ++x) {
            }
        } else if (srcPaddings[i] == 0) {
            REPORTER_ASSERT(reporter, skGlyph.width() == glyphRect.width());
            REPORTER_ASSERT(reporter, skGlyph.height() == glyphRect.height());
        } else {
            REPORTER_ASSERT(reporter, skGlyph.width() - 4 == glyphRect.width());
            REPORTER_ASSERT(reporter, skGlyph.height() - 4 == glyphRect.height());
            // TODO: Check that there are no zeros at all?
        }
        checked[i] = true;
        return true;
    }

    void drawText(int srcPadding, SkScalar fontSize) {

        SkFont defaultFont(typeface, fontSize);
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(defaultFont);
        sk_sp<SkStrike> strike = strikeSpec.findOrCreateStrike();

        for (auto i = 0ul; i < text.size(); ++i) {
            char c = text[i];
            SkPackedGlyphID id(defaultFont.unicharToGlyph(c));
            SkGlyph skGlyph = strike->getScalerContext()->makeGlyph(id, &alloc);
            SkTArray<unsigned char> ones;
            ones.push_back_n(skGlyph.imageSize(), 0xff);
            skGlyph.setImage(&alloc, ones.data());
            SkPackedGlyphID glyphID;
            Glyph grGlyph(glyphID);
            auto errorCode = atlasManager->addGlyphToAtlas(
                    skGlyph, &grGlyph, srcPadding, resourceProvider, &uploadTarget);
            REPORTER_ASSERT(reporter, errorCode == GrDrawOpAtlas::ErrorCode::kSucceeded);
            add(std::move(skGlyph), std::move(grGlyph), std::move(srcPadding));
        }
    }
};

// Testing that every glyph has a zero-pixel padding
// (0, 1, or 2 without slug or 1, 2 with slug)
// Testing happen on plot eviction event to make sure all plots are checked
// (the active plots will be manually evicted, too)
void testPlots(skiatest::Reporter* reporter,
              const sk_gpu_test::ContextInfo& ctxInfo,
              SkString text) {
    Data data(ctxInfo, reporter, text);
    auto repeat = 1;
    // Draw glyphs and tests them on plot eviction
    for (; repeat > 0; --repeat) {
        for (auto i = 0ul; i < text.size(); ++i) {
            auto srcPadding = i % 2;
            auto fontSize = i + 10.0f;
            data.drawText(srcPadding, fontSize);
            data.drawText(srcPadding, text.size() - i + 10.0f);

            auto oldText = data.text;
            data.text = SkString(&text[i], 1);
            data.drawText(2, 180);
            data.text = oldText;
        }
    }

    if (FLAGS_verboseSkyline) {
        // Print all the plots that are not evicted
        SkDebugf("Summary: optimized=%.2f%% / %.2f%%, allocated=%.2f%% sum=%d "
                 "evicted plots=%d evicted glyphs/plot=%.2f evicted area/plot=%.2f\n",
                 data.optimizedBytes*100.0f/data.allocatedBytes,
                 data.optimizedBytes*100.0f/data.allBytes,
                 data.allocatedBytes*100.0f/data.allBytes,
                 data.optimizedBytes + data.allocatedBytes,
                 data.evicted,
                 (float)data.evictedGlyphs/data.evicted,
                 (float)data.evictedArea/data.evicted);
        SkDebugf("Current plots:\n");
    }

    // Manually call evict to test all the glyphs that were not evicted
    GrDrawOpAtlas* atlas = data.atlasManager->getAtlas_TestingOnly(MaskFormat::kA8);
    for (auto page = 0ul; page < atlas->numActivePages(); ++page) {
        for (auto plot = 0; plot < 4; ++plot) {
            auto p = atlas->getPlot_testingOnly(page, plot);
            data.evict(p->plotLocator());
        }
    }

    if (FLAGS_verboseSkyline) {
        SkDebugf("Summary: optimized=%.2f%% / %.2f%%, allocated=%.2f%% sum=%d "
                 "used plots=%d average glyphs/plot=%.2f average area/plot=%.2f\n",
                 data.optimizedBytes*100.0f/data.allocatedBytes,
                 data.optimizedBytes*100.0f/data.allBytes,
                 data.allocatedBytes*100.0f/data.allBytes,
                 data.optimizedBytes + data.allocatedBytes,
                 data.evicted,
                 (float)data.evictedGlyphs/data.evicted,
                 (float)data.evictedArea/data.evicted);
    }
}

static const char* TEXT = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

DEF_GPUTEST_FOR_CONTEXTS(GrAtlasManager_withOpt,
                         sk_gpu_test::GrContextFactory::IsRenderingContext,
                         reporter,
                         ctxInfo,
                         supportBilerpFromGlyphAtlas) {
    testPlots(reporter, ctxInfo, SkString(TEXT));
}

DEF_GPUTEST_FOR_CONTEXTS(GrAtlasManager_withoutOpt,
                         sk_gpu_test::GrContextFactory::IsRenderingContext,
                         reporter,
                         ctxInfo,
                         doNotSupportBilerpFromGlyphAtlas) {
    testPlots(reporter, ctxInfo, SkString(TEXT));
}
