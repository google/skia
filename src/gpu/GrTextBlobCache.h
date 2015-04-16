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
    typedef GrAtlasTextContext::BitmapTextBlob BitmapTextBlob;

    /**
     * The callback function used by the cache when it is still over budget after a purge. The
     * passed in 'data' is the same 'data' handed to setOverbudgetCallback.
     */
    typedef void (*PFOverBudgetCB)(void* data);

    GrTextBlobCache(PFOverBudgetCB cb, void* data)
        : fPool(kPreAllocSize, kMinGrowthSize)
        , fCallback(cb)
        , fData(data) {
        SkASSERT(cb && data);
    }
    ~GrTextBlobCache();

    // creates an uncached blob
    BitmapTextBlob* createBlob(int glyphCount, int runCount, size_t maxVASize);
    BitmapTextBlob* createBlob(const SkTextBlob* blob, size_t maxVAStride) {
        int glyphCount = 0;
        int runCount = 0;
        BlobGlyphCount(&glyphCount, &runCount, blob);
        BitmapTextBlob* cacheBlob = this->createBlob(glyphCount, runCount, maxVAStride);
        return cacheBlob;
    }

    BitmapTextBlob* createCachedBlob(const SkTextBlob* blob,
                                     const BitmapTextBlob::Key& key,
                                     const SkMaskFilter::BlurRec& blurRec,
                                     const SkPaint& paint,
                                     size_t maxVAStride) {
        int glyphCount = 0;
        int runCount = 0;
        BlobGlyphCount(&glyphCount, &runCount, blob);
        BitmapTextBlob* cacheBlob = this->createBlob(glyphCount, runCount, maxVAStride);
        cacheBlob->fKey = key;
        if (key.fHasBlur) {
            cacheBlob->fBlurRec = blurRec;
        }
        if (key.fStyle != SkPaint::kFill_Style) {
            cacheBlob->fStrokeInfo.fFrameWidth = paint.getStrokeWidth();
            cacheBlob->fStrokeInfo.fMiterLimit = paint.getStrokeMiter();
            cacheBlob->fStrokeInfo.fJoin = paint.getStrokeJoin();
        }
        this->add(cacheBlob);
        return cacheBlob;
    }

    BitmapTextBlob* find(const BitmapTextBlob::Key& key) {
        return fCache.find(key);
    }

    void remove(BitmapTextBlob* blob) {
        fCache.remove(blob->fKey);
        fBlobList.remove(blob);
        blob->unref();
    }

    void add(BitmapTextBlob* blob) {
        fCache.add(blob);
        fBlobList.addToHead(blob);

        // If we are overbudget, then unref until we are below budget again
        if (fPool.size() > kBudget) {
            BitmapBlobList::Iter iter;
            iter.init(fBlobList, BitmapBlobList::Iter::kTail_IterStart);
            BitmapTextBlob* lruBlob = iter.get();
            SkASSERT(lruBlob);
            while (fPool.size() > kBudget && (lruBlob = iter.get()) && lruBlob != blob) {
                fCache.remove(lruBlob->fKey);

                // Backup the iterator before removing and unrefing the blob
                iter.prev();
                fBlobList.remove(lruBlob);
                lruBlob->unref();
            }

            // If we break out of the loop with lruBlob == blob, then we haven't purged enough
            // use the call back and try to free some more.  If we are still overbudget after this,
            // then this single textblob is over our budget
            if (lruBlob == blob) {
                (*fCallback)(fData);
            }

#ifdef SK_DEBUG
            if (fPool.size() > kBudget) {
                SkDebugf("Single textblob is larger than our whole budget");
            }
#endif
        }
    }

    void makeMRU(BitmapTextBlob* blob) {
        if (fBlobList.head() == blob) {
            return;
        }

        fBlobList.remove(blob);
        fBlobList.addToHead(blob);
    }

    void freeAll();

private:
    // TODO move to SkTextBlob
    void BlobGlyphCount(int* glyphCount, int* runCount, const SkTextBlob* blob) {
        SkTextBlob::RunIterator itCounter(blob);
        for (; !itCounter.done(); itCounter.next(), (*runCount)++) {
            *glyphCount += itCounter.glyphCount();
        }
    }

    typedef SkTInternalLList<BitmapTextBlob> BitmapBlobList;

    // Budget was chosen to be ~4 megabytes.  The min alloc and pre alloc sizes in the pool are
    // based off of the largest cached textblob I have seen in the skps(a couple of kilobytes).
    static const int kPreAllocSize = 1 << 17;
    static const int kMinGrowthSize = 1 << 17;
    static const int kBudget = 1 << 22;
    BitmapBlobList fBlobList;
    SkTDynamicHash<BitmapTextBlob, BitmapTextBlob::Key> fCache;
    GrMemoryPool fPool;
    PFOverBudgetCB fCallback;
    void* fData;
};

#endif
