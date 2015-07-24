/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkAnnotation.h"
#include "SkAutoKern.h"
#include "SkChecksum.h"
#include "SkColorFilter.h"
#include "SkData.h"
#include "SkDraw.h"
#include "SkFontDescriptor.h"
#include "SkGlyphCache.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkMaskGamma.h"
#include "SkMutex.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPaintDefaults.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkScalar.h"
#include "SkScalerContext.h"
#include "SkShader.h"
#include "SkStringUtils.h"
#include "SkStroke.h"
#include "SkTextFormatParams.h"
#include "SkTextToPathIter.h"
#include "SkTLazy.h"
#include "SkTypeface.h"
#include "SkSurfacePriv.h"
#include "SkXfermode.h"

// define this to get a printf for out-of-range parameter in setters
// e.g. setTextSize(-1)
//#define SK_REPORT_API_RANGE_CHECK

SkPaint::SkPaint() {
    fTypeface    = NULL;
    fPathEffect  = NULL;
    fShader      = NULL;
    fXfermode    = NULL;
    fMaskFilter  = NULL;
    fColorFilter = NULL;
    fRasterizer  = NULL;
    fLooper      = NULL;
    fImageFilter = NULL;
    fAnnotation  = NULL;

    fTextSize   = SkPaintDefaults_TextSize;
    fTextScaleX = SK_Scalar1;
    fTextSkewX  = 0;
    fColor      = SK_ColorBLACK;
    fWidth      = 0;
    fMiterLimit = SkPaintDefaults_MiterLimit;

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

SkPaint::SkPaint(const SkPaint& src) {
#define COPY(field) field = src.field
#define REF_COPY(field) field = SkSafeRef(src.field)

    REF_COPY(fTypeface);
    REF_COPY(fPathEffect);
    REF_COPY(fShader);
    REF_COPY(fXfermode);
    REF_COPY(fMaskFilter);
    REF_COPY(fColorFilter);
    REF_COPY(fRasterizer);
    REF_COPY(fLooper);
    REF_COPY(fImageFilter);
    REF_COPY(fAnnotation);

    COPY(fTextSize);
    COPY(fTextScaleX);
    COPY(fTextSkewX);
    COPY(fColor);
    COPY(fWidth);
    COPY(fMiterLimit);
    COPY(fBitfields);

#undef COPY
#undef REF_COPY
}

SkPaint::~SkPaint() {
    SkSafeUnref(fTypeface);
    SkSafeUnref(fPathEffect);
    SkSafeUnref(fShader);
    SkSafeUnref(fXfermode);
    SkSafeUnref(fMaskFilter);
    SkSafeUnref(fColorFilter);
    SkSafeUnref(fRasterizer);
    SkSafeUnref(fLooper);
    SkSafeUnref(fImageFilter);
    SkSafeUnref(fAnnotation);
}

SkPaint& SkPaint::operator=(const SkPaint& src) {
    if (this == &src) {
        return *this;
    }

#define COPY(field) field = src.field
#define REF_COPY(field) SkSafeUnref(field); field = SkSafeRef(src.field)

    REF_COPY(fTypeface);
    REF_COPY(fPathEffect);
    REF_COPY(fShader);
    REF_COPY(fXfermode);
    REF_COPY(fMaskFilter);
    REF_COPY(fColorFilter);
    REF_COPY(fRasterizer);
    REF_COPY(fLooper);
    REF_COPY(fImageFilter);
    REF_COPY(fAnnotation);

    COPY(fTextSize);
    COPY(fTextScaleX);
    COPY(fTextSkewX);
    COPY(fColor);
    COPY(fWidth);
    COPY(fMiterLimit);
    COPY(fBitfields);

    return *this;

#undef COPY
#undef REF_COPY
}

bool operator==(const SkPaint& a, const SkPaint& b) {
#define EQUAL(field) (a.field == b.field)
    return EQUAL(fTypeface)
        && EQUAL(fPathEffect)
        && EQUAL(fShader)
        && EQUAL(fXfermode)
        && EQUAL(fMaskFilter)
        && EQUAL(fColorFilter)
        && EQUAL(fRasterizer)
        && EQUAL(fLooper)
        && EQUAL(fImageFilter)
        && EQUAL(fAnnotation)
        && EQUAL(fTextSize)
        && EQUAL(fTextScaleX)
        && EQUAL(fTextSkewX)
        && EQUAL(fColor)
        && EQUAL(fWidth)
        && EQUAL(fMiterLimit)
        && EQUAL(fBitfieldsUInt)
        ;
#undef EQUAL
}

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
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doAA, kAntiAlias_Flag));
}

void SkPaint::setDither(bool doDither) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doDither, kDither_Flag));
}

void SkPaint::setSubpixelText(bool doSubpixel) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doSubpixel, kSubpixelText_Flag));
}

void SkPaint::setLCDRenderText(bool doLCDRender) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doLCDRender, kLCDRenderText_Flag));
}

void SkPaint::setEmbeddedBitmapText(bool doEmbeddedBitmapText) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doEmbeddedBitmapText, kEmbeddedBitmapText_Flag));
}

void SkPaint::setAutohinted(bool useAutohinter) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, useAutohinter, kAutoHinting_Flag));
}

void SkPaint::setLinearText(bool doLinearText) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doLinearText, kLinearText_Flag));
}

void SkPaint::setVerticalText(bool doVertical) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doVertical, kVerticalText_Flag));
}

void SkPaint::setUnderlineText(bool doUnderline) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doUnderline, kUnderlineText_Flag));
}

void SkPaint::setStrikeThruText(bool doStrikeThru) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doStrikeThru, kStrikeThruText_Flag));
}

void SkPaint::setFakeBoldText(bool doFakeBold) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doFakeBold, kFakeBoldText_Flag));
}

void SkPaint::setDevKernText(bool doDevKern) {
    this->setFlags(SkSetClearMask(fBitfields.fFlags, doDevKern, kDevKernText_Flag));
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

SkTypeface* SkPaint::setTypeface(SkTypeface* font) {
    SkRefCnt_SafeAssign(fTypeface, font);
    return font;
}

SkRasterizer* SkPaint::setRasterizer(SkRasterizer* r) {
    SkRefCnt_SafeAssign(fRasterizer, r);
    return r;
}

SkDrawLooper* SkPaint::setLooper(SkDrawLooper* looper) {
    SkRefCnt_SafeAssign(fLooper, looper);
    return looper;
}

SkImageFilter* SkPaint::setImageFilter(SkImageFilter* imageFilter) {
    SkRefCnt_SafeAssign(fImageFilter, imageFilter);
    return imageFilter;
}

SkAnnotation* SkPaint::setAnnotation(SkAnnotation* annotation) {
    SkRefCnt_SafeAssign(fAnnotation, annotation);
    return annotation;
}

///////////////////////////////////////////////////////////////////////////////

static SkScalar mag2(SkScalar x, SkScalar y) {
    return x * x + y * y;
}

static bool tooBig(const SkMatrix& m, SkScalar ma2max) {
    return  mag2(m[SkMatrix::kMScaleX], m[SkMatrix::kMSkewY]) > ma2max
            ||
            mag2(m[SkMatrix::kMSkewX], m[SkMatrix::kMScaleY]) > ma2max;
}

bool SkPaint::TooBigToUseCache(const SkMatrix& ctm, const SkMatrix& textM) {
    SkASSERT(!ctm.hasPerspective());
    SkASSERT(!textM.hasPerspective());

    SkMatrix matrix;
    matrix.setConcat(ctm, textM);
    return tooBig(matrix, MaxCacheSize2());
}


///////////////////////////////////////////////////////////////////////////////

#include "SkGlyphCache.h"
#include "SkUtils.h"

static void DetachDescProc(SkTypeface* typeface, const SkDescriptor* desc,
                           void* context) {
    *((SkGlyphCache**)context) = SkGlyphCache::DetachCache(typeface, desc);
}

int SkPaint::textToGlyphs(const void* textData, size_t byteLength,
                          uint16_t glyphs[]) const {
    if (byteLength == 0) {
        return 0;
    }

    SkASSERT(textData != NULL);

    if (NULL == glyphs) {
        switch (this->getTextEncoding()) {
        case kUTF8_TextEncoding:
            return SkUTF8_CountUnichars((const char*)textData, byteLength);
        case kUTF16_TextEncoding:
            return SkUTF16_CountUnichars((const uint16_t*)textData, SkToInt(byteLength >> 1));
        case kUTF32_TextEncoding:
            return SkToInt(byteLength >> 2);
        case kGlyphID_TextEncoding:
            return SkToInt(byteLength >> 1);
        default:
            SkDEBUGFAIL("unknown text encoding");
        }
        return 0;
    }

    // if we get here, we have a valid glyphs[] array, so time to fill it in

    // handle this encoding before the setup for the glyphcache
    if (this->getTextEncoding() == kGlyphID_TextEncoding) {
        // we want to ignore the low bit of byteLength
        memcpy(glyphs, textData, byteLength >> 1 << 1);
        return SkToInt(byteLength >> 1);
    }

    SkAutoGlyphCache autoCache(*this, NULL, NULL);
    SkGlyphCache*    cache = autoCache.getCache();

    const char* text = (const char*)textData;
    const char* stop = text + byteLength;
    uint16_t*   gptr = glyphs;

    switch (this->getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding:
            while (text < stop) {
                *gptr++ = cache->unicharToGlyph(SkUTF8_NextUnichar(&text));
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

    SkASSERT(textData != NULL);

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

    SkAutoGlyphCache autoCache(*this, NULL, NULL);
    SkGlyphCache*    cache = autoCache.getCache();

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

    SkASSERT(glyphs != NULL);
    SkASSERT(textData != NULL);

    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    SkAutoGlyphCache autoCache(*this, &props, NULL);
    SkGlyphCache*    cache = autoCache.getCache();

    for (int index = 0; index < count; index++) {
        textData[index] = cache->glyphToUnichar(glyphs[index]);
    }
}

///////////////////////////////////////////////////////////////////////////////

static const SkGlyph& sk_getMetrics_utf8_next(SkGlyphCache* cache,
                                              const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharMetrics(SkUTF8_NextUnichar(text));
}

static const SkGlyph& sk_getMetrics_utf16_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharMetrics(SkUTF16_NextUnichar((const uint16_t**)text));
}

static const SkGlyph& sk_getMetrics_utf32_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *ptr++;
    *text = (const char*)ptr;
    return cache->getUnicharMetrics(uni);
}

static const SkGlyph& sk_getMetrics_glyph_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    return cache->getGlyphIDMetrics(glyphID);
}

