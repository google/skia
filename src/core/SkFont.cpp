/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"

#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPaintDefaults.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

using namespace skia_private;

#define kDefault_Size       SkPaintDefaults_TextSize
#define kDefault_Flags      SkFont::kBaselineSnap_PrivFlag
#define kDefault_Edging     SkFont::Edging::kAntiAlias
#define kDefault_Hinting    SkPaintDefaults_Hinting

static inline SkScalar valid_size(SkScalar size) {
    return std::max<SkScalar>(0, size);
}

SkFont::SkFont(sk_sp<SkTypeface> face, SkScalar size, SkScalar scaleX, SkScalar skewX)
    : fTypeface(std::move(face))
    , fSize(valid_size(size))
    , fScaleX(scaleX)
    , fSkewX(skewX)
    , fFlags(kDefault_Flags)
    , fEdging(static_cast<unsigned>(kDefault_Edging))
    , fHinting(static_cast<unsigned>(kDefault_Hinting))
{}

SkFont::SkFont(sk_sp<SkTypeface> face, SkScalar size) : SkFont(std::move(face), size, 1, 0) {}

SkFont::SkFont(sk_sp<SkTypeface> face) : SkFont(std::move(face), kDefault_Size, 1, 0) {}

SkFont::SkFont() : SkFont(nullptr, kDefault_Size) {}

bool SkFont::operator==(const SkFont& b) const {
    return  fTypeface.get() == b.fTypeface.get() &&
            fSize           == b.fSize &&
            fScaleX         == b.fScaleX &&
            fSkewX          == b.fSkewX &&
            fFlags          == b.fFlags &&
            fEdging         == b.fEdging &&
            fHinting        == b.fHinting;
}

void SkFont::dump() const {
    SkDebugf("typeface %p\n", fTypeface.get());
    SkDebugf("size %g\n", fSize);
    SkDebugf("skewx %g\n", fSkewX);
    SkDebugf("scalex %g\n", fScaleX);
    SkDebugf("flags 0x%X\n", fFlags);
    SkDebugf("edging %d\n", (unsigned)fEdging);
    SkDebugf("hinting %d\n", (unsigned)fHinting);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint32_t set_clear_mask(uint32_t bits, bool cond, uint32_t mask) {
    return cond ? bits | mask : bits & ~mask;
}

void SkFont::setForceAutoHinting(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kForceAutoHinting_PrivFlag);
}
void SkFont::setEmbeddedBitmaps(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kEmbeddedBitmaps_PrivFlag);
}
void SkFont::setSubpixel(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kSubpixel_PrivFlag);
}
void SkFont::setLinearMetrics(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kLinearMetrics_PrivFlag);
}
void SkFont::setEmbolden(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kEmbolden_PrivFlag);
}
void SkFont::setBaselineSnap(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kBaselineSnap_PrivFlag);
}
void SkFont::setEdging(Edging e) {
    fEdging = SkToU8(e);
}

void SkFont::setHinting(SkFontHinting h) {
    fHinting = SkToU8(h);
}

void SkFont::setSize(SkScalar size) {
    fSize = valid_size(size);
}
void SkFont::setScaleX(SkScalar scale) {
    fScaleX = scale;
}
void SkFont::setSkewX(SkScalar skew) {
    fSkewX = skew;
}

SkFont SkFont::makeWithSize(SkScalar newSize) const {
    SkFont font = *this;
    font.setSize(newSize);
    return font;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkScalar SkFont::setupForAsPaths(SkPaint* paint) {
    constexpr uint32_t flagsToIgnore = kEmbeddedBitmaps_PrivFlag |
                                       kForceAutoHinting_PrivFlag;

    fFlags = (fFlags & ~flagsToIgnore) | kSubpixel_PrivFlag;
    this->setHinting(SkFontHinting::kNone);

    if (this->getEdging() == Edging::kSubpixelAntiAlias) {
        this->setEdging(Edging::kAntiAlias);
    }

    if (paint) {
        paint->setStyle(SkPaint::kFill_Style);
        paint->setPathEffect(nullptr);
    }
    SkScalar textSize = fSize;
    this->setSize(SkIntToScalar(SkFontPriv::kCanonicalTextSizeForPaths));
    return textSize / SkFontPriv::kCanonicalTextSizeForPaths;
}

bool SkFont::hasSomeAntiAliasing() const {
    Edging edging = this->getEdging();
    return edging == SkFont::Edging::kAntiAlias
        || edging == SkFont::Edging::kSubpixelAntiAlias;
}

SkGlyphID SkFont::unicharToGlyph(SkUnichar uni) const {
    return this->getTypefaceOrDefault()->unicharToGlyph(uni);
}

void SkFont::unicharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
    this->getTypefaceOrDefault()->unicharsToGlyphs(uni, count, glyphs);
}

