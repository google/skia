
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrResourceCache_DEFINED
#define GrResourceCache_DEFINED

#include "GrConfig.h"
#include "GrTypes.h"
#include "GrTHashCache.h"
#include "GrBinHashKey.h"
#include "SkTInternalLList.h"

class GrResource;
class GrResourceEntry;

class GrResourceKey {
public:
    enum {
        kHashBits   = 7,
        kHashCount  = 1 << kHashBits,
        kHashMask   = kHashCount - 1
    };

    static GrCacheID::Domain ScratchDomain() {
        static const GrCacheID::Domain gDomain = GrCacheID::GenerateDomain();
        return gDomain;
    }

    /** Uniquely identifies the GrResource subclass in the key to avoid collisions
        across resource types. */
    typedef uint8_t ResourceType;

    /** Flags set by the GrResource subclass. */
    typedef uint8_t ResourceFlags;

    /** Generate a unique ResourceType */
    static ResourceType GenerateResourceType();

    /** Creates a key for resource */
    GrResourceKey(const GrCacheID& id, ResourceType type, ResourceFlags flags) {
        this->init(id.getDomain(), id.getKey(), type, flags);
    };

    GrResourceKey(const GrResourceKey& src) {
        fKey = src.fKey;
    }

    GrResourceKey() {
        fKey.fHashedKey.reset();
    }

    void reset(const GrCacheID& id, ResourceType type, ResourceFlags flags) {
        this->init(id.getDomain(), id.getKey(), type, flags);
    }

    //!< returns hash value [0..kHashMask] for the key
    int getHash() const {
        return fKey.fHashedKey.getHash() & kHashMask;
    }

    bool isScratch() const {
        return ScratchDomain() ==
            *reinterpret_cast<const GrCacheID::Domain*>(fKey.fHashedKey.getData() +
                                                        kCacheIDDomainOffset);
    }

    ResourceType getResourceType() const {
        return *reinterpret_cast<const ResourceType*>(fKey.fHashedKey.getData() +
                                                      kResourceTypeOffset);
    }

    ResourceFlags getResourceFlags() const {
        return *reinterpret_cast<const ResourceFlags*>(fKey.fHashedKey.getData() +
                                                       kResourceFlagsOffset);
    }

    int compare(const GrResourceKey& other) const {
        return fKey.fHashedKey.compare(other.fKey.fHashedKey);
    }

    static bool LT(const GrResourceKey& a, const GrResourceKey& b) {
        return a.compare(b) < 0;
    }

    static bool EQ(const GrResourceKey& a, const GrResourceKey& b) {
        return 0 == a.compare(b);
    }

    inline static bool LT(const GrResourceEntry& entry, const GrResourceKey& key);
    inline static bool EQ(const GrResourceEntry& entry, const GrResourceKey& key);
    inline static bool LT(const GrResourceEntry& a, const GrResourceEntry& b);
    inline static bool EQ(const GrResourceEntry& a, const GrResourceEntry& b);

private:
    enum {
        kCacheIDKeyOffset = 0,
        kCacheIDDomainOffset = kCacheIDKeyOffset + sizeof(GrCacheID::Key),
        kResourceTypeOffset = kCacheIDDomainOffset + sizeof(GrCacheID::Domain),
        kResourceFlagsOffset = kResourceTypeOffset + sizeof(ResourceType),
        kPadOffset = kResourceFlagsOffset + sizeof(ResourceFlags),
        kKeySize = SkAlign4(kPadOffset),
        kPadSize = kKeySize - kPadOffset
    };

    void init(const GrCacheID::Domain domain,
              const GrCacheID::Key& key,
              ResourceType type,
              ResourceFlags flags) {
        union {
            uint8_t  fKey8[kKeySize];
            uint32_t fKey32[kKeySize / 4];
        } keyData;

        uint8_t* k = keyData.fKey8;
        memcpy(k + kCacheIDKeyOffset, key.fData8, sizeof(GrCacheID::Key));
        memcpy(k + kCacheIDDomainOffset, &domain, sizeof(GrCacheID::Domain));
        memcpy(k + kResourceTypeOffset, &type, sizeof(ResourceType));
        memcpy(k + kResourceFlagsOffset, &flags, sizeof(ResourceFlags));
        memset(k + kPadOffset, 0, kPadSize);
        fKey.fHashedKey.setKeyData(keyData.fKey32);
    }

    struct Key;
    typedef GrTBinHashKey<Key, kKeySize> HashedKey;

    struct Key {
        int compare(const HashedKey& hashedKey) const {
            return fHashedKey.compare(hashedKey);
        }

        HashedKey fHashedKey;
    };

    Key fKey;
};

///////////////////////////////////////////////////////////////////////////////

class GrResourceEntry {
public:
    GrResource* resource() const { return fResource; }
    const GrResourceKey& key() const { return fKey; }

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

private:
    GrResourceEntry(const GrResourceKey& key, GrResource* resource);
    ~GrResourceEntry();

    GrResourceKey    fKey;
    GrResource*      fResource;

    // we're a linked list
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrResourceEntry);

    friend class GrResourceCache;
    friend class GrDLinkedList;
};

bool GrResourceKey::LT(const GrResourceEntry& entry, const GrResourceKey& key) {
    return LT(entry.key(), key);
}

bool GrResourceKey::EQ(const GrResourceEntry& entry, const GrResourceKey& key) {
    return EQ(entry.key(), key);
}

bool GrResourceKey::LT(const GrResourceEntry& a, const GrResourceEntry& b) {
    return LT(a.key(), b.key());
}

