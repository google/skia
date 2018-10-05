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
#include "SkGlyphCache.h"
#include "SkGraphics.h"
#include "SkPaintDefaults.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkSafeRange.h"
#include "SkScalar.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkShaderBase.h"
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

bool SkPaint::TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM, SkScalar maxLimit) {
    SkASSERT(!ctm.hasPerspective());
    SkASSERT(!textM.hasPerspective());

    SkMatrix matrix;
    matrix.setConcat(ctm, textM);
    return tooBig(matrix, MaxCacheSize2(maxLimit));
}

SkScalar SkPaint::MaxCacheSize2(SkScalar maxLimit) {
    // we have a self-imposed maximum, just for memory-usage sanity
    const int limit = SkMin32(SkGraphics::GetFontCachePointSizeLimit(), maxLimit);
    const SkScalar maxSize = SkIntToScalar(limit);
    return maxSize * maxSize;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGlyphCache.h"
#include "SkUtils.h"

int SkPaint::countText(const void* text, size_t byteLength) const {
    SkASSERT(text != nullptr);
    switch (this->getTextEncoding()) {
        case kUTF8_TextEncoding:
            return SkUTF::CountUTF8((const char*)text, byteLength);
        case kUTF16_TextEncoding:
            return SkUTF::CountUTF16((const uint16_t*)text, byteLength);
        case kUTF32_TextEncoding:
            return SkToInt(byteLength >> 2);
        case kGlyphID_TextEncoding:
            return SkToInt(byteLength >> 1);
        default:
            SkDEBUGFAIL("unknown text encoding");
    }

    return 0;
}

static SkTypeface::Encoding to_encoding(SkPaint::TextEncoding e) {
    static_assert((int)SkTypeface::kUTF8_Encoding  == (int)SkPaint::kUTF8_TextEncoding,  "");
    static_assert((int)SkTypeface::kUTF16_Encoding == (int)SkPaint::kUTF16_TextEncoding, "");
    static_assert((int)SkTypeface::kUTF32_Encoding == (int)SkPaint::kUTF32_TextEncoding, "");
    return (SkTypeface::Encoding)e;
}

int SkPaint::textToGlyphs(const void* textData, size_t byteLength, uint16_t glyphs[]) const {
    SkASSERT(textData != nullptr);

    if (nullptr == glyphs) {
        return this->countText(textData, byteLength);
    }

    // if we get here, we have a valid glyphs[] array, so time to fill it in

    // handle this encoding before the setup for the glyphcache
    if (this->getTextEncoding() == kGlyphID_TextEncoding) {
        // we want to ignore the low bit of byteLength
        memcpy(glyphs, textData, byteLength >> 1 << 1);
        return SkToInt(byteLength >> 1);
    }

    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(*this);

    const void* stop = (const char*)textData + byteLength;
    uint16_t*   gptr = glyphs;
    const SkTypeface::Encoding encoding = to_encoding(this->getTextEncoding());

    while (textData < stop) {
        SkUnichar unichar = SkUTFN_Next(encoding, &textData, stop);
        if (unichar < 0) {
            return 0;  // bad UTF-N sequence
        }
        *gptr++ = cache->unicharToGlyph(unichar);
    }
    return SkToInt(gptr - glyphs);
}

bool SkPaint::containsText(const void* textData, size_t byteLength) const {
    if (0 == byteLength) {
        return true;
    }

    SkASSERT(textData != nullptr);

    // handle this encoding before the setup for the glyphcache
    if (this->getTextEncoding() == kGlyphID_TextEncoding) {
        const uint16_t* glyphID = static_cast<const uint16_t*>(textData);
        size_t count = byteLength >> 1;
        for (size_t i = 0; i < count; i++) {
            if (0 == glyphID[i]) {
                return false;
            }
        }
        return true;
    }

    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(*this);
    const void* stop = (const char*)textData + byteLength;
    const SkTypeface::Encoding encoding = to_encoding(this->getTextEncoding());
    while (textData < stop) {
        if (0 == cache->unicharToGlyph(SkUTFN_Next(encoding, &textData, stop))) {
            return false;
        }
    }
    return true;
}

void SkPaint::glyphsToUnichars(const uint16_t glyphs[], int count, SkUnichar textData[]) const {
    if (count <= 0) {
        return;
    }

    SkASSERT(glyphs != nullptr);
    SkASSERT(textData != nullptr);

    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
            *this, &props, SkScalerContextFlags::kFakeGammaAndBoostContrast, nullptr);

    for (int index = 0; index < count; index++) {
        textData[index] = cache->glyphToUnichar(glyphs[index]);
    }
}

