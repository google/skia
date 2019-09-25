/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkZip_DEFINED
#define SkZip_DEFINED

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "include/core/SkTypes.h"
#include "include/private/SkTLogic.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkSpan.h"

// Take a list of things that can be pointers, and use them all in parallel. The iterators and
// accessor operator[] for the class produce a tuple of the items.
template<typename... Ts>
class SkZip {
    using ReturnTuple = std::tuple<Ts&...>;

    class Iterator {
    public:
        constexpr Iterator(const SkZip* zip, size_t index) : fZip{zip}, fIndex{index} { }
        constexpr Iterator(const Iterator& that) : Iterator{ that.fZip, that.fIndex } { }
        constexpr Iterator& operator++() { ++fIndex; return *this; }
        constexpr Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        constexpr bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        constexpr bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        constexpr ReturnTuple operator*() { return (*fZip)[fIndex]; }

    private:
        const SkZip* const fZip = nullptr;
        size_t fIndex = 0;
    };

public:
    SkZip(size_t) = delete;
    constexpr SkZip(size_t size, Ts*... ts)
            : fPointers{ts...}
            , fSize{size} {}

    template<typename... Us>
    constexpr SkZip(const SkZip<Us...>& that)
            : fPointers{that.fPointers}
            , fSize{that.fSize} {}

    constexpr ReturnTuple operator[](size_t i) const {
        return this->index(i);
    }

    constexpr size_t size() const { return fSize; }
    constexpr bool empty() const { return this->size() == 0; }
    constexpr ReturnTuple front() const { return this->index(0); }
    constexpr ReturnTuple back() const { return this->index(this->size() - 1); }
    constexpr Iterator begin() const { return Iterator{this, 0}; }
    constexpr Iterator end() const { return Iterator{this, this->size()}; }
    template<size_t I> constexpr auto get() const {
        return SkMakeSpan(std::get<I>(fPointers), fSize);
    }

private:
    constexpr ReturnTuple index(size_t i) const {
        SkASSERT(this->size() > 0);
        SkASSERT(i < this->size());
        return indexDetail(i, skstd::make_index_sequence<sizeof...(Ts)>{});
    }

    template<std::size_t... Is>
    constexpr ReturnTuple indexDetail(size_t i, skstd::index_sequence<Is...>) const {
        return ReturnTuple((std::get<Is>(fPointers))[i]...);
    }

    std::tuple<Ts*...> fPointers;
    size_t fSize;
};

class SkMakeZipDetail {
    template<typename T> struct DecayPointer{
        using U = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        using type = typename std::conditional<std::is_pointer<U>::value, U, T>::type;
    };
    template<typename T> using DecayPointerT = typename DecayPointer<T>::type;

    template<typename C> struct ContiguousMemory { };
    template<typename T> struct ContiguousMemory<T*> {
        using value_type = T;
        static constexpr value_type* Data(T* t) { return t; }
        static constexpr size_t Size(T* s) { return SIZE_MAX; }
    };
    template<typename T, size_t N> struct ContiguousMemory<T(&)[N]> {
        using value_type = T;
        static constexpr value_type* Data(T(&t)[N]) { return t; }
        static constexpr size_t Size(T(&)[N]) { return N; }
    };
    template<typename C> struct ContiguousMemory<C&> {
        using value_type = typename std::remove_pointer<decltype(std::declval<C>().data())>::type;
        static constexpr value_type* Data(C& c) { return c.data(); }
        static constexpr size_t Size(C& c) { return c.size(); }
    };
    template<typename C> using Span = ContiguousMemory<DecayPointerT<C>>;
    template<typename C> using ValueType = typename Span<C>::value_type;

    template<typename C, typename... Ts> struct PickOneSize { };
    template <typename T, typename... Ts> struct PickOneSize<T*, Ts...> {
        static constexpr size_t Size(T* t, Ts... ts) {
            return PickOneSize<Ts...>::Size(std::forward<Ts>(ts)...);
        }
    };
    template <typename T, typename... Ts, size_t N> struct PickOneSize<T(&)[N], Ts...> {
        static constexpr size_t Size(T(&)[N], Ts...) { return N; }
    };
    template<typename C, typename... Ts> struct PickOneSize<C&, Ts...> {
        static constexpr size_t Size(C& c, Ts...) { return c.size(); }
    };

public:
    template<typename... Ts>
    static constexpr auto MakeZip(Ts&& ... ts) {

        // Pick the first collection that has a size, and use that for the size.
        size_t size = PickOneSize<DecayPointerT<Ts>...>::Size(std::forward<Ts>(ts)...);

#ifdef SK_DEBUG
        // Check that all sizes are the same.
        size_t minSize = SIZE_MAX;
        size_t maxSize = 0;
        for (size_t s : {Span<Ts>::Size(std::forward<Ts>(ts))...}) {
            if (s != SIZE_MAX) {
                minSize = SkTMin(minSize, s);
                maxSize = SkTMax(maxSize, s);
            }
        }
        SkASSERT(minSize == maxSize);
#endif

        return SkZip<ValueType<Ts>...>{size, Span<Ts>::Data(std::forward<Ts>(ts))...};
    }
};

template<typename... Ts>
inline constexpr auto SkMakeZip(Ts&& ... ts) {
    return SkMakeZipDetail::MakeZip(std::forward<Ts>(ts)...);
}
#endif //SkZip_DEFINED
