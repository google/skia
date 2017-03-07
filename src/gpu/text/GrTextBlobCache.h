/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlobCache_DEFINED
#define GrTextBlobCache_DEFINED

#include "GrAtlasTextContext.h"
#include "SkTArray.h"
#include "SkTextBlobRunIterator.h"
#include "SkTHash.h"

class GrTextBlobCache {
public:
    /**
     * The callback function used by the cache when it is still over budget after a purge. The
     * passed in 'data' is the same 'data' handed to setOverbudgetCallback.
     */
    typedef void (*PFOverBudgetCB)(void* data);

    GrTextBlobCache(PFOverBudgetCB cb, void* data)
        : fPool(kPreAllocSize, kMinGrowthSize)
        , fCallback(cb)
        , fData(data)
        , fBudget(kDefaultBudget) {
        SkASSERT(cb && data);
    }
    ~GrTextBlobCache();

    // creates an uncached blob
    GrAtlasTextBlob* createBlob(int glyphCount, int runCount) {
        return GrAtlasTextBlob::Create(&fPool, glyphCount, runCount);
    }
    GrAtlasTextBlob* createBlob(const SkTextBlob* blob) {
        int glyphCount = 0;
        int runCount = 0;
        BlobGlyphCount(&glyphCount, &runCount, blob);
        GrAtlasTextBlob* cacheBlob = GrAtlasTextBlob::Create(&fPool, glyphCount, runCount);
        return cacheBlob;
    }

    GrAtlasTextBlob* createCachedBlob(const SkTextBlob* blob,
                                      const GrAtlasTextBlob::Key& key,
                                      const SkMaskFilter::BlurRec& blurRec,
                                      const SkPaint& paint) {
        int glyphCount = 0;
        int runCount = 0;
        BlobGlyphCount(&glyphCount, &runCount, blob);
        GrAtlasTextBlob* cacheBlob = GrAtlasTextBlob::Create(&fPool, glyphCount, runCount);
        cacheBlob->setupKey(key, blurRec, paint);
        this->add(cacheBlob);
        return cacheBlob;
    }

    GrAtlasTextBlob* find(const GrAtlasTextBlob::Key& key) const {
        const auto* idEntry = fBlobIDCache.find(key.fUniqueID);
        return idEntry ? idEntry->find(key) : nullptr;
    }

    void remove(GrAtlasTextBlob* blob) {
        auto  id      = GrAtlasTextBlob::GetKey(*blob).fUniqueID;
        auto* idEntry = fBlobIDCache.find(id);
        SkASSERT(idEntry);

        idEntry->removeBlob(blob);
        if (idEntry->fBlobs.empty()) {
            fBlobIDCache.remove(id);
        }

        fBlobList.remove(blob);
        blob->unref();
    }

    void add(GrAtlasTextBlob* blob) {
        auto  id      = GrAtlasTextBlob::GetKey(*blob).fUniqueID;
        auto* idEntry = fBlobIDCache.find(id);
        if (!idEntry) {
            idEntry = fBlobIDCache.set(id, BlobIDCacheEntry(id));
        }

        idEntry->addBlob(blob);
        fBlobList.addToHead(blob);

        this->checkPurge(blob);
    }

    void makeMRU(GrAtlasTextBlob* blob) {
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
        fBudget = budget;
        this->checkPurge();
    }

private:
    using BitmapBlobList = SkTInternalLList<GrAtlasTextBlob>;

    struct BlobIDCacheEntry {
        BlobIDCacheEntry() : fID(SK_InvalidGenID) {}
        explicit BlobIDCacheEntry(uint32_t id) : fID(id) {}

        static uint32_t GetKey(const BlobIDCacheEntry& entry) {
            return entry.fID;
        }

        void addBlob(GrAtlasTextBlob* blob) {
            SkASSERT(blob);
            SkASSERT(GrAtlasTextBlob::GetKey(*blob).fUniqueID == fID);
            SkASSERT(!this->find(GrAtlasTextBlob::GetKey(*blob)));

            fBlobs.push_back(blob);
        }

        void removeBlob(GrAtlasTextBlob* blob) {
            SkASSERT(blob);
            SkASSERT(GrAtlasTextBlob::GetKey(*blob).fUniqueID == fID);

            auto index = this->findBlobIndex(GrAtlasTextBlob::GetKey(*blob));
            SkASSERT(index >= 0);

            fBlobs.removeShuffle(index);
        }

        GrAtlasTextBlob* find(const GrAtlasTextBlob::Key& key) const {
            auto index = this->findBlobIndex(key);
            return index < 0 ? nullptr : fBlobs[index];
        }

        int findBlobIndex(const GrAtlasTextBlob::Key& key) const{
            for (int i = 0; i < fBlobs.count(); ++i) {
                if (GrAtlasTextBlob::GetKey(*fBlobs[i]) == key) {
                    return i;
                }
            }
            return -1;
        }

        uint32_t                             fID;
        // Current clients don't generate multiple GrAtlasTextBlobs per SkTextBlob, so an array w/
        // linear search is acceptable.  If usage changes, we should re-evaluate this structure.
        SkSTArray<1, GrAtlasTextBlob*, true> fBlobs;
    };

    void checkPurge(GrAtlasTextBlob* blob = nullptr) {
        // If we are overbudget, then unref until we are below budget again
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

    // Budget was chosen to be ~4 megabytes.  The min alloc and pre alloc sizes in the pool are
    // based off of the largest cached textblob I have seen in the skps(a couple of kilobytes).
    static const int kPreAllocSize = 1 << 17;
    static const int kMinGrowthSize = 1 << 17;
    static const int kDefaultBudget = 1 << 22;
    BitmapBlobList fBlobList;
    SkTHashMap<uint32_t, BlobIDCacheEntry> fBlobIDCache;
    GrMemoryPool fPool;
    PFOverBudgetCB fCallback;
    void* fData;
    size_t fBudget;
};

#endif
