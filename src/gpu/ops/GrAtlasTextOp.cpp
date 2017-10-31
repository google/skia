/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextOp.h"

#include "GrContext.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"

#include "SkGlyphCache.h"
#include "SkMathPriv.h"

#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrDistanceFieldGeoProc.h"
#include "text/GrAtlasGlyphCache.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

static const int kDistanceAdjustLumShift = 5;

SkString GrAtlasTextOp::dumpInfo() const {
    SkString str;

    for (int i = 0; i < fGeoCount; ++i) {
        str.appendf("%d: Color: 0x%08x Trans: %.2f,%.2f Runs: %d\n",
                    i,
                    fGeoData[i].fColor,
                    fGeoData[i].fX,
                    fGeoData[i].fY,
                    fGeoData[i].fBlob->runCount());
    }

    str += fProcessors.dumpProcessors();
    str += INHERITED::dumpInfo();
    return str;
}

GrDrawOp::FixedFunctionFlags GrAtlasTextOp::fixedFunctionFlags() const {
    return FixedFunctionFlags::kNone;
}

GrDrawOp::RequiresDstTexture GrAtlasTextOp::finalize(const GrCaps& caps,
                                                     const GrAppliedClip* clip,
                                                     GrPixelConfigIsClamped dstIsClamped) {
    GrProcessorAnalysisCoverage coverage;
    GrProcessorAnalysisColor color;
    if (kColorBitmapMask_MaskType == fMaskType) {
        color.setToUnknown();
    } else {
        color.setToConstant(fColor);
    }
    switch (fMaskType) {
        case kGrayscaleCoverageMask_MaskType:
        case kAliasedDistanceField_MaskType:
        case kGrayscaleDistanceField_MaskType:
            coverage = GrProcessorAnalysisCoverage::kSingleChannel;
            break;
        case kLCDCoverageMask_MaskType:
        case kLCDDistanceField_MaskType:
        case kLCDBGRDistanceField_MaskType:
            coverage = GrProcessorAnalysisCoverage::kLCD;
            break;
        case kColorBitmapMask_MaskType:
            coverage = GrProcessorAnalysisCoverage::kNone;
            break;
    }
    auto analysis = fProcessors.finalize(color, coverage, clip, false, caps, dstIsClamped, &fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    fCanCombineOnTouchOrOverlap =
            !analysis.requiresDstTexture() &&
            !(fProcessors.xferProcessor() && fProcessors.xferProcessor()->xferBarrierType(caps));
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

static void clip_quads(const SkIRect& clipRect,
                       unsigned char* currVertex, unsigned char* blobVertices,
                       size_t vertexStride, int glyphCount) {
    for (int i = 0; i < glyphCount; ++i) {
        SkPoint* blobPositionLT = reinterpret_cast<SkPoint*>(blobVertices);
        SkPoint* blobPositionRB = reinterpret_cast<SkPoint*>(blobVertices + 3*vertexStride);

        // positions for bitmap glyphs are pixel boundary aligned
        SkIRect positionRect = SkIRect::MakeLTRB(SkScalarFloorToInt(blobPositionLT->fX),
                                                 SkScalarFloorToInt(blobPositionLT->fY),
                                                 SkScalarFloorToInt(blobPositionRB->fX),
                                                 SkScalarFloorToInt(blobPositionRB->fY));
        if (clipRect.contains(positionRect)) {
            memcpy(currVertex, blobVertices, 4 * vertexStride);
            currVertex += 4 * vertexStride;
        } else {
            // Pull out some more data that we'll need.
            // In the LCD case the color will be garbage, but we'll overwrite it with the texcoords
            // and it avoids a lot of conditionals.
            SkColor color = *reinterpret_cast<SkColor*>(blobVertices + sizeof(SkPoint));
            size_t coordOffset = vertexStride - 2*sizeof(uint16_t);
            uint16_t* blobCoordsLT = reinterpret_cast<uint16_t*>(blobVertices + coordOffset);
            uint16_t* blobCoordsRB = reinterpret_cast<uint16_t*>(blobVertices + 3*vertexStride +
                                                                 coordOffset);
            // Pull out the texel coordinates and texture index bits
            uint16_t coordsRectL = blobCoordsLT[0] >> 1;
            uint16_t coordsRectT = blobCoordsLT[1] >> 1;
            uint16_t coordsRectR = blobCoordsRB[0] >> 1;
            uint16_t coordsRectB = blobCoordsRB[1] >> 1;
            uint16_t pageIndexX = blobCoordsLT[0] & 0x1;
            uint16_t pageIndexY = blobCoordsLT[1] & 0x1;

            int positionRectWidth = positionRect.width();
            int positionRectHeight = positionRect.height();
            SkASSERT(positionRectWidth == (coordsRectR - coordsRectL));
            SkASSERT(positionRectHeight == (coordsRectB - coordsRectT));

            // Clip position and texCoords to the clipRect
            unsigned int delta;
            delta = SkTMin(SkTMax(clipRect.fLeft - positionRect.fLeft, 0), positionRectWidth);
            coordsRectL += delta;
            positionRect.fLeft += delta;

            delta = SkTMin(SkTMax(clipRect.fTop - positionRect.fTop, 0), positionRectHeight);
            coordsRectT += delta;
            positionRect.fTop += delta;

            delta = SkTMin(SkTMax(positionRect.fRight - clipRect.fRight, 0), positionRectWidth);
            coordsRectR -= delta;
            positionRect.fRight -= delta;

            delta = SkTMin(SkTMax(positionRect.fBottom - clipRect.fBottom, 0), positionRectHeight);
            coordsRectB -= delta;
            positionRect.fBottom -= delta;

            // Repack texel coordinates and index
            coordsRectL = coordsRectL << 1 | pageIndexX;
            coordsRectT = coordsRectT << 1 | pageIndexY;
            coordsRectR = coordsRectR << 1 | pageIndexX;
            coordsRectB = coordsRectB << 1 | pageIndexY;

            // Set new positions and coords
            SkPoint* currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fLeft;
            currPosition->fY = positionRect.fTop;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            uint16_t* currCoords = reinterpret_cast<uint16_t*>(currVertex + coordOffset);
            currCoords[0] = coordsRectL;
            currCoords[1] = coordsRectT;
            currVertex += vertexStride;

            currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fLeft;
            currPosition->fY = positionRect.fBottom;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            currCoords = reinterpret_cast<uint16_t*>(currVertex + coordOffset);
            currCoords[0] = coordsRectL;
            currCoords[1] = coordsRectB;
            currVertex += vertexStride;

            currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fRight;
            currPosition->fY = positionRect.fTop;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            currCoords = reinterpret_cast<uint16_t*>(currVertex + coordOffset);
            currCoords[0] = coordsRectR;
            currCoords[1] = coordsRectT;
            currVertex += vertexStride;

            currPosition = reinterpret_cast<SkPoint*>(currVertex);
            currPosition->fX = positionRect.fRight;
            currPosition->fY = positionRect.fBottom;
            *(reinterpret_cast<SkColor*>(currVertex + sizeof(SkPoint))) = color;
            currCoords = reinterpret_cast<uint16_t*>(currVertex + coordOffset);
            currCoords[0] = coordsRectR;
            currCoords[1] = coordsRectB;
            currVertex += vertexStride;
        }

        blobVertices += 4 * vertexStride;
    }
}

void GrAtlasTextOp::onPrepareDraws(Target* target) {
    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix.
    // TODO actually only invert if we don't have RGBA
    SkMatrix localMatrix;
    if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    GrMaskFormat maskFormat = this->maskFormat();

    uint32_t atlasPageCount = fFontCache->getAtlasPageCount(maskFormat);
    const sk_sp<GrTextureProxy>* proxies = fFontCache->getProxies(maskFormat);
    if (!atlasPageCount || !proxies[0]) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }

    FlushInfo flushInfo;
    flushInfo.fPipeline =
            target->makePipeline(fSRGBFlags, std::move(fProcessors), target->detachAppliedClip());
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor = this->setupDfProcessor();
    } else {
        flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(
            this->color(), proxies, GrSamplerState::ClampNearest(), maskFormat,
            localMatrix, this->usesLocalCoords());
    }

    flushInfo.fGlyphsToFlush = 0;
    size_t vertexStride = flushInfo.fGeometryProcessor->getVertexStride();
    SkASSERT(vertexStride == GrAtlasTextBlob::GetVertexStride(maskFormat));

    int glyphCount = this->numGlyphs();
    const GrBuffer* vertexBuffer;

    void* vertices = target->makeVertexSpace(
            vertexStride, glyphCount * kVerticesPerGlyph, &vertexBuffer, &flushInfo.fVertexOffset);
    flushInfo.fVertexBuffer.reset(SkRef(vertexBuffer));
    flushInfo.fIndexBuffer = target->resourceProvider()->refQuadIndexBuffer();
    if (!vertices || !flushInfo.fVertexBuffer) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    unsigned char* currVertex = reinterpret_cast<unsigned char*>(vertices);

    GrBlobRegenHelper helper(this, target, &flushInfo);
    SkAutoGlyphCache glyphCache;
    // each of these is a SubRun
    for (int i = 0; i < fGeoCount; i++) {
        const Geometry& args = fGeoData[i];
        Blob* blob = args.fBlob;
        size_t byteCount;
        void* blobVertices;
        int subRunGlyphCount;
        blob->regenInOp(target->deferredUploadTarget(), fFontCache, &helper, args.fRun,
                        args.fSubRun, &glyphCache, vertexStride, args.fViewMatrix, args.fX, args.fY,
                        args.fColor, &blobVertices, &byteCount, &subRunGlyphCount);

        // now copy all vertices
        if (args.fClipRect.isEmpty()) {
            memcpy(currVertex, blobVertices, byteCount);
        } else {
            clip_quads(args.fClipRect, currVertex, reinterpret_cast<unsigned char*>(blobVertices),
                       vertexStride, subRunGlyphCount);
        }

        currVertex += byteCount;
    }

    this->flush(target, &flushInfo);
}

