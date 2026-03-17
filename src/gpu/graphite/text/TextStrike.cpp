/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/text/TextStrike.h"

#include "src/core/SkGlyph.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/graphite/text/GlyphData.h"

namespace skgpu::graphite {

TextStrike::TextStrike(sktext::gpu::StrikeCache* strikeCache, const SkStrikeSpec& strikeSpec)
        : TextStrikeBase(strikeCache, strikeSpec) {}

sk_sp<TextStrike> TextStrike::GetOrCreate(sktext::gpu::StrikeCache* strikeCache,
                                          const SkStrikeSpec& strikeSpec) {
    auto existingStrike = Find(strikeCache, strikeSpec.descriptor());
    if (existingStrike) {
        return sk_ref_sp(static_cast<TextStrike*>(existingStrike.get()));
    }

    auto newStrike = sk_make_sp<TextStrike>(strikeCache, strikeSpec);
    Add(strikeCache, newStrike);
    return newStrike;
}

GlyphEntry* TextStrike::getGlyph(SkPackedGlyphID packedGlyphID, MaskFormat format) {
    GlyphEntryKey localKey(packedGlyphID, format);
    GlyphEntry* glyph = fCache.findOrNull(localKey);
    if (glyph == nullptr) {
        glyph = fAlloc.make<GlyphEntry>(packedGlyphID, format);
        fCache.set(glyph);
        this->addMemoryUsed(sizeof(GlyphEntry));
    }
    return glyph;
}

const GlyphEntryKey& TextStrike::HashTraits::GetKey(const GlyphEntry* glyph) {
    return glyph->fGlyphEntryKey;
}

uint32_t TextStrike::HashTraits::Hash(GlyphEntryKey key) { return key.hash(); }

}  // namespace skgpu::graphite
