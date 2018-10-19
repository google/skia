/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGraphics.h"

#include "SkBlitter.h"
#include "SkCanvas.h"
#include "SkCpu.h"
#include "SkGeometry.h"
#include "SkImageFilter.h"
#include "SkMath.h"
#include "SkMatrix.h"
#include "SkOpts.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkRefCnt.h"
#include "SkResourceCache.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkStrikeCache.h"
#include "SkTSearch.h"
#include "SkTime.h"
#include "SkTypefaceCache.h"
#include "SkUTF.h"

#include <stdlib.h>

void SkGraphics::GetVersion(int32_t* major, int32_t* minor, int32_t* patch) {
    if (major) {
        *major = SKIA_VERSION_MAJOR;
    }
    if (minor) {
        *minor = SKIA_VERSION_MINOR;
    }
    if (patch) {
        *patch = SKIA_VERSION_PATCH;
    }
}

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
    SkImageFilter::PurgeCache();
}

///////////////////////////////////////////////////////////////////////////////

static const char kFontCacheLimitStr[] = "font-cache-limit";
static const size_t kFontCacheLimitLen = sizeof(kFontCacheLimitStr) - 1;

static const struct {
    const char* fStr;
    size_t fLen;
    size_t (*fFunc)(size_t);
} gFlags[] = {
    { kFontCacheLimitStr, kFontCacheLimitLen, SkGraphics::SetFontCacheLimit }
};

/* flags are of the form param; or param=value; */
void SkGraphics::SetFlags(const char* flags) {
    if (!flags) {
        return;
    }
    const char* nextSemi;
    do {
        size_t len = strlen(flags);
        const char* paramEnd = flags + len;
        const char* nextEqual = strchr(flags, '=');
        if (nextEqual && paramEnd > nextEqual) {
            paramEnd = nextEqual;
        }
        nextSemi = strchr(flags, ';');
        if (nextSemi && paramEnd > nextSemi) {
            paramEnd = nextSemi;
        }
        size_t paramLen = paramEnd - flags;
        for (int i = 0; i < (int)SK_ARRAY_COUNT(gFlags); ++i) {
            if (paramLen != gFlags[i].fLen) {
                continue;
            }
            if (strncmp(flags, gFlags[i].fStr, paramLen) == 0) {
                size_t val = 0;
                if (nextEqual) {
                    val = (size_t) atoi(nextEqual + 1);
                }
                (gFlags[i].fFunc)(val);
                break;
            }
        }
        flags = nextSemi + 1;
    } while (nextSemi);
}

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

int SkGraphics::GetFontCachePointSizeLimit() {
    return SkStrikeCache::GlobalStrikeCache()->getCachePointSizeLimit();
}

int SkGraphics::SetFontCachePointSizeLimit(int limit) {
    return SkStrikeCache::GlobalStrikeCache()->setCachePointSizeLimit(limit);
}

void SkGraphics::PurgeFontCache() {
    SkStrikeCache::GlobalStrikeCache()->purgeAll();
    SkTypefaceCache::PurgeAll();
}
