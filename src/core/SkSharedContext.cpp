/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSharedContext.h"

#include "include/core/SkContextOptions.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
#include "include/private/chromium/SkDiscardableMemory.h"
#endif

SkSharedContext::SkSharedContext(const SkContextOptions& options) {
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

SkSharedContext::~SkSharedContext() = default;

size_t SkSharedContext::fontCacheLimit() const { return fStrikeCache->getCacheSizeLimit(); }

size_t SkSharedContext::setFontCacheLimit(size_t bytes) {
    return fStrikeCache->setCacheSizeLimit(bytes);
}

size_t SkSharedContext::fontCacheUsed() const { return fStrikeCache->getTotalMemoryUsed(); }

int SkSharedContext::fontCacheCountLimit() const { return fStrikeCache->getCacheCountLimit(); }

int SkSharedContext::setFontCacheCountLimit(int count) {
    return fStrikeCache->setCacheCountLimit(count);
}

int SkSharedContext::fontCacheCountUsed() const { return fStrikeCache->getCacheCountUsed(); }

void SkSharedContext::purgeFontCache() { fStrikeCache->purgeAll(); }

void SkSharedContext::purgePinnedFontCache() { fStrikeCache->purgePinned(); }

size_t SkSharedContext::resourceCacheTotalBytesUsed() const {
    return fResourceCache->getTotalBytesUsed();
}

size_t SkSharedContext::resourceCacheTotalByteLimit() const {
    return fResourceCache->getTotalByteLimit();
}

size_t SkSharedContext::setResourceCacheTotalByteLimit(size_t newLimit) {
    return fResourceCache->setTotalByteLimit(newLimit);
}

size_t SkSharedContext::resourceCacheSingleAllocationByteLimit() const {
    return fResourceCache->getSingleAllocationByteLimit();
}

size_t SkSharedContext::setResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return fResourceCache->setSingleAllocationByteLimit(newLimit);
}

void SkSharedContext::purgeResourceCache() { fResourceCache->purgeAll(); }
