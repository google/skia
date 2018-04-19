/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRemoteGlyphCache.h"
#include "SkTypeface_remote.h"
#include "Test.h"

class DiscardableManager : public SkStrikeServer::DiscardableHandleManager,
                           public SkStrikeClient::DiscardableHandleManager {
public:
    DiscardableManager() = default;
    ~DiscardableManager() override = default;

    // Server implementation.
    SkDiscardableHandleId createHandle() override { return ++nextHandleId; }
    bool lockHandle(SkDiscardableHandleId) override { return true; }

    // Client implementation.
    bool deleteHandle(SkDiscardableHandleId) override { return false; }

    SkDiscardableHandleId next_handle_id() const { return nextHandleId; }

private:
    SkDiscardableHandleId nextHandleId = 0u;
};

DEF_TEST(SkRemoteGlyphCache_TypefaceSerialization, reporter) {
    DiscardableManager discardableManager;
    SkStrikeServer server(&discardableManager);
    sk_sp<SkStrikeClient> client = sk_make_sp<SkStrikeClient>(&discardableManager);

    auto server_tf = SkTypeface::MakeDefault();
    auto tf_data = server.serializeTypeface(server_tf.get());
    std::vector<uint8_t> fontData;
    server.writeStrikeData(&fontData);
    REPORTER_ASSERT(reporter, !fontData.empty());

    REPORTER_ASSERT(reporter, client->readStrikeData(fontData.data(), fontData.size()));
    auto client_tf = client->deserializeTypeface(tf_data->data(), tf_data->size());
    REPORTER_ASSERT(reporter, client_tf);
    REPORTER_ASSERT(reporter, SkTypefaceProxy::DownCast(client_tf.get())->remoteTypefaceID() ==
                                      server_tf->uniqueID());
}
