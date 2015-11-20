/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextBatch.h"

#include "GrBatchFontCache.h"
#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrResourceProvider.h"

#include "SkDistanceFieldGen.h"
#include "SkGlyphCache.h"

#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrDistanceFieldGeoProc.h"

static inline GrColor skcolor_to_grcolor_nopremultiply(SkColor c) {
    unsigned r = SkColorGetR(c);
    unsigned g = SkColorGetG(c);
    unsigned b = SkColorGetB(c);
    return GrColorPackRGBA(r, g, b, 0xff);
}

static const int kDistanceAdjustLumShift = 5;

SkString GrAtlasTextBatch::dumpInfo() const {
    SkString str;

    for (int i = 0; i < fGeoCount; ++i) {
        str.appendf("%d: Color: 0x%08x Trans: %.2f,%.2f Runs: %d\n",
                    i,
                    fGeoData[i].fColor,
                    fGeoData[i].fTransX,
                    fGeoData[i].fTransY,
                    fGeoData[i].fBlob->fRunCount);
    }

    str.append(INHERITED::dumpInfo());
    return str;
}

void GrAtlasTextBatch::getInvariantOutputColor(GrInitInvariantOutput* out) const {
    if (kColorBitmapMask_MaskType == fMaskType) {
        out->setUnknownFourComponents();
    } else {
        out->setKnownFourComponents(fBatch.fColor);
    }
}

void GrAtlasTextBatch::getInvariantOutputCoverage(GrInitInvariantOutput* out) const {
    switch (fMaskType) {
        case kGrayscaleDistanceField_MaskType:
        case kGrayscaleCoverageMask_MaskType:
            out->setUnknownSingleComponent();
            break;
        case kLCDCoverageMask_MaskType:
        case kLCDDistanceField_MaskType:
            out->setUnknownOpaqueFourComponents();
            out->setUsingLCDCoverage();
            break;
        case kColorBitmapMask_MaskType:
            out->setKnownSingleComponent(0xff);
    }
}

void GrAtlasTextBatch::initBatchTracker(const GrPipelineOptimizations& opt) {
    // Handle any color overrides
    if (!opt.readsColor()) {
        fGeoData[0].fColor = GrColor_ILLEGAL;
    }
    opt.getOverrideColorIfSet(&fGeoData[0].fColor);

    // setup batch properties
    fBatch.fColorIgnored = !opt.readsColor();
    fBatch.fColor = fGeoData[0].fColor;
    fBatch.fUsesLocalCoords = opt.readsLocalCoords();
    fBatch.fCoverageIgnored = !opt.readsCoverage();
}

