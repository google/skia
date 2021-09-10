/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeCache.h"

#include <cctype>

#include "include/core/SkGraphics.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkScalerCache.h"

bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = false;

SkStrikeCache* SkStrikeCache::GlobalStrikeCache() {
    if (gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental) {
        static thread_local auto* cache = new SkStrikeCache;
        return cache;
    }
    static auto* cache = new SkStrikeCache;
    return cache;
}

auto SkStrikeCache::findOrCreateStrike(const SkDescriptor& desc,
                                       const SkScalerContextEffects& effects,
                                       const SkTypeface& typeface) -> sk_sp<Strike> {
    SkAutoMutexExclusive ac(fLock);
    sk_sp<Strike> strike = this->internalFindStrikeOrNull(desc);
    if (strike == nullptr) {
        auto scaler = typeface.createScalerContext(effects, &desc);
        strike = this->internalCreateStrike(desc, std::move(scaler));
    }
    this->internalPurge();
    return strike;
}

SkScopedStrikeForGPU SkStrikeCache::findOrCreateScopedStrike(const SkDescriptor& desc,
                                                             const SkScalerContextEffects& effects,
                                                             const SkTypeface& typeface) {
    return SkScopedStrikeForGPU{this->findOrCreateStrike(desc, effects, typeface).release()};
}

void SkStrikeCache::PurgeAll() {
    GlobalStrikeCache()->purgeAll();
}

