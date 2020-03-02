/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathCache_DEFINED
#define GrCCPathCache_DEFINED

#include "include/private/SkIDChangeListener.h"
#include "include/private/SkTHash.h"
#include "src/core/SkExchange.h"
#include "src/core/SkTInternalLList.h"
#include "src/gpu/ccpr/GrCCAtlas.h"
#include "src/gpu/ccpr/GrCCPathProcessor.h"
#include "src/gpu/geometry/GrShape.h"

class GrCCPathCacheEntry;
class GrShape;

/**
 * This class implements an LRU cache that maps from GrShape to GrCCPathCacheEntry objects. Shapes
 * are only given one entry in the cache, so any time they are accessed with a different matrix, the
 * old entry gets evicted.
 */
class GrCCPathCache {
public:
    GrCCPathCache(uint32_t contextUniqueID);
    ~GrCCPathCache();

    class Key : public SkIDChangeListener {
    public:
        static sk_sp<Key> Make(uint32_t pathCacheUniqueID, int dataCountU32,
                               const void* data = nullptr);

        uint32_t pathCacheUniqueID() const { return fPathCacheUniqueID; }

        int dataSizeInBytes() const { return fDataSizeInBytes; }
        const uint32_t* data() const;

        void resetDataCountU32(int dataCountU32) {
            SkASSERT(dataCountU32 <= fDataReserveCountU32);
            fDataSizeInBytes = dataCountU32 * sizeof(uint32_t);
        }
        uint32_t* data();

        bool operator==(const Key& that) const {
            return fDataSizeInBytes == that.fDataSizeInBytes &&
                   !memcmp(this->data(), that.data(), fDataSizeInBytes);
        }

        // Called when our corresponding path is modified or deleted. Not threadsafe.
        void changed() override;

        // TODO(b/30449950): use sized delete once P0722R3 is available
        static void operator delete(void* p);

    private:
        Key(uint32_t pathCacheUniqueID, int dataCountU32)
                : fPathCacheUniqueID(pathCacheUniqueID)
                , fDataSizeInBytes(dataCountU32 * sizeof(uint32_t))
                SkDEBUGCODE(, fDataReserveCountU32(dataCountU32)) {
            SkASSERT(SK_InvalidUniqueID != fPathCacheUniqueID);
        }

        const uint32_t fPathCacheUniqueID;
        int fDataSizeInBytes;
        SkDEBUGCODE(const int fDataReserveCountU32);
        // The GrShape's unstyled key is stored as a variable-length footer to this class. GetKey
        // provides access to it.
    };

    // Stores the components of a transformation that affect a path mask (i.e. everything but
    // integer translation). During construction, any integer portions of the matrix's translate are
    // shaved off and returned to the caller. The caller is responsible for those integer shifts.
    struct MaskTransform {
        MaskTransform(const SkMatrix& m, SkIVector* shift);
        float fMatrix2x2[4];
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
        // Except on AOSP, cache hits must have matching subpixel portions of their view matrix.
        // On AOSP we follow after HWUI and ignore the subpixel translate.
        float fSubpixelTranslate[2];
#endif
    };

    // Represents a ref on a GrCCPathCacheEntry that should only be used during the current flush.
    class OnFlushEntryRef : SkNoncopyable {
    public:
        static OnFlushEntryRef OnFlushRef(GrCCPathCacheEntry*);
        OnFlushEntryRef() = default;
        OnFlushEntryRef(OnFlushEntryRef&& ref) : fEntry(skstd::exchange(ref.fEntry, nullptr)) {}
        ~OnFlushEntryRef();

        GrCCPathCacheEntry* get() const { return fEntry; }
        GrCCPathCacheEntry* operator->() const { return fEntry; }
        GrCCPathCacheEntry& operator*() const { return *fEntry; }
        explicit operator bool() const { return fEntry; }
        void operator=(OnFlushEntryRef&& ref) { fEntry = skstd::exchange(ref.fEntry, nullptr); }

    private:
        OnFlushEntryRef(GrCCPathCacheEntry* entry) : fEntry(entry) {}
        GrCCPathCacheEntry* fEntry = nullptr;
    };

