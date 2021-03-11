/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPromiseImageTexture.h"
#include "src/core/SkMessageBus.h"

#if SK_SUPPORT_GPU

std::atomic<uint32_t> SkPromiseImageTexture::gUniqueID{1};

SkPromiseImageTexture::SkPromiseImageTexture(const GrBackendTexture& backendTexture) {
    SkASSERT(backendTexture.isValid());
    fBackendTexture = backendTexture;
    fUniqueID = gUniqueID.fetch_add(1, std::memory_order_relaxed);
}

SkPromiseImageTexture::~SkPromiseImageTexture() {
    for (const auto& msg : fMessages) {
        SkMessageBus<GrUniqueKeyInvalidatedMessage, uint32_t>::Post(msg);
    }
}

void SkPromiseImageTexture::addKeyToInvalidate(uint32_t contextID, const GrUniqueKey& key) {
    SkASSERT(contextID != SK_InvalidUniqueID);
    SkASSERT(key.isValid());
    for (const auto& msg : fMessages) {
        if (msg.contextID() == contextID && msg.key() == key) {
            return;
        }
    }
    fMessages.emplace_back(key, contextID, /* inThreadSafeCache */ false);
}

#if GR_TEST_UTILS
SkTArray<GrUniqueKey> SkPromiseImageTexture::testingOnly_uniqueKeysToInvalidate() const {
    SkTArray<GrUniqueKey> results;
    for (const auto& msg : fMessages) {
        results.push_back(msg.key());
    }
    return results;
}
#endif

#endif
