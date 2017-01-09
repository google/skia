/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algorithm>
#include "SkArenaAlloc.h"

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

char* SkArenaAlloc::callFooterAction(char* end) {
    Footer footer;
    memcpy(&footer, end - sizeof(Footer), sizeof(Footer));

    FooterAction releaser = (FooterAction)((char*)EndChain + (footer >> 5));
    ptrdiff_t padding = footer & 31;

    return releaser(end) - padding;
}
