/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSharedContext.h"

#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
#include "include/private/chromium/SkDiscardableMemory.h"
#endif

#ifndef SK_DEFAULT_IMAGE_CACHE_LIMIT
#define SK_DEFAULT_IMAGE_CACHE_LIMIT (32 * 1024 * 1024)
#endif

SkSharedContext::SkSharedContext() {
#if defined(SK_USE_DISCARDABLE_SCALEDIMAGECACHE)
    fResourceCache = std::make_unique<SkSynchronizedResourceCache>(SkDiscardableMemory::Create);
#else
    fResourceCache = std::make_unique<SkSynchronizedResourceCache>(SK_DEFAULT_IMAGE_CACHE_LIMIT);
#endif

    fFontCache = std::make_unique<SkStrikeCache>();
    fTypefaceCache = std::make_unique<SkTypefaceCache>();
}

SkSharedContext::~SkSharedContext() = default;
