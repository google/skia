/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkMutex.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkFontMetricsPriv.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkTypeface_remote.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/SlugImpl.h"
#include "src/text/gpu/SubRunContainer.h"
#include "src/text/gpu/SubRunControl.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/fonts/TestEmptyTypeface.h"
#include "tools/text/gpu/TextBlobTools.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#endif

#include <cmath>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <optional>
#include <vector>
#include <type_traits>
using namespace skia_private;
using Slug = sktext::gpu::Slug;

class DiscardableManager : public SkStrikeServer::DiscardableHandleManager,
                           public SkStrikeClient::DiscardableHandleManager {
public:
    DiscardableManager() { sk_bzero(&fCacheMissCount, sizeof(fCacheMissCount)); }
    ~DiscardableManager() override = default;

    // Server implementation.
    SkDiscardableHandleId createHandle() override {
        SkAutoMutexExclusive l(fMutex);

        // Handles starts as locked.
        fLockedHandles.add(++fNextHandleId);
        return fNextHandleId;
    }
    bool lockHandle(SkDiscardableHandleId id) override {
        SkAutoMutexExclusive l(fMutex);

        if (id <= fLastDeletedHandleId) return false;
        fLockedHandles.add(id);
        return true;
    }

    // Client implementation.
    bool deleteHandle(SkDiscardableHandleId id) override {
        SkAutoMutexExclusive l(fMutex);

        return id <= fLastDeletedHandleId;
    }

    void notifyCacheMiss(SkStrikeClient::CacheMissType type, int fontSize) override {
        SkAutoMutexExclusive l(fMutex);

        fCacheMissCount[type]++;
    }
    bool isHandleDeleted(SkDiscardableHandleId id) override {
        SkAutoMutexExclusive l(fMutex);

        return id <= fLastDeletedHandleId;
    }

    void unlockAll() {
        SkAutoMutexExclusive l(fMutex);

        fLockedHandles.reset();
    }
    void unlockAndDeleteAll() {
        SkAutoMutexExclusive l(fMutex);

        fLockedHandles.reset();
        fLastDeletedHandleId = fNextHandleId;
    }
    const THashSet<SkDiscardableHandleId>& lockedHandles() const {
        SkAutoMutexExclusive l(fMutex);

        return fLockedHandles;
    }
    SkDiscardableHandleId handleCount() {
        SkAutoMutexExclusive l(fMutex);

        return fNextHandleId;
    }
    int cacheMissCount(uint32_t type) {
        SkAutoMutexExclusive l(fMutex);

        return fCacheMissCount[type];
    }
    bool hasCacheMiss() const {
        SkAutoMutexExclusive l(fMutex);

        for (uint32_t i = 0; i <= SkStrikeClient::CacheMissType::kLast; ++i) {
            if (fCacheMissCount[i] > 0) { return true; }
        }
        return false;
    }
    void resetCacheMissCounts() {
        SkAutoMutexExclusive l(fMutex);
        sk_bzero(&fCacheMissCount, sizeof(fCacheMissCount));
    }

private:
    // The tests below run in parallel on multiple threads and use the same
    // process global SkStrikeCache. So the implementation needs to be
    // thread-safe.
    mutable SkMutex fMutex;

    SkDiscardableHandleId fNextHandleId = 0u;
    SkDiscardableHandleId fLastDeletedHandleId = 0u;
    THashSet<SkDiscardableHandleId> fLockedHandles;
    int fCacheMissCount[SkStrikeClient::CacheMissType::kLast + 1u];
};

sk_sp<SkTextBlob> buildTextBlob(sk_sp<SkTypeface> tf, int glyphCount, int textSize = 1) {
    SkFont font;
    SkASSERT(tf);
    font.setTypeface(tf);
    font.setHinting(SkFontHinting::kNormal);
    font.setSize(textSize);
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setSubpixel(true);

    SkTextBlobBuilder builder;
    SkRect bounds = SkRect::MakeWH(10, 10);
    const auto& runBuffer = builder.allocRunPosH(font, glyphCount, 0, &bounds);
    SkASSERT(runBuffer.utf8text == nullptr);
    SkASSERT(runBuffer.clusters == nullptr);

    for (int i = 0; i < glyphCount; i++) {
        runBuffer.glyphs[i] = static_cast<SkGlyphID>(i);
        runBuffer.pos[i] = SkIntToScalar(i);
    }
    return builder.make();
}

#if defined(SK_GANESH)
static void compare_blobs(const SkBitmap& expected, const SkBitmap& actual,
                          skiatest::Reporter* reporter, int tolerance = 0) {
    SkASSERT(expected.width() == actual.width());
    SkASSERT(expected.height() == actual.height());
    for (int i = 0; i < expected.width(); ++i) {
        for (int j = 0; j < expected.height(); ++j) {
            SkColor expectedColor = expected.getColor(i, j);
            SkColor actualColor = actual.getColor(i, j);
            if (0 == tolerance) {
                REPORTER_ASSERT(reporter, expectedColor == actualColor);
            } else {
                for (int k = 0; k < 4; ++k) {
                    int expectedChannel = (expectedColor >> (k*8)) & 0xff;
                    int actualChannel = (actualColor >> (k*8)) & 0xff;
                    REPORTER_ASSERT(reporter, abs(expectedChannel - actualChannel) <= tolerance);
                }
            }
        }
    }
}

