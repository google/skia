/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrAtlasTextOp.h"

#include "include/core/SkPoint3.h"
#include "include/private/GrRecordingContext.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/effects/GrBitmapTextGeoProc.h"
#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrAtlasTextOp> GrAtlasTextOp::MakeBitmap(GrRecordingContext* context,
                                                         GrPaint&& paint,
                                                         GrMaskFormat maskFormat,
                                                         int glyphCount,
                                                         bool needsTransform) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        std::unique_ptr<GrAtlasTextOp> op = pool->allocate<GrAtlasTextOp>(std::move(paint));

        switch (maskFormat) {
            case kA8_GrMaskFormat:
                op->fMaskType = kGrayscaleCoverageMask_MaskType;
                break;
            case kA565_GrMaskFormat:
                op->fMaskType = kLCDCoverageMask_MaskType;
                break;
            case kARGB_GrMaskFormat:
                op->fMaskType = kColorBitmapMask_MaskType;
                break;
        }
        op->fNumGlyphs = glyphCount;
        op->fGeoCount = 1;
        op->fLuminanceColor = 0;
        op->fNeedsGlyphTransform = needsTransform;
        return op;
    }

std::unique_ptr<GrAtlasTextOp> GrAtlasTextOp::MakeDistanceField(
                                            GrRecordingContext* context,
                                            GrPaint&& paint,
                                            int glyphCount,
                                            const GrDistanceFieldAdjustTable* distanceAdjustTable,
                                            bool useGammaCorrectDistanceTable,
                                            SkColor luminanceColor,
                                            const SkSurfaceProps& props,
                                            bool isAntiAliased,
                                            bool useLCD) {
        GrOpMemoryPool* pool = context->priv().opMemoryPool();

        std::unique_ptr<GrAtlasTextOp> op = pool->allocate<GrAtlasTextOp>(std::move(paint));

        bool isBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
        bool isLCD = useLCD && SkPixelGeometryIsH(props.pixelGeometry());
        op->fMaskType = !isAntiAliased ? kAliasedDistanceField_MaskType
                                       : isLCD ? (isBGR ? kLCDBGRDistanceField_MaskType
                                                        : kLCDDistanceField_MaskType)
                                               : kGrayscaleDistanceField_MaskType;
        op->fDistanceAdjustTable.reset(SkRef(distanceAdjustTable));
        op->fUseGammaCorrectDistanceTable = useGammaCorrectDistanceTable;
        op->fLuminanceColor = luminanceColor;
        op->fNumGlyphs = glyphCount;
        op->fGeoCount = 1;
        return op;
    }

static const int kDistanceAdjustLumShift = 5;

void GrAtlasTextOp::init() {
    const Geometry& geo = fGeoData[0];
    if (this->usesDistanceFields()) {
        bool isLCD = this->isLCD();

        const SkMatrix& drawMatrix = geo.fDrawMatrix;

        fDFGPFlags = drawMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= drawMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= drawMatrix.hasPerspective() ? kPerspective_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= fUseGammaCorrectDistanceTable ? kGammaCorrect_DistanceFieldEffectFlag : 0;
        fDFGPFlags |= (kAliasedDistanceField_MaskType == fMaskType)
                              ? kAliased_DistanceFieldEffectFlag
                              : 0;

        if (isLCD) {
            fDFGPFlags |= kUseLCD_DistanceFieldEffectFlag;
            fDFGPFlags |=
                    (kLCDBGRDistanceField_MaskType == fMaskType) ? kBGR_DistanceFieldEffectFlag : 0;
        }

        fNeedsGlyphTransform = true;
    }

    SkRect bounds;
    geo.fBlob->computeSubRunBounds(
            &bounds, *geo.fSubRunPtr, geo.fDrawMatrix, geo.fDrawOrigin, fNeedsGlyphTransform);
    // We don't have tight bounds on the glyph paths in device space. For the purposes of bounds
    // we treat this as a set of non-AA rects rendered with a texture.
    this->setBounds(bounds, HasAABloat::kNo, IsHairline::kNo);
}

