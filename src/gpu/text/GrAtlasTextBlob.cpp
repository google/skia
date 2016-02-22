/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextBlob.h"

#include "GrBlurUtils.h"
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrTextUtils.h"
#include "SkColorFilter.h"
#include "SkDrawFilter.h"
#include "SkGlyphCache.h"
#include "SkTextBlobRunIterator.h"
#include "batches/GrAtlasTextBatch.h"

GrAtlasTextBlob* GrAtlasTextBlob::Create(GrMemoryPool* pool, int glyphCount, int runCount) {
    // We allocate size for the GrAtlasTextBlob itself, plus size for the vertices array,
    // and size for the glyphIds array.
    size_t verticesCount = glyphCount * kVerticesPerGlyph * kMaxVASize;
    size_t size = sizeof(GrAtlasTextBlob) +
                  verticesCount +
                  glyphCount * sizeof(GrGlyph**) +
                  sizeof(GrAtlasTextBlob::Run) * runCount;

    void* allocation = pool->allocate(size);
    if (CACHE_SANITY_CHECK) {
        sk_bzero(allocation, size);
    }

    GrAtlasTextBlob* cacheBlob = new (allocation) GrAtlasTextBlob;
    cacheBlob->fSize = size;

    // setup offsets for vertices / glyphs
    cacheBlob->fVertices = sizeof(GrAtlasTextBlob) + reinterpret_cast<unsigned char*>(cacheBlob);
    cacheBlob->fGlyphs = reinterpret_cast<GrGlyph**>(cacheBlob->fVertices + verticesCount);
    cacheBlob->fRuns = reinterpret_cast<GrAtlasTextBlob::Run*>(cacheBlob->fGlyphs + glyphCount);

    // Initialize runs
    for (int i = 0; i < runCount; i++) {
        new (&cacheBlob->fRuns[i]) GrAtlasTextBlob::Run;
    }
    cacheBlob->fRunCount = runCount;
    cacheBlob->fPool = pool;
    return cacheBlob;
}


SkGlyphCache* GrAtlasTextBlob::setupCache(int runIndex,
                                          const SkSurfaceProps& props,
                                          SkPaint::FakeGamma fakeGamma,
                                          const SkPaint& skPaint,
                                          const SkMatrix* viewMatrix) {
    GrAtlasTextBlob::Run* run = &fRuns[runIndex];

    // if we have an override descriptor for the run, then we should use that
    SkAutoDescriptor* desc = run->fOverrideDescriptor.get() ? run->fOverrideDescriptor.get() :
                                                              &run->fDescriptor;
    skPaint.getScalerContextDescriptor(desc, props, fakeGamma, viewMatrix);
    run->fTypeface.reset(SkSafeRef(skPaint.getTypeface()));
    return SkGlyphCache::DetachCache(run->fTypeface, desc->getDesc());
}

void GrAtlasTextBlob::appendGlyph(int runIndex,
                                  const SkRect& positions,
                                  GrColor color,
                                  GrBatchTextStrike* strike,
                                  GrGlyph* glyph,
                                  GrFontScaler* scaler, const SkGlyph& skGlyph,
                                  SkScalar x, SkScalar y, SkScalar scale, bool applyVM) {

    // If the glyph is too large we fall back to paths
    if (glyph->fTooLargeForAtlas) {
        this->appendLargeGlyph(glyph, scaler, skGlyph, x, y, scale, applyVM);
        return;
    }

    Run& run = fRuns[runIndex];
    GrMaskFormat format = glyph->fMaskFormat;

    Run::SubRunInfo* subRun = &run.fSubRunInfo.back();
    if (run.fInitialized && subRun->maskFormat() != format) {
        subRun = &run.push_back();
        subRun->setStrike(strike);
    } else if (!run.fInitialized) {
        subRun->setStrike(strike);
    }

    run.fInitialized = true;

    size_t vertexStride = GetVertexStride(format);

    subRun->setMaskFormat(format);

    subRun->joinGlyphBounds(positions);
    subRun->setColor(color);

    intptr_t vertex = reinterpret_cast<intptr_t>(this->fVertices + subRun->vertexEndIndex());

    if (kARGB_GrMaskFormat != glyph->fMaskFormat) {
        // V0
        SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fTop);
        SkColor* colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V1
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fBottom);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V2
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fBottom);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
        vertex += vertexStride;

        // V3
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fTop);
        colorPtr = reinterpret_cast<SkColor*>(vertex + sizeof(SkPoint));
        *colorPtr = color;
    } else {
        // V0
        SkPoint* position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fTop);
        vertex += vertexStride;

        // V1
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fLeft, positions.fBottom);
        vertex += vertexStride;

        // V2
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fBottom);
        vertex += vertexStride;

        // V3
        position = reinterpret_cast<SkPoint*>(vertex);
        position->set(positions.fRight, positions.fTop);
    }
    subRun->appendVertices(vertexStride);
    fGlyphs[subRun->glyphEndIndex()] = glyph;
    subRun->glyphAppended();
}