int SkFont::textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding,
                         SkGlyphID glyphs[], int maxGlyphCount) const {
    return this->getTypefaceOrDefault()->textToGlyphs(text, byteLength, encoding,
                                                      glyphs, maxGlyphCount);
}

SkScalar SkFont::measureText(const void* text, size_t length, SkTextEncoding encoding,
                             SkRect* bounds, const SkPaint* paint) const {

    SkAutoToGlyphs atg(*this, text, length, encoding);
    const int glyphCount = atg.count();
    if (glyphCount == 0) {
        if (bounds) {
            bounds->setEmpty();
        }
        return 0;
    }
    const SkGlyphID* glyphIDs = atg.glyphs();

    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this, paint);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(SkSpan(glyphIDs, glyphCount));

    SkScalar width = 0;
    if (bounds) {
        *bounds = glyphs[0]->rect();
        width = glyphs[0]->advanceX();
        for (int i = 1; i < glyphCount; ++i) {
            SkRect r = glyphs[i]->rect();
            r.offset(width, 0);
            bounds->join(r);
            width += glyphs[i]->advanceX();
        }
    } else {
        for (auto glyph : glyphs) {
            width += glyph->advanceX();
        }
    }

    if (strikeToSourceScale != 1) {
        width *= strikeToSourceScale;
        if (bounds) {
            bounds->fLeft   *= strikeToSourceScale;
            bounds->fTop    *= strikeToSourceScale;
            bounds->fRight  *= strikeToSourceScale;
            bounds->fBottom *= strikeToSourceScale;
        }
    }

    return width;
}

void SkFont::getWidthsBounds(const SkGlyphID glyphIDs[],
                             int count,
                             SkScalar widths[],
                             SkRect bounds[],
                             const SkPaint* paint) const {
    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this, paint);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(SkSpan(glyphIDs, count));

    if (bounds) {
        SkMatrix scaleMat = SkMatrix::Scale(strikeToSourceScale, strikeToSourceScale);
        SkRect* cursor = bounds;
        for (auto glyph : glyphs) {
            scaleMat.mapRectScaleTranslate(cursor++, glyph->rect());
        }
    }

    if (widths) {
        SkScalar* cursor = widths;
        for (auto glyph : glyphs) {
            *cursor++ = glyph->advanceX() * strikeToSourceScale;
        }
    }
}

void SkFont::getPos(const SkGlyphID glyphIDs[], int count, SkPoint pos[], SkPoint origin) const {
    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(SkSpan(glyphIDs, count));

    SkPoint sum = origin;
    for (auto glyph : glyphs) {
        *pos++ = sum;
        sum += glyph->advanceVector() * strikeToSourceScale;
    }
}

void SkFont::getXPos(
        const SkGlyphID glyphIDs[], int count, SkScalar xpos[], SkScalar origin) const {

    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(SkSpan(glyphIDs, count));

    SkScalar loc = origin;
    SkScalar* cursor = xpos;
    for (auto glyph : glyphs) {
        *cursor++ = loc;
        loc += glyph->advanceX() * strikeToSourceScale;
    }
}

