/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPromiseImageTexture.h"
#include "SkMessageBus.h"

#if SK_SUPPORT_GPU

std::atomic<uint32_t> SkPromiseImageTexture::gUniqueID{1};

SkPromiseImageTexture::SkPromiseImageTexture(const GrBackendTexture& backendTexture) {
    if (backendTexture.isValid()) {
        fBackendTexture = backendTexture;
        fUniqueID = gUniqueID++;
    }
}

SkPromiseImageTexture::SkPromiseImageTexture(SkPromiseImageTexture&& that) {
    *this = std::move(that);
}

SkPromiseImageTexture& SkPromiseImageTexture::operator=(SkPromiseImageTexture&& that) {
    for (const auto& msg : fMessages) {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(msg);
    }
    fMessages = that.fMessages;
    that.fMessages.reset();
    fBackendTexture = that.fBackendTexture;
    that.fBackendTexture = {};
    fUniqueID = that.fUniqueID;
    that.fUniqueID = SK_InvalidUniqueID;
    return *this;
}

SkPromiseImageTexture::~SkPromiseImageTexture() {
    for (const auto& msg : fMessages) {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(msg);
    }
}

void SkPromiseImageTexture::addKeyToInvalidate(uint32_t contextID, const GrUniqueKey& key) {
    SkASSERT(contextID != SK_InvalidUniqueID);
    SkASSERT(key.isValid());
    fMessages.emplace_back(key, contextID);
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
