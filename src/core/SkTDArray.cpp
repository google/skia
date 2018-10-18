/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTDArray.h"

SkMemoryBlock::SkMemoryBlock() = default;
SkMemoryBlock::SkMemoryBlock(size_t bytes, const void* data) {
    this->allocate(bytes);
    if (data != nullptr) {
        memcpy(fData, data, bytes);
    }
}

void SkMemoryBlock::grow(size_t n, size_t insertion, size_t tailEnd) {
    SkASSERT(n > 0);
    SkASSERT(insertion <= tailEnd);
    if (insertion == tailEnd) {
        fData = realloc(fData, n);
        if (fData == nullptr) {
            SK_ABORT("Can't allocate more memory");
        }
    } else {
        uint8_t* newData = (uint8_t*)malloc(n);
        if (newData == nullptr) {
            SK_ABORT("Can't allocate more memory");
        }
        uint8_t* oldData = (uint8_t*)fData;
        if (oldData != nullptr) {
            if (insertion > 0) {
                memmove(newData, oldData, insertion);
            }
            memmove(newData + insertion + n, oldData + insertion, tailEnd - insertion);
        }
    }
}

void SkMemoryBlock::shrinkToFit(size_t n) {
    if (n > 0) {
        fData = realloc(fData, n);
    } else {
        free(fData);
        fData = nullptr;
    }
}

#if 0
SkMemoryBlock::SkMemoryBlock(size_t reserveSize) {
    this->moveTail(0, reserveSize, 0);
}

void* malloc_if_not_zero(size_t s) {
    return s > 0 ? sk_malloc_throw(s) : nullptr;
}

SkMemoryBlock::SkMemoryBlock(const SkMemoryBlock& src, size_t n)
        : fBlock{(uint8_t*)malloc_if_not_zero(src.fSize)}
        , fSize{src.fSize} {
    if (src.fBlock != nullptr) {
        this->copyTo(0, src.fBlock, n);
    }
}

SkMemoryBlock::SkMemoryBlock(SkMemoryBlock&& src)
        : fBlock{src.fBlock}
        , fSize{src.fSize} {
    new (&src) SkMemoryBlock{};
}

SkMemoryBlock::~SkMemoryBlock() {
    sk_free(fBlock);
}

uint8_t* SkMemoryBlock::copyTo(size_t to, const void* from, size_t n) {
    SkASSERT(from != nullptr);
    return (uint8_t*) memmove(fBlock + to, from, n);
}

uint8_t* SkMemoryBlock::copyFrom(void* to, size_t from, size_t n) {
    return (uint8_t*)memmove(to, fBlock + from, n);
}

uint8_t* SkMemoryBlock::insert(size_t i, size_t e, size_t n) {
    SkASSERT_RELEASE(i <= i + n);
    SkASSERT(e <= fSize);
    SkASSERT(i <= e);
    SkASSERT(n > 0);

    // Check that there is room and no overflow.
    if (e + n < fSize) {
        if (i < e) {
            this->copyTo(i + n, fBlock + i, e - i);
        }
        return fBlock + i;
    }

    return this->moveTail(i, i + n, e - i);
}

uint8_t* SkMemoryBlock::remove(size_t r, size_t e, size_t n) {
    SkASSERT_RELEASE(r <= r + n);
    SkASSERT(e <= fSize);
    SkASSERT(r <= e);
    SkASSERT(r + n <= e);

    return this->copyTo(r, fBlock + r + n, e - (r + n));
}

uint8_t* SkMemoryBlock::moveTail(size_t from, size_t to, size_t n) {
    if (from < to) {
        // Insert case - a hole is created on the range [from, to).

        size_t atLeastSize = to + n;

        // If overflow just crash.
        SkASSERT_RELEASE(to <= to + n);
        size_t newSize = atLeastSize + atLeastSize / 4 + 1;

        uint8_t* newBlock;
        if (n == 0) {
            newBlock = (uint8_t*)sk_realloc_throw(fBlock, newSize);
        } else {
            newBlock = (uint8_t*)sk_malloc_throw(newSize);

            if (fBlock != nullptr) {
                // Move bytes before the hole.
                this->copyFrom(newBlock, 0, from);

                // Move bytes after the hole.
                this->copyFrom(newBlock + to, from, n);
            }
        }
        fBlock = newBlock;
        fSize = newSize;

        // Return the hole.
        return fBlock + from;
    }

    // from and to are the same, so return one of them.
    return fBlock + to;
}

void* SkMemoryBlock::shrinkToFit(size_t n) {
    if (n == 0) {
        sk_free(fBlock);
        fBlock = nullptr;
        fSize = 0;
    } else {
        fSize = n;
        fBlock = (uint8_t*)sk_realloc_throw(fBlock, fSize);
    }

    return fBlock;
}

void SkMemoryBlock::swap(SkMemoryBlock& that) {
    using std::swap;
    swap(fBlock, that.fBlock);
    swap(fSize, that.fSize);
}

size_t SkMemoryBlock::SafeSize(size_t Tsize, size_t n) {
    return SkSafeMath::Mul(Tsize, n);
}

uint8_t* SkMemoryBlock::appendHelper(size_t e, size_t n) {
    SkASSERT_RELEASE(e <= e + n);
    return this->moveTail(e, e + n, 0);
}

#endif