    // Finds an entry in the cache that matches the given shape and transformation matrix.
    // 'maskShift' is filled with an integer post-translate that the caller must apply when drawing
    // the entry's mask to the device.
    //
    // NOTE: Shapes are only given one entry, so any time they are accessed with a new
    // transformation, the old entry gets evicted.
    OnFlushEntryRef find(GrOnFlushResourceProvider*, const GrShape&,
                         const SkIRect& clippedDrawBounds, const SkMatrix& viewMatrix,
                         SkIVector* maskShift);

    void doPreFlushProcessing();

    void purgeEntriesOlderThan(GrProxyProvider*, const GrStdSteadyClock::time_point& purgeTime);

    // As we evict entries from our local path cache, we accumulate a list of invalidated atlas
    // textures. This call purges the invalidated atlas textures from the mainline GrResourceCache.
    // This call is available with two different "provider" objects, to accomodate whatever might
    // be available at the callsite.
    void purgeInvalidatedAtlasTextures(GrOnFlushResourceProvider*);
    void purgeInvalidatedAtlasTextures(GrProxyProvider*);

private:
    // This is a special ref ptr for GrCCPathCacheEntry, used by the hash table. It provides static
    // methods for SkTHash, and can only be moved. This guarantees the hash table holds exactly one
    // reference for each entry. Also, when a HashNode goes out of scope, that means it is exiting
    // the hash table. We take that opportunity to remove it from the LRU list and do some cleanup.
    class HashNode : SkNoncopyable {
    public:
        static const Key& GetKey(const HashNode&);
        inline static uint32_t Hash(const Key& key) {
            return GrResourceKeyHash(key.data(), key.dataSizeInBytes());
        }

        HashNode() = default;
        HashNode(GrCCPathCache*, sk_sp<Key>, const MaskTransform&, const GrShape&);
        HashNode(HashNode&& node)
                : fPathCache(node.fPathCache), fEntry(std::move(node.fEntry)) {
            SkASSERT(!node.fEntry);
        }

        ~HashNode();

        void operator=(HashNode&& node);

        GrCCPathCacheEntry* entry() const { return fEntry.get(); }

    private:
        GrCCPathCache* fPathCache = nullptr;
        sk_sp<GrCCPathCacheEntry> fEntry;
    };

    GrStdSteadyClock::time_point quickPerFlushTimestamp() {
        // time_point::min() means it's time to update fPerFlushTimestamp with a newer clock read.
        if (GrStdSteadyClock::time_point::min() == fPerFlushTimestamp) {
            fPerFlushTimestamp = GrStdSteadyClock::now();
        }
        return fPerFlushTimestamp;
    }

    void evict(const GrCCPathCache::Key&, GrCCPathCacheEntry* = nullptr);

    // Evicts all the cache entries whose keys have been queued up in fInvalidatedKeysInbox via
    // SkPath listeners.
    void evictInvalidatedCacheKeys();

    const uint32_t fContextUniqueID;

    SkTHashTable<HashNode, const Key&> fHashTable;
    SkTInternalLList<GrCCPathCacheEntry> fLRU;
    SkMessageBus<sk_sp<Key>>::Inbox fInvalidatedKeysInbox;
    sk_sp<Key> fScratchKey;  // Reused for creating a temporary key in the find() method.

    // We only read the clock once per flush, and cache it in this variable. This prevents us from
    // excessive clock reads for cache timestamps that might degrade performance.
    GrStdSteadyClock::time_point fPerFlushTimestamp = GrStdSteadyClock::time_point::min();

    // As we evict entries from our local path cache, we accumulate lists of invalidated atlas
    // textures in these two members. We hold these until we purge them from the GrResourceCache
    // (e.g. via purgeInvalidatedAtlasTextures().)
    SkSTArray<4, sk_sp<GrTextureProxy>> fInvalidatedProxies;
    SkSTArray<4, GrUniqueKey> fInvalidatedProxyUniqueKeys;

