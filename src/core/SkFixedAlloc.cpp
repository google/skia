/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFixedAlloc.h"

static char* align_ptr(void* ptr) {
    return (char*)(((uintptr_t)ptr + 7) & ~7);
}

static size_t align_limit(void* vptr, size_t len) {
    char* ptr = (char*)vptr;
    return (ptr + len) - align_ptr(vptr);
}

SkFixedAlloc::SkFixedAlloc(void* ptr, size_t len)
    : fBuffer(align_ptr(ptr)), fUsed(0), fLimit(align_limit(ptr, len)) {}

void SkFixedAlloc::undo() {
    // This function is essentially make() in reverse.

    // First, read the Footer we stamped at the end.
    Footer footer;
    memcpy(&footer, fBuffer + fUsed - sizeof(Footer), sizeof(Footer));

    // That tells us where the T starts, how to destroy it and returns the size used.
    fUsed -= footer.dtor(fBuffer + fUsed);
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
