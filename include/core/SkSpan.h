/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSpan_DEFINED
#define SkSpan_DEFINED

/*
 *  Define this to keep the bespoke SkSpan implementation, until clients are ready.
 *  When this is not defined, SkSpan becomes just an alias for std::span
 */
#define SK_USE_LEGACY_SKSPAN

/*
 *  SkSpan handles single POD initialization differently from std::span
 *  This macro allows clients to initialize a span with a single POD that will work with
 *  either implementation of SkSpan.
 *
 *  When we switch to std::span entirely, callsites can remove this macro, and juse type {{a}}
 */
#ifdef SK_USE_LEGACY_SKSPAN
    #define SKSPAN_INIT_ONE(elem)   {elem}
#else
    #define SKSPAN_INIT_ONE(elem)   {{elem}}
#endif

#ifdef SK_USE_LEGACY_SKSPAN

#include "include/private/SkAssert.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTo.h"

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <utility>

// Having this be an export works around IWYU churn related to
// https://github.com/include-what-you-use/include-what-you-use/issues/1121
#include <type_traits> // IWYU pragma: export

// Add macro to check the lifetime of initializer_list arguments. initializer_list has a very
// short life span, and can only be used as a parameter, and not as a variable.
#if defined(__clang__) && defined(__has_cpp_attribute) && __has_cpp_attribute(clang::lifetimebound)
#define SK_CHECK_IL_LIFETIME [[clang::lifetimebound]]
#else
#define SK_CHECK_IL_LIFETIME
#endif

inline constexpr size_t SkDynamicExtent = std::numeric_limits<size_t>::max();

template <typename T, size_t Extent = SkDynamicExtent>
class SkSpan;

namespace sk_span_internal {

template <typename From, typename To>
inline constexpr bool kLegalDataConversion =
        std::is_convertible_v<std::remove_reference_t<From>(*)[],
                              std::remove_reference_t<To>(*)[]>;

template <typename R>
inline constexpr bool kIsSkSpan = false;
template <typename T, size_t Extent>
inline constexpr bool kIsSkSpan<SkSpan<T, Extent>> = true;

template <typename R>
constexpr size_t compute_extent() {
    using U = std::remove_cvref_t<R>;
    if constexpr (std::is_array_v<U>) {
        return std::extent_v<U>;
    } else if constexpr (requires {
                             U::extent;
                             requires std::is_convertible_v<decltype(U::extent), size_t>;
                         }) {
        return U::extent;
    } else if constexpr (requires {
                             std::tuple_size<U>::value;
                             requires std::is_convertible_v<decltype(std::tuple_size<U>::value),
                                                            size_t>;
                         }) {
        return std::tuple_size<U>::value;
    } else {
        return SkDynamicExtent;
    }
}

template <typename R>
inline constexpr size_t kComputedExtent = compute_extent<R>();

template <typename T, typename R>
concept CompatibleContainer =
        !kIsSkSpan<std::remove_cvref_t<R>> &&
        !std::is_array_v<std::remove_cvref_t<R>> &&
        requires(R&& r) {
            std::data(r);
            std::size(r);
            requires kLegalDataConversion<std::remove_pointer_t<decltype(std::data(r))>, T>;
        };

template <typename T, size_t Extent>
struct SpanStorage {
    constexpr SpanStorage(T* ptr, size_t size) : fPtr(ptr) {
        if (size != Extent) SK_UNLIKELY {
            SkASSERT_RELEASE(size == Extent);
            SkUNREACHABLE;
        }
    }
    constexpr size_t size() const { return Extent; }
    T* fPtr;
};

template <typename T>
struct SpanStorage<T, SkDynamicExtent> {
    constexpr SpanStorage(T* ptr, size_t size) : fPtr(ptr), fSize(size) {}
    constexpr size_t size() const { return fSize; }
    T* fPtr;
    size_t fSize;
};

}  // namespace sk_span_internal

