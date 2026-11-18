/*
 * Copyright 2018 Google LLC
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
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrAtlasTypes.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrOnFlushResourceProvider.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/ops/SmallPathAtlasMgr.h"
#include "src/gpu/ganesh/text/GlyphData.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/GlyphVector.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/SubRunControl.h"
#include "src/text/gpu/TextBlob.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/ganesh/AtlasTextOpTools.h"
#include "tools/ganesh/GrAtlasTools.h"
#include "tools/text/gpu/TextBlobTools.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

class GrResourceProvider;
struct GrContextOptions;

using namespace skgpu::ganesh;
using MaskFormat = skgpu::MaskFormat;


namespace {
const int kNumPlots = 2;
const int kPlotSize = 32;
const int kAtlasSize = kNumPlots * kPlotSize;

class AssertOnEvict : public GrPlotEvictionCallback {
public:
    void evict(GrPlotLocator) override {
        SkASSERT(0); // The unit test shouldn't exercise this code path
    }
};

void check(skiatest::Reporter* r, GrDrawOpAtlas* atlas,
           uint32_t expectedActive, int expectedAlloced) {
    REPORTER_ASSERT(r, atlas->numActivePages() == expectedActive);
    REPORTER_ASSERT(r, GrDrawOpAtlasTools::NumAllocated(atlas) == expectedAlloced);
    REPORTER_ASSERT(r, atlas->maxPages() == GrPlotLocator::kMaxMultitexturePages);
}

bool fill_plot(GrDrawOpAtlas* atlas,
               GrResourceProvider* resourceProvider,
               GrDeferredUploadTarget* target,
               GrAtlasLocator* atlasLocator,
               int alpha) {
    SkImageInfo ii = SkImageInfo::MakeA8(kPlotSize, kPlotSize);

    SkBitmap data;
    data.allocPixels(ii);
    data.eraseARGB(alpha, 0, 0, 0);

    GrDrawOpAtlas::ErrorCode code;
    code = atlas->addToAtlas(resourceProvider, target, kPlotSize, kPlotSize,
                             data.getAddr(0, 0), atlasLocator);
    return GrDrawOpAtlas::ErrorCode::kSucceeded == code;
}

sk_sp<sktext::gpu::TextBlob> make_atlas_blob(GrDirectContext* dContext,
                                             sk_sp<SkTypeface> typeface,
                                             SkScalar size,
                                             std::string_view text) {
    SkFont font(std::move(typeface), size);
    font.setEdging(SkFont::Edging::kAlias);
    SkPaint paint;
    sktext::GlyphRunBuilder builder;
    const sktext::GlyphRunList& glyphRunList =
            builder.textToGlyphRunList(font, paint, text.data(), text.size(), {16, 16});
    sktext::gpu::SubRunControl control = dContext->priv().getSubRunControl(false);
    SkStrikeDeviceInfo strikeDeviceInfo{SkSurfaceProps(), SkScalerContextFlags::kNone, &control};
    return sktext::gpu::TextBlob::Make(glyphRunList,
                                       paint,
                                       SkMatrix::I(),
                                       strikeDeviceInfo,
                                       SkStrikeCache::GlobalStrikeCache());
}

GlyphData& init_glyph_data(const sktext::gpu::AtlasSubRun& subRun,
                           GrOpFlushState* flushState,
                           GrAtlasManager* atlasManager) {
    subRun.glyphVector().initBackendData<GlyphData>(flushState->strikeCache(),
                                                    atlasManager,
                                                    subRun.maskFormat(),
                                                    subRun.glyphSrcPadding(),
                                                    subRun.glyphParams().isSDF);
    return subRun.glyphVector().accessBackendData<GlyphData>();
}
}  // anonymous namespace

// Can't be in anonymous namespace because it needs friend access to TokenTracker
class TestingUploadTarget : public GrDeferredUploadTarget {
public:
    TestingUploadTarget() { }

    const skgpu::TokenTracker* tokenTracker() final { return &fTokenTracker; }
    skgpu::TokenTracker* writeableTokenTracker() { return &fTokenTracker; }

    skgpu::Token addInlineUpload(GrDeferredTextureUploadFn&&) final {
        SkASSERT(0); // this test shouldn't invoke this code path
        return fTokenTracker.nextDrawToken();
    }

    skgpu::Token addASAPUpload(GrDeferredTextureUploadFn&& upload) final {
        fNumASAPUploads++;
        return fTokenTracker.nextFlushToken();
    }

    void issueDrawToken() { fTokenTracker.issueDrawToken(); }
    void issueFlushToken() { fTokenTracker.issueFlushToken(); }

    int numASAPUploads() const { return fNumASAPUploads; }

private:
    skgpu::TokenTracker fTokenTracker;
    int fNumASAPUploads = 0;

    using INHERITED = GrDeferredUploadTarget;
};

// This is a basic DrawOpAtlas test. It simply verifies that multitexture atlases correctly
// add and remove pages. Note that this is simulating flush-time behavior.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(BasicDrawOpAtlas,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
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
    GrAtlasGenerationCounter counter;

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
                                                /*label=*/"BasicDrawOpAtlasTest");
    check(reporter, atlas.get(), 0, 0);

    // Fill up the first level
    std::array<GrAtlasLocator, kNumPlots * kNumPlots> atlasLocators;
    for (int i = 0; i < kNumPlots * kNumPlots; ++i) {
        bool result = fill_plot(
                atlas.get(), resourceProvider, &uploadTarget, &atlasLocators[i], i * 32);
        REPORTER_ASSERT(reporter, result);
        check(reporter, atlas.get(), 1, 1);
    }

    atlas->instantiate(&onFlushResourceProvider);
    check(reporter, atlas.get(), 1, 1);

    // Force allocation of a second level
    GrAtlasLocator atlasLocator;
    bool result = fill_plot(atlas.get(), resourceProvider, &uploadTarget, &atlasLocator, 4 * 32);
    REPORTER_ASSERT(reporter, result);
    check(reporter, atlas.get(), 2, 2);

    // Simulate a lot of draws using only the first plot. The last texture should be compacted.
    for (int i = 0; i < 512; ++i) {
        atlas->setLastUseToken(atlasLocators[0], uploadTarget.tokenTracker()->nextDrawToken());
        uploadTarget.issueDrawToken();
        uploadTarget.issueFlushToken();
        atlas->compact(uploadTarget.tokenTracker()->nextFlushToken());
    }

    check(reporter, atlas.get(), 1, 1);
}

