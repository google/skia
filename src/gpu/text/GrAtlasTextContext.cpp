/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GrAtlasTextContext.h"

#include "GrDrawContext.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrStrokeInfo.h"
#include "GrTextBlobCache.h"
#include "GrTexturePriv.h"
#include "GrTextUtils.h"
#include "GrVertexBuffer.h"

#include "SkAutoKern.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkDistanceFieldGen.h"
#include "SkDraw.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphCache.h"
#include "SkGpuDevice.h"
#include "SkGrPriv.h"
#include "SkPath.h"
#include "SkRTConf.h"
#include "SkStrokeRec.h"
#include "SkTextBlob.h"
#include "SkTextMapStateProc.h"

#include "batches/GrAtlasTextBatch.h"

namespace {
static const int kMinDFFontSize = 18;
static const int kSmallDFFontSize = 32;
static const int kSmallDFFontLimit = 32;
static const int kMediumDFFontSize = 72;
static const int kMediumDFFontLimit = 72;
static const int kLargeDFFontSize = 162;
#ifdef SK_BUILD_FOR_ANDROID
static const int kLargeDFFontLimit = 384;
#else
static const int kLargeDFFontLimit = 2 * kLargeDFFontSize;
#endif
};

GrAtlasTextContext::GrAtlasTextContext(GrContext* context, const SkSurfaceProps& surfaceProps)
    : INHERITED(context, surfaceProps)
    , fDistanceAdjustTable(new GrDistanceFieldAdjustTable) {
    // We overallocate vertices in our textblobs based on the assumption that A8 has the greatest
    // vertexStride
    static_assert(GrAtlasTextBlob::kGrayTextVASize >= GrAtlasTextBlob::kColorTextVASize &&
                  GrAtlasTextBlob::kGrayTextVASize >= GrAtlasTextBlob::kLCDTextVASize,
                  "vertex_attribute_changed");
    fCurrStrike = nullptr;
    fCache = context->getTextBlobCache();
}


GrAtlasTextContext* GrAtlasTextContext::Create(GrContext* context,
                                               const SkSurfaceProps& surfaceProps) {
    return new GrAtlasTextContext(context, surfaceProps);
}

bool GrAtlasTextContext::canDraw(const SkPaint& skPaint, const SkMatrix& viewMatrix) {
    return this->canDrawAsDistanceFields(skPaint, viewMatrix) ||
           !SkDraw::ShouldDrawTextAsPaths(skPaint, viewMatrix);
}

GrColor GrAtlasTextContext::ComputeCanonicalColor(const SkPaint& paint, bool lcd) {
    GrColor canonicalColor = paint.computeLuminanceColor();
    if (lcd) {
        // This is the correct computation, but there are tons of cases where LCD can be overridden.
        // For now we just regenerate if any run in a textblob has LCD.
        // TODO figure out where all of these overrides are and see if we can incorporate that logic
        // at a higher level *OR* use sRGB
        SkASSERT(false);
        //canonicalColor = SkMaskGamma::CanonicalColor(canonicalColor);
    } else {
        // A8, though can have mixed BMP text but it shouldn't matter because BMP text won't have
        // gamma corrected masks anyways, nor color
        U8CPU lum = SkComputeLuminance(SkColorGetR(canonicalColor),
                                       SkColorGetG(canonicalColor),
                                       SkColorGetB(canonicalColor));
        // reduce to our finite number of bits
        canonicalColor = SkMaskGamma::CanonicalColor(SkColorSetRGB(lum, lum, lum));
    }
    return canonicalColor;
}

// TODO if this function ever shows up in profiling, then we can compute this value when the
// textblob is being built and cache it.  However, for the time being textblobs mostly only have 1
// run so this is not a big deal to compute here.
bool GrAtlasTextContext::HasLCD(const SkTextBlob* blob) {
    SkTextBlobRunIterator it(blob);
    for (; !it.done(); it.next()) {
        if (it.isLCD()) {
            return true;
        }
    }
    return false;
}