void GrAtlasTextBatch::onPrepareDraws(Target* target) {
    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix.
    // TODO actually only invert if we don't have RGBA
    SkMatrix localMatrix;
    if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    GrTexture* texture = fFontCache->getTexture(this->maskFormat());
    if (!texture) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }

    bool usesDistanceFields = this->usesDistanceFields();
    GrMaskFormat maskFormat = this->maskFormat();
    bool isLCD = this->isLCD();

    SkAutoTUnref<const GrGeometryProcessor> gp;
    if (usesDistanceFields) {
        gp.reset(this->setupDfProcessor(this->viewMatrix(), fFilteredColor, this->color(),
                                        texture));
    } else {
        GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
        gp.reset(GrBitmapTextGeoProc::Create(this->color(),
                                             texture,
                                             params,
                                             maskFormat,
                                             localMatrix,
                                             this->usesLocalCoords()));
    }

    FlushInfo flushInfo;
    flushInfo.fGlyphsToFlush = 0;
    size_t vertexStride = gp->getVertexStride();
    SkASSERT(vertexStride == (usesDistanceFields ?
                              GetVertexStrideDf(maskFormat, isLCD) :
                              GetVertexStride(maskFormat)));

    target->initDraw(gp, this->pipeline());

    int glyphCount = this->numGlyphs();
    const GrVertexBuffer* vertexBuffer;

    void* vertices = target->makeVertexSpace(vertexStride,
                                             glyphCount * kVerticesPerGlyph,
                                             &vertexBuffer,
                                             &flushInfo.fVertexOffset);
    flushInfo.fVertexBuffer.reset(SkRef(vertexBuffer));
    flushInfo.fIndexBuffer.reset(target->resourceProvider()->refQuadIndexBuffer());
    if (!vertices || !flushInfo.fVertexBuffer) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    unsigned char* currVertex = reinterpret_cast<unsigned char*>(vertices);

    // We cache some values to avoid going to the glyphcache for the same fontScaler twice
    // in a row
    const SkDescriptor* desc = nullptr;
    SkGlyphCache* cache = nullptr;
    GrFontScaler* scaler = nullptr;
    SkTypeface* typeface = nullptr;

    for (int i = 0; i < fGeoCount; i++) {
        Geometry& args = fGeoData[i];
        Blob* blob = args.fBlob;
        Run& run = blob->fRuns[args.fRun];
        TextInfo& info = run.fSubRunInfo[args.fSubRun];

        uint64_t currentAtlasGen = fFontCache->atlasGeneration(maskFormat);
        bool regenerateTextureCoords = info.fAtlasGeneration != currentAtlasGen ||
                                       info.fStrike->isAbandoned();
        bool regenerateColors;
        if (usesDistanceFields) {
            regenerateColors = !isLCD && run.fColor != args.fColor;
        } else {
            regenerateColors = kA8_GrMaskFormat == maskFormat && run.fColor != args.fColor;
        }
        bool regeneratePositions = args.fTransX != 0.f || args.fTransY != 0.f;
        int glyphCount = info.fGlyphEndIndex - info.fGlyphStartIndex;

        // We regenerate both texture coords and colors in the blob itself, and update the
        // atlas generation.  If we don't end up purging any unused plots, we can avoid
        // regenerating the coords.  We could take a finer grained approach to updating texture
        // coords but its not clear if the extra bookkeeping would offset any gains.
        // To avoid looping over the glyphs twice, we do one loop and conditionally update color
        // or coords as needed.  One final note, if we have to break a run for an atlas eviction
        // then we can't really trust the atlas has all of the correct data.  Atlas evictions
        // should be pretty rare, so we just always regenerate in those cases
        if (regenerateTextureCoords || regenerateColors || regeneratePositions) {
            // first regenerate texture coordinates / colors if need be
            bool brokenRun = false;

            // Because the GrBatchFontCache may evict the strike a blob depends on using for
            // generating its texture coords, we have to track whether or not the strike has
            // been abandoned.  If it hasn't been abandoned, then we can use the GrGlyph*s as is
            // otherwise we have to get the new strike, and use that to get the correct glyphs.
            // Because we do not have the packed ids, and thus can't look up our glyphs in the
            // new strike, we instead keep our ref to the old strike and use the packed ids from
            // it.  These ids will still be valid as long as we hold the ref.  When we are done
            // updating our cache of the GrGlyph*s, we drop our ref on the old strike
            bool regenerateGlyphs = false;
            GrBatchTextStrike* strike = nullptr;
            if (regenerateTextureCoords) {
                info.fBulkUseToken.reset();

                // We can reuse if we have a valid strike and our descriptors / typeface are the
                // same.  The override descriptor is only for the non distance field text within
                // a run
                const SkDescriptor* newDesc = (run.fOverrideDescriptor && !usesDistanceFields) ?
                                              run.fOverrideDescriptor->getDesc() :
                                              run.fDescriptor.getDesc();
                if (!cache || !SkTypeface::Equal(typeface, run.fTypeface) ||
                              !(desc->equals(*newDesc))) {
                    if (cache) {
                        SkGlyphCache::AttachCache(cache);
                    }
                    desc = newDesc;
                    cache = SkGlyphCache::DetachCache(run.fTypeface, desc);
                    scaler = GrTextContext::GetGrFontScaler(cache);
                    strike = info.fStrike;
                    typeface = run.fTypeface;
                }

                if (info.fStrike->isAbandoned()) {
                    regenerateGlyphs = true;
                    strike = fFontCache->getStrike(scaler);
                } else {
                    strike = info.fStrike;
                }
            }

            for (int glyphIdx = 0; glyphIdx < glyphCount; glyphIdx++) {
                if (regenerateTextureCoords) {
                    size_t glyphOffset = glyphIdx + info.fGlyphStartIndex;

                    GrGlyph* glyph = blob->fGlyphs[glyphOffset];
                    GrGlyph::PackedID id = glyph->fPackedID;
                    const SkGlyph& skGlyph = scaler->grToSkGlyph(id);
                    if (regenerateGlyphs) {
                        // Get the id from the old glyph, and use the new strike to lookup
                        // the glyph.
                        blob->fGlyphs[glyphOffset] = strike->getGlyph(skGlyph, id, maskFormat,
                                                                      scaler);
                    }
                    glyph = blob->fGlyphs[glyphOffset];
                    SkASSERT(glyph);
                    SkASSERT(id == glyph->fPackedID);
                    // We want to be able to assert this but cannot for testing purposes.
                    // once skbug:4143 has landed we can revist this assert
                    //SkASSERT(glyph->fMaskFormat == this->maskFormat());

                    if (!fFontCache->hasGlyph(glyph) &&
                        !strike->addGlyphToAtlas(target, glyph, scaler, skGlyph, maskFormat)) {
                        this->flush(target, &flushInfo);
                        target->initDraw(gp, this->pipeline());
                        brokenRun = glyphIdx > 0;

                        SkDEBUGCODE(bool success =) strike->addGlyphToAtlas(target,
                                                                            glyph,
                                                                            scaler,
                                                                            skGlyph,
                                                                            maskFormat);
                        SkASSERT(success);
                    }
                    fFontCache->addGlyphToBulkAndSetUseToken(&info.fBulkUseToken, glyph,
                                                             target->currentToken());

                    // Texture coords are the last vertex attribute so we get a pointer to the
                    // first one and then map with stride in regenerateTextureCoords
                    intptr_t vertex = reinterpret_cast<intptr_t>(blob->fVertices);
                    vertex += info.fVertexStartIndex;
                    vertex += vertexStride * glyphIdx * kVerticesPerGlyph;
                    vertex += vertexStride - sizeof(SkIPoint16);

                    this->regenerateTextureCoords(glyph, vertex, vertexStride);
                }

                if (regenerateColors) {
                    intptr_t vertex = reinterpret_cast<intptr_t>(blob->fVertices);
                    vertex += info.fVertexStartIndex;
                    vertex += vertexStride * glyphIdx * kVerticesPerGlyph + sizeof(SkPoint);
                    this->regenerateColors(vertex, vertexStride, args.fColor);
                }

                if (regeneratePositions) {
                    intptr_t vertex = reinterpret_cast<intptr_t>(blob->fVertices);
                    vertex += info.fVertexStartIndex;
                    vertex += vertexStride * glyphIdx * kVerticesPerGlyph;
                    SkScalar transX = args.fTransX;
                    SkScalar transY = args.fTransY;
                    this->regeneratePositions(vertex, vertexStride, transX, transY);
                }
                flushInfo.fGlyphsToFlush++;
            }

            // We my have changed the color so update it here
            run.fColor = args.fColor;
            if (regenerateTextureCoords) {
                if (regenerateGlyphs) {
                    info.fStrike.reset(SkRef(strike));
                }
                info.fAtlasGeneration = brokenRun ? GrBatchAtlas::kInvalidAtlasGeneration :
                                                    fFontCache->atlasGeneration(maskFormat);
            }
        } else {
            flushInfo.fGlyphsToFlush += glyphCount;

            // set use tokens for all of the glyphs in our subrun.  This is only valid if we
            // have a valid atlas generation
            fFontCache->setUseTokenBulk(info.fBulkUseToken, target->currentToken(), maskFormat);
        }

        // now copy all vertices
        size_t byteCount = info.fVertexEndIndex - info.fVertexStartIndex;
        memcpy(currVertex, blob->fVertices + info.fVertexStartIndex, byteCount);

        currVertex += byteCount;
    }
    // Make sure to attach the last cache if applicable
    if (cache) {
        SkGlyphCache::AttachCache(cache);
    }
    this->flush(target, &flushInfo);
}

