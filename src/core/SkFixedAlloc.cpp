/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFixedAlloc.h"

SkFixedAlloc::SkFixedAlloc(void* ptr, size_t len)
    : fBuffer((char*)ptr), fUsed(0), fLimit(len) {}

void SkFixedAlloc::undo() {
    uint32_t skip_and_size;
    memcpy(&skip_and_size, fBuffer + fUsed - 4, 4);
    fUsed -= skip_and_size + 4;
}

void SkFixedAlloc::reset() {
    fUsed = 0;
}

void* SkFixedAlloc::alloc(size_t size, size_t align) {
    auto aligned = ((uintptr_t)(fBuffer+fUsed) + align-1) & ~(align-1);
    size_t skip = aligned - (uintptr_t)(fBuffer+fUsed);

    if (!SkTFitsIn<uint32_t>(skip + size) ||
        fUsed + skip + size + 4 > fLimit) {
        return nullptr;
    }

    // Skip ahead until aligned.
    fUsed += skip;

    // Allocate size bytes.
    void* ptr = (fBuffer+fUsed);
    fUsed += size;

    // Stamp a footer that we can use to clean up.
    uint32_t skip_and_size = SkToU32(skip + size);
    memcpy(fBuffer+fUsed, &skip_and_size, 4);
    fUsed += 4;

    return ptr;
}


SkFallbackAlloc::SkFallbackAlloc(SkFixedAlloc* fixed) : fFixedAlloc(fixed) {}

void SkFallbackAlloc::undo() {
    if (fHeapAllocs.empty()) {
        return fFixedAlloc->undo();
    }
    sk_free(fHeapAllocs.back());
    fHeapAllocs.pop_back();
}

void SkFallbackAlloc::reset() {
    while (!fHeapAllocs.empty()) {
        this->undo();
    }
    fFixedAlloc->reset();
}