///////////////////////////////////////////////////////////////////////////////

static const SkGlyph& sk_getMetrics_utf8_next(SkGlyphCache* cache,
                                              const char** text,
                                              const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(SkUTF::NextUTF8(text, stop));
}

static const SkGlyph& sk_getMetrics_utf16_next(SkGlyphCache* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(
            SkUTF::NextUTF16((const uint16_t**)text, (const uint16_t*)stop));
}

static const SkGlyph& sk_getMetrics_utf32_next(SkGlyphCache* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(SkUTF::NextUTF32((const int32_t**)text, (const int32_t*)stop));
}

static const SkGlyph& sk_getMetrics_glyph_next(SkGlyphCache* cache,
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

static const SkGlyph& sk_getAdvance_utf8_next(SkGlyphCache* cache,
                                              const char** text,
                                              const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(SkUTF::NextUTF8(text, stop));
}

static const SkGlyph& sk_getAdvance_utf16_next(SkGlyphCache* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(
            SkUTF::NextUTF16((const uint16_t**)text, (const uint16_t*)stop));
}

static const SkGlyph& sk_getAdvance_utf32_next(SkGlyphCache* cache,
                                               const char** text,
                                               const char* stop) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(SkUTF::NextUTF32((const int32_t**)text, (const int32_t*)stop));
}

