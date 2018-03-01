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
#include "SkMatrixPriv.h"
#include "SkPoint3.h"
#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrDistanceFieldGeoProc.h"
#include "text/GrAtlasManager.h"
#include "text/GrGlyphCache.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

static const int kDistanceAdjustLumShift = 5;

void GrAtlasTextOp::init() {
    const Geometry& geo = fGeoData[0];
    SkRect bounds;
    geo.fBlob->computeSubRunBounds(&bounds, geo.fRun, geo.fSubRun, geo.fViewMatrix, geo.fX, geo.fY);
    // We don't have tight bounds on the glyph paths in device space. For the purposes of bounds
    // we treat this as a set of non-AA rects rendered with a texture.
    this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
    if (this->usesDistanceFields()) {
        bool isLCD = this->isLCD();

        const SkMatrix& viewMatrix = geo.fViewMatrix;

        fDFGPFlags = viewMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= viewMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= viewMatrix.hasPerspective() ? kPerspective_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= fUseGammaCorrectDistanceTable ? kGammaCorrect_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= (kAliasedDistanceField_MaskType == fMaskType)
                              ? kAliased_DistanceFieldEffectFlag
                              : 0;

        if (isLCD) {
            fDFGPFlags |= kUseLCD_DistanceFieldEffectFlag;
            fDFGPFlags |=
                    (kLCDBGRDistanceField_MaskType == fMaskType) ? kBGR_DistanceFieldEffectFlag : 0;
        }
    }
}

