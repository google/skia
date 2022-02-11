/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/text/GrGlyphVector.h"

#include "src/core/SkReadBuffer.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkWriteBuffer.h"
#include "src/gpu/text/GrAtlasManager.h"

GrGlyphVector::GrGlyphVector(sk_sp<SkStrike>&& strike, SkSpan<Variant> glyphs)
        : fStrike{std::move(strike)}
        , fGlyphs{glyphs} {
    SkASSERT(fStrike != nullptr);
    SkASSERT(fGlyphs.size() > 0);
}

GrGlyphVector GrGlyphVector::Make(
        sk_sp<SkStrike>&& strike, SkSpan<SkGlyphVariant> glyphs, GrSubRunAllocator* alloc) {
    SkASSERT(strike != nullptr);
    SkASSERT(glyphs.size() > 0);
    Variant* variants = alloc->makePODArray<Variant>(glyphs.size());
    for (auto [i, gv] : SkMakeEnumerate(glyphs)) {
        variants[i] = gv.glyph()->getPackedID();
    }

    return GrGlyphVector{std::move(strike), SkMakeSpan(variants, glyphs.size())};
}

std::optional<GrGlyphVector> GrGlyphVector::MakeFromBuffer(SkReadBuffer& buffer,
                                                           GrSubRunAllocator* alloc) {
    auto descriptor = SkAutoDescriptor::MakeFromBuffer(buffer);
    if (!descriptor.has_value()) { return {}; }

    sk_sp<SkStrike> strike = SkStrikeCache::GlobalStrikeCache()->findStrike(*descriptor->getDesc());

    int32_t glyphCount = buffer.read32();
    // Since the glyph count can never be zero. There was a buffer reading problem.
    if (glyphCount == 0) { return {}; }

    // Make sure we can do the multiply in the check below and not overflow an int.
    if ((int)(INT_MAX / sizeof(uint32_t)) < glyphCount) { return {}; }

    // Check for enough bytes to populate the packedGlyphID array. If not enought something has
    // gone wrong.
    if (glyphCount * sizeof(uint32_t) > buffer.available()) { return {}; }

    Variant* variants = alloc->makePODArray<Variant>(glyphCount);
    for (int i = 0; i < glyphCount; i++) {
        variants[i].packedGlyphID = SkPackedGlyphID(buffer.readUInt());
    }
    return {GrGlyphVector{std::move(strike), SkMakeSpan(variants, glyphCount)}};
}

void GrGlyphVector::flatten(SkWriteBuffer& buffer) {
    // There should never be a glyph vector with zero glyphs.
    SkASSERT(fGlyphs.size() != 0);
    if (!fStrike) { SK_ABORT("Can't flatten with already drawn."); }

    fStrike->getDescriptor().flatten(buffer);

    // Write out the span of packedGlyphIDs.
    buffer.write32(SkTo<int32_t>(fGlyphs.size()));
    for (auto variant : fGlyphs) {
        buffer.writeUInt(variant.packedGlyphID.value());
    }
}

SkSpan<const GrGlyph*> GrGlyphVector::glyphs() const {
    return SkMakeSpan(reinterpret_cast<const GrGlyph**>(fGlyphs.data()), fGlyphs.size());
}

// packedGlyphIDToGrGlyph must be run in single-threaded mode.
// If fStrike != nullptr then the conversion to GrGlyph* has not happened.
void GrGlyphVector::packedGlyphIDToGrGlyph(GrStrikeCache* cache) {
    if (fStrike != nullptr) {
        fGrStrike = cache->findOrCreateStrike(fStrike->strikeSpec());

        for (auto& variant : fGlyphs) {
            variant.grGlyph = fGrStrike->getGlyph(variant.packedGlyphID);
        }

        // Drop the ref on the strike that was taken in the SkGlyphRunPainter process* methods.
        fStrike = nullptr;
    }
}

std::tuple<bool, int> GrGlyphVector::regenerateAtlas(int begin, int end,
                                                     GrMaskFormat maskFormat,
                                                     int srcPadding,
                                                     GrMeshDrawTarget* target,
                                                     bool bilerpPadding) {
    GrAtlasManager* atlasManager = target->atlasManager();
    GrDeferredUploadTarget* uploadTarget = target->deferredUploadTarget();

    uint64_t currentAtlasGen = atlasManager->atlasGeneration(maskFormat);

    this->packedGlyphIDToGrGlyph(target->strikeCache());

    if (fAtlasGeneration != currentAtlasGen) {
        // Calculate the texture coordinates for the vertexes during first use (fAtlasGeneration
        // is set to kInvalidAtlasGeneration) or the atlas has changed in subsequent calls..
        fBulkUseToken.reset();

        SkBulkGlyphMetricsAndImages metricsAndImages{fGrStrike->strikeSpec()};

        // Update the atlas information in the GrStrike.
        auto tokenTracker = uploadTarget->tokenTracker();
        auto glyphs = fGlyphs.subspan(begin, end - begin);
        int glyphsPlacedInAtlas = 0;
        bool success = true;
        for (const Variant& variant : glyphs) {
            GrGlyph* grGlyph = variant.grGlyph;
            SkASSERT(grGlyph != nullptr);

            if (!atlasManager->hasGlyph(maskFormat, grGlyph)) {
                const SkGlyph& skGlyph = *metricsAndImages.glyph(grGlyph->fPackedID);
                auto code = atlasManager->addGlyphToAtlas(
                        skGlyph, grGlyph, srcPadding, target->resourceProvider(),
                        uploadTarget, bilerpPadding);
                if (code != GrDrawOpAtlas::ErrorCode::kSucceeded) {
                    success = code != GrDrawOpAtlas::ErrorCode::kError;
                    break;
                }
            }
            atlasManager->addGlyphToBulkAndSetUseToken(
                    &fBulkUseToken, maskFormat, grGlyph,
                    tokenTracker->nextDrawToken());
            glyphsPlacedInAtlas++;
        }

        // Update atlas generation if there are no more glyphs to put in the atlas.
        if (success && begin + glyphsPlacedInAtlas == SkCount(fGlyphs)) {
            // Need to get the freshest value of the atlas' generation because
            // updateTextureCoordinates may have changed it.
            fAtlasGeneration = atlasManager->atlasGeneration(maskFormat);
        }

        return {success, glyphsPlacedInAtlas};
    } else {
        // The atlas hasn't changed, so our texture coordinates are still valid.
        if (end == SkCount(fGlyphs)) {
            // The atlas hasn't changed and the texture coordinates are all still valid. Update
            // all the plots used to the new use token.
            atlasManager->setUseTokenBulk(fBulkUseToken,
                                          uploadTarget->tokenTracker()->nextDrawToken(),
                                          maskFormat);
        }
        return {true, end - begin};
    }
}

