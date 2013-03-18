/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPurgeableImageCache_DEFINED
#define SkPurgeableImageCache_DEFINED

#include "SkImageCache.h"

#ifdef SK_DEBUG
    #include "SkTDArray.h"
#endif

/**
 *  Implementation for SkImageCache that uses system defined purgeable memory.
 */
class SkPurgeableImageCache : public SkImageCache {

public:
    static SkImageCache* Create();

    virtual void* allocAndPinCache(size_t bytes, intptr_t* ID) SK_OVERRIDE;
    virtual void* pinCache(intptr_t ID, SkImageCache::DataStatus*) SK_OVERRIDE;
    virtual void releaseCache(intptr_t ID) SK_OVERRIDE;
    virtual void throwAwayCache(intptr_t ID) SK_OVERRIDE;

#ifdef SK_DEBUG
    virtual MemoryStatus getMemoryStatus(intptr_t ID) const SK_OVERRIDE;
    virtual void purgeAllUnpinnedCaches() SK_OVERRIDE;
    virtual ~SkPurgeableImageCache();
#endif

private:
    SkPurgeableImageCache();

#ifdef SK_DEBUG
    SkTDArray<intptr_t> fRecs;
    int findRec(intptr_t) const;
#endif
    void removeRec(intptr_t);
};
#endif // SkPurgeableImageCache_DEFINED
