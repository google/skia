/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageCache_DEFINED
#define SkImageCache_DEFINED

#include "SkRefCnt.h"
#include "SkTypes.h"

/**
 *  Interface for a cache that manages pixel memory.
 */
class SkImageCache : public SkRefCnt {

public:
    SK_DECLARE_INST_COUNT(SkImageCache)

    typedef intptr_t ID;

    /**
     *  Allocate memory whose lifetime is managed by the cache. On success, MUST be balanced with a
     *  call to releaseCache and a call to throwAwayCache.
     *  @param bytes Number of bytes needed.
     *  @param ID Output parameter which must not be NULL. On success, ID will be set to a value
     *      associated with that memory which can be used as a parameter to the other functions
     *      in SkImageCache. On failure, ID is unchanged.
     *  @return Pointer to the newly allocated memory, or NULL. This memory is safe to use until
     *      releaseCache is called with ID.
     */
    virtual void* allocAndPinCache(size_t bytes, ID*) = 0;

    /**
     *  Output parameter for pinCache, stating whether the memory still contains the data it held
     *  when releaseCache was last called for the same ID.
     */
    enum DataStatus {
        /**
         *  The data has been purged, and therefore needs to be rewritten to the returned memory.
         */
        kUninitialized_DataStatus,

        /**
         *  The memory still contains the data it held when releaseCache was last called with the
         *  same ID.
         */
        kRetained_DataStatus,
    };

    /**
     *  Re-request the memory associated with ID and pin it so that it will not be reclaimed until
     *  the next call to releaseCache with the same ID.
     *  @param ID Unique ID for the memory block.
     *  @param status Output parameter which must not be NULL. On success (i.e. the return value is
     *      not NULL), status will be set to one of two states representing the cached memory. If
     *      status is set to kRetained_DataStatus, the memory contains the same data it did
     *      before releaseCache was called with this ID. If status is set to
     *      kUninitialized_DataStatus, the memory is still pinned, but the previous data is no
     *      longer available. If the return value is NULL, status is unchanged.
     *  @return Pointer: If non-NULL, points to the previously allocated memory, in which case
     *      this call must be balanced with a call to releaseCache. If NULL, the memory
     *      has been reclaimed, and throwAwayCache MUST NOT be called.
     */
    virtual void* pinCache(ID, DataStatus* status) = 0;

    /**
     *  Inform the cache that it is safe to free the block of memory corresponding to ID. After
     *  calling this function, the pointer returned by allocAndPinCache or pinCache must not be
     *  used again. In order to access the same memory after this, pinCache must be called with
     *  the same ID.
     *  @param ID Unique ID for the memory block which is now safe to age out of the cache.
     */
    virtual void releaseCache(ID) = 0;

    /**
     *  Inform the cache that the block of memory associated with ID will not be asked for again.
     *  After this call, ID is no longer valid. Must not be called while the associated memory is
     *  pinned. Must be called to balance a successful allocAndPinCache.
     */
    virtual void throwAwayCache(ID) = 0;

    /**
     *  ID which does not correspond to any valid cache.
     */
    static const ID UNINITIALIZED_ID = 0;

#ifdef SK_DEBUG
    /**
     *  Debug only status of a memory block.
     */
    enum MemoryStatus {
        /**
         *  It is safe to use the pointer returned by the most recent of allocAndPinCache(ID) or
         *  pinCache(ID) with the same ID.
         */
        kPinned_MemoryStatus,

        /**
         *  The pointer returned by the most recent call to allocAndPinCache(ID) or pinCache(ID) has
         *  since been released by releaseCache(ID). In order to reuse it, pinCache(ID) must be
         *  called again. Note that after calling releaseCache(ID), the status of that particular
         *  ID may not be kUnpinned_MemoryStatus, depending on the implementation, but it will not
         *  be kPinned_MemoryStatus.
         */
        kUnpinned_MemoryStatus,

        /**
         *  The memory associated with ID has been thrown away. No calls should be made using the
         *  same ID.
         */
        kFreed_MemoryStatus,
    };

    /**
     *  Debug only function to get the status of a particular block of memory. Safe to call after
     *  throwAwayCache has been called with this ID.
     */
    virtual MemoryStatus getMemoryStatus(intptr_t ID) const = 0;

    /**
     *  Debug only function to clear all unpinned caches.
     */
    virtual void purgeAllUnpinnedCaches() = 0;
#endif

private:
    typedef SkRefCnt INHERITED;
};
#endif // SkImageCache_DEFINED
