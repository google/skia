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
    // This function is essentially make() in reverse.

    // First, read the Footer we stamped at the end.
    Footer footer;
    memcpy(&footer, fBuffer + fUsed - sizeof(Footer), sizeof(Footer));

    // That tells us where the T starts and how to destroy it.
    footer.dtor(fBuffer + fUsed - sizeof(Footer) - footer.len);

    // We can reuse bytes that stored the Footer, the T, and any that we skipped for alignment.
    fUsed -= sizeof(Footer) + footer.len + footer.skip;
}

void SkFixedAlloc::reset() {
    while (fUsed) {
        this->undo();
    }
}


SkFallbackAlloc::SkFallbackAlloc(SkFixedAlloc* fixed) : fFixedAlloc(fixed) {}

void SkFallbackAlloc::undo() {
    if (fHeapAllocs.empty()) {
        return fFixedAlloc->undo();
    }
    HeapAlloc alloc = fHeapAllocs.back();
    alloc.deleter(alloc.ptr);
    fHeapAllocs.pop_back();
}

void SkFallbackAlloc::reset() {
    while (!fHeapAllocs.empty()) {
        this->undo();
    }
    fFixedAlloc->reset();
}