static const SkGlyph& sk_getAdvance_glyph_next(SkGlyphCache* cache,
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

SkPaint::GlyphCacheProc SkPaint::GetGlyphCacheProc(TextEncoding encoding,
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

    unsigned index = encoding;

    if (!needFullMetrics) {
        index += 4;
    }

    SkASSERT(index < SK_ARRAY_COUNT(gGlyphCacheProcs));
    return gGlyphCacheProcs[index];
}

///////////////////////////////////////////////////////////////////////////////

SkScalar SkPaint::setupForAsPaths() {

    constexpr uint32_t flagsToIgnore = SkPaint::kLinearText_Flag         |
                                       SkPaint::kLCDRenderText_Flag      |
                                       SkPaint::kEmbeddedBitmapText_Flag |
                                       SkPaint::kAutoHinting_Flag;

    uint32_t flags = this->getFlags();

    // clear the flags we don't care about
    flags &= ~flagsToIgnore;

    // set the flags we do care about
    flags |= SkPaint::kSubpixelText_Flag;

    this->setFlags(flags);
    this->setHinting(SkPaint::kNo_Hinting);
    this->setStyle(SkPaint::kFill_Style);
    this->setPathEffect(nullptr);

    SkScalar textSize = fTextSize;
    this->setTextSize(kCanonicalTextSizeForPaths);
    return textSize / kCanonicalTextSizeForPaths;
}

class SkCanonicalizePaint {
public:
    SkCanonicalizePaint(const SkPaint& paint) : fPaint(&paint), fScale(0) {
        if (paint.isLinearText() || SkDraw::ShouldDrawTextAsPaths(paint, SkMatrix::I())) {
            SkPaint* p = fLazy.set(paint);
            fScale = p->setupForAsPaths();
            fPaint = p;
        }
    }

    const SkPaint& getPaint() const { return *fPaint; }

    /**
     *  Returns 0 if the paint was unmodified, or the scale factor need to
     *  the original textSize
     */
    SkScalar getScale() const { return fScale; }

private:
    const SkPaint*   fPaint;
    SkScalar         fScale;
    SkTLazy<SkPaint> fLazy;
};

static void set_bounds(const SkGlyph& g, SkRect* bounds) {
    bounds->set(SkIntToScalar(g.fLeft),
                SkIntToScalar(g.fTop),
                SkIntToScalar(g.fLeft + g.fWidth),
                SkIntToScalar(g.fTop + g.fHeight));
}

static void join_bounds_x(const SkGlyph& g, SkRect* bounds, SkScalar dx) {
    bounds->join(SkIntToScalar(g.fLeft) + dx,
                 SkIntToScalar(g.fTop),
                 SkIntToScalar(g.fLeft + g.fWidth) + dx,
                 SkIntToScalar(g.fTop + g.fHeight));
}

static void join_bounds_y(const SkGlyph& g, SkRect* bounds, SkScalar dy) {
    bounds->join(SkIntToScalar(g.fLeft),
                 SkIntToScalar(g.fTop) + dy,
                 SkIntToScalar(g.fLeft + g.fWidth),
                 SkIntToScalar(g.fTop + g.fHeight) + dy);
}

typedef void (*JoinBoundsProc)(const SkGlyph&, SkRect*, SkScalar);

// xyIndex is 0 for fAdvanceX or 1 for fAdvanceY
static SkScalar advance(const SkGlyph& glyph, int xyIndex) {
    SkASSERT(0 == xyIndex || 1 == xyIndex);
    return SkFloatToScalar((&glyph.fAdvanceX)[xyIndex]);
}

SkScalar SkPaint::measure_text(SkGlyphCache* cache,
                               const char* text, size_t byteLength,
                               int* count, SkRect* bounds) const {
    SkASSERT(count);
    if (byteLength == 0) {
        *count = 0;
        if (bounds) {
            bounds->setEmpty();
        }
        return 0;
    }

    GlyphCacheProc glyphCacheProc = SkPaint::GetGlyphCacheProc(this->getTextEncoding(),
                                                               nullptr != bounds);

    int xyIndex;
    JoinBoundsProc joinBoundsProc;
    if (this->isVerticalText()) {
        xyIndex = 1;
        joinBoundsProc = join_bounds_y;
    } else {
        xyIndex = 0;
        joinBoundsProc = join_bounds_x;
    }

    int         n = 1;
    const char* stop = (const char*)text + byteLength;
    const SkGlyph* g = &glyphCacheProc(cache, &text, stop);
    SkScalar x = advance(*g, xyIndex);

    if (nullptr == bounds) {
        for (; text < stop; n++) {
            x += advance(glyphCacheProc(cache, &text, stop), xyIndex);
        }
    } else {
        set_bounds(*g, bounds);

        for (; text < stop; n++) {
            g = &glyphCacheProc(cache, &text, stop);
            joinBoundsProc(*g, bounds, x);
            x += advance(*g, xyIndex);
        }
    }
    SkASSERT(text == stop);

    *count = n;
    return x;
}

SkScalar SkPaint::measureText(const void* textData, size_t length, SkRect* bounds) const {
    const char* text = (const char*)textData;
    SkASSERT(text != nullptr || length == 0);

    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);

    SkScalar width = 0;

    if (length > 0) {
        int tempCount;

        width = paint.measure_text(cache.get(), text, length, &tempCount, bounds);
        if (scale) {
            width *= scale;
            if (bounds) {
                bounds->fLeft *= scale;
                bounds->fTop *= scale;
                bounds->fRight *= scale;
                bounds->fBottom *= scale;
            }
        }
    } else if (bounds) {
        // ensure that even if we don't measure_text we still update the bounds
        bounds->setEmpty();
    }
    return width;
}

size_t SkPaint::breakText(const void* textD, size_t length, SkScalar maxWidth,
                          SkScalar* measuredWidth) const {
    if (0 == length || 0 >= maxWidth) {
        if (measuredWidth) {
            *measuredWidth = 0;
        }
        return 0;
    }

    if (0 == fTextSize) {
        if (measuredWidth) {
            *measuredWidth = 0;
        }
        return length;
    }

    SkASSERT(textD != nullptr);
    const char* text = (const char*)textD;
    const char* stop = text + length;

    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    // adjust max in case we changed the textSize in paint
    if (scale) {
        maxWidth /= scale;
    }

    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);

    GlyphCacheProc   glyphCacheProc = SkPaint::GetGlyphCacheProc(paint.getTextEncoding(),
                                                                 false);
    const int        xyIndex = paint.isVerticalText() ? 1 : 0;
    SkScalar         width = 0;

    while (text < stop) {
        const char* curr = text;
        SkScalar x = advance(glyphCacheProc(cache.get(), &text, stop), xyIndex);
        if ((width += x) > maxWidth) {
            width -= x;
            text = curr;
            break;
        }
    }

    if (measuredWidth) {
        if (scale) {
            width *= scale;
        }
        *measuredWidth = width;
    }

    // return the number of bytes measured
    return text - stop + length;
}

