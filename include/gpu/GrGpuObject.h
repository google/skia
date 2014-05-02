/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuObject_DEFINED
#define GrGpuObject_DEFINED

#include "GrCacheable.h"
#include "SkTInternalLList.h"

class GrGpu;
class GrContext;

/**
 * Base class for the GPU objects created by a GrContext.
 */
class GrGpuObject : public GrCacheable {
public:
    SK_DECLARE_INST_COUNT(GrGpuObject)

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

    virtual bool isValidOnGpu() const SK_OVERRIDE { return !this->wasDestroyed(); }

protected:
    /**
     * isWrapped indicates we have wrapped a client-created backend object in a GrGpuObject. If it
     * is true then the client is responsible for the lifetime of the underlying backend object.
     * Otherwise, our onRelease() should free the object.
     */
    GrGpuObject(GrGpu* gpu, bool isWrapped);
    virtual ~GrGpuObject();

    GrGpu* getGpu() const { return fGpu; }

    // Derived classes should always call their parent class' onRelease
    // and onAbandon methods in their overrides.
    virtual void onRelease() {};
    virtual void onAbandon() {};

    bool isWrapped() const { return kWrapped_FlagBit & fFlags; }
    bool needsDeferredUnref() const { return SkToBool(kDeferredUnref_FlagBit & fFlags); }

private:
#ifdef SK_DEBUG
    friend class GrGpu; // for assert in GrGpu to access getGpu
#endif

    // We're in an internal doubly linked list
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrGpuObject);

    GrGpu*              fGpu;               // not reffed. The GrGpu can be deleted while there
                                            // are still live GrGpuObjects. It will call
                                            // release() on all such objects in its destructor.
    mutable int         fDeferredRefCount;  // How many references in deferred drawing buffers.

    enum Flags {
        /**
         * This object wraps a GPU object given to us by the user.
         * Lifetime management is left up to the user (i.e., we will not
         * free it).
         */
        kWrapped_FlagBit         = 0x1,

        /**
         * This texture should be de-refed when the deferred ref count goes
         * to zero. An object gets into this state when the resource cache
         * is holding a ref-of-obligation (i.e., someone needs to own it but
         * no one else wants to) but doesn't really want to keep it around.
         */
        kDeferredUnref_FlagBit  = 0x2,
    };
    uint32_t         fFlags;

    typedef GrCacheable INHERITED;
};

#endif