void GrAtlasTextOp::visitProxies(const VisitProxyFunc& func) const {
    fProcessors.visitProxies(func);

    // We need to visit the atlasManager's proxies because, although the atlasManager explicitly
    // manages their lifetimes, if they fail to allocate the draws that reference them need to
    // be dropped.
    unsigned int numProxies;
    const sk_sp<GrTextureProxy>* proxies = fRestrictedAtlasManager->getProxies(
                                                            this->maskFormat(), &numProxies);
    for (unsigned int i = 0; i < numProxies; ++i) {
        if (proxies[i]) {
            func(proxies[i].get());
        }
    }
}

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
        color.setToConstant(this->color());
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
    auto analysis = fProcessors.finalize(color, coverage, clip, false, caps, dstIsClamped,
                                         &fGeoData[0].fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    fCanCombineOnTouchOrOverlap =
            !analysis.requiresDstTexture() &&
            !(fProcessors.xferProcessor() && fProcessors.xferProcessor()->xferBarrierType(caps));
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

static void clip_quads(const SkIRect& clipRect, char* currVertex, const char* blobVertices,
                       size_t vertexStride, int glyphCount) {
    for (int i = 0; i < glyphCount; ++i) {
        const SkPoint* blobPositionLT = reinterpret_cast<const SkPoint*>(blobVertices);
        const SkPoint* blobPositionRB =
                reinterpret_cast<const SkPoint*>(blobVertices + 3 * vertexStride);

        // positions for bitmap glyphs are pixel boundary aligned
        SkIRect positionRect = SkIRect::MakeLTRB(SkScalarRoundToInt(blobPositionLT->fX),
                                                 SkScalarRoundToInt(blobPositionLT->fY),
                                                 SkScalarRoundToInt(blobPositionRB->fX),
                                                 SkScalarRoundToInt(blobPositionRB->fY));
        if (clipRect.contains(positionRect)) {
            memcpy(currVertex, blobVertices, 4 * vertexStride);
            currVertex += 4 * vertexStride;
        } else {
            // Pull out some more data that we'll need.
            // In the LCD case the color will be garbage, but we'll overwrite it with the texcoords
            // and it avoids a lot of conditionals.
            auto color = *reinterpret_cast<const SkColor*>(blobVertices + sizeof(SkPoint));
            size_t coordOffset = vertexStride - 2*sizeof(uint16_t);
            auto* blobCoordsLT = reinterpret_cast<const uint16_t*>(blobVertices + coordOffset);
            auto* blobCoordsRB = reinterpret_cast<const uint16_t*>(blobVertices + 3 * vertexStride +
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
    auto resourceProvider = target->resourceProvider();

    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix.
    // TODO actually only invert if we don't have RGBA
    SkMatrix localMatrix;
    if (this->usesLocalCoords() && !fGeoData[0].fViewMatrix.invert(&localMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    GrAtlasManager* fullAtlasManager = target->fullAtlasManager();
    SkASSERT(fRestrictedAtlasManager == fullAtlasManager);
    GrGlyphCache* glyphCache = target->glyphCache();

    GrMaskFormat maskFormat = this->maskFormat();

    unsigned int atlasPageCount;
    const sk_sp<GrTextureProxy>* proxies = fullAtlasManager->getProxies(maskFormat,
                                                                        &atlasPageCount);
    if (!proxies[0]) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }

    FlushInfo flushInfo;
    flushInfo.fPipeline =
            target->makePipeline(fSRGBFlags, std::move(fProcessors), target->detachAppliedClip());
    SkDEBUGCODE(bool dfPerspective = false);
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor = this->setupDfProcessor(fullAtlasManager);
        SkDEBUGCODE(dfPerspective = fGeoData[0].fViewMatrix.hasPerspective());
    } else {
        flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(
            this->color(), proxies, atlasPageCount, GrSamplerState::ClampNearest(), maskFormat,
            localMatrix, this->usesLocalCoords());
    }

    flushInfo.fGlyphsToFlush = 0;
    size_t vertexStride = flushInfo.fGeometryProcessor->getVertexStride();
    SkASSERT(vertexStride == GrAtlasTextBlob::GetVertexStride(maskFormat, dfPerspective));

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

    char* currVertex = reinterpret_cast<char*>(vertices);

    SkAutoGlyphCache autoGlyphCache;
    // each of these is a SubRun
    for (int i = 0; i < fGeoCount; i++) {
        const Geometry& args = fGeoData[i];
        Blob* blob = args.fBlob;
        GrAtlasTextBlob::VertexRegenerator regenerator(
                resourceProvider, blob, args.fRun, args.fSubRun, args.fViewMatrix, args.fX, args.fY,
                args.fColor, target->deferredUploadTarget(), glyphCache, fullAtlasManager,
                &autoGlyphCache);
        GrAtlasTextBlob::VertexRegenerator::Result result;
        do {
            result = regenerator.regenerate();
            // Copy regenerated vertices from the blob to our vertex buffer.
            size_t vertexBytes = result.fGlyphsRegenerated * kVerticesPerGlyph * vertexStride;
            if (args.fClipRect.isEmpty()) {
                memcpy(currVertex, result.fFirstVertex, vertexBytes);
            } else {
                SkASSERT(!dfPerspective);
                clip_quads(args.fClipRect, currVertex, result.fFirstVertex, vertexStride,
                           result.fGlyphsRegenerated);
            }
            if (this->usesDistanceFields() && !args.fViewMatrix.isIdentity()) {
                // We always do the distance field view matrix transformation after copying rather
                // than during blob vertex generation time in the blob as handling successive
                // arbitrary transformations would be complicated and accumulate error.
                if (args.fViewMatrix.hasPerspective()) {
                    auto* pos = reinterpret_cast<SkPoint3*>(currVertex);
                    SkMatrixPriv::MapHomogeneousPointsWithStride(
                            args.fViewMatrix, pos, vertexStride, pos, vertexStride,
                            result.fGlyphsRegenerated * kVerticesPerGlyph);
                } else {
                    auto* pos = reinterpret_cast<SkPoint*>(currVertex);
                    SkMatrixPriv::MapPointsWithStride(
                            args.fViewMatrix, pos, vertexStride,
                            result.fGlyphsRegenerated * kVerticesPerGlyph);
                }
            }
            flushInfo.fGlyphsToFlush += result.fGlyphsRegenerated;
            if (!result.fFinished) {
                this->flush(target, &flushInfo);
            }
            currVertex += vertexBytes;
        } while (!result.fFinished);
    }
    this->flush(target, &flushInfo);
}

void GrAtlasTextOp::flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
    auto fullAtlasManager = target->fullAtlasManager();
    SkASSERT(fRestrictedAtlasManager == fullAtlasManager);

    GrGeometryProcessor* gp = flushInfo->fGeometryProcessor.get();
    GrMaskFormat maskFormat = this->maskFormat();

    unsigned int numProxies;
    const sk_sp<GrTextureProxy>* proxies = fullAtlasManager->getProxies(maskFormat, &numProxies);
    if (gp->numTextureSamplers() != (int) numProxies) {
        // During preparation the number of atlas pages has increased.
        // Update the proxies used in the GP to match.
        if (this->usesDistanceFields()) {
            if (this->isLCD()) {
                reinterpret_cast<GrDistanceFieldLCDTextGeoProc*>(gp)->addNewProxies(
                    proxies, numProxies, GrSamplerState::ClampBilerp());
            } else {
                reinterpret_cast<GrDistanceFieldA8TextGeoProc*>(gp)->addNewProxies(
                    proxies, numProxies, GrSamplerState::ClampBilerp());
            }
        } else {
            reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewProxies(
                proxies, numProxies, GrSamplerState::ClampNearest());
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

    const SkMatrix& thisFirstMatrix = fGeoData[0].fViewMatrix;
    const SkMatrix& thatFirstMatrix = that->fGeoData[0].fViewMatrix;

    if (this->usesLocalCoords() && !thisFirstMatrix.cheapEqualTo(thatFirstMatrix)) {
        return false;
    }

    if (this->usesDistanceFields()) {
        if (fDFGPFlags != that->fDFGPFlags) {
            return false;
        }

        if (fLuminanceColor != that->fLuminanceColor) {
            return false;
        }
    } else {
        if (kColorBitmapMask_MaskType == fMaskType && this->color() != that->color()) {
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
sk_sp<GrGeometryProcessor> GrAtlasTextOp::setupDfProcessor(
                                        GrRestrictedAtlasManager* restrictedAtlasManager) const {
    unsigned int numProxies;
    const sk_sp<GrTextureProxy>* proxies = restrictedAtlasManager->getProxies(this->maskFormat(),
                                                                              &numProxies);
    bool isLCD = this->isLCD();

    SkMatrix localMatrix = SkMatrix::I();
    if (this->usesLocalCoords()) {
        // If this fails we'll just use I().
        bool result = fGeoData[0].fViewMatrix.invert(&localMatrix);
        (void)result;
    }

    // see if we need to create a new effect
    if (isLCD) {
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
        return GrDistanceFieldLCDTextGeoProc::Make(proxies, numProxies,
                                                   GrSamplerState::ClampBilerp(), widthAdjust,
                                                   fDFGPFlags, localMatrix);
    } else {
#ifdef SK_GAMMA_APPLY_TO_A8
        float correction = 0;
        if (kAliasedDistanceField_MaskType != fMaskType) {
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT,
                                                                fLuminanceColor);
            correction = fDistanceAdjustTable->getAdjustment(lum >> kDistanceAdjustLumShift,
                                                             fUseGammaCorrectDistanceTable);
        }
        return GrDistanceFieldA8TextGeoProc::Make(proxies, numProxies,
                                                  GrSamplerState::ClampBilerp(),
                                                  correction, fDFGPFlags, localMatrix);
#else
        return GrDistanceFieldA8TextGeoProc::Make(proxies, numProxies,
                                                  GrSamplerState::ClampBilerp(),
                                                  fDFGPFlags, localMatrix);
#endif
    }
}

