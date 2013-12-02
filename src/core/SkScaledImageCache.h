/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScaledImageCache_DEFINED
#define SkScaledImageCache_DEFINED

#include "SkBitmap.h"

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

    static size_t GetBytesUsed();
    static size_t GetByteLimit();
    static size_t SetByteLimit(size_t newLimit);

    ///////////////////////////////////////////////////////////////////////////

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

    size_t getBytesUsed() const { return fBytesUsed; }
    size_t getByteLimit() const { return fByteLimit; }

    /**
     *  Set the maximum number of bytes available to this cache. If the current
     *  cache exceeds this new value, it will be purged to try to fit within
     *  this new limit.
     */
    size_t setByteLimit(size_t newLimit);

public:
    struct Rec;
    struct Key;
private:
    Rec*    fHead;
    Rec*    fTail;

    class Hash;
    Hash*   fHash;

    size_t  fBytesUsed;
    size_t  fByteLimit;
    int     fCount;

    Rec* findAndLock(uint32_t generationID, SkScalar sx, SkScalar sy,
                     const SkIRect& bounds);
    Rec* findAndLock(const Key& key);
    ID* addAndLock(Rec* rec);

    void purgeAsNeeded();

    // linklist management
    void moveToHead(Rec*);
    void addToHead(Rec*);
    void detach(Rec*);
#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};
#endif
