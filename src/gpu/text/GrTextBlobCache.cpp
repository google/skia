/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextBlobCache.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrTextBlobCache::PurgeBlobMessage)

GrTextBlobCache::~GrTextBlobCache() {
    SkDEBUGCODE(this->freeAll();)
}

void GrTextBlobCache::freeAll() {
    fBlobIDCache.foreach([this](uint32_t, BlobIDCacheEntry* entry) {
        for (const auto& blob : entry->fBlobs) {
            fBlobList.remove(blob.get());
        }
    });

    fBlobIDCache.reset();

    // There should be no allocations in the memory pool at this point
    SkASSERT(fPool.isEmpty());
    SkASSERT(fBlobList.isEmpty());
}

void GrTextBlobCache::PostPurgeBlobMessage(uint32_t id) {
    SkASSERT(id != SK_InvalidGenID);
    SkMessageBus<PurgeBlobMessage>::Post(PurgeBlobMessage({id}));
}
