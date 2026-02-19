/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_TextStrike_DEFINED
#define skgpu_ganesh_TextStrike_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkTHash.h"
#include "src/text/gpu/StrikeCache.h"

struct SkPackedGlyphID;
class SkStrikeSpec;

namespace skgpu::ganesh {

struct GlyphEntry;

/**
 * Ganesh-specific text strike cache entry.
 *
 * TextStrike manages a cache of Glyph objects for a specific font strike.
 * It inherits the cache management functionality from TextStrikeBase and provides
 * typed access to Ganesh-specific glyphs. Each Glyph object maps from a SkPackedGlyphID
 * to a location in the text atlas system.
 */
class TextStrike final : public sktext::gpu::TextStrikeBase {
public:
    TextStrike(sktext::gpu::StrikeCache* strikeCache, const SkStrikeSpec& strikeSpec);

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

}  // namespace skgpu::ganesh

#endif  // skgpu_ganesh_TextStrike_DEFINED
