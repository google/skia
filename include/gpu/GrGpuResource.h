/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGpuResource_DEFINED
#define GrGpuResource_DEFINED

#include "GrResourceKey.h"
#include "GrTypesPriv.h"
#include "SkData.h"
#include "SkInstCnt.h"
#include "SkTInternalLList.h"

class GrContext;
class GrGpu;
class GrResourceCache2;

/**
 * Base class for GrGpuResource. Handles the various types of refs we need. Separated out as a base
 * class to isolate the ref-cnting behavior and provide friendship without exposing all of
 * GrGpuResource.
 *
 * Gpu resources can have three types of refs:
 *   1) Normal ref (+ by ref(), - by unref()): These are used by code that is issuing draw calls
 *      that read and write the resource via GrDrawTarget and by any object that must own a
 *      GrGpuResource and is itself owned (directly or indirectly) by Skia-client code.
 *   2) Pending read (+ by addPendingRead(), - by completedRead()): GrContext has scheduled a read
 *      of the resource by the GPU as a result of a skia API call but hasn't executed it yet.
 *   3) Pending write (+ by addPendingWrite(), - by completedWrite()): GrContext has scheduled a
 *      write to the resource by the GPU as a result of a skia API call but hasn't executed it yet.
 *
 * The latter two ref types are private and intended only for Gr core code.
 *
 * When an item is purgable DERIVED:notifyIsPurgable() will be called (static poly morphism using
 * CRTP). GrIORef and GrGpuResource are separate classes for organizational reasons and to be
 * able to give access via friendship to only the functions related to pending IO operations.
 */
template <typename DERIVED> class GrIORef : public SkNoncopyable {
public:
    SK_DECLARE_INST_COUNT(GrIORef)

    // Some of the signatures are written to mirror SkRefCnt so that GrGpuResource can work with
    // templated helper classes (e.g. SkAutoTUnref). However, we have different categories of
    // refs (e.g. pending reads). We also don't require thread safety as GrCacheable objects are
    // not intended to cross thread boundaries.
    void ref() const {
        this->validate();
        ++fRefCnt;
    }

    void unref() const {
        this->validate();
        --fRefCnt;
        this->didUnref();
    }

    void validate() const {
#ifdef SK_DEBUG
        SkASSERT(fRefCnt >= 0);
        SkASSERT(fPendingReads >= 0);
        SkASSERT(fPendingWrites >= 0);
        SkASSERT(fRefCnt + fPendingReads + fPendingWrites >= 0);
#endif
    }

protected:
    GrIORef() : fRefCnt(1), fPendingReads(0), fPendingWrites(0) { }

    bool isPurgable() const { return !this->internalHasRef() && !this->internalHasPendingIO(); }

    bool internalHasPendingRead() const { return SkToBool(fPendingReads); }
    bool internalHasPendingWrite() const { return SkToBool(fPendingWrites); }
    bool internalHasPendingIO() const { return SkToBool(fPendingWrites | fPendingReads); }

    bool internalHasRef() const { return SkToBool(fRefCnt); }

private:
    void addPendingRead() const {
        this->validate();
        ++fPendingReads;
    }

    void completedRead() const {
        this->validate();
        --fPendingReads;
        this->didUnref();
    }

    void addPendingWrite() const {
        this->validate();
        ++fPendingWrites;
    }

    void completedWrite() const {
        this->validate();
        --fPendingWrites;
        this->didUnref();
    }

private:
    void didUnref() const {
        if (0 == fPendingReads && 0 == fPendingWrites && 0 == fRefCnt) {
            static_cast<const DERIVED*>(this)->notifyIsPurgable();
        }
    }

    mutable int32_t fRefCnt;
    mutable int32_t fPendingReads;
    mutable int32_t fPendingWrites;

    // This class is used to manage conversion of refs to pending reads/writes.
    friend class GrGpuResourceRef;
    friend class GrResourceCache2; // to check IO ref counts.

    template <typename, GrIOType> friend class GrPendingIOResource;
};

/**
 * Base class for objects that can be kept in the GrResourceCache2.
 */
class SK_API GrGpuResource : public GrIORef<GrGpuResource> {
public:
    SK_DECLARE_INST_COUNT(GrGpuResource)

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
    size_t gpuMemorySize() const {
        if (kInvalidGpuMemorySize == fGpuMemorySize) {
            fGpuMemorySize = this->onGpuMemorySize();
            SkASSERT(kInvalidGpuMemorySize != fGpuMemorySize);
        }
        return fGpuMemorySize;
    }