///////////////////////////////////////////////////////////////////////////////

SkScalar SkPaint::getFontMetrics(FontMetrics* metrics, SkScalar zoom) const {
    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    SkMatrix zoomMatrix, *zoomPtr = nullptr;
    if (zoom) {
        zoomMatrix.setScale(zoom, zoom);
        zoomPtr = &zoomMatrix;
    }

    FontMetrics storage;
    if (nullptr == metrics) {
        metrics = &storage;
    }

    SkAutoDescriptor ad;
    SkScalerContextEffects effects;

    auto desc = SkScalerContext::CreateDescriptorAndEffectsUsingPaint(
        paint, nullptr, SkScalerContextFlags::kNone, zoomPtr, &ad, &effects);

    {
        auto typeface = SkPaintPriv::GetTypefaceOrDefault(paint);
        auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(*desc, effects, *typeface);
        *metrics = cache->getFontMetrics();
    }

    if (scale) {
        SkPaintPriv::ScaleFontMetrics(metrics, scale);
    }
    return metrics->fDescent - metrics->fAscent + metrics->fLeading;
}

///////////////////////////////////////////////////////////////////////////////

static void set_bounds(const SkGlyph& g, SkRect* bounds, SkScalar scale) {
    bounds->set(g.fLeft * scale,
                g.fTop * scale,
                (g.fLeft + g.fWidth) * scale,
                (g.fTop + g.fHeight) * scale);
}