void GrAtlasTextContext::drawTextBlob(GrDrawContext* dc,
                                      const GrClip& clip, const SkPaint& skPaint,
                                      const SkMatrix& viewMatrix, const SkTextBlob* blob,
                                      SkScalar x, SkScalar y,
                                      SkDrawFilter* drawFilter, const SkIRect& clipBounds) {
    // If we have been abandoned, then don't draw
    if (fContext->abandoned()) {
        return;
    }

    SkAutoTUnref<GrAtlasTextBlob> cacheBlob;
    SkMaskFilter::BlurRec blurRec;
    GrAtlasTextBlob::Key key;
    // It might be worth caching these things, but its not clear at this time
    // TODO for animated mask filters, this will fill up our cache.  We need a safeguard here
    const SkMaskFilter* mf = skPaint.getMaskFilter();
    bool canCache = !(skPaint.getPathEffect() ||
                      (mf && !mf->asABlur(&blurRec)) ||
                      drawFilter);

    if (canCache) {
        bool hasLCD = HasLCD(blob);

        // We canonicalize all non-lcd draws to use kUnknown_SkPixelGeometry
        SkPixelGeometry pixelGeometry = hasLCD ? fSurfaceProps.pixelGeometry() :
                                                 kUnknown_SkPixelGeometry;

        // TODO we want to figure out a way to be able to use the canonical color on LCD text,
        // see the note on ComputeCanonicalColor above.  We pick a dummy value for LCD text to
        // ensure we always match the same key
        GrColor canonicalColor = hasLCD ? SK_ColorTRANSPARENT :
                                          ComputeCanonicalColor(skPaint, hasLCD);

        key.fPixelGeometry = pixelGeometry;
        key.fUniqueID = blob->uniqueID();
        key.fStyle = skPaint.getStyle();
        key.fHasBlur = SkToBool(mf);
        key.fCanonicalColor = canonicalColor;
        cacheBlob.reset(SkSafeRef(fCache->find(key)));
    }

    SkScalar transX = 0.f;
    SkScalar transY = 0.f;

    // Though for the time being runs in the textblob can override the paint, they only touch font
    // info.
    GrPaint grPaint;
    if (!SkPaintToGrPaint(fContext, skPaint, viewMatrix, &grPaint)) {
        return;
    }

    if (cacheBlob) {
        if (cacheBlob->mustRegenerate(&transX, &transY, skPaint, grPaint.getColor(), blurRec,
                                      viewMatrix, x, y)) {
            // We have to remake the blob because changes may invalidate our masks.
            // TODO we could probably get away reuse most of the time if the pointer is unique,
            // but we'd have to clear the subrun information
            fCache->remove(cacheBlob);
            cacheBlob.reset(SkRef(fCache->createCachedBlob(blob, key, blurRec, skPaint,
                                                           GrAtlasTextBlob::kGrayTextVASize)));
            this->regenerateTextBlob(cacheBlob, skPaint, grPaint.getColor(), viewMatrix,
                                     blob, x, y, drawFilter, clip);
        } else {
            fCache->makeMRU(cacheBlob);
#ifdef CACHE_SANITY_CHECK
            {
                int glyphCount = 0;
                int runCount = 0;
                GrTextBlobCache::BlobGlyphCount(&glyphCount, &runCount, blob);
                SkAutoTUnref<GrAtlasTextBlob> sanityBlob(fCache->createBlob(glyphCount, runCount,
                                                                            kGrayTextVASize));
                GrTextBlobCache::SetupCacheBlobKey(sanityBlob, key, blurRec, skPaint);
                this->regenerateTextBlob(sanityBlob, skPaint, grPaint.getColor(), viewMatrix,
                                         blob, x, y, drawFilter, clip);
                GrAtlasTextBlob::AssertEqual(*sanityBlob, *cacheBlob);
            }

#endif
        }
    } else {
        if (canCache) {
            cacheBlob.reset(SkRef(fCache->createCachedBlob(blob, key, blurRec, skPaint,
                                                           GrAtlasTextBlob::kGrayTextVASize)));
        } else {
            cacheBlob.reset(fCache->createBlob(blob, GrAtlasTextBlob::kGrayTextVASize));
        }
        this->regenerateTextBlob(cacheBlob, skPaint, grPaint.getColor(), viewMatrix,
                                 blob, x, y, drawFilter, clip);
    }

    cacheBlob->flushCached(fContext, dc, blob, fSurfaceProps, fDistanceAdjustTable, skPaint,
                           grPaint, drawFilter, clip, viewMatrix, clipBounds, x, y, transX, transY);
}