// This test verifies that the AtlasTextOp::onPrepare method correctly handles a failure
// when allocating an atlas page.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrAtlasTextOpPreparation,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    auto gpu = dContext->priv().getGpu();
    auto resourceProvider = dContext->priv().resourceProvider();

    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext,
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {32, 32},
                                                       SkSurfaceProps(),
                                                       /*label=*/"AtlasTextOpPreparation");

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    SkFont font = ToolUtils::DefaultFont();
    font.setEdging(SkFont::Edging::kAlias);

    const char* text = "a";

    GrOp::Owner op =
            AtlasTextOpTools::CreateOp(sdc.get(), paint, font, SkMatrix::I(), text, 16, 16);
    if (!op) {
        return;
    }

    auto atlasTextOp = (AtlasTextOp*)op.get();
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
    GrAtlasManagerTools::SetMaxPages(atlasManager, 0);

    flushState.setOpArgs(&opArgs);
    REPORTER_ASSERT(reporter, !op->prepare(&flushState));
    flushState.setOpArgs(nullptr);
}

namespace {
void test_atlas_config(skiatest::Reporter* reporter, int maxTextureSize, size_t maxBytes,
                       MaskFormat maskFormat, SkISize expectedDimensions,
                       SkISize expectedPlotDimensions) {
    GrDrawOpAtlasConfig config(maxTextureSize, maxBytes);
    REPORTER_ASSERT(reporter, config.atlasDimensions(maskFormat) == expectedDimensions);
    REPORTER_ASSERT(reporter, config.plotDimensions(maskFormat) == expectedPlotDimensions);
}
}  // anonymous namespace

DEF_GANESH_TEST(GrDrawOpAtlasConfig_Basic, reporter, options, CtsEnforcement::kApiLevel_T) {
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

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrOpFlushStateAbort,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto gpu = dContext->priv().getGpu();
    auto resourceProvider = dContext->priv().resourceProvider();

    TestingUploadTarget uploadTarget;
    GrOpFlushState flushState(gpu, resourceProvider, uploadTarget.writeableTokenTracker());

    // Advance draw token by simulating an uncompleted draw
    uploadTarget.issueDrawToken();
    REPORTER_ASSERT(reporter,
                    uploadTarget.tokenTracker()->nextDrawToken() !=
                    uploadTarget.tokenTracker()->nextFlushToken());

    // abort() should catch up flush tokens
    flushState.abort();
    REPORTER_ASSERT(reporter,
                    uploadTarget.tokenTracker()->nextDrawToken() ==
                    uploadTarget.tokenTracker()->nextFlushToken());
    flushState.reset();
}

