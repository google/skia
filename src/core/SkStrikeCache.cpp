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
#if !defined(SK_BUILD_FOR_IOS)
    if (gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental) {
        static thread_local auto* cache = new SkStrikeCache;
        return cache;
    }
#endif
    static auto* cache = new SkStrikeCache;
    return cache;
}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(sk_sp<Strike> strike)
    : fStrike{std::move(strike)} {}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr()
    : fStrike{nullptr} {}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(ExclusiveStrikePtr&& o)
    : fStrike{std::move(o.fStrike)} {
    o.fStrike = nullptr;
}

SkStrikeCache::ExclusiveStrikePtr&
SkStrikeCache::ExclusiveStrikePtr::operator = (ExclusiveStrikePtr&& that) {
    fStrike = std::move(that.fStrike);
    return *this;
}

SkStrike* SkStrikeCache::ExclusiveStrikePtr::get() const {
    return fStrike.get();
}

SkStrike* SkStrikeCache::ExclusiveStrikePtr::operator -> () const {
    return this->get();
}

SkStrike& SkStrikeCache::ExclusiveStrikePtr::operator *  () const {
    return *this->get();
}

SkStrikeCache::ExclusiveStrikePtr::operator bool () const {
    return fStrike != nullptr;
}

bool operator == (const SkStrikeCache::ExclusiveStrikePtr& lhs,
                  const SkStrikeCache::ExclusiveStrikePtr& rhs) {
    return lhs.fStrike == rhs.fStrike;
}

bool operator == (const SkStrikeCache::ExclusiveStrikePtr& lhs, decltype(nullptr)) {
    return lhs.fStrike == nullptr;
}

bool operator == (decltype(nullptr), const SkStrikeCache::ExclusiveStrikePtr& rhs) {
    return nullptr == rhs.fStrike;
}

SkStrikeCache::~SkStrikeCache() {
    Strike* strike = fHead;
    while (strike) {
        Strike* next = strike->fNext;
        strike->unref();
        strike = next;
    }
}

SkExclusiveStrikePtr SkStrikeCache::findOrCreateStrikeExclusive(
        const SkDescriptor& desc, const SkScalerContextEffects& effects, const SkTypeface& typeface)
{
    return SkExclusiveStrikePtr(this->findOrCreateStrike(desc, effects, typeface));
}

