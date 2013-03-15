/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPurgeableMemoryBlock_DEFINED
#define SkPurgeableMemoryBlock_DEFINED

#include "SkTypes.h"

class SkPurgeableMemoryBlock : public SkNoncopyable {

public:
    /**
     *  Whether or not this platform has an implementation for purgeable memory.
     */
    static bool IsSupported();

    /**
     *  Create a new purgeable memory block of 'size' bytes. Returns NULL if not supported on this
     *  platform or on failure.
     *  @param size Number of bytes requested.
     *  @return A new block, or NULL on failure.
     */
    static SkPurgeableMemoryBlock* Create(size_t size);

#ifdef SK_DEBUG
    /**
     *  Whether the platform supports one shot purge of all unpinned blocks. If so,
     *  PurgeAllUnpinnedBlocks will be used to test a purge. Otherwise, purge will be called on
     *  individual blocks.
     */
    static bool PlatformSupportsPurgingAllUnpinnedBlocks();

    /**
     *  Purge all unpinned blocks at once, if the platform supports it.
     */
    static bool PurgeAllUnpinnedBlocks();

    // If PlatformSupportsPurgingAllUnpinnedBlocks returns true, this will not be called, so it can
    // simply return false.
    bool purge();

    bool isPinned() const { return fPinned; }
#endif

    ~SkPurgeableMemoryBlock();

    /**
     *  Output parameter for pin(), stating whether the data has been retained.
     */
    enum PinResult {
        /**
         *  The data has been purged, or this is the first call to pin.
         */
        kUninitialized_PinResult,

        /**
         *  The data has been retained. The memory contains the same data it held when unpin() was
         *  called.
         */
        kRetained_PinResult,
    };

    /**
     *  Pin the memory for use. Must not be called while already pinned.
     *  @param PinResult Whether the data was retained. Ignored on failure.
     *  @return Pointer to the pinned data on success. NULL on failure.
     */
    void* pin(PinResult*);

    /**
     *  Unpin the data so it can be purged if necessary.
     */
    void unpin();

private:
    void*       fAddr;
    size_t      fSize;
    bool        fPinned;
#ifdef SK_BUILD_FOR_ANDROID
    int         fFD;
#endif

    // Unimplemented default constructor is private, to prevent manual creation.
    SkPurgeableMemoryBlock();

    // The correct way to create a new one is from the static Create.
    SkPurgeableMemoryBlock(size_t);
};

#endif // SkPurgeableMemoryBlock_DEFINED
