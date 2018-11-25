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

void GrTextBlobCache::PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID) {
    SkASSERT(blobID != SK_InvalidGenID);
    SkMessageBus<PurgeBlobMessage>::Post(PurgeBlobMessage({blobID}), cacheID);
}

void GrTextBlobCache::purgeStaleBlobs() {
    SkTArray<PurgeBlobMessage> msgs;
    fPurgeBlobInbox.poll(&msgs);

    for (const auto& msg : msgs) {
        auto* idEntry = fBlobIDCache.find(msg.fID);
        if (!idEntry) {
            // no cache entries for id
            continue;
        }

        // remove all blob entries from the LRU list
        for (const auto& blob : idEntry->fBlobs) {
            fBlobList.remove(blob.get());
        }

        // drop the idEntry itself (unrefs all blobs)
        fBlobIDCache.remove(msg.fID);
    }
}

void GrTextBlobCache::checkPurge(GrAtlasTextBlob* blob) {
    // First, purge all stale blob IDs.
    this->purgeStaleBlobs();

    // If we are still over budget, then unref until we are below budget again
    if (fPool.size() > fBudget) {
        BitmapBlobList::Iter iter;
        iter.init(fBlobList, BitmapBlobList::Iter::kTail_IterStart);
        GrAtlasTextBlob* lruBlob = nullptr;
        while (fPool.size() > fBudget && (lruBlob = iter.get()) && lruBlob != blob) {
            // Backup the iterator before removing and unrefing the blob
            iter.prev();

            this->remove(lruBlob);
        }

        // If we break out of the loop with lruBlob == blob, then we haven't purged enough
        // use the call back and try to free some more.  If we are still overbudget after this,
        // then this single textblob is over our budget
        if (blob && lruBlob == blob) {
            (*fCallback)(fData);
        }

#ifdef SPEW_BUDGET_MESSAGE
        if (fPool.size() > fBudget) {
            SkDebugf("Single textblob is larger than our whole budget");
        }
#endif
    }
}



