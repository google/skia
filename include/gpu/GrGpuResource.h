/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuResource_DEFINED
#define GrGpuResource_DEFINED

#include "SkInstCnt.h"
#include "SkTInternalLList.h"

class GrResourceCacheEntry;
class GrGpu;
class GrContext;

/**
 * Base class for objects that can be kept in the GrResourceCache.
 */
class GrGpuResource : public SkNoncopyable {
public:
    SK_DECLARE_INST_COUNT_ROOT(GrGpuResource)

    // These method signatures are written to mirror SkRefCnt. However, we don't require
    // thread safety as GrCacheable objects are not intended to cross thread boundaries.
    // internal_dispose() exists because of GrTexture's reliance on it. It will be removed
    // soon.
    void ref() const { ++fRefCnt; }
    void unref() const { --fRefCnt; if (0 == fRefCnt) { this->internal_dispose(); } }
    virtual void internal_dispose() const { SkDELETE(this); }
    bool unique() const { return 1 == fRefCnt; }
#ifdef SK_DEBUG
    void validate() const {
        SkASSERT(fRefCnt > 0);
    }
#endif

    /**
     * Frees the object in the underlying 3D API. It must be safe to call this
     * when the object has been previously abandoned.
     */
    void release();

    /**
     * Removes references to objects in the underlying 3D API without freeing
     * them. Used when the API context has been torn down before the GrContext.
     */
    void abandon();

    /**
     * Tests whether a object has been abandoned or released. All objects will
     * be in this state after their creating GrContext is destroyed or has
     * contextLost called. It's up to the client to test wasDestroyed() before
     * attempting to use an object if it holds refs on objects across
     * ~GrContext, freeResources with the force flag, or contextLost.
     *
     * @return true if the object has been released or abandoned,
     *         false otherwise.
     */
    bool wasDestroyed() const { return NULL == fGpu; }

    /**
     * Retrieves the context that owns the object. Note that it is possible for
     * this to return NULL. When objects have been release()ed or abandon()ed
     * they no longer have an owning context. Destroying a GrContext
     * automatically releases all its resources.
     */
    const GrContext* getContext() const;
    GrContext* getContext();

    /**
     * Retrieves the amount of GPU memory used by this resource in bytes. It is
     * approximate since we aren't aware of additional padding or copies made
     * by the driver.
     *
     * @return the amount of GPU memory used in bytes
     */
    virtual size_t gpuMemorySize() const = 0;

    void setCacheEntry(GrResourceCacheEntry* cacheEntry) { fCacheEntry = cacheEntry; }
    GrResourceCacheEntry* getCacheEntry() { return fCacheEntry; }

    /**
     * Gets an id that is unique for this GrCacheable object. It is static in that it does
     * not change when the content of the GrCacheable object changes. This will never return
     * 0.
     */
    uint32_t getUniqueID() const { return fUniqueID; }

protected:
    GrGpuResource(GrGpu*, bool isWrapped);
    virtual ~GrGpuResource();

    bool isInCache() const { return NULL != fCacheEntry; }

    GrGpu* getGpu() const { return fGpu; }

    // Derived classes should always call their parent class' onRelease
    // and onAbandon methods in their overrides.
    virtual void onRelease() {};
    virtual void onAbandon() {};

    bool isWrapped() const { return kWrapped_FlagBit & fFlags; }

    /**
     * This entry point should be called whenever gpuMemorySize() begins
     * reporting a different size. If the object is in the cache, it will call
     * gpuMemorySize() immediately and pass the new size on to the resource
     * cache.
     */
    void didChangeGpuMemorySize() const;

private:
#ifdef SK_DEBUG
    friend class GrGpu; // for assert in GrGpu to access getGpu
#endif

    static uint32_t CreateUniqueID();

    // We're in an internal doubly linked list
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrGpuResource);

    GrGpu*              fGpu;               // not reffed. The GrGpu can be deleted while there
                                            // are still live GrGpuResources. It will call
                                            // release() on all such objects in its destructor.
    enum Flags {
        /**
         * This object wraps a GPU object given to us by the user.
         * Lifetime management is left up to the user (i.e., we will not
         * free it).
         */
        kWrapped_FlagBit         = 0x1,
    };

    uint32_t                fFlags;

    mutable int32_t         fRefCnt;
    GrResourceCacheEntry*   fCacheEntry;  // NULL if not in cache
    const uint32_t          fUniqueID;

    typedef SkNoncopyable INHERITED;
};

#endif
