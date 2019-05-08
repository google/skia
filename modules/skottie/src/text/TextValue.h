/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextValue_DEFINED
#define SkottieTextValue_DEFINED

#include "include/core/SkTypeface.h"
#include "include/utils/SkTextUtils.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/SkottieShaper.h"

namespace skottie {

struct TextValue {
    sk_sp<SkTypeface>  fTypeface;
    SkString           fText;
    float              fTextSize    = 0,
                       fStrokeWidth = 0,
                       fLineHeight  = 0;
    SkTextUtils::Align fHAlign      = SkTextUtils::kLeft_Align;
    Shaper::VAlign     fVAlign      = Shaper::VAlign::kTop;
    SkRect             fBox         = SkRect::MakeEmpty();
    SkColor            fFillColor   = SK_ColorTRANSPARENT,
                       fStrokeColor = SK_ColorTRANSPARENT;
    bool               fHasFill   : 1,
                       fHasStroke : 1;

    bool operator==(const TextValue&) const;
    bool operator!=(const TextValue&) const;
};

} // namespace skottie

#endif // SkottieTextValue_DEFINED
