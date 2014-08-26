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
    struct Key {
        // Call this to access your private contents. Must not use the address after calling init()
        void* writableContents() { return this + 1; }

        // must call this after your private data has been written.
        // length must be a multiple of 4
        void init(size_t length);

        // This is only valid after having called init().
        uint32_t hash() const { return fHash; }

        bool operator==(const Key& other) const {
            const uint32_t* a = this->as32();
            const uint32_t* b = other.as32();
            for (int i = 0; i < fCount32; ++i) {
                if (a[i] != b[i]) {
                    return false;
                }
            }
            return true;
        }

    private:
        // store fCount32 first, so we don't consider it in operator<
        int32_t  fCount32;  // 2 + user contents count32
        uint32_t fHash;
        /* uint32_t fContents32[] */

        const uint32_t* as32() const { return (const uint32_t*)this; }
        const uint32_t* as32SkipCount() const { return this->as32() + 1; }
    };

    struct Rec {
        typedef SkScaledImageCache::Key Key;

        Rec() : fLockCount(1) {}
        virtual ~Rec() {}

        uint32_t getHash() const { return this->getKey().hash(); }
        
        virtual const Key& getKey() const = 0;
        virtual size_t bytesUsed() const = 0;
        
        // for SkTDynamicHash::Traits
        static uint32_t Hash(const Key& key) { return key.hash(); }
        static const Key& GetKey(const Rec& rec) { return rec.getKey(); }

    private:
        Rec*    fNext;
        Rec*    fPrev;
        int32_t fLockCount;
        int32_t fPad;
        
        friend class SkScaledImageCache;
    };

    typedef const Rec* ID;

    /**
     *  Returns a locked/pinned SkDiscardableMemory instance for the specified
     *  number of bytes, or NULL on failure.
     */
    typedef SkDiscardableMemory* (*DiscardableFactory)(size_t bytes);

    /*
     *  The following static methods are thread-safe wrappers around a global
     *  instance of this cache.
     */

    static const Rec* FindAndLock(const Key& key);
    static const Rec* AddAndLock(Rec*);
    static void Add(Rec*);
    static void Unlock(ID);

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
    explicit SkScaledImageCache(size_t byteLimit);
    ~SkScaledImageCache();

    const Rec* findAndLock(const Key& key);
    const Rec* addAndLock(Rec*);
    void add(Rec*);

    /**
     *  Given a non-null ID ptr returned by either findAndLock or addAndLock,
     *  this releases the associated resources to be available to be purged
     *  if needed. After this, the cached bitmap should no longer be
     *  referenced by the caller.
     */
    void unlock(ID);

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
