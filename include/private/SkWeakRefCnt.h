/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWeakRefCnt_DEFINED
#define SkWeakRefCnt_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkSpinlock.h"
#include <atomic>

/** \class SkWeakRefCnt

    SkWeakRefCnt is the base class for objects that may be shared by multiple
    objects. When an existing strong owner wants to share a reference, it calls
    ref(). When a strong owner wants to release its reference, it calls
    unref(). When the shared object's strong reference count goes to zero as
    the result of an unref() call, its (virtual) weak_dispose method is called.
    It is an error for the destructor to be called explicitly (or via the
    object going out of scope on the stack or calling delete) if
    getRefCnt() > 1.

    In addition to strong ownership, an owner may instead obtain a weak
    reference by calling weak_ref(). A call to weak_ref() must be balanced by a
    call to weak_unref(). To obtain a strong reference from a weak reference,
    call try_ref(). If try_ref() returns true, the owner's pointer is now also
    a strong reference on which unref() must be called. Note that this does not
    affect the original weak reference, weak_unref() must still be called. When
    the weak reference count goes to zero, the object is deleted. While the
    weak reference count is positive and the strong reference count is zero the
    object still exists, but will be in the disposed state. It is up to the
    object to define what this means.

    Note that a strong reference implicitly implies a weak reference. As a
    result, it is allowable for the owner of a strong ref to call try_ref().
    This will have the same effect as calling ref(), but may be more expensive.

    Example:

    SkWeakRefCnt myRef = strongRef.weak_ref();
    ... // strongRef.unref() may or may not be called
    if (myRef.try_ref()) {
        ... // use myRef
        myRef.unref();
    } else {
        // myRef is in the disposed state
    }
    myRef.weak_unref();
*/
class SK_API SkWeakRefCnt : public SkRefCnt {
public:
    /** Default construct, initializing the reference counts to 1.
        The strong references collectively hold one weak reference. When the
        strong reference count goes to zero, the collectively held weak
        reference is released.
    */
    SkWeakRefCnt() : SkRefCnt(), fWeakCnt(1) {}

    /** Destruct, asserting that the weak reference count is 1.
    */
    ~SkWeakRefCnt() override {
#ifdef SK_DEBUG
        SkASSERT(getWeakCnt() == 1);
        fWeakCnt.store(0, std::memory_order_relaxed);
#endif
    }

#ifdef SK_DEBUG
    /** Return the weak reference count. */
    int32_t getWeakCnt() const {
        return fWeakCnt.load(std::memory_order_relaxed);
    }
#endif

private:
    /** If fRefCnt is 0, returns 0.
     *  Otherwise increments fRefCnt, acquires, and returns the old value.
     */
    int32_t atomic_conditional_acquire_strong_ref() const {
        int32_t prev = fRefCnt.load(std::memory_order_relaxed);
        do {
            if (0 == prev) {
                break;
            }
        } while(!fRefCnt.compare_exchange_weak(prev, prev+1, std::memory_order_acquire,
                                                             std::memory_order_relaxed));
        return prev;
    }

public:

    /** SkWeakRefCnt breaks SkRefCnt::unique. When there are weak references they may try_ref a
     *  strong reference into existence after SkRefCnt::unique returns true.
     *
     *  The strong references collectively hold a single weak reference. A weak reference can only
     *  be created by a holder of some reference. So ownership is unique if one of the following are
     *  true
     *  1. simultaneously check for exactly one strong and one weak reference (the one weak
     *     reference is owned by the strong references and will not be used to try_ref)
     *  2. that there are no strong references and only one weak reference (the last owner is weak).
     *
     *  In the first case if the weak reference count is compared against one first there may be two
     *  strong references and the other may create a new weak reference and give up a strong
     *  reference before the check that the strong reference count is one. If the strong reference
     *  count is compared against one first there may be two weak references and the other may
     *  try_ref a new strong reference and give up the weak reference before the weak count is
     *  compared against one. The second case is simple since a zero strong reference count cannot
     *  be incremented, so just check that the weak reference count is one.
     *
     *  Therefore it is necessary to check both values are equal to one atomically. It isn't clear
     *  how to do this with c++11 atomics.
     *
     *  One way to make this work is for unique and try_ref to be mutually exclusive. This prevents
     *  weak references from making a unique strong reference non-unique during the unique check.
     *
     *  Ensures that all previous owner's actions are complete.
     */
    bool unique() const {
        SkAutoSpinlock lock(fWeakRefMuckingWithStrongRef);
        // These two checks need to be done atomically, or at least exclusive of try_ref running.
        // The order of the checks is also important, we need to aquire the uniqueness (or non-
        // existence) of the strong reference count first, then check the weak reference count.

        // If the weak reference count is compared against one first there may be two
        // strong references and the other may create a new weak reference and give up a strong
        // reference before the check that the strong reference count is one.

        // In this case we first observe that the strong reference count is zero or one so all
        // strong references are held by the caller. Since try_ref is blocked there is no one who
        // can create a new strong reference, ensuring that it stays that way. Then check that the
        // weak reference count is one; for once observed it to be one there must be a single owner
        // (either the strong references collectively, of which there are just one at this point) or
        // a single weak reference.
        if ( fRefCnt.load(std::memory_order_acquire) <= 1 &&
            fWeakCnt.load(std::memory_order_acquire) == 1)
        {
            // The acquire barrier is only really needed if we return true.  It
            // prevents code conditioned on the result of unique() from running
            // until previous owners are all totally done calling unref().
            return true;
        }
        return false;
    }

    /** Creates a strong reference from a weak reference, if possible. The
        caller must already be an owner. If try_ref() returns true the owner
        is in posession of an additional strong reference. Both the original
        reference and new reference must be properly unreferenced. If try_ref()
        returns false, no strong reference could be created and the owner's
        reference is in the same state as before the call.
    */
    bool SK_WARN_UNUSED_RESULT try_ref() const {
        SkAutoSpinlock lock(fWeakRefMuckingWithStrongRef);
        if (atomic_conditional_acquire_strong_ref() != 0) {
            // Acquire barrier (L/SL), if not provided above.
            // Prevents subsequent code from happening before the increment.
            return true;
        }
        return false;
    }

    /** Increment the weak reference count. Must be balanced by a call to
        weak_unref().
    */
    void weak_ref() const {
        SkASSERT(getRefCnt() > 0);
        SkASSERT(getWeakCnt() > 0);
        // No barrier required.
        (void)fWeakCnt.fetch_add(+1, std::memory_order_relaxed);
    }

    /** Decrement the weak reference count. If the weak reference count is 1
        before the decrement, then call delete on the object. Note that if this
        is the case, then the object needs to have been allocated via new, and
        not on the stack.
    */
    void weak_unref() const {
        SkASSERT(getWeakCnt() > 0);
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == fWeakCnt.fetch_add(-1, std::memory_order_acq_rel)) {
            // Like try_ref(), the acquire is only needed on success, to make sure
            // code in internal_dispose() doesn't happen before the decrement.
#ifdef SK_DEBUG
            // so our destructor won't complain
            fWeakCnt.store(1, std::memory_order_relaxed);
#endif
            this->INHERITED::internal_dispose();
        }
    }

    /** Returns true if there are no strong references to the object. When this
        is the case all future calls to try_ref() will return false.
    */
    bool weak_expired() const {
        return fRefCnt.load(std::memory_order_relaxed) == 0;
    }

protected:
    /** Called when the strong reference count goes to zero. This allows the
        object to free any resources it may be holding. Weak references may
        still exist and their level of allowed access to the object is defined
        by the object's class.
    */
    virtual void weak_dispose() const {
    }

private:
    /** Called when the strong reference count goes to zero. Calls weak_dispose
        on the object and releases the implicit weak reference held
        collectively by the strong references.
    */
    void internal_dispose() const override {
        weak_dispose();
        weak_unref();
    }

    /* Invariant: fWeakCnt = #weak + (fRefCnt > 0 ? 1 : 0) */
    mutable std::atomic<int32_t> fWeakCnt;
    mutable SkSpinlock fWeakRefMuckingWithStrongRef;

    typedef SkRefCnt INHERITED;
};

#endif
