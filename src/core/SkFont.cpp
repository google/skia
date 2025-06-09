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
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkUTF.h"
#include "src/base/SkZip.h"
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
    , fHinting(static_cast<unsigned>(kDefault_Hinting)) {
    if (!fTypeface) {
        fTypeface = SkTypeface::MakeEmpty();
    }
}

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
    SkDebugf("edging %u\n", (unsigned)fEdging);
    SkDebugf("hinting %u\n", (unsigned)fHinting);
}

void SkFont::setTypeface(sk_sp<SkTypeface> tf) {
    fTypeface = std::move(tf);
    if (!fTypeface) {
        fTypeface = SkTypeface::MakeEmpty();
    }
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
    return this->getTypeface()->unicharToGlyph(uni);
}

void SkFont::unicharsToGlyphs(SkSpan<const SkUnichar> unis, SkSpan<SkGlyphID> glyphs) const {
    this->getTypeface()->unicharsToGlyphs(unis, glyphs);
}

size_t SkFont::textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding,
                         SkSpan<SkGlyphID> glyphs) const {
    return this->getTypeface()->textToGlyphs(text, byteLength, encoding, glyphs);
}

SkScalar SkFont::measureText(const void* text, size_t length, SkTextEncoding encoding,
                             SkRect* bounds, const SkPaint* paint) const {

    SkAutoToGlyphs atg(*this, text, length, encoding);
    const SkSpan<const SkGlyphID> glyphIDs = atg.glyphs();

    if (glyphIDs.size() == 0) {
        if (bounds) {
            bounds->setEmpty();
        }
        return 0;
    }

    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this, paint);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(glyphIDs);

    SkScalar width = 0;
    if (bounds) {
        *bounds = glyphs[0]->rect();
        width = glyphs[0]->advanceX();
        for (size_t i = 1; i < glyphIDs.size(); ++i) {
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

void SkFont::getWidthsBounds(SkSpan<const SkGlyphID> glyphIDs,
                             SkSpan<SkScalar> widths,
                             SkSpan<SkRect> bounds,
                             const SkPaint* paint) const {
    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this, paint);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(glyphIDs);

    if (bounds.size()) {
        const auto scaleMat = SkMatrix::Scale(strikeToSourceScale, strikeToSourceScale);
        const auto n = std::min(bounds.size(), glyphs.size());
        for (auto [bound, glyph] : SkMakeZip(bounds.first(n), glyphs.first(n))) {
            scaleMat.mapRectScaleTranslate(&bound, glyph->rect());
        }
    }

    if (widths.size()) {
        const auto n = std::min(widths.size(), glyphs.size());
        for (auto [width, glyph] : SkMakeZip(widths.first(n), glyphs.first(n))) {
            width = glyph->advanceX() * strikeToSourceScale;
        }
    }
}

void SkFont::getPos(SkSpan<const SkGlyphID> glyphIDs, SkSpan<SkPoint> pos, SkPoint origin) const {
    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(glyphIDs);

    SkPoint sum = origin;
    const auto n = std::min(pos.size(), glyphs.size());
    for (auto [position, glyph] : SkMakeZip(pos.first(n), glyphs.first(n))) {
        position = sum;
        sum += glyph->advanceVector() * strikeToSourceScale;
    }
}

void SkFont::getXPos(SkSpan<const SkGlyphID> gIDs, SkSpan<SkScalar> xpos, SkScalar origin) const {
    auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(*this);
    SkBulkGlyphMetrics metrics{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = metrics.glyphs(gIDs);

    SkScalar loc = origin;
    const auto n = std::min(xpos.size(), glyphs.size());
    for (auto [xposition, glyph] : SkMakeZip(xpos.first(n), glyphs.first(n))) {
        xposition = loc;
        loc += glyph->advanceX() * strikeToSourceScale;
    }
}

void SkFont::getPaths(SkSpan<const SkGlyphID> glyphIDs,
                      void (*proc)(const SkPath*, const SkMatrix&, void*), void* ctx) const {
    SkFont font(*this);
    SkScalar scale = font.setupForAsPaths(nullptr);
    const SkMatrix mx = SkMatrix::Scale(scale, scale);

    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(font);
    SkBulkGlyphMetricsAndPaths paths{strikeSpec};
    SkSpan<const SkGlyph*> glyphs = paths.glyphs(glyphIDs);

    for (auto glyph : glyphs) {
        proc(glyph->path(), mx, ctx);
    }
}

bool SkFont::getPath(SkGlyphID glyphID, SkPath* path) const {
    struct Pair {
        SkPath* fPath;
        bool    fWasSet;
    } pair = { path, false };

    this->getPaths({&glyphID, 1}, [](const SkPath* orig, const SkMatrix& mx, void* ctx) {
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

    SkTypeface* typeface = font.getTypeface();

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
        if (SkIsFinite(maxScaleSq) && !SkScalarNearlyZero(maxScaleSq)) {
            return font.getSize() * SkScalarSqrt(maxScaleSq);
        } else {
            return -font.getSize();
        }
    }
}

size_t SkFontPriv::CountTextElements(const void* text, size_t byteLength, SkTextEncoding encoding) {
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

    auto typeface = font.getTypeface();
    const unsigned numGlyphsInTypeface = typeface->countGlyphs();
    AutoTArray<SkUnichar> unichars(static_cast<size_t>(numGlyphsInTypeface));
    typeface->getGlyphToUnicodeMap(unichars.get());

    for (int i = 0; i < count; ++i) {
        unsigned id = glyphs[i];
        text[i] = (id < numGlyphsInTypeface) ? unichars[id] : 0xFFFD;
    }
}
