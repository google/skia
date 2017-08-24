/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextBlob.h"

#include "GrOpFlushState.h"
#include "GrTextUtils.h"

#include "SkDistanceFieldGen.h"
#include "SkGlyphCache.h"

#include "ops/GrAtlasTextOp.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// A large template to handle regenerating the vertices of a textblob with as few branches as
// possible
template <bool regenPos, bool regenCol, bool regenTexCoords>
inline void regen_vertices(intptr_t vertex, const GrGlyph* glyph, size_t vertexStride,
                           bool useDistanceFields, SkScalar transX, SkScalar transY,
                           int32_t log2Width, int32_t log2Height,
                           GrColor color) {
    int u0, v0, u1, v1;
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

        // normalize
        u0 *= 65535;
        u0 >>= log2Width;
        u1 *= 65535;
        u1 >>= log2Width;
        v0 *= 65535;
        v0 >>= log2Height;
        v1 *= 65535;
        v1 >>= log2Height;
        SkASSERT(u0 >= 0 && u0 <= 65535);
        SkASSERT(u1 >= 0 && u1 <= 65535);
        SkASSERT(v0 >= 0 && v0 <= 65535);
        SkASSERT(v1 >= 0 && v1 <= 65535);
    }

    // This is a bit wonky, but sometimes we have LCD text, in which case we won't have color
    // vertices, hence vertexStride - sizeof(SkIPoint16)
    intptr_t colorOffset = sizeof(SkPoint);
    intptr_t texCoordOffset = vertexStride - sizeof(SkIPoint16);

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
        textureCoords[0] = (uint16_t) u0;
        textureCoords[1] = (uint16_t) v0;
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
        textureCoords[0] = (uint16_t)u0;
        textureCoords[1] = (uint16_t)v1;
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
        textureCoords[0] = (uint16_t)u1;
        textureCoords[1] = (uint16_t)v1;
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
        textureCoords[0] = (uint16_t)u1;
        textureCoords[1] = (uint16_t)v0;
    }
}

template <bool regenPos, bool regenCol, bool regenTexCoords, bool regenGlyphs>
void GrAtlasTextBlob::regenInOp(GrDrawOp::Target* target, GrAtlasGlyphCache* fontCache,
                                GrBlobRegenHelper* helper, Run* run, Run::SubRunInfo* info,
                                SkAutoGlyphCache* lazyCache, int glyphCount, size_t vertexStride,
                                GrColor color, SkScalar transX, SkScalar transY) const {
    SkASSERT(lazyCache);
    static_assert(!regenGlyphs || regenTexCoords, "must regenTexCoords along regenGlyphs");
    GrAtlasTextStrike* strike = nullptr;
    if (regenTexCoords) {
        info->resetBulkUseToken();

        const SkDescriptor* desc = (run->fOverrideDescriptor && !info->drawAsDistanceFields())
                                      ? run->fOverrideDescriptor->getDesc()
                                      : run->fDescriptor.getDesc();

        if (!*lazyCache || (*lazyCache)->getDescriptor() != *desc) {
            SkScalerContextEffects effects;
            effects.fPathEffect = run->fPathEffect.get();
            effects.fRasterizer = run->fRasterizer.get();
            effects.fMaskFilter = run->fMaskFilter.get();
            lazyCache->reset(SkGlyphCache::DetachCache(run->fTypeface.get(), effects, desc));
        }

        if (regenGlyphs) {
            strike = fontCache->getStrike(lazyCache->get());
        } else {
            strike = info->strike();
        }
    }

    bool brokenRun = false;
    for (int glyphIdx = 0; glyphIdx < glyphCount; glyphIdx++) {
        GrGlyph* glyph = nullptr;
        int log2Width = 0, log2Height = 0;
        if (regenTexCoords) {
            size_t glyphOffset = glyphIdx + info->glyphStartIndex();

            if (regenGlyphs) {
                // Get the id from the old glyph, and use the new strike to lookup
                // the glyph.
                GrGlyph::PackedID id = fGlyphs[glyphOffset]->fPackedID;
                fGlyphs[glyphOffset] = strike->getGlyph(id, info->maskFormat(), lazyCache->get());
                SkASSERT(id == fGlyphs[glyphOffset]->fPackedID);
            }
            glyph = fGlyphs[glyphOffset];
            SkASSERT(glyph && glyph->fMaskFormat == info->maskFormat());

            if (!fontCache->hasGlyph(glyph) &&
                !strike->addGlyphToAtlas(target, glyph, lazyCache->get(), info->maskFormat())) {
                helper->flush();
                brokenRun = glyphIdx > 0;

                SkDEBUGCODE(bool success =) strike->addGlyphToAtlas(target,
                                                                    glyph,
                                                                    lazyCache->get(),
                                                                    info->maskFormat());
                SkASSERT(success);
            }
            fontCache->addGlyphToBulkAndSetUseToken(info->bulkUseToken(), glyph,
                                                    target->nextDrawToken());
            log2Width = fontCache->log2Width(info->maskFormat());
            log2Height = fontCache->log2Height(info->maskFormat());
        }

        intptr_t vertex = reinterpret_cast<intptr_t>(fVertices);
        vertex += info->vertexStartIndex();
        vertex += vertexStride * glyphIdx * GrAtlasTextOp::kVerticesPerGlyph;
        regen_vertices<regenPos, regenCol, regenTexCoords>(vertex, glyph, vertexStride,
                                                           info->drawAsDistanceFields(), transX,
                                                           transY, log2Width, log2Height, color);
        helper->incGlyphCount();
    }

    // We may have changed the color so update it here
    info->setColor(color);
    if (regenTexCoords) {
        if (regenGlyphs) {
            info->setStrike(strike);
        }
        info->setAtlasGeneration(brokenRun ? GrDrawOpAtlas::kInvalidAtlasGeneration
                                           : fontCache->atlasGeneration(info->maskFormat()));
    }
}

