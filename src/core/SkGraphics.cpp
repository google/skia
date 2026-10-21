/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"

#include "src/core/SkContextPriv.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTypefaceCache.h"

void SkGraphics::Init() {
    // SkGraphics::Init() must be thread-safe and idempotent.
    SkContexts::GetGlobalSkContext();
}

///////////////////////////////////////////////////////////////////////////////

void SkGraphics::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    SkResourceCache::DumpMemoryStatistics(dump);
    SkStrikeCache::DumpMemoryStatistics(dump);
}

void SkGraphics::PurgeAllCaches() {
    SkGraphics::PurgeFontCache();
    SkGraphics::PurgeResourceCache();
    SkImageFilter_Base::PurgeCache();
}

///////////////////////////////////////////////////////////////////////////////

size_t SkGraphics::GetFontCacheLimit() {
    return SkContexts::GetGlobalSkContext()->fontCacheLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return SkContexts::GetGlobalSkContext()->setFontCacheLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() { return SkContexts::GetGlobalSkContext()->fontCacheUsed(); }

int SkGraphics::GetFontCacheCountLimit() {
    return SkContexts::GetGlobalSkContext()->fontCacheCountLimit();
}

int SkGraphics::SetFontCacheCountLimit(int count) {
    return SkContexts::GetGlobalSkContext()->setFontCacheCountLimit(count);
}

int SkGraphics::GetFontCacheCountUsed() {
    return SkContexts::GetGlobalSkContext()->fontCacheCountUsed();
}

void SkGraphics::PurgeFontCache() {
    SkContexts::GetGlobalSkContext()->purgeFontCache();
    SkTypefaceCache::PurgeAll();
}

void SkGraphics::PurgePinnedFontCache() {
    SkContexts::GetGlobalSkContext()->purgePinnedFontCache();
}

size_t SkGraphics::GetResourceCacheTotalBytesUsed() {
    return SkContexts::GetGlobalSkContext()->resourceCacheTotalBytesUsed();
}

size_t SkGraphics::GetResourceCacheTotalByteLimit() {
    return SkContexts::GetGlobalSkContext()->resourceCacheTotalByteLimit();
}

size_t SkGraphics::SetResourceCacheTotalByteLimit(size_t newLimit) {
    return SkContexts::GetGlobalSkContext()->setResourceCacheTotalByteLimit(newLimit);
}

size_t SkGraphics::GetResourceCacheSingleAllocationByteLimit() {
    return SkContexts::GetGlobalSkContext()->resourceCacheSingleAllocationByteLimit();
}

size_t SkGraphics::SetResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return SkContexts::GetGlobalSkContext()->setResourceCacheSingleAllocationByteLimit(newLimit);
}

void SkGraphics::PurgeResourceCache() {
    SkImageFilter_Base::PurgeCache();
    return SkContexts::GetGlobalSkContext()->purgeResourceCache();
}

int SkGraphics::GetTypefaceCacheCountLimit() {
    return SkContexts::GetGlobalSkContext()->typefaceCacheCountLimit();
}

int SkGraphics::SetTypefaceCacheCountLimit(int count) {
    return SkContexts::GetGlobalSkContext()->setTypefaceCacheCountLimit(count);
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::SetOpenTypeSVGDecoderFactory(
        OpenTypeSVGDecoderFactory svgDecoderFactory) {
    SkSharedContext* shrdCtx = SkContexts::GetGlobalSkContext()->priv().sharedContext();
    return shrdCtx->setSVGDecoderFactory(svgDecoderFactory);
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::GetOpenTypeSVGDecoderFactory() {
    SkSharedContext* shrdCtx = SkContexts::GetGlobalSkContext()->priv().sharedContext();
    return shrdCtx->svgDecoderFactory();
}
