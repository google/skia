
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
    bool setContentKey(const GrContentKey& contentKey) {
        return fResource->setContentKey(contentKey);
    }

    /**
     * Is the resource currently cached as scratch? This means it is cached, has a valid scratch
     * key, and does not have a content key.
     */
    bool isScratch() const {
        return !this->getContentKey().isValid() && fResource->fScratchKey.isValid() &&
                this->isBudgeted();
    }

    /** 
     * If this resource can be used as a scratch resource this returns a valid scratch key.
     * Otherwise it returns a key for which isNullScratch is true. The resource may currently be
     * used as a content resource rather than scratch. Check isScratch().
     */
    const GrScratchKey& getScratchKey() const { return fResource->fScratchKey; }

    /**
     * If the resource has a scratch key, the key will be removed. Since scratch keys are installed
     * at resource creation time, this means the resource will never again be used as scratch.
     */
    void removeScratchKey() const { fResource->removeScratchKey();  }

    /**
     * If the resource is currently cached by a content key, the key is returned, otherwise NULL.
     */
    const GrContentKey& getContentKey() const { return fResource->fContentKey; }

    /**
     * Is the resource object wrapping an externally allocated GPU resource?
     */
    bool isWrapped() const { return GrGpuResource::kWrapped_LifeCycle == fResource->fLifeCycle; }

    /**
     * Does the resource count against the resource budget?
     */
    bool isBudgeted() const {
        bool ret = GrGpuResource::kCached_LifeCycle == fResource->fLifeCycle;
        SkASSERT(ret || !this->getContentKey().isValid());
        return ret;
    }

    /**
     * If the resource is uncached make it cached. Has no effect on resources that are wrapped or
     * already cached.
     */
    void makeBudgeted() { fResource->makeBudgeted(); }

    /**
     * If the resource is cached make it uncached. Has no effect on resources that are wrapped or
     * already uncached. Furthermore, resources with content keys cannot be made unbudgeted.
     */
    void makeUnbudgeted() { fResource->makeUnbudgeted(); }

    /**
     * Called by the cache to delete the resource under normal circumstances.
     */
    void release() {
        fResource->release();
        if (fResource->isPurgable()) {            
            SkDELETE(fResource);
        }
    }

    /**
     * Called by the cache to delete the resource when the backend 3D context is no longer valid.
     */
    void abandon() {
        fResource->abandon();
        if (fResource->isPurgable()) {            
            SkDELETE(fResource);
        }
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
