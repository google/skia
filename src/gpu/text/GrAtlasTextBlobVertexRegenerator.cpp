/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextBlob.h"
#include "GrTextUtils.h"
#include "SkDistanceFieldGen.h"
#include "SkGlyphCache.h"
#include "ops/GrAtlasTextOp.h"

using Regenerator = GrAtlasTextBlob::VertexRegenerator;

enum RegenMask {
    kNoRegen    = 0x0,
    kRegenPos   = 0x1,
    kRegenCol   = 0x2,
    kRegenTex   = 0x4,
    kRegenGlyph = 0x8 | kRegenTex,  // we have to regenerate the texture coords when we regen glyphs

    // combinations
    kRegenPosCol = kRegenPos | kRegenCol,
    kRegenPosTex = kRegenPos | kRegenTex,
    kRegenPosTexGlyph = kRegenPos | kRegenGlyph,
    kRegenPosColTex = kRegenPos | kRegenCol | kRegenTex,
    kRegenPosColTexGlyph = kRegenPos | kRegenCol | kRegenGlyph,
    kRegenColTex = kRegenCol | kRegenTex,
    kRegenColTexGlyph = kRegenCol | kRegenGlyph,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// A large template to handle regenerating the vertices of a textblob with as few branches as
// possible
template <bool regenPos, bool regenCol, bool regenTexCoords>
inline void regen_vertices(char* vertex, const GrGlyph* glyph, size_t vertexStride,
                           bool useDistanceFields, SkScalar transX, SkScalar transY,
                           GrColor color) {
    uint16_t u0, v0, u1, v1;
    if (regenTexCoords) {
        SkASSERT(glyph);
        int width = glyph->fBounds.width();
        int height = glyph->fBounds.height();

        if (useDistanceFields) {
            u0 = glyph->fAtlasLocation.fX + SK_DistanceFieldInset;
            v0 = glyph->fAtlasLocation.fY + SK_DistanceFieldInset;
            u1 = u0 + width - 2 * SK_DistanceFieldInset;
            v1 = v0 + height - 2 * SK_DistanceFieldInset;
        } else {
            u0 = glyph->fAtlasLocation.fX;
            v0 = glyph->fAtlasLocation.fY;
            u1 = u0 + width;
            v1 = v0 + height;
        }
        // We pack the 2bit page index in the low bit of the u and v texture coords
        uint32_t pageIndex = glyph->pageIndex();
        SkASSERT(pageIndex < 4);
        uint16_t uBit = (pageIndex >> 1) & 0x1;
        uint16_t vBit = pageIndex & 0x1;
        u0 <<= 1;
        u0 |= uBit;
        v0 <<= 1;
        v0 |= vBit;
        u1 <<= 1;
        u1 |= uBit;
        v1 <<= 1;
        v1 |= vBit;
    }

    // This is a bit wonky, but sometimes we have LCD text, in which case we won't have color
    // vertices, hence vertexStride - sizeof(SkIPoint16)
    intptr_t texCoordOffset = vertexStride - sizeof(SkIPoint16);
    intptr_t colorOffset = texCoordOffset - sizeof(GrColor);

    // V0
    if (regenPos) {
        SkPoint* point = reinterpret_cast<SkPoint*>(vertex);
        point->fX += transX;
        point->fY += transY;
    }

    if (regenCol) {
        SkColor* vcolor = reinterpret_cast<SkColor*>(vertex + colorOffset);
        *vcolor = color;
    }

    if (regenTexCoords) {
        uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
        textureCoords[0] = u0;
        textureCoords[1] = v0;
    }
    vertex += vertexStride;

    // V1
    if (regenPos) {
        SkPoint* point = reinterpret_cast<SkPoint*>(vertex);
        point->fX += transX;
        point->fY += transY;
    }

    if (regenCol) {
        SkColor* vcolor = reinterpret_cast<SkColor*>(vertex + colorOffset);
        *vcolor = color;
    }

    if (regenTexCoords) {
        uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
        textureCoords[0] = u0;
        textureCoords[1] = v1;
    }
    vertex += vertexStride;

    // V2
    if (regenPos) {
        SkPoint* point = reinterpret_cast<SkPoint*>(vertex);
        point->fX += transX;
        point->fY += transY;
    }

    if (regenCol) {
        SkColor* vcolor = reinterpret_cast<SkColor*>(vertex + colorOffset);
        *vcolor = color;
    }

    if (regenTexCoords) {
        uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
        textureCoords[0] = u1;
        textureCoords[1] = v0;
    }
    vertex += vertexStride;

    // V3
    if (regenPos) {
        SkPoint* point = reinterpret_cast<SkPoint*>(vertex);
        point->fX += transX;
        point->fY += transY;
    }

    if (regenCol) {
        SkColor* vcolor = reinterpret_cast<SkColor*>(vertex + colorOffset);
        *vcolor = color;
    }

    if (regenTexCoords) {
        uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
        textureCoords[0] = u1;
        textureCoords[1] = v1;
    }
}

Regenerator::VertexRegenerator(GrAtlasTextBlob* blob, int runIdx, int subRunIdx,
                               const SkMatrix& viewMatrix, SkScalar x, SkScalar y, GrColor color,
                               GrDeferredUploadTarget* uploadTarget, GrAtlasGlyphCache* glyphCache,
                               SkAutoGlyphCache* lazyCache)
        : fViewMatrix(viewMatrix)
        , fBlob(blob)
        , fUploadTarget(uploadTarget)
        , fGlyphCache(glyphCache)
        , fLazyCache(lazyCache)
        , fRun(&blob->fRuns[runIdx])
        , fSubRun(&blob->fRuns[runIdx].fSubRunInfo[subRunIdx])
        , fColor(color) {
    // Compute translation if any
    fSubRun->computeTranslation(fViewMatrix, x, y, &fTransX, &fTransY);

    // Because the GrAtlasGlyphCache may evict the strike a blob depends on using for
    // generating its texture coords, we have to track whether or not the strike has
    // been abandoned.  If it hasn't been abandoned, then we can use the GrGlyph*s as is
    // otherwise we have to get the new strike, and use that to get the correct glyphs.
    // Because we do not have the packed ids, and thus can't look up our glyphs in the
    // new strike, we instead keep our ref to the old strike and use the packed ids from
    // it.  These ids will still be valid as long as we hold the ref.  When we are done
    // updating our cache of the GrGlyph*s, we drop our ref on the old strike
    if (fSubRun->strike()->isAbandoned()) {
        fRegenFlags |= kRegenGlyph;
        fRegenFlags |= kRegenTex;
    }
    if (kARGB_GrMaskFormat != fSubRun->maskFormat() && fSubRun->color() != color) {
        fRegenFlags |= kRegenCol;
    }
    if (0.f != fTransX || 0.f != fTransY) {
        fRegenFlags |= kRegenPos;
    }
}

template <bool regenPos, bool regenCol, bool regenTexCoords, bool regenGlyphs>
Regenerator::Result Regenerator::doRegen() {
    static_assert(!regenGlyphs || regenTexCoords, "must regenTexCoords along regenGlyphs");
    GrAtlasTextStrike* strike = nullptr;
    if (regenTexCoords) {
        fSubRun->resetBulkUseToken();

        const SkDescriptor* desc = (fRun->fOverrideDescriptor && !fSubRun->drawAsDistanceFields())
                                           ? fRun->fOverrideDescriptor->getDesc()
                                           : fRun->fDescriptor.getDesc();

        if (!*fLazyCache || (*fLazyCache)->getDescriptor() != *desc) {
            SkScalerContextEffects effects;
            effects.fPathEffect = fRun->fPathEffect.get();
            effects.fRasterizer = fRun->fRasterizer.get();
            effects.fMaskFilter = fRun->fMaskFilter.get();
            fLazyCache->reset(SkGlyphCache::DetachCache(fRun->fTypeface.get(), effects, desc));
        }

        if (regenGlyphs) {
            strike = fGlyphCache->getStrike(fLazyCache->get());
        } else {
            strike = fSubRun->strike();
        }
    }

    bool hasW = fSubRun->hasWCoord();
    Result result;
    auto vertexStride = GetVertexStride(fSubRun->maskFormat(), hasW);
    char* currVertex = fBlob->fVertices + fSubRun->vertexStartIndex() +
                       fCurrGlyph * kVerticesPerGlyph * vertexStride;
    result.fFirstVertex = currVertex;

    for (int glyphIdx = fCurrGlyph; glyphIdx < (int)fSubRun->glyphCount(); glyphIdx++) {
        GrGlyph* glyph = nullptr;
        if (regenTexCoords) {
            size_t glyphOffset = glyphIdx + fSubRun->glyphStartIndex();

            if (regenGlyphs) {
                // Get the id from the old glyph, and use the new strike to lookup
                // the glyph.
                GrGlyph::PackedID id = fBlob->fGlyphs[glyphOffset]->fPackedID;
                fBlob->fGlyphs[glyphOffset] =
                        strike->getGlyph(id, fSubRun->maskFormat(), fLazyCache->get());
                SkASSERT(id == fBlob->fGlyphs[glyphOffset]->fPackedID);
            }
            glyph = fBlob->fGlyphs[glyphOffset];
            SkASSERT(glyph && glyph->fMaskFormat == fSubRun->maskFormat());

            if (!fGlyphCache->hasGlyph(glyph) &&
                !strike->addGlyphToAtlas(fUploadTarget, glyph, fLazyCache->get(),
                                         fSubRun->maskFormat())) {
                fBrokenRun = glyphIdx > 0;
                result.fFinished = false;
                return result;
            }
            fGlyphCache->addGlyphToBulkAndSetUseToken(fSubRun->bulkUseToken(), glyph,
                                                      fUploadTarget->nextDrawToken());
        }

        regen_vertices<regenPos, regenCol, regenTexCoords>(currVertex, glyph, vertexStride,
                                                           fSubRun->drawAsDistanceFields(), fTransX,
                                                           fTransY, fColor);
        currVertex += vertexStride * GrAtlasTextOp::kVerticesPerGlyph;
        ++result.fGlyphsRegenerated;
        ++fCurrGlyph;
    }

    // We may have changed the color so update it here
    fSubRun->setColor(fColor);
    if (regenTexCoords) {
        if (regenGlyphs) {
            fSubRun->setStrike(strike);
        }
        fSubRun->setAtlasGeneration(fBrokenRun
                                            ? GrDrawOpAtlas::kInvalidAtlasGeneration
                                            : fGlyphCache->atlasGeneration(fSubRun->maskFormat()));
    }
    return result;
}

Regenerator::Result Regenerator::regenerate() {
    uint64_t currentAtlasGen = fGlyphCache->atlasGeneration(fSubRun->maskFormat());
    // If regenerate() is called multiple times then the atlas gen may have changed. So we check
    // this each time.
    if (fSubRun->atlasGeneration() != currentAtlasGen) {
        fRegenFlags |= kRegenTex;
    }

    switch (static_cast<RegenMask>(fRegenFlags)) {
        case kRegenPos:
            return this->doRegen<true, false, false, false>();
        case kRegenCol:
            return this->doRegen<false, true, false, false>();
        case kRegenTex:
            return this->doRegen<false, false, true, false>();
        case kRegenGlyph:
            return this->doRegen<false, false, true, true>();

        // combinations
        case kRegenPosCol:
            return this->doRegen<true, true, false, false>();
        case kRegenPosTex:
            return this->doRegen<true, false, true, false>();
        case kRegenPosTexGlyph:
            return this->doRegen<true, false, true, true>();
        case kRegenPosColTex:
            return this->doRegen<true, true, true, false>();
        case kRegenPosColTexGlyph:
            return this->doRegen<true, true, true, true>();
        case kRegenColTex:
            return this->doRegen<false, true, true, false>();
        case kRegenColTexGlyph:
            return this->doRegen<false, true, true, true>();
        case kNoRegen: {
            Result result;
            bool hasW = fSubRun->hasWCoord();
            auto vertexStride = GetVertexStride(fSubRun->maskFormat(), hasW);
            result.fGlyphsRegenerated = fSubRun->glyphCount() - fCurrGlyph;
            result.fFirstVertex = fBlob->fVertices + fSubRun->vertexStartIndex() +
                                  fCurrGlyph * kVerticesPerGlyph * vertexStride;
            fCurrGlyph = fSubRun->glyphCount();

            // set use tokens for all of the glyphs in our subrun.  This is only valid if we
            // have a valid atlas generation
            fGlyphCache->setUseTokenBulk(*fSubRun->bulkUseToken(), fUploadTarget->nextDrawToken(),
                                         fSubRun->maskFormat());
            return result;
        }
    }
    SK_ABORT("Should not get here");
    return Result();
}