inline bool GrAtlasTextContext::canDrawAsDistanceFields(const SkPaint& skPaint,
                                                        const SkMatrix& viewMatrix) {
    // TODO: support perspective (need getMaxScale replacement)
    if (viewMatrix.hasPerspective()) {
        return false;
    }

    SkScalar maxScale = viewMatrix.getMaxScale();
    SkScalar scaledTextSize = maxScale*skPaint.getTextSize();
    // Hinted text looks far better at small resolutions
    // Scaling up beyond 2x yields undesireable artifacts
    if (scaledTextSize < kMinDFFontSize || scaledTextSize > kLargeDFFontLimit) {
        return false;
    }

    bool useDFT = fSurfaceProps.isUseDeviceIndependentFonts();
#if SK_FORCE_DISTANCE_FIELD_TEXT
    useDFT = true;
#endif

    if (!useDFT && scaledTextSize < kLargeDFFontSize) {
        return false;
    }

    // rasterizers and mask filters modify alpha, which doesn't
    // translate well to distance
    if (skPaint.getRasterizer() || skPaint.getMaskFilter() ||
        !fContext->caps()->shaderCaps()->shaderDerivativeSupport()) {
        return false;
    }

    // TODO: add some stroking support
    if (skPaint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    return true;
}

void GrAtlasTextContext::regenerateTextBlob(GrAtlasTextBlob* cacheBlob,
                                            const SkPaint& skPaint, GrColor color,
                                            const SkMatrix& viewMatrix,
                                            const SkTextBlob* blob, SkScalar x, SkScalar y,
                                            SkDrawFilter* drawFilter,
                                            const GrClip& clip) {
    // The color here is the GrPaint color, and it is used to determine whether we
    // have to regenerate LCD text blobs.
    // We use this color vs the SkPaint color because it has the colorfilter applied.
    cacheBlob->fPaintColor = color;
    cacheBlob->fViewMatrix = viewMatrix;
    cacheBlob->fX = x;
    cacheBlob->fY = y;

    // Regenerate textblob
    SkPaint runPaint = skPaint;
    SkTextBlobRunIterator it(blob);
    for (int run = 0; !it.done(); it.next(), run++) {
        int glyphCount = it.glyphCount();
        size_t textLen = glyphCount * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        // applyFontToPaint() always overwrites the exact same attributes,
        // so it is safe to not re-seed the paint for this reason.
        it.applyFontToPaint(&runPaint);

        if (drawFilter && !drawFilter->filter(&runPaint, SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            runPaint = skPaint;
            continue;
        }

        runPaint.setFlags(FilterTextFlags(fSurfaceProps, runPaint));

        cacheBlob->push_back_run(run);

        if (this->canDrawAsDistanceFields(runPaint, viewMatrix)) {
            cacheBlob->setHasDistanceField();
            SkPaint dfPaint = runPaint;
            SkScalar textRatio;
            this->initDistanceFieldPaint(cacheBlob, &dfPaint, &textRatio, viewMatrix);
            Run& runIdx = cacheBlob->fRuns[run];
            PerSubRunInfo& subRun = runIdx.fSubRunInfo.back();
            subRun.setUseLCDText(runPaint.isLCDRenderText());
            subRun.setDrawAsDistanceFields();

            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning: {
                    this->internalDrawDFText(cacheBlob, run, dfPaint, color, viewMatrix,
                                             (const char *)it.glyphs(), textLen,
                                             x + offset.x(), y + offset.y(), textRatio, runPaint);
                    break;
                }
                case SkTextBlob::kHorizontal_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y + offset.y());
                    this->internalDrawDFPosText(cacheBlob, run, dfPaint, color, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(),
                                                1, dfOffset, textRatio,
                                                runPaint);
                    break;
                }
                case SkTextBlob::kFull_Positioning: {
                    SkPoint dfOffset = SkPoint::Make(x, y);
                    this->internalDrawDFPosText(cacheBlob, run, dfPaint, color, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(),
                                                2, dfOffset, textRatio, runPaint);
                    break;
                }
            }
        } else if (SkDraw::ShouldDrawTextAsPaths(runPaint, viewMatrix)) {
            cacheBlob->fRuns[run].fDrawAsPaths = true;
        } else {
            cacheBlob->setHasBitmap();
            switch (it.positioning()) {
                case SkTextBlob::kDefault_Positioning:
                    GrTextUtils::DrawBmpText(cacheBlob, run, fContext->getBatchFontCache(),
                                             fSurfaceProps, runPaint, color, viewMatrix,
                                             (const char *)it.glyphs(), textLen,
                                             x + offset.x(), y + offset.y());
                    break;
                case SkTextBlob::kHorizontal_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fContext->getBatchFontCache(),
                                                fSurfaceProps, runPaint, color, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(), 1,
                                                SkPoint::Make(x, y + offset.y()));
                    break;
                case SkTextBlob::kFull_Positioning:
                    GrTextUtils::DrawBmpPosText(cacheBlob, run, fContext->getBatchFontCache(),
                                                fSurfaceProps, runPaint, color, viewMatrix,
                                                (const char*)it.glyphs(), textLen, it.pos(), 2,
                                                SkPoint::Make(x, y));
                    break;
            }
        }

        if (drawFilter) {
            // A draw filter may change the paint arbitrarily, so we must re-seed in this case.
            runPaint = skPaint;
        }
    }
}

