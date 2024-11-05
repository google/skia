/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/text/gpu/StrikeCache.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/Glyph.h"

#include <algorithm>
#include <optional>
#include <utility>

class SkStrike;

namespace sktext::gpu {

StrikeCache::~StrikeCache() {
    this->freeAll();
}

void StrikeCache::freeAll() {
    this->internalPurge(fTotalMemoryUsed);
}

sk_sp<TextStrike> StrikeCache::findOrCreateStrike(const SkStrikeSpec& strikeSpec) {
    if (sk_sp<TextStrike>* cached = fCache.find(strikeSpec.descriptor())) {
        return *cached;
    }
    sk_sp<TextStrike> strike = this->generateStrike(strikeSpec);
    this->internalPurge();

    return strike;
}

sk_sp<TextStrike> StrikeCache::internalFindStrikeOrNull(const SkDescriptor& desc) {
    // Check head because it is likely the strike we are looking for.
    if (fHead != nullptr && fHead->getDescriptor() == desc) { return sk_ref_sp(fHead); }

    // Do the heavy search looking for the strike.
    sk_sp<TextStrike>* strikeHandle = fCache.find(desc);
    if (strikeHandle == nullptr) { return nullptr; }
    TextStrike* strikePtr = strikeHandle->get();
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

sk_sp<TextStrike> StrikeCache::generateStrike(const SkStrikeSpec& strikeSpec) {
    sk_sp<TextStrike> strike = sk_make_sp<TextStrike>(this, strikeSpec);
    this->internalAttachToHead(strike);
    return strike;
}

size_t StrikeCache::internalPurge(size_t minBytesNeeded) {
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
    TextStrike* strike = fTail;
    while (strike != nullptr && (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        TextStrike* prev = strike->fPrev;

        bytesFreed += strike->fMemoryUsed;
        countFreed += 1;
        this->internalRemoveStrike(strike);

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

void StrikeCache::internalAttachToHead(sk_sp<TextStrike> strike) {
    SkASSERT(fCache.find(strike->getDescriptor()) == nullptr);
    TextStrike* strikePtr = strike.get();
    fCache.set(std::move(strike));
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

void StrikeCache::internalRemoveStrike(TextStrike* strike) {
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
    fCache.remove(strike->getDescriptor());
}

void StrikeCache::validate() const {
#ifdef SK_DEBUG
    size_t computedBytes = 0;
    int computedCount = 0;

    const TextStrike* strike = fHead;
    while (strike != nullptr) {
        computedBytes += strike->fMemoryUsed;
        computedCount += 1;
        SkASSERT(fCache.findOrNull(strike->getDescriptor()) != nullptr);
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

const SkDescriptor& StrikeCache::HashTraits::GetKey(const sk_sp<TextStrike>& strike) {
    return strike->fStrikeSpec.descriptor();
}

uint32_t StrikeCache::HashTraits::Hash(const SkDescriptor& descriptor) {
    return descriptor.getChecksum();
}

TextStrike::TextStrike(StrikeCache* strikeCache, const SkStrikeSpec& strikeSpec)
        : fStrikeCache(strikeCache)
        , fStrikeSpec{strikeSpec} {}

Glyph* TextStrike::getGlyph(SkPackedGlyphID packedGlyphID) {
    Glyph* glyph = fCache.findOrNull(packedGlyphID);
    if (glyph == nullptr) {
        glyph = fAlloc.make<Glyph>(packedGlyphID);
        fCache.set(glyph);
        fMemoryUsed += sizeof(Glyph);
        if (!fRemoved) {
            fStrikeCache->fTotalMemoryUsed += sizeof(Glyph);
        }
    }
    return glyph;
}

const SkPackedGlyphID& TextStrike::HashTraits::GetKey(const Glyph* glyph) {
    return glyph->fPackedID;
}

uint32_t TextStrike::HashTraits::Hash(SkPackedGlyphID key) {
    return key.hash();
}

}  // namespace sktext::gpu

namespace sktext {
std::optional<SkStrikePromise> SkStrikePromise::MakeFromBuffer(
        SkReadBuffer& buffer, const SkStrikeClient* client, SkStrikeCache* strikeCache) {
    std::optional<SkAutoDescriptor> descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) {
        return std::nullopt;
    }

    // If there is a client, then this from a different process. Translate the SkTypefaceID from
    // the strike server (Renderer) process to strike client (GPU) process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) {
            return std::nullopt;
        }
    }

    sk_sp<SkStrike> strike = strikeCache->findStrike(*descriptor->getDesc());
    SkASSERT(strike != nullptr);
    if (!buffer.validate(strike != nullptr)) {
        return std::nullopt;
    }

    return SkStrikePromise{std::move(strike)};
}
}  // namespace sktext
