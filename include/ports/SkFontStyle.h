/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontStyle_DEFINED
#define SkFontStyle_DEFINED

#include "SkTypes.h"

class SkFontStyle {
public:
    enum Weight {
        kThin_Weight        = 100,
        kExtraLight_Weight  = 200,
        kLight_Weight       = 300,
        kNormal_Weight      = 400,
        kMedium_Weight      = 500,
        kSemiBold_Weight    = 600,
        kBold_Weight        = 700,
        kExtraBold_Weight   = 800,
        kBlack_Weight       = 900
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
        kUltaExpanded_Width     = 9
    };

    enum Slant {
        kUpright_Slant,
        kItalic_Slant,
    };

    SkFontStyle();
    SkFontStyle(int weight, int width, Slant);

    bool operator==(const SkFontStyle& rhs) const {
        return fUnion.fU32 == rhs.fUnion.fU32;
    }

    int weight() const { return fUnion.fR.fWeight; }
    int width() const { return fUnion.fR.fWidth; }

    bool isItalic() const {
        return kItalic_Slant == fUnion.fR.fSlant;
    }

private:
    union {
        struct {
            uint16_t fWeight;   // 100 .. 900
            uint8_t  fWidth;    // 1 .. 9
            uint8_t  fSlant;    // 0 .. 2
        } fR;
        uint32_t    fU32;
    } fUnion;
};

#endif
