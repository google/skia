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
#include "include/core/SkSize.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/AtlasTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDeferredUpload.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrOnFlushResourceProvider.h"
#include "src/gpu/ganesh/GrOpFlushState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/ganesh/GrAtlasTools.h"

#include <cstddef>
#include <cstdint>
#include <memory>

class GrResourceProvider;
struct GrContextOptions;

using namespace skgpu::ganesh;
using MaskFormat = skgpu::MaskFormat;

static const int kNumPlots = 2;
static const int kPlotSize = 32;
static const int kAtlasSize = kNumPlots * kPlotSize;

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
    REPORTER_ASSERT(r, expectedAlloced == GrDrawOpAtlasTools::NumAllocated(atlas));
}

class TestingUploadTarget : public GrDeferredUploadTarget {
public:
    TestingUploadTarget() { }

    const skgpu::TokenTracker* tokenTracker() final { return &fTokenTracker; }
    skgpu::TokenTracker* writeableTokenTracker() { return &fTokenTracker; }

    skgpu::AtlasToken addInlineUpload(GrDeferredTextureUploadFn&&) final {
        SkASSERT(0); // this test shouldn't invoke this code path
        return fTokenTracker.nextDrawToken();
    }

    skgpu::AtlasToken addASAPUpload(GrDeferredTextureUploadFn&& upload) final {
        return fTokenTracker.nextFlushToken();
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
                                                /*label=*/"BasicDrawOpAtlasTest");
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
        atlas->compact(uploadTarget.tokenTracker()->nextFlushToken());
    }

    check(reporter, atlas.get(), 1, 4, 1);
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

    GrOp::Owner op = AtlasTextOp::CreateOpTestingOnly(sdc.get(), paint,
                                                      font, SkMatrix::I(),
                                                      text, 16, 16);
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
    op->prepare(&flushState);
    flushState.setOpArgs(nullptr);
}

void test_atlas_config(skiatest::Reporter* reporter, int maxTextureSize, size_t maxBytes,
                       MaskFormat maskFormat, SkISize expectedDimensions,
                       SkISize expectedPlotDimensions) {
    GrDrawOpAtlasConfig config(maxTextureSize, maxBytes);
    REPORTER_ASSERT(reporter, config.atlasDimensions(maskFormat) == expectedDimensions);
    REPORTER_ASSERT(reporter, config.plotDimensions(maskFormat) == expectedPlotDimensions);
}

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