static const SkGlyph& sk_getAdvance_utf8_next(SkGlyphCache* cache,
                                              const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharAdvance(SkUTF8_NextUnichar(text));
}

static const SkGlyph& sk_getAdvance_utf16_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharAdvance(SkUTF16_NextUnichar((const uint16_t**)text));
}

static const SkGlyph& sk_getAdvance_utf32_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *ptr++;
    *text = (const char*)ptr;
    return cache->getUnicharAdvance(uni);
}

static const SkGlyph& sk_getAdvance_glyph_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    return cache->getGlyphIDAdvance(glyphID);
}

SkMeasureCacheProc SkPaint::getMeasureCacheProc(bool needFullMetrics) const {
    static const SkMeasureCacheProc gMeasureCacheProcs[] = {
        sk_getMetrics_utf8_next,
        sk_getMetrics_utf16_next,
        sk_getMetrics_utf32_next,
        sk_getMetrics_glyph_next,

        sk_getAdvance_utf8_next,
        sk_getAdvance_utf16_next,
        sk_getAdvance_utf32_next,
        sk_getAdvance_glyph_next,
    };

    unsigned index = this->getTextEncoding();

    if (!needFullMetrics && !this->isDevKernText()) {
        index += 4;
    }

    SkASSERT(index < SK_ARRAY_COUNT(gMeasureCacheProcs));
    return gMeasureCacheProcs[index];
}

///////////////////////////////////////////////////////////////////////////////

static const SkGlyph& sk_getMetrics_utf8_00(SkGlyphCache* cache,
                                        const char** text, SkFixed, SkFixed) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharMetrics(SkUTF8_NextUnichar(text));
}

static const SkGlyph& sk_getMetrics_utf8_xy(SkGlyphCache* cache,
                                    const char** text, SkFixed x, SkFixed y) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharMetrics(SkUTF8_NextUnichar(text), x, y);
}

static const SkGlyph& sk_getMetrics_utf16_00(SkGlyphCache* cache,
                                        const char** text, SkFixed, SkFixed) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharMetrics(SkUTF16_NextUnichar((const uint16_t**)text));
}

static const SkGlyph& sk_getMetrics_utf16_xy(SkGlyphCache* cache,
                                     const char** text, SkFixed x, SkFixed y) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharMetrics(SkUTF16_NextUnichar((const uint16_t**)text),
                                    x, y);
}

static const SkGlyph& sk_getMetrics_utf32_00(SkGlyphCache* cache,
                                    const char** text, SkFixed, SkFixed) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *ptr++;
    *text = (const char*)ptr;
    return cache->getUnicharMetrics(uni);
}

static const SkGlyph& sk_getMetrics_utf32_xy(SkGlyphCache* cache,
                                    const char** text, SkFixed x, SkFixed y) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *ptr++;
    *text = (const char*)ptr;
    return cache->getUnicharMetrics(uni, x, y);
}

static const SkGlyph& sk_getMetrics_glyph_00(SkGlyphCache* cache,
                                         const char** text, SkFixed, SkFixed) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    return cache->getGlyphIDMetrics(glyphID);
}

static const SkGlyph& sk_getMetrics_glyph_xy(SkGlyphCache* cache,
                                     const char** text, SkFixed x, SkFixed y) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const uint16_t* ptr = *(const uint16_t**)text;
    unsigned glyphID = *ptr;
    ptr += 1;
    *text = (const char*)ptr;
    return cache->getGlyphIDMetrics(glyphID, x, y);
}

SkDrawCacheProc SkPaint::getDrawCacheProc() const {
    static const SkDrawCacheProc gDrawCacheProcs[] = {
        sk_getMetrics_utf8_00,
        sk_getMetrics_utf16_00,
        sk_getMetrics_utf32_00,
        sk_getMetrics_glyph_00,

        sk_getMetrics_utf8_xy,
        sk_getMetrics_utf16_xy,
        sk_getMetrics_utf32_xy,
        sk_getMetrics_glyph_xy
    };

    unsigned index = this->getTextEncoding();
    if (fBitfields.fFlags & kSubpixelText_Flag) {
        index += 4;
    }

    SkASSERT(index < SK_ARRAY_COUNT(gDrawCacheProcs));
    return gDrawCacheProcs[index];
}

///////////////////////////////////////////////////////////////////////////////

#define TEXT_AS_PATHS_PAINT_FLAGS_TO_IGNORE (   \
SkPaint::kDevKernText_Flag          |       \
SkPaint::kLinearText_Flag           |       \
SkPaint::kLCDRenderText_Flag        |       \
SkPaint::kEmbeddedBitmapText_Flag   |       \
SkPaint::kAutoHinting_Flag          |       \
SkPaint::kGenA8FromLCD_Flag )

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

static void join_bounds_x(const SkGlyph& g, SkRect* bounds, Sk48Dot16 dx) {
    SkScalar sx = Sk48Dot16ToScalar(dx);
    bounds->join(SkIntToScalar(g.fLeft) + sx,
                 SkIntToScalar(g.fTop),
                 SkIntToScalar(g.fLeft + g.fWidth) + sx,
                 SkIntToScalar(g.fTop + g.fHeight));
}

static void join_bounds_y(const SkGlyph& g, SkRect* bounds, Sk48Dot16 dy) {
    SkScalar sy = Sk48Dot16ToScalar(dy);
    bounds->join(SkIntToScalar(g.fLeft),
                 SkIntToScalar(g.fTop) + sy,
                 SkIntToScalar(g.fLeft + g.fWidth),
                 SkIntToScalar(g.fTop + g.fHeight) + sy);
}

typedef void (*JoinBoundsProc)(const SkGlyph&, SkRect*, Sk48Dot16);

