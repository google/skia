/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkResourceCache_DEFINED
#define SkResourceCache_DEFINED

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
class SkResourceCache {
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
        typedef SkResourceCache::Key Key;

        Rec() {}
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

        friend class SkResourceCache;
    };

    typedef const Rec* ID;

    /**
     *  Callback function for find(). If called, the cache will have found a match for the
     *  specified Key, and will pass in the corresponding Rec, along with a caller-specified
     *  context. The function can read the data in Rec, and copy whatever it likes into context
     *  (casting context to whatever it really is).
     *
     *  The return value determines what the cache will do with the Rec. If the function returns
     *  true, then the Rec is considered "valid". If false is returned, the Rec will be considered
     *  "stale" and will be purged from the cache.
     */
    typedef bool (*VisitorProc)(const Rec&, void* context);

    /**
     *  Returns a locked/pinned SkDiscardableMemory instance for the specified
     *  number of bytes, or NULL on failure.
     */
    typedef SkDiscardableMemory* (*DiscardableFactory)(size_t bytes);

    /*
     *  The following static methods are thread-safe wrappers around a global
     *  instance of this cache.
     */

    /**
     *  Returns true if the visitor was called on a matching Key, and the visitor returned true.
     *
     *  Find() will search the cache for the specified Key. If no match is found, return false and
     *  do not call the VisitorProc. If a match is found, return whatever the visitor returns.
     *  Its return value is interpreted to mean:
     *      true  : Rec is valid
     *      false : Rec is "stale" -- the cache will purge it.
     */
    static bool Find(const Key& key, VisitorProc, void* context);
    static void Add(Rec*);

    static size_t GetTotalBytesUsed();
    static size_t GetTotalByteLimit();
    static size_t SetTotalByteLimit(size_t newLimit);

    static size_t SetSingleAllocationByteLimit(size_t);
    static size_t GetSingleAllocationByteLimit();

    static void PurgeAll();

    /**
     *  Returns the DiscardableFactory used by the global cache, or NULL.
     */
    static DiscardableFactory GetDiscardableFactory();

    /**
     * Use this allocator for bitmaps, so they can use ashmem when available.
     * Returns NULL if the ResourceCache has not been initialized with a DiscardableFactory.
     */
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
    SkResourceCache(DiscardableFactory);

    /**
     *  Construct the cache, allocating memory with malloc, and respect the
     *  byteLimit, purging automatically when a new image is added to the cache
     *  that pushes the total bytesUsed over the limit. Note: The limit can be
     *  changed at runtime with setTotalByteLimit.
     */
    explicit SkResourceCache(size_t byteLimit);
    ~SkResourceCache();

    /**
     *  Returns true if the visitor was called on a matching Key, and the visitor returned true.
     *
     *  find() will search the cache for the specified Key. If no match is found, return false and
     *  do not call the VisitorProc. If a match is found, return whatever the visitor returns.
     *  Its return value is interpreted to mean:
     *      true  : Rec is valid
     *      false : Rec is "stale" -- the cache will purge it.
     */
    bool find(const Key&, VisitorProc, void* context);
    void add(Rec*);

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

    void purgeAll() {
        this->purgeAsNeeded(true);
    }

    DiscardableFactory discardableFactory() const { return fDiscardableFactory; }
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

    void purgeAsNeeded(bool forcePurge = false);

    // linklist management
    void moveToHead(Rec*);
    void addToHead(Rec*);
    void detach(Rec*);
    void remove(Rec*);

    void init();    // called by constructors

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};
#endif