void GrAtlasTextOp::flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
    GrGeometryProcessor* gp = flushInfo->fGeometryProcessor.get();
    GrMaskFormat maskFormat = this->maskFormat();
    if (gp->numTextureSamplers() != (int)fFontCache->getAtlasPageCount(maskFormat)) {
        // During preparation the number of atlas pages has increased.
        // Update the proxies used in the GP to match.
        if (this->usesDistanceFields()) {
            if (this->isLCD()) {
                reinterpret_cast<GrDistanceFieldLCDTextGeoProc*>(gp)->addNewProxies(
                    fFontCache->getProxies(maskFormat), GrSamplerState::ClampBilerp());
            } else {
                reinterpret_cast<GrDistanceFieldA8TextGeoProc*>(gp)->addNewProxies(
                    fFontCache->getProxies(maskFormat), GrSamplerState::ClampBilerp());
            }
        } else {
            reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewProxies(
                fFontCache->getProxies(maskFormat), GrSamplerState::ClampNearest());
        }
    }

    GrMesh mesh(GrPrimitiveType::kTriangles);
    int maxGlyphsPerDraw =
            static_cast<int>(flushInfo->fIndexBuffer->gpuMemorySize() / sizeof(uint16_t) / 6);
    mesh.setIndexedPatterned(flushInfo->fIndexBuffer.get(), kIndicesPerGlyph, kVerticesPerGlyph,
                             flushInfo->fGlyphsToFlush, maxGlyphsPerDraw);
    mesh.setVertexData(flushInfo->fVertexBuffer.get(), flushInfo->fVertexOffset);
    target->draw(flushInfo->fGeometryProcessor.get(), flushInfo->fPipeline, mesh);
    flushInfo->fVertexOffset += kVerticesPerGlyph * flushInfo->fGlyphsToFlush;
    flushInfo->fGlyphsToFlush = 0;
}

