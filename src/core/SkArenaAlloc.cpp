/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <cstddef>
#include "SkArenaAlloc.h"

static char* end_chain(char*) { return nullptr; }

char* SkArenaAlloc::SkipPod(char* footerEnd) {
    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(int32_t));
    int32_t skip;
    memmove(&skip, objEnd, sizeof(int32_t));
    return objEnd - skip;
}

void SkArenaAlloc::RunDtorsOnBlock(char* footerEnd) {
    while (footerEnd != nullptr) {
        Footer footer;
        memcpy(&footer, footerEnd - sizeof(Footer), sizeof(Footer));

        FooterAction* action = (FooterAction*)((char*)end_chain + (footer >> 5));
        ptrdiff_t padding = footer & 31;

        footerEnd = action(footerEnd) - padding;
    }
}

char* SkArenaAlloc::NextBlock(char* footerEnd) {
    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(char*));
    char* next;
    memmove(&next, objEnd, sizeof(char*));
    RunDtorsOnBlock(next);
    delete [] objEnd;
    return nullptr;
}

SkArenaAlloc::SkArenaAlloc(char* block, size_t size, size_t extraSize)
    : fDtorCursor {block}
    , fCursor     {block}
    , fEnd        {block + size}
    , fFirstBlock {block}
    , fFirstSize  {size}
    , fExtraSize  {extraSize}
{
    if (size < sizeof(Footer)) {
        fEnd = fCursor = fDtorCursor = nullptr;
    }

    if (fCursor != nullptr) {
        this->installFooter(end_chain, 0);
    }
}

SkArenaAlloc::~SkArenaAlloc() {
    RunDtorsOnBlock(fDtorCursor);
}

void SkArenaAlloc::reset() {
    this->~SkArenaAlloc();
    new (this) SkArenaAlloc{fFirstBlock, fFirstSize, fExtraSize};
}

void SkArenaAlloc::installFooter(FooterAction* releaser, ptrdiff_t padding) {
    ptrdiff_t releaserDiff = (char *)releaser - (char *)end_chain;
    ptrdiff_t footerData = SkLeftShift((int64_t)releaserDiff, 5) | padding;
    if (padding >= 32 || !SkTFitsIn<int32_t>(footerData)) {
        // Footer data will not fit.
        SkFAIL("Constraints are busted.");
    }

    Footer footer = (Footer)(footerData);
    memmove(fCursor, &footer, sizeof(Footer));
    fCursor += sizeof(Footer);
    fDtorCursor = fCursor;
}

void SkArenaAlloc::installPtrFooter(FooterAction* action, char* ptr, ptrdiff_t padding) {
    memmove(fCursor, &ptr, sizeof(char*));
    fCursor += sizeof(char*);
    this->installFooter(action, padding);
}

void SkArenaAlloc::installUint32Footer(FooterAction* action, uint32_t value, ptrdiff_t padding) {
    memmove(fCursor, &value, sizeof(uint32_t));
    fCursor += sizeof(uint32_t);
    this->installFooter(action, padding);
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
        size_t mask = allocationSize > (1 << 15) ? (1 << 12) - 1 : 16 - 1;
        allocationSize = (allocationSize + mask) & ~mask;
    }

    char* newBlock = new char[allocationSize];

    auto previousDtor = fDtorCursor;
    fCursor = newBlock;
    fDtorCursor = newBlock;
    fEnd = fCursor + allocationSize;
    this->installPtrFooter(NextBlock, previousDtor, 0);
}

char* SkArenaAlloc::allocObject(size_t size, size_t alignment) {
    size_t mask = alignment - 1;
    char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
    if ((ptrdiff_t)size > fEnd - objStart) {
        this->ensureSpace(size, alignment);
        objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
    }
    return objStart;
}

char* SkArenaAlloc::allocObjectWithFooter(size_t sizeIncludingFooter, size_t alignment) {
    size_t mask = alignment - 1;

restart:
    size_t skipOverhead = 0;
    bool needsSkipFooter = fCursor != fDtorCursor;
    if (needsSkipFooter) {
        skipOverhead = sizeof(Footer) + sizeof(uint32_t);
    }
    char* objStart = (char*)((uintptr_t)(fCursor + skipOverhead + mask) & ~mask);
    size_t totalSize = sizeIncludingFooter + skipOverhead;

    if ((ptrdiff_t)totalSize > fEnd - objStart) {
        this->ensureSpace(totalSize, alignment);
        goto restart;
    }

    SkASSERT((ptrdiff_t)totalSize <= fEnd - objStart);

    // Install a skip footer if needed, thus terminating a run of POD data. The calling code is
    // responsible for installing the footer after the object.
    if (needsSkipFooter) {
        this->installUint32Footer(SkipPod, SkTo<uint32_t>(fCursor - fDtorCursor), 0);
    }

    return objStart;
}

