/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathCache.h"

#include "GrShape.h"
#include "SkNx.h"

static constexpr int kMaxKeyDataCountU32 = 256;  // 1kB of uint32_t's.

DECLARE_SKMESSAGEBUS_MESSAGE(sk_sp<GrCCPathCache::Key>);

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
        const sk_sp<GrCCPathCache::Key>& key, uint32_t msgBusUniqueID) {
    return key->pathCacheUniqueID() == msgBusUniqueID;
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

sk_sp<GrCCPathCache::Key> GrCCPathCache::Key::Make(uint32_t pathCacheUniqueID,
                                                   int dataCountU32, const void* data) {
    void* memory = ::operator new (sizeof(Key) + dataCountU32 * sizeof(uint32_t));
    sk_sp<GrCCPathCache::Key> key(new (memory) Key(pathCacheUniqueID, dataCountU32));
    if (data) {
        memcpy(key->data(), data, key->dataSizeInBytes());
    }
    return key;
}

const uint32_t* GrCCPathCache::Key::data() const {
    // The shape key is a variable-length footer to the entry allocation.
    return reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(this) + sizeof(Key));
}

uint32_t* GrCCPathCache::Key::data() {
    // The shape key is a variable-length footer to the entry allocation.
    return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(this) + sizeof(Key));
}

inline bool GrCCPathCache::Key::operator==(const GrCCPathCache::Key& that) const {
    return fDataSizeInBytes == that.fDataSizeInBytes &&
           !memcmp(this->data(), that.data(), fDataSizeInBytes);
}

void GrCCPathCache::Key::onChange() {
    // Our key's corresponding path was invalidated. Post a thread-safe eviction message.
    SkMessageBus<sk_sp<Key>>::Post(sk_ref_sp(this));
}

inline const GrCCPathCache::Key& GrCCPathCache::HashNode::GetKey(
        const GrCCPathCache::HashNode& node) {
    return *node.entry()->fCacheKey;
}

inline uint32_t GrCCPathCache::HashNode::Hash(const Key& key) {
    return GrResourceKeyHash(key.data(), key.dataSizeInBytes());
}

inline GrCCPathCache::HashNode::HashNode(GrCCPathCache* pathCache, sk_sp<Key> key,
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

    fEntry->fCacheKey->markShouldUnregisterFromPath();  // Unregister the path listener.
    fPathCache->fLRU.remove(fEntry.get());
}


GrCCPathCache::GrCCPathCache()
        : fInvalidatedKeysInbox(next_path_cache_id())
        , fScratchKey(Key::Make(fInvalidatedKeysInbox.uniqueID(), kMaxKeyDataCountU32)) {
}

GrCCPathCache::~GrCCPathCache() {
    fHashTable.reset();  // Must be cleared first; ~HashNode calls fLRU.remove() on us.
    SkASSERT(fLRU.isEmpty());  // Ensure the hash table and LRU list were coherent.
}

namespace {

// Produces a key that accounts both for a shape's path geometry, as well as any stroke/style.
class WriteKeyHelper {
public:
    static constexpr int kStrokeWidthIdx = 0;
    static constexpr int kStrokeMiterIdx = 1;
    static constexpr int kStrokeCapJoinIdx = 2;
    static constexpr int kShapeUnstyledKeyIdx = 3;

    WriteKeyHelper(const GrShape& shape) : fShapeUnstyledKeyCount(shape.unstyledKeySize()) {}

    // Returns the total number of uint32_t's to allocate for the key.
    int allocCountU32() const { return kShapeUnstyledKeyIdx + fShapeUnstyledKeyCount; }

