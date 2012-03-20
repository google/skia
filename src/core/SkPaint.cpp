
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPaint.h"
#include "SkColorFilter.h"
#include "SkFontHost.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkScalar.h"
#include "SkScalerContext.h"
#include "SkStroke.h"
#include "SkTextFormatParams.h"
#include "SkTypeface.h"
#include "SkXfermode.h"
#include "SkAutoKern.h"
#include "SkGlyphCache.h"
#include "SkPaintDefaults.h"

// define this to get a printf for out-of-range parameter in setters
// e.g. setTextSize(-1)
//#define SK_REPORT_API_RANGE_CHECK

#ifdef SK_BUILD_FOR_ANDROID
#define GEN_ID_INC                  fGenerationID++
#define GEN_ID_INC_EVAL(expression) if (expression) { fGenerationID++; }
#else
#define GEN_ID_INC
#define GEN_ID_INC_EVAL(expression)
#endif

SkPaint::SkPaint() {
    // since we may have padding, we zero everything so that our memcmp() call
    // in operator== will work correctly.
    // with this, we can skip 0 and null individual initializations
    sk_bzero(this, sizeof(*this));

#if 0   // not needed with the bzero call above
    fTypeface   = NULL;
    fTextSkewX  = 0;
    fPathEffect  = NULL;
    fShader      = NULL;
    fXfermode    = NULL;
    fMaskFilter  = NULL;
    fColorFilter = NULL;
    fRasterizer  = NULL;
    fLooper      = NULL;
    fImageFilter = NULL;
    fWidth      = 0;
#endif

    fTextSize   = SkPaintDefaults_TextSize;
    fTextScaleX = SK_Scalar1;
    fColor      = SK_ColorBLACK;
    fMiterLimit = SkPaintDefaults_MiterLimit;
    fFlags      = SkPaintDefaults_Flags;
    fCapType    = kDefault_Cap;
    fJoinType   = kDefault_Join;
    fTextAlign  = kLeft_Align;
    fStyle      = kFill_Style;
    fTextEncoding = kUTF8_TextEncoding;
    fHinting    = SkPaintDefaults_Hinting;
#ifdef SK_BUILD_FOR_ANDROID
    fGenerationID = 0;
#endif
}

SkPaint::SkPaint(const SkPaint& src) {
    memcpy(this, &src, sizeof(src));

    SkSafeRef(fTypeface);
    SkSafeRef(fPathEffect);
    SkSafeRef(fShader);
    SkSafeRef(fXfermode);
    SkSafeRef(fMaskFilter);
    SkSafeRef(fColorFilter);
    SkSafeRef(fRasterizer);
    SkSafeRef(fLooper);
    SkSafeRef(fImageFilter);
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
}

SkPaint& SkPaint::operator=(const SkPaint& src) {
    SkASSERT(&src);

    SkSafeRef(src.fTypeface);
    SkSafeRef(src.fPathEffect);
    SkSafeRef(src.fShader);
    SkSafeRef(src.fXfermode);
    SkSafeRef(src.fMaskFilter);
    SkSafeRef(src.fColorFilter);
    SkSafeRef(src.fRasterizer);
    SkSafeRef(src.fLooper);
    SkSafeRef(src.fImageFilter);

    SkSafeUnref(fTypeface);
    SkSafeUnref(fPathEffect);
    SkSafeUnref(fShader);
    SkSafeUnref(fXfermode);
    SkSafeUnref(fMaskFilter);
    SkSafeUnref(fColorFilter);
    SkSafeUnref(fRasterizer);
    SkSafeUnref(fLooper);
    SkSafeUnref(fImageFilter);

#ifdef SK_BUILD_FOR_ANDROID
    uint32_t oldGenerationID = fGenerationID;
#endif
    memcpy(this, &src, sizeof(src));
#ifdef SK_BUILD_FOR_ANDROID
    fGenerationID = oldGenerationID + 1;
#endif

    return *this;
}

bool operator==(const SkPaint& a, const SkPaint& b) {
#ifdef SK_BUILD_FOR_ANDROID
    //assumes that fGenerationID is the last field in the struct
    return !memcmp(&a, &b, SK_OFFSETOF(SkPaint, fGenerationID));
#else
    return !memcmp(&a, &b, sizeof(a));
#endif
}

void SkPaint::reset() {
    SkPaint init;

#ifdef SK_BUILD_FOR_ANDROID
    uint32_t oldGenerationID = fGenerationID;
#endif
    *this = init;
#ifdef SK_BUILD_FOR_ANDROID
    fGenerationID = oldGenerationID + 1;
#endif
}

#ifdef SK_BUILD_FOR_ANDROID
uint32_t SkPaint::getGenerationID() const {
    return fGenerationID;
}
#endif

#ifdef SK_BUILD_FOR_ANDROID
unsigned SkPaint::getBaseGlyphCount(SkUnichar text) const {
    SkAutoGlyphCache autoCache(*this, NULL);
    SkGlyphCache* cache = autoCache.getCache();
    return cache->getBaseGlyphCount(text);
}
#endif

void SkPaint::setHinting(Hinting hintingLevel) {
    GEN_ID_INC_EVAL((unsigned) hintingLevel != fHinting);
    fHinting = hintingLevel;
}

void SkPaint::setFlags(uint32_t flags) {
    GEN_ID_INC_EVAL(fFlags != flags);
    fFlags = flags;
}

void SkPaint::setAntiAlias(bool doAA) {
    GEN_ID_INC_EVAL(doAA != isAntiAlias());
    this->setFlags(SkSetClearMask(fFlags, doAA, kAntiAlias_Flag));
}

void SkPaint::setDither(bool doDither) {
    GEN_ID_INC_EVAL(doDither != isDither());
    this->setFlags(SkSetClearMask(fFlags, doDither, kDither_Flag));
}

void SkPaint::setSubpixelText(bool doSubpixel) {
    GEN_ID_INC_EVAL(doSubpixel != isSubpixelText());
    this->setFlags(SkSetClearMask(fFlags, doSubpixel, kSubpixelText_Flag));
}

