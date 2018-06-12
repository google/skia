/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"

#include "SkColorFilter.h"
#include "SkData.h"
#include "SkDraw.h"
#include "SkFontDescriptor.h"
#include "SkGlyphCache.h"
#include "SkGraphics.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkMaskGamma.h"
#include "SkMutex.h"
#include "SkOpts.h"
#include "SkPaintDefaults.h"
#include "SkPaintPriv.h"
#include "SkPathEffect.h"
#include "SkReadBuffer.h"
#include "SkSafeRange.h"
#include "SkScalar.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkShaderBase.h"
#include "SkStringUtils.h"
#include "SkStroke.h"
#include "SkStrokeRec.h"
#include "SkSurfacePriv.h"
#include "SkTLazy.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"
#include "SkTextFormatParams.h"
#include "SkTextToPathIter.h"
#include "SkTo.h"
#include "SkTypeface.h"
#include "SkWriteBuffer.h"

static inline uint32_t set_clear_mask(uint32_t bits, bool cond, uint32_t mask) {
    return cond ? bits | mask : bits & ~mask;
}

// define this to get a printf for out-of-range parameter in setters
// e.g. setTextSize(-1)
//#define SK_REPORT_API_RANGE_CHECK

SkPaint::SkPaint() {
    fTextSize   = SkPaintDefaults_TextSize;
    fTextScaleX = SK_Scalar1;
    fTextSkewX  = 0;
    fColor      = SK_ColorBLACK;
    fWidth      = 0;
    fMiterLimit = SkPaintDefaults_MiterLimit;
    fBlendMode  = (unsigned)SkBlendMode::kSrcOver;

    // Zero all bitfields, then set some non-zero defaults.
    fBitfieldsUInt           = 0;
    fBitfields.fFlags        = SkPaintDefaults_Flags;
    fBitfields.fCapType      = kDefault_Cap;
    fBitfields.fJoinType     = kDefault_Join;
    fBitfields.fTextAlign    = kLeft_Align;
    fBitfields.fStyle        = kFill_Style;
    fBitfields.fTextEncoding = kUTF8_TextEncoding;
    fBitfields.fHinting      = SkPaintDefaults_Hinting;
}

SkPaint::SkPaint(const SkPaint& src)
#define COPY(field) field(src.field)
    : COPY(fTypeface)
    , COPY(fPathEffect)
    , COPY(fShader)
    , COPY(fMaskFilter)
    , COPY(fColorFilter)
    , COPY(fDrawLooper)
    , COPY(fImageFilter)
    , COPY(fTextSize)
    , COPY(fTextScaleX)
    , COPY(fTextSkewX)
    , COPY(fColor)
    , COPY(fWidth)
    , COPY(fMiterLimit)
    , COPY(fBlendMode)
    , COPY(fBitfields)
#undef COPY
{}

SkPaint::SkPaint(SkPaint&& src) {
#define MOVE(field) field = std::move(src.field)
    MOVE(fTypeface);
    MOVE(fPathEffect);
    MOVE(fShader);
    MOVE(fMaskFilter);
    MOVE(fColorFilter);
    MOVE(fDrawLooper);
    MOVE(fImageFilter);
    MOVE(fTextSize);
    MOVE(fTextScaleX);
    MOVE(fTextSkewX);
    MOVE(fColor);
    MOVE(fWidth);
    MOVE(fMiterLimit);
    MOVE(fBlendMode);
    MOVE(fBitfields);
#undef MOVE
}

SkPaint::~SkPaint() {}

SkPaint& SkPaint::operator=(const SkPaint& src) {
    if (this == &src) {
        return *this;
    }

#define ASSIGN(field) field = src.field
    ASSIGN(fTypeface);
    ASSIGN(fPathEffect);
    ASSIGN(fShader);
    ASSIGN(fMaskFilter);
    ASSIGN(fColorFilter);
    ASSIGN(fDrawLooper);
    ASSIGN(fImageFilter);
    ASSIGN(fTextSize);
    ASSIGN(fTextScaleX);
    ASSIGN(fTextSkewX);
    ASSIGN(fColor);
    ASSIGN(fWidth);
    ASSIGN(fMiterLimit);
    ASSIGN(fBlendMode);
    ASSIGN(fBitfields);
#undef ASSIGN

    return *this;
}

