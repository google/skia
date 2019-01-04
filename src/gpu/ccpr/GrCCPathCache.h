/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPathCache_DEFINED
#define GrCCPathCache_DEFINED

#include "SkExchange.h"
#include "SkTHash.h"
#include "SkTInternalLList.h"
#include "ccpr/GrCCAtlas.h"
#include "ccpr/GrCCPathProcessor.h"

class GrCCPathCacheEntry;
class GrShape;

/**
 * This class implements an LRU cache that maps from GrShape to GrCCPathCacheEntry objects. Shapes
 * are only given one entry in the cache, so any time they are accessed with a different matrix, the
 * old entry gets evicted.
 */
class GrCCPathCache {
public:
    GrCCPathCache();
    ~GrCCPathCache();

    class Key : public SkPathRef::GenIDChangeListener {
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

        bool operator==(const Key&) const;

        // Called when our corresponding path is modified or deleted. Not threadsafe.
        void onChange() override;

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

    enum class CreateIfAbsent : bool {
        kNo = false,
        kYes = true
    };

    // Finds an entry in the cache. Shapes are only given one entry, so any time they are accessed
    // with a different MaskTransform, the old entry gets evicted.
    sk_sp<GrCCPathCacheEntry> find(const GrShape&, const MaskTransform&,
                                   CreateIfAbsent = CreateIfAbsent::kNo);

    void doPostFlushProcessing();
    void purgeEntriesOlderThan(const GrStdSteadyClock::time_point& purgeTime);

private:
    // This is a special ref ptr for GrCCPathCacheEntry, used by the hash table. It provides static
    // methods for SkTHash, and can only be moved. This guarantees the hash table holds exactly one
    // reference for each entry. Also, when a HashNode goes out of scope, that means it is exiting
    // the hash table. We take that opportunity to remove it from the LRU list and do some cleanup.
    class HashNode : SkNoncopyable {
    public:
        static const Key& GetKey(const HashNode&);
        static uint32_t Hash(const Key&);

        HashNode() = default;
        HashNode(GrCCPathCache*, sk_sp<Key>, const MaskTransform&, const GrShape&);
        HashNode(HashNode&& node)
                : fPathCache(node.fPathCache), fEntry(std::move(node.fEntry)) {
            SkASSERT(!node.fEntry);
        }

        ~HashNode();

        HashNode& operator=(HashNode&& node);

        GrCCPathCacheEntry* entry() const { return fEntry.get(); }

    private:
        void willExitHashTable();

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

    void evict(const GrCCPathCache::Key& key) {
        fHashTable.remove(key);  // HashNode::willExitHashTable() takes care of the rest.
    }

    void purgeInvalidatedKeys();

    SkTHashTable<HashNode, const GrCCPathCache::Key&> fHashTable;
    SkTInternalLList<GrCCPathCacheEntry> fLRU;
    SkMessageBus<sk_sp<Key>>::Inbox fInvalidatedKeysInbox;
    sk_sp<Key> fScratchKey;  // Reused for creating a temporary key in the find() method.

    // We only read the clock once per flush, and cache it in this variable. This prevents us from
    // excessive clock reads for cache timestamps that might degrade performance.
    GrStdSteadyClock::time_point fPerFlushTimestamp = GrStdSteadyClock::time_point::min();
};

/**
 * This class stores all the data necessary to draw a specific path + matrix combination from their
 * corresponding cached atlas.
 */
class GrCCPathCacheEntry : public GrNonAtomicRef<GrCCPathCacheEntry> {
public:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrCCPathCacheEntry);

    ~GrCCPathCacheEntry() {
        SkASSERT(!fCurrFlushAtlas);  // Client is required to reset fCurrFlushAtlas back to null.
        this->invalidateAtlas();
    }

    // The number of times this specific entry (path + matrix combination) has been pulled from
    // the path cache. As long as the caller does exactly one lookup per draw, this translates to
    // the number of times the path has been drawn with a compatible matrix.
    //
    // If the entry did not previously exist and was created during
    // GrCCPathCache::find(.., CreateIfAbsent::kYes), its hit count will be 1.
    int hitCount() const { return fHitCount; }

