/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFixedAlloc.h"

SkFixedAlloc::SkFixedAlloc(void* ptr, size_t len)
    : fStorage((char*)ptr), fCursor(fStorage), fEnd(fStorage + len) {}

void SkFixedAlloc::undo() {
    // This function is essentially make() in reverse.

    // First, read the Footer we stamped at the end.
    Footer footer;
    memcpy(&footer, fCursor - sizeof(Footer), sizeof(Footer));

    Releaser releaser = (Releaser)((char*)Base + (footer >> 5));
    ptrdiff_t padding = footer & 31;

    fCursor = releaser(fCursor);
    fCursor -= padding;
}

void SkFixedAlloc::reset() {
    while (fCursor > fStorage) {
        this->undo();
    }
}

void SkFixedAlloc::Base() { }

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