void GrAtlasTextOp::visitProxies(const VisitProxyFunc& func) const {
    fProcessors.visitProxies(func);
}

#ifdef SK_DEBUG
SkString GrAtlasTextOp::dumpInfo() const {
    SkString str;

    for (int i = 0; i < fGeoCount; ++i) {
        str.appendf("%d: Color: 0x%08x Trans: %.2f,%.2f\n",
                    i,
                    fGeoData[i].fColor.toBytes_RGBA(),
                    fGeoData[i].fDrawOrigin.x(),
                    fGeoData[i].fDrawOrigin.y());
    }

    str += fProcessors.dumpProcessors();
    str += INHERITED::dumpInfo();
    return str;
}
#endif

GrDrawOp::FixedFunctionFlags GrAtlasTextOp::fixedFunctionFlags() const {
    return FixedFunctionFlags::kNone;
}

GrProcessorSet::Analysis GrAtlasTextOp::finalize(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType) {
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
    auto analysis = fProcessors.finalize(
            color, coverage, clip, &GrUserStencilSettings::kUnused, hasMixedSampledCoverage, caps,
            clampType, &fGeoData[0].fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    return analysis;
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
    if (this->usesLocalCoords() && !fGeoData[0].fDrawMatrix.invert(&localMatrix)) {
        return;
    }

    GrAtlasManager* atlasManager = target->atlasManager();
    GrStrikeCache* glyphCache = target->glyphCache();

    GrMaskFormat maskFormat = this->maskFormat();

    unsigned int numActiveViews;
    const GrSurfaceProxyView* views = atlasManager->getViews(maskFormat, &numActiveViews);
    if (!views) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }
    SkASSERT(views[0].proxy());

    static constexpr int kMaxTextures = GrBitmapTextGeoProc::kMaxTextures;
    GR_STATIC_ASSERT(GrDistanceFieldA8TextGeoProc::kMaxTextures == kMaxTextures);
    GR_STATIC_ASSERT(GrDistanceFieldLCDTextGeoProc::kMaxTextures == kMaxTextures);

    auto fixedDynamicState = target->makeFixedDynamicState(kMaxTextures);
    for (unsigned i = 0; i < numActiveViews; ++i) {
        fixedDynamicState->fPrimitiveProcessorTextures[i] = views[i].proxy();
        // This op does not know its atlas proxies when it is added to a GrOpsTasks, so the proxies
        // don't get added during the visitProxies call. Thus we add them here.
        target->sampledProxyArray()->push_back(views[i].proxy());
    }

    FlushInfo flushInfo;
    flushInfo.fFixedDynamicState = fixedDynamicState;

    bool vmPerspective = fGeoData[0].fDrawMatrix.hasPerspective();
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor = this->setupDfProcessor(target->allocator(),
                                                              *target->caps().shaderCaps(),
                                                              views, numActiveViews);
    } else {
        GrSamplerState samplerState = fNeedsGlyphTransform ? GrSamplerState::ClampBilerp()
                                                           : GrSamplerState::ClampNearest();
        flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(target->allocator(),
            *target->caps().shaderCaps(), this->color(), false, views, numActiveViews,
            samplerState, maskFormat, localMatrix, vmPerspective);
    }

    flushInfo.fGlyphsToFlush = 0;
    size_t vertexStride = flushInfo.fGeometryProcessor->vertexStride();

    int glyphCount = this->numGlyphs();

    void* vertices = target->makeVertexSpace(vertexStride, glyphCount * kVerticesPerGlyph,
                                             &flushInfo.fVertexBuffer, &flushInfo.fVertexOffset);
    flushInfo.fIndexBuffer = resourceProvider->refNonAAQuadIndexBuffer();
    if (!vertices || !flushInfo.fVertexBuffer) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    char* currVertex = reinterpret_cast<char*>(vertices);

    // each of these is a SubRun
    for (int i = 0; i < fGeoCount; i++) {
        const Geometry& args = fGeoData[i];
        // TODO4F: Preserve float colors
        GrTextBlob::VertexRegenerator regenerator(
                resourceProvider, args.fSubRunPtr, args.fDrawMatrix, args.fDrawOrigin,
                args.fColor.toBytes_RGBA(), target->deferredUploadTarget(), glyphCache,
                atlasManager);
        bool done = false;
        while (!done) {
            GrTextBlob::VertexRegenerator::Result result;
            if (!regenerator.regenerate(&result)) {
                break;
            }
            done = result.fFinished;

            // Copy regenerated vertices from the blob to our vertex buffer.
            size_t vertexBytes = result.fGlyphsRegenerated * kVerticesPerGlyph * vertexStride;
            if (args.fClipRect.isEmpty()) {
                memcpy(currVertex, result.fFirstVertex, vertexBytes);
            } else {
                SkASSERT(!vmPerspective);
                clip_quads(args.fClipRect, currVertex, result.fFirstVertex, vertexStride,
                           result.fGlyphsRegenerated);
            }
            if (fNeedsGlyphTransform && !args.fDrawMatrix.isIdentity()) {
                // We always do the distance field view matrix transformation after copying rather
                // than during blob vertex generation time in the blob as handling successive
                // arbitrary transformations would be complicated and accumulate error.
                if (args.fDrawMatrix.hasPerspective()) {
                    auto* pos = reinterpret_cast<SkPoint3*>(currVertex);
                    SkMatrixPriv::MapHomogeneousPointsWithStride(
                            args.fDrawMatrix, pos, vertexStride, pos, vertexStride,
                            result.fGlyphsRegenerated * kVerticesPerGlyph);
                } else {
                    auto* pos = reinterpret_cast<SkPoint*>(currVertex);
                    SkMatrixPriv::MapPointsWithStride(
                            args.fDrawMatrix, pos, vertexStride,
                            result.fGlyphsRegenerated * kVerticesPerGlyph);
                }
            }
            flushInfo.fGlyphsToFlush += result.fGlyphsRegenerated;
            if (!result.fFinished) {
                this->flush(target, &flushInfo);
            }
            currVertex += vertexBytes;
        }
    }
    this->flush(target, &flushInfo);
}

void GrAtlasTextOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                             std::move(fProcessors),
                                                             GrPipeline::InputFlags::kNone);

    flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline);
}

void GrAtlasTextOp::flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
    if (!flushInfo->fGlyphsToFlush) {
        return;
    }

    auto atlasManager = target->atlasManager();

    GrGeometryProcessor* gp = flushInfo->fGeometryProcessor;
    GrMaskFormat maskFormat = this->maskFormat();

    unsigned int numActiveViews;
    const GrSurfaceProxyView* views = atlasManager->getViews(maskFormat, &numActiveViews);
    SkASSERT(views);
    // Something has gone terribly wrong, bail
    if (!views || 0 == numActiveViews) {
        return;
    }
    if (gp->numTextureSamplers() != (int) numActiveViews) {
        // During preparation the number of atlas pages has increased.
        // Update the proxies used in the GP to match.
        for (unsigned i = gp->numTextureSamplers(); i < numActiveViews; ++i) {
            flushInfo->fFixedDynamicState->fPrimitiveProcessorTextures[i] = views[i].proxy();
            // This op does not know its atlas proxies when it is added to a GrOpsTasks, so the
            // proxies don't get added during the visitProxies call. Thus we add them here.
            target->sampledProxyArray()->push_back(views[i].proxy());
        }
        if (this->usesDistanceFields()) {
            if (this->isLCD()) {
                reinterpret_cast<GrDistanceFieldLCDTextGeoProc*>(gp)->addNewViews(
                    views, numActiveViews, GrSamplerState::ClampBilerp());
            } else {
                reinterpret_cast<GrDistanceFieldA8TextGeoProc*>(gp)->addNewViews(
                    views, numActiveViews, GrSamplerState::ClampBilerp());
            }
        } else {
            GrSamplerState samplerState = fNeedsGlyphTransform ? GrSamplerState::ClampBilerp()
                                                               : GrSamplerState::ClampNearest();
            reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewViews(views, numActiveViews,
                                                                      samplerState);
        }
    }
    int maxGlyphsPerDraw = static_cast<int>(flushInfo->fIndexBuffer->size() / sizeof(uint16_t) / 6);
    GrMesh* mesh = target->allocMesh(GrPrimitiveType::kTriangles);
    mesh->setIndexedPatterned(flushInfo->fIndexBuffer, kIndicesPerGlyph, kVerticesPerGlyph,
                              flushInfo->fGlyphsToFlush, maxGlyphsPerDraw);
    mesh->setVertexData(flushInfo->fVertexBuffer, flushInfo->fVertexOffset);
    target->recordDraw(flushInfo->fGeometryProcessor, mesh, 1, flushInfo->fFixedDynamicState,
                       nullptr, GrPrimitiveType::kTriangles);
    flushInfo->fVertexOffset += kVerticesPerGlyph * flushInfo->fGlyphsToFlush;
    flushInfo->fGlyphsToFlush = 0;
}