// Every flush advances the flush token, even one that executes no draws. Otherwise an atlas
// addition in the next flush would assume the previous (already executed) upload of the plot is
// still pending and not schedule its own.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrDrawOpAtlasUploadAfterFlushWithoutDraws,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    auto dContext = ctxInfo.directContext();
    auto gpu = dContext->priv().getGpu();
    auto proxyProvider = dContext->priv().proxyProvider();
    auto resourceProvider = dContext->priv().resourceProvider();
    auto drawingManager = dContext->priv().drawingManager();
    const GrCaps* caps = dContext->priv().caps();

    GrOnFlushResourceProvider onFlushResourceProvider(drawingManager);
    TestingUploadTarget uploadTarget;
    GrOpFlushState flushState(gpu, resourceProvider, uploadTarget.writeableTokenTracker());

    GrColorType atlasColorType = GrColorType::kAlpha_8;
    GrBackendFormat format = caps->getDefaultBackendFormat(atlasColorType, GrRenderable::kNo);

    AssertOnEvict evictor;
    GrAtlasGenerationCounter counter;
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
            /*label=*/"UploadAfterFlushWithoutDrawsTest");
    REPORTER_ASSERT(reporter, atlas);
    if (!atlas) {
        return;
    }

    // Entries small enough that several fit in a single plot.
    constexpr int kEntrySize = kPlotSize / 4;
    SkBitmap data;
    data.allocPixels(SkImageInfo::MakeA8(kEntrySize, kEntrySize));
    data.eraseARGB(0xFF, 0, 0, 0);

    GrAtlasLocator first;
    REPORTER_ASSERT(reporter,
                    atlas->addToAtlas(resourceProvider, &uploadTarget, kEntrySize, kEntrySize,
                                      data.getAddr(0, 0), &first) ==
                    GrDrawOpAtlas::ErrorCode::kSucceeded);
    atlas->instantiate(&onFlushResourceProvider);
    REPORTER_ASSERT(reporter, uploadTarget.numASAPUploads() == 1);

    // End the flush without executing any draws.
    flushState.abort();
    flushState.reset();

    GrAtlasLocator second;
    REPORTER_ASSERT(reporter,
                    atlas->addToAtlas(resourceProvider, &uploadTarget, kEntrySize, kEntrySize,
                                      data.getAddr(0, 0), &second) ==
                    GrDrawOpAtlas::ErrorCode::kSucceeded);
    // The second entry lands in the same plot as the first and gets its own upload, since the
    // plot's previous upload belonged to an earlier flush.
    REPORTER_ASSERT(reporter, second.pageIndex() == first.pageIndex());
    REPORTER_ASSERT(reporter, second.plotIndex() == first.plotIndex());
    REPORTER_ASSERT(reporter, uploadTarget.numASAPUploads() == 2);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GrAtlasManagerOnFlushFailed,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto atlasManager = dContext->priv().getAtlasManager();
    if (!atlasManager) {
        return;
    }

    unsigned int numProxies = 0;
    const GrSurfaceProxyView* views = atlasManager->getViews(MaskFormat::kA8, &numProxies);
    REPORTER_ASSERT(reporter, views != nullptr);
    REPORTER_ASSERT(reporter, GrAtlasManagerTools::HasAtlas(atlasManager, MaskFormat::kA8));

    // Calling onFlushFailed should free all atlases
    atlasManager->onFlushFailed();
    REPORTER_ASSERT(reporter, !GrAtlasManagerTools::HasAtlas(atlasManager, MaskFormat::kA8));

    // After onFlushFailed, atlas is re-initialized on demand
    views = atlasManager->getViews(MaskFormat::kA8, &numProxies);
    REPORTER_ASSERT(reporter, views != nullptr);
    REPORTER_ASSERT(reporter, GrAtlasManagerTools::HasAtlas(atlasManager, MaskFormat::kA8));
}

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SmallPathAtlasMgrOnFlushFailed,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();
    auto smallPathAtlasMgr = dContext->priv().getSmallPathAtlasMgr();
    if (!smallPathAtlasMgr) {
        return;
    }

    bool initSuccess = smallPathAtlasMgr->initAtlas(dContext->priv().proxyProvider(),
                                                    dContext->priv().caps());
    REPORTER_ASSERT(reporter, initSuccess);
    REPORTER_ASSERT(reporter, smallPathAtlasMgr->hasAtlas());

    int numProxies = 0;
    const GrSurfaceProxyView* views = smallPathAtlasMgr->getViews(&numProxies);
    REPORTER_ASSERT(reporter, views != nullptr);

    // Calling onFlushFailed should reset the atlas manager and free the atlas
    smallPathAtlasMgr->onFlushFailed();
    REPORTER_ASSERT(reporter, !smallPathAtlasMgr->hasAtlas());

    // After onFlushFailed, atlas can be re-initialized on demand
    initSuccess = smallPathAtlasMgr->initAtlas(dContext->priv().proxyProvider(),
                                               dContext->priv().caps());
    REPORTER_ASSERT(reporter, initSuccess);
    REPORTER_ASSERT(reporter, smallPathAtlasMgr->hasAtlas());
}
#endif