inline void GrAtlasTextContext::initDistanceFieldPaint(GrAtlasTextBlob* blob,
                                                       SkPaint* skPaint,
                                                       SkScalar* textRatio,
                                                       const SkMatrix& viewMatrix) {
    // getMaxScale doesn't support perspective, so neither do we at the moment
    SkASSERT(!viewMatrix.hasPerspective());
    SkScalar maxScale = viewMatrix.getMaxScale();
    SkScalar textSize = skPaint->getTextSize();
    SkScalar scaledTextSize = textSize;
    // if we have non-unity scale, we need to choose our base text size
    // based on the SkPaint's text size multiplied by the max scale factor
    // TODO: do we need to do this if we're scaling down (i.e. maxScale < 1)?
    if (maxScale > 0 && !SkScalarNearlyEqual(maxScale, SK_Scalar1)) {
        scaledTextSize *= maxScale;
    }

    // We have three sizes of distance field text, and within each size 'bucket' there is a floor
    // and ceiling.  A scale outside of this range would require regenerating the distance fields
    SkScalar dfMaskScaleFloor;
    SkScalar dfMaskScaleCeil;
    if (scaledTextSize <= kSmallDFFontLimit) {
        dfMaskScaleFloor = kMinDFFontSize;
        dfMaskScaleCeil = kSmallDFFontLimit;
        *textRatio = textSize / kSmallDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kSmallDFFontSize));
    } else if (scaledTextSize <= kMediumDFFontLimit) {
        dfMaskScaleFloor = kSmallDFFontLimit;
        dfMaskScaleCeil = kMediumDFFontLimit;
        *textRatio = textSize / kMediumDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kMediumDFFontSize));
    } else {
        dfMaskScaleFloor = kMediumDFFontLimit;
        dfMaskScaleCeil = kLargeDFFontLimit;
        *textRatio = textSize / kLargeDFFontSize;
        skPaint->setTextSize(SkIntToScalar(kLargeDFFontSize));
    }

    // Because there can be multiple runs in the blob, we want the overall maxMinScale, and
    // minMaxScale to make regeneration decisions.  Specifically, we want the maximum minimum scale
    // we can tolerate before we'd drop to a lower mip size, and the minimum maximum scale we can
    // tolerate before we'd have to move to a large mip size.  When we actually test these values
    // we look at the delta in scale between the new viewmatrix and the old viewmatrix, and test
    // against these values to decide if we can reuse or not(ie, will a given scale change our mip
    // level)
    SkASSERT(dfMaskScaleFloor <= scaledTextSize && scaledTextSize <= dfMaskScaleCeil);
    blob->fMaxMinScale = SkMaxScalar(dfMaskScaleFloor / scaledTextSize, blob->fMaxMinScale);
    blob->fMinMaxScale = SkMinScalar(dfMaskScaleCeil / scaledTextSize, blob->fMinMaxScale);

    skPaint->setLCDRenderText(false);
    skPaint->setAutohinted(false);
    skPaint->setHinting(SkPaint::kNormal_Hinting);
    skPaint->setSubpixelText(true);
}

