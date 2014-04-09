/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFont.h"
#include "SkTypeface.h"
#include "SkUtils.h"

static SkTypeface* ref_or_default(SkTypeface* face) {
    return face ? SkRef(face) : SkTypeface::RefDefault();
}

SkFont::SkFont(SkTypeface* face, SkScalar size, SkScalar scaleX, SkScalar skewX, MaskType mt,
               uint32_t flags)
    : fTypeface(ref_or_default(face))
    , fSize(size)
    , fScaleX(scaleX)
    , fSkewX(skewX)
    , fFlags(flags)
    , fMaskType(SkToU8(mt))
{
    SkASSERT(size > 0);
    SkASSERT(scaleX > 0);
    SkASSERT(SkScalarIsFinite(skewX));
    SkASSERT(0 == (flags & ~kAllFlags));
}

SkFont* SkFont::Create(SkTypeface* face, SkScalar size, SkScalar scaleX, SkScalar skewX,
                       MaskType mt, uint32_t flags) {
    if (size <= 0 || !SkScalarIsFinite(size)) {
        return NULL;
    }
    if (scaleX <= 0 || !SkScalarIsFinite(scaleX)) {
        return NULL;
    }
    if (!SkScalarIsFinite(skewX)) {
        return NULL;
    }
    flags &= kAllFlags;
    return SkNEW_ARGS(SkFont, (face, size, scaleX, skewX, mt, flags));
}

SkFont* SkFont::Create(SkTypeface* face, SkScalar size, MaskType mt, uint32_t flags) {
    return SkFont::Create(face, size, 1, 0, mt, flags);
}

SkFont* SkFont::cloneWithSize(SkScalar newSize) const {
    return SkFont::Create(this->getTypeface(), newSize, this->getScaleX(), this->getSkewX(),
                          this->getMaskType(), this->getFlags());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkFont::~SkFont() {
    SkSafeUnref(fTypeface);
}

int SkFont::textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding,
                         uint16_t glyphs[], int maxGlyphCount) const {
    if (0 == byteLength) {
        return 0;
    }

    SkASSERT(text);

    int count = 0;  // fix uninitialized warning (even though the switch is complete!)

    switch (encoding) {
        case kUTF8_SkTextEncoding:
            count = SkUTF8_CountUnichars((const char*)text, byteLength);
            break;
        case kUTF16_SkTextEncoding:
            count = SkUTF16_CountUnichars((const uint16_t*)text, SkToInt(byteLength >> 1));
            break;
        case kUTF32_SkTextEncoding:
            count = SkToInt(byteLength >> 2);
            break;
        case kGlyphID_SkTextEncoding:
            count = SkToInt(byteLength >> 1);
            break;
    }
    if (NULL == glyphs) {
        return count;
    }

    // TODO: unify/eliminate SkTypeface::Encoding with SkTextEncoding
    SkTypeface::Encoding typeface_encoding;
    switch (encoding) {
        case kUTF8_SkTextEncoding:
            typeface_encoding = SkTypeface::kUTF8_Encoding;
            break;
        case kUTF16_SkTextEncoding:
            typeface_encoding = SkTypeface::kUTF16_Encoding;
            break;
        case kUTF32_SkTextEncoding:
            typeface_encoding = SkTypeface::kUTF32_Encoding;
            break;
        default:
            SkASSERT(kGlyphID_SkTextEncoding == encoding);
            // we can early exit, since we already have glyphIDs
            memcpy(glyphs, text, count << 1);
            return count;
    }

    (void)fTypeface->charsToGlyphs(text, typeface_encoding, glyphs, count);
    return count;
}

SkScalar SkFont::measureText(const void* text, size_t byteLength, SkTextEncoding encoding) const {
    // TODO: need access to the cache
    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPaint.h"

SkFont* SkFont::Testing_CreateFromPaint(const SkPaint& paint) {
    uint32_t flags = 0;
    if (paint.isVerticalText()) {
        flags |= kVertical_Flag;
    }
    if (paint.isEmbeddedBitmapText()) {
        flags |= kEmbeddedBitmaps_Flag;
    }
    if (paint.getFlags() & SkPaint::kGenA8FromLCD_Flag) {
        flags |= kGenA8FromLCD_Flag;
    }
    if (paint.isFakeBoldText()) {
        flags |= kEmbolden_Flag;
    }

    if (SkPaint::kFull_Hinting == paint.getHinting()) {
        flags |= kEnableByteCodeHints_Flag;
    }
    if (paint.isAutohinted()) {
        flags |= kEnableAutoHints_Flag;
    }
    if (paint.isSubpixelText() || paint.isLinearText()) {
        // this is our default
    } else {
        flags |= kUseNonlinearMetrics_Flag;
    }

    MaskType maskType = SkFont::kBW_MaskType;
    if (paint.isAntiAlias()) {
        maskType = paint.isLCDRenderText() ? kLCD_MaskType : kA8_MaskType;
    }

    return Create(paint.getTypeface(),
                  paint.getTextSize(), paint.getTextScaleX(), paint.getTextSkewX(),
                  maskType, flags);
}