// Regenerating a cached non-final slice of a GlyphVector must still update its plots' use token
// so later atlas uploads in the same draw cannot evict glyphs queued by that partial slice
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(GlyphDataRegenerateAtlasPartialSliceUpdatesUseToken,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNextRelease) {
    GrDirectContext* dContext = ctxInfo.directContext();
    GrAtlasManager* atlasManager = dContext->priv().getAtlasManager();
    REPORTER_ASSERT(reporter, atlasManager != nullptr);
    GrAtlasManagerTools::SetAtlasDimensionsToMinimum(atlasManager);
    unsigned int numProxies = 0;
    REPORTER_ASSERT(reporter, atlasManager->getViews(MaskFormat::kA8, &numProxies) != nullptr);
    GrAtlasManagerTools::SetMaxPages(atlasManager, 1);

    sk_sp<SkTypeface> typeface = ToolUtils::DefaultPortableTypeface();
    sk_sp<sktext::gpu::TextBlob> blob = make_atlas_blob(dContext, typeface, 24, "ab");
    const sktext::gpu::AtlasSubRun* subRun = sktext::gpu::TextBlobTools::FirstSubRun(blob.get());
    REPORTER_ASSERT(reporter, subRun != nullptr);
    if (!subRun) {
        return;
    }
    REPORTER_ASSERT(reporter, subRun->glyphCount() == 2, "got %d", subRun->glyphCount());
    REPORTER_ASSERT(reporter, subRun->maskFormat() == MaskFormat::kA8);

    constexpr std::string_view kFillerText =
            "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    sk_sp<sktext::gpu::TextBlob> fillerBlob =
            make_atlas_blob(dContext, std::move(typeface), 128, kFillerText);
    const sktext::gpu::AtlasSubRun* fillerSubRun =
            sktext::gpu::TextBlobTools::FirstSubRun(fillerBlob.get());
    REPORTER_ASSERT(reporter, fillerSubRun != nullptr);
    if (!fillerSubRun) {
        return;
    }
    const int fillerCount = fillerSubRun->glyphCount();
    REPORTER_ASSERT(reporter, fillerCount > 0, "got %d", fillerCount);
    REPORTER_ASSERT(reporter, fillerSubRun->maskFormat() == MaskFormat::kA8);

    TestingUploadTarget uploadTarget;
    GrOpFlushState flushState(dContext->priv().getGpu(),
                              dContext->priv().resourceProvider(),
                              uploadTarget.writeableTokenTracker());
    GlyphData& glyphData = init_glyph_data(*subRun, &flushState, atlasManager);
    GlyphData& fillerGlyphData = init_glyph_data(*fillerSubRun, &flushState, atlasManager);
    SkSpan<const Glyph> glyphs = subRun->glyphVector().accessBackendGlyphs<Glyph>();
    REPORTER_ASSERT(reporter, glyphs.size() == 2, "got %zu", glyphs.size());

    // Flush 1 populates the full GlyphVector (caching fAtlasGeneration) and fills all plots
    auto [ok1, placed1] = glyphData.regenerateAtlas(0, 2, subRun->glyphVector(), &flushState);
    REPORTER_ASSERT(reporter, ok1 && placed1 == 2, "ok1=%d placed1=%d", ok1, placed1);
    auto [fillOk1, fillPlaced1] = fillerGlyphData.regenerateAtlas(
            0, fillerCount, fillerSubRun->glyphVector(), &flushState);
    REPORTER_ASSERT(reporter,
                    fillOk1 && fillPlaced1 > 0 && fillPlaced1 < fillerCount,
                    "fillOk1=%d fillPlaced1=%d fillerCount=%d",
                    fillOk1,
                    fillPlaced1,
                    fillerCount);
    REPORTER_ASSERT(reporter, atlasManager->hasGlyph(glyphs[0].entry()));
    uploadTarget.issueDrawToken();
    flushState.abort();
    flushState.reset();

    // Flush 2 prepares only slice [0, 1); subsequent uploads in the same draw must not evict it
    auto [ok2, placed2] = glyphData.regenerateAtlas(0, 1, subRun->glyphVector(), &flushState);
    REPORTER_ASSERT(reporter, ok2 && placed2 == 1, "ok2=%d placed2=%d", ok2, placed2);
    auto [fillOk2, fillPlaced2] = fillerGlyphData.regenerateAtlas(
            fillPlaced1, fillerCount, fillerSubRun->glyphVector(), &flushState);
    REPORTER_ASSERT(reporter,
                    fillOk2 && fillPlaced2 > 0 && fillPlaced1 + fillPlaced2 < fillerCount,
                    "fillOk2=%d fillPlaced1=%d fillPlaced2=%d fillerCount=%d",
                    fillOk2,
                    fillPlaced1,
                    fillPlaced2,
                    fillerCount);
    REPORTER_ASSERT(reporter, atlasManager->hasGlyph(glyphs[0].entry()));
    flushState.abort();
    flushState.reset();
}