void GrAtlasTextBlob::appendLargeGlyph(GrGlyph* glyph, GrFontScaler* scaler, const SkGlyph& skGlyph,
                                       SkScalar x, SkScalar y, SkScalar scale, bool applyVM) {
    if (nullptr == glyph->fPath) {
        const SkPath* glyphPath = scaler->getGlyphPath(skGlyph);
        if (!glyphPath) {
            return;
        }

        glyph->fPath = new SkPath(*glyphPath);
    }
    fBigGlyphs.push_back(GrAtlasTextBlob::BigGlyph(*glyph->fPath, x, y, scale, applyVM));
}

bool GrAtlasTextBlob::mustRegenerate(const SkPaint& paint,
                                     GrColor color, const SkMaskFilter::BlurRec& blurRec,
                                     const SkMatrix& viewMatrix, SkScalar x, SkScalar y) {
    // If we have LCD text then our canonical color will be set to transparent, in this case we have
    // to regenerate the blob on any color change
    // We use the grPaint to get any color filter effects
    if (fKey.fCanonicalColor == SK_ColorTRANSPARENT &&
        fPaintColor != color) {
        return true;
    }

    if (fInitialViewMatrix.hasPerspective() != viewMatrix.hasPerspective()) {
        return true;
    }

    if (fInitialViewMatrix.hasPerspective() && !fInitialViewMatrix.cheapEqualTo(viewMatrix)) {
        return true;
    }

    // We only cache one masked version
    if (fKey.fHasBlur &&
        (fBlurRec.fSigma != blurRec.fSigma ||
         fBlurRec.fStyle != blurRec.fStyle ||
         fBlurRec.fQuality != blurRec.fQuality)) {
        return true;
    }

    // Similarly, we only cache one version for each style
    if (fKey.fStyle != SkPaint::kFill_Style &&
        (fStrokeInfo.fFrameWidth != paint.getStrokeWidth() ||
         fStrokeInfo.fMiterLimit != paint.getStrokeMiter() ||
         fStrokeInfo.fJoin != paint.getStrokeJoin())) {
        return true;
    }

    // Mixed blobs must be regenerated.  We could probably figure out a way to do integer scrolls
    // for mixed blobs if this becomes an issue.
    if (this->hasBitmap() && this->hasDistanceField()) {
        // Identical viewmatrices and we can reuse in all cases
        if (fInitialViewMatrix.cheapEqualTo(viewMatrix) && x == fInitialX && y == fInitialY) {
            return false;
        }
        return true;
    }

    if (this->hasBitmap()) {
        if (fInitialViewMatrix.getScaleX() != viewMatrix.getScaleX() ||
            fInitialViewMatrix.getScaleY() != viewMatrix.getScaleY() ||
            fInitialViewMatrix.getSkewX() != viewMatrix.getSkewX() ||
            fInitialViewMatrix.getSkewY() != viewMatrix.getSkewY()) {
            return true;
        }

        // We can update the positions in the cachedtextblobs without regenerating the whole blob,
        // but only for integer translations.
        // This cool bit of math will determine the necessary translation to apply to the already
        // generated vertex coordinates to move them to the correct position
        SkScalar transX = viewMatrix.getTranslateX() +
                          viewMatrix.getScaleX() * (x - fInitialX) +
                          viewMatrix.getSkewX() * (y - fInitialY) -
                          fInitialViewMatrix.getTranslateX();
        SkScalar transY = viewMatrix.getTranslateY() +
                          viewMatrix.getSkewY() * (x - fInitialX) +
                          viewMatrix.getScaleY() * (y - fInitialY) -
                          fInitialViewMatrix.getTranslateY();
        if (!SkScalarIsInt(transX) || !SkScalarIsInt(transY)) {
            return true;
        }
    } else if (this->hasDistanceField()) {
        // A scale outside of [blob.fMaxMinScale, blob.fMinMaxScale] would result in a different
        // distance field being generated, so we have to regenerate in those cases
        SkScalar newMaxScale = viewMatrix.getMaxScale();
        SkScalar oldMaxScale = fInitialViewMatrix.getMaxScale();
        SkScalar scaleAdjust = newMaxScale / oldMaxScale;
        if (scaleAdjust < fMaxMinScale || scaleAdjust > fMinMaxScale) {
            return true;
        }
    }

    // It is possible that a blob has neither distanceField nor bitmaptext.  This is in the case
    // when all of the runs inside the blob are drawn as paths.  In this case, we always regenerate
    // the blob anyways at flush time, so no need to regenerate explicitly
    return false;
}

