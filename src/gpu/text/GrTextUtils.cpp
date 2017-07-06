/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextUtils.h"
#include "GrAtlasGlyphCache.h"
#include "GrAtlasTextBlob.h"
#include "GrBlurUtils.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrSurfaceContextPriv.h"
#include "SkDistanceFieldGen.h"
#include "SkDrawFilter.h"
#include "SkDrawProcs.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkGlyphCache.h"
#include "SkGr.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextMapStateProc.h"
#include "SkTextToPathIter.h"

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

bool GrTextUtils::Paint::toGrPaint(GrMaskFormat maskFormat, GrRenderTargetContext* rtc,
                                   const SkMatrix& viewMatrix, GrPaint* grPaint) const {
    // TODO: this is the last use of GrSurfaceContextPriv
    GrContext* context = rtc->surfPriv().getContext();
    if (kARGB_GrMaskFormat == maskFormat) {
        return SkPaintToGrPaintWithPrimitiveColor(context, rtc, this->skPaint(), grPaint);
    } else {
        return SkPaintToGrPaint(context, rtc, this->skPaint(), viewMatrix, grPaint);
    }
}

void GrTextUtils::Paint::initFilteredColor() {
    // This mirrors the logic in skpaint_to_grpaint_impl for handling paint colors
    if (fDstColorSpace) {
        GrColor4f filteredColor = SkColorToUnpremulGrColor4f(fPaint->getColor(), fDstColorSpace,
                                                             fColorXformFromSRGB);
        if (fPaint->getColorFilter()) {
            filteredColor = GrColor4f::FromSkColor4f(
                fPaint->getColorFilter()->filterColor4f(filteredColor.toSkColor4f()));
        }
        fFilteredPremulColor = filteredColor.premul().toGrColor();
    } else {
        SkColor filteredSkColor = fPaint->getColor();
        if (fPaint->getColorFilter()) {
            filteredSkColor = fPaint->getColorFilter()->filterColor(filteredSkColor);
        }
        fFilteredPremulColor = SkColorToPremulGrColor(filteredSkColor);
    }
}