/**
 * SkSpan holds a reference to contiguous data of type T along with a count. SkSpan does not own
 * the data itself but is merely a reference, therefore you must take care with the lifetime of
 * the underlying data.
 *
 * SkSpan is a count and a pointer into existing array or data type that stores its data in
 * contiguous memory like std::vector. Any container that works with std::size() and std::data()
 * can be used.
 *
 * SkSpan makes a convenient parameter for a routine to accept array like things. This allows you to
 * write the routine without overloads for all different container types.
 *
 * Example:
 *     void routine(SkSpan<const int> a) { ... }
 *
 *     std::vector v = {1, 2, 3, 4, 5};
 *
 *     routine(a);
 *
 * A word of caution when working with initializer_list, initializer_lists have a lifetime that is
 * limited to the current statement. The following is correct and safe:
 *
 * Example:
 *     routine({1,2,3,4,5});
 *
 * The following is undefined, and will result in erratic execution:
 *
 * Bad Example:
 *     initializer_list l = {1, 2, 3, 4, 5};   // The data behind l dies at the ;.
 *     routine(l);
 */
template <typename T, size_t Extent>
class SkSpan : private sk_span_internal::SpanStorage<T, Extent> {
public:
    static constexpr size_t extent = Extent;

    constexpr SkSpan()
        requires (Extent == 0 || Extent == SkDynamicExtent)
            : INHERITED{nullptr, 0} {}

    template <typename Integer>
        requires std::is_integral_v<Integer>
    constexpr explicit(Extent != SkDynamicExtent) SkSpan(T* ptr, Integer size)
            : INHERITED{ptr, SkToSizeT(size)} {
        SkASSERT_RELEASE(SkTFitsIn<size_t>(size));
        SkASSERT_RELEASE(ptr || this->size() == 0);  // disallow nullptr + a nonzero size
        SkASSERT_RELEASE(this->size() < (std::numeric_limits<size_t>::max() / sizeof(T)));
    }

    template <typename U, size_t OtherExtent>
        requires (sk_span_internal::kLegalDataConversion<U, T> &&
                  (Extent == SkDynamicExtent || OtherExtent == SkDynamicExtent ||
                   Extent == OtherExtent))
    constexpr explicit(Extent != SkDynamicExtent && OtherExtent == SkDynamicExtent)
    SkSpan(const SkSpan<U, OtherExtent>& that)
            : INHERITED{std::data(that), std::size(that)} {}

    constexpr SkSpan(const SkSpan& o) = default;

    template <size_t N>
        requires (Extent == SkDynamicExtent || Extent == N)
    constexpr SkSpan(T (&a)[N]) : SkSpan(a, N) {}

    template <typename Container, size_t N = sk_span_internal::kComputedExtent<Container>>
        requires (sk_span_internal::CompatibleContainer<T, Container> &&
                  (Extent == SkDynamicExtent || N == SkDynamicExtent || Extent == N))
    constexpr explicit(Extent != SkDynamicExtent && N == SkDynamicExtent)
    SkSpan(Container&& c) : SkSpan(std::data(c), std::size(c)) {}

    constexpr explicit(Extent != SkDynamicExtent)
    SkSpan(std::initializer_list<T> il SK_CHECK_IL_LIFETIME)
            : SkSpan(std::data(il), std::size(il)) {}

    constexpr SkSpan& operator=(const SkSpan& that) = default;

    constexpr T& operator[](size_t i) const {
        return this->fPtr[sk_collection_check_bounds(i, this->size())];
    }
    constexpr T& front() const {
        sk_collection_not_empty(this->empty());
        return this->fPtr[0];
    }
    constexpr T& back() const {
        sk_collection_not_empty(this->empty());
        return this->fPtr[this->size() - 1];
    }
    constexpr T* begin() const { return this->fPtr; }
    constexpr T* end() const { return this->fPtr + this->size(); }
    constexpr auto rbegin() const { return std::make_reverse_iterator(this->end()); }
    constexpr auto rend() const { return std::make_reverse_iterator(this->begin()); }
    constexpr T* data() const { return this->begin(); }
    using sk_span_internal::SpanStorage<T, Extent>::size;
    constexpr bool empty() const { return this->size() == 0; }
    constexpr size_t size_bytes() const { return this->size() * sizeof(T); }

    template <size_t Count>
        requires (Count != SkDynamicExtent && Count <= Extent)
    constexpr SkSpan<T, Count> first() const {
        if constexpr (Extent == SkDynamicExtent) {
            (void)sk_collection_check_length(Count, this->size());
        }
        return SkSpan<T, Count>{this->fPtr, Count};
    }