inline GrDrawBatch* GrAtlasTextBlob::createBatch(
                                              const Run::SubRunInfo& info,
                                              int glyphCount, int run, int subRun,
                                              const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                              GrColor color,
                                              const SkPaint& skPaint, const SkSurfaceProps& props,
                                              const GrDistanceFieldAdjustTable* distanceAdjustTable,
                                              GrBatchFontCache* cache) {
    GrMaskFormat format = info.maskFormat();
    GrColor subRunColor;
    if (kARGB_GrMaskFormat == format) {
        uint8_t paintAlpha = skPaint.getAlpha();
        subRunColor = SkColorSetARGB(paintAlpha, paintAlpha, paintAlpha, paintAlpha);
    } else {
        subRunColor = color;
    }

    GrAtlasTextBatch* batch;
    if (info.drawAsDistanceFields()) {
        SkColor filteredColor;
        SkColorFilter* colorFilter = skPaint.getColorFilter();
        if (colorFilter) {
            filteredColor = colorFilter->filterColor(skPaint.getColor());
        } else {
            filteredColor = skPaint.getColor();
        }
        bool useBGR = SkPixelGeometryIsBGR(props.pixelGeometry());
        batch = GrAtlasTextBatch::CreateDistanceField(glyphCount, cache,
                                                      distanceAdjustTable, filteredColor,
                                                      info.hasUseLCDText(), useBGR);
    } else {
        batch = GrAtlasTextBatch::CreateBitmap(format, glyphCount, cache);
    }
    GrAtlasTextBatch::Geometry& geometry = batch->geometry();
    geometry.fViewMatrix = viewMatrix;
    geometry.fBlob = SkRef(this);
    geometry.fRun = run;
    geometry.fSubRun = subRun;
    geometry.fColor = subRunColor;
    geometry.fX = x;
    geometry.fY = y;
    batch->init();

    return batch;
}

inline
void GrAtlasTextBlob::flushRun(GrDrawContext* dc, GrPipelineBuilder* pipelineBuilder,
                               int run, const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                               GrColor color,
                               const SkPaint& skPaint, const SkSurfaceProps& props,
                               const GrDistanceFieldAdjustTable* distanceAdjustTable,
                               GrBatchFontCache* cache) {
    for (int subRun = 0; subRun < fRuns[run].fSubRunInfo.count(); subRun++) {
        const Run::SubRunInfo& info = fRuns[run].fSubRunInfo[subRun];
        int glyphCount = info.glyphCount();
        if (0 == glyphCount) {
            continue;
        }

        SkAutoTUnref<GrDrawBatch> batch(this->createBatch(info, glyphCount, run,
                                                          subRun, viewMatrix, x, y, color,
                                                          skPaint, props,
                                                          distanceAdjustTable, cache));
        dc->drawBatch(pipelineBuilder, batch);
    }
}

