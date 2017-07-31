/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontStyle_DEFINED
#define SkFontStyle_DEFINED

#include "SkTypes.h"

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

    constexpr SkFontStyle()
        : fWeight(kNormal_Weight), fWidth(kNormal_Width), fSlant(kUpright_Slant)
    { }

    constexpr SkFontStyle(int weight, int width, Slant slant)
        : fWeight(static_cast<uint16_t>(SkTPin<int>(weight, kInvisible_Weight, kExtraBlack_Weight)))
        , fWidth (static_cast<uint8_t >(SkTPin<int>(width, kUltraCondensed_Width, kUltraExpanded_Width)))
        , fSlant (static_cast<uint8_t >(SkTPin<int>(slant, kUpright_Slant, kOblique_Slant)))
    { }

    static SkFontStyle FromOldStyle(unsigned oldStyle);

    bool operator==(const SkFontStyle& rhs) const {
        static_assert(sizeof(SkFontStyle) == sizeof(fWeight) + sizeof(fWidth) + sizeof(fSlant),
                      "SkFontStyle must be packed.");
        return memcmp(this, &rhs, sizeof(SkFontStyle)) == 0;
    }

    int weight() const { return fWeight; }
    int width() const { return fWidth; }
    Slant slant() const { return (Slant)fSlant; }

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
    uint16_t fWeight;   // 100 .. 900
    uint8_t fWidth;     // 1 .. 9
    uint8_t fSlant;     // 0 .. 2
};

#endif
