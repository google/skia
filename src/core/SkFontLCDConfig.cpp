/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontLCDConfig.h"

static SkFontLCDConfig::LCDOrientation gLCDOrientation = SkFontLCDConfig::kHorizontal_LCDOrientation;
static SkFontLCDConfig::LCDOrder gLCDOrder = SkFontLCDConfig::kRGB_LCDOrder;
static SkScalar gTextGamma = SK_GAMMA_EXPONENT;
static SkScalar gTextContrast = SK_GAMMA_CONTRAST;

SkFontLCDConfig::LCDOrientation SkFontLCDConfig::GetSubpixelOrientation() {
    return gLCDOrientation;
}

void SkFontLCDConfig::SetSubpixelOrientation(LCDOrientation orientation) {
    gLCDOrientation = orientation;
}

SkFontLCDConfig::LCDOrder SkFontLCDConfig::GetSubpixelOrder() {
    return gLCDOrder;
}

void SkFontLCDConfig::SetSubpixelOrder(LCDOrder order) {
    gLCDOrder = order;
}

SkScalar SkFontLCDConfig::GetTextGamma() { return gTextGamma; }

void SkFontLCDConfig::SetTextGamma(SkScalar gamma) { gTextGamma = gamma; }

SkScalar SkFontLCDConfig::GetTextContrast() { return gTextContrast; }

void SkFontLCDConfig::SetTextContrast(SkScalar contrast) { gTextContrast = contrast; }
