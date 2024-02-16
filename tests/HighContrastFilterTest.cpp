/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/effects/SkHighContrastFilter.h"
#include "tests/Test.h"

#include <initializer_list>

DEF_TEST(HighContrastFilter_SmokeTest, reporter) {
    SkHighContrastConfig config;
    config.fInvertStyle = SkHighContrastConfig::InvertStyle::kInvertLightness;
    sk_sp<SkColorFilter> filter = SkHighContrastFilter::Make(config);
    REPORTER_ASSERT(reporter, filter->isAlphaUnchanged());

    SkColorSpace* cs = nullptr;
    SkColor4f white_inverted = filter->filterColor4f(SkColors::kWhite, cs, cs);
    REPORTER_ASSERT(reporter, white_inverted == SkColors::kBlack);

    SkColor4f black_inverted = filter->filterColor4f(SkColors::kBlack, cs, cs);
    REPORTER_ASSERT(reporter, black_inverted == SkColors::kWhite);
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
