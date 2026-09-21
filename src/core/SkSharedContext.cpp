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

    fFontCache = std::make_unique<SkStrikeCache>();

    fTypefaceCache = std::make_unique<SkTypefaceCache>();
    fTypefaceCacheCountLimit = options.fTypefaceCacheCountLimit;

    fResourceCache->setSingleAllocationByteLimit(options.fResourceCacheSingleAllocationByteLimit);
    fFontCache->setCacheCountLimit(options.fFontCacheCountLimit);
    fFontCache->setCacheSizeLimit(options.fFontCacheLimit);
}

SkSharedContext::~SkSharedContext() = default;
