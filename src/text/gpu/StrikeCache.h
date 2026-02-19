/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_StrikeCache_DEFINED
#define sktext_gpu_StrikeCache_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTHash.h"

#include <cstddef>
#include <cstdint>

struct SkPackedGlyphID;

//  SK_DEFAULT_GPU_FONT_CACHE_COUNT_LIMIT and SK_DEFAULT_GPU_FONT_CACHE_LIMIT can be set using -D
//  on your ompiler commandline, or by using the defines in SkUserConfig.h
#ifndef SK_DEFAULT_GPU_FONT_CACHE_COUNT_LIMIT
    #define SK_DEFAULT_GPU_FONT_CACHE_COUNT_LIMIT   2048
#endif

#ifndef SK_DEFAULT_GPU_FONT_CACHE_LIMIT
    #define SK_DEFAULT_GPU_FONT_CACHE_LIMIT     (2 * 1024 * 1024)
#endif

namespace sktext::gpu {

class StrikeCache;

/**
 * Abstract base class for backend-specific text strike caches. This allows a
 * shared StrikeCache implementation.
 */
class TextStrikeBase : public SkRefCnt {
public:
    ~TextStrikeBase() override = default;

    const SkStrikeSpec& strikeSpec() const { return fStrikeSpec; }
    const SkDescriptor& getDescriptor() const { return fStrikeSpec.descriptor(); }

    size_t memoryUsed() const { return fMemoryUsed; }

protected:
    TextStrikeBase(StrikeCache* strikeCache, const SkStrikeSpec& strikeSpec)
            : fStrikeCache(strikeCache), fStrikeSpec(strikeSpec) {}

    static sk_sp<TextStrikeBase> Find(const StrikeCache*, const SkDescriptor&);
    static void Add(StrikeCache*, sk_sp<TextStrikeBase>);

    // Called by derived classes when allocating glyphs to update cache accounting
    void addMemoryUsed(size_t bytes);

    StrikeCache* const fStrikeCache;
    const SkStrikeSpec fStrikeSpec;

    // Store for the glyph information (backend-specific glyphs allocated here)
    SkArenaAlloc fAlloc{512};

    // Linked list for LRU cache management
    TextStrikeBase* fNext{nullptr};
    TextStrikeBase* fPrev{nullptr};
    size_t fMemoryUsed{sizeof(TextStrikeBase)};
    bool fRemoved{false};

    friend class StrikeCache;
};

// StrikeCache manages strikes which are indexed by a SkStrike. These strikes can then be
// used to generate individual Glyph Masks.
class StrikeCache {
public:
    ~StrikeCache();

    void freeAll();

private:
    friend class TextStrikeBase;

    void internalRemoveStrike(TextStrikeBase* strike);
    void internalAttachToHead(sk_sp<TextStrikeBase> strike);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0);

    // A simple accounting of what each glyph cache reports and the strike cache total.
    void validate() const;

    TextStrikeBase* fHead{nullptr};
    TextStrikeBase* fTail{nullptr};

    struct HashTraits {
        static const SkDescriptor& GetKey(const sk_sp<TextStrikeBase>& strike);
        static uint32_t Hash(const SkDescriptor& strikeSpec);
    };
    using StrikeHash =
            skia_private::THashTable<sk_sp<TextStrikeBase>, const SkDescriptor&, HashTraits>;

    StrikeHash fCache;

    size_t  fCacheSizeLimit{SK_DEFAULT_GPU_FONT_CACHE_LIMIT};
    size_t  fTotalMemoryUsed{0};
    int32_t fCacheCountLimit{SK_DEFAULT_GPU_FONT_CACHE_COUNT_LIMIT};
    int32_t fCacheCount{0};
};

inline sk_sp<TextStrikeBase> TextStrikeBase::Find(const StrikeCache* cache,
                                                  const SkDescriptor& desc) {
    auto entry = cache->fCache.find(desc);
    return entry ? *entry : nullptr;
}

inline void TextStrikeBase::Add(StrikeCache* cache, sk_sp<TextStrikeBase> strike) {
    SkASSERT(!Find(cache, strike->getDescriptor()));
    cache->internalAttachToHead(std::move(strike));
    cache->internalPurge();
}

}  // namespace sktext::gpu

#endif  // sktext_gpu_StrikeCache_DEFINED