void SkStrikeCache::Dump() {
    SkDebugf("GlyphCache [     used    budget ]\n");
    SkDebugf("    bytes  [ %8zu  %8zu ]\n",
             SkGraphics::GetFontCacheUsed(), SkGraphics::GetFontCacheLimit());
    SkDebugf("    count  [ %8d  %8d ]\n",
             SkGraphics::GetFontCacheCountUsed(), SkGraphics::GetFontCacheCountLimit());

    int counter = 0;

    auto visitor = [&counter](const Strike& strike) {
        const SkScalerContextRec& rec = strike.fScalerCache.getScalerContext()->getRec();

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

    auto visitor = [&dump](const Strike& strike) {
        const SkTypeface* face = strike.fScalerCache.getScalerContext()->getTypeface();
        const SkScalerContextRec& rec = strike.fScalerCache.getScalerContext()->getRec();

        SkString fontName;
        face->getFamilyName(&fontName);
        // Replace all special characters with '_'.
        for (size_t index = 0; index < fontName.size(); ++index) {
            if (!std::isalnum(fontName[index])) {
                fontName[index] = '_';
            }
        }

        SkString dumpName = SkStringPrintf(
                "%s/%s_%d/%p", gGlyphCacheDumpName, fontName.c_str(), rec.fFontID, &strike);

        dump->dumpNumericValue(dumpName.c_str(),
                               "size", "bytes", strike.fMemoryUsed);
        dump->dumpNumericValue(dumpName.c_str(),
                               "glyph_count", "objects",
                               strike.fScalerCache.countCachedGlyphs());
        dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
    };

    GlobalStrikeCache()->forEachStrike(visitor);
}

sk_sp<SkStrike> SkStrikeCache::findStrike(const SkDescriptor& desc) {
    SkAutoMutexExclusive ac(fLock);
    sk_sp<SkStrike> result = this->internalFindStrikeOrNull(desc);
    this->internalPurge();
    return result;
}

auto SkStrikeCache::internalFindStrikeOrNull(const SkDescriptor& desc) -> sk_sp<Strike> {

    // Check head because it is likely the strike we are looking for.
    if (fHead != nullptr && fHead->getDescriptor() == desc) { return sk_ref_sp(fHead); }

    // Do the heavy search looking for the strike.
    sk_sp<Strike>* strikeHandle = fStrikeLookup.find(desc);
    if (strikeHandle == nullptr) { return nullptr; }
    Strike* strikePtr = strikeHandle->get();
    SkASSERT(strikePtr != nullptr);
    if (fHead != strikePtr) {
        // Make most recently used
        strikePtr->fPrev->fNext = strikePtr->fNext;
        if (strikePtr->fNext != nullptr) {
            strikePtr->fNext->fPrev = strikePtr->fPrev;
        } else {
            fTail = strikePtr->fPrev;
        }
        fHead->fPrev = strikePtr;
        strikePtr->fNext = fHead;
        strikePtr->fPrev = nullptr;
        fHead = strikePtr;
    }
    return sk_ref_sp(strikePtr);
}

sk_sp<SkStrike> SkStrikeCache::createStrike(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkFontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner) {
    SkAutoMutexExclusive ac(fLock);
    return this->internalCreateStrike(desc, std::move(scaler), maybeMetrics, std::move(pinner));
}

auto SkStrikeCache::internalCreateStrike(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkFontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner) -> sk_sp<Strike> {
    auto strike =
            sk_make_sp<Strike>(this, desc, std::move(scaler), maybeMetrics, std::move(pinner));
    this->internalAttachToHead(strike);
    return strike;
}

void SkStrikeCache::purgeAll() {
    SkAutoMutexExclusive ac(fLock);
    this->internalPurge(fTotalMemoryUsed);
}

size_t SkStrikeCache::getTotalMemoryUsed() const {
    SkAutoMutexExclusive ac(fLock);
    return fTotalMemoryUsed;
}

int SkStrikeCache::getCacheCountUsed() const {
    SkAutoMutexExclusive ac(fLock);
    return fCacheCount;
}

int SkStrikeCache::getCacheCountLimit() const {
    SkAutoMutexExclusive ac(fLock);
    return fCacheCountLimit;
}

size_t SkStrikeCache::setCacheSizeLimit(size_t newLimit) {
    SkAutoMutexExclusive ac(fLock);

    size_t prevLimit = fCacheSizeLimit;
    fCacheSizeLimit = newLimit;
    this->internalPurge();
    return prevLimit;
}

size_t  SkStrikeCache::getCacheSizeLimit() const {
    SkAutoMutexExclusive ac(fLock);
    return fCacheSizeLimit;
}

int SkStrikeCache::setCacheCountLimit(int newCount) {
    if (newCount < 0) {
        newCount = 0;
    }

    SkAutoMutexExclusive ac(fLock);

    int prevCount = fCacheCountLimit;
    fCacheCountLimit = newCount;
    this->internalPurge();
    return prevCount;
}

void SkStrikeCache::forEachStrike(std::function<void(const Strike&)> visitor) const {
    SkAutoMutexExclusive ac(fLock);

    this->validate();

    for (Strike* strike = fHead; strike != nullptr; strike = strike->fNext) {
        visitor(*strike);
    }
}

size_t SkStrikeCache::internalPurge(size_t minBytesNeeded) {
    size_t bytesNeeded = 0;
    if (fTotalMemoryUsed > fCacheSizeLimit) {
        bytesNeeded = fTotalMemoryUsed - fCacheSizeLimit;
    }
    bytesNeeded = std::max(bytesNeeded, minBytesNeeded);
    if (bytesNeeded) {
        // no small purges!
        bytesNeeded = std::max(bytesNeeded, fTotalMemoryUsed >> 2);
    }

    int countNeeded = 0;
    if (fCacheCount > fCacheCountLimit) {
        countNeeded = fCacheCount - fCacheCountLimit;
        // no small purges!
        countNeeded = std::max(countNeeded, fCacheCount >> 2);
    }

    // early exit
    if (!countNeeded && !bytesNeeded) {
        return 0;
    }

    size_t  bytesFreed = 0;
    int     countFreed = 0;

    // Start at the tail and proceed backwards deleting; the list is in LRU
    // order, with unimportant entries at the tail.
    Strike* strike = fTail;
    while (strike != nullptr && (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        Strike* prev = strike->fPrev;

        // Only delete if the strike is not pinned.
        if (strike->fPinner == nullptr || strike->fPinner->canDelete()) {
            bytesFreed += strike->fMemoryUsed;
            countFreed += 1;
            this->internalRemoveStrike(strike);
        }
        strike = prev;
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

void SkStrikeCache::internalAttachToHead(sk_sp<Strike> strike) {
    SkASSERT(fStrikeLookup.find(strike->getDescriptor()) == nullptr);
    Strike* strikePtr = strike.get();
    fStrikeLookup.set(std::move(strike));
    SkASSERT(nullptr == strikePtr->fPrev && nullptr == strikePtr->fNext);

    fCacheCount += 1;
    fTotalMemoryUsed += strikePtr->fMemoryUsed;

    if (fHead != nullptr) {
        fHead->fPrev = strikePtr;
        strikePtr->fNext = fHead;
    }

    if (fTail == nullptr) {
        fTail = strikePtr;
    }

    fHead = strikePtr; // Transfer ownership of strike to the cache list.
}

void SkStrikeCache::internalRemoveStrike(Strike* strike) {
    SkASSERT(fCacheCount > 0);
    fCacheCount -= 1;
    fTotalMemoryUsed -= strike->fMemoryUsed;

    if (strike->fPrev) {
        strike->fPrev->fNext = strike->fNext;
    } else {
        fHead = strike->fNext;
    }
    if (strike->fNext) {
        strike->fNext->fPrev = strike->fPrev;
    } else {
        fTail = strike->fPrev;
    }

    strike->fPrev = strike->fNext = nullptr;
    strike->fRemoved = true;
    fStrikeLookup.remove(strike->getDescriptor());
}

void SkStrikeCache::validate() const {
#ifdef SK_DEBUG
    size_t computedBytes = 0;
    int computedCount = 0;

    const Strike* strike = fHead;
    while (strike != nullptr) {
        computedBytes += strike->fMemoryUsed;
        computedCount += 1;
        SkASSERT(fStrikeLookup.findOrNull(strike->getDescriptor()) != nullptr);
        strike = strike->fNext;
    }

    if (fCacheCount != computedCount) {
        SkDebugf("fCacheCount: %d, computedCount: %d", fCacheCount, computedCount);
        SK_ABORT("fCacheCount != computedCount");
    }
    if (fTotalMemoryUsed != computedBytes) {
        SkDebugf("fTotalMemoryUsed: %zu, computedBytes: %zu", fTotalMemoryUsed, computedBytes);
        SK_ABORT("fTotalMemoryUsed == computedBytes");
    }
#endif
}

void SkStrikeCache::Strike::updateDelta(size_t increase) {
    if (increase != 0) {
        SkAutoMutexExclusive lock{fStrikeCache->fLock};
        fMemoryUsed += increase;
        if (!fRemoved) {
            fStrikeCache->fTotalMemoryUsed += increase;
        }
    }
}
