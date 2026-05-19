/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_NOOP_RAW_REF_H_
#define SRC_PARTITION_ALLOC_NOOP_RAW_REF_H_

// `raw_ref<T>` is a non-owning smart pointer that has improved memory-safety over raw reference.
// See the documentation for details:
// https://source.chromium.org/chromium/chromium/src/+/main:base/memory/raw_ref.md
//
// Here, skia provides a "no-op" implementation, because partition_alloc dependendency is missing.

#include <functional>
#include <type_traits>
#include <utility>

#include "src/partition_alloc/raw_ptr.h"

namespace partition_alloc::internal {

template <class T, RawPtrTraits Traits>
class raw_ref;

template <class T>
struct is_raw_ref : std::false_type {};

template <class T, RawPtrTraits Traits>
struct is_raw_ref<raw_ref<T, Traits>> : std::true_type {};

template <class T>
constexpr inline bool is_raw_ref_v = is_raw_ref<T>::value;

// A smart pointer for a pointer which can not be null, and which provides Use-after-Free protection
// in the same ways as raw_ptr. This class acts like a combination of std::reference_wrapper and
// raw_ptr.
//
// See raw_ptr and Chrome's //base/memory/raw_ptr.md for more details on the Use-after-Free
// protection.
//
// # Use after move
//
// The raw_ref type will abort if used after being moved.
//
// # Constness
//
// Use a `const raw_ref<T>` when the smart pointer should not be able to rebind to a new reference.
// Use a `const raw_ref<const T>` do the same for a const reference, which is like `const T&`.
//
// Unlike a native `T&` reference, a mutable `raw_ref<T>` can be changed independent of the
// underlying `T`, similar to `std::reference_wrapper`. That means the reference inside it can be
// moved and reassigned.
// RawPtrTraits are documented in Chromium's `base/memory/raw_ptr.md`.
template <class T, RawPtrTraits Traits = 0>
class SK_PA_TRIVIAL_ABI raw_ref {
  public:
    // Construct a raw_ref from a pointer, which must not be null.
    SK_PA_ALWAYS_INLINE constexpr static raw_ref from_ptr(T* ptr) noexcept { return raw_ref(*ptr); }

    // Construct a raw_ref from a reference.
    SK_PA_ALWAYS_INLINE constexpr explicit raw_ref(T& p) noexcept : inner_(std::addressof(p)) {}

    // Assign a new reference to the raw_ref, replacing the existing reference.
    SK_PA_ALWAYS_INLINE constexpr raw_ref& operator=(T& p) noexcept {
        inner_.operator=(&p);
        return *this;
    }

    // Disallow holding references to temporaries.
    explicit raw_ref(const T&& p) = delete;
    raw_ref& operator=(const T&& p) = delete;

    SK_PA_ALWAYS_INLINE constexpr raw_ref(const raw_ref& p) noexcept : inner_(p.inner_) {}

    SK_PA_ALWAYS_INLINE constexpr raw_ref(raw_ref&& p) noexcept : inner_(std::move(p.inner_)) {
        p.inner_ = nullptr;
    }

    SK_PA_ALWAYS_INLINE constexpr raw_ref& operator=(const raw_ref& p) noexcept {
        inner_.operator=(p.inner_);
        return *this;
    }

    SK_PA_ALWAYS_INLINE constexpr raw_ref& operator=(raw_ref&& p) noexcept {
        inner_.operator=(std::move(p.inner_));
        p.inner_ = nullptr;
        return *this;
    }

    // Deliberately implicit in order to support implicit upcast.
    // Delegate cross-kind conversion to the inner raw_ptr, which decides when to allow it.
    template <class U,
              RawPtrTraits PassedTraits,
              class = std::enable_if_t<std::is_convertible_v<U&, T&>>>
    // NOLINTNEXTLINE
    SK_PA_ALWAYS_INLINE constexpr raw_ref(const raw_ref<U, PassedTraits>& p) noexcept
        : inner_(p.inner_) {}
    // Deliberately implicit in order to support implicit upcast.
    // Delegate cross-kind conversion to the inner raw_ptr, which decides when to allow it.
    template <class U,
              RawPtrTraits PassedTraits,
              class = std::enable_if_t<std::is_convertible_v<U&, T&>>>
    // NOLINTNEXTLINE
    SK_PA_ALWAYS_INLINE constexpr raw_ref(raw_ref<U, PassedTraits>&& p) noexcept
        : inner_(std::move(p.inner_)) {
        p.inner_ = nullptr;
    }

    // Upcast assignment
    // Delegate cross-kind conversion to the inner raw_ptr, which decides when to allow it.
    template <class U,
              RawPtrTraits PassedTraits,
              class = std::enable_if_t<std::is_convertible_v<U&, T&>>>
    SK_PA_ALWAYS_INLINE constexpr raw_ref& operator=(const raw_ref<U, PassedTraits>& p) noexcept {
        inner_.operator=(p.inner_);
        return *this;
    }
    // Delegate cross-kind conversion to the inner raw_ptr, which decides when to
    // allow it.
    template <class U,
              RawPtrTraits PassedTraits,
              class = std::enable_if_t<std::is_convertible_v<U&, T&>>>
    SK_PA_ALWAYS_INLINE constexpr raw_ref& operator=(raw_ref<U, PassedTraits>&& p) noexcept {
        inner_.operator=(std::move(p.inner_));
        p.inner_ = nullptr;
        return *this;
    }

