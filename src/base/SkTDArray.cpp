/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkTDArray.h"

#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <utility>

SkTDStorage::SkTDStorage(int sizeOfT) : fSizeOfT{sizeOfT} {}

SkTDStorage::SkTDStorage(const void* src, int size, int sizeOfT)
        : fSizeOfT{sizeOfT}
        , fCapacity{size}
        , fSize{size} {
    if (size > 0) {
        SkASSERT(src != nullptr);
        size_t storageSize = this->bytes(size);
        fStorage = static_cast<std::byte*>(sk_malloc_throw(storageSize));
        memcpy(fStorage, src, storageSize);
    }
}

SkTDStorage::SkTDStorage(const SkTDStorage& that)
        : SkTDStorage{that.fStorage, that.fSize, that.fSizeOfT} {}

SkTDStorage& SkTDStorage::operator=(const SkTDStorage& that) {
    if (this != &that) {
        if (that.fSize <= fCapacity) {
            fSize = that.fSize;
            if (fSize > 0) {
                memcpy(fStorage, that.data(), that.size_bytes());
            }
        } else {
            *this = SkTDStorage{that.data(), that.size(), that.fSizeOfT};
        }
    }
    return *this;
}

SkTDStorage::SkTDStorage(SkTDStorage&& that)
        : fSizeOfT{that.fSizeOfT}
        , fStorage(std::exchange(that.fStorage, nullptr))
        , fCapacity{that.fCapacity}
        , fSize{that.fSize} {}

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
    const int sizeOfT = fSizeOfT;
    this->~SkTDStorage();
    new (this) SkTDStorage{sizeOfT};
}

void SkTDStorage::swap(SkTDStorage& that) {
    SkASSERT(fSizeOfT == that.fSizeOfT);
    using std::swap;
    swap(fStorage, that.fStorage);
    swap(fCapacity, that.fCapacity);
    swap(fSize, that.fSize);
}

void SkTDStorage::resize(int newSize) {
    SkASSERT(newSize >= 0);
    if (newSize > fCapacity) {
        this->reserve(newSize);
    }
    fSize = newSize;
}

void SkTDStorage::reserve(int newCapacity) {
    SkASSERT(newCapacity >= 0);
    if (newCapacity > fCapacity) {
        // Establish the maximum number of elements that includes a valid count for end. In the
        // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
        static constexpr int kMaxCount = INT_MAX;

        // Assume that the array will max out.
        int expandedReserve = kMaxCount;
        if (kMaxCount - newCapacity > 4) {
            // Add 1/4 more than we need. Add 4 to ensure this grows by at least 1. Pin to
            // kMaxCount if no room for 1/4 growth.
            int growth = 4 + ((newCapacity + 4) >> 2);
            // Read this line as: if (count + growth < kMaxCount) { ... }
            // It's rewritten to avoid signed integer overflow.
            if (kMaxCount - newCapacity > growth) {
                expandedReserve = newCapacity + growth;
            }
        }


        // With a T size of 1, the above allocator produces the progression of 7, 15, ... Since,
        // the sizeof max_align_t is often 16, there is no reason to allocate anything less than
        // 16 bytes. This eliminates a realloc when pushing back bytes to an SkTDArray.
        if (fSizeOfT == 1) {
            // Round up to the multiple of 16.
            expandedReserve = (expandedReserve + 15) & ~15;
        }

        fCapacity = expandedReserve;
        size_t newStorageSize = this->bytes(fCapacity);
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, newStorageSize));
    }
}

void SkTDStorage::shrink_to_fit() {
    if (fCapacity != fSize) {
        fCapacity = fSize;
        // Because calling realloc with size of 0 is implementation defined, force to a good state
        // by freeing fStorage.
        if (fCapacity > 0) {
            fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, this->bytes(fCapacity)));
        } else {
            sk_free(fStorage);
            fStorage = nullptr;
        }
    }
}

void SkTDStorage::erase(int index, int count) {
    SkASSERT(count >= 0);
    SkASSERT(fSize >= count);
    SkASSERT(0 <= index && index <= fSize);

    if (count > 0) {
        // Check that the resulting size fits in an int. This will abort if not.
        const int newCount = this->calculateSizeOrDie(-count);
        this->moveTail(index, index + count, fSize);
        this->resize(newCount);
    }
}

void SkTDStorage::removeShuffle(int index) {
    SkASSERT(fSize > 0);
    SkASSERT(0 <= index && index < fSize);
    // Check that the new count is valid.
    const int newCount = this->calculateSizeOrDie(-1);
    this->moveTail(index, fSize - 1, fSize);
    this->resize(newCount);
}

void* SkTDStorage::prepend() {
    return this->insert(/*index=*/0);
}

void SkTDStorage::append() {
    if (fSize < fCapacity) {
        fSize++;
    } else {
        this->insert(fSize);
    }
}

void SkTDStorage::append(int count) {
    SkASSERT(count >= 0);
    // Read as: if (fSize + count <= fCapacity) {...}. This is a UB safe way to avoid the add.
    if (fCapacity - fSize >= count) {
        fSize += count;
    } else {
        this->insert(fSize, count, nullptr);
    }
}

void* SkTDStorage::append(const void* src, int count) {
    return this->insert(fSize, count, src);
}

void* SkTDStorage::insert(int index) {
    return this->insert(index, /*count=*/1, nullptr);
}

void* SkTDStorage::insert(int index, int count, const void* src) {
    SkASSERT(0 <= index && index <= fSize);
    SkASSERT(count >= 0);

    if (count > 0) {
        const int oldCount = fSize;
        const int newCount = this->calculateSizeOrDie(count);
        this->resize(newCount);
        this->moveTail(index + count, index, oldCount);

        if (src != nullptr) {
            this->copySrc(index, src, count);
        }
    }

    return this->address(index);
}

bool operator==(const SkTDStorage& a, const SkTDStorage& b) {
    return a.size() == b.size() && (a.empty() || !memcmp(a.data(), b.data(), a.bytes(a.size())));
}

int SkTDStorage::calculateSizeOrDie(int delta) {
    // Check that count will not go negative.
    SkASSERT_RELEASE(-fSize <= delta);

    // We take care to avoid overflow here.
    // Because count and delta are both signed 32-bit ints, the sum of count and delta is at
    // most 4294967294, which fits fine in uint32_t. Proof follows in assert.
    static_assert(UINT32_MAX >= (uint32_t)INT_MAX + (uint32_t)INT_MAX);
    uint32_t testCount = (uint32_t)fSize + (uint32_t)delta;
    SkASSERT_RELEASE(SkTFitsIn<int>(testCount));
    return SkToInt(testCount);
}

void SkTDStorage::moveTail(int to, int tailStart, int tailEnd) {
    SkASSERT(0 <= to && to <= fSize);
    SkASSERT(0 <= tailStart && tailStart <= tailEnd && tailEnd <= fSize);
    if (to != tailStart && tailStart != tailEnd) {
        this->copySrc(to, this->address(tailStart), tailEnd - tailStart);
    }
}

void SkTDStorage::copySrc(int dstIndex, const void* src, int count) {
    SkASSERT(count > 0);
    memmove(this->address(dstIndex), src, this->bytes(count));
}