    /**
     * Gets an id that is unique for this GrGpuResource object. It is static in that it does
     * not change when the content of the GrGpuResource object changes. This will never return
     * 0.
     */
    uint32_t getUniqueID() const { return fUniqueID; }

    /**
     * Attach a custom data object to this resource. The data will remain attached
     * for the lifetime of this resource (until it is abandoned or released).
     * Takes a ref on data. Previously attached data, if any, is unrefed.
     * Returns the data argument, for convenience.
     */
    const SkData* setCustomData(const SkData* data);

    /**
     * Returns the custom data object that was attached to this resource by
     * calling setCustomData.
     */
    const SkData* getCustomData() const { return fData.get(); }

    /**
     * Internal-only helper class used for cache manipulations of the reosurce.
     */
    class CacheAccess;
    inline CacheAccess cacheAccess();
    inline const CacheAccess cacheAccess() const;

    /**
     * Removes references to objects in the underlying 3D API without freeing them.
     * Called by CacheAccess.
     * In general this method should not be called outside of skia. It was
     * made by public for a special case where it needs to be called in Blink
     * when a texture becomes unsafe to use after having been shared through
     * a texture mailbox.
     */
    void abandon();

protected:
    // This must be called by every GrGpuObject. It should be called once the object is fully
    // initialized (i.e. not in a base class constructor).
    void registerWithCache();

    GrGpuResource(GrGpu*, bool isWrapped);
    virtual ~GrGpuResource();

    GrGpu* getGpu() const { return fGpu; }

    /** Overridden to free GPU resources in the backend API. */
    virtual void onRelease() { }
    /** Overridden to abandon any internal handles, ptrs, etc to backend API resources.
        This may be called when the underlying 3D context is no longer valid and so no
        backend API calls should be made. */
    virtual void onAbandon() { }

    bool isWrapped() const { return SkToBool(kWrapped_Flag & fFlags); }

    /**
     * This entry point should be called whenever gpuMemorySize() should report a different size.
     * The cache will call gpuMemorySize() to update the current size of the resource.
     */
    void didChangeGpuMemorySize() const;

    /**
     * Optionally called by the GrGpuResource subclass if the resource can be used as scratch.
     * By default resources are not usable as scratch. This should only be called once.
     **/
    void setScratchKey(const GrScratchKey& scratchKey);

private:
    /**
     * Frees the object in the underlying 3D API. Called by CacheAccess.
     */
    void release();

    virtual size_t onGpuMemorySize() const = 0;

    // See comments in CacheAccess.
    bool setContentKey(const GrResourceKey& contentKey);
    void setBudgeted(bool countsAgainstBudget);
    void notifyIsPurgable() const;
    void removeScratchKey();

#ifdef SK_DEBUG
    friend class GrGpu; // for assert in GrGpu to access getGpu
#endif

    static uint32_t CreateUniqueID();

    // We're in an internal doubly linked list owned by GrResourceCache2
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrGpuResource);


    static const size_t kInvalidGpuMemorySize = ~static_cast<size_t>(0);
    enum Flags {
        /**
         * The resource counts against the resource cache's budget.
         */
        kBudgeted_Flag      = 0x1,

        /**
         * This object wraps a GPU object given to us by Skia's client. Skia will not free the
         * underlying backend API GPU resources when the GrGpuResource is destroyed. This also
         * implies that kBudgeted_Flag is not set.
         */
        kWrapped_Flag       = 0x2,

        /**
         * If set then fContentKey is valid and the resource is cached based on its content.
         */
        kContentKeySet_Flag = 0x4,
    };

    GrScratchKey            fScratchKey;
    // TODO(bsalomon): Remove GrResourceKey and use different simpler type for content keys.
    GrResourceKey           fContentKey;

    // This is not ref'ed but abandon() or release() will be called before the GrGpu object
    // is destroyed. Those calls set will this to NULL.
    GrGpu*                  fGpu;
    mutable size_t          fGpuMemorySize;

    uint32_t                fFlags;
    const uint32_t          fUniqueID;

    SkAutoTUnref<const SkData> fData;

    typedef GrIORef<GrGpuResource> INHERITED;
    friend class GrIORef<GrGpuResource>; // to access notifyIsPurgable.
};

#endif
