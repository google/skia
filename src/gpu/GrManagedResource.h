/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrManagedResource_DEFINED
#define GrManagedResource_DEFINED

#include "include/private/GrTypesPriv.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTHash.h"
#include "include/utils/SkRandom.h"
#include <atomic>

class GrTexture;

// uncomment to enable tracing of resource refs
#ifdef SK_DEBUG
#define SK_TRACE_MANAGED_RESOURCES
#endif

/** \class GrManagedResource

  GrManagedResource is the base class for GPU resources that may be shared by
  multiple objects, in particular objects that are tracked by a command buffer.
  When an existing owner wants to share a reference, it calls ref().
  When an owner wants to release its reference, it calls unref(). When the
  shared object's reference count goes to zero as the result of an unref()
  call, its (virtual) destructor is called. It is an error for the
  destructor to be called explicitly (or via the object going out of scope on
  the stack or calling delete) if getRefCnt() > 1.

  This is nearly identical to SkRefCntBase. The exceptions are that unref()
  takes a GrGpu, and any derived classes must implement freeGPUData().
*/

class GrManagedResource : SkNoncopyable {
public:
    // Simple refCount tracing, to ensure that everything ref'ed is unref'ed.
#ifdef SK_TRACE_MANAGED_RESOURCES
    struct Hash {
        uint32_t operator()(const GrManagedResource* const& r) const {
            SkASSERT(r);
            return r->fKey;
        }
    };

    class Trace {
    public:
        ~Trace() {
            fHashSet.foreach([](const GrManagedResource* r) {
                r->dumpInfo();
            });
            SkASSERT(0 == fHashSet.count());
        }

        void add(const GrManagedResource* r) {
            SkAutoMutexExclusive locked(fLock);
            fHashSet.add(r);
        }

        void remove(const GrManagedResource* r) {
            SkAutoMutexExclusive locked(fLock);
            fHashSet.remove(r);
        }

    private:
        SkMutex fLock;
        SkTHashSet<const GrManagedResource*, GrManagedResource::Hash> fHashSet SK_GUARDED_BY(fLock);
    };

    static std::atomic<uint32_t> fKeyCounter;
#endif

    /** Default construct, initializing the reference count to 1.
     */
    GrManagedResource() : fRefCnt(1) {
#ifdef SK_TRACE_MANAGED_RESOURCES
        fKey = fKeyCounter.fetch_add(+1, std::memory_order_relaxed);
        GetTrace()->add(this);
#endif
    }

    /** Destruct, asserting that the reference count is 1.
     */
    virtual ~GrManagedResource() {
#ifdef SK_DEBUG
        auto count = this->getRefCnt();
        SkASSERTF(count == 1, "fRefCnt was %d", count);
        fRefCnt.store(0);    // illegal value, to catch us if we reuse after delete
#endif
    }

#ifdef SK_DEBUG
    /** Return the reference count. Use only for debugging. */
    int32_t getRefCnt() const { return fRefCnt.load(); }
#endif

    /** May return true if the caller is the only owner.
     *  Ensures that all previous owner's actions are complete.
     */
    bool unique() const {
        // The acquire barrier is only really needed if we return true.  It
        // prevents code conditioned on the result of unique() from running
        // until previous owners are all totally done calling unref().
        return 1 == fRefCnt.load(std::memory_order_acquire);
    }

    /** Increment the reference count.
        Must be balanced by a call to unref() or unrefAndFreeResources().
     */
    void ref() const {
        // No barrier required.
        SkDEBUGCODE(int newRefCount = )fRefCnt.fetch_add(+1, std::memory_order_relaxed);
        SkASSERT(newRefCount >= 1);
    }

    /** Decrement the reference count. If the reference count is 1 before the
        decrement, then delete the object. Note that if this is the case, then
        the object needs to have been allocated via new, and not on the stack.
        Any GPU data associated with this resource will be freed before it's deleted.
     */
    void unref() const {
        // A release here acts in place of all releases we "should" have been doing in ref().
        int newRefCount = fRefCnt.fetch_add(-1, std::memory_order_acq_rel);
        SkASSERT(newRefCount >= 0);
        if (newRefCount == 1) {
            // Like unique(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
            this->internal_dispose();
        }
    }

#ifdef SK_DEBUG
    // This is used for validating in the vulkan backend when using a main command buffer and temp
    // command buffer at the same time. We need to validate that no images in the temp command
    // buffer have been used in the main command buffer.
    virtual const GrManagedResource* asVkImageResource() const { return nullptr; }
#endif

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT(this->getRefCnt() > 0);
    }
#endif

#ifdef SK_TRACE_MANAGED_RESOURCES
    /** Output a human-readable dump of this resource's information
     */
    virtual void dumpInfo() const = 0;
#endif

private:
#ifdef SK_TRACE_MANAGED_RESOURCES
    static Trace* GetTrace() {
        static Trace kTrace;
        return &kTrace;
    }
#endif

    /** Must be implemented by any subclasses.
     *  Deletes any GPU data associated with this resource
     */
    virtual void freeGPUData() const = 0;

    /**
     *  Called when the ref count goes to 0. Will free GPU resources.
     */
    void internal_dispose() const {
        this->freeGPUData();
#ifdef SK_TRACE_MANAGED_RESOURCES
        GetTrace()->remove(this);
#endif

#ifdef SK_DEBUG
        SkASSERT(0 == this->getRefCnt());
        fRefCnt.store(1);
#endif
        delete this;
    }

    mutable std::atomic<int32_t> fRefCnt;
#ifdef SK_TRACE_MANAGED_RESOURCES
    uint32_t fKey;
#endif

    using INHERITED = SkNoncopyable;
};

// This subclass allows for recycling
class GrRecycledResource : public GrManagedResource {
public:
    // When recycle is called and there is only one ref left on the resource, we will signal that
    // the resource can be recycled for reuse. If the subclass (or whoever is managing this resource)
    // decides not to recycle the objects, it is their responsibility to call unref on the object.
    void recycle() const {
        if (this->unique()) {
            this->onRecycle();
        } else {
            this->unref();
        }
    }

private:
    virtual void onRecycle() const = 0;
};

/** \class GrTextureResource

  GrTextureResource is the base class for managed texture resources, and implements the
  basic releaseProc functionality for them.

*/
class GrTextureResource : public GrManagedResource {
public:
    GrTextureResource() {}

    ~GrTextureResource() override {
        SkASSERT(!fReleaseHelper);
    }

    void setRelease(sk_sp<GrRefCntedCallback> releaseHelper) {
        fReleaseHelper = std::move(releaseHelper);
    }

protected:
    mutable sk_sp<GrRefCntedCallback> fReleaseHelper;

    void invokeReleaseProc() const {
        if (fReleaseHelper) {
            // Depending on the ref count of fReleaseHelper this may or may not actually trigger
            // the ReleaseProc to be called.
            fReleaseHelper.reset();
        }
    }

private:
    using INHERITED = GrManagedResource;
};

#endif
