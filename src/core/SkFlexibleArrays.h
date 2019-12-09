/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlexibleArrays_DEFINED
#define SkFlexibleArrays_DEFINED

#include "src/core/SkEnumerate.h"
#include "src/core/SkZip.h"
#include "include/private/SkTLogic.h"

#include <array>
#include <new>

template <typename Base, typename... As>
class SkFlexibleArrays {
public:
    using ArraysT = SkZip<As...>;

    explicit SkFlexibleArrays(size_t n) : fN{n} { }

    template <typename... Args>
    static std::unique_ptr<Base> Make(size_t n, Args&&... args) {
        if (n > kMaxN) {
            return nullptr;
        }

        size_t sizeToAllocate = sizeof(Base) + TotalPadding() + n * CrossElementSize();
        void* memory = ::operator new (sizeToAllocate);

        return std::unique_ptr<Base>{new (memory) Base{n, std::forward<Args>(args)...}};
    }

    ArraysT arrays() {
        std::array<void*, sizeof...(As)> as;
        intptr_t ptr = (intptr_t)this;
        size_t previousAlignment = alignof(Base);
        ptr += sizeof(Base);
        for (auto [i, alignment] : SkMakeEnumerate(alignments)) {
            ptr += MaxPadding(previousAlignment, alignment);
            ptr &= ~(alignment - 1);
            as[i] = (void *)ptr;
            ptr += sizes[i] * fN;
        }
        return GenerateZip(fN, as, skstd::make_index_sequence<sizeof...(As)>{});
    }

    // Override operators to work well with deletes that take size. Only can be used by placement
    // new.
    void* operator new(size_t size) = delete;
    void operator delete(void* p) { ::operator delete(p); }

private:
    static constexpr std::array<size_t, sizeof...(As)> alignments{alignof(As)...};
    static constexpr std::array<size_t, sizeof...(As)> sizes{sizeof(As)...};

    static constexpr size_t MaxPadding(size_t previousAlignment, size_t alignment) {
        return (previousAlignment >= alignment) ? 0 : alignment - previousAlignment;
    }

    static constexpr size_t TotalPadding() {
        size_t padding = 0;
        size_t previousAlignment = alignof(Base);

        for (size_t i = 0; i < alignments.size(); i++) {
            padding += MaxPadding(previousAlignment, alignments[i]);
        }
        return padding;
    }

    static constexpr size_t CrossElementSize() {
        size_t totalSize = 0;
        for (size_t i = 0; i < sizes.size(); i++) {
            totalSize += sizes[i];
        }
        return totalSize;
    }

    template<std::size_t... Is>
    static ArraysT GenerateZip(
            size_t n, const std::array<void*, sizeof...(As)>& as, skstd::index_sequence<Is...>) {
        return ArraysT{n, (As*)as[Is]...};
    }

    static constexpr size_t kMaxN =
            (std::numeric_limits<intptr_t>::max() - TotalPadding() - sizeof(Base))
             / CrossElementSize();

    // Can only be built using Make().
    void* operator new(size_t, void* p) { return p; }

    // Number of elements in the arrays.
    const size_t fN;

};  // SkFlexibleArrays

#endif  // SkFlexibleArrays_DEFINED
