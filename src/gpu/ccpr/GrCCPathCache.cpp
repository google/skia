/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathCache.h"

#include "GrShape.h"
#include "SkNx.h"

DECLARE_SKMESSAGEBUS_MESSAGE(sk_sp<GrCCPathCacheEntry>);

static inline bool SkShouldPostMessageToBus(
        const sk_sp<GrCCPathCacheEntry>& entry, uint32_t msgBusUniqueID) {
    return entry->pathCacheUniqueID() == msgBusUniqueID;
}

// The maximum number of cache entries we allow in our own cache.
static constexpr int kMaxCacheCount = 1 << 16;

GrCCPathCache::MaskTransform::MaskTransform(const SkMatrix& m, SkIVector* shift)
        : fMatrix2x2{m.getScaleX(), m.getSkewX(), m.getSkewY(), m.getScaleY()} {
    SkASSERT(!m.hasPerspective());
    Sk2f translate = Sk2f(m.getTranslateX(), m.getTranslateY());
    Sk2f transFloor;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    // On Android framework we pre-round view matrix translates to integers for better caching.
    transFloor = translate;
#else
    transFloor = translate.floor();
    (translate - transFloor).store(fSubpixelTranslate);
#endif
    shift->set((int)transFloor[0], (int)transFloor[1]);
    SkASSERT((float)shift->fX == transFloor[0]);  // Make sure transFloor had integer values.
    SkASSERT((float)shift->fY == transFloor[1]);
}

inline static bool fuzzy_equals(const GrCCPathCache::MaskTransform& a,
                                const GrCCPathCache::MaskTransform& b) {
    if ((Sk4f::Load(a.fMatrix2x2) != Sk4f::Load(b.fMatrix2x2)).anyTrue()) {
        return false;
    }
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    if (((Sk2f::Load(a.fSubpixelTranslate) -
          Sk2f::Load(b.fSubpixelTranslate)).abs() > 1.f/256).anyTrue()) {
        return false;
    }
#endif
    return true;
}

namespace {

// Produces a key that accounts both for a shape's path geometry, as well as any stroke/style.
class WriteStyledKey {
public:
    static constexpr int kStyledKeySizeInBytesIdx = 0;
    static constexpr int kStrokeWidthIdx = 1;
    static constexpr int kStrokeMiterIdx = 2;
    static constexpr int kStrokeCapJoinIdx = 3;
    static constexpr int kShapeUnstyledKeyIdx = 4;

    static constexpr int kStrokeKeyCount = 3;  // [width, miterLimit, cap|join].

    WriteStyledKey(const GrShape& shape) : fShapeUnstyledKeyCount(shape.unstyledKeySize()) {}

    // Returns the total number of uint32_t's to allocate for the key.
    int allocCountU32() const { return kShapeUnstyledKeyIdx + fShapeUnstyledKeyCount; }

    // Writes the key to out[].
    void write(const GrShape& shape, uint32_t* out) {
        out[kStyledKeySizeInBytesIdx] =
                (kStrokeKeyCount + fShapeUnstyledKeyCount) * sizeof(uint32_t);

        // Stroke key.
        // We don't use GrStyle::WriteKey() because it does not account for hairlines.
        // http://skbug.com/8273
        SkASSERT(!shape.style().hasPathEffect());
        const SkStrokeRec& stroke = shape.style().strokeRec();
        if (stroke.isFillStyle()) {
            // Use a value for width that won't collide with a valid fp32 value >= 0.
            out[kStrokeWidthIdx] = ~0;
            out[kStrokeMiterIdx] = out[kStrokeCapJoinIdx] = 0;
        } else {
            float width = stroke.getWidth(), miterLimit = stroke.getMiter();
            memcpy(&out[kStrokeWidthIdx], &width, sizeof(float));
            memcpy(&out[kStrokeMiterIdx], &miterLimit, sizeof(float));
            out[kStrokeCapJoinIdx] = (stroke.getCap() << 16) | stroke.getJoin();
            GR_STATIC_ASSERT(sizeof(out[kStrokeWidthIdx]) == sizeof(float));
        }

        // Shape unstyled key.
        shape.writeUnstyledKey(&out[kShapeUnstyledKeyIdx]);
    }

private:
    int fShapeUnstyledKeyCount;
};

}

