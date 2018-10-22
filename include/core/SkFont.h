/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFont_DEFINED
#define SkFont_DEFINED

#include "SkScalar.h"
#include "SkRefCnt.h"

class SkPaint;
class SkTypeface;

enum SkTextEncoding {
    kUTF8_SkTextEncoding,
    kUTF16_SkTextEncoding,
    kUTF32_SkTextEncoding,
    kGlyphID_SkTextEncoding,
};

class SkFont {
public:
    enum Flags {
        /**
         *  Use the system's automatic hinting mechanism to hint the typeface.
         */
        kForceAutoHinting_Flag      = 1 << 0,

        /**
         *  If the typeface contains explicit bitmaps for hinting, use them.
         *  If both bytecode and auto hints are also specified, attempt to use the bitmaps first;
         *  if that fails (e.g. there are no bitmaps), then attempt to bytecode or autohint.
         */
        kEmbeddedBitmaps_Flag       = 1 << 1,

        kSubpixel_Flag              = 1 << 2,
        kLinearMetrics_Flag         = 1 << 3,

        kVertical_Flag              = 1 << 4,
        kEmbolden_Flag              = 1 << 5,

        kHinting_FlagShift          = 6,
        kHinting_FlagMask           = 3,    // 2 bits

        kDEPRECATED_Antialias_Flag  = 1 << 8,   // want to rely on paint for this
    };

    SkFont(sk_sp<SkTypeface>, SkScalar size, uint32_t flags);
    SkFont(sk_sp<SkTypeface>, SkScalar size, SkScalar scaleX, SkScalar skewX, uint32_t flags);

    /**
     *  Return a font with the same attributes of this font, but with the specified size.
     *  If size is not supported (e.g. <= 0 or non-finite) NULL will be returned.
     */
    SkFont makeWithSize(SkScalar size) const;

    /**
     *  Return a font with the same attributes of this font, but with the flags.
     */
    SkFont makeWithFlags(uint32_t newFlags) const;

    SkTypeface* getTypeface() const { return fTypeface.get(); }
    SkScalar    getSize() const { return fSize; }
    SkScalar    getScaleX() const { return fScaleX; }
    SkScalar    getSkewX() const { return fSkewX; }
    uint32_t    getFlags() const { return fFlags; }

    sk_sp<SkTypeface> refTypeface() const { return fTypeface; }

    void setTypeface(sk_sp<SkTypeface>);
    void setSize(SkScalar);
    void setScaleX(SkScalar);
    void setSkewX(SkScalar);
    void setFlags(uint32_t);

    int textToGlyphs(const void* text, size_t byteLength, SkTextEncoding,
                     SkGlyphID glyphs[], int maxGlyphCount) const;

    int countText(const void* text, size_t byteLength, SkTextEncoding encoding) {
        return this->textToGlyphs(text, byteLength, encoding, nullptr, 0);
    }

    SkScalar measureText(const void* text, size_t byteLength, SkTextEncoding) const;

    void LEGACY_applyToPaint(SkPaint*) const;
    static SkFont LEGACY_ExtractFromPaint(const SkPaint&);

private:
    static constexpr unsigned kAllFlags = 0x1FF;

    sk_sp<SkTypeface> fTypeface;
    SkScalar    fSize;
    SkScalar    fScaleX;
    SkScalar    fSkewX;
    uint32_t    fFlags;
};

#endif