sk_sp<SkSurface> MakeSurface(int width, int height, GrRecordingContext* rContext) {
    const SkImageInfo info =
            SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
    return SkSurfaces::RenderTarget(rContext, skgpu::Budgeted::kNo, info);
}

SkSurfaceProps FindSurfaceProps(GrRecordingContext* rContext) {
    auto surface = MakeSurface(1, 1, rContext);
    return surface->props();
}

SkBitmap RasterBlob(sk_sp<SkTextBlob> blob, int width, int height, const SkPaint& paint,
                    GrRecordingContext* rContext, const SkMatrix* matrix = nullptr,
                    SkScalar x = 0) {
    auto surface = MakeSurface(width, height, rContext);
    if (matrix) {
        surface->getCanvas()->concat(*matrix);
    }
    surface->getCanvas()->drawTextBlob(blob.get(), x, height/2, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

SkBitmap RasterBlobThroughSlug(sk_sp<SkTextBlob> blob, int width, int height, const SkPaint& paint,
                               GrRecordingContext* rContext, const SkMatrix* matrix = nullptr,
                               SkScalar x = 0) {
    auto surface = MakeSurface(width, height, rContext);
    if (matrix) {
        surface->getCanvas()->concat(*matrix);
    }
    auto canvas = surface->getCanvas();
    auto slug = Slug::ConvertBlob(canvas, *blob, {x, height/2.0f}, paint);
    slug->draw(canvas, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

SkBitmap RasterSlug(sk_sp<Slug> slug, int width, int height, const SkPaint& paint,
                    GrRecordingContext* rContext, const SkMatrix* matrix = nullptr,
                    SkScalar x = 0) {
    auto surface = MakeSurface(width, height, rContext);
    auto canvas = surface->getCanvas();
    if (matrix) {
        canvas->concat(*matrix);
    }
    slug->draw(canvas, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_StrikeSerialization,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    auto props = FindSurfaceProps(dContext);
    std::unique_ptr<SkCanvas> analysisCanvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, dContext->supportsDistanceFieldText(),
            !dContext->priv().caps()->disablePerspectiveSDFText());
    analysisCanvas->drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);

    // Ensure typeface serialization/deserialization worked.
    REPORTER_ASSERT(reporter,
                    static_cast<SkTypefaceProxy*>(clientTypeface.get())->remoteTypefaceID() ==
                        serverTypefaceID);

    auto clientBlob = buildTextBlob(clientTypeface, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, dContext);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, dContext);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

static void use_padding_options(GrContextOptions* options) {
    options->fSupportBilerpFromGlyphAtlas = true;
}

DEF_GANESH_TEST_FOR_CONTEXTS(SkRemoteGlyphCache_StrikeSerializationSlug,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             use_padding_options,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    auto props = FindSurfaceProps(dContext);
    std::unique_ptr<SkCanvas> analysisCanvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, dContext->supportsDistanceFieldText(),
            !dContext->priv().caps()->disablePerspectiveSDFText());

    // Generate strike updates.
    (void)Slug::ConvertBlob(analysisCanvas.get(), *serverBlob, {0, 0}, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);
    auto clientBlob = buildTextBlob(clientTypeface, glyphCount);

    SkBitmap expected = RasterBlobThroughSlug(serverBlob, 10, 10, paint, dContext);
    SkBitmap actual = RasterBlobThroughSlug(clientBlob, 10, 10, paint, dContext);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_GANESH_TEST_FOR_CONTEXTS(SkRemoteGlyphCache_StrikeSerializationSlugForcePath,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             use_padding_options,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount, 360);
    auto props = FindSurfaceProps(dContext);
    std::unique_ptr<SkCanvas> analysisCanvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, dContext->supportsDistanceFieldText(),
            !dContext->priv().caps()->disablePerspectiveSDFText());

    // Generate strike updates.
    (void)Slug::ConvertBlob(analysisCanvas.get(), *serverBlob, {0, 0}, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);
    auto clientBlob = buildTextBlob(clientTypeface, glyphCount, 360);

    SkBitmap expected = RasterBlobThroughSlug(serverBlob, 10, 10, paint, dContext);
    SkBitmap actual = RasterBlobThroughSlug(clientBlob, 10, 10, paint, dContext);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_GANESH_TEST_FOR_CONTEXTS(SkRemoteGlyphCache_SlugSerialization,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             use_padding_options,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    auto props = FindSurfaceProps(dContext);
    std::unique_ptr<SkCanvas> analysisCanvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, dContext->supportsDistanceFieldText(),
            !dContext->priv().caps()->disablePerspectiveSDFText());

    // Generate strike updates.
    auto srcSlug = Slug::ConvertBlob(analysisCanvas.get(), *serverBlob, {0.3f, 0}, paint);
    auto dstSlugData = srcSlug->serialize();

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

    SkBitmap expected = RasterSlug(srcSlug, 10, 10, paint, dContext);
    auto dstSlug = client.deserializeSlugForTest(dstSlugData->data(), dstSlugData->size());
    REPORTER_ASSERT(reporter, dstSlug != nullptr);
    SkBitmap actual = RasterSlug(dstSlug, 10, 10, paint, dContext);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_GANESH_TEST_FOR_CONTEXTS(SkRemoteGlyphCache_SlugSerialization_LargeOffset,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             use_padding_options,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    auto props = FindSurfaceProps(dContext);
    std::unique_ptr<SkCanvas> analysisCanvas = server.makeAnalysisCanvas(
            200, 200, props, nullptr, dContext->supportsDistanceFieldText(),
            !dContext->priv().caps()->disablePerspectiveSDFText());

    // Generate slug with a large offset (e.g., 100, 100)
    auto srcSlug = Slug::ConvertBlob(analysisCanvas.get(), *serverBlob, {100.f, 100.f}, paint);
    auto dstSlugData = srcSlug->serialize();

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

    auto draw_with_clip = [&](sk_sp<Slug> slug) {
        auto surface = MakeSurface(200, 200, dContext);
        auto canvas = surface->getCanvas();
        canvas->clipRect(SkRect::MakeLTRB(90, 90, 200, 200));
        slug->draw(canvas, paint);
        SkBitmap bitmap;
        bitmap.allocN32Pixels(200, 200);
        surface->readPixels(bitmap, 0, 0);
        return bitmap;
    };

    SkBitmap expected = draw_with_clip(srcSlug);
    auto dstSlug = client.deserializeSlugForTest(dstSlugData->data(), dstSlugData->size());
    REPORTER_ASSERT(reporter, dstSlug != nullptr);
    SkBitmap actual = draw_with_clip(dstSlug);

    compare_blobs(expected, actual, reporter);

    discardableManager->unlockAndDeleteAll();
}

DEF_GANESH_TEST_FOR_CONTEXTS(SkRemoteGlyphCache_SubRunBoundsReconstruction,
                             skgpu::IsRenderingContext,
                             reporter,
                             ctxInfo,
                             use_padding_options,
                             CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    auto colrTypeface = ToolUtils::CreateTypefaceFromResource("fonts/colr.ttf");
    if (!colrTypeface) {
        return;
    }

    auto props = FindSurfaceProps(dContext);

    auto check_bounds = [&](sk_sp<SkTextBlob> blob, const SkMatrix& matrix, const char* name) {
        std::unique_ptr<SkCanvas> analysisCanvas =
                server.makeAnalysisCanvas(500,
                                          500,
                                          props,
                                          nullptr,
                                          dContext->supportsDistanceFieldText(),
                                          !dContext->priv().caps()->disablePerspectiveSDFText());

        analysisCanvas->save();
        analysisCanvas->concat(matrix);
        auto srcSlug =
                sktext::gpu::Slug::ConvertBlob(analysisCanvas.get(), *blob, {0.f, 0.f}, paint);
        analysisCanvas->restore();

        REPORTER_ASSERT(reporter, srcSlug != nullptr, "Slug is null for %s", name);
        auto dstSlugData = srcSlug->serialize();

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);

        REPORTER_ASSERT(reporter,
                        client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

        auto dstSlug = client.deserializeSlugForTest(dstSlugData->data(), dstSlugData->size());
        REPORTER_ASSERT(reporter, dstSlug != nullptr, "Dst slug is null for %s", name);

        auto dstSlugImpl = static_cast<sktext::gpu::SlugImpl*>(dstSlug.get());
        const sktext::gpu::SubRunContainer* dstContainer = dstSlugImpl->subRuns().get();
        REPORTER_ASSERT(reporter, dstContainer != nullptr, "Dst container is null for %s", name);

        const sktext::gpu::AtlasSubRun* dstSubRun =
                sktext::gpu::TextBlobTools::FirstSubRun(dstContainer);

        if (dstSubRun) {
            auto dstRect = std::get<1>(dstSubRun->deviceRectAndNeedsTransform(matrix));
            REPORTER_ASSERT(reporter,
                            !dstRect.isEmpty(),
                            "Dst bounds failed to reconstruct (empty) for %s",
                            name);

            SkRect mappedBlobBounds = blob->bounds();
            matrix.mapRect(&mappedBlobBounds);
            REPORTER_ASSERT(reporter,
                            mappedBlobBounds.contains(dstRect),
                            "Mapped blob bounds [%f, %f, %f, %f] do not contain reconstructed "
                            "bounds [%f, %f, %f, %f] for %s",
                            mappedBlobBounds.fLeft,
                            mappedBlobBounds.fTop,
                            mappedBlobBounds.fRight,
                            mappedBlobBounds.fBottom,
                            dstRect.fLeft,
                            dstRect.fTop,
                            dstRect.fRight,
                            dstRect.fBottom,
                            name);

            SkTextBlobRunIterator it(blob.get());
            while (!it.done()) {
                SkASSERT(it.positioning() == SkTextBlobRunIterator::kFull_Positioning);

                SkFont font = it.font();
                int count = it.glyphCount();
                const uint16_t* glyphs = it.glyphs();
                const SkPoint* pos = reinterpret_cast<const SkPoint*>(it.pos());

                std::vector<SkRect> glyphBounds(count);
                font.getBounds(SkSpan<const SkGlyphID>(glyphs, count),
                               SkSpan<SkRect>(glyphBounds.data(), count),
                               nullptr);

                // outset the dstRect to account for rounding or SDF text (has 2 pixels of padding)
                SkScalar outset = dstSubRun->glyphParams().isSDF ? 2.5f : 1.5f;
                dstRect.outset(outset, outset);

                for (int i = 0; i < count; ++i) {
                    SkRect mappedGlyphRect = glyphBounds[i];
                    if (mappedGlyphRect.isEmpty()) {
                        continue;
                    }
                    mappedGlyphRect.offset(pos[i]);
                    matrix.mapRect(&mappedGlyphRect);

                    REPORTER_ASSERT(reporter,
                                    dstRect.contains(mappedGlyphRect),
                                    "Reconstructed bounds [%f, %f, %f, %f] do not contain glyph %d "
                                    "[%f, %f, %f, %f] in %s",
                                    dstRect.fLeft,
                                    dstRect.fTop,
                                    dstRect.fRight,
                                    dstRect.fBottom,
                                    i,
                                    mappedGlyphRect.fLeft,
                                    mappedGlyphRect.fTop,
                                    mappedGlyphRect.fRight,
                                    mappedGlyphRect.fBottom,
                                    name);
                }

                it.next();
            }
        } else if (!dstContainer->isEmpty()) {
            INFOF(reporter, "GlyphRun was drawn with drawable or as a path in %s", name);
        } else {
            ERRORF(reporter, "Dst subrun is null for %s", name);
        }

        discardableManager->unlockAndDeleteAll();
    };

    auto build_blob = [](sk_sp<SkTypeface> tf, SkScalar size, const char* text = "abcdefgh") {
        SkFont font(tf, size);
        int len = strlen(text);
        int count = font.countText(text, len, SkTextEncoding::kUTF8);

        SkTextBlobBuilder builder;
        const auto& runBuffer = builder.allocRunPos(font, count);

        font.textToGlyphs(
                text, len, SkTextEncoding::kUTF8, SkSpan<SkGlyphID>(runBuffer.glyphs, count));
        font.getPos(SkSpan<const SkGlyphID>(runBuffer.glyphs, count),
                    SkSpan<SkPoint>(reinterpret_cast<SkPoint*>(runBuffer.pos), count),
                    {0, 0});

        return builder.make();
    };

    auto directBlob = build_blob(serverTypeface, 20);
    check_bounds(directBlob, SkMatrix::I(), "DirectMaskSubRun");

    if (dContext->supportsDistanceFieldText()) {
        SkScalar sdftSize = dContext->priv().options().fGlyphsAsPathsFontSize / 2.f;
        auto sdftBlob = build_blob(serverTypeface, sdftSize);
        check_bounds(sdftBlob, SkMatrix::I(), "SDFTSubRun");
    }

    auto hugeBlob = build_blob(colrTypeface, 20, "\U0001F600");
    SkMatrix perspective = SkMatrix::I();
    perspective.setPerspX(0.001f);
    perspective.setPerspY(0.001f);
    check_bounds(hugeBlob, perspective, "TransformedMaskSubRun");
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_ReleaseTypeFace,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTypeface = TestEmptyTypeface::Make();
    REPORTER_ASSERT(reporter, serverTypeface->unique());

    {
        const SkPaint paint;
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
        const SkSurfaceProps props;
        std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
                10, 10, props, nullptr, ctxInfo.directContext()->supportsDistanceFieldText(),
                !ctxInfo.directContext()->priv().caps()->disablePerspectiveSDFText());
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, !serverTypeface->unique());

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);
    }
    REPORTER_ASSERT(reporter, serverTypeface->unique());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}