void GrAtlasTextBatch::regenerateTextureCoords(GrGlyph* glyph, intptr_t vertex,
                                               size_t vertexStride) {
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    int u0, v0, u1, v1;
    if (this->usesDistanceFields()) {
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

    SkIPoint16* textureCoords;
    // V0
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
    textureCoords->set(u0, v0);
    vertex += vertexStride;

    // V1
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
    textureCoords->set(u0, v1);
    vertex += vertexStride;

    // V2
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
    textureCoords->set(u1, v1);
    vertex += vertexStride;

    // V3
    textureCoords = reinterpret_cast<SkIPoint16*>(vertex);
    textureCoords->set(u1, v0);
}

void GrAtlasTextBatch::regenerateColors(intptr_t vertex, size_t vertexStride, GrColor color) {
    for (int i = 0; i < kVerticesPerGlyph; i++) {
        SkColor* vcolor = reinterpret_cast<SkColor*>(vertex);
        *vcolor = color;
        vertex += vertexStride;
    }
}

void GrAtlasTextBatch::regeneratePositions(intptr_t vertex, size_t vertexStride, SkScalar transX,
                                           SkScalar transY) {
    for (int i = 0; i < kVerticesPerGlyph; i++) {
        SkPoint* point = reinterpret_cast<SkPoint*>(vertex);
        point->fX += transX;
        point->fY += transY;
        vertex += vertexStride;
    }
}

void GrAtlasTextBatch::flush(GrVertexBatch::Target* target, FlushInfo* flushInfo) {
    GrVertices vertices;
    int maxGlyphsPerDraw = flushInfo->fIndexBuffer->maxQuads();
    vertices.initInstanced(kTriangles_GrPrimitiveType, flushInfo->fVertexBuffer,
                           flushInfo->fIndexBuffer, flushInfo->fVertexOffset,
                           kVerticesPerGlyph, kIndicesPerGlyph, flushInfo->fGlyphsToFlush,
                           maxGlyphsPerDraw);
    target->draw(vertices);
    flushInfo->fVertexOffset += kVerticesPerGlyph * flushInfo->fGlyphsToFlush;
    flushInfo->fGlyphsToFlush = 0;
}

bool GrAtlasTextBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    GrAtlasTextBatch* that = t->cast<GrAtlasTextBatch>();
    if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                that->bounds(), caps)) {
        return false;
    }

    if (fMaskType != that->fMaskType) {
        return false;
    }

    if (!this->usesDistanceFields()) {
        // TODO we can often batch across LCD text if we have dual source blending and don't
        // have to use the blend constant
        if (kGrayscaleCoverageMask_MaskType != fMaskType && this->color() != that->color()) {
            return false;
        }
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }
    } else {
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (fFilteredColor != that->fFilteredColor) {
            return false;
        }

        if (fUseBGR != that->fUseBGR) {
            return false;
        }

        // TODO see note above
        if (kLCDDistanceField_MaskType == fMaskType && this->color() != that->color()) {
            return false;
        }
    }

    fBatch.fNumGlyphs += that->numGlyphs();

    // Reallocate space for geo data if necessary and then import that's geo data.
    int newGeoCount = that->fGeoCount + fGeoCount;
    // We assume (and here enforce) that the allocation size is the smallest power of two that
    // is greater than or equal to the number of geometries (and at least
    // kMinGeometryAllocated).
    int newAllocSize = GrNextPow2(newGeoCount);
    int currAllocSize = SkTMax<int>(kMinGeometryAllocated, GrNextPow2(fGeoCount));

    if (newGeoCount > currAllocSize) {
        fGeoData.realloc(newAllocSize);
    }

    memcpy(&fGeoData[fGeoCount], that->fGeoData.get(), that->fGeoCount * sizeof(Geometry));
    // We steal the ref on the blobs from the other TextBatch and set its count to 0 so that
    // it doesn't try to unref them.
