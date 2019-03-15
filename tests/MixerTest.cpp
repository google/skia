/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMixer.h"
#include "Test.h"
#include "SkColorData.h"
#include "SkMixerBase.h"

static void check_color(skiatest::Reporter* reporter,
                        const SkPMColor4f& result, const SkPMColor4f& expected) {
    const float tol = 1/510.f; // made up
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(result[i], expected[i], tol));
    }
}

static void check_mixer(skiatest::Reporter* reporter, sk_sp<SkMixer> mixer,
                        const SkPMColor4f& a, const SkPMColor4f& b, const SkPMColor4f& expected) {
    SkPMColor4f result = as_MB(mixer)->test_mix(a, b);
    check_color(reporter, result, expected);

    auto data = mixer->serialize();
    auto m2 = SkMixerBase::Deserialize(data->data(), data->size());
    SkPMColor4f r2 = as_MB(m2)->test_mix(a, b);
    REPORTER_ASSERT(reporter, result == r2);
}

DEF_TEST(Mixer, r) {
    const SkPMColor4f transparent = {0, 0, 0, 0};
    const SkPMColor4f red = {1, 0, 0, 1};
    const SkPMColor4f magenta = {1, 0, 1, 1};
    const SkPMColor4f blue = {0, 0, 1, 1};

    auto first = SkMixer::MakeFirst();
    auto second = SkMixer::MakeSecond();
    check_mixer(r, first, red, blue, red);
    check_mixer(r, second, red, blue, blue);

    check_mixer(r, first->makeReverse(), red, blue, blue);
    check_mixer(r, second->makeReverse(), red, blue, red);

    check_mixer(r, SkMixer::MakeLerp(0), red, blue, red);
    check_mixer(r, SkMixer::MakeLerp(0.0001f), red, blue, red);
    check_mixer(r, SkMixer::MakeLerp(0.5f), red, blue, {0.5f, 0, 0.5f, 1});
    check_mixer(r, SkMixer::MakeLerp(0.9999f), red, blue, blue);
    check_mixer(r, SkMixer::MakeLerp(1), red, blue, blue);

    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kClear), red, blue, transparent);
    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kSrc), red, blue, blue);
    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kDst), red, blue, red);
    check_mixer(r, SkMixer::MakeBlend(SkBlendMode::kPlus), red, blue, magenta);

    // mx should average the results of Plus and Clear
    auto mx = SkMixer::MakeLerp(0.5f)->makeMerge(SkMixer::MakeBlend(SkBlendMode::kPlus),
                                                 SkMixer::MakeConst(0));
    check_mixer(r, mx, red, blue, {0.5f, 0, 0.5f, 0.5f});
}
