/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"

const char gProgram[] = R"(
uniform half4 color0;
uniform half4 color1;
uniform half4 color2;
uniform half4 color3;
uniform half4 color4;
uniform half4 color5;

void main(inout half4 color) {
    half alpha = 255.0 * color.a;
    if (alpha < 0.5) {
        color = color0;
    } else if (alpha < 1.5) {
        color = color1;
    } else if (alpha < 2.5) {
        color = color2;
    } else if (alpha < 3.5) {
        color = color3;
    } else if (alpha < 4.5) {
        color = color4;
    } else {
        color = color5;
    }
}
)";

sk_sp<SkColorFilter> SkOverdrawColorFilter::Make(const SkPMColor array[]) {
    auto [effect, error] = SkRuntimeEffect::Make(SkString(gProgram));
    if (!effect) {
        return nullptr;
    }

    auto data = SkData::MakeUninitialized(kNumColors * sizeof(SkColor4f));
    SkPMColor4f* c = static_cast<SkPMColor4f*>(data->writable_data());
    for (int i = 0; i < kNumColors; ++i) {
        c[i] = SkPMColor4f::FromPMColor(array[i]);
    }

    return effect->makeColorFilter(std::move(data));
}