    // Writes the key data to out[].
    void write(const GrShape& shape, uint32_t* out) {
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

sk_sp<GrCCPathCacheEntry> GrCCPathCache::find(const GrShape& shape, const MaskTransform& m,
                                              CreateIfAbsent createIfAbsent) {
    if (!shape.hasUnstyledKey()) {
        return nullptr;
    }

    WriteKeyHelper writeKeyHelper(shape);
    if (writeKeyHelper.allocCountU32() > kMaxKeyDataCountU32) {
        return nullptr;
    }

    SkASSERT(fScratchKey->unique());
    fScratchKey->resetDataCountU32(writeKeyHelper.allocCountU32());
    writeKeyHelper.write(shape, fScratchKey->data());

    GrCCPathCacheEntry* entry = nullptr;
    if (HashNode* node = fHashTable.find(*fScratchKey)) {
        entry = node->entry();
        SkASSERT(fLRU.isInList(entry));
        if (!fuzzy_equals(m, entry->fMaskTransform)) {
            // The path was reused with an incompatible matrix.
            if (CreateIfAbsent::kYes == createIfAbsent && entry->unique()) {
                // This entry is unique: recycle it instead of deleting and malloc-ing a new one.
                entry->fMaskTransform = m;
                entry->fHitCount = 0;
                entry->invalidateAtlas();
                SkASSERT(!entry->fCurrFlushAtlas);  // Should be null because 'entry' is unique.
            } else {
                this->evict(*fScratchKey);
                entry = nullptr;
            }
        }
    }

    if (!entry) {
        if (CreateIfAbsent::kNo == createIfAbsent) {
            return nullptr;
        }
        if (fHashTable.count() >= kMaxCacheCount) {
            SkDEBUGCODE(HashNode* node = fHashTable.find(*fLRU.tail()->fCacheKey));
            SkASSERT(node && node->entry() == fLRU.tail());
            this->evict(*fLRU.tail()->fCacheKey);  // We've exceeded our limit.
        }

        // Create a new entry in the cache.
        sk_sp<Key> permanentKey = Key::Make(fInvalidatedKeysInbox.uniqueID(),
                                            writeKeyHelper.allocCountU32(), fScratchKey->data());
        SkASSERT(*permanentKey == *fScratchKey);
        SkASSERT(!fHashTable.find(*permanentKey));
        entry = fHashTable.set(HashNode(this, std::move(permanentKey), m, shape))->entry();

        SkASSERT(fHashTable.count() <= kMaxCacheCount);
    } else {
        fLRU.remove(entry);  // Will be re-added at head.
    }

    SkDEBUGCODE(HashNode* node = fHashTable.find(*fScratchKey));
    SkASSERT(node && node->entry() == entry);
    fLRU.addToHead(entry);

    entry->fTimestamp = this->quickPerFlushTimestamp();
    ++entry->fHitCount;
    return sk_ref_sp(entry);
}

void GrCCPathCache::doPostFlushProcessing() {
    this->purgeInvalidatedKeys();

    // Mark the per-flush timestamp as needing to be updated with a newer clock reading.
    fPerFlushTimestamp = GrStdSteadyClock::time_point::min();
}

void GrCCPathCache::purgeEntriesOlderThan(const GrStdSteadyClock::time_point& purgeTime) {
    this->purgeInvalidatedKeys();

#ifdef SK_DEBUG
    auto lastTimestamp = (fLRU.isEmpty())
            ? GrStdSteadyClock::time_point::max()
            : fLRU.tail()->fTimestamp;
#endif

    // Drop every cache entry whose timestamp is older than purgeTime.
    while (!fLRU.isEmpty() && fLRU.tail()->fTimestamp < purgeTime) {
#ifdef SK_DEBUG
        // Verify that fLRU is sorted by timestamp.
        auto timestamp = fLRU.tail()->fTimestamp;
        SkASSERT(timestamp >= lastTimestamp);
        lastTimestamp = timestamp;
#endif
        this->evict(*fLRU.tail()->fCacheKey);
    }
}

void GrCCPathCache::purgeInvalidatedKeys() {
    SkTArray<sk_sp<Key>> invalidatedKeys;
    fInvalidatedKeysInbox.poll(&invalidatedKeys);
    for (const sk_sp<Key>& key : invalidatedKeys) {
        bool isInCache = !key->shouldUnregisterFromPath();  // Gets set upon exiting the cache.
        if (isInCache) {
            this->evict(*key);
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
