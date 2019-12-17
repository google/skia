/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCPathCache.h"

#include "include/private/SkNx.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrProxyProvider.h"

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

void GrCCPathCache::Key::operator delete(void* p) { ::operator delete(p); }

const uint32_t* GrCCPathCache::Key::data() const {
    // The shape key is a variable-length footer to the entry allocation.
    return reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(this) + sizeof(Key));
}

uint32_t* GrCCPathCache::Key::data() {
    // The shape key is a variable-length footer to the entry allocation.
    return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(this) + sizeof(Key));
}

void GrCCPathCache::Key::onChange() {
    // Our key's corresponding path was invalidated. Post a thread-safe eviction message.
    SkMessageBus<sk_sp<Key>>::Post(sk_ref_sp(this));
}

GrCCPathCache::GrCCPathCache(uint32_t contextUniqueID)
        : fContextUniqueID(contextUniqueID)
        , fInvalidatedKeysInbox(next_path_cache_id())
        , fScratchKey(Key::Make(fInvalidatedKeysInbox.uniqueID(), kMaxKeyDataCountU32)) {
}

GrCCPathCache::~GrCCPathCache() {
    while (!fLRU.isEmpty()) {
        this->evict(*fLRU.tail()->fCacheKey, fLRU.tail());
    }
    SkASSERT(0 == fHashTable.count());  // Ensure the hash table and LRU list were coherent.

    // Now take all the atlas textures we just invalidated and purge them from the GrResourceCache.
    // We just purge via message bus since we don't have any access to the resource cache right now.
    for (sk_sp<GrTextureProxy>& proxy : fInvalidatedProxies) {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
                GrUniqueKeyInvalidatedMessage(proxy->getUniqueKey(), fContextUniqueID));
    }
    for (const GrUniqueKey& key : fInvalidatedProxyUniqueKeys) {
        SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
                GrUniqueKeyInvalidatedMessage(key, fContextUniqueID));
    }
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

