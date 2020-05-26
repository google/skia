/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrAtlasTextOp.h"

#include "include/core/SkPoint3.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkSpan.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrBitmapTextGeoProc.h"
#include "src/gpu/effects/GrDistanceFieldGeoProc.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrDistanceFieldAdjustTable.h"

#if GR_TEST_UTILS
#include "src/gpu/GrDrawOpTest.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

GrAtlasTextOp::GrAtlasTextOp(MaskType maskType,
                             bool needsTransform,
                             int glyphCount,
                             SkRect deviceRect,
                             const Geometry& geo,
                             GrPaint&& paint)
         : INHERITED{ClassID()}
         , fMaskType{maskType}
         , fNeedsGlyphTransform{needsTransform}
         , fLuminanceColor{0}
         , fUseGammaCorrectDistanceTable{false}
         , fDFGPFlags{0}
         , fGeoDataAllocSize{kMinGeometryAllocated}
         , fProcessors{std::move(paint)}
         , fNumGlyphs{glyphCount} {
    new (&fGeoData[0]) Geometry{geo};
    fGeoCount = 1;

    // We don't have tight bounds on the glyph paths in device space. For the purposes of bounds
    // we treat this as a set of non-AA rects rendered with a texture.
    this->setBounds(deviceRect, HasAABloat::kNo, IsHairline::kNo);
}

GrAtlasTextOp::GrAtlasTextOp(MaskType maskType,
                             bool needsTransform,
                             int glyphCount,
                             SkRect deviceRect,
                             SkColor luminanceColor,
                             bool useGammaCorrectDistanceTable,
                             uint32_t DFGPFlags,
                             const Geometry& geo,
                             GrPaint&& paint)
        : INHERITED{ClassID()}
        , fMaskType{maskType}
        , fNeedsGlyphTransform{needsTransform}
        , fLuminanceColor{luminanceColor}
        , fUseGammaCorrectDistanceTable{useGammaCorrectDistanceTable}
        , fDFGPFlags{DFGPFlags}
        , fGeoDataAllocSize{kMinGeometryAllocated}
        , fProcessors{std::move(paint)}
        , fNumGlyphs{glyphCount} {
    new (&fGeoData[0]) Geometry{geo};
    fGeoCount = 1;

    // We don't have tight bounds on the glyph paths in device space. For the purposes of bounds
    // we treat this as a set of non-AA rects rendered with a texture.
    this->setBounds(deviceRect, HasAABloat::kNo, IsHairline::kNo);
}

void GrAtlasTextOp::Geometry::fillVertexData(void *dst, int offset, int count) const {
    fSubRun.fillVertexData(dst, offset, count, fColor.toBytes_RGBA(),
                           fDrawMatrix, fDrawOrigin, fClipRect);
}

void GrAtlasTextOp::visitProxies(const VisitProxyFunc& func) const {
    fProcessors.visitProxies(func);
}

