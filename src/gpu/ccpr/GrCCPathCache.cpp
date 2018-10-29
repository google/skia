/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathCache.h"

#include "GrShape.h"
#include "SkNx.h"

DECLARE_SKMESSAGEBUS_MESSAGE(sk_sp<GrCCPathCache::KeyRef>);

static inline uint32_t next_path_cache_id() {
    static std::atomic<uint32_t> gNextID(1);
    for (;;) {
        uint32_t id = gNextID.fetch_add(+1, std::memory_order_acquire);
        if (SK_InvalidUniqueID != id) {
            return id;
        }
    }
}

static inline bool SkShouldPostMessageToBus(
        const sk_sp<GrCCPathCache::KeyRef>& keyRef, uint32_t msgBusUniqueID) {
    return keyRef->pathCacheUniqueID() == msgBusUniqueID;
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

inline bool operator==(const GrCCPathCache::Key& key1, const GrCCPathCache::Key& key2) {
    return key1.fData[0] == key2.fData[0] && !memcmp(&key1.fData[1], &key2.fData[1], key1.fData[0]);
}

sk_sp<GrCCPathCache::KeyRef> GrCCPathCache::KeyRef::Make(uint32_t pathCacheUniqueID, Key key) {
    int keyLength = 1 + key.fData[0];
    void* memory = ::operator new (sizeof(KeyRef) + keyLength * sizeof(uint32_t));

    // The shape key is a variable-length footer to the entry allocation.
    uint32_t* keyLocation = (uint32_t*)((char*)memory + sizeof(KeyRef));
    memcpy(keyLocation, key.fData, keyLength * sizeof(uint32_t));

    return sk_sp<GrCCPathCache::KeyRef>(new (memory) KeyRef(pathCacheUniqueID));
}

GrCCPathCache::Key GrCCPathCache::KeyRef::key() const {
    // The shape key is a variable-length footer to the entry allocation.
    return Key{(const uint32_t*)((const char*)this + sizeof(KeyRef))};
}

void GrCCPathCache::KeyRef::onChange() {
    // Post a thread-safe eviction message.
    SkMessageBus<sk_sp<KeyRef>>::Post(sk_ref_sp(this));
}

inline GrCCPathCache::Key GrCCPathCache::HashNode::GetKey(const GrCCPathCache::HashNode& node) {
    return node.entry()->fKeyRef->key();
}

inline uint32_t GrCCPathCache::HashNode::Hash(Key key) {
    return GrResourceKeyHash(&key.fData[1], key.fData[0]);
}

inline GrCCPathCache::HashNode::HashNode(GrCCPathCache* pathCache, sk_sp<KeyRef> key,
                                         const MaskTransform& m, const GrShape& shape)
        : fPathCache(pathCache)
        , fEntry(new GrCCPathCacheEntry(key, m)) {
    SkASSERT(shape.hasUnstyledKey());
    shape.addGenIDChangeListener(std::move(key));
}

inline GrCCPathCache::HashNode::~HashNode() {
    this->willExitHashTable();
}

inline GrCCPathCache::HashNode& GrCCPathCache::HashNode::operator=(HashNode&& node) {
    this->willExitHashTable();
    fPathCache = node.fPathCache;
    fEntry = std::move(node.fEntry);
    SkASSERT(!node.fEntry);
    return *this;
}

inline void GrCCPathCache::HashNode::willExitHashTable() {
    if (!fEntry) {
        return;  // We were moved.
    }

    SkASSERT(fPathCache);
    SkASSERT(fPathCache->fLRU.isInList(fEntry.get()));

    fEntry->fKeyRef->markShouldUnregisterFromPath();
    fPathCache->fLRU.remove(fEntry.get());
}


GrCCPathCache::GrCCPathCache()
        : fInvalidatedKeysInbox(next_path_cache_id()) {
}

GrCCPathCache::~GrCCPathCache() {
    fHashTable.reset();  // Must be cleared first; ~HashNode calls fLRU.remove() on us.
    SkASSERT(fLRU.isEmpty());  // Ensure the hash table and LRU list were coherent.
}

namespace {

// Attempts to produce a GrCCPathCache::Key array on the stack.
class StackAllocKey {
public:
    static constexpr int kStyledKeySizeInBytesIdx = 0;
    static constexpr int kStrokeWidthIdx = 1;
    static constexpr int kStrokeMiterIdx = 2;
    static constexpr int kStrokeCapJoinIdx = 3;
    static constexpr int kShapeUnstyledKeyIdx = 4;

    static constexpr int kStrokeKeyCount = 3;  // [width, miterLimit, cap|join].

    StackAllocKey(const GrShape& shape)
            : fShapeUnstyledKeyCount(shape.unstyledKeySize())
            , fKeyStorage(kShapeUnstyledKeyIdx + fShapeUnstyledKeyCount) {
        fKeyStorage[kStyledKeySizeInBytesIdx] =
                (kStrokeKeyCount + fShapeUnstyledKeyCount) * sizeof(uint32_t);

        // Stroke key.
        // We don't use GrStyle::WriteKey() because it does not account for hairlines.
        // http://skbug.com/8273
        SkASSERT(!shape.style().hasPathEffect());
        const SkStrokeRec& stroke = shape.style().strokeRec();
        if (stroke.isFillStyle()) {
            // Use a value for width that won't collide with a valid fp32 value >= 0.
            fKeyStorage[kStrokeWidthIdx] = ~0;
            fKeyStorage[kStrokeMiterIdx] = fKeyStorage[kStrokeCapJoinIdx] = 0;
        } else {
            float width = stroke.getWidth(), miterLimit = stroke.getMiter();
            memcpy(&fKeyStorage[kStrokeWidthIdx], &width, sizeof(float));
            memcpy(&fKeyStorage[kStrokeMiterIdx], &miterLimit, sizeof(float));
            fKeyStorage[kStrokeCapJoinIdx] = (stroke.getCap() << 16) | stroke.getJoin();
            GR_STATIC_ASSERT(sizeof(fKeyStorage[kStrokeWidthIdx]) == sizeof(float));
        }

        // Shape unstyled key.
        shape.writeUnstyledKey(&fKeyStorage[kShapeUnstyledKeyIdx]);
    }

    const GrCCPathCache::Key key() const { return {fKeyStorage}; }

private:
    int fShapeUnstyledKeyCount;
    SkAutoSTMalloc<GrShape::kMaxKeyFromDataVerbCnt * 4, uint32_t> fKeyStorage;
};

}

sk_sp<GrCCPathCacheEntry> GrCCPathCache::find(const GrShape& shape, const MaskTransform& m,
                                              CreateIfAbsent createIfAbsent) {
    if (!shape.hasUnstyledKey()) {
        return nullptr;
    }

    StackAllocKey stackKey(shape);
    GrCCPathCacheEntry* entry = nullptr;
    if (HashNode* node = fHashTable.find(stackKey.key())) {
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
            this->evict(stackKey.key());
            entry = nullptr;
        }
    }

    if (!entry) {
        if (CreateIfAbsent::kNo == createIfAbsent) {
            return nullptr;
        }
        if (fHashTable.count() >= kMaxCacheCount) {
            this->evict(fLRU.tail()->fKeyRef->key());  // We've exceeded our limit.
        }
        SkASSERT(!fHashTable.find(stackKey.key()));
        sk_sp<KeyRef> cacheKey = KeyRef::Make(fInvalidatedKeysInbox.uniqueID(), stackKey.key());
        entry = fHashTable.set(HashNode(this, std::move(cacheKey), m, shape))->entry();
        SkASSERT(fHashTable.count() <= kMaxCacheCount);
    } else {
        fLRU.remove(entry);  // Will be re-added at head.
    }

    SkDEBUGCODE(HashNode* node = fHashTable.find(stackKey.key()));
    SkASSERT(node && node->entry() == entry);
    fLRU.addToHead(entry);
    return sk_ref_sp(entry);
}

void GrCCPathCache::purgeAsNeeded() {
    SkTArray<sk_sp<KeyRef>> invalidatedKeys;
    fInvalidatedKeysInbox.poll(&invalidatedKeys);
    for (const sk_sp<KeyRef>& keyRef : invalidatedKeys) {
        bool isInCache = !keyRef->shouldUnregisterFromPath();  // This gets set exiting the cache.
        if (isInCache) {
            this->evict(keyRef->key());
        }
    }
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
            SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
                    GrUniqueKeyInvalidatedMessage(fAtlasKey, fCachedAtlasInfo->fContextUniqueID));
            fCachedAtlasInfo->fIsPurgedFromResourceCache = true;
        }
    }

    fAtlasKey.reset();
    fCachedAtlasInfo = nullptr;
}