int SkPaint::getTextWidths(const void* textData, size_t byteLength,
                           SkScalar widths[], SkRect bounds[]) const {
    if (0 == byteLength) {
        return 0;
    }

    SkASSERT(textData);

    if (nullptr == widths && nullptr == bounds) {
        return this->countText(textData, byteLength);
    }

    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(paint);
    GlyphCacheProc      glyphCacheProc = SkPaint::GetGlyphCacheProc(paint.getTextEncoding(),
                                                                    nullptr != bounds);

    const char* text = (const char*)textData;
    const char* stop = text + byteLength;
    int         count = 0;
    const int   xyIndex = paint.isVerticalText() ? 1 : 0;

    if (scale) {
        while (text < stop) {
            const SkGlyph& g = glyphCacheProc(cache.get(), &text, stop);
            if (widths) {
                *widths++ = advance(g, xyIndex) * scale;
            }
            if (bounds) {
                set_bounds(g, bounds++, scale);
            }
            ++count;
        }
    } else {
        while (text < stop) {
            const SkGlyph& g = glyphCacheProc(cache.get(), &text, stop);
            if (widths) {
                *widths++ = advance(g, xyIndex);
            }
            if (bounds) {
                set_bounds(g, bounds++);
            }
            ++count;
        }
    }

    SkASSERT(text == stop);
    return count;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkDraw.h"

void SkPaint::getTextPath(const void* textData, size_t length,
                          SkScalar x, SkScalar y, SkPath* path) const {
    SkASSERT(length == 0 || textData != nullptr);

    const char* text = (const char*)textData;
    if (text == nullptr || length == 0 || path == nullptr) {
        return;
    }

    SkTextToPathIter    iter(text, length, *this, false);
    SkMatrix            matrix;
    SkScalar            prevXPos = 0;

    matrix.setScale(iter.getPathScale(), iter.getPathScale());
    matrix.postTranslate(x, y);
    path->reset();

    SkScalar        xpos;
    const SkPath*   iterPath;
    while (iter.next(&iterPath, &xpos)) {
        matrix.postTranslate(xpos - prevXPos, 0);
        if (iterPath) {
            path->addPath(*iterPath, matrix);
        }
        prevXPos = xpos;
    }
}

void SkPaint::getPosTextPath(const void* textData, size_t length,
                             const SkPoint pos[], SkPath* path) const {
    SkASSERT(length == 0 || textData != nullptr);

    const char* text = (const char*)textData;
    if (text == nullptr || length == 0 || path == nullptr) {
        return;
    }

    SkTextToPathIter    iter(text, length, *this, false);
    SkMatrix            matrix;
    SkPoint             prevPos;
    prevPos.set(0, 0);

    matrix.setScale(iter.getPathScale(), iter.getPathScale());
    path->reset();

    unsigned int    i = 0;
    const SkPath*   iterPath;
    while (iter.next(&iterPath, nullptr)) {
        matrix.postTranslate(pos[i].fX - prevPos.fX, pos[i].fY - prevPos.fY);
        if (iterPath) {
            path->addPath(*iterPath, matrix);
        }
        prevPos = pos[i];
        i++;
    }
}

template <SkTextInterceptsIter::TextType TextType, typename Func>
int GetTextIntercepts(const SkPaint& paint, const void* text, size_t length,
                      const SkScalar bounds[2], SkScalar* array, Func posMaker) {
    SkASSERT(length == 0 || text != nullptr);
    if (!length) {
        return 0;
    }

    const SkPoint pos0 = posMaker(0);
    SkTextInterceptsIter iter(static_cast<const char*>(text), length, paint, bounds,
                              pos0.x(), pos0.y(), TextType);

    int i = 0;
    int count = 0;
    while (iter.next(array, &count)) {
        if (TextType == SkTextInterceptsIter::TextType::kPosText) {
            const SkPoint pos = posMaker(++i);
            iter.setPosition(pos.x(), pos.y());
        }
    }

    return count;
}

int SkPaint::getTextIntercepts(const void* textData, size_t length,
                               SkScalar x, SkScalar y, const SkScalar bounds[2],
                               SkScalar* array) const {

    return GetTextIntercepts<SkTextInterceptsIter::TextType::kText>(
        *this, textData, length, bounds, array, [&x, &y] (int) -> SkPoint {
            return SkPoint::Make(x, y);
        });
}

int SkPaint::getPosTextIntercepts(const void* textData, size_t length, const SkPoint pos[],
                                  const SkScalar bounds[2], SkScalar* array) const {

    return GetTextIntercepts<SkTextInterceptsIter::TextType::kPosText>(
        *this, textData, length, bounds, array, [&pos] (int i) -> SkPoint {
            return pos[i];
        });
}

int SkPaint::getPosTextHIntercepts(const void* textData, size_t length, const SkScalar xpos[],
                                   SkScalar constY, const SkScalar bounds[2],
                                   SkScalar* array) const {

    return GetTextIntercepts<SkTextInterceptsIter::TextType::kPosText>(
        *this, textData, length, bounds, array, [&xpos, &constY] (int i) -> SkPoint {
            return SkPoint::Make(xpos[i], constY);
        });
}

int SkPaint::getTextBlobIntercepts(const SkTextBlob* blob, const SkScalar bounds[2],
                                   SkScalar* intervals) const {
    int count = 0;
    SkPaint runPaint(*this);

    SkTextBlobRunIterator it(blob);
    while (!it.done()) {
        it.applyFontToPaint(&runPaint);
        const size_t runByteCount = it.glyphCount() * sizeof(SkGlyphID);
        SkScalar* runIntervals = intervals ? intervals + count : nullptr;

        switch (it.positioning()) {
        case SkTextBlobRunIterator::kDefault_Positioning:
            count += runPaint.getTextIntercepts(it.glyphs(), runByteCount, it.offset().x(),
                                                it.offset().y(), bounds, runIntervals);
            break;
        case SkTextBlobRunIterator::kHorizontal_Positioning:
            count += runPaint.getPosTextHIntercepts(it.glyphs(), runByteCount, it.pos(),
                                                    it.offset().y(), bounds, runIntervals);
            break;
        case SkTextBlobRunIterator::kFull_Positioning:
            count += runPaint.getPosTextIntercepts(it.glyphs(), runByteCount,
                                                   reinterpret_cast<const SkPoint*>(it.pos()),
                                                   bounds, runIntervals);
            break;
        }

        it.next();
    }

    return count;
}

SkRect SkPaint::getFontBounds() const {
    SkMatrix m;
    m.setScale(fTextSize * fTextScaleX, fTextSize);
    m.postSkew(fTextSkewX, 0);

    SkTypeface* typeface = SkPaintPriv::GetTypefaceOrDefault(*this);

    SkRect bounds;
    m.mapRect(&bounds, typeface->getBounds());
    return bounds;
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

SkTextBaseIter::SkTextBaseIter(const char text[], size_t length,
                                   const SkPaint& paint,
                                   bool applyStrokeAndPathEffects)
    : fPaint(paint) {
    fGlyphCacheProc = SkPaint::GetGlyphCacheProc(paint.getTextEncoding(), true);

    fPaint.setLinearText(true);
    fPaint.setMaskFilter(nullptr);   // don't want this affecting our path-cache lookup

    if (fPaint.getPathEffect() == nullptr && !has_thick_frame(fPaint)) {
        applyStrokeAndPathEffects = false;
    }

    // can't use our canonical size if we need to apply patheffects
    if (fPaint.getPathEffect() == nullptr) {
        fPaint.setTextSize(SkIntToScalar(SkPaint::kCanonicalTextSizeForPaths));
        fScale = paint.getTextSize() / SkPaint::kCanonicalTextSizeForPaths;
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

    if (!applyStrokeAndPathEffects) {
        fPaint.setStyle(SkPaint::kFill_Style);
        fPaint.setPathEffect(nullptr);
    }

    // SRGBTODO: Is this correct?
    fCache = SkStrikeCache::FindOrCreateStrikeExclusive(
        fPaint, nullptr,
        SkScalerContextFlags::kFakeGammaAndBoostContrast, nullptr);

    SkPaint::Style  style = SkPaint::kFill_Style;
    sk_sp<SkPathEffect> pe;

    if (!applyStrokeAndPathEffects) {
        style = paint.getStyle();       // restore
        pe = paint.refPathEffect();     // restore
    }
    fPaint.setStyle(style);
    fPaint.setPathEffect(pe);
    fPaint.setMaskFilter(paint.refMaskFilter());    // restore

    // now compute fXOffset if needed

    SkScalar xOffset = 0;
    if (paint.getTextAlign() != SkPaint::kLeft_Align) { // need to measure first
        int      count;
        SkScalar width = fPaint.measure_text(fCache.get(), text, length, &count, nullptr) * fScale;
        if (paint.getTextAlign() == SkPaint::kCenter_Align) {
            width = SkScalarHalf(width);
        }
        xOffset = -width;
    }
    fXPos = xOffset;
    fPrevAdvance = 0;

    fText = text;
    fStop = text + length;

    fXYIndex = paint.isVerticalText() ? 1 : 0;
}

bool SkTextToPathIter::next(const SkPath** path, SkScalar* xpos) {
    if (fText < fStop) {
        const SkGlyph& glyph = fGlyphCacheProc(fCache.get(), &fText, fStop);

        fXPos += fPrevAdvance * fScale;
        fPrevAdvance = advance(glyph, fXYIndex);   // + fPaint.getTextTracking();

        if (glyph.fWidth) {
            if (path) {
                *path = fCache->findPath(glyph);
            }
        } else {
            if (path) {
                *path = nullptr;
            }
        }
        if (xpos) {
            *xpos = fXPos;
        }
        return true;
    }
    return false;
}

bool SkTextInterceptsIter::next(SkScalar* array, int* count) {
    const SkGlyph& glyph = fGlyphCacheProc(fCache.get(), &fText, fStop);
    fXPos += fPrevAdvance * fScale;
    fPrevAdvance = advance(glyph, fXYIndex);   // + fPaint.getTextTracking();
    if (fCache->findPath(glyph)) {
        fCache->findIntercepts(fBounds, fScale, fXPos, SkToBool(fXYIndex),
                const_cast<SkGlyph*>(&glyph), array, count);
    }
    return fText < fStop;
}
