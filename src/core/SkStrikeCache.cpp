/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStrikeCache.h"

#include <cctype>

#include "SkGlyphCache.h"
#include "SkGraphics.h"
#include "SkMutex.h"
#include "SkTemplates.h"
#include "SkTraceMemoryDump.h"
#include "SkTypeface.h"
#include "SkPaintPriv.h"

SkStrikeCache* SkStrikeCache::GlobalStrikeCache() {
    static auto* cache = new SkStrikeCache;
    return cache;
}

struct SkStrikeCache::Node {
    Node(const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        const SkPaint::FontMetrics& metrics,
        std::unique_ptr<SkStrikePinner> pinner)
        : fCache{desc, std::move(scaler), metrics}
        , fPinner{std::move(pinner)} {}

    Node*                           fNext{nullptr};
    Node*                           fPrev{nullptr};
    SkGlyphCache                    fCache;
    std::unique_ptr<SkStrikePinner> fPinner;
};

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(
        SkStrikeCache::Node* node, SkStrikeCache* strikeCache)
    : fNode{node}
    , fStrikeCache{strikeCache} {}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr()
    : fNode{nullptr}
    , fStrikeCache{nullptr} {}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(ExclusiveStrikePtr&& o)
    : fNode{o.fNode}
    , fStrikeCache{o.fStrikeCache}{
    o.fNode = nullptr;
    o.fStrikeCache = nullptr;
}

SkStrikeCache::ExclusiveStrikePtr&
SkStrikeCache::ExclusiveStrikePtr::operator = (ExclusiveStrikePtr&& o) {
    if (fStrikeCache != nullptr) {
        fStrikeCache->attachNode(fNode);
    }
    fNode = o.fNode;
    fStrikeCache = o.fStrikeCache;
    o.fNode = nullptr;
    o.fStrikeCache = nullptr;
    return *this;
}

SkStrikeCache::ExclusiveStrikePtr::~ExclusiveStrikePtr() {
    if (fStrikeCache != nullptr) {
        fStrikeCache->attachNode(fNode);
    }
}

SkGlyphCache* SkStrikeCache::ExclusiveStrikePtr::get() const {
    return &fNode->fCache;
}

SkGlyphCache* SkStrikeCache::ExclusiveStrikePtr::operator -> () const {
    return this->get();
}

SkGlyphCache& SkStrikeCache::ExclusiveStrikePtr::operator *  () const {
    return *this->get();
}

SkStrikeCache::ExclusiveStrikePtr::operator bool () const {
    return fNode != nullptr;
}

bool operator == (const SkStrikeCache::ExclusiveStrikePtr& lhs,
                  const SkStrikeCache::ExclusiveStrikePtr& rhs) {
    return lhs.fNode == rhs.fNode;
}

bool operator == (const SkStrikeCache::ExclusiveStrikePtr& lhs, decltype(nullptr)) {
    return lhs.fNode == nullptr;
}

bool operator == (decltype(nullptr), const SkStrikeCache::ExclusiveStrikePtr& rhs) {
    return nullptr == rhs.fNode;
}

SkStrikeCache::~SkStrikeCache() {
    Node* node = fHead;
    while (node) {
        Node* next = node->fNext;
        delete node;
        node = next;
    }
}

SkExclusiveStrikePtr SkStrikeCache::FindStrikeExclusive(const SkDescriptor& desc) {
    return GlobalStrikeCache()->findStrikeExclusive(desc);
}

std::unique_ptr<SkScalerContext> SkStrikeCache::CreateScalerContext(
        const SkDescriptor& desc,
        const SkScalerContextEffects& effects,
        const SkTypeface& typeface) {
    auto scaler = typeface.createScalerContext(effects, &desc, true /* can fail */);

    // Check if we can create a scaler-context before creating the glyphcache.
    // If not, we may have exhausted OS/font resources, so try purging the
    // cache once and try again
    // pass true the first time, to notice if the scalercontext failed,
    if (scaler == nullptr) {
        PurgeAll();
        scaler = typeface.createScalerContext(effects, &desc, false /* must succeed */);
    }
    return scaler;
}

