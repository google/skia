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

    static ID* FindAndLock(const SkBitmap& original, SkScalar scaleX,
                           SkScalar scaleY, SkBitmap* scaled);
    static ID* FindAndLockMip(const SkBitmap& original, SkMipMap const**);

    static ID* AddAndLock(const SkBitmap& original, SkScalar scaleX,
                          SkScalar scaleY, const SkBitmap& scaled);
    static ID* AddAndLockMip(const SkBitmap& original, const SkMipMap*);

    static void Unlock(ID*);

    static size_t GetBytesUsed();
    static size_t GetByteLimit();
    static size_t SetByteLimit(size_t newLimit);

    ///////////////////////////////////////////////////////////////////////////

    SkScaledImageCache(size_t byteLimit);
    ~SkScaledImageCache();

    /**
     *  Search the cache for a scaled version of original. If found, return it
     *  in scaled, and return its ID pointer. Use the returned ptr to unlock
     *  the cache when you are done using scaled.
     *
     *  If a match is not found, scaled will be unmodifed, and NULL will be
     *  returned.
     */
    ID* findAndLock(const SkBitmap& original, SkScalar scaleX,
                    SkScalar scaleY, SkBitmap* scaled);
    ID* findAndLockMip(const SkBitmap& original, SkMipMap const**);

    /**
     *  To add a new (scaled) bitmap to the cache, call AddAndLock. Use the
     *  returned ptr to unlock the cache when you are done using scaled.
     */
    ID* addAndLock(const SkBitmap& original, SkScalar scaleX,
                   SkScalar scaleY, const SkBitmap& scaled);
    ID* addAndLockMip(const SkBitmap& original, const SkMipMap*);

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
private:
    Rec*    fHead;
    Rec*    fTail;

    class Hash;
    Hash*   fHash;

    size_t  fBytesUsed;
    size_t  fByteLimit;
    int     fCount;

    Rec* findAndLock(const SkBitmap& original, SkScalar sx, SkScalar sy);

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
