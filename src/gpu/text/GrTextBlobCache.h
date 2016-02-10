/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextBlobCache_DEFINED
#define GrTextBlobCache_DEFINED

#include "GrAtlasTextContext.h"
#include "SkTDynamicHash.h"
#include "SkTextBlobRunIterator.h"

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

    GrAtlasTextBlob* find(const GrAtlasTextBlob::Key& key) {
        return fCache.find(key);
    }

    void remove(GrAtlasTextBlob* blob) {
        fCache.remove(blob->key());
        fBlobList.remove(blob);
        blob->unref();
    }

    void add(GrAtlasTextBlob* blob) {
        fCache.add(blob);
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
    typedef SkTInternalLList<GrAtlasTextBlob> BitmapBlobList;

    void checkPurge(GrAtlasTextBlob* blob = nullptr) {
        // If we are overbudget, then unref until we are below budget again
        if (fPool.size() > fBudget) {
            BitmapBlobList::Iter iter;
            iter.init(fBlobList, BitmapBlobList::Iter::kTail_IterStart);
            GrAtlasTextBlob* lruBlob = nullptr;
            while (fPool.size() > fBudget && (lruBlob = iter.get()) && lruBlob != blob) {
                fCache.remove(lruBlob->key());

                // Backup the iterator before removing and unrefing the blob
                iter.prev();
                fBlobList.remove(lruBlob);
                lruBlob->unref();
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
    SkTDynamicHash<GrAtlasTextBlob, GrAtlasTextBlob::Key> fCache;
    GrMemoryPool fPool;
    PFOverBudgetCB fCallback;
    void* fData;
    size_t fBudget;
};

#endif
