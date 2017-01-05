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

SkArenaAlloc::~SkArenaAlloc() {
    this->reset();
}

void SkArenaAlloc::reset() {
    char* releaser = fDtorCursor;
    while (releaser != nullptr) {
        releaser = this->callFooterAction(releaser);
    }
}

void SkArenaAlloc::ensureSpace(size_t size, size_t alignment) {
    constexpr size_t headerSize = sizeof(Footer) + sizeof(ptrdiff_t);
    auto objSizeAndOverhead = size + headerSize + sizeof(Footer);
    if (alignment > alignof(std::max_align_t)) {
        objSizeAndOverhead += alignment - 1;
    }

    auto allocationSize = std::max(objSizeAndOverhead, fExtraSize);

    // Round up to a nice size. If > 32K align to 4K boundary else up to max_align_t. The > 32K
    // heuristic is from the JEMalloc behavior.
    {
        size_t mask = allocationSize > (1 << 15) ? (1 << 12) - 1 : alignof(std::max_align_t) - 1;
        allocationSize = (allocationSize + mask) & ~mask;
    }

    char* newBlock = new char[allocationSize];

    auto previousDtor = fDtorCursor;
    fCursor = newBlock;
    fDtorCursor = newBlock;
    fEnd = fCursor + allocationSize;
    this->installIntFooter<NextBlock>(previousDtor - fCursor, 0);
}

char* SkArenaAlloc::callFooterAction(char* end) {
    Footer footer;
    memcpy(&footer, end - sizeof(Footer), sizeof(Footer));

    FooterAction releaser = (FooterAction)((char*)EndChain + (footer >> 5));
    ptrdiff_t padding = footer & 31;

    return releaser(end) - padding;
}
