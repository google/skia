/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWeakRefCnt_DEFINED
#define SkWeakRefCnt_DEFINED

#include "SkRefCnt.h"
#include "SkThread.h"

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
    SK_DECLARE_INST_COUNT(SkWeakRefCnt)

    /** Default construct, initializing the reference counts to 1.
        The strong references collectively hold one weak reference. When the
        strong reference count goes to zero, the collectively held weak
        reference is released.
    */
    SkWeakRefCnt() : SkRefCnt(), fWeakCnt(1) {}

    /** Destruct, asserting that the weak reference count is 1.
    */
    virtual ~SkWeakRefCnt() {
#ifdef SK_DEBUG
        SkASSERT(fWeakCnt == 1);
        fWeakCnt = 0;
#endif
    }

    /** Return the weak reference count.
    */
    int32_t getWeakCnt() const { return fWeakCnt; }

#ifdef SK_DEBUG
    void validate() const {
        this->INHERITED::validate();
        SkASSERT(fWeakCnt > 0);
    }
#endif

    /** Creates a strong reference from a weak reference, if possible. The
        caller must already be an owner. If try_ref() returns true the owner
        is in posession of an additional strong reference. Both the original
        reference and new reference must be properly unreferenced. If try_ref()
        returns false, no strong reference could be created and the owner's
        reference is in the same state as before the call.
    */
    bool SK_WARN_UNUSED_RESULT try_ref() const {
        if (sk_atomic_conditional_inc(&fRefCnt) != 0) {
            // Aquire barrier (L/SL), if not provided above.
            // Prevents subsequent code from happening before the increment.
            sk_membar_aquire__after_atomic_conditional_inc();
            return true;
        }
        return false;
    }

    /** Increment the weak reference count. Must be balanced by a call to
        weak_unref().
    */
    void weak_ref() const {
        SkASSERT(fRefCnt > 0);
        SkASSERT(fWeakCnt > 0);
        sk_atomic_inc(&fWeakCnt);  // No barrier required.
    }

    /** Decrement the weak reference count. If the weak reference count is 1
        before the decrement, then call delete on the object. Note that if this
        is the case, then the object needs to have been allocated via new, and
        not on the stack.
    */
    void weak_unref() const {
        SkASSERT(fWeakCnt > 0);
        // Release barrier (SL/S), if not provided below.
        if (sk_atomic_dec(&fWeakCnt) == 1) {
            // Aquire barrier (L/SL), if not provided above.
            // Prevents code in destructor from happening before the decrement.
            sk_membar_aquire__after_atomic_dec();
#ifdef SK_DEBUG
            // so our destructor won't complain
            fWeakCnt = 1;
#endif
            this->INHERITED::internal_dispose();
        }
    }

    /** Returns true if there are no strong references to the object. When this
        is the case all future calls to try_ref() will return false.
    */
    bool weak_expired() const {
        return fRefCnt == 0;
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
    virtual void internal_dispose() const SK_OVERRIDE {
        weak_dispose();
        weak_unref();
    }

    /* Invariant: fWeakCnt = #weak + (fRefCnt > 0 ? 1 : 0) */
    mutable int32_t fWeakCnt;

    typedef SkRefCnt INHERITED;
};

#endif
