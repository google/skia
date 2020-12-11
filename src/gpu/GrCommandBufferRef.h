/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCommandBufferRef_DEFINED
#define GrCommandBufferRef_DEFINED

#include "include/core/SkRefCnt.h"

 /** Check if the argument is non-null, and if so, call obj->addCommandBufferUsage() and return obj.
 */
template <typename T> static inline T* GrSafeRefCBUsage(T* obj) {
    if (obj) {
        obj->addCommandBufferUsage();
    }
    return obj;
}

/** Check if the argument is non-null, and if so, call obj->removeCommandBufferUsage()
 */
template <typename T> static inline void GrSafeUnrefCBUsage(T* obj) {
    if (obj) {
        obj->removeCommandBufferUsage();
    }
}

/**
 * Shared pointer class to wrap classes that support a addCommandBufferUsage() and
 * removeCommandBufferUsage() interface.
 *
 * This class supports copying, moving, and assigning an sk_sp into it. In general these commands do
 * not modify the sk_sp at all but just call addCommandBufferUsage() on the underlying object.
 *
 * This class is designed to be used by GrGpuResources that need to track when they are in use on
 * gpu (usually via a command buffer) separately from tracking if there are any current logical
 * usages in Ganesh. This allows for a scratch GrGpuResource to be reused for new draw calls even
 * if it is in use on the GPU.
 */
template <typename T> class gr_cb {
public:
    using element_type = T;

    constexpr gr_cb() : fPtr(nullptr) {}
    constexpr gr_cb(std::nullptr_t) : fPtr(nullptr) {}

    /**
     * Shares the underlying object by calling addCommandBufferUsage(), so that both the argument
     * and the newly created gr_cb both have a reference to it.
     */
    gr_cb(const gr_cb<T>& that) : fPtr(GrSafeRefCBUsage(that.get())) {}

    gr_cb(const sk_sp<T>& that) : fPtr(GrSafeRefCBUsage(that.get())) {}

    /**
     * Move the underlying object from the argument to the newly created gr_cb. Afterwards only
     * the new gr_cb will have a reference to the object, and the argument will point to null.
     * No call to addCommandBufferUsage() or removeCommandBufferUsage() will be made.
     */
    gr_cb(gr_cb<T>&& that) : fPtr(that.release()) {}

    /**
     * Copies the underlying object pointer from the argument to the gr_cb. It will then call
     * addCommandBufferUsage() on the new object.
     */
    gr_cb(sk_sp<T>&& that) : fPtr(GrSafeRefCBUsage(that.get())) {}

    /**
     * Calls removeCommandBufferUsage() on the underlying object pointer.
     */
    ~gr_cb() {
        GrSafeUnrefCBUsage(fPtr);
        SkDEBUGCODE(fPtr = nullptr);
    }

    gr_cb<T>& operator=(std::nullptr_t) {
        this->reset();
        return *this;
    }

    /**
     * Shares the underlying object referenced by the argument by calling addCommandBufferUsage() on
     * it. If this gr_cb previously had a reference to an object (i.e. not null) it will call
     * removeCommandBufferUsage() on thatobject.
     */
    gr_cb<T>& operator=(const gr_cb<T>& that) {
        if (this != &that) {
            this->reset(GrSafeRefCBUsage(that.get()));
        }
        return *this;
    }

    /**
     * Copies the underlying object pointer from the argument to the gr_cb. If the gr_cb previously
     * held a reference to another object, removeCommandBufferUsage() will be called on that object.
     * It will then call addCommandBufferUsage() on the new object.
     */
    gr_cb<T>& operator=(const sk_sp<T>& that) {
        this->reset(GrSafeRefCBUsage(that.get()));
        return *this;
    }

    /**
     * Move the underlying object from the argument to the gr_cb. If the gr_cb previously held
     * a reference to another object, removeCommandBufferUsage() will be called on that object. No
     * call to addCommandBufferUsage() will be made.
     */
    gr_cb<T>& operator=(gr_cb<T>&& that) {
        this->reset(that.release());
        return *this;
    }

    /**
     * Copies the underlying object pointer from the argument to the gr_cb. If the gr_cb previously
     * held a reference to another object, removeCommandBufferUsage() will be called on that object.
     * It will then call addCommandBufferUsage() on the new object.
     */
    gr_cb<T>& operator=(sk_sp<T>&& that) {
        this->reset(GrSafeRefCBUsage(that.get()));
        return *this;
    }

    T& operator*() const {
        SkASSERT(this->get() != nullptr);
        return *this->get();
    }

    explicit operator bool() const { return this->get() != nullptr; }

    T* get() const { return fPtr; }
    T* operator->() const { return fPtr; }

private:
    /**
     * Adopt the new bare pointer, and call removeCommandBufferUsage() on any previously held object
     * (if not null). No call to addCommandBufferUsage() will be made.
     */
    void reset(T* ptr = nullptr) {
        T* oldPtr = fPtr;
        fPtr = ptr;
        GrSafeUnrefCBUsage(oldPtr);
    }

    /**
     * Return the bare pointer, and set the internal object pointer to nullptr.
     * The caller must assume ownership of the object, and manage its reference count directly.
     * No call to removeCommandBufferUsage() will be made.
     */
    T* SK_WARN_UNUSED_RESULT release() {
        T* ptr = fPtr;
        fPtr = nullptr;
        return ptr;
    }

    T* fPtr;
};

template <typename T, typename U> inline bool operator==(const gr_cb<T>& a, const gr_cb<U>& b) {
    return a.get() == b.get();
}
template <typename T> inline bool operator==(const gr_cb<T>& a, std::nullptr_t) /*noexcept*/ {
    return !a;
}
template <typename T> inline bool operator==(std::nullptr_t, const gr_cb<T>& b) /*noexcept*/ {
    return !b;
}

template <typename T, typename U> inline bool operator!=(const gr_cb<T>& a, const gr_cb<U>& b) {
    return a.get() != b.get();
}
template <typename T> inline bool operator!=(const gr_cb<T>& a, std::nullptr_t) /*noexcept*/ {
    return static_cast<bool>(a);
}
template <typename T> inline bool operator!=(std::nullptr_t, const gr_cb<T>& b) /*noexcept*/ {
    return static_cast<bool>(b);
}

#endif