SkPaint& SkPaint::operator=(SkPaint&& src) {
    if (this == &src) {
        return *this;
    }

#define MOVE(field) field = std::move(src.field)
    MOVE(fTypeface);
    MOVE(fPathEffect);
    MOVE(fShader);
    MOVE(fMaskFilter);
    MOVE(fColorFilter);
    MOVE(fDrawLooper);
    MOVE(fImageFilter);
    MOVE(fTextSize);
    MOVE(fTextScaleX);
    MOVE(fTextSkewX);
    MOVE(fColor);
    MOVE(fWidth);
    MOVE(fMiterLimit);
    MOVE(fBlendMode);
    MOVE(fBitfields);
#undef MOVE

    return *this;
}

bool operator==(const SkPaint& a, const SkPaint& b) {
#define EQUAL(field) (a.field == b.field)
    return EQUAL(fTypeface)
        && EQUAL(fPathEffect)
        && EQUAL(fShader)
        && EQUAL(fMaskFilter)
        && EQUAL(fColorFilter)
        && EQUAL(fDrawLooper)
        && EQUAL(fImageFilter)
        && EQUAL(fTextSize)
        && EQUAL(fTextScaleX)
        && EQUAL(fTextSkewX)
        && EQUAL(fColor)
        && EQUAL(fWidth)
        && EQUAL(fMiterLimit)
        && EQUAL(fBlendMode)
        && EQUAL(fBitfieldsUInt)
        ;
#undef EQUAL
}

#define DEFINE_REF_FOO(type)    sk_sp<Sk##type> SkPaint::ref##type() const { return f##type; }
DEFINE_REF_FOO(ColorFilter)
DEFINE_REF_FOO(DrawLooper)
DEFINE_REF_FOO(ImageFilter)
DEFINE_REF_FOO(MaskFilter)
DEFINE_REF_FOO(PathEffect)
DEFINE_REF_FOO(Shader)
DEFINE_REF_FOO(Typeface)
#undef DEFINE_REF_FOO

void SkPaint::reset() {
    SkPaint init;
    *this = init;
}

void SkPaint::setFilterQuality(SkFilterQuality quality) {
    fBitfields.fFilterQuality = quality;
}

void SkPaint::setHinting(Hinting hintingLevel) {
    fBitfields.fHinting = hintingLevel;
}

void SkPaint::setFlags(uint32_t flags) {
    fBitfields.fFlags = flags;
}

void SkPaint::setAntiAlias(bool doAA) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doAA, kAntiAlias_Flag));
}

void SkPaint::setDither(bool doDither) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doDither, kDither_Flag));
}

void SkPaint::setSubpixelText(bool doSubpixel) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doSubpixel, kSubpixelText_Flag));
}

void SkPaint::setLCDRenderText(bool doLCDRender) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doLCDRender, kLCDRenderText_Flag));
}

void SkPaint::setEmbeddedBitmapText(bool doEmbeddedBitmapText) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doEmbeddedBitmapText, kEmbeddedBitmapText_Flag));
}

void SkPaint::setAutohinted(bool useAutohinter) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, useAutohinter, kAutoHinting_Flag));
}

void SkPaint::setLinearText(bool doLinearText) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doLinearText, kLinearText_Flag));
}

void SkPaint::setVerticalText(bool doVertical) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doVertical, kVerticalText_Flag));
}

void SkPaint::setFakeBoldText(bool doFakeBold) {
    this->setFlags(set_clear_mask(fBitfields.fFlags, doFakeBold, kFakeBoldText_Flag));
}

void SkPaint::setStyle(Style style) {
    if ((unsigned)style < kStyleCount) {
        fBitfields.fStyle = style;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStyle(%d) out of range\n", style);
#endif
    }
}

