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
    SK_DECLARE_INST_COUNT(SkPurgeableImageCache)

    static SkImageCache* Create();

    virtual void* allocAndPinCache(size_t bytes, ID*) SK_OVERRIDE;
    virtual void* pinCache(ID, SkImageCache::DataStatus*) SK_OVERRIDE;
    virtual void releaseCache(ID) SK_OVERRIDE;
    virtual void throwAwayCache(ID) SK_OVERRIDE;

#ifdef SK_DEBUG
    virtual MemoryStatus getMemoryStatus(ID) const SK_OVERRIDE;
    virtual void purgeAllUnpinnedCaches() SK_OVERRIDE;
    virtual ~SkPurgeableImageCache();
#endif

private:
    SkPurgeableImageCache();

#ifdef SK_DEBUG
    SkTDArray<ID> fRecs;
    int findRec(ID) const;
#endif
    void removeRec(ID);
};
#endif // SkPurgeableImageCache_DEFINED