    template <size_t Count>
        requires (Count != SkDynamicExtent && Count <= Extent)
    constexpr SkSpan<T, Count> last() const {
        if constexpr (Extent == SkDynamicExtent) {
            (void)sk_collection_check_length(Count, this->size());
        }
        return SkSpan<T, Count>{this->fPtr + (this->size() - Count), Count};
    }

    template <size_t Offset, size_t Count = SkDynamicExtent>
        requires (Offset <= Extent &&
                  (Count == SkDynamicExtent || Extent == SkDynamicExtent ||
                   Count <= Extent - Offset))
    constexpr auto subspan() const {
        constexpr size_t kSubExtent =
                Count != SkDynamicExtent
                        ? Count
                        : (Extent != SkDynamicExtent ? Extent - Offset : SkDynamicExtent);
        if constexpr (Extent == SkDynamicExtent) {
            const size_t safeOffset = sk_collection_check_length(Offset, this->size());
            if constexpr (Count != SkDynamicExtent) {
                if (Count > this->size() - safeOffset) SK_UNLIKELY {
                    SkASSERT_RELEASE(Count <= this->size() - safeOffset);
                    SkUNREACHABLE;
                }
            }
            return SkSpan<T, kSubExtent>{
                    this->fPtr + safeOffset,
                    Count != SkDynamicExtent ? Count : this->size() - safeOffset};
        } else {
            return SkSpan<T, kSubExtent>{this->fPtr + Offset, kSubExtent};
        }
    }

    constexpr SkSpan<T> first(size_t prefixLen) const {
        return SkSpan<T>{this->fPtr, sk_collection_check_length(prefixLen, this->size())};
    }
    constexpr SkSpan<T> last(size_t postfixLen) const {
        return SkSpan<T>{this->fPtr + (this->size() - postfixLen),
                         sk_collection_check_length(postfixLen, this->size())};
    }
    constexpr SkSpan<T> subspan(size_t offset) const {
        return this->subspan(offset, this->size() - offset);
    }
    constexpr SkSpan<T> subspan(size_t offset, size_t count) const {
        const size_t safeOffset = sk_collection_check_length(offset, this->size());

        // Should read offset + count > size(), but that could overflow. We know that safeOffset
        // is <= size, therefore the subtraction will not overflow.
        if (count > this->size() - safeOffset) SK_UNLIKELY {
            // The count is too large.
            SkASSERT_RELEASE(count <= this->size() - safeOffset);
            SkUNREACHABLE;
        }
        return SkSpan<T>{this->fPtr + safeOffset, count};
    }

private:
    using INHERITED = sk_span_internal::SpanStorage<T, Extent>;
};

template <typename T, typename Integer>
    requires std::is_integral_v<Integer>
SkSpan(T*, Integer) -> SkSpan<T>;

template <typename T, size_t N>
SkSpan(T (&)[N]) -> SkSpan<T, N>;

template <typename T, size_t N>
SkSpan(const T (&)[N]) -> SkSpan<const T, N>;

template <typename Container>
SkSpan(Container&&) -> SkSpan<
        std::remove_pointer_t<decltype(std::data(std::declval<Container>()))>,
        sk_span_internal::kComputedExtent<Container>>;

#else

#include <span>

inline constexpr size_t SkDynamicExtent = std::dynamic_extent;

template <typename T, size_t Extent = SkDynamicExtent>
using SkSpan = std::span<T, Extent>;

#endif  // legacy

// These helper functions are temporary and only meant to be used during the Spanify process
// to ease the transition from pointers to spans (e.g. replacing `ptr++`).
// Once the Spanify process is complete, these callsites should be refactored to use
// more idiomatic span patterns, and these functions should be removed.
template <typename T>
constexpr SkSpan<T> SkPreIncrementSpan(SkSpan<T>& s) {
    s = s.subspan(1);
    return s;
}

template <typename T>
constexpr SkSpan<T> SkPostIncrementSpan(SkSpan<T>& s) {
    SkSpan<T> old = s;
    s = s.subspan(1);
    return old;
}

#endif  // SkSpan_DEFINED
