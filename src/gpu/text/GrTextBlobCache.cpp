/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/text/GrTextBlobCache.h"

DECLARE_SKMESSAGEBUS_MESSAGE(GrTextBlobCache::PurgeBlobMessage)

// This function is captured by the above macro using implementations from SkMessageBus.h
static inline bool SkShouldPostMessageToBus(
        const GrTextBlobCache::PurgeBlobMessage& msg, uint32_t msgBusUniqueID) {
    return msg.fContextID == msgBusUniqueID;
}

GrTextBlobCache::GrTextBlobCache(uint32_t messageBusID)
        : fSizeBudget(kDefaultBudget)
        , fMessageBusID(messageBusID)
        , fPurgeBlobInbox(messageBusID) { }

void GrTextBlobCache::add(const SkGlyphRunList& glyphRunList, sk_sp<GrTextBlob> blob) {
    SkAutoSpinlock lock{fSpinLock};
    this->internalAdd(std::move(blob));
    glyphRunList.temporaryShuntBlobNotifyAddedToCache(fMessageBusID);
}

sk_sp<GrTextBlob> GrTextBlobCache::find(const GrTextBlob::Key& key) {
    SkAutoSpinlock lock{fSpinLock};
    const BlobIDCacheEntry* idEntry = fBlobIDCache.find(key.fUniqueID);
    if (idEntry == nullptr) {
        return nullptr;
    }

    sk_sp<GrTextBlob> blob = idEntry->find(key);
    GrTextBlob* blobPtr = blob.get();
    if (blobPtr != nullptr && blobPtr != fBlobList.head()) {
        fBlobList.remove(blobPtr);
        fBlobList.addToHead(blobPtr);
    }
    return blob;
}

void GrTextBlobCache::remove(GrTextBlob* blob) {
    SkAutoSpinlock lock{fSpinLock};
    this->internalRemove(blob);
}

void GrTextBlobCache::internalRemove(GrTextBlob* blob) {
    auto  id      = GrTextBlob::GetKey(*blob).fUniqueID;
    auto* idEntry = fBlobIDCache.find(id);
    SkASSERT(idEntry);

    fCurrentSize -= blob->size();
    fBlobList.remove(blob);
    idEntry->removeBlob(blob);
    if (idEntry->fBlobs.empty()) {
        fBlobIDCache.remove(id);
    }
}

void GrTextBlobCache::freeAll() {
    SkAutoSpinlock lock{fSpinLock};
    fBlobIDCache.reset();
    fBlobList.reset();
    fCurrentSize = 0;
}

void GrTextBlobCache::PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID) {
    SkASSERT(blobID != SK_InvalidGenID);
    SkMessageBus<PurgeBlobMessage>::Post(PurgeBlobMessage(blobID, cacheID));
}

void GrTextBlobCache::purgeStaleBlobs() {
    SkAutoSpinlock lock{fSpinLock};
    this->internalPurgeStaleBlobs();
}

void GrTextBlobCache::internalPurgeStaleBlobs() {
    SkTArray<PurgeBlobMessage> msgs;
    fPurgeBlobInbox.poll(&msgs);

    for (const auto& msg : msgs) {
        auto* idEntry = fBlobIDCache.find(msg.fBlobID);
        if (!idEntry) {
            // no cache entries for id
            continue;
        }

        // remove all blob entries from the LRU list
        for (const auto& blob : idEntry->fBlobs) {
            fCurrentSize -= blob->size();
            fBlobList.remove(blob.get());
        }

        // drop the idEntry itself (unrefs all blobs)
        fBlobIDCache.remove(msg.fBlobID);
    }
}

size_t GrTextBlobCache::usedBytes() const {
    SkAutoSpinlock lock{fSpinLock};
    return fCurrentSize;
}

bool GrTextBlobCache::isOverBudget() const {
    SkAutoSpinlock lock{fSpinLock};
    return fCurrentSize > fSizeBudget;
}

void GrTextBlobCache::internalCheckPurge(GrTextBlob* blob) {
    // First, purge all stale blob IDs.
    this->internalPurgeStaleBlobs();

    // If we are still over budget, then unref until we are below budget again
    if (fCurrentSize > fSizeBudget) {
        TextBlobList::Iter iter;
        iter.init(fBlobList, TextBlobList::Iter::kTail_IterStart);
        GrTextBlob* lruBlob = nullptr;
        while (fCurrentSize > fSizeBudget && (lruBlob = iter.get()) && lruBlob != blob) {
            // Backup the iterator before removing and unrefing the blob
            iter.prev();

            this->internalRemove(lruBlob);
        }

    #ifdef SPEW_BUDGET_MESSAGE
        if (fCurrentSize > fSizeBudget) {
            SkDebugf("Single textblob is larger than our whole budget");
        }
    #endif
    }
}

void GrTextBlobCache::internalAdd(sk_sp<GrTextBlob> blob) {
    auto  id      = GrTextBlob::GetKey(*blob).fUniqueID;
    auto* idEntry = fBlobIDCache.find(id);
    if (!idEntry) {
        idEntry = fBlobIDCache.set(id, BlobIDCacheEntry(id));
    }

    // Safe to retain a raw ptr temporarily here, because the cache will hold a ref.
    GrTextBlob* rawBlobPtr = blob.get();
    fBlobList.addToHead(rawBlobPtr);
    fCurrentSize += blob->size();
    idEntry->addBlob(std::move(blob));

    this->internalCheckPurge(rawBlobPtr);
}

GrTextBlobCache::BlobIDCacheEntry::BlobIDCacheEntry() : fID(SK_InvalidGenID) {}

GrTextBlobCache::BlobIDCacheEntry::BlobIDCacheEntry(uint32_t id) : fID(id) {}

uint32_t GrTextBlobCache::BlobIDCacheEntry::GetKey(const GrTextBlobCache::BlobIDCacheEntry& entry) {
    return entry.fID;
}

void GrTextBlobCache::BlobIDCacheEntry::addBlob(sk_sp<GrTextBlob> blob) {
    SkASSERT(blob);
    SkASSERT(GrTextBlob::GetKey(*blob).fUniqueID == fID);
    SkASSERT(!this->find(GrTextBlob::GetKey(*blob)));

    fBlobs.emplace_back(std::move(blob));
}

void GrTextBlobCache::BlobIDCacheEntry::removeBlob(GrTextBlob* blob) {
    SkASSERT(blob);
    SkASSERT(GrTextBlob::GetKey(*blob).fUniqueID == fID);

    auto index = this->findBlobIndex(GrTextBlob::GetKey(*blob));
    SkASSERT(index >= 0);

    fBlobs.removeShuffle(index);
}

sk_sp<GrTextBlob> GrTextBlobCache::BlobIDCacheEntry::find(const GrTextBlob::Key& key) const {
    auto index = this->findBlobIndex(key);
    return index < 0 ? nullptr : fBlobs[index];
}

int GrTextBlobCache::BlobIDCacheEntry::findBlobIndex(const GrTextBlob::Key& key) const {
    for (int i = 0; i < fBlobs.count(); ++i) {
        if (GrTextBlob::GetKey(*fBlobs[i]) == key) {
            return i;
        }
    }
    return -1;
}