#if GR_TEST_UTILS
SkString GrAtlasTextOp::onDumpInfo() const {
    SkString str;

    for (int i = 0; i < fGeoCount; ++i) {
        str.appendf("%d: Color: 0x%08x Trans: %.2f,%.2f\n",
                    i,
                    fGeoData[i].fColor.toBytes_RGBA(),
                    fGeoData[i].fDrawOrigin.x(),
                    fGeoData[i].fDrawOrigin.y());
    }

    str += fProcessors.dumpProcessors();
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

void GrAtlasTextOp::onPrepareDraws(Target* target) {
    auto resourceProvider = target->resourceProvider();

    // if we have RGB, then we won't have any SkShaders so no need to use a localmatrix.
    // TODO actually only invert if we don't have RGBA
    SkMatrix localMatrix;
    if (this->usesLocalCoords() && !fGeoData[0].fDrawMatrix.invert(&localMatrix)) {
        return;
    }

    GrAtlasManager* atlasManager = target->atlasManager();

    GrMaskFormat maskFormat = this->maskFormat();

    unsigned int numActiveViews;
    const GrSurfaceProxyView* views = atlasManager->getViews(maskFormat, &numActiveViews);
    if (!views) {
        SkDebugf("Could not allocate backing texture for atlas\n");
        return;
    }
    SkASSERT(views[0].proxy());

    static constexpr int kMaxTextures = GrBitmapTextGeoProc::kMaxTextures;
    static_assert(GrDistanceFieldA8TextGeoProc::kMaxTextures == kMaxTextures);
    static_assert(GrDistanceFieldLCDTextGeoProc::kMaxTextures == kMaxTextures);

    auto primProcProxies = target->allocPrimProcProxyPtrs(kMaxTextures);
    for (unsigned i = 0; i < numActiveViews; ++i) {
        primProcProxies[i] = views[i].proxy();
        // This op does not know its atlas proxies when it is added to a GrOpsTasks, so the proxies
        // don't get added during the visitProxies call. Thus we add them here.
        target->sampledProxyArray()->push_back(views[i].proxy());
    }

    FlushInfo flushInfo;
    flushInfo.fPrimProcProxies = primProcProxies;
    flushInfo.fIndexBuffer = resourceProvider->refNonAAQuadIndexBuffer();

    bool vmPerspective = fGeoData[0].fDrawMatrix.hasPerspective();
    if (this->usesDistanceFields()) {
        flushInfo.fGeometryProcessor = this->setupDfProcessor(target->allocator(),
                                                              *target->caps().shaderCaps(),
                                                              views, numActiveViews);
    } else {
        auto filter = fNeedsGlyphTransform ? GrSamplerState::Filter::kLinear
                                           : GrSamplerState::Filter::kNearest;
        flushInfo.fGeometryProcessor = GrBitmapTextGeoProc::Make(
                target->allocator(), *target->caps().shaderCaps(), this->color(), false, views,
                numActiveViews, filter, maskFormat, localMatrix, vmPerspective);
    }

    const int vertexStride = (int)flushInfo.fGeometryProcessor->vertexStride();

    // Ensure we don't request an insanely large contiguous vertex allocation.
    static const int kMaxVertexBytes = GrBufferAllocPool::kDefaultBufferSize;
    const int quadSize = vertexStride * kVerticesPerGlyph;
    const int maxQuadsPerBuffer = kMaxVertexBytes / quadSize;

    int allGlyphsCursor = 0;
    const int allGlyphsEnd = this->numGlyphs();
    int quadCursor;
    int quadEnd;
    char* vertices;

    auto resetVertexBuffer = [&] {
        quadCursor = 0;
        quadEnd = std::min(maxQuadsPerBuffer, allGlyphsEnd - allGlyphsCursor);

        vertices = (char*)target->makeVertexSpace(
                vertexStride,
                kVerticesPerGlyph * quadEnd,
                &flushInfo.fVertexBuffer,
                &flushInfo.fVertexOffset);

        if (!vertices || !flushInfo.fVertexBuffer) {
            SkDebugf("Could not allocate vertices\n");
            return false;
        }
        return true;
    };

    resetVertexBuffer();

    for (const Geometry& geo : SkSpan(fGeoData.get(), fGeoCount)) {
        const GrAtlasSubRun& subRun = geo.fSubRun;
        SkASSERT((int)subRun.vertexStride() == vertexStride);

        const int subRunEnd = subRun.glyphCount();
        for (int subRunCursor = 0; subRunCursor < subRunEnd;) {
            // Regenerate the atlas for the remainder of the glyphs in the run, or the remainder
            // of the glyphs to fill the vertex buffer.
            int regenEnd = subRunCursor + std::min(subRunEnd - subRunCursor, quadEnd - quadCursor);
            auto[ok, glyphsRegenerated] = subRun.regenerateAtlas(subRunCursor, regenEnd, target);
            // There was a problem allocating the glyph in the atlas. Bail.
            if (!ok) {
                return;
            }

            geo.fillVertexData(vertices + quadCursor * quadSize, subRunCursor, glyphsRegenerated);

            subRunCursor += glyphsRegenerated;
            quadCursor += glyphsRegenerated;
            allGlyphsCursor += glyphsRegenerated;
            flushInfo.fGlyphsToFlush += glyphsRegenerated;

            if (quadCursor == quadEnd || subRunCursor < subRunEnd) {
                // Flush if not all the glyphs are drawn because either the quad buffer is full or
                // the atlas is out of space.
                this->createDrawForGeneratedGlyphs(target, &flushInfo);
                if (quadCursor == quadEnd && allGlyphsCursor < allGlyphsEnd) {
                    // If the vertex buffer is full and there are still glyphs to draw then
                    // get a new buffer.
                    if(!resetVertexBuffer()) {
                        return;
                    }
                }
            }
        }
    }
}

void GrAtlasTextOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                             std::move(fProcessors),
                                                             GrPipeline::InputFlags::kNone);

    flushState->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds, pipeline,
                                                    &GrUserStencilSettings::kUnused);
}

