/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeCache.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMutex.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"

#include <algorithm>
#include <utility>

class SkScalerContext;
struct SkFontMetrics;

using namespace sktext;

bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = false;

SkStrikeCache* SkStrikeCache::GlobalStrikeCache() {
    if (gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental) {
        static thread_local auto* cache = new SkStrikeCache;
        return cache;
    }
    static auto* cache = new SkStrikeCache;
    return cache;
}

auto SkStrikeCache::findOrCreateStrike(const SkStrikeSpec& strikeSpec) -> sk_sp<SkStrike> {
    SkAutoMutexExclusive ac(fLock);
    sk_sp<SkStrike> strike = this->internalFindStrikeOrNull(strikeSpec.descriptor());
    if (strike == nullptr) {
        strike = this->internalCreateStrike(strikeSpec);
    }
    this->internalPurge();
    return strike;
}

sk_sp<StrikeForGPU> SkStrikeCache::findOrCreateScopedStrike(const SkStrikeSpec& strikeSpec) {
    return this->findOrCreateStrike(strikeSpec);
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

    auto visitor = [](const SkStrike& strike) {
        strike.dump();
    };

    GlobalStrikeCache()->forEachStrike(visitor);
}

void SkStrikeCache::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    dump->dumpNumericValue(kGlyphCacheDumpName, "size", "bytes", SkGraphics::GetFontCacheUsed());
    dump->dumpNumericValue(kGlyphCacheDumpName, "budget_size", "bytes",
                           SkGraphics::GetFontCacheLimit());
    dump->dumpNumericValue(kGlyphCacheDumpName, "glyph_count", "objects",
                           SkGraphics::GetFontCacheCountUsed());
    dump->dumpNumericValue(kGlyphCacheDumpName, "budget_glyph_count", "objects",
                           SkGraphics::GetFontCacheCountLimit());

    if (dump->getRequestedDetails() == SkTraceMemoryDump::kLight_LevelOfDetail) {
        dump->setMemoryBacking(kGlyphCacheDumpName, "malloc", nullptr);
        return;
    }

    auto visitor = [&](const SkStrike& strike) {
        strike.dumpMemoryStatistics(dump);
    };

    GlobalStrikeCache()->forEachStrike(visitor);
}

sk_sp<SkStrike> SkStrikeCache::findStrike(const SkDescriptor& desc) {
    SkAutoMutexExclusive ac(fLock);
    sk_sp<SkStrike> result = this->internalFindStrikeOrNull(desc);
    this->internalPurge();
    return result;
}

auto SkStrikeCache::internalFindStrikeOrNull(const SkDescriptor& desc) -> sk_sp<SkStrike> {

    // Check head because it is likely the strike we are looking for.
    if (fHead != nullptr && fHead->getDescriptor() == desc) { return sk_ref_sp(fHead); }

    // Do the heavy search looking for the strike.
    sk_sp<SkStrike>* strikeHandle = fStrikeLookup.find(desc);
    if (strikeHandle == nullptr) { return nullptr; }
    SkStrike* strikePtr = strikeHandle->get();
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
        const SkStrikeSpec& strikeSpec,
        SkFontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner) {
    SkAutoMutexExclusive ac(fLock);
    return this->internalCreateStrike(strikeSpec, maybeMetrics, std::move(pinner));
}

auto SkStrikeCache::internalCreateStrike(
        const SkStrikeSpec& strikeSpec,
        SkFontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner) -> sk_sp<SkStrike> {
    std::unique_ptr<SkScalerContext> scaler = strikeSpec.createScalerContext();
    auto strike =
        sk_make_sp<SkStrike>(this, strikeSpec, std::move(scaler), maybeMetrics, std::move(pinner));
    this->internalAttachToHead(strike);
    return strike;
}

void SkStrikeCache::purgePinned(size_t minBytesNeeded) {
    SkAutoMutexExclusive ac(fLock);
    this->internalPurge(minBytesNeeded, /* checkPinners= */ true);
}

void SkStrikeCache::purgeAll() {
    SkAutoMutexExclusive ac(fLock);
    this->internalPurge(fTotalMemoryUsed, /* checkPinners= */ true);
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

void SkStrikeCache::forEachStrike(std::function<void(const SkStrike&)> visitor) const {
    SkAutoMutexExclusive ac(fLock);

    this->validate();

    for (SkStrike* strike = fHead; strike != nullptr; strike = strike->fNext) {
        visitor(*strike);
    }
}

size_t SkStrikeCache::internalPurge(size_t minBytesNeeded, bool checkPinners) {
#ifndef SK_STRIKE_CACHE_DOESNT_AUTO_CHECK_PINNERS
    // Temporarily default to checking pinners, for staging.
    checkPinners = true;
#endif

    if (fPinnerCount == fCacheCount && !checkPinners)
        return 0;

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
    SkStrike* strike = fTail;
    while (strike != nullptr && (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        SkStrike* prev = strike->fPrev;

        // Only delete if the strike is not pinned.
        if (strike->fPinner == nullptr || (checkPinners && strike->fPinner->canDelete())) {
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

void SkStrikeCache::internalAttachToHead(sk_sp<SkStrike> strike) {
    SkASSERT(fStrikeLookup.find(strike->getDescriptor()) == nullptr);
    SkStrike* strikePtr = strike.get();
    fStrikeLookup.set(std::move(strike));
    SkASSERT(nullptr == strikePtr->fPrev && nullptr == strikePtr->fNext);

    fCacheCount += 1;
    fPinnerCount += strikePtr->fPinner != nullptr ? 1 : 0;
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

void SkStrikeCache::internalRemoveStrike(SkStrike* strike) {
    SkASSERT(fCacheCount > 0);
    fCacheCount -= 1;
    fPinnerCount -= strike->fPinner != nullptr ? 1 : 0;
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

    const SkStrike* strike = fHead;
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

const SkDescriptor& SkStrikeCache::StrikeTraits::GetKey(const sk_sp<SkStrike>& strike) {
    return strike->getDescriptor();
}

uint32_t SkStrikeCache::StrikeTraits::Hash(const SkDescriptor& descriptor) {
    return descriptor.getChecksum();
}


