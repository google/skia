
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDeque.h"

struct SkDeque::Block {
    Block*  fNext;
    Block*  fPrev;
    char*   fBegin; // start of used section in this chunk
    char*   fEnd;   // end of used section in this chunk
    char*   fStop;  // end of the allocated chunk

    char*       start() { return (char*)(this + 1); }
    const char* start() const { return (const char*)(this + 1); }

    void init(size_t size) {
        fNext   = fPrev = nullptr;
        fBegin  = fEnd = nullptr;
        fStop   = (char*)this + size;
    }
};

SkDeque::SkDeque(size_t elemSize, int allocCount)
        : fElemSize(elemSize)
        , fInitialStorage(nullptr)
        , fCount(0)
        , fAllocCount(allocCount) {
    SkASSERT(allocCount >= 1);
    fFrontBlock = fBackBlock = nullptr;
    fFront = fBack = nullptr;
}

SkDeque::SkDeque(size_t elemSize, void* storage, size_t storageSize, int allocCount)
        : fElemSize(elemSize)
        , fInitialStorage(storage)
        , fCount(0)
        , fAllocCount(allocCount) {
    SkASSERT(storageSize == 0 || storage != nullptr);
    SkASSERT(allocCount >= 1);

    if (storageSize >= sizeof(Block) + elemSize) {
        fFrontBlock = (Block*)storage;
        fFrontBlock->init(storageSize);
    } else {
        fFrontBlock = nullptr;
    }
    fBackBlock = fFrontBlock;
    fFront = fBack = nullptr;
}

SkDeque::~SkDeque() {
    Block* head = fFrontBlock;
    Block* initialHead = (Block*)fInitialStorage;

    while (head) {
        Block* next = head->fNext;
        if (head != initialHead) {
            this->freeBlock(head);
        }
        head = next;
    }
}

void* SkDeque::push_front() {
    fCount += 1;

    if (nullptr == fFrontBlock) {
        fFrontBlock = this->allocateBlock(fAllocCount);
        fBackBlock = fFrontBlock;     // update our linklist
    }

    Block*  first = fFrontBlock;
    char*   begin;

    if (nullptr == first->fBegin) {
    INIT_CHUNK:
        first->fEnd = first->fStop;
        begin = first->fStop - fElemSize;
    } else {
        begin = first->fBegin - fElemSize;
        if (begin < first->start()) {    // no more room in this chunk
            // should we alloc more as we accumulate more elements?
            first = this->allocateBlock(fAllocCount);
            first->fNext = fFrontBlock;
            fFrontBlock->fPrev = first;
            fFrontBlock = first;
            goto INIT_CHUNK;
        }
    }

    first->fBegin = begin;

    if (nullptr == fFront) {
        SkASSERT(nullptr == fBack);
        fFront = fBack = begin;
    } else {
        SkASSERT(fBack);
        fFront = begin;
    }

    return begin;
}

void* SkDeque::push_back() {
    fCount += 1;

    if (nullptr == fBackBlock) {
        fBackBlock = this->allocateBlock(fAllocCount);
        fFrontBlock = fBackBlock; // update our linklist
    }

    Block*  last = fBackBlock;
    char*   end;

    if (nullptr == last->fBegin) {
    INIT_CHUNK:
        last->fBegin = last->start();
        end = last->fBegin + fElemSize;
    } else {
        end = last->fEnd + fElemSize;
        if (end > last->fStop) {  // no more room in this chunk
            // should we alloc more as we accumulate more elements?
            last = this->allocateBlock(fAllocCount);
            last->fPrev = fBackBlock;
            fBackBlock->fNext = last;
            fBackBlock = last;
            goto INIT_CHUNK;
        }
    }

    last->fEnd = end;
    end -= fElemSize;

    if (nullptr == fBack) {
        SkASSERT(nullptr == fFront);
        fFront = fBack = end;
    } else {
        SkASSERT(fFront);
        fBack = end;
    }

    return end;
}

void SkDeque::pop_front() {
    SkASSERT(fCount > 0);
    fCount -= 1;

    Block*  first = fFrontBlock;

    SkASSERT(first != nullptr);

    if (first->fBegin == nullptr) {  // we were marked empty from before
        first = first->fNext;
        first->fPrev = nullptr;
        this->freeBlock(fFrontBlock);
        fFrontBlock = first;
        SkASSERT(first != nullptr);    // else we popped too far
    }

    char* begin = first->fBegin + fElemSize;
    SkASSERT(begin <= first->fEnd);

    if (begin < fFrontBlock->fEnd) {
        first->fBegin = begin;
        SkASSERT(first->fBegin);
        fFront = first->fBegin;
    } else {
        first->fBegin = first->fEnd = nullptr;  // mark as empty
        if (nullptr == first->fNext) {
            fFront = fBack = nullptr;
        } else {
            SkASSERT(first->fNext->fBegin);
            fFront = first->fNext->fBegin;
        }
    }
}