inline void GrAtlasTextContext::fallbackDrawPosText(GrAtlasTextBlob* blob,
                                                    int runIndex,
                                                    GrColor color,
                                                    const SkPaint& skPaint,
                                                    const SkMatrix& viewMatrix,
                                                    const SkTDArray<char>& fallbackTxt,
                                                    const SkTDArray<SkScalar>& fallbackPos,
                                                    int scalarsPerPosition,
                                                    const SkPoint& offset) {
    SkASSERT(fallbackTxt.count());
    blob->setHasBitmap();
    Run& run = blob->fRuns[runIndex];
    // Push back a new subrun to fill and set the override descriptor
    run.push_back();
    run.fOverrideDescriptor.reset(new SkAutoDescriptor);
    GrTextUtils::DrawBmpPosText(blob, runIndex, fContext->getBatchFontCache(), fSurfaceProps,
                                skPaint, color, viewMatrix, fallbackTxt.begin(), fallbackTxt.count(),
                                fallbackPos.begin(), scalarsPerPosition, offset);
}

inline GrAtlasTextBlob*
GrAtlasTextContext::setupDFBlob(int glyphCount, const SkPaint& origPaint,
                                const SkMatrix& viewMatrix, SkPaint* dfPaint,
                                SkScalar* textRatio) {
    GrAtlasTextBlob* blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBlob::kGrayTextVASize);

    *dfPaint = origPaint;
    this->initDistanceFieldPaint(blob, dfPaint, textRatio, viewMatrix);
    blob->fViewMatrix = viewMatrix;
    Run& run = blob->fRuns[0];
    PerSubRunInfo& subRun = run.fSubRunInfo.back();
    subRun.setUseLCDText(origPaint.isLCDRenderText());
    subRun.setDrawAsDistanceFields();

    return blob;
}

inline GrAtlasTextBlob*
GrAtlasTextContext::createDrawTextBlob(const GrPaint& paint, const SkPaint& skPaint,
                                       const SkMatrix& viewMatrix,
                                       const char text[], size_t byteLength,
                                       SkScalar x, SkScalar y) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob;
    if (this->canDrawAsDistanceFields(skPaint, viewMatrix)) {
        SkPaint dfPaint;
        SkScalar textRatio;
        blob = this->setupDFBlob(glyphCount, skPaint, viewMatrix, &dfPaint, &textRatio);

        this->internalDrawDFText(blob, 0, dfPaint, paint.getColor(), viewMatrix, text,
                                 byteLength, x, y, textRatio, skPaint);
    } else {
        blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBlob::kGrayTextVASize);
        blob->fViewMatrix = viewMatrix;

        GrTextUtils::DrawBmpText(blob, 0, fContext->getBatchFontCache(), fSurfaceProps, skPaint,
                                 paint.getColor(), viewMatrix, text, byteLength, x, y);
    }
    return blob;
}