SkExclusiveStrikePtr SkStrikeCache::FindOrCreateStrikeExclusive(
        const SkDescriptor& desc, const SkScalerContextEffects& effects, const SkTypeface& typeface)
{
    return GlobalStrikeCache()->findOrCreateStrikeExclusive(desc, effects, typeface);
}

SkExclusiveStrikePtr SkStrikeCache::findOrCreateStrikeExclusive(
        const SkDescriptor& desc, const SkScalerContextEffects& effects, const SkTypeface& typeface)
{
    auto cache = this->findStrikeExclusive(desc);
    if (cache == nullptr) {
        auto scaler = CreateScalerContext(desc, effects, typeface);
        cache = this->createStrikeExclusive(desc, std::move(scaler));
    }
    return cache;
}

SkExclusiveStrikePtr SkStrikeCache::FindOrCreateStrikeExclusive(
        const SkPaint& paint,
        const SkSurfaceProps* surfaceProps,
        SkScalerContextFlags scalerContextFlags,
        const SkMatrix* deviceMatrix)
{
    SkAutoDescriptor ad;
    SkScalerContextEffects effects;

    auto desc = SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
            paint, surfaceProps, scalerContextFlags, deviceMatrix, &ad, &effects);

    auto tf = SkPaintPriv::GetTypefaceOrDefault(paint);

    return FindOrCreateStrikeExclusive(*desc, effects, *tf);
}

SkExclusiveStrikePtr SkStrikeCache::FindOrCreateStrikeExclusive(const SkPaint& paint) {
    return FindOrCreateStrikeExclusive(
            paint, nullptr, kFakeGammaAndBoostContrast, nullptr);
}

void SkStrikeCache::PurgeAll() {
    GlobalStrikeCache()->purgeAll();
}

void SkStrikeCache::Dump() {
    SkDebugf("GlyphCache [     used    budget ]\n");
    SkDebugf("    bytes  [ %8zu  %8zu ]\n",
             SkGraphics::GetFontCacheUsed(), SkGraphics::GetFontCacheLimit());
    SkDebugf("    count  [ %8zu  %8zu ]\n",
             SkGraphics::GetFontCacheCountUsed(), SkGraphics::GetFontCacheCountLimit());

    int counter = 0;

    auto visitor = [&counter](const SkGlyphCache& cache) {
        const SkScalerContextRec& rec = cache.getScalerContext()->getRec();

        SkDebugf("index %d\n", counter);
        SkDebugf("%s", rec.dump().c_str());
        counter += 1;
    };

    GlobalStrikeCache()->forEachStrike(visitor);
}

namespace {
    const char gGlyphCacheDumpName[] = "skia/sk_glyph_cache";
}  // namespace

void SkStrikeCache::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    dump->dumpNumericValue(gGlyphCacheDumpName, "size", "bytes", SkGraphics::GetFontCacheUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_size", "bytes",
                           SkGraphics::GetFontCacheLimit());
    dump->dumpNumericValue(gGlyphCacheDumpName, "glyph_count", "objects",
                           SkGraphics::GetFontCacheCountUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_glyph_count", "objects",
                           SkGraphics::GetFontCacheCountLimit());

    if (dump->getRequestedDetails() == SkTraceMemoryDump::kLight_LevelOfDetail) {
        dump->setMemoryBacking(gGlyphCacheDumpName, "malloc", nullptr);
        return;
    }

    auto visitor = [&dump](const SkGlyphCache& cache) {
        const SkTypeface* face = cache.getScalerContext()->getTypeface();
        const SkScalerContextRec& rec = cache.getScalerContext()->getRec();

        SkString fontName;
        face->getFamilyName(&fontName);
        // Replace all special characters with '_'.
        for (size_t index = 0; index < fontName.size(); ++index) {
            if (!std::isalnum(fontName[index])) {
                fontName[index] = '_';
            }
        }

        SkString dumpName = SkStringPrintf(
                "%s/%s_%d/%p", gGlyphCacheDumpName, fontName.c_str(), rec.fFontID, &cache);

        dump->dumpNumericValue(dumpName.c_str(),
                               "size", "bytes", cache.getMemoryUsed());
        dump->dumpNumericValue(dumpName.c_str(),
                               "glyph_count", "objects", cache.countCachedGlyphs());
        dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
    };

    GlobalStrikeCache()->forEachStrike(visitor);
}


