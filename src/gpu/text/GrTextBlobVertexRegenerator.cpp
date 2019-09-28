/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDistanceFieldGen.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrTextBlob.h"
#include "src/gpu/text/GrTextTarget.h"

enum RegenMask {
    kNoRegen    = 0x0,
    kRegenPos   = 0x1,
    kRegenCol   = 0x2,
    kRegenTex   = 0x4,
    kRegenGlyph = 0x8,
};

////////////////////////////////////////////////////////////////////////////////////////////////////

static void regen_positions(char* vertex, size_t vertexStride, SkScalar transX, SkScalar transY) {
    SkPoint* point = reinterpret_cast<SkPoint*>(vertex);
    for (int i = 0; i < 4; ++i) {
        point->fX += transX;
        point->fY += transY;
        point = SkTAddOffset<SkPoint>(point, vertexStride);
    }
}

static void regen_colors(char* vertex, size_t vertexStride, GrColor color) {
    // This is a bit wonky, but sometimes we have LCD text, in which case we won't have color
    // vertices, hence vertexStride - sizeof(SkIPoint16)
    size_t colorOffset = vertexStride - sizeof(SkIPoint16) - sizeof(GrColor);
    GrColor* vcolor = reinterpret_cast<GrColor*>(vertex + colorOffset);
    for (int i = 0; i < 4; ++i) {
        *vcolor = color;
        vcolor = SkTAddOffset<GrColor>(vcolor, vertexStride);
    }
}

static void regen_texcoords(char* vertex, size_t vertexStride, const GrGlyph* glyph,
                            bool useDistanceFields) {
    // This is a bit wonky, but sometimes we have LCD text, in which case we won't have color
    // vertices, hence vertexStride - sizeof(SkIPoint16)
    size_t texCoordOffset = vertexStride - sizeof(SkIPoint16);

    uint16_t u0, v0, u1, v1;
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

    uint16_t* textureCoords = reinterpret_cast<uint16_t*>(vertex + texCoordOffset);
    textureCoords[0] = u0;
    textureCoords[1] = v0;
    textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    textureCoords[0] = u0;
    textureCoords[1] = v1;
    textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    textureCoords[0] = u1;
    textureCoords[1] = v0;
    textureCoords = SkTAddOffset<uint16_t>(textureCoords, vertexStride);
    textureCoords[0] = u1;
    textureCoords[1] = v1;

#ifdef DISPLAY_PAGE_INDEX
    // Enable this to visualize the page from which each glyph is being drawn.
    // Green Red Magenta Cyan -> 0 1 2 3; Black -> error
    GrColor hackColor;
    switch (pageIndex) {
        case 0:
            hackColor = GrColorPackRGBA(0, 255, 0, 255);
            break;
        case 1:
            hackColor = GrColorPackRGBA(255, 0, 0, 255);;
            break;
        case 2:
            hackColor = GrColorPackRGBA(255, 0, 255, 255);
            break;
        case 3:
            hackColor = GrColorPackRGBA(0, 255, 255, 255);
            break;
        default:
            hackColor = GrColorPackRGBA(0, 0, 0, 255);
            break;
    }
    regen_colors(vertex, vertexStride, hackColor);
#endif
}

