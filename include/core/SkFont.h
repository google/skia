/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFont_DEFINED
#define SkFont_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"

class SkPaint;
class SkTypeface;

enum SkTextEncoding {
    kUTF8_SkTextEncoding,
    kUTF16_SkTextEncoding,
    kUTF32_SkTextEncoding,
    kGlyphID_SkTextEncoding,
};

/*
 1. The Hinting enum in SkPaint is gone entirely, absorbed into SkFont's flags.

 2. SkPaint Flags look like this today

 enum Flags {
     kAntiAlias_Flag       = 0x01,   //!< mask to enable antialiasing
     kDither_Flag          = 0x04,   //!< mask to enable dithering
     kUnderlineText_Flag   = 0x08,   //!< mask to enable underline text
     kStrikeThruText_Flag  = 0x10,   //!< mask to enable strike-thru text
     kFakeBoldText_Flag    = 0x20,   //!< mask to enable fake-bold text
     kLinearText_Flag      = 0x40,   //!< mask to enable linear-text
     kSubpixelText_Flag    = 0x80,   //!< mask to enable subpixel text positioning
     kDevKernText_Flag     = 0x100,  //!< mask to enable device kerning text
     kLCDRenderText_Flag   = 0x200,  //!< mask to enable subpixel glyph renderering
     kEmbeddedBitmapText_Flag = 0x400, //!< mask to enable embedded bitmap strikes
     kAutoHinting_Flag     = 0x800,  //!< mask to force Freetype's autohinter
     kVerticalText_Flag    = 0x1000,
     kGenA8FromLCD_Flag    = 0x2000, // hack for GDI -- do not use if you can help it
 };

 SkFont would absorb these:

     kFakeBoldText_Flag    = 0x20,   //!< mask to enable fake-bold text
     kLinearText_Flag      = 0x40,   //!< mask to enable linear-text
     kSubpixelText_Flag    = 0x80,   //!< mask to enable subpixel text positioning
     kDevKernText_Flag     = 0x100,  //!< mask to enable device kerning text
     kLCDRenderText_Flag   = 0x200,  //!< mask to enable subpixel glyph renderering
     kEmbeddedBitmapText_Flag = 0x400, //!< mask to enable embedded bitmap strikes
     kAutoHinting_Flag     = 0x800,  //!< mask to force Freetype's autohinter
     kVerticalText_Flag    = 0x1000,
     kGenA8FromLCD_Flag    = 0x2000, // hack for GDI -- do not use if you can help it

 leaving these still in paint

     kAntiAlias_Flag       = 0x01,   //!< mask to enable antialiasing
     kDither_Flag          = 0x04,   //!< mask to enable dithering
     kUnderlineText_Flag   = 0x08,   //!< mask to enable underline text
     kStrikeThruText_Flag  = 0x10,   //!< mask to enable strike-thru text

 3. Antialiasing

    SkFont has a mask-type: BW, AA, LCD
    SkPaint has antialias boolean

    What to do if the font's mask-type disagrees with the paint?

 */

class SkFont : public SkRefCnt {
public:
    enum Flags {
        /**
         *  Use the system's automatic hinting mechanism to hint the typeface.
         *  This is a last resort hinting method applied only if other hinting methods do not apply.
         *  TODO: where to put auto-normal vs auto-light?
         */
        kEnableAutoHints_Flag       = 1 << 0,

        /**
         *  If the typeface contains explicit bytecodes for hinting, use them.
         *  If both bytecode and auto hints are specified, attempt to use the bytecodes first;
         *  if that fails (e.g. there are no codes), then attempt to autohint.
         */
        kEnableByteCodeHints_Flag   = 1 << 1,

        /**
         *  If the typeface contains explicit bitmaps for hinting, use them.
         *  If both bytecode and auto hints are also specified, attempt to use the bitmaps first;
         *  if that fails (e.g. there are no bitmaps), then attempt to bytecode or autohint.
         */
        kEmbeddedBitmaps_Flag       = 1 << 2,

        /**
         *  Use rounded metric values (e.g. advance).
         *  If either auto or bytecode hinting was used, apply those results to the metrics of the
         *  glyphs as well. If no hinting was applied, the metrics will just be rounded to the
         *  nearest integer.
         *
         *  This applies to calls that return metrics (e.g. measureText) and to drawing the glyphs
         *  (see SkCanvas drawText and drawPosText).
         */
        kUseNonlinearMetrics_Flag   = 1 << 3,

        kVertical_Flag              = 1 << 4,
        kGenA8FromLCD_Flag          = 1 << 5,
        kEmbolden_Flag              = 1 << 6,
        kDevKern_Flag               = 1 << 7,   // ifdef ANDROID ?
    };

    enum MaskType {
        kBW_MaskType,
        kA8_MaskType,
        kLCD_MaskType,
    };

    static sk_sp<SkFont> Make(sk_sp<SkTypeface>, SkScalar size, MaskType, uint32_t flags);
    static sk_sp<SkFont> Make(sk_sp<SkTypeface>, SkScalar size, SkScalar scaleX, SkScalar skewX,
                              MaskType, uint32_t flags);

    /**
     *  Return a font with the same attributes of this font, but with the specified size.
     *  If size is not supported (e.g. <= 0 or non-finite) NULL will be returned.
     */
    sk_sp<SkFont> makeWithSize(SkScalar size) const;
    /**
     *  Return a font with the same attributes of this font, but with the flags.
     */
    sk_sp<SkFont> makeWithFlags(uint32_t newFlags) const;

    SkTypeface* getTypeface() const { return fTypeface.get(); }
    SkScalar    getSize() const { return fSize; }
    SkScalar    getScaleX() const { return fScaleX; }
    SkScalar    getSkewX() const { return fSkewX; }
    uint32_t    getFlags() const { return fFlags; }
    MaskType    getMaskType() const { return (MaskType)fMaskType; }

    bool isVertical() const { return SkToBool(fFlags & kVertical_Flag); }
    bool isEmbolden() const { return SkToBool(fFlags & kEmbolden_Flag); }
    bool isEnableAutoHints() const { return SkToBool(fFlags & kEnableAutoHints_Flag); }
    bool isEnableByteCodeHints() const { return SkToBool(fFlags & kEnableByteCodeHints_Flag); }
    bool isUseNonLinearMetrics() const { return SkToBool(fFlags & kUseNonlinearMetrics_Flag); }
    bool isDevKern() const { return SkToBool(fFlags & kDevKern_Flag); }

    int textToGlyphs(const void* text, size_t byteLength, SkTextEncoding,
                     SkGlyphID glyphs[], int maxGlyphCount) const;

    int countText(const void* text, size_t byteLength, SkTextEncoding encoding) {
        return this->textToGlyphs(text, byteLength, encoding, nullptr, 0);
    }

    SkScalar measureText(const void* text, size_t byteLength, SkTextEncoding) const;

    static sk_sp<SkFont> Testing_CreateFromPaint(const SkPaint&);

private:
    enum {
        kAllFlags = 0xFF,
    };

    SkFont(sk_sp<SkTypeface>, SkScalar size, SkScalar scaleX, SkScalar skewX, MaskType,
           uint32_t flags);

    sk_sp<SkTypeface> fTypeface;
    SkScalar    fSize;
    SkScalar    fScaleX;
    SkScalar    fSkewX;
    uint16_t    fFlags;
    uint8_t     fMaskType;
//  uint8_t     fPad;
};

#endif
