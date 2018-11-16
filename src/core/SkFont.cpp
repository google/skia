/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDraw.h"
#include "SkFontPriv.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkScalerContext.h"
#include "SkStrikeCache.h"
#include "SkTo.h"
#include "SkTLazy.h"
#include "SkTypeface.h"
#include "SkUTF.h"

#define kDefault_Size       12
#define kDefault_Flags      0
#define kDefault_Edging     SkFont::Edging::kAntiAlias
#define kDefault_Hinting    kNormal_SkFontHinting

static inline SkScalar valid_size(SkScalar size) {
    return SkTMax<SkScalar>(0, size);
}

SkFont::SkFont(sk_sp<SkTypeface> face, SkScalar size, SkScalar scaleX, SkScalar skewX)
    : fTypeface(face ? std::move(face) : SkTypeface::MakeDefault())
    , fSize(valid_size(size))
    , fScaleX(scaleX)
    , fSkewX(skewX)
    , fFlags(kDefault_Flags)
    , fEdging(static_cast<unsigned>(kDefault_Edging))
    , fHinting(static_cast<unsigned>(kDefault_Hinting))
{}

SkFont::SkFont(sk_sp<SkTypeface> face, SkScalar size) : SkFont(std::move(face), size, 1, 0) {}

SkFont::SkFont() : SkFont(nullptr, kDefault_Size) {}

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
    constexpr uint32_t flagsToIgnore = kLinearMetrics_PrivFlag        |
                                       kEmbeddedBitmaps_PrivFlag      |
                                       kForceAutoHinting_PrivFlag;

    fFlags = (fFlags & ~flagsToIgnore) | kSubpixel_PrivFlag;
    this->setHinting(kNo_SkFontHinting);

    if (paint) {
        paint->setStyle(SkPaint::kFill_Style);
        paint->setPathEffect(nullptr);
    }
    SkScalar textSize = fSize;
    this->setSize(SkIntToScalar(SkPaint::kCanonicalTextSizeForPaths));
    return textSize / SkPaint::kCanonicalTextSizeForPaths;
}

class SkCanonicalizeFont {
public:
    SkCanonicalizeFont(const SkFont& font) : fFont(&font), fScale(0) {
        if (font.isLinearMetrics() ||
            SkDraw::ShouldDrawTextAsPaths(font, SkPaint(), SkMatrix::I()))
        {
            SkFont* f = fLazy.set(font);
            fScale = f->setupForAsPaths(nullptr);
            fFont = f;
        }
    }

    const SkFont& getFont() const { return *fFont; }
    SkScalar getScale() const { return fScale; }

private:
    const SkFont*   fFont;
    SkTLazy<SkFont> fLazy;
    SkScalar        fScale;
};


int SkFont::textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding,
                         uint16_t glyphs[], int maxGlyphCount) const {
    if (0 == byteLength) {
        return 0;
    }

    SkASSERT(text);

    int count = 0;  // fix uninitialized warning (even though the switch is complete!)

    switch (encoding) {
        case kUTF8_SkTextEncoding:
            count = SkUTF::CountUTF8((const char*)text, byteLength);
            break;
        case kUTF16_SkTextEncoding:
            count = SkUTF::CountUTF16((const uint16_t*)text, byteLength);
            break;
        case kUTF32_SkTextEncoding:
            count = SkToInt(byteLength >> 2);
            break;
        case kGlyphID_SkTextEncoding:
            count = SkToInt(byteLength >> 1);
            break;
    }
    if (!glyphs || count > maxGlyphCount) {
        return count;
    }

    // TODO: unify/eliminate SkTypeface::Encoding with SkTextEncoding
    SkTypeface::Encoding typefaceEncoding;
    switch (encoding) {
        case kUTF8_SkTextEncoding:
            typefaceEncoding = SkTypeface::kUTF8_Encoding;
            break;
        case kUTF16_SkTextEncoding:
            typefaceEncoding = SkTypeface::kUTF16_Encoding;
            break;
        case kUTF32_SkTextEncoding:
            typefaceEncoding = SkTypeface::kUTF32_Encoding;
            break;
        default:
            SkASSERT(kGlyphID_SkTextEncoding == encoding);
            // we can early exit, since we already have glyphIDs
            memcpy(glyphs, text, count << 1);
            return count;
    }

    (void)fTypeface->charsToGlyphs(text, typefaceEncoding, glyphs, count);
    return count;
}

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

SkScalar SkFont::measureText(const void* textD, size_t length, SkTextEncoding encoding,
                             SkRect* bounds) const {
    if (length == 0) {
        if (bounds) {
            bounds->setEmpty();
        }
        return 0;
    }

    SkCanonicalizeFont canon(*this);
    const SkFont& font = canon.getFont();
    SkScalar scale = canon.getScale();

    auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font);

    const char* text = static_cast<const char*>(textD);
    const char* stop = text + length;
    auto glyphCacheProc = SkFontPriv::GetGlyphCacheProc(encoding, nullptr != bounds);
    const SkGlyph* g = &glyphCacheProc(cache.get(), &text, stop);
    SkScalar width = g->fAdvanceX;

    if (nullptr == bounds) {
        while (text < stop) {
            width += glyphCacheProc(cache.get(), &text, stop).fAdvanceX;
        }
    } else {
        set_bounds(*g, bounds);
        while (text < stop) {
            g = &glyphCacheProc(cache.get(), &text, stop);
            join_bounds_x(*g, bounds, width);
            width += g->fAdvanceX;
        }
    }
    SkASSERT(text == stop);

    if (scale) {
        width *= scale;
        if (bounds) {
            bounds->fLeft *= scale;
            bounds->fTop *= scale;
            bounds->fRight *= scale;
            bounds->fBottom *= scale;
        }
    }
    return width;
}