#endif

DEF_TEST(SkRemoteGlyphCache_StrikeLockingServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);

    const SkSurfaceProps props;
    std::unique_ptr<SkCanvas> cache_diff_canvas =
            server.makeAnalysisCanvas(10, 10, props, nullptr, true, true);
    SkPaint paint;
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);

    // The strike from the blob should be locked after it has been drawn on the canvas.
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 1u);

    // Write the strike data and unlock everything. Re-analyzing the blob should lock the handle
    // again.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    discardableManager->unlockAll();
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 0u);

    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 1u);

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikeDeletionServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);

    const SkSurfaceProps props;
    std::unique_ptr<SkCanvas> cache_diff_canvas =
            server.makeAnalysisCanvas(10, 10, props, nullptr, true, true);
    SkPaint paint;
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);

    // Write the strike data and delete all the handles. Re-analyzing the blob should create new
    // handles.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);

    // Another analysis pass, to ensure that deleting handles after a complete cache hit still
    // works. This is a regression test for crbug.com/999682.
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);
    server.writeStrikeData(&fontData);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);

    discardableManager->unlockAndDeleteAll();
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 2u);

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikePinningClient, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);

    const SkSurfaceProps props;
    std::unique_ptr<SkCanvas> cache_diff_canvas =
            server.makeAnalysisCanvas(10, 10, props, nullptr, true, true);
    SkPaint paint;
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto* clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID).get();

    // The cache remains alive until it is pinned in the discardable manager.
    SkGraphics::PurgeFontCache();
    REPORTER_ASSERT(reporter, !clientTypeface->unique());

    // Once the strike is unpinned and purged, SkStrikeClient should be the only owner of the
    // clientTf.
    discardableManager->unlockAndDeleteAll();
    SkGraphics::PurgeFontCache();
    REPORTER_ASSERT(reporter, clientTypeface->unique());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_ClientMemoryAccounting, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);

    const SkSurfaceProps props;
    std::unique_ptr<SkCanvas> cache_diff_canvas =
            server.makeAnalysisCanvas(10, 10, props, nullptr, true, true);
    SkPaint paint;
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_PurgesServerEntries, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    server.setMaxEntriesInDescriptorMapForTesting(1u);
    SkStrikeClient client(discardableManager, false);

    {
        auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTypeface, glyphCount);

        const SkSurfaceProps props;
        std::unique_ptr<SkCanvas> cache_diff_canvas =
            server.makeAnalysisCanvas(10, 10, props, nullptr, true, true);
        SkPaint paint;
        REPORTER_ASSERT(reporter, server.remoteStrikeMapSizeForTesting() == 0u);
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, server.remoteStrikeMapSizeForTesting() == 1u);
    }

    // Serialize to release the lock from the strike server and delete all current
    // handles.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    discardableManager->unlockAndDeleteAll();

    // Use a different typeface. Creating a new strike should evict the previous
    // one.
    {
        auto serverTypeface = ToolUtils::CreateTestTypeface("Georgia", SkFontStyle());
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTypeface, glyphCount);

        const SkSurfaceProps props;
        std::unique_ptr<SkCanvas> cache_diff_canvas =
            server.makeAnalysisCanvas(10, 10, props, nullptr, true, true);
        SkPaint paint;
        REPORTER_ASSERT(reporter, server.remoteStrikeMapSizeForTesting() == 1u);
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, server.remoteStrikeMapSizeForTesting() == 1u);
    }

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsPath,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto direct = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    SkFont font = ToolUtils::DefaultFont();
    REPORTER_ASSERT(reporter,
            SkStrikeSpec::ShouldDrawAsPath(paint, font, SkMatrix::I()));

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    auto props = FindSurfaceProps(direct);
    std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, direct->supportsDistanceFieldText(),
            !direct->priv().caps()->disablePerspectiveSDFText());
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);
    auto clientBlob = buildTextBlob(clientTypeface, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, direct);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, direct);
    compare_blobs(expected, actual, reporter, 1);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