    friend class GrCCCachedAtlas;  // To append to fInvalidatedProxies, fInvalidatedProxyUniqueKeys.

public:
    const SkTHashTable<HashNode, const Key&>& testingOnly_getHashTable() const;
    const SkTInternalLList<GrCCPathCacheEntry>& testingOnly_getLRU() const;
};

/**
 * This class stores all the data necessary to draw a specific path + matrix combination from their
 * corresponding cached atlas.
 */
class GrCCPathCacheEntry : public GrNonAtomicRef<GrCCPathCacheEntry> {
public:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrCCPathCacheEntry);

    ~GrCCPathCacheEntry() {
        SkASSERT(this->hasBeenEvicted());  // Should have called GrCCPathCache::evict().
        SkASSERT(!fCachedAtlas);
        SkASSERT(0 == fOnFlushRefCnt);
    }

    const GrCCPathCache::Key& cacheKey() const { SkASSERT(fCacheKey); return *fCacheKey; }

    // The number of flushes during which this specific entry (path + matrix combination) has been
    // pulled from the path cache. If a path is pulled from the cache more than once in a single
    // flush, the hit count is only incremented once.
    //
    // If the entry did not previously exist, its hit count will be 1.
    int hitCount() const { return fHitCount; }

    // The accumulative region of the path that has been drawn during the lifetime of this cache
    // entry (as defined by the 'clippedDrawBounds' parameter for GrCCPathCache::find).
    const SkIRect& hitRect() const { return fHitRect; }

    const GrCCCachedAtlas* cachedAtlas() const { return fCachedAtlas.get(); }

    const SkIRect& devIBounds() const { return fDevIBounds; }
    int width() const { return fDevIBounds.width(); }
    int height() const { return fDevIBounds.height(); }

    enum class ReleaseAtlasResult : bool {
        kNone,
        kDidInvalidateFromCache
    };

    // Called once our path has been rendered into the mainline CCPR (fp16, coverage count) atlas.
    // The caller will stash this atlas texture away after drawing, and during the next flush,
    // recover it and attempt to copy any paths that got reused into permanent 8-bit atlases.
    void setCoverageCountAtlas(
            GrOnFlushResourceProvider*, GrCCAtlas*, const SkIVector& atlasOffset,
            const GrOctoBounds& octoBounds, const SkIRect& devIBounds, const SkIVector& maskShift);

    // Called once our path mask has been copied into a permanent, 8-bit atlas. This method points
    // the entry at the new atlas and updates the GrCCCCachedAtlas data.
    ReleaseAtlasResult upgradeToLiteralCoverageAtlas(GrCCPathCache*, GrOnFlushResourceProvider*,
                                                     GrCCAtlas*, const SkIVector& newAtlasOffset);

private:
    using MaskTransform = GrCCPathCache::MaskTransform;

    GrCCPathCacheEntry(sk_sp<GrCCPathCache::Key> cacheKey, const MaskTransform& maskTransform)
            : fCacheKey(std::move(cacheKey)), fMaskTransform(maskTransform) {
    }

    bool hasBeenEvicted() const { return fCacheKey->shouldDeregister(); }

    // Resets this entry back to not having an atlas, and purges its previous atlas texture from the
    // resource cache if needed.
    ReleaseAtlasResult releaseCachedAtlas(GrCCPathCache*);

    sk_sp<GrCCPathCache::Key> fCacheKey;
    GrStdSteadyClock::time_point fTimestamp;
    int fHitCount = 0;
    SkIRect fHitRect = SkIRect::MakeEmpty();

    sk_sp<GrCCCachedAtlas> fCachedAtlas;
    SkIVector fAtlasOffset;

    MaskTransform fMaskTransform;
    GrOctoBounds fOctoBounds;
    SkIRect fDevIBounds;

    int fOnFlushRefCnt = 0;

    friend class GrCCPathCache;
    friend void GrCCPathProcessor::Instance::set(const GrCCPathCacheEntry&, const SkIVector&,
                                                 uint64_t color, GrFillRule);  // To access data.

public:
    int testingOnly_peekOnFlushRefCnt() const;
};

