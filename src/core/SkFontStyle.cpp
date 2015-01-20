/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontStyle.h"
#include "SkTypeface.h"
#include "SkTypes.h"

SkFontStyle::SkFontStyle() {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = kNormal_Weight;
    fUnion.fR.fWidth = kNormal_Width;
    fUnion.fR.fSlant = kUpright_Slant;
}

SkFontStyle::SkFontStyle(int weight, int width, Slant slant) {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = SkPin32(weight, kThin_Weight, kBlack_Weight);
    fUnion.fR.fWidth = SkPin32(width, kUltraCondensed_Width, kUltaExpanded_Width);
    fUnion.fR.fSlant = SkPin32(slant, kUpright_Slant, kItalic_Slant);
}

SkFontStyle::SkFontStyle(unsigned oldStyle) {
    fUnion.fU32 = 0;
    fUnion.fR.fWeight = (oldStyle & SkTypeface::kBold) ? SkFontStyle::kBold_Weight
                                                       : SkFontStyle::kNormal_Weight;
    fUnion.fR.fWidth = SkFontStyle::kNormal_Width;
    fUnion.fR.fSlant = (oldStyle & SkTypeface::kItalic) ? SkFontStyle::kItalic_Slant
                                                        : SkFontStyle::kUpright_Slant;
}
