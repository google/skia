/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkVarAlloc.h"

#include "SkMalloc.h"

struct SkVarAlloc::Block {
    Block* prev;
    char* data() { return (char*)(this + 1); }

    static Block* Alloc(Block* prev, size_t size) {
        SkASSERT(size >= sizeof(Block));
        Block* b = (Block*)sk_malloc_throw(size);
        b->prev = prev;
        return b;
    }
};

SkVarAlloc::SkVarAlloc(size_t minLgSize)
    : fBytesAllocated(0)
    , fByte(nullptr)
    , fRemaining(0)
    , fLgSize(minLgSize)
    , fBlock(nullptr) {}

SkVarAlloc::SkVarAlloc(size_t minLgSize, char* storage, size_t len)
    : fBytesAllocated(0)
    , fByte(storage)
    , fRemaining(len)
    , fLgSize(minLgSize)
    , fBlock(nullptr) {}

SkVarAlloc::~SkVarAlloc() {
    Block* b = fBlock;
    while (b) {
        Block* prev = b->prev;
        sk_free(b);
        b = prev;
    }
}

void SkVarAlloc::makeSpace(size_t bytes) {
    SkASSERT(SkIsAlignPtr(bytes));

    size_t alloc = static_cast<size_t>(1)<<fLgSize++;
    while (alloc < bytes + sizeof(Block)) {
        alloc *= 2;
    }
    fBytesAllocated += alloc;
    fBlock = Block::Alloc(fBlock, alloc);
    fByte = fBlock->data();
    fRemaining = alloc - sizeof(Block);
}