void SkStrikeCache::attachNode(Node* node) {
    if (node == nullptr) {
        return;
    }
    SkAutoExclusive ac(fLock);

    this->validate();
    node->fCache.validate();

    this->internalAttachToHead(node);
    this->internalPurge();
}

SkExclusiveStrikePtr SkStrikeCache::findStrikeExclusive(const SkDescriptor& desc) {
    SkAutoExclusive ac(fLock);

    for (Node* node = internalGetHead(); node != nullptr; node = node->fNext) {
        if (node->fCache.getDescriptor() == desc) {
            this->internalDetachCache(node);
            return SkExclusiveStrikePtr(node, this);
        }
    }

    return SkExclusiveStrikePtr();
}

static bool loose_compare(const SkDescriptor& lhs, const SkDescriptor& rhs) {
    uint32_t size;
    auto ptr = lhs.findEntry(kRec_SkDescriptorTag, &size);
    SkScalerContextRec lhsRec;
    std::memcpy(&lhsRec, ptr, size);

    ptr = rhs.findEntry(kRec_SkDescriptorTag, &size);
    SkScalerContextRec rhsRec;
    std::memcpy(&rhsRec, ptr, size);

    // If these don't match, there's no way we can use these strikes interchangeably.
    // Note that a typeface from each renderer maps to a unique proxy typeface on the GPU,
    // keyed in the glyph cache using fontID in the SkDescriptor. By limiting this search
    // to descriptors with the same fontID, we ensure that a renderer never uses glyphs
    // generated by a different renderer.
    return
        lhsRec.fFontID == rhsRec.fFontID &&
        lhsRec.fTextSize == rhsRec.fTextSize &&
        lhsRec.fPreScaleX == rhsRec.fPreScaleX &&
        lhsRec.fPreSkewX == rhsRec.fPreSkewX &&
        lhsRec.fPost2x2[0][0] == rhsRec.fPost2x2[0][0] &&
        lhsRec.fPost2x2[0][1] == rhsRec.fPost2x2[0][1] &&
        lhsRec.fPost2x2[1][0] == rhsRec.fPost2x2[1][0] &&
        lhsRec.fPost2x2[1][1] == rhsRec.fPost2x2[1][1];
}

bool SkStrikeCache::desperationSearchForImage(const SkDescriptor& desc, SkGlyph* glyph,
                                              SkGlyphCache* targetCache) {
    SkAutoExclusive ac(fLock);

    SkGlyphID glyphID = glyph->getGlyphID();
    SkFixed targetSubX = glyph->getSubXFixed(),
            targetSubY = glyph->getSubYFixed();

    for (Node* node = internalGetHead(); node != nullptr; node = node->fNext) {
        if (loose_compare(node->fCache.getDescriptor(), desc)) {
            auto targetGlyphID = SkPackedGlyphID(glyphID, targetSubX, targetSubY);
            if (node->fCache.isGlyphCached(glyphID, targetSubX, targetSubY)) {
                SkGlyph* fallback = node->fCache.getRawGlyphByID(targetGlyphID);
                // This desperate-match node may disappear as soon as we drop fLock, so we
                // need to copy the glyph from node into this strike, including a
                // deep copy of the mask.
                targetCache->initializeGlyphFromFallback(glyph, *fallback);
                return true;
            }

            // Look for any sub-pixel pos for this glyph, in case there is a pos mismatch.
            if (const auto* fallback = node->fCache.getCachedGlyphAnySubPix(glyphID)) {
                targetCache->initializeGlyphFromFallback(glyph, *fallback);
                return true;
            }
        }
    }

    return false;
}

