/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCFObject_DEFINED
#define SkCFObject_DEFINED

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#import <CoreFoundation/CoreFoundation.h>

/**
 * Wrapper class for managing lifetime of CoreFoundation objects. It will call
 * CFRetain and CFRelease appropriately on creation, assignment, and deletion.
 * Based on sk_sp<>.
 */
template <typename T> static inline T SkCFSafeRetain(T obj) {
    if (obj) {
        CFRetain(obj);
    }
    return obj;
}

template <typename T> static inline void SkCFSafeRelease(T obj) {
    if (obj) {
        CFRelease(obj);
    }
}

template <typename T> class sk_cf_obj {
public:
    using element_type = T;

    constexpr sk_cf_obj() {}
    constexpr sk_cf_obj(std::nullptr_t) {}

    /**
     *  Shares the underlying object by calling CFRetain(), so that both the argument and the newly
     *  created sk_cf_obj both have a reference to it.
     */
    sk_cf_obj(const sk_cf_obj<T>& that) : fObject(SkCFSafeRetain(that.get())) {}

    /**
     *  Move the underlying object from the argument to the newly created sk_cf_obj. Afterwards only
     *  the new sk_cf_obj will have a reference to the object, and the argument will point to null.
     *  No call to CFRetain() or CFRelease() will be made.
     */
    sk_cf_obj(sk_cf_obj<T>&& that) : fObject(that.release()) {}

    /**
     *  Adopt the bare object into the newly created sk_cf_obj.
     *  No call to CFRetain() or CFRelease() will be made.
     */
    explicit sk_cf_obj(T obj) {
        fObject = obj;
    }

    /**
     *  Calls CFRelease() on the underlying object pointer.
     */
    ~sk_cf_obj() {
        SkCFSafeRelease(fObject);
        SkDEBUGCODE(fObject = nil);
    }

    sk_cf_obj<T>& operator=(std::nullptr_t) { this->reset(); return *this; }

    /**
     *  Shares the underlying object referenced by the argument by calling CFRetain() on it. If this
     *  sk_cf_obj previously had a reference to an object (i.e. not null) it will call CFRelease()
     *  on that object.
     */
    sk_cf_obj<T>& operator=(const sk_cf_obj<T>& that) {
        if (this != &that) {
            this->reset(SkCFSafeRetain(that.get()));
        }
        return *this;
    }

    /**
     *  Move the underlying object from the argument to the sk_cf_obj. If the sk_cf_obj
     * previously held a reference to another object, CFRelease() will be called on that object.
     * No call to CFRetain() will be made.
     */
    sk_cf_obj<T>& operator=(sk_cf_obj<T>&& that) {
        this->reset(that.release());
        return *this;
    }

    explicit operator bool() const { return this->get() != nil; }

    T get() const { return fObject; }
    T operator*() const {
        SkASSERT(fObject);
        return fObject;
    }

    /**
     *  Adopt the new object, and call CFRelease() on any previously held object (if not null).
     *  No call to CFRetain() will be made.
     */
    void reset(T object = nil) {
        // Need to unref after assigning, see
        // http://wg21.cmeerw.net/lwg/issue998
        // http://wg21.cmeerw.net/lwg/issue2262
        T oldObject = fObject;
        fObject = object;
        SkCFSafeRelease(oldObject);
    }

    /**
     *  Shares the new object by calling CFRetain() on it. If this sk_cf_obj previously had a
     *  reference to an object (i.e. not null) it will call CFRelease() on that object.
     */
    void retain(T object) {
        if (fObject != object) {
            this->reset(SkCFSafeRetain(object));
        }
    }

    /**
     *  Return the original object, and set the internal object to nullptr.
     *  The caller must assume ownership of the object, and manage its reference count directly.
     *  No call to CFRelease() will be made.
     */
    T SK_WARN_UNUSED_RESULT release() {
        T obj = fObject;
        fObject = nil;
        return obj;
    }

private:
    T fObject = nil;
};

template <typename T> inline bool operator==(const sk_cf_obj<T>& a,
                                             const sk_cf_obj<T>& b) {
    return a.get() == b.get();
}
template <typename T> inline bool operator==(const sk_cf_obj<T>& a,
                                             std::nullptr_t) {
    return !a;
}
template <typename T> inline bool operator==(std::nullptr_t,
                                             const sk_cf_obj<T>& b) {
    return !b;
}

template <typename T> inline bool operator!=(const sk_cf_obj<T>& a,
                                             const sk_cf_obj<T>& b) {
    return a.get() != b.get();
}
template <typename T> inline bool operator!=(const sk_cf_obj<T>& a,
                                             std::nullptr_t) {
    return static_cast<bool>(a);
}
template <typename T> inline bool operator!=(std::nullptr_t,
                                             const sk_cf_obj<T>& b) {
    return static_cast<bool>(b);
}

/*
 *  Returns a sk_cf_obj wrapping the provided object AND calls retain on it (if not null).
 *
 *  This is different than the semantics of the constructor for sk_cf_obj, which just wraps the
 *  object, effectively "adopting" it.
 */
template <typename T> sk_cf_obj<T> sk_ret_cf_obj(T obj) {
    return sk_cf_obj<T>(SkCFSafeRetain(obj));
}

#endif  // SK_BUILD_FOR_MAC || SK_BUILD_FOR_IOS
#endif  // SkCFObject_DEFINED