bool GrAtlasTextOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    GrAtlasTextOp* that = t->cast<GrAtlasTextOp>();
    if (fProcessors != that->fProcessors) {
        return false;
    }

    if (!fCanCombineOnTouchOrOverlap && GrRectsTouchOrOverlap(this->bounds(), that->bounds())) {
        return false;
    }

    if (fMaskType != that->fMaskType) {
        return false;
    }

    if (!this->usesDistanceFields()) {
        if (kColorBitmapMask_MaskType == fMaskType && this->color() != that->color()) {
            return false;
        }
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }
    } else {
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (fLuminanceColor != that->fLuminanceColor) {
            return false;
        }
    }

    // Keep the batch vertex buffer size below 32K so we don't have to create a special one
    // We use the largest possible vertex size for this
    static const int kVertexSize = sizeof(SkPoint) + sizeof(SkColor) + 2 * sizeof(uint16_t);
    static const int kMaxGlyphs = 32768 / (kVerticesPerGlyph * kVertexSize);
    if (this->fNumGlyphs + that->fNumGlyphs > kMaxGlyphs) {
        return false;
    }

    fNumGlyphs += that->numGlyphs();

    // Reallocate space for geo data if necessary and then import that geo's data.
    int newGeoCount = that->fGeoCount + fGeoCount;

    // We reallocate at a rate of 1.5x to try to get better total memory usage
    if (newGeoCount > fGeoDataAllocSize) {
        int newAllocSize = fGeoDataAllocSize + fGeoDataAllocSize / 2;
        while (newAllocSize < newGeoCount) {
            newAllocSize += newAllocSize / 2;
        }
        fGeoData.realloc(newAllocSize);
        fGeoDataAllocSize = newAllocSize;
    }

    // We steal the ref on the blobs from the other AtlasTextOp and set its count to 0 so that
    // it doesn't try to unref them.
    memcpy(&fGeoData[fGeoCount], that->fGeoData.get(), that->fGeoCount * sizeof(Geometry));