inline GrAtlasTextBlob*
GrAtlasTextContext::createDrawPosTextBlob(const GrPaint& paint, const SkPaint& skPaint,
                                          const SkMatrix& viewMatrix,
                                          const char text[], size_t byteLength,
                                          const SkScalar pos[], int scalarsPerPosition,
                                          const SkPoint& offset) {
    int glyphCount = skPaint.countText(text, byteLength);

    GrAtlasTextBlob* blob;
    if (this->canDrawAsDistanceFields(skPaint, viewMatrix)) {
        SkPaint dfPaint;
        SkScalar textRatio;
        blob = this->setupDFBlob(glyphCount, skPaint, viewMatrix, &dfPaint, &textRatio);

        this->internalDrawDFPosText(blob, 0, dfPaint, paint.getColor(), viewMatrix, text,
                                    byteLength, pos, scalarsPerPosition, offset, textRatio,
                                    skPaint);
    } else {
        blob = fCache->createBlob(glyphCount, 1, GrAtlasTextBlob::kGrayTextVASize);
        blob->fViewMatrix = viewMatrix;
        GrTextUtils::DrawBmpPosText(blob, 0, fContext->getBatchFontCache(), fSurfaceProps, skPaint,
                                    paint.getColor(), viewMatrix, text,
                                    byteLength, pos, scalarsPerPosition, offset);
    }
    return blob;
}

void GrAtlasTextContext::onDrawText(GrDrawContext* dc,
                                    const GrClip& clip,
                                    const GrPaint& paint, const SkPaint& skPaint,
                                    const SkMatrix& viewMatrix,
                                    const char text[], size_t byteLength,
                                    SkScalar x, SkScalar y, const SkIRect& regionClipBounds) {
    SkAutoTUnref<GrAtlasTextBlob> blob(
        this->createDrawTextBlob(paint, skPaint, viewMatrix, text, byteLength, x, y));
    blob->flushThrowaway(fContext, dc, fSurfaceProps, fDistanceAdjustTable, skPaint, paint,
                         clip, regionClipBounds);
}

void GrAtlasTextContext::onDrawPosText(GrDrawContext* dc,
                                       const GrClip& clip,
                                       const GrPaint& paint, const SkPaint& skPaint,
                                       const SkMatrix& viewMatrix,
                                       const char text[], size_t byteLength,
                                       const SkScalar pos[], int scalarsPerPosition,
                                       const SkPoint& offset, const SkIRect& regionClipBounds) {
    SkAutoTUnref<GrAtlasTextBlob> blob(
        this->createDrawPosTextBlob(paint, skPaint, viewMatrix,
                                    text, byteLength,
                                    pos, scalarsPerPosition,
                                    offset));

    blob->flushThrowaway(fContext, dc, fSurfaceProps, fDistanceAdjustTable, skPaint, paint, clip,
                         regionClipBounds);
}