    SK_PA_ALWAYS_INLINE constexpr T& operator*() const { return inner_.operator*(); }
    SK_PA_ALWAYS_INLINE constexpr T& get() const { return *inner_.get(); }
    SK_PA_ALWAYS_INLINE constexpr T* operator->() const {
        return inner_.operator->();
    }

    SK_PA_ALWAYS_INLINE friend constexpr void swap(raw_ref& lhs, raw_ref& rhs) noexcept {
        swap(lhs.inner_, rhs.inner_);
    }

    template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
    friend bool operator==(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs);
    template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
    friend bool operator!=(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs);
    template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
    friend bool operator<(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs);
    template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
    friend bool operator>(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs);
    template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
    friend bool operator<=(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs);
    template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
    friend bool operator>=(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs);

    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator==(const raw_ref& lhs, const U& rhs) {
        return lhs.inner_ == &rhs;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator!=(const raw_ref& lhs, const U& rhs) {
        return lhs.inner_ != &rhs;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator<(const raw_ref& lhs, const U& rhs) {
        return lhs.inner_ < &rhs;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator>(const raw_ref& lhs, const U& rhs) {
        return lhs.inner_ > &rhs;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator<=(const raw_ref& lhs, const U& rhs) {
        return lhs.inner_ <= &rhs;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator>=(const raw_ref& lhs, const U& rhs) {
        return lhs.inner_ >= &rhs;
    }

    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator==(const U& lhs, const raw_ref& rhs) {
        return &lhs == rhs.inner_;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator!=(const U& lhs, const raw_ref& rhs) {
        return &lhs != rhs.inner_;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator<(const U& lhs, const raw_ref& rhs) {
        return &lhs < rhs.inner_;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator>(const U& lhs, const raw_ref& rhs) {
        return &lhs > rhs.inner_;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator<=(const U& lhs, const raw_ref& rhs) {
        return &lhs <= rhs.inner_;
    }
    template <class U, class = std::enable_if_t<!internal::is_raw_ref_v<U>, void>>
    SK_PA_ALWAYS_INLINE friend bool operator>=(const U& lhs, const raw_ref& rhs) {
        return &lhs >= rhs.inner_;
    }

  private:
    template <class U, RawPtrTraits R>
    friend class raw_ref;

    raw_ptr<T, Traits> inner_;
};

template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
SK_PA_ALWAYS_INLINE bool operator==(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs) {
    return lhs.inner_ == rhs.inner_;
}
template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
SK_PA_ALWAYS_INLINE bool operator!=(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs) {
    return lhs.inner_ != rhs.inner_;
}
template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
SK_PA_ALWAYS_INLINE bool operator<(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs) {
    return lhs.inner_ < rhs.inner_;
}
template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
SK_PA_ALWAYS_INLINE bool operator>(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs) {
    return lhs.inner_ > rhs.inner_;
}
template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
SK_PA_ALWAYS_INLINE bool operator<=(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs) {
    return lhs.inner_ <= rhs.inner_;
}
template <typename U, typename V, RawPtrTraits Traits1, RawPtrTraits Traits2>
SK_PA_ALWAYS_INLINE bool operator>=(const raw_ref<U, Traits1>& lhs, const raw_ref<V, Traits2>& rhs) {
    return lhs.inner_ >= rhs.inner_;
}

// CTAD deduction guide.
template <class T>
raw_ref(T&) -> raw_ref<T>;
template <class T>
raw_ref(const T&) -> raw_ref<const T>;

}  // namespace partition_alloc::internal

using partition_alloc::internal::raw_ref;

namespace std {

// Override so set/map lookups do not create extra raw_ref. This also allows C++ references to be
// used for lookup.
template <typename T, RawPtrTraits Traits>
struct less<raw_ref<T, Traits>> {
    using is_transparent = void;

    bool operator()(const raw_ref<T, Traits>& lhs, const raw_ref<T, Traits>& rhs) const {
        return lhs < rhs;
    }
    bool operator()(T& lhs, const raw_ref<T, Traits>& rhs) const { return lhs < rhs; }
    bool operator()(const raw_ref<T, Traits>& lhs, T& rhs) const { return lhs < rhs; }
};

// Specialize std::pointer_traits. The latter is required to obtain the underlying raw pointer in
// the std::to_address(pointer) overload. Implementing the pointer_traits is the standard blessed
// way to customize `std::to_address(pointer)` in C++20 [3].
//
// [1] https://wg21.link/pointer.traits.optmem
template <typename T, ::RawPtrTraits Traits>
struct pointer_traits<::raw_ref<T, Traits>> {
    using pointer = ::raw_ref<T, Traits>;
    using element_type = T;
    using difference_type = ptrdiff_t;

    template <typename U>
    using rebind = ::raw_ref<U, Traits>;

    static constexpr pointer pointer_to(element_type& r) noexcept { return pointer(r); }

    static constexpr element_type* to_address(pointer p) noexcept {
        // `raw_ref::get` is used instead of raw_ref::operator*`. It provides GetForExtraction
        // rather rather than GetForDereference semantics (see raw_ptr.h). This should be used when
        // we we don't know the memory will be accessed.
        return &(p.get());
    }
};

}  // namespace std

#endif  // SRC_PARTITION_ALLOC_NOOP_RAW_REF_H_