GrCCPathCache::OnFlushEntryRef GrCCPathCache::find(
        GrOnFlushResourceProvider* onFlushRP, const GrShape& shape,
        const SkIRect& clippedDrawBounds, const SkMatrix& viewMatrix, SkIVector* maskShift) {
    if (!shape.hasUnstyledKey()) {
        return OnFlushEntryRef();
    }

    WriteKeyHelper writeKeyHelper(shape);
    if (writeKeyHelper.allocCountU32() > kMaxKeyDataCountU32) {
        return OnFlushEntryRef();
    }

    SkASSERT(fScratchKey->unique());
    fScratchKey->resetDataCountU32(writeKeyHelper.allocCountU32());
    writeKeyHelper.write(shape, fScratchKey->data());

    MaskTransform m(viewMatrix, maskShift);
    GrCCPathCacheEntry* entry = nullptr;
    if (HashNode* node = fHashTable.find(*fScratchKey)) {
        entry = node->entry();
        SkASSERT(fLRU.isInList(entry));

        if (!fuzzy_equals(m, entry->fMaskTransform)) {
            // The path was reused with an incompatible matrix.
            if (entry->unique()) {
                // This entry is unique: recycle it instead of deleting and malloc-ing a new one.
                SkASSERT(0 == entry->fOnFlushRefCnt);  // Because we are unique.
                entry->fMaskTransform = m;
                entry->fHitCount = 0;
                entry->fHitRect = SkIRect::MakeEmpty();
                entry->releaseCachedAtlas(this);
            } else {
                this->evict(*fScratchKey);
                entry = nullptr;
            }
        }
    }

    if (!entry) {
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

    if (0 == entry->fOnFlushRefCnt) {
        // Only update the time stamp and hit count if we haven't seen this entry yet during the
        // current flush.
        entry->fTimestamp = this->quickPerFlushTimestamp();
        ++entry->fHitCount;

        if (entry->fCachedAtlas) {
            SkASSERT(SkToBool(entry->fCachedAtlas->peekOnFlushRefCnt()) ==
                     SkToBool(entry->fCachedAtlas->getOnFlushProxy()));
            if (!entry->fCachedAtlas->getOnFlushProxy()) {
                auto ct = GrCCAtlas::CoverageTypeToColorType(entry->fCachedAtlas->coverageType());
                if (sk_sp<GrTextureProxy> onFlushProxy = onFlushRP->findOrCreateProxyByUniqueKey(
                            entry->fCachedAtlas->textureKey(), ct, GrCCAtlas::kTextureOrigin,
                            GrSurfaceProxy::UseAllocator::kNo)) {
                    entry->fCachedAtlas->setOnFlushProxy(std::move(onFlushProxy));
                }
            }
            if (!entry->fCachedAtlas->getOnFlushProxy()) {
                // Our atlas's backing texture got purged from the GrResourceCache. Release the
                // cached atlas.
                entry->releaseCachedAtlas(this);
            }
        }
    }
    entry->fHitRect.join(clippedDrawBounds.makeOffset(-*maskShift));
    SkASSERT(!entry->fCachedAtlas || entry->fCachedAtlas->getOnFlushProxy());
    return OnFlushEntryRef::OnFlushRef(entry);
}

void GrCCPathCache::evict(const GrCCPathCache::Key& key, GrCCPathCacheEntry* entry) {
    if (!entry) {
        HashNode* node = fHashTable.find(key);
        SkASSERT(node);
        entry = node->entry();
    }
    SkASSERT(*entry->fCacheKey == key);
    SkASSERT(!entry->hasBeenEvicted());
    entry->fCacheKey->markShouldUnregisterFromPath();  // Unregister the path listener.
    entry->releaseCachedAtlas(this);
    fLRU.remove(entry);
    fHashTable.remove(key);
}

void GrCCPathCache::doPreFlushProcessing() {
    this->evictInvalidatedCacheKeys();

    // Mark the per-flush timestamp as needing to be updated with a newer clock reading.
    fPerFlushTimestamp = GrStdSteadyClock::time_point::min();
}

void GrCCPathCache::purgeEntriesOlderThan(GrProxyProvider* proxyProvider,
                                          const GrStdSteadyClock::time_point& purgeTime) {
    this->evictInvalidatedCacheKeys();

#ifdef SK_DEBUG
    auto lastTimestamp = (fLRU.isEmpty())
            ? GrStdSteadyClock::time_point::max()
            : fLRU.tail()->fTimestamp;
#endif

    // Evict every entry from our local path cache whose timestamp is older than purgeTime.
    while (!fLRU.isEmpty() && fLRU.tail()->fTimestamp < purgeTime) {
#ifdef SK_DEBUG
        // Verify that fLRU is sorted by timestamp.
        auto timestamp = fLRU.tail()->fTimestamp;
        SkASSERT(timestamp >= lastTimestamp);
        lastTimestamp = timestamp;
#endif
        this->evict(*fLRU.tail()->fCacheKey);
    }

    // Now take all the atlas textures we just invalidated and purge them from the GrResourceCache.
    this->purgeInvalidatedAtlasTextures(proxyProvider);
}

void GrCCPathCache::purgeInvalidatedAtlasTextures(GrOnFlushResourceProvider* onFlushRP) {
    for (sk_sp<GrTextureProxy>& proxy : fInvalidatedProxies) {
        onFlushRP->removeUniqueKeyFromProxy(proxy.get());
    }
    fInvalidatedProxies.reset();

    for (const GrUniqueKey& key : fInvalidatedProxyUniqueKeys) {
        onFlushRP->processInvalidUniqueKey(key);
    }
    fInvalidatedProxyUniqueKeys.reset();
}

void GrCCPathCache::purgeInvalidatedAtlasTextures(GrProxyProvider* proxyProvider) {
    for (sk_sp<GrTextureProxy>& proxy : fInvalidatedProxies) {
        proxyProvider->removeUniqueKeyFromProxy(proxy.get());
    }
    fInvalidatedProxies.reset();

    for (const GrUniqueKey& key : fInvalidatedProxyUniqueKeys) {
        proxyProvider->processInvalidUniqueKey(key, nullptr,
                                               GrProxyProvider::InvalidateGPUResource::kYes);
    }
    fInvalidatedProxyUniqueKeys.reset();
}

void GrCCPathCache::evictInvalidatedCacheKeys() {
    SkTArray<sk_sp<Key>> invalidatedKeys;
    fInvalidatedKeysInbox.poll(&invalidatedKeys);
    for (const sk_sp<Key>& key : invalidatedKeys) {
        bool isInCache = !key->shouldUnregisterFromPath();  // Gets set upon exiting the cache.
        if (isInCache) {
            this->evict(*key);
        }
    }
}

GrCCPathCache::OnFlushEntryRef
GrCCPathCache::OnFlushEntryRef::OnFlushRef(GrCCPathCacheEntry* entry) {
    entry->ref();
    ++entry->fOnFlushRefCnt;
    if (entry->fCachedAtlas) {
        entry->fCachedAtlas->incrOnFlushRefCnt();
    }
    return OnFlushEntryRef(entry);
}

GrCCPathCache::OnFlushEntryRef::~OnFlushEntryRef() {
    if (!fEntry) {
        return;
    }
    --fEntry->fOnFlushRefCnt;
    SkASSERT(fEntry->fOnFlushRefCnt >= 0);
    if (fEntry->fCachedAtlas) {
        fEntry->fCachedAtlas->decrOnFlushRefCnt();
    }
    fEntry->unref();
}


void GrCCPathCacheEntry::setCoverageCountAtlas(
        GrOnFlushResourceProvider* onFlushRP, GrCCAtlas* atlas, const SkIVector& atlasOffset,
        const GrOctoBounds& octoBounds, const SkIRect& devIBounds, const SkIVector& maskShift) {
    SkASSERT(fOnFlushRefCnt > 0);
    SkASSERT(!fCachedAtlas);  // Otherwise we would need to call releaseCachedAtlas().

    if (this->hasBeenEvicted()) {
        // This entry will never be found in the path cache again. Don't bother trying to save an
        // atlas texture for it in the GrResourceCache.
        return;
    }

    fCachedAtlas = atlas->refOrMakeCachedAtlas(onFlushRP);
    fCachedAtlas->incrOnFlushRefCnt(fOnFlushRefCnt);
    fCachedAtlas->addPathPixels(devIBounds.height() * devIBounds.width());

    fAtlasOffset = atlasOffset + maskShift;

    fOctoBounds.setOffset(octoBounds, -maskShift.fX, -maskShift.fY);
    fDevIBounds = devIBounds.makeOffset(-maskShift);
}

GrCCPathCacheEntry::ReleaseAtlasResult GrCCPathCacheEntry::upgradeToLiteralCoverageAtlas(
        GrCCPathCache* pathCache, GrOnFlushResourceProvider* onFlushRP, GrCCAtlas* atlas,
        const SkIVector& newAtlasOffset) {
    SkASSERT(!this->hasBeenEvicted());
    SkASSERT(fOnFlushRefCnt > 0);
    SkASSERT(fCachedAtlas);
    SkASSERT(GrCCAtlas::CoverageType::kA8_LiteralCoverage != fCachedAtlas->coverageType());

    ReleaseAtlasResult releaseAtlasResult = this->releaseCachedAtlas(pathCache);

    fCachedAtlas = atlas->refOrMakeCachedAtlas(onFlushRP);
    fCachedAtlas->incrOnFlushRefCnt(fOnFlushRefCnt);
    fCachedAtlas->addPathPixels(this->height() * this->width());

    fAtlasOffset = newAtlasOffset;
    return releaseAtlasResult;
}

GrCCPathCacheEntry::ReleaseAtlasResult GrCCPathCacheEntry::releaseCachedAtlas(
        GrCCPathCache* pathCache) {
    ReleaseAtlasResult result = ReleaseAtlasResult::kNone;
    if (fCachedAtlas) {
        result = fCachedAtlas->invalidatePathPixels(pathCache, this->height() * this->width());
        if (fOnFlushRefCnt) {
            SkASSERT(fOnFlushRefCnt > 0);
            fCachedAtlas->decrOnFlushRefCnt(fOnFlushRefCnt);
        }
        fCachedAtlas = nullptr;
    }
    return result;
}

GrCCPathCacheEntry::ReleaseAtlasResult GrCCCachedAtlas::invalidatePathPixels(
        GrCCPathCache* pathCache, int numPixels) {
    // Mark the pixels invalid in the cached atlas texture.
    fNumInvalidatedPathPixels += numPixels;
    SkASSERT(fNumInvalidatedPathPixels <= fNumPathPixels);
    if (!fIsInvalidatedFromResourceCache && fNumInvalidatedPathPixels >= fNumPathPixels / 2) {
        // Too many invalidated pixels: purge the atlas texture from the resource cache.
        if (fOnFlushProxy) {
            // Don't clear (or std::move) fOnFlushProxy. Other path cache entries might still have a
            // reference on this atlas and expect to use our proxy during the current flush.
            // fOnFlushProxy will be cleared once fOnFlushRefCnt decrements to zero.
            pathCache->fInvalidatedProxies.push_back(fOnFlushProxy);
        } else {
            pathCache->fInvalidatedProxyUniqueKeys.push_back(fTextureKey);
        }
        fIsInvalidatedFromResourceCache = true;
        return ReleaseAtlasResult::kDidInvalidateFromCache;
    }
    return ReleaseAtlasResult::kNone;
}

void GrCCCachedAtlas::decrOnFlushRefCnt(int count) const {
    SkASSERT(count > 0);
    fOnFlushRefCnt -= count;
    SkASSERT(fOnFlushRefCnt >= 0);
    if (0 == fOnFlushRefCnt) {
        // Don't hold the actual proxy past the end of the current flush.
        SkASSERT(fOnFlushProxy);
        fOnFlushProxy = nullptr;
    }
}
