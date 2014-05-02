/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCacheable_DEFINED
#define GrCacheable_DEFINED

#include "SkRefCnt.h"

class GrResourceCacheEntry;

/**
 * Base class for objects that can be kept in the GrResourceCache.
 */
class GrCacheable : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrCacheable)

    /**
     * Retrieves the amount of GPU memory used by this resource in bytes. It is
     * approximate since we aren't aware of additional padding or copies made
     * by the driver.
     *
     * @return the amount of GPU memory used in bytes
     */
    virtual size_t gpuMemorySize() const = 0;

    /**
     * Checks whether the GPU memory allocated to this resource is still in effect.
     * It can become invalid if its context is destroyed or lost, in which case it
     * should no longer count against the GrResourceCache budget.
     *
     * @return true if this resource is still holding GPU memory
     *         false otherwise.
     */
    virtual bool isValidOnGpu() const = 0;

    void setCacheEntry(GrResourceCacheEntry* cacheEntry) { fCacheEntry = cacheEntry; }
    GrResourceCacheEntry* getCacheEntry() { return fCacheEntry; }

protected:
    GrCacheable() : fCacheEntry(NULL) {}

    bool isInCache() const { return NULL != fCacheEntry; }

private:
    GrResourceCacheEntry* fCacheEntry;  // NULL if not in cache

    typedef SkRefCnt INHERITED;
};

#endif
