/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include <cstddef>
#include "SkArenaAlloc.h"


template <typename BlockAlloc>
char* SkArenaAllocBase<BlockAlloc>::EndChain(char*, SkArenaAllocBase*) { return nullptr; }

template <typename BlockAlloc>
char* SkArenaAllocBase<BlockAlloc>::SkipPod(char* footerEnd, SkArenaAllocBase*) {
    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(int32_t));
    int32_t skip;
    memmove(&skip, objEnd, sizeof(int32_t));
    return objEnd - skip;
}

template <typename BlockAlloc>
void SkArenaAllocBase<BlockAlloc>::RunDtorsOnBlock(char* footerEnd, SkArenaAllocBase* alloc) {
    while (footerEnd != nullptr) {
        Footer footer;
        memcpy(&footer, footerEnd - sizeof(Footer), sizeof(Footer));

        FooterAction* action = (FooterAction*)(footer >> 6);
        ptrdiff_t padding = footer & 63;

        footerEnd = action(footerEnd, alloc) - padding;
    }
}

template <typename BlockAlloc>
char* SkArenaAllocBase<BlockAlloc>::NextBlock(char* footerEnd, SkArenaAllocBase* alloc) {
    char* objEnd = footerEnd - (sizeof(Footer) + sizeof(char*));
    char* next;
    memmove(&next, objEnd, sizeof(char*));
    RunDtorsOnBlock(next, alloc);
    alloc->deleteBlock(objEnd);
    return nullptr;
}

template <typename BlockAlloc>
SkArenaAllocBase<BlockAlloc>::SkArenaAllocBase(char* block, size_t size, size_t extraSize)
    : BlockAlloc  {SkTo<uint32_t>(extraSize)}
    , fDtorCursor {block}
    , fCursor     {block}
    , fEnd        {block + SkTo<uint32_t>(size)}
    , fFirstBlock {block}
    , fFirstSize  {SkTo<uint32_t>(size)}
    , fExtraSize  {SkTo<uint32_t>(extraSize)}
{
    if (size < sizeof(Footer)) {
        fEnd = fCursor = fDtorCursor = nullptr;
    }

    if (fCursor != nullptr) {
        this->installFooter(EndChain, 0);
    }
}

template <typename BlockAlloc>
SkArenaAllocBase<BlockAlloc>::~SkArenaAllocBase() {
    RunDtorsOnBlock(fDtorCursor, this);
}

template <typename BlockAlloc>
void SkArenaAllocBase<BlockAlloc>::reset() {
    this->~SkArenaAllocBase();
    new (this) SkArenaAllocBase{fFirstBlock, fFirstSize, fExtraSize};
}

template <typename BlockAlloc>
void SkArenaAllocBase<BlockAlloc>::installFooter(FooterAction* action, uint32_t padding) {
    SkASSERT(padding < 64);
    int64_t actionInt = (int64_t)(intptr_t)action;

    // The top 14 bits should be either all 0s or all 1s. Check this.
    SkASSERT((actionInt << 6) >> 6 == actionInt);
    Footer encodedFooter = (actionInt << 6) | padding;
    memmove(fCursor, &encodedFooter, sizeof(Footer));
    fCursor += sizeof(Footer);
    fDtorCursor = fCursor;
}

template <typename BlockAlloc>
void SkArenaAllocBase<BlockAlloc>::installPtrFooter(FooterAction* action, char* ptr, uint32_t padding) {
    memmove(fCursor, &ptr, sizeof(char*));
    fCursor += sizeof(char*);
    this->installFooter(action, padding);
}

template <typename BlockAlloc>
void SkArenaAllocBase<BlockAlloc>::installUint32Footer(FooterAction* action, uint32_t value, uint32_t padding) {
    memmove(fCursor, &value, sizeof(uint32_t));
    fCursor += sizeof(uint32_t);
    this->installFooter(action, padding);
}

template <typename BlockAlloc>
void SkArenaAllocBase<BlockAlloc>::ensureSpace(uint32_t size, uint32_t alignment) {
    constexpr uint32_t headerSize = sizeof(Footer) + sizeof(ptrdiff_t);
    // The chrome c++ library we use does not define std::max_align_t.
    // This must be conservative to add the right amount of extra memory to handle the alignment
    // padding.
    constexpr uint32_t alignof_max_align_t = 8;
    uint32_t objSizeAndOverhead = size + headerSize + sizeof(Footer);
    if (alignment > alignof_max_align_t) {
        objSizeAndOverhead += alignment - 1;
    }

    char* newBlock;
    uint32_t allocationSize;

    std::tie(newBlock, allocationSize) = this->newBlock(objSizeAndOverhead);

    auto previousDtor = fDtorCursor;
    fCursor = newBlock;
    fDtorCursor = newBlock;
    fEnd = fCursor + allocationSize;
    this->installPtrFooter(NextBlock, previousDtor, 0);
}

template <typename BlockAlloc>
char* SkArenaAllocBase<BlockAlloc>::allocObject(uint32_t size, uint32_t alignment) {
    uintptr_t mask = alignment - 1;
    char* objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
    if ((ptrdiff_t)size > fEnd - objStart) {
        this->ensureSpace(size, alignment);
        objStart = (char*)((uintptr_t)(fCursor + mask) & ~mask);
    }
    return objStart;
}

template <typename BlockAlloc>
char* SkArenaAllocBase<BlockAlloc>::allocObjectWithFooter(uint32_t sizeIncludingFooter, uint32_t alignment) {
    uintptr_t mask = alignment - 1;

restart:
    uint32_t skipOverhead = 0;
    bool needsSkipFooter = fCursor != fDtorCursor;
    if (needsSkipFooter) {
        skipOverhead = sizeof(Footer) + sizeof(uint32_t);
    }
    char* objStart = (char*)((uintptr_t)(fCursor + skipOverhead + mask) & ~mask);
    uint32_t totalSize = sizeIncludingFooter + skipOverhead;

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

template class SkArenaAllocBase<SkArenaBlockAlloc>;

