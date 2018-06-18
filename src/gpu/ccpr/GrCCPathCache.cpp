/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPathCache.h"

#include "GrShape.h"
#include "SkNx.h"
#include "ccpr/GrCCPathParser.h"

// The maximum number of cache entries we allow in our own cache.
static constexpr int kMaxCacheCount = 1 << 16;

GrCCPathCache::MaskTransform::MaskTransform(const SkMatrix& m, SkIVector* shift)
        : fMatrix2x2{m.getScaleX(), m.getSkewX(), m.getSkewY(), m.getScaleY()} {
    SkASSERT(!m.hasPerspective());
    Sk2f translate = Sk2f(m.getTranslateX(), m.getTranslateY());
    Sk2f floor = translate.floor();
    (translate - floor).store(fSubpixelTranslate);
    shift->set((int)floor[0], (int)floor[1]);
    SkASSERT((float)shift->fX == floor[0]);
    SkASSERT((float)shift->fY == floor[1]);
}

inline static bool fuzzy_equals(const GrCCPathCache::MaskTransform& a,
                                const GrCCPathCache::MaskTransform& b) {
    return (Sk4f::Load(a.fMatrix2x2) == Sk4f::Load(b.fMatrix2x2)).allTrue() &&
           ((Sk2f::Load(a.fSubpixelTranslate) -
             Sk2f::Load(b.fSubpixelTranslate)).abs() < 1.f/256).allTrue();
}

inline GrCCPathCache::HashNode::HashNode(GrCCPathCache* cache, const MaskTransform& m,
                                         const GrShape& shape) {
    SkASSERT(shape.hasUnstyledKey());

    int keyLength = 1 + shape.unstyledKeySize();
    void* mem = ::operator new (sizeof(GrCCPathCacheEntry) + keyLength * sizeof(uint32_t));
    fEntry = new (mem) GrCCPathCacheEntry(cache, m);

    // The shape key is a variable-length footer to the entry allocation.
    uint32_t* keyData = (uint32_t*)((char*)mem + sizeof(GrCCPathCacheEntry));
    keyData[0] = keyLength - 1;
    shape.writeUnstyledKey(&keyData[1]);
}

inline bool operator==(const GrCCPathCache::HashKey& key1, const GrCCPathCache::HashKey& key2) {
    return key1.fData[0] == key2.fData[0] &&
           !memcmp(&key1.fData[1], &key2.fData[1], key1.fData[0] * sizeof(uint32_t));
}

inline GrCCPathCache::HashKey GrCCPathCache::HashNode::GetKey(const GrCCPathCacheEntry* entry) {
    // The shape key is a variable-length footer to the entry allocation.
    return HashKey{(const uint32_t*)((const char*)entry + sizeof(GrCCPathCacheEntry))};
}

inline uint32_t GrCCPathCache::HashNode::Hash(HashKey key) {
    return GrResourceKeyHash(&key.fData[1], key.fData[0]);
}

GrCCPathCache::HashNode::~HashNode() {
    if (!fEntry) {
        return;
    }

    // Finalize our eviction from the path cache.
    SkASSERT(fEntry->fCacheWeakPtr);
    fEntry->fCacheWeakPtr->fLRU.remove(fEntry);
    fEntry->fCacheWeakPtr = nullptr;

    if (GrCCAtlas::CachedAtlasInfo* info = fEntry->fCachedAtlasInfo.get()) {
        // Mark our own pixels invalid in the cached atlas texture now that we have been evicted.
        info->fNumInvalidatedPathPixels += fEntry->height() * fEntry->width();
        if (!info->fIsPurgedFromResourceCache &&
            info->fNumInvalidatedPathPixels >= info->fNumPathPixels / 2) {
            // Too many invalidated pixels: purge the atlas texture from the resource cache.
            SkMessageBus<GrUniqueKeyInvalidatedMessage>::Post(
                    GrUniqueKeyInvalidatedMessage(fEntry->fAtlasKey));
            info->fIsPurgedFromResourceCache = true;
        }
    }

    fEntry->unref();
}

GrCCPathCache::HashNode& GrCCPathCache::HashNode::operator=(HashNode&& node) {
    this->~HashNode();
    return *new (this) HashNode(std::move(node));
}

sk_sp<GrCCPathCacheEntry> GrCCPathCache::find(const GrShape& shape, const MaskTransform& m,
                                              CreateIfAbsent createIfAbsent) {
    if (!shape.hasUnstyledKey()) {
        return nullptr;
    }

    int keyLength = 1 + shape.unstyledKeySize();
    SkAutoSTMalloc<GrShape::kMaxKeyFromDataVerbCnt * 4, uint32_t> keyData(keyLength);
    keyData[0] = keyLength - 1;
    shape.writeUnstyledKey(&keyData[1]);

    GrCCPathCacheEntry* entry = nullptr;
    if (HashNode* node = fHashTable.find({keyData.get()})) {
        entry = node->entry();
        SkASSERT(this == entry->fCacheWeakPtr);
        if (!fuzzy_equals(m, entry->fMaskTransform)) {
            this->evict(entry);  // The path was reused with an incompatible matrix.
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
        entry = fHashTable.set(HashNode(this, m, shape))->entry();
        SkASSERT(fHashTable.count() <= kMaxCacheCount);
    } else {
        fLRU.remove(entry);  // Will be re-added at head.
    }

    fLRU.addToHead(entry);
    return sk_ref_sp(entry);
}

void GrCCPathCache::evict(const GrCCPathCacheEntry* entry) {
    SkASSERT(entry);
    SkASSERT(this == entry->fCacheWeakPtr);
    SkASSERT(fLRU.isInList(entry));
    SkASSERT(fHashTable.find(HashNode::GetKey(entry))->entry() == entry);

    fHashTable.remove(HashNode::GetKey(entry));  // ~HashNode() handles the rest.
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

void GrCCPathCacheEntry::onChange() {
    // Our corresponding path was modified or deleted. Evict ourselves.
    if (fCacheWeakPtr) {
        fCacheWeakPtr->evict(this);
    }
}
