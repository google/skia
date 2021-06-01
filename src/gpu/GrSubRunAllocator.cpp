/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMath.h"
#include "src/gpu/GrSubRunAllocator.h"

#include <cstddef>
#include <memory>
#include <new>

// -- GrBagOfBytes ---------------------------------------------------------------------------------
GrBagOfBytes::GrBagOfBytes(char* bytes, size_t size, size_t firstHeapAllocation)
        : fFibProgression(size, firstHeapAllocation) {
    SkASSERT_RELEASE(size < kMaxByteSize);
    SkASSERT_RELEASE(firstHeapAllocation < kMaxByteSize);

    std::size_t space = size;
    void* ptr = bytes;
    if (bytes && std::align(kMaxAlignment, sizeof(Block), ptr, space)) {
        this->setupBytesAndCapacity(bytes, size);
        new (fEndByte) Block(nullptr, nullptr);
    }
}

GrBagOfBytes::GrBagOfBytes(size_t firstHeapAllocation)
        : GrBagOfBytes(nullptr, 0, firstHeapAllocation) {}

GrBagOfBytes::~GrBagOfBytes() {
    Block* cursor = reinterpret_cast<Block*>(fEndByte);
    while (cursor != nullptr) {
        char* toDelete = cursor->fBlockStart;
        cursor = cursor->fPrevious;
        delete [] toDelete;
    }
}

GrBagOfBytes::Block::Block(char* previous, char* startOfBlock)
        : fBlockStart{startOfBlock}
        , fPrevious{reinterpret_cast<Block*>(previous)} {}

void* GrBagOfBytes::alignedBytes(int size, int alignment) {
    SkASSERT_RELEASE(0 < size && size < kMaxByteSize);
    SkASSERT_RELEASE(0 < alignment && alignment <= kMaxAlignment);
    SkASSERT_RELEASE(SkIsPow2(alignment));

    return this->allocateBytes(size, alignment);
}

void GrBagOfBytes::setupBytesAndCapacity(char* bytes, int size) {
    // endByte must be aligned to the maximum alignment to allow tracking alignment using capacity;
    // capacity and endByte are both aligned to max alignment.
    intptr_t endByte = reinterpret_cast<intptr_t>(bytes + size - sizeof(Block)) & -kMaxAlignment;
    fEndByte  = reinterpret_cast<char*>(endByte);
    fCapacity = fEndByte - bytes;
}

void GrBagOfBytes::needMoreBytes(int requestedSize, int alignment) {
    int nextBlockSize = fFibProgression.nextBlockSize();
    const int size = PlatformMinimumSizeWithOverhead(
            std::max(requestedSize, nextBlockSize), kAllocationAlignment);
    char* const bytes = new char[size];
    // fEndByte is changed by setupBytesAndCapacity. Remember it to link back to.
    char* const previousBlock = fEndByte;
    this->setupBytesAndCapacity(bytes, size);

    // Make a block to delete these bytes, and points to the previous block.
    new (fEndByte) Block{previousBlock, bytes};

    // Make fCapacity the alignment for the requested object.
    fCapacity = fCapacity & -alignment;
    SkASSERT(fCapacity >= requestedSize);
}

// -- GrSubRunAllocator ----------------------------------------------------------------------------
GrSubRunAllocator::GrSubRunAllocator(char* bytes, int size, int firstHeapAllocation)
        : fAlloc{bytes, SkTo<size_t>(size), SkTo<size_t>(firstHeapAllocation)} {}

GrSubRunAllocator::GrSubRunAllocator(int firstHeapAllocation)
        : GrSubRunAllocator(nullptr, 0, firstHeapAllocation) {}

void* GrSubRunAllocator::alignedBytes(int unsafeSize, int unsafeAlignment) {
    return fAlloc.alignedBytes(unsafeSize, unsafeAlignment);
}
