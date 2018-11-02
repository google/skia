/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFont.h"

#include "SkTo.h"
#include "SkTypeface.h"
#include "SkUTF.h"

#define kDefault_Size       12
#define kDefault_Flags      0
#define kDefault_Hinting    SkFont::kNormal_Hinting

static inline SkScalar valid_size(SkScalar size) {
    return SkTMax<SkScalar>(0, size);
}

SkFont::SkFont(sk_sp<SkTypeface> face, SkScalar size, SkScalar scaleX, SkScalar skewX,
               uint32_t flags)
    : fTypeface(face ? std::move(face) : SkTypeface::MakeDefault())
    , fSize(valid_size(size))
    , fScaleX(scaleX)
    , fSkewX(skewX)
    , fFlags(flags & kAllFlags)
    , fHinting(kDefault_Hinting)
{}

SkFont::SkFont() : SkFont(nullptr, kDefault_Size, 1, 0, kDefault_Flags)
{}

SkFont::SkFont(sk_sp<SkTypeface> face, SkScalar size, uint32_t flags)
    : SkFont(std::move(face), size, 1, 0, flags) {}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline uint32_t set_clear_mask(uint32_t bits, bool cond, uint32_t mask) {
    return cond ? bits | mask : bits & ~mask;
}

void SkFont::setForceAutoHinting(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kForceAutoHinting_Flag);
}
void SkFont::setEmbeddedBitmaps(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kEmbeddedBitmaps_Flag);
}
void SkFont::setSubpixel(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kSubpixel_Flag);
}
void SkFont::setLinearMetrics(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kLinearMetrics_Flag);
}
void SkFont::setEmbolden(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kEmbolden_Flag);
}

void SkFont::DEPRECATED_setAntiAlias(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kDEPRECATED_Antialias_Flag);
}

void SkFont::DEPRECATED_setLCDRender(bool predicate) {
    fFlags = set_clear_mask(fFlags, predicate, kDEPRECATED_LCDRender_Flag);
}

void SkFont::setHinting(Hinting h) {
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
void SkFont::setFlags(uint32_t flags) {
    fFlags = flags & kAllFlags;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkFont SkFont::makeWithSize(SkScalar newSize) const {
    return {this->refTypeface(), newSize, this->getScaleX(), this->getSkewX(), this->getFlags()};
}

SkFont SkFont::makeWithFlags(uint32_t newFlags) const {
    return {this->refTypeface(), this->getSize(), this->getScaleX(), this->getSkewX(), newFlags};
}
///////////////////////////////////////////////////////////////////////////////////////////////////

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

SkScalar SkFont::measureText(const void* text, size_t byteLength, SkTextEncoding encoding) const {
    // TODO: need access to the cache
    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPaint.h"

void SkFont::LEGACY_applyToPaint(SkPaint* paint) const {
    paint->setTypeface(fTypeface);
    paint->setTextSize(fSize);
    paint->setTextScaleX(fScaleX);
    paint->setTextSkewX(fSkewX);

    paint->setEmbeddedBitmapText(SkToBool(fFlags & kEmbeddedBitmaps_Flag));
    paint->setFakeBoldText(SkToBool(fFlags & kEmbolden_Flag));
    paint->setAutohinted(SkToBool(fFlags & kForceAutoHinting_Flag));
    paint->setSubpixelText(SkToBool(fFlags & kSubpixel_Flag));
    paint->setLinearText(SkToBool(fFlags & kLinearMetrics_Flag));
    paint->setAntiAlias(SkToBool(fFlags & kDEPRECATED_Antialias_Flag));
    paint->setLCDRenderText(SkToBool(fFlags & kDEPRECATED_LCDRender_Flag));

    paint->setHinting((SkPaint::Hinting)this->getHinting());
}

SkFont SkFont::LEGACY_ExtractFromPaint(const SkPaint& paint) {
    uint32_t flags = 0;
    if (paint.isEmbeddedBitmapText()) {
        flags |= kEmbeddedBitmaps_Flag;
    }
    if (paint.isFakeBoldText()) {
        flags |= kEmbolden_Flag;
    }
    if (paint.isAutohinted()) {
        flags |= kForceAutoHinting_Flag;
    }
    if (paint.isSubpixelText()) {
        flags |= kSubpixel_Flag;
    }
    if (paint.isLinearText()) {
        flags |= kLinearMetrics_Flag;
    }

    if (paint.isAntiAlias()) {
        flags |= kDEPRECATED_Antialias_Flag;
    }
    if (paint.isLCDRenderText()) {
        flags |= kDEPRECATED_LCDRender_Flag;
    }

    SkFont font(sk_ref_sp(paint.getTypeface()), paint.getTextSize(), paint.getTextScaleX(),
                paint.getTextSkewX(), flags);
    font.setHinting((Hinting)paint.getHinting());
    return font;
}
