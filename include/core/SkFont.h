/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFont_DEFINED
#define SkFont_DEFINED

#include "SkFontTypes.h"
#include "SkScalar.h"
#include "SkTypeface.h"

// TODO: remove this, and opt in/out per client
#define SK_SUPPORT_LEGACY_FONT_FLAGS

class SkPaint;
class SkPath;
struct SkFontMetrics;

class SK_API SkFont {
public:
#ifdef SK_SUPPORT_LEGACY_FONT_FLAGS
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
#endif

    enum class Edging {
        kAlias,
        kAntiAlias,
        kSubpixelAntiAlias,
    };

    enum Hinting : uint8_t {
        kNo_Hinting     = 0, //!< glyph outlines unchanged
        kSlight_Hinting = 1, //!< minimal modification to improve constrast
        kNormal_Hinting = 2, //!< glyph outlines modified to improve constrast
        kFull_Hinting   = 3, //!< modifies glyph outlines for maximum constrast
    };

    SkFont();
    SkFont(sk_sp<SkTypeface>, SkScalar size);
    SkFont(sk_sp<SkTypeface>, SkScalar size, SkScalar scaleX, SkScalar skewX);
#ifdef SK_SUPPORT_LEGACY_FONT_FLAGS
    SkFont(sk_sp<SkTypeface>, SkScalar size, uint32_t flags);
    SkFont(sk_sp<SkTypeface>, SkScalar size, SkScalar scaleX, SkScalar skewX, uint32_t flags);
#endif

    bool isForceAutoHinting() const { return SkToBool(fFlags & kForceAutoHinting_PrivFlag); }
    bool isEmbeddedBitmaps() const { return SkToBool(fFlags & kEmbeddedBitmaps_PrivFlag); }
    bool isSubpixel() const { return SkToBool(fFlags & kSubpixel_PrivFlag); }
    bool isLinearMetrics() const { return SkToBool(fFlags & kLinearMetrics_PrivFlag); }
    bool isEmbolden() const { return SkToBool(fFlags & kEmbolden_PrivFlag); }

    void setForceAutoHinting(bool);
    void setEmbeddedBitmaps(bool);
    void setSubpixel(bool);
    void setLinearMetrics(bool);
    void setEmbolden(bool);

    Edging getEdging() const { return (Edging)fEdging; }
    void setEdging(Edging);

    void setHinting(SkFontHinting);

#ifdef SK_SUPPORT_LEGACY_NESTED_HINTINGENUM
    Hinting getHinting() const { return (Hinting)fHinting; }
    void setHinting(Hinting hinting) {
        this->setHinting((SkFontHinting)hinting);
    }
#else
    SkFontHinting getHinting() const { return (SkFontHinting)fHinting; }
#endif

    /**
     *  Return a font with the same attributes of this font, but with the specified size.
     *  If size is not supported (e.g. <= 0 or non-finite) NULL will be returned.
     */
    SkFont makeWithSize(SkScalar size) const;

#ifdef SK_SUPPORT_LEGACY_FONT_FLAGS
    bool DEPRECATED_isAntiAlias() const { return SkToBool(fFlags & kDEPRECATED_Antialias_Flag); }
    bool DEPRECATED_isLCDRender() const { return SkToBool(fFlags & kDEPRECATED_LCDRender_Flag); }

    void DEPRECATED_setAntiAlias(bool);
    void DEPRECATED_setLCDRender(bool);

    /**
     *  Return a font with the same attributes of this font, but with the flags.
     */
    SkFont makeWithFlags(uint32_t newFlags) const;
    uint32_t    getFlags() const { return fFlags; }
    void setFlags(uint32_t);
#endif

    SkTypeface* getTypeface() const { return fTypeface.get(); }
    SkScalar    getSize() const { return fSize; }
    SkScalar    getScaleX() const { return fScaleX; }
    SkScalar    getSkewX() const { return fSkewX; }

    sk_sp<SkTypeface> refTypeface() const { return fTypeface; }

    void setTypeface(sk_sp<SkTypeface> tf) { fTypeface = tf; }
    void setSize(SkScalar);
    void setScaleX(SkScalar);
    void setSkewX(SkScalar);

    /** Converts text into glyph indices.
        Returns the number of glyph indices represented by text.
        SkTextEncoding specifies how text represents characters or glyphs.
        glyphs may be nullptr, to compute the glyph count.

        Does not check text for valid character codes or valid glyph indices.

        If byteLength equals zero, returns zero.
        If byteLength includes a partial character, the partial character is ignored.

        If SkTextEncoding is kUTF8_TextEncoding and text contains an invalid UTF-8 sequence,
        zero is returned.

        If maxGlyphCount is not sufficient to store all the glyphs, no glyphs are copied
        (but the total glyph count is returned for subsequent buffer reallocation).

        @param text          character storage encoded with SkPaint::TextEncoding
        @param byteLength    length of character storage in bytes
        @param glyphs        storage for glyph indices; may be nullptr
        @param maxGlyphCount storage capacity
        @return              number of glyphs represented by text of length byteLength
    */
    int textToGlyphs(const void* text, size_t byteLength, SkTextEncoding,
                     SkGlyphID glyphs[], int maxGlyphCount) const;

    uint16_t unicharToGlyph(SkUnichar uni) const {
        return fTypeface->unicharToGlyph(uni);
    }

    int countText(const void* text, size_t byteLength, SkTextEncoding encoding) const {
        return this->textToGlyphs(text, byteLength, encoding, nullptr, 0);
    }

    SkScalar measureText(const void* text, size_t byteLength, SkTextEncoding,
                         SkRect* bounds = nullptr) const;

    void getWidths(const uint16_t glyphs[], int count, SkScalar widths[],
                   SkRect bounds[] = nullptr) const;

    /**
     *  Returns true if the glyph has an outline (even if its empty), and sets the path.
     *  If the glyph does not have an outline (e.g. it is a bitmap), this returns false
     *  and ignores the path parameter.
     */
    bool getPath(uint16_t glyphID, SkPath* path) const;
    void getPaths(const uint16_t glyphIDs[], int count,
                  void (*GlyphPathProc)(uint16_t glyphID, const SkPath* pathOrNull, void* ctx),
                  void* ctx) const;

    SkScalar getMetrics(SkFontMetrics* metrics) const;
    SkScalar getSpacing() const { return this->getMetrics(nullptr); }

    void LEGACY_applyToPaint(SkPaint*) const;
    static SkFont LEGACY_ExtractFromPaint(const SkPaint&);

private:
    enum PrivFlags {
        kForceAutoHinting_PrivFlag      = 1 << 0,
        kEmbeddedBitmaps_PrivFlag       = 1 << 1,
        kSubpixel_PrivFlag              = 1 << 2,
        kLinearMetrics_PrivFlag         = 1 << 3,
        kEmbolden_PrivFlag              = 1 << 4,
    };

    static constexpr unsigned kAllFlags = 0x07F;

    sk_sp<SkTypeface> fTypeface;
    SkScalar    fSize;
    SkScalar    fScaleX;
    SkScalar    fSkewX;
    uint8_t     fFlags;
    uint8_t     fEdging;
    uint8_t     fHinting;

    SkScalar setupForAsPaths(SkPaint*);

    friend class SkCanonicalizeFont;
};

#endif
