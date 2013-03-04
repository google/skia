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
    /**
     *  Allocate memory whose lifetime is managed by the cache. On success, MUST be balanced with a
     *  call to releaseCache and a call to throwAwayCache.
     *  @param bytes Number of bytes needed.
     *  @param ID Output parameter which must not be NULL. On success, ID will be set to a value
     *         associated with that memory which can be used as a parameter to the other functions
     *         in SkImageCache. On failure, ID is unchanged.
     *  @return Pointer to the newly allocated memory, or NULL. This memory is safe to use until
     *          releaseCache is called with ID.
     */
    virtual void* allocAndPinCache(size_t bytes, intptr_t* ID) = 0;

    /**
     *  Re-request the memory associated with ID.
     *  @param ID Unique ID for the memory block.
     *  @return Pointer: If non-NULL, points to the previously allocated memory, in which case
     *          this call must be balanced with a call to releaseCache. If NULL, the memory
     *          has been reclaimed, so allocAndPinCache must be called again with a pointer to
     *          the same ID.
     */
    virtual void* pinCache(intptr_t ID) = 0;

    /**
     *  Inform the cache that it is safe to free the block of memory corresponding to ID. After
     *  calling this function, the pointer returned by allocAndPinCache or pinCache must not be
     *  used again. In order to access the same memory after this, pinCache must be called with
     *  the same ID.
     *  @param ID Unique ID for the memory block which is now safe to age out of the cache.
     */
    virtual void releaseCache(intptr_t ID) = 0;

    /**
     *  Inform the cache that the block of memory associated with ID will not be asked for again.
     *  After this call, ID is no longer valid. Must not be called while the associated memory is
     *  pinned. Must be called to balance a successful allocAndPinCache.
     */
    virtual void throwAwayCache(intptr_t ID) = 0;

    /**
     *  ID which does not correspond to any valid cache.
     */
    static const intptr_t UNINITIALIZED_ID = 0;

#ifdef SK_DEBUG
    enum CacheStatus {
        kPinned_CacheStatus,
        kUnpinned_CacheStatus,
        kThrownAway_CacheStatus,
    };

    /**
     *  Debug only function to get the status of a particular block of memory.
     */
    virtual CacheStatus getCacheStatus(intptr_t ID) const = 0;
#endif
};
#endif // SkImageCache_DEFINED