// xyIndex is 0 for fAdvanceX or 1 for fAdvanceY
static SkFixed advance(const SkGlyph& glyph, int xyIndex) {
    SkASSERT(0 == xyIndex || 1 == xyIndex);
    return (&glyph.fAdvanceX)[xyIndex];
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

    SkMeasureCacheProc glyphCacheProc = this->getMeasureCacheProc(NULL != bounds);

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
    // our accumulated fixed-point advances might overflow 16.16, so we use
    // a 48.16 (64bit) accumulator, and then convert that to scalar at the
    // very end.
    Sk48Dot16 x = advance(*g, xyIndex);

    SkAutoKern  autokern;

    if (NULL == bounds) {
        if (this->isDevKernText()) {
            int rsb;
            for (; text < stop; n++) {
                rsb = g->fRsbDelta;
                g = &glyphCacheProc(cache, &text);
                x += SkAutoKern_AdjustF(rsb, g->fLsbDelta) + advance(*g, xyIndex);
            }
        } else {
            for (; text < stop; n++) {
                x += advance(glyphCacheProc(cache, &text), xyIndex);
            }
        }
    } else {
        set_bounds(*g, bounds);
        if (this->isDevKernText()) {
            int rsb;
            for (; text < stop; n++) {
                rsb = g->fRsbDelta;
                g = &glyphCacheProc(cache, &text);
                x += SkAutoKern_AdjustF(rsb, g->fLsbDelta);
                joinBoundsProc(*g, bounds, x);
                x += advance(*g, xyIndex);
            }
        } else {
            for (; text < stop; n++) {
                g = &glyphCacheProc(cache, &text);
                joinBoundsProc(*g, bounds, x);
                x += advance(*g, xyIndex);
            }
        }
    }
    SkASSERT(text == stop);

    *count = n;
    return Sk48Dot16ToScalar(x);
}