GrOp::CombineResult GrAtlasTextOp::onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) {
    GrAtlasTextOp* that = t->cast<GrAtlasTextOp>();
    if (fProcessors != that->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    if (fMaskType != that->fMaskType) {
        return CombineResult::kCannotCombine;
    }

    const SkMatrix& thisFirstMatrix = fGeoData[0].fDrawMatrix;
    const SkMatrix& thatFirstMatrix = that->fGeoData[0].fDrawMatrix;

    if (this->usesLocalCoords() && !thisFirstMatrix.cheapEqualTo(thatFirstMatrix)) {
        return CombineResult::kCannotCombine;
    }

    if (fNeedsGlyphTransform != that->fNeedsGlyphTransform) {
        return CombineResult::kCannotCombine;
    }

    if (fNeedsGlyphTransform &&
        (thisFirstMatrix.hasPerspective() != thatFirstMatrix.hasPerspective())) {
        return CombineResult::kCannotCombine;
    }

    if (this->usesDistanceFields()) {
        if (fDFGPFlags != that->fDFGPFlags) {
            return CombineResult::kCannotCombine;
        }

        if (fLuminanceColor != that->fLuminanceColor) {
            return CombineResult::kCannotCombine;
        }
    } else {
        if (kColorBitmapMask_MaskType == fMaskType && this->color() != that->color()) {
            return CombineResult::kCannotCombine;
        }
    }

    // Keep the batch vertex buffer size below 32K so we don't have to create a special one
    // We use the largest possible vertex size for this
    static const int kVertexSize = sizeof(SkPoint) + sizeof(SkColor) + 2 * sizeof(uint16_t);
    static const int kMaxGlyphs = 32768 / (kVerticesPerGlyph * kVertexSize);
    if (this->fNumGlyphs + that->fNumGlyphs > kMaxGlyphs) {
        return CombineResult::kCannotCombine;
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
        that->fGeoData.get()[i].fBlob = (GrTextBlob*)0x1;
    }
#endif
    that->fGeoCount = 0;
    fGeoCount = newGeoCount;

    return CombineResult::kMerged;
}

// TODO trying to figure out why lcd is so whack
// (see comments in GrTextContext::ComputeCanonicalColor)
GrGeometryProcessor* GrAtlasTextOp::setupDfProcessor(SkArenaAlloc* arena,
                                                     const GrShaderCaps& caps,
                                                     const GrSurfaceProxyView* views,
                                                     unsigned int numActiveViews) const {
    bool isLCD = this->isLCD();

    SkMatrix localMatrix = SkMatrix::I();
    if (this->usesLocalCoords()) {
        // If this fails we'll just use I().
        bool result = fGeoData[0].fDrawMatrix.invert(&localMatrix);
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
        return GrDistanceFieldLCDTextGeoProc::Make(arena, caps, views, numActiveViews,
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
        return GrDistanceFieldA8TextGeoProc::Make(arena, caps, views, numActiveViews,
                                                  GrSamplerState::ClampBilerp(),
                                                  correction, fDFGPFlags, localMatrix);
#else
        return GrDistanceFieldA8TextGeoProc::Make(arena, caps, views, numActiveViews,
                                                  GrSamplerState::ClampBilerp(),
                                                  fDFGPFlags, localMatrix);
#endif
    }
}

