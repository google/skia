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

    sk_sp<GrTextBlob> makeCachedBlob(const SkGlyphRunList& glyphRunList,
                                     const GrTextBlob::Key& key,
                                     const SkMaskFilterBase::BlurRec& blurRec,
                                     const SkMatrix& viewMatrix) SK_EXCLUDES(fMutex);

    sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) const SK_EXCLUDES(fMutex);

    void remove(GrTextBlob* blob) SK_EXCLUDES(fMutex);

    void makeMRU(GrTextBlob* blob) SK_EXCLUDES(fMutex);

    void freeAll() SK_EXCLUDES(fMutex);

    struct PurgeBlobMessage {
        PurgeBlobMessage(uint32_t blobID, uint32_t contextUniqueID)
                : fBlobID(blobID), fContextID(contextUniqueID) {}

        uint32_t fBlobID;
        uint32_t fContextID;
    };

    static void PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID);

    void purgeStaleBlobs() SK_EXCLUDES(fMutex);

    size_t usedBytes() const SK_EXCLUDES(fMutex);

    bool isOverBudget() const SK_EXCLUDES(fMutex);

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

    void internalPurgeStaleBlobs() SK_REQUIRES(fMutex);

    void internalAdd(sk_sp<GrTextBlob> blob) SK_REQUIRES(fMutex);
    void internalRemove(GrTextBlob* blob) SK_REQUIRES(fMutex);

    void internalCheckPurge(GrTextBlob* blob = nullptr) SK_REQUIRES(fMutex);

    static const int kDefaultBudget = 1 << 22;

    mutable SkMutex fMutex;
    TextBlobList fBlobList SK_GUARDED_BY(fMutex);
    SkTHashMap<uint32_t, BlobIDCacheEntry> fBlobIDCache SK_GUARDED_BY(fMutex);
    size_t fSizeBudget SK_GUARDED_BY(fMutex);
    size_t fCurrentSize SK_GUARDED_BY(fMutex) {0};

    // In practice 'messageBusID' is always the unique ID of the owning GrContext
    const uint32_t fMessageBusID;
    SkMessageBus<PurgeBlobMessage>::Inbox fPurgeBlobInbox SK_GUARDED_BY(fMutex);
};

#endif
