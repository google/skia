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

DEF_TEST(Mixer, r) {
    auto first = SkMixer::MakeFirst();
    auto second = SkMixer::MakeSecond();
    check_mixer(r, first, SK_ColorRED, SK_ColorBLUE, SK_ColorRED);
    check_mixer(r, second, SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);

    check_mixer(r, first->makeReverse(), SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);
    check_mixer(r, second->makeReverse(), SK_ColorRED, SK_ColorBLUE, SK_ColorRED);

    check_mixer(r, SkMixer::MakeLerp(0), SK_ColorRED, SK_ColorBLUE, SK_ColorRED);
    check_mixer(r, SkMixer::MakeLerp(0.0001f), SK_ColorRED, SK_ColorBLUE, SK_ColorRED);
    check_mixer(r, SkMixer::MakeLerp(0.5f), SK_ColorRED, SK_ColorBLUE, 0xFF800080);
    check_mixer(r, SkMixer::MakeLerp(0.9999f), SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);
    check_mixer(r, SkMixer::MakeLerp(1), SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);

    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kClear), SK_ColorRED, SK_ColorBLUE, 0);
    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kSrc), SK_ColorRED, SK_ColorBLUE, SK_ColorRED);
    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kDst), SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE);
    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kPlus), SK_ColorRED, SK_ColorBLUE, 0xFFFF00FF);

    // mx should average the results of Plus and Clear
    auto mx = SkMixer::MakeLerp(0.5f)->makeSplit(SkMixer::MakeBlend(SkBlendMode::kPlus),
                                                 SkMixer::MakeConst(SK_ColorBLACK));
    check_mixer(r, mx, SK_ColorRED, SK_ColorBLUE, 0xFF800080);
}