void SkPaint::setColor(SkColor color) {
    fColor = color;
}

void SkPaint::setAlpha(U8CPU a) {
    this->setColor(SkColorSetARGB(a, SkColorGetR(fColor),
                                  SkColorGetG(fColor), SkColorGetB(fColor)));
}

void SkPaint::setARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    this->setColor(SkColorSetARGB(a, r, g, b));
}

void SkPaint::setStrokeWidth(SkScalar width) {
    if (width >= 0) {
        fWidth = width;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeWidth() called with negative value\n");
#endif
    }
}

void SkPaint::setStrokeMiter(SkScalar limit) {
    if (limit >= 0) {
        fMiterLimit = limit;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeMiter() called with negative value\n");
#endif
    }
}

void SkPaint::setStrokeCap(Cap ct) {
    if ((unsigned)ct < kCapCount) {
        fBitfields.fCapType = SkToU8(ct);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeCap(%d) out of range\n", ct);
#endif
    }
}

void SkPaint::setStrokeJoin(Join jt) {
    if ((unsigned)jt < kJoinCount) {
        fBitfields.fJoinType = SkToU8(jt);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeJoin(%d) out of range\n", jt);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkPaint::setTextAlign(Align align) {
    if ((unsigned)align < kAlignCount) {
        fBitfields.fTextAlign = SkToU8(align);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setTextAlign(%d) out of range\n", align);
#endif
    }
}

void SkPaint::setTextSize(SkScalar ts) {
    if (ts >= 0) {
        fTextSize = ts;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setTextSize() called with negative value\n");
#endif
    }
}

void SkPaint::setTextScaleX(SkScalar scaleX) {
    fTextScaleX = scaleX;
}

void SkPaint::setTextSkewX(SkScalar skewX) {
    fTextSkewX = skewX;
}

void SkPaint::setTextEncoding(TextEncoding encoding) {
    if ((unsigned)encoding <= kGlyphID_TextEncoding) {
        fBitfields.fTextEncoding = encoding;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setTextEncoding(%d) out of range\n", encoding);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////

#define MOVE_FIELD(Field) void SkPaint::set##Field(sk_sp<Sk##Field> f) { f##Field = std::move(f); }
MOVE_FIELD(Typeface)
MOVE_FIELD(ImageFilter)
MOVE_FIELD(Shader)
MOVE_FIELD(ColorFilter)
MOVE_FIELD(PathEffect)
MOVE_FIELD(MaskFilter)
MOVE_FIELD(DrawLooper)
#undef MOVE_FIELD
void SkPaint::setLooper(sk_sp<SkDrawLooper> looper) { fDrawLooper = std::move(looper); }

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
            return SkUTF8_CountUnichars(text, byteLength);
        case kUTF16_TextEncoding:
            return SkUTF16_CountUnichars(text, byteLength);
        case kUTF32_TextEncoding:
            return SkToInt(byteLength >> 2);
        case kGlyphID_TextEncoding:
            return SkToInt(byteLength >> 1);
        default:
            SkDEBUGFAIL("unknown text encoding");
    }

    return 0;
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

    const char* text = (const char*)textData;
    const char* stop = text + byteLength;
    uint16_t*   gptr = glyphs;

    switch (this->getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding:
            while (text < stop) {
                SkUnichar u = SkUTF8_NextUnicharWithError(&text, stop);
                if (u < 0) {
                    return 0;  // bad UTF-8 sequence
                }
                *gptr++ = cache->unicharToGlyph(u);
            }
            break;
        case SkPaint::kUTF16_TextEncoding: {
            const uint16_t* text16 = (const uint16_t*)text;
            const uint16_t* stop16 = (const uint16_t*)stop;
            while (text16 < stop16) {
                *gptr++ = cache->unicharToGlyph(SkUTF16_NextUnichar(&text16));
            }
            break;
        }
        case kUTF32_TextEncoding: {
            const int32_t* text32 = (const int32_t*)text;
            const int32_t* stop32 = (const int32_t*)stop;
            while (text32 < stop32) {
                *gptr++ = cache->unicharToGlyph(*text32++);
            }
            break;
        }
        default:
            SkDEBUGFAIL("unknown text encoding");
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

    switch (this->getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding: {
            const char* text = static_cast<const char*>(textData);
            const char* stop = text + byteLength;
            while (text < stop) {
                if (0 == cache->unicharToGlyph(SkUTF8_NextUnichar(&text))) {
                    return false;
                }
            }
            break;
        }
        case SkPaint::kUTF16_TextEncoding: {
            const uint16_t* text = static_cast<const uint16_t*>(textData);
            const uint16_t* stop = text + (byteLength >> 1);
            while (text < stop) {
                if (0 == cache->unicharToGlyph(SkUTF16_NextUnichar(&text))) {
                    return false;
                }
            }
            break;
        }
        case SkPaint::kUTF32_TextEncoding: {
            const int32_t* text = static_cast<const int32_t*>(textData);
            const int32_t* stop = text + (byteLength >> 2);
            while (text < stop) {
                if (0 == cache->unicharToGlyph(*text++)) {
                    return false;
                }
            }
            break;
        }
        default:
            SkDEBUGFAIL("unknown text encoding");
            return false;
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
                                              const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(SkUTF8_NextUnichar(text));
}

static const SkGlyph& sk_getMetrics_utf16_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharMetrics(SkUTF16_NextUnichar((const uint16_t**)text));
}

static const SkGlyph& sk_getMetrics_utf32_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *ptr++;
    *text = (const char*)ptr;
    return cache->getUnicharMetrics(uni);
}

static const SkGlyph& sk_getMetrics_glyph_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    return cache->getGlyphIDMetrics(glyphID);
}

