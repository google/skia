/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/private/SkMutex.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRemoteGlyphCache.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkSurfacePriv.h"
#include "src/core/SkTypeface_remote.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/TestEmptyTypeface.h"

#include "src/gpu/text/GrTextContext.h"

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

    void notifyCacheMiss(SkStrikeClient::CacheMissType type) override {
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
    const SkTHashSet<SkDiscardableHandleId>& lockedHandles() const {
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
            if (fCacheMissCount[i] > 0) return true;
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
    SkTHashSet<SkDiscardableHandleId> fLockedHandles;
    int fCacheMissCount[SkStrikeClient::CacheMissType::kLast + 1u];
};

sk_sp<SkTextBlob> buildTextBlob(sk_sp<SkTypeface> tf, int glyphCount) {
    SkFont font;
    font.setTypeface(tf);
    font.setHinting(SkFontHinting::kNormal);
    font.setSize(1u);
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

sk_sp<SkSurface> MakeSurface(int width, int height, GrContext* context) {
    const SkImageInfo info =
            SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
    return SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);
}

const SkSurfaceProps FindSurfaceProps(GrContext* context) {
    auto surface = MakeSurface(1, 1, context);
    return surface->props();
}

SkBitmap RasterBlob(sk_sp<SkTextBlob> blob, int width, int height, const SkPaint& paint,
                    GrContext* context, const SkMatrix* matrix = nullptr,
                    SkScalar x = 0) {
    auto surface = MakeSurface(width, height, context);
    if (matrix) surface->getCanvas()->concat(*matrix);
    surface->getCanvas()->drawTextBlob(blob.get(), x, height/2, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

DEF_TEST(SkRemoteGlyphCache_TypefaceSerialization, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto server_tf = SkTypeface::MakeDefault();
    auto tf_data = server.serializeTypeface(server_tf.get());

    auto client_tf = client.deserializeTypeface(tf_data->data(), tf_data->size());
    REPORTER_ASSERT(reporter, client_tf);
    REPORTER_ASSERT(reporter, static_cast<SkTypefaceProxy*>(client_tf.get())->remoteTypefaceID() ==
                                      server_tf->uniqueID());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_StrikeSerialization, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    const SkPaint paint;

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, props, &server,
                                                ctxInfo.grContext()->supportsDistanceFieldText());
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext());
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext());
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_ReleaseTypeFace, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTf     = TestEmptyTypeface::Make();
    auto serverTfData = server.serializeTypeface(serverTf.get());
    REPORTER_ASSERT(reporter, serverTf->unique());

    {
        const SkPaint paint;
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTf, glyphCount);
        const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        SkTextBlobCacheDiffCanvas cache_diff_canvas(
                10, 10, props, &server, ctxInfo.grContext()->supportsDistanceFieldText());
        cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, !serverTf->unique());

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);
    }
    REPORTER_ASSERT(reporter, serverTf->unique());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikeLockingServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    server.serializeTypeface(serverTf.get());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    // The strike from the blob should be locked after it has been drawn on the canvas.
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 1u);

    // Write the strike data and unlock everything. Re-analyzing the blob should lock the handle
    // again.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    discardableManager->unlockAll();
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 0u);

    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);
    REPORTER_ASSERT(reporter, discardableManager->lockedHandles().count() == 1u);

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikeDeletionServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    server.serializeTypeface(serverTf.get());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 1u);

    // Write the strike data and delete all the handles. Re-analyzing the blob should create new
    // handles.
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    discardableManager->unlockAndDeleteAll();
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
    REPORTER_ASSERT(reporter, discardableManager->handleCount() == 2u);

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_StrikePinningClient, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto* clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size()).get();

    // The cache remains alive until it is pinned in the discardable manager.
    SkGraphics::PurgeFontCache();
    REPORTER_ASSERT(reporter, !clientTf->unique());

    // Once the strike is unpinned and purged, SkStrikeClient should be the only owner of the
    // clientTf.
    discardableManager->unlockAndDeleteAll();
    SkGraphics::PurgeFontCache();
    REPORTER_ASSERT(reporter, clientTf->unique());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_ClientMemoryAccounting, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_PurgesServerEntries, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    server.setMaxEntriesInDescriptorMapForTesting(1u);
    SkStrikeClient client(discardableManager, false);

    {
        auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTf, glyphCount);

        const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, props, &server);
        SkPaint paint;
        REPORTER_ASSERT(reporter, server.remoteStrikeMapSizeForTesting() == 0u);
        cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
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
        auto serverTf = SkTypeface::MakeFromName("Georgia", SkFontStyle());
        int glyphCount = 10;
        auto serverBlob = buildTextBlob(serverTf, glyphCount);

        const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
        SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, props, &server);
        SkPaint paint;
        REPORTER_ASSERT(reporter, server.remoteStrikeMapSizeForTesting() == 1u);
        cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);
        REPORTER_ASSERT(reporter, server.remoteStrikeMapSizeForTesting() == 1u);
    }

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsPath, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    REPORTER_ASSERT(reporter,
            SkStrikeSpec::ShouldDrawAsPath(paint, SkFont(), SkMatrix::I()));

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(
            10, 10, props, &server, ctxInfo.grContext()->supportsDistanceFieldText());
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext());
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext());
    compare_blobs(expected, actual, reporter, 1);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

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

    REPORTER_ASSERT(reporter, glyphBounds.width() > SkStrikeCommon::kSkSideTooBigForAtlas);

    for (int i = 0; i < runSize; i++) {
        runBuffer.pos[i] = i * 10;
    }

    return builder.make();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsMaskWithPathFallback,
        reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    SkPaint paint;

    auto serverTf = MakeResourceAsTypeface("fonts/HangingS.ttf");
    // TODO: when the cq bots can handle this font remove the check.
    if (serverTf == nullptr) {
        return;
    }
    auto serverTfData = server.serializeTypeface(serverTf.get());

    auto serverBlob = make_blob_causing_fallback(serverTf, serverTf.get(), reporter);

    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(
            10, 10, props, &server, ctxInfo.grContext()->supportsDistanceFieldText());
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

    auto clientBlob = make_blob_causing_fallback(clientTf, serverTf.get(), reporter);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext());
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext());
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

