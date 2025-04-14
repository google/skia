/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"

#include "src/core/SkBitmapProcState.h"
#include "src/core/SkBlitMask.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkCpu.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkMemset.h"
#include "src/core/SkOpts.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/core/SkTypefaceCache.h"

void SkGraphics::Init() {
    // SkGraphics::Init() must be thread-safe and idempotent.
    SkCpu::CacheRuntimeFeatures();
    SkOpts::Init();
    SkOpts::Init_BitmapProcState();
    SkOpts::Init_BlitMask();
    SkOpts::Init_BlitRow();
    SkOpts::Init_Memset();
    SkOpts::Init_Swizzler();
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
    return SkStrikeCache::GlobalStrikeCache()->getCacheSizeLimit();
}

size_t SkGraphics::SetFontCacheLimit(size_t bytes) {
    return SkStrikeCache::GlobalStrikeCache()->setCacheSizeLimit(bytes);
}

size_t SkGraphics::GetFontCacheUsed() {
    return SkStrikeCache::GlobalStrikeCache()->getTotalMemoryUsed();
}

int SkGraphics::GetFontCacheCountLimit() {
    return SkStrikeCache::GlobalStrikeCache()->getCacheCountLimit();
}

int SkGraphics::SetFontCacheCountLimit(int count) {
    return SkStrikeCache::GlobalStrikeCache()->setCacheCountLimit(count);
}

int SkGraphics::GetFontCacheCountUsed() {
    return SkStrikeCache::GlobalStrikeCache()->getCacheCountUsed();
}

void SkGraphics::PurgeFontCache() {
    SkStrikeCache::GlobalStrikeCache()->purgeAll();
    SkTypefaceCache::PurgeAll();
}

void SkGraphics::PurgePinnedFontCache() {
    SkStrikeCache::GlobalStrikeCache()->purgePinned();
}

size_t SkGraphics::GetResourceCacheTotalBytesUsed() { return SkResourceCache::GetTotalBytesUsed(); }

size_t SkGraphics::GetResourceCacheTotalByteLimit() { return SkResourceCache::GetTotalByteLimit(); }

size_t SkGraphics::SetResourceCacheTotalByteLimit(size_t newLimit) {
    return SkResourceCache::SetTotalByteLimit(newLimit);
}

size_t SkGraphics::GetResourceCacheSingleAllocationByteLimit() {
    return SkResourceCache::GetSingleAllocationByteLimit();
}

size_t SkGraphics::SetResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return SkResourceCache::SetSingleAllocationByteLimit(newLimit);
}

void SkGraphics::PurgeResourceCache() {
    SkImageFilter_Base::PurgeCache();
    return SkResourceCache::PurgeAll();
}

static int gTypefaceCacheCountLimit = 1024; // historical default value

int SkGraphics::GetTypefaceCacheCountLimit() {
    return gTypefaceCacheCountLimit;
}

int SkGraphics::SetTypefaceCacheCountLimit(int count) {
    const int prev = gTypefaceCacheCountLimit;
    gTypefaceCacheCountLimit = count;
    return prev;
}

static SkGraphics::OpenTypeSVGDecoderFactory gSVGDecoderFactory = nullptr;

SkGraphics::OpenTypeSVGDecoderFactory
SkGraphics::SetOpenTypeSVGDecoderFactory(OpenTypeSVGDecoderFactory svgDecoderFactory) {
    OpenTypeSVGDecoderFactory old(gSVGDecoderFactory);
    gSVGDecoderFactory = svgDecoderFactory;
    return old;
}

SkGraphics::OpenTypeSVGDecoderFactory SkGraphics::GetOpenTypeSVGDecoderFactory() {
    return gSVGDecoderFactory;
}
