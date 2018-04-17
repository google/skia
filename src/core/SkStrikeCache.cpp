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

// Returns the shared globals
static SkStrikeCache& get_globals() {
    static SkOnce once;
    static SkStrikeCache* globals;

    once([]{ globals = new SkStrikeCache; });
    return *globals;
}

SkStrikeCache::~SkStrikeCache() {
    SkGlyphCache* cache = fHead;
    while (cache) {
        SkGlyphCache* next = cache->fNext;
        delete cache;
        cache = next;
    }
}

void SkStrikeCache::AttachCache(SkGlyphCache* cache) {
    get_globals().attachCache(cache);
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


void SkStrikeCache::attachCache(SkGlyphCache *cache) {
    if (cache == nullptr) {
        return;
    }
    SkAutoExclusive ac(fLock);

    this->validate();
    cache->validate();

    this->internalAttachCacheToHead(cache);
    this->internalPurge();
}

SkExclusiveStrikePtr SkStrikeCache::findStrikeExclusive(const SkDescriptor& desc) {
    SkGlyphCache*         cache;
    SkAutoExclusive       ac(fLock);

    for (cache = internalGetHead(); cache != nullptr; cache = cache->fNext) {
        if (cache->getDescriptor() == desc) {
            this->internalDetachCache(cache);
            return SkExclusiveStrikePtr(cache);
        }
    }

    return SkExclusiveStrikePtr(nullptr);
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

    for (SkGlyphCache* cache = this->internalGetHead(); cache != nullptr; cache = cache->fNext) {
        visitor(*cache);
    }
}

SkGlyphCache* SkStrikeCache::internalGetTail() const {
    SkGlyphCache* cache = fHead;
    if (cache) {
        while (cache->fNext) {
            cache = cache->fNext;
        }
    }
    return cache;
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

    // we start at the tail and proceed backwards, as the linklist is in LRU
    // order, with unimportant entries at the tail.
    SkGlyphCache* cache = this->internalGetTail();
    while (cache != nullptr &&
           (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        SkGlyphCache* prev = cache->fPrev;
        bytesFreed += cache->fMemoryUsed;
        countFreed += 1;

        this->internalDetachCache(cache);
        delete cache;
        cache = prev;
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

void SkStrikeCache::internalAttachCacheToHead(SkGlyphCache* cache) {
    SkASSERT(nullptr == cache->fPrev && nullptr == cache->fNext);
    if (fHead) {
        fHead->fPrev = cache;
        cache->fNext = fHead;
    }
    fHead = cache;

    fCacheCount += 1;
    fTotalMemoryUsed += cache->fMemoryUsed;
}

void SkStrikeCache::internalDetachCache(SkGlyphCache* cache) {
    SkASSERT(fCacheCount > 0);
    fCacheCount -= 1;
    fTotalMemoryUsed -= cache->fMemoryUsed;

    if (cache->fPrev) {
        cache->fPrev->fNext = cache->fNext;
    } else {
        fHead = cache->fNext;
    }
    if (cache->fNext) {
        cache->fNext->fPrev = cache->fPrev;
    }
    cache->fPrev = cache->fNext = nullptr;
}

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

SkExclusiveStrikePtr SkStrikeCache::CreateStrikeExclusive(
    const SkDescriptor& desc,
    std::unique_ptr<SkScalerContext> scaler,
    SkPaint::FontMetrics* maybeMetrics)
{
    SkPaint::FontMetrics fontMetrics;
    if (maybeMetrics != nullptr) {
        fontMetrics = *maybeMetrics;
    } else {
        scaler->getFontMetrics(&fontMetrics);
    }

    return SkExclusiveStrikePtr(new SkGlyphCache(desc, move(scaler), fontMetrics));
}