#ifdef SK_DEBUG
    for (int i = 0; i < that->fGeoCount; ++i) {
        that->fGeoData.get()[i].fBlob = (Blob*)0x1;
    }
#endif
    that->fGeoCount = 0;
    fGeoCount = newGeoCount;

    this->joinBounds(that->bounds());
    return true;
}

// TODO just use class params
// TODO trying to figure out why lcd is so whack
GrGeometryProcessor* GrAtlasTextBatch::setupDfProcessor(const SkMatrix& viewMatrix,
                                                        SkColor filteredColor,
                                                        GrColor color, GrTexture* texture) {
    GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
    bool isLCD = this->isLCD();
    // set up any flags
    uint32_t flags = viewMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;

    // see if we need to create a new effect
    if (isLCD) {
        flags |= kUseLCD_DistanceFieldEffectFlag;
        flags |= viewMatrix.rectStaysRect() ? kRectToRect_DistanceFieldEffectFlag : 0;
        flags |= fUseBGR ? kBGR_DistanceFieldEffectFlag : 0;

        GrColor colorNoPreMul = skcolor_to_grcolor_nopremultiply(filteredColor);

        float redCorrection =
            (*fDistanceAdjustTable)[GrColorUnpackR(colorNoPreMul) >> kDistanceAdjustLumShift];
        float greenCorrection =
            (*fDistanceAdjustTable)[GrColorUnpackG(colorNoPreMul) >> kDistanceAdjustLumShift];
        float blueCorrection =
            (*fDistanceAdjustTable)[GrColorUnpackB(colorNoPreMul) >> kDistanceAdjustLumShift];
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust widthAdjust =
            GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(redCorrection,
                                                                greenCorrection,
                                                                blueCorrection);

        return GrDistanceFieldLCDTextGeoProc::Create(color,
                                                     viewMatrix,
                                                     texture,
                                                     params,
                                                     widthAdjust,
                                                     flags,
                                                     this->usesLocalCoords());
    } else {
        flags |= kColorAttr_DistanceFieldEffectFlag;
#ifdef SK_GAMMA_APPLY_TO_A8
        U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT, filteredColor);
        float correction = (*fDistanceAdjustTable)[lum >> kDistanceAdjustLumShift];
        return GrDistanceFieldA8TextGeoProc::Create(color,
                                                    viewMatrix,
                                                    texture,
                                                    params,
                                                    correction,
                                                    flags,
                                                    this->usesLocalCoords());
#else
        return GrDistanceFieldA8TextGeoProc::Create(color,
                                                    viewMatrix,
                                                    texture,
                                                    params,
                                                    flags,
                                                    this->usesLocalCoords());
#endif
    }

}
