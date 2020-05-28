/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrRingBuffer.h"

// Get offset into buffer that has enough space for size
// Returns fTotalSize if no space
size_t GrRingBuffer::getSliceOffset(size_t size) {
    // capture current state locally (because fTail could be overwritten by the completion handler)
    size_t head, tail;
    SkAutoSpinlock lock(fMutex);
    head = fHead;
    tail = fTail;

    // The head and tail indices increment without bound, wrapping with overflow,
    // so we need to mod them down to the actual bounds of the allocation to determine
    // which blocks are available.
    size_t modHead = head & (fTotalSize - 1);
    size_t modTail = tail & (fTotalSize - 1);

    bool full = (head != tail && modHead == modTail);

    if (full) {
        return fTotalSize;
    }

    // case 1: free space lies at the beginning and/or the end of the buffer
    if (modHead >= modTail) {
        // check for room at the end
        if (fTotalSize - modHead < size) {
            // no room at the end, check the beginning
            if (modTail < size) {
                // no room at the beginning
                return fTotalSize;
            }
            // we are going to allocate from the beginning, adjust head to '0' position
            head += fTotalSize - modHead;
            modHead = 0;
        }
        // case 2: free space lies in the middle of the buffer, check for room there
    } else if (modTail - modHead < size) {
        // no room in the middle
        return fTotalSize;
    }

    fHead = GrAlignTo(head + size, fAlignment);
    return modHead;
}

size_t GrRingBuffer::suballocate(size_t size) {
#ifdef SK_BUILD_FOR_MAC
    // Mac requires 4-byte alignment for didModifyRange:
    size = SkAlign4(size);
#endif
    size_t offset = this->getSliceOffset(size);
    if (offset < fTotalSize) {
        return offset;
    }

    //*** unmap buffer and add to completion list?
    // With stored head?

    // Try to grow allocation (old allocation will age out).
    fTotalSize *= 2;
    fBuffer = this->createBuffer(fTotalSize);
    SkASSERT(fBuffer); // *** what to do about failure?
    SkAutoSpinlock lock(fMutex);
    fHead = 0;
    fTail = 0;
    offset = this->getSliceOffset(size);
    SkASSERT(offset < fTotalSize); //*** assumes we won't have a giant allocation
    return offset;
}

// used when current command buffer/command list is submitted
size_t GrRingBuffer::head() {
    SkAutoSpinlock lock(fMutex);
    return fHead;
}

// used when current command buffer/command list is completed
void GrRingBuffer::resetTail(size_t newTail) {
    SkAutoSpinlock lock(fMutex);
    fTail = newTail;
}
