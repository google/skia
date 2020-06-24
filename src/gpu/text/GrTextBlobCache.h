/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlobCache_DEFINED
#define GrTextBlobCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkMessageBus.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/gpu/text/GrTextBlob.h"

#include <functional>

class GrTextBlobCache {
public:
     // The callback function used by the cache when it is still over budget after a purge.
    using PurgeMore = std::function<void()>;

    GrTextBlobCache(PurgeMore purgeMore, uint32_t messageBusID);
    ~GrTextBlobCache();

    sk_sp<GrTextBlob> makeCachedBlob(const SkGlyphRunList& glyphRunList,
                                     const GrTextBlob::Key& key,
                                     const SkMaskFilterBase::BlurRec& blurRec,
                                     const SkMatrix& viewMatrix);

    sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) const;

    void remove(GrTextBlob* blob);

    void makeMRU(GrTextBlob* blob);

    void freeAll();

    void setBudget(size_t budget);

    struct PurgeBlobMessage {
        PurgeBlobMessage(uint32_t blobID, uint32_t contextUniqueID)
                : fBlobID(blobID), fContextID(contextUniqueID) {}

        uint32_t fBlobID;
        uint32_t fContextID;
    };

    static void PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID);

    void purgeStaleBlobs();

    size_t usedBytes() const;

private:
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

    void internalPurgeStaleBlobs();

    void internalAdd(sk_sp<GrTextBlob> blob);
    void internalRemove(GrTextBlob* blob);

    void internalCheckPurge(GrTextBlob* blob = nullptr);

    static const int kDefaultBudget = 1 << 22;

    TextBlobList fBlobList;
    SkTHashMap<uint32_t, BlobIDCacheEntry> fBlobIDCache;
    PurgeMore fPurgeMore;
    size_t fSizeBudget;
    size_t fCurrentSize{0};

    // In practice 'messageBusID' is always the unique ID of the owning GrContext
    uint32_t fMessageBusID;
    SkMessageBus<PurgeBlobMessage>::Inbox fPurgeBlobInbox;
};

#endif
