/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGraphics.h"
#include "SkMutex.h"
#include "SkRemoteGlyphCache.h"
#include "SkStrikeCache.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkTypeface_remote.h"
#include "Test.h"

class DiscardableManager : public SkStrikeServer::DiscardableHandleManager,
                           public SkStrikeClient::DiscardableHandleManager {
public:
    DiscardableManager() = default;
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

    void unlockAll() { fLockedHandles.reset(); }
    void unlockAndDeleteAll() {
        unlockAll();
        fLastDeletedHandleId = fNextHandleId;
    }
    const SkTHashSet<SkDiscardableHandleId>& lockedHandles() const { return fLockedHandles; }
    SkDiscardableHandleId handleCount() { return fNextHandleId; }

private:
    SkDiscardableHandleId fNextHandleId = 0u;
    SkDiscardableHandleId fLastDeletedHandleId = 0u;
    SkTHashSet<SkDiscardableHandleId> fLockedHandles;
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

SkBitmap RasterBlob(sk_sp<SkTextBlob> blob, int width, int height) {
    auto surface = SkSurface::MakeRasterN32Premul(width, height);
    SkPaint paint;
    surface->getCanvas()->drawTextBlob(blob.get(), 0u, 0u, paint);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    surface->readPixels(bitmap, 0, 0);
    return bitmap;
}

DEF_TEST(SkRemoteGlyphCache_TypefaceSerialization, reporter) {
    sk_sp<DiscardableManager> discardableManager = sk_make_sp<DiscardableManager>();
    SkStrikeServer server(discardableManager.get());
    SkStrikeClient client(discardableManager);

    auto server_tf = SkTypeface::MakeDefault();
    auto tf_data = server.serializeTypeface(server_tf.get());

    auto client_tf = client.deserializeTypeface(tf_data->data(), tf_data->size());
    REPORTER_ASSERT(reporter, client_tf);
    REPORTER_ASSERT(reporter, SkTypefaceProxy::DownCast(client_tf.get())->remoteTypefaceID() ==
                                      server_tf->uniqueID());
}

DEF_TEST(SkRemoteGlyphCache_StrikeSerialization, reporter) {
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
    auto clientTf = client.deserializeTypeface(serverTfData->data(), serverTfData->size());
    REPORTER_ASSERT(reporter,
                    client.readStrikeData(serverStrikeData.data(), serverStrikeData.size()));
    auto clientBlob = buildTextBlob(clientTf, glyphCount);

    SkBitmap expected = RasterBlob(serverBlob, 10, 10);
    SkBitmap actual = RasterBlob(clientBlob, 10, 10);
    for (int i = 0; i < expected.width(); ++i) {
        for (int j = 0; j < expected.height(); ++j) {
            REPORTER_ASSERT(reporter, expected.getColor(i, j) == actual.getColor(i, j));
        }
    }
}

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
}
