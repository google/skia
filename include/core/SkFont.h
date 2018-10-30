/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFont_DEFINED
#define SkFont_DEFINED

#include "SkScalar.h"
#include "SkTypeface.h"

class SkPaint;

enum SkTextEncoding {
    kUTF8_SkTextEncoding,
    kUTF16_SkTextEncoding,
    kUTF32_SkTextEncoding,
    kGlyphID_SkTextEncoding,
};

class SK_API SkFont {
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
        kEmbolden_Flag              = 1 << 4,

        kDEPRECATED_Antialias_Flag  = 1 << 5,
        kDEPRECATED_LCDRender_Flag  = 1 << 6,
    };

    enum Hinting {
        kNo_Hinting     = 0, //!< glyph outlines unchanged
        kSlight_Hinting = 1, //!< minimal modification to improve constrast
        kNormal_Hinting = 2, //!< glyph outlines modified to improve constrast
        kFull_Hinting   = 3, //!< modifies glyph outlines for maximum constrast
    };

    SkFont();
    SkFont(sk_sp<SkTypeface>, SkScalar size, uint32_t flags);
    SkFont(sk_sp<SkTypeface>, SkScalar size, SkScalar scaleX, SkScalar skewX, uint32_t flags,
           int align = 0);

    bool isForceAutoHinting() const { return SkToBool(fFlags & kForceAutoHinting_Flag); }
    bool isEmbeddedBitmaps() const { return SkToBool(fFlags & kEmbeddedBitmaps_Flag); }
    bool isSubpixel() const { return SkToBool(fFlags & kSubpixel_Flag); }
    bool isLinearMetrics() const { return SkToBool(fFlags & kLinearMetrics_Flag); }
    bool isEmbolden() const { return SkToBool(fFlags & kEmbolden_Flag); }

    bool DEPRECATED_isAntiAlias() const { return SkToBool(fFlags & kDEPRECATED_Antialias_Flag); }
    bool DEPRECATED_isLCDRender() const { return SkToBool(fFlags & kDEPRECATED_LCDRender_Flag); }

    void setForceAutoHinting(bool);
    void setEmbeddedBitmaps(bool);
    void setSubpixel(bool);
    void setLinearMetrics(bool);
    void setEmbolden(bool);

    void DEPRECATED_setAntiAlias(bool);
    void DEPRECATED_setLCDRender(bool);

    Hinting getHinting() const { return (Hinting)fHinting; }
    void setHinting(Hinting);

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

    void setTypeface(sk_sp<SkTypeface> tf) { fTypeface = tf; }
    void setSize(SkScalar);
    void setScaleX(SkScalar);
    void setSkewX(SkScalar);
    void setFlags(uint32_t);

    int textToGlyphs(const void* text, size_t byteLength, SkTextEncoding,
                     SkGlyphID glyphs[], int maxGlyphCount) const;

    uint16_t unicharToGlyph(SkUnichar uni) const {
        return fTypeface->unicharToGlyph(uni);
    }

    int countText(const void* text, size_t byteLength, SkTextEncoding encoding) {
        return this->textToGlyphs(text, byteLength, encoding, nullptr, 0);
    }

    SkScalar measureText(const void* text, size_t byteLength, SkTextEncoding) const;

    void LEGACY_applyToPaint(SkPaint*) const;
    static SkFont LEGACY_ExtractFromPaint(const SkPaint&);

private:
    static constexpr unsigned kAllFlags = 0x07F;

    sk_sp<SkTypeface> fTypeface;
    SkScalar    fSize;
    SkScalar    fScaleX;
    SkScalar    fSkewX;
    uint8_t     fFlags;
    uint8_t     fAlign;
    uint8_t     fHinting;
};

#endif
