
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrResource_DEFINED
#define GrResource_DEFINED

#include "GrRefCnt.h"

#include "SkTDLinkedList.h"

class GrGpu;
class GrContext;
class GrResourceEntry;

/**
 * Base class for the GPU resources created by a GrContext.
 */
class GrResource : public GrRefCnt {
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

protected:
    explicit GrResource(GrGpu* gpu);
    virtual ~GrResource();

    GrGpu* getGpu() const { return fGpu; }

    // Derived classes should always call their parent class' onRelease
    // and onAbandon methods in their overrides.
    virtual void onRelease() {};
    virtual void onAbandon() {};

    bool isInCache() const { return NULL != fCacheEntry; }

private:

#if GR_DEBUG
    friend class GrGpu; // for assert in GrGpu to access getGpu
#endif

    GrGpu*      fGpu;       // not reffed. The GrGpu can be deleted while there
                            // are still live GrResources. It will call
                            // release() on all such resources in its
                            // destructor.

    // we're a dlinklist
    SK_DEFINE_DLINKEDLIST_INTERFACE(GrResource);

    GrResourceEntry* fCacheEntry;  // NULL if not in cache

    typedef GrRefCnt INHERITED;
};

#endif