sk_sp<SkTextBlob> make_blob_causing_fallback(
        sk_sp<SkTypeface> targetTf, const SkTypeface* glyphTf, skiatest::Reporter* reporter) {
    SkFont font = ToolUtils::DefaultFont();
    font.setSubpixel(true);
    font.setSize(96);
    font.setHinting(SkFontHinting::kNormal);
    font.setTypeface(targetTf);

    REPORTER_ASSERT(reporter,
            !SkStrikeSpec::ShouldDrawAsPath(SkPaint(), font, SkMatrix::I()));

    char s[] = "Skia";
    size_t runSize = strlen(s);

    SkTextBlobBuilder builder;
    SkRect bounds = SkRect::MakeIWH(100, 100);
    const auto& runBuffer = builder.allocRunPosH(font, runSize, 10, &bounds);
    SkASSERT(runBuffer.utf8text == nullptr);
    SkASSERT(runBuffer.clusters == nullptr);

    SkFont(sk_ref_sp(glyphTf)).textToGlyphs(s, strlen(s), SkTextEncoding::kUTF8,
                                            {runBuffer.glyphs, runSize});

    SkRect glyphBounds = font.getBounds(runBuffer.glyphs[0], nullptr);

    REPORTER_ASSERT(reporter, glyphBounds.width() > SkGlyphDigest::kSkSideTooBigForAtlas);

    for (size_t i = 0; i < runSize; i++) {
        runBuffer.pos[i] = i * 10;
    }

    return builder.make();
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsMaskWithPathFallback,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto direct = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    SkPaint paint;

    auto serverTypeface = ToolUtils::CreateTypefaceFromResource("fonts/HangingS.ttf");
    // TODO: when the cq bots can handle this font remove the check.
    if (serverTypeface == nullptr) {
        return;
    }
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    auto serverBlob = make_blob_causing_fallback(serverTypeface, serverTypeface.get(), reporter);

    auto props = FindSurfaceProps(direct);
    std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, direct->supportsDistanceFieldText(),
            !direct->priv().caps()->disablePerspectiveSDFText());
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);
    auto clientBlob = make_blob_causing_fallback(clientTypeface, serverTypeface.get(), reporter);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, direct);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, direct);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextXY,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto direct = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    paint.setAntiAlias(true);

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    auto props = FindSurfaceProps(direct);
    std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, direct->supportsDistanceFieldText(),
            !direct->priv().caps()->disablePerspectiveSDFText());
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0.5, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);
    auto clientBlob = buildTextBlob(clientTypeface, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, direct, nullptr, 0.5);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, direct, nullptr, 0.5);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