#if 0
// TODO: turn this one when I figure out how to deal with the pixel variance from linear
//  interpolation from GPU to GPU.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsSDFTWithAllARGBFallback,
                                   reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    SkPaint paint;

    auto serverTf = ToolUtils::planet_typeface();
    // TODO: when the cq bots can handle this font remove the check.
    if (serverTf == nullptr) {
        return;
    }
    auto serverTfData = server.serializeTypeface(serverTf.get());

    auto makeBlob = [&reporter](sk_sp<SkTypeface> typeface) {
        SkFont font;
        font.setSubpixel(true);
        font.setSize(96);
        font.setHinting(SkFontHinting::kNormal);
        font.setTypeface(typeface);

        REPORTER_ASSERT(reporter, !SkDraw::ShouldDrawTextAsPaths(font, SkPaint(), SkMatrix::I()));

        // Mercury to Uranus.
        SkGlyphID glyphs[] = {1, 2, 3, 4, 5, 6, 7, 8};

        SkTextBlobBuilder builder;
        SkRect bounds = SkRect::MakeIWH(100, 100);
        const auto& runBuffer = builder.allocRunPosH(font, SK_ARRAY_COUNT(glyphs), 100, &bounds);
        SkASSERT(runBuffer.utf8text == nullptr);
        SkASSERT(runBuffer.clusters == nullptr);

        std::copy(std::begin(glyphs), std::end(glyphs), runBuffer.glyphs);

        for (size_t i = 0; i < SK_ARRAY_COUNT(glyphs); i++) {
            runBuffer.pos[i] = i * 100;
        }

        return builder.make();
    };

    auto serverBlob = makeBlob(serverTf);

    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(
            800, 800, props, &server, ctxInfo.grContext()->supportsDistanceFieldText());
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 400, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));

    auto clientBlob = makeBlob(clientTf);

    SkBitmap expected = RasterBlob(serverBlob, 800, 800, paint, ctxInfo.grContext());
    SkBitmap actual = RasterBlob(clientBlob, 800, 800, paint, ctxInfo.grContext());

    // Pixel variance can be high because of the atlas placement, and large scaling in the linear
    // interpolation.
    compare_blobs(expected, actual, reporter, 36);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}
