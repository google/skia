/*
 * Copyright 2018 Google Inc.
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
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMutex.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "include/private/chromium/Slug.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTHash.h"
#include "src/core/SkTypeface_remote.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/text/gpu/SDFTControl.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/TestEmptyTypeface.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <optional>
#include <vector>

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
    slug->draw(canvas);
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
    slug->draw(canvas);
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
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
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
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
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
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
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
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTypeface, glyphCount);
    auto props = FindSurfaceProps(dContext);
    std::unique_ptr<SkCanvas> analysisCanvas = server.makeAnalysisCanvas(
            10, 10, props, nullptr, dContext->supportsDistanceFieldText(),
            !dContext->priv().caps()->disablePerspectiveSDFText());

    // Generate strike updates.
    auto srcSlug = Slug::ConvertBlob(analysisCanvas.get(), *serverBlob, {0.3f, 0}, paint);
    auto dstSlugData = srcSlug->serialize({});

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

    SkBitmap expected = RasterSlug(srcSlug, 10, 10, paint, dContext);
    auto dstSlug = client.deserializeSlugForTest(dstSlugData->data(), dstSlugData->size(), {});
    REPORTER_ASSERT(reporter, dstSlug != nullptr);
    SkBitmap actual = RasterSlug(dstSlug, 10, 10, paint, dContext);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikeLockingServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikeDeletionServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikePinningClient, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_ClientMemoryAccounting, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());

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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_PurgesServerEntries, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    server.setMaxEntriesInDescriptorMapForTesting(1u);
    SkStrikeClient client(discardableManager, false);

    {
        auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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
        auto serverTypeface = SkTypeface::MakeFromName("Georgia", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

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
    REPORTER_ASSERT(reporter,
            SkStrikeSpec::ShouldDrawAsPath(paint, SkFont(), SkMatrix::I()));

    // Server.
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

sk_sp<SkTextBlob> make_blob_causing_fallback(
        sk_sp<SkTypeface> targetTf, const SkTypeface* glyphTf, skiatest::Reporter* reporter) {
    SkFont font;
    font.setSubpixel(true);
    font.setSize(96);
    font.setHinting(SkFontHinting::kNormal);
    font.setTypeface(targetTf);

    REPORTER_ASSERT(reporter,
            !SkStrikeSpec::ShouldDrawAsPath(SkPaint(), font, SkMatrix::I()));

    char s[] = "Skia";
    int runSize = strlen(s);

    SkTextBlobBuilder builder;
    SkRect bounds = SkRect::MakeIWH(100, 100);
    const auto& runBuffer = builder.allocRunPosH(font, runSize, 10, &bounds);
    SkASSERT(runBuffer.utf8text == nullptr);
    SkASSERT(runBuffer.clusters == nullptr);

    SkFont(sk_ref_sp(glyphTf)).textToGlyphs(s, strlen(s), SkTextEncoding::kUTF8,
                                            runBuffer.glyphs, runSize);

    SkRect glyphBounds;
    font.getWidths(runBuffer.glyphs, 1, nullptr, &glyphBounds);

    REPORTER_ASSERT(reporter, glyphBounds.width() > SkGlyphDigest::kSkSideTooBigForAtlas);

    for (int i = 0; i < runSize; i++) {
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

    auto serverTypeface = MakeResourceAsTypeface("fonts/HangingS.ttf");
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
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
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
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
    SkFont font;

    // A scale transform forces fallback to dft.
    SkMatrix matrix = SkMatrix::Scale(16, 16);
    sktext::gpu::SDFTControl control =
            direct->priv().asRecordingContext()->priv().getSDFTControl(true);
    SkScalar approximateDeviceTextSize = SkFontPriv::ApproximateTransformedTextSize(font, matrix,
                                                                                    {0, 0});
    REPORTER_ASSERT(reporter, control.isSDFT(approximateDeviceTextSize, paint, matrix));

    // Server.
    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
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

    auto serverTypeface = SkTypeface::MakeFromName("monospace", SkFontStyle());

    // Create the clientTypeface proxy directly from the serverTypeface.
    auto clientTypeface = sk_make_sp<SkTypefaceProxy>(
            SkTypeface::UniqueID(serverTypeface.get()),
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

sk_sp<SkTextBlob> MakeEmojiBlob(sk_sp<SkTypeface> serverTf, SkScalar textSize,
                                sk_sp<SkTypeface> clientTf = nullptr) {
    SkFont font;
    font.setTypeface(serverTf);
    font.setSize(textSize);

    const char* text = ToolUtils::emoji_sample_text();
    auto blob = SkTextBlob::MakeFromText(text, strlen(text), font);
    if (clientTf == nullptr) return blob;

    SkSerialProcs s_procs;
    s_procs.fTypefaceProc = [](SkTypeface*, void* ctx) -> sk_sp<SkData> {
        return SkData::MakeUninitialized(1u);
    };
    auto serialized = blob->serialize(s_procs);

    SkDeserialProcs d_procs;
    d_procs.fTypefaceCtx = &clientTf;
    d_procs.fTypefaceProc = [](const void* data, size_t length, void* ctx) -> sk_sp<SkTypeface> {
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

    auto serverTypeface = ToolUtils::emoji_typeface();
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

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

class SkRemoteGlyphCacheTest {
    public:
    static sk_sp<SkTextBlob> MakeNormalBlob(SkPaint* paint,
                                            sk_sp<SkTypeface> serverTf, bool asPaths, SkScalar textSize,
                                            sk_sp<SkTypeface> clientTf = nullptr) {
        SkFont font;
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
        s_procs.fTypefaceProc = [](SkTypeface*, void* ctx) -> sk_sp<SkData> {
            return SkData::MakeUninitialized(1u);
        };
        auto serialized = blob->serialize(s_procs);

        SkDeserialProcs d_procs;
        d_procs.fTypefaceCtx = &clientTf;
        d_procs.fTypefaceProc = [](const void* data, size_t length, void* ctx) -> sk_sp<SkTypeface> {
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

    auto serverTypeface = ToolUtils::create_portable_typeface();
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
    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
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

    auto serverTypeface = ToolUtils::create_portable_typeface();
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
    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkTypefaceProxy_Basic_Serial, reporter) {
    auto typeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
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