bool SkStrikeCache::desperationSearchForPath(
        const SkDescriptor& desc, SkGlyphID glyphID, SkPath* path) {
    SkAutoExclusive ac(fLock);

    // The following is wrong there is subpixel positioning with paths...
    // Paths are only ever at sub-pixel position (0,0), so we can just try that directly rather
    // than try our packed position first then search all others on failure like for masks.
    //
    // This will have to search the sub-pixel positions too.
    // There is also a problem with accounting for cache size with shared path data.
    for (Node* node = internalGetHead(); node != nullptr; node = node->fNext) {
        if (loose_compare(node->fCache.getDescriptor(), desc)) {
            if (node->fCache.isGlyphCached(glyphID, 0, 0)) {
                SkGlyph* from = node->fCache.getRawGlyphByID(SkPackedGlyphID(glyphID));
                if (from->fPathData->fPath != nullptr) {
                    // We can just copy the path out by value here, so no need to worry
                    // about the lifetime of this desperate-match node.
                    *path = *from->fPathData->fPath;
                    return true;
                }
            }
        }
    }
    return false;
}

SkExclusiveStrikePtr SkStrikeCache::CreateStrikeExclusive(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkPaint::FontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner)
{
    return GlobalStrikeCache()->createStrikeExclusive(
            desc, std::move(scaler), maybeMetrics, std::move(pinner));
}

SkExclusiveStrikePtr SkStrikeCache::createStrikeExclusive(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkPaint::FontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner)
{
    SkPaint::FontMetrics fontMetrics;
    if (maybeMetrics != nullptr) {
        fontMetrics = *maybeMetrics;
    } else {
        scaler->getFontMetrics(&fontMetrics);
    }

    auto* node = new Node(desc, std::move(scaler), fontMetrics, std::move(pinner));
    return SkExclusiveStrikePtr(node, this);
}

void SkStrikeCache::purgeAll() {
    SkAutoExclusive ac(fLock);
    this->internalPurge(fTotalMemoryUsed);
}

size_t SkStrikeCache::getTotalMemoryUsed() const {
    SkAutoExclusive ac(fLock);
    return fTotalMemoryUsed;
}

int SkStrikeCache::getCacheCountUsed() const {
    SkAutoExclusive ac(fLock);
    return fCacheCount;
}

int SkStrikeCache::getCacheCountLimit() const {
    SkAutoExclusive ac(fLock);
    return fCacheCountLimit;
}

size_t SkStrikeCache::setCacheSizeLimit(size_t newLimit) {
    static const size_t minLimit = 256 * 1024;
    if (newLimit < minLimit) {
        newLimit = minLimit;
    }

    SkAutoExclusive ac(fLock);

    size_t prevLimit = fCacheSizeLimit;
    fCacheSizeLimit = newLimit;
    this->internalPurge();
    return prevLimit;
}

size_t  SkStrikeCache::getCacheSizeLimit() const {
    SkAutoExclusive ac(fLock);
    return fCacheSizeLimit;
}

int SkStrikeCache::setCacheCountLimit(int newCount) {
    if (newCount < 0) {
        newCount = 0;
    }

    SkAutoExclusive ac(fLock);

    int prevCount = fCacheCountLimit;
    fCacheCountLimit = newCount;
    this->internalPurge();
    return prevCount;
}

int SkStrikeCache::getCachePointSizeLimit() const {
    SkAutoExclusive ac(fLock);
    return fPointSizeLimit;
}

int SkStrikeCache::setCachePointSizeLimit(int newLimit) {
    if (newLimit < 0) {
        newLimit = 0;
    }

    SkAutoExclusive ac(fLock);

    int prevLimit = fPointSizeLimit;
    fPointSizeLimit = newLimit;
    return prevLimit;
}

