
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrResourceCache_DEFINED
#define GrResourceCache_DEFINED

#include "GrTypes.h"
#include "GrTHashCache.h"

class GrResource;

// return true if a<b, or false if b<a
//
#define RET_IF_LT_OR_GT(a, b)   \
    do {                        \
        if ((a) < (b)) {        \
            return true;        \
        }                       \
        if ((b) < (a)) {        \
            return false;       \
        }                       \
    } while (0)

/**
 *  Helper class for GrResourceCache, the Key is used to identify src data for
 *  a resource. It is identified by 2 32bit data fields which can hold any
 *  data (uninterpreted by the cache) and a width/height.
 */
class GrResourceKey {
public:
    enum {
        kHashBits   = 7,
        kHashCount  = 1 << kHashBits,
        kHashMask   = kHashCount - 1
    };

    GrResourceKey(uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3) {
        fP[0] = p0;
        fP[1] = p1;
        fP[2] = p2;
        fP[3] = p3;
        this->computeHashIndex();
    }

    GrResourceKey(uint32_t v[4]) {
        memcpy(fP, v, 4 * sizeof(uint32_t));
        this->computeHashIndex();
    }

    GrResourceKey(const GrResourceKey& src) {
        memcpy(fP, src.fP, 4 * sizeof(uint32_t));
#if GR_DEBUG
        this->computeHashIndex();
        GrAssert(fHashIndex == src.fHashIndex);
#endif
        fHashIndex = src.fHashIndex;
    }

    //!< returns hash value [0..kHashMask] for the key
    int hashIndex() const { return fHashIndex; }

    friend bool operator==(const GrResourceKey& a, const GrResourceKey& b) {
        GR_DEBUGASSERT(-1 != a.fHashIndex && -1 != b.fHashIndex);
        return 0 == memcmp(a.fP, b.fP, 4 * sizeof(uint32_t));
    }

    friend bool operator!=(const GrResourceKey& a, const GrResourceKey& b) {
        GR_DEBUGASSERT(-1 != a.fHashIndex && -1 != b.fHashIndex);
        return !(a == b);
    }

    friend bool operator<(const GrResourceKey& a, const GrResourceKey& b) {
        RET_IF_LT_OR_GT(a.fP[0], b.fP[0]);
        RET_IF_LT_OR_GT(a.fP[1], b.fP[1]);
        RET_IF_LT_OR_GT(a.fP[2], b.fP[2]);
        return a.fP[3] < b.fP[3];
    }

    uint32_t getValue32(int i) const {
        GrAssert(i >=0 && i < 4);
        return fP[i];
    }
private:

    static uint32_t rol(uint32_t x) {
        return (x >> 24) | (x << 8);
    }
    static uint32_t ror(uint32_t x) {
        return (x >> 8) | (x << 24);
    }
    static uint32_t rohalf(uint32_t x) {
        return (x >> 16) | (x << 16);
    }

    void computeHashIndex() {
        uint32_t hash = fP[0] ^ rol(fP[1]) ^ ror(fP[2]) ^ rohalf(fP[3]);
        // this way to mix and reduce hash to its index may have to change
        // depending on how many bits we allocate to the index
        hash ^= hash >> 16;
        hash ^= hash >> 8;
        fHashIndex = hash & kHashMask;
    }

    uint32_t    fP[4];

    // this is computed from the fP... fields
    int         fHashIndex;

    friend class GrContext;
};

///////////////////////////////////////////////////////////////////////////////

class GrResourceEntry {
public:
    GrResource* resource() const { return fResource; }
    const GrResourceKey& key() const { return fKey; }

#if GR_DEBUG
    GrResourceEntry* next() const { return fNext; }
    GrResourceEntry* prev() const { return fPrev; }
#endif

#if GR_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

private:
    GrResourceEntry(const GrResourceKey& key, GrResource* resource);
    ~GrResourceEntry();

    bool isLocked() const { return fLockCount != 0; }
    void lock() { ++fLockCount; }
    void unlock() {
        GrAssert(fLockCount > 0);
        --fLockCount;
    }

    GrResourceKey    fKey;
    GrResource*      fResource;

    // track if we're in use, used when we need to purge
    // we only purge unlocked entries
    int fLockCount;

    // we're a dlinklist
    GrResourceEntry* fPrev;
    GrResourceEntry* fNext;

    friend class GrResourceCache;
};

///////////////////////////////////////////////////////////////////////////////

#include "GrTHashCache.h"

