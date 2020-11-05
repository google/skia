/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkRandom.h"
#include "tools/Resources.h"
#include "samplecode/Sample.h"

static SkScalar get_anim_sin(double secs, SkScalar amplitude, SkScalar periodInSec, SkScalar phaseInSec) {
    if (!periodInSec) {
        return 0;
    }
    double t = secs + phaseInSec;
    t *= SkScalarToFloat(2 * SK_ScalarPI) / periodInSec;
    amplitude = SK_ScalarHalf * amplitude;
    return amplitude * SkDoubleToScalar(sin(t)) + amplitude;
}

class AnimHOIFontView : public Sample {
    SkScalar fOldVariationValue = 0;
    SkScalar fVariationValue = 500;
    sk_sp<SkTypeface> fBaseTypeface;
    SkFont fFont;

    SkString name() override { return SkString("AnimHOIFont"); }

    void onOnceBeforeDraw() override {
        fBaseTypeface = MakeResourceAsTypeface("fonts/VaryAlongQuad.ttf");
        fFont.setTypeface(fBaseTypeface);
        fFont.setSize(64);
    }
    void onDrawContent(SkCanvas* canvas) override {
        if (fVariationValue != fOldVariationValue) {
            SkFontArguments args;
            SkFontArguments::VariationPosition::Coordinate coordinates[] = {
                { SkSetFourByteTag('w','g','h','t'), fVariationValue },
            };
            SkFontArguments::VariationPosition variation{coordinates, SK_ARRAY_COUNT(coordinates)};
            args.setVariationDesignPosition(variation);
            fFont.setTypeface(fBaseTypeface->makeClone(args));
            fOldVariationValue = fVariationValue;
        }

        SkPaint paint;
        canvas->drawSimpleText("abc", 3, SkTextEncoding::kUTF8, 100, 100, fFont, paint);
    }

    bool onAnimate(double nanos) override {
        fVariationValue = 1 + get_anim_sin(1e-9 * nanos, 998, 4, 2);
        return true;
    }
};
DEF_SAMPLE( return new AnimHOIFontView(); )
