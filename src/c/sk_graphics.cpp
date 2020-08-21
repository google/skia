/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkGraphics.h"

#include "include/c/sk_graphics.h"

#include "src/c/sk_types_priv.h"

void sk_graphics_init() {
    SkGraphics::Init();
}

void sk_graphics_purge_font_cache() {
    SkGraphics::PurgeFontCache();
}

void sk_graphics_purge_resource_cache() {
    SkGraphics::PurgeResourceCache();
}

void sk_graphics_purge_all_caches() {
    SkGraphics::PurgeAllCaches();
}

size_t sk_graphics_get_font_cache_used() {
    return SkGraphics::GetFontCacheUsed();
}

size_t sk_graphics_get_font_cache_limit() {
    return SkGraphics::GetFontCacheLimit();
}

size_t sk_graphics_set_font_cache_limit(size_t bytes) {
    return SkGraphics::SetFontCacheLimit(bytes);
}

int sk_graphics_get_font_cache_count_used() {
    return SkGraphics::GetFontCacheCountUsed();
}

int sk_graphics_get_font_cache_count_limit() {
    return SkGraphics::GetFontCacheCountLimit();
}

int sk_graphics_set_font_cache_count_limit(int count) {
    return SkGraphics::SetFontCacheCountLimit(count);
}

int sk_graphics_get_font_cache_point_size_limit() {
    return SkGraphics::GetFontCachePointSizeLimit();
}

int sk_graphics_set_font_cache_point_size_limit(int maxPointSize) {
    return SkGraphics::SetFontCachePointSizeLimit(maxPointSize);
}

size_t sk_graphics_get_resource_cache_total_bytes_used() {
    return SkGraphics::GetResourceCacheTotalBytesUsed();
}

size_t sk_graphics_get_resource_cache_total_byte_limit() {
    return SkGraphics::GetResourceCacheTotalByteLimit();
}

size_t sk_graphics_set_resource_cache_total_byte_limit(size_t newLimit) {
    return SkGraphics::SetResourceCacheTotalByteLimit(newLimit);
}

size_t sk_graphics_get_resource_cache_single_allocation_byte_limit() {
    return SkGraphics::GetResourceCacheSingleAllocationByteLimit();
}

size_t sk_graphics_set_resource_cache_single_allocation_byte_limit(size_t newLimit) {
    return SkGraphics::SetResourceCacheSingleAllocationByteLimit(newLimit);
}

void sk_graphics_dump_memory_statistics(sk_tracememorydump_t* dump) {
    SkGraphics::DumpMemoryStatistics(AsTraceMemoryDump(dump));
}