#endif

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextXY, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    paint.setAntiAlias(true);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    auto props = FindSurfaceProps(ctxInfo.grContext());
    SkTextBlobCacheDiffCanvas cache_diff_canvas(
            10, 10, props, &server, ctxInfo.grContext()->supportsDistanceFieldText());
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0.5, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext(), nullptr, 0.5);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext(), nullptr, 0.5);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsDFT, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);
    SkPaint paint;
    SkFont font;

    // A perspective transform forces fallback to dft.
    SkMatrix matrix = SkMatrix::I();
    matrix[SkMatrix::kMPersp0] = 0.5f;
    REPORTER_ASSERT(reporter, matrix.hasPerspective());
    SkSurfaceProps surfaceProps(0, kUnknown_SkPixelGeometry);
    GrTextContext::Options options;
    GrTextContext::SanitizeOptions(&options);
    REPORTER_ASSERT(reporter, GrTextContext::CanDrawAsDistanceFields(
                                      paint, font, matrix, surfaceProps, true, options));

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(
            10, 10, props, &server, ctxInfo.grContext()->supportsDistanceFieldText());
    cache_diff_canvas.concat(matrix);
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10, paint, ctxInfo.grContext(), &matrix);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext(), &matrix);
    compare_blobs(expected, actual, reporter);
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
    SkStrikeCache::ValidateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_CacheMissReporting, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto tfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(tfData->data(), tfData->size());
    REPORTER_ASSERT(reporter, clientTf);
    int glyphCount = 10;
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    // Raster the client-side blob without the glyph data, we should get cache miss notifications.
    SkPaint paint;
    SkMatrix matrix = SkMatrix::I();
    RasterBlob(clientBlob, 10, 10, paint, ctxInfo.grContext(), &matrix);
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
    SkFont serverFont = font;
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

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_TypefaceWithNoPaths, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = ToolUtils::emoji_typeface();
    auto serverTfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());

    for (SkScalar textSize : { 70, 180, 270, 340}) {
        auto serverBlob = MakeEmojiBlob(serverTf, textSize);
        auto props = FindSurfaceProps(ctxInfo.grContext());
        SkTextBlobCacheDiffCanvas cache_diff_canvas(
                500, 500, props, &server, ctxInfo.grContext()->supportsDistanceFieldText());
        SkPaint paint;
        cache_diff_canvas.drawTextBlob(serverBlob.get(), 100, 100, paint);

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);

        REPORTER_ASSERT(reporter,
                        client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
        auto clientBlob = MakeEmojiBlob(serverTf, textSize, clientTf);
        REPORTER_ASSERT(reporter, clientBlob);

        RasterBlob(clientBlob, 500, 500, paint, ctxInfo.grContext());
        REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());
        SkStrikeCache::ValidateGlyphCacheDataSize();
        discardableManager->resetCacheMissCounts();
    }

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_SearchOfDesperation, reporter) {
    // Build proxy typeface on the client for initializing the cache.
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto tfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(tfData->data(), tfData->size());
    REPORTER_ASSERT(reporter, clientTf);

    SkFont font;
    font.setTypeface(clientTf);
    font.setSubpixel(true);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);

    auto lostGlyphID = SkPackedGlyphID(1, SK_FixedHalf, SK_FixedHalf);
    const uint8_t glyphImage[] = {0xFF, 0xFF};

    SkStrikeCache strikeCache;

    // Build a fallback cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findOrCreateStrikeExclusive(*desc, effects, *clientTf);
        SkGlyphPrototype proto = {lostGlyphID, 0.f, 0.f, 2, 1, 0, 0, SkMask::kA8_Format, false};
        fallbackCache->glyphFromPrototype(proto, (void*)glyphImage);
    }

    // Make sure we can find the fall back cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);
        auto testCache = strikeCache.findStrikeExclusive(*desc);
        REPORTER_ASSERT(reporter, !(testCache == nullptr));
    }

    // Create the target cache.
    SkExclusiveStrikePtr testCache;
    SkAutoDescriptor ad;
    SkScalerContextRec rec;
    SkScalerContextEffects effects;
    SkScalerContextFlags flags = SkScalerContextFlags::kNone;
    SkScalerContext::MakeRecAndEffects(
            font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
            SkMatrix::I(), &rec, &effects);
    auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);
    testCache = strikeCache.findStrikeExclusive(*desc);
    REPORTER_ASSERT(reporter, testCache == nullptr);
    testCache = strikeCache.createStrikeExclusive(*desc,
                                                     clientTf->createScalerContext(effects, desc));
    auto scalerProxy = static_cast<SkScalerContextProxy*>(testCache->getScalerContext());
    scalerProxy->initCache(testCache.get(), &strikeCache);

    // Look for the lost glyph.
    {
        SkPoint pt{SkFixedToScalar(lostGlyphID.getSubXFixed()),
                   SkFixedToScalar(lostGlyphID.getSubYFixed())};
        SkGlyph* lostGlyph = testCache->glyph(lostGlyphID.code(), pt);
        testCache->prepareImage(lostGlyph);

        REPORTER_ASSERT(reporter, lostGlyph->height() == 1);
        REPORTER_ASSERT(reporter, lostGlyph->width() == 2);
        REPORTER_ASSERT(reporter, lostGlyph->maskFormat() == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, memcmp(lostGlyph->image(), glyphImage, sizeof(glyphImage)) == 0);
    }

    // Look for the lost glyph with a different sub-pix position.
    {
        SkPoint pt{SkFixedToScalar(SK_FixedQuarter),
                   SkFixedToScalar(SK_FixedQuarter)};
        SkGlyph* lostGlyph = testCache->glyph(lostGlyphID.code(), pt);
        testCache->prepareImage(lostGlyph);

        REPORTER_ASSERT(reporter, lostGlyph->height() == 1);
        REPORTER_ASSERT(reporter, lostGlyph->width() == 2);
        REPORTER_ASSERT(reporter, lostGlyph->maskFormat() == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, memcmp(lostGlyph->image(), glyphImage, sizeof(glyphImage)) == 0);
    }

    for (uint32_t i = 0; i <= SkStrikeClient::CacheMissType::kLast; ++i) {
        if (i == SkStrikeClient::CacheMissType::kGlyphMetricsFallback ||
            i == SkStrikeClient::CacheMissType::kFontMetrics) {
            REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(i) == 2);
        } else {
            REPORTER_ASSERT(reporter, discardableManager->cacheMissCount(i) == 0);
        }
    }
    strikeCache.validateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_TEST(SkRemoteGlyphCache_ReWriteGlyph, reporter) {
    // Build proxy typeface on the client for initializing the cache.
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto tfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(tfData->data(), tfData->size());
    REPORTER_ASSERT(reporter, clientTf);

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint paint;
    paint.setColor(SK_ColorRED);

    auto lostGlyphID = SkPackedGlyphID(1, SK_FixedHalf, SK_FixedHalf);
    const uint8_t glyphImage[] = {0xFF, 0xFF};
    SkMask::Format realMask;
    SkMask::Format fakeMask;

    SkStrikeCache strikeCache;

    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        font.setTypeface(serverTf);
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto context = serverTf->createScalerContext(effects, desc, false);
        SkGlyph glyph{lostGlyphID};
        context->getMetrics(&glyph);
        realMask = glyph.maskFormat();
    }

    // Build a fallback cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        font.setTypeface(clientTf);
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfacePropsCopyOrDefault(nullptr), flags,
                SkMatrix::I(), &rec, &effects);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findOrCreateStrikeExclusive(*desc, effects, *clientTf);
        fakeMask = (realMask == SkMask::kA8_Format) ? SkMask::kBW_Format : SkMask::kA8_Format;
        SkGlyphPrototype proto = {lostGlyphID, 0.f, 0.f, 2, 1, 0, 0, fakeMask, false};
        fallbackCache->glyphFromPrototype(proto, (void *)glyphImage);
    }

    // Send over the real glyph and make sure the client cache stays intact.
    {
        SkAutoDescriptor ad;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        font.setTypeface(serverTf);
        auto* cacheState = server.getOrCreateCache(
                paint, font, SkSurfacePropsCopyOrDefault(nullptr),
                SkMatrix::I(), flags, &effects);
        SkStrikeServer::AddGlyphForTesting(cacheState, lostGlyphID, false);

        std::vector<uint8_t> serverStrikeData;
        server.writeStrikeData(&serverStrikeData);
        REPORTER_ASSERT(reporter,
                        client.readStrikeData(
                                serverStrikeData.data(),
                                serverStrikeData.size()));
    }

    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        font.setTypeface(clientTf);
        SkScalerContext::MakeRecAndEffects(
                font, paint, SkSurfaceProps(0, kUnknown_SkPixelGeometry), flags,
                SkMatrix::I(), &rec, &effects);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findStrikeExclusive(*desc);
        REPORTER_ASSERT(reporter, fallbackCache.get() != nullptr);
        auto glyph = fallbackCache->glyphOrNull(lostGlyphID);
        REPORTER_ASSERT(reporter, glyph && glyph->maskFormat() == fakeMask);
        if (glyph) {
            REPORTER_ASSERT(reporter,
                            memcmp(glyph->image(), glyphImage, glyph->imageSize()) == 0);
        }
    }

    strikeCache.validateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}