#if !defined(SK_DISABLE_SDF_TEXT)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsDFT,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto direct = ctxInfo.directContext();
    if (!direct->priv().caps()->shaderCaps()->supportsDistanceFieldText()) {
        return;
    }
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    SkFont font = ToolUtils::DefaultFont();

    // A scale transform forces fallback to dft.
    SkMatrix matrix = SkMatrix::Scale(16, 16);
    sktext::gpu::SubRunControl control =
            direct->priv().asRecordingContext()->priv().getSubRunControl(true);
    SkScalar approximateDeviceTextSize = SkFontPriv::ApproximateTransformedTextSize(font, matrix,
                                                                                    {0, 0});
    REPORTER_ASSERT(reporter, control.isSDFT(approximateDeviceTextSize, paint, matrix));

    // Server.
    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    const SkSurfaceProps props;
    std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, direct->supportsDistanceFieldText(),
            !direct->priv().caps()->disablePerspectiveSDFText());
    cache_diff_canvas->concat(matrix);
    cache_diff_canvas->drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);
    auto clientBlob = buildTextBlob(clientTypeface, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, direct, &matrix);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, direct, &matrix);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}
#endif // !defined(SK_DISABLE_SDF_TEXT)

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_CacheMissReporting,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTypeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    REPORTER_ASSERT(reporter, serverTypeface);

    // Create the clientTypeface proxy directly from the serverTypeface.
    auto clientTypeface = sk_make_sp<SkTypefaceProxy>(
            serverTypeface->uniqueID(),
            serverTypeface->countGlyphs(),
            serverTypeface->fontStyle(),
            serverTypeface->isFixedPitch(),
            /*glyphMaskNeedsCurrentColor=*/ false,
            discardableManager,
            /*isLogging=*/ false);

    REPORTER_ASSERT(reporter, clientTypeface);
    int glyphCount = 10;
    auto clientBlob = buildTextBlob(clientTypeface, glyphCount);

    // Raster the client-side blob without the glyph data, we should get cache miss notifications.
    SkPaint paint;
    SkMatrix matrix = SkMatrix::I();
    RasterBlob(clientBlob, 10, 10, paint, ctxInfo.directContext(), &matrix);
    REPORTER_ASSERT(reporter,
                    discardableManager->cacheMissCount(SkStrikeClient::kFontMetrics) == 1);
    REPORTER_ASSERT(reporter,
                    discardableManager->cacheMissCount(SkStrikeClient::kGlyphMetrics) == 10);

    // There shouldn't be any image or path requests, since we mark the glyph as empty on a cache
    // miss.
    REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(SkStrikeClient::kGlyphImage) == 0);
    REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(SkStrikeClient::kGlyphPath) == 0);

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

