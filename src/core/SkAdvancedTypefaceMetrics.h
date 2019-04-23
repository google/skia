/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAdvancedTypefaceMetrics_DEFINED
#define SkAdvancedTypefaceMetrics_DEFINED

#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/private/SkBitmaskEnum.h"

/** \class SkAdvancedTypefaceMetrics

    The SkAdvancedTypefaceMetrics class is used by the PDF backend to correctly
    embed typefaces. This class is created and filled in with information by
    SkTypeface::getAdvancedMetrics.
*/
struct SkAdvancedTypefaceMetrics {
    // The PostScript name of the font. See `FontName` and `BaseFont` in PDF standard.
    SkString fPostScriptName;
    SkString fFontName;

    // These enum values match the values used in the PDF file format.
    enum StyleFlags : uint32_t {
        kFixedPitch_Style  = 0x00000001,
        kSerif_Style       = 0x00000002,
        kScript_Style      = 0x00000008,
        kItalic_Style      = 0x00000040,
        kAllCaps_Style     = 0x00010000,
        kSmallCaps_Style   = 0x00020000,
        kForceBold_Style   = 0x00040000
    };
    StyleFlags fStyle = (StyleFlags)0;        // Font style characteristics.

    enum FontType : uint8_t {
        kType1_Font,
        kType1CID_Font,
        kCFF_Font,
        kTrueType_Font,
        kOther_Font,
    };
    // The type of the underlying font program.  This field determines which
    // of the following fields are valid.  If it is kOther_Font the per glyph
    // information will never be populated.
    FontType fType = kOther_Font;

    enum FontFlags : uint8_t {
        kMultiMaster_FontFlag    = 0x01,  //!<May be true for Type1, CFF, or TrueType fonts.
        kNotEmbeddable_FontFlag  = 0x02,  //!<May not be embedded.
        kNotSubsettable_FontFlag = 0x04,  //!<May not be subset.
    };
    FontFlags fFlags = (FontFlags)0;  // Global font flags.

    int16_t fItalicAngle = 0;  // Counterclockwise degrees from vertical of the
                               // dominant vertical stroke for an Italic face.
    // The following fields are all in font units.
    int16_t fAscent = 0;       // Max height above baseline, not including accents.
    int16_t fDescent = 0;      // Max depth below baseline (negative).
    int16_t fStemV = 0;        // Thickness of dominant vertical stem.
    int16_t fCapHeight = 0;    // Height (from baseline) of top of flat capitals.

    SkIRect fBBox = {0, 0, 0, 0};  // The bounding box of all glyphs (in font units).
};

namespace skstd {
template <> struct is_bitmask_enum<SkAdvancedTypefaceMetrics::FontFlags> : std::true_type {};
template <> struct is_bitmask_enum<SkAdvancedTypefaceMetrics::StyleFlags> : std::true_type {};
}

#endif
