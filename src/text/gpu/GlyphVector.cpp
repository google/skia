/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/GlyphVector.h"
#include <optional>
#include <variant>

#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {
// -- StrikeRef ------------------------------------------------------------------------------------
StrikeRef::StrikeRef(sk_sp<SkStrike>&& strike) : fStrike{std::move(strike)} {
    SkASSERT(std::get<sk_sp<SkStrike>>(fStrike) != nullptr);
}

StrikeRef::StrikeRef(SkStrikeForGPU* strike) : fStrike {strike} {
    SkASSERT(std::get<SkStrikeForGPU*>(fStrike) != nullptr);
}

StrikeRef::StrikeRef(StrikeRef&&) = default;
StrikeRef& StrikeRef::operator=(StrikeRef&&) = default;

void StrikeRef::flatten(SkWriteBuffer& buffer) const {
    if (std::holds_alternative<std::monostate>(fStrike)) {
        SK_ABORT("Can't flatten. getStrikeAndSetToNullptr has already been called.");
    }

    if (const sk_sp<SkStrike>* strikePtr = std::get_if<sk_sp<SkStrike>>(&fStrike)) {
        (*strikePtr)->getDescriptor().flatten(buffer);
    } else if (SkStrikeForGPU*const* GPUstrikePtr = std::get_if<SkStrikeForGPU*>(&fStrike)) {
        (*GPUstrikePtr)->getDescriptor().flatten(buffer);
    }
}

std::optional<StrikeRef> StrikeRef::MakeFromBuffer(SkReadBuffer& buffer,
                                                   const SkStrikeClient* client) {
    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!buffer.validate(descriptor.has_value())) {
        return std::nullopt;
    }

    // If there is a client, then this from a different process. Translate the typeface id from
    // that process to this process.
    if (client != nullptr) {
        if (!client->translateTypefaceID(&descriptor.value())) {
            return std::nullopt;
        }
    }

    sk_sp<SkStrike> strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());
    if (!buffer.validate(strike != nullptr)) {
        return std::nullopt;
    }

    return StrikeRef{std::move(strike)};
}

sk_sp<SkStrike> StrikeRef::getStrikeAndSetToNullptr() {
    if (std::holds_alternative<sk_sp<SkStrike>>(fStrike)) {
        // Force a copy out of the variant because there is no more efficient way to do it.
        sk_sp<SkStrike> strike = std::get<sk_sp<SkStrike>>(fStrike);
        fStrike = std::monostate();
        return strike;
    }
    return nullptr;
}

// -- GlyphVector ----------------------------------------------------------------------------------
GlyphVector::GlyphVector(sktext::gpu::StrikeRef&& strikeRef, SkSpan<Variant> glyphs)
        : fStrikeRef{std::move(strikeRef)}
        , fGlyphs{glyphs} {
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
    std::optional<StrikeRef> strikeRef = StrikeRef::MakeFromBuffer(buffer, client);
    if (!buffer.validate(strikeRef.has_value())) {
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
    return GlyphVector{std::move(strikeRef.value()), SkSpan(variants, glyphCount)};
}

void GlyphVector::flatten(SkWriteBuffer& buffer) const {
    // There should never be a glyph vector with zero glyphs.
    SkASSERT(fGlyphs.size() != 0);
    fStrikeRef.flatten(buffer);

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
    if (sk_sp<SkStrike> strike = fStrikeRef.getStrikeAndSetToNullptr()) {
        fTextStrike = cache->findOrCreateStrike(strike->strikeSpec());

        for (Variant& variant : fGlyphs) {
            variant.glyph = fTextStrike->getGlyph(variant.packedGlyphID);
        }

        // This must be pinned for the Atlas filling to work.
        strike->verifyPinnedStrike();
    }
}
}  // namespace sktext::gpu
