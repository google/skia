/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POOLEDPTR
#define SKSL_POOLEDPTR

#include <cstddef>
#include <utility>

namespace SkSL {

/**
 *  A PooledPtr<T> exposes the same interface as a unique_ptr<T>, but the pointer is owned by our
 *  pooling system, not the system allocator. Deleters are not supported.
 *
 *  This class (and more generally, pooling in SkSL) is a work in progress.
 */
template <typename T>
class PooledPtr {
public:
    using pointer = T*;
    using element_type = T;

    // Construction
    constexpr PooledPtr() : fPtr(nullptr) {}
    constexpr PooledPtr(std::nullptr_t) : PooledPtr() {}
    explicit PooledPtr(pointer p) : fPtr(p) {}
    PooledPtr(PooledPtr&& u) : fPtr(u.release()) {}

    // Destruction
    ~PooledPtr() { this->returnToPool(fPtr); }

    // Copy-assignment (disallowed)
    PooledPtr(const PooledPtr&) = delete;
    PooledPtr& operator=(const PooledPtr&) = delete;

    // Move-assignment from another PooledPtr
    PooledPtr& operator=(PooledPtr&& that) {
        this->reset(that.release());
        return *this;
    }

    template <typename U> PooledPtr& operator=(PooledPtr<U>&& that) {
        this->reset(that.release());
        return *this;
    }

    // Null-assignment
    PooledPtr& operator=(std::nullptr_t) {
        this->reset();
        return *this;
    }

    // Methods on unique_ptr.
    explicit operator bool() const { return fPtr != nullptr; }
    T& operator *() const { return *fPtr; }
    T* operator ->() const { return fPtr; }

    T* get() const { return fPtr; }

    void reset(T* p = nullptr) {
        this->returnToPool(fPtr);
        fPtr = p;
    }

    void swap(PooledPtr& that) { std::swap(fPtr, that.fPtr); }

    T* release() { return std::exchange(fPtr, nullptr); }

private:
    void returnToPool(T* p) {
        // Today, our pool is the system allocator.
        delete p;
    }

    T* fPtr;
};

template <typename T> inline void swap(PooledPtr<T>& a, PooledPtr<T>& b) {
    a.swap(b);
}

template <typename T, typename U>
inline bool operator==(const PooledPtr<T>& a, const PooledPtr<U>& b) {
    return a.get() == b.get();
}

template <typename T, typename U>
inline bool operator!=(const PooledPtr<T>& a, const PooledPtr<U>& b) {
    return a.get() != b.get();
}

template <typename T, typename U>
inline bool operator<(const PooledPtr<T>& a, const PooledPtr<U>& b) {
    return a.get() < b.get();
}

template <typename T, typename U>
inline bool operator<=(const PooledPtr<T>& a, const PooledPtr<U>& b) {
    return a.get() <= b.get();
}

template <typename T, typename U>
inline bool operator>(const PooledPtr<T>& a, const PooledPtr<U>& b) {
    return a.get() > b.get();
}

template <typename T, typename U>
inline bool operator>=(const PooledPtr<T>& a, const PooledPtr<U>& b) {
    return a.get() >= b.get();
}

// We don't support < <= > >= against nullptr; plain pointers don't either. (unique_ptr does!)
template <typename T>
inline bool operator==(const PooledPtr<T>& a, std::nullptr_t) {
    return a.get() == nullptr;
}

template <typename T>
inline bool operator!=(const PooledPtr<T>& a, std::nullptr_t) {
    return a.get() != nullptr;
}

template <typename U>
inline bool operator==(std::nullptr_t, const PooledPtr<U>& b) {
    return nullptr == b.get();
}

template <typename U>
inline bool operator!=(std::nullptr_t, const PooledPtr<U>& b) {
    return nullptr != b.get();
}

}  // namespace SkSL

#endif
