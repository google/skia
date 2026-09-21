/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextOptions_DEFINED
#define SkContextOptions_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkAPI.h"

#include <cstddef>

struct SK_API SkContextOptions {
    SkContextOptions() = default;

    /**
     * Maximum number of entries in the typeface cache. Each entry corresponds to a cached
     * SkTypeface. (1024 is the historical default value.)
     */
    int fTypefaceCacheCountLimit = 1024;

    /**
     * Maximum total memory (in bytes) for the CPU resource cache, used for temporary bitmaps,
     * scaled images, and other decoded resources. Entries are purged from the cache when memory
     * usage exceeds this limit.
     */
    size_t fResourceCacheTotalByteLimit = 32 * 1024 * 1024;

    /**
     * Maximum size (in bytes) for a single allocation in the CPU resource cache. When a cacheable
     * entry is very large (e.g. a large scaled bitmap), adding it to the cache can cause most or
     * all existing entries to be purged. If an entry's size exceeds this limit, it is not cached
     * at all.
     *
     * 0 means no separate single-allocation cap beyond fResourceCacheTotalByteLimit.
     */
    size_t fResourceCacheSingleAllocationByteLimit = 0;

    /**
     * Maximum number of entries (strikes) in the font cache. A cache entry is associated with each
     * unique combination of typeface, point size, and matrix.
     */
    int fFontCacheCountLimit = 2048;

    /**
     * Maximum total memory (in bytes) used by the font/strike cache for glyph metrics, masks, and
     * paths. If the cache needs to allocate more, it will purge previous entries.
     */
    size_t fFontCacheLimit = 2 * 1024 * 1024;
};

#endif  // SkContextOptions_DEFINED
