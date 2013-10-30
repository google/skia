/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrResource_DEFINED
#define GrResource_DEFINED

#include "SkRefCnt.h"
#include "SkTInternalLList.h"

class GrGpu;
class GrContext;
class GrResourceEntry;

/**
 * Base class for the GPU resources created by a GrContext.
 */
class GrResource : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrResource)

    /**
     * Frees the resource in the underlying 3D API. It must be safe to call this
     * when the resource has been previously abandoned.
     */
    void release();

    /**
     * Removes references to objects in the underlying 3D API without freeing
     * them. Used when the API context has been torn down before the GrContext.
     */
    void abandon();

    /**
     * Tests whether a resource has been abandoned or released. All resources
     * will be in this state after their creating GrContext is destroyed or has
     * contextLost called. It's up to the client to test isValid() before
     * attempting to use a resource if it holds refs on resources across
     * ~GrContext, freeResources with the force flag, or contextLost.
     *
     * @return true if the resource has been released or abandoned,
     *         false otherwise.
     */
    bool isValid() const { return NULL != fGpu; }

    /**
     * Retrieves the size of the object in GPU memory. This is approximate since
     * we aren't aware of additional padding or copies made by the driver.
     *
     * @return the size of the buffer in bytes
     */
    virtual size_t sizeInBytes() const = 0;

    /**
     * Retrieves the context that owns the resource. Note that it is possible
     * for this to return NULL. When resources have been release()ed or
     * abandon()ed they no longer have an owning context. Destroying a
     * GrContext automatically releases all its resources.
     */
    const GrContext* getContext() const;
    GrContext* getContext();

    void setCacheEntry(GrResourceEntry* cacheEntry) { fCacheEntry = cacheEntry; }
    GrResourceEntry* getCacheEntry() { return fCacheEntry; }

    void incDeferredRefCount() const {
        SkASSERT(fDeferredRefCount >= 0);
        ++fDeferredRefCount;
    }

    void decDeferredRefCount() const {
        SkASSERT(fDeferredRefCount > 0);
        --fDeferredRefCount;
        if (0 == fDeferredRefCount && this->needsDeferredUnref()) {
            SkASSERT(this->getRefCnt() > 1);
            this->unref();
        }
    }

    int getDeferredRefCount() const { return fDeferredRefCount; }

    void setNeedsDeferredUnref() { fFlags |= kDeferredUnref_FlagBit; }

protected:
    /**
     * isWrapped indicates we have wrapped a client-created backend resource in a GrResource. If it
     * is true then the client is responsible for the lifetime of the underlying backend resource.
     * Otherwise, our onRelease() should free the resource.
     */
    GrResource(GrGpu* gpu, bool isWrapped);
    virtual ~GrResource();

    GrGpu* getGpu() const { return fGpu; }

    // Derived classes should always call their parent class' onRelease
    // and onAbandon methods in their overrides.
    virtual void onRelease() {};
    virtual void onAbandon() {};

    bool isInCache() const { return NULL != fCacheEntry; }
    bool isWrapped() const { return kWrapped_FlagBit & fFlags; }
    bool needsDeferredUnref() const { return SkToBool(kDeferredUnref_FlagBit & fFlags); }

private:
#ifdef SK_DEBUG
    friend class GrGpu; // for assert in GrGpu to access getGpu
#endif

    // We're in an internal doubly linked list
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrResource);

    GrGpu*              fGpu;               // not reffed. The GrGpu can be deleted while there
                                            // are still live GrResources. It will call
                                            // release() on all such resources in its
                                            // destructor.
    GrResourceEntry*    fCacheEntry;        // NULL if not in cache
    mutable int         fDeferredRefCount;  // How many references in deferred drawing buffers.

    enum Flags {
        /**
         * This resource wraps a GPU resource given to us by the user.
         * Lifetime management is left up to the user (i.e., we will not
         * free it).
         */
        kWrapped_FlagBit         = 0x1,

        /**
         * This texture should be de-refed when the deferred ref count goes
         * to zero. A resource gets into this state when the resource cache
         * is holding a ref-of-obligation (i.e., someone needs to own it but
         * no one else wants to) but doesn't really want to keep it around.
         */
        kDeferredUnref_FlagBit  = 0x2,
    };
    uint32_t         fFlags;

    typedef SkRefCnt INHERITED;
};

#endif