enum RegenMask {
    kNoRegen    = 0x0,
    kRegenPos   = 0x1,
    kRegenCol   = 0x2,
    kRegenTex   = 0x4,
    kRegenGlyph = 0x8 | kRegenTex, // we have to regenerate the texture coords when we regen glyphs

    // combinations
    kRegenPosCol = kRegenPos | kRegenCol,
    kRegenPosTex = kRegenPos | kRegenTex,
    kRegenPosTexGlyph = kRegenPos | kRegenGlyph,
    kRegenPosColTex = kRegenPos | kRegenCol | kRegenTex,
    kRegenPosColTexGlyph = kRegenPos | kRegenCol | kRegenGlyph,
    kRegenColTex = kRegenCol | kRegenTex,
    kRegenColTexGlyph = kRegenCol | kRegenGlyph,
};

#define REGEN_ARGS target, fontCache, helper, &run, &info, lazyCache, \
                   *glyphCount, vertexStride, color, transX, transY

void GrAtlasTextBlob::regenInOp(GrDrawOp::Target* target,
                                GrAtlasGlyphCache* fontCache,
                                GrBlobRegenHelper* helper,
                                int runIndex, int subRunIndex, SkAutoGlyphCache* lazyCache,
                                size_t vertexStride, const SkMatrix& viewMatrix,
                                SkScalar x, SkScalar y, GrColor color,
                                void** vertices, size_t* byteCount, int* glyphCount) {
    Run& run = fRuns[runIndex];
    Run::SubRunInfo& info = run.fSubRunInfo[subRunIndex];

    uint64_t currentAtlasGen = fontCache->atlasGeneration(info.maskFormat());

    // Compute translation if any
    SkScalar transX, transY;
    info.computeTranslation(viewMatrix, x, y, &transX, &transY);

    // Because the GrAtlasGlyphCache may evict the strike a blob depends on using for
    // generating its texture coords, we have to track whether or not the strike has
    // been abandoned.  If it hasn't been abandoned, then we can use the GrGlyph*s as is
    // otherwise we have to get the new strike, and use that to get the correct glyphs.
    // Because we do not have the packed ids, and thus can't look up our glyphs in the
    // new strike, we instead keep our ref to the old strike and use the packed ids from
    // it.  These ids will still be valid as long as we hold the ref.  When we are done
    // updating our cache of the GrGlyph*s, we drop our ref on the old strike
    bool regenerateGlyphs = info.strike()->isAbandoned();
    bool regenerateTextureCoords = info.atlasGeneration() != currentAtlasGen ||
                                   regenerateGlyphs;
    bool regenerateColors = kARGB_GrMaskFormat != info.maskFormat() &&
                            info.color() != color;
    bool regeneratePositions = transX != 0.f || transY != 0.f;
    *glyphCount = info.glyphCount();

    uint32_t regenMaskBits = kNoRegen;
    regenMaskBits |= regeneratePositions ? kRegenPos : 0;
    regenMaskBits |= regenerateColors ? kRegenCol : 0;
    regenMaskBits |= regenerateTextureCoords ? kRegenTex : 0;
    regenMaskBits |= regenerateGlyphs ? kRegenGlyph : 0;
    RegenMask regenMask = (RegenMask)regenMaskBits;

    switch (regenMask) {
        case kRegenPos:
            this->regenInOp<true, false, false, false>(REGEN_ARGS);
            break;
        case kRegenCol:
            this->regenInOp<false, true, false, false>(REGEN_ARGS);
            break;
        case kRegenTex:
            this->regenInOp<false, false, true, false>(REGEN_ARGS);
            break;
        case kRegenGlyph:
            this->regenInOp<false, false, true, true>(REGEN_ARGS);
            break;

        // combinations
        case kRegenPosCol:
            this->regenInOp<true, true, false, false>(REGEN_ARGS);
            break;
        case kRegenPosTex:
            this->regenInOp<true, false, true, false>(REGEN_ARGS);
            break;
        case kRegenPosTexGlyph:
            this->regenInOp<true, false, true, true>(REGEN_ARGS);
            break;
        case kRegenPosColTex:
            this->regenInOp<true, true, true, false>(REGEN_ARGS);
            break;
        case kRegenPosColTexGlyph:
            this->regenInOp<true, true, true, true>(REGEN_ARGS);
            break;
        case kRegenColTex:
            this->regenInOp<false, true, true, false>(REGEN_ARGS);
            break;
        case kRegenColTexGlyph:
            this->regenInOp<false, true, true, true>(REGEN_ARGS);
            break;
        case kNoRegen:
            helper->incGlyphCount(*glyphCount);

            // set use tokens for all of the glyphs in our subrun.  This is only valid if we
            // have a valid atlas generation
            fontCache->setUseTokenBulk(*info.bulkUseToken(), target->nextDrawToken(),
                                        info.maskFormat());
            break;
    }

    *byteCount = info.byteCount();
    *vertices = fVertices + info.vertexStartIndex();
}
