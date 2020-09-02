/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontStyle_DEFINED
#define SkFontStyle_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"

class SK_API SkFontStyle {
public:
    enum Weight {
        kInvisible_Weight   =    0,
        kThin_Weight        =  100,
        kExtraLight_Weight  =  200,
        kLight_Weight       =  300,
        kNormal_Weight      =  400,
        kMedium_Weight      =  500,
        kSemiBold_Weight    =  600,
        kBold_Weight        =  700,
        kExtraBold_Weight   =  800,
        kBlack_Weight       =  900,
        kExtraBlack_Weight  = 1000,
    };

    enum Width {
        kUltraCondensed_Width   = 1,
        kExtraCondensed_Width   = 2,
        kCondensed_Width        = 3,
        kSemiCondensed_Width    = 4,
        kNormal_Width           = 5,
        kSemiExpanded_Width     = 6,
        kExpanded_Width         = 7,
        kExtraExpanded_Width    = 8,
        kUltraExpanded_Width    = 9,
    };

    enum Slant {
        kUpright_Slant,
        kItalic_Slant,
        kOblique_Slant,
    };

    constexpr SkFontStyle(int weight, int width, Slant slant)
        : fWeight(SkTPin<SkScalar>(weight, kInvisible_Weight, kExtraBlack_Weight))
        , fWidth(percentWidth_for_usWidth[SkTPin<int>(width, kUltraCondensed_Width, kUltraExpanded_Width) - 1])
        , fSlant(slant == kUpright_Slant ? 0 : 14)
        , fItalic(slant == SkFontStyle::kItalic_Slant ? 1 : 0)
        , fFixedPitch(false)
     {}

    constexpr SkFontStyle(SkScalar weight, SkScalar width, SkScalar slant, SkScalar italic, bool fixedPitch)
        : fWeight(SkTPin<SkScalar>(weight, kInvisible_Weight, kExtraBlack_Weight))
        , fWidth(SkTPin<SkScalar>(width, 0, SK_ScalarMax))
        , fSlant(SkTPin<SkScalar>(slant, -90, 90))
        , fItalic(SkTPin<SkScalar>(italic, 0, 1))
        , fFixedPitch(fixedPitch)
     {}

    constexpr SkFontStyle() : SkFontStyle{kNormal_Weight, kNormal_Width, kUpright_Slant} { }

    bool operator==(const SkFontStyle& that) const {
        return
            this->fWeight == that.fWeight &&
            this->fWidth == that.fWidth &&
            this->fSlant == that.fSlant &&
            this->fItalic == that.fItalic &&
            this->fFixedPitch == that.fFixedPitch;
    }

    /** Indicates weight with 400 being normal, 0 being invisible, 1000 completely covering. */
    SkScalar weight() const { return fWeight; }

    // Deprecated, use stretch instead.
    int width() const {
        return SkScalarRoundToInt(SkScalarInterpFunc(fWidth, percentWidth_for_usWidth, usWidths, 9));
    }

    /** Indicates percentage width with 100 being 'normal'. */
    SkScalar stretch() const { return fWidth; }

    // Deprecated, use angle and italic instead.
    Slant slant() const {
        if (0 < fItalic) { return kItalic_Slant; }
        if (fSlant != 0) { return kOblique_Slant; }
        return kUpright_Slant;
    }

    /** Indicates the overall slant. A value of 0 is upright, 14 is typical for leaning right. */
    SkScalar angle() const { return fSlant; }

    /** Indicates amount of italic-ness. A value of 0 is 'roman' and 1 'fully italic'. */
    SkScalar italic() const { return fItalic; }

    /** Indicates advances should be multiples of some minimum advance.
     *  This is a style bit, advance widths may vary even if this returns true.
     */
    bool isFixedPitch() const { return fFixedPitch; }

    static constexpr SkFontStyle Normal() {
        return SkFontStyle(kNormal_Weight, kNormal_Width, kUpright_Slant);
    }
    static constexpr SkFontStyle Bold() {
        return SkFontStyle(kBold_Weight,   kNormal_Width, kUpright_Slant);
    }
    static constexpr SkFontStyle Italic() {
        return SkFontStyle(kNormal_Weight, kNormal_Width, kItalic_Slant );
    }
    static constexpr SkFontStyle BoldItalic() {
        return SkFontStyle(kBold_Weight,   kNormal_Width, kItalic_Slant );
    }

private:
    SkScalar fWeight;
    SkScalar fWidth;
    SkScalar fSlant;
    SkScalar fItalic;
    bool fFixedPitch;

    static constexpr SkScalar usWidths[9] {
        1, 2, 3, 4, 5, 6, 7, 8, 9
    };
    static constexpr SkScalar percentWidth_for_usWidth[9] = {
        50, 62.5, 75, 87.5, 100, 112.5, 125, 150, 200,
    };
};

#endif
