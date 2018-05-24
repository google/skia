/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDraw.h"
#include "SkGraphics.h"
#include "SkMutex.h"
#include "SkRemoteGlyphCache.h"
#include "SkStrikeCache.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkTypeface_remote.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "text/GrTextContext.h"
#endif

class DiscardableManager : public SkStrikeServer::DiscardableHandleManager,
                           public SkStrikeClient::DiscardableHandleManager {
public:
    DiscardableManager() { sk_bzero(&fCacheMissCount, sizeof(fCacheMissCount)); }
    ~DiscardableManager() override = default;

    // Server implementation.
    SkDiscardableHandleId createHandle() override {
        // Handles starts as locked.
        fLockedHandles.add(++fNextHandleId);
        return fNextHandleId;
    }
    bool lockHandle(SkDiscardableHandleId id) override {
        if (id <= fLastDeletedHandleId) return false;
        fLockedHandles.add(id);
        return true;
    }

    // Client implementation.
    bool deleteHandle(SkDiscardableHandleId id) override { return id <= fLastDeletedHandleId; }
    void NotifyCacheMiss(SkStrikeClient::CacheMissType type) override { fCacheMissCount[type]++; }

    void unlockAll() { fLockedHandles.reset(); }
    void unlockAndDeleteAll() {
        unlockAll();
        fLastDeletedHandleId = fNextHandleId;
    }
    const SkTHashSet<SkDiscardableHandleId>& lockedHandles() const { return fLockedHandles; }
    SkDiscardableHandleId handleCount() { return fNextHandleId; }
    int cacheMissCount(SkStrikeClient::CacheMissType type) { return fCacheMissCount[type]; }

private:
    SkDiscardableHandleId fNextHandleId = 0u;
    SkDiscardableHandleId fLastDeletedHandleId = 0u;
    SkTHashSet<SkDiscardableHandleId> fLockedHandles;
    int fCacheMissCount[SkStrikeClient::CacheMissType::kLast + 1u];
};

sk_sp<SkTextBlob> buildTextBlob(sk_sp<SkTypeface> tf, int glyphCount) {
    SkPaint font;
    font.setTypeface(tf);
    font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    font.setTextAlign(SkPaint::kLeft_Align);
    font.setStyle(SkPaint::kFill_Style);
    font.setHinting(SkPaint::kNormal_Hinting);
    font.setTextSize(1u);

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

#define COMPARE_BLOBS(expected, actual, reporter)                                        \
    for (int i = 0; i < expected.width(); ++i) {                                         \
        for (int j = 0; j < expected.height(); ++j) {                                    \
            REPORTER_ASSERT(reporter, expected.getColor(i, j) == actual.getColor(i, j)); \
        }                                                                                \
    }

#if SK_SUPPORT_GPU
SkTextBlobCacheDiffCanvas::Settings MakeSettings(GrContext* context) {
    SkTextBlobCacheDiffCanvas::Settings settings;
    settings.fContextSupportsDistanceFieldText = context->supportsDistanceFieldText();
    return settings;
}

SkBitmap RasterBlob(sk_sp<SkTextBlob> blob, int width, int height, const SkPaint& paint,
                    GrContext* context, const SkMatrix* matrix = nullptr) {
    const SkImageInfo info =
            SkImageInfo::Make(width, height, kN32_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info);
    if (matrix) surface->getCanvas()->concat(*matrix);
    surface->getCanvas()->drawTextBlob(blob.get(), 0u, 0u, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}
#endif

DEF_TEST(SkRemoteGlyphCache_TypefaceSerialization, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager);

    auto server_tf = SkTypeface::MakeDefault();
    auto tf_data = server.serializeTypeface(server_tf.get());

    auto client_tf = client.deserializeTypeface(tf_data->data(), tf_data->size());
    REPORTER_ASSERT(reporter, client_tf);
    REPORTER_ASSERT(reporter, static_cast<SkTypefaceProxy*>(client_tf.get())->remoteTypefaceID() ==
                                      server_tf->uniqueID());

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_StrikeSerialization, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager);
    const SkPaint paint;

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
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
    COMPARE_BLOBS(expected, actual, reporter);

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}
#endif

DEF_TEST(SkRemoteGlyphCache_StrikeLockingServer, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    server.serializeTypeface(serverTf.get());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
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
    SkStrikeClient client(discardableManager);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    server.serializeTypeface(serverTf.get());
    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
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
    SkStrikeClient client(discardableManager);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
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
    SkStrikeClient client(discardableManager);

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);

    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server);
    SkPaint paint;
    cache_diff_canvas.drawTextBlob(serverBlob.get(), 0, 0, paint);

    std::vector<uint8_t> serverStrikeData;
    server.writeStrikeData(&serverStrikeData);

    // Client.
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    SkStrikeCache::Validate();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsPath, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0);
    REPORTER_ASSERT(reporter, SkDraw::ShouldDrawTextAsPaths(paint, SkMatrix::I()));

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
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
    COMPARE_BLOBS(expected, actual, reporter);
    SkStrikeCache::Validate();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}
#endif

#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_DrawTextAsDFT, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager);
    SkPaint paint;

    // A perspective transform forces fallback to dft.
    SkMatrix matrix = SkMatrix::I();
    matrix[SkMatrix::kMPersp0] = 0.5f;
    REPORTER_ASSERT(reporter, matrix.hasPerspective());
    SkSurfaceProps surfaceProps(0, kUnknown_SkPixelGeometry);
    GrTextContext::Options options;
    GrTextContext::SanitizeOptions(&options);
    REPORTER_ASSERT(reporter, GrTextContext::CanDrawAsDistanceFields(
                                      paint, matrix, surfaceProps, true, options));

    // Server.
    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto serverTfData = server.serializeTypeface(serverTf.get());

    int glyphCount = 10;
    auto serverBlob = buildTextBlob(serverTf, glyphCount);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkTextBlobCacheDiffCanvas cache_diff_canvas(10, 10, SkMatrix::I(), props, &server,
                                                MakeSettings(ctxInfo.grContext()));
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
    COMPARE_BLOBS(expected, actual, reporter);
    SkStrikeCache::Validate();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(SkRemoteGlyphCache_CacheMissReporting, reporter, ctxInfo) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager);

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
#endif
