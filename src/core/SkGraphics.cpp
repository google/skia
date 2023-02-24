/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkOpenTypeSVGDecoder.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "include/private/base/SkMath.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkCpu.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkOpts.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTypefaceCache.h"

#include <stdlib.h>

void SkGraphics::Init() {
    // SkGraphics::Init() must be thread-safe and idempotent.
    SkCpu::CacheRuntimeFeatures();
    SkOpts::Init();
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

extern bool gSkVMAllowJIT;

void SkGraphics::AllowJIT() {
    gSkVMAllowJIT = true;
}
