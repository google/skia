//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/PoolAlloc.h"

#include "compiler/translator/InitializeGlobals.h"

#include "common/platform.h"
#include "common/angleutils.h"
#include "common/tls.h"

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

TLSIndex PoolIndex = TLS_INVALID_INDEX;

bool InitializePoolIndex()
{
    assert(PoolIndex == TLS_INVALID_INDEX);

    PoolIndex = CreateTLSIndex();
    return PoolIndex != TLS_INVALID_INDEX;
}

void FreePoolIndex()
{
    assert(PoolIndex != TLS_INVALID_INDEX);

    DestroyTLSIndex(PoolIndex);
    PoolIndex = TLS_INVALID_INDEX;
}

TPoolAllocator* GetGlobalPoolAllocator()
{
    assert(PoolIndex != TLS_INVALID_INDEX);
    return static_cast<TPoolAllocator*>(GetTLSValue(PoolIndex));
}

void SetGlobalPoolAllocator(TPoolAllocator* poolAllocator)
{
    assert(PoolIndex != TLS_INVALID_INDEX);
    SetTLSValue(PoolIndex, poolAllocator);
}

//
// Implement the functionality of the TPoolAllocator class, which
// is documented in PoolAlloc.h.
//
TPoolAllocator::TPoolAllocator(int growthIncrement, int allocationAlignment) : 
    pageSize(growthIncrement),
    alignment(allocationAlignment),
    freeList(0),
    inUseList(0),
    numCalls(0),
    totalBytes(0)
{
    //
    // Don't allow page sizes we know are smaller than all common
    // OS page sizes.
    //
    if (pageSize < 4*1024)
        pageSize = 4*1024;

    //
    // A large currentPageOffset indicates a new page needs to
    // be obtained to allocate memory.
    //
    currentPageOffset = pageSize;

    //
    // Adjust alignment to be at least pointer aligned and
    // power of 2.
    //
    size_t minAlign = sizeof(void*);
    alignment &= ~(minAlign - 1);
    if (alignment < minAlign)
        alignment = minAlign;
    size_t a = 1;
    while (a < alignment)
        a <<= 1;
    alignment = a;
    alignmentMask = a - 1;

    //
    // Align header skip
    //
    headerSkip = minAlign;
    if (headerSkip < sizeof(tHeader)) {
        headerSkip = (sizeof(tHeader) + alignmentMask) & ~alignmentMask;
    }
}

TPoolAllocator::~TPoolAllocator()
{
    while (inUseList) {
        tHeader* next = inUseList->nextPage;
        inUseList->~tHeader();
        delete [] reinterpret_cast<char*>(inUseList);
        inUseList = next;
    }

    // We should not check the guard blocks
    // here, because we did it already when the block was
    // placed into the free list.
    //
    while (freeList) {
        tHeader* next = freeList->nextPage;
        delete [] reinterpret_cast<char*>(freeList);
        freeList = next;
    }
}

// Support MSVC++ 6.0
const unsigned char TAllocation::guardBlockBeginVal = 0xfb;
const unsigned char TAllocation::guardBlockEndVal   = 0xfe;
const unsigned char TAllocation::userDataFill       = 0xcd;

#ifdef GUARD_BLOCKS
    const size_t TAllocation::guardBlockSize = 16;
#else
    const size_t TAllocation::guardBlockSize = 0;
#endif

//
// Check a single guard block for damage
//
void TAllocation::checkGuardBlock(unsigned char* blockMem, unsigned char val, const char* locText) const
{
#ifdef GUARD_BLOCKS
    for (size_t x = 0; x < guardBlockSize; x++) {
        if (blockMem[x] != val) {
            char assertMsg[80];

            // We don't print the assert message.  It's here just to be helpful.
#if defined(_MSC_VER)
            snprintf(assertMsg, sizeof(assertMsg), "PoolAlloc: Damage %s %Iu byte allocation at 0x%p\n",
                    locText, size, data());
#else
            snprintf(assertMsg, sizeof(assertMsg), "PoolAlloc: Damage %s %zu byte allocation at 0x%p\n",
                    locText, size, data());
#endif
            assert(0 && "PoolAlloc: Damage in guard block");
        }
    }
#endif
}