bool GrResourceKey::EQ(const GrResourceEntry& a, const GrResourceEntry& b) {
    return EQ(a.key(), b.key());
}

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
 *
 *  It is a goal to make the GrResourceCache the central repository and bookkeeper
 *  of all resources. It should replace the linked list of GrResources that
 *  GrGpu uses to call abandon/release.
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
     *                     gpu memory that can be held in the cache.
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
    void setLimits(int maxResources, size_t maxResourceBytes);

    /**
     *  The callback function used by the cache when it is still over budget
     *  after a purge. The passed in 'data' is the same 'data' handed to
     *  setOverbudgetCallback. The callback returns true if some resources
     *  have been freed.
     */
    typedef bool (*PFOverbudgetCB)(void* data);

    /**
     *  Set the callback the cache should use when it is still over budget
     *  after a purge. The 'data' provided here will be passed back to the
     *  callback. Note that the cache will attempt to purge any resources newly
     *  freed by the callback.
     */
    void setOverbudgetCallback(PFOverbudgetCB overbudgetCB, void* data) {
        fOverbudgetCB = overbudgetCB;
        fOverbudgetData = data;
    }

    /**
     * Returns the number of bytes consumed by cached resources.
     */
    size_t getCachedResourceBytes() const { return fEntryBytes; }

    // For a found or added resource to be completely exclusive to the caller
    // both the kNoOtherOwners and kHide flags need to be specified
    enum OwnershipFlags {
        kNoOtherOwners_OwnershipFlag = 0x1, // found/added resource has no other owners
        kHide_OwnershipFlag = 0x2  // found/added resource is hidden from future 'find's
    };

    /**
     *  Search for an entry with the same Key. If found, return it.
     *  If not found, return null.
     *  If ownershipFlags includes kNoOtherOwners and a resource is returned
     *  then that resource has no other refs to it.
     *  If ownershipFlags includes kHide and a resource is returned then that
     *  resource will not be returned from future 'find' calls until it is
     *  'freed' (and recycled) or makeNonExclusive is called.
     *  For a resource to be completely exclusive to a caller both kNoOtherOwners
     *  and kHide must be specified.
     */
    GrResource* find(const GrResourceKey& key,
                     uint32_t ownershipFlags = 0);

    /**
     *  Add the new resource to the cache (by creating a new cache entry based
     *  on the provided key and resource).
     *
     *  Ownership of the resource is transferred to the resource cache,
     *  which will unref() it when it is purged or deleted.
     *
     *  If ownershipFlags includes kHide, subsequent calls to 'find' will not
     *  return 'resource' until it is 'freed' (and recycled) or makeNonExclusive
     *  is called.
     */
    void addResource(const GrResourceKey& key,
                     GrResource* resource,
                     uint32_t ownershipFlags = 0);

    /**
     * Determines if the cache contains an entry matching a key. If a matching
     * entry exists but was detached then it will not be found.
     */
    bool hasKey(const GrResourceKey& key) const { return NULL != fCache.find(key); }

    /**
     * Hide 'entry' so that future searches will not find it. Such
     * hidden entries will not be purged. The entry still counts against
     * the cache's budget and should be made non-exclusive when exclusive access
     * is no longer needed.
     */
    void makeExclusive(GrResourceEntry* entry);

    /**
     * Restore 'entry' so that it can be found by future searches. 'entry'
     * will also be purgeable (provided its lock count is now 0.)
     */
    void makeNonExclusive(GrResourceEntry* entry);

    /**
     * Remove a resource from the cache and delete it!
     */
    void deleteResource(GrResourceEntry* entry);

    /**
     * Removes every resource in the cache that isn't locked.
     */
    void purgeAllUnlocked();

    /**
     * Allow cache to purge unused resources to obey resource limitations
     * Note: this entry point will be hidden (again) once totally ref-driven
     * cache maintenance is implemented. Note that the overbudget callback
     * will be called if the initial purge doesn't get the cache under
     * its budget.
     *
     * extraCount and extraBytes are added to the current resource allocation
     * to make sure enough room is available for future additions (e.g,
     * 10MB across 10 textures is about to be added).
     */
    void purgeAsNeeded(int extraCount = 0, size_t extraBytes = 0);

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

#if GR_CACHE_STATS
    void printStats();
#endif

private:
    enum BudgetBehaviors {
        kAccountFor_BudgetBehavior,
        kIgnore_BudgetBehavior
    };

    void internalDetach(GrResourceEntry*, BudgetBehaviors behavior = kAccountFor_BudgetBehavior);
    void attachToHead(GrResourceEntry*, BudgetBehaviors behavior = kAccountFor_BudgetBehavior);

    void removeInvalidResource(GrResourceEntry* entry);

    GrTHashTable<GrResourceEntry, GrResourceKey, 8> fCache;

    // We're an internal doubly linked list
    typedef SkTInternalLList<GrResourceEntry> EntryList;
    EntryList      fList;

#ifdef SK_DEBUG
    // These objects cannot be returned by a search
    EntryList      fExclusiveList;
#endif

    // our budget, used in purgeAsNeeded()
    int            fMaxCount;
    size_t         fMaxBytes;

    // our current stats, related to our budget
#if GR_CACHE_STATS
    int            fHighWaterEntryCount;
    size_t         fHighWaterEntryBytes;
    int            fHighWaterClientDetachedCount;
    size_t         fHighWaterClientDetachedBytes;
#endif

    int            fEntryCount;
    size_t         fEntryBytes;
    int            fClientDetachedCount;
    size_t         fClientDetachedBytes;

    // prevents recursive purging
    bool           fPurging;

    PFOverbudgetCB fOverbudgetCB;
    void*          fOverbudgetData;

    void internalPurge(int extraCount, size_t extraBytes);

#ifdef SK_DEBUG
    static size_t countBytes(const SkTInternalLList<GrResourceEntry>& list);
#endif
};

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
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
