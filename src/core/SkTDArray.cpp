/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafeMath.h"
#include "SkTDArray.h"

SkMemoryBlock::SkMemoryBlock(size_t Tsize, size_t n) {
    this->moveTail(Tsize, 0, n, 0);
}

SkMemoryBlock::~SkMemoryBlock() {
    sk_free(fBlock);
}

void* SkMemoryBlock::moveTail(size_t Tsize, size_t from, size_t to, size_t n) {
    if (from < to) {
        // Insert case - a hole is created on the range [from, to).

        size_t growthCount = (to + n) / 4 + 1;
        size_t newSize = SkSafeMath::Mul(growthCount, Tsize);

        uint8_t* newBlock;
        if (n == 0) {
            newBlock = (uint8_t*)sk_realloc_throw(fBlock, newSize);
        } else {
            newBlock = (uint8_t*)sk_malloc_throw(newSize);

            if (fBlock != nullptr) {
                // Move bytes before the hole.
                memmove(newBlock, fBlock, from * Tsize);

                // Move bytes after the hole.
                memmove(&newBlock[to * Tsize], &fBlock[from * Tsize], n * Tsize);
            }
        }
        fBlock = newBlock;
        fSize = newSize;

        // Return the hole.
        return &fBlock[from * Tsize];
    } else if (from > to) {
        // Remove case - the range [to, from) is removed.

        // Move the tail into the deleted bytes.
        memmove(&fBlock[to * Tsize], &fBlock[from * Tsize], n * Tsize);

        // Return start of tail
        return &fBlock[to * Tsize];
    }

    // from and to are the same, so return one of them.
    return &fBlock[from * Tsize];
}

void* SkMemoryBlock::shrinkToFit(size_t Tsize, size_t n) {
    if (n == 0) {
        sk_free(fBlock);
        fBlock = nullptr;
        fSize = 0;
    } else {
        fSize = SkSafeMath::Mul(n, Tsize);
        fBlock = (uint8_t*)sk_realloc_throw(fBlock, fSize);
    }

    return fBlock;
}

void SkMemoryBlock::swap(SkMemoryBlock& that) {
    using std::swap;
    swap(fBlock, that.fBlock);
    swap(fSize, that.fSize);
}

size_t SkMemoryBlock::capacity(size_t Tsize) const {
    return fSize / Tsize;
}

bool SkMemoryBlock::containsAddr(void* addr) const {
    auto a = (uint8_t*)addr;
    return !this->empty() && fBlock <= a && a < fBlock + fSize;
}