static void calculate_translation(bool applyVM,
                                  const SkMatrix& newViewMatrix, SkScalar newX, SkScalar newY,
                                  const SkMatrix& currentViewMatrix, SkScalar currentX,
                                  SkScalar currentY, SkScalar* transX, SkScalar* transY) {
    if (applyVM) {
        *transX = newViewMatrix.getTranslateX() +
                  newViewMatrix.getScaleX() * (newX - currentX) +
                  newViewMatrix.getSkewX() * (newY - currentY) -
                  currentViewMatrix.getTranslateX();

        *transY = newViewMatrix.getTranslateY() +
                  newViewMatrix.getSkewY() * (newX - currentX) +
                  newViewMatrix.getScaleY() * (newY - currentY) -
                  currentViewMatrix.getTranslateY();
    } else {
        *transX = newX - currentX;
        *transY = newY - currentY;
    }
}


void GrAtlasTextBlob::flushBigGlyphs(GrContext* context, GrDrawContext* dc,
                                     const GrClip& clip, const SkPaint& skPaint,
                                     const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                     const SkIRect& clipBounds) {
    SkScalar transX, transY;
    for (int i = 0; i < fBigGlyphs.count(); i++) {
        GrAtlasTextBlob::BigGlyph& bigGlyph = fBigGlyphs[i];
        calculate_translation(bigGlyph.fApplyVM, viewMatrix, x, y,
                              fInitialViewMatrix, fInitialX, fInitialY, &transX, &transY);
        SkMatrix ctm;
        ctm.setScale(bigGlyph.fScale, bigGlyph.fScale);
        ctm.postTranslate(bigGlyph.fX + transX, bigGlyph.fY + transY);
        if (bigGlyph.fApplyVM) {
            ctm.postConcat(viewMatrix);
        }

        GrBlurUtils::drawPathWithMaskFilter(context, dc, clip, bigGlyph.fPath,
                                            skPaint, ctm, nullptr, clipBounds, false);
    }
}

void GrAtlasTextBlob::flushRunAsPaths(GrContext* context, GrDrawContext* dc,
                                      const SkSurfaceProps& props,
                                      const SkTextBlobRunIterator& it,
                                      const GrClip& clip, const SkPaint& skPaint,
                                      SkDrawFilter* drawFilter, const SkMatrix& viewMatrix,
                                      const SkIRect& clipBounds, SkScalar x, SkScalar y) {
    SkPaint runPaint = skPaint;

    size_t textLen = it.glyphCount() * sizeof(uint16_t);
    const SkPoint& offset = it.offset();

    it.applyFontToPaint(&runPaint);

    if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
        return;
    }

    runPaint.setFlags(GrTextUtils::FilterTextFlags(props, runPaint));

    switch (it.positioning()) {
        case SkTextBlob::kDefault_Positioning:
            GrTextUtils::DrawTextAsPath(context, dc, clip, runPaint, viewMatrix,
                                        (const char *)it.glyphs(),
                                        textLen, x + offset.x(), y + offset.y(), clipBounds);
            break;
        case SkTextBlob::kHorizontal_Positioning:
            GrTextUtils::DrawPosTextAsPath(context, dc, props, clip, runPaint, viewMatrix,
                                           (const char*)it.glyphs(),
                                           textLen, it.pos(), 1, SkPoint::Make(x, y + offset.y()),
                                           clipBounds);
            break;
        case SkTextBlob::kFull_Positioning:
            GrTextUtils::DrawPosTextAsPath(context, dc, props, clip, runPaint, viewMatrix,
                                           (const char*)it.glyphs(),
                                           textLen, it.pos(), 2, SkPoint::Make(x, y), clipBounds);
            break;
    }
}

