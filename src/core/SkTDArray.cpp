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

SkTDStorage::SkTDStorage(const void* src, int count, int sizeOfT) : fSizeOfT{sizeOfT} {
    SkASSERT(src || count == 0);
    this->assign(src, count);
}

SkTDStorage::SkTDStorage(const SkTDStorage& that)
        : SkTDStorage{that.fStorage, that.fCount, that.fSizeOfT} {}

SkTDStorage& SkTDStorage::operator=(const SkTDStorage& that) {
    if (this != &that) {
        this->assign(that.fStorage, that.fCount);
    }
    return *this;
}

SkTDStorage::SkTDStorage(SkTDStorage&& that)
        : fSizeOfT{that.fSizeOfT}
        , fStorage(that.fStorage)
        , fReserve{that.fReserve}
        , fCount{that.fCount} {
    that.fStorage = nullptr;
}

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
    swap(fReserve, that.fReserve);
    swap(fCount, that.fCount);
}

void SkTDStorage::resize(int newCount) {
    SkASSERT(newCount >= 0);
    if (newCount > fReserve) {
        this->reserve(newCount);
    }
    fCount = newCount;
}

void SkTDStorage::reserve(int newReserve) {
    SkASSERT(newReserve >= 0);
    if (newReserve > fReserve) {
        // Establish the maximum number of elements that includes a valid count for end. In the
        // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
        static constexpr int kMaxCount = INT_MAX;

        // Assume that the array will max out.
        int expandedReserve = kMaxCount;
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
        size_t newStorageSize = this->bytes(fReserve);
        fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, newStorageSize));
    }
}

void SkTDStorage::shrink_to_fit() {
    if (fReserve != fCount) {
        fReserve = fCount;
        // Because calling realloc with size of 0 is implementation defined, force to a good state
        // by freeing fStorage.
        if (fReserve > 0) {
            fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, this->bytes(fReserve)));
        } else {
            sk_free(fStorage);
            fStorage = nullptr;
        }
    }
}

void SkTDStorage::erase(int index, int count) {
    SkASSERT(count >= 0);
    SkASSERT(fCount >= count);
    SkASSERT(0 <= index && index <= fCount);

    if (count > 0) {
        // Check that the resulting size fits in an int. This will abort if not.
        const int newCount = this->calculateSizeOrDie(-count);
        this->moveTail(index, index + count, fCount);
        this->resize(newCount);
    }
}

void SkTDStorage::removeShuffle(int index) {
    SkASSERT(fCount > 0);
    SkASSERT(0 <= index && index < fCount);
    // Check that the new count is valid.
    const int newCount = this->calculateSizeOrDie(-1);
    this->moveTail(index, fCount - 1, fCount);
    this->resize(newCount);
}

void SkTDStorage::assign(const void* src, int count) {
    fCount = count;
    if (fReserve < fCount) {
        // This looks like shrinking, but it allocates exactly fCount elements instead of
        // padding by adding the growth factor.
        this->shrink_to_fit();
    }
    if (count > 0) {
        this->copySrc(/*dstIndex=*/0, src, count);
    }
}

void* SkTDStorage::prepend() {
    return this->insert(/*index=*/0);
}

void* SkTDStorage::append() {
    return this->insert(fCount);
}

void* SkTDStorage::append(const void* src, int count) {
    return this->insert(fCount, count, src);
}

void* SkTDStorage::insert(int index) {
    return this->insert(index, /*count=*/1, nullptr);
}

void* SkTDStorage::insert(int index, int count, const void* src) {
    SkASSERT(0 <= index && index <= fCount);
    SkASSERT(count >= 0);

    if (count > 0) {
        const int oldCount = fCount;
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
    return a.size() == b.size() &&
           (a.size() == 0 || !memcmp(a.data(), b.data(), a.bytes(a.size())));
}

int SkTDStorage::calculateSizeOrDie(int delta) {
    // Check that count will not go negative.
    SkASSERT_RELEASE(-fCount <= delta);

    // We take care to avoid overflow here.
    // Because count and delta are both signed 32-bit ints, the sum of count and delta is at
    // most 4294967294, which fits fine in uint32_t. Proof follows in assert.
    static_assert(UINT32_MAX >= (uint32_t)INT_MAX + (uint32_t)INT_MAX);
    uint32_t testCount = (uint32_t)fCount + (uint32_t)delta;
    SkASSERT_RELEASE(SkTFitsIn<int>(testCount));
    return SkToInt(testCount);
}

void SkTDStorage::moveTail(int to, int tailStart, int tailEnd) {
    SkASSERT(0 <= to && to <= fCount);
    SkASSERT(0 <= tailStart && tailStart <= tailEnd && tailEnd <= fCount);
    if (to != tailStart && tailStart != tailEnd) {
        this->copySrc(to, this->address(tailStart), tailEnd - tailStart);
    }
}

void SkTDStorage::copySrc(int dstIndex, const void* src, int count) {
    SkASSERT(count > 0);
    memmove(this->address(dstIndex), src, this->bytes(count));
}