static const SkGlyph& sk_getAdvance_utf8_next(SkGlyphCache* cache,
                                              const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(SkUTF8_NextUnichar(text));
}

static const SkGlyph& sk_getAdvance_utf16_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    return cache->getUnicharAdvance(SkUTF16_NextUnichar((const uint16_t**)text));
}

static const SkGlyph& sk_getAdvance_utf32_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != nullptr);
    SkASSERT(text != nullptr);

    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *ptr++;
    *text = (const char*)ptr;
    return cache->getUnicharAdvance(uni);
}

static const SkGlyph& sk_getAdvance_glyph_next(SkGlyphCache* cache,
                                               const char** text) {
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

#define TEXT_AS_PATHS_PAINT_FLAGS_TO_IGNORE (   \
SkPaint::kLinearText_Flag           |       \
SkPaint::kLCDRenderText_Flag        |       \
SkPaint::kEmbeddedBitmapText_Flag   |       \
SkPaint::kAutoHinting_Flag          )

SkScalar SkPaint::setupForAsPaths() {
    uint32_t flags = this->getFlags();
    // clear the flags we don't care about
    flags &= ~TEXT_AS_PATHS_PAINT_FLAGS_TO_IGNORE;
    // set the flags we do care about
    flags |= SkPaint::kSubpixelText_Flag;

    this->setFlags(flags);
    this->setHinting(SkPaint::kNo_Hinting);

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
    const SkGlyph* g = &glyphCacheProc(cache, &text);
    SkScalar x = advance(*g, xyIndex);

    if (nullptr == bounds) {
        for (; text < stop; n++) {
            x += advance(glyphCacheProc(cache, &text), xyIndex);
        }
    } else {
        set_bounds(*g, bounds);

        for (; text < stop; n++) {
            g = &glyphCacheProc(cache, &text);
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
        SkScalar x = advance(glyphCacheProc(cache.get(), &text), xyIndex);
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
            const SkGlyph& g = glyphCacheProc(cache.get(), &text);
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
            const SkGlyph& g = glyphCacheProc(cache.get(), &text);
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
        case SkTextBlob::kDefault_Positioning:
            count += runPaint.getTextIntercepts(it.glyphs(), runByteCount, it.offset().x(),
                                                it.offset().y(), bounds, runIntervals);
            break;
        case SkTextBlob::kHorizontal_Positioning:
            count += runPaint.getPosTextHIntercepts(it.glyphs(), runByteCount, it.pos(),
                                                    it.offset().y(), bounds, runIntervals);
            break;
        case SkTextBlob::kFull_Positioning:
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

#include "SkStream.h"

static uintptr_t asint(const void* p) {
    return reinterpret_cast<uintptr_t>(p);
}

static uint32_t pack_4(unsigned a, unsigned b, unsigned c, unsigned d) {
    SkASSERT(a == (uint8_t)a);
    SkASSERT(b == (uint8_t)b);
    SkASSERT(c == (uint8_t)c);
    SkASSERT(d == (uint8_t)d);
    return (a << 24) | (b << 16) | (c << 8) | d;
}

#ifdef SK_DEBUG
    static void ASSERT_FITS_IN(uint32_t value, int bitCount) {
        SkASSERT(bitCount > 0 && bitCount <= 32);
        uint32_t mask = ~0U;
        mask >>= (32 - bitCount);
        SkASSERT(0 == (value & ~mask));
    }
#else
    #define ASSERT_FITS_IN(value, bitcount)
#endif

enum FlatFlags {
    kHasTypeface_FlatFlag = 0x1,
    kHasEffects_FlatFlag  = 0x2,

    kFlatFlagMask         = 0x3,
};

enum BitsPerField {
    kFlags_BPF  = 16,
    kHint_BPF   = 2,
    kAlign_BPF  = 2,
    kFilter_BPF = 2,
    kFlatFlags_BPF  = 3,
};

static inline int BPF_Mask(int bits) {
    return (1 << bits) - 1;
}

static uint32_t pack_paint_flags(unsigned flags, unsigned hint, unsigned align,
                                 unsigned filter, unsigned flatFlags) {
    ASSERT_FITS_IN(flags, kFlags_BPF);
    ASSERT_FITS_IN(hint, kHint_BPF);
    ASSERT_FITS_IN(align, kAlign_BPF);
    ASSERT_FITS_IN(filter, kFilter_BPF);
    ASSERT_FITS_IN(flatFlags, kFlatFlags_BPF);

    // left-align the fields of "known" size, and right-align the last (flatFlags) so it can easly
    // add more bits in the future.
    return (flags << 16) | (hint << 14) | (align << 12) | (filter << 10) | flatFlags;
}

static FlatFlags unpack_paint_flags(SkPaint* paint, uint32_t packed) {
    paint->setFlags(packed >> 16);
    paint->setHinting((SkPaint::Hinting)((packed >> 14) & BPF_Mask(kHint_BPF)));
    paint->setTextAlign((SkPaint::Align)((packed >> 12) & BPF_Mask(kAlign_BPF)));
    paint->setFilterQuality((SkFilterQuality)((packed >> 10) & BPF_Mask(kFilter_BPF)));
    return (FlatFlags)(packed & kFlatFlagMask);
}

/*  To save space/time, we analyze the paint, and write a truncated version of
    it if there are not tricky elements like shaders, etc.
 */
void SkPaintPriv::Flatten(const SkPaint& paint, SkWriteBuffer& buffer) {
    // We force recording our typeface, even if its "default" since the receiver process
    // may have a different notion of default.
    SkTypeface* tf = SkPaintPriv::GetTypefaceOrDefault(paint);
    SkASSERT(tf);

    uint8_t flatFlags = kHasTypeface_FlatFlag;

    if (asint(paint.getPathEffect()) |
        asint(paint.getShader()) |
        asint(paint.getMaskFilter()) |
        asint(paint.getColorFilter()) |
        asint(paint.getLooper()) |
        asint(paint.getImageFilter())) {
        flatFlags |= kHasEffects_FlatFlag;
    }

    buffer.writeScalar(paint.getTextSize());
    buffer.writeScalar(paint.getTextScaleX());
    buffer.writeScalar(paint.getTextSkewX());
    buffer.writeScalar(paint.getStrokeWidth());
    buffer.writeScalar(paint.getStrokeMiter());
    buffer.writeColor(paint.getColor());

    buffer.writeUInt(pack_paint_flags(paint.getFlags(), paint.getHinting(), paint.getTextAlign(),
                                      paint.getFilterQuality(), flatFlags));
    buffer.writeUInt(pack_4(paint.getStrokeCap(), paint.getStrokeJoin(),
                            (paint.getStyle() << 4) | paint.getTextEncoding(),
                            paint.fBlendMode));

    buffer.writeTypeface(tf);

    if (flatFlags & kHasEffects_FlatFlag) {
        buffer.writeFlattenable(paint.getPathEffect());
        buffer.writeFlattenable(paint.getShader());
        buffer.writeFlattenable(paint.getMaskFilter());
        buffer.writeFlattenable(paint.getColorFilter());
        buffer.write32(0);  // use to be SkRasterizer
        buffer.writeFlattenable(paint.getLooper());
        buffer.writeFlattenable(paint.getImageFilter());
    }
}

bool SkPaintPriv::Unflatten(SkPaint* paint, SkReadBuffer& buffer) {
    SkSafeRange safe;

    paint->setTextSize(buffer.readScalar());
    paint->setTextScaleX(buffer.readScalar());
    paint->setTextSkewX(buffer.readScalar());
    paint->setStrokeWidth(buffer.readScalar());
    paint->setStrokeMiter(buffer.readScalar());
    paint->setColor(buffer.readColor());

    unsigned flatFlags = unpack_paint_flags(paint, buffer.readUInt());

    uint32_t tmp = buffer.readUInt();
    paint->setStrokeCap(safe.checkLE((tmp >> 24) & 0xFF, SkPaint::kLast_Cap));
    paint->setStrokeJoin(safe.checkLE((tmp >> 16) & 0xFF, SkPaint::kLast_Join));
    paint->setStyle(safe.checkLE((tmp >> 12) & 0xF, SkPaint::kStrokeAndFill_Style));
    paint->setTextEncoding(safe.checkLE((tmp >> 8) & 0xF, SkPaint::kGlyphID_TextEncoding));
    paint->setBlendMode(safe.checkLE(tmp & 0xFF, SkBlendMode::kLastMode));

    if (flatFlags & kHasTypeface_FlatFlag) {
        paint->setTypeface(buffer.readTypeface());
    } else {
        paint->setTypeface(nullptr);
    }

    if (flatFlags & kHasEffects_FlatFlag) {
        paint->setPathEffect(buffer.readPathEffect());
        paint->setShader(buffer.readShader());
        paint->setMaskFilter(buffer.readMaskFilter());
        paint->setColorFilter(buffer.readColorFilter());
        (void)buffer.read32();  // use to be SkRasterizer
        paint->setLooper(buffer.readDrawLooper());
        paint->setImageFilter(buffer.readImageFilter());
    } else {
        paint->setPathEffect(nullptr);
        paint->setShader(nullptr);
        paint->setMaskFilter(nullptr);
        paint->setColorFilter(nullptr);
        paint->setLooper(nullptr);
        paint->setImageFilter(nullptr);
    }

    if (!buffer.validate(safe)) {
        paint->reset();
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkPaint::getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                          SkScalar resScale) const {
    if (!src.isFinite()) {
        dst->reset();
        return false;
    }

    SkStrokeRec rec(*this, resScale);

    const SkPath* srcPtr = &src;
    SkPath tmpPath;

    if (fPathEffect && fPathEffect->filterPath(&tmpPath, src, &rec, cullRect)) {
        srcPtr = &tmpPath;
    }

    if (!rec.applyToPath(dst, *srcPtr)) {
        if (srcPtr == &tmpPath) {
            // If path's were copy-on-write, this trick would not be needed.
            // As it is, we want to save making a deep-copy from tmpPath -> dst
            // since we know we're just going to delete tmpPath when we return,
            // so the swap saves that copy.
            dst->swap(tmpPath);
        } else {
            *dst = *srcPtr;
        }
    }

    if (!dst->isFinite()) {
        dst->reset();
        return false;
    }
    return !rec.isHairlineStyle();
}

bool SkPaint::canComputeFastBounds() const {
    if (this->getLooper()) {
        return this->getLooper()->canComputeFastBounds(*this);
    }
    if (this->getImageFilter() && !this->getImageFilter()->canComputeFastBounds()) {
        return false;
    }
    return true;
}

const SkRect& SkPaint::doComputeFastBounds(const SkRect& origSrc,
                                           SkRect* storage,
                                           Style style) const {
    SkASSERT(storage);

    const SkRect* src = &origSrc;

    if (this->getLooper()) {
        SkASSERT(this->getLooper()->canComputeFastBounds(*this));
        this->getLooper()->computeFastBounds(*this, *src, storage);
        return *storage;
    }

    SkRect tmpSrc;
    if (this->getPathEffect()) {
        this->getPathEffect()->computeFastBounds(&tmpSrc, origSrc);
        src = &tmpSrc;
    }

    SkScalar radius = SkStrokeRec::GetInflationRadius(*this, style);
    *storage = src->makeOutset(radius, radius);

    if (this->getMaskFilter()) {
        as_MFB(this->getMaskFilter())->computeFastBounds(*storage, storage);
    }

    if (this->getImageFilter()) {
        *storage = this->getImageFilter()->computeFastBounds(*storage);
    }

    return *storage;
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
        const SkGlyph& glyph = fGlyphCacheProc(fCache.get(), &fText);

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
    const SkGlyph& glyph = fGlyphCacheProc(fCache.get(), &fText);
    fXPos += fPrevAdvance * fScale;
    fPrevAdvance = advance(glyph, fXYIndex);   // + fPaint.getTextTracking();
    if (fCache->findPath(glyph)) {
        fCache->findIntercepts(fBounds, fScale, fXPos, SkToBool(fXYIndex),
                const_cast<SkGlyph*>(&glyph), array, count);
    }
    return fText < fStop;
}

///////////////////////////////////////////////////////////////////////////////

// return true if the filter exists, and may affect alpha
static bool affects_alpha(const SkColorFilter* cf) {
    return cf && !(cf->getFlags() & SkColorFilter::kAlphaUnchanged_Flag);
}

// return true if the filter exists, and may affect alpha
static bool affects_alpha(const SkImageFilter* imf) {
    // TODO: check if we should allow imagefilters to broadcast that they don't affect alpha
    // ala colorfilters
    return imf != nullptr;
}

bool SkPaint::nothingToDraw() const {
    if (fDrawLooper) {
        return false;
    }
    switch ((SkBlendMode)fBlendMode) {
        case SkBlendMode::kSrcOver:
        case SkBlendMode::kSrcATop:
        case SkBlendMode::kDstOut:
        case SkBlendMode::kDstOver:
        case SkBlendMode::kPlus:
            if (0 == this->getAlpha()) {
                return !affects_alpha(fColorFilter.get()) && !affects_alpha(fImageFilter.get());
            }
            break;
        case SkBlendMode::kDst:
            return true;
        default:
            break;
    }
    return false;
}

uint32_t SkPaint::getHash() const {
    // We're going to hash 7 pointers and 7 32-bit values, finishing up with fBitfields,
    // so fBitfields should be 7 pointers and 6 32-bit values from the start.
    static_assert(offsetof(SkPaint, fBitfields) == 7 * sizeof(void*) + 7 * sizeof(uint32_t),
                  "SkPaint_notPackedTightly");
    return SkOpts::hash(reinterpret_cast<const uint32_t*>(this),
                        offsetof(SkPaint, fBitfields) + sizeof(fBitfields));
}