void TPoolAllocator::push()
{
    tAllocState state = { currentPageOffset, inUseList };

    stack.push_back(state);
        
    //
    // Indicate there is no current page to allocate from.
    //
    currentPageOffset = pageSize;
}

//
// Do a mass-deallocation of all the individual allocations
// that have occurred since the last push(), or since the
// last pop(), or since the object's creation.
//
// The deallocated pages are saved for future allocations.
//
void TPoolAllocator::pop()
{
    if (stack.size() < 1)
        return;

    tHeader* page = stack.back().page;
    currentPageOffset = stack.back().offset;

    while (inUseList != page) {
        // invoke destructor to free allocation list
        inUseList->~tHeader();
        
        tHeader* nextInUse = inUseList->nextPage;
        if (inUseList->pageCount > 1)
            delete [] reinterpret_cast<char*>(inUseList);
        else {
            inUseList->nextPage = freeList;
            freeList = inUseList;
        }
        inUseList = nextInUse;
    }

    stack.pop_back();
}

//
// Do a mass-deallocation of all the individual allocations
// that have occurred.
//
void TPoolAllocator::popAll()
{
    while (stack.size() > 0)
        pop();
}

void* TPoolAllocator::allocate(size_t numBytes)
{
    //
    // Just keep some interesting statistics.
    //
    ++numCalls;
    totalBytes += numBytes;

    // If we are using guard blocks, all allocations are bracketed by
    // them: [guardblock][allocation][guardblock].  numBytes is how
    // much memory the caller asked for.  allocationSize is the total
    // size including guard blocks.  In release build,
    // guardBlockSize=0 and this all gets optimized away.
    size_t allocationSize = TAllocation::allocationSize(numBytes);
    // Detect integer overflow.
    if (allocationSize < numBytes)
        return 0;

    //
    // Do the allocation, most likely case first, for efficiency.
    // This step could be moved to be inline sometime.
    //
    if (allocationSize <= pageSize - currentPageOffset) {
        //
        // Safe to allocate from currentPageOffset.
        //
        unsigned char* memory = reinterpret_cast<unsigned char *>(inUseList) + currentPageOffset;
        currentPageOffset += allocationSize;
        currentPageOffset = (currentPageOffset + alignmentMask) & ~alignmentMask;

        return initializeAllocation(inUseList, memory, numBytes);
    }

    if (allocationSize > pageSize - headerSkip) {
        //
        // Do a multi-page allocation.  Don't mix these with the others.
        // The OS is efficient and allocating and free-ing multiple pages.
        //
        size_t numBytesToAlloc = allocationSize + headerSkip;
        // Detect integer overflow.
        if (numBytesToAlloc < allocationSize)
            return 0;

        tHeader* memory = reinterpret_cast<tHeader*>(::new char[numBytesToAlloc]);
        if (memory == 0)
            return 0;

        // Use placement-new to initialize header
        new(memory) tHeader(inUseList, (numBytesToAlloc + pageSize - 1) / pageSize);
        inUseList = memory;

        currentPageOffset = pageSize;  // make next allocation come from a new page

        // No guard blocks for multi-page allocations (yet)
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory) + headerSkip);
    }

    //
    // Need a simple page to allocate from.
    //
    tHeader* memory;
    if (freeList) {
        memory = freeList;
        freeList = freeList->nextPage;
    } else {
        memory = reinterpret_cast<tHeader*>(::new char[pageSize]);
        if (memory == 0)
            return 0;
    }

    // Use placement-new to initialize header
    new(memory) tHeader(inUseList, 1);
    inUseList = memory;
    
    unsigned char* ret = reinterpret_cast<unsigned char *>(inUseList) + headerSkip;
    currentPageOffset = (headerSkip + allocationSize + alignmentMask) & ~alignmentMask;

    return initializeAllocation(inUseList, ret, numBytes);
}


//
// Check all allocations in a list for damage by calling check on each.
//
void TAllocation::checkAllocList() const
{
    for (const TAllocation* alloc = this; alloc != 0; alloc = alloc->prevAlloc)
        alloc->check();
}