void GrAtlasTextContext::internalDrawDFText(GrAtlasTextBlob* blob, int runIndex,
                                            const SkPaint& skPaint, GrColor color,
                                            const SkMatrix& viewMatrix,
                                            const char text[], size_t byteLength,
                                            SkScalar x, SkScalar y,
                                            SkScalar textRatio,
                                            const SkPaint& origPaint) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    SkDrawCacheProc glyphCacheProc = origPaint.getDrawCacheProc();
    SkAutoDescriptor desc;
    origPaint.getScalerContextDescriptor(&desc, fSurfaceProps, nullptr, true);
    SkGlyphCache* origPaintCache = SkGlyphCache::DetachCache(origPaint.getTypeface(),
                                                             desc.getDesc());

    SkTArray<SkScalar> positions;

    const char* textPtr = text;
    SkFixed stopX = 0;
    SkFixed stopY = 0;
    SkFixed origin = 0;
    switch (origPaint.getTextAlign()) {
        case SkPaint::kRight_Align: origin = SK_Fixed1; break;
        case SkPaint::kCenter_Align: origin = SK_FixedHalf; break;
        case SkPaint::kLeft_Align: origin = 0; break;
    }

    SkAutoKern autokern;
    const char* stop = text + byteLength;
    while (textPtr < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(origPaintCache, &textPtr, 0, 0);

        SkFixed width = glyph.fAdvanceX + autokern.adjust(glyph);
        positions.push_back(SkFixedToScalar(stopX + SkFixedMul(origin, width)));

        SkFixed height = glyph.fAdvanceY;
        positions.push_back(SkFixedToScalar(stopY + SkFixedMul(origin, height)));

        stopX += width;
        stopY += height;
    }
    SkASSERT(textPtr == stop);

    SkGlyphCache::AttachCache(origPaintCache);

    // now adjust starting point depending on alignment
    SkScalar alignX = SkFixedToScalar(stopX);
    SkScalar alignY = SkFixedToScalar(stopY);
    if (origPaint.getTextAlign() == SkPaint::kCenter_Align) {
        alignX = SkScalarHalf(alignX);
        alignY = SkScalarHalf(alignY);
    } else if (origPaint.getTextAlign() == SkPaint::kLeft_Align) {
        alignX = 0;
        alignY = 0;
    }
    x -= alignX;
    y -= alignY;
    SkPoint offset = SkPoint::Make(x, y);

    this->internalDrawDFPosText(blob, runIndex, skPaint, color, viewMatrix, text, byteLength,
                                positions.begin(), 2, offset, textRatio, origPaint);
}

void GrAtlasTextContext::internalDrawDFPosText(GrAtlasTextBlob* blob, int runIndex,
                                               const SkPaint& skPaint,
                                               GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const char text[], size_t byteLength,
                                               const SkScalar pos[], int scalarsPerPosition,
                                               const SkPoint& offset,
                                               SkScalar textRatio,
                                               const SkPaint& origPaint) {

    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    SkTDArray<char> fallbackTxt;
    SkTDArray<SkScalar> fallbackPos;

    fCurrStrike = nullptr;

    SkGlyphCache* cache = blob->setupCache(runIndex, fSurfaceProps, skPaint, nullptr, true);
    SkDrawCacheProc glyphCacheProc = skPaint.getDrawCacheProc();
    GrFontScaler* fontScaler = GetGrFontScaler(cache);

    const char* stop = text + byteLength;

    if (SkPaint::kLeft_Align == skPaint.getTextAlign()) {
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                if (!this->dfAppendGlyph(blob,
                                         runIndex,
                                         glyph,
                                         x, y, color, fontScaler,
                                         textRatio, viewMatrix)) {
                    // couldn't append, send to fallback
                    fallbackTxt.append(SkToInt(text-lastText), lastText);
                    *fallbackPos.append() = pos[0];
                    if (2 == scalarsPerPosition) {
                        *fallbackPos.append() = pos[1];
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    } else {
        SkScalar alignMul = SkPaint::kCenter_Align == skPaint.getTextAlign() ? SK_ScalarHalf
                                                                             : SK_Scalar1;
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                SkScalar advanceX = SkFixedToScalar(glyph.fAdvanceX) * alignMul * textRatio;
                SkScalar advanceY = SkFixedToScalar(glyph.fAdvanceY) * alignMul * textRatio;

                if (!this->dfAppendGlyph(blob,
                                         runIndex,
                                         glyph,
                                         x - advanceX, y - advanceY, color,
                                         fontScaler,
                                         textRatio,
                                         viewMatrix)) {
                    // couldn't append, send to fallback
                    fallbackTxt.append(SkToInt(text-lastText), lastText);
                    *fallbackPos.append() = pos[0];
                    if (2 == scalarsPerPosition) {
                        *fallbackPos.append() = pos[1];
                    }
                }
            }
            pos += scalarsPerPosition;
        }
    }

    SkGlyphCache::AttachCache(cache);
    if (fallbackTxt.count()) {
        this->fallbackDrawPosText(blob, runIndex, origPaint.getColor(), origPaint, viewMatrix,
                                  fallbackTxt, fallbackPos, scalarsPerPosition, offset);
    }
}