void GrAtlasTextOp::createDrawForGeneratedGlyphs(
        GrMeshDrawOp::Target* target, FlushInfo* flushInfo) const {
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
            flushInfo->fPrimProcProxies[i] = views[i].proxy();
            // This op does not know its atlas proxies when it is added to a GrOpsTasks, so the
            // proxies don't get added during the visitProxies call. Thus we add them here.
            target->sampledProxyArray()->push_back(views[i].proxy());
            // These will get unreffed when the previously recorded draws destruct.
            for (int d = 0; d < flushInfo->fNumDraws; ++d) {
                flushInfo->fPrimProcProxies[i]->ref();
            }
        }
        if (this->usesDistanceFields()) {
            if (this->isLCD()) {
                reinterpret_cast<GrDistanceFieldLCDTextGeoProc*>(gp)->addNewViews(
                        views, numActiveViews, GrSamplerState::Filter::kLinear);
            } else {
                reinterpret_cast<GrDistanceFieldA8TextGeoProc*>(gp)->addNewViews(
                        views, numActiveViews, GrSamplerState::Filter::kLinear);
            }
        } else {
            auto filter = fNeedsGlyphTransform ? GrSamplerState::Filter::kLinear
                                               : GrSamplerState::Filter::kNearest;
            reinterpret_cast<GrBitmapTextGeoProc*>(gp)->addNewViews(views, numActiveViews, filter);
        }
    }
    int maxGlyphsPerDraw = static_cast<int>(flushInfo->fIndexBuffer->size() / sizeof(uint16_t) / 6);
    GrSimpleMesh* mesh = target->allocMesh();
    mesh->setIndexedPatterned(flushInfo->fIndexBuffer, kIndicesPerGlyph, flushInfo->fGlyphsToFlush,
                              maxGlyphsPerDraw, flushInfo->fVertexBuffer, kVerticesPerGlyph,
                              flushInfo->fVertexOffset);
    target->recordDraw(flushInfo->fGeometryProcessor, mesh, 1, flushInfo->fPrimProcProxies,
                       GrPrimitiveType::kTriangles);
    flushInfo->fVertexOffset += kVerticesPerGlyph * flushInfo->fGlyphsToFlush;
    flushInfo->fGlyphsToFlush = 0;
    ++flushInfo->fNumDraws;
}

