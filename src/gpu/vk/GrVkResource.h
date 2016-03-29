/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkResource_DEFINED
#define GrVkResource_DEFINED

#include "SkAtomics.h"
#include "SkTDynamicHash.h"
#include "SkRandom.h"

class GrVkGpu;

// uncomment to enable tracing of resource refs
//#ifdef SK_DEBUG
//#define SK_TRACE_VK_RESOURCES
//#endif

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
  possibly abandonSubResources().
*/

class GrVkResource : SkNoncopyable {
public:
    // Simple refCount tracing, to ensure that everything ref'ed is unref'ed.
#ifdef SK_TRACE_VK_RESOURCES
    static const uint32_t& GetKey(const GrVkResource& r) { return r.fKey; }
    static uint32_t Hash(const uint32_t& k) { return k; }
    static SkTDynamicHash<GrVkResource, uint32_t> fTrace;
    static SkRandom fRandom;
#endif

    /** Default construct, initializing the reference count to 1.
     */
    GrVkResource() : fRefCnt(1) {
#ifdef SK_TRACE_VK_RESOURCES
        fKey = fRandom.nextU();
        fTrace.add(this);
#endif
    }

    /** Destruct, asserting that the reference count is 1.
     */
    virtual ~GrVkResource() {
#ifdef SK_DEBUG
        SkASSERTF(fRefCnt == 1, "fRefCnt was %d", fRefCnt);
        fRefCnt = 0;    // illegal value, to catch us if we reuse after delete
#endif
    }

#ifdef SK_DEBUG
    /** Return the reference count. Use only for debugging. */
    int32_t getRefCnt() const { return fRefCnt; }
#endif

    /** May return true if the caller is the only owner.
     *  Ensures that all previous owner's actions are complete.
     */
    bool unique() const {
        if (1 == sk_atomic_load(&fRefCnt, sk_memory_order_acquire)) {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of unique() from running
            // until previous owners are all totally done calling unref().
            return true;
        }
        return false;
    }

    /** Increment the reference count.
        Must be balanced by a call to unref() or unrefAndFreeResources().
     */
    void ref() const {
        SkASSERT(fRefCnt > 0);
        (void)sk_atomic_fetch_add(&fRefCnt, +1, sk_memory_order_relaxed);  // No barrier required.
    }

    /** Decrement the reference count. If the reference count is 1 before the
        decrement, then delete the object. Note that if this is the case, then
        the object needs to have been allocated via new, and not on the stack.
        Any GPU data associated with this resource will be freed before it's deleted.
     */
    void unref(const GrVkGpu* gpu) const {
        SkASSERT(fRefCnt > 0);
        SkASSERT(gpu);
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == sk_atomic_fetch_add(&fRefCnt, -1, sk_memory_order_acq_rel)) {
            // Like unique(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
            this->internal_dispose(gpu);
        }
    }

    /** Unref without freeing GPU data. Used only when we're abandoning the resource */
    void unrefAndAbandon() const {
        SkASSERT(fRefCnt > 0);
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == sk_atomic_fetch_add(&fRefCnt, -1, sk_memory_order_acq_rel)) {
            // Like unique(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
            this->internal_dispose();
        }
    }

#ifdef SK_DEBUG
    void validate() const {
        SkASSERT(fRefCnt > 0);
    }
#endif

private:
    /** Must be implemented by any subclasses.
     *  Deletes any Vk data associated with this resource
     */
    virtual void freeGPUData(const GrVkGpu* gpu) const = 0;

    /** Must be overridden by subclasses that themselves store GrVkResources.
     *  Will unrefAndAbandon those resources without deleting the underlying Vk data
     */
    virtual void abandonSubResources() const {}

    /**
     *  Called when the ref count goes to 0. Will free Vk resources.
     */
    void internal_dispose(const GrVkGpu* gpu) const {
        this->freeGPUData(gpu);
#ifdef SK_TRACE_VK_RESOURCES
        fTrace.remove(GetKey(*this));
#endif
        SkASSERT(0 == fRefCnt);
        fRefCnt = 1;
        delete this;
    }

    /**
     *  Internal_dispose without freeing Vk resources. Used when we've lost context.
     */
    void internal_dispose() const {
        this->abandonSubResources();
#ifdef SK_TRACE_VK_RESOURCES
        fTrace.remove(GetKey(*this));
#endif
        SkASSERT(0 == fRefCnt);
        fRefCnt = 1;
        delete this;
    }

    mutable int32_t fRefCnt;
#ifdef SK_TRACE_VK_RESOURCES
    uint32_t fKey;
#endif

    typedef SkNoncopyable INHERITED;
};


#endif
