/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGlyphCache_Globals_DEFINED
#define SkGlyphCache_Globals_DEFINED

#include "SkGlyphCache.h"
#include "SkTLS.h"

#ifndef SK_DEFAULT_FONT_CACHE_COUNT_LIMIT
    #define SK_DEFAULT_FONT_CACHE_COUNT_LIMIT   2048
#endif

#ifndef SK_DEFAULT_FONT_CACHE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

///////////////////////////////////////////////////////////////////////////////

class SkMutex;

class SkGlyphCache_Globals {
public:
    enum UseMutex {
        kNo_UseMutex,  // thread-local cache
        kYes_UseMutex  // shared cache
    };

    SkGlyphCache_Globals(UseMutex um) {
        fHead = NULL;
        fTotalMemoryUsed = 0;
        fCacheSizeLimit = SK_DEFAULT_FONT_CACHE_LIMIT;
        fCacheCount = 0;
        fCacheCountLimit = SK_DEFAULT_FONT_CACHE_COUNT_LIMIT;

        fMutex = (kYes_UseMutex == um) ? SkNEW(SkMutex) : NULL;
    }

    ~SkGlyphCache_Globals() {
        SkGlyphCache* cache = fHead;
        while (cache) {
            SkGlyphCache* next = cache->fNext;
            SkDELETE(cache);
            cache = next;
        }

        SkDELETE(fMutex);
    }

    SkMutex*        fMutex;

    SkGlyphCache* internalGetHead() const { return fHead; }
    SkGlyphCache* internalGetTail() const;

    size_t getTotalMemoryUsed() const { return fTotalMemoryUsed; }
    int getCacheCountUsed() const { return fCacheCount; }

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    int getCacheCountLimit() const { return fCacheCountLimit; }
    int setCacheCountLimit(int limit);

    size_t  getCacheSizeLimit() const { return fCacheSizeLimit; }
    size_t  setCacheSizeLimit(size_t limit);

    // returns true if this cache is over-budget either due to size limit
    // or count limit.
    bool isOverBudget() const {
        return fCacheCount > fCacheCountLimit ||
               fTotalMemoryUsed > fCacheSizeLimit;
    }

    void purgeAll(); // does not change budget

    // call when a glyphcache is available for caching (i.e. not in use)
    void attachCacheToHead(SkGlyphCache*);

    // can only be called when the mutex is already held
    void internalDetachCache(SkGlyphCache*);
    void internalAttachCacheToHead(SkGlyphCache*);

    // can return NULL
    static SkGlyphCache_Globals* FindTLS() {
        return (SkGlyphCache_Globals*)SkTLS::Find(CreateTLS);
    }

    static SkGlyphCache_Globals& GetTLS() {
        return *(SkGlyphCache_Globals*)SkTLS::Get(CreateTLS, DeleteTLS);
    }

    static void DeleteTLS() { SkTLS::Delete(CreateTLS); }

private:
    SkGlyphCache* fHead;
    size_t  fTotalMemoryUsed;
    size_t  fCacheSizeLimit;
    int32_t fCacheCountLimit;
    int32_t fCacheCount;

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0);

    static void* CreateTLS() {
        return SkNEW_ARGS(SkGlyphCache_Globals, (kNo_UseMutex));
    }

    static void DeleteTLS(void* ptr) {
        SkDELETE((SkGlyphCache_Globals*)ptr);
    }
};

#endif
