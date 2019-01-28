/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkColorFilter.h"
#include "SkDraw.h"
#include "SkFontDescriptor.h"
#include "SkGraphics.h"
#include "SkPaintDefaults.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkSafeRange.h"
#include "SkScalar.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkShaderBase.h"
#include "SkStrike.h"
#include "SkStringUtils.h"
#include "SkTLazy.h"
#include "SkTextBlob.h"
#include "SkTextBlobPriv.h"
#include "SkTextFormatParams.h"
#include "SkTextToPathIter.h"
#include "SkTo.h"
#include "SkTypeface.h"

///////////////////////////////////////////////////////////////////////////////

static SkScalar mag2(SkScalar x, SkScalar y) {
    return x * x + y * y;
}

static bool tooBig(const SkMatrix& m, SkScalar ma2max) {
    return  mag2(m[SkMatrix::kMScaleX], m[SkMatrix::kMSkewY]) > ma2max
            ||
            mag2(m[SkMatrix::kMSkewX], m[SkMatrix::kMScaleY]) > ma2max;
}

bool SkFontPriv::TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM, SkScalar maxLimit) {
    SkASSERT(!ctm.hasPerspective());
    SkASSERT(!textM.hasPerspective());

    SkMatrix matrix;
    matrix.setConcat(ctm, textM);
    return tooBig(matrix, MaxCacheSize2(maxLimit));
}

SkScalar SkFontPriv::MaxCacheSize2(SkScalar maxLimit) {
    // we have a self-imposed maximum, just for memory-usage sanity
    const int limit = SkMin32(SkGraphics::GetFontCachePointSizeLimit(), maxLimit);
    const SkScalar maxSize = SkIntToScalar(limit);
    return maxSize * maxSize;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkUtils.h"

static const SkGlyph& sk_getMetrics_utf8_next(SkStrike* cache,
                                              const char** text,
                                              const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(SkUTF::NextUTF8(text, stop));
}

static const SkGlyph& sk_getMetrics_utf16_next(SkStrike* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(
            SkUTF::NextUTF16((const uint16_t**)text, (const uint16_t*)stop));
}

static const SkGlyph& sk_getMetrics_utf32_next(SkStrike* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(SkUTF::NextUTF32((const int32_t**)text, (const int32_t*)stop));
}

static const SkGlyph& sk_getMetrics_glyph_next(SkStrike* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    return cache->getGlyphIDMetrics(glyphID);
}

static const SkGlyph& sk_getAdvance_utf8_next(SkStrike* cache,
                                              const char** text,
                                              const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(SkUTF::NextUTF8(text, stop));
}

static const SkGlyph& sk_getAdvance_utf16_next(SkStrike* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(
            SkUTF::NextUTF16((const uint16_t**)text, (const uint16_t*)stop));
}

static const SkGlyph& sk_getAdvance_utf32_next(SkStrike* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(SkUTF::NextUTF32((const int32_t**)text, (const int32_t*)stop));
}

static const SkGlyph& sk_getAdvance_glyph_next(SkStrike* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    return cache->getGlyphIDAdvance(glyphID);
}

SkFontPriv::GlyphCacheProc SkFontPriv::GetGlyphCacheProc(SkTextEncoding encoding,
                                                         bool needFullMetrics) {
    static const GlyphCacheProc gGlyphCacheProcs[] = {
        sk_getMetrics_utf8_next,
        sk_getMetrics_utf16_next,
        sk_getMetrics_utf32_next,
        sk_getMetrics_glyph_next,

        sk_getAdvance_utf8_next,
        sk_getAdvance_utf16_next,
        sk_getAdvance_utf32_next,
        sk_getAdvance_glyph_next,
    };

    unsigned index = static_cast<unsigned>(encoding);

    if (!needFullMetrics) {
        index += 4;
    }

    SkASSERT(index < SK_ARRAY_COUNT(gGlyphCacheProcs));
    return gGlyphCacheProcs[index];
}

///////////////////////////////////////////////////////////////////////////////

// xyIndex is 0 for fAdvanceX or 1 for fAdvanceY
static SkScalar advance(const SkGlyph& glyph) {
    return SkFloatToScalar(glyph.fAdvanceX);
}

///////////////////////////////////////////////////////////////////////////////

int SkPaint::getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                                   SkScalar* intervals) const {
    return blob->getIntercepts(bounds, intervals, this);
}

