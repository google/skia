/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkMalloc.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"

#include <utility>

namespace {
size_t offset(int n, size_t sizeOfT) {
    return SkToSizeT(n) * sizeOfT;
}

size_t mem_size(int n, size_t sizeOfT) {
    return SkToSizeT(n) * sizeOfT;
}
}  // namespace

SkTDStorage::SkTDStorage(SkTDStorage&& that)
        : fStorage{std::move(std::exchange(that.fStorage, nullptr))}
        , fReserve{std::exchange(that.fReserve, 0)}
        , fCount{std::exchange(that.fCount, 0)} {}

SkTDStorage& SkTDStorage::operator=(SkTDStorage&& that) {
    if (this != &that) {
        this->~SkTDStorage();
        new (this) SkTDStorage{std::move(that)};
    }
    return *this;
}

SkTDStorage::~SkTDStorage() {
    sk_free(fStorage);
}

void SkTDStorage::reset() {
    this->~SkTDStorage();
    new (this) SkTDStorage{};
}

void SkTDStorage::assign(const void* src, int count, size_t sizeOfT) {
    SkASSERT(count >= 0);
    fCount = count;
    this->shrinkToFit(sizeOfT);
    if (count > 0 && src != nullptr) {
        memcpy(fStorage, src, this->size_bytes(sizeOfT));
    }
}

void SkTDStorage::resize(int newCount, size_t sizeOfT) {
    SkASSERT(newCount >= 0);
    if (fReserve < newCount) {
        this->reserve(newCount, sizeOfT);
    }
    fCount = newCount;
}

size_t SkTDStorage::size_bytes(size_t sizeOfT) const {
    return offset(fCount, sizeOfT);
}

void SkTDStorage::reserve(size_t reserveSize, size_t sizeOfT) {
    // Note: this takes a size_t to centralize size checking.
    SkASSERT_RELEASE(SkTFitsIn<int>(reserveSize));

    if (SkToInt(reserveSize) > fReserve) {
        // Establish the maximum number of elements that includes a valid count for end. In the
        // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
        static constexpr int kMaxCount = INT_MAX;

        // Assume that the array will max out.
        int expandedReserve = kMaxCount;
        int newReserve = SkToInt(reserveSize);
        if (kMaxCount - newReserve > 4) {
            // Add 1/4 more than we need. Add 4 to ensure this grows by at least 1. Pin to
            // kMaxCount if no room for 1/4 growth.
            int growth = 4 + ((newReserve + 4) >> 2);
            // Read this line as: if (count + growth < kMaxCount) { ... }
            // It's rewritten to avoid signed integer overflow.
            if (kMaxCount - newReserve > growth) {
                expandedReserve = newReserve + growth;
            }
        }

        fReserve = expandedReserve;
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, mem_size(fReserve, sizeOfT)));
    }
}

void SkTDStorage::shrinkToFit(size_t sizeOfT) {
    // Because calling realloc with size of 0 is implementation defined, force to a good state by
    // freeing fStorage and setting reserve to 0.
    if (fCount != 0) {
        fReserve = fCount;
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, mem_size(fReserve, sizeOfT)));
    } else {
        fReserve = 0;
        sk_free(fStorage);
        fStorage = nullptr;
    }
}

void* SkTDStorage::erase(int index, int count, size_t sizeOfT) {
    SkASSERT(index >= 0);
    SkASSERT(count >= 0);
    SkASSERT(index + count <= fCount);

    // Check that the resulting size fits in an int. This will abort if not.
    const int newCount = this->calculateSizeDeltaOrDie(-count);

    const int tailBegin = index + count;
    const size_t tailBeginOffset = offset(tailBegin, sizeOfT);
    if (count > 0) {
        // Move the tail down if there is one.
        if (tailBegin < fCount) {
            const size_t headEndOffset = offset(index, sizeOfT);
            const size_t gapSize = this->size_bytes(sizeOfT) - tailBeginOffset;
            memmove(fStorage + headEndOffset, fStorage + tailBeginOffset, gapSize);
        }
        this->resize(newCount, sizeOfT);
    }

    return fStorage + tailBeginOffset;
}

void* SkTDStorage::removeShuffle(int index, size_t sizeOfT) {
    SkASSERT(0 <= index && index < fCount);
    // Check that the new count is valid.
    int newCount = this->calculateSizeDeltaOrDie(-1);

    const size_t indexOffset = offset(index, sizeOfT);
    const size_t lastElementOffset = offset(newCount, sizeOfT);

    // Fill the index if not the last element.
    if (index < newCount) {
        memmove(fStorage + indexOffset, fStorage + lastElementOffset, sizeOfT);
    }
    this->resize(newCount, sizeOfT);
    return fStorage + lastElementOffset;
}

void* SkTDStorage::prepend(size_t sizeOfT) {
    return this->insert(/*index=*/0, sizeOfT);
}

void* SkTDStorage::append(size_t sizeOfT) {
    return this->insert(fCount, sizeOfT);
}

void* SkTDStorage::append(const void* src, int count, size_t sizeOfT) {
    return this->insert(fCount, src, count, sizeOfT);
}

void* SkTDStorage::insert(int index, size_t sizeOfT) {
    return this->insert(index, nullptr, /*count=*/1, sizeOfT);
}

void* SkTDStorage::insert(int index, const void* src, int count, size_t sizeOfT) {
    SkASSERT(0 <= index && index <= fCount);
    SkASSERT(count >= 0);
    const size_t indexOffset = offset(index, sizeOfT);

    if (count > 0) {
        const int oldCount = fCount;

        size_t oldCountOffset = this->size_bytes(sizeOfT);

        const int newCount = this->calculateSizeDeltaOrDie(count);
        this->resize(newCount, sizeOfT);

        // Shift memory to make space.
        if (index < oldCount) {
            // Safe because index <= oldCount and oldCount + count is safe.
            size_t shiftOffset = offset(index + count, sizeOfT);

            // Move the tail of data from index to oldCount.
            memmove(fStorage + shiftOffset, fStorage + indexOffset, oldCountOffset - indexOffset);
        }

        if (src != nullptr) {
            memcpy(fStorage + indexOffset, src, mem_size(count, sizeOfT));
        }
    }

    return fStorage + indexOffset;
}

int SkTDStorage::calculateSizeDeltaOrDie(int delta) const {
    // Check that count will not go negative.
    SkASSERT_RELEASE(-fCount <= delta);

    // We take care to avoid overflow here.
    // Because fCount and delta are both signed 32-bit ints, the sum of fCount and delta is at
    // most 4294967294, which fits fine in uint32_t. Proof follows in assert.
    static_assert(UINT32_MAX >= (uint32_t)INT_MAX + (uint32_t)INT_MAX);
    uint32_t count = (uint32_t)this->size() + (uint32_t)delta;
    SkASSERT_RELEASE(SkTFitsIn<int>(count));
    return SkToInt(count);
}

