/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/gpu/GlyphVector.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"
#include "src/text/gpu/SubRunAllocator.h"

#include <climits>
#include <optional>
#include <utility>

class SkStrikeClient;

using MaskFormat = skgpu::MaskFormat;

namespace sktext::gpu {
class Glyph;

// -- GlyphVector ----------------------------------------------------------------------------------
GlyphVector::GlyphVector(SkStrikePromise&& strikePromise, SkSpan<Variant> glyphs)
        : fStrikePromise{std::move(strikePromise)}
        , fGlyphs{glyphs} {
    SkASSERT(!fGlyphs.empty());
}

GlyphVector GlyphVector::Make(SkStrikePromise&& promise,
                              SkSpan<const SkPackedGlyphID> packedIDs,
                              SubRunAllocator* alloc) {
    SkASSERT(!packedIDs.empty());
    auto packedIDToVariant = [] (SkPackedGlyphID packedID) {
        return Variant{packedID};
    };

    return GlyphVector{std::move(promise),
                       alloc->makePODArray<Variant>(packedIDs, packedIDToVariant)};
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
    SkASSERT(!fGlyphs.empty());
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
