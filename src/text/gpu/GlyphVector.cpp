/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/GlyphVector.h"

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {

GlyphVector::GlyphVector(sk_sp<SkStrike>&& strike, SkSpan<Variant> glyphs)
        : fSkStrike{std::move(strike)}
        , fGlyphs{glyphs} {
    SkASSERT(fSkStrike != nullptr);
    SkASSERT(fGlyphs.size() > 0);
}

GlyphVector GlyphVector::Make(
        sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, SubRunAllocator* alloc) {
    SkASSERT(strike != nullptr);
    SkASSERT(glyphs.size() > 0);
    Variant* variants = alloc->makePODArray<Variant>(glyphs.size());
    for (auto [i, gv] : SkMakeEnumerate(glyphs)) {
        variants[i] = gv.glyph()->getPackedID();
    }

    return GlyphVector{std::move(strike), SkMakeSpan(variants, glyphs.size())};
}

std::optional<GlyphVector> GlyphVector::MakeFromBuffer(SkReadBuffer& buffer,
                                                       const SkStrikeClient* client,
                                                       SubRunAllocator* alloc) {
    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) { return {}; }

    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) { return {}; }
    }

    sk_sp<SkStrike> strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) { return {}; }

    int32_t glyphCount = buffer.read32();
    // Since the glyph count can never be zero. There was a buffer reading problem.
    if (!buffer.validate(glyphCount > 0)) { return {}; }

    // Make sure we can multiply without overflow in the check below.
    static constexpr int kMaxCount = (int)(INT_MAX / sizeof(uint32_t));
    if (!buffer.validate(glyphCount <= kMaxCount)) { return {}; }

    // Check for enough bytes to populate the packedGlyphID array. If not enough something has
    // gone wrong.
    if (!buffer.validate(glyphCount * sizeof(uint32_t) <= buffer.available())) { return {}; }

    Variant* variants = alloc->makePODArray<Variant>(glyphCount);
    for (int i = 0; i < glyphCount; i++) {
        variants[i].packedGlyphID = SkPackedGlyphID(buffer.readUInt());
    }
    return {GlyphVector{std::move(strike), SkMakeSpan(variants, glyphCount)}};
}

void GlyphVector::flatten(SkWriteBuffer& buffer) {
    // There should never be a glyph vector with zero glyphs.
    SkASSERT(fGlyphs.size() != 0);
    if (!fSkStrike) { SK_ABORT("Can't flatten with already drawn."); }

    fSkStrike->getDescriptor().flatten(buffer);

    // Write out the span of packedGlyphIDs.
    buffer.write32(SkTo<int32_t>(fGlyphs.size()));
    for (auto variant : fGlyphs) {
        buffer.writeUInt(variant.packedGlyphID.value());
    }
}

SkSpan<const Glyph*> GlyphVector::glyphs() const {
    return SkMakeSpan(reinterpret_cast<const Glyph**>(fGlyphs.data()), fGlyphs.size());
}

// packedGlyphIDToGlyph must be run in single-threaded mode.
// If fSkStrike != nullptr then the conversion to Glyph* has not happened.
void GlyphVector::packedGlyphIDToGlyph(StrikeCache* cache) {
    if (fSkStrike != nullptr) {
        fTextStrike = cache->findOrCreateStrike(fSkStrike->strikeSpec());

        for (auto& variant : fGlyphs) {
            variant.glyph = fTextStrike->getGlyph(variant.packedGlyphID);
        }

        // This must be pinned for the Atlas filling to work.
        // TODO(herb): re-enable after the cut on 20220414
        // fSkStrike->verifyPinnedStrike();

        // Drop the ref on the strike that was taken in the SkGlyphRunPainter process* methods.
        fSkStrike = nullptr;
    }
}

}  // namespace sktext::gpu