void SkStrikeCache::forEachStrike(std::function<void(const SkGlyphCache&)> visitor) const {
    SkAutoExclusive ac(fLock);

    this->validate();

    for (Node* node = this->internalGetHead(); node != nullptr; node = node->fNext) {
        visitor(node->fCache);
    }
}

size_t SkStrikeCache::internalPurge(size_t minBytesNeeded) {
    this->validate();

    size_t bytesNeeded = 0;
    if (fTotalMemoryUsed > fCacheSizeLimit) {
        bytesNeeded = fTotalMemoryUsed - fCacheSizeLimit;
    }
    bytesNeeded = SkTMax(bytesNeeded, minBytesNeeded);
    if (bytesNeeded) {
        // no small purges!
        bytesNeeded = SkTMax(bytesNeeded, fTotalMemoryUsed >> 2);
    }

    int countNeeded = 0;
    if (fCacheCount > fCacheCountLimit) {
        countNeeded = fCacheCount - fCacheCountLimit;
        // no small purges!
        countNeeded = SkMax32(countNeeded, fCacheCount >> 2);
    }

    // early exit
    if (!countNeeded && !bytesNeeded) {
        return 0;
    }

    size_t  bytesFreed = 0;
    int     countFreed = 0;

    // Start at the tail and proceed backwards deleting; the list is in LRU
    // order, with unimportant entries at the tail.
    Node* node = this->internalGetTail();
    while (node != nullptr && (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        Node* prev = node->fPrev;

        // Only delete if the strike is not pinned.
        if (node->fPinner == nullptr || node->fPinner->canDelete()) {
            bytesFreed += node->fCache.getMemoryUsed();
            countFreed += 1;
            this->internalDetachCache(node);
            delete node;
        }
        node = prev;
    }

    this->validate();

#ifdef SPEW_PURGE_STATUS
    if (countFreed) {
        SkDebugf("purging %dK from font cache [%d entries]\n",
                 (int)(bytesFreed >> 10), countFreed);
    }
#endif

    return bytesFreed;
}

void SkStrikeCache::internalAttachToHead(Node* node) {
    SkASSERT(nullptr == node->fPrev && nullptr == node->fNext);
    if (fHead) {
        fHead->fPrev = node;
        node->fNext = fHead;
    }
    fHead = node;

    if (fTail == nullptr) {
        fTail = node;
    }

    fCacheCount += 1;
    fTotalMemoryUsed += node->fCache.getMemoryUsed();
}

void SkStrikeCache::internalDetachCache(Node* node) {
    SkASSERT(fCacheCount > 0);
    fCacheCount -= 1;
    fTotalMemoryUsed -= node->fCache.getMemoryUsed();

    if (node->fPrev) {
        node->fPrev->fNext = node->fNext;
    } else {
        fHead = node->fNext;
    }
    if (node->fNext) {
        node->fNext->fPrev = node->fPrev;
    } else {
        fTail = node->fPrev;
    }
    node->fPrev = node->fNext = nullptr;
}

void SkStrikeCache::ValidateGlyphCacheDataSize() {
#ifdef SK_DEBUG
    GlobalStrikeCache()->validateGlyphCacheDataSize();
#endif
}

#ifdef SK_DEBUG
void SkStrikeCache::validateGlyphCacheDataSize() const {
    this->forEachStrike(
            [](const SkGlyphCache& cache) { cache.forceValidate();
    });
}
#endif

#ifdef SK_DEBUG
void SkStrikeCache::validate() const {
    size_t computedBytes = 0;
    int computedCount = 0;

    const Node* node = fHead;
    while (node != nullptr) {
        computedBytes += node->fCache.getMemoryUsed();
        computedCount += 1;
        node = node->fNext;
    }

    SkASSERTF(fCacheCount == computedCount, "fCacheCount: %d, computedCount: %d", fCacheCount,
              computedCount);
    SkASSERTF(fTotalMemoryUsed == computedBytes, "fTotalMemoryUsed: %d, computedBytes: %d",
              fTotalMemoryUsed, computedBytes);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
