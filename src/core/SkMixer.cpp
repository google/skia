/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkMixer> SkMixer::MakeFirst() {
    return MakeBlend(SkBlendMode::kSrc);
}

sk_sp<SkMixer> SkMixer::MakeSecond() {
    return MakeBlend(SkBlendMode::kDst);
}

sk_sp<SkMixer> SkMixer::MakeReverse(sk_sp<SkMixer> proxy) {
    return sk_sp<SkMixer>(new SkMixer_Reverse(std::move(proxy)));
}

sk_sp<SkMixer> SkMixer::MakeBlend(SkBlendMode mode) {
    return sk_sp<SkMixer>(new SkMixer_Blend(mode));
}

sk_sp<SkMixer> SkMixer::MakeLerp(float t) {
    if (SkScalarIsNaN(t)) {
        t = 0;  // is some other value better? return null?
    }
    if (t <= 0) {
        return MakeFirst();
    }
    if (t >= 1) {
        return MakeSecond();
    }
    return sk_sp<SkMixer>(new SkMixer_Lerp(t));
}

sk_sp<SkMixer> SkMixer::MakeArithmetic(float k1, float k2, float k3, float k4) {
    return nullptr; // TODO
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkColorFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkMixer_Reverse);
    SK_REGISTER_FLATTENABLE(SkMixer_Blend);
    SK_REGISTER_FLATTENABLE(SkMixer_Lerp);
}
