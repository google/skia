/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_TextStrike_DEFINED
#define skgpu_graphite_TextStrike_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkTHash.h"
#include "src/text/gpu/StrikeCache.h"

struct SkPackedGlyphID;
class SkStrikeSpec;

namespace skgpu::graphite {

struct GlyphEntry;

/**
 * Graphite-specific text strike cache.
 *
 * TextStrike manages a cache of Glyph objects for a specific font strike.
 */
class TextStrike final : public sktext::gpu::TextStrikeBase {
public:
    TextStrike(sktext::gpu::StrikeCache* strikeCache, const SkStrikeSpec& strikeSpec);

    // Find or create a TextStrike for the given strikeSpec
    static sk_sp<TextStrike> GetOrCreate(sktext::gpu::StrikeCache* strikeCache,
                                         const SkStrikeSpec& strikeSpec);

    GlyphEntry* getGlyph(SkPackedGlyphID packedGlyphID);

private:
    struct HashTraits {
        static const SkPackedGlyphID& GetKey(const GlyphEntry* glyph);
        static uint32_t Hash(SkPackedGlyphID key);
    };
    skia_private::THashTable<GlyphEntry*, SkPackedGlyphID, HashTraits> fCache;

    friend class sktext::gpu::StrikeCache;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_TextStrike_DEFINED