GrTextBlob::VertexRegenerator::VertexRegenerator(GrResourceProvider* resourceProvider,
                                                 GrTextBlob* blob,
                                                 int runIdx, int subRunIdx,
                                                 const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                                 GrColor color,
                                                 GrDeferredUploadTarget* uploadTarget,
                                                 GrStrikeCache* glyphCache,
                                                 GrAtlasManager* fullAtlasManager,
                                                 SkExclusiveStrikePtr* lazyStrike)
        : fResourceProvider(resourceProvider)
        , fViewMatrix(viewMatrix)
        , fBlob(blob)
        , fUploadTarget(uploadTarget)
        , fGlyphCache(glyphCache)
        , fFullAtlasManager(fullAtlasManager)
        , fLazyStrike(lazyStrike)
        , fSubRun(&blob->fRuns[runIdx].fSubRunInfo[subRunIdx])
        , fColor(color) {
    // Compute translation if any
    fSubRun->computeTranslation(fViewMatrix, x, y, &fTransX, &fTransY);

    // Because the GrStrikeCache may evict the strike a blob depends on using for
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

bool GrTextBlob::VertexRegenerator::doRegen(GrTextBlob::VertexRegenerator::Result* result,
                                            bool regenPos, bool regenCol, bool regenTexCoords,
                                            bool regenGlyphs) {
    SkASSERT(!regenGlyphs || regenTexCoords);
    sk_sp<GrTextStrike> strike;
    if (regenTexCoords) {
        fSubRun->resetBulkUseToken();

        const SkStrikeSpec& strikeSpec = fSubRun->strikeSpec();

        if (!*fLazyStrike || (*fLazyStrike)->getDescriptor() != strikeSpec.descriptor()) {
            *fLazyStrike =
                    strikeSpec.findOrCreateExclusiveStrike(SkStrikeCache::GlobalStrikeCache());
        }

        if (regenGlyphs) {
            strike = strikeSpec.findOrCreateGrStrike(fGlyphCache);
        } else {
            strike = fSubRun->refStrike();
        }
    }

    bool hasW = fSubRun->hasWCoord();
    auto vertexStride = GetVertexStride(fSubRun->maskFormat(), hasW);
    char* currVertex = fBlob->fVertices + fSubRun->vertexStartIndex() +
                       fCurrGlyph * kVerticesPerGlyph * vertexStride;
    result->fFirstVertex = currVertex;

    for (int glyphIdx = fCurrGlyph; glyphIdx < (int)fSubRun->glyphCount(); glyphIdx++) {
        GrGlyph* glyph = nullptr;
        if (regenTexCoords) {
            size_t glyphOffset = glyphIdx + fSubRun->glyphStartIndex();

            if (regenGlyphs) {
                // Get the id from the old glyph, and use the new strike to lookup
                // the glyph.
                SkPackedGlyphID id = fBlob->fGlyphs[glyphOffset]->fPackedID;
                fBlob->fGlyphs[glyphOffset] = strike->getGlyph(id, fLazyStrike->get());
                SkASSERT(id == fBlob->fGlyphs[glyphOffset]->fPackedID);
            }
            glyph = fBlob->fGlyphs[glyphOffset];
            SkASSERT(glyph && glyph->fMaskFormat == fSubRun->maskFormat());

            if (!fFullAtlasManager->hasGlyph(glyph)) {
                GrDrawOpAtlas::ErrorCode code;
                code = strike->addGlyphToAtlas(fResourceProvider, fUploadTarget, fGlyphCache,
                                              fFullAtlasManager, glyph,
                                              fLazyStrike->get(), fSubRun->maskFormat(),
                                              fSubRun->needsTransform());
                if (GrDrawOpAtlas::ErrorCode::kError == code) {
                    // Something horrible has happened - drop the op
                    return false;
                }
                else if (GrDrawOpAtlas::ErrorCode::kTryAgain == code) {
                    fBrokenRun = glyphIdx > 0;
                    result->fFinished = false;
                    return true;
                }
            }
            auto tokenTracker = fUploadTarget->tokenTracker();
            fFullAtlasManager->addGlyphToBulkAndSetUseToken(fSubRun->bulkUseToken(), glyph,
                                                            tokenTracker->nextDrawToken());
        }

        if (regenPos) {
            regen_positions(currVertex, vertexStride, fTransX, fTransY);
        }
        if (regenCol) {
            regen_colors(currVertex, vertexStride, fColor);
        }
        if (regenTexCoords) {
            regen_texcoords(currVertex, vertexStride, glyph, fSubRun->drawAsDistanceFields());
        }

        currVertex += vertexStride * GrAtlasTextOp::kVerticesPerGlyph;
        ++result->fGlyphsRegenerated;
        ++fCurrGlyph;
    }

    // We may have changed the color so update it here
    fSubRun->setColor(fColor);
    if (regenTexCoords) {
        if (regenGlyphs) {
            fSubRun->setStrike(std::move(strike));
        }
        fSubRun->setAtlasGeneration(fBrokenRun
                                    ? GrDrawOpAtlas::kInvalidAtlasGeneration
                                    : fFullAtlasManager->atlasGeneration(fSubRun->maskFormat()));
    } else {
        // For the non-texCoords case we need to ensure that we update the associated use tokens
        fFullAtlasManager->setUseTokenBulk(*fSubRun->bulkUseToken(),
                                           fUploadTarget->tokenTracker()->nextDrawToken(),
                                           fSubRun->maskFormat());
    }
    return true;
}

bool GrTextBlob::VertexRegenerator::regenerate(GrTextBlob::VertexRegenerator::Result* result) {
    uint64_t currentAtlasGen = fFullAtlasManager->atlasGeneration(fSubRun->maskFormat());
    // If regenerate() is called multiple times then the atlas gen may have changed. So we check
    // this each time.
    if (fSubRun->atlasGeneration() != currentAtlasGen) {
        fRegenFlags |= kRegenTex;
    }

    if (fRegenFlags) {
        return this->doRegen(result,
                             fRegenFlags & kRegenPos,
                             fRegenFlags & kRegenCol,
                             fRegenFlags & kRegenTex,
                             fRegenFlags & kRegenGlyph);
    } else {
        bool hasW = fSubRun->hasWCoord();
        auto vertexStride = GetVertexStride(fSubRun->maskFormat(), hasW);
        result->fFinished = true;
        result->fGlyphsRegenerated = fSubRun->glyphCount() - fCurrGlyph;
        result->fFirstVertex = fBlob->fVertices + fSubRun->vertexStartIndex() +
                               fCurrGlyph * kVerticesPerGlyph * vertexStride;
        fCurrGlyph = fSubRun->glyphCount();

        // set use tokens for all of the glyphs in our subrun.  This is only valid if we
        // have a valid atlas generation
        fFullAtlasManager->setUseTokenBulk(*fSubRun->bulkUseToken(),
                                           fUploadTarget->tokenTracker()->nextDrawToken(),
                                           fSubRun->maskFormat());
        return true;
    }
    SK_ABORT("Should not get here");
}
