/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeCache_DEFINED
#define SkStrikeCache_DEFINED

#include "SkDescriptor.h"
#include "SkSpinlock.h"
#include "SkTemplates.h"
#include "SkArenaAlloc.h"

class SkGlyphCache;
class SkTraceMemoryDump;

#ifndef SK_DEFAULT_FONT_CACHE_COUNT_LIMIT
    #define SK_DEFAULT_FONT_CACHE_COUNT_LIMIT   2048
#endif

#ifndef SK_DEFAULT_FONT_CACHE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

#ifndef SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT
    #define SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT  256
#endif

///////////////////////////////////////////////////////////////////////////////

class SkGlyphCache;

class SkStrikeCache {
public:
    SkStrikeCache() = default;
    ~SkStrikeCache();

    static void AttachCache(SkGlyphCache* cache);

    using ExclusiveStrikePtr =
        std::unique_ptr<
            SkGlyphCache,
            SkFunctionWrapper<void, SkGlyphCache, SkStrikeCache::AttachCache>>;

    static ExclusiveStrikePtr FindStrikeExclusive(const SkDescriptor&);

    static std::unique_ptr<SkScalerContext> CreateScalerContext(
        const SkDescriptor&, const SkScalerContextEffects&, const SkTypeface&);

    static ExclusiveStrikePtr CreateStrikeExclusive(
            const SkDescriptor& desc,
            std::unique_ptr<SkScalerContext> scaler,
            SkPaint::FontMetrics* maybeMetrics = nullptr);

    static void PurgeAll();

    static void Dump();

    // Dump memory usage statistics of all the attaches caches in the process using the
    // SkTraceMemoryDump interface.
    static void DumpMemoryStatistics(SkTraceMemoryDump* dump);

    // call when a glyphcache is available for caching (i.e. not in use)
    void attachCache(SkGlyphCache *cache);
    ExclusiveStrikePtr findStrikeExclusive(const SkDescriptor&);

    void purgeAll(); // does not change budget

    int getCacheCountLimit() const;
    int setCacheCountLimit(int limit);
    int getCacheCountUsed() const;

    size_t getCacheSizeLimit() const;
    size_t setCacheSizeLimit(size_t limit);
    size_t getTotalMemoryUsed() const;

    int  getCachePointSizeLimit() const;
    int  setCachePointSizeLimit(int limit);

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

private:
    friend class SkGlyphCache;
    struct Node {
        Node(const SkDescriptor& desc) : fDesc{desc} {}
        const SkDescriptor& getDescriptor() const {return *fDesc.getDesc(); }
        SkGlyphCache* fNext{nullptr};
        SkGlyphCache* fPrev{nullptr};
        SkAutoDescriptor fDesc;
    };

    // The following methods can only be called when mutex is already held.
    SkGlyphCache* internalGetHead() const { return fHead; }
    SkGlyphCache* internalGetTail() const;
    void internalDetachCache(SkGlyphCache*);
    void internalAttachCacheToHead(SkGlyphCache*);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0);

    void forEachStrike(std::function<void(const SkGlyphCache&)> visitor) const;

    mutable SkSpinlock fLock;
    SkGlyphCache*      fHead{nullptr};
    size_t             fTotalMemoryUsed{0};
    size_t             fCacheSizeLimit{SK_DEFAULT_FONT_CACHE_LIMIT};
    int32_t            fCacheCountLimit{SK_DEFAULT_FONT_CACHE_COUNT_LIMIT};
    int32_t            fCacheCount{0};
    int32_t            fPointSizeLimit{SK_DEFAULT_FONT_CACHE_POINT_SIZE_LIMIT};
};

using SkExclusiveStrikePtr = SkStrikeCache::ExclusiveStrikePtr;

#endif  // SkStrikeCache_DEFINED
