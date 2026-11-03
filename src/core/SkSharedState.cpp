/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSharedState.h"

#include "include/core/SkContextOptions.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
#include "include/private/chromium/SkDiscardableMemory.h"
#endif

SkSharedState::SkSharedState(const SkContextOptions& options) {
#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
    fResourceCache = std::make_unique<SkSynchronizedResourceCache>(SkDiscardableMemory::Create);
#else
    fResourceCache =
            std::make_unique<SkSynchronizedResourceCache>(options.fResourceCacheTotalByteLimit);
#endif

    fStrikeCache = std::make_unique<SkStrikeCache>();

    fTypefaceCache = std::make_unique<SkTypefaceCache>();
    fTypefaceCacheCountLimit = options.fTypefaceCacheCountLimit;

    fResourceCache->setSingleAllocationByteLimit(options.fResourceCacheSingleAllocationByteLimit);
    fStrikeCache->setCacheCountLimit(options.fFontCacheCountLimit);
    fStrikeCache->setCacheSizeLimit(options.fFontCacheLimit);
}

SkSharedState::~SkSharedState() = default;

size_t SkSharedState::fontCacheLimit() const { return fStrikeCache->getCacheSizeLimit(); }

size_t SkSharedState::setFontCacheLimit(size_t bytes) {
    return fStrikeCache->setCacheSizeLimit(bytes);
}

size_t SkSharedState::fontCacheUsed() const { return fStrikeCache->getTotalMemoryUsed(); }

int SkSharedState::fontCacheCountLimit() const { return fStrikeCache->getCacheCountLimit(); }

int SkSharedState::setFontCacheCountLimit(int count) {
    return fStrikeCache->setCacheCountLimit(count);
}

int SkSharedState::fontCacheCountUsed() const { return fStrikeCache->getCacheCountUsed(); }

void SkSharedState::purgeFontCache() { fStrikeCache->purgeAll(); }

void SkSharedState::purgePinnedFontCache() { fStrikeCache->purgePinned(); }

size_t SkSharedState::resourceCacheTotalBytesUsed() const {
    return fResourceCache->getTotalBytesUsed();
}

size_t SkSharedState::resourceCacheTotalByteLimit() const {
    return fResourceCache->getTotalByteLimit();
}

size_t SkSharedState::setResourceCacheTotalByteLimit(size_t newLimit) {
    return fResourceCache->setTotalByteLimit(newLimit);
}

size_t SkSharedState::resourceCacheSingleAllocationByteLimit() const {
    return fResourceCache->getSingleAllocationByteLimit();
}

size_t SkSharedState::setResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return fResourceCache->setSingleAllocationByteLimit(newLimit);
}

void SkSharedState::purgeResourceCache() { fResourceCache->purgeAll(); }
