/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRefCnt_DEFINED
#define GrRefCnt_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/SkAssert.h"
#include "include/private/SkDebug.h"
#include "src/partition_alloc/raw_ptr.h"

#include <cstddef>
#include <type_traits>

template <typename T, typename Policy> class gr_sp {
private:
    static inline T* SafeRef(T* obj) {
        if (obj) {
            Policy::Ref(obj);
        }
        return obj;
    }

    static inline void SafeUnref(T* obj) {
        if (obj) {
            Policy::Unref(obj);
        }
    }

public:
    using element_type = T;

    constexpr gr_sp() : fPtr(nullptr) {}
    constexpr gr_sp(std::nullptr_t) : fPtr(nullptr) {}

    /**
     * Shares the underlying object by calling Ref(), so that both the argument and the newly
     * created gr_sp both have a reference to it.
     */
    gr_sp(const gr_sp<T, Policy>& that) : fPtr(SafeRef(that.get())) {}
    template <typename U,
              typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    gr_sp(const gr_sp<U, Policy>& that) : fPtr(SafeRef(that.get())) {}

    gr_sp(const sk_sp<T>& that) : fPtr(SafeRef(that.get())) {}


    /**
     * Move the underlying object from the argument to the newly created gr_sp. Afterwards only the
     * new gr_sp will have a reference to the object, and the argument will point to null.
     * No call to Ref() or Unref() will be made.
     */
    gr_sp(gr_sp<T, Policy>&& that) : fPtr(that.release()) {}

    /**
     * Copies the underlying object pointer from the argument to the gr_sp. It will then call
     * Ref() on the new object.
     */
    gr_sp(sk_sp<T>&& that) : fPtr(SafeRef(that.get())) {}

    /**
     *  Adopt the bare pointer into the newly created gr_sp.
     *  No call to Ref() or Unref() will be made.
     */
    explicit gr_sp(T* obj) : fPtr(obj) {}

    /**
     * Calls Unref() on the underlying object pointer.
     */
    ~gr_sp() {
        SafeUnref(this->release());
    }

    gr_sp& operator=(std::nullptr_t) {
        this->reset();
        return *this;
    }

    /**
     * Shares the underlying object referenced by the argument by calling Ref() on it. If this gr_sp
     * previously had a reference to an object (i.e. not null) it will call Unref() on that object.
     */
    gr_sp& operator=(const gr_sp<T, Policy>& that) {
        if (this != &that) {
            this->reset(SafeRef(that.get()));
        }
        return *this;
    }

    /**
     * Copies the underlying object pointer from the argument to the gr_sp. If the gr_sp previously
     * held a reference to another object, Unref() will be called on that object. It will then call
     * Ref() on the new object.
     */
    gr_sp& operator=(const sk_sp<T>& that) {
        this->reset(SafeRef(that.get()));
        return *this;
    }

    /**
     * Move the underlying object from the argument to the gr_sp. If the gr_sp previously held
     * a reference to another object, Unref() will be called on that object. No call to Ref() will
     * be made.
     */
    gr_sp& operator=(gr_sp<T, Policy>&& that) {
        this->reset(that.release());
        return *this;
    }

    /**
     * Copies the underlying object pointer from the argument to the gr_sp. If the gr_sp previously
     * held a reference to another object, Unref() will be called on that object. It will then call
     * Ref() on the new object.
     */
    gr_sp& operator=(sk_sp<T>&& that) {
        this->reset(SafeRef(that.get()));
        return *this;
    }

    T& operator*() const {
        SkASSERT(this->get() != nullptr);
        return *this->get();
    }

    explicit operator bool() const { return this->get() != nullptr; }

    T* get() const { return fPtr; }
    T* operator->() const { return fPtr; }

    /**
     * Adopt the new bare pointer, and call Unref() on any previously held object (if not null).
     * No call to Ref() will be made.
     */
    void reset(T* ptr = nullptr) {
        T* oldPtr = fPtr;
        fPtr = ptr;
        SafeUnref(oldPtr);
    }

private:
    /**
     * Return the bare pointer, and set the internal object pointer to nullptr.
     * The caller must assume ownership of the object, and manage its reference count directly.
     * No call to Unref() will be made.
     */
    [[nodiscard]] T* release() {
        T* ptr = fPtr;
        fPtr = nullptr;
        return ptr;
    }

    raw_ptr<T> fPtr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Shared pointer class to wrap classes that support a refCommandBuffer() and unrefCommandBuffer()
 * interface.
 *
 * This class supports copying, moving, and assigning an sk_sp into it. In general these commands do
 * not modify the sk_sp at all but just call refCommandBuffer() on the underlying object.
 *
 * This class is designed to be used by GrGpuResources/graphite::Resources that need to track when
 * they are in use on gpu (usually via a command buffer) separately from tracking if there are any
 * current logical usages in Skia. This allows for a scratch resources to be reused for new draw
 * calls even if it is in use on the GPU.
 */
struct GrCommandBufferRefPolicy {
    template <typename T> static void Ref(T* obj) { obj->refCommandBuffer(); }
    template <typename T> static void Unref(T* obj) { obj->unrefCommandBuffer(); }
};
template <typename T> using gr_cb = gr_sp<T, GrCommandBufferRefPolicy>;

////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * This class mimics sk_sp but instead of calling unref it calls recycle instead.
 */
struct GrRecycleRefPolicy {
    template <typename T> static void Ref(T* obj) { obj->ref(); }
    template <typename T> static void Unref(T* obj) { obj->recycle(); }
};
template <typename T> using gr_rp = gr_sp<T, GrRecycleRefPolicy>;

/**
 *  Returns a gr_rp wrapping the provided ptr AND calls ref on it (if not null).
 *
 *  This is different than the semantics of the constructor for gr_rp, which just wraps the ptr,
 *  effectively "adopting" it.
 */
template <typename T> gr_rp<T> gr_ref_rp(T* obj) { return gr_rp<T>(SkSafeRef(obj)); }

template <typename T> gr_rp<T> gr_ref_rp(const T* obj) {
    return gr_rp<T>(const_cast<T*>(SkSafeRef(obj)));
}
#endif
