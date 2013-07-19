
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkRefCnt_DEFINED
#define SkRefCnt_DEFINED

#include "SkThread.h"
#include "SkInstCnt.h"
#include "SkTemplates.h"

/** \class SkRefCnt

    SkRefCnt is the base class for objects that may be shared by multiple
    objects. When an existing owner wants to share a reference, it calls ref().
    When an owner wants to release its reference, it calls unref(). When the
    shared object's reference count goes to zero as the result of an unref()
    call, its (virtual) destructor is called. It is an error for the
    destructor to be called explicitly (or via the object going out of scope on
    the stack or calling delete) if getRefCnt() > 1.
*/
class SK_API SkRefCnt : SkNoncopyable {
public:
    SK_DECLARE_INST_COUNT_ROOT(SkRefCnt)

    /** Default construct, initializing the reference count to 1.
    */
    SkRefCnt() : fRefCnt(1) {}

    /** Destruct, asserting that the reference count is 1.
    */
    virtual ~SkRefCnt() {
#ifdef SK_DEBUG
        SkASSERT(fRefCnt == 1);
        fRefCnt = 0;    // illegal value, to catch us if we reuse after delete
#endif
    }

    /** Return the reference count. Use only for debugging. */
    int32_t getRefCnt() const { return fRefCnt; }

    /** Returns true if the caller is the only owner.
     *  Ensures that all previous owner's actions are complete.
     */
    bool unique() const {
        bool const unique = (1 == fRefCnt);
        if (unique) {
            // Aquire barrier (L/SL), if not provided by load of fRefCnt.
            // Prevents user's 'unique' code from happening before decrements.
            //TODO: issue the barrier.
        }
        return unique;
    }

    /** Increment the reference count. Must be balanced by a call to unref().
    */
    void ref() const {
        SkASSERT(fRefCnt > 0);
        sk_atomic_inc(&fRefCnt);  // No barrier required.
    }

    /** Decrement the reference count. If the reference count is 1 before the
        decrement, then delete the object. Note that if this is the case, then
        the object needs to have been allocated via new, and not on the stack.
    */
    void unref() const {
        SkASSERT(fRefCnt > 0);
        // Release barrier (SL/S), if not provided below.
        if (sk_atomic_dec(&fRefCnt) == 1) {
            // Aquire barrier (L/SL), if not provided above.
            // Prevents code in dispose from happening before the decrement.
            sk_membar_aquire__after_atomic_dec();
            internal_dispose();
        }
    }

    void validate() const {
        SkASSERT(fRefCnt > 0);
    }

    /**
     * Alias for unref(), for compatibility with WTF::RefPtr.
     */
    void deref() { this->unref(); }

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
    friend class GrTexture;
    friend class SkWeakRefCnt;

    mutable int32_t fRefCnt;

    typedef SkNoncopyable INHERITED;
};

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

    /**
     *  BlockRef<B> is a type which inherits from B, cannot be created,
     *  cannot be deleted, and makes ref and unref private.
     */
    template<typename B> class BlockRef : public B {
    private:
        BlockRef();
        ~BlockRef();
        void ref() const;
        void unref() const;
    };

    /** If T is const, the type returned from operator-> will also be const. */
    typedef typename SkTConstType<BlockRef<T>, SkTIsConst<T>::value>::type BlockRefType;

    /**
     *  SkAutoTUnref assumes ownership of the ref. As a result, it is an error
     *  for the user to ref or unref through SkAutoTUnref. Therefore
     *  SkAutoTUnref::operator-> returns BlockRef<T>*. This prevents use of
     *  skAutoTUnrefInstance->ref() and skAutoTUnrefInstance->unref().
     */
    BlockRefType *operator->() const {
        return static_cast<BlockRefType*>(fObj);
    }
    operator T*() { return fObj; }

private:
    T*  fObj;
};

class SkAutoUnref : public SkAutoTUnref<SkRefCnt> {
public:
    SkAutoUnref(SkRefCnt* obj) : SkAutoTUnref<SkRefCnt>(obj) {}
};

class SkAutoRef : SkNoncopyable {
public:
    SkAutoRef(SkRefCnt* obj) : fObj(obj) { SkSafeRef(obj); }
    ~SkAutoRef() { SkSafeUnref(fObj); }
private:
    SkRefCnt* fObj;
};

/** Wrapper class for SkRefCnt pointers. This manages ref/unref of a pointer to
    a SkRefCnt (or subclass) object.
 */
template <typename T> class SkRefPtr {
public:
    SkRefPtr() : fObj(NULL) {}
    SkRefPtr(T* obj) : fObj(obj) { SkSafeRef(fObj); }
    SkRefPtr(const SkRefPtr& o) : fObj(o.fObj) { SkSafeRef(fObj); }
    ~SkRefPtr() { SkSafeUnref(fObj); }

    SkRefPtr& operator=(const SkRefPtr& rp) {
        SkRefCnt_SafeAssign(fObj, rp.fObj);
        return *this;
    }
    SkRefPtr& operator=(T* obj) {
        SkRefCnt_SafeAssign(fObj, obj);
        return *this;
    }

    T* get() const { return fObj; }
    T& operator*() const { return *fObj; }
    T* operator->() const { return fObj; }

    typedef T* SkRefPtr::*unspecified_bool_type;
    operator unspecified_bool_type() const {
        return fObj ? &SkRefPtr::fObj : NULL;
    }

private:
    T* fObj;
};

#endif
