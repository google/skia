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

class Glyph;
class StrikeCache;

// The TextStrike manages an SkArenaAlloc for Glyphs. The SkStrike is what actually creates
// the mask. The TextStrike may outlive the generating SkStrike. However, it retains a copy
// of it's SkDescriptor as a key to access (or regenerate) the SkStrike. TextStrikes are
// created by and owned by a StrikeCache.
class TextStrike : public SkNVRefCnt<TextStrike> {
public:
    TextStrike(StrikeCache* strikeCache,
               const SkStrikeSpec& strikeSpec);

    Glyph* getGlyph(SkPackedGlyphID);
    const SkStrikeSpec& strikeSpec() const { return fStrikeSpec; }
    const SkDescriptor& getDescriptor() const { return fStrikeSpec.descriptor(); }

private:
    StrikeCache* const fStrikeCache;

    // Key for retrieving the SkStrike for creating new atlas data.
    const SkStrikeSpec fStrikeSpec;

    struct HashTraits {
        static const SkPackedGlyphID& GetKey(const Glyph* glyph);
        static uint32_t Hash(SkPackedGlyphID key);
    };
    // Map SkPackedGlyphID -> Glyph*.
    skia_private::THashTable<Glyph*, SkPackedGlyphID, HashTraits> fCache;

    // Store for the glyph information.
    SkArenaAlloc fAlloc{512};

    TextStrike*  fNext{nullptr};
    TextStrike*  fPrev{nullptr};
    size_t       fMemoryUsed{sizeof(TextStrike)};
    bool         fRemoved{false};

    friend class StrikeCache;
};

// StrikeCache manages strikes which are indexed by a SkStrike. These strikes can then be
// used to generate individual Glyph Masks.
class StrikeCache {
public:
    ~StrikeCache();

    // The user of the cache may hold a long-lived ref to the returned strike.
    sk_sp<TextStrike> findOrCreateStrike(const SkStrikeSpec& strikeSpec);

    void freeAll();

private:
    friend class TextStrike;  // for TextStrike::getGlyph
    sk_sp<TextStrike> internalFindStrikeOrNull(const SkDescriptor& desc);
    sk_sp<TextStrike> generateStrike(const SkStrikeSpec& strikeSpec);

    void internalRemoveStrike(TextStrike* strike);
    void internalAttachToHead(sk_sp<TextStrike> strike);

    // Checkout budgets, modulated by the specified min-bytes-needed-to-purge,
    // and attempt to purge caches to match.
    // Returns number of bytes freed.
    size_t internalPurge(size_t minBytesNeeded = 0);

    // A simple accounting of what each glyph cache reports and the strike cache total.
    void validate() const;

    TextStrike* fHead{nullptr};
    TextStrike* fTail{nullptr};

    struct HashTraits {
        static const SkDescriptor& GetKey(const sk_sp<TextStrike>& strike);
        static uint32_t Hash(const SkDescriptor& strikeSpec);
    };
    using StrikeHash = skia_private::THashTable<sk_sp<TextStrike>, const SkDescriptor&, HashTraits>;

    StrikeHash fCache;

    size_t  fCacheSizeLimit{SK_DEFAULT_GPU_FONT_CACHE_LIMIT};
    size_t  fTotalMemoryUsed{0};
    int32_t fCacheCountLimit{SK_DEFAULT_GPU_FONT_CACHE_COUNT_LIMIT};
    int32_t fCacheCount{0};
};

}  // namespace sktext::gpu

#endif  // sktext_gpu_StrikeCache_DEFINED
