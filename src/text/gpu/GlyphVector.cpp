/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/GlyphVector.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"

#include <optional>

class SkStrikeClient;

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {
// -- GlyphVector ----------------------------------------------------------------------------------
GlyphVector::GlyphVector(SkStrikePromise&& strikePromise, SkSpan<Variant> glyphs)
        : fStrikePromise{std::move(strikePromise)}
        , fGlyphs{glyphs} {
    SkASSERT(fGlyphs.size() > 0);
}

GlyphVector::Variant*
GlyphVector::MakeGlyphs(SkSpan<SkPackedGlyphID> glyphs, sktext::gpu::SubRunAllocator* alloc) {
    Variant* variants = alloc->makePODArray<Variant>(glyphs.size());
    for (auto [i, gv] : SkMakeEnumerate(glyphs)) {
        variants[i] = gv;
    }
    return variants;
}

GlyphVector GlyphVector::Make(
        SkStrikePromise&& promise, SkSpan<SkPackedGlyphID> glyphs, SubRunAllocator* alloc) {
    SkASSERT(glyphs.size() > 0);
    Variant* variants = MakeGlyphs(glyphs, alloc);
    return GlyphVector{std::move(promise), SkSpan(variants, glyphs.size())};
}

std::optional<GlyphVector> GlyphVector::MakeFromBuffer(SkReadBuffer& buffer,
                                                       const SkStrikeClient* client,
                                                       SubRunAllocator* alloc) {
    std::optional<SkStrikePromise> promise =
            SkStrikePromise::MakeFromBuffer(buffer, client, SkStrikeCache::GlobalStrikeCache());
    if (!buffer.validate(promise.has_value())) {
        return std::nullopt;
    }

    int32_t glyphCount = buffer.read32();
    // Since the glyph count can never be zero. There was a buffer reading problem.
    if (!buffer.validate(glyphCount > 0)) {
        return std::nullopt;
    }

    // Make sure we can multiply without overflow in the check below.
    static constexpr int kMaxCount = (int)(INT_MAX / sizeof(uint32_t));
    if (!buffer.validate(glyphCount <= kMaxCount)) {
        return std::nullopt;
    }

    // Check for enough bytes to populate the packedGlyphID array. If not enough something has
    // gone wrong.
    if (!buffer.validate(glyphCount * sizeof(uint32_t) <= buffer.available())) {
        return std::nullopt;
    }

    Variant* variants = alloc->makePODArray<Variant>(glyphCount);
    for (int i = 0; i < glyphCount; i++) {
        variants[i].packedGlyphID = SkPackedGlyphID(buffer.readUInt());
    }
    return GlyphVector{std::move(promise.value()), SkSpan(variants, glyphCount)};
}

void GlyphVector::flatten(SkWriteBuffer& buffer) const {
    // There should never be a glyph vector with zero glyphs.
    SkASSERT(fGlyphs.size() != 0);
    fStrikePromise.flatten(buffer);

    // Write out the span of packedGlyphIDs.
    buffer.write32(SkTo<int32_t>(fGlyphs.size()));
    for (Variant variant : fGlyphs) {
        buffer.writeUInt(variant.packedGlyphID.value());
    }
}

SkSpan<const Glyph*> GlyphVector::glyphs() const {
    return SkSpan(reinterpret_cast<const Glyph**>(fGlyphs.data()), fGlyphs.size());
}

// packedGlyphIDToGlyph must be run in single-threaded mode.
// If fSkStrike is not sk_sp<SkStrike> then the conversion to Glyph* has not happened.
void GlyphVector::packedGlyphIDToGlyph(StrikeCache* cache) {
    if (fTextStrike == nullptr) {
        SkStrike* strike = fStrikePromise.strike();
        fTextStrike = cache->findOrCreateStrike(strike->strikeSpec());

        // Get all the atlas locations for each glyph.
        for (Variant& variant : fGlyphs) {
            variant.glyph = fTextStrike->getGlyph(variant.packedGlyphID);
        }

        // This must be pinned for the Atlas filling to work.
        strike->verifyPinnedStrike();

        // Drop the ref to the strike so that it can be purged if needed.
        fStrikePromise.resetStrike();
    }
}
}  // namespace sktext::gpu
