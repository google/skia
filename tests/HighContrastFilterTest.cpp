/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/effects/SkHighContrastFilter.h"
#include "tests/Test.h"

DEF_TEST(HighContrastFilter_FilterImage, reporter) {
    SkHighContrastConfig config;
    config.fInvertStyle = SkHighContrastConfig::InvertStyle::kInvertLightness;

    int w = 10, h = 10;
    SkBitmap filterResult, paintResult;

    filterResult.allocN32Pixels(w, h);
    SkCanvas canvasFilter(filterResult);
    canvasFilter.clear(0x00000000);

    paintResult.allocN32Pixels(w, h);
    SkCanvas canvasPaint(paintResult);
    canvasPaint.clear(0x00000000);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    SkRect r = SkRect::MakeLTRB(SkIntToScalar(2), SkIntToScalar(2),
                                SkIntToScalar(8), SkIntToScalar(8));
    canvasPaint.drawRect(r, paint);

    paint.setColorFilter(SkHighContrastFilter::Make(config));
    canvasFilter.drawRect(r, paint);

    for (int y = r.top(); y < r.bottom(); ++y) {
        for (int x = r.left(); x < r.right(); ++x) {
            SkColor paintColor = paintResult.getColor(x, y);
            SkColor filterColor = filterResult.getColor(x, y);
            REPORTER_ASSERT(
                reporter, filterColor ==
                paint.getColorFilter()->filterColor(paintColor));
        }
    }
}

DEF_TEST(HighContrastFilter_SmokeTest, reporter) {
    SkHighContrastConfig config;
    config.fInvertStyle = SkHighContrastConfig::InvertStyle::kInvertLightness;
    sk_sp<SkColorFilter> filter = SkHighContrastFilter::Make(config);
    REPORTER_ASSERT(reporter, filter->isAlphaUnchanged());

    SkColor white_inverted = filter->filterColor(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, white_inverted == SK_ColorBLACK);

    SkColor black_inverted = filter->filterColor(SK_ColorBLACK);
    REPORTER_ASSERT(reporter, black_inverted == SK_ColorWHITE);
}

DEF_TEST(HighContrastFilter_InvalidInputs, reporter) {
    SkHighContrastConfig config;
    REPORTER_ASSERT(reporter, config.isValid());

    // Valid invert style
    config.fInvertStyle = SkHighContrastConfig::InvertStyle::kInvertBrightness;
    REPORTER_ASSERT(reporter, config.isValid());
    config.fInvertStyle = SkHighContrastConfig::InvertStyle::kInvertLightness;
    REPORTER_ASSERT(reporter, config.isValid());
    sk_sp<SkColorFilter> filter = SkHighContrastFilter::Make(config);
    REPORTER_ASSERT(reporter, filter);

    // Invalid invert style
    config.fInvertStyle = static_cast<SkHighContrastConfig::InvertStyle>(999);
    REPORTER_ASSERT(reporter, !config.isValid());
    filter = SkHighContrastFilter::Make(config);
    REPORTER_ASSERT(reporter, !filter);

    // Valid contrast
    for (float contrast : {0.5f, +1.0f, -1.0f}) {
        config.fInvertStyle = SkHighContrastConfig::InvertStyle::kInvertBrightness;
        config.fContrast = contrast;
        REPORTER_ASSERT(reporter, config.isValid());
        filter = SkHighContrastFilter::Make(config);
        REPORTER_ASSERT(reporter, filter);
    }

    // Invalid contrast
    config.fContrast = 1.1f;
    REPORTER_ASSERT(reporter, !config.isValid());
    filter = SkHighContrastFilter::Make(config);
    REPORTER_ASSERT(reporter, !filter);
}
