/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTDArray.h"

SkMemoryBlock::SkMemoryBlock() = default;

void* SkMemoryBlock::growAndInsert(size_t elementSize, size_t maxCapacity,
                                   size_t n, size_t insertion) {
    SkASSERT(n > 0);
    SkASSERT(insertion <= fSize);

    auto toBytes = [elementSize](size_t n) { return n * elementSize; };
    auto toPtr = [toBytes](void* ptr, size_t i) {
        return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(ptr) + toBytes(i));
    };

    if (fCapacity - fSize < n) {

        if (maxCapacity - fCapacity < n) {
            SK_ABORT("Too large");
        }

        size_t minimumCapacity = fSize + n;
        size_t extraCapacity = minimumCapacity / 4 + 1;

        // If adding the extraCapacity is too big for maxCapacity just use maxCapacity.
        size_t targetCapacity = extraCapacity <= maxCapacity - minimumCapacity
                                ? minimumCapacity + extraCapacity
                                : maxCapacity;

        if (insertion == fSize) {
            // The multiply is safe because target capacity <= maxCapacity.
            fData = realloc(fData, toBytes(targetCapacity));
            if (fData == nullptr) {
                SK_ABORT("Can't reallocate more memory");
            }
        } else {
            void* newData = malloc(toBytes(targetCapacity));
            if (newData == nullptr) {
                SK_ABORT("Can't malloc more memory");
            }
            if (fData != nullptr) {
                // Move from the beginning to the insertion point to the new block.
                if (insertion > 0) {
                    memmove(newData, fData, toBytes(insertion));
                }

                // Move the tail to the new block. There is at least one tail; the realloc above
                // handles no tail elements.
                memmove(toPtr(newData, insertion + n),
                        toPtr(fData, insertion),
                        toBytes(fSize - insertion));
            }
            free(fData);
            fData = newData;
        }

        fCapacity = targetCapacity;
    }
    fSize += n;
    return toPtr(fData, insertion);
}

void SkMemoryBlock::shrinkToFit(size_t elementSize, size_t n) {
    if (n > 0) {
        fData = realloc(fData, n * elementSize);
        fSize = n;
        fCapacity = n;
    } else {
        free(fData);
        fData = nullptr;
        fSize = 0;
        fCapacity = 0;
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
