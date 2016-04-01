/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextBatch.h"

#include "GrBatchFlushState.h"
#include "GrResourceProvider.h"

#include "SkGlyphCache.h"

#include "effects/GrBitmapTextGeoProc.h"
#include "effects/GrDistanceFieldGeoProc.h"
#include "text/GrBatchFontCache.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

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
                    fGeoData[i].fX,
                    fGeoData[i].fY,
                    fGeoData[i].fBlob->runCount());
    }

    str.append(INHERITED::dumpInfo());
    return str;
}

void GrAtlasTextBatch::computePipelineOptimizations(GrInitInvariantOutput* color,
                                                    GrInitInvariantOutput* coverage,
                                                    GrBatchToXPOverrides* overrides) const {
    if (kColorBitmapMask_MaskType == fMaskType) {
        color->setUnknownFourComponents();
    } else {
        color->setKnownFourComponents(fBatch.fColor);
    }
    switch (fMaskType) {
        case kGrayscaleDistanceField_MaskType:
        case kGrayscaleCoverageMask_MaskType:
            coverage->setUnknownSingleComponent();
            break;
        case kLCDCoverageMask_MaskType:
        case kLCDDistanceField_MaskType:
            coverage->setUnknownOpaqueFourComponents();
            coverage->setUsingLCDCoverage();
            break;
        case kColorBitmapMask_MaskType:
            coverage->setKnownSingleComponent(0xff);
    }
}

void GrAtlasTextBatch::initBatchTracker(const GrXPOverridesForBatch& overrides) {
    // Handle any color overrides
    if (!overrides.readsColor()) {
        fGeoData[0].fColor = GrColor_ILLEGAL;
    }
    overrides.getOverrideColorIfSet(&fGeoData[0].fColor);

    // setup batch properties
    fBatch.fColorIgnored = !overrides.readsColor();
    fBatch.fColor = fGeoData[0].fColor;
    fBatch.fUsesLocalCoords = overrides.readsLocalCoords();
    fBatch.fCoverageIgnored = !overrides.readsCoverage();
}

void GrAtlasTextBatch::onPrepareDraws(Target* target) const {
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

    GrMaskFormat maskFormat = this->maskFormat();

    FlushInfo flushInfo;
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor.reset(
            this->setupDfProcessor(this->viewMatrix(), fFilteredColor, this->color(), texture));
    } else {
        GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kNone_FilterMode);
        flushInfo.fGeometryProcessor.reset(
            GrBitmapTextGeoProc::Create(this->color(),
                                        texture,
                                        params,
                                        maskFormat,
                                        localMatrix,
                                        this->usesLocalCoords()));
    }

    flushInfo.fGlyphsToFlush = 0;
    size_t vertexStride = flushInfo.fGeometryProcessor->getVertexStride();
    SkASSERT(vertexStride == GrAtlasTextBlob::GetVertexStride(maskFormat));

    int glyphCount = this->numGlyphs();
    const GrBuffer* vertexBuffer;

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

    GrBlobRegenHelper helper(this, target, &flushInfo);

    for (int i = 0; i < fGeoCount; i++) {
        const Geometry& args = fGeoData[i];
        Blob* blob = args.fBlob;
        size_t byteCount;
        void* blobVertices;
        int subRunGlyphCount;
        blob->regenInBatch(target, fFontCache, &helper, args.fRun, args.fSubRun, &cache,
                           &typeface, &scaler, &desc, vertexStride, args.fViewMatrix, args.fX,
                           args.fY, args.fColor, &blobVertices, &byteCount, &subRunGlyphCount);

        // now copy all vertices
        memcpy(currVertex, blobVertices, byteCount);

#ifdef SK_DEBUG
        // bounds sanity check
        SkRect rect;
        rect.setLargestInverted();
        SkPoint* vertex = (SkPoint*) ((char*)blobVertices);
        rect.growToInclude(vertex, vertexStride, kVerticesPerGlyph * subRunGlyphCount);

        if (this->usesDistanceFields()) {
            args.fViewMatrix.mapRect(&rect);
        }
        SkASSERT(fBounds.contains(rect));
#endif

        currVertex += byteCount;
    }

    // Make sure to attach the last cache if applicable
    if (cache) {
        SkGlyphCache::AttachCache(cache);
    }
    this->flush(target, &flushInfo);
}

void GrAtlasTextBatch::flush(GrVertexBatch::Target* target, FlushInfo* flushInfo) const {
    GrMesh mesh;
    int maxGlyphsPerDraw =
        static_cast<int>(flushInfo->fIndexBuffer->gpuMemorySize() / sizeof(uint16_t) / 6);
    mesh.initInstanced(kTriangles_GrPrimitiveType, flushInfo->fVertexBuffer,
                       flushInfo->fIndexBuffer, flushInfo->fVertexOffset,
                       kVerticesPerGlyph, kIndicesPerGlyph, flushInfo->fGlyphsToFlush,
                       maxGlyphsPerDraw);
    target->draw(flushInfo->fGeometryProcessor, mesh);
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

        if (fFilteredColor != that->fFilteredColor) {
            return false;
        }

        if (fUseBGR != that->fUseBGR) {
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
                                                        GrColor color, GrTexture* texture) const {
    GrTextureParams params(SkShader::kClamp_TileMode, GrTextureParams::kBilerp_FilterMode);
    bool isLCD = this->isLCD();
    // set up any flags
    uint32_t flags = viewMatrix.isSimilarity() ? kSimilarity_DistanceFieldEffectFlag : 0;
    flags |= viewMatrix.isScaleTranslate() ? kScaleOnly_DistanceFieldEffectFlag : 0;

    // see if we need to create a new effect
    if (isLCD) {
        flags |= kUseLCD_DistanceFieldEffectFlag;
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

void GrBlobRegenHelper::flush() {
    fBatch->flush(fTarget, fFlushInfo);
}