void SkDeque::pop_back() {
    SkASSERT(fCount > 0);
    fCount -= 1;

    Block* last = fBackBlock;

    SkASSERT(last != nullptr);

    if (last->fEnd == nullptr) {  // we were marked empty from before
        last = last->fPrev;
        last->fNext = nullptr;
        this->freeBlock(fBackBlock);
        fBackBlock = last;
        SkASSERT(last != nullptr);  // else we popped too far
    }

    char* end = last->fEnd - fElemSize;
    SkASSERT(end >= last->fBegin);

    if (end > last->fBegin) {
        last->fEnd = end;
        SkASSERT(last->fEnd);
        fBack = last->fEnd - fElemSize;
    } else {
        last->fBegin = last->fEnd = nullptr;    // mark as empty
        if (nullptr == last->fPrev) {
            fFront = fBack = nullptr;
        } else {
            SkASSERT(last->fPrev->fEnd);
            fBack = last->fPrev->fEnd - fElemSize;
        }
    }
}

int SkDeque::numBlocksAllocated() const {
    int numBlocks = 0;

    for (const Block* temp = fFrontBlock; temp; temp = temp->fNext) {
        ++numBlocks;
    }

    return numBlocks;
}

SkDeque::Block* SkDeque::allocateBlock(int allocCount) {
    Block* newBlock = (Block*)sk_malloc_throw(sizeof(Block) + allocCount * fElemSize);
    newBlock->init(sizeof(Block) + allocCount * fElemSize);
    return newBlock;
}

void SkDeque::freeBlock(Block* block) {
    sk_free(block);
}

///////////////////////////////////////////////////////////////////////////////

SkDeque::Iter::Iter() : fCurBlock(nullptr), fPos(nullptr), fElemSize(0) {}

SkDeque::Iter::Iter(const SkDeque& d, IterStart startLoc) {
    this->reset(d, startLoc);
}

// Due to how reset and next work, next actually returns the current element
// pointed to by fPos and then updates fPos to point to the next one.
void* SkDeque::Iter::next() {
    char* pos = fPos;

    if (pos) {   // if we were valid, try to move to the next setting
        char* next = pos + fElemSize;
        SkASSERT(next <= fCurBlock->fEnd);
        if (next == fCurBlock->fEnd) { // exhausted this chunk, move to next
            do {
                fCurBlock = fCurBlock->fNext;
            } while (fCurBlock != nullptr && fCurBlock->fBegin == nullptr);
            next = fCurBlock ? fCurBlock->fBegin : nullptr;
        }
        fPos = next;
    }
    return pos;
}

// Like next, prev actually returns the current element pointed to by fPos and
// then makes fPos point to the previous element.
void* SkDeque::Iter::prev() {
    char* pos = fPos;

    if (pos) {   // if we were valid, try to move to the prior setting
        char* prev = pos - fElemSize;
        SkASSERT(prev >= fCurBlock->fBegin - fElemSize);
        if (prev < fCurBlock->fBegin) { // exhausted this chunk, move to prior
            do {
                fCurBlock = fCurBlock->fPrev;
            } while (fCurBlock != nullptr && fCurBlock->fEnd == nullptr);
            prev = fCurBlock ? fCurBlock->fEnd - fElemSize : nullptr;
        }
        fPos = prev;
    }
    return pos;
}

// reset works by skipping through the spare blocks at the start (or end)
// of the doubly linked list until a non-empty one is found. The fPos
// member is then set to the first (or last) element in the block. If
// there are no elements in the deque both fCurBlock and fPos will come
// out of this routine nullptr.
void SkDeque::Iter::reset(const SkDeque& d, IterStart startLoc) {
    fElemSize = d.fElemSize;

    if (kFront_IterStart == startLoc) {
        // initialize the iterator to start at the front
        fCurBlock = d.fFrontBlock;
        while (fCurBlock && nullptr == fCurBlock->fBegin) {
            fCurBlock = fCurBlock->fNext;
        }
        fPos = fCurBlock ? fCurBlock->fBegin : nullptr;
    } else {
        // initialize the iterator to start at the back
        fCurBlock = d.fBackBlock;
        while (fCurBlock && nullptr == fCurBlock->fEnd) {
            fCurBlock = fCurBlock->fPrev;
        }
        fPos = fCurBlock ? fCurBlock->fEnd - fElemSize : nullptr;
    }
}