inline GrCCPathCache::HashNode::HashNode(uint32_t pathCacheUniqueID, const MaskTransform& m,
                                         const GrShape& shape) {
    SkASSERT(shape.hasUnstyledKey());

    WriteStyledKey writeKey(shape);
    void* memory = ::operator new (sizeof(GrCCPathCacheEntry) +
                                   writeKey.allocCountU32() * sizeof(uint32_t));
    fEntry.reset(new (memory) GrCCPathCacheEntry(pathCacheUniqueID, m));

    // The shape key is a variable-length footer to the entry allocation.
    uint32_t* keyData = (uint32_t*)((char*)memory + sizeof(GrCCPathCacheEntry));
    writeKey.write(shape, keyData);
}

inline bool operator==(const GrCCPathCache::HashKey& key1, const GrCCPathCache::HashKey& key2) {
    return key1.fData[0] == key2.fData[0] && !memcmp(&key1.fData[1], &key2.fData[1], key1.fData[0]);
}

inline GrCCPathCache::HashKey GrCCPathCache::HashNode::GetKey(const GrCCPathCacheEntry* entry) {
    // The shape key is a variable-length footer to the entry allocation.
    return HashKey{(const uint32_t*)((const char*)entry + sizeof(GrCCPathCacheEntry))};
}

inline uint32_t GrCCPathCache::HashNode::Hash(HashKey key) {
    return GrResourceKeyHash(&key.fData[1], key.fData[0]);
}

#ifdef SK_DEBUG
GrCCPathCache::~GrCCPathCache() {
    // Ensure the hash table and LRU list are still coherent.
    int lruCount = 0;
    for (const GrCCPathCacheEntry* entry : fLRU) {
        SkASSERT(fHashTable.find(HashNode::GetKey(entry))->entry() == entry);
        ++lruCount;
    }
    SkASSERT(fHashTable.count() == lruCount);
}
#endif

sk_sp<GrCCPathCacheEntry> GrCCPathCache::find(const GrShape& shape, const MaskTransform& m,
                                              CreateIfAbsent createIfAbsent) {
    if (!shape.hasUnstyledKey()) {
        return nullptr;
    }

    WriteStyledKey writeKey(shape);
    SkAutoSTMalloc<GrShape::kMaxKeyFromDataVerbCnt * 4, uint32_t> keyData(writeKey.allocCountU32());
    writeKey.write(shape, keyData.get());

    GrCCPathCacheEntry* entry = nullptr;
    if (HashNode* node = fHashTable.find({keyData.get()})) {
        entry = node->entry();
        SkASSERT(fLRU.isInList(entry));
        if (fuzzy_equals(m, entry->fMaskTransform)) {
            ++entry->fHitCount;  // The path was reused with a compatible matrix.
        } else if (CreateIfAbsent::kYes == createIfAbsent && entry->unique()) {
            // This entry is unique: we can recycle it instead of deleting and malloc-ing a new one.
            entry->fMaskTransform = m;
            entry->fHitCount = 1;
            entry->invalidateAtlas();
            SkASSERT(!entry->fCurrFlushAtlas);  // Should be null because 'entry' is unique.
        } else {
            this->evict(entry);
            entry = nullptr;
        }
    }

    if (!entry) {
        if (CreateIfAbsent::kNo == createIfAbsent) {
            return nullptr;
        }
        if (fHashTable.count() >= kMaxCacheCount) {
            this->evict(fLRU.tail());  // We've exceeded our limit.
        }
        entry = fHashTable.set(HashNode(fInvalidatedEntriesInbox.uniqueID(), m, shape))->entry();
        shape.addGenIDChangeListener(sk_ref_sp(entry));
        SkASSERT(fHashTable.count() <= kMaxCacheCount);
    } else {
        fLRU.remove(entry);  // Will be re-added at head.
    }

    fLRU.addToHead(entry);
    return sk_ref_sp(entry);
}

