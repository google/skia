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
#include "SkTextBlob.h"

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
    GrAtlasTextBlob* createBlob(int glyphCount, int runCount, size_t maxVASize);
    GrAtlasTextBlob* createBlob(const SkTextBlob* blob, size_t maxVAStride) {
        int glyphCount = 0;
        int runCount = 0;
        BlobGlyphCount(&glyphCount, &runCount, blob);
        GrAtlasTextBlob* cacheBlob = this->createBlob(glyphCount, runCount, maxVAStride);
        return cacheBlob;
    }

    static void SetupCacheBlobKey(GrAtlasTextBlob* cacheBlob,
                                  const GrAtlasTextBlob::Key& key,
                                  const SkMaskFilter::BlurRec& blurRec,
                                  const SkPaint& paint) {
        cacheBlob->fKey = key;
        if (key.fHasBlur) {
            cacheBlob->fBlurRec = blurRec;
        }
        if (key.fStyle != SkPaint::kFill_Style) {
            cacheBlob->fStrokeInfo.fFrameWidth = paint.getStrokeWidth();
            cacheBlob->fStrokeInfo.fMiterLimit = paint.getStrokeMiter();
            cacheBlob->fStrokeInfo.fJoin = paint.getStrokeJoin();
        }
    }

    GrAtlasTextBlob* createCachedBlob(const SkTextBlob* blob,
                                      const GrAtlasTextBlob::Key& key,
                                      const SkMaskFilter::BlurRec& blurRec,
                                      const SkPaint& paint,
                                      size_t maxVAStride) {
        int glyphCount = 0;
        int runCount = 0;
        BlobGlyphCount(&glyphCount, &runCount, blob);
        GrAtlasTextBlob* cacheBlob = this->createBlob(glyphCount, runCount, maxVAStride);
        SetupCacheBlobKey(cacheBlob, key, blurRec, paint);
        this->add(cacheBlob);
        return cacheBlob;
    }

    GrAtlasTextBlob* find(const GrAtlasTextBlob::Key& key) {
        return fCache.find(key);
    }

    void remove(GrAtlasTextBlob* blob) {
        fCache.remove(blob->fKey);
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
        SkTextBlob::RunIterator itCounter(blob);
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

    void checkPurge(GrAtlasTextBlob* blob = NULL) {
        // If we are overbudget, then unref until we are below budget again
        if (fPool.size() > fBudget) {
            BitmapBlobList::Iter iter;
            iter.init(fBlobList, BitmapBlobList::Iter::kTail_IterStart);
            GrAtlasTextBlob* lruBlob = iter.get();
            SkASSERT(lruBlob);
            while (fPool.size() > fBudget && (lruBlob = iter.get()) && lruBlob != blob) {
                fCache.remove(lruBlob->fKey);

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
