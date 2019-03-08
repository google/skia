/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMixer.h"
#include "Test.h"

static void check_color(skiatest::Reporter* reporter,
                        const SkColor4f result, const SkColor4f& expected) {
    const float tol = 2/255.0f; // made up
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[i], expected[i], tol));
    }
}

static void check_mixer(skiatest::Reporter* reporter, sk_sp<SkMixer> mixer,
                        SkColor a, SkColor b, SkColor expected) {
    const SkColor4f expected4 = SkColor4f::FromColor(expected);
    check_color(reporter, SkColor4f::FromColor(mixer->mix(a, b)), expected4);
    check_color(reporter, mixer->mix(SkColor4f::FromColor(a), SkColor4f::FromColor(b)),
                expected4);
}

DEF_TEST(Mixer, reporter) {
    check_mixer(reporter, SkMixer::MakeFirst(), SK_ColorRED, SK_ColorBLUE, SK_ColorRED);
    check_mixer(reporter, SkMixer::MakeSecond(), SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);

    check_mixer(reporter, SkMixer::MakeLerp(0), SK_ColorRED, SK_ColorBLUE, SK_ColorRED);
    check_mixer(reporter, SkMixer::MakeLerp(0.0001f), SK_ColorRED, SK_ColorBLUE, SK_ColorRED);
    check_mixer(reporter, SkMixer::MakeLerp(0.5f), SK_ColorRED, SK_ColorBLUE, 0xFF800080);
    check_mixer(reporter, SkMixer::MakeLerp(0.9999f), SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);
    check_mixer(reporter, SkMixer::MakeLerp(1), SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);
}