void GrAtlasTextBlob::flushCached(GrContext* context,
                                  GrDrawContext* dc,
                                  const SkTextBlob* blob,
                                  const SkSurfaceProps& props,
                                  const GrDistanceFieldAdjustTable* distanceAdjustTable,
                                  const SkPaint& skPaint,
                                  const GrPaint& grPaint,
                                  SkDrawFilter* drawFilter,
                                  const GrClip& clip,
                                  const SkMatrix& viewMatrix,
                                  const SkIRect& clipBounds,
                                  SkScalar x, SkScalar y) {
    // We loop through the runs of the blob, flushing each.  If any run is too large, then we flush
    // it as paths
    GrPipelineBuilder pipelineBuilder(grPaint, dc->accessRenderTarget(), clip);

    GrColor color = grPaint.getColor();

    SkTextBlobRunIterator it(blob);
    for (int run = 0; !it.done(); it.next(), run++) {
        if (fRuns[run].fDrawAsPaths) {
            this->flushRunAsPaths(context, dc, props, it, clip, skPaint,
                                  drawFilter, viewMatrix, clipBounds, x, y);
            continue;
        }
        this->flushRun(dc, &pipelineBuilder, run, viewMatrix, x, y, color, skPaint, props,
                       distanceAdjustTable, context->getBatchFontCache());
    }

    // Now flush big glyphs
    this->flushBigGlyphs(context, dc, clip, skPaint, viewMatrix, x, y, clipBounds);
}

void GrAtlasTextBlob::flushThrowaway(GrContext* context,
                                     GrDrawContext* dc,
                                     const SkSurfaceProps& props,
                                     const GrDistanceFieldAdjustTable* distanceAdjustTable,
                                     const SkPaint& skPaint,
                                     const GrPaint& grPaint,
                                     const GrClip& clip,
                                     const SkMatrix& viewMatrix,
                                     const SkIRect& clipBounds,
                                     SkScalar x, SkScalar y) {
    GrPipelineBuilder pipelineBuilder(grPaint, dc->accessRenderTarget(), clip);

    GrColor color = grPaint.getColor();
    for (int run = 0; run < fRunCount; run++) {
        this->flushRun(dc, &pipelineBuilder, run, viewMatrix, x, y, color, skPaint, props,
                       distanceAdjustTable, context->getBatchFontCache());
    }

    // Now flush big glyphs
    this->flushBigGlyphs(context, dc, clip, skPaint, viewMatrix, x, y, clipBounds);
}

GrDrawBatch* GrAtlasTextBlob::test_createBatch(
                                              int glyphCount, int run, int subRun,
                                              const SkMatrix& viewMatrix, SkScalar x, SkScalar y,
                                              GrColor color,
                                              const SkPaint& skPaint, const SkSurfaceProps& props,
                                              const GrDistanceFieldAdjustTable* distanceAdjustTable,
                                              GrBatchFontCache* cache) {
    const GrAtlasTextBlob::Run::SubRunInfo& info = fRuns[run].fSubRunInfo[subRun];
    return this->createBatch(info, glyphCount, run, subRun, viewMatrix, x, y, color, skPaint,
                             props, distanceAdjustTable, cache);
}