#ifdef SK_DEBUG
    for (int i = 0; i < that->fGeoCount; ++i) {
        that->fGeoData.get()[i].fBlob = (Blob*)0x1;
    }
#endif
    that->fGeoCount = 0;
    fGeoCount = newGeoCount;

    this->joinBounds(*that);
    return true;
}

// TODO trying to figure out why lcd is so whack
// (see comments in GrAtlasTextContext::ComputeCanonicalColor)
sk_sp<GrGeometryProcessor> GrAtlasTextOp::setupDfProcessor() const {
    const SkMatrix& viewMatrix = this->viewMatrix();
    const sk_sp<GrTextureProxy>* p = fFontCache->getProxies(this->maskFormat());
    bool isLCD = this->isLCD();
    // set up any flags
    uint32_t flags = viewMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    flags |= viewMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
    flags |= fUseGammaCorrectDistanceTable ? kGammaCorrect_DistanceFieldEffectFlag : 0;
    flags |= (kAliasedDistanceField_MaskType == fMaskType) ? kAliased_DistanceFieldEffectFlag : 0;

    // see if we need to create a new effect
    if (isLCD) {
        flags |= kUseLCD_DistanceFieldEffectFlag;
        flags |= (kLCDBGRDistanceField_MaskType == fMaskType) ? kBGR_DistanceFieldEffectFlag : 0;

        float redCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetR(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float greenCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetG(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float blueCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetB(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust widthAdjust =
                GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(
                        redCorrection, greenCorrection, blueCorrection);

        return GrDistanceFieldLCDTextGeoProc::Make(this->color(), viewMatrix, p,
                                                   GrSamplerState::ClampBilerp(), widthAdjust,
                                                   flags, this->usesLocalCoords());
    } else {
#ifdef SK_GAMMA_APPLY_TO_A8
        float correction = 0;
        if (kAliasedDistanceField_MaskType != fMaskType) {
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT,
                                                                fLuminanceColor);
            correction = fDistanceAdjustTable->getAdjustment(lum >> kDistanceAdjustLumShift,
                                                             fUseGammaCorrectDistanceTable);
        }
        return GrDistanceFieldA8TextGeoProc::Make(this->color(), viewMatrix, p,
                                                  GrSamplerState::ClampBilerp(), correction, flags,
                                                  this->usesLocalCoords());
#else
        return GrDistanceFieldA8TextGeoProc::Make(this->color(), viewMatrix, p,
                                                  GrSamplerState::ClampBilerp(), flags,
                                                  this->usesLocalCoords());
#endif
    }
}

void GrBlobRegenHelper::flush() { fOp->flush(fTarget, fFlushInfo); }
