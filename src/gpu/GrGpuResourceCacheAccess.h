
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuResourceCacheAccess_DEFINED
#define GrGpuResourceCacheAccess_DEFINED

#include "GrGpuResource.h"

/**
 * This class allows code internal to Skia privileged access to manage the cache keys of a
 * GrGpuResource object.
 */
class GrGpuResource::CacheAccess {
public:
    /**
     * Sets a content key for the resource. If the resource was previously cached as scratch it will
     * be converted to a content resource. Currently this may only be called once per resource. It
     * fails if there is already a resource with the same content key. TODO: make this supplant the
     * resource that currently is using the content key, allow resources' content keys to change,
     * and allow removal of a content key to convert a resource back to scratch.
     */
    bool setContentKey(const GrResourceKey& contentKey) {
        return fResource->setContentKey(contentKey);
    }

    /**
     * Used by legacy cache to attach a cache entry. This is to be removed soon.
     */
    void setCacheEntry(GrResourceCacheEntry* cacheEntry) {
        // GrResourceCache never changes the cacheEntry once one has been added.
        SkASSERT(NULL == cacheEntry || NULL == fResource->fCacheEntry);
        fResource->fCacheEntry = cacheEntry;
    }

    /**
     * Is the resource in the legacy cache? This is to be removed soon.
     */
    bool isInCache() const { return SkToBool(fResource->fCacheEntry); }

    /**
     * Returns the cache entry for the legacy cache. This is to be removed soon.
     */
    GrResourceCacheEntry* getCacheEntry() const { return fResource->fCacheEntry; }

    /**
     * Is the resource currently cached as scratch? This means it has a valid scratch key and does
     * not have a content key.
     */
    bool isScratch() const {
        SkASSERT(fResource->fScratchKey.isScratch());
        return NULL == this->getContentKey() && !fResource->fScratchKey.isNullScratch();
    }

    /** 
     * If this resource can be used as a scratch resource this returns a valid scratch key.
     * Otherwise it returns a key for which isNullScratch is true. The resource may currently be
     * used as content resource rather than scratch. Check isScratch().
     */
    const GrResourceKey& getScratchKey() const { return fResource->fScratchKey; }

    /**
     * If the resource is currently cached by a content key, the key is returned, otherwise NULL.
     */
    const GrResourceKey* getContentKey() const {
        if (fResource->fContentKeySet) {
            return &fResource->fContentKey;
        }
        return NULL;
    }

private:
    CacheAccess(GrGpuResource* resource) : fResource(resource) { }
    CacheAccess(const CacheAccess& that) : fResource(that.fResource) { }
    CacheAccess& operator=(const CacheAccess&); // unimpl

    // No taking addresses of this type.
    const CacheAccess* operator&() const;
    CacheAccess* operator&();

    GrGpuResource* fResource;

    friend class GrGpuResource; // to construct/copy this type.
};

inline GrGpuResource::CacheAccess GrGpuResource::cacheAccess() { return CacheAccess(this); }

inline const GrGpuResource::CacheAccess GrGpuResource::cacheAccess() const {
    return CacheAccess(const_cast<GrGpuResource*>(this));
}

#endif