GrOp::CombineResult GrAtlasTextOp::onCombineIfPossible(GrOp* t, GrRecordingContext::Arenas*,
                                                       const GrCaps& caps) {
    GrAtlasTextOp* that = t->cast<GrAtlasTextOp>();
    if (fProcessors != that->fProcessors) {
        return CombineResult::kCannotCombine;
    }

    if (fMaskType != that->fMaskType) {
        return CombineResult::kCannotCombine;
    }

    const SkMatrix& thisFirstMatrix = fGeoData[0].fDrawMatrix;
    const SkMatrix& thatFirstMatrix = that->fGeoData[0].fDrawMatrix;

    if (this->usesLocalCoords() && !SkMatrixPriv::CheapEqual(thisFirstMatrix, thatFirstMatrix)) {
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
    for (int i = 0; i < that->fGeoCount; i++) {
        new (&fGeoData[fGeoCount + i]) Geometry{that->fGeoData[i]};
    }

    that->fGeoCount = 0;
    fGeoCount = newGeoCount;

    return CombineResult::kMerged;
}

static const int kDistanceAdjustLumShift = 5;

// TODO trying to figure out why lcd is so whack
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

    auto dfAdjustTable = GrDistanceFieldAdjustTable::Get();

    // see if we need to create a new effect
    if (isLCD) {
        float redCorrection = dfAdjustTable->getAdjustment(
                SkColorGetR(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float greenCorrection = dfAdjustTable->getAdjustment(
                SkColorGetG(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        float blueCorrection = dfAdjustTable->getAdjustment(
                SkColorGetB(fLuminanceColor) >> kDistanceAdjustLumShift,
                fUseGammaCorrectDistanceTable);
        GrDistanceFieldLCDTextGeoProc::DistanceAdjust widthAdjust =
                GrDistanceFieldLCDTextGeoProc::DistanceAdjust::Make(
                        redCorrection, greenCorrection, blueCorrection);
        return GrDistanceFieldLCDTextGeoProc::Make(arena, caps, views, numActiveViews,
                                                   GrSamplerState::Filter::kLinear, widthAdjust,
                                                   fDFGPFlags, localMatrix);
    } else {
#ifdef SK_GAMMA_APPLY_TO_A8
        float correction = 0;
        if (kAliasedDistanceField_MaskType != fMaskType) {
            U8CPU lum = SkColorSpaceLuminance::computeLuminance(SK_GAMMA_EXPONENT,
                                                                fLuminanceColor);
            correction = dfAdjustTable->getAdjustment(lum >> kDistanceAdjustLumShift,
                                                      fUseGammaCorrectDistanceTable);
        }
        return GrDistanceFieldA8TextGeoProc::Make(arena, caps, views, numActiveViews,
                                                  GrSamplerState::Filter::kLinear, correction,
                                                  fDFGPFlags, localMatrix);
#else
        return GrDistanceFieldA8TextGeoProc::Make(arena, caps, views, numActiveViews,
                                                  GrSamplerState::Filter::kLinear, fDFGPFlags,
                                                  localMatrix);
#endif
    }
}

#if GR_TEST_UTILS
std::unique_ptr<GrDrawOp> GrAtlasTextOp::CreateOpTestingOnly(GrRenderTargetContext* rtc,
                                                             const SkPaint& skPaint,
                                                             const SkFont& font,
                                                             const SkMatrixProvider& mtxProvider,
                                                             const char* text,
                                                             int x,
                                                             int y) {
    size_t textLen = (int)strlen(text);

    const SkMatrix& drawMatrix(mtxProvider.localToDevice());

    auto drawOrigin = SkPoint::Make(x, y);
    SkGlyphRunBuilder builder;
    builder.drawTextUTF8(skPaint, font, text, textLen, drawOrigin);

    auto glyphRunList = builder.useGlyphRunList();
    if (glyphRunList.empty()) {
        return nullptr;
    }


    auto rContext = rtc->priv().recordingContext();
    GrSDFTOptions SDFOptions = rContext->priv().SDFTOptions();

    sk_sp<GrTextBlob> blob = GrTextBlob::Make(glyphRunList, drawMatrix);
    SkGlyphRunListPainter* painter = rtc->priv().testingOnly_glyphRunPainter();
    painter->processGlyphRunList(
            glyphRunList, drawMatrix, rtc->surfaceProps(),
            rContext->priv().caps()->shaderCaps()->supportsDistanceFieldText(),
            SDFOptions, blob.get());
    if (!blob->subRunList().head()) {
        return nullptr;
    }

    GrAtlasSubRun* subRun = static_cast<GrAtlasSubRun*>(blob->subRunList().head());
    std::unique_ptr<GrDrawOp> op;
    std::tie(std::ignore, op) = subRun->makeAtlasTextOp(nullptr, mtxProvider, glyphRunList, rtc);
    return op;
}

GR_DRAW_OP_TEST_DEFINE(GrAtlasTextOp) {
    // Setup dummy SkPaint / GrPaint / GrRenderTargetContext
    auto rtc = GrRenderTargetContext::Make(
            context, GrColorType::kRGBA_8888, nullptr, SkBackingFit::kApprox, {1024, 1024});

    SkSimpleMatrixProvider matrixProvider(GrTest::TestMatrixInvertible(random));

    SkPaint skPaint;
    skPaint.setColor(random->nextU());

    SkFont font;
    if (random->nextBool()) {
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    } else {
        font.setEdging(random->nextBool() ? SkFont::Edging::kAntiAlias : SkFont::Edging::kAlias);
    }
    font.setSubpixel(random->nextBool());

    const char* text = "The quick brown fox jumps over the lazy dog.";

    // create some random x/y offsets, including negative offsets
    static const int kMaxTrans = 1024;
    int xPos = (random->nextU() % 2) * 2 - 1;
    int yPos = (random->nextU() % 2) * 2 - 1;
    int xInt = (random->nextU() % kMaxTrans) * xPos;
    int yInt = (random->nextU() % kMaxTrans) * yPos;

    return GrAtlasTextOp::CreateOpTestingOnly(
            rtc.get(), skPaint, font, matrixProvider, text, xInt, yInt);
}

#endif


