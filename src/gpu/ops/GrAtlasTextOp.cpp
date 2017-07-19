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
                                                     const GrAppliedClip* clip) {
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
    auto analysis = fProcessors.finalize(color, coverage, clip, false, caps, &fColor);
    fUsesLocalCoords = analysis.usesLocalCoords();
    fCanCombineOnTouchOrOverlap =
            !analysis.requiresDstTexture() &&
            !(fProcessors.xferProcessor() && fProcessors.xferProcessor()->xferBarrierType(caps));
    return analysis.requiresDstTexture() ? RequiresDstTexture::kYes : RequiresDstTexture::kNo;
}

void GrAtlasTextOp::onPrepareDraws(Target* target) const {
    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix.
    // TODO actually only invert if we don't have RGBA
    SkMatrix localMatrix;
    if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
        SkDebugf("Cannot invert viewmatrix\n");
        return;
    }

    sk_sp<GrTextureProxy> proxy = fFontCache->getProxy(this->maskFormat());
    if (!proxy) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }

    GrMaskFormat maskFormat = this->maskFormat();

    FlushInfo flushInfo;
    flushInfo.fPipeline = target->makePipeline(fSRGBFlags, &fProcessors);
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor =
                this->setupDfProcessor(this->viewMatrix(),
                                       fLuminanceColor, this->color(), std::move(proxy));
    } else {
        GrSamplerParams params(SkShader::kClamp_TileMode, GrSamplerParams::kNone_FilterMode);
        flushInfo.fGeometryProcessor =
                GrBitmapTextGeoProc::Make(this->color(), std::move(proxy), params, maskFormat,
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
    flushInfo.fIndexBuffer.reset(target->resourceProvider()->refQuadIndexBuffer());
    if (!vertices || !flushInfo.fVertexBuffer) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    unsigned char* currVertex = reinterpret_cast<unsigned char*>(vertices);

    GrBlobRegenHelper helper(this, target, &flushInfo);
    SkAutoGlyphCache glyphCache;
    for (int i = 0; i < fGeoCount; i++) {
        const Geometry& args = fGeoData[i];
        Blob* blob = args.fBlob;
        size_t byteCount;
        void* blobVertices;
        int subRunGlyphCount;
        blob->regenInOp(target, fFontCache, &helper, args.fRun, args.fSubRun, &glyphCache,
                        vertexStride, args.fViewMatrix, args.fX, args.fY, args.fColor,
                        &blobVertices, &byteCount, &subRunGlyphCount);

        // now copy all vertices
        memcpy(currVertex, blobVertices, byteCount);

        currVertex += byteCount;
    }

    this->flush(target, &flushInfo);
}

void GrAtlasTextOp::flush(GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
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

    fNumGlyphs += that->numGlyphs();

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

// TODO just use class params
// TODO trying to figure out why lcd is so whack
sk_sp<GrGeometryProcessor> GrAtlasTextOp::setupDfProcessor(const SkMatrix& viewMatrix,
                                                           SkColor luminanceColor,
                                                           GrColor color,
                                                           sk_sp<GrTextureProxy> proxy) const {
    GrSamplerParams params(SkShader::kClamp_TileMode, GrSamplerParams::kBilerp_FilterMode);
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
                SkColorGetR(luminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float greenCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetG(luminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float blueCorrection = fDistanceAdjustTable->getAdjustment(
                SkColorGetB(luminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust widthAdjust =
                GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(
                        redCorrection, greenCorrection, blueCorrection);

        return GrDistanceFieldLCDTextGeoProc::Make(color, viewMatrix, std::move(proxy), params,
                                                   widthAdjust, flags, this->usesLocalCoords());
    } else {
#ifdef SK_GAMMA_APPLY_TO_A8
        float correction = 0;
        if (kAliasedDistanceField_MaskType != fMaskType) {
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT, luminanceColor);
            correction = fDistanceAdjustTable->getAdjustment(lum >> kDistanceAdjustLumShift,
                                                             fUseGammaCorrectDistanceTable);
        }
        return GrDistanceFieldA8TextGeoProc::Make(color, viewMatrix, std::move(proxy), params,
                                                  correction, flags, this->usesLocalCoords());
#else
        return GrDistanceFieldA8TextGeoProc::Make(color,
                                                  viewMatrix, std::move(proxy),
                                                  params, flags, this->usesLocalCoords());
#endif
    }
}

void GrBlobRegenHelper::flush() { fOp->flush(fTarget, fFlushInfo); }
