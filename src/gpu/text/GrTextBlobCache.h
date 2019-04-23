/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlobCache_DEFINED
#define GrTextBlobCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkMessageBus.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/gpu/text/GrTextBlob.h"

class GrTextBlobCache {
public:
    /**
     * The callback function used by the cache when it is still over budget after a purge. The
     * passed in 'data' is the same 'data' handed to setOverbudgetCallback.
     */
    typedef void (*PFOverBudgetCB)(void* data);

    GrTextBlobCache(PFOverBudgetCB cb, void* data, uint32_t uniqueID)
        : fCallback(cb)
        , fData(data)
        , fSizeBudget(kDefaultBudget)
        , fUniqueID(uniqueID)
        , fPurgeBlobInbox(uniqueID) {
        SkASSERT(cb && data);
    }
    ~GrTextBlobCache();

    sk_sp<GrTextBlob> makeBlob(const SkGlyphRunList& glyphRunList,
                               GrColor color,
                               GrStrikeCache* strikeCache) {
        return GrTextBlob::Make(
                glyphRunList.totalGlyphCount(), glyphRunList.size(), color, strikeCache);
    }

    sk_sp<GrTextBlob> makeCachedBlob(const SkGlyphRunList& glyphRunList,
                                     const GrTextBlob::Key& key,
                                     const SkMaskFilterBase::BlurRec& blurRec,
                                     const SkPaint& paint,
                                     GrColor color,
                                     GrStrikeCache* strikeCache) {
        sk_sp<GrTextBlob> cacheBlob(makeBlob(glyphRunList, color, strikeCache));
        cacheBlob->setupKey(key, blurRec, paint);
        this->add(cacheBlob);
        glyphRunList.temporaryShuntBlobNotifyAddedToCache(fUniqueID);
        return cacheBlob;
    }

    sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) const {
        const auto* idEntry = fBlobIDCache.find(key.fUniqueID);
        return idEntry ? idEntry->find(key) : nullptr;
    }

    void remove(GrTextBlob* blob) {
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

    void makeMRU(GrTextBlob* blob) {
        if (fBlobList.head() == blob) {
            return;
        }

        fBlobList.remove(blob);
        fBlobList.addToHead(blob);
    }

    void freeAll();

    // TODO move to SkTextBlob
    static void BlobGlyphCount(int* glyphCount, int* runCount, const SkTextBlob* blob) {
        SkTextBlobRunIterator itCounter(blob);
        for (; !itCounter.done(); itCounter.next(), (*runCount)++) {
            *glyphCount += itCounter.glyphCount();
        }
    }

    void setBudget(size_t budget) {
        fSizeBudget = budget;
        this->checkPurge();
    }

    struct PurgeBlobMessage {
        PurgeBlobMessage(uint32_t blobID, uint32_t contextUniqueID)
                : fBlobID(blobID), fContextID(contextUniqueID) {}

        uint32_t fBlobID;
        uint32_t fContextID;
    };

    static void PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID);

    void purgeStaleBlobs();

    size_t usedBytes() const { return fCurrentSize; }

private:
    using BitmapBlobList = SkTInternalLList<GrTextBlob>;

    struct BlobIDCacheEntry {
        BlobIDCacheEntry() : fID(SK_InvalidGenID) {}
        explicit BlobIDCacheEntry(uint32_t id) : fID(id) {}

        static uint32_t GetKey(const BlobIDCacheEntry& entry) {
            return entry.fID;
        }

        void addBlob(sk_sp<GrTextBlob> blob) {
            SkASSERT(blob);
            SkASSERT(GrTextBlob::GetKey(*blob).fUniqueID == fID);
            SkASSERT(!this->find(GrTextBlob::GetKey(*blob)));

            fBlobs.emplace_back(std::move(blob));
        }

        void removeBlob(GrTextBlob* blob) {
            SkASSERT(blob);
            SkASSERT(GrTextBlob::GetKey(*blob).fUniqueID == fID);

            auto index = this->findBlobIndex(GrTextBlob::GetKey(*blob));
            SkASSERT(index >= 0);

            fBlobs.removeShuffle(index);
        }

        sk_sp<GrTextBlob> find(const GrTextBlob::Key& key) const {
            auto index = this->findBlobIndex(key);
            return index < 0 ? nullptr : fBlobs[index];
        }

        int findBlobIndex(const GrTextBlob::Key& key) const{
            for (int i = 0; i < fBlobs.count(); ++i) {
                if (GrTextBlob::GetKey(*fBlobs[i]) == key) {
                    return i;
                }
            }
            return -1;
        }

        uint32_t                             fID;
        // Current clients don't generate multiple GrAtlasTextBlobs per SkTextBlob, so an array w/
        // linear search is acceptable.  If usage changes, we should re-evaluate this structure.
        SkSTArray<1, sk_sp<GrTextBlob>, true> fBlobs;
    };

    void add(sk_sp<GrTextBlob> blob) {
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

        this->checkPurge(rawBlobPtr);
    }

    void checkPurge(GrTextBlob* blob = nullptr);

    static const int kMinGrowthSize = 1 << 16;
    static const int kDefaultBudget = 1 << 22;
    BitmapBlobList fBlobList;
    SkTHashMap<uint32_t, BlobIDCacheEntry> fBlobIDCache;
    PFOverBudgetCB fCallback;
    void* fData;
    size_t fSizeBudget;
    size_t fCurrentSize{0};
    uint32_t fUniqueID;      // unique id to use for messaging
    SkMessageBus<PurgeBlobMessage>::Inbox fPurgeBlobInbox;
};

#endif