bool GrTextUtils::RunPaint::modifyForRun(const SkTextBlobRunIterator& run) {
    if (!fModifiedPaint.isValid()) {
        fModifiedPaint.init(fOriginalPaint->skPaint());
        fPaint = fModifiedPaint.get();
    } else if (fFilter) {
        // We have to reset before applying the run because the filter could have arbitrary
        // changed the paint.
        *fModifiedPaint.get() = fOriginalPaint->skPaint();
    }
    run.applyFontToPaint(fModifiedPaint.get());

    if (fFilter) {
        if (!fFilter->filter(fModifiedPaint.get(), SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            return false;
        }
        // The draw filter could have changed either the paint color or color filter.
        this->initFilteredColor();
    }
    fModifiedPaint.get()->setFlags(FilterTextFlags(fProps, *fModifiedPaint.get()));
    return true;
}

void GrTextUtils::DrawBmpText(GrAtlasTextBlob* blob, int runIndex, GrAtlasGlyphCache* fontCache,
                              const SkSurfaceProps& props, const GrTextUtils::Paint& paint,
                              uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
                              const char text[], size_t byteLength, SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    // Ensure the blob is set for bitmaptext
    blob->setHasBitmap();

    GrAtlasTextStrike* currStrike = nullptr;

    SkGlyphCache* cache = blob->setupCache(runIndex, props, scalerContextFlags, paint, &viewMatrix);
    SkFindAndPlaceGlyph::ProcessText(
        paint.skPaint().getTextEncoding(), text, byteLength,
        {x, y}, viewMatrix, paint.skPaint().getTextAlign(),
        cache,
        [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
             position += rounding;
             BmpAppendGlyph(
                 blob, runIndex, fontCache, &currStrike, glyph,
                 SkScalarFloorToInt(position.fX), SkScalarFloorToInt(position.fY),
                 paint.filteredPremulColor(), cache);
        }
    );

    SkGlyphCache::AttachCache(cache);
}

void GrTextUtils::DrawBmpPosText(GrAtlasTextBlob* blob, int runIndex, GrAtlasGlyphCache* fontCache,
                                 const SkSurfaceProps& props, const GrTextUtils::Paint& paint,
                                 uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
                                 const char text[], size_t byteLength, const SkScalar pos[],
                                 int scalarsPerPosition, const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    // Ensure the blob is set for bitmaptext
    blob->setHasBitmap();

    GrAtlasTextStrike* currStrike = nullptr;

    SkGlyphCache* cache = blob->setupCache(runIndex, props, scalerContextFlags, paint, &viewMatrix);

    SkFindAndPlaceGlyph::ProcessPosText(
        paint.skPaint().getTextEncoding(), text, byteLength,
        offset, viewMatrix, pos, scalarsPerPosition,
        paint.skPaint().getTextAlign(), cache,
        [&](const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
            position += rounding;
            BmpAppendGlyph(
                blob, runIndex, fontCache, &currStrike, glyph,
                SkScalarFloorToInt(position.fX), SkScalarFloorToInt(position.fY),
                paint.filteredPremulColor(), cache);
        }
    );

    SkGlyphCache::AttachCache(cache);
}

void GrTextUtils::BmpAppendGlyph(GrAtlasTextBlob* blob, int runIndex,
                                 GrAtlasGlyphCache* fontCache,
                                 GrAtlasTextStrike** strike, const SkGlyph& skGlyph,
                                 int vx, int vy, GrColor color, SkGlyphCache* cache) {
    if (!*strike) {
        *strike = fontCache->getStrike(cache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kCoverage_MaskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, cache);
    if (!glyph) {
        return;
    }

    int x = vx + glyph->fBounds.fLeft;
    int y = vy + glyph->fBounds.fTop;

    // keep them as ints until we've done the clip-test
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();

    SkRect r;
    r.fLeft = SkIntToScalar(x);
    r.fTop = SkIntToScalar(y);
    r.fRight = r.fLeft + SkIntToScalar(width);
    r.fBottom = r.fTop + SkIntToScalar(height);

    blob->appendGlyph(runIndex, r, color, *strike, glyph, cache, skGlyph,
                      SkIntToScalar(vx), SkIntToScalar(vy), 1.0f, true);
}

bool GrTextUtils::CanDrawAsDistanceFields(const SkPaint& skPaint, const SkMatrix& viewMatrix,
                                          const SkSurfaceProps& props, const GrShaderCaps& caps) {
    if (!viewMatrix.hasPerspective()) {
        SkScalar maxScale = viewMatrix.getMaxScale();
        SkScalar scaledTextSize = maxScale * skPaint.getTextSize();
        // Hinted text looks far better at small resolutions
        // Scaling up beyond 2x yields undesireable artifacts
        if (scaledTextSize < kMinDFFontSize ||
            scaledTextSize > kLargeDFFontLimit) {
            return false;
        }

        bool useDFT = props.isUseDeviceIndependentFonts();
#if SK_FORCE_DISTANCE_FIELD_TEXT
        useDFT = true;
#endif

        if (!useDFT && scaledTextSize < kLargeDFFontSize) {
            return false;
        }
    }

    // rasterizers and mask filters modify alpha, which doesn't
    // translate well to distance
    if (skPaint.getRasterizer() || skPaint.getMaskFilter() || !caps.shaderDerivativeSupport()) {
        return false;
    }

    // TODO: add some stroking support
    if (skPaint.getStyle() != SkPaint::kFill_Style) {
        return false;
    }

    return true;
}

void GrTextUtils::InitDistanceFieldPaint(GrAtlasTextBlob* blob,
                                         SkPaint* skPaint,
                                         SkScalar* textRatio,
                                         const SkMatrix& viewMatrix) {
    SkScalar textSize = skPaint->getTextSize();
    SkScalar scaledTextSize = textSize;

    if (viewMatrix.hasPerspective()) {
        // for perspective, we simply force to the medium size
        // TODO: compute a size based on approximate screen area
        scaledTextSize = kMediumDFFontLimit;
    } else {
        SkScalar maxScale = viewMatrix.getMaxScale();
        // if we have non-unity scale, we need to choose our base text size
        // based on the SkPaint's text size multiplied by the max scale factor
        // TODO: do we need to do this if we're scaling down (i.e. maxScale < 1)?
        if (maxScale > 0 && !SkScalarNearlyEqual(maxScale, SK_Scalar1)) {
            scaledTextSize *= maxScale;
        }
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
    blob->setMinAndMaxScale(dfMaskScaleFloor / scaledTextSize, dfMaskScaleCeil / scaledTextSize);

    skPaint->setAntiAlias(true);
    skPaint->setLCDRenderText(false);
    skPaint->setAutohinted(false);
    skPaint->setHinting(SkPaint::kNormal_Hinting);
    skPaint->setSubpixelText(true);
}

void GrTextUtils::DrawDFText(GrAtlasTextBlob* blob, int runIndex,
                             GrAtlasGlyphCache* fontCache, const SkSurfaceProps& props,
                             const GrTextUtils::Paint& paint, uint32_t scalerContextFlags,
                             const SkMatrix& viewMatrix,
                             const char text[], size_t byteLength,
                             SkScalar x, SkScalar y) {
    SkASSERT(byteLength == 0 || text != nullptr);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    const SkPaint& skPaint = paint.skPaint();
    SkPaint::GlyphCacheProc glyphCacheProc = SkPaint::GetGlyphCacheProc(skPaint.getTextEncoding(),
                                                                        skPaint.isDevKernText(),
                                                                        true);
    SkAutoDescriptor desc;
    SkScalerContextEffects effects;
    // We apply the fake-gamma by altering the distance in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    skPaint.getScalerContextDescriptor(&effects, &desc, props, SkPaint::kNone_ScalerContextFlags,
                                       nullptr);
    SkGlyphCache* origPaintCache = SkGlyphCache::DetachCache(skPaint.getTypeface(), effects,
                                                             desc.getDesc());

    SkTArray<SkScalar> positions;

    const char* textPtr = text;
    SkScalar stopX = 0;
    SkScalar stopY = 0;
    SkScalar origin = 0;
    switch (skPaint.getTextAlign()) {
        case SkPaint::kRight_Align: origin = SK_Scalar1; break;
        case SkPaint::kCenter_Align: origin = SK_ScalarHalf; break;
        case SkPaint::kLeft_Align: origin = 0; break;
    }

    SkAutoKern autokern;
    const char* stop = text + byteLength;
    while (textPtr < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(origPaintCache, &textPtr);

        SkScalar width = SkFloatToScalar(glyph.fAdvanceX) + autokern.adjust(glyph);
        positions.push_back(stopX + origin * width);

        SkScalar height = SkFloatToScalar(glyph.fAdvanceY);
        positions.push_back(stopY + origin * height);

        stopX += width;
        stopY += height;
    }
    SkASSERT(textPtr == stop);

    SkGlyphCache::AttachCache(origPaintCache);

    // now adjust starting point depending on alignment
    SkScalar alignX = stopX;
    SkScalar alignY = stopY;
    if (skPaint.getTextAlign() == SkPaint::kCenter_Align) {
        alignX = SkScalarHalf(alignX);
        alignY = SkScalarHalf(alignY);
    } else if (skPaint.getTextAlign() == SkPaint::kLeft_Align) {
        alignX = 0;
        alignY = 0;
    }
    x -= alignX;
    y -= alignY;
    SkPoint offset = SkPoint::Make(x, y);

    DrawDFPosText(blob, runIndex, fontCache, props, paint, scalerContextFlags, viewMatrix, text,
                  byteLength, positions.begin(), 2, offset);
}

void GrTextUtils::DrawDFPosText(GrAtlasTextBlob* blob, int runIndex, GrAtlasGlyphCache* fontCache,
                                const SkSurfaceProps& props, const GrTextUtils::Paint& paint,
                                uint32_t scalerContextFlags, const SkMatrix& viewMatrix,
                                const char text[], size_t byteLength, const SkScalar pos[],
                                int scalarsPerPosition, const SkPoint& offset) {
    SkASSERT(byteLength == 0 || text != nullptr);
    SkASSERT(1 == scalarsPerPosition || 2 == scalarsPerPosition);

    // nothing to draw
    if (text == nullptr || byteLength == 0) {
        return;
    }

    SkTDArray<char> fallbackTxt;
    SkTDArray<SkScalar> fallbackPos;

    // Setup distance field paint and text ratio
    SkScalar textRatio;
    SkPaint dfPaint(paint);
    GrTextUtils::InitDistanceFieldPaint(blob, &dfPaint, &textRatio, viewMatrix);
    blob->setHasDistanceField();
    blob->setSubRunHasDistanceFields(runIndex, paint.skPaint().isLCDRenderText(),
                                     paint.skPaint().isAntiAlias());

    GrAtlasTextStrike* currStrike = nullptr;

    // We apply the fake-gamma by altering the distance in the shader, so we ignore the
    // passed-in scaler context flags. (It's only used when we fall-back to bitmap text).
    SkGlyphCache* cache = blob->setupCache(runIndex, props, SkPaint::kNone_ScalerContextFlags,
                                           dfPaint, nullptr);
    SkPaint::GlyphCacheProc glyphCacheProc = SkPaint::GetGlyphCacheProc(dfPaint.getTextEncoding(),
                                                                        dfPaint.isDevKernText(),
                                                                        true);

    const char* stop = text + byteLength;

    if (SkPaint::kLeft_Align == dfPaint.getTextAlign()) {
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                if (!DfAppendGlyph(blob, runIndex, fontCache, &currStrike, glyph, x, y,
                                   paint.filteredPremulColor(), cache, textRatio, viewMatrix)) {
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
        SkScalar alignMul = SkPaint::kCenter_Align == dfPaint.getTextAlign() ? SK_ScalarHalf
                                                                             : SK_Scalar1;
        while (text < stop) {
            const char* lastText = text;
            // the last 2 parameters are ignored
            const SkGlyph& glyph = glyphCacheProc(cache, &text);

            if (glyph.fWidth) {
                SkScalar x = offset.x() + pos[0];
                SkScalar y = offset.y() + (2 == scalarsPerPosition ? pos[1] : 0);

                SkScalar advanceX = SkFloatToScalar(glyph.fAdvanceX) * alignMul * textRatio;
                SkScalar advanceY = SkFloatToScalar(glyph.fAdvanceY) * alignMul * textRatio;

                if (!DfAppendGlyph(blob, runIndex, fontCache, &currStrike, glyph, x - advanceX,
                                   y - advanceY, paint.filteredPremulColor(), cache, textRatio,
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
        blob->initOverride(runIndex);
        GrTextUtils::DrawBmpPosText(blob, runIndex, fontCache, props, paint, scalerContextFlags,
                                    viewMatrix, fallbackTxt.begin(), fallbackTxt.count(),
                                    fallbackPos.begin(), scalarsPerPosition, offset);
    }
}

bool GrTextUtils::DfAppendGlyph(GrAtlasTextBlob* blob, int runIndex, GrAtlasGlyphCache* cache,
                                GrAtlasTextStrike** strike, const SkGlyph& skGlyph,
                                SkScalar sx, SkScalar sy, GrColor color,
                                SkGlyphCache* glyphCache,
                                SkScalar textRatio, const SkMatrix& viewMatrix) {
    if (!*strike) {
        *strike = cache->getStrike(glyphCache);
    }

    GrGlyph::PackedID id = GrGlyph::Pack(skGlyph.getGlyphID(),
                                         skGlyph.getSubXFixed(),
                                         skGlyph.getSubYFixed(),
                                         GrGlyph::kDistance_MaskStyle);
    GrGlyph* glyph = (*strike)->getGlyph(skGlyph, id, glyphCache);
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

    blob->appendGlyph(runIndex, glyphRect, color, *strike, glyph, glyphCache, skGlyph,
                      sx - dx, sy - dy, scale, false);
    return true;
}

void GrTextUtils::DrawTextAsPath(GrContext* context, GrRenderTargetContext* rtc, const GrClip& clip,
                                 const SkPaint& paint, const SkMatrix& viewMatrix,
                                 const char text[], size_t byteLength, SkScalar x, SkScalar y,
                                 const SkIRect& clipBounds) {
    SkTextToPathIter iter(text, byteLength, paint, true);

    SkMatrix    matrix;
    matrix.setScale(iter.getPathScale(), iter.getPathScale());
    matrix.postTranslate(x, y);

    const SkPath* iterPath;
    SkScalar xpos, prevXPos = 0;

    while (iter.next(&iterPath, &xpos)) {
        matrix.postTranslate(xpos - prevXPos, 0);
        if (iterPath) {
            const SkPaint& pnt = iter.getPaint();
            GrBlurUtils::drawPathWithMaskFilter(context, rtc, clip, *iterPath,
                                                pnt, viewMatrix, &matrix, clipBounds, false);
        }
        prevXPos = xpos;
    }
}

void GrTextUtils::DrawPosTextAsPath(GrContext* context,
                                    GrRenderTargetContext* rtc,
                                    const SkSurfaceProps& props,
                                    const GrClip& clip,
                                    const SkPaint& origPaint, const SkMatrix& viewMatrix,
                                    const char text[], size_t byteLength,
                                    const SkScalar pos[], int scalarsPerPosition,
                                    const SkPoint& offset, const SkIRect& clipBounds) {
    // setup our std paint, in hopes of getting hits in the cache
    SkPaint paint(origPaint);
    SkScalar matrixScale = paint.setupForAsPaths();

    SkMatrix matrix;
    matrix.setScale(matrixScale, matrixScale);

    // Temporarily jam in kFill, so we only ever ask for the raw outline from the cache.
    paint.setStyle(SkPaint::kFill_Style);
    paint.setPathEffect(nullptr);

    SkPaint::GlyphCacheProc    glyphCacheProc = SkPaint::GetGlyphCacheProc(paint.getTextEncoding(),
                                                                           paint.isDevKernText(),
                                                                           true);
    SkAutoGlyphCache           autoCache(paint, &props, nullptr);
    SkGlyphCache*              cache = autoCache.getCache();

    const char*        stop = text + byteLength;
    SkTextAlignProc    alignProc(paint.getTextAlign());
    SkTextMapStateProc tmsProc(SkMatrix::I(), offset, scalarsPerPosition);

    // Now restore the original settings, so we "draw" with whatever style/stroking.
    paint.setStyle(origPaint.getStyle());
    paint.setPathEffect(origPaint.refPathEffect());

    while (text < stop) {
        const SkGlyph& glyph = glyphCacheProc(cache, &text);
        if (glyph.fWidth) {
            const SkPath* path = cache->findPath(glyph);
            if (path) {
                SkPoint tmsLoc;
                tmsProc(pos, &tmsLoc);
                SkPoint loc;
                alignProc(tmsLoc, glyph, &loc);

                matrix[SkMatrix::kMTransX] = loc.fX;
                matrix[SkMatrix::kMTransY] = loc.fY;
                GrBlurUtils::drawPathWithMaskFilter(context, rtc, clip, *path, paint,
                                                    viewMatrix, &matrix, clipBounds, false);
            }
        }
        pos += scalarsPerPosition;
    }
}

bool GrTextUtils::ShouldDisableLCD(const SkPaint& paint) {
    return paint.getMaskFilter() ||
           paint.getRasterizer() ||
           paint.getPathEffect() ||
           paint.isFakeBoldText() ||
           paint.getStyle() != SkPaint::kFill_Style;
}

uint32_t GrTextUtils::FilterTextFlags(const SkSurfaceProps& surfaceProps, const SkPaint& paint) {
    uint32_t flags = paint.getFlags();

    if (!paint.isLCDRenderText() || !paint.isAntiAlias()) {
        return flags;
    }

    if (kUnknown_SkPixelGeometry == surfaceProps.pixelGeometry() || ShouldDisableLCD(paint)) {
        flags &= ~SkPaint::kLCDRenderText_Flag;
        flags |= SkPaint::kGenA8FromLCD_Flag;
    }

    return flags;
}
