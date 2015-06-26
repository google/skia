
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkRefCnt_DEFINED
#define SkRefCnt_DEFINED

#include "SkAtomics.h"
#include "SkTemplates.h"

/** \class SkRefCntBase

    SkRefCntBase is the base class for objects that may be shared by multiple
    objects. When an existing owner wants to share a reference, it calls ref().
    When an owner wants to release its reference, it calls unref(). When the
    shared object's reference count goes to zero as the result of an unref()
    call, its (virtual) destructor is called. It is an error for the
    destructor to be called explicitly (or via the object going out of scope on
    the stack or calling delete) if getRefCnt() > 1.
*/
class SK_API SkRefCntBase : SkNoncopyable {
public:
    /** Default construct, initializing the reference count to 1.
    */
    SkRefCntBase() : fRefCnt(1) {}

    /** Destruct, asserting that the reference count is 1.
    */
    virtual ~SkRefCntBase() {
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

    /** Increment the reference count. Must be balanced by a call to unref().
    */
    void ref() const {
        SkASSERT(fRefCnt > 0);
        (void)sk_atomic_fetch_add(&fRefCnt, +1, sk_memory_order_relaxed);  // No barrier required.
    }

    /** Decrement the reference count. If the reference count is 1 before the
        decrement, then delete the object. Note that if this is the case, then
        the object needs to have been allocated via new, and not on the stack.
    */
    void unref() const {
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

protected:
    /**
     *  Allow subclasses to call this if they've overridden internal_dispose
     *  so they can reset fRefCnt before the destructor is called. Should only
     *  be called right before calling through to inherited internal_dispose()
     *  or before calling the destructor.
     */
    void internal_dispose_restore_refcnt_to_1() const {
#ifdef SK_DEBUG
        SkASSERT(0 == fRefCnt);
        fRefCnt = 1;
#endif
    }

private:
    /**
     *  Called when the ref count goes to 0.
     */
    virtual void internal_dispose() const {
        this->internal_dispose_restore_refcnt_to_1();
        SkDELETE(this);
    }

    // The following friends are those which override internal_dispose()
    // and conditionally call SkRefCnt::internal_dispose().
    friend class SkWeakRefCnt;

    mutable int32_t fRefCnt;

    typedef SkNoncopyable INHERITED;
};

#ifdef SK_REF_CNT_MIXIN_INCLUDE
// It is the responsibility of the following include to define the type SkRefCnt.
// This SkRefCnt should normally derive from SkRefCntBase.
#include SK_REF_CNT_MIXIN_INCLUDE
#else
class SK_API SkRefCnt : public SkRefCntBase { };
#endif

///////////////////////////////////////////////////////////////////////////////

/** Helper macro to safely assign one SkRefCnt[TS]* to another, checking for
    null in on each side of the assignment, and ensuring that ref() is called
    before unref(), in case the two pointers point to the same object.
 */
#define SkRefCnt_SafeAssign(dst, src)   \
    do {                                \
        if (src) src->ref();            \
        if (dst) dst->unref();          \
        dst = src;                      \
    } while (0)


/** Call obj->ref() and return obj. The obj must not be NULL.
 */
template <typename T> static inline T* SkRef(T* obj) {
    SkASSERT(obj);
    obj->ref();
    return obj;
}

/** Check if the argument is non-null, and if so, call obj->ref() and return obj.
 */
template <typename T> static inline T* SkSafeRef(T* obj) {
    if (obj) {
        obj->ref();
    }
    return obj;
}

/** Check if the argument is non-null, and if so, call obj->unref()
 */
template <typename T> static inline void SkSafeUnref(T* obj) {
    if (obj) {
        obj->unref();
    }
}

template<typename T> static inline void SkSafeSetNull(T*& obj) {
    if (obj) {
        obj->unref();
        obj = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Utility class that simply unref's its argument in the destructor.
 */
template <typename T> class SkAutoTUnref : SkNoncopyable {
public:
    explicit SkAutoTUnref(T* obj = NULL) : fObj(obj) {}
    ~SkAutoTUnref() { SkSafeUnref(fObj); }

    T* get() const { return fObj; }

    T* reset(T* obj) {
        SkSafeUnref(fObj);
        fObj = obj;
        return obj;
    }

    void swap(SkAutoTUnref* other) {
        T* tmp = fObj;
        fObj = other->fObj;
        other->fObj = tmp;
    }

    /**
     *  Return the hosted object (which may be null), transferring ownership.
     *  The reference count is not modified, and the internal ptr is set to NULL
     *  so unref() will not be called in our destructor. A subsequent call to
     *  detach() will do nothing and return null.
     */
    T* detach() {
        T* obj = fObj;
        fObj = NULL;
        return obj;
    }

    T* operator->() const { return fObj; }
    operator T*() const { return fObj; }

private:
    T*  fObj;
};
// Can't use the #define trick below to guard a bare SkAutoTUnref(...) because it's templated. :(

class SkAutoUnref : public SkAutoTUnref<SkRefCnt> {
public:
    SkAutoUnref(SkRefCnt* obj) : SkAutoTUnref<SkRefCnt>(obj) {}
};
#define SkAutoUnref(...) SK_REQUIRE_LOCAL_VAR(SkAutoUnref)

// This is a variant of SkRefCnt that's Not Virtual, so weighs 4 bytes instead of 8 or 16.
// There's only benefit to using this if the deriving class does not otherwise need a vtable.
template <typename Derived>
class SkNVRefCnt : SkNoncopyable {
public:
    SkNVRefCnt() : fRefCnt(1) {}
    ~SkNVRefCnt() { SkASSERTF(1 == fRefCnt, "NVRefCnt was %d", fRefCnt); }

    // Implementation is pretty much the same as SkRefCntBase. All required barriers are the same:
    //   - unique() needs acquire when it returns true, and no barrier if it returns false;
    //   - ref() doesn't need any barrier;
    //   - unref() needs a release barrier, and an acquire if it's going to call delete.

    bool unique() const { return 1 == sk_atomic_load(&fRefCnt, sk_memory_order_acquire); }
    void    ref() const { (void)sk_atomic_fetch_add(&fRefCnt, +1, sk_memory_order_relaxed); }
    void  unref() const {
        if (1 == sk_atomic_fetch_add(&fRefCnt, -1, sk_memory_order_acq_rel)) {
            SkDEBUGCODE(fRefCnt = 1;)   // restore the 1 for our destructor's assert
            SkDELETE((const Derived*)this);
        }
    }
    void  deref() const { this->unref(); }

private:
    mutable int32_t fRefCnt;
};

#endif
