/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuResourcePriv_DEFINED
#define GrGpuResourcePriv_DEFINED

#include "src/gpu/GrGpuResource.h"

/**
 * This class allows code internal to Skia privileged access to manage the cache keys and budget
 * status of a GrGpuResource object.
 */
class GrGpuResource::ResourcePriv {
public:
    /**
     * Sets a unique key for the resource. If the resource was previously cached as scratch it will
     * be converted to a uniquely-keyed resource. If the key is invalid then this is equivalent to
     * removeUniqueKey(). If another resource is using the key then its unique key is removed and
     * this resource takes over the key.
     */
    void setUniqueKey(const GrUniqueKey& key) { fResource->setUniqueKey(key); }

    /** Removes the unique key from a resource. If the resource has a scratch key, it may be
        preserved for recycling as scratch. */
    void removeUniqueKey() { fResource->removeUniqueKey(); }

    /**
     * If the resource is uncached make it cached. Has no effect on resources that are wrapped or
     * already cached.
     */
    void makeBudgeted() { fResource->makeBudgeted(); }

    /**
     * If the resource is cached make it uncached. Has no effect on resources that are wrapped or
     * already uncached. Furthermore, resources with unique keys cannot be made unbudgeted.
     */
    void makeUnbudgeted() { fResource->makeUnbudgeted(); }

    /**
     * Get the resource's budgeted-type which indicates whether it counts against the resource cache
     * budget and if not whether it is allowed to be cached.
     */
    GrBudgetedType budgetedType() const {
        SkASSERT(GrBudgetedType::kBudgeted == fResource->fBudgetedType ||
                 !fResource->getUniqueKey().isValid() || fResource->fRefsWrappedObjects);
        return fResource->fBudgetedType;
    }

    /**
     * Is the resource object wrapping an externally allocated GPU resource?
     */
    bool refsWrappedObjects() const { return fResource->fRefsWrappedObjects; }

    /**
     * If this resource can be used as a scratch resource this returns a valid scratch key.
     * Otherwise it returns a key for which isNullScratch is true. The resource may currently be
     * used as a uniquely keyed resource rather than scratch. Check isScratch().
     */
    const GrScratchKey& getScratchKey() const { return fResource->fScratchKey; }

    /**
     * If the resource has a scratch key, the key will be removed. Since scratch keys are installed
     * at resource creation time, this means the resource will never again be used as scratch.
     */
    void removeScratchKey() const { fResource->removeScratchKey();  }

    bool isPurgeable() const { return fResource->isPurgeable(); }

    bool hasRef() const { return fResource->hasRef(); }

protected:
    ResourcePriv(GrGpuResource* resource) : fResource(resource) {   }
    ResourcePriv(const ResourcePriv& that) : fResource(that.fResource) {}
    ResourcePriv& operator=(const CacheAccess&); // unimpl

    // No taking addresses of this type.
    const ResourcePriv* operator&() const;
    ResourcePriv* operator&();

    GrGpuResource* fResource;

    friend class GrGpuResource; // to construct/copy this type.
};

inline GrGpuResource::ResourcePriv GrGpuResource::resourcePriv() { return ResourcePriv(this); }

inline const GrGpuResource::ResourcePriv GrGpuResource::resourcePriv() const {
    return ResourcePriv(const_cast<GrGpuResource*>(this));
}

#endif