void GrAtlasTextBlob::AssertEqual(const GrAtlasTextBlob& l, const GrAtlasTextBlob& r) {
    SkASSERT_RELEASE(l.fSize == r.fSize);
    SkASSERT_RELEASE(l.fPool == r.fPool);

    SkASSERT_RELEASE(l.fBlurRec.fSigma == r.fBlurRec.fSigma);
    SkASSERT_RELEASE(l.fBlurRec.fStyle == r.fBlurRec.fStyle);
    SkASSERT_RELEASE(l.fBlurRec.fQuality == r.fBlurRec.fQuality);

    SkASSERT_RELEASE(l.fStrokeInfo.fFrameWidth == r.fStrokeInfo.fFrameWidth);
    SkASSERT_RELEASE(l.fStrokeInfo.fMiterLimit == r.fStrokeInfo.fMiterLimit);
    SkASSERT_RELEASE(l.fStrokeInfo.fJoin == r.fStrokeInfo.fJoin);

    SkASSERT_RELEASE(l.fBigGlyphs.count() == r.fBigGlyphs.count());
    for (int i = 0; i < l.fBigGlyphs.count(); i++) {
        const BigGlyph& lBigGlyph = l.fBigGlyphs[i];
        const BigGlyph& rBigGlyph = r.fBigGlyphs[i];

        SkASSERT_RELEASE(lBigGlyph.fPath == rBigGlyph.fPath);
        // We can't assert that these have the same translations
    }

    SkASSERT_RELEASE(l.fKey == r.fKey);
    //SkASSERT_RELEASE(l.fPaintColor == r.fPaintColor); // Colors might not actually be identical
    SkASSERT_RELEASE(l.fMaxMinScale == r.fMaxMinScale);
    SkASSERT_RELEASE(l.fMinMaxScale == r.fMinMaxScale);
    SkASSERT_RELEASE(l.fTextType == r.fTextType);

    SkASSERT_RELEASE(l.fRunCount == r.fRunCount);
    for (int i = 0; i < l.fRunCount; i++) {
        const Run& lRun = l.fRuns[i];
        const Run& rRun = r.fRuns[i];

        if (lRun.fTypeface.get()) {
            SkASSERT_RELEASE(rRun.fTypeface.get());
            SkASSERT_RELEASE(SkTypeface::Equal(lRun.fTypeface, rRun.fTypeface));
        } else {
            SkASSERT_RELEASE(!rRun.fTypeface.get());
        }


        SkASSERT_RELEASE(lRun.fDescriptor.getDesc());
        SkASSERT_RELEASE(rRun.fDescriptor.getDesc());
        SkASSERT_RELEASE(lRun.fDescriptor.getDesc()->equals(*rRun.fDescriptor.getDesc()));

        if (lRun.fOverrideDescriptor.get()) {
            SkASSERT_RELEASE(lRun.fOverrideDescriptor->getDesc());
            SkASSERT_RELEASE(rRun.fOverrideDescriptor.get() && rRun.fOverrideDescriptor->getDesc());
            SkASSERT_RELEASE(lRun.fOverrideDescriptor->getDesc()->equals(
                    *rRun.fOverrideDescriptor->getDesc()));
        } else {
            SkASSERT_RELEASE(!rRun.fOverrideDescriptor.get());
        }

        // color can be changed
        //SkASSERT(lRun.fColor == rRun.fColor);
        SkASSERT_RELEASE(lRun.fInitialized == rRun.fInitialized);
        SkASSERT_RELEASE(lRun.fDrawAsPaths == rRun.fDrawAsPaths);

        SkASSERT_RELEASE(lRun.fSubRunInfo.count() == rRun.fSubRunInfo.count());
        for(int j = 0; j < lRun.fSubRunInfo.count(); j++) {
            const Run::SubRunInfo& lSubRun = lRun.fSubRunInfo[j];
            const Run::SubRunInfo& rSubRun = rRun.fSubRunInfo[j];

            // TODO we can do this check, but we have to apply the VM to the old vertex bounds
            //SkASSERT_RELEASE(lSubRun.vertexBounds() == rSubRun.vertexBounds());

            if (lSubRun.strike()) {
                SkASSERT_RELEASE(rSubRun.strike());
                SkASSERT_RELEASE(GrBatchTextStrike::GetKey(*lSubRun.strike()) ==
                                 GrBatchTextStrike::GetKey(*rSubRun.strike()));

            } else {
                SkASSERT_RELEASE(!rSubRun.strike());
            }

            SkASSERT_RELEASE(lSubRun.vertexStartIndex() == rSubRun.vertexStartIndex());
            SkASSERT_RELEASE(lSubRun.vertexEndIndex() == rSubRun.vertexEndIndex());
            SkASSERT_RELEASE(lSubRun.glyphStartIndex() == rSubRun.glyphStartIndex());
            SkASSERT_RELEASE(lSubRun.glyphEndIndex() == rSubRun.glyphEndIndex());
            SkASSERT_RELEASE(lSubRun.maskFormat() == rSubRun.maskFormat());
            SkASSERT_RELEASE(lSubRun.drawAsDistanceFields() == rSubRun.drawAsDistanceFields());
            SkASSERT_RELEASE(lSubRun.hasUseLCDText() == rSubRun.hasUseLCDText());
        }
    }
}

void GrAtlasTextBlob::Run::SubRunInfo::computeTranslation(const SkMatrix& viewMatrix,
                                                          SkScalar x, SkScalar y, SkScalar* transX,
                                                          SkScalar* transY) {
    calculate_translation(!this->drawAsDistanceFields(), viewMatrix, x, y,
                          fCurrentViewMatrix, fX, fY, transX, transY);
    fCurrentViewMatrix = viewMatrix;
    fX = x;
    fY = y;
}