void SkFont::getPaths(const SkGlyphID glyphIDs[], int count,
                      void (*proc)(const SkPath*, const SkMatrix&, void*), void* ctx) const {
    SkFont font(*this);
    SkScalar scale = font.setupForAsPaths(nullptr);
    const SkMatrix mx = SkMatrix::Scale(scale, scale);

    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(font);
    SkBulkGlyphMetricsAndPaths paths{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = paths.glyphs(SkSpan(glyphIDs, count));

    for (auto glyph : glyphs) {
        proc(glyph->path(), mx, ctx);
    }
}

bool SkFont::getPath(SkGlyphID glyphID, SkPath* path) const {
    struct Pair {
        SkPath* fPath;
        bool    fWasSet;
    } pair = { path, false };

    this->getPaths(&glyphID, 1, [](const SkPath* orig, const SkMatrix& mx, void* ctx) {
        Pair* pair = static_cast<Pair*>(ctx);
        if (orig) {
            orig->transform(mx, pair->fPath);
            pair->fWasSet = true;
        }
    }, &pair);
    return pair.fWasSet;
}

SkScalar SkFont::getMetrics(SkFontMetrics* metrics) const {

    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this, nullptr);

    SkFontMetrics storage;
    if (nullptr == metrics) {
        metrics = &storage;
    }

    auto cache = strikeSpec.findOrCreateStrike();
    *metrics = cache->getFontMetrics();

    if (strikeToSourceScale != 1) {
        SkFontPriv::ScaleFontMetrics(metrics, strikeToSourceScale);
    }
    return metrics->fDescent - metrics->fAscent + metrics->fLeading;
}

SkTypeface* SkFont::getTypefaceOrDefault() const {
    return fTypeface ? fTypeface.get() : SkTypeface::GetDefaultTypeface();
}

sk_sp<SkTypeface> SkFont::refTypefaceOrDefault() const {
    return fTypeface ? fTypeface : SkTypeface::MakeDefault();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void SkFontPriv::ScaleFontMetrics(SkFontMetrics* metrics, SkScalar scale) {
    metrics->fTop *= scale;
    metrics->fAscent *= scale;
    metrics->fDescent *= scale;
    metrics->fBottom *= scale;
    metrics->fLeading *= scale;
    metrics->fAvgCharWidth *= scale;
    metrics->fMaxCharWidth *= scale;
    metrics->fXMin *= scale;
    metrics->fXMax *= scale;
    metrics->fXHeight *= scale;
    metrics->fCapHeight *= scale;
    metrics->fUnderlineThickness *= scale;
    metrics->fUnderlinePosition *= scale;
    metrics->fStrikeoutThickness *= scale;
    metrics->fStrikeoutPosition *= scale;
}

SkRect SkFontPriv::GetFontBounds(const SkFont& font) {
    SkMatrix m;
    m.setScale(font.getSize() * font.getScaleX(), font.getSize());
    m.postSkew(font.getSkewX(), 0);

    SkTypeface* typeface = font.getTypefaceOrDefault();

    SkRect bounds;
    m.mapRect(&bounds, typeface->getBounds());
    return bounds;
}

SkScalar SkFontPriv::ApproximateTransformedTextSize(const SkFont& font, const SkMatrix& matrix,
                                                    const SkPoint& textLocation) {
    if (!matrix.hasPerspective()) {
        return font.getSize() * matrix.getMaxScale();
    } else {
        // approximate the scale since we can't get it directly from the matrix
        SkScalar maxScaleSq = SkMatrixPriv::DifferentialAreaScale(matrix, textLocation);
        if (SkScalarIsFinite(maxScaleSq) && !SkScalarNearlyZero(maxScaleSq)) {
            return font.getSize() * SkScalarSqrt(maxScaleSq);
        } else {
            return -font.getSize();
        }
    }
}

int SkFontPriv::CountTextElements(const void* text, size_t byteLength, SkTextEncoding encoding) {
    switch (encoding) {
        case SkTextEncoding::kUTF8:
            return SkUTF::CountUTF8(reinterpret_cast<const char*>(text), byteLength);
        case SkTextEncoding::kUTF16:
            return SkUTF::CountUTF16(reinterpret_cast<const uint16_t*>(text), byteLength);
        case SkTextEncoding::kUTF32:
            return byteLength >> 2;
        case SkTextEncoding::kGlyphID:
            return byteLength >> 1;
    }
    SkASSERT(false);
    return 0;
}

void SkFontPriv::GlyphsToUnichars(const SkFont& font, const SkGlyphID glyphs[], int count,
                                  SkUnichar text[]) {
    if (count <= 0) {
        return;
    }

    auto typeface = font.getTypefaceOrDefault();
    const unsigned numGlyphsInTypeface = typeface->countGlyphs();
    AutoTArray<SkUnichar> unichars(static_cast<size_t>(numGlyphsInTypeface));
    typeface->getGlyphToUnicodeMap(unichars.get());

    for (int i = 0; i < count; ++i) {
        unsigned id = glyphs[i];
        text[i] = (id < numGlyphsInTypeface) ? unichars[id] : 0xFFFD;
    }
}