sk_sp<SkTextBlob> MakeEmojiBlob(sk_sp<SkTypeface> serverTf, SkScalar textSize,
                                sk_sp<SkTypeface> clientTf = nullptr) {
    SkFont font = ToolUtils::DefaultFont();
    font.setTypeface(serverTf);
    font.setSize(textSize);

    const char* text = ToolUtils::EmojiSample().sampleText;
    auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
    if (clientTf == nullptr) return blob;

    SkSerialProcs s_procs;
    s_procs.fTypefaceProc = [](SkTypeface*, void* ctx) -> sk_sp<const SkData> {
        return SkData::MakeUninitialized(1u);
    };
    auto serialized = blob->serialize(s_procs);

    SkDeserialProcs d_procs;
    d_procs.fTypefaceCtx = &clientTf;
    d_procs.fTypefaceStreamProc = [](SkStream& stream, void* ctx) -> sk_sp<SkTypeface> {
        char u;
        if (stream.read(&u, 1) != 1) {
            return nullptr;
        }
        return *(static_cast<sk_sp<SkTypeface>*>(ctx));
    };
    return SkTextBlob::Deserialize(serialized->data(), serialized->size(), d_procs);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_TypefaceWithNoPaths,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto direct = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTypeface = ToolUtils::EmojiSample().typeface;
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    auto props = FindSurfaceProps(direct);
    std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
            500, 500, props, nullptr, direct->supportsDistanceFieldText(),
            !direct->priv().caps()->disablePerspectiveSDFText());
    for (SkScalar textSize : { 70, 180, 270, 340}) {
        auto serverBlob = MakeEmojiBlob(serverTypeface, textSize);

        SkPaint paint;
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 100, 100, paint);

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);
        if (!serverStrikeData.empty()) {
            REPORTER_ASSERT(reporter,
                            client.readStrikeData(serverStrikeData.data(),
                                                  serverStrikeData.size()));
        }
        auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);
        auto clientBlob = MakeEmojiBlob(serverTypeface, textSize, clientTypeface);
        REPORTER_ASSERT(reporter, clientBlob);

        RasterBlob(clientBlob, 500, 500, paint, direct);
        REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
        discardableManager->resetCacheMissCounts();
    }

    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

