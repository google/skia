/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/GlyphVector.h"
#include <variant>

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {
GlyphVector::GlyphVector(sk_sp<SkStrike>&& strike, SkSpan<Variant> glyphs)
        : fStrike{std::move(strike)}
        , fGlyphs{glyphs} {
    SkASSERT(std::get<sk_sp<SkStrike>>(fStrike) != nullptr);
    SkASSERT(fGlyphs.size() > 0);
}

GlyphVector::GlyphVector(SkStrikeForGPU* strike, SkSpan<Variant> glyphs)
        : fStrike{strike}
        , fGlyphs{glyphs} {
    SkASSERT(std::get<SkStrikeForGPU*>(fStrike) != nullptr);
    SkASSERT(fGlyphs.size() > 0);
}

GlyphVector::Variant*
GlyphVector::MakeGlyphs(SkSpan<SkGlyphVariant> glyphs, sktext::gpu::SubRunAllocator* alloc) {
    Variant* variants = alloc->makePODArray<Variant>(glyphs.size());
    for (auto [i, gv] : SkMakeEnumerate(glyphs)) {
        variants[i] = gv.glyph()->getPackedID();
    }
    return variants;
}

GlyphVector GlyphVector::Make(
        sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, SubRunAllocator* alloc) {
    SkASSERT(strike != nullptr);
    SkASSERT(glyphs.size() > 0);
    Variant* variants = MakeGlyphs(glyphs, alloc);
    return GlyphVector{std::move(strike), SkSpan(variants, glyphs.size())};
}

GlyphVector GlyphVector::Make(
        SkStrikeForGPU* strike, SkSpan<SkGlyphVariant> glyphs, SubRunAllocator* alloc) {
    SkASSERT(strike != nullptr);
    SkASSERT(glyphs.size() > 0);
    Variant* variants = MakeGlyphs(glyphs, alloc);
    return GlyphVector{strike, SkSpan(variants, glyphs.size())};
}

std::optional<GlyphVector> GlyphVector::MakeFromBuffer(SkReadBuffer& buffer,
                                                       const SkStrikeClient* client,
                                                       SubRunAllocator* alloc) {
    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) {
        return std::nullopt;
    }

    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) {
            return std::nullopt;
        }
    }

    sk_sp<SkStrike> strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) {
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
    return GlyphVector{std::move(strike), SkSpan(variants, glyphCount)};
}

void GlyphVector::flatten(SkWriteBuffer& buffer) {
    // There should never be a glyph vector with zero glyphs.
    SkASSERT(fGlyphs.size() != 0);
    if (std::holds_alternative<std::monostate>(fStrike)) {
        SK_ABORT("Can't flatten with already drawn.");
    }

    if (sk_sp<SkStrike>* strikePtr = std::get_if<sk_sp<SkStrike>>(&fStrike)) {
        (*strikePtr)->getDescriptor().flatten(buffer);
    } else if (SkStrikeForGPU** GPUstrikePtr = std::get_if<SkStrikeForGPU*>(&fStrike)) {
        (*GPUstrikePtr)->getDescriptor().flatten(buffer);
    }

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
    if (std::holds_alternative<sk_sp<SkStrike>>(fStrike)) {
        SkStrike* strike = std::get<sk_sp<SkStrike>>(fStrike).get();
        fTextStrike = cache->findOrCreateStrike(strike->strikeSpec());

        for (Variant& variant : fGlyphs) {
            variant.glyph = fTextStrike->getGlyph(variant.packedGlyphID);
        }

        // This must be pinned for the Atlas filling to work.
        strike->verifyPinnedStrike();

        // Drop the ref on the strike that was taken in the SkGlyphRunPainter process* methods.
        fStrike = std::monostate();
    }
}
}  // namespace sktext::gpu
