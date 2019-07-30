/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextPropertyValue_DEFINED
#define SkottieTextPropertyValue_DEFINED

#include "include/core/SkTypeface.h"
#include "include/utils/SkTextUtils.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/SkottieShaper.h"

namespace skottie {

struct TextPropertyValue {
    sk_sp<SkTypeface>  fTypeface;
    SkString           fText;
    float              fTextSize    = 0,
                       fStrokeWidth = 0,
                       fLineHeight  = 0,
                       fAscent      = 0;
    SkTextUtils::Align fHAlign      = SkTextUtils::kLeft_Align;
    Shaper::VAlign     fVAlign      = Shaper::VAlign::kTop;
    SkRect             fBox         = SkRect::MakeEmpty();
    SkColor            fFillColor   = SK_ColorTRANSPARENT,
                       fStrokeColor = SK_ColorTRANSPARENT;
    bool               fHasFill   : 1,
                       fHasStroke : 1;

    bool operator==(const TextPropertyValue& other) const {
        return fTypeface == other.fTypeface
            && fText == other.fText
            && fTextSize == other.fTextSize
            && fStrokeWidth == other.fStrokeWidth
            && fLineHeight == other.fLineHeight
            && fHAlign == other.fHAlign
            && fVAlign == other.fVAlign
            && fBox == other.fBox
            && fFillColor == other.fFillColor
            && fStrokeColor == other.fStrokeColor
            && fHasFill == other.fHasFill
            && fHasStroke == other.fHasStroke;
    }

    bool operator!=(const TextPropertyValue& other) const {
        return !(*this== other);
    }
};

} // namespace skottie

#endif // SkottieTextPropertyValue_DEFINED