    // Does this entry reference a permanent, 8-bit atlas that resides in the resource cache?
    // (i.e. not a temporarily-stashed, fp16 coverage count atlas.)
    bool hasCachedAtlas() const { return SkToBool(fCachedAtlasInfo); }

    const SkIRect& devIBounds() const { return fDevIBounds; }
    int width() const { return fDevIBounds.width(); }
    int height() const { return fDevIBounds.height(); }

    // Called once our path has been rendered into the mainline CCPR (fp16, coverage count) atlas.
    // The caller will stash this atlas texture away after drawing, and during the next flush,
    // recover it and attempt to copy any paths that got reused into permanent 8-bit atlases.
    void initAsStashedAtlas(const GrUniqueKey& atlasKey, const SkIVector& atlasOffset,
                            const SkRect& devBounds, const SkRect& devBounds45,
                            const SkIRect& devIBounds, const SkIVector& maskShift);

    // Called once our path mask has been copied into a permanent, 8-bit atlas. This method points
    // the entry at the new atlas and updates the CachedAtlasInfo data.
    void updateToCachedAtlas(const GrUniqueKey& atlasKey, const SkIVector& newAtlasOffset,
                             sk_sp<GrCCAtlas::CachedAtlasInfo>);

    const GrUniqueKey& atlasKey() const { return fAtlasKey; }

    void resetAtlasKeyAndInfo() {
        fAtlasKey.reset();
        fCachedAtlasInfo.reset();
    }

    // This is a utility for the caller to detect when a path gets drawn more than once during the
    // same flush, with compatible matrices. Before adding a path to an atlas, the caller may check
    // here to see if they have already placed the path previously during the same flush. The caller
    // is required to reset all currFlushAtlas references back to null before any subsequent flush.
    void setCurrFlushAtlas(const GrCCAtlas* currFlushAtlas) {
        // This should not get called more than once in a single flush. Once fCurrFlushAtlas is
        // non-null, it can only be set back to null (once the flush is over).
        SkASSERT(!fCurrFlushAtlas || !currFlushAtlas);
        fCurrFlushAtlas = currFlushAtlas;
    }
    const GrCCAtlas* currFlushAtlas() const { return fCurrFlushAtlas; }

private:
    using MaskTransform = GrCCPathCache::MaskTransform;

    GrCCPathCacheEntry(sk_sp<GrCCPathCache::Key> cacheKey, const MaskTransform& maskTransform)
            : fCacheKey(std::move(cacheKey)), fMaskTransform(maskTransform) {
    }

    // Resets this entry back to not having an atlas, and purges its previous atlas texture from the
    // resource cache if needed.
    void invalidateAtlas();

    sk_sp<GrCCPathCache::Key> fCacheKey;

    GrStdSteadyClock::time_point fTimestamp;
    int fHitCount = 0;
    MaskTransform fMaskTransform;

    GrUniqueKey fAtlasKey;
    SkIVector fAtlasOffset;

    SkRect fDevBounds;
    SkRect fDevBounds45;
    SkIRect fDevIBounds;

    // If null, then we are referencing a "stashed" atlas (see initAsStashedAtlas()).
    sk_sp<GrCCAtlas::CachedAtlasInfo> fCachedAtlasInfo;

    // This field is for when a path gets drawn more than once during the same flush.
    const GrCCAtlas* fCurrFlushAtlas = nullptr;

    friend class GrCCPathCache;
    friend void GrCCPathProcessor::Instance::set(const GrCCPathCacheEntry&, const SkIVector&,
                                                 GrColor, DoEvenOddFill);  // To access data.
};

inline void GrCCPathProcessor::Instance::set(const GrCCPathCacheEntry& entry,
                                             const SkIVector& shift, GrColor color,
                                             DoEvenOddFill doEvenOddFill) {
    float dx = (float)shift.fX, dy = (float)shift.fY;
    this->set(entry.fDevBounds.makeOffset(dx, dy), MakeOffset45(entry.fDevBounds45, dx, dy),
              entry.fAtlasOffset - shift, color, doEvenOddFill);
}

#endif
