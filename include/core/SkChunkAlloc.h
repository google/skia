
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkChunkAlloc_DEFINED
#define SkChunkAlloc_DEFINED

#include "SkTypes.h"

class SkChunkAlloc : SkNoncopyable {
public:
    SkChunkAlloc(size_t minSize);
    ~SkChunkAlloc();

    /**
     *  Free up all allocated blocks. This invalidates all returned
     *  pointers.
     */
    void reset();
    /**
     *  Reset to 0 used bytes preserving as much memory as possible.
     *  This invalidates all returned pointers.
     */
    void rewind();

    enum AllocFailType {
        kReturnNil_AllocFailType,
        kThrow_AllocFailType
    };

    void* alloc(size_t bytes, AllocFailType);
    void* allocThrow(size_t bytes) {
        return this->alloc(bytes, kThrow_AllocFailType);
    }

    /** Call this to unalloc the most-recently allocated ptr by alloc(). On
        success, the number of bytes freed is returned, or 0 if the block could
        not be unallocated. This is a hint to the underlying allocator that
        the previous allocation may be reused, but the implementation is free
        to ignore this call (and return 0).
     */
    size_t unalloc(void* ptr);

    size_t totalCapacity() const { return fTotalCapacity; }
    size_t totalUsed() const { return fTotalUsed; }
    SkDEBUGCODE(int blockCount() const { return fBlockCount; })
    SkDEBUGCODE(size_t totalLost() const { return fTotalLost; })

    /**
     *  Returns true if the specified address is within one of the chunks, and
     *  has at least 1-byte following the address (i.e. if addr points to the
     *  end of a chunk, then contains() will return false).
     */
    bool contains(const void* addr) const;

private:
    struct Block;

    Block*  fBlock;
    size_t  fMinSize;
    size_t  fChunkSize;
    size_t  fTotalCapacity;
    size_t  fTotalUsed;     // will be <= fTotalCapacity
    SkDEBUGCODE(int     fBlockCount;)
    SkDEBUGCODE(size_t  fTotalLost;)     // will be <= fTotalCapacity

    Block* newBlock(size_t bytes, AllocFailType ftype);
    Block* addBlockIfNecessary(size_t bytes, AllocFailType ftype);

    SkDEBUGCODE(void validate();)
};

#endif
