/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStrikeCache.h"

#include <cctype>

#include "SkDeduper.h"
#include "SkGlyphCache.h"
#include "SkGraphics.h"
#include "SkMutex.h"
#include "SkOnce.h"
#include "SkTraceMemoryDump.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "SkPaintPriv.h"

// Returns the shared globals
static SkStrikeCache& get_globals() {
    static SkOnce once;
    static SkStrikeCache* globals;

    once([]{ globals = new SkStrikeCache; });
    return *globals;
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

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(SkStrikeCache::Node* node) : fNode{node} {}
SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr() : fNode{nullptr} {}
SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(ExclusiveStrikePtr&& o)
    : fNode{o.fNode} {
    o.fNode = nullptr;
}

SkStrikeCache::ExclusiveStrikePtr&
SkStrikeCache::ExclusiveStrikePtr::operator = (ExclusiveStrikePtr&& o) {
    Attach(fNode);
    fNode = o.fNode;
    o.fNode = nullptr;
    return *this;
}

SkStrikeCache::ExclusiveStrikePtr::~ExclusiveStrikePtr() {
    SkStrikeCache::Attach(fNode);
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

void SkStrikeCache::Attach(Node* node) {
    get_globals().attachNode(node);
}

SkExclusiveStrikePtr SkStrikeCache::FindStrikeExclusive(const SkDescriptor& desc) {
    return get_globals().findStrikeExclusive(desc);
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
    auto cache = FindStrikeExclusive(desc);
    if (cache == nullptr) {
        auto scaler = CreateScalerContext(desc, effects, typeface);
        cache = CreateStrikeExclusive(desc, std::move(scaler));
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
    get_globals().purgeAll();
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

    get_globals().forEachStrike(visitor);
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

    get_globals().forEachStrike(visitor);
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
    Node*           node;
    SkAutoExclusive ac(fLock);

    for (node = internalGetHead(); node != nullptr; node = node->fNext) {
        if (node->fCache.getDescriptor() == desc) {
            this->internalDetachCache(node);
            return SkExclusiveStrikePtr(node);
        }
    }

    return SkExclusiveStrikePtr(nullptr);
}

SkExclusiveStrikePtr SkStrikeCache::CreateStrikeExclusive(
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

    return SkExclusiveStrikePtr(new Node(desc, std::move(scaler), fontMetrics, std::move(pinner)));
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

SkStrikeCache::Node* SkStrikeCache::internalGetTail() const {
    Node* node = fHead;
    if (node) {
        while (node->fNext) {
            node = node->fNext;
        }
    }
    return node;
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
    }
    node->fPrev = node->fNext = nullptr;
}

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

size_t SkGraphics::GetFontCacheLimit() {
    return get_globals().getCacheSizeLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return get_globals().setCacheSizeLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() {
    return get_globals().getTotalMemoryUsed();
}

int SkGraphics::GetFontCacheCountLimit() {
    return get_globals().getCacheCountLimit();
}

int SkGraphics::SetFontCacheCountLimit(int count) {
    return get_globals().setCacheCountLimit(count);
}

int SkGraphics::GetFontCacheCountUsed() {
    return get_globals().getCacheCountUsed();
}

int SkGraphics::GetFontCachePointSizeLimit() {
    return get_globals().getCachePointSizeLimit();
}

int SkGraphics::SetFontCachePointSizeLimit(int limit) {
    return get_globals().setCachePointSizeLimit(limit);
}

void SkGraphics::PurgeFontCache() {
    get_globals().purgeAll();
    SkTypefaceCache::PurgeAll();
}
