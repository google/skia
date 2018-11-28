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

int SkPaint::countText(const void* text, size_t length) const {
    return SkFont::LEGACY_ExtractFromPaint(*this).countText(text, length,
                                                        (SkTextEncoding)this->getTextEncoding());
}

int SkPaint::textToGlyphs(const void* text, size_t length, uint16_t glyphs[]) const {
    return SkFont::LEGACY_ExtractFromPaint(*this).textToGlyphs(text, length,
                                                           (SkTextEncoding)this->getTextEncoding(),
                                                               glyphs, length);
}

bool SkPaint::containsText(const void* text, size_t length) const {
    return SkFont::LEGACY_ExtractFromPaint(*this).containsText(text, length,
                                                           (SkTextEncoding)this->getTextEncoding());
}

void SkPaint::glyphsToUnichars(const uint16_t glyphs[], int count, SkUnichar textData[]) const {
    if (count <= 0) {
        return;
    }

    SkASSERT(glyphs != nullptr);
    SkASSERT(textData != nullptr);

    SkFont font = SkFont::LEGACY_ExtractFromPaint(*this);
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
            font, *this, props, SkScalerContextFlags::kFakeGammaAndBoostContrast, SkMatrix::I());

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
    this->setHinting(kNo_SkFontHinting);
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

// xyIndex is 0 for fAdvanceX or 1 for fAdvanceY
static SkScalar advance(const SkGlyph& glyph) {
    return SkFloatToScalar(glyph.fAdvanceX);
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

    SkFontPriv::GlyphCacheProc glyphCacheProc = SkFontPriv::GetGlyphCacheProc(
                    static_cast<SkTextEncoding>(this->getTextEncoding()), nullptr != bounds);

    int         n = 1;
    const char* stop = (const char*)text + byteLength;
    const SkGlyph* g = &glyphCacheProc(cache, &text, stop);
    SkScalar x = advance(*g);

    if (nullptr == bounds) {
        for (; text < stop; n++) {
            x += advance(glyphCacheProc(cache, &text, stop));
        }
    } else {
        set_bounds(*g, bounds);

        for (; text < stop; n++) {
            g = &glyphCacheProc(cache, &text, stop);
            join_bounds_x(*g, bounds, x);
            x += advance(*g);
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

    const SkFont font = SkFont::LEGACY_ExtractFromPaint(paint);
    auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font, paint);

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

    const SkFont font = SkFont::LEGACY_ExtractFromPaint(paint);
    auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font, paint);

    SkFontPriv::GlyphCacheProc glyphCacheProc = SkFontPriv::GetGlyphCacheProc(
                                  static_cast<SkTextEncoding>(paint.getTextEncoding()), false);
    SkScalar width = 0;

    while (text < stop) {
        const char* curr = text;
        SkScalar x = advance(glyphCacheProc(cache.get(), &text, stop));
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

SkScalar SkPaint::getFontMetrics(SkFontMetrics* metrics) const {
    return SkFont::LEGACY_ExtractFromPaint(*this).getMetrics(metrics);
}

///////////////////////////////////////////////////////////////////////////////

int SkPaint::getTextWidths(const void* text, size_t len, SkScalar widths[], SkRect bounds[]) const {
    const SkFont font = SkFont::LEGACY_ExtractFromPaint(*this);
    SkAutoToGlyphs gly(font, text, len, (SkTextEncoding)this->getTextEncoding());
    font.getWidthsBounds(gly.glyphs(), gly.count(), widths, bounds, this);
    return gly.count();
}

///////////////////////////////////////////////////////////////////////////////

#include "SkDraw.h"

struct PathPosRec {
    SkPath*         fDst;
    const SkPoint*  fPos;
};
static void PathPosProc(const SkPath* src, const SkMatrix& mx, void* ctx) {
    PathPosRec* rec = static_cast<PathPosRec*>(ctx);
    if (src) {
        SkMatrix m(mx);
        m.postTranslate(rec->fPos->fX, rec->fPos->fY);
        rec->fDst->addPath(*src, m);
    }
    rec->fPos += 1;
}

void SkPaint::getTextPath(const void* text, size_t length,
                          SkScalar x, SkScalar y, SkPath* path) const {
    SkFont font = SkFont::LEGACY_ExtractFromPaint(*this);
    SkAutoToGlyphs gly(font, text, length, (SkTextEncoding)this->getTextEncoding());
    SkAutoSTArray<32, SkPoint> fPos(gly.count());
    font.getPos(gly.glyphs(), gly.count(), fPos.get(), {x, y});

    path->reset();
    PathPosRec rec = { path, fPos.get() };
    font.getPaths(gly.glyphs(), gly.count(), PathPosProc, &rec);
}

void SkPaint::getPosTextPath(const void* text, size_t length,
                             const SkPoint pos[], SkPath* path) const {
    SkFont font = SkFont::LEGACY_ExtractFromPaint(*this);
    SkAutoToGlyphs gly(font, text, length, (SkTextEncoding)this->getTextEncoding());

    path->reset();
    PathPosRec rec = { path, pos };
    font.getPaths(gly.glyphs(), gly.count(), PathPosProc, &rec);
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
    fGlyphCacheProc = SkFontPriv::GetGlyphCacheProc(
                                    static_cast<SkTextEncoding>(paint.getTextEncoding()), true);

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
    const SkFont font = SkFont::LEGACY_ExtractFromPaint(fPaint);
    fCache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font, fPaint);

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
    fXPos = xOffset;
    fPrevAdvance = 0;

    fText = text;
    fStop = text + length;
}

bool SkTextToPathIter::next(const SkPath** path, SkScalar* xpos) {
    if (fText < fStop) {
        const SkGlyph& glyph = fGlyphCacheProc(fCache.get(), &fText, fStop);

        fXPos += fPrevAdvance * fScale;
        fPrevAdvance = advance(glyph);   // + fPaint.getTextTracking();

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
    fPrevAdvance = advance(glyph);   // + fPaint.getTextTracking();
    if (fCache->findPath(glyph)) {
        fCache->findIntercepts(fBounds, fScale, fXPos, false,
                const_cast<SkGlyph*>(&glyph), array, count);
    }
    return fText < fStop;
}
