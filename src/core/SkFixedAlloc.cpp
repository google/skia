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
    uint32_t skip_and_len;
    memcpy(&skip_and_len, fBuffer + fUsed - 4, 4);
    fUsed -= skip_and_len + 4;
}

void SkFixedAlloc::reset() {
    fUsed = 0;
}

void* SkFixedAlloc::alloc(size_t align, size_t len) {
    auto aligned = ((uintptr_t)(fBuffer+fUsed) + align-1) & ~(align-1);
    size_t skip = aligned - (uintptr_t)(fBuffer+fUsed);

    if (!SkTFitsIn<uint32_t>(skip + len) ||
        fUsed + skip + len + 4 > fLimit) {
        return nullptr;
    }

    // Skip ahead until aligned.
    fUsed += skip;

    // Allocate len bytes.
    void* ptr = (fBuffer+fUsed);
    fUsed += len;

    // Stamp a footer that we can use to clean up.
    uint32_t skip_and_len = SkToU32(skip + len);
    memcpy(fBuffer+fUsed, &skip_and_len, 4);
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
