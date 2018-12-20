/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkResource_DEFINED
#define GrVkResource_DEFINED


#include "SkRandom.h"
#include "SkTHash.h"
#include <atomic>

class GrVkGpu;

// uncomment to enable tracing of resource refs
#ifdef SK_DEBUG
#define SK_TRACE_VK_RESOURCES
#endif

/** \class GrVkResource

  GrVkResource is the base class for Vulkan resources that may be shared by multiple
  objects. When an existing owner wants to share a reference, it calls ref().
  When an owner wants to release its reference, it calls unref(). When the
  shared object's reference count goes to zero as the result of an unref()
  call, its (virtual) destructor is called. It is an error for the
  destructor to be called explicitly (or via the object going out of scope on
  the stack or calling delete) if getRefCnt() > 1.

  This is nearly identical to SkRefCntBase. The exceptions are that unref()
  takes a GrVkGpu, and any derived classes must implement freeGPUData() and
  possibly abandonGPUData().
*/

class GrVkResource : SkNoncopyable {
public:
    // Simple refCount tracing, to ensure that everything ref'ed is unref'ed.
#ifdef SK_TRACE_VK_RESOURCES
    struct Hash {
        uint32_t operator()(const GrVkResource* const& r) const {
            SkASSERT(r);
            return r->fKey;
        }
    };

    class Trace {
    public:
        ~Trace() {
            fHashSet.foreach([](const GrVkResource* r) {
                r->dumpInfo();
            });
            SkASSERT(0 == fHashSet.count());
        }

        void add(const GrVkResource* r) {
            fHashSet.add(r);
        }

        void remove(const GrVkResource* r) {
            fHashSet.remove(r);
        }

    private:
        SkTHashSet<const GrVkResource*, GrVkResource::Hash> fHashSet;
    };

    static std::atomic<uint32_t> fKeyCounter;
#endif

    /** Default construct, initializing the reference count to 1.
     */
    GrVkResource() : fRefCnt(1) {
#ifdef SK_TRACE_VK_RESOURCES
        fKey = fKeyCounter.fetch_add(+1, std::memory_order_relaxed);
        GetTrace()->add(this);
#endif
    }

    /** Destruct, asserting that the reference count is 1.
     */
    virtual ~GrVkResource() {
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
    void unref(GrVkGpu* gpu) const {
        SkASSERT(gpu);
        // A release here acts in place of all releases we "should" have been doing in ref().
        int newRefCount = fRefCnt.fetch_add(-1, std::memory_order_acq_rel);
        SkASSERT(newRefCount >= 0);
        if (newRefCount == 1) {
            // Like unique(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
            this->internal_dispose(gpu);
        }
    }

    /** Unref without freeing GPU data. Used only when we're abandoning the resource */
    void unrefAndAbandon() const {
        SkASSERT(this->getRefCnt() > 0);
        // A release here acts in place of all releases we "should" have been doing in ref().
        int newRefCount = fRefCnt.fetch_add(-1, std::memory_order_acq_rel);
        SkASSERT(newRefCount >= 0);
        if (newRefCount == 1) {
            // Like unique(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
            this->internal_dispose();
        }
    }

    // Called every time this resource is added to a command buffer.
    virtual void notifyAddedToCommandBuffer() const {}
    // Called every time this resource is removed from a command buffer (typically because
    // the command buffer finished execution on the GPU but also when the command buffer
    // is abandoned.)
    virtual void notifyRemovedFromCommandBuffer() const {}

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT(this->getRefCnt() > 0);
    }
#endif

#ifdef SK_TRACE_VK_RESOURCES
    /** Output a human-readable dump of this resource's information
     */
    virtual void dumpInfo() const = 0;
#endif

private:
#ifdef SK_TRACE_VK_RESOURCES
    static Trace* GetTrace() {
        static Trace kTrace;
        return &kTrace;
    }
#endif

    /** Must be implemented by any subclasses.
     *  Deletes any Vk data associated with this resource
     */
    virtual void freeGPUData(GrVkGpu* gpu) const = 0;

    /**
     * Called from unrefAndAbandon. Resources should do any necessary cleanup without freeing
     * underlying Vk objects. This must be overridden by subclasses that themselves store
     * GrVkResources since those resource will need to be unrefed.
     */
    virtual void abandonGPUData() const {}

    /**
     *  Called when the ref count goes to 0. Will free Vk resources.
     */
    void internal_dispose(GrVkGpu* gpu) const {
        this->freeGPUData(gpu);
#ifdef SK_TRACE_VK_RESOURCES
        GetTrace()->remove(this);
#endif

#ifdef SK_DEBUG
        SkASSERT(0 == this->getRefCnt());
        fRefCnt.store(1);
#endif
        delete this;
    }

    /**
     *  Internal_dispose without freeing Vk resources. Used when we've lost context.
     */
    void internal_dispose() const {
        this->abandonGPUData();
#ifdef SK_TRACE_VK_RESOURCES
        GetTrace()->remove(this);
#endif

#ifdef SK_DEBUG
        SkASSERT(0 == this->getRefCnt());
        fRefCnt.store(1);
#endif
        delete this;
    }

    mutable std::atomic<int32_t> fRefCnt;
#ifdef SK_TRACE_VK_RESOURCES
    uint32_t fKey;
#endif

    typedef SkNoncopyable INHERITED;
};

// This subclass allows for recycling
class GrVkRecycledResource : public GrVkResource {
public:
    // When recycle is called and there is only one ref left on the resource, we will signal that
    // the resource can be recycled for reuse. If the sublass (or whoever is managing this resource)
    // decides not to recycle the objects, it is their responsibility to call unref on the object.
    void recycle(GrVkGpu* gpu) const {
        if (this->unique()) {
            this->onRecycle(gpu);
        } else {
            this->unref(gpu);
        }
    }

private:
    virtual void onRecycle(GrVkGpu* gpu) const = 0;
};

#endif
