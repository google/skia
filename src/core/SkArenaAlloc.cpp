/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include <algorithm>
#include <new>

static char* end_chain(char*) { return nullptr; }

static uint32_t first_allocated_block(uint32_t blockSize, uint32_t firstHeapAllocation) {
    return firstHeapAllocation > 0 ? firstHeapAllocation :
           blockSize           > 0 ? blockSize           : 1024;
}

SkArenaAlloc::SkArenaAlloc(char* block, size_t size, size_t firstHeapAllocation)
    : fDtorCursor {block}
    , fCursor     {block}
    , fEnd        {block + ToU32(size)}
    , fNextHeapAlloc{first_allocated_block(ToU32(size), ToU32(firstHeapAllocation))}
    , fYetNextHeapAlloc{fNextHeapAlloc}
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

void SkArenaAlloc::installFooter(FooterAction* action, uint32_t padding) {
    assert(padding < 64);
    int64_t actionInt = (int64_t)(intptr_t)action;

    // The top 14 bits should be either all 0s or all 1s. Check this.
    assert((actionInt << 6) >> 6 == actionInt);
    Footer encodedFooter = (actionInt << 6) | padding;
    memmove(fCursor, &encodedFooter, sizeof(Footer));
    fCursor += sizeof(Footer);
    fDtorCursor = fCursor;
}

void SkArenaAlloc::installPtrFooter(FooterAction* action, char* ptr, uint32_t padding) {
    memmove(fCursor, &ptr, sizeof(char*));
    fCursor += sizeof(char*);
    this->installFooter(action, padding);
}

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

        FooterAction* action = (FooterAction*)(footer >> 6);
        ptrdiff_t padding = footer & 63;

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

void SkArenaAlloc::installUint32Footer(FooterAction* action, uint32_t value, uint32_t padding) {
    memmove(fCursor, &value, sizeof(uint32_t));
    fCursor += sizeof(uint32_t);
    this->installFooter(action, padding);
}

void SkArenaAlloc::ensureSpace(uint32_t size, uint32_t alignment) {
    constexpr uint32_t headerSize = sizeof(Footer) + sizeof(ptrdiff_t);
    // The chrome c++ library we use does not define std::max_align_t.
    // This must be conservative to add the right amount of extra memory to handle the alignment
    // padding.
    constexpr uint32_t alignof_max_align_t = 8;
    constexpr uint32_t maxSize = std::numeric_limits<uint32_t>::max();
    constexpr uint32_t overhead = headerSize + sizeof(Footer);
    AssertRelease(size <= maxSize - overhead);
    uint32_t objSizeAndOverhead = size + overhead;
    if (alignment > alignof_max_align_t) {
        uint32_t alignmentOverhead = alignment - 1;
        AssertRelease(objSizeAndOverhead <= maxSize - alignmentOverhead);
        objSizeAndOverhead += alignmentOverhead;
    }

    uint32_t minAllocationSize = fNextHeapAlloc;

    // Calculate the next heap alloc that won't overflow.
    if (fYetNextHeapAlloc <= maxSize - fNextHeapAlloc) {
        fNextHeapAlloc += fYetNextHeapAlloc;
        std::swap(fNextHeapAlloc, fYetNextHeapAlloc);
    } else {
        fNextHeapAlloc = maxSize;
    }
    uint32_t allocationSize = std::max(objSizeAndOverhead, minAllocationSize);

    // Round up to a nice size. If > 32K align to 4K boundary else up to max_align_t. The > 32K
    // heuristic is from the JEMalloc behavior.
    {
        uint32_t mask = allocationSize > (1 << 15) ? (1 << 12) - 1 : 16 - 1;
        AssertRelease(allocationSize <= maxSize - mask);
        allocationSize = (allocationSize + mask) & ~mask;
    }

    char* newBlock = new char[allocationSize];

    auto previousDtor = fDtorCursor;
    fCursor = newBlock;
    fDtorCursor = newBlock;
    fEnd = fCursor + allocationSize;
    this->installPtrFooter(NextBlock, previousDtor, 0);
}

char* SkArenaAlloc::allocObjectWithFooter(uint32_t sizeIncludingFooter, uint32_t alignment) {
    uintptr_t mask = alignment - 1;

restart:
    uint32_t skipOverhead = 0;
    const bool needsSkipFooter = fCursor != fDtorCursor;
    if (needsSkipFooter) {
        skipOverhead = sizeof(Footer) + sizeof(uint32_t);
    }
    const uint32_t totalSize = sizeIncludingFooter + skipOverhead;

    // Math on null fCursor/fEnd is undefined behavior, so explicitly check for first alloc.
    if (!fCursor) {
        this->ensureSpace(totalSize, alignment);
        goto restart;
    }

    assert(fEnd);
    // This test alone would be enough nullptr were defined to be 0, but it's not.
    char* objStart = (char*)((uintptr_t)(fCursor + skipOverhead + mask) & ~mask);
    if ((ptrdiff_t)totalSize > fEnd - objStart) {
        this->ensureSpace(totalSize, alignment);
        goto restart;
    }

    AssertRelease((ptrdiff_t)totalSize <= fEnd - objStart);

    // Install a skip footer if needed, thus terminating a run of POD data. The calling code is
    // responsible for installing the footer after the object.
    if (needsSkipFooter) {
        this->installUint32Footer(SkipPod, ToU32(fCursor - fDtorCursor), 0);
    }

    return objStart;
}

static uint32_t to_uint32_t(size_t v) {
    assert(SkTFitsIn<uint32_t>(v));
    return (uint32_t)v;
}

SkArenaAllocWithReset::SkArenaAllocWithReset(char* block,
                                             size_t size,
                                             size_t firstHeapAllocation)
        : SkArenaAlloc(block, size, firstHeapAllocation)
        , fFirstBlock{block}
        , fFirstSize{to_uint32_t(size)}
        , fFirstHeapAllocationSize{to_uint32_t(firstHeapAllocation)} {}

void SkArenaAllocWithReset::reset() {
    this->~SkArenaAllocWithReset();
    new (this) SkArenaAllocWithReset{fFirstBlock, fFirstSize, fFirstHeapAllocationSize};
}