auto SkStrikeCache::findOrCreateStrike(const SkDescriptor& desc,
                                       const SkScalerContextEffects& effects,
                                       const SkTypeface& typeface) -> sk_sp<Strike> {
    SkAutoSpinlock ac(fLock);
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
    SkDebugf("    count  [ %8zu  %8zu ]\n",
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

SkExclusiveStrikePtr SkStrikeCache::findStrikeExclusive(const SkDescriptor& desc) {
    SkAutoSpinlock ac(fLock);
    sk_sp<SkStrike> result = this->internalFindStrikeOrNull(desc);
    this->internalPurge();
    return SkExclusiveStrikePtr(result);
}

auto SkStrikeCache::internalFindStrikeOrNull(const SkDescriptor& desc) -> sk_sp<Strike> {
    for (Strike* strike = fHead; strike != nullptr; strike = strike->fNext) {
        if (strike->fScalerCache.getDescriptor() == desc) {
            if (fHead != strike) {
                // Make most recently used
                strike->fPrev->fNext = strike->fNext;
                if (strike->fNext != nullptr) {
                    strike->fNext->fPrev = strike->fPrev;
                } else {
                    fTail = strike->fPrev;
                }
                fHead->fPrev = strike;
                strike->fNext = fHead;
                strike->fPrev = nullptr;
                fHead = strike;
            }

            return sk_ref_sp(strike);
        }
    }

    return nullptr;
}

SkExclusiveStrikePtr SkStrikeCache::createStrikeExclusive(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkFontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner)
{
    SkAutoSpinlock ac(fLock);
    return SkExclusiveStrikePtr(
            this->internalCreateStrike(desc, std::move(scaler), maybeMetrics, std::move(pinner)));
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
    SkAutoSpinlock ac(fLock);
    this->internalPurge(fTotalMemoryUsed);
}

size_t SkStrikeCache::getTotalMemoryUsed() const {
    SkAutoSpinlock ac(fLock);
    return fTotalMemoryUsed;
}

int SkStrikeCache::getCacheCountUsed() const {
    SkAutoSpinlock ac(fLock);
    return fCacheCount;
}

int SkStrikeCache::getCacheCountLimit() const {
    SkAutoSpinlock ac(fLock);
    return fCacheCountLimit;
}

size_t SkStrikeCache::setCacheSizeLimit(size_t newLimit) {
    SkAutoSpinlock ac(fLock);

    size_t prevLimit = fCacheSizeLimit;
    fCacheSizeLimit = newLimit;
    this->internalPurge();
    return prevLimit;
}

size_t  SkStrikeCache::getCacheSizeLimit() const {
    SkAutoSpinlock ac(fLock);
    return fCacheSizeLimit;
}

int SkStrikeCache::setCacheCountLimit(int newCount) {
    if (newCount < 0) {
        newCount = 0;
    }

    SkAutoSpinlock ac(fLock);

    int prevCount = fCacheCountLimit;
    fCacheCountLimit = newCount;
    this->internalPurge();
    return prevCount;
}

int SkStrikeCache::getCachePointSizeLimit() const {
    SkAutoSpinlock ac(fLock);
    return fPointSizeLimit;
}

int SkStrikeCache::setCachePointSizeLimit(int newLimit) {
    if (newLimit < 0) {
        newLimit = 0;
    }

    SkAutoSpinlock ac(fLock);

    int prevLimit = fPointSizeLimit;
    fPointSizeLimit = newLimit;
    return prevLimit;
}

void SkStrikeCache::forEachStrike(std::function<void(const Strike&)> visitor) const {
    SkAutoSpinlock ac(fLock);

    this->validate();

    for (Strike* strike = fHead; strike != nullptr; strike = strike->fNext) {
        visitor(*strike);
    }
}

size_t SkStrikeCache::internalPurge(size_t minBytesNeeded) {
    this->validate();

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
    SkASSERT(nullptr == strike->fPrev && nullptr == strike->fNext);

    fCacheCount += 1;
    fTotalMemoryUsed += strike->fMemoryUsed;

    if (fHead) {
        fHead->fPrev = strike.get();
        strike->fNext = fHead;
    }

    if (fTail == nullptr) {
        fTail = strike.get();
    }

    fHead = strike.release(); // Transfer ownership of strike to the cache list.
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
    strike->unref();
}

void SkStrikeCache::validate() const {
#ifdef SK_DEBUG
    size_t computedBytes = 0;
    int computedCount = 0;

    const Strike* strike = fHead;
    while (strike != nullptr) {
        computedBytes += strike->fMemoryUsed;
        computedCount += 1;
        strike = strike->fNext;
    }

    // Can't use SkASSERTF because it looses thread annotations.
    if (fCacheCount != computedCount) {
        SkDebugf("fCacheCount: %d, computedCount: %d", fCacheCount, computedCount);
        SK_ABORT("fCacheCount != computedCount");
    }
    if (fTotalMemoryUsed != computedBytes) {
        SkDebugf("fTotalMemoryUsed: %d, computedBytes: %d", fTotalMemoryUsed, computedBytes);
        SK_ABORT("fTotalMemoryUsed == computedBytes");
    }
#endif
}

void SkStrikeCache::Strike::updateDelta(size_t increase) {
    if (increase != 0) {
        SkAutoSpinlock lock{fStrikeCache->fLock};
        fMemoryUsed += increase;
        if (!fRemoved) {
            fStrikeCache->fTotalMemoryUsed += increase;
        }
    }
}
