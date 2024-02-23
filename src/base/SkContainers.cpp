// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/private/base/SkContainers.h"

#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"

#include <algorithm>
#include <cstddef>

namespace {
// Return at least as many bytes to keep malloc aligned.
constexpr size_t kMinBytes = alignof(max_align_t);

SkSpan<std::byte> complete_size(void* ptr, size_t size) {
    if (ptr == nullptr) {
        return {};
    }

    return {static_cast<std::byte*>(ptr), sk_malloc_size(ptr, size)};
}
}  // namespace

SkSpan<std::byte> SkContainerAllocator::allocate(int capacity, double growthFactor) {
    SkASSERT(capacity >= 0);
    SkASSERT(growthFactor >= 1.0);
    SkASSERT_RELEASE(capacity <= fMaxCapacity);

    if (growthFactor > 1.0 && capacity > 0) {
        capacity = this->growthFactorCapacity(capacity, growthFactor);
    }

    return sk_allocate_throw(capacity * fSizeOfT);
}

size_t SkContainerAllocator::roundUpCapacity(int64_t capacity) const {
    SkASSERT(capacity >= 0);

    // If round will not go above fMaxCapacity return rounded capacity.
    if (capacity < fMaxCapacity - kCapacityMultiple) {
        return SkAlignTo(capacity, kCapacityMultiple);
    }

    return SkToSizeT(fMaxCapacity);
}

size_t SkContainerAllocator::growthFactorCapacity(int capacity, double growthFactor) const {
    SkASSERT(capacity >= 0);
    SkASSERT(growthFactor >= 1.0);
    // Multiply by the growthFactor. Remember this must be done in 64-bit ints and not
    // size_t because size_t changes.
    const int64_t capacityGrowth = static_cast<int64_t>(capacity * growthFactor);

    // Notice that for small values of capacity, rounding up will provide most of the growth.
    return this->roundUpCapacity(capacityGrowth);
}


SkSpan<std::byte> sk_allocate_canfail(size_t size) {
    // Make sure to ask for at least the minimum number of bytes.
    const size_t adjustedSize = std::max(size, kMinBytes);
    void* ptr = sk_malloc_canfail(adjustedSize);
    return complete_size(ptr, adjustedSize);
}

SkSpan<std::byte> sk_allocate_throw(size_t size) {
    if (size == 0) {
        return {};
    }
    // Make sure to ask for at least the minimum number of bytes.
    const size_t adjustedSize = std::max(size, kMinBytes);
    void* ptr = sk_malloc_throw(adjustedSize);
    return complete_size(ptr, adjustedSize);
}

void sk_report_container_overflow_and_die() {
    SK_ABORT("Requested capacity is too large.");
}