class SkRemoteGlyphCacheTest {
    public:
    static sk_sp<SkTextBlob> MakeNormalBlob(SkPaint* paint,
                                            sk_sp<SkTypeface> serverTf, bool asPaths, SkScalar textSize,
                                            sk_sp<SkTypeface> clientTf = nullptr) {
        SkFont font = ToolUtils::DefaultFont();
        font.setTypeface(serverTf);
        font.setSize(textSize);

        const char* text = "Hel lo";
        if (asPaths) {
            font.setupForAsPaths(paint);
        } else {
            SkFont font2(font);
            font2.setupForAsPaths(paint);
        }
        auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
        if (clientTf == nullptr) return blob;

        SkSerialProcs s_procs;
        s_procs.fTypefaceProc = [](SkTypeface*, void* ctx) -> sk_sp<const SkData> {
            return SkData::MakeUninitialized(1u);
        };
        auto serialized = blob->serialize(s_procs);

        SkDeserialProcs d_procs;
        d_procs.fTypefaceCtx = &clientTf;
        d_procs.fTypefaceStreamProc = [](SkStream& stream, void* ctx) -> sk_sp<SkTypeface> {
            char u;
            if (stream.read(&u, 1) != 1) {
                return nullptr;
            }
            return *(static_cast<sk_sp<SkTypeface>*>(ctx));
        };
        return SkTextBlob::Deserialize(serialized->data(), serialized->size(), d_procs);
    }
};
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_TypefaceWithPaths_MaskThenPath,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto direct = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, true);

    auto serverTypeface = ToolUtils::DefaultPortableTypeface();
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    auto props = FindSurfaceProps(direct);
    std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
            500, 500, props, nullptr, direct->supportsDistanceFieldText(),
            !direct->priv().caps()->disablePerspectiveSDFText());
    SkPaint paint;
    using Rgct = SkRemoteGlyphCacheTest;

    // Draw from mask out of the strike which provides paths.
    {
        auto serverBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, true, 64);
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 100, 100, paint);
    }
    // Draw from path out of the strike which provides paths.
    {
        auto serverBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, false, 440);
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 100, 100, paint);
    }
    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);
    if (!serverStrikeData.empty()) {
        REPORTER_ASSERT(reporter,
                        client.readStrikeData(serverStrikeData.data(),
                                              serverStrikeData.size()));
    }

    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);

    {
        auto clientBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, true, 64, clientTypeface);
        REPORTER_ASSERT(reporter, clientBlob);

        RasterBlob(clientBlob, 100, 100, paint, direct);
        REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
        discardableManager->resetCacheMissCounts();
    }
    {
        auto clientBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, false, 440, clientTypeface);
        REPORTER_ASSERT(reporter, clientBlob);

        RasterBlob(clientBlob, 100, 100, paint, direct);
        REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
        discardableManager->resetCacheMissCounts();
    }
    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_TypefaceWithPaths_PathThenMask,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto direct = ctxInfo.directContext();
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, true);

    auto serverTypeface = ToolUtils::DefaultPortableTypeface();
    const SkTypefaceID serverTypefaceID = serverTypeface->uniqueID();

    auto props = FindSurfaceProps(direct);
    std::unique_ptr<SkCanvas> cache_diff_canvas = server.makeAnalysisCanvas(
            500, 500, props, nullptr, direct->supportsDistanceFieldText(),
            !direct->priv().caps()->disablePerspectiveSDFText());
    SkPaint paint;
    using Rgct = SkRemoteGlyphCacheTest;

    // Draw from path out of the strike which provides paths.
    {
        auto serverBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, false, 440);
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 100, 100, paint);
    }
    // Draw from mask out of the strike which provides paths.
    {
        auto serverBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, true, 64);
        cache_diff_canvas->drawTextBlob(serverBlob.get(), 100, 100, paint);
    }
    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);
    if (!serverStrikeData.empty()) {
        REPORTER_ASSERT(reporter,
                        client.readStrikeData(serverStrikeData.data(),
                                              serverStrikeData.size()));
    }

    auto clientTypeface = client.retrieveTypefaceUsingServerIDForTest(serverTypefaceID);

    {
        auto clientBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, true, 64, clientTypeface);
        REPORTER_ASSERT(reporter, clientBlob);

        RasterBlob(clientBlob, 100, 100, paint, direct);
        REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
        discardableManager->resetCacheMissCounts();
    }
    {
        auto clientBlob = Rgct::MakeNormalBlob(&paint, serverTypeface, false, 440, clientTypeface);
        REPORTER_ASSERT(reporter, clientBlob);

        RasterBlob(clientBlob, 100, 100, paint, direct);
        REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
        discardableManager->resetCacheMissCounts();
    }
    // Must unlock everything on termination, otherwise memory leaks can be reported.
    discardableManager->unlockAndDeleteAll();
}
#endif