static void set_bounds(const SkGlyph& g, SkRect* bounds, SkScalar scale) {
    bounds->set(g.fLeft * scale,
                g.fTop * scale,
                (g.fLeft + g.fWidth) * scale,
                (g.fTop + g.fHeight) * scale);
}

void SkFont::getWidths(const uint16_t glyphs[], int count, SkScalar widths[], SkRect bounds[]) const {
    if (count <= 0 || (!widths && !bounds)) {
        return;
    }

    SkCanonicalizeFont canon(*this);
    const SkFont& font = canon.getFont();
    SkScalar scale = canon.getScale();
    if (!scale) {
        scale = 1;
    }

    auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font);
    auto glyphCacheProc = SkFontPriv::GetGlyphCacheProc(kGlyphID_SkTextEncoding, nullptr != bounds);

    const char* text = reinterpret_cast<const char*>(glyphs);
    const char* stop = text + (count << 1);
    for (int i = 0; i < count; ++i) {
        const SkGlyph& g = glyphCacheProc(cache.get(), &text, stop);
        if (widths) {
            *widths++ = g.fAdvanceX * scale;
        }
        if (bounds) {
            set_bounds(g, bounds++, scale);
        }
    }
}

void SkFont::getPaths(const uint16_t glyphs[], int count,
                      void (*proc)(uint16_t, const SkPath*, void*), void* ctx) const {
    SkFont font(*this);
    SkScalar scale = font.setupForAsPaths(nullptr);

    auto exclusive = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font);
    auto cache = exclusive.get();

    for (int i = 0; i < count; ++i) {
        const SkPath* orig = cache->findPath(cache->getGlyphIDMetrics(glyphs[i]));
        if (orig && scale) {
            SkPath tmp;
            orig->transform(SkMatrix::MakeScale(scale, scale), &tmp);
            proc(glyphs[i], &tmp, ctx);
        } else {
            proc(glyphs[i], orig, ctx);
        }
    }
}

bool SkFont::getPath(uint16_t glyphID, SkPath* path) const {
    struct Pair {
        SkPath* fPath;
        bool    fWasSet;
    } pair = { path, false };

    this->getPaths(&glyphID, 1, [](uint16_t, const SkPath* orig, void* ctx) {
        Pair* pair = static_cast<Pair*>(ctx);
        if (orig) {
            *pair->fPath = *orig;
            pair->fWasSet = true;
        }
    }, &pair);
    return pair.fWasSet;
}

SkScalar SkFont::getMetrics(SkFontMetrics* metrics) const {
    SkCanonicalizeFont canon(*this);
    const SkFont& font = canon.getFont();
    SkScalar scale = canon.getScale();

    SkFontMetrics storage;
    if (nullptr == metrics) {
        metrics = &storage;
    }

    auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font);
    *metrics = cache->getFontMetrics();

    if (scale) {
        SkPaintPriv::ScaleFontMetrics(metrics, scale);
    }
    return metrics->fDescent - metrics->fAscent + metrics->fLeading;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPaint.h"

void SkFont::LEGACY_applyToPaint(SkPaint* paint) const {
    paint->setTypeface(fTypeface);
    paint->setTextSize(fSize);
    paint->setTextScaleX(fScaleX);
    paint->setTextSkewX(fSkewX);

    paint->setEmbeddedBitmapText(SkToBool(fFlags & kEmbeddedBitmaps_PrivFlag));
    paint->setFakeBoldText(SkToBool(fFlags & kEmbolden_PrivFlag));
    paint->setAutohinted(SkToBool(fFlags & kForceAutoHinting_PrivFlag));
    paint->setSubpixelText(SkToBool(fFlags & kSubpixel_PrivFlag));
    paint->setLinearText(SkToBool(fFlags & kLinearMetrics_PrivFlag));

    bool doAA = false,
         doLCD = false;
    switch (this->getEdging()) {
        case Edging::kAlias:                                        break;
        case Edging::kAntiAlias:         doAA = true;               break;
        case Edging::kSubpixelAntiAlias: doAA = true; doLCD = true; break;
    }
    paint->setAntiAlias(doAA);
    paint->setLCDRenderText(doLCD);

    paint->setHinting((SkFontHinting)this->getHinting());
}

SkFont SkFont::LEGACY_ExtractFromPaint(const SkPaint& paint) {
    uint32_t flags = 0;
    if (paint.isEmbeddedBitmapText()) {
        flags |= kEmbeddedBitmaps_PrivFlag;
    }
    if (paint.isFakeBoldText()) {
        flags |= kEmbolden_PrivFlag;
    }
    if (paint.isAutohinted()) {
        flags |= kForceAutoHinting_PrivFlag;
    }
    if (paint.isSubpixelText()) {
        flags |= kSubpixel_PrivFlag;
    }
    if (paint.isLinearText()) {
        flags |= kLinearMetrics_PrivFlag;
    }

    Edging edging = Edging::kAlias;
    if (paint.isAntiAlias()) {
        edging = Edging::kAntiAlias;
    }
    if (paint.isLCDRenderText()) {
        edging = Edging::kSubpixelAntiAlias;
    }

    SkFont font(sk_ref_sp(paint.getTypeface()), paint.getTextSize(), paint.getTextScaleX(),
                paint.getTextSkewX());
    font.fFlags = flags;
    font.setEdging(edging);
    font.setHinting((SkFontHinting)paint.getHinting());
    return font;
}
