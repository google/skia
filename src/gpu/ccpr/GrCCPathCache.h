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
#ifdef SK_DEBUG
    ~GrCCPathCache() {
        // Ensure the hash table and LRU list are still coherent.
        fHashTable.reset();
        SkASSERT(fLRU.isEmpty());
    }
#endif

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

    void evict(const GrCCPathCacheEntry*);

private:
    // Wrapper around a raw GrShape key that has a specialized operator==. Used by the hash table.
    struct HashKey {
        const uint32_t* fData;
    };
    friend bool operator==(const HashKey&, const HashKey&);

    // This is a special ref ptr for GrCCPathCacheEntry, used by the hash table. It can only be
    // moved, which guarantees the hash table holds exactly one reference for each entry. When a
    // HashNode goes out of scope, it therefore means the entry has been evicted from the cache.
    class HashNode : SkNoncopyable {
    public:
        static HashKey GetKey(const HashNode& node) { return GetKey(node.fEntry); }
        static HashKey GetKey(const GrCCPathCacheEntry*);
        static uint32_t Hash(HashKey);

        HashNode() = default;
        HashNode(GrCCPathCache*, const MaskTransform&, const GrShape&);
        HashNode(HashNode&& node) { fEntry = skstd::exchange(node.fEntry, nullptr); }
        ~HashNode();  // Called when fEntry (if not null) has been evicted from the cache.

        HashNode& operator=(HashNode&&);

        GrCCPathCacheEntry* entry() const { return fEntry; }

    private:
        GrCCPathCacheEntry* fEntry = nullptr;
        // The GrShape's unstyled key is stored as a variable-length footer to the 'fEntry'
        // allocation. GetKey provides access to it.
    };

    SkTHashTable<HashNode, HashKey> fHashTable;
    SkTInternalLList<GrCCPathCacheEntry> fLRU;
};

/**
 * This class stores all the data necessary to draw a specific path + matrix combination from their
 * corresponding cached atlas.
 */
class GrCCPathCacheEntry : public SkPathRef::GenIDChangeListener {
public:
    SK_DECLARE_INTERNAL_LLIST_INTERFACE(GrCCPathCacheEntry);

    ~GrCCPathCacheEntry() override;

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
    void initAsStashedAtlas(const GrUniqueKey& atlasKey, uint32_t contextUniqueID,
                            const SkIVector& atlasOffset, const SkRect& devBounds,
                            const SkRect& devBounds45, const SkIRect& devIBounds,
                            const SkIVector& maskShift);

    // Called once our path mask has been copied into a permanent, 8-bit atlas. This method points
    // the entry at the new atlas and updates the CachedAtlasInfo data.
    void updateToCachedAtlas(const GrUniqueKey& atlasKey, uint32_t contextUniqueID,
                             const SkIVector& newAtlasOffset, sk_sp<GrCCAtlas::CachedAtlasInfo>);

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

    GrCCPathCacheEntry(GrCCPathCache* cache, const MaskTransform& m)
            : fCacheWeakPtr(cache), fMaskTransform(m) {}

    // Resets this entry back to not having an atlas, and purges its previous atlas texture from the
    // resource cache if needed.
    void invalidateAtlas();

    // Called when our corresponding path is modified or deleted.
    void onChange() override;

    uint32_t fContextUniqueID;
    GrCCPathCache* fCacheWeakPtr;  // Gets manually reset to null by the path cache upon eviction.
    MaskTransform fMaskTransform;
    int fHitCount = 1;

    GrUniqueKey fAtlasKey;
    SkIVector fAtlasOffset;

    // If null, then we are referencing a "stashed" atlas (see initAsStashedAtlas()).
    sk_sp<GrCCAtlas::CachedAtlasInfo> fCachedAtlasInfo;

    SkRect fDevBounds;
    SkRect fDevBounds45;
    SkIRect fDevIBounds;

    // This field is for when a path gets drawn more than once during the same flush.
    const GrCCAtlas* fCurrFlushAtlas = nullptr;

    friend class GrCCPathCache;
    friend void GrCCPathProcessor::Instance::set(const GrCCPathCacheEntry&, const SkIVector&,
                                                 uint32_t, DoEvenOddFill);  // To access data.
};

inline void GrCCPathProcessor::Instance::set(const GrCCPathCacheEntry& entry,
                                             const SkIVector& shift, GrColor color,
                                             DoEvenOddFill doEvenOddFill) {
    float dx = (float)shift.fX, dy = (float)shift.fY;
    this->set(entry.fDevBounds.makeOffset(dx, dy), MakeOffset45(entry.fDevBounds45, dx, dy),
              entry.fAtlasOffset - shift, color, doEvenOddFill);
}

#endif
