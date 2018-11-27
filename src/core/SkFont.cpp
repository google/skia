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
#include "SkUtils.h"

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

bool SkFont::operator==(const SkFont& b) const {
    return  fTypeface.get() == b.fTypeface.get() &&
            fSize           == b.fSize &&
            fScaleX         == b.fScaleX &&
            fSkewX          == b.fSkewX &&
            fFlags          == b.fFlags &&
            fEdging         == b.fEdging &&
            fHinting        == b.fHinting;
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

    if (this->getEdging() == Edging::kSubpixelAntiAlias) {
        this->setEdging(Edging::kAntiAlias);
    }

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

    (void)SkFontPriv::GetTypefaceOrDefault(*this)->charsToGlyphs(text, typefaceEncoding, glyphs,
                                                                 count);
    return count;
}

static SkTypeface::Encoding to_encoding(SkTextEncoding e) {
    static_assert((int)SkTypeface::kUTF8_Encoding  == (int)kUTF8_SkTextEncoding,  "");
    static_assert((int)SkTypeface::kUTF16_Encoding == (int)kUTF16_SkTextEncoding, "");
    static_assert((int)SkTypeface::kUTF32_Encoding == (int)kUTF32_SkTextEncoding, "");
    return (SkTypeface::Encoding)e;
}

bool SkFont::containsText(const void* textData, size_t byteLength, SkTextEncoding textEnc) const {
    if (0 == byteLength) {
        return true;
    }

    SkASSERT(textData != nullptr);

    // handle this encoding before the setup for the glyphcache
    if (textEnc == kGlyphID_SkTextEncoding) {
        const uint16_t* glyphID = static_cast<const uint16_t*>(textData);
        size_t count = byteLength >> 1;
        for (size_t i = 0; i < count; i++) {
            if (0 == glyphID[i]) {
                return false;
            }
        }
        return true;
    }

    auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(*this);
    const void* stop = (const char*)textData + byteLength;
    const SkTypeface::Encoding encoding = to_encoding(textEnc);
    while (textData < stop) {
        if (0 == cache->unicharToGlyph(SkUTFN_Next(encoding, &textData, stop))) {
            return false;
        }
    }
    return true;
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

static SkRect make_bounds(const SkGlyph& g, SkScalar scale) {
    return {
        g.fLeft * scale,
        g.fTop * scale,
        (g.fLeft + g.fWidth) * scale,
        (g.fTop + g.fHeight) * scale
    };
}

template <typename HANDLER>
void VisitGlyphs(const SkFont& origFont, const SkPaint* paint, const uint16_t glyphs[], int count,
                 HANDLER handler) {
    if (count <= 0) {
        return;
    }

    SkCanonicalizeFont canon(origFont);
    const SkFont& font = canon.getFont();
    SkScalar scale = canon.getScale();
    if (!scale) {
        scale = 1;
    }

    auto cache = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font,
                                                                        paint ? *paint : SkPaint());
    handler(cache.get(), glyphs, count, scale);
}

void SkFont::getWidthsBounds(const uint16_t glyphs[], int count, SkScalar widths[], SkRect bounds[],
                             const SkPaint* paint) const {
    VisitGlyphs(*this, paint, glyphs, count, [widths, bounds]
                (SkGlyphCache* cache, const uint16_t glyphs[], int count, SkScalar scale) {
        for (int i = 0; i < count; ++i) {
            const SkGlyph* g;
            if (bounds) {
                g = &cache->getGlyphIDMetrics(glyphs[i]);
                bounds[i] = make_bounds(*g, scale);
            } else {
                g = &cache->getGlyphIDAdvance(glyphs[i]);
            }
            if (widths) {
                widths[i] = g->fAdvanceX * scale;
            }
        }
    });
}

void SkFont::getPos(const uint16_t glyphs[], int count, SkPoint pos[], SkPoint origin) const {
    VisitGlyphs(*this, nullptr, glyphs, count, [pos, origin]
                      (SkGlyphCache* cache, const uint16_t glyphs[], int count, SkScalar scale) {
        SkPoint loc = origin;
        for (int i = 0; i < count; ++i) {
            pos[i] = loc;
            loc.fX += cache->getGlyphIDAdvance(glyphs[i]).fAdvanceX * scale;
        }
    });
}

void SkFont::getXPos(const uint16_t glyphs[], int count, SkScalar xpos[], SkScalar origin) const {
    VisitGlyphs(*this, nullptr, glyphs, count, [xpos, origin]
                      (SkGlyphCache* cache, const uint16_t glyphs[], int count, SkScalar scale) {
        SkScalar x = origin;
        for (int i = 0; i < count; ++i) {
            xpos[i] = x;
            x += cache->getGlyphIDAdvance(glyphs[i]).fAdvanceX * scale;
        }
    });
}

void SkFont::getPaths(const uint16_t glyphs[], int count,
                      void (*proc)(const SkPath*, const SkMatrix&, void*), void* ctx) const {
    SkFont font(*this);
    SkScalar scale = font.setupForAsPaths(nullptr);
    if (!scale) {
        scale = 1;
    }
    const SkMatrix mx = SkMatrix::MakeScale(scale, scale);

    auto exclusive = SkStrikeCache::FindOrCreateStrikeWithNoDeviceExclusive(font);
    auto cache = exclusive.get();

    for (int i = 0; i < count; ++i) {
        proc(cache->findPath(cache->getGlyphIDMetrics(glyphs[i])), mx, ctx);
    }
}

bool SkFont::getPath(uint16_t glyphID, SkPath* path) const {
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
    SkFont font(sk_ref_sp(paint.getTypeface()), paint.getTextSize(), paint.getTextScaleX(),
                paint.getTextSkewX());
    font.LEGACY_applyPaintFlags(paint.getFlags());
    font.setHinting((SkFontHinting)paint.getHinting());
    return font;
}

void SkFont::LEGACY_applyPaintFlags(uint32_t paintFlags) {
    uint32_t flags = 0;
    if (paintFlags & SkPaint::kEmbeddedBitmapText_Flag) {
        flags |= kEmbeddedBitmaps_PrivFlag;
    }
    if (paintFlags & SkPaint::kFakeBoldText_Flag) {
        flags |= kEmbolden_PrivFlag;
    }
    if (paintFlags & SkPaint::kAutoHinting_Flag) {
        flags |= kForceAutoHinting_PrivFlag;
    }
    if (paintFlags & SkPaint::kSubpixelText_Flag) {
        flags |= kSubpixel_PrivFlag;
    }
    if (paintFlags & SkPaint::kLinearText_Flag) {
        flags |= kLinearMetrics_PrivFlag;
    }
    fFlags = flags;

    Edging edging = Edging::kAlias;
    if (paintFlags & SkPaint::kAntiAlias_Flag) {
        edging = Edging::kAntiAlias;
        if (paintFlags & SkPaint::kLCDRenderText_Flag) {
            edging = Edging::kSubpixelAntiAlias;
        }
    }
    this->setEdging(edging);
}