DEF_TEST(SkTypefaceProxy_Basic_Serial, reporter) {
    auto typeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkTypefaceProxyPrototype srcProto{*typeface};

    SkBinaryWriteBuffer writeBuffer({});
    srcProto.flatten(writeBuffer);

    auto data = writeBuffer.snapshotAsData();
    SkReadBuffer readBuffer{data->data(), data->size()};
    std::optional<SkTypefaceProxyPrototype> dstProto =
            SkTypefaceProxyPrototype::MakeFromBuffer(readBuffer);
    REPORTER_ASSERT(reporter, dstProto.has_value());
    auto proxy = sk_make_sp<SkTypefaceProxy>(dstProto.value(), discardableManager, false);
    REPORTER_ASSERT(reporter, typeface->uniqueID() == proxy->remoteTypefaceID());
    REPORTER_ASSERT(reporter, typeface->uniqueID() != proxy->uniqueID());
    REPORTER_ASSERT(reporter, typeface->countGlyphs() == proxy->countGlyphs());
    REPORTER_ASSERT(reporter, typeface->fontStyle() == proxy->fontStyle());
    REPORTER_ASSERT(reporter, typeface->isFixedPitch() == proxy->isFixedPitch());

    // Must be multiple of 4 bytes or the buffer will be invalid.
    uint8_t brokenBytes[] = {1, 2, 3, 4, 5, 6, 7, 8};
    SkReadBuffer brokenBuffer{std::data(brokenBytes), std::size(brokenBytes)};
    std::optional<SkTypefaceProxyPrototype> brokenProto =
            SkTypefaceProxyPrototype::MakeFromBuffer(brokenBuffer);
    REPORTER_ASSERT(reporter, !brokenProto.has_value());
}

DEF_TEST(SkGraphics_Limits, reporter) {
    const auto prev1 = SkGraphics::GetTypefaceCacheCountLimit();

    auto prev2 = SkGraphics::SetTypefaceCacheCountLimit(prev1 + 1);
    REPORTER_ASSERT(reporter, prev1 == prev2);
    prev2 = SkGraphics::GetTypefaceCacheCountLimit();
    REPORTER_ASSERT(reporter, prev2 == prev1 + 1);

    SkGraphics::SetTypefaceCacheCountLimit(prev1);  // restore orig
}

DEF_TEST(SkRemoteGlyphCache_b513780208, reporter) {
    // Legitimate path glyphs should only use kBW_Format, kA8_Format, or kLCD16_Format.
    // If a malicious strike provided a path with kARGB32_Format, it could trigger an
    // uninitialized memory issue in SkScalerContext::GenerateImageFromPath.
    //
    // SkGlyph::addPathFromBuffer now rejects any path whose format is not an allowed type.

    constexpr int kW = 200;
    constexpr int kH = 4;
    constexpr SkTypefaceID kServerTypefaceID = 0x65000000u;  // arbitrary
    constexpr uint32_t kPackedGlyphID = 0x42u;               // arbitrary

    auto BuildMaliciousStrikeData = [&]() {
        SkBinaryWriteBuffer buffer{nullptr, 0, {}};

        // --- typefaces ---
        buffer.writeInt(1);  // typefaceCount
        buffer.writeUInt(kServerTypefaceID);
        buffer.writeInt(256);
        buffer.write32(0);
        buffer.writeBool(false);
        buffer.writeBool(false);

        // --- strikes ---
        buffer.writeInt(1);                   // strikeCount
        buffer.writeUInt(kServerTypefaceID);  // serverTypefaceID
        buffer.writeUInt(1);                  // discardableHandleID

        // SkDescriptor: Craft a Rec that triggers GenerateImageFromPath (fFrameWidth >= 0).
        {
            SkScalerContextRec rec{};
            rec.fTypefaceID = kServerTypefaceID;
            rec.fTextSize = 16.0f;
            rec.fPreScaleX = 1.0f;
            rec.fPost2x2[0][0] = 1.0f;
            rec.fPost2x2[1][1] = 1.0f;
            rec.fFrameWidth = 0.0f;  // triggers fGenerateImageFromPath
            rec.fMaskFormat = SkMask::kA8_Format;
            SkAutoDescriptor ad{SkDescriptor::ComputeOverhead(1) + sizeof(rec)};
            SkDescriptor* desc = ad.getDesc();
            desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
            desc->computeChecksum();
            desc->flatten(buffer);
        }

        buffer.writeBool(false);  // fontMetricsInitialized == false
        {
            SkFontMetrics fm{};
            SkFontMetricsPriv::Flatten(buffer, fm);
        }

        buffer.writeInt(0);  // imagesCount

        // Craft a path glyph with an invalid mask format (kARGB32_Format).
        buffer.writeInt(1);  // pathsCount
        buffer.writeUInt(kPackedGlyphID);
        buffer.writePoint(SkPoint::Make(static_cast<float>(kW), 0.0f));
        buffer.writeUInt((static_cast<uint32_t>(kW) << 16) | kH);
        buffer.writeUInt(0);
        buffer.writeUInt(static_cast<uint32_t>(SkMask::kARGB32_Format));
        buffer.writeBool(true);  // hasPath
        buffer.writeBool(false);
        buffer.writeBool(false);
        buffer.writePath(SkPath::Rect(SkRect::MakeXYWH(0, 0, 1, 1)));

        buffer.writeInt(0);  // drawablesCount

        return buffer.snapshotAsData();
    };

    sk_sp<SkData> blob = BuildMaliciousStrikeData();
    REPORTER_ASSERT(reporter, blob);

    SkStrikeCache cache;
    auto discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeClient client(discardableManager, /*isLogging=*/false, &cache);

    // This should fail to read the strike data because SkGlyph::addPathFromBuffer
    // now validates that path glyphs have an expected mask format.
    REPORTER_ASSERT(reporter, !client.readStrikeData(blob->data(), blob->size()));
}
