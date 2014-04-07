/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAllocPool_DEFINED
#define GrAllocPool_DEFINED

#include "SkTypes.h"

class GrAllocPool : SkNoncopyable {
public:
    GrAllocPool(size_t blockSize = 0);
    ~GrAllocPool();

    /**
     *  Frees all blocks that have been allocated with alloc().
     */
    void reset();

    /**
     *  Returns a block of memory bytes size big. This address must not be
     *  passed to realloc/free/delete or any other function that assumes the
     *  address was allocated by malloc or new (because it hasn't).
     */
    void* alloc(size_t bytes);

    /**
     * Releases the most recently allocated bytes back to allocpool.
     */
    void release(size_t bytes);

private:
    struct Block;

    Block*  fBlock;
    size_t  fMinBlockSize;

#ifdef SK_DEBUG
    int fBlocksAllocated;
    void validate() const;
#else
    void validate() const {}
#endif
};

template <typename T> class GrTAllocPool {
public:
    GrTAllocPool(int count) : fPool(count * sizeof(T)) {}

    void reset() { fPool.reset(); }
    T* alloc() { return (T*)fPool.alloc(sizeof(T)); }

private:
    GrAllocPool fPool;
};

#endif
