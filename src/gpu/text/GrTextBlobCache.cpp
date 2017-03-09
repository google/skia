/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextBlobCache.h"

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
