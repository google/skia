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

#include "text/GrTextContext.h"

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
    void notifyCacheMiss(SkStrikeClient::CacheMissType type) override { fCacheMissCount[type]++; }

    void unlockAll() { fLockedHandles.reset(); }
    void unlockAndDeleteAll() {
        unlockAll();
        fLastDeletedHandleId = fNextHandleId;
    }
    const SkTHashSet<SkDiscardableHandleId>& lockedHandles() const { return fLockedHandles; }
    SkDiscardableHandleId handleCount() { return fNextHandleId; }
    int cacheMissCount(uint32_t type) { return fCacheMissCount[type]; }
    bool hasCacheMiss() const {
        for (uint32_t i = 0; i <= SkStrikeClient::CacheMissType::kLast; ++i) {
            if (fCacheMissCount[i] > 0) return true;
        }
        return false;
    }

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

SkTextBlobCacheDiffCanvas::Settings MakeSettings(GrContext* context) {
    SkTextBlobCacheDiffCanvas::Settings settings;
    settings.fContextSupportsDistanceFieldText = context->supportsDistanceFieldText();
    settings.fMaxTextureSize = context->maxTextureSize();
    settings.fMaxTextureBytes = GrContextOptions().fGlyphCacheTextureMaximumBytes;
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
    REPORTER_ASSERT(reporter, !discardableManager->hasCacheMiss());

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
    SkStrikeClient client(discardableManager, false);

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
    SkStrikeClient client(discardableManager, false);

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
    SkStrikeClient client(discardableManager, false);

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
    SkStrikeCache::ValidateGlyphCacheDataSize();

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

DEF_TEST(SkRemoteGlyphCache_SearchOfDesperation, reporter) {
    // Build proxy typeface on the client for initializing the cache.
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager, false);

    auto serverTf = SkTypeface::MakeFromName("monospace", SkFontStyle());
    auto tfData = server.serializeTypeface(serverTf.get());
    auto clientTf = client.deserializeTypeface(tfData->data(), tfData->size());
    REPORTER_ASSERT(reporter, clientTf);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTypeface(clientTf);
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
        SkScalerContext::MakeRecAndEffects(paint, nullptr, nullptr, flags, &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findOrCreateStrikeExclusive(*desc, effects, *clientTf);
        auto glyph = fallbackCache->getRawGlyphByID(lostGlyphID);
        glyph->fMaskFormat = SkMask::kA8_Format;
        glyph->fHeight = 1;
        glyph->fWidth = 2;
        fallbackCache->initializeImage(glyphImage, glyph->computeImageSize(), glyph);
        glyph->fImage = (void *)glyphImage;
    }

    // Make sure we can find the fall back cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        SkScalerContext::MakeRecAndEffects(paint, nullptr, nullptr, flags, &rec, &effects, false);
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
    SkScalerContext::MakeRecAndEffects(paint, nullptr, nullptr, flags, &rec, &effects, false);
    auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);
    testCache = strikeCache.findStrikeExclusive(*desc);
    REPORTER_ASSERT(reporter, testCache == nullptr);
    testCache = strikeCache.createStrikeExclusive(*desc,
                                                     clientTf->createScalerContext(effects, desc));
    auto scalerProxy = static_cast<SkScalerContextProxy*>(testCache->getScalerContext());
    scalerProxy->initCache(testCache.get(), &strikeCache);

    // Look for the lost glyph.
    {
        const auto& lostGlyph = testCache->getGlyphIDMetrics(
                lostGlyphID.code(), lostGlyphID.getSubXFixed(), lostGlyphID.getSubYFixed());
        testCache->findImage(lostGlyph);

        REPORTER_ASSERT(reporter, lostGlyph.fHeight == 1);
        REPORTER_ASSERT(reporter, lostGlyph.fWidth == 2);
        REPORTER_ASSERT(reporter, lostGlyph.fMaskFormat == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, memcmp(lostGlyph.fImage, glyphImage, sizeof(glyphImage)) == 0);
    }

    // Look for the lost glyph with a different sub-pix position.
    {
        const auto& lostGlyph =
                testCache->getGlyphIDMetrics(lostGlyphID.code(), SK_FixedQuarter, SK_FixedQuarter);
        testCache->findImage(lostGlyph);

        REPORTER_ASSERT(reporter, lostGlyph.fHeight == 1);
        REPORTER_ASSERT(reporter, lostGlyph.fWidth == 2);
        REPORTER_ASSERT(reporter, lostGlyph.fMaskFormat == SkMask::kA8_Format);
        REPORTER_ASSERT(reporter, memcmp(lostGlyph.fImage, glyphImage, sizeof(glyphImage)) == 0);
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

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);

    auto lostGlyphID = SkPackedGlyphID(1, SK_FixedHalf, SK_FixedHalf);
    const uint8_t glyphImage[] = {0xFF, 0xFF};
    uint32_t realMask;
    uint32_t fakeMask;

    SkStrikeCache strikeCache;

    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        paint.setTypeface(serverTf);
        SkScalerContext::MakeRecAndEffects(paint, nullptr, nullptr, flags, &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto context = serverTf->createScalerContext(effects, desc, false);
        SkGlyph glyph;
        glyph.initWithGlyphID(lostGlyphID);
        context->getMetrics(&glyph);
        realMask = glyph.fMaskFormat;
        REPORTER_ASSERT(reporter, realMask != MASK_FORMAT_UNKNOWN);
    }

    // Build a fallback cache.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        paint.setTypeface(clientTf);
        SkScalerContext::MakeRecAndEffects(paint, nullptr, nullptr, flags, &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findOrCreateStrikeExclusive(*desc, effects, *clientTf);
        auto glyph = fallbackCache->getRawGlyphByID(lostGlyphID);
        fakeMask = (realMask == SkMask::kA8_Format) ? SkMask::kBW_Format : SkMask::kA8_Format;
        glyph->fMaskFormat = fakeMask;
        glyph->fHeight = 1;
        glyph->fWidth = 2;
        fallbackCache->initializeImage(glyphImage, glyph->computeImageSize(), glyph);
    }

    // Send over the real glyph and make sure the client cache stays intact.
    {
        SkAutoDescriptor ad;
        SkScalerContextRec rec;
        SkScalerContextEffects effects;
        SkScalerContextFlags flags = SkScalerContextFlags::kFakeGammaAndBoostContrast;
        paint.setTypeface(serverTf);
        auto* cacheState = server.getOrCreateCache(paint, nullptr, nullptr, flags, &rec, &effects);
        cacheState->addGlyph(serverTf.get(), effects, lostGlyphID, false);

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
        paint.setTypeface(clientTf);
        SkScalerContext::MakeRecAndEffects(paint, nullptr, nullptr, flags, &rec, &effects, false);
        auto desc = SkScalerContext::AutoDescriptorGivenRecAndEffects(rec, effects, &ad);

        auto fallbackCache = strikeCache.findStrikeExclusive(*desc);
        REPORTER_ASSERT(reporter, fallbackCache.get() != nullptr);
        auto glyph = fallbackCache->getRawGlyphByID(lostGlyphID);
        REPORTER_ASSERT(reporter, glyph->fMaskFormat == fakeMask);

        // Try overriding the image, it should stay the same.
        REPORTER_ASSERT(reporter,
                        memcmp(glyph->fImage, glyphImage, glyph->computeImageSize()) == 0);
        const uint8_t newGlyphImage[] = {0, 0};
        fallbackCache->initializeImage(newGlyphImage, glyph->computeImageSize(), glyph);
        REPORTER_ASSERT(reporter,
                        memcmp(glyph->fImage, glyphImage, glyph->computeImageSize()) == 0);
    }

    strikeCache.validateGlyphCacheDataSize();

    // Must unlock everything on termination, otherwise valgrind complains about memory leaks.
    discardableManager->unlockAndDeleteAll();
}
