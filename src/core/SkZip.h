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
        Iterator(const SkZip* zip, size_t index) : fZip{zip}, fIndex{index} { }
        Iterator(const Iterator& that) : Iterator{ that.fZip, that.fIndex } { }
        Iterator& operator++() { ++fIndex; return *this; }
        Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
        bool operator==(const Iterator& rhs) const { return fIndex == rhs.fIndex; }
        bool operator!=(const Iterator& rhs) const { return fIndex != rhs.fIndex; }
        ReturnTuple operator*() { return (*fZip)[fIndex]; }

    private:
        const SkZip* const fZip = nullptr;
        size_t fIndex = 0;
    };

public:
    SkZip(size_t) = delete;
    explicit SkZip(size_t size, Ts*... ts)
            : fPointers{ts...}
            , fSize{size} {}

    template<typename... Us>
    SkZip(const SkZip<Us...>& that)
            : fPointers{that.fPointers}
            , fSize{that.fSize} {}

    ReturnTuple operator[](size_t i) const {
        return this->index(i);
    }

    size_t size() const { return fSize; }
    bool empty() const { return this->size() == 0; }
    ReturnTuple front() const { return this->index(0); }
    ReturnTuple back() const { return this->index(this->size() - 1); }
    Iterator begin() const { return Iterator{this, 0}; }
    Iterator end() const { return Iterator{this, this->size()}; }
    template<size_t I> auto get() const { return SkMakeSpan(std::get<I>(fPointers), fSize); }
    template<typename T> auto get() const { return SkMakeSpan(std::get<T*>(fPointers), fSize); }

private:
    ReturnTuple index(size_t i) const {
        SkASSERT(this->size() > 0);
        SkASSERT(i < this->size());
        return indexDetail(i, skstd::make_index_sequence<sizeof...(Ts)>{});
    }

    template<std::size_t... Is>
    ReturnTuple indexDetail(size_t i, skstd::index_sequence<Is...>) const {
        return ReturnTuple((std::get<Is>(fPointers))[i]...);
    }

    std::tuple<Ts*...> fPointers;
    size_t fSize;
};
#endif //SkZip_DEFINED