/**
 * Encapsulates the data for an atlas whose texture is stored in the mainline GrResourceCache. Many
 * instances of GrCCPathCacheEntry will reference the same GrCCCachedAtlas.
 *
 * We use this object to track the percentage of the original atlas pixels that could still ever
 * potentially be reused (i.e., those which still represent an extant path). When the percentage
 * of useful pixels drops below 50%, we purge the entire texture from the resource cache.
 *
 * This object also holds a ref on the atlas's actual texture proxy during flush. When
 * fOnFlushRefCnt decrements back down to zero, we release fOnFlushProxy and reset it back to null.
 */
class GrCCCachedAtlas : public GrNonAtomicRef<GrCCCachedAtlas> {
public:
    using ReleaseAtlasResult = GrCCPathCacheEntry::ReleaseAtlasResult;

    GrCCCachedAtlas(GrCCAtlas::CoverageType type, const GrUniqueKey& textureKey,
                    sk_sp<GrTextureProxy> onFlushProxy)
            : fCoverageType(type)
            , fTextureKey(textureKey)
            , fOnFlushProxy(std::move(onFlushProxy)) {}

    ~GrCCCachedAtlas() {
        SkASSERT(!fOnFlushProxy);
        SkASSERT(!fOnFlushRefCnt);
    }

    GrCCAtlas::CoverageType coverageType() const  { return fCoverageType; }
    const GrUniqueKey& textureKey() const { return fTextureKey; }

    GrTextureProxy* getOnFlushProxy() const { return fOnFlushProxy.get(); }

    void setOnFlushProxy(sk_sp<GrTextureProxy> proxy) {
        SkASSERT(!fOnFlushProxy);
        fOnFlushProxy = std::move(proxy);
    }

    void addPathPixels(int numPixels) { fNumPathPixels += numPixels; }
    ReleaseAtlasResult invalidatePathPixels(GrCCPathCache*, int numPixels);

    int peekOnFlushRefCnt() const { return fOnFlushRefCnt; }
    void incrOnFlushRefCnt(int count = 1) const {
        SkASSERT(count > 0);
        SkASSERT(fOnFlushProxy);
        fOnFlushRefCnt += count;
    }
    void decrOnFlushRefCnt(int count = 1) const;

private:
    const GrCCAtlas::CoverageType fCoverageType;
    const GrUniqueKey fTextureKey;

    int fNumPathPixels = 0;
    int fNumInvalidatedPathPixels = 0;
    bool fIsInvalidatedFromResourceCache = false;

    mutable sk_sp<GrTextureProxy> fOnFlushProxy;
    mutable int fOnFlushRefCnt = 0;

public:
    int testingOnly_peekOnFlushRefCnt() const;
};


inline GrCCPathCache::HashNode::HashNode(GrCCPathCache* pathCache, sk_sp<Key> key,
                                         const MaskTransform& m, const GrShape& shape)
        : fPathCache(pathCache)
        , fEntry(new GrCCPathCacheEntry(key, m)) {
    SkASSERT(shape.hasUnstyledKey());
    shape.addGenIDChangeListener(std::move(key));
}

inline const GrCCPathCache::Key& GrCCPathCache::HashNode::GetKey(
        const GrCCPathCache::HashNode& node) {
    return *node.entry()->fCacheKey;
}

inline GrCCPathCache::HashNode::~HashNode() {
    SkASSERT(!fEntry || fEntry->hasBeenEvicted());  // Should have called GrCCPathCache::evict().
}

inline void GrCCPathCache::HashNode::operator=(HashNode&& node) {
    SkASSERT(!fEntry || fEntry->hasBeenEvicted());  // Should have called GrCCPathCache::evict().
    fEntry = skstd::exchange(node.fEntry, nullptr);
}

inline void GrCCPathProcessor::Instance::set(
        const GrCCPathCacheEntry& entry, const SkIVector& shift, uint64_t color,
        GrFillRule fillRule) {
    float dx = (float)shift.fX, dy = (float)shift.fY;
    this->set(entry.fOctoBounds.makeOffset(dx, dy), entry.fAtlasOffset - shift, color, fillRule);
}

#endif
