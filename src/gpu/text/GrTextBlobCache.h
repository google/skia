/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlobCache_DEFINED
#define GrTextBlobCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/gpu/text/GrTextBlob.h"

#include <functional>

class GrTextBlobCache {
public:
    GrTextBlobCache(uint32_t messageBusID);

    // If not already in the cache, then add it else, return the text blob from the cache.
    sk_sp<GrTextBlob> addOrReturnExisting(
            const SkGlyphRunList& glyphRunList, sk_sp<GrTextBlob> blob) SK_EXCLUDES(fSpinLock);

    sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) SK_EXCLUDES(fSpinLock);

    void remove(GrTextBlob* blob) SK_EXCLUDES(fSpinLock);

    void freeAll() SK_EXCLUDES(fSpinLock);

    struct PurgeBlobMessage {
        PurgeBlobMessage(uint32_t blobID, uint32_t contextUniqueID)
                : fBlobID(blobID), fContextID(contextUniqueID) {}

        uint32_t fBlobID;
        uint32_t fContextID;
    };

    static void PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID);

    void purgeStaleBlobs() SK_EXCLUDES(fSpinLock);

    size_t usedBytes() const SK_EXCLUDES(fSpinLock);

    bool isOverBudget() const SK_EXCLUDES(fSpinLock);

private:
    friend class GrTextBlobTestingPeer;
    using TextBlobList = SkTInternalLList<GrTextBlob>;

    struct BlobIDCacheEntry {
        BlobIDCacheEntry();
        explicit BlobIDCacheEntry(uint32_t id);

        static uint32_t GetKey(const BlobIDCacheEntry& entry);

        void addBlob(sk_sp<GrTextBlob> blob);

        void removeBlob(GrTextBlob* blob);

        sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) const;

        int findBlobIndex(const GrTextBlob::Key& key) const;

        uint32_t fID;
        // Current clients don't generate multiple GrAtlasTextBlobs per SkTextBlob, so an array w/
        // linear search is acceptable.  If usage changes, we should re-evaluate this structure.
        SkSTArray<1, sk_sp<GrTextBlob>> fBlobs;
    };

    void internalPurgeStaleBlobs() SK_REQUIRES(fSpinLock);

    sk_sp<GrTextBlob> internalAdd(sk_sp<GrTextBlob> blob) SK_REQUIRES(fSpinLock);
    void internalRemove(GrTextBlob* blob) SK_REQUIRES(fSpinLock);

    void internalCheckPurge(GrTextBlob* blob = nullptr) SK_REQUIRES(fSpinLock);

    static const int kDefaultBudget = 1 << 22;

    mutable SkSpinlock fSpinLock;
    TextBlobList fBlobList SK_GUARDED_BY(fSpinLock);
    SkTHashMap<uint32_t, BlobIDCacheEntry> fBlobIDCache SK_GUARDED_BY(fSpinLock);
    size_t fSizeBudget SK_GUARDED_BY(fSpinLock);
    size_t fCurrentSize SK_GUARDED_BY(fSpinLock) {0};

    // In practice 'messageBusID' is always the unique ID of the owning GrContext
    const uint32_t fMessageBusID;
    SkMessageBus<PurgeBlobMessage, uint32_t>::Inbox fPurgeBlobInbox SK_GUARDED_BY(fSpinLock);
};

#endif