bool GrAtlasTextContext::dfAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                       const SkGlyph& skGlyph,
                                       SkScalar sx, SkScalar sy, GrColor color,
                                       GrFontScaler* scaler,
                                       SkScalar textRatio, const SkMatrix& viewMatrix) {
    if (!fCurrStrike) {
        fCurrStrike = fContext->getBatchFontCache()->getStrike(scaler);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kDistance_MaskStyle);
    GrGlyph* glyph = fCurrStrike->getGlyph(skGlyph, id, scaler);
    if (!glyph) {
        return true;
    }

    // fallback to color glyph support
    if (kA8_GrMaskFormat != glyph->fMaskFormat) {
        return false;
    }

    SkScalar dx = SkIntToScalar(glyph->fBounds.fLeft + SK_DistanceFieldInset);
    SkScalar dy = SkIntToScalar(glyph->fBounds.fTop + SK_DistanceFieldInset);
    SkScalar width = SkIntToScalar(glyph->fBounds.width() - 2 * SK_DistanceFieldInset);
    SkScalar height = SkIntToScalar(glyph->fBounds.height() - 2 * SK_DistanceFieldInset);

    SkScalar scale = textRatio;
    dx *= scale;
    dy *= scale;
    width *= scale;
    height *= scale;
    sx += dx;
    sy += dy;
    SkRect glyphRect = SkRect::MakeXYWH(sx, sy, width, height);

    blob->appendGlyph(runIndex, glyphRect, color, fCurrStrike, glyph, scaler, skGlyph,
                      sx - dx, sy - dy, scale, true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(TextBlobBatch) {
    static uint32_t gContextID = SK_InvalidGenID;
    static GrAtlasTextContext* gTextContext = nullptr;
    static SkSurfaceProps gSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);

    if (context->uniqueID() != gContextID) {
        gContextID = context->uniqueID();
        delete gTextContext;

        // We don't yet test the fall back to paths in the GrTextContext base class.  This is mostly
        // because we don't really want to have a gpu device here.
        // We enable distance fields by twiddling a knob on the paint
        gTextContext = GrAtlasTextContext::Create(context, gSurfaceProps);
    }

    // Setup dummy SkPaint / GrPaint
    GrColor color = GrRandomColor(random);
    SkMatrix viewMatrix = GrTest::TestMatrixInvertible(random);
    SkPaint skPaint;
    skPaint.setColor(color);
    skPaint.setLCDRenderText(random->nextBool());
    skPaint.setAntiAlias(skPaint.isLCDRenderText() ? true : random->nextBool());
    skPaint.setSubpixelText(random->nextBool());

    GrPaint grPaint;
    if (!SkPaintToGrPaint(context, skPaint, viewMatrix, &grPaint)) {
        SkFAIL("couldn't convert paint\n");
    }

    const char* text = "The quick brown fox jumps over the lazy dog.";
    int textLen = (int)strlen(text);

    // Setup clip
    GrClip clip;

    // right now we don't handle textblobs, nor do we handle drawPosText.  Since we only
    // intend to test the batch with this unit test, that is okay.
    SkAutoTUnref<GrAtlasTextBlob> blob(
            gTextContext->createDrawTextBlob(grPaint, skPaint, viewMatrix, text,
                                             static_cast<size_t>(textLen), 0, 0));

    SkScalar transX = static_cast<SkScalar>(random->nextU());
    SkScalar transY = static_cast<SkScalar>(random->nextU());
    const GrAtlasTextBlob::Run::SubRunInfo& info = blob->fRuns[0].fSubRunInfo[0];
    return blob->createBatch(info, textLen, 0, 0, color, transX, transY, skPaint,
                             gSurfaceProps, gTextContext->dfAdjustTable(),
                             context->getBatchFontCache());
}

#endif
