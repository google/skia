/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"

const char code[] = R"(
    void main(inout half4 color) {
        float a = color.r*0.2126f + color.g*0.7152f + color.b*0.0722f;
        color.a = half(clamp(a, 0, 1.0f));
        color.rgb = 0;
    }
)";

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    auto [effect, err] = SkRuntimeEffect::Make(SkString(code));
    return effect ? effect->makeColorFilter(nullptr) : nullptr;
}
