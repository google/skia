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

namespace sktext::gpu {

GlyphVector::GlyphVector(SkStrikePromise&& strikePromise, SkSpan<GlyphBytes> glyphs)
        : fStrikePromise{std::move(strikePromise)}, fGlyphs{glyphs} {
    SkASSERT(!fGlyphs.empty());
}

GlyphVector::GlyphVector(GlyphVector&& that)
        : fStrikePromise{std::move(that.fStrikePromise)}, fGlyphs{that.fGlyphs} {
    // We move this when creating AtlasSubRuns, before initializing backend data.
    // To support backend data we'd have to record a move proc in initBackendData to
    // do type-specific move.
    SkASSERT(!that.hasBackendData());
}

GlyphVector::~GlyphVector() {
    if (!this->hasBackendData()) {
        return;
    }
    fBackendDataReleaser(fBackendDataBytes.data());
}

GlyphVector GlyphVector::Make(SkStrikePromise&& promise,
                              SkSpan<const SkPackedGlyphID> packedIDs,
                              SubRunAllocator* alloc) {
    SkASSERT(!packedIDs.empty());
    int count = SkToInt(packedIDs.size());
    GlyphBytes* glyphs =
        alloc->makePODArray<GlyphBytes, GlyphVector_Concepts::kMaxGlyphTypeSize>(count);
    for (int i = 0; i < count; i++) {
        *reinterpret_cast<SkPackedGlyphID*>(glyphs[i].data()) = packedIDs[i];
    }
    return GlyphVector{std::move(promise), SkSpan{glyphs, count}};
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

    // Make sure we won't overflow the glyphCount math or allocation below.
    if (!buffer.validate(BagOfBytes::WillCountFit<GlyphBytes>(glyphCount))) {
        return std::nullopt;
    }

    // Check for enough bytes to populate the packedGlyphID array. If not enough something has
    // gone wrong.
    static_assert(sizeof(GlyphBytes) >= sizeof(uint32_t));
    if (!buffer.validate(glyphCount * sizeof(uint32_t) <= buffer.available())) {
        return std::nullopt;
    }

    GlyphBytes* glyphs = alloc->makePODArray<GlyphBytes>(glyphCount);
    for (int i = 0; i < glyphCount; i++) {
        *reinterpret_cast<SkPackedGlyphID*>(glyphs[i].data()) = SkPackedGlyphID(buffer.readUInt());
    }
    return GlyphVector{std::move(promise.value()), SkSpan{glyphs, glyphCount}};
}

void GlyphVector::flatten(SkWriteBuffer& buffer) const {
    // There should never be a glyph vector with zero glyphs.
    SkASSERT(!fGlyphs.empty());
    fStrikePromise.flatten(buffer);

    // Write out the span of packedGlyphIDs.
    buffer.write32(SkTo<int32_t>(fGlyphs.size()));
    for (const auto& g : fGlyphs) {
        SkPackedGlyphID id;
        if (this->hasBackendData()) {
            id = fGetGlyphID(g.data());
        } else {
            id = *reinterpret_cast<const SkPackedGlyphID*>(g.data());
        }
        buffer.writeUInt(id.value());
    }
}

}  // namespace sktext::gpu