void SkPaint::setLCDRenderText(bool doLCDRender) {
    GEN_ID_INC_EVAL(doLCDRender != isLCDRenderText());
    this->setFlags(SkSetClearMask(fFlags, doLCDRender, kLCDRenderText_Flag));
}

void SkPaint::setEmbeddedBitmapText(bool doEmbeddedBitmapText) {
    GEN_ID_INC_EVAL(doEmbeddedBitmapText != isEmbeddedBitmapText());
    this->setFlags(SkSetClearMask(fFlags, doEmbeddedBitmapText, kEmbeddedBitmapText_Flag));
}

void SkPaint::setAutohinted(bool useAutohinter) {
    GEN_ID_INC_EVAL(useAutohinter != isAutohinted());
    this->setFlags(SkSetClearMask(fFlags, useAutohinter, kAutoHinting_Flag));
}

void SkPaint::setLinearText(bool doLinearText) {
    GEN_ID_INC_EVAL(doLinearText != isLinearText());
    this->setFlags(SkSetClearMask(fFlags, doLinearText, kLinearText_Flag));
}

void SkPaint::setVerticalText(bool doVertical) {
    GEN_ID_INC_EVAL(doVertical != isVerticalText());
    this->setFlags(SkSetClearMask(fFlags, doVertical, kVerticalText_Flag));
}

void SkPaint::setUnderlineText(bool doUnderline) {
    GEN_ID_INC_EVAL(doUnderline != isUnderlineText());
    this->setFlags(SkSetClearMask(fFlags, doUnderline, kUnderlineText_Flag));
}

void SkPaint::setStrikeThruText(bool doStrikeThru) {
    GEN_ID_INC_EVAL(doStrikeThru != isStrikeThruText());
    this->setFlags(SkSetClearMask(fFlags, doStrikeThru, kStrikeThruText_Flag));
}

void SkPaint::setFakeBoldText(bool doFakeBold) {
    GEN_ID_INC_EVAL(doFakeBold != isFakeBoldText());
    this->setFlags(SkSetClearMask(fFlags, doFakeBold, kFakeBoldText_Flag));
}

void SkPaint::setDevKernText(bool doDevKern) {
    GEN_ID_INC_EVAL(doDevKern != isDevKernText());
    this->setFlags(SkSetClearMask(fFlags, doDevKern, kDevKernText_Flag));
}

void SkPaint::setFilterBitmap(bool doFilter) {
    GEN_ID_INC_EVAL(doFilter != isFilterBitmap());
    this->setFlags(SkSetClearMask(fFlags, doFilter, kFilterBitmap_Flag));
}

void SkPaint::setStyle(Style style) {
    if ((unsigned)style < kStyleCount) {
        GEN_ID_INC_EVAL((unsigned)style != fStyle);
        fStyle = style;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStyle(%d) out of range\n", style);
#endif
    }
}

void SkPaint::setColor(SkColor color) {
    GEN_ID_INC_EVAL(color != fColor);
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
        GEN_ID_INC_EVAL(width != fWidth);
        fWidth = width;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeWidth() called with negative value\n");
#endif
    }
}

void SkPaint::setStrokeMiter(SkScalar limit) {
    if (limit >= 0) {
        GEN_ID_INC_EVAL(limit != fMiterLimit);
        fMiterLimit = limit;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeMiter() called with negative value\n");
#endif
    }
}

void SkPaint::setStrokeCap(Cap ct) {
    if ((unsigned)ct < kCapCount) {
        GEN_ID_INC_EVAL((unsigned)ct != fCapType);
        fCapType = SkToU8(ct);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeCap(%d) out of range\n", ct);
#endif
    }
}

