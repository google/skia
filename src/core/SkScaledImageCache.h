/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScaledImageCache_DEFINED
#define SkScaledImageCache_DEFINED

#include "SkBitmap.h"

class SkDiscardableMemory;
class SkMipMap;

/**
 *  Cache object for bitmaps (with possible scale in X Y as part of the key).
 *
 *  Multiple caches can be instantiated, but each instance is not implicitly
 *  thread-safe, so if a given instance is to be shared across threads, the
 *  caller must manage the access itself (e.g. via a mutex).
 *
 *  As a convenience, a global instance is also defined, which can be safely
 *  access across threads via the static methods (e.g. FindAndLock, etc.).
 */
class SkScaledImageCache {
public:
    struct ID;

    /**
     *  Returns a locked/pinned SkDiscardableMemory instance for the specified
     *  number of bytes, or NULL on failure.
     */
    typedef SkDiscardableMemory* (*DiscardableFactory)(size_t bytes);

    /*
     *  The following static methods are thread-safe wrappers around a global
     *  instance of this cache.
     */

    static ID* FindAndLock(uint32_t pixelGenerationID,
                           int32_t width,
                           int32_t height,
                           SkBitmap* returnedBitmap);

    static ID* FindAndLock(const SkBitmap& original, SkScalar scaleX,
                           SkScalar scaleY, SkBitmap* returnedBitmap);
    static ID* FindAndLockMip(const SkBitmap& original,
                              SkMipMap const** returnedMipMap);


    static ID* AddAndLock(uint32_t pixelGenerationID,
                          int32_t width,
                          int32_t height,
                          const SkBitmap& bitmap);

    static ID* AddAndLock(const SkBitmap& original, SkScalar scaleX,
                          SkScalar scaleY, const SkBitmap& bitmap);
    static ID* AddAndLockMip(const SkBitmap& original, const SkMipMap* mipMap);

    static void Unlock(ID*);

    static size_t GetTotalBytesUsed();
    static size_t GetTotalByteLimit();
    static size_t SetTotalByteLimit(size_t newLimit);

    static size_t SetSingleAllocationByteLimit(size_t);
    static size_t GetSingleAllocationByteLimit();

    static SkBitmap::Allocator* GetAllocator();

    /**
     *  Call SkDebugf() with diagnostic information about the state of the cache
     */
    static void Dump();

    ///////////////////////////////////////////////////////////////////////////

    /**
     *  Construct the cache to call DiscardableFactory when it
     *  allocates memory for the pixels. In this mode, the cache has
     *  not explicit budget, and so methods like getTotalBytesUsed()
     *  and getTotalByteLimit() will return 0, and setTotalByteLimit
     *  will ignore its argument and return 0.
     */
    SkScaledImageCache(DiscardableFactory);

    /**
     *  Construct the cache, allocating memory with malloc, and respect the
     *  byteLimit, purging automatically when a new image is added to the cache
     *  that pushes the total bytesUsed over the limit. Note: The limit can be
     *  changed at runtime with setTotalByteLimit.
     */
    SkScaledImageCache(size_t byteLimit);

    ~SkScaledImageCache();

    /**
     *  Search the cache for a matching bitmap (using generationID,
     *  width, and height as a search key). If found, return it in
     *  returnedBitmap, and return its ID pointer. Use the returned
     *  ptr to unlock the cache when you are done using
     *  returnedBitmap.
     *
     *  If a match is not found, returnedBitmap will be unmodifed, and
     *  NULL will be returned.
     *
     *  This is used if there is no scaling or subsetting, for example
     *  by SkLazyPixelRef.
     */
    ID* findAndLock(uint32_t pixelGenerationID, int32_t width, int32_t height,
                    SkBitmap* returnedBitmap);

    /**
     *  Search the cache for a scaled version of original. If found,
     *  return it in returnedBitmap, and return its ID pointer. Use
     *  the returned ptr to unlock the cache when you are done using
     *  returnedBitmap.
     *
     *  If a match is not found, returnedBitmap will be unmodifed, and
     *  NULL will be returned.
     */
    ID* findAndLock(const SkBitmap& original, SkScalar scaleX,
                    SkScalar scaleY, SkBitmap* returnedBitmap);
    ID* findAndLockMip(const SkBitmap& original,
                       SkMipMap const** returnedMipMap);

    /**
     *  To add a new bitmap (or mipMap) to the cache, call
     *  AddAndLock. Use the returned ptr to unlock the cache when you
     *  are done using scaled.
     *
     *  Use (generationID, width, and height) or (original, scaleX,
     *  scaleY) or (original) as a search key
     */
    ID* addAndLock(uint32_t pixelGenerationID, int32_t width, int32_t height,
                   const SkBitmap& bitmap);
    ID* addAndLock(const SkBitmap& original, SkScalar scaleX,
                   SkScalar scaleY, const SkBitmap& bitmap);
    ID* addAndLockMip(const SkBitmap& original, const SkMipMap* mipMap);

    /**
     *  Given a non-null ID ptr returned by either findAndLock or addAndLock,
     *  this releases the associated resources to be available to be purged
     *  if needed. After this, the cached bitmap should no longer be
     *  referenced by the caller.
     */
    void unlock(ID*);

    size_t getTotalBytesUsed() const { return fTotalBytesUsed; }
    size_t getTotalByteLimit() const { return fTotalByteLimit; }

    /**
     *  This is respected by SkBitmapProcState::possiblyScaleImage.
     *  0 is no maximum at all; this is the default.
     *  setSingleAllocationByteLimit() returns the previous value.
     */
    size_t setSingleAllocationByteLimit(size_t maximumAllocationSize);
    size_t getSingleAllocationByteLimit() const;
    /**
     *  Set the maximum number of bytes available to this cache. If the current
     *  cache exceeds this new value, it will be purged to try to fit within
     *  this new limit.
     */
    size_t setTotalByteLimit(size_t newLimit);

    SkBitmap::Allocator* allocator() const { return fAllocator; };

    /**
     *  Call SkDebugf() with diagnostic information about the state of the cache
     */
    void dump() const;

public:
    struct Rec;
    struct Key;
private:
    Rec*    fHead;
    Rec*    fTail;

    class Hash;
    Hash*   fHash;

    DiscardableFactory  fDiscardableFactory;
    // the allocator is NULL or one that matches discardables
    SkBitmap::Allocator* fAllocator;

    size_t  fTotalBytesUsed;
    size_t  fTotalByteLimit;
    size_t  fSingleAllocationByteLimit;
    int     fCount;

    Rec* findAndLock(uint32_t generationID, SkScalar sx, SkScalar sy,
                     const SkIRect& bounds);
    Rec* findAndLock(const Key& key);
    ID* addAndLock(Rec* rec);

    void purgeRec(Rec*);
    void purgeAsNeeded();

    // linklist management
    void moveToHead(Rec*);
    void addToHead(Rec*);
    void detach(Rec*);

    void init();    // called by constructors

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};
#endif
