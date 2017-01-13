/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFixedAlloc.h"

#include <algorithm>

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

struct Skipper {
    char* operator()(char* objEnd, ptrdiff_t size) { return objEnd + size; }
};

struct NextBlock {
    char* operator()(char* objEnd, ptrdiff_t size) { delete [] objEnd; return objEnd + size; }
};

SkArenaAlloc::SkArenaAlloc(char* block, size_t size, size_t extraSize)
    : fDtorCursor{block}
    , fCursor    {block}
    , fEnd       {block + size}
    , fExtraSize {extraSize}
{
    if (size < sizeof(Footer)) {
        fEnd = fCursor = fDtorCursor = nullptr;
    }

    if (fCursor != nullptr) {
        this->installFooter(EndChain, 0);
    }
}

SkArenaAlloc::~SkArenaAlloc() {
    this->reset();
}

void SkArenaAlloc::reset() {
    Footer f;
    memmove(&f, fDtorCursor - sizeof(Footer), sizeof(Footer));
    char* releaser = fDtorCursor;
    while (releaser != nullptr) {
        releaser = this->callFooterAction(releaser);
    }
}

void SkArenaAlloc::installFooter(FooterAction* releaser, ptrdiff_t padding) {
    ptrdiff_t releaserDiff = (char *)releaser - (char *)EndChain;
    ptrdiff_t footerData = SkLeftShift((int64_t)releaserDiff, 5) | padding;
    if (padding >= 32 || !SkTFitsIn<int32_t>(footerData)) {
        // Footer data will not fit.
        SkFAIL("Constraints are busted.");
    }

    Footer footer = (Footer)(footerData);
    memmove(fCursor, &footer, sizeof(Footer));
    Footer check;
    memmove(&check, fCursor, sizeof(Footer));
    fCursor += sizeof(Footer);
    fDtorCursor = fCursor;
}

void SkArenaAlloc::ensureSpace(size_t size, size_t alignment) {
    constexpr size_t headerSize = sizeof(Footer) + sizeof(ptrdiff_t);
    // The chrome c++ library we use does not define std::max_align_t.
    // This must be conservative to add the right amount of extra memory to handle the alignment
    // padding.
    constexpr size_t alignof_max_align_t = 8;
    auto objSizeAndOverhead = size + headerSize + sizeof(Footer);
    if (alignment > alignof_max_align_t) {
        objSizeAndOverhead += alignment - 1;
    }

    auto allocationSize = std::max(objSizeAndOverhead, fExtraSize);

    // Round up to a nice size. If > 32K align to 4K boundary else up to max_align_t. The > 32K
    // heuristic is from the JEMalloc behavior.
    {
        size_t mask = allocationSize > (1 << 15) ? (1 << 12) - 1 : 32 - 1;
        allocationSize = (allocationSize + mask) & ~mask;
    }

    char* newBlock = new char[allocationSize];

    auto previousDtor = fDtorCursor;
    fCursor = newBlock;
    fDtorCursor = newBlock;
    fEnd = fCursor + allocationSize;
    this->installIntFooter<NextBlock>(previousDtor - fCursor, 0);
}

char* SkArenaAlloc::allocObject(size_t size, size_t alignment) {
    size_t mask = alignment - 1;
    char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
    if (objStart + size > fEnd) {
        this->ensureSpace(size, alignment);
        objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
    }
    return objStart;
}

// * sizeAndFooter - the memory for the footer in addition to the size for the object.
// * alignment - alignment needed by the object.
char* SkArenaAlloc::allocObjectWithFooter(size_t sizeIncludingFooter, size_t alignment) {
    size_t mask = alignment - 1;

    restart:
    size_t skipOverhead = 0;
    bool needsSkipFooter = fCursor != fDtorCursor;
    if (needsSkipFooter) {
        size_t skipSize = SkTFitsIn<int32_t>(fDtorCursor - fCursor)
                          ? sizeof(int32_t)
                          : sizeof(ptrdiff_t);
        skipOverhead = sizeof(Footer) + skipSize;
    }
    char* objStart = (char*)((uintptr_t)(fCursor + skipOverhead + mask) & ~mask);
    size_t totalSize = sizeIncludingFooter + skipOverhead;

    if (objStart + totalSize > fEnd) {
        this->ensureSpace(totalSize, alignment);
        goto restart;
    }

    SkASSERT(objStart + totalSize <= fEnd);

    // Install a skip footer if needed, thus terminating a run of POD data. The calling code is
    // responsible for installing the footer after the object.
    if (needsSkipFooter) {
        this->installIntFooter<Skipper>(fDtorCursor - fCursor, 0);
    }

    return objStart;
}

char* SkArenaAlloc::callFooterAction(char* end) {
    Footer footer;
    memcpy(&footer, end - sizeof(Footer), sizeof(Footer));

    FooterAction* releaser = (FooterAction*)((char*)EndChain + (footer >> 5));
    ptrdiff_t padding = footer & 31;

    char* r = releaser(end) - padding;

    return r;
}

char* SkArenaAlloc::EndChain(char*) { return nullptr; }