SkScalar SkPaint::measureText(const void* textData, size_t length, SkRect* bounds) const {
    const char* text = (const char*)textData;
    SkASSERT(text != NULL || length == 0);

    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    SkAutoGlyphCache    autoCache(paint, NULL, NULL);
    SkGlyphCache*       cache = autoCache.getCache();

    SkScalar width = 0;

    if (length > 0) {
        int tempCount;

        width = paint.measure_text(cache, text, length, &tempCount, bounds);
        if (scale) {
            width = SkScalarMul(width, scale);
            if (bounds) {
                bounds->fLeft = SkScalarMul(bounds->fLeft, scale);
                bounds->fTop = SkScalarMul(bounds->fTop, scale);
                bounds->fRight = SkScalarMul(bounds->fRight, scale);
                bounds->fBottom = SkScalarMul(bounds->fBottom, scale);
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

    SkASSERT(textD != NULL);
    const char* text = (const char*)textD;
    const char* stop = text + length;

    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    // adjust max in case we changed the textSize in paint
    if (scale) {
        maxWidth /= scale;
    }

    SkAutoGlyphCache    autoCache(paint, NULL, NULL);
    SkGlyphCache*       cache = autoCache.getCache();

    SkMeasureCacheProc glyphCacheProc = paint.getMeasureCacheProc(false);
    const int        xyIndex = paint.isVerticalText() ? 1 : 0;
    // use 64bits for our accumulator, to avoid overflowing 16.16
    Sk48Dot16        max = SkScalarToFixed(maxWidth);
    Sk48Dot16        width = 0;

    SkAutoKern  autokern;

    if (this->isDevKernText()) {
        int rsb = 0;
        while (text < stop) {
            const char* curr = text;
            const SkGlyph& g = glyphCacheProc(cache, &text);
            SkFixed x = SkAutoKern_AdjustF(rsb, g.fLsbDelta) + advance(g, xyIndex);
            if ((width += x) > max) {
                width -= x;
                text = curr;
                break;
            }
            rsb = g.fRsbDelta;
        }
    } else {
        while (text < stop) {
            const char* curr = text;
            SkFixed x = advance(glyphCacheProc(cache, &text), xyIndex);
            if ((width += x) > max) {
                width -= x;
                text = curr;
                break;
            }
        }
    }

    if (measuredWidth) {
        SkScalar scalarWidth = Sk48Dot16ToScalar(width);
        if (scale) {
            scalarWidth = SkScalarMul(scalarWidth, scale);
        }
        *measuredWidth = scalarWidth;
    }

    // return the number of bytes measured
    return text - stop + length;
}

///////////////////////////////////////////////////////////////////////////////

static bool FontMetricsCacheProc(const SkGlyphCache* cache, void* context) {
    *(SkPaint::FontMetrics*)context = cache->getFontMetrics();
    return false;   // don't detach the cache
}

static void FontMetricsDescProc(SkTypeface* typeface, const SkDescriptor* desc,
                                void* context) {
    SkGlyphCache::VisitCache(typeface, desc, FontMetricsCacheProc, context);
}

SkScalar SkPaint::getFontMetrics(FontMetrics* metrics, SkScalar zoom) const {
    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    SkMatrix zoomMatrix, *zoomPtr = NULL;
    if (zoom) {
        zoomMatrix.setScale(zoom, zoom);
        zoomPtr = &zoomMatrix;
    }

    FontMetrics storage;
    if (NULL == metrics) {
        metrics = &storage;
    }

    paint.descriptorProc(NULL, zoomPtr, FontMetricsDescProc, metrics, true);

    if (scale) {
        metrics->fTop = SkScalarMul(metrics->fTop, scale);
        metrics->fAscent = SkScalarMul(metrics->fAscent, scale);
        metrics->fDescent = SkScalarMul(metrics->fDescent, scale);
        metrics->fBottom = SkScalarMul(metrics->fBottom, scale);
        metrics->fLeading = SkScalarMul(metrics->fLeading, scale);
        metrics->fAvgCharWidth = SkScalarMul(metrics->fAvgCharWidth, scale);
        metrics->fXMin = SkScalarMul(metrics->fXMin, scale);
        metrics->fXMax = SkScalarMul(metrics->fXMax, scale);
        metrics->fXHeight = SkScalarMul(metrics->fXHeight, scale);
        metrics->fUnderlineThickness = SkScalarMul(metrics->fUnderlineThickness, scale);
        metrics->fUnderlinePosition = SkScalarMul(metrics->fUnderlinePosition, scale);
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

    if (NULL == widths && NULL == bounds) {
        return this->countText(textData, byteLength);
    }

    SkCanonicalizePaint canon(*this);
    const SkPaint& paint = canon.getPaint();
    SkScalar scale = canon.getScale();

    SkAutoGlyphCache    autoCache(paint, NULL, NULL);
    SkGlyphCache*       cache = autoCache.getCache();
    SkMeasureCacheProc  glyphCacheProc;
    glyphCacheProc = paint.getMeasureCacheProc(NULL != bounds);

    const char* text = (const char*)textData;
    const char* stop = text + byteLength;
    int         count = 0;
    const int   xyIndex = paint.isVerticalText() ? 1 : 0;

    if (this->isDevKernText()) {
        // we adjust the widths returned here through auto-kerning
        SkAutoKern  autokern;
        SkFixed     prevWidth = 0;

        if (scale) {
            while (text < stop) {
                const SkGlyph& g = glyphCacheProc(cache, &text);
                if (widths) {
                    SkFixed  adjust = autokern.adjust(g);

                    if (count > 0) {
                        SkScalar w = SkFixedToScalar(prevWidth + adjust);
                        *widths++ = SkScalarMul(w, scale);
                    }
                    prevWidth = advance(g, xyIndex);
                }
                if (bounds) {
                    set_bounds(g, bounds++, scale);
                }
                ++count;
            }
            if (count > 0 && widths) {
                *widths = SkScalarMul(SkFixedToScalar(prevWidth), scale);
            }
        } else {
            while (text < stop) {
                const SkGlyph& g = glyphCacheProc(cache, &text);
                if (widths) {
                    SkFixed  adjust = autokern.adjust(g);

                    if (count > 0) {
                        *widths++ = SkFixedToScalar(prevWidth + adjust);
                    }
                    prevWidth = advance(g, xyIndex);
                }
                if (bounds) {
                    set_bounds(g, bounds++);
                }
                ++count;
            }
            if (count > 0 && widths) {
                *widths = SkFixedToScalar(prevWidth);
            }
        }
    } else {    // no devkern
        if (scale) {
            while (text < stop) {
                const SkGlyph& g = glyphCacheProc(cache, &text);
                if (widths) {
                    *widths++ = SkScalarMul(SkFixedToScalar(advance(g, xyIndex)),
                                            scale);
                }
                if (bounds) {
                    set_bounds(g, bounds++, scale);
                }
                ++count;
            }
        } else {
            while (text < stop) {
                const SkGlyph& g = glyphCacheProc(cache, &text);
                if (widths) {
                    *widths++ = SkFixedToScalar(advance(g, xyIndex));
                }
                if (bounds) {
                    set_bounds(g, bounds++);
                }
                ++count;
            }
        }
    }

    SkASSERT(text == stop);
    return count;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkDraw.h"

void SkPaint::getTextPath(const void* textData, size_t length,
                          SkScalar x, SkScalar y, SkPath* path) const {
    SkASSERT(length == 0 || textData != NULL);

    const char* text = (const char*)textData;
    if (text == NULL || length == 0 || path == NULL) {
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
    SkASSERT(length == 0 || textData != NULL);

    const char* text = (const char*)textData;
    if (text == NULL || length == 0 || path == NULL) {
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
    while (iter.next(&iterPath, NULL)) {
        matrix.postTranslate(pos[i].fX - prevPos.fX, pos[i].fY - prevPos.fY);
        if (iterPath) {
            path->addPath(*iterPath, matrix);
        }
        prevPos = pos[i];
        i++;
    }
}

SkRect SkPaint::getFontBounds() const {
    SkMatrix m;
    m.setScale(fTextSize * fTextScaleX, fTextSize);
    m.postSkew(fTextSkewX, 0);

    SkTypeface* typeface = this->getTypeface();
    if (NULL == typeface) {
        typeface = SkTypeface::GetDefaultTypeface();
    }

    SkRect bounds;
    m.mapRect(&bounds, typeface->getBounds());
    return bounds;
}

static void add_flattenable(SkDescriptor* desc, uint32_t tag,
                            SkWriteBuffer* buffer) {
    buffer->writeToMemory(desc->addEntry(tag, buffer->bytesWritten(), NULL));
}

static SkMask::Format compute_mask_format(const SkPaint& paint) {
    uint32_t flags = paint.getFlags();

    // Antialiasing being disabled trumps all other settings.
    if (!(flags & SkPaint::kAntiAlias_Flag)) {
        return SkMask::kBW_Format;
    }

    if (flags & SkPaint::kLCDRenderText_Flag) {
        return SkMask::kLCD16_Format;
    }

    return SkMask::kA8_Format;
}

// if linear-text is on, then we force hinting to be off (since that's sort of
// the point of linear-text.
static SkPaint::Hinting computeHinting(const SkPaint& paint) {
    SkPaint::Hinting h = paint.getHinting();
    if (paint.isLinearText()) {
        h = SkPaint::kNo_Hinting;
    }
    return h;
}

// return true if the paint is just a single color (i.e. not a shader). If its
// a shader, then we can't compute a const luminance for it :(
static bool justAColor(const SkPaint& paint, SkColor* color) {
    SkColor c = paint.getColor();

    SkShader* shader = paint.getShader();
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
    if (!justAColor(*this, &c)) {
        c = SkColorSetRGB(0x7F, 0x80, 0x7F);
    }
    return c;
}

#define assert_byte(x)  SkASSERT(0 == ((x) >> 8))

// Beyond this size, LCD doesn't appreciably improve quality, but it always
// cost more RAM and draws slower, so we set a cap.
#ifndef SK_MAX_SIZE_FOR_LCDTEXT
    #define SK_MAX_SIZE_FOR_LCDTEXT    48
#endif

const SkScalar gMaxSize2ForLCDText = SK_MAX_SIZE_FOR_LCDTEXT * SK_MAX_SIZE_FOR_LCDTEXT;

static bool too_big_for_lcd(const SkScalerContext::Rec& rec, bool checkPost2x2) {
    if (checkPost2x2) {
        SkScalar area = rec.fPost2x2[0][0] * rec.fPost2x2[1][1] -
                        rec.fPost2x2[1][0] * rec.fPost2x2[0][1];
        area *= rec.fTextSize * rec.fTextSize;
        return area > gMaxSize2ForLCDText;
    } else {
        return rec.fTextSize > SK_MAX_SIZE_FOR_LCDTEXT;
    }
}

/*
 *  Return the scalar with only limited fractional precision. Used to consolidate matrices
 *  that vary only slightly when we create our key into the font cache, since the font scaler
 *  typically returns the same looking resuts for tiny changes in the matrix.
 */
static SkScalar sk_relax(SkScalar x) {
    int n = sk_float_round2int(x * 1024);
    return n / 1024.0f;
}

void SkScalerContext::MakeRec(const SkPaint& paint,
                              const SkSurfaceProps* surfaceProps,
                              const SkMatrix* deviceMatrix,
                              Rec* rec) {
    SkASSERT(deviceMatrix == NULL || !deviceMatrix->hasPerspective());

    SkTypeface* typeface = paint.getTypeface();
    if (NULL == typeface) {
        typeface = SkTypeface::GetDefaultTypeface();
    }
    rec->fFontID = typeface->uniqueID();
    rec->fTextSize = paint.getTextSize();
    rec->fPreScaleX = paint.getTextScaleX();
    rec->fPreSkewX  = paint.getTextSkewX();

    bool checkPost2x2 = false;

    if (deviceMatrix) {
        const SkMatrix::TypeMask mask = deviceMatrix->getType();
        if (mask & SkMatrix::kScale_Mask) {
            rec->fPost2x2[0][0] = sk_relax(deviceMatrix->getScaleX());
            rec->fPost2x2[1][1] = sk_relax(deviceMatrix->getScaleY());
            checkPost2x2 = true;
        } else {
            rec->fPost2x2[0][0] = rec->fPost2x2[1][1] = SK_Scalar1;
        }
        if (mask & SkMatrix::kAffine_Mask) {
            rec->fPost2x2[0][1] = sk_relax(deviceMatrix->getSkewX());
            rec->fPost2x2[1][0] = sk_relax(deviceMatrix->getSkewY());
            checkPost2x2 = true;
        } else {
            rec->fPost2x2[0][1] = rec->fPost2x2[1][0] = 0;
        }
    } else {
        rec->fPost2x2[0][0] = rec->fPost2x2[1][1] = SK_Scalar1;
        rec->fPost2x2[0][1] = rec->fPost2x2[1][0] = 0;
    }

    SkPaint::Style  style = paint.getStyle();
    SkScalar        strokeWidth = paint.getStrokeWidth();

    unsigned flags = 0;

    if (paint.isFakeBoldText()) {
#ifdef SK_USE_FREETYPE_EMBOLDEN
        flags |= SkScalerContext::kEmbolden_Flag;
#else
        SkScalar fakeBoldScale = SkScalarInterpFunc(paint.getTextSize(),
                                                    kStdFakeBoldInterpKeys,
                                                    kStdFakeBoldInterpValues,
                                                    kStdFakeBoldInterpLength);
        SkScalar extra = SkScalarMul(paint.getTextSize(), fakeBoldScale);

        if (style == SkPaint::kFill_Style) {
            style = SkPaint::kStrokeAndFill_Style;
            strokeWidth = extra;    // ignore paint's strokeWidth if it was "fill"
        } else {
            strokeWidth += extra;
        }
#endif
    }

    if (paint.isDevKernText()) {
        flags |= SkScalerContext::kDevKernText_Flag;
    }

    if (style != SkPaint::kFill_Style && strokeWidth > 0) {
        rec->fFrameWidth = strokeWidth;
        rec->fMiterLimit = paint.getStrokeMiter();
        rec->fStrokeJoin = SkToU8(paint.getStrokeJoin());

        if (style == SkPaint::kStrokeAndFill_Style) {
            flags |= SkScalerContext::kFrameAndFill_Flag;
        }
    } else {
        rec->fFrameWidth = 0;
        rec->fMiterLimit = 0;
        rec->fStrokeJoin = 0;
    }

    rec->fMaskFormat = SkToU8(compute_mask_format(paint));

    if (SkMask::kLCD16_Format == rec->fMaskFormat) {
        if (too_big_for_lcd(*rec, checkPost2x2)) {
            rec->fMaskFormat = SkMask::kA8_Format;
            flags |= SkScalerContext::kGenA8FromLCD_Flag;
        } else {
            SkPixelGeometry geometry = surfaceProps
                                     ? surfaceProps->pixelGeometry()
                                     : SkSurfacePropsDefaultPixelGeometry();
            switch (geometry) {
                case kUnknown_SkPixelGeometry:
                    // eeek, can't support LCD
                    rec->fMaskFormat = SkMask::kA8_Format;
                    flags |= SkScalerContext::kGenA8FromLCD_Flag;
                    break;
                case kRGB_H_SkPixelGeometry:
                    // our default, do nothing.
                    break;
                case kBGR_H_SkPixelGeometry:
                    flags |= SkScalerContext::kLCD_BGROrder_Flag;
                    break;
                case kRGB_V_SkPixelGeometry:
                    flags |= SkScalerContext::kLCD_Vertical_Flag;
                    break;
                case kBGR_V_SkPixelGeometry:
                    flags |= SkScalerContext::kLCD_Vertical_Flag;
                    flags |= SkScalerContext::kLCD_BGROrder_Flag;
                    break;
            }
        }
    }

    if (paint.isEmbeddedBitmapText()) {
        flags |= SkScalerContext::kEmbeddedBitmapText_Flag;
    }
    if (paint.isSubpixelText()) {
        flags |= SkScalerContext::kSubpixelPositioning_Flag;
    }
    if (paint.isAutohinted()) {
        flags |= SkScalerContext::kForceAutohinting_Flag;
    }
    if (paint.isVerticalText()) {
        flags |= SkScalerContext::kVertical_Flag;
    }
    if (paint.getFlags() & SkPaint::kGenA8FromLCD_Flag) {
        flags |= SkScalerContext::kGenA8FromLCD_Flag;
    }
    rec->fFlags = SkToU16(flags);

    // these modify fFlags, so do them after assigning fFlags
    rec->setHinting(computeHinting(paint));

    rec->setLuminanceColor(paint.computeLuminanceColor());

    //For now always set the paint gamma equal to the device gamma.
    //The math in SkMaskGamma can handle them being different,
    //but it requires superluminous masks when
    //Ex : deviceGamma(x) < paintGamma(x) and x is sufficiently large.
    rec->setDeviceGamma(SK_GAMMA_EXPONENT);
    rec->setPaintGamma(SK_GAMMA_EXPONENT);

#ifdef SK_GAMMA_CONTRAST
    rec->setContrast(SK_GAMMA_CONTRAST);
#else
    /**
     * A value of 0.5 for SK_GAMMA_CONTRAST appears to be a good compromise.
     * With lower values small text appears washed out (though correctly so).
     * With higher values lcd fringing is worse and the smoothing effect of
     * partial coverage is diminished.
     */
    rec->setContrast(0.5f);
#endif

    rec->fReservedAlign = 0;

    /*  Allow the fonthost to modify our rec before we use it as a key into the
        cache. This way if we're asking for something that they will ignore,
        they can modify our rec up front, so we don't create duplicate cache
        entries.
     */
    typeface->onFilterRec(rec);

    // be sure to call PostMakeRec(rec) before you actually use it!
}

/**
 * In order to call cachedDeviceLuminance, cachedPaintLuminance, or
 * cachedMaskGamma the caller must hold the gMaskGammaCacheMutex and continue
 * to hold it until the returned pointer is refed or forgotten.
 */
SK_DECLARE_STATIC_MUTEX(gMaskGammaCacheMutex);

static SkMaskGamma* gLinearMaskGamma = NULL;
static SkMaskGamma* gMaskGamma = NULL;
static SkScalar gContrast = SK_ScalarMin;
static SkScalar gPaintGamma = SK_ScalarMin;
static SkScalar gDeviceGamma = SK_ScalarMin;
/**
 * The caller must hold the gMaskGammaCacheMutex and continue to hold it until
 * the returned SkMaskGamma pointer is refed or forgotten.
 */
static const SkMaskGamma& cachedMaskGamma(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma) {
    gMaskGammaCacheMutex.assertHeld();
    if (0 == contrast && SK_Scalar1 == paintGamma && SK_Scalar1 == deviceGamma) {
        if (NULL == gLinearMaskGamma) {
            gLinearMaskGamma = SkNEW(SkMaskGamma);
        }
        return *gLinearMaskGamma;
    }
    if (gContrast != contrast || gPaintGamma != paintGamma || gDeviceGamma != deviceGamma) {
        SkSafeUnref(gMaskGamma);
        gMaskGamma = SkNEW_ARGS(SkMaskGamma, (contrast, paintGamma, deviceGamma));
        gContrast = contrast;
        gPaintGamma = paintGamma;
        gDeviceGamma = deviceGamma;
    }
    return *gMaskGamma;
}

/*static*/ void SkPaint::Term() {
    SkAutoMutexAcquire ama(gMaskGammaCacheMutex);

    SkSafeUnref(gLinearMaskGamma);
    gLinearMaskGamma = NULL;
    SkSafeUnref(gMaskGamma);
    gMaskGamma = NULL;
    SkDEBUGCODE(gContrast = SK_ScalarMin;)
    SkDEBUGCODE(gPaintGamma = SK_ScalarMin;)
    SkDEBUGCODE(gDeviceGamma = SK_ScalarMin;)
}

/**
 *  We ensure that the rec is self-consistent and efficient (where possible)
 */
void SkScalerContext::PostMakeRec(const SkPaint&, SkScalerContext::Rec* rec) {
    /**
     *  If we're asking for A8, we force the colorlum to be gray, since that
     *  limits the number of unique entries, and the scaler will only look at
     *  the lum of one of them.
     */
    switch (rec->fMaskFormat) {
        case SkMask::kLCD16_Format: {
            // filter down the luminance color to a finite number of bits
            SkColor color = rec->getLuminanceColor();
            rec->setLuminanceColor(SkMaskGamma::CanonicalColor(color));
            break;
        }
        case SkMask::kA8_Format: {
            // filter down the luminance to a single component, since A8 can't
            // use per-component information
            SkColor color = rec->getLuminanceColor();
            U8CPU lum = SkComputeLuminance(SkColorGetR(color),
                                           SkColorGetG(color),
                                           SkColorGetB(color));
            // reduce to our finite number of bits
            color = SkColorSetRGB(lum, lum, lum);
            rec->setLuminanceColor(SkMaskGamma::CanonicalColor(color));
            break;
        }
        case SkMask::kBW_Format:
            // No need to differentiate gamma if we're BW
            rec->ignorePreBlend();
            break;
    }
}

#define MIN_SIZE_FOR_EFFECT_BUFFER  1024

#ifdef SK_DEBUG
    #define TEST_DESC
#endif

static void write_out_descriptor(SkDescriptor* desc, const SkScalerContext::Rec& rec,
                                 const SkPathEffect* pe, SkWriteBuffer* peBuffer,
                                 const SkMaskFilter* mf, SkWriteBuffer* mfBuffer,
                                 const SkRasterizer* ra, SkWriteBuffer* raBuffer,
                                 size_t descSize) {
    desc->init();
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);

    if (pe) {
        add_flattenable(desc, kPathEffect_SkDescriptorTag, peBuffer);
    }
    if (mf) {
        add_flattenable(desc, kMaskFilter_SkDescriptorTag, mfBuffer);
    }
    if (ra) {
        add_flattenable(desc, kRasterizer_SkDescriptorTag, raBuffer);
    }

    desc->computeChecksum();
}

static size_t fill_out_rec(const SkPaint& paint, SkScalerContext::Rec* rec,
                           const SkSurfaceProps* surfaceProps,
                           const SkMatrix* deviceMatrix, bool ignoreGamma,
                           const SkPathEffect* pe, SkWriteBuffer* peBuffer,
                           const SkMaskFilter* mf, SkWriteBuffer* mfBuffer,
                           const SkRasterizer* ra, SkWriteBuffer* raBuffer) {
    SkScalerContext::MakeRec(paint, surfaceProps, deviceMatrix, rec);
    if (ignoreGamma) {
        rec->ignorePreBlend();
    }

    int entryCount = 1;
    size_t descSize = sizeof(*rec);

    if (pe) {
        peBuffer->writeFlattenable(pe);
        descSize += peBuffer->bytesWritten();
        entryCount += 1;
        rec->fMaskFormat = SkMask::kA8_Format;  // force antialiasing when we do the scan conversion
        // seems like we could support kLCD as well at this point...
    }
    if (mf) {
        mfBuffer->writeFlattenable(mf);
        descSize += mfBuffer->bytesWritten();
        entryCount += 1;
        rec->fMaskFormat = SkMask::kA8_Format;   // force antialiasing with maskfilters
        /* Pre-blend is not currently applied to filtered text.
           The primary filter is blur, for which contrast makes no sense,
           and for which the destination guess error is more visible.
           Also, all existing users of blur have calibrated for linear. */
        rec->ignorePreBlend();
    }
    if (ra) {
        raBuffer->writeFlattenable(ra);
        descSize += raBuffer->bytesWritten();
        entryCount += 1;
        rec->fMaskFormat = SkMask::kA8_Format;  // force antialiasing when we do the scan conversion
    }

    ///////////////////////////////////////////////////////////////////////////
    // Now that we're done tweaking the rec, call the PostMakeRec cleanup
    SkScalerContext::PostMakeRec(paint, rec);

    descSize += SkDescriptor::ComputeOverhead(entryCount);
    return descSize;
}

#ifdef TEST_DESC
static void test_desc(const SkScalerContext::Rec& rec,
                      const SkPathEffect* pe, SkWriteBuffer* peBuffer,
                      const SkMaskFilter* mf, SkWriteBuffer* mfBuffer,
                      const SkRasterizer* ra, SkWriteBuffer* raBuffer,
                      const SkDescriptor* desc, size_t descSize) {
    // Check that we completely write the bytes in desc (our key), and that
    // there are no uninitialized bytes. If there were, then we would get
    // false-misses (or worse, false-hits) in our fontcache.
    //
    // We do this buy filling 2 others, one with 0s and the other with 1s
    // and create those, and then check that all 3 are identical.
    SkAutoDescriptor    ad1(descSize);
    SkAutoDescriptor    ad2(descSize);
    SkDescriptor*       desc1 = ad1.getDesc();
    SkDescriptor*       desc2 = ad2.getDesc();

    memset(desc1, 0x00, descSize);
    memset(desc2, 0xFF, descSize);

    desc1->init();
    desc2->init();
    desc1->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);
    desc2->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);

    if (pe) {
        add_flattenable(desc1, kPathEffect_SkDescriptorTag, peBuffer);
        add_flattenable(desc2, kPathEffect_SkDescriptorTag, peBuffer);
    }
    if (mf) {
        add_flattenable(desc1, kMaskFilter_SkDescriptorTag, mfBuffer);
        add_flattenable(desc2, kMaskFilter_SkDescriptorTag, mfBuffer);
    }
    if (ra) {
        add_flattenable(desc1, kRasterizer_SkDescriptorTag, raBuffer);
        add_flattenable(desc2, kRasterizer_SkDescriptorTag, raBuffer);
    }

    SkASSERT(descSize == desc1->getLength());
    SkASSERT(descSize == desc2->getLength());
    desc1->computeChecksum();
    desc2->computeChecksum();
    SkASSERT(!memcmp(desc, desc1, descSize));
    SkASSERT(!memcmp(desc, desc2, descSize));
}
#endif

/* see the note on ignoreGamma on descriptorProc */
void SkPaint::getScalerContextDescriptor(SkAutoDescriptor* ad,
                                         const SkSurfaceProps& surfaceProps,
                                         const SkMatrix* deviceMatrix, bool ignoreGamma) const {
    SkScalerContext::Rec    rec;

    SkPathEffect*   pe = this->getPathEffect();
    SkMaskFilter*   mf = this->getMaskFilter();
    SkRasterizer*   ra = this->getRasterizer();

    SkWriteBuffer   peBuffer, mfBuffer, raBuffer;
    size_t descSize = fill_out_rec(*this, &rec, &surfaceProps, deviceMatrix, ignoreGamma,
                                   pe, &peBuffer, mf, &mfBuffer, ra, &raBuffer);

    ad->reset(descSize);
    SkDescriptor* desc = ad->getDesc();

    write_out_descriptor(desc, rec, pe, &peBuffer, mf, &mfBuffer, ra, &raBuffer, descSize);

    SkASSERT(descSize == desc->getLength());

#ifdef TEST_DESC
    test_desc(rec, pe, &peBuffer, mf, &mfBuffer, ra, &raBuffer, desc, descSize);
#endif
}

/*
 *  ignoreGamma tells us that the caller just wants metrics that are unaffected
 *  by gamma correction, so we set the rec to ignore preblend: i.e. gamma = 1,
 *  contrast = 0, luminanceColor = transparent black.
 */
void SkPaint::descriptorProc(const SkSurfaceProps* surfaceProps,
                             const SkMatrix* deviceMatrix,
                             void (*proc)(SkTypeface*, const SkDescriptor*, void*),
                             void* context, bool ignoreGamma) const {
    SkScalerContext::Rec    rec;

    SkPathEffect*   pe = this->getPathEffect();
    SkMaskFilter*   mf = this->getMaskFilter();
    SkRasterizer*   ra = this->getRasterizer();

    SkWriteBuffer   peBuffer, mfBuffer, raBuffer;
    size_t descSize = fill_out_rec(*this, &rec, surfaceProps, deviceMatrix, ignoreGamma,
                                   pe, &peBuffer, mf, &mfBuffer, ra, &raBuffer);

    SkAutoDescriptor    ad(descSize);
    SkDescriptor*       desc = ad.getDesc();

    write_out_descriptor(desc, rec, pe, &peBuffer, mf, &mfBuffer, ra, &raBuffer, descSize);

    SkASSERT(descSize == desc->getLength());

#ifdef TEST_DESC
    test_desc(rec, pe, &peBuffer, mf, &mfBuffer, ra, &raBuffer, desc, descSize);
#endif

    proc(fTypeface, desc, context);
}

SkGlyphCache* SkPaint::detachCache(const SkSurfaceProps* surfaceProps,
                                   const SkMatrix* deviceMatrix,
                                   bool ignoreGamma) const {
    SkGlyphCache* cache;
    this->descriptorProc(surfaceProps, deviceMatrix, DetachDescProc, &cache, ignoreGamma);
    return cache;
}

/**
 * Expands fDeviceGamma, fPaintGamma, fContrast, and fLumBits into a mask pre-blend.
 */
//static
SkMaskGamma::PreBlend SkScalerContext::GetMaskPreBlend(const SkScalerContext::Rec& rec) {
    SkAutoMutexAcquire ama(gMaskGammaCacheMutex);
    const SkMaskGamma& maskGamma = cachedMaskGamma(rec.getContrast(),
                                                   rec.getPaintGamma(),
                                                   rec.getDeviceGamma());
    return maskGamma.preBlend(rec.getLuminanceColor());
}

size_t SkScalerContext::GetGammaLUTSize(SkScalar contrast, SkScalar paintGamma,
                                        SkScalar deviceGamma, int* width, int* height) {
    SkAutoMutexAcquire ama(gMaskGammaCacheMutex);
    const SkMaskGamma& maskGamma = cachedMaskGamma(contrast,
                                                   paintGamma,
                                                   deviceGamma);

    maskGamma.getGammaTableDimensions(width, height);
    size_t size = (*width)*(*height)*sizeof(uint8_t);

    return size;
}

void SkScalerContext::GetGammaLUTData(SkScalar contrast, SkScalar paintGamma, SkScalar deviceGamma,
                                      void* data) {
    SkAutoMutexAcquire ama(gMaskGammaCacheMutex);
    const SkMaskGamma& maskGamma = cachedMaskGamma(contrast,
                                                   paintGamma,
                                                   deviceGamma);
    int width, height;
    maskGamma.getGammaTableDimensions(&width, &height);
    size_t size = width*height*sizeof(uint8_t);
    const uint8_t* gammaTables = maskGamma.getGammaTables();
    memcpy(data, gammaTables, size);
}


///////////////////////////////////////////////////////////////////////////////

#include "SkStream.h"

static uintptr_t asint(const void* p) {
    return reinterpret_cast<uintptr_t>(p);
}

union Scalar32 {
    SkScalar    fScalar;
    uint32_t    f32;
};

static uint32_t* write_scalar(uint32_t* ptr, SkScalar value) {
    SkASSERT(sizeof(SkScalar) == sizeof(uint32_t));
    Scalar32 tmp;
    tmp.fScalar = value;
    *ptr = tmp.f32;
    return ptr + 1;
}

static SkScalar read_scalar(const uint32_t*& ptr) {
    SkASSERT(sizeof(SkScalar) == sizeof(uint32_t));
    Scalar32 tmp;
    tmp.f32 = *ptr++;
    return tmp.fScalar;
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

// The size of a flat paint's POD fields
static const uint32_t kPODPaintSize =   5 * sizeof(SkScalar) +
                                        1 * sizeof(SkColor) +
                                        1 * sizeof(uint16_t) +
                                        6 * sizeof(uint8_t);

/*  To save space/time, we analyze the paint, and write a truncated version of
    it if there are not tricky elements like shaders, etc.
 */
void SkPaint::flatten(SkWriteBuffer& buffer) const {
    uint8_t flatFlags = 0;
    if (this->getTypeface()) {
        flatFlags |= kHasTypeface_FlatFlag;
    }
    if (asint(this->getPathEffect()) |
        asint(this->getShader()) |
        asint(this->getXfermode()) |
        asint(this->getMaskFilter()) |
        asint(this->getColorFilter()) |
        asint(this->getRasterizer()) |
        asint(this->getLooper()) |
        asint(this->getAnnotation()) |
        asint(this->getImageFilter())) {
        flatFlags |= kHasEffects_FlatFlag;
    }

    SkASSERT(SkAlign4(kPODPaintSize) == kPODPaintSize);
    uint32_t* ptr = buffer.reserve(kPODPaintSize);

    ptr = write_scalar(ptr, this->getTextSize());
    ptr = write_scalar(ptr, this->getTextScaleX());
    ptr = write_scalar(ptr, this->getTextSkewX());
    ptr = write_scalar(ptr, this->getStrokeWidth());
    ptr = write_scalar(ptr, this->getStrokeMiter());
    *ptr++ = this->getColor();

    *ptr++ = pack_paint_flags(this->getFlags(), this->getHinting(), this->getTextAlign(),
                              this->getFilterQuality(), flatFlags);
    *ptr++ = pack_4(this->getStrokeCap(), this->getStrokeJoin(),
                    this->getStyle(), this->getTextEncoding());

    // now we're done with ptr and the (pre)reserved space. If we need to write
    // additional fields, use the buffer directly
    if (flatFlags & kHasTypeface_FlatFlag) {
        buffer.writeTypeface(this->getTypeface());
    }
    if (flatFlags & kHasEffects_FlatFlag) {
        buffer.writeFlattenable(this->getPathEffect());
        buffer.writeFlattenable(this->getShader());
        buffer.writeFlattenable(this->getXfermode());
        buffer.writeFlattenable(this->getMaskFilter());
        buffer.writeFlattenable(this->getColorFilter());
        buffer.writeFlattenable(this->getRasterizer());
        buffer.writeFlattenable(this->getLooper());
        buffer.writeFlattenable(this->getImageFilter());

        if (fAnnotation) {
            buffer.writeBool(true);
            fAnnotation->writeToBuffer(buffer);
        } else {
            buffer.writeBool(false);
        }
    }
}

void SkPaint::unflatten(SkReadBuffer& buffer) {
    SkASSERT(SkAlign4(kPODPaintSize) == kPODPaintSize);
    const void* podData = buffer.skip(kPODPaintSize);
    const uint32_t* pod = reinterpret_cast<const uint32_t*>(podData);

    // the order we read must match the order we wrote in flatten()
    this->setTextSize(read_scalar(pod));
    this->setTextScaleX(read_scalar(pod));
    this->setTextSkewX(read_scalar(pod));
    this->setStrokeWidth(read_scalar(pod));
    this->setStrokeMiter(read_scalar(pod));
    this->setColor(*pod++);

    unsigned flatFlags = unpack_paint_flags(this, *pod++);

    uint32_t tmp = *pod++;
    this->setStrokeCap(static_cast<Cap>((tmp >> 24) & 0xFF));
    this->setStrokeJoin(static_cast<Join>((tmp >> 16) & 0xFF));
    this->setStyle(static_cast<Style>((tmp >> 8) & 0xFF));
    this->setTextEncoding(static_cast<TextEncoding>((tmp >> 0) & 0xFF));

    if (flatFlags & kHasTypeface_FlatFlag) {
        this->setTypeface(buffer.readTypeface());
    } else {
        this->setTypeface(NULL);
    }

    if (flatFlags & kHasEffects_FlatFlag) {
        SkSafeUnref(this->setPathEffect(buffer.readPathEffect()));
        SkSafeUnref(this->setShader(buffer.readShader()));
        SkSafeUnref(this->setXfermode(buffer.readXfermode()));
        SkSafeUnref(this->setMaskFilter(buffer.readMaskFilter()));
        SkSafeUnref(this->setColorFilter(buffer.readColorFilter()));
        SkSafeUnref(this->setRasterizer(buffer.readRasterizer()));
        SkSafeUnref(this->setLooper(buffer.readDrawLooper()));
        SkSafeUnref(this->setImageFilter(buffer.readImageFilter()));

        if (buffer.readBool()) {
            this->setAnnotation(SkAnnotation::Create(buffer))->unref();
        }
    } else {
        this->setPathEffect(NULL);
        this->setShader(NULL);
        this->setXfermode(NULL);
        this->setMaskFilter(NULL);
        this->setColorFilter(NULL);
        this->setRasterizer(NULL);
        this->setLooper(NULL);
        this->setImageFilter(NULL);
    }
}

///////////////////////////////////////////////////////////////////////////////

SkShader* SkPaint::setShader(SkShader* shader) {
    SkRefCnt_SafeAssign(fShader, shader);
    return shader;
}

SkColorFilter* SkPaint::setColorFilter(SkColorFilter* filter) {
    SkRefCnt_SafeAssign(fColorFilter, filter);
    return filter;
}

SkXfermode* SkPaint::setXfermode(SkXfermode* mode) {
    SkRefCnt_SafeAssign(fXfermode, mode);
    return mode;
}

SkXfermode* SkPaint::setXfermodeMode(SkXfermode::Mode mode) {
    SkSafeUnref(fXfermode);
    fXfermode = SkXfermode::Create(mode);
    return fXfermode;
}

SkPathEffect* SkPaint::setPathEffect(SkPathEffect* effect) {
    SkRefCnt_SafeAssign(fPathEffect, effect);
    return effect;
}

SkMaskFilter* SkPaint::setMaskFilter(SkMaskFilter* filter) {
    SkRefCnt_SafeAssign(fMaskFilter, filter);
    return filter;
}

///////////////////////////////////////////////////////////////////////////////

bool SkPaint::getFillPath(const SkPath& src, SkPath* dst, const SkRect* cullRect,
                          SkScalar resScale) const {
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
    return !rec.isHairlineStyle();
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

    if (kFill_Style != style) {
        // since we're stroked, outset the rect by the radius (and join type)
        SkScalar radius = SkScalarHalf(this->getStrokeWidth());
        if (0 == radius) {  // hairline
            radius = SK_Scalar1;
        } else if (this->getStrokeJoin() == SkPaint::kMiter_Join) {
            SkScalar scale = this->getStrokeMiter();
            if (scale > SK_Scalar1) {
                radius = SkScalarMul(radius, scale);
            }
        }
        storage->set(src->fLeft - radius, src->fTop - radius,
                     src->fRight + radius, src->fBottom + radius);
    } else {
        *storage = *src;
    }

    if (this->getMaskFilter()) {
        this->getMaskFilter()->computeFastBounds(*storage, storage);
    }

    if (this->getImageFilter()) {
        this->getImageFilter()->computeFastBounds(*storage, storage);
    }

    return *storage;
}

#ifndef SK_IGNORE_TO_STRING

void SkPaint::toString(SkString* str) const {
    str->append("<dl><dt>SkPaint:</dt><dd><dl>");

    SkTypeface* typeface = this->getTypeface();
    if (typeface) {
        SkDynamicMemoryWStream ostream;
        typeface->serialize(&ostream);
        SkAutoTDelete<SkStreamAsset> istream(ostream.detachAsStream());
        SkFontDescriptor descriptor(istream);

        str->append("<dt>Font Family Name:</dt><dd>");
        str->append(descriptor.getFamilyName());
        str->append("</dd><dt>Font Full Name:</dt><dd>");
        str->append(descriptor.getFullName());
        str->append("</dd><dt>Font PS Name:</dt><dd>");
        str->append(descriptor.getPostscriptName());
        str->append("</dd>");
    }

    str->append("<dt>TextSize:</dt><dd>");
    str->appendScalar(this->getTextSize());
    str->append("</dd>");

    str->append("<dt>TextScaleX:</dt><dd>");
    str->appendScalar(this->getTextScaleX());
    str->append("</dd>");

    str->append("<dt>TextSkewX:</dt><dd>");
    str->appendScalar(this->getTextSkewX());
    str->append("</dd>");

    SkPathEffect* pathEffect = this->getPathEffect();
    if (pathEffect) {
        str->append("<dt>PathEffect:</dt><dd>");
        pathEffect->toString(str);
        str->append("</dd>");
    }

    SkShader* shader = this->getShader();
    if (shader) {
        str->append("<dt>Shader:</dt><dd>");
        shader->toString(str);
        str->append("</dd>");
    }

    SkXfermode* xfer = this->getXfermode();
    if (xfer) {
        str->append("<dt>Xfermode:</dt><dd>");
        xfer->toString(str);
        str->append("</dd>");
    }

    SkMaskFilter* maskFilter = this->getMaskFilter();
    if (maskFilter) {
        str->append("<dt>MaskFilter:</dt><dd>");
        maskFilter->toString(str);
        str->append("</dd>");
    }

    SkColorFilter* colorFilter = this->getColorFilter();
    if (colorFilter) {
        str->append("<dt>ColorFilter:</dt><dd>");
        colorFilter->toString(str);
        str->append("</dd>");
    }

    SkRasterizer* rasterizer = this->getRasterizer();
    if (rasterizer) {
        str->append("<dt>Rasterizer:</dt><dd>");
        str->append("</dd>");
    }

    SkDrawLooper* looper = this->getLooper();
    if (looper) {
        str->append("<dt>DrawLooper:</dt><dd>");
        looper->toString(str);
        str->append("</dd>");
    }

    SkImageFilter* imageFilter = this->getImageFilter();
    if (imageFilter) {
        str->append("<dt>ImageFilter:</dt><dd>");
        imageFilter->toString(str);
        str->append("</dd>");
    }

    SkAnnotation* annotation = this->getAnnotation();
    if (annotation) {
        str->append("<dt>Annotation:</dt><dd>");
        str->append("</dd>");
    }

    str->append("<dt>Color:</dt><dd>0x");
    SkColor color = this->getColor();
    str->appendHex(color);
    str->append("</dd>");

    str->append("<dt>Stroke Width:</dt><dd>");
    str->appendScalar(this->getStrokeWidth());
    str->append("</dd>");

    str->append("<dt>Stroke Miter:</dt><dd>");
    str->appendScalar(this->getStrokeMiter());
    str->append("</dd>");

    str->append("<dt>Flags:</dt><dd>(");
    if (this->getFlags()) {
        bool needSeparator = false;
        SkAddFlagToString(str, this->isAntiAlias(), "AntiAlias", &needSeparator);
        SkAddFlagToString(str, this->isDither(), "Dither", &needSeparator);
        SkAddFlagToString(str, this->isUnderlineText(), "UnderlineText", &needSeparator);
        SkAddFlagToString(str, this->isStrikeThruText(), "StrikeThruText", &needSeparator);
        SkAddFlagToString(str, this->isFakeBoldText(), "FakeBoldText", &needSeparator);
        SkAddFlagToString(str, this->isLinearText(), "LinearText", &needSeparator);
        SkAddFlagToString(str, this->isSubpixelText(), "SubpixelText", &needSeparator);
        SkAddFlagToString(str, this->isDevKernText(), "DevKernText", &needSeparator);
        SkAddFlagToString(str, this->isLCDRenderText(), "LCDRenderText", &needSeparator);
        SkAddFlagToString(str, this->isEmbeddedBitmapText(),
                          "EmbeddedBitmapText", &needSeparator);
        SkAddFlagToString(str, this->isAutohinted(), "Autohinted", &needSeparator);
        SkAddFlagToString(str, this->isVerticalText(), "VerticalText", &needSeparator);
        SkAddFlagToString(str, SkToBool(this->getFlags() & SkPaint::kGenA8FromLCD_Flag),
                          "GenA8FromLCD", &needSeparator);
    } else {
        str->append("None");
    }
    str->append(")</dd>");

    str->append("<dt>FilterLevel:</dt><dd>");
    static const char* gFilterQualityStrings[] = { "None", "Low", "Medium", "High" };
    str->append(gFilterQualityStrings[this->getFilterQuality()]);
    str->append("</dd>");

    str->append("<dt>TextAlign:</dt><dd>");
    static const char* gTextAlignStrings[SkPaint::kAlignCount] = { "Left", "Center", "Right" };
    str->append(gTextAlignStrings[this->getTextAlign()]);
    str->append("</dd>");

    str->append("<dt>CapType:</dt><dd>");
    static const char* gStrokeCapStrings[SkPaint::kCapCount] = { "Butt", "Round", "Square" };
    str->append(gStrokeCapStrings[this->getStrokeCap()]);
    str->append("</dd>");

    str->append("<dt>JoinType:</dt><dd>");
    static const char* gJoinStrings[SkPaint::kJoinCount] = { "Miter", "Round", "Bevel" };
    str->append(gJoinStrings[this->getStrokeJoin()]);
    str->append("</dd>");

    str->append("<dt>Style:</dt><dd>");
    static const char* gStyleStrings[SkPaint::kStyleCount] = { "Fill", "Stroke", "StrokeAndFill" };
    str->append(gStyleStrings[this->getStyle()]);
    str->append("</dd>");

    str->append("<dt>TextEncoding:</dt><dd>");
    static const char* gTextEncodingStrings[] = { "UTF8", "UTF16", "UTF32", "GlyphID" };
    str->append(gTextEncodingStrings[this->getTextEncoding()]);
    str->append("</dd>");

    str->append("<dt>Hinting:</dt><dd>");
    static const char* gHintingStrings[] = { "None", "Slight", "Normal", "Full" };
    str->append(gHintingStrings[this->getHinting()]);
    str->append("</dd>");

    str->append("</dd></dl></dl>");
}
#endif

///////////////////////////////////////////////////////////////////////////////

static bool has_thick_frame(const SkPaint& paint) {
    return  paint.getStrokeWidth() > 0 &&
            paint.getStyle() != SkPaint::kFill_Style;
}

SkTextToPathIter::SkTextToPathIter(const char text[], size_t length,
                                   const SkPaint& paint,
                                   bool applyStrokeAndPathEffects)
    : fPaint(paint) {
    fGlyphCacheProc = paint.getMeasureCacheProc(true);

    fPaint.setLinearText(true);
    fPaint.setMaskFilter(NULL);   // don't want this affecting our path-cache lookup

    if (fPaint.getPathEffect() == NULL && !has_thick_frame(fPaint)) {
        applyStrokeAndPathEffects = false;
    }

    // can't use our canonical size if we need to apply patheffects
    if (fPaint.getPathEffect() == NULL) {
        fPaint.setTextSize(SkIntToScalar(SkPaint::kCanonicalTextSizeForPaths));
        fScale = paint.getTextSize() / SkPaint::kCanonicalTextSizeForPaths;
        if (has_thick_frame(fPaint)) {
            fPaint.setStrokeWidth(fPaint.getStrokeWidth() / fScale);
        }
    } else {
        fScale = SK_Scalar1;
    }

    if (!applyStrokeAndPathEffects) {
        fPaint.setStyle(SkPaint::kFill_Style);
        fPaint.setPathEffect(NULL);
    }

    fCache = fPaint.detachCache(NULL, NULL, false);

    SkPaint::Style  style = SkPaint::kFill_Style;
    SkPathEffect*   pe = NULL;

    if (!applyStrokeAndPathEffects) {
        style = paint.getStyle();   // restore
        pe = paint.getPathEffect();     // restore
    }
    fPaint.setStyle(style);
    fPaint.setPathEffect(pe);
    fPaint.setMaskFilter(paint.getMaskFilter());    // restore

    // now compute fXOffset if needed

    SkScalar xOffset = 0;
    if (paint.getTextAlign() != SkPaint::kLeft_Align) { // need to measure first
        int      count;
        SkScalar width = SkScalarMul(fPaint.measure_text(fCache, text, length,
                                                         &count, NULL), fScale);
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

SkTextToPathIter::~SkTextToPathIter() {
    SkGlyphCache::AttachCache(fCache);
}

bool SkTextToPathIter::next(const SkPath** path, SkScalar* xpos) {
    if (fText < fStop) {
        const SkGlyph& glyph = fGlyphCacheProc(fCache, &fText);

        fXPos += SkScalarMul(SkFixedToScalar(fPrevAdvance + fAutoKern.adjust(glyph)), fScale);
        fPrevAdvance = advance(glyph, fXYIndex);   // + fPaint.getTextTracking();

        if (glyph.fWidth) {
            if (path) {
                *path = fCache->findPath(glyph);
            }
        } else {
            if (path) {
                *path = NULL;
            }
        }
        if (xpos) {
            *xpos = fXPos;
        }
        return true;
    }
    return false;
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
    return imf != NULL;
}

bool SkPaint::nothingToDraw() const {
    if (fLooper) {
        return false;
    }
    SkXfermode::Mode mode;
    if (SkXfermode::AsMode(fXfermode, &mode)) {
        switch (mode) {
            case SkXfermode::kSrcOver_Mode:
            case SkXfermode::kSrcATop_Mode:
            case SkXfermode::kDstOut_Mode:
            case SkXfermode::kDstOver_Mode:
            case SkXfermode::kPlus_Mode:
                if (0 == this->getAlpha()) {
                    return !affects_alpha(fColorFilter) && !affects_alpha(fImageFilter);
                }
                break;
            case SkXfermode::kDst_Mode:
                return true;
            default:
                break;
        }
    }
    return false;
}

uint32_t SkPaint::getHash() const {
    // We're going to hash 10 pointers and 7 32-bit values, finishing up with fBitfields,
    // so fBitfields should be 10 pointers and 6 32-bit values from the start.
    SK_COMPILE_ASSERT(offsetof(SkPaint, fBitfields) == 10 * sizeof(void*) + 6 * sizeof(uint32_t),
                      SkPaint_notPackedTightly);
    return SkChecksum::Murmur3(reinterpret_cast<const uint32_t*>(this),
                               offsetof(SkPaint, fBitfields) + sizeof(fBitfields));
}