void SkPaint::setStrokeJoin(Join jt) {
    if ((unsigned)jt < kJoinCount) {
        GEN_ID_INC_EVAL((unsigned)jt != fJoinType);
        fJoinType = SkToU8(jt);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setStrokeJoin(%d) out of range\n", jt);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkPaint::setTextAlign(Align align) {
    if ((unsigned)align < kAlignCount) {
        GEN_ID_INC_EVAL((unsigned)align != fTextAlign);
        fTextAlign = SkToU8(align);
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setTextAlign(%d) out of range\n", align);
#endif
    }
}

void SkPaint::setTextSize(SkScalar ts) {
    if (ts >= 0) {
        GEN_ID_INC_EVAL(ts != fTextSize);
        fTextSize = ts;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setTextSize() called with negative value\n");
#endif
    }
}

void SkPaint::setTextScaleX(SkScalar scaleX) {
    GEN_ID_INC_EVAL(scaleX != fTextScaleX);
    fTextScaleX = scaleX;
}

void SkPaint::setTextSkewX(SkScalar skewX) {
    GEN_ID_INC_EVAL(skewX != fTextSkewX);
    fTextSkewX = skewX;
}

void SkPaint::setTextEncoding(TextEncoding encoding) {
    if ((unsigned)encoding <= kGlyphID_TextEncoding) {
        GEN_ID_INC_EVAL((unsigned)encoding != fTextEncoding);
        fTextEncoding = encoding;
    } else {
#ifdef SK_REPORT_API_RANGE_CHECK
        SkDebugf("SkPaint::setTextEncoding(%d) out of range\n", encoding);
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////

SkTypeface* SkPaint::setTypeface(SkTypeface* font) {
    SkRefCnt_SafeAssign(fTypeface, font);
    GEN_ID_INC;
    return font;
}

SkRasterizer* SkPaint::setRasterizer(SkRasterizer* r) {
    SkRefCnt_SafeAssign(fRasterizer, r);
    GEN_ID_INC;
    return r;
}

SkDrawLooper* SkPaint::setLooper(SkDrawLooper* looper) {
    SkRefCnt_SafeAssign(fLooper, looper);
    GEN_ID_INC;
    return looper;
}

SkImageFilter* SkPaint::setImageFilter(SkImageFilter* imageFilter) {
    SkRefCnt_SafeAssign(fImageFilter, imageFilter);
    GEN_ID_INC;
    return imageFilter;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGlyphCache.h"
#include "SkUtils.h"

static void DetachDescProc(const SkDescriptor* desc, void* context) {
    *((SkGlyphCache**)context) = SkGlyphCache::DetachCache(desc);
}

#ifdef SK_BUILD_FOR_ANDROID
const SkGlyph& SkPaint::getUnicharMetrics(SkUnichar text) {
    SkGlyphCache* cache;
    descriptorProc(NULL, DetachDescProc, &cache, true);

    const SkGlyph& glyph = cache->getUnicharMetrics(text);

    SkGlyphCache::AttachCache(cache);
    return glyph;
}

const SkGlyph& SkPaint::getGlyphMetrics(uint16_t glyphId) {
    SkGlyphCache* cache;
    descriptorProc(NULL, DetachDescProc, &cache, true);

    const SkGlyph& glyph = cache->getGlyphIDMetrics(glyphId);

    SkGlyphCache::AttachCache(cache);
    return glyph;
}

const void* SkPaint::findImage(const SkGlyph& glyph) {
    // See ::detachCache()
    SkGlyphCache* cache;
    descriptorProc(NULL, DetachDescProc, &cache, true);

    const void* image = cache->findImage(glyph);

    SkGlyphCache::AttachCache(cache);
    return image;
}
#endif

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
            return SkUTF16_CountUnichars((const uint16_t*)textData,
                                         byteLength >> 1);
        case kUTF32_TextEncoding:
            return byteLength >> 2;
        case kGlyphID_TextEncoding:
            return byteLength >> 1;
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
        return byteLength >> 1;
    }

    SkAutoGlyphCache autoCache(*this, NULL);
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
    return gptr - glyphs;
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

    SkAutoGlyphCache autoCache(*this, NULL);
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

void SkPaint::glyphsToUnichars(const uint16_t glyphs[], int count,
                               SkUnichar textData[]) const {
    if (count <= 0) {
        return;
    }

    SkASSERT(glyphs != NULL);
    SkASSERT(textData != NULL);

    SkAutoGlyphCache autoCache(*this, NULL);
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

static const SkGlyph& sk_getMetrics_utf8_prev(SkGlyphCache* cache,
                                              const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharMetrics(SkUTF8_PrevUnichar(text));
}

static const SkGlyph& sk_getMetrics_utf16_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);
    
    return cache->getUnicharMetrics(SkUTF16_NextUnichar((const uint16_t**)text));
}

static const SkGlyph& sk_getMetrics_utf16_prev(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);
    
    return cache->getUnicharMetrics(SkUTF16_PrevUnichar((const uint16_t**)text));
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

static const SkGlyph& sk_getMetrics_utf32_prev(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);
    
    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *--ptr;
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

static const SkGlyph& sk_getMetrics_glyph_prev(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const uint16_t* ptr = *(const uint16_t**)text;
    ptr -= 1;
    unsigned glyphID = *ptr;
    *text = (const char*)ptr;
    return cache->getGlyphIDMetrics(glyphID);
}

static const SkGlyph& sk_getAdvance_utf8_next(SkGlyphCache* cache,
                                              const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharAdvance(SkUTF8_NextUnichar(text));
}

static const SkGlyph& sk_getAdvance_utf8_prev(SkGlyphCache* cache,
                                              const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharAdvance(SkUTF8_PrevUnichar(text));
}

static const SkGlyph& sk_getAdvance_utf16_next(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharAdvance(SkUTF16_NextUnichar((const uint16_t**)text));
}

static const SkGlyph& sk_getAdvance_utf16_prev(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    return cache->getUnicharAdvance(SkUTF16_PrevUnichar((const uint16_t**)text));
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

static const SkGlyph& sk_getAdvance_utf32_prev(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);
    
    const int32_t* ptr = *(const int32_t**)text;
    SkUnichar uni = *--ptr;
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

static const SkGlyph& sk_getAdvance_glyph_prev(SkGlyphCache* cache,
                                               const char** text) {
    SkASSERT(cache != NULL);
    SkASSERT(text != NULL);

    const uint16_t* ptr = *(const uint16_t**)text;
    ptr -= 1;
    unsigned glyphID = *ptr;
    *text = (const char*)ptr;
    return cache->getGlyphIDAdvance(glyphID);
}

SkMeasureCacheProc SkPaint::getMeasureCacheProc(TextBufferDirection tbd,
                                                bool needFullMetrics) const {
    static const SkMeasureCacheProc gMeasureCacheProcs[] = {
        sk_getMetrics_utf8_next,
        sk_getMetrics_utf16_next,
        sk_getMetrics_utf32_next,
        sk_getMetrics_glyph_next,

        sk_getMetrics_utf8_prev,
        sk_getMetrics_utf16_prev,
        sk_getMetrics_utf32_prev,
        sk_getMetrics_glyph_prev,

        sk_getAdvance_utf8_next,
        sk_getAdvance_utf16_next,
        sk_getAdvance_utf32_next,
        sk_getAdvance_glyph_next,

        sk_getAdvance_utf8_prev,
        sk_getAdvance_utf16_prev,
        sk_getAdvance_utf32_prev,
        sk_getAdvance_glyph_prev
    };

    unsigned index = this->getTextEncoding();

    if (kBackward_TextBufferDirection == tbd) {
        index += 4;
    }
    if (!needFullMetrics && !this->isDevKernText()) {
        index += 8;
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
    SkUnichar uni = *--ptr;
    *text = (const char*)ptr;
    return cache->getUnicharMetrics(uni);
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
    if (fFlags & kSubpixelText_Flag) {
        index += 4;
    }

    SkASSERT(index < SK_ARRAY_COUNT(gDrawCacheProcs));
    return gDrawCacheProcs[index];
}

///////////////////////////////////////////////////////////////////////////////

class SkAutoRestorePaintTextSizeAndFrame {
public:
    SkAutoRestorePaintTextSizeAndFrame(const SkPaint* paint)
            : fPaint((SkPaint*)paint) {
        fTextSize = paint->getTextSize();
        fStyle = paint->getStyle();
        fPaint->setStyle(SkPaint::kFill_Style);
    }

    ~SkAutoRestorePaintTextSizeAndFrame() {
        fPaint->setStyle(fStyle);
        fPaint->setTextSize(fTextSize);
    }

private:
    SkPaint*        fPaint;
    SkScalar        fTextSize;
    SkPaint::Style  fStyle;
};

static void set_bounds(const SkGlyph& g, SkRect* bounds) {
    bounds->set(SkIntToScalar(g.fLeft),
                SkIntToScalar(g.fTop),
                SkIntToScalar(g.fLeft + g.fWidth),
                SkIntToScalar(g.fTop + g.fHeight));
}

// 64bits wide, with a 16bit bias. Useful when accumulating lots of 16.16 so
// we don't overflow along the way
typedef int64_t Sk48Dot16;

#ifdef SK_SCALAR_IS_FLOAT
    static inline float Sk48Dot16ToScalar(Sk48Dot16 x) {
        return (float) (x * 1.5258789e-5);   // x * (1 / 65536.0f)
    }
#else
    static inline SkFixed Sk48Dot16ToScalar(Sk48Dot16 x) {
        // just return the low 32bits
        return static_cast<SkFixed>(x);
    }
#endif

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

    SkMeasureCacheProc glyphCacheProc;
    glyphCacheProc = this->getMeasureCacheProc(kForward_TextBufferDirection,
                                               NULL != bounds);

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

SkScalar SkPaint::measureText(const void* textData, size_t length,
                              SkRect* bounds, SkScalar zoom) const {
    const char* text = (const char*)textData;
    SkASSERT(text != NULL || length == 0);

    SkScalar                            scale = 0;
    SkAutoRestorePaintTextSizeAndFrame  restore(this);

    if (this->isLinearText()) {
        scale = fTextSize / kCanonicalTextSizeForPaths;
        // this gets restored by restore
        ((SkPaint*)this)->setTextSize(SkIntToScalar(kCanonicalTextSizeForPaths));
    }

    SkMatrix zoomMatrix, *zoomPtr = NULL;
    if (zoom) {
        zoomMatrix.setScale(zoom, zoom);
        zoomPtr = &zoomMatrix;
    }

    SkAutoGlyphCache    autoCache(*this, zoomPtr);
    SkGlyphCache*       cache = autoCache.getCache();

    SkScalar width = 0;

    if (length > 0) {
        int tempCount;

        width = this->measure_text(cache, text, length, &tempCount, bounds);
        if (scale) {
            width = SkScalarMul(width, scale);
            if (bounds) {
                bounds->fLeft = SkScalarMul(bounds->fLeft, scale);
                bounds->fTop = SkScalarMul(bounds->fTop, scale);
                bounds->fRight = SkScalarMul(bounds->fRight, scale);
                bounds->fBottom = SkScalarMul(bounds->fBottom, scale);
            }
        }
    }
    return width;
}

typedef bool (*SkTextBufferPred)(const char* text, const char* stop);

static bool forward_textBufferPred(const char* text, const char* stop) {
    return text < stop;
}

static bool backward_textBufferPred(const char* text, const char* stop) {
    return text > stop;
}

static SkTextBufferPred chooseTextBufferPred(SkPaint::TextBufferDirection tbd,
                                             const char** text, size_t length,
                                             const char** stop) {
    if (SkPaint::kForward_TextBufferDirection == tbd) {
        *stop = *text + length;
        return forward_textBufferPred;
    } else {
        // text should point to the end of the buffer, and stop to the beginning
        *stop = *text;
        *text += length;
        return backward_textBufferPred;
    }
}

size_t SkPaint::breakText(const void* textD, size_t length, SkScalar maxWidth,
                          SkScalar* measuredWidth,
                          TextBufferDirection tbd) const {
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

    SkScalar                            scale = 0;
    SkAutoRestorePaintTextSizeAndFrame  restore(this);

    if (this->isLinearText()) {
        scale = fTextSize / kCanonicalTextSizeForPaths;
        maxWidth = SkScalarMulDiv(maxWidth, kCanonicalTextSizeForPaths, fTextSize);
        // this gets restored by restore
        ((SkPaint*)this)->setTextSize(SkIntToScalar(kCanonicalTextSizeForPaths));
    }

    SkAutoGlyphCache    autoCache(*this, NULL);
    SkGlyphCache*       cache = autoCache.getCache();

    SkMeasureCacheProc glyphCacheProc = this->getMeasureCacheProc(tbd, false);
    const char*      stop;
    SkTextBufferPred pred = chooseTextBufferPred(tbd, &text, length, &stop);
    const int        xyIndex = this->isVerticalText() ? 1 : 0;
    // use 64bits for our accumulator, to avoid overflowing 16.16
    Sk48Dot16        max = SkScalarToFixed(maxWidth);
    Sk48Dot16        width = 0;

    SkAutoKern  autokern;

    if (this->isDevKernText()) {
        int rsb = 0;
        while (pred(text, stop)) {
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
        while (pred(text, stop)) {
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
    return (kForward_TextBufferDirection == tbd) ?
                text - stop + length : stop - text + length;
}

///////////////////////////////////////////////////////////////////////////////

static bool FontMetricsCacheProc(const SkGlyphCache* cache, void* context) {
    *(SkPaint::FontMetrics*)context = cache->getFontMetricsY();
    return false;   // don't detach the cache
}

static void FontMetricsDescProc(const SkDescriptor* desc, void* context) {
    SkGlyphCache::VisitCache(desc, FontMetricsCacheProc, context);
}

SkScalar SkPaint::getFontMetrics(FontMetrics* metrics, SkScalar zoom) const {
    SkScalar                            scale = 0;
    SkAutoRestorePaintTextSizeAndFrame  restore(this);

    if (this->isLinearText()) {
        scale = fTextSize / kCanonicalTextSizeForPaths;
        // this gets restored by restore
        ((SkPaint*)this)->setTextSize(SkIntToScalar(kCanonicalTextSizeForPaths));
    }

    SkMatrix zoomMatrix, *zoomPtr = NULL;
    if (zoom) {
        zoomMatrix.setScale(zoom, zoom);
        zoomPtr = &zoomMatrix;
    }

#if 0
    SkAutoGlyphCache    autoCache(*this, zoomPtr);
    SkGlyphCache*       cache = autoCache.getCache();
    const FontMetrics&  my = cache->getFontMetricsY();
#endif
    FontMetrics storage;
    if (NULL == metrics) {
        metrics = &storage;
    }

    this->descriptorProc(zoomPtr, FontMetricsDescProc, metrics, true);

    if (scale) {
        metrics->fTop = SkScalarMul(metrics->fTop, scale);
        metrics->fAscent = SkScalarMul(metrics->fAscent, scale);
        metrics->fDescent = SkScalarMul(metrics->fDescent, scale);
        metrics->fBottom = SkScalarMul(metrics->fBottom, scale);
        metrics->fLeading = SkScalarMul(metrics->fLeading, scale);
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

    SkASSERT(NULL != textData);

    if (NULL == widths && NULL == bounds) {
        return this->countText(textData, byteLength);
    }

    SkAutoRestorePaintTextSizeAndFrame  restore(this);
    SkScalar                            scale = 0;

    if (this->isLinearText()) {
        scale = fTextSize / kCanonicalTextSizeForPaths;
        // this gets restored by restore
        ((SkPaint*)this)->setTextSize(SkIntToScalar(kCanonicalTextSizeForPaths));
    }

    SkAutoGlyphCache    autoCache(*this, NULL);
    SkGlyphCache*       cache = autoCache.getCache();
    SkMeasureCacheProc  glyphCacheProc;
    glyphCacheProc = this->getMeasureCacheProc(kForward_TextBufferDirection,
                                               NULL != bounds);

    const char* text = (const char*)textData;
    const char* stop = text + byteLength;
    int         count = 0;
    const int   xyIndex = this->isVerticalText() ? 1 : 0;

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
    while ((iterPath = iter.next(&xpos)) != NULL) {
        matrix.postTranslate(xpos - prevXPos, 0);
        path->addPath(*iterPath, matrix);
        prevXPos = xpos;
    }
}

static void add_flattenable(SkDescriptor* desc, uint32_t tag,
                            SkFlattenableWriteBuffer* buffer) {
    buffer->flatten(desc->addEntry(tag, buffer->size(), NULL));
}

// SkFontHost can override this choice in FilterRec()
static SkMask::Format computeMaskFormat(const SkPaint& paint) {
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
    if (paint.getShader()) {
        return false;
    }
    SkColor c = paint.getColor();
    if (paint.getColorFilter()) {
        c = paint.getColorFilter()->filterColor(c);
    }
    if (color) {
        *color = c;
    }
    return true;
}

#ifdef SK_USE_COLOR_LUMINANCE
static SkColor computeLuminanceColor(const SkPaint& paint) {
    SkColor c;
    if (!justAColor(paint, &c)) {
        c = SkColorSetRGB(0x7F, 0x80, 0x7F);
    }
    return c;
}

#define assert_byte(x)  SkASSERT(0 == ((x) >> 8))

static U8CPU reduce_lumbits(U8CPU x) {
    static const uint8_t gReduceBits[] = {
        0x0, 0x55, 0xAA, 0xFF
    };
    assert_byte(x);
    return gReduceBits[x >> 6];
}

static unsigned computeLuminance(SkColor c) {
    int r = SkColorGetR(c);
    int g = SkColorGetG(c);
    int b = SkColorGetB(c);
    // compute luminance
    // R=0.2126 G=0.7152 B=0.0722
    // scaling by 127 yields 27, 92, 9
    int luminance = r * 27 + g * 92 + b * 9;
    luminance >>= 7;
    assert_byte(luminance);
    return luminance;
}

#else
// returns 0..kLuminance_Max
static unsigned computeLuminance(const SkPaint& paint) {
    SkColor c;
    if (justAColor(paint, &c)) {
        int r = SkColorGetR(c);
        int g = SkColorGetG(c);
        int b = SkColorGetB(c);
        // compute luminance
        // R=0.2126 G=0.7152 B=0.0722
        // scaling by 127 yields 27, 92, 9
#if 1
        int luminance = r * 27 + g * 92 + b * 9;
        luminance >>= 15 - SkScalerContext::kLuminance_Bits;
#else
        int luminance = r * 2 + g * 5 + b * 1;
        luminance >>= 11 - SkScalerContext::kLuminance_Bits;
#endif
        SkASSERT(luminance <= SkScalerContext::kLuminance_Max);
        return luminance;
    }
    // if we're not a single color, return the middle of the luminance range
    return SkScalerContext::kLuminance_Max >> 1;
}
#endif

// Beyond this size, LCD doesn't appreciably improve quality, but it always
// cost more RAM and draws slower, so we set a cap.
#ifndef SK_MAX_SIZE_FOR_LCDTEXT
    #define SK_MAX_SIZE_FOR_LCDTEXT    48
#endif

static bool tooBigForLCD(const SkScalerContext::Rec& rec) {
    SkScalar area = SkScalarMul(rec.fPost2x2[0][0], rec.fPost2x2[1][1]) -
                    SkScalarMul(rec.fPost2x2[1][0], rec.fPost2x2[0][1]);
    SkScalar size = SkScalarMul(area, rec.fTextSize);
    return SkScalarAbs(size) > SkIntToScalar(SK_MAX_SIZE_FOR_LCDTEXT);
}

/*
 *  Return the scalar with only limited fractional precision. Used to consolidate matrices
 *  that vary only slightly when we create our key into the font cache, since the font scaler
 *  typically returns the same looking resuts for tiny changes in the matrix.
 */
static SkScalar sk_relax(SkScalar x) {
#ifdef SK_SCALAR_IS_FLOAT
    int n = sk_float_round2int(x * 1024);
    return n / 1024.0f;
#else
    // round to the nearest 10 fractional bits
    return (x + (1 << 5)) & ~(1024 - 1);
#endif
}

void SkScalerContext::MakeRec(const SkPaint& paint,
                              const SkMatrix* deviceMatrix, Rec* rec) {
    SkASSERT(deviceMatrix == NULL || !deviceMatrix->hasPerspective());

    rec->fOrigFontID = SkTypeface::UniqueID(paint.getTypeface());
    rec->fFontID = rec->fOrigFontID;
    rec->fTextSize = paint.getTextSize();
    rec->fPreScaleX = paint.getTextScaleX();
    rec->fPreSkewX  = paint.getTextSkewX();

    if (deviceMatrix) {
        rec->fPost2x2[0][0] = sk_relax(deviceMatrix->getScaleX());
        rec->fPost2x2[0][1] = sk_relax(deviceMatrix->getSkewX());
        rec->fPost2x2[1][0] = sk_relax(deviceMatrix->getSkewY());
        rec->fPost2x2[1][1] = sk_relax(deviceMatrix->getScaleY());
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

    rec->fMaskFormat = SkToU8(computeMaskFormat(paint));

    if (SkMask::kLCD16_Format == rec->fMaskFormat ||
        SkMask::kLCD32_Format == rec->fMaskFormat)
    {
        SkFontHost::LCDOrder order = SkFontHost::GetSubpixelOrder();
        SkFontHost::LCDOrientation orient = SkFontHost::GetSubpixelOrientation();
        if (SkFontHost::kNONE_LCDOrder == order || tooBigForLCD(*rec)) {
            // eeek, can't support LCD
            rec->fMaskFormat = SkMask::kA8_Format;
        } else {
            if (SkFontHost::kVertical_LCDOrientation == orient) {
                flags |= SkScalerContext::kLCD_Vertical_Flag;
            }
            if (SkFontHost::kBGR_LCDOrder == order) {
                flags |= SkScalerContext::kLCD_BGROrder_Flag;
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
        flags |= SkScalerContext::kAutohinting_Flag;
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
#ifdef SK_USE_COLOR_LUMINANCE
    rec->setLuminanceColor(computeLuminanceColor(paint));
#else
    rec->setLuminanceBits(computeLuminance(paint));
#endif

    /*  Allow the fonthost to modify our rec before we use it as a key into the
        cache. This way if we're asking for something that they will ignore,
        they can modify our rec up front, so we don't create duplicate cache
        entries.
     */
    SkFontHost::FilterRec(rec);

    // be sure to call PostMakeRec(rec) before you actually use it!
}

/**
 *  We ensure that the rec is self-consistent and efficient (where possible)
 */
void SkScalerContext::PostMakeRec(SkScalerContext::Rec* rec) {

    /**
     *  If we're asking for A8, we force the colorlum to be gray, since that
     *  that limits the number of unique entries, and the scaler will only
     *  look at the lum of one of them.
     */
    switch (rec->fMaskFormat) {
        case SkMask::kLCD16_Format:
        case SkMask::kLCD32_Format: {
#ifdef SK_USE_COLOR_LUMINANCE
            // filter down the luminance color to a finite number of bits
            SkColor c = rec->getLuminanceColor();
            c = SkColorSetRGB(reduce_lumbits(SkColorGetR(c)),
                              reduce_lumbits(SkColorGetG(c)),
                              reduce_lumbits(SkColorGetB(c)));
            rec->setLuminanceColor(c);
#endif
            break;
        }
        case SkMask::kA8_Format: {
#ifdef SK_USE_COLOR_LUMINANCE
            // filter down the luminance to a single component, since A8 can't
            // use per-component information
            unsigned lum = computeLuminance(rec->getLuminanceColor());
            // reduce to our finite number of bits
            lum = reduce_lumbits(lum);
            rec->setLuminanceColor(SkColorSetRGB(lum, lum, lum));
#endif
            break;
        }
        case SkMask::kBW_Format:
            // No need to differentiate gamma if we're BW
#ifdef SK_USE_COLOR_LUMINANCE
            rec->setLuminanceColor(0);
#else
            rec->setLuminanceBits(0);
#endif
            break;
    }
}

#define MIN_SIZE_FOR_EFFECT_BUFFER  1024

#ifdef SK_DEBUG
    #define TEST_DESC
#endif

/*
 *  ignoreGamma tells us that the caller just wants metrics that are unaffected
 *  by gamma correction, so we jam the luminance field to 0 (most common value
 *  for black text) in hopes that we get a cache hit easier. A better solution
 *  would be for the fontcache lookup to know to ignore the luminance field
 *  entirely, but not sure how to do that and keep it fast.
 */
void SkPaint::descriptorProc(const SkMatrix* deviceMatrix,
                             void (*proc)(const SkDescriptor*, void*),
                             void* context, bool ignoreGamma) const {
    SkScalerContext::Rec    rec;

    SkScalerContext::MakeRec(*this, deviceMatrix, &rec);
    if (ignoreGamma) {
#ifdef SK_USE_COLOR_LUMINANCE
        rec.setLuminanceColor(0);
#else
        rec.setLuminanceBits(0);
#endif
    }

    size_t          descSize = sizeof(rec);
    int             entryCount = 1;
    SkPathEffect*   pe = this->getPathEffect();
    SkMaskFilter*   mf = this->getMaskFilter();
    SkRasterizer*   ra = this->getRasterizer();

    SkFlattenableWriteBuffer    peBuffer(MIN_SIZE_FOR_EFFECT_BUFFER);
    SkFlattenableWriteBuffer    mfBuffer(MIN_SIZE_FOR_EFFECT_BUFFER);
    SkFlattenableWriteBuffer    raBuffer(MIN_SIZE_FOR_EFFECT_BUFFER);

    if (pe) {
        peBuffer.writeFlattenable(pe);
        descSize += peBuffer.size();
        entryCount += 1;
        rec.fMaskFormat = SkMask::kA8_Format;   // force antialiasing when we do the scan conversion
        // seems like we could support kLCD as well at this point...
    }
    if (mf) {
        mfBuffer.writeFlattenable(mf);
        descSize += mfBuffer.size();
        entryCount += 1;
        rec.fMaskFormat = SkMask::kA8_Format;   // force antialiasing with maskfilters
    }
    if (ra) {
        raBuffer.writeFlattenable(ra);
        descSize += raBuffer.size();
        entryCount += 1;
        rec.fMaskFormat = SkMask::kA8_Format;   // force antialiasing when we do the scan conversion
    }

    ///////////////////////////////////////////////////////////////////////////
    // Now that we're done tweaking the rec, call the PostMakeRec cleanup
    SkScalerContext::PostMakeRec(&rec);
    
    descSize += SkDescriptor::ComputeOverhead(entryCount);

    SkAutoDescriptor    ad(descSize);
    SkDescriptor*       desc = ad.getDesc();

    desc->init();
    desc->addEntry(kRec_SkDescriptorTag, sizeof(rec), &rec);

    if (pe) {
        add_flattenable(desc, kPathEffect_SkDescriptorTag, &peBuffer);
    }
    if (mf) {
        add_flattenable(desc, kMaskFilter_SkDescriptorTag, &mfBuffer);
    }
    if (ra) {
        add_flattenable(desc, kRasterizer_SkDescriptorTag, &raBuffer);
    }

    SkASSERT(descSize == desc->getLength());
    desc->computeChecksum();

#ifdef TEST_DESC
    {
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
            add_flattenable(desc1, kPathEffect_SkDescriptorTag, &peBuffer);
            add_flattenable(desc2, kPathEffect_SkDescriptorTag, &peBuffer);
        }
        if (mf) {
            add_flattenable(desc1, kMaskFilter_SkDescriptorTag, &mfBuffer);
            add_flattenable(desc2, kMaskFilter_SkDescriptorTag, &mfBuffer);
        }
        if (ra) {
            add_flattenable(desc1, kRasterizer_SkDescriptorTag, &raBuffer);
            add_flattenable(desc2, kRasterizer_SkDescriptorTag, &raBuffer);
        }
        
        SkASSERT(descSize == desc1->getLength());
        SkASSERT(descSize == desc2->getLength());
        desc1->computeChecksum();
        desc2->computeChecksum();
        SkASSERT(!memcmp(desc, desc1, descSize));
        SkASSERT(!memcmp(desc, desc2, descSize));
    }
#endif
    
    proc(desc, context);
}

SkGlyphCache* SkPaint::detachCache(const SkMatrix* deviceMatrix) const {
    SkGlyphCache* cache;
    this->descriptorProc(deviceMatrix, DetachDescProc, &cache);
    return cache;
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

enum FlatFlags {
    kHasTypeface_FlatFlag   = 0x01,
    kHasEffects_FlatFlag    = 0x02
};

// The size of a flat paint's POD fields
static const uint32_t kPODPaintSize =   5 * sizeof(SkScalar) +
                                        1 * sizeof(SkColor) +
                                        1 * sizeof(uint16_t) +
                                        6 * sizeof(uint8_t);

/*  To save space/time, we analyze the paint, and write a truncated version of
    it if there are not tricky elements like shaders, etc.
 */
void SkPaint::flatten(SkFlattenableWriteBuffer& buffer) const {
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
    // previously flags:16, textAlign:8, flatFlags:8
    // now flags:16, hinting:4, textAlign:4, flatFlags:8
    *ptr++ = (this->getFlags() << 16) |
             // hinting added later. 0 in this nibble means use the default.
             ((this->getHinting()+1) << 12) |
             (this->getTextAlign() << 8) |
             flatFlags;
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
    }
}

void SkPaint::unflatten(SkFlattenableReadBuffer& buffer) {
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

    // previously flags:16, textAlign:8, flatFlags:8
    // now flags:16, hinting:4, textAlign:4, flatFlags:8
    uint32_t tmp = *pod++;
    this->setFlags(tmp >> 16);

    // hinting added later. 0 in this nibble means use the default.
    uint32_t hinting = (tmp >> 12) & 0xF;
    this->setHinting(0 == hinting ? kNormal_Hinting : static_cast<Hinting>(hinting-1));

    this->setTextAlign(static_cast<Align>((tmp >> 8) & 0xF));

    uint8_t flatFlags = tmp & 0xFF;

    tmp = *pod++;
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
        SkSafeUnref(this->setPathEffect((SkPathEffect*) buffer.readFlattenable()));
        SkSafeUnref(this->setShader((SkShader*) buffer.readFlattenable()));
        SkSafeUnref(this->setXfermode((SkXfermode*) buffer.readFlattenable()));
        SkSafeUnref(this->setMaskFilter((SkMaskFilter*) buffer.readFlattenable()));
        SkSafeUnref(this->setColorFilter((SkColorFilter*) buffer.readFlattenable()));
        SkSafeUnref(this->setRasterizer((SkRasterizer*) buffer.readFlattenable()));
        SkSafeUnref(this->setLooper((SkDrawLooper*) buffer.readFlattenable()));
        SkSafeUnref(this->setImageFilter((SkImageFilter*) buffer.readFlattenable()));
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
    GEN_ID_INC_EVAL(shader != fShader);
    SkRefCnt_SafeAssign(fShader, shader);
    return shader;
}

SkColorFilter* SkPaint::setColorFilter(SkColorFilter* filter) {
    GEN_ID_INC_EVAL(filter != fColorFilter);
    SkRefCnt_SafeAssign(fColorFilter, filter);
    return filter;
}

SkXfermode* SkPaint::setXfermode(SkXfermode* mode) {
    GEN_ID_INC_EVAL(mode != fXfermode);
    SkRefCnt_SafeAssign(fXfermode, mode);
    return mode;
}

SkXfermode* SkPaint::setXfermodeMode(SkXfermode::Mode mode) {
    SkSafeUnref(fXfermode);
    fXfermode = SkXfermode::Create(mode);
    GEN_ID_INC;
    return fXfermode;
}

SkPathEffect* SkPaint::setPathEffect(SkPathEffect* effect) {
    GEN_ID_INC_EVAL(effect != fPathEffect);
    SkRefCnt_SafeAssign(fPathEffect, effect);
    return effect;
}

SkMaskFilter* SkPaint::setMaskFilter(SkMaskFilter* filter) {
    GEN_ID_INC_EVAL(filter != fMaskFilter);
    SkRefCnt_SafeAssign(fMaskFilter, filter);
    return filter;
}

///////////////////////////////////////////////////////////////////////////////

bool SkPaint::getFillPath(const SkPath& src, SkPath* dst) const {
    SkPath          effectPath, strokePath;
    const SkPath*   path = &src;

    SkScalar width = this->getStrokeWidth();

    switch (this->getStyle()) {
        case SkPaint::kFill_Style:
            width = -1; // mark it as no-stroke
            break;
        case SkPaint::kStrokeAndFill_Style:
            if (width == 0) {
                width = -1; // mark it as no-stroke
            }
            break;
        case SkPaint::kStroke_Style:
            break;
        default:
            SkDEBUGFAIL("unknown paint style");
    }

    if (this->getPathEffect()) {
        // lie to the pathEffect if our style is strokeandfill, so that it treats us as just fill
        if (this->getStyle() == SkPaint::kStrokeAndFill_Style) {
            width = -1; // mark it as no-stroke
        }

        if (this->getPathEffect()->filterPath(&effectPath, src, &width)) {
            path = &effectPath;
        }

        // restore the width if we earlier had to lie, and if we're still set to no-stroke
        // note: if we're now stroke (width >= 0), then the pathEffect asked for that change
        // and we want to respect that (i.e. don't overwrite their setting for width)
        if (this->getStyle() == SkPaint::kStrokeAndFill_Style && width < 0) {
            width = this->getStrokeWidth();
            if (width == 0) {
                width = -1;
            }
        }
    }

    if (width > 0 && !path->isEmpty()) {
        SkStroke stroker(*this, width);
        stroker.strokePath(*path, &strokePath);
        path = &strokePath;
    }

    if (path == &src) {
        *dst = src;
    } else {
        SkASSERT(path == &effectPath || path == &strokePath);
        dst->swap(*(SkPath*)path);
    }

    return width != 0;  // return true if we're filled, or false if we're hairline (width == 0)
}

const SkRect& SkPaint::doComputeFastBounds(const SkRect& src,
                                                 SkRect* storage) const {
    SkASSERT(storage);

    if (this->getLooper()) {
        SkASSERT(this->getLooper()->canComputeFastBounds(*this));
        this->getLooper()->computeFastBounds(*this, src, storage);
        return *storage;
    }

    if (this->getStyle() != SkPaint::kFill_Style) {
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
        storage->set(src.fLeft - radius, src.fTop - radius,
                     src.fRight + radius, src.fBottom + radius);
    } else {
        *storage = src;
    }

    // check the mask filter
    if (this->getMaskFilter()) {
        this->getMaskFilter()->computeFastBounds(*storage, storage);
    }

    return *storage;
}

///////////////////////////////////////////////////////////////////////////////

static bool has_thick_frame(const SkPaint& paint) {
    return  paint.getStrokeWidth() > 0 &&
            paint.getStyle() != SkPaint::kFill_Style;
}

SkTextToPathIter::SkTextToPathIter( const char text[], size_t length,
                                    const SkPaint& paint,
                                    bool applyStrokeAndPathEffects)
                                    : fPaint(paint) {
    fGlyphCacheProc = paint.getMeasureCacheProc(SkPaint::kForward_TextBufferDirection,
                                                true);

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
            fPaint.setStrokeWidth(SkScalarDiv(fPaint.getStrokeWidth(), fScale));
        }
    } else {
        fScale = SK_Scalar1;
    }

    if (!applyStrokeAndPathEffects) {
        fPaint.setStyle(SkPaint::kFill_Style);
        fPaint.setPathEffect(NULL);
    }

    fCache = fPaint.detachCache(NULL);

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

const SkPath* SkTextToPathIter::next(SkScalar* xpos) {
    while (fText < fStop) {
        const SkGlyph& glyph = fGlyphCacheProc(fCache, &fText);

        fXPos += SkScalarMul(SkFixedToScalar(fPrevAdvance + fAutoKern.adjust(glyph)), fScale);
        fPrevAdvance = advance(glyph, fXYIndex);   // + fPaint.getTextTracking();

        if (glyph.fWidth) {
            if (xpos) {
                *xpos = fXPos;
            }
            return fCache->findPath(glyph);
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

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
                return 0 == this->getAlpha();
            case SkXfermode::kDst_Mode:
                return true;
            default:
                break;
        }
    }
    return false;
}


//////////// Move these to their own file soon.

bool SkImageFilter::filterImage(Proxy* proxy, const SkBitmap& src,
                                const SkMatrix& ctm,
                                SkBitmap* result, SkIPoint* loc) {
    SkASSERT(proxy);
    SkASSERT(result);
    SkASSERT(loc);
    /*
     *  Give the proxy first shot at the filter. If it returns false, ask
     *  the filter to do it.
     */
    return proxy->filterImage(this, src, ctm, result, loc) ||
           this->onFilterImage(proxy, src, ctm, result, loc);
}

bool SkImageFilter::filterBounds(const SkIRect& src, const SkMatrix& ctm,
                                 SkIRect* dst) {
    SkASSERT(&src);
    SkASSERT(dst);
    return this->onFilterBounds(src, ctm, dst);
}

bool SkImageFilter::onFilterImage(Proxy*, const SkBitmap&, const SkMatrix&,
                                  SkBitmap*, SkIPoint*) {
    return false;
}

bool SkImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                   SkIRect* dst) {
    *dst = src;
    return true;
}

bool SkImageFilter::asABlur(SkSize* sigma) const {
    return false;
}

bool SkImageFilter::asAnErode(SkISize* radius) const {
    return false;
}

bool SkImageFilter::asADilate(SkISize* radius) const {
    return false;
}

//////

bool SkDrawLooper::canComputeFastBounds(const SkPaint& paint) {
    SkCanvas canvas;

    this->init(&canvas);
    for (;;) {
        SkPaint p(paint);
        if (this->next(&canvas, &p)) {
            p.setLooper(NULL);
            if (!p.canComputeFastBounds()) {
                return false;
            }
        } else {
            break;
        }
    }
    return true;
}

void SkDrawLooper::computeFastBounds(const SkPaint& paint, const SkRect& src,
                                     SkRect* dst) {
    SkCanvas canvas;
    
    this->init(&canvas);
    for (bool firstTime = true;; firstTime = false) {
        SkPaint p(paint);
        if (this->next(&canvas, &p)) {
            SkRect r(src);

            p.setLooper(NULL);
            p.computeFastBounds(r, &r);
            canvas.getTotalMatrix().mapRect(&r);

            if (firstTime) {
                *dst = r;
            } else {
                dst->join(r);
            }
        } else {
            break;
        }
    }
}