void GrCCPathCache::evict(GrCCPathCacheEntry* entry) {
    // Has the entry already been evicted? (SkPaths can post eviction messages from any thread.)
    if (entry->shouldUnregisterFromPath()) {
        SkASSERT(!fLRU.isInList(entry));
        return;
    }

    entry->markShouldUnregisterFromPath();
    fLRU.remove(entry);
    fHashTable.remove(HashNode::GetKey(entry));  // Do this last, as it might delete the entry.
}

void GrCCPathCache::purgeAsNeeded() {
    SkTArray<sk_sp<GrCCPathCacheEntry>> invalidatedEntries;
    fInvalidatedEntriesInbox.poll(&invalidatedEntries);
    for (const sk_sp<GrCCPathCacheEntry>& entry : invalidatedEntries) {
        this->evict(entry.get());
    }
}


GrCCPathCacheEntry::~GrCCPathCacheEntry() {
    SkASSERT(!fCurrFlushAtlas);  // Client is required to reset fCurrFlushAtlas back to null.
    this->invalidateAtlas();
}

void GrCCPathCacheEntry::initAsStashedAtlas(const GrUniqueKey& atlasKey,
                                            const SkIVector& atlasOffset, const SkRect& devBounds,
                                            const SkRect& devBounds45, const SkIRect& devIBounds,
                                            const SkIVector& maskShift) {
    SkASSERT(atlasKey.isValid());
    SkASSERT(!fCurrFlushAtlas);  // Otherwise we should reuse the atlas from last time.

    fAtlasKey = atlasKey;
    fAtlasOffset = atlasOffset + maskShift;
    SkASSERT(!fCachedAtlasInfo);  // Otherwise they should have reused the cached atlas instead.

    float dx = (float)maskShift.fX, dy = (float)maskShift.fY;
    fDevBounds = devBounds.makeOffset(-dx, -dy);
    fDevBounds45 = GrCCPathProcessor::MakeOffset45(devBounds45, -dx, -dy);
    fDevIBounds = devIBounds.makeOffset(-maskShift.fX, -maskShift.fY);
}

void GrCCPathCacheEntry::updateToCachedAtlas(const GrUniqueKey& atlasKey,
                                             const SkIVector& newAtlasOffset,
                                             sk_sp<GrCCAtlas::CachedAtlasInfo> info) {
    SkASSERT(atlasKey.isValid());
    SkASSERT(!fCurrFlushAtlas);  // Otherwise we should reuse the atlas from last time.

    fAtlasKey = atlasKey;
    fAtlasOffset = newAtlasOffset;

    SkASSERT(!fCachedAtlasInfo);  // Otherwise we need to invalidate our pixels in the old info.
    fCachedAtlasInfo = std::move(info);
    fCachedAtlasInfo->fNumPathPixels += this->height() * this->width();
}

void GrCCPathCacheEntry::invalidateAtlas() {
    if (fCachedAtlasInfo) {
        // Mark our own pixels invalid in the cached atlas texture.
        fCachedAtlasInfo->fNumInvalidatedPathPixels += this->height() * this->width();
        if (!fCachedAtlasInfo->fIsPurgedFromResourceCache &&
            fCachedAtlasInfo->fNumInvalidatedPathPixels >= fCachedAtlasInfo->fNumPathPixels / 2) {
            // Too many invalidated pixels: purge the atlas texture from the resource cache.
            // The GrContext and CCPR path cache both share the same unique ID.
            uint32_t contextUniqueID = fPathCacheUniqueID;
            SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
                    GrUniqueKeyInvalidatedMessage(fAtlasKey, contextUniqueID));
            fCachedAtlasInfo->fIsPurgedFromResourceCache = true;
        }
    }

    fAtlasKey.reset();
    fCachedAtlasInfo = nullptr;
}

void GrCCPathCacheEntry::onChange() {
    // Post a thread-safe eviction message.
    SkMessageBus<sk_sp<GrCCPathCacheEntry>>::Post(sk_ref_sp(this));
}