/**
 *  Cache of GrResource objects.
 *
 *  These have a corresponding GrResourceKey, built from 128bits identifying the
 *  resource.
 *
 *  The cache stores the entries in a double-linked list, which is its LRU.
 *  When an entry is "locked" (i.e. given to the caller), it is moved to the
 *  head of the list. If/when we must purge some of the entries, we walk the
 *  list backwards from the tail, since those are the least recently used.
 *
 *  For fast searches, we maintain a sorted array (based on the GrResourceKey)
 *  which we can bsearch. When a new entry is added, it is inserted into this
 *  array.
 *
 *  For even faster searches, a hash is computed from the Key. If there is
 *  a collision between two keys with the same hash, we fall back on the
 *  bsearch, and update the hash to reflect the most recent Key requested.
 */
class GrResourceCache {
public:
    GrResourceCache(int maxCount, size_t maxBytes);
    ~GrResourceCache();

    /**
     *  Return the current resource cache limits.
     *
     *  @param maxResource If non-null, returns maximum number of resources 
     *                     that can be held in the cache.
     *  @param maxBytes    If non-null, returns maximum number of bytes of
     *                         gpu memory that can be held in the cache.
     */
    void getLimits(int* maxResources, size_t* maxBytes) const;

    /**
     *  Specify the resource cache limits. If the current cache exceeds either
     *  of these, it will be purged (LRU) to keep the cache within these limits.
     *
     *  @param maxResources The maximum number of resources that can be held in
     *                      the cache.
     *  @param maxBytes     The maximum number of bytes of resource memory that
     *                      can be held in the cache.
     */
    void setLimits(int maxResource, size_t maxResourceBytes);

    /**
     * Returns the number of bytes consumed by cached resources.
     */
    size_t getCachedResourceBytes() const { return fEntryBytes; }

    /**
     * Controls whether locks should be nestable or not.
     */
    enum LockType {
        kNested_LockType,
        kSingle_LockType,
    };

    /**
     *  Search for an entry with the same Key. If found, "lock" it and return it.
     *  If not found, return null.
     */
    GrResourceEntry* findAndLock(const GrResourceKey&, LockType style);

    /**
     *  Create a new entry, based on the specified key and resource, and return
     *  its "locked" entry.
     *
     *  Ownership of the resource is transferred to the Entry, which will unref()
     *  it when we are purged or deleted.
     */
    GrResourceEntry* createAndLock(const GrResourceKey&, GrResource*);

    /**
     * Determines if the cache contains an entry matching a key. If a matching
     * entry exists but was detached then it will not be found.
     */
    bool hasKey(const GrResourceKey& key) const;

    /**
     * Detach removes an entry from the cache. This prevents the entry from
     * being found by a subsequent findAndLock() until it is reattached. The
     * entry still counts against the cache's budget and should be reattached
     * when exclusive access is no longer needed.
     */
    void detach(GrResourceEntry*);

    /**
     * Reattaches a resource to the cache and unlocks it. Allows it to be found
     * by a subsequent findAndLock or be purged (provided its lock count is
     * now 0.)
     */
    void reattachAndUnlock(GrResourceEntry*);

    /**
     *  When done with an entry, call unlock(entry) on it, which returns it to
     *  a purgable state.
     */
    void unlock(GrResourceEntry*);

    void removeAll();

#if GR_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

private:
    void internalDetach(GrResourceEntry*, bool);
    void attachToHead(GrResourceEntry*, bool);
    void purgeAsNeeded();

    class Key;
    GrTHashTable<GrResourceEntry, Key, 8> fCache;

    // manage the dlink list
    GrResourceEntry* fHead;
    GrResourceEntry* fTail;

    // our budget, used in purgeAsNeeded()
    int fMaxCount;
    size_t fMaxBytes;

    // our current stats, related to our budget
    int fEntryCount;
    int fUnlockedEntryCount;
    size_t fEntryBytes;
    int fClientDetachedCount;
    size_t fClientDetachedBytes;
    
    // prevents recursive purging
    bool fPurging;
};

///////////////////////////////////////////////////////////////////////////////

#if GR_DEBUG
    class GrAutoResourceCacheValidate {
    public:
        GrAutoResourceCacheValidate(GrResourceCache* cache) : fCache(cache) {
            cache->validate();
        }
        ~GrAutoResourceCacheValidate() {
            fCache->validate();
        }
    private:
        GrResourceCache* fCache;
    };
#else
    class GrAutoResourceCacheValidate {
    public:
        GrAutoResourceCacheValidate(GrResourceCache*) {}
    };
#endif

#endif