// return true if the paint is just a single color (i.e. not a shader). If its
// a shader, then we can't compute a const luminance for it :(
static bool just_a_color(const SkPaint& paint, SkColor* color) {
    SkColor c = paint.getColor();

    const auto* shader = as_SB(paint.getShader());
    if (shader && !shader->asLuminanceColor(&c)) {
        return false;
    }
    if (paint.getColorFilter()) {
        c = paint.getColorFilter()->filterColor(c);
    }
    if (color) {
        *color = c;
    }
    return true;
}

SkColor SkPaint::computeLuminanceColor() const {
    SkColor c;
    if (!just_a_color(*this, &c)) {
        c = SkColorSetRGB(0x7F, 0x80, 0x7F);
    }
    return c;
}

///////////////////////////////////////////////////////////////////////////////

static bool has_thick_frame(const SkPaint& paint) {
    return  paint.getStrokeWidth() > 0 &&
            paint.getStyle() != SkPaint::kFill_Style;
}

SkTextBaseIter::SkTextBaseIter(const SkGlyphID glyphs[], int count, const SkFont& font,
                               const SkPaint* paint)
        : fFont(font)
{
    SkAssertResult(count >= 0);

    fFont.setLinearMetrics(true);

    if (paint) {
        fPaint = *paint;
    }
    fPaint.setMaskFilter(nullptr);   // don't want this affecting our path-cache lookup

    // can't use our canonical size if we need to apply patheffects
    if (fPaint.getPathEffect() == nullptr) {
        fScale = fFont.getSize() / SkFontPriv::kCanonicalTextSizeForPaths;
        fFont.setSize(SkIntToScalar(SkFontPriv::kCanonicalTextSizeForPaths));
        // Note: fScale can be zero here (even if it wasn't before the divide). It can also
        // be very very small. We call sk_ieee_float_divide below to ensure IEEE divide behavior,
        // since downstream we will check for the resulting coordinates being non-finite anyway.
        // Thus we don't need to check for zero here.
        if (has_thick_frame(fPaint)) {
            fPaint.setStrokeWidth(sk_ieee_float_divide(fPaint.getStrokeWidth(), fScale));
        }
    } else {
        fScale = SK_Scalar1;
    }

    SkPaint::Style prevStyle = fPaint.getStyle();
    auto prevPE = fPaint.refPathEffect();
    auto prevMF = fPaint.refMaskFilter();
    fPaint.setStyle(SkPaint::kFill_Style);
    fPaint.setPathEffect(nullptr);

    fCache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(fFont, fPaint);

    fPaint.setStyle(prevStyle);
    fPaint.setPathEffect(std::move(prevPE));
    fPaint.setMaskFilter(std::move(prevMF));

    // now compute fXOffset if needed

    SkScalar xOffset = 0;
    fXPos = xOffset;
    fPrevAdvance = 0;

    fGlyphs = glyphs;
    fStop = glyphs + count;
}

bool SkTextInterceptsIter::next(SkScalar* array, int* count) {
    SkASSERT(fGlyphs < fStop);
    const SkGlyph& glyph = fCache->getGlyphIDMetrics(*fGlyphs++);
    fXPos += fPrevAdvance * fScale;
    fPrevAdvance = advance(glyph);   // + fPaint.getTextTracking();
    if (fCache->findPath(glyph)) {
        fCache->findIntercepts(fBounds, fScale, fXPos, false,
                const_cast<SkGlyph*>(&glyph), array, count);
    }
    return fGlyphs < fStop;
}